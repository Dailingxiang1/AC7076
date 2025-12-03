/**
 * @file ui_phone_status.c
 * \Brief : 电话状态页面显示: 来电 去电 通话中
 * @date 2024-05-14
 */
#include "system/includes.h"
#include "app_task.h"
#include "system/timer.h"
#include "app_common.h"
#include "app_main.h"
#include "btstack/avctp_user.h"
#include "audio_config.h"
#include "audio_cvp.h"
#include "ui.h"
#include "ui_api.h"
#include "jlui_app/ui_style.h"
#include "data_storage.h"
#include "events_adapter.h"
#include "smartbox_info_manager.h"


#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-CALL-STATUS]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_phone_call_status.data.bss")
#pragma data_seg(".ui_action_phone_call_status.data")
#pragma const_seg(".ui_action_phone_call_status.text.const")
#pragma code_seg(".ui_action_phone_call_status.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if (defined TCFG_UI_PHONE_CALL_ENABLE) && TCFG_UI_PHONE_CALL_ENABLE

#define STYLE_NAME  JL

typedef struct layout_map {
    int id;
    int state;
} layout_map_t;

const layout_map_t layout_map[] = {
    {PHONE_CALL_STATUS_INCOMING_LAYOUT, CALL_STATUS_INCOME},
    {PHONE_CALL_STATUS_ACTIVE_LAYOUT, CALL_STATUS_ACTIVE},
    {PHONE_CALL_STATUS_OUTGOING_LAYOUT, CALL_STATUS_OUTGOING},
};

static int ui_phone_call_state_get_layoutt_id(int state)
{
    int i = 0;
    for (i = 0; i < sizeof(layout_map) / sizeof(layout_map[0]); i++) {
        if (layout_map[i].state == state) {
            return layout_map[i].id;
        }
    }
    return 0;
}


/***********************************************************************************
 *
 * UI刷新逻辑操作接口
 *
 **********************************************************************************/

// 页面切换更新函数
static int ui_phone_call_state_layout_update(int state, int redraw)
{
    struct layout *layout = (struct layout *)ui_core_get_element_by_id(PHONE_CALL_STATE_LAYOUT);
    struct element  *p, *n;                //遍历图标使用
    if (!layout) {
        log_error("layout is null");
        return -1;
    }
    if (state == CALL_STATUS_HANGUP) {
        log_error("state:%d \n", state);
        return -2;
    }
    log_info("%s state:%d redraw:%d\n", __func__, state, redraw);
    list_for_each_child_element_safe(p, n, &layout->elm) {
        if (p->id == ui_phone_call_state_get_layoutt_id(state)) {
            p->css.invisible = 0;
        } else {
            p->css.invisible = 1;
        }
        log_info("%s id:%x invisible:%d\n", __func__, p->id, p->css.invisible);
    }
    if (redraw) {
        ui_core_redraw(layout);
    }
    return 0;
}


// 子控件刷新逻辑操作接口
static int pic_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }

        if (!strcmp(pic->source, "hangup")) {
            custom_client_send_call(PHONE_CTRL_HANGUP);
            sbox_phone_call_state_set(CALL_STATUS_HANGUP);
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_PHONE_CALL_STATUS);
        } else if (!strcmp(pic->source, "answer")) {
            custom_client_send_call(PHONE_CTRL_ANSWER);
            sbox_phone_call_state_set(CALL_STATUS_ACTIVE);
            ui_phone_call_state_layout_update(CALL_STATUS_ACTIVE, 1);
        } else if (!strcmp(pic->source, "mute")) {
            if (sbox_phone_call_mute_get() == PHONE_CTRL_MUTE_OFF) {
                custom_client_send_call(PHONE_CTRL_MUTE_ON);
                sbox_phone_call_mute_set(PHONE_CTRL_MUTE_ON);
                ui_pic_show_image_by_id(pic->elm.id, 1);
            } else {
                custom_client_send_call(PHONE_CTRL_MUTE_OFF);
                sbox_phone_call_mute_set(PHONE_CTRL_MUTE_OFF);
                ui_pic_show_image_by_id(pic->elm.id, 0);
            }
        }
        return true;
    default:
        break;
    }
    return false;
}

static int ui_phone_state_child_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)_ctrl;
    int ret;
    int type = ui_id2type(elm->id);

    log_info("func:%s , type:%d event:%d \n", __func__, type, e->event);
    switch (type) {
    case CTRL_TYPE_PIC:
        ret = pic_ontouch(elm, e);
        return ret;
    default:
        break;
    }

    return false;
}

static int ui_phone_state_pic_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        if (!strcmp(pic->source, "mute")) {
            if (sbox_phone_call_mute_get() == PHONE_CTRL_MUTE_OFF) {
                ui_pic_set_image_index(pic, 0);
            } else {
                ui_pic_set_image_index(pic, 1);
            }
        }
        break;
    default:
        break;
    }
    return false;
}

static int ui_phone_state_chile_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    int ret;
    int type = ui_id2type(elm->id);
    switch (type) {
    case CTRL_TYPE_PIC:
        ret = ui_phone_state_pic_onchange(_ctrl, event, arg);
        return ret;
    default:
        break;
    }
    return false;
}


//  外部通过UI_MSG_POST 函数触发ui刷新逻辑
static int phone_state_handler(const char *type, u32 arg)
{
    if (!type) {
        return -1;
    }
    log_info("func:%s , type:%s , arg:%d\n", __func__, type, arg);
    if (!strcmp(type, "state")) {

        /*点击接听，ui先刷新。等到通话事件过来时，立即刷新时间*/
        struct utime t = {0};
        t.sec = 1;/*为了和手机显示的通话时间对应上，给初始化值*/
        ui_time_update_by_id(PHONE_9, &t);

        ui_phone_call_state_layout_update(arg, 1);
    }
    return 0;
}

static const struct uimsg_handl ui_msg_handler[] = {
    { "phone",        phone_state_handler     }, //
    { NULL, NULL},      /* 必须以此结尾！ */
};

static int phone_call_state_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        ui_auto_shut_down_disable();
        sbox_phone_call_mute_set(PHONE_CTRL_MUTE_OFF);
        ui_register_msg_handler(ui_get_current_window_id(), ui_msg_handler);
        ui_phone_call_state_layout_update(sbox_phone_call_state_get(), 0);
        ui_set_default_handler(&layout->elm, ui_phone_state_child_ontouch, NULL, NULL);
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(PHONE_CALL_STATE_LAYOUT)
.onchange = phone_call_state_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif /* if TCFG_UI_ENABLE_PHONE_ACTION */
#endif
