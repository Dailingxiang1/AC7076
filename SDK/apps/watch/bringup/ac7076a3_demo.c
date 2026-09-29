#include "app_config.h"
#include "system/includes.h"
#include "system/timer.h"
#include "asm/includes.h"
#include "gpio.h"
#include "gpadc.h"
#include "iic_soft.h"
#include "clock.h"
#include "asm/power/power_gate.h"
#include "ac7076a3_demo.h"

#if AC7076A3_DEMO_ENABLE
#define LOG(fmt, ...) printf("[BRINGUP] " fmt "\n", ##__VA_ARGS__)

#if DEMO_I2C_ENABLE
/* The exclusive demo task owns this bus; stock UI/sensor tasks never start. */
static const soft_iic_dev demo_iic = 0;
static struct iic_master_config demo_iic_cfg = {
    .role = IIC_MASTER,
    .scl_io = DEMO_SCL_PIN,
    .sda_io = DEMO_SDA_PIN,
    .io_mode = PORT_INPUT_PULLUP_10K,
    .hdrive = PORT_DRIVE_STRENGT_2p4mA,
    .master_frequency = 50000, /* SDK software driver ignores this field. */
};
static int bus_ok;
static int bus_initialized;
#if DEMO_I2C_GPIO_ENABLE
static u8 gpio_i2c_fault;
static u8 gpio_i2c_stage, gpio_i2c_bit = 0xff, gpio_i2c_addr;
static int gpio_i2c_mode_return;
static struct demo_i2c_fault gpio_i2c_first_fault;

static void gpio_i2c_record_fault(u8 code)
{
    gpio_i2c_fault = code;
    if (gpio_i2c_first_fault.code) {
        return; /* Preserve the first failure before STOP/recovery changes IO. */
    }
    gpio_i2c_first_fault.code = code;
    gpio_i2c_first_fault.stage = gpio_i2c_stage;
    gpio_i2c_first_fault.bit = gpio_i2c_bit;
    gpio_i2c_first_fault.addr = gpio_i2c_addr;
    gpio_i2c_first_fault.mode_return = gpio_i2c_mode_return;
    gpio_i2c_first_fault.api[0] = gpio_read(DEMO_SDA_PIN);
    gpio_i2c_first_fault.api[1] = gpio_read(DEMO_SCL_PIN);
    gpio_i2c_first_fault.raw = (JL_PORTB->IN >> 1) & 3;
    gpio_i2c_first_fault.dir = (JL_PORTB->DIR >> 1) & 3;
    gpio_i2c_first_fault.die = (JL_PORTB->DIE >> 1) & 3;
    gpio_i2c_first_fault.dieh = (JL_PORTB->DIEH >> 1) & 3;
}

static int gpio_i2c_sda(u8 high)
{
#if DEMO_I2C_DIR_ONLY
    int ret = gpio_hw_set_direction(IO_PORT_SPILT(DEMO_SDA_PIN), high);
#else
    int ret = gpio_set_mode(IO_PORT_SPILT(DEMO_SDA_PIN), high ?
                           PORT_INPUT_PULLUP_10K : PORT_OUTPUT_LOW);
#endif
    if (ret < 0) {
        gpio_i2c_mode_return = ret;
        gpio_i2c_record_fault(5);
    }
    return ret < 0 ? -1 : 0;
}

static int gpio_i2c_scl_low(void)
{
#if DEMO_I2C_DIR_ONLY
    int ret = gpio_hw_set_direction(IO_PORT_SPILT(DEMO_SCL_PIN), 0);
#else
    int ret = gpio_set_mode(IO_PORT_SPILT(DEMO_SCL_PIN), PORT_OUTPUT_LOW);
#endif
    if (ret < 0) {
        gpio_i2c_mode_return = ret;
        gpio_i2c_record_fault(5);
    }
    return ret < 0 ? -1 : 0;
}

static int gpio_i2c_scl_release(void)
{
#if DEMO_I2C_DIR_ONLY
    return gpio_hw_set_direction(IO_PORT_SPILT(DEMO_SCL_PIN), 1);
#else
    return gpio_set_mode(IO_PORT_SPILT(DEMO_SCL_PIN), PORT_INPUT_PULLUP_10K);
#endif
}

static void gpio_i2c_release(void)
{
    gpio_i2c_sda(1);
    gpio_i2c_scl_release();
    udelay(DEMO_I2C_HALF_PERIOD_US);
}

static int gpio_i2c_scl_high(void)
{
    gpio_i2c_mode_return = gpio_i2c_scl_release();
    if (gpio_i2c_mode_return < 0) {
        gpio_i2c_record_fault(5);
        return -1;
    }
    /* Wait for rise/clock stretching instead of assuming SCL is already high. */
    for (unsigned int us = 0; us < DEMO_I2C_SCL_TIMEOUT_US; us += 5) {
        if (gpio_read(DEMO_SCL_PIN) == 1) {
            udelay(DEMO_I2C_HALF_PERIOD_US);
            return 0;
        }
        udelay(5);
    }
    gpio_i2c_record_fault(1);
    return -1;
}

static int gpio_i2c_start(void)
{
    gpio_i2c_stage = 1;
    gpio_i2c_bit = 0xff;
    if (gpio_i2c_sda(1)) {
        gpio_i2c_release();
        return -1;
    }
    udelay(DEMO_I2C_HALF_PERIOD_US);
    if (gpio_i2c_scl_high()) {
        gpio_i2c_release();
        return -1;
    }
    if (gpio_read(DEMO_SDA_PIN) != 1) {
        gpio_i2c_record_fault(2);
        gpio_i2c_release();
        return -1;
    }
    if (gpio_i2c_sda(0)) {
        gpio_i2c_release();
        return -1;
    }
    udelay(DEMO_I2C_HALF_PERIOD_US);
    if (gpio_i2c_scl_low()) {
        gpio_i2c_release();
        return -1;
    }
    return 0;
}

static int gpio_i2c_stop(void)
{
    gpio_i2c_stage = 6;
    gpio_i2c_bit = 0xff;
    if (gpio_i2c_scl_low() || gpio_i2c_sda(0)) {
        gpio_i2c_release();
        return -1;
    }
    udelay(DEMO_I2C_HALF_PERIOD_US);
    if (gpio_i2c_scl_high()) {
        gpio_i2c_release(); /* Release both MCU outputs on every error path. */
        return -1;
    }
    if (gpio_i2c_sda(1)) {
        gpio_i2c_release();
        return -1;
    }
    udelay(DEMO_I2C_HALF_PERIOD_US);
    if (gpio_read(DEMO_SCL_PIN) != 1 || gpio_read(DEMO_SDA_PIN) != 1) {
        gpio_i2c_record_fault(3);
        return -1;
    }
    return 0;
}

/* Return 1=ACK, 0=NACK, -1=clock/bus error. Only drive low or release. */
static int gpio_i2c_tx(u8 byte)
{
    for (int bit = 7; bit >= 0; --bit) {
        gpio_i2c_stage = 2;
        gpio_i2c_bit = bit;
        if (gpio_i2c_sda((byte >> bit) & 1)) {
            gpio_i2c_release();
            return -1;
        }
        udelay(DEMO_I2C_HALF_PERIOD_US);
        if (gpio_i2c_scl_high()) {
            gpio_i2c_release();
            return -1;
        }
        if (gpio_i2c_scl_low()) {
            gpio_i2c_release();
            return -1;
        }
    }
    if (gpio_i2c_sda(1)) {
        gpio_i2c_release();
        return -1;
    }
    gpio_i2c_stage = 3;
    gpio_i2c_bit = 0xff;
    udelay(DEMO_I2C_HALF_PERIOD_US);
    if (gpio_i2c_scl_high()) {
        gpio_i2c_release();
        return -1;
    }
    int ack = gpio_read(DEMO_SDA_PIN) == 0;
    if (gpio_i2c_scl_low()) {
        gpio_i2c_release();
        return -1;
    }
    return ack;
}

static int gpio_i2c_rx(u8 *byte, u8 ack)
{
    u8 value = 0;
    if (gpio_i2c_sda(1)) {
        gpio_i2c_release();
        return -1;
    }
    for (int bit = 0; bit < 8; ++bit) {
        gpio_i2c_stage = 4;
        gpio_i2c_bit = 7 - bit;
        udelay(DEMO_I2C_HALF_PERIOD_US);
        if (gpio_i2c_scl_high()) {
            gpio_i2c_release();
            return -1;
        }
        value = (value << 1) | (gpio_read(DEMO_SDA_PIN) == 1);
        if (gpio_i2c_scl_low()) {
            gpio_i2c_release();
            return -1;
        }
    }
    if (gpio_i2c_sda(!ack)) {
        gpio_i2c_release();
        return -1;
    }
    gpio_i2c_stage = 5;
    gpio_i2c_bit = 0xff;
    udelay(DEMO_I2C_HALF_PERIOD_US);
    if (gpio_i2c_scl_high()) {
        gpio_i2c_release();
        return -1;
    }
    if (gpio_i2c_scl_low() || gpio_i2c_sda(1)) {
        gpio_i2c_release();
        return -1;
    }
    *byte = value;
    return 0;
}

static int gpio_i2c_reg_read(u8 addr, u8 reg, u8 *data, int len)
{
    gpio_i2c_addr = addr;
    int ret = -1;
    if (gpio_i2c_start() || gpio_i2c_tx(addr << 1) != 1 ||
        gpio_i2c_tx(reg) != 1 || gpio_i2c_start() ||
        gpio_i2c_tx((addr << 1) | 1) != 1) {
        goto out;
    }
    for (int i = 0; i < len; ++i) {
        if (gpio_i2c_rx(&data[i], i + 1 < len)) {
            goto out;
        }
    }
    ret = 0;
out:
    if (gpio_i2c_stop()) {
        ret = -1;
    }
    return ret;
}

static int gpio_i2c_reg_write(u8 addr, u8 reg, u8 value)
{
    gpio_i2c_addr = addr;
    int ret = -1;
    if (!gpio_i2c_start() && gpio_i2c_tx(addr << 1) == 1 &&
        gpio_i2c_tx(reg) == 1 && gpio_i2c_tx(value) == 1) {
        ret = 0;
    }
    if (gpio_i2c_stop()) {
        ret = -1;
    }
    return ret;
}

static int gpio_i2c_probe(u8 addr, u8 read)
{
    gpio_i2c_addr = addr;
    int ret = -1;
    u8 dummy;
    if (!gpio_i2c_start()) {
        ret = gpio_i2c_tx((addr << 1) | read);
        if (ret == 1 && read && gpio_i2c_rx(&dummy, 0)) {
            ret = -1;
        }
    }
    if (gpio_i2c_stop()) {
        ret = -1;
    }
    return ret;
}
#endif
#if DEMO_I2C_SCREEN_ENABLE
static OS_MUTEX scan_mutex;
static int scan_mutex_ready;
static struct demo_i2c_sample scan_sample;
static u8 scan_cursor = 0x08;
static u32 scan_next_at;

void ac7076a3_demo_i2c_snapshot(struct demo_i2c_sample *sample)
{
    memset(sample, 0, sizeof(*sample));
    if (!scan_mutex_ready) {
        return;
    }
    os_mutex_pend(&scan_mutex, 0);
    *sample = scan_sample;
    os_mutex_post(&scan_mutex);
}

static void scan_bus_status(void)
{
    if (!scan_mutex_ready) {
        return;
    }
    os_mutex_pend(&scan_mutex, 0);
    scan_sample.bus_ready = bus_ok;
    scan_sample.sda = gpio_read(DEMO_SDA_PIN) ? 1 : 0;
    scan_sample.scl = gpio_read(DEMO_SCL_PIN) ? 1 : 0;
    scan_sample.irq = gpio_read(DEMO_TP_IRQ_PIN) ? 1 : 0;
#if DEMO_I2C_GPIO_ENABLE
    scan_sample.bus_fault = gpio_i2c_fault;
    scan_sample.first_fault = gpio_i2c_first_fault;
#endif
#if DEMO_I2C_PIN_TEST || DEMO_I2C_DIR_ONLY
    /* Live transaction-boundary samples; preserve API error returns. */
    scan_sample.pin_api[0] = gpio_read(DEMO_SDA_PIN);
    scan_sample.pin_api[1] = gpio_read(DEMO_SCL_PIN);
    scan_sample.pin_raw = (JL_PORTB->IN >> 1) & 3;
    scan_sample.pin_dir = (JL_PORTB->DIR >> 1) & 3;
    scan_sample.pin_die = (JL_PORTB->DIE >> 1) & 3;
    scan_sample.pin_dieh = (JL_PORTB->DIEH >> 1) & 3;
    scan_sample.pin_out = (JL_PORTB->OUT >> 1) & 3;
#endif
    os_mutex_post(&scan_mutex);
}

/* Address phase only for write probes. Read probes consume one byte with
 * NACK before STOP; no register/configuration bytes are written to unknown ICs.
 */
static int scan_address(u8 addr, u8 read)
{
#if DEMO_I2C_GPIO_ENABLE
    return gpio_i2c_probe(addr, read);
#else
    if (soft_iic_check_busy(demo_iic) != IIC_OK) {
        return -1;
    }
    if (soft_iic_start(demo_iic) != IIC_OK) {
        soft_iic_stop(demo_iic);
        return -1;
    }
    u8 ack = soft_iic_tx_byte(demo_iic, (addr << 1) | read);
    if (ack && read) {
        soft_iic_rx_byte(demo_iic, 0, NULL);
    }
    soft_iic_stop(demo_iic); /* Also releases SDK software bus busy state. */
    return ack ? 1 : 0;
#endif
}
#endif

static int reg_read(u8 addr, u8 reg, u8 *data, int len)
{
#if DEMO_I2C_GPIO_ENABLE
    return gpio_i2c_reg_read(addr, reg, data, len);
#else
    int ret = soft_i2c_master_read_nbytes_from_device_reg(demo_iic,
              addr << 1, &reg, 1, data, len);
    return ret == len ? 0 : -1;
#endif
}

#if DEMO_ACCEL_ENABLE
static int reg_update(u8 addr, u8 reg, u8 mask, u8 value)
{
    u8 old;
    if (reg_read(addr, reg, &old, 1)) {
        return -1;
    }
    old = (old & ~mask) | (value & mask);
#if DEMO_I2C_GPIO_ENABLE
    if (gpio_i2c_reg_write(addr, reg, old)) {
        return -1;
    }
#else
    if (soft_i2c_master_write_nbytes_to_device_reg(demo_iic, addr << 1,
            &reg, 1, &old, 1) != 1) {
        return -1;
    }
#endif
    if (reg_read(addr, reg, &old, 1) || (old & mask) != (value & mask)) {
        return -1;
    }
    return 0;
}
#endif

static int bus_init(void)
{
    int scl_ret = gpio_set_mode(IO_PORT_SPILT(DEMO_SCL_PIN), PORT_INPUT_PULLUP_10K);
    int sda_ret = gpio_set_mode(IO_PORT_SPILT(DEMO_SDA_PIN), PORT_INPUT_PULLUP_10K);
    if (scl_ret < 0 || sda_ret < 0) {
        return -1;
    }
#if DEMO_I2C_DIR_ONLY
    /* OUT is permanently zero; DIR=0 pulls low, DIR=1 releases to pullups.
     * Keep digital input buffers enabled even while driving low. SDK HW
     * helpers update only the selected pins, preserving PB0 LED/PB8 LCD. */
    u32 pins = BIT(DEMO_SDA_PIN % 16) | BIT(DEMO_SCL_PIN % 16);
    if (gpio_hw_set_output_value(PORTB, pins, 0) < 0 ||
        gpio_hw_set_die(PORTB, pins, 1) < 0 ||
        gpio_hw_set_dieh(PORTB, pins, 1) < 0) {
        gpio_i2c_mode_return = -1;
        gpio_i2c_record_fault(5);
        return -1;
    }
#endif
    os_time_dly(1);
#if DEMO_I2C_PIN_TEST
    /* No SDK I2C init and no START/address/STOP in this profile. */
    return 0;
#endif
    if (!gpio_read(DEMO_SCL_PIN) || !gpio_read(DEMO_SDA_PIN)) {
        LOG("I2C BUS LOW: check PB2/PB1, R3/R4 and 3V3; tests skipped");
        return -1;
    }
#if DEMO_I2C_GPIO_ENABLE
    gpio_i2c_release();
    LOG("I2C GPIO backend PB1/PB2: low/high delay=%uus, SCL timeout=%uus",
        DEMO_I2C_HALF_PERIOD_US, DEMO_I2C_SCL_TIMEOUT_US);
    return 0;
#else
    /* A line-low retry must not reinitialize an already owned SDK bus:
     * soft_iic_init would reject it as occupied even after lines recovered.
     */
    if (bus_initialized) {
        return 0;
    }
    int ret = soft_iic_init(demo_iic, &demo_iic_cfg);
    bus_initialized = ret == IIC_OK;
    LOG("I2C init=%d SDA=PB1 SCL=PB2; SDK software timing unverified", ret);
    return ret == IIC_OK ? 0 : -1;
#endif
}

#if DEMO_I2C_SCAN_ENABLE && !DEMO_I2C_SCREEN_ENABLE
static void bus_scan(void)
{
    int count = 0;
    for (u8 addr = 0x08; addr <= 0x77; ++addr) {
#if DEMO_I2C_GPIO_ENABLE
        int ack = gpio_i2c_probe(addr, 0);
        if (ack < 0) {
            LOG("I2C scan stopped: GPIO bus error");
            break;
        }
#else
        if (soft_iic_check_busy(demo_iic) != IIC_OK) {
            LOG("I2C scan stopped: bus busy");
            break;
        }
        if (soft_iic_start(demo_iic) != IIC_OK) {
            soft_iic_stop(demo_iic);
            break;
        }
        u8 ack = soft_iic_tx_byte(demo_iic, addr << 1);
        soft_iic_stop(demo_iic);
#endif
        if (ack) {
            LOG("I2C ACK address7=0x%02x", addr);
            ++count;
        }
        wdt_clear();
        os_time_dly(1);
    }
    LOG("I2C scan complete: %d device(s); ACK is not a functional PASS", count);
}
#endif
#endif

#if DEMO_TOUCH_ENABLE
static u8 tp_online;
static u8 tp_last[6];
static u32 tp_retry_at;
static u32 tp_next_report;
static void touch_probe(void)
{
    u8 id[3];
    int ret = reg_read(DEMO_TOUCH_ADDR, 0xa7, id, sizeof(id));
    if (ret) {
        LOG("CST816S no response (sleep/unprogrammed/disconnected); touch to wake");
        tp_online = 0;
    } else {
        LOG("TP ID=%02x project=%02x FW=%02x (verify module firmware)", id[0], id[1], id[2]);
        tp_online = 1;
        memset(tp_last, 0xff, sizeof(tp_last));
    }
#if DEMO_I2C_SCREEN_ENABLE
    if (scan_mutex_ready) {
        os_mutex_pend(&scan_mutex, 0);
        scan_sample.tp_id_valid = !ret;
        if (!ret) {
            memcpy(scan_sample.tp_id, id, sizeof(id));
            scan_sample.ack[DEMO_TOUCH_ADDR] |= 3; /* Register read ACKed both. */
        }
        os_mutex_post(&scan_mutex);
    }
#endif
    tp_retry_at = sys_timer_get_ms() + 3000;
}
static void touch_poll(u32 now)
{
    if (!tp_online) {
        if ((s32)(now - tp_retry_at) >= 0) {
            touch_probe();
        }
        return;
    }
    /* Poll every 20 ms as well as observing IRQ. No flash/firmware writes. */
    u8 data[6];
    if (reg_read(DEMO_TOUCH_ADDR, 0x01, data, sizeof(data))) {
        tp_online = 0;
        tp_retry_at = now + 3000;
        LOG("TP read failed; retry in 3 s (standby may NACK)");
        return;
    }
    if (memcmp(data, tp_last, sizeof(data)) && (s32)(now - tp_next_report) >= 0) {
        LOG("TP irq=%d gesture=%02x fingers=%u event=%u x=%u y=%u",
            gpio_read(DEMO_TP_IRQ_PIN), data[0], data[1] & 0x0f,
            data[2] >> 6, ((data[2] & 0x0f) << 8) | data[3],
            ((data[4] & 0x0f) << 8) | data[5]);
        memcpy(tp_last, data, sizeof(data));
        tp_next_report = now + 100;
    }
}
#endif

#if DEMO_I2C_SCREEN_ENABLE
#if DEMO_I2C_EDGE_TEST
static u8 edge_phase;
static void scan_edge_test(u32 now)
{
    if ((s32)(now - scan_next_at) < 0) {
        return;
    }
    if (!edge_phase) {
        /* SDA stays released throughout: no START/address/register write. */
        gpio_i2c_release();
        os_time_dly(1);
        u8 pre = (JL_PORTB->IN >> 1) & 3;
        int low_ret = gpio_set_mode(IO_PORT_SPILT(DEMO_SCL_PIN), PORT_OUTPUT_LOW);
        os_time_dly(1);
        u8 low = (JL_PORTB->IN >> 1) & 3;
        gpio_i2c_mode_return = gpio_set_mode(IO_PORT_SPILT(DEMO_SCL_PIN), PORT_INPUT_PULLUP_10K);
        os_time_dly(1);
        u8 up = (JL_PORTB->IN >> 1) & 3;
        if (scan_mutex_ready) {
            os_mutex_pend(&scan_mutex, 0);
            scan_sample.edge_pre = pre;
            scan_sample.edge_low = low;
            scan_sample.edge_up = up;
            scan_sample.edge_config_error = low_ret < 0 || gpio_i2c_mode_return < 0;
            os_mutex_post(&scan_mutex);
        }
        scan_next_at = sys_timer_get_ms() + 100;
        edge_phase = 1;
        return;
    }
    u8 late = (JL_PORTB->IN >> 1) & 3;
    if (scan_mutex_ready) {
        os_mutex_pend(&scan_mutex, 0);
        scan_sample.edge_late = late;
        scan_sample.edge_done = 1;
        u8 failed = scan_sample.edge_pre != 3 || scan_sample.edge_low != 1 ||
                    scan_sample.edge_up != 3 || late != 3 || scan_sample.edge_config_error;
        os_mutex_post(&scan_mutex);
        if (failed) {
            gpio_i2c_stage = 7;
            gpio_i2c_bit = 0xff;
            gpio_i2c_record_fault(4);
        }
    }
    edge_phase = 2;
    scan_next_at = sys_timer_get_ms();
}
#endif
static void scan_poll(u32 now)
{
#if DEMO_I2C_PIN_TEST
    if ((s32)(now - scan_next_at) >= 0) {
        scan_next_at = now + 1000;
        int sda_ret = gpio_set_mode(IO_PORT_SPILT(DEMO_SDA_PIN), PORT_INPUT_PULLUP_10K);
        int scl_ret = gpio_set_mode(IO_PORT_SPILT(DEMO_SCL_PIN), PORT_INPUT_PULLUP_10K);
        if (scan_mutex_ready) {
            os_mutex_pend(&scan_mutex, 0);
            scan_sample.pin_config_error = sda_ret < 0 || scl_ret < 0;
            os_mutex_post(&scan_mutex);
        }
        os_time_dly(1); /* Sample settled, released lines. */
    }
    scan_bus_status();
    return;
#endif
#if DEMO_I2C_EDGE_TEST
    if (edge_phase < 2) {
        scan_edge_test(now);
        scan_bus_status();
        return;
    }
    if (gpio_i2c_first_fault.code) {
        /* Freeze first failure, keep LED/display alive, release MCU outputs. */
        scan_bus_status();
        return;
    }
#endif
    scan_bus_status();
    if ((s32)(now - scan_next_at) < 0) {
        return;
    }
    scan_next_at = now + 20;
    if (!gpio_read(DEMO_SDA_PIN) || !gpio_read(DEMO_SCL_PIN)) {
        bus_ok = 0;
    }
    if (!bus_ok) {
        scan_next_at = now + 3000;
        bus_ok = bus_init() == 0;
        scan_bus_status();
        return;
    }
    if (scan_cursor == 0x08) {
        /* Identification test: wake via RESET each round, probe ID first. */
        gpio_set_mode(IO_PORT_SPILT(DEMO_TP_RST_PIN), PORT_OUTPUT_LOW);
        os_time_dly(2);
        gpio_set_mode(IO_PORT_SPILT(DEMO_TP_RST_PIN), PORT_OUTPUT_HIGH);
        os_time_dly(12);
        touch_probe();
    }
    int w = scan_address(scan_cursor, 0);
    int r = scan_address(scan_cursor, 1);
    if (scan_mutex_ready) {
        os_mutex_pend(&scan_mutex, 0);
        scan_sample.scan_addr = scan_cursor;
        scan_sample.ack[scan_cursor] |= (w > 0 ? 1 : 0) | (r > 0 ? 2 : 0);
        if (w < 0 || r < 0) {
            ++scan_sample.probe_errors;
        }
        if (scan_cursor == 0x77) {
            ++scan_sample.rounds;
        }
        os_mutex_post(&scan_mutex);
    }
    if (w > 0 || r > 0) {
        LOG("I2C addr7=%02x write_ACK=%d read_ACK=%d", scan_cursor, w, r);
    }
    scan_bus_status();
    if (scan_cursor == 0x77) {
        scan_cursor = 0x08;
        scan_next_at = sys_timer_get_ms() + 3000;
    } else {
        ++scan_cursor;
        scan_next_at = sys_timer_get_ms() + 20;
    }
}
#endif

#if DEMO_ACCEL_ENABLE
static u8 accel_online;
static u32 accel_retry_at;
static u32 accel_log_at;
static OS_MUTEX accel_mutex;
static int accel_mutex_ready;
static struct demo_accel_sample accel_sample;

void ac7076a3_demo_accel_snapshot(struct demo_accel_sample *sample)
{
    memset(sample, 0, sizeof(*sample));
    if (!accel_mutex_ready) {
        return;
    }
    os_mutex_pend(&accel_mutex, 0);
    *sample = accel_sample;
    os_mutex_post(&accel_mutex);
}

static void accel_set_status(enum demo_accel_status status, u8 id)
{
    if (!accel_mutex_ready) {
        return;
    }
    os_mutex_pend(&accel_mutex, 0);
    accel_sample.status = status;
    accel_sample.chip_id = id;
    accel_sample.valid = 0;
    os_mutex_post(&accel_mutex);
}

static void accel_init(void)
{
    u8 id;
    accel_online = 0;
    accel_retry_at = sys_timer_get_ms() + 3000;
    if (reg_read(DEMO_ACCEL_ADDR, 0x01, &id, 1)) {
        accel_set_status(DEMO_ACC_NO_DEVICE, 0);
        LOG("DA213B absent at 0x27; retry in 3 s");
        return;
    }
    LOG("DA213B chip_id=0x%02x (expected 0x13)", id);
    if (id != 0x13) {
        accel_set_status(DEMO_ACC_BAD_ID, id);
        LOG("DA213B ID mismatch: no configuration writes");
        return;
    }
    /* DS_da213B Rev0.2: +/-2g, 62.5 Hz, active, autosleep disabled.
     * Preserve reserved/default bits. Read back every configuration write.
     */
    if (reg_update(DEMO_ACCEL_ADDR, 0x0f, 0x03, 0x00) ||
        reg_update(DEMO_ACCEL_ADDR, 0x10, 0x0f, 0x06) ||
        reg_update(DEMO_ACCEL_ADDR, 0x11, 0x81, 0x00)) {
        accel_set_status(DEMO_ACC_CONFIG_ERROR, id);
        LOG("DA213B config/readback failed");
        return;
    }
    accel_online = 1;
    accel_set_status(DEMO_ACC_WAIT_DATA, id);
    os_time_dly(2);
    LOG("DA213B configured: +/-2g, 62.5Hz, polling (INT is NC on board)");
}
static void accel_poll(u32 now)
{
    u8 raw[6];
    if (!accel_online) {
        if ((s32)(now - accel_retry_at) >= 0) {
            accel_init();
        }
        return;
    }
    if (reg_read(DEMO_ACCEL_ADDR, 0x02, raw, sizeof(raw))) {
        if (accel_mutex_ready) {
            os_mutex_pend(&accel_mutex, 0);
            accel_sample.status = DEMO_ACC_READ_ERROR;
            accel_sample.valid = 0;
            ++accel_sample.errors;
            os_mutex_post(&accel_mutex);
        }
        return;
    }
    int xyz[3];
    for (int i = 0; i < 3; ++i) {
        /* Signed, left-aligned 14-bit data; 4096 counts/g at +/-2g. */
        unsigned int bits = ((u16)raw[2 * i] | ((u16)raw[2 * i + 1] << 8)) >> 2;
        xyz[i] = (bits & 0x2000) ? (int)bits - 0x4000 : (int)bits;
        xyz[i] = xyz[i] * 1000 / 4096;
    }
    if (accel_mutex_ready) {
        os_mutex_pend(&accel_mutex, 0);
        accel_sample.status = DEMO_ACC_READY;
        accel_sample.valid = 1;
        memcpy(accel_sample.mg, xyz, sizeof(xyz));
        ++accel_sample.samples;
        accel_sample.sampled_at = sys_timer_get_ms();
        os_mutex_post(&accel_mutex);
    }
    if ((s32)(now - accel_log_at) >= 0) {
        accel_log_at = now + 1000;
        LOG("ACC mg x=%d y=%d z=%d", xyz[0], xyz[1], xyz[2]);
    }
}
#endif

#if DEMO_KEY_ENABLE
static u8 key_sample = 1, key_stable = 1;
static u32 key_changed_at;
static void key_press(void)
{
    LOG("KEY DOWN PB7");
#if DEMO_SPEAKER_ENABLE
    ac7076a3_demo_beep_start();
#endif
}
#endif
#if DEMO_MOTOR_ENABLE
static int motor_ready;
static volatile int motor_active;
static void motor_off(void *priv)
{
    (void)priv;
    power_gate_pwm_set_duty(IO_MT_PG, 0);
    motor_active = 0;
}
#endif

void ac7076a3_demo_task(void *priv)
{
    u32 slow_at = 0, touch_at = 0;
    u32 heartbeat = 0;
#if DEMO_ACCEL_ENABLE
    u32 accel_at = 0;
#endif
    (void)priv;
    LOG("AC7076A3 Board1 demo %s %s", __DATE__, __TIME__);
#if DEMO_USB_CDC_ENABLE
    LOG("LOG=USB CDC virtual COM; PA2 UART unused; original watch UI/BT not started");
#else
    LOG("USB CDC disabled; PA2 unused; original watch UI/BT not started");
#endif
    LOG("LED=%d KEY=%d MOTOR=%d SCAN=%d TP=%d ACC=%d POWER=%d LCD=%d DAC=%d MIC=%d",
        DEMO_LED_ENABLE, DEMO_KEY_ENABLE, DEMO_MOTOR_ENABLE, DEMO_I2C_SCAN_ENABLE,
        DEMO_TOUCH_ENABLE, DEMO_ACCEL_ENABLE, DEMO_POWER_ENABLE, DEMO_LCD_ENABLE,
        DEMO_SPEAKER_ENABLE, DEMO_MIC_ENABLE);
    /* Off first: these are power-gate pins, not normal push-pull GPIO. */
    power_gate_open_drain_output(IO_MT_PG, 1);
    power_gate_open_drain_output(IO_LCD_PG, 1);
    /* Initialize the clock-manager mutex before optional audio/USB drivers. */
    ac7076a3_demo_clock_init();
#if DEMO_I2C_SCREEN_ENABLE
    scan_mutex_ready = os_mutex_create(&scan_mutex) == 0;
#endif
#if DEMO_ACCEL_ENABLE
    accel_mutex_ready = os_mutex_create(&accel_mutex) == 0;
#endif
#if DEMO_LED_ENABLE
    gpio_set_mode(IO_PORT_SPILT(DEMO_LED_PIN), PORT_OUTPUT_HIGH);
#endif
#if DEMO_KEY_ENABLE
    gpio_set_mode(IO_PORT_SPILT(DEMO_KEY_PIN), PORT_INPUT_PULLUP_10K);
#endif
#if DEMO_MOTOR_ENABLE
    motor_ready = power_gate_pwm_init(IO_MT_PG, 10000, 0) == 0;
    LOG("MOTOR init=%d; key triggers %ums pulse", motor_ready, DEMO_MOTOR_PULSE_MS);
#endif
#if DEMO_POWER_ENABLE || DEMO_MIC_ENABLE || DEMO_SPEAKER_ENABLE
    adc_init();
#endif
#if DEMO_TOUCH_ENABLE
    gpio_set_mode(IO_PORT_SPILT(DEMO_TP_IRQ_PIN), PORT_INPUT_PULLUP_10K);
    gpio_set_mode(IO_PORT_SPILT(DEMO_TP_RST_PIN), PORT_OUTPUT_LOW);
    os_time_dly(2);
    gpio_set_mode(IO_PORT_SPILT(DEMO_TP_RST_PIN), PORT_OUTPUT_HIGH);
    os_time_dly(12);
#endif
#if DEMO_I2C_ENABLE
    bus_ok = bus_init() == 0;
#if DEMO_ACCEL_ENABLE
    if (!bus_ok) {
        accel_set_status(DEMO_ACC_BUS_LOW, 0);
    }
#endif
    if (bus_ok) {
        /* Probe touch immediately after reset, before the slow bus scan. */
#if DEMO_TOUCH_ENABLE && !DEMO_I2C_PIN_TEST && !DEMO_I2C_EDGE_TEST
        touch_probe();
#endif
#if DEMO_ACCEL_ENABLE
        os_time_dly(10); /* DA213B power-on settling: at least 100 ms. */
        accel_init();
#endif
#if DEMO_I2C_SCAN_ENABLE && !DEMO_I2C_SCREEN_ENABLE
        bus_scan();
#endif
    }
#endif
#if DEMO_SPEAKER_ENABLE || DEMO_MIC_ENABLE
    int audio_ret = ac7076a3_demo_audio_init();
    LOG("audio subsystem init=%d", audio_ret);
#if DEMO_MIC_ENABLE
    if (!audio_ret) {
        LOG("MIC open=%d", ac7076a3_demo_mic_start());
    }
#endif
#endif
#if !DEMO_LCD_ENABLE
    LOG("LCD skipped; add JD9855 panel init + confirm resolution to enable");
#endif
    while (1) {
        u32 now = sys_timer_get_ms();
        wdt_clear();
#if DEMO_KEY_ENABLE
        u8 level = gpio_read(DEMO_KEY_PIN) ? 1 : 0;
        if (level != key_sample) {
            key_sample = level;
            key_changed_at = now;
        }
        if (level != key_stable && (u32)(now - key_changed_at) >= 30) {
            key_stable = level;
            if (!level) {
                key_press();
#if DEMO_MOTOR_ENABLE
                if (motor_ready && !motor_active) {
                    motor_active = 1;
                    power_gate_pwm_set_duty(IO_MT_PG, DEMO_MOTOR_DUTY);
                    /* Hardware-timer callback: independent of task queue/I2C. */
                    if (!usr_timeout_add(NULL, motor_off, DEMO_MOTOR_PULSE_MS, 1)) {
                        motor_off(NULL);
                        LOG("MOTOR timeout allocation failed; switched off");
                    }
                }
#endif
            } else {
                LOG("KEY UP PB7");
            }
        }
#endif
#if DEMO_TOUCH_ENABLE && !DEMO_I2C_SCREEN_ENABLE
        if (bus_ok && (s32)(now - touch_at) >= 0) {
            touch_at = now + 20;
            touch_poll(now);
        }
#endif
#if DEMO_I2C_SCREEN_ENABLE
        scan_poll(now);
#endif
#if DEMO_ACCEL_ENABLE
        if (bus_ok && (s32)(now - accel_at) >= 0) {
            accel_at = now + DEMO_ACCEL_POLL_MS;
            accel_poll(now);
        }
#endif
#if DEMO_SPEAKER_ENABLE || DEMO_MIC_ENABLE
        ac7076a3_demo_audio_poll();
#endif
#if DEMO_USB_CDC_ENABLE
        ac7076a3_demo_usb_poll();
#endif
#if DEMO_BLE_ENABLE
        ac7076a3_demo_ble_poll();
#endif
        if ((s32)(now - slow_at) >= 0) {
            slow_at = now + 1000;
            LOG("alive %u uptime_ms=%u", ++heartbeat, now);
#if DEMO_BLE_ENABLE
            if (heartbeat == 4) {
                ac7076a3_demo_ble_start();
            }
#endif
#if DEMO_LED_ENABLE
            gpio_set_mode(IO_PORT_SPILT(DEMO_LED_PIN),
                          (heartbeat & 1) ? PORT_OUTPUT_LOW : PORT_OUTPUT_HIGH);
#endif
#if DEMO_USB_CDC_ENABLE
            if (heartbeat == 2) {
                ac7076a3_demo_usb_start();
            }
#endif
#if DEMO_LCD_ENABLE
            if (heartbeat == 2) {
                int lcd_task_err = task_create(ac7076a3_demo_lcd_task, NULL, "lcd_demo");
                LOG("LCD demo task create=%d", lcd_task_err);
            }
#endif
#if DEMO_POWER_ENABLE
            u32 bat = adc_get_voltage_blocking(AD_CH_PMU_VBAT) * AD_CH_PMU_VBAT_DIV;
            u32 usb = adc_get_voltage_blocking(AD_CH_PMU_VPWR_4) * 4;
            LOG("POWER VBAT=%umV VPWR=%umV USB_5V=%d (charge disabled)", bat, usb, usb > 4000);
#endif
        }
        /* Use the normal SDK queue dispatcher so internal timer/Q_CALLBACK
         * messages run; the timeout still services polling without events. */
        int msg[16];
        int pend_ret = os_taskq_pend_timeout(NULL, msg, ARRAY_SIZE(msg), 1);
#if DEMO_BLE_ENABLE
        if (pend_ret == OS_TASKQ) {
            ac7076a3_demo_ble_message(msg);
        }
#endif
    }
}
#endif
