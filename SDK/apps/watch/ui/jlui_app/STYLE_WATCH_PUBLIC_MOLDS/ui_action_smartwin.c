#include "app_config.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"

#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"
#include "health_manager/health_manager.h"
#include "app_mode_manager/app_mode_manager.h"
#include "btstack/avctp_user.h"
#include "ui_action_smartwin.h"



#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_SMARTWIN]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_smartwin.data.bss")
#pragma data_seg(".ui_action_smartwin.data")
#pragma const_seg(".ui_action_smartwin.text.const")
#pragma code_seg(".ui_action_smartwin.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_SMARTWIN

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)

/******************************************************************************
 * 说明：目前灵动岛仅实现：连接手机之后，打开音乐，弹出弹窗。或者有新消息弹出的时候的
 * 时候弹出消息通知的弹窗。未连接手机的时候，灵动岛不弹出（用户可自行修改逻辑）
 ******************************************************************************/
int smartwin_status_handler(const char *type, u32 arg);
static const struct uimsg_handl ui_msg_handler[] = {
    { "smartwin_status",        smartwin_status_handler     },
    { NULL, NULL},
};

struct smartwin_ctrl {
    SmartwinType cur_type;  /* 当前灵动岛显示的类型 */
    ui_anim_t anim;         /* 用于显示灵动岛的动态效果 */
    u16 smartwin_timer;     /* 定时器用来检查灵动岛显示状态 */

    /* 定义结构体用来存放各个功能的信息 */
    SmartWinChargeInfo charge_info;
    SmartWinMusicInfo  music_info;
    SmartWinMessageInfo message_info;
};

static struct smartwin_ctrl *__smartwin_ctrl = NULL;
#define __this 		__smartwin_ctrl

typedef struct {
    u32 layout_id;      /* 灵动岛每种类型对应的布局 */
} SmartWinUI;

/* 灵动岛功能设置(用于用户设置灵动岛开关)  */
u8 smartwin_if_enable(void)
{
    int ret = get_ui_sys_param(smartwin_en);
    if (ret < 0) {
        log_error("param get fail!!!\n");
        return 0;
    }
    return ret;
}
void smartwin_if_enable_set(bool enable)
{
    u8 en = (u8)enable;
    if (smartwin_if_enable() != en) {
        set_ui_sys_param(smartwin_en, en);
    }
}

SmartwinType smartwin_get_type(void)
{
    return __this->cur_type;
}

static void smartwin_parm_init(void)
{
    if (__this) {
        memset(__this, 0, sizeof(struct smartwin_ctrl));
    }
}

/* 获取灵动岛音乐状态(0:停止播放；1：播放) */
u8 smartwin_get_music_state(void)
{
    u8 ret = 0;
    if (__this->music_info.music_status == SMARTWIN_MUSIC_STATUS_PLAY) {
        ret = 1;
    } else {
        ret = 0;
    }
    return ret;
}

void smartwin_set_type(SmartwinType new_type)
{
    __this->cur_type = new_type;
}

static void smartwin_show(u8 redraw)
{
    struct element *elm_charge, *elm_music, *elm_notice, *elm_null;

    elm_charge = ui_core_get_element_by_id(SMARTWIN_CHARGE);
    elm_music = ui_core_get_element_by_id(SMARTWIN_MUSIC);
    elm_notice = ui_core_get_element_by_id(SMARTWIN_NOTICE);
    elm_null = ui_core_get_element_by_id(SMARTWIN_NULL);
    if (!smartwin_if_enable()) {
        elm_charge->css.invisible = 1;
        elm_music->css.invisible = 1;
        elm_notice->css.invisible = 1;
        elm_null->css.invisible = 1;
        return;
    }


    switch (__this->cur_type) {
    case SMARTWIN_TYPE_CHARGE:
        elm_charge->css.invisible = 0;
        elm_music->css.invisible = 1;
        elm_notice->css.invisible = 1;
        elm_null->css.invisible = 1;
        break;

    case SMARTWIN_TYPE_MUSIC:
        elm_charge->css.invisible = 1;
        elm_music->css.invisible = 0;
        elm_notice->css.invisible = 1;
        elm_null->css.invisible = 1;
        break;

    case SMARTWIN_TYPE_NOTICE:
        elm_charge->css.invisible = 1;
        elm_music->css.invisible = 1;
        elm_notice->css.invisible = 0;
        elm_null->css.invisible = 1;
        break;

    case SMARTWIN_TYPE_NULL:
        elm_charge->css.invisible = 1;
        elm_music->css.invisible = 1;
        elm_notice->css.invisible = 1;
        elm_null->css.invisible = 0;
        break;

    default:
        break;
    }
    if (redraw) {
        ui_core_redraw(ui_core_get_element_by_id(SMARTWIN));
    }
}

/* 设置灵动岛充电状态 */
static void smartwin_charge_con_set(SwChargeStatus charge_con)
{
    __this->charge_info.charge_status = charge_con;
}

/* 获取系统音乐状态(0:停止,1:播放,2:暂停) */
u8 smartwin_music_status_check(void)
{
    struct app_mode *cur_mode;
    u8 ret = 0;

    cur_mode = app_get_current_mode();
#if TCFG_APP_BT_EN
    if (cur_mode->name == APP_MODE_BT) {
        u8 a2dp_state = bt_a2dp_get_status();
        // printf("a2dp_state :%d \n", a2dp_state);
        if (a2dp_state == BT_MUSIC_STATUS_STARTING) {
            ret = 1;
        } else if (a2dp_state == BT_MUSIC_STATUS_SUSPENDING) {
            ret = 2;
        } else {
            ret = 0;
        }
    }
#endif
    return ret;
}

static void smartwin_music_status_set(SwMusicStatus status)
{
    __this->music_info.music_status = status;
    switch (status) {
    case SMARTWIN_MUSIC_STATUS_PLAY:
        /* code */
        ui_pic_show_image_by_id(SMARTWIN_MUSIC_PS_PIC, 1);
        break;

    case SMARTWIN_MUSIC_STATUS_PAUSE:
        /* code */
        ui_pic_show_image_by_id(SMARTWIN_MUSIC_PS_PIC, 0);
        break;

    default:
        break;
    }
}

/*********************************
            消息响应入口
 *********************************/
int smartwin_status_handler(const char *type, u32 arg)
{
    if (type && (!strcmp(type, "charge"))) {
        switch (arg) {
        case SMARTWIN_CHARGE_STATUS_CHARGING:
            /* 正在充电 */
            break;
        case SMARTWIN_CHARGE_STATUS_DISCONNECT:
            /* 充电断开 */
            break;
        default:
            break;
        }
    } else if (type && (!strcmp(type, "music"))) {
        switch (arg) {
        case SMARTWIN_MUSIC_STATUS_PLAY:
            /* 播放 */
            /* 检测到当前播放音乐，检查一下灵动岛的状态，
                1、如果当前灵动岛类型为SMARTWIN_TYPE_NULL（不显示），则显示灵动岛并且类型为SMARTWIN_TYPE_MUSIC
                2、如果当前灵动岛类型为SMARTWIN_TYPE_MUSIC（显示音乐），则检查音乐状态（播放或者暂停）是否正确 */
            if (smartwin_get_type() == SMARTWIN_TYPE_NULL) {
                smartwin_set_type(SMARTWIN_TYPE_MUSIC);
                /* 灵动岛不显示，需要将其显示出来 */
                smartwin_show(1);
                smartwin_music_status_set(SMARTWIN_MUSIC_STATUS_PLAY);
            } else if (smartwin_get_type() == SMARTWIN_TYPE_MUSIC) {
                if (!smartwin_get_music_state()) {
                    smartwin_music_status_set(SMARTWIN_MUSIC_STATUS_PLAY);
                }
            } else {
                smartwin_set_type(SMARTWIN_TYPE_MUSIC);
                smartwin_show(1);
                smartwin_music_status_set(SMARTWIN_MUSIC_STATUS_PLAY);
            }
            break;
        case SMARTWIN_MUSIC_STATUS_PAUSE:
            /* 暂停 */
            if (smartwin_get_type() == SMARTWIN_TYPE_NULL) {
                smartwin_set_type(SMARTWIN_TYPE_MUSIC);
                /* 灵动岛不显示，需要将其显示出来 */
                smartwin_show(1);
                smartwin_music_status_set(SMARTWIN_MUSIC_STATUS_PAUSE);
            } else if (smartwin_get_type() == SMARTWIN_TYPE_MUSIC) {
                if (smartwin_get_music_state()) {
                    smartwin_music_status_set(SMARTWIN_MUSIC_STATUS_PAUSE);
                }
            } else {
                smartwin_set_type(SMARTWIN_TYPE_MUSIC);
                smartwin_show(1);
                smartwin_music_status_set(SMARTWIN_MUSIC_STATUS_PAUSE);
            }
            break;
        case SMARTWIN_MUSIC_STATUS_STOP:
            /* 停止 */
            /* TODO:可做逻辑将灵动岛隐藏 */
            break;
        default:
            break;
        }
    } else if (type && (!strcmp(type, "message"))) {
        switch (arg) {
        case SMARTWIN_MESSAGE_STATUS_ADD:
            break;
        case SMARTWIN_MESSAGE_STATUS_UPDATE:
            /* 消息更新 */
            if (__this->cur_type == SMARTWIN_TYPE_NULL) {
                smartwin_set_type(SMARTWIN_TYPE_NOTICE);
                smartwin_show(1);
            } else {
                smartwin_set_type(SMARTWIN_TYPE_NOTICE);
                smartwin_show(1);
            }
            // smartwin_set_type(SMARTWIN_TYPE_NOTICE);
            // smartwin_show(1);
            // smartwin_show_start(__this->cur_type);
            break;
        default:
            break;
        }
    }
    return 0;
}

static void SMARTWIN_timer(void *priv)
{
    /* 在表盘的定时器中添加灵动岛音乐状态的检测 */
    u8 smartwin_music_status_check(void);
    if (smartwin_music_status_check() == 1) {
        if (smartwin_get_type() == SMARTWIN_TYPE_NULL) {
            UI_MSG_POST("smartwin_status:music=%4", SMARTWIN_MUSIC_STATUS_PLAY);
        } else if (smartwin_get_type() == SMARTWIN_TYPE_MUSIC) {
            if (!smartwin_get_music_state()) {
                UI_MSG_POST("smartwin_status:music=%4", SMARTWIN_MUSIC_STATUS_PLAY);
            }
        } else {
            if (!smartwin_get_music_state()) {
                UI_MSG_POST("smartwin_status:music=%4", SMARTWIN_MUSIC_STATUS_PLAY);
            }
        }
    } else if (smartwin_music_status_check() == 2) {
        if (smartwin_get_type() == SMARTWIN_TYPE_NULL) {
            UI_MSG_POST("smartwin_status:music=%4", SMARTWIN_MUSIC_STATUS_PAUSE);
        } else if (smartwin_get_type() == SMARTWIN_TYPE_MUSIC) {
            if (smartwin_get_music_state()) {
                UI_MSG_POST("smartwin_status:music=%4", SMARTWIN_MUSIC_STATUS_PAUSE);
            }
        } else {
            if (smartwin_get_music_state()) {
                UI_MSG_POST("smartwin_status:music=%4", SMARTWIN_MUSIC_STATUS_PAUSE);
            }
        }
    }
}

static int smartwin_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        if (!__this) {
            __this = malloc(sizeof(struct smartwin_ctrl));
        }
        smartwin_parm_init();

        ui_register_msg_handler(ID_WINDOW_SMARTWIN, ui_msg_handler);   //注册消息交互的回调

        if (!__this->smartwin_timer) {
            __this->smartwin_timer = sys_timer_add(NULL, SMARTWIN_timer, 500);
        }

        // TODO: 用户自行获取系统状态去设置灵动岛状态
        smartwin_set_type(SMARTWIN_TYPE_NULL);
        smartwin_show(0);
        break;
    case ON_CHANGE_SHOW:
        // smartwin_show_start(__this->cur_type);
        break;
    case ON_CHANGE_RELEASE:
        if (__this) {
            if (__this->smartwin_timer) {
                sys_timer_del(__this->smartwin_timer);
                __this->smartwin_timer = 0;
            }
            free(__this);
            __this = NULL;
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SMARTWIN)
.onchange = smartwin_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif /*if TCFG_UI_ENABLE_HEART*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/


