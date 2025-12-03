#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lcd_lvgl_api.data.bss")
#pragma data_seg(".lcd_lvgl_api.data")
#pragma const_seg(".lcd_lvgl_api.text.const")
#pragma code_seg(".lcd_lvgl_api.text")
#endif

#define BOOL_DEFINE_CONFLICT

#include "sdk_config.h"

#if CONFIG_LVGL_UI_ENABLE
#include "lvgl_v8/lvgl.h"
#include "lvgl_v8/demos/lv_demos.h"
#include "lvgl_v8/examples/lv_examples.h"
#include "lcd_lvgl_api.h"
#include "ui_core.h"
#include "dbi.h"
#include "lcd/lcd_drive.h"
#include "btstack/avctp_user.h"
#include "lcd/lcd_lvgl_api.h"


struct lv_server_env {
    u8 init: 1;
    u8 key_lock : 1;
    u8 lcd_bl_idle_flag : 1;
    u8 lcd_need_keep_open: 1;
    u8 open_flag: 1;
    u16 ui_auto_shut_down_timer;
    u16 shut_down_time;
};

#if     TCFG_SDFILE_INSERT_FLASH_ENABLE
#define LVGL_MODE_PHY_BASE            TCFG_MODE_INSERT_FLASH_BASE
#else
#define LVGL_MODE_PHY_BASE  0
#endif

static const u32 lv_phy_addr = LVGL_MODE_PHY_BASE;
static u8 screen_saver_status = 0;
static struct lv_server_env __ui_display = {0};
#define __this (&__ui_display)

#define UI_TASK_NAME	"ui"
int lcd_sleep_ctrl(u8 enter);
int lcd_sleep_status();
void lcd_bl_open();
static u8 lv_auto_shut_down_disable(void);
static void lv_auto_shut_down_enable(void);
static void lv_lcd_sleep_out(void);
static void lv_lcd_sleep_in(void);
void lcd_wait_busy();
void lvgl_ui_task_suspend_resume(uint8_t state);
int ui_disp_line_buf_uninit();
void lv_set_lcd_keep_open_flag(u8 flag);
void lv_set_screen_saver_status(u8 status);
int ui_gpu_buf_release();
void gpu_free_all_tasks();

#if LCD_POWER_DOWN_EN
void lcd_reinit_wait_finish(void);
#endif

u32 lv_get_phy_addr(void)
{
    return lv_phy_addr;
}

//进入屏保
void lv_screen_saver(void *p)
{
    if (!lcd_sleep_status()) {
        //关闭自动息屏定时器
        lv_auto_shut_down_disable();
        //lvgl挂起
        lvgl_ui_task_suspend_resume(0);
        lv_lcd_sleep_in();
    }
}

//退出屏保
int lv_screen_recover(void)
{
    if (lv_get_screen_saver_status()) {

        lv_lcd_sleep_out();
        __this->lcd_bl_idle_flag = 0;

#if LCD_POWER_DOWN_EN
        lcd_reinit_wait_finish();
#endif
        lcd_bl_open();//开启屏幕背光

        lv_set_screen_saver_status(0);

        //lvgl全屏刷新
        lv_area_t area_p;
        area_p.x1 = 0;
        area_p.y1 = 0;
        area_p.x2 = lcd_get_screen_width();
        area_p.y2 = lcd_get_screen_height();
        _lv_inv_area(NULL, &area_p);

        return 0;
    }
    return 1;
}

uint8_t lv_get_screen_saver_status()
{
    return screen_saver_status;
}

void lv_set_screen_saver_status(u8 status)
{
    screen_saver_status = status;
}

uint8_t lv_get_ui_open_flag()
{
    return __this->open_flag;
}

void lv_set_ui_open_flag(uint8_t flag)
{
    __this->open_flag = !!flag;
}

void lv_set_lcd_keep_open_flag(uint8_t flag)
{
    __this->lcd_need_keep_open = !!flag;
}

void lv_ui_auto_shut_down_re_run(void)
{
#if TCFG_UI_SHUT_DOWN_TIME
    if (__this->ui_auto_shut_down_timer) {
        sys_timer_re_run(__this->ui_auto_shut_down_timer);
    }
#endif
}

void set_lcd_bl_idle_flag(u8 flag)
{
    __this->lcd_bl_idle_flag = flag;
}

static u8 lv_lcd_bl_idle_query(void)
{
    return __this->lcd_bl_idle_flag;
}
REGISTER_LP_TARGET(lv_lcd_backlight_lp_target) = {
    .name = "lv_lcd_backlight",
    .is_idle = lv_lcd_bl_idle_query,
};

static int ui_relate_uninit()
{
    LV_LOG_INFO("ui relate uninit.\n");
    gpu_free_all_tasks();
    /* ui_disp_line_buf_uninit(); */
    /* ui_gpu_buf_release(); */
    return 0;
}

static void lv_lcd_sleep_in(void)
{
    if (!__ui_display.init) {
        return;
    }
    __this->open_flag = 0;

    if (!lcd_sleep_status()) {
        lcd_clear(0, 0, lcd_get_screen_width() - 1, 0, lcd_get_screen_height() - 1);
        lcd_sleep_ctrl(true);//进入低功耗
        ui_relate_uninit();
        __this->lcd_bl_idle_flag = 1;
        lv_set_screen_saver_status(1);//设置屏幕处于休眠状态
    }
}

static void lv_lcd_sleep_out(void)
{
    if (!__ui_display.init) {
        return;
    }

    if (__this->open_flag) {
        return;
    }

    if (lv_get_screen_saver_status()) {
        __this->open_flag = 1;
        lv_auto_shut_down_enable();//开启自动息屏
        lcd_sleep_ctrl(false);     //退出低功耗
        lcd_wait_busy();
    }
}

//开启自动息屏定时器
static void lv_auto_shut_down_enable(void)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
#if TCFG_UI_SHUT_DOWN_TIME
    if (!__ui_display.init) {
        return;
    }
    if (bt_get_call_status() != BT_CALL_HANGUP) {
        LV_LOG_INFO("%s call_status:%d", __func__, bt_get_call_status());
        return;
    }
    if (__this->lcd_need_keep_open) {
        LV_LOG_INFO("need keep open");
        return;
    }
    if (__this->ui_auto_shut_down_timer == 0) {
        if (__ui_display.shut_down_time == 0) {
            __ui_display.shut_down_time = 10;
        }
        if (__ui_display.shut_down_time > 20) {
            /* __ui_display.shut_down_time = 20; */
        }
        LV_LOG_INFO("[%s]time:%d rets:0x%x\n", __func__, __ui_display.shut_down_time, rets);
        __this->ui_auto_shut_down_timer = sys_timeout_add(NULL, lv_screen_saver, __ui_display.shut_down_time * 1000);
    }
#endif
}

//关闭自动息屏定时器
u8 lv_auto_shut_down_disable(void)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
#if TCFG_UI_SHUT_DOWN_TIME
    if (__this->ui_auto_shut_down_timer) {
        LV_LOG_INFO("[%s]rets:0x%x", __func__, rets);
        sys_timeout_del(__this->ui_auto_shut_down_timer);
        __this->ui_auto_shut_down_timer = 0;
        return true;
    }
    return false;
#endif
    return false;
}

//初始化息屏管理
void lv_screen_manage_init(void)
{
    __this->init = 1;
    //添加自动熄屏定时器
    lv_auto_shut_down_enable();
}





#endif//CONFIG_LVGL_UI_ENABLE



