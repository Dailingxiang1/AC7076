/**
 * @file lv_port_disp_templ.h
 *
 */

/*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_DISP_TEMPL_H
#define LV_PORT_DISP_TEMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define TCFG_LCD_FB_CNT     2  //配置多少个行buffer
#define TCFG_LCD_FB_LINES   16 //配置每个buffer多少行,最大 LCD_HEIGHT

extern const uint8_t TCFG_DEBUG_RENDER_LCD_TIME;  //统计刷屏帧率,以及UI框架渲染一帧时间,以及显示屏驱动造成的延时时间

//启动推屏时te可以越过推屏区域起始y坐标的最大值;如果页面合成速度小于1TE周期则该值较大更好，
//如果合成速度较慢则该值较小更好;如果出现切线尽量减小改值
extern const uint16_t lvgl_te_contenue;


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_disp_init(void *param);

void debug_draw_start_time_us_record(void);

void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

uint64_t *get_debug_lcd_latency_uspf(void);
/**********************
 *      MACROS
 **********************/

int64_t get_system_us(void);

unsigned int time_lapse(unsigned int *handle, unsigned int time_out);

uint32_t lv_get_phy_addr(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/

#endif /*Disable/Enable content*/
