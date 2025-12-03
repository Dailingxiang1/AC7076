#include "app_config.h"
#include "ui/ui_api.h"
#include "system/timer.h"
#include "events_adapter.h"
#include "rtc/rtc.h"
#include "app_mode_manager/app_mode_manager.h"
#include "smartbox_user_app.h"
#include "smartbox_info_manager.h"
#include "app_task.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_CS_CHARGING]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_cs_charging.data.bss")
#pragma data_seg(".ui_cs_charging.data")
#pragma const_seg(".ui_cs_charging.text.const")
#pragma code_seg(".ui_cs_charging.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if (defined TCFG_UI_ENABLE_BATCHARGE) && TCFG_UI_ENABLE_BATCHARGE

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)

static int progressbar_reflash_id = 0;
static int bat_progressbar_percent = 0;

static void batcharge_progressbar_reflash(void *p)
{
    struct element *elm;
    elm = ui_core_get_element_by_id(CHARGE_PROG);
    ui_progress_set_persent((struct ui_progress *)elm, bat_progressbar_percent);

    u8 box_bat = 0;
#if TCFG_EARPHONE_PROTOCOL
    box_bat = sbox_battery_box_get();
#endif
    struct unumber unum;
    unum.type = TYPE_NUM;
    unum.numbs = 1;
    unum.number[0] = box_bat;
    elm = ui_core_get_element_by_id(BATCHARGE_PERSENT_VALUE);
    ui_number_update((struct ui_number *)elm, &unum);

    bat_progressbar_percent += 5;
    if (box_bat == 100) {
        bat_progressbar_percent = 100;
    } else {
        if (bat_progressbar_percent >= 100) {
            bat_progressbar_percent = 0;
        }
    }

    elm = ui_core_get_element_by_id(BATCHARGE_LAYOUT);
    ui_core_redraw(elm);
}

static int batcharge_progressbar_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_progress *progress = (struct ui_progress *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        u8 box_bat = 0;
#if TCFG_EARPHONE_PROTOCOL
        box_bat = sbox_battery_box_get();
#endif
        log_info("%s box_bat:%d", __func__, box_bat);
        if (box_bat == 100) {
            bat_progressbar_percent = box_bat;
            ui_progress_set_persent(progress, bat_progressbar_percent);
        }
        if (!progressbar_reflash_id) {
            progressbar_reflash_id = sys_timer_add(NULL, batcharge_progressbar_reflash, 100);
        }
        break;

    case ON_CHANGE_RELEASE:
        if (progressbar_reflash_id) {
            sys_timer_del(progressbar_reflash_id);
            progressbar_reflash_id = 0;
        }
        break;

    default:
        return false;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(CHARGE_PROG)
.onchange = batcharge_progressbar_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};

static int batcharge_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        //息屏插入
        ui_auto_shut_down_enable();
        //更新定时器
        ui_auto_shut_down_re_run();
        break;
    default:
        return false;
    }
    return false;
}

static int batcharge_ontouch(void *_ctrl, struct element_touch_event *e)
{
    if (app_in_mode(APP_MODE_IDLE)) {
        return true;
    } else {
        if (e->event ==  ELM_EVENT_TOUCH_R_MOVE) {
            if (! ui_return_prev_page_id()) {
                UI_SHOW_WINDOW(ID_WINDOW_DEFAULT);
                return true;
            }
        }
        return false;
    }
}

REGISTER_UI_EVENT_HANDLER(BATCHARGE_LAYOUT)
.onchange = batcharge_onchange,
 .onkey    = NULL,
  .ontouch  = batcharge_ontouch,
};

#endif //#if (defined TCFG_UI_ENABLE_BATCHARGE) && TCFG_UI_ENABLE_BATCHARGE
#endif //#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
