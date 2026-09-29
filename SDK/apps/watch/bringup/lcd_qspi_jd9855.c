#include "app_config.h"
#include "includes.h"
#include "system/timer.h"
#include "ui/ui_api.h"
#include "ui/lcd/lcd_drive.h"
#include "asm/power/power_gate.h"
#include "asm/dcache.h"
#include "ac7076a3_demo.h"
#if DEMO_BLE_ENABLE
#include "ble_user.h"
#endif

#if AC7076A3_DEMO_ENABLE && TCFG_LCD_QSPI_JD9855_ENABLE
#include "jd9855_panel_init.h"
#define DRIVE_CONFIG QSPI_RGB565_SUBMODE1_1T2B

static const struct dbi_param jd9855_param = {
    .scr_x = DEMO_LCD_X_OFFSET,
    .scr_y = DEMO_LCD_Y_OFFSET,
    .scr_w = DEMO_LCD_WIDTH,
    .scr_h = DEMO_LCD_HEIGHT,
    .lcd_width = DEMO_LCD_WIDTH,
    .lcd_height = DEMO_LCD_HEIGHT,
    .lcd_type = LCD_TYPE_SPI,
    .in_width = DEMO_LCD_WIDTH,
    .in_height = DEMO_LCD_HEIGHT,
    .in_format = OUTPUT_FORMAT_RGB565,
    .buffer_num = 2,
    .buffer_size = DEMO_LCD_WIDTH * 16 * 2,
    .fps = DEMO_LCD_FPS,
    .spi = {
        .spi_mode = SPI_IF_MODE(DRIVE_CONFIG),
        .pixel_type = PIXEL_TYPE(DRIVE_CONFIG),
        .out_format = OUT_FORMAT(DRIVE_CONFIG),
        .spi_dat_mode = SPI_MODE_UNIDIR,
        .cs_pin_select = CS_PIN_SEL_PA7,
        .dc_pin_select = DC_PIN_SEL_SOFT,
        .soft_dc_pin = IO_PORTB_08,
        .read_pin_select = READ_PIN_SEL_PA9,
        .qspi_cmd = {
            .write_cmd = 0x02,
            .read_cmd = 0x03,
            .submode0_cmd = 0x02,
            .submode1_cmd = 0x32,
            .submode2_cmd = 0x12,
        },
    },
};

static int jd9855_power(u8 on)
{
    /* FPC2 logic supply is permanently connected to IOVDD/3V3. */
    (void)on;
    return 0;
}

static void jd9855_reset(void)
{
    gpio_set_mode(IO_PORT_SPILT(IO_PORTC_03), PORT_OUTPUT_HIGH);
    os_time_dly(1);
    gpio_set_mode(IO_PORT_SPILT(IO_PORTC_03), PORT_OUTPUT_LOW);
    os_time_dly(2);
    gpio_set_mode(IO_PORT_SPILT(IO_PORTC_03), PORT_OUTPUT_HIGH);
    os_time_dly(12);
}

REGISTER_LCD_DEVICE(jd9855) = {
    .logo = "jd9855",
    .row_addr_align = 1,
    .column_addr_align = 2,
    .radius = 0,
    .fill_argb = 0xff000000,
    .lcd_cmd = (void *)jd9855_panel_init,
    .cmd_cnt = sizeof(jd9855_panel_init),
    .param = (void *)&jd9855_param,
    .reset = jd9855_reset,
    .power_ctrl = jd9855_power,
};

/* Compile the test implementation even when disabled, to check SDK API types.
 * Only the guarded calls in ac7076a3_demo.c may start the display. */
static struct lcd_interface *demo_lcd;
static u32 next_color_at;
static u8 color_index;
#if (DEMO_ACCEL_SCREEN_ENABLE && DEMO_ACCEL_ENABLE) || DEMO_I2C_SCREEN_ENABLE || DEMO_BLE_ENABLE
/* Small ASCII font, independent of the disabled watch UI/resource partition. */
static const struct { char ch; u8 col[5]; } accel_font[] = {
    {'0',{0x3e,0x51,0x49,0x45,0x3e}}, {'1',{0,0x42,0x7f,0x40,0}},
    {'2',{0x42,0x61,0x51,0x49,0x46}}, {'3',{0x21,0x41,0x45,0x4b,0x31}},
    {'4',{0x18,0x14,0x12,0x7f,0x10}}, {'5',{0x27,0x45,0x45,0x45,0x39}},
    {'6',{0x3c,0x4a,0x49,0x49,0x30}}, {'7',{1,0x71,9,5,3}},
    {'8',{0x36,0x49,0x49,0x49,0x36}}, {'9',{6,0x49,0x49,0x29,0x1e}},
    {'A',{0x7e,0x11,0x11,0x11,0x7e}}, {'B',{0x7f,0x49,0x49,0x49,0x36}},
    {'C',{0x3e,0x41,0x41,0x41,0x22}}, {'D',{0x7f,0x41,0x41,0x22,0x1c}},
    {'E',{0x7f,0x49,0x49,0x49,0x41}}, {'F',{0x7f,9,9,9,1}},
    {'G',{0x3e,0x41,0x49,0x49,0x7a}}, {'H',{0x7f,8,8,8,0x7f}},
    {'I',{0,0x41,0x7f,0x41,0}}, {'J',{0x20,0x40,0x41,0x3f,1}},
    {'K',{0x7f,8,0x14,0x22,0x41}}, {'L',{0x7f,0x40,0x40,0x40,0x40}},
    {'M',{0x7f,2,0x0c,2,0x7f}}, {'N',{0x7f,4,8,0x10,0x7f}},
    {'O',{0x3e,0x41,0x41,0x41,0x3e}}, {'P',{0x7f,9,9,9,6}},
    {'Q',{0x3e,0x41,0x51,0x21,0x5e}}, {'R',{0x7f,9,0x19,0x29,0x46}},
    {'S',{0x46,0x49,0x49,0x49,0x31}}, {'T',{1,1,0x7f,1,1}},
    {'U',{0x3f,0x40,0x40,0x40,0x3f}}, {'V',{0x1f,0x20,0x40,0x20,0x1f}},
    {'W',{0x3f,0x40,0x38,0x40,0x3f}}, {'X',{0x63,0x14,8,0x14,0x63}},
    {'Y',{7,8,0x70,8,7}}, {'Z',{0x61,0x51,0x49,0x45,0x43}},
    {'-',{8,8,8,8,8}}, {':',{0,0x36,0x36,0,0}},
    {'/',{0x20,0x10,8,4,2}}, {'+',{8,8,0x3e,8,8}},
    {'(',{0,0x1c,0x22,0x41,0}}, {')',{0,0x41,0x22,0x1c,0}}
};
#define ACC_TEXT_W 252
#define ACC_TEXT_H 24
static u8 accel_text_pixels[ACC_TEXT_W * ACC_TEXT_H * 2] ALIGNED(32);
static u32 accel_refresh_at;
static u8 accel_screen_cleared;

static void accel_text(int y, const char *text, u16 color)
{
    const int scale = 3;
    int len = strlen(text);
    if (len > 14) {
        len = 14;
    }
    memset(accel_text_pixels, 0, sizeof(accel_text_pixels));
    int left = (ACC_TEXT_W - len * 6 * scale) / 2;
    for (int n = 0; n < len; ++n) {
        const u8 *col = NULL;
        for (int g = 0; g < ARRAY_SIZE(accel_font); ++g) {
            if (accel_font[g].ch == text[n]) {
                col = accel_font[g].col;
                break;
            }
        }
        if (!col) {
            continue; /* Space. */
        }
        for (int x = 0; x < 5; ++x) {
            for (int r = 0; r < 7; ++r) {
                if (!(col[x] & BIT(r))) {
                    continue;
                }
                for (int dy = 0; dy < scale; ++dy) {
                    for (int dx = 0; dx < scale; ++dx) {
                        int p = ((r * scale + dy + 1) * ACC_TEXT_W +
                                 left + n * 6 * scale + x * scale + dx) * 2;
                        /* SDK RGB565 input byte order: R5G3, G3B5. */
                        accel_text_pixels[p] = color >> 8;
                        accel_text_pixels[p + 1] = color & 0xff;
                    }
                }
            }
        }
    }
    DcuFlushRegion((unsigned int *)accel_text_pixels, sizeof(accel_text_pixels));
    demo_lcd->set_draw_area(DEMO_LCD_X_OFFSET + 54, DEMO_LCD_X_OFFSET + 305,
                            DEMO_LCD_Y_OFFSET + y,
                            DEMO_LCD_Y_OFFSET + y + ACC_TEXT_H - 1);
    demo_lcd->draw(accel_text_pixels, DEMO_LCD_X_OFFSET + 54,
                   DEMO_LCD_X_OFFSET + 305, DEMO_LCD_Y_OFFSET + y,
                   DEMO_LCD_Y_OFFSET + y + ACC_TEXT_H - 1);
    /* draw is asynchronous. Do not modify the buffer while DMA reads it. */
    lcd_wait_busy();
}

#if DEMO_BLE_ENABLE
static void ble_screen_poll(u32 now)
{
    if (!demo_lcd || !demo_lcd->draw || !demo_lcd->set_draw_area ||
        (s32)(now - accel_refresh_at) < 0) {
        return;
    }
    accel_refresh_at = now + 500;
    if (!accel_screen_cleared) {
        demo_lcd->clear_screen(0, DEMO_LCD_X_OFFSET,
            DEMO_LCD_X_OFFSET + DEMO_LCD_WIDTH - 1, DEMO_LCD_Y_OFFSET,
            DEMO_LCD_Y_OFFSET + DEMO_LCD_HEIGHT - 1);
        lcd_wait_busy();
        accel_screen_cleared = 1;
    }
    struct demo_ble_sample sample;
    char line[32];
    ac7076a3_demo_ble_snapshot(&sample);
    const char *state = "WAIT START";
    if (sample.started) {
        state = sample.init_return < 0 ? "INIT ERROR" :
                sample.timed_out ? "INIT TIMEOUT" : "WAIT INIT";
    }
    if (sample.init_ok) {
        switch (sample.state) {
        case BLE_ST_ADV: state = "ADV REQUEST"; break;
        case BLE_ST_CONNECT: state = "CONNECTED"; break;
        case BLE_ST_NOTIFY_IDICATE: state = "NOTIFY ON"; break;
        case BLE_ST_DISCONN: state = "DISCONNECTED"; break;
        default: state = "STACK READY"; break;
        }
    }
    accel_text(34, "BLE TEST", 0xffff);
    accel_text(68, "AC7076A3(BLE)", 0xffff);
    accel_text(102, state, sample.init_ok ? 0x07e0 : 0xffff);
    sprintf(line, "MAC:%02X-%02X-%02X", sample.mac[5], sample.mac[4], sample.mac[3]);
    accel_text(140, line, 0xffff);
    sprintf(line, "%02X-%02X-%02X", sample.mac[2], sample.mac[1], sample.mac[0]);
    accel_text(174, line, 0xffff);
    sprintf(line, "RX:%u", sample.rx_packets);
    accel_text(208, line, 0xffff);
    sprintf(line, "BYTES:%u", sample.rx_bytes);
    accel_text(242, line, 0xffff);
    sprintf(line, "RET:%d ST:%02X", sample.init_return, sample.state);
    accel_text(272, line, 0xffff);
    accel_text(302, "BLE SCAN", 0xffff);
}
#endif

#if DEMO_ACCEL_SCREEN_ENABLE && DEMO_ACCEL_ENABLE
static void accel_screen_poll(u32 now)
{
    static const char *const status[] = {
        "INIT", "BUS LOW", "NO DEVICE", "BAD ID", "CONFIG ERR",
        "WAIT DATA", "READY", "READ ERROR"
    };
    struct demo_accel_sample sample;
    char line[32];
    if (!demo_lcd || !demo_lcd->draw || !demo_lcd->set_draw_area ||
        (s32)(now - accel_refresh_at) < 0) {
        return;
    }
    accel_refresh_at = now + 250;
    if (!accel_screen_cleared) {
        demo_lcd->clear_screen(0, DEMO_LCD_X_OFFSET,
            DEMO_LCD_X_OFFSET + DEMO_LCD_WIDTH - 1, DEMO_LCD_Y_OFFSET,
            DEMO_LCD_Y_OFFSET + DEMO_LCD_HEIGHT - 1);
        lcd_wait_busy();
        accel_text(34, "DA213B MG", 0xffff);
        accel_text(302, "FLIP / TILT", 0xffff);
        accel_screen_cleared = 1;
    }
    ac7076a3_demo_accel_snapshot(&sample);
    u8 fresh = sample.valid && (u32)(sys_timer_get_ms() - sample.sampled_at) < 500;
    sprintf(line, "ID:%02X ADDR:27", sample.chip_id);
    accel_text(68, line, 0xffff);
    accel_text(102, sample.valid && !fresh ? "STALE DATA" : status[sample.status],
               fresh ? 0x07e0 : 0xf800);
    for (int axis = 0; axis < 3; ++axis) {
        if (fresh) {
            sprintf(line, "%c: %d", 'X' + axis, sample.mg[axis]);
        } else {
            sprintf(line, "%c: ----", 'X' + axis);
        }
        accel_text(140 + axis * 34, line, 0xffff);
    }
    sprintf(line, "N:%u", sample.samples);
    accel_text(242, line, 0xffff);
    sprintf(line, "ERR:%u", sample.errors);
    accel_text(272, line, sample.errors ? 0xf800 : 0x07e0);
}
#endif
#if DEMO_I2C_SCREEN_ENABLE
static void i2c_screen_poll(u32 now)
{
    /* Only lcd_demo owns these; keep the address list off the task stack. */
    static struct demo_i2c_sample sample;
    static u8 found[112];
    int count = 0;
    char line[32];
    if (!demo_lcd || !demo_lcd->draw || !demo_lcd->set_draw_area ||
        (s32)(now - accel_refresh_at) < 0) {
        return;
    }
    accel_refresh_at = now + 250;
#if DEMO_I2C_DIR_ONLY
    static u8 dir_page = 0xff;
    u8 live_page = (now / 4000) & 1;
    if (live_page != dir_page) {
        dir_page = live_page;
        accel_screen_cleared = 0; /* Different stripe positions: clear on page change. */
    }
#endif
    if (!accel_screen_cleared) {
        demo_lcd->clear_screen(0, DEMO_LCD_X_OFFSET,
            DEMO_LCD_X_OFFSET + DEMO_LCD_WIDTH - 1, DEMO_LCD_Y_OFFSET,
            DEMO_LCD_Y_OFFSET + DEMO_LCD_HEIGHT - 1);
        lcd_wait_busy();
#if DEMO_I2C_PIN_TEST
        accel_text(34, "I2C PINS", 0xffff);
        accel_text(68, "PB1 / PB2", 0xffff);
        accel_text(302, "NO I2C", 0xffff);
#else
        accel_text(34, "I2C SCAN", 0xffff);
        accel_text(302, "TOUCH WAKE", 0xffff);
#endif
        accel_screen_cleared = 1;
    }
    ac7076a3_demo_i2c_snapshot(&sample);
#if DEMO_I2C_DIR_ONLY
    accel_text(34, "SOFT I2C", 0xffff);
    if (live_page) {
        accel_text(68, "PB1 / PB2", 0xffff);
        sprintf(line, "API:%d %d", sample.pin_api[0], sample.pin_api[1]);
        accel_text(102, line, 0xffff);
        sprintf(line, "RAW:%u %u", sample.pin_raw & 1, (sample.pin_raw >> 1) & 1);
        accel_text(136, line, 0xffff);
        sprintf(line, "DIR:%u %u", sample.pin_dir & 1, (sample.pin_dir >> 1) & 1);
        accel_text(174, line, 0xffff);
        sprintf(line, "DIE:%u%u H:%u%u", sample.pin_die & 1, (sample.pin_die >> 1) & 1,
                sample.pin_dieh & 1, (sample.pin_dieh >> 1) & 1);
        accel_text(208, line, 0xffff);
        sprintf(line, "OUT:%u %u", sample.pin_out & 1, (sample.pin_out >> 1) & 1);
        accel_text(242, line, 0xffff);
        sprintf(line, "FAULT:%u", sample.bus_fault);
        accel_text(272, line, 0xffff);
        accel_text(302, "LIVE GPIO", 0xffff);
        return;
    }
#endif
#if DEMO_I2C_PIN_TEST
    sprintf(line, "API:%d %d", sample.pin_api[0], sample.pin_api[1]);
    accel_text(102, line, sample.pin_api[0] == 1 && sample.pin_api[1] == 1 ? 0x07e0 : 0xf800);
    sprintf(line, "RAW:%u %u", sample.pin_raw & 1, (sample.pin_raw >> 1) & 1);
    accel_text(136, line, sample.pin_raw == 3 ? 0x07e0 : 0xf800);
    sprintf(line, "DIR:%u %u", sample.pin_dir & 1, (sample.pin_dir >> 1) & 1);
    accel_text(174, line, sample.pin_dir == 3 ? 0x07e0 : 0xf800);
    sprintf(line, "DIE:%u %u", sample.pin_die & 1, (sample.pin_die >> 1) & 1);
    accel_text(208, line, sample.pin_die == 3 ? 0x07e0 : 0xf800);
    sprintf(line, "DIEH:%u %u", sample.pin_dieh & 1, (sample.pin_dieh >> 1) & 1);
    accel_text(242, line, sample.pin_dieh == 3 ? 0x07e0 : 0xf800);
    accel_text(272, sample.pin_config_error ? "CONFIG ERR" : "INPUT UP",
               sample.pin_config_error ? 0xf800 : 0x07e0);
    return;
#endif
    /* In the edge profile the two pages preserve pre/low/release samples and
     * the first fault's registers, not a later recovery state. */
#if DEMO_I2C_EDGE_TEST
    if ((now / 4000) % 2 == 0) {
        accel_text(34, "GPIO EDGES", 0xffff);
        accel_text(68, "PB1 / PB2", 0xffff);
        sprintf(line, "PRE:%u %u", sample.edge_pre & 1, (sample.edge_pre >> 1) & 1);
        accel_text(102, line, 0xffff);
        sprintf(line, "LOW:%u %u", sample.edge_low & 1, (sample.edge_low >> 1) & 1);
        accel_text(136, line, 0xffff);
        sprintf(line, "UP:%u %u", sample.edge_up & 1, (sample.edge_up >> 1) & 1);
        accel_text(174, line, 0xffff);
        sprintf(line, "LATE:%u %u", sample.edge_late & 1, (sample.edge_late >> 1) & 1);
        accel_text(208, line, 0xffff);
        sprintf(line, "CFG ERR:%u", sample.edge_config_error);
        accel_text(242, line, 0xffff);
        u8 passed = sample.edge_done && sample.edge_pre == 3 && sample.edge_low == 1 &&
                    sample.edge_up == 3 && sample.edge_late == 3 && !sample.edge_config_error;
        accel_text(272, !sample.edge_done ? "TEST WAIT" : passed ? "EDGE PASS" : "EDGE FAIL",
                   passed ? 0x07e0 : 0xf800);
        accel_text(302, "EDGES P1", 0xffff);
    } else if (sample.first_fault.code) {
        const struct demo_i2c_fault *f = &sample.first_fault;
        accel_text(34, "FAULT PAGE2", 0xffff);
        accel_text(68, "PB1 / PB2", 0xffff);
        sprintf(line, "F:%u A:%02X", f->code, f->addr);
        accel_text(102, line, 0xf800);
        sprintf(line, "PH:%u BIT:%u", f->stage, f->bit);
        accel_text(136, line, 0xffff);
        sprintf(line, "API:%d %d", f->api[0], f->api[1]);
        accel_text(174, line, 0xffff);
        sprintf(line, "RAW:%u %u", f->raw & 1, (f->raw >> 1) & 1);
        accel_text(208, line, 0xffff);
        sprintf(line, "DIR:%u %u", f->dir & 1, (f->dir >> 1) & 1);
        accel_text(242, line, 0xffff);
        sprintf(line, "DIE:%u%u H:%u%u", f->die & 1, (f->die >> 1) & 1,
                f->dieh & 1, (f->dieh >> 1) & 1);
        accel_text(272, line, 0xffff);
        sprintf(line, "RET:%d", f->mode_return);
        accel_text(302, line, 0xffff);
    } else {
        accel_text(34, "I2C STATE", 0xffff);
        accel_text(68, "PB1 / PB2", 0xffff);
        accel_text(102, "NO FAULT", 0x07e0);
        if (sample.tp_id_valid) {
            sprintf(line, "TP:%02X FW:%02X", sample.tp_id[0], sample.tp_id[2]);
        } else {
            strcpy(line, "TP: NO ID");
        }
        accel_text(136, line, 0xffff);
        int ack_count = 0;
        for (int a = 0x08; a <= 0x77; ++a) { ack_count += sample.ack[a] != 0; }
        sprintf(line, "ACK:%u", ack_count);
        accel_text(174, line, 0xffff);
        sprintf(line, "SDA:%u SCL:%u", sample.sda, sample.scl);
        accel_text(208, line, 0xffff);
        sprintf(line, "RUN:%u", sample.rounds);
        accel_text(242, line, 0xffff);
        accel_text(272, "SCAN RUN", 0xffff);
        accel_text(302, "STATE P2", 0xffff);
    }
    return;
#endif
    sprintf(line, "SDA:%u SCL:%u", sample.sda, sample.scl);
    accel_text(68, line, sample.bus_ready && sample.sda && sample.scl ? 0x07e0 : 0xf800);
    if (sample.tp_id_valid) {
        sprintf(line, "TP:%02X FW:%02X", sample.tp_id[0], sample.tp_id[2]);
    } else {
        strcpy(line, "TP: NO ID");
    }
    accel_text(102, line, sample.tp_id_valid ? 0x07e0 : 0xf800);
    sprintf(line, "RUN:%u IRQ:%u", sample.rounds % 10000, sample.irq);
    accel_text(136, line, 0xffff);
    for (int addr = 0x08; addr <= 0x77; ++addr) {
        if (sample.ack[addr]) {
            found[count++] = addr;
        }
    }
    int pages = count ? (count + 11) / 12 : 1;
    int page = (now / 4000) % pages;
    for (int row = 0; row < 4; ++row) {
        int used = 0;
        line[0] = 0;
        for (int col = 0; col < 3; ++col) {
            int index = page * 12 + row * 3 + col;
            if (index < count) {
                u8 addr = found[index];
                char mode = sample.ack[addr] == 3 ? 'B' :
                            sample.ack[addr] == 1 ? 'W' : 'R';
                used += sprintf(line + used, "%s%02X%c", col ? " " : "", addr, mode);
            }
        }
        if (!count && row == 0) {
            strcpy(line, sample.bus_ready ? "NO ACK" :
                   (!sample.sda || !sample.scl) ? "BUS LOW" : "BUS INIT ERR");
        } else if (!count && row == 1) {
            sprintf(line, "AT:%02X ERR:%u", sample.scan_addr, sample.probe_errors % 10000);
        } else if (!count && row == 2) {
#if DEMO_I2C_GPIO_ENABLE
            sprintf(line, "FAULT:%u", sample.bus_fault);
#else
            strcpy(line, "RANGE 08-77");
#endif
        }
        accel_text(174 + row * 24, line, count ? 0x07e0 : 0xffff);
    }
    sprintf(line, "ACK:%u P:%u/%u", count, page + 1, pages);
    accel_text(272, line, 0xffff);
}
#endif
#endif
#if DEMO_LCD_BRIGHTNESS_PHASE_TEST
static u32 next_brightness_at;
static u8 brightness_high;
#endif
void ac7076a3_demo_lcd_start(void)
{
    extern const struct ui_devices_cfg ui_cfg_data;
    demo_lcd = lcd_get_hdl();
    if (!demo_lcd || !demo_lcd->init || !demo_lcd->clear_screen) {
        printf("[BRINGUP] LCD interface unavailable\n");
        demo_lcd = NULL;
        return;
    }
    printf("[BRINGUP] JD9855 QSPI init %ux%u, vendor profile %s\n",
           DEMO_LCD_WIDTH, DEMO_LCD_HEIGHT, JD9855_PANEL_PROFILE);
#if DEMO_LCD_RS_HOLD_HIGH
    /* Optional test for modules that require DCX/RS to be held high in QSPI. */
    gpio_set_mode(IO_PORT_SPILT(IO_PORTB_08), PORT_OUTPUT_HIGH);
#endif
    /* .init is void in the SDK: success here does NOT prove panel response. */
    demo_lcd->init((void *)&ui_cfg_data);
    if (power_gate_pwm_init(IO_LCD_PG, 10000,
                            DEMO_LCD_BRIGHTNESS_PHASE_TEST ?
                            DEMO_LCD_DIAGNOSTIC_LOW_DUTY : DEMO_LCD_BACKLIGHT_DUTY)) {
        printf("[BRINGUP] LCD backlight PWM failed\n");
    }
    next_color_at = sys_timer_get_ms();
#if DEMO_LCD_BRIGHTNESS_PHASE_TEST
    brightness_high = 0;
    next_brightness_at = next_color_at + 15000;
#endif
}

void ac7076a3_demo_lcd_poll(void)
{
    u32 now = sys_timer_get_ms();
#if DEMO_LCD_BRIGHTNESS_PHASE_TEST
    if ((s32)(now - next_brightness_at) >= 0) {
        brightness_high = !brightness_high;
        power_gate_pwm_set_duty(IO_LCD_PG, brightness_high ?
                                DEMO_LCD_BACKLIGHT_DUTY :
                                DEMO_LCD_DIAGNOSTIC_LOW_DUTY);
        next_brightness_at = now + 15000;
        printf("[BRINGUP] LCD backlight phase=%u%%\n",
               brightness_high ? 80 : 20);
    }
#endif
#if DEMO_BLE_ENABLE
    ble_screen_poll(now);
#elif DEMO_I2C_SCREEN_ENABLE
    i2c_screen_poll(now);
#elif DEMO_ACCEL_SCREEN_ENABLE && DEMO_ACCEL_ENABLE
    accel_screen_poll(now);
#elif DEMO_LCD_INTERNAL_RAM_FILL
    static u8 next_white = 1;
    u8 level;
    u8 fill_enable;
    if (!demo_lcd || (s32)(now - next_color_at) < 0) {
        return;
    }
    next_color_at = now + 2000;
    level = next_white ? 0xfc : 0x00; /* six bits occupy D7..D2 */
    /* JD9855 RAMCLSET{R,G,B} + RAMCLACT: the controller fills its own RAM.
     * This probes single-lane command delivery without QSPI pixel traffic. */
    fill_enable = 0;
    lcd_write_cmd(0x4c, &fill_enable, 1);
    lcd_write_cmd(0x4d, &level, 1);
    lcd_write_cmd(0x4e, &level, 1);
    lcd_write_cmd(0x4f, &level, 1);
    fill_enable = 1;
    lcd_write_cmd(0x4c, &fill_enable, 1);
    printf("[BRINGUP] JD9855 internal %s RAM fill requested\n",
           next_white ? "white" : "black");
    next_white = !next_white;
#elif !DEMO_LCD_COLOR_CYCLE_ENABLE
    static u8 filled;
    if (filled || !demo_lcd) {
        return;
    }
    filled = 1;
    demo_lcd->clear_screen(0xffffff, DEMO_LCD_X_OFFSET,
        DEMO_LCD_X_OFFSET + DEMO_LCD_WIDTH - 1, DEMO_LCD_Y_OFFSET,
        DEMO_LCD_Y_OFFSET + DEMO_LCD_HEIGHT - 1);
    printf("[BRINGUP] LCD static white frame written; no more pixel traffic\n");
#else
    static const u32 colors[] = {0xff0000, 0x00ff00, 0x0000ff, 0xffffff, 0};
    if (!demo_lcd || (s32)(now - next_color_at) < 0) {
        return;
    }
    next_color_at = now + DEMO_LCD_COLOR_DWELL_MS;
    demo_lcd->clear_screen(colors[color_index], DEMO_LCD_X_OFFSET,
        DEMO_LCD_X_OFFSET + DEMO_LCD_WIDTH - 1, DEMO_LCD_Y_OFFSET,
        DEMO_LCD_Y_OFFSET + DEMO_LCD_HEIGHT - 1);
    printf("[BRINGUP] LCD color=%06x TE=%d (visually verify)\n",
           colors[color_index], gpio_read(IO_PORTC_00));
    color_index = (color_index + 1) % ARRAY_SIZE(colors);
#endif
}

void ac7076a3_demo_lcd_task(void *priv)
{
    (void)priv;
    ac7076a3_demo_lcd_start();
    while (1) {
        ac7076a3_demo_lcd_poll();
        os_time_dly(2);
    }
}
#endif
