
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lcd_rgb_480x272.data.bss")
#pragma data_seg(".lcd_rgb_480x272.data")
#pragma const_seg(".lcd_rgb_480x272.text.const")
#pragma code_seg(".lcd_rgb_480x272.text")
#endif
/*
** 包含board的头文件，确定baord里面开关的屏驱宏
*/
#include "app_config.h"


/*
** 驱动代码的宏开关
** 注意：rgb屏需要一直推才能正常显示
*/
#if TCFG_LCD_RGB_ENABLE


#define LCD_DRIVE_CONFIG                    RGB_SPI_3WIRE_RGB888

/*
** 包含imd头文件，屏驱相关的变量和结构体都定义在imd.h
*/
#include "system/includes.h"
#include "ui/ui_api.h"
#include "ui/lcd/lcd_drive.h"
#include "ui/lcd/lcd_conf.h"

/*
 * 推RGB屏有以下配置需要打开
 * <<sdk_config.h>>
 * #define TCFG_PSRAM_DEV_ENABLE 1 //需要内置PSRAM的封装
 * #define TCFG_PSRAM_SIZE 0x200000 //内置PSRAM容量
 * <<lib_jlui_config.c>>
 * const int JLUI_GPU_DMA_TO_PSRAM = 1
 * const int JLUI_LCD_RAMLESS_ENABLE = 1
 * <<board_ac707n_demo_cfg.h>>
 * #define TCFG_LCD_RGB_ENABLE 1 // 关闭其他屏驱
 * #define TCFG_LCD_TE_USED_PEND 0
 * */


#define SCR_X       0
#define SCR_Y       0
#define SCR_W       LCD_WIDTH
#define SCR_H       LCD_HEIGHT
#define LCD_W       LCD_WIDTH
#define LCD_H       LCD_HEIGHT
#define LCD_BLOCK_W LCD_WIDTH
#define LCD_BLOCK_H 16
#define BUF_NUM     2


static struct dbi_param rgb_param = {
    .scr_x    = SCR_X,
    .scr_y	  = SCR_Y,
    .scr_w	  = SCR_W,
    .scr_h	  = SCR_H,
    .in_width  = SCR_W,
    .in_height = SCR_H,

    .lcd_width  = LCD_W,
    .lcd_height = LCD_H,

    .in_format = OUTPUT_FORMAT_RGB565,//-1,
    .lcd_type = LCD_TYPE_RGB,

    .buffer_num	= 2,
    .buffer_size = LCD_BLOCK_W * LCD_BLOCK_H * 2,
    .fps = 60,

    .rgb = {
        .out_format = OUT_FORMAT(LCD_DRIVE_CONFIG),
        .continue_frames = 0,

        .hpw_prd = 3,
        .hbw_prd = 42 + 1,
        .hfw_prd = 102 * 1,
        .hact_prd = 480,

        .vpw_prd = 2,
        .vbw_prd = 11,
        .vfw_prd = 8,
        .vact_prd = 272,
    },

    .debug_mode_en = false,
    .debug_mode_color = 0xff0000,
};

/*
** lcd背光控制
** 考虑到手表应用lcd背光控制需要更灵活自由，可能需要pwm调光，随时亮灭等
** 因此内部不操作lcd背光，全部由外部自行控制
*/


/*
** 设置lcd进入睡眠
*/
static void lcd_entersleep(void)
{
    //TODO
}

/*
** 设置lcd退出睡眠
*/
static void lcd_exitsleep(void)
{
    //TODO
}


REGISTER_LCD_DEVICE(rgb) = {
    .logo = "rgb",

    .lcd_cmd = NULL,
    .cmd_cnt = 0,
    .param	= (void *) &rgb_param,

    .reset			= NULL,// 没有特殊的复位操作，用内部普通复位函数即可
    .backlight_ctrl = NULL,
    .entersleep		= NULL,//lcd_entersleep,
    .exitsleep		= NULL,//lcd_exitsleep,
};



#endif
