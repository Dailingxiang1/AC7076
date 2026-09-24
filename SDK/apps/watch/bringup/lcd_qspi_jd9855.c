#include "app_config.h"
#include "includes.h"
#include "system/timer.h"
#include "ui/ui_api.h"
#include "ui/lcd/lcd_drive.h"
#include "asm/power/power_gate.h"
#include "ac7076a3_demo.h"

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
void ac7076a3_demo_lcd_start(void)
{
    extern const struct ui_devices_cfg ui_cfg_data;
    demo_lcd = lcd_get_hdl();
    if (!demo_lcd || !demo_lcd->init || !demo_lcd->clear_screen) {
        printf("[BRINGUP] LCD interface unavailable\n");
        demo_lcd = NULL;
        return;
    }
    printf("[BRINGUP] JD9855 QSPI init %ux%u, provisional panel profile\n",
           DEMO_LCD_WIDTH, DEMO_LCD_HEIGHT);
    /* .init is void in the SDK: success here does NOT prove panel response. */
    demo_lcd->init((void *)&ui_cfg_data);
    if (power_gate_pwm_init(IO_LCD_PG, 10000, DEMO_LCD_BACKLIGHT_DUTY)) {
        printf("[BRINGUP] LCD backlight PWM failed\n");
    }
    next_color_at = sys_timer_get_ms();
}

void ac7076a3_demo_lcd_poll(void)
{
    static const u32 colors[] = {0xff0000, 0x00ff00, 0x0000ff, 0xffffff, 0};
    u32 now = sys_timer_get_ms();
    if (!demo_lcd || (s32)(now - next_color_at) < 0) {
        return;
    }
    next_color_at = now + 1500;
    demo_lcd->clear_screen(colors[color_index], DEMO_LCD_X_OFFSET,
        DEMO_LCD_X_OFFSET + DEMO_LCD_WIDTH - 1, DEMO_LCD_Y_OFFSET,
        DEMO_LCD_Y_OFFSET + DEMO_LCD_HEIGHT - 1);
    printf("[BRINGUP] LCD color=%06x TE=%d (visually verify)\n",
           colors[color_index], gpio_read(IO_PORTC_00));
    color_index = (color_index + 1) % ARRAY_SIZE(colors);
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
