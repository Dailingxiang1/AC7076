#include "app_config.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-SPORT_INTENSITY]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_sport_intensity.data.bss")
#pragma data_seg(".ui_action_sport_intensity.data")
#pragma const_seg(".ui_action_sport_intensity.text.const")
#pragma code_seg(".ui_action_sport_intensity.text")
#endif


#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_ENABLE_SPORT_INTENSITY

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)


static int sport_intersity_handler(const char *type, u32 arg)
{
    struct unumber num = {0};

    if (type && !strcmp(type, "num")) {
        num.type = TYPE_NUM;
        num.numbs = 1;
        num.number[0] = arg;
        ui_number_update_by_id(SPORT_INTENSITY_NUM, &num);
    }

    return 0;
}


static const struct uimsg_handl ui_msg_handler[] = {
    { "sport_intensity",        sport_intersity_handler     },
    { NULL, NULL},      /* 必须以此结尾！ */
};


static int sport_intensity_num_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct unumber num = {0};
    /* printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__); */

    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        num.type = TYPE_NUM;
        num.numbs = 1;
        num.number[0] = 6;
        ui_number_update((struct ui_number *)ctr, &num);
        break;

    default:
        return FALSE;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(SPORT_INTENSITY_NUM)
.onchange = sport_intensity_num_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};


static int sport_intensity_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
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


REGISTER_UI_EVENT_HANDLER(ID_WINDOW_SPORT_INTENSITY)
.onchange = sport_intensity_page_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};


#endif/*#if TCFG_UI_ENABLE_SPORT_INTENSITY*/
#endif/*#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE*/



