#ifndef AC7076A3_DEMO_CONFIG_H
#define AC7076A3_DEMO_CONFIG_H

/* Board1 / SCH_Schematic1_2026-09-10.pdf. Set master to 0 for original app. */
#ifndef AC7076A3_DEMO_ENABLE
#define AC7076A3_DEMO_ENABLE       1
#endif

/* Board1 reports 4 MiB internal NOR. The standalone peripheral demo does
 * not start the watch UI and cannot carry its 8 MiB resource layout.
 * This header is included after the generated sdk_config.h, before the
 * board's flash partition definitions and the linker/INI generators.
 */
#if AC7076A3_DEMO_ENABLE
#undef CONFIG_FLASH_SIZE
#define CONFIG_FLASH_SIZE         0x400000
#undef TCFG_UI_ENABLE
#define TCFG_UI_ENABLE            0
#undef CONFIG_JL_UI_ENABLE
#define CONFIG_JL_UI_ENABLE       0
#undef CONFIG_LVGL_UI_ENABLE
#define CONFIG_LVGL_UI_ENABLE     0
#endif
#ifndef DEMO_LED_ENABLE
#define DEMO_LED_ENABLE           1
#endif
#ifndef DEMO_KEY_ENABLE
#define DEMO_KEY_ENABLE           1
#endif
#ifndef DEMO_MOTOR_ENABLE
#define DEMO_MOTOR_ENABLE         1  /* one short pulse on each key press */
#endif
#ifndef DEMO_I2C_SCAN_ENABLE
#define DEMO_I2C_SCAN_ENABLE       1
#endif
#ifndef DEMO_TOUCH_ENABLE
#define DEMO_TOUCH_ENABLE         1
#endif
#ifndef DEMO_ACCEL_ENABLE
#define DEMO_ACCEL_ENABLE         1
#endif
#ifndef DEMO_POWER_ENABLE
#define DEMO_POWER_ENABLE         1  /* VBAT and USB VPWR voltage, no charging */
#endif
#ifndef DEMO_USB_CDC_ENABLE
#define DEMO_USB_CDC_ENABLE       0  /* disabled while bringing up LCD */
#endif
#ifndef DEMO_SPEAKER_ENABLE
#define DEMO_SPEAKER_ENABLE       0  /* short 1 kHz tone on key press */
#endif
#ifndef DEMO_MIC_ENABLE
#define DEMO_MIC_ENABLE           0  /* peak/mean absolute value, no feedback */
#endif
#ifndef DEMO_LCD_ENABLE
#define DEMO_LCD_ENABLE           1
#endif

/* UART fallback only when DEMO_USB_CDC_ENABLE=0; default CDC uses no PA2. */
#define DEMO_UART_TX              IO_PORTA_02
#define DEMO_UART_BAUD            115200
#define DEMO_LED_PIN              IO_PORTB_00 /* low = on */
#define DEMO_KEY_PIN              IO_PORTB_07 /* low = pressed */
#define DEMO_SDA_PIN              IO_PORTB_01
#define DEMO_SCL_PIN              IO_PORTB_02
#define DEMO_TP_RST_PIN           IO_PORTA_06
#define DEMO_TP_IRQ_PIN           IO_PORTA_05
#define DEMO_TOUCH_ADDR           0x15        /* 7-bit; configurable firmware */
#define DEMO_ACCEL_ADDR           0x27        /* DA213B, 7-bit */
#define DEMO_MOTOR_DUTY           2500        /* 25%, 0..10000 */
#define DEMO_MOTOR_PULSE_MS        150
#define DEMO_SPEAKER_VOLUME       4
#define DEMO_SPEAKER_DURATION_MS  200
#define DEMO_MIC_GAIN             4

/* User-confirmed geometry: 360 x 360 round JD9855 QSPI module. */
#define DEMO_LCD_WIDTH            360
#define DEMO_LCD_HEIGHT           360
#define DEMO_LCD_X_OFFSET         0
#define DEMO_LCD_Y_OFFSET         0
#define DEMO_LCD_FPS              10
#define DEMO_LCD_BACKLIGHT_DUTY   2000        /* PG low duty, 20% */
#define DEMO_LCD_PANEL_CONFIRMED   1
#define DEMO_I2C_ENABLE (DEMO_I2C_SCAN_ENABLE || DEMO_TOUCH_ENABLE || DEMO_ACCEL_ENABLE)

#if AC7076A3_DEMO_ENABLE
#if DEMO_LCD_ENABLE && !DEMO_LCD_PANEL_CONFIRMED
#error "JD9855: confirm module resolution and provide vendor init table first"
#endif
#if DEMO_LCD_WIDTH <= 0 || DEMO_LCD_HEIGHT <= 0 || (DEMO_LCD_WIDTH % 2)
#error "JD9855 geometry must be positive and width must be even"
#endif
#if DEMO_MOTOR_DUTY > 10000 || DEMO_MOTOR_PULSE_MS > 500
#error "Bringup motor pulse exceeds configured demo limits"
#endif
#if DEMO_MOTOR_ENABLE && !DEMO_KEY_ENABLE
#error "Motor demo requires the key demo (no autonomous motor loop)"
#endif
#if DEMO_SPEAKER_ENABLE && !DEMO_KEY_ENABLE
#error "Speaker demo requires the key demo"
#endif
#endif
#endif
