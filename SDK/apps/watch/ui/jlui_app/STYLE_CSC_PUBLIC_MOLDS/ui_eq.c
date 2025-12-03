#include "app_config.h"
#include "ui/ui_api.h"
#include "screen_trans/smartbox_user_app.h"
#include "events_adapter.h"
#include "smartbox_info_manager.h"
#ifndef CONFIG_MEDIA_NEW_ENABLE
#include "media/eq_config.h"
#else
#include "effects/audio_eq.h"
#endif

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-ACTION]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_eq.data.bss")
#pragma data_seg(".ui_eq.data")
#pragma const_seg(".ui_eq.text.const")
#pragma code_seg(".ui_eq.text")
#endif

#define STYLE_NAME  JL

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if (defined TCFG_UI_EQ_ENABLE) && TCFG_UI_EQ_ENABLE

typedef enum {
    ICON_STATUS_DISCONNECT = 0,
    ICON_STATUS_CONNECT,
    ICON_STATUS_PRESSING,
} ICON_STATUS;

#define     EQ_TYPE_MAX                     (EQ_MODE_CUSTOM)
#define     EQ_TYPE_MIN                     (EQ_MODE_NORMAL)

void eq_ui_set_rcsp_vm(u8 mode);

static int eq_sel_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 new_eq_mode = 0;
    u8 eq_mode = sbox_equalizer_mode_get();

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }

        if (!sbox_ble_connect_flag_get()) {
            return false;
        }

        struct element *elm = _ctrl;
        switch (elm->id) {
        case EQUALIZER_NEXT_PIC:
            new_eq_mode = eq_mode >= EQ_TYPE_MAX ? EQ_TYPE_MAX : eq_mode + 1;
            break;
        case EQUALIZER_PRE_PIC:
            new_eq_mode = eq_mode == EQ_TYPE_MIN ? EQ_TYPE_MIN : eq_mode - 1;
            break;
        }

        log_info("%s new_eq_mode:%d", __func__, new_eq_mode);

        custom_client_send_eq_mode(new_eq_mode);
        sbox_equalizer_mode_set(new_eq_mode);

        ui_text_show_index_by_id(EQUALIZER_DISPLAY_TEXT, new_eq_mode);
        eq_ui_set_rcsp_vm(new_eq_mode);
        return true;
    default :
        break;
    }
    return false;
}


static int eq_status_update(const char *type, u32 arg)
{
    ui_text_show_index_by_id(EQUALIZER_DISPLAY_TEXT, sbox_equalizer_mode_get());
    return 0;
}

void ui_eq_window_bt_status_update(void)
{
    struct ui_pic *pic = NULL;

    pic = (struct ui_pic *)ui_core_get_element_by_id(EQUALIZER_PRE_PIC);
    if (pic) {
        ui_pic_set_image_index(pic, sbox_ble_connect_flag_get() ? ICON_STATUS_CONNECT : ICON_STATUS_DISCONNECT);
    }

    pic = (struct ui_pic *)ui_core_get_element_by_id(EQUALIZER_NEXT_PIC);
    if (pic) {
        ui_pic_set_image_index(pic, sbox_ble_connect_flag_get() ? ICON_STATUS_CONNECT : ICON_STATUS_DISCONNECT);
    }
}

static const struct uimsg_handl ui_msg_handler[] = {
    { "EQ_UPDATA",        eq_status_update     },
    { NULL, NULL},      /* 必须以此结尾！ */
};

static int eq_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        ui_register_msg_handler(ID_WINDOW_EQUALIZER, ui_msg_handler);

        switch (elm->id) {
        case EQUALIZER_DISPLAY_TEXT:
            struct ui_text *text = (struct ui_text *)elm;
            ui_text_set_index(text, sbox_equalizer_mode_get());
            break;
        case EQUALIZER_NEXT_PIC:
        case EQUALIZER_PRE_PIC:
            struct ui_pic *pic = (struct ui_pic *)elm;
            ui_pic_set_image_index(pic, (sbox_ble_connect_flag_get() == 1) ? ICON_STATUS_CONNECT : ICON_STATUS_DISCONNECT);
            break;
        }

        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(EQUALIZER_DISPLAY_TEXT)
.onchange = eq_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(EQUALIZER_NEXT_PIC)
.onchange = eq_onchange,
 .onkey = NULL,
  .ontouch = eq_sel_ontouch,
};

REGISTER_UI_EVENT_HANDLER(EQUALIZER_PRE_PIC)
.onchange = eq_onchange,
 .onkey = NULL,
  .ontouch = eq_sel_ontouch,
};

#endif//TCFG_UI_EQ_ENABLE
#endif//CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE
