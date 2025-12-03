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

#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_PHONE_ACTION

#define STYLE_NAME  JL
/*********************
 *      DEFINES
 *********************/
#define __this  (p_phone_call_status_handle)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct phone_call_status_handle {
    u32 show_layout_id;
    struct utime call_active_time;
    int elapse;
    char name[PHONEBOOK_NAME_LEN];
} phone_calll_status_handle_t;


/**********************
 *  STATIC VARIABLES
 **********************/
static phone_calll_status_handle_t *p_phone_call_status_handle;
static u16 phone_end_timer_id;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void phone_call_status_set_show_layout(u32 layout_id, bool is_redraw);
static void phone_call_status_ui_init(int is_redraw);
static int phone_call_layout_is_show_layout(u32 layout_id);
static int phone_ui_voice_to_percent(s8 volume);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
extern void volume_up(void);
extern void volume_down(void);
extern uint32_t timer_get_ms(void);
extern unsigned long bt_phone_call_start_time_get();
extern int jiffies_msec2offset(unsigned long begin_msec, unsigned long end_msec);

/************************************************
 *              UI消息回调处理
 ***********************************************/
static int ui_bt_phone_number_handler(const char *type, u32 arg)
{
    if (!arg) {
        return false;
    }
    if (!__this) {
        return false;
    }
    struct unumber n;
    u8 call_status;
    n.type = TYPE_STRING;
    n.num_str = g_bt_hdl.income_phone_num;
    call_status = call_ctrl_get_status();
    small_file_phonebook_get_name_by_number(__this->name, (char *)g_bt_hdl.income_phone_num);

    log_info("[%s] phone_call_number:%s name:%s", __func__, g_bt_hdl.income_phone_num, __this->name);

    if ((call_status == BT_CALL_OUTGOING) || (call_status == BT_CALL_ALERT)) {
        if (!ui_core_get_disp_status_by_id(PHONE_CALL_STATUS_OUTGOING_LAYOUT)) {
            return false;
        }

        if (strcmp(__this->name, "UNKNOW") == 0) {
            ui_show(PHONE_CALL_OUTGOING_NUMBER);
            ui_number_update_by_id(PHONE_CALL_OUTGOING_NUMBER, &n);
        } else {
            ui_show(PHONE_CALL_OUTGOING_NAME_TEXT);
            ui_text_set_textu_by_id(PHONE_CALL_OUTGOING_NAME_TEXT, (const char *)__this->name, strlen(__this->name), FONT_DEFAULT | FONT_SHOW_SCROLL);
        }
    } else if (call_status == BT_CALL_INCOMING) {
        if (!ui_core_get_disp_status_by_id(PHONE_CALL_STATUS_INCOMING_LAYOUT)) {
            return false;
        }

        if (strcmp(__this->name, "UNKNOW") == 0) {
            ui_show(PHONE_CALL_INCOMING_NUMBER);
            ui_number_update_by_id(PHONE_CALL_INCOMING_NUMBER, &n);
        } else {
            ui_show(PHONE_CALL_INCOMING_NAME_TEXT);
            ui_text_set_textu_by_id(PHONE_CALL_INCOMING_NAME_TEXT, (const char *)__this->name, strlen(__this->name), FONT_DEFAULT | FONT_SHOW_SCROLL);
        }
    } else if (call_status == BT_CALL_ACTIVE) {
        if (strcmp(__this->name, "UNKNOW") == 0) {
            ui_show(PHONE_CALL_ACTIVE_NUMBER);
            ui_number_update_by_id(PHONE_CALL_ACTIVE_NUMBER, &n);
        } else {
            ui_show(PHONE_CALL_ACTIVE_NAME_TEXT);
            ui_text_set_textu_by_id(PHONE_CALL_ACTIVE_NAME_TEXT, (const char *)__this->name, strlen(__this->name), FONT_DEFAULT | FONT_SHOW_SCROLL);
        }
    }
    return true;
}

static int ui_bt_phone_active_handler(const char *type, u32 arg)
{
    if (arg) {
        phone_call_status_set_show_layout(PHONE_CALL_STATUS_ACTIVE_LAYOUT, 1);
    }
    return 0;
}

static int ui_bt_phone_name_handler(const char *type, u32 arg)
{
    if (!arg) {
        return false;
    }
    char *phone_name = (char *)arg;
    log_info("<%s> phone_name:%s", __func__, phone_name);
    return true;
}

static void phone_call_status_page_exit(void *p)
{
    log_info("[%s]", __func__);
    if (get_need_password() == 1) {
        /* UI_HIDE_CURR_WINDOW(); */
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_PHONE_CALL_STATUS);
        UI_SHOW_WINDOW(ID_WINDOW_POWERON_PASSWORD);
    } else {
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_PHONE_CALL_STATUS);
    }
    /* UI_WINDOW_PREEMPTION_POP(ID_WINDOW_PHONE_CALL_STATUS); */
    phone_end_timer_id = 0;
}

static int ui_bt_phone_income_handler(const char *type, u32 arg)
{
    if (!arg) {
        return false;
    }

    log_info("[%s]", __func__);
    phone_call_status_set_show_layout(PHONE_CALL_STATUS_INCOMING_LAYOUT, 1);
    return true;
}

static int ui_bt_phone_hangup_handler(const char *type, u32 arg)
{
    if (!arg) {
        return false;
    }

    log_info("[%s]", __func__);
    phone_end_timer_id = sys_timeout_add(NULL, phone_call_status_page_exit, 500);
    phone_call_status_set_show_layout(PHONE_CALL_STATUS_ENDED_LAYOUT, 1);
    return true;
}

static int ui_bt_phone_out_handler(const char *type, u32 arg)
{
    if (!arg) {
        return false;
    }

    log_info("[%s]", __func__);
    phone_call_status_set_show_layout(PHONE_CALL_STATUS_OUTGOING_LAYOUT, 1);
    return true;
}

static int ui_bt_phone_volume_handler(const char *type, u32 arg)
{
    log_info("[%s]", __func__);
    s8 phone_volume = (s8)arg;
    int vslider_percent;
    vslider_percent = phone_ui_voice_to_percent(phone_volume);
    ui_vslider_set_persent_by_id(PHONE_CALL_VOLUME_VSLIDER, vslider_percent);

    return true;
}

static const struct uimsg_handl ui_msg_handler[] = {
    { "phone_num",          ui_bt_phone_number_handler         },
    { "phone_income",       ui_bt_phone_income_handler         },
    { "phone_active",       ui_bt_phone_active_handler         },
    { "phone_hangup",       ui_bt_phone_hangup_handler         },
    { "phone_out",          ui_bt_phone_out_handler            },
    { "phone_name",         ui_bt_phone_name_handler           },
    { "phone_volume",       ui_bt_phone_volume_handler         },
    { NULL, NULL},      /* 必须以此结尾！ */
};



/************************************************
 *          通话状态页面 初始化相关
 ***********************************************/
static int phone_call_status_window_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        __this = (phone_calll_status_handle_t *)zalloc(sizeof(phone_calll_status_handle_t));
        if (!__this) {
            log_error("[%s] line:%d zalloc fail", __func__, __LINE__);
            break;
        }
        key_ui_takeover(1);
        ui_register_msg_handler(ID_WINDOW_PHONE_CALL_STATUS, ui_msg_handler);
        phone_call_status_ui_init(0);
        /*通话页面处于抢占页面缓存时候,当前的抢占页面退出时候,
          去显示通话页面时候,要主动在去读取号码进行匹配*/
        if (g_bt_hdl.phone_num_flag == 1) {
            small_file_phonebook_get_name_by_number(__this->name, (char *)g_bt_hdl.income_phone_num);
        } else {
            bt_cmd_prepare(USER_CTRL_HFP_CALL_CURRENT, 0, NULL); //发命令获取电话号码
        }
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_RELEASE:
        if (__this) {
            free(__this);
            __this = NULL;
        }
        key_ui_takeover(0);
        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return false;
}

static int phone_call_status_window_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
    case ELM_EVENT_TOUCH_L_MOVE:
        return true; // 通话左右滑动不响应
    default:
        break;
    }
    return false;
}

static int ui_phone_call_status_window_onkey(void *ctr, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_UI_HOME:
        u8 call_status = bt_get_call_status();
        log_info("<%s> call_status:%d", __func__, call_status);
        if ((call_status == BT_CALL_OUTGOING) || (call_status == BT_CALL_ALERT) || \
            (call_status == BT_CALL_ACTIVE)) {
            small_file_call_log_set_type(CALL_INCOME_REJECT);
            call_ctrl_hangup();
        } else if (call_status == BT_CALL_INCOMING) {
            call_ctrl_answer();
        }
        return true;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_PHONE_CALL_STATUS)
.onchange = phone_call_status_window_onchange,
 .onkey = ui_phone_call_status_window_onkey,
  .ontouch = phone_call_status_window_ontouch,
};


static int phone_call_status_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT_PROBE:
        if (phone_call_layout_is_show_layout(layout->elm.id) == true) {
            layout->elm.css.invisible = 0;
        } else {
            layout->elm.css.invisible = 1;
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_STATUS_INCOMING_LAYOUT)
.onchange = phone_call_status_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_STATUS_ACTIVE_LAYOUT)
.onchange = phone_call_status_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_STATUS_OUTGOING_LAYOUT)
.onchange = phone_call_status_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_STATUS_ENDED_LAYOUT)
.onchange = phone_call_status_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/************************************************
 *         去电(OUTGOING) 相关控件处理
 ***********************************************/
static int phone_call_outgoing_hangup_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            u8 call_status = bt_get_call_status();
            if ((call_status == BT_CALL_OUTGOING) || (call_status == BT_CALL_ALERT)) {
                call_ctrl_hangup();
            }
        }
        return true;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_OUTGOING_HANGUP_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_call_outgoing_hangup_ontouch,
};

static int phone_call_outgoing_number_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_number *number = (struct ui_number *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (strcmp(__this->name, "UNKNOW") != 0 || \
            g_bt_hdl.phone_num_flag == 0) {
            number->text.elm.css.invisible = 1;
            break;
        }

        struct unumber n;
        n.type = TYPE_STRING;
        n.num_str = g_bt_hdl.income_phone_num;
        ui_number_update(number, &n);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_OUTGOING_NUMBER)
.onchange = phone_call_outgoing_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int phone_call_outgoing_name_text_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (strcmp(__this->name, "UNKNOW") == 0) {
            text->elm.css.invisible = 1;
            break;
        }

        ui_text_set_text_attrs(
            text,
            (const char *)__this->name,
            strlen(__this->name),
            FONT_ENCODE_UTF8,
            1,
            FONT_DEFAULT | FONT_SHOW_MULTI_LINE
        );
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_OUTGOING_NAME_TEXT)
.onchange = phone_call_outgoing_name_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 *         通话中(ACTIVE) 相关控件处理
 ***********************************************/
static int phone_call_active_hangup_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            if (bt_get_call_status() == BT_CALL_ACTIVE) {
                call_ctrl_hangup();
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_ACTIVE_HANGUP_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_call_active_hangup_pic_ontouch,
};

static int phone_call_active_mute_switch_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            call_ctrl_input_mute(!call_ctrl_get_input_mute());
            log_debug("call_ctrl_get_input_mute():%d", call_ctrl_get_input_mute());
            if (call_ctrl_get_input_mute()) {
                ui_pic_show_image_by_id(pic->elm.id, 1);
            } else {
                ui_pic_show_image_by_id(pic->elm.id, 0);
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_ACTIVE_MUTE_SWITCH)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_call_active_mute_switch_pic_ontouch,
};

static int phone_call_active_number_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_number *number = (struct ui_number *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (strcmp(__this->name, "UNKNOW") != 0) {
            printf("__this->name:%s", __this->name);
            number->text.elm.css.invisible = 1;
            break;
        }

        struct unumber n;
        n.type = TYPE_STRING;
        n.num_str = g_bt_hdl.income_phone_num;
        ui_number_update(number, &n);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_ACTIVE_NUMBER)
.onchange = phone_call_active_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int phone_call_active_name_text_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (strcmp(__this->name, "UNKNOW") == 0) {
            text->elm.css.invisible = 1;
            break;
        }

        ui_text_set_text_attrs(
            text,
            (const char *)__this->name,
            strlen(__this->name),
            FONT_ENCODE_UTF8,
            1,
            FONT_DEFAULT | FONT_SHOW_MULTI_LINE
        );
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_ACTIVE_NAME_TEXT)
.onchange = phone_call_active_name_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_phone_call_active_time_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        int time_sec = jiffies_msec2offset(bt_phone_call_start_time_get(), jiffies_msec()) / 1000;
        __this->call_active_time.hour = time_sec / 3600;
        __this->call_active_time.min = (time_sec % 3600) / 60;
        __this->call_active_time.sec = time_sec % 60;
        ui_time_update(time, &__this->call_active_time);
        break;
    case ON_CHANGE_RELEASE:
        if (!__this) {
            break;
        }
        /* __this->call_active_time.min = time->min; */
        /* __this->call_active_time.sec = time->sec; */
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_ACTIVE_TIME)
.onchange = ui_phone_call_active_time_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/************************************************
 *        来电(INCOMING) 相关控件处理
 ***********************************************/
static int phone_call_incoming_hangup_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            u8 call_status = bt_get_call_status();
            if (call_status == BT_CALL_INCOMING) {
                small_file_call_log_set_type(CALL_INCOME_REJECT);
                call_ctrl_hangup();
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_INCOMING_HANDUP)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_call_incoming_hangup_pic_ontouch,
};

static int phone_call_incoming_answer_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            u8 call_status = bt_get_call_status();
            if (call_status == BT_CALL_INCOMING) {
                call_ctrl_answer();
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_INCOMING_ANSWER)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_call_incoming_answer_pic_ontouch,
};

static int phone_call_incoming_number_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_number *number = (struct ui_number *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (strcmp(__this->name, "UNKNOW") != 0 || \
            g_bt_hdl.phone_num_flag == 0) {
            number->text.elm.css.invisible = 1;
            break;
        }

        struct unumber n;
        n.type = TYPE_STRING;
        n.num_str = g_bt_hdl.income_phone_num;
        ui_number_update(number, &n);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_INCOMING_NUMBER)
.onchange = phone_call_incoming_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int phone_call_incoming_name_text_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (strcmp(__this->name, "UNKNOW") == 0) {
            text->elm.css.invisible = 1;
            break;
        }

        ui_text_set_text_attrs(
            text,
            (const char *)__this->name,
            strlen(__this->name),
            FONT_ENCODE_UTF8,
            1,
            FONT_DEFAULT | FONT_SHOW_MULTI_LINE
        );
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_INCOMING_NAME_TEXT)
.onchange = phone_call_incoming_name_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 *      通话已结束(ENDED) 相关控件处理
 ***********************************************/
static int phone_call_ended_number_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_number *number = (struct ui_number *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (strcmp(__this->name, "UNKNOW") != 0) {
            number->text.elm.css.invisible = 1;
            break;
        }

        struct unumber n;
        n.type = TYPE_STRING;
        n.num_str = g_bt_hdl.income_phone_num;
        ui_number_update(number, &n);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_ENDED_NUMBER)
.onchange = phone_call_ended_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int phone_call_ended_name_text_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (strcmp(__this->name, "UNKNOW") == 0) {
            text->elm.css.invisible = 1;
            break;
        }

        ui_text_set_text_attrs(
            text,
            (const char *)__this->name,
            strlen(__this->name),
            FONT_ENCODE_UTF8,
            1,
            FONT_DEFAULT | FONT_SHOW_MULTI_LINE
        );
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_ENDED_NAME_TEXT)
.onchange = phone_call_ended_name_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/************************************************
 *          音量控制 相关控件处理
 ***********************************************/
/**
 * 音量控制布局(PHONE_CALL_VOLUME_LAYOUT)进入
 */
static int phone_call_volume_enter_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    /* int vslider_percent; */
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            /* vslider_percent = phone_ui_voice_to_percent(app_audio_get_volume(APP_AUDIO_STATE_CALL)); */
            /* log_info("%s %d--vslider_percent:%d",__func__,__LINE__,vslider_percent); */
            /* ui_vslider_set_persent(vslider, vslider_percent); */
            phone_call_status_set_show_layout(PHONE_CALL_VOLUME_LAYOUT, 1);
        }
        return true;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_OUTGOING_VOLUME_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_call_volume_enter_pic_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_ACTIVE_VOLUME_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_call_volume_enter_pic_ontouch,
};


/**
 * 音量增减控制
 */
static void phone_ui_set_voice(int precent)
{
    s8 volume;

    volume = app_audio_get_max_volume() * precent / 100;
    if (volume > app_audio_get_max_volume()) {
        volume = app_audio_get_max_volume();
    }

    log_info("%s %d--volume:%d, max_volume:%d", __FUNCTION__, __LINE__, volume, app_audio_get_max_volume());
    app_audio_set_volume(APP_AUDIO_STATE_CALL, volume, 1);
    bt_cmd_prepare(USER_CTRL_HFP_CALL_SET_VOLUME, 1, (u8 *)&volume);
}

static int phone_ui_voice_to_percent(s8 volume)
{
    log_info("<%s> volume:%d, max_volume:%d", __func__, volume, app_audio_get_max_volume());
    return volume * 100 / app_audio_get_max_volume();
}

static void phone_ui_volume_up(void)
{
    volume_up();
}

static void phone_ui_volume_down(void)
{
    volume_down();
}

static int vslider_voice_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_vslider *vslider = (struct ui_vslider *)_ctrl;
    int vslider_percent;
    switch (event) {
    case ON_CHANGE_INIT:
        vslider_percent = phone_ui_voice_to_percent(app_audio_get_volume(APP_AUDIO_STATE_CALL));
        log_debug("%s %d--vslider_percent:%d", __func__, __LINE__, vslider_percent);
        ui_vslider_set_persent(vslider, vslider_percent);
        /* __this->elapse = timer_get_ms(); */
        break;
    case ON_CHANGE_RELEASE:
        if (!__this) {
            break;
        }
        /* __this->elapse = timer_get_ms() - __this->elapse; */
        /* u8 min =  __this->elapse / 1000 / 60; */
        /* u8 sec =  __this->elapse / 1000 % 60; */
        /* __this->call_active_time.min += min; */
        /* __this->call_active_time.sec += sec ; */
        break;
    default :
        break;
    }
    return 0;
}

static int vslider_voice_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_vslider *vslider = (struct ui_vslider *)ctr;
    int vslider_percent;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_MOVE:
        vslider_touch_slider_move(vslider, e);
        break;
    case ELM_EVENT_TOUCH_UP:
        vslider_percent = vslider_get_percent(vslider);
        log_info("%s %d--vslider_percent:%d", __FUNCTION__, __LINE__, vslider_percent);
        phone_ui_set_voice(vslider_percent);
        break;
    default :
        break;
    }

    return false;
}

static int ui_phone_call_volume_vslider_onkey(void *ctr, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_UI_PLUS:
        phone_ui_volume_up();
        return true;
    case KEY_UI_MINUS:
        phone_ui_volume_down();
        return true;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PHONE_CALL_VOLUME_VSLIDER)
.onchange = vslider_voice_onchange,
 .onkey = ui_phone_call_volume_vslider_onkey,
  .ontouch = vslider_voice_ontouch,
};

static int pic_voice_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    int vslider_percent;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            switch (pic->elm.id) {
            case PHONE_CALL_VOLUME_UP_PIC:
                phone_ui_volume_up();
                break;
            case PHONE_CALL_VOLUME_DOWN_PIC:
                phone_ui_volume_down();
                break;
            default:
                return false;
            }
            vslider_percent = phone_ui_voice_to_percent(app_audio_get_volume(APP_AUDIO_STATE_CALL));
            ui_vslider_set_persent_by_id(PHONE_CALL_VOLUME_VSLIDER, vslider_percent);
        }
        break;
    default :
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_VOLUME_UP_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = pic_voice_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_VOLUME_DOWN_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = pic_voice_ontouch,
};

/* 音量控制布局反回通话状态布局 */
static int phone_call_volume_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        phone_call_status_ui_init(1);
        return true;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_VOLUME_LAYOUT)//声音
.onchange = phone_call_status_layout_onchange,
 .onkey = NULL,
  .ontouch = phone_call_volume_ontouch,
};



/************************************************
 *             控制布局显示API
 ***********************************************/

/* ------------------------------------------------------------------------------------*/
/**
 * @brief  控制布局显示
 *
 * @Params layout_id:要去显示的布局
 * @Params is_redraw 0:不重绘 1:重绘
 */
/* ------------------------------------------------------------------------------------*/
static void phone_call_status_set_show_layout(u32 layout_id, bool is_redraw)
{

    if (is_redraw == 0) {
        log_debug("<%s> hide %x -> show %x", __func__, __this->show_layout_id, layout_id);
        __this->show_layout_id = layout_id;
    } else {
        log_debug("<%s> hide %x -> show %x", __func__, __this->show_layout_id, layout_id);
        ui_hide(__this->show_layout_id);
        __this->show_layout_id = layout_id;
        ui_show(__this->show_layout_id);
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief 根据蓝牙状态，确定应该显示哪个布局
 *
 * @Params is_redraw 0:不重绘 1:重绘
 */
/* ------------------------------------------------------------------------------------*/
static void phone_call_status_ui_init(int is_redraw)
{
    u8 status = call_ctrl_get_status();
    log_info("[%s] STATUS = %d\n", __func__, status);

    switch (status) {
    case BT_CALL_OUTGOING:
    case BT_CALL_ALERT:
        phone_call_status_set_show_layout(PHONE_CALL_STATUS_OUTGOING_LAYOUT, is_redraw);
        break;
    case BT_CALL_INCOMING:
        phone_call_status_set_show_layout(PHONE_CALL_STATUS_INCOMING_LAYOUT, is_redraw);
        break;
    case BT_CALL_ACTIVE:
        phone_call_status_set_show_layout(PHONE_CALL_STATUS_ACTIVE_LAYOUT, is_redraw);
        break;
    default:
        phone_call_status_set_show_layout(0xffffffff, is_redraw);
        break;
    }
}

static int phone_call_layout_is_show_layout(u32 layout_id)
{
    if (__this->show_layout_id == layout_id) {
        return true;
    }
    return false;
}
void ui_phone_call_end_status_check(void)
{
    log_debug("<%s> phone_end_timer_id:%d", __func__, phone_end_timer_id);
    if (phone_end_timer_id) {
        log_info("<%s> del id", __func__);
        sys_timeout_del(phone_end_timer_id);
        phone_end_timer_id = 0;
    }
}

#endif /* if TCFG_UI_ENABLE_PHONE_ACTION */
#endif /* #if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) */

