
#include "app_task.h"
#include "btstack/avctp_user.h"
#include "system/timer.h"
#include "ui_style.h"
#include "ui.h"
#include "ui_api.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-ACTION]"
#define LOG_ERROR_ENABLE
/* #define LOG_DEBUG_ENABLE */
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_photogragh.data.bss")
#pragma data_seg(".ui_action_photogragh.data")
#pragma const_seg(".ui_action_photogragh.text.const")
#pragma code_seg(".ui_action_photogragh.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_PHOTOGRAGH

#define STYLE_NAME  JL



/************************************************
 * 1.初始化要显示的布局 2.添加该页面消息处理
 ***********************************************/

int ui_photogragh_bt_status_handler(const char *type, u32 arg)
{
    log_debug("<%s> type:%s arg:%d", __func__, type, arg);
    if (type && (!strcmp(type, "event"))) {
        switch (arg) {
        case BT_STATUS_SECOND_CONNECTED:
        case BT_STATUS_FIRST_CONNECTED:
        case BT_STATUS_CONN_A2DP_CH:
            ui_hide(PHOTOGRAGH_NOT_CONNECT_LAYOUT);
            ui_show(PHOTOGRAGH_CONNECT_LAYOUT);
            break;
        case BT_STATUS_FIRST_DISCONNECT:
        case BT_STATUS_SECOND_DISCONNECT:
            ui_hide(PHOTOGRAGH_CONNECT_LAYOUT);
            ui_show(PHOTOGRAGH_NOT_CONNECT_LAYOUT);
            break;
        }
    }
    return 0;
}
static const struct uimsg_handl ui_photogragh_msg_handler[] = {
    { "bt_status",      ui_photogragh_bt_status_handler     }, /* 蓝牙状态 */
    { NULL, NULL},      /* 必须以此结尾！ */
};

static int ui_photogragh_connet_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        log_debug("<%s> get_bt_connect_status():%d", __func__, bt_get_connect_status());
        ui_register_msg_handler(ID_WINDOW_PHOTOGRAGH, ui_photogragh_msg_handler);//注册消息交互的回调
        if (bt_get_connect_status() !=  BT_STATUS_WAITINT_CONN) {
            layout->elm.css.invisible = 0;
        } else {
            layout->elm.css.invisible = 1;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHOTOGRAGH_CONNECT_LAYOUT)
.onchange = ui_photogragh_connet_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int ui_photogragh_not_connet_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        if (bt_get_connect_status() !=  BT_STATUS_WAITINT_CONN) {
            layout->elm.css.invisible = 1;
        } else {
            layout->elm.css.invisible = 0;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHOTOGRAGH_NOT_CONNECT_LAYOUT)
.onchange = ui_photogragh_not_connet_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************************
 * 点击拍照处理
 **********************************************************/
static int ui_photogragh_click_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;    /*PIC类型控件，需要拦截down事件，才会有up*/
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            log_info("<%s> bt_cmd_prepare:USER_CTRL_HID_BOTH", __func__);
            bt_cmd_prepare(USER_CTRL_HID_BOTH, 0, NULL);

        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHOTOGRAGH_CLICK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_photogragh_click_pic_ontouch,
};



/************************************************
 * 接受btstatck的事件进行处理
 ***********************************************/
static int jlui_photogragph_btstack_event_handler(int *_event)
{
    struct bt_event *event = (struct bt_event *)_event;

    switch (event->event) {
    case BT_STATUS_FIRST_CONNECTED:
    case BT_STATUS_SECOND_CONNECTED:
    case BT_STATUS_CONN_A2DP_CH:
        if (ui_get_current_window_id() == ID_WINDOW_PHOTOGRAGH) {
            UI_MSG_POST("bt_status:event=%4", event->event);
        }
        break;
    case BT_STATUS_FIRST_DISCONNECT:
    case BT_STATUS_SECOND_DISCONNECT:
        if (ui_get_current_window_id() == ID_WINDOW_PHOTOGRAGH) {
            UI_MSG_POST("bt_status:event=%4", event->event);
        }
        break;
    }
    return 0;
}

APP_MSG_HANDLER(jlui_photogragph_stack_msg_entry) = {
    .owner      = 0xff,
    .from       = MSG_FROM_BT_STACK,
    .handler    = jlui_photogragph_btstack_event_handler,
};



#endif /* if TCFG_UI_ENABLE_PHOTOGRAGH */
#endif /* #if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) */

