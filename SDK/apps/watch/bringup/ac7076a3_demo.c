#include "app_config.h"
#include "system/includes.h"
#include "system/timer.h"
#include "asm/includes.h"
#include "gpio.h"
#include "gpadc.h"
#include "iic_soft.h"
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
    .master_frequency = 50000,
};
static int bus_ok;

static int reg_read(u8 addr, u8 reg, u8 *data, int len)
{
    int ret = soft_i2c_master_read_nbytes_from_device_reg(demo_iic,
              addr << 1, &reg, 1, data, len);
    return ret == len ? 0 : -1;
}

#if DEMO_ACCEL_ENABLE
static int reg_update(u8 addr, u8 reg, u8 mask, u8 value)
{
    u8 old;
    if (reg_read(addr, reg, &old, 1)) {
        return -1;
    }
    old = (old & ~mask) | (value & mask);
    if (soft_i2c_master_write_nbytes_to_device_reg(demo_iic, addr << 1,
            &reg, 1, &old, 1) != 1) {
        return -1;
    }
    if (reg_read(addr, reg, &old, 1) || (old & mask) != (value & mask)) {
        return -1;
    }
    return 0;
}
#endif

static int bus_init(void)
{
    gpio_set_mode(IO_PORT_SPILT(DEMO_SCL_PIN), PORT_INPUT_PULLUP_10K);
    gpio_set_mode(IO_PORT_SPILT(DEMO_SDA_PIN), PORT_INPUT_PULLUP_10K);
    os_time_dly(1);
    if (!gpio_read(DEMO_SCL_PIN) || !gpio_read(DEMO_SDA_PIN)) {
        LOG("I2C BUS LOW: check PB2/PB1, R3/R4 and 3V3; tests skipped");
        return -1;
    }
    int ret = soft_iic_init(demo_iic, &demo_iic_cfg);
    LOG("I2C init=%d SDA=PB1 SCL=PB2 nominal=50kHz", ret);
    return ret == IIC_OK ? 0 : -1;
}

#if DEMO_I2C_SCAN_ENABLE
static void bus_scan(void)
{
    int count = 0;
    for (u8 addr = 0x08; addr <= 0x77; ++addr) {
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
    if (reg_read(DEMO_TOUCH_ADDR, 0xa7, id, sizeof(id))) {
        LOG("CST816S no response (sleep/unprogrammed/disconnected); touch to wake");
        tp_online = 0;
    } else {
        LOG("TP ID=%02x project=%02x FW=%02x (verify module firmware)", id[0], id[1], id[2]);
        tp_online = 1;
        memset(tp_last, 0xff, sizeof(tp_last));
    }
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

#if DEMO_ACCEL_ENABLE
static u8 accel_online;
static void accel_init(void)
{
    u8 id;
    if (reg_read(DEMO_ACCEL_ADDR, 0x01, &id, 1)) {
        LOG("DA213B absent at 0x27; accel test skipped");
        return;
    }
    LOG("DA213B chip_id=0x%02x (expected 0x13)", id);
    if (id != 0x13) {
        LOG("DA213B ID mismatch: no configuration writes");
        return;
    }
    /* DS_da213B Rev0.2: +/-2g, 62.5 Hz, active, autosleep disabled.
     * Preserve reserved/default bits. Read back every configuration write.
     */
    if (reg_update(DEMO_ACCEL_ADDR, 0x0f, 0x03, 0x00) ||
        reg_update(DEMO_ACCEL_ADDR, 0x10, 0x0f, 0x06) ||
        reg_update(DEMO_ACCEL_ADDR, 0x11, 0x81, 0x00)) {
        LOG("DA213B config/readback failed");
        return;
    }
    accel_online = 1;
    LOG("DA213B configured: +/-2g, 62.5Hz, polling (INT is NC on board)");
}
static void accel_poll(void)
{
    u8 raw[6];
    if (!accel_online) {
        return;
    }
    if (reg_read(DEMO_ACCEL_ADDR, 0x02, raw, sizeof(raw))) {
        LOG("DA213B read failed; no stale sample reported");
        return;
    }
    int xyz[3];
    for (int i = 0; i < 3; ++i) {
        /* Signed, left-aligned 14-bit data; 4096 counts/g at +/-2g. */
        xyz[i] = (s16)((u16)raw[2 * i] | ((u16)raw[2 * i + 1] << 8));
        xyz[i] /= 4;
    }
    LOG("ACC mg x=%d y=%d z=%d", xyz[0] * 1000 / 4096,
        xyz[1] * 1000 / 4096, xyz[2] * 1000 / 4096);
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
    if (bus_ok) {
        /* Probe touch immediately after reset, before the slow bus scan. */
#if DEMO_TOUCH_ENABLE
        touch_probe();
#endif
#if DEMO_ACCEL_ENABLE
        accel_init();
#endif
#if DEMO_I2C_SCAN_ENABLE
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
#if DEMO_TOUCH_ENABLE
        if (bus_ok && (s32)(now - touch_at) >= 0) {
            touch_at = now + 20;
            touch_poll(now);
        }
#endif
#if DEMO_SPEAKER_ENABLE || DEMO_MIC_ENABLE
        ac7076a3_demo_audio_poll();
#endif
#if DEMO_USB_CDC_ENABLE
        ac7076a3_demo_usb_poll();
#endif
        if ((s32)(now - slow_at) >= 0) {
            slow_at = now + 1000;
            LOG("alive %u uptime_ms=%u", ++heartbeat, now);
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
#if DEMO_ACCEL_ENABLE
            if (bus_ok) {
                accel_poll();
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
        os_taskq_pend_timeout(NULL, msg, ARRAY_SIZE(msg), 1);
    }
}
#endif
