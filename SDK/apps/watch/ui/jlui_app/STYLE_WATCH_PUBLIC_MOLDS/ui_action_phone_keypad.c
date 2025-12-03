/**
 * @file ui_action_phone_keypad.c
 * \Brief : 电话拨号盘功能
 */
#include "system/includes.h"
#include "app_task.h"
#include "system/timer.h"
#include "app_common.h"
#include "avctp_user.h"
#include "ui.h"
#include "ui_api.h"
#include "jlui_app/ui_style.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-PHONE-KEYPAD]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_phone_kaypad.data.bss")
#pragma data_seg(".ui_action_phone_kaypad.data")
#pragma const_seg(".ui_action_phone_kaypad.text.const")
#pragma code_seg(".ui_action_phone_kaypad.text")
#endif


#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_PHONE_ACTION

#define STYLE_NAME  JL

/**********************
 *      DEFINES
 *********************/
#define __this  (p_phone_keypad_handle)

/**********************
 *      TYPEDEFS
 *********************/
typedef struct phone_keypad_handle {
    char number[32];
    int index;
} phone_keypad_handle_t;

/**********************
 *  STATIC VARIABLES
 *********************/
static phone_keypad_handle_t *p_phone_keypad_handle;


/************************************************
 *                 初始化相关
 ***********************************************/
static int phone_keypad_layer_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layer *layer = (struct layer *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        __this = (phone_keypad_handle_t *)zalloc(sizeof(phone_keypad_handle_t));
        if (!__this) {
            log_error("[%s] line:%d zalloc fail", __func__, __LINE__);
            return true;
        }
        break;
    case ON_CHANGE_RELEASE:
        if (__this) {
            free(__this);
            __this = NULL;
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_LAYER)
.onchange = phone_keypad_layer_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/************************************************
 *         电话拨号键盘下相关页面跳转
 ***********************************************/
static int phone_keypad_back_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            UI_WINDOW_BACK_SHOW(2);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_BACK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_back_pic_ontouch,
};



/************************************************
 *                 点击拨号
 ***********************************************/
static void phone_keypad_call_handler(void)
{
    log_info("[%s] line:%d", __func__, __LINE__);
    if (!strcmp(__this->number, "*")) {              // 按*进入工程模式
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_ENGINEERING_MODE);
        return;
    }
    call_dial_number(__this->index, __this->number);
}

static void phone_keypad_add_number(char key_value)
{
    struct unumber n;

    if (__this->index >= 19) {
        return;
    }
    __this->number[__this->index++] = key_value;
    __this->number[__this->index] = '\0';
    n.type = TYPE_STRING;
    n.num_str = (u8 *)__this->number;

    log_info("add __this->number:%s", __this->number);
    ui_number_update_by_id(PHONE_KEYPAD_NUMBER, &n);
}

static void phone_keypad_del_number(void)
{
    struct unumber n;

    if (__this->index <= 0) {
        return;
    }
    __this->number[--__this->index] = '\0';
    n.type = TYPE_STRING;
    n.num_str = (u8 *)__this->number;
    log_info("del __this->number:%s", __this->number);
    ui_number_update_by_id(PHONE_KEYPAD_NUMBER, &n);
}

static void phone_keypad_sel_elm_handler(u32 sel_elm_id)
{
    /* log_info("[%s] sel_elm_id:%x", __func__, sel_elm_id); */
    switch (sel_elm_id) {
    case PHONE_KEYPAD_0_PIC:
        phone_keypad_add_number('0');
        break;
    case PHONE_KEYPAD_1_PIC:
        phone_keypad_add_number('1');
        break;
    case PHONE_KEYPAD_2_PIC:
        phone_keypad_add_number('2');
        break;
    case PHONE_KEYPAD_3_PIC:
        phone_keypad_add_number('3');
        break;
    case PHONE_KEYPAD_4_PIC:
        phone_keypad_add_number('4');
        break;
    case PHONE_KEYPAD_5_PIC:
        phone_keypad_add_number('5');
        break;
    case PHONE_KEYPAD_6_PIC:
        phone_keypad_add_number('6');
        break;
    case PHONE_KEYPAD_7_PIC:
        phone_keypad_add_number('7');
        break;
    case PHONE_KEYPAD_8_PIC:
        phone_keypad_add_number('8');
        break;
    case PHONE_KEYPAD_9_PIC:
        phone_keypad_add_number('9');
        break;
    case PHONE_KEYPAD_STAR_PIC:
        phone_keypad_add_number('*');
        break;
    case PHONE_KEYPAD_POUND_PIC:
        phone_keypad_add_number('#');
        break;
    case PHONE_KEYPAD_DEL_PIC:
        phone_keypad_del_number();
        break;
    case PHONE_KEYPAD_CALL_PIC:
        phone_keypad_call_handler();
        break;
    default:
        break;
    }
}

static int phone_keypad_key_common_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        /* log_info("[%s] pic_id:0x%x", __func__, pic->elm.id); */
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            phone_keypad_sel_elm_handler(pic->elm.id);
        }
        return true;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_0_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_1_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_2_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_3_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_4_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_5_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_6_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_7_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_8_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_9_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_STAR_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_POUND_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_CALL_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_DEL_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = phone_keypad_key_common_ontouch,
};



/************************************************
 *              电话号码显示
 ***********************************************/
extern void *ui_core_load_widget_info(void *__head, u8 page);
static int phone_keypad_number_vsprintf(struct ui_number *number, struct ui_number_info *info, u16 *buf)
{
    u8 *str;

    if (!number->num_str) {
        return 0;
    }

    str = number->num_str;
    int len = strlen((const char *)str);

    ASSERT(len < 20);

    log_info("ui_number str:%s", str);
    int i;
    for (i = 0; i < len; i++) {
        if (str[i] == ' ') {    /*' '字符对应的图片*/
            if (info->space[0] != 0xffff) {
                buf[i] = info->space[0];        /*空格图片列表，最多2张*/
            } else {
                buf[i] = 0xff;
                break;
            }
        } else if (str[i] >= '0' && str[i] <= '9') {    /*0-9字符对应的图片*/
            if (info->number[str[i] - '0'] != 0xffff) {
                buf[i] = info->number[str[i] - '0'];    /*数字图片列表，最多10张*/
            } else {
                buf[i] = 0xff;
                break;
            }
        } else if (str[i] == '*') {     /*'*'字符对应的图片*/
            if (info->delimiter[0] != 0xffff) {
                buf[i] = info->delimiter[0];
            } else {
                buf[i] = 0xff;
            }
        } else if (str[i] == '#') {     /*'#'字符对应的图片*/
            if (info->delimiter[1] != 0xffff) {
                buf[i] = info->delimiter[1];
            } else {
                buf[i] = 0xff;
            }
        } else {
            buf[i] = 0xff;
        }
    }
    buf[i] = buf[i + 1] = 0xff;
    /* put_buf(buf,len+1); */
    return 0;
}

/**
 * @brief 用于数字控件，string类型，图片显示类型
 *
 * @note 最多22张
 */
void phone_keypad_number_update(struct ui_number *number)
{
    struct ui_number_info *info = ui_core_load_widget_info((void *)number->info, -1);

    phone_keypad_number_vsprintf(number, info, number->buf);

    if ((info->number[0] > 0) && (info->number[0] != 0xffff)) {
        text_element_set_text(&number->text, (char *)number->buf, UI_TEXT_ENCODE_IMAGE, number->text.elm.highlight ? number->hi_color : number->color);
    } else {
        log_error("ui number no picture");
        return;
    }
}

static int phone_keypad_number_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_number *number = (struct ui_number *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW_PROBE:
        phone_keypad_number_update(number);
        return true;    /*接管事件,应用层处理number数据*/
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_KEYPAD_NUMBER)
.onchange = phone_keypad_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif /* if TCFG_UI_ENABLE_PHONE_ACTION */
#endif /* #if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) */

