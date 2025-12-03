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
#define LOG_TAG     		"[UI-SPORTING]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_sporting.data.bss")
#pragma data_seg(".ui_action_sporting.data")
#pragma const_seg(".ui_action_sporting.text.const")
#pragma code_seg(".ui_action_sporting.text")
#endif


#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_ENABLE_SPORTING

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)


#define abs(x)  ((x)>0?(x):-(x))

static struct sport_result_type {
    int timer;
} *__this = NULL;


static int sport_result_init(int p)
{
    struct rect item_child_rect;
    struct element *elm;

    elm = ui_core_get_element_by_id(SPORTING_RESULT_LIST);
    ui_grid_energy_auto_center((struct ui_grid *)elm, 1);
    ui_grid_set_energy_target_line((struct ui_grid *)elm, 122);

    struct sport_value sp_value = {0};
    sp_value.file_index = 0;
    sport_health_get_sport_file_value(&sp_value);
    struct utime time = {0};
    /* time.hour = 24; */
    /* time.min  = 12; */
    /* time.sec  = 6; */
    time.hour = sp_value.run_sec / 3600;
    time.min  = (sp_value.run_sec % 3600) / 60;
    time.sec  = (sp_value.run_sec % 3600) % 60;
    ui_time_update_by_id(SPORTING_RES_TIME, &time);


    struct unumber num;
    num.type = TYPE_NUM;
    num.numbs = 2;
    num.number[0] = sp_value.heart_max;
    num.number[1] = sp_value.heart_min;
    ui_number_update_by_id(SPORTING_RES_AHEART, &num);

    num.number[0] = sp_value.distance_c / 1000;
    num.number[1] = sp_value.distance_c % 1000;
    ui_number_update_by_id(SPORTING_RES_KM, &num);
    if (sp_value.distance_c) {
        int pace_s = 1000 * sp_value.run_sec / sp_value.distance_c;
        num.number[0] = pace_s / 60;
        num.number[1]  = pace_s % 60;
    } else {
        num.number[0] = 0;
        num.number[1] = 0;
    }
    ui_number_update_by_id(SPORTING_RES_SPEED, &num);

    num.numbs = 1;
    num.number[0] = sp_value.calories_c;
    num.number[1] = 0;
    ui_number_update_by_id(SPORTING_RES_CAL, &num);
    num.number[0] = sp_value.heart_val;
    num.number[1] = 0;
    ui_number_update_by_id(SPORTING_RES_HEART, &num);

    printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
    return 0;
}


static int sport_result_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct element *elm;

    printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
    elm = ui_core_get_element_by_id(SPORTING_RESULT_LIST);
    switch (e) {
    case ON_CHANGE_INIT:
        break;

    case ON_CHANGE_FIRST_SHOW:
        ui_set_call(sport_result_init, 0);
        break;

    case ON_CHANGE_RELEASE:
        break;

    case ON_CHANGE_SHOW_COMPLETED:
        //ui_grid_slide(elm, SCROLL_DIRECTION_UD, -59);
        break;

    default:
        return FALSE;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(SPORTING_RESULT_LAYOUT)
.onchange = sport_result_layout_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};


static int sport_result_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
    switch (e) {
    case ON_CHANGE_INIT:
        if (!__this) {
            __this = zalloc(sizeof(struct sport_result_type));
        }
        break;

    case ON_CHANGE_FIRST_SHOW:
        ui_set_call(sport_result_init, 0);
        break;

    case ON_CHANGE_RELEASE:
        if (__this) {
            free(__this);
            __this = NULL;
        }
        break;

    case ON_CHANGE_SHOW_COMPLETED:
        break;

    default:
        return FALSE;
    }
    return FALSE;
}


static int sport_result_list_ontouch(void *elm, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide_curr_main();
        ui_show_main(ID_WINDOW_MENU_LIST);
        UI_WINDOW_BACK_DEL(ID_WINDOW_SPORT_RESULT);
        UI_WINDOW_BACK_DEL(ID_WINDOW_SPORTING);
        return TRUE;


    default:
        break;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(ID_WINDOW_SPORT_RESULT)
.onchange = sport_result_page_onchange,
 .onkey    = NULL,
  .ontouch  = sport_result_list_ontouch,
};

#endif/*#if TCFG_UI_ENABLE_SPORTING*/
#endif/*#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE*/



