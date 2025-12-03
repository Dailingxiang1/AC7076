#include "app_config.h"
#include "ui/ui_api.h"
#include "screen_trans/smartbox_user_app.h"
#include "events_adapter.h"
#include "adv_anc_voice.h"
#include "smartbox_info_manager.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-ACTION]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if (defined TCFG_UI_DENOISE_ENABLE) && TCFG_UI_DENOISE_ENABLE

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_cs_denoise.data.bss")
#pragma data_seg(".ui_cs_denoise.data")
#pragma const_seg(".ui_cs_denoise.text.const")
#pragma code_seg(".ui_cs_denoise.text")
#endif

#define STYLE_NAME  JL


static void ui_set_denoise_pic_status(struct ui_pic *pic, u8 anc_mode, u8 is_earphone_connet)
{
    if (!pic) {
        return;
    }

    if (is_earphone_connet == 0) {
        ui_pic_set_image_index(pic, 0);
        return;
    }

    ui_pic_set_image_index(pic, 1);

    log_info("%s anc_mode:%d", __func__, anc_mode);
    switch (pic->elm.id) {
    case DENOISE_OFF_PIC:
        if (anc_mode == ANC_MODE_OFF) {
            ui_pic_set_image_index(pic, 2);
        }
        break;
    case DENOISE_ON_PIC:
        if (anc_mode == ANC_MODE_ON) {
            ui_pic_set_image_index(pic, 2);
        }
        break;
    case DENOISE_TRANSPARENCY_PIC:
        if (anc_mode == ANC_TRANSPARENCY_MODE) {
            ui_pic_set_image_index(pic, 2);
        }
        break;
    case DENOISE_ADAPTIVE_PIC:
        if (anc_mode == ANC_ADAPTIVE_MODE) {
            ui_pic_set_image_index(pic, 2);
        }
        break;
    default:
        break;
    }
}

static void ui_update_denoise_status(u32 layout_id, u8 is_redraw)
{
    struct element *p, *layout_elm;
    layout_elm = ui_core_get_element_by_id(layout_id);
    if (!layout_elm) {
        return;
    }

    list_for_each_child_element(p, layout_elm) {
        struct ui_pic *pic = (struct ui_pic *)p;
        if (!strcmp(pic->source, "denoise")) {
            ui_set_denoise_pic_status(pic, sbox_anc_mode_get(), sbox_ble_connect_flag_get());
        }
    }

    if (is_redraw) {
        ui_redraw(layout_id);
    }
}

static void send_anc_mode_change(u8 mode)
{
    sbox_anc_mode_set(mode);  //即时响应
    anc_ui_set_mode_rcsp_vm(mode);
    custom_client_send_anc_mode(mode);
}


static int denoise_status_update(const char *type, uint32_t argc)
{
    ui_update_denoise_status(DENOISE_LAYOUT, 1);
    return 0;
}

static const struct uimsg_handl ui_msg_handler[] = {
    { "denoise", denoise_status_update },
    { NULL, NULL } // 必须以此结尾
};

static int denoise_pic_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        ui_set_denoise_pic_status(pic, sbox_anc_mode_get(), sbox_ble_connect_flag_get());
        ui_register_msg_handler(ID_WINDOW_EARPHONE_DISNOISE, ui_msg_handler);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

static int denoise_pic_ontouch(void *_ctrl, struct element_touch_event *e)
{

    struct ui_pic *pic = (struct ui_pic *)_ctrl;
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

        switch (pic->elm.id) {
        case DENOISE_OFF_PIC:
            send_anc_mode_change(ANC_MODE_OFF);
            break;
        case DENOISE_ON_PIC:
            send_anc_mode_change(ANC_MODE_ON);
            break;
        case DENOISE_TRANSPARENCY_PIC:
            send_anc_mode_change(ANC_TRANSPARENCY_MODE);
            break;
        case DENOISE_ADAPTIVE_PIC:
            send_anc_mode_change(ANC_ADAPTIVE_MODE);
            break;
        };
        ui_update_denoise_status(DENOISE_LAYOUT, 1);
        return true;
    default :
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(DENOISE_OFF_PIC)
.onchange = denoise_pic_onchange,
 .onkey = NULL,
  .ontouch = denoise_pic_ontouch,
};

REGISTER_UI_EVENT_HANDLER(DENOISE_ON_PIC)
.onchange = denoise_pic_onchange,
 .onkey = NULL,
  .ontouch = denoise_pic_ontouch,
};


REGISTER_UI_EVENT_HANDLER(DENOISE_TRANSPARENCY_PIC)
.onchange = denoise_pic_onchange,
 .onkey = NULL,
  .ontouch = denoise_pic_ontouch,
};

REGISTER_UI_EVENT_HANDLER(DENOISE_ADAPTIVE_PIC)
.onchange = denoise_pic_onchange,
 .onkey = NULL,
  .ontouch = denoise_pic_ontouch,
};


void ui_denoise_window_bt_status_update(void)
{
    ui_update_denoise_status(DENOISE_LAYOUT, 0);
}

#endif
#endif
