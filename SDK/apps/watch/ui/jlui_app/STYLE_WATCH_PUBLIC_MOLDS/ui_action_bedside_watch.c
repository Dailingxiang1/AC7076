#include "app_config.h"
/* #include "app_task.h" */
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "rtc.h"
#include "app_mode_manager/app_mode_manager.h"
#include "app_task.h"

#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_BEDSIDE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_draw_demo.data.bss")
#pragma data_seg(".ui_action_draw_demo.data")
#pragma const_seg(".ui_action_draw_demo.text.const")
#pragma code_seg(".ui_action_draw_demo.text")
#endif

#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_BEDSIDE_WATCH_ENABLE

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)
#define BEDSIDE_WATCH_TEST_EN		 (!TCFG_SPORT_HEALTH_ENABLE)//不使能sensor时，通过触摸屏幕调整方向

extern u8 get_vbat_percent(void);
#define RGB565(r,g,b) (((((u8)r)>>3)<<11)|((((u8)g)>>2)<<5)|(((u8)b)>>3))
enum {
    BEDSIDE_WATCH_DIR_TOP,
    BEDSIDE_WATCH_DIR_BTM,
    BEDSIDE_WATCH_DIR_LEFT,
    BEDSIDE_WATCH_DIR_RIGHT,
};
struct bedside_watch_ctrl {
    u8 bedside_watch_dir : 2;
    u8 last_dir: 2;
    u8 target_dir: 2;
    u16 timer_id;
    ui_anim_t *anim_hd;
};
static struct bedside_watch_ctrl  __bedside_watch_ctrl = {0};

#define __this	(&__bedside_watch_ctrl)
__attribute__((weak))
u8 gsensor_dir_get()
{
    return 0;
}
int bedside_watch_dir_get()
{
    return __this->bedside_watch_dir;
}
int bedside_watch_dir_set(int dir)
{
    __this->bedside_watch_dir = dir;
    return __this->bedside_watch_dir;
}
static int bedside_watch_angle_map(int dir)
{
    int angle = 0;
    switch (dir) {
    case BEDSIDE_WATCH_DIR_TOP:
        angle = 0;
        break;
    case BEDSIDE_WATCH_DIR_BTM:
        angle = 180;
        break;
    case BEDSIDE_WATCH_DIR_LEFT:
        angle = 270;
        break;
    case BEDSIDE_WATCH_DIR_RIGHT:
        angle = 90;
        break;
    default:
        ASSERT(0);
        break;
    }
    /* printf("%s dir:%d angle:%d", __func__, dir, angle); */
    return angle;
}
static void bedside_watch_anim_exec_cb(int var, int32_t v)
{
    struct element *elm  = ui_core_get_element_by_id(var);
    if (!elm) {
        return;
    }
    if (!__this->anim_hd) {
        return;
    }
    struct rect elm_r;
    struct rect parent_r;

    ui_core_get_element_abs_rect(elm, &elm_r);
    ui_core_get_element_abs_rect(elm->parent, &parent_r);
    int cx = elm_r.width / 2;
    int cy = elm_r.height / 2;
    int dx = parent_r.left + parent_r.width / 2;
    int dy = parent_r.top + parent_r.height / 2;
    float angle = (float)v;
    ui_core_set_element_rotate((void *)elm, cx, cy, dx, dy, angle, 1);
    ui_core_redraw(elm->parent);
}


static int bedside_watch_anim_stop()
{
    if (__this->anim_hd) {
        free(__this->anim_hd);
        __this->anim_hd = NULL;
    }
    return 0;
}
static int bedside_watch_anim_start(struct element *elm)
{
    log_error("%s wdir:%d tdir:%d", __func__, __this->bedside_watch_dir, __this->target_dir);
    if (__this->bedside_watch_dir  == __this->target_dir) {
        return -1;
    }

    bedside_watch_anim_stop();
    __this->anim_hd = zalloc(sizeof(ui_anim_t));
    ASSERT(__this->anim_hd);
    int cur_angle = 0;
    if (elm->css.part) {
        cur_angle = (int)elm->css.part->rotate.angle;
    }

    int target_angle = bedside_watch_angle_map(__this->bedside_watch_dir);
    if ((target_angle - cur_angle) > 180) {
        target_angle -= 360;
    }
    if ((cur_angle - target_angle) > 180) {
        target_angle += 360;
    }

    int run_time = 200;
    __this->target_dir = __this->bedside_watch_dir;
    ui_anim_init(__this->anim_hd);
    ui_anim_set_var(__this->anim_hd, elm->id);
    ui_anim_set_path_cb(__this->anim_hd, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(__this->anim_hd, bedside_watch_anim_exec_cb);		// 运行回调
    ui_anim_set_values(__this->anim_hd, cur_angle, target_angle);		// 路径设置
    ui_anim_set_time(__this->anim_hd, run_time);						// 运行时间设置
    ui_anim_start(__this->anim_hd);
    return 0;
}

static int bedside_watch_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_SHOW_PROBE:
        if (elm->id == BEDSIDE_WATCH_TIMER) {
            struct sys_time time;
            rtc_read_time(&time);
            ui_core_set_element_ratio((void *)elm, 2.0f, 2.0f, 1);
            ui_time_update((struct ui_time *)elm, (struct utime *)&time);
        } else if (elm->id == BEDSIDE_WATCH_PIC) {
            ui_core_set_element_ratio((void *)elm, 2.0f, 2.0f, 1);
        } else if (elm->id == BEDSIDE_WATCH_BAT_NUM) {
            struct unumber unum;
            unum.type = TYPE_NUM;
            unum.numbs = 1;
            unum.number[0] =  get_vbat_percent();
            ui_number_update((struct ui_number *)elm, &unum);
        }
        break;
    case ON_CHANGE_SHOW:
        dc = (struct draw_context *)arg;
        dc->custom_color = BIT(UI_CUSTOM_COLOR_BIT_IMAGE);
        dc->custom_argb8888 = 0xff31ff88;


        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(BEDSIDE_WATCH_TIMER)
.onchange = bedside_watch_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BEDSIDE_WATCH_PIC)
.onchange = bedside_watch_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BEDSIDE_WATCH_BAT_NUM)
.onchange = bedside_watch_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static void bedside_watch_update(void *p)
{
    struct element *elm = (struct element *)p;
    u8 dir  = gsensor_dir_get();
    bedside_watch_dir_set(dir);
    ui_core_redraw(elm);
}
static int bedside_watch_layout_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_INIT:
        if (!__this->timer_id) {
            __this->timer_id = sys_timer_add(elm, bedside_watch_update, 1000);
        }
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_SHOW_PROBE:

        bedside_watch_anim_start(elm);
        break;
    case ON_CHANGE_RELEASE:
        if (__this->timer_id) {
            sys_timer_del(__this->timer_id);
            __this->timer_id = 0;
        }
        bedside_watch_anim_stop();

        ui_auto_shut_down_enable();
        break;
    default:
        return false;
    }
    return false;
}

static int bedside_watch_layout_ontouch(void *_ctrl, struct element_touch_event *e)
{
    //idle模式下不允许滑动
    if (app_in_mode(APP_MODE_IDLE)) {
        return true;
    }
    struct element *elm = (struct element *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
#if BEDSIDE_WATCH_TEST_EN
        int dir = bedside_watch_dir_get();
        dir ++;
        dir %= 4;
        bedside_watch_dir_set(dir);
#endif//BEDSIDE_WATCH_TEST_EN
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(BEDSIDE_WATCH_LAYOUT)
.onchange = bedside_watch_layout_onchange,
 .onkey = NULL,
  .ontouch = bedside_watch_layout_ontouch,
};

#endif// TCFG_UI_DRAW_DEMO
#endif// CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE






