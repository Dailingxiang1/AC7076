#include "app_config.h"
#include "jlui_app/ui_style.h"
#include "jlui/ui.h"
#include "ui/ui_api.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "res/resfile.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "system/includes.h"
#include "audio_config.h"
#include "asm/mcpwm.h"
#include "jlui_app/ui_sys_param.h"
#include "jlui_app/watch_syscfg_manage.h"
#include "font/language_list.h"
#include "bt_common.h"
#include "btstack/btstack_task.h"
#include "btstack/avctp_user.h"
#include "custom_cfg.h"
#include "app_main.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_VOICE_ASSISTANT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_voice_assistant.data.bss")
#pragma data_seg(".ui_action_voice_assistant.data")
#pragma const_seg(".ui_action_voice_assistant.text.const")
#pragma code_seg(".ui_action_voice_assistant.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_VOICE_ASSISTANT

#define STYLE_NAME JL

struct siri_t {
    u8 is_siri_open;
    u32 timer_id;
    u32 siri_ui_timer;
    u32 rotate_time;
};

struct siri_t *siri = NULL;

static void get_siri_status(void *priv)
{
    extern APP_VAR app_var;
    bt_cmd_prepare(USER_CTRL_HFP_GET_SIRI_STATUS, 0, NULL);
    printf("app_var.siri_stu: %d\n", app_var.siri_stu);

    if (app_var.siri_stu == 0) {     // 手机断开siri
        siri->is_siri_open = 0;

        printf("get_siri_status  siri close!!\n");

        ui_text_show_index_by_id(SIRI_BUTTON, 0);

        bt_cmd_prepare(USER_CTRL_HFP_GET_SIRI_CLOSE, 0, NULL);

        siri->rotate_time = 0;

        if (siri && siri->timer_id) {
            sys_timer_del(siri->timer_id);
            siri->timer_id = 0;
        }
        if (siri && siri->siri_ui_timer) {
            sys_timer_del(siri->siri_ui_timer);
            siri->siri_ui_timer = 0;
        }
    }
}

static void siri_ui_rotate(void *priv)
{
    struct ui_pic *pic = NULL;

    pic = ui_pic_for_id(SIRI_PIC);
    ui_core_set_element_rotate(pic, 70, 70, 150, 170, 50 * siri->rotate_time, true);
    ui_core_redraw(pic);
    siri->rotate_time++;
}

static int siri_button_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_text *text = (struct ui_text *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (text->elm.id) {
        case SIRI_BUTTON:
            siri->is_siri_open = !siri->is_siri_open;
            if (siri->is_siri_open) {
                printf("siri open!!!\n");

                ui_text_show_index_by_id(SIRI_BUTTON, 1);

                bt_cmd_prepare(USER_CTRL_HFP_GET_SIRI_OPEN, 0, NULL);
                if (!siri->timer_id) {
                    siri->timer_id = sys_timer_add(NULL, get_siri_status, 1000);
                }
                if (!siri->siri_ui_timer) {
                    siri->siri_ui_timer = sys_timer_add(NULL, siri_ui_rotate, 100);
                }
            } else {
                printf("siri close!!!\n");

                ui_text_show_index_by_id(SIRI_BUTTON, 0);

                bt_cmd_prepare(USER_CTRL_HFP_GET_SIRI_CLOSE, 0, NULL);

                if (siri && siri->timer_id) {
                    sys_timer_del(siri->timer_id);
                    siri->timer_id = 0;
                }
                if (siri && siri->siri_ui_timer) {
                    sys_timer_del(siri->siri_ui_timer);
                    siri->siri_ui_timer = 0;
                }
            }
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SIRI_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = siri_button_ontouch,
};


static int siri_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***siri_onchange***\n");
        if (!siri) {
            siri = zalloc(sizeof(struct siri_t));
        }
        break;
    case ON_CHANGE_RELEASE:
        if (siri && siri->timer_id) {
            sys_timer_del(siri->timer_id);
            siri->timer_id = 0;
        }
        if (siri && siri->siri_ui_timer) {
            sys_timer_del(siri->siri_ui_timer);
            siri->siri_ui_timer = 0;
        }
        if (siri) {
            free(siri);
            siri = NULL;
        }
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_SIRI)
.onchange = siri_page_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif
#endif
