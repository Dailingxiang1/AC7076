#include "app_config.h"
#include "ui/ui_api.h"
#include "ui_sys_param.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_CS_LANGUAGE_SELECT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_cs_language_select.data.bss")
#pragma data_seg(".ui_cs_language_select.data")
#pragma const_seg(".ui_cs_language_select.text.const")
#pragma code_seg(".ui_cs_language_select.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if (defined TCFG_UI_LANGUAGE_SEL_ENABLE) && TCFG_UI_LANGUAGE_SEL_ENABLE

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)


static void next_language_type(void)
{
    u8 set_language_index;
    set_language_index = csc_get_ui_language_type() + 1;

    if (set_language_index >= LANGUAGE_NUM) {
        set_language_index = LANGUAGE_NUM - 1;
    }
    csc_set_ui_language_type(set_language_index);
}

static void prev_language_type()
{
    u8 set_language_index;
    set_language_index = csc_get_ui_language_type();

    if (set_language_index <= 0) {
        set_language_index = 0;
    } else {
        set_language_index -= 1;
    }

    csc_set_ui_language_type(set_language_index);
}


static int setting_language_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_LANGUAGE_LEFT_PIC:
            prev_language_type();
            break;
        case SETTING_LANGUAGE_RIGHT_PIC:
            next_language_type();
            break;
        default:
            break;
        }
        ui_redraw(LANGUAGE_TEXT);
        ui_redraw(LANGUAGE_TITLE_TEXT);
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_LANGUAGE_LEFT_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_language_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SETTING_LANGUAGE_RIGHT_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_language_ontouch,
};


#endif /*#if (defined TCFG_UI_LANGUAGE_SEL_ENABLE) && TCFG_UI_LANGUAGE_SEL_ENABLE*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))*/
