#include "app_config.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"

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

int ui_show_main(int id);
int ui_hide_curr_main();
void sporting_target_index_set(u8 index);

static struct outdoor_sports_type {
    s8 index;
    struct element *target;
} *__this = NULL;


static int sporting_target_init(int p)
{

    return 0;
}


static int sporting_target_ontouch(void *ctr, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctr;
    struct rect item_rect, child_rect;
    struct element *item_elm, *child;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        __this->target = elm;
        break;

    case ELM_EVENT_TOUCH_UP:
        if (__this->target && __this->target == elm) {
            __this->target = NULL;
            ui_hide_curr_main();
            if (ui_core_get_element_by_id(SPORTING_TARGET_KM) == elm) {
                sporting_target_index_set(0);
                ui_show_main(ID_WINDOW_SPORT_TARGET_KM_SET);
                UI_WINDOW_BACK_DEL(ID_WINDOW_SPORT_TARGET);
            } else if (ui_core_get_element_by_id(SPORTING_TARGET_TIME) == elm) {
                sporting_target_index_set(1);
                ui_show_main(ID_WINDOW_SPORT_TARGET_TIME_SET);
                UI_WINDOW_BACK_DEL(ID_WINDOW_SPORT_TARGET);
            } else if (ui_core_get_element_by_id(SPORTING_TARGET_CAL) == elm) {
                sporting_target_index_set(2);
                ui_show_main(ID_WINDOW_SPORT_TARGET_CAL_SET);
                UI_WINDOW_BACK_DEL(ID_WINDOW_SPORT_TARGET);
            }
        }
        break;

    case ELM_EVENT_TOUCH_MOVE:
        __this->target = NULL;
        break;

    default:
        break;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(SPORTING_TARGET_KM)
.onchange = NULL,
 .onkey    = NULL,
  .ontouch  = sporting_target_ontouch,
};


REGISTER_UI_EVENT_HANDLER(SPORTING_TARGET_TIME)
.onchange = NULL,
 .onkey    = NULL,
  .ontouch  = sporting_target_ontouch,
};


REGISTER_UI_EVENT_HANDLER(SPORTING_TARGET_CAL)
.onchange = NULL,
 .onkey    = NULL,
  .ontouch  = sporting_target_ontouch,
};




static int sporting_target_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        if (!__this) {
            __this = zalloc(sizeof(struct outdoor_sports_type));
        }
        break;

    case ON_CHANGE_FIRST_SHOW:
        //sporting_target_init(outdoor_sports_init, 0);
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


static u32 g_record = 0;
void sporting_target_record(u32 priv)
{
    g_record = priv;
}


static int sporting_target_page_ontouch(void *elm, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide_curr_main();
        ui_show_main(g_record);
        UI_WINDOW_BACK_DEL(ID_WINDOW_SPORT_TARGET);
        return TRUE;


    default:
        break;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(ID_WINDOW_SPORT_TARGET)
.onchange = sporting_target_page_onchange,
 .onkey    = NULL,
  .ontouch  = sporting_target_page_ontouch,
};


#endif/*#if TCFG_UI_ENABLE_SPORTING*/
#endif/*#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE*/



