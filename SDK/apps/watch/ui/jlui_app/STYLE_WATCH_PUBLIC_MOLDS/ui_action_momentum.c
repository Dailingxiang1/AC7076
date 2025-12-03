#include "app_config.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "health_manager/health_manager.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-MOMENTUM]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_momentum.data.bss")
#pragma data_seg(".ui_action_momentum.data")
#pragma const_seg(".ui_action_momentum.text.const")
#pragma code_seg(".ui_action_momentum.text")
#endif


#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_ENABLE_MOMENTUM

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)
static u16 timer_id  = 0;

static int momentum_handler(const char *type, u32 arg)
{
    char str[8] = {0};
    struct unumber num = {0};

    if (!type) {
        return -1;
    }

    if (!strcmp(type, "step_per")) {
        ui_multiprogress_set_persent_by_id(MOMENTUM_ARC, arg);
    } else if (!strcmp(type, "cal_per")) {
        ui_multiprogress_set_second_persent_by_id(MOMENTUM_ARC, arg);
    } else if (!strcmp(type, "km_per")) {
        ui_multiprogress_set_third_persent_by_id(MOMENTUM_ARC, arg);
    } else if (!strcmp(type, "step_num")) {
        sprintf(str, "%d", arg);
        num.type = TYPE_STRING;
        num.num_str = (u8 *)str;
        ui_number_update_by_id(MOMENTUM_STEP, &num);
    } else if (!strcmp(type, "cal_num")) {
        sprintf(str, "%d", arg);
        num.type = TYPE_STRING;
        num.num_str = (u8 *)str;
        ui_number_update_by_id(MOMENTUM_CAL, &num);
    } else if (!strcmp(type, "km_num")) {
        sprintf(str, "%d", arg);
        num.type = TYPE_STRING;
        num.num_str = (u8 *)str;
        ui_number_update_by_id(MOMENTUM_KM, &num);
    }

    return 0;
}


static const struct uimsg_handl ui_msg_handler[] = {
    { "momentum",        momentum_handler     }, //
    { NULL, NULL},      /* 必须以此结尾！ */
};

static void ui_momentum_cb(void *p)
{

    char str[8] = {0};
    int distance = sport_health_get_value_daily_distance();
    int steps = sport_health_get_value_daily_steps();
    int kcal = sport_health_get_value_daily_calories();
    int target_distance  = sport_health_get_value_daily_target_distance();
    int target_steps  = sport_health_get_value_daily_target_steps();
    int target_kcal  = sport_health_get_value_daily_target_calories();
    int percent_distance = (target_distance) ? (distance * 100 / target_distance) : 0;
    int percent_steps = (target_steps) ? steps * 100 / target_steps : 0;
    int percent_kcal  = (target_kcal) ? kcal * 100 / target_kcal : 0;
    percent_distance = percent_distance > 100 ? 100 : percent_distance;
    percent_steps = percent_steps > 100 ? 100 : percent_steps;
    percent_kcal = percent_kcal > 100 ? 100 : percent_kcal;

    struct unumber num = {0};
    num.type = TYPE_STRING;
    num.num_str = (u8 *)str;
    sprintf(str, "%d", steps);
    ui_number_update((struct ui_number *)ui_core_get_element_by_id(MOMENTUM_STEP), &num);
    sprintf(str, "%d", kcal);
    ui_number_update((struct ui_number *)ui_core_get_element_by_id(MOMENTUM_CAL), &num);
    sprintf(str, "%d", distance);
    ui_number_update((struct ui_number *)ui_core_get_element_by_id(MOMENTUM_KM), &num);

    ui_multiprogress_set_persent((struct ui_multiprogress *)ui_core_get_element_by_id(MOMENTUM_ARC_BACKUP), 100);
    ui_multiprogress_set_second_persent((struct ui_multiprogress *)ui_core_get_element_by_id(MOMENTUM_ARC_BACKUP), 100);
    ui_multiprogress_set_third_persent((struct ui_multiprogress *)ui_core_get_element_by_id(MOMENTUM_ARC_BACKUP), 100);

    ui_multiprogress_set_persent((struct ui_multiprogress *)ui_core_get_element_by_id(MOMENTUM_ARC), percent_distance);
    ui_multiprogress_set_second_persent((struct ui_multiprogress *)ui_core_get_element_by_id(MOMENTUM_ARC), percent_kcal);
    ui_multiprogress_set_third_persent((struct ui_multiprogress *)ui_core_get_element_by_id(MOMENTUM_ARC), percent_steps);

    ui_core_redraw((struct element *)p);
}
static int momentum_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{

    switch (e) {
    case ON_CHANGE_INIT:
        if (!timer_id) {
            timer_id =  sys_timer_add(ctr, ui_momentum_cb, 1000);
        }
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_momentum_cb(ctr);
        break;
    case ON_CHANGE_RELEASE:
        if (timer_id) {
            sys_timer_del(timer_id);
            timer_id = 0;
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(MOMENTUM_LAYOUT)
.onchange = momentum_layout_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};


static int momentum_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_register_msg_handler(window->elm.id, ui_msg_handler);
        break;

    default:
        return FALSE;
    }
    return FALSE;
}


/* REGISTER_UI_EVENT_HANDLER(ID_WINDOW_MOMENTUM) */
/* .onchange = momentum_page_onchange, */
/* .onkey    = NULL, */
/* .ontouch  = NULL, */
/* }; */


#endif/*#if ((defined TCFG_UI_ENABLE)&&(TCFG_UI_ENABLE))*/
#endif/*#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE*/



