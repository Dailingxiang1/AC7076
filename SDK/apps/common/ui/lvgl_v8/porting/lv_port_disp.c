#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lv_ui_core.data.bss")
#pragma data_seg(".lv_ui_core.data")
#pragma const_seg(".lv_ui_core.text.const")
#pragma code_seg(".lv_ui_core.text")
#endif
/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
/*********************
 *      INCLUDES
 *********************/
#define BOOL_DEFINE_CONFLICT
#include "lv_port_disp.h"
#include "lvgl.h"
#include "rect.h"
#include "os/os_api.h"
#include "perf_counter/perf_counter.h"
#include "ui/lcd/lcd_drive.h"
#include "ui/lcd/lcd_conf.h"

/*********************
 *      DEFINES
 *********************/
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void *param);
void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void lv_port_flush_init();



/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
#if ((TCFG_LCD_FB_CNT==1) || LV_USE_GPU_LIST_DRAW)
static OS_SEM lv_disp_flush_wait_sem;
static void lcd_flush_finish_callback(int err)
{
    os_sem_post(&lv_disp_flush_wait_sem);
}
static void lv_disp_flush_wait_finish(struct _lv_disp_drv_t *disp_drv)
{
    u64 temp_system_us = get_system_us();

    while (1) {
        int err = os_sem_pend(&lv_disp_flush_wait_sem, 10);
        if (err == OS_NO_ERR) {
            break;
        } else if (err == OS_TIMEOUT) {
            printf("lv_disp_flush_wait_finish timeout..\n");
            continue;
        }
    }
    lv_disp_flush_ready(disp_drv);

    u64 *debug_lcd_latency_uspf = get_debug_lcd_latency_uspf();
    *debug_lcd_latency_uspf += (get_system_us() - temp_system_us);
}

#endif


static void disp_rounder(struct _lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    u8 row, column;
    lcd_get_align(&row, &column);
    row--;
    column--;
    area->x1 = area->x1 & (~row);
    area->x2 = (area->x2 & (~row)) + row;
    area->y1 = area->y1 & (~column);
    area->y2 = (area->y2 & (~column)) + column;
}

void lv_port_disp_init(void *param)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init(param);

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */

    static lv_disp_draw_buf_t draw_buf_dsc_2;
    static lv_color_t buf_2[TCFG_LCD_FB_CNT][LCD_WIDTH * TCFG_LCD_FB_LINES] __attribute__((aligned(4)));                    /*A buffer for N rows*/

#if LV_USE_GPU_LIST_DRAW
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2[0], TCFG_LCD_FB_CNT == 1 ? NULL : buf_2[1], LCD_WIDTH * LCD_HEIGHT); /*Initialize the display buffer*/
#else
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2[0], TCFG_LCD_FB_CNT == 1 ? NULL : buf_2[1], LCD_WIDTH * TCFG_LCD_FB_LINES); /*Initialize the display buffer*/
#endif


#if LV_USE_GPU_LIST_DRAW
    lv_gpu_task_list_port_init(LCD_WIDTH,  LCD_HEIGHT); /*GPU模块初始化*/
    struct rect rect;
    jlgpu_get_win_rect(&rect);
    rect.left = 0;
    rect.top = 0;
    jlgpu_set_win_rect(&rect); /*调整GPU窗口位置*/
#endif

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    u8 row, column;
    lcd_get_align(&row, &column);
    if (row > 1 || column > 1) {
        LV_LOG_INFO("lcd pixel need disp_rounder");
        disp_drv.rounder_cb = disp_rounder;
    }

#if ((TCFG_LCD_FB_CNT==1) || LV_USE_GPU_LIST_DRAW)
    os_sem_create(&lv_disp_flush_wait_sem, 0);
    disp_drv.wait_cb = lv_disp_flush_wait_finish;
#endif

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_2;

    disp_drv.full_refresh = LV_DISPLAY_FULL_REFRESH;//0:局部区域重绘,适合MCU屏, 1:整帧刷,适合RGB/MIPI屏
    disp_drv.direct_mode = LV_DISPLAY_DIRECT_MODE;//1：按照绝对坐标去修改帧FB像素，仅适用于RGB/MIPI屏

    /*Finally register the driver*/
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    lv_timer_del(disp->refr_timer);
    disp->refr_timer = NULL;
    lv_port_flush_init();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/*Initialize your display and the required peripherals.*/
static void disp_init(void *param)
{
    printf("lv disp_init");

    struct lcd_interface *lcd;

    lcd = lcd_get_hdl();
    ASSERT(lcd);

    if (lcd->init) {
        lcd->init(param);
    }

#if ((TCFG_LCD_FB_CNT==1) || LV_USE_GPU_LIST_DRAW)
    lcd_draw_set_callback(lcd_flush_finish_callback);
#endif

    //开背光
    if (lcd->backlight_ctrl) {
        lcd->backlight_ctrl(100);
    }
}

