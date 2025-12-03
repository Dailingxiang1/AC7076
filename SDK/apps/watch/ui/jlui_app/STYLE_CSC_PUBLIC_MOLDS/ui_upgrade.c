#include "app_config.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "ui_draw/ui_type.h"
#include "jlui_app/style_upgrade_new.h"
#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-UPGRATE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_upgrate.data.bss")
#pragma data_seg(".ui_action_upgrate.data")
#pragma const_seg(".ui_action_upgrate.text.const")
#pragma code_seg(".ui_action_upgrate.text")
#endif

#ifdef  CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE
#if TCFG_UI_ENABLE_UPGRATE

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

extern void set_lcd_keep_open_flag(u8 flag);
struct upgrade_info {
    u8 cur_progress;
    u8 cnt: 4;
    u8 steps: 4;
    u16 tid;
    u32 last_msec;
};
#define RGB565(r,g,b) (((((u8)r)>>3)<<11)|((((u8)g)>>2)<<5)|(((u8)b)>>3))
#define STEP_TOTAL          3//最多分3步
#define BG_COLOR_565        RGB565(128,128,128)//背景色
#define CNT_FG_COLOR        RGB565(255,255,255)//自增计数值前景色
#define STEP_FG_COLOR       RGB565(255,255,145)//阶段前景色
#define PROGRESS_FG_COLOR   RGB565(161,250,79)//圆环前景色
static struct upgrade_info info = {0};
#define __this (&info)
//禁止用字库等升级过程中不跟随代码一起擦除的内容，如字库文本,或使用字库的文本
//避免在升级过程中使用复杂的自定义绘图

static void upgrade_redraw(void *p)
{
    struct element *elm = ui_core_get_element_by_id(STYLE_UPGRADE_ID(UPGRADE_LAYOUT));
    if (elm) {
        ui_core_redraw(elm);
    }
}

static int upgrade_handler(const char *type, u32 arg)
{
    static char process_record = -1;
    log_info("msg test %s %s %d \n", __FUNCTION__, type, arg);
    struct unumber num;
    /* ui_progress_set_persent_by_id(STYLE_UPGRADE_ID(UPGRADE_PROGRESS_BCAKUP), 100); */
    if (type && !strcmp(type, "process")) {
        __this->cur_progress = arg;
    }

    if (type && !strcmp(type, "file_num")) {
        __this->steps = arg;
    }
    /* upgrade_redraw(0); */
    return 0;
}


static const struct uimsg_handl ui_msg_handler[] = {
    { "upgrade",        upgrade_handler     }, //
    { NULL, NULL},      /* 必须以此结尾！ */
};

static int upgrade_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    log_info("window call = %s %d id =%x \n", __FUNCTION__, __LINE__, window->elm.id);

    switch (e) {
    case ON_CHANGE_INIT:
        if (!ui_auto_shut_down_disable()) {
            set_lcd_keep_open_flag(1);
        }
        if (__this && (!__this->tid)) {
            __this->tid = sys_timer_add(NULL, upgrade_redraw, 300);
        }
        ui_register_msg_handler(window->elm.id, ui_msg_handler);
        break;

    case ON_CHANGE_RELEASE:
        if (__this && __this->tid) {
            sys_timer_del(__this->tid);
            __this->tid = 0;
            memset(__this, 0, sizeof(struct upgrade_info)); //退出时清除
        }
        set_lcd_keep_open_flag(0);
        ui_auto_shut_down_enable();
        break;

    default:
        return false;
    }
    return false;
}


static int upgrade_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
    case ELM_EVENT_TOUCH_L_MOVE:
        return true;
    default:
        break;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(ID_WINDOW_UPGRADE)
.onchange = upgrade_onchange,
 .onkey    = NULL,
  .ontouch  = upgrade_ontouch,
};


int gpu_fill_rect(struct draw_context *dc, int left, int top, int width, int height, u32 acolor);
static int upgrade_display_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct element *elm = (struct element *)ctr;
    struct rect r;
    switch (e) {

    case ON_CHANGE_SHOW_POST:
        if (!__this) {
            break;
        }
        struct draw_context *dc = (struct draw_context *)arg;
        ui_core_get_element_abs_rect(elm, &r);
        int cx = r.left + r.width / 2;
        int cy = r.top + r.height / 2;
        int ring_r = r.height / 2 - 10;
        int bar_width = r.height / 2;
        //圆环背景
        ui_draw_ring(arg, cx, cy, ring_r, ring_r - 20, 0, 360, BG_COLOR_565, 100);
        //环
        ui_draw_ring(arg, cx, cy, ring_r, ring_r - 20, 0 + 270, 360 + 270, PROGRESS_FG_COLOR, __this->cur_progress);
        /* ui_draw_bar(arg, cx - bar_width / 2, cy - 40, bar_width, 30, BG_COLOR_565, 100, 0); */
        gpu_fill_rect(arg, cx - bar_width / 2, cy - 20, bar_width, 20, (0xff << 24) | BG_COLOR_565);
        int percent = __this->cnt * 10;
        /* ui_draw_bar(arg, cx - bar_width / 2, cy - 40, bar_width, 30, CNT_FG_COLOR, percent, 0); */
        gpu_fill_rect(arg, cx - bar_width / 2, cy - 20, bar_width * percent / 100, 20, (0xff << 24) | CNT_FG_COLOR);
        __this->cnt++;
        __this->cnt %= 10;
        if (__this->steps) {
            int draw_left = cx - 30;
            int count = 0;
            for (; count < __this->steps; count++) {
                /* ui_draw_bar(arg, draw_left + count * 30, cy + 40, 10, 20, STEP_FG_COLOR, 100, 0); */
                gpu_fill_rect(arg, draw_left + count * 30, cy + 20, 10, 10, (0xff << 24) | STEP_FG_COLOR);
            }
            for (; count < STEP_TOTAL; count++) {
                /* ui_draw_bar(arg, draw_left + count * 30, cy + 40, 10, 20, BG_COLOR_565, 100, 0); */
                gpu_fill_rect(arg, draw_left + count * 30, cy + 20, 10, 10, (0xff << 24) |  BG_COLOR_565);
            }
        }

        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(STYLE_UPGRADE_ID(UPGRADE_LAYOUT))
.onchange = upgrade_display_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};

#endif/* #if TCFG_UI_ENABLE_UPGRATE */
#endif/*#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE*/






