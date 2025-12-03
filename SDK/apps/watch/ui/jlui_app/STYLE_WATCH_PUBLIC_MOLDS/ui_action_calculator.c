/**
 * @file ui_action_calculator.c
 * @brief 显示总数字(包括小数点 负号)12位
 *        计算结果:数字数量没有超过12位,正常显示
          超过12位，按科学技术法显示，最多显示小数点后6位, 会进行四舍五入
 */

#include "stdlib.h"
#include "app_config.h"
#include "jlui_app/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "app_main.h"
#include "init.h"
#include <math.h>

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-CALCULATOR]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_calculator.data.bss")
#pragma data_seg(".ui_action_calculator.data")
#pragma const_seg(".ui_action_calculator.text.const")
#pragma code_seg(".ui_action_calculator.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_CALCULATOR

#define STYLE_NAME  JL


/**********************
 *      DEFINES
 *********************/
#define NUM_SHOW_MAX 		12  //最大输入位数, 也是显示位数

#ifndef fabs
#define fabs(x)		(((x) < 0.0f) ? (-x) : (x))		// 浮点绝对值
#endif

#define __this (p_calculator_handle)

/**********************
 * TYPEDEFS
 *********************/
enum {
    OPERATOR_NONE,
    OPERATOR_ADD,
    OPERATOR_SUB,
    OPERATOR_MUL,
    OPERATOR_DIV,
};

enum {
    ERR_NONE,
    ERR_CALCULATE,  /*计算结果错误*/
};

typedef struct calculator_handle {
    double operand1;
    double operand2;
    u8 operator;
    u8 operator_input;                   /* 当前输入的operator */
    char num_show_str[NUM_SHOW_MAX + 1]; /* 当前输入值 也是正在显示的数字 */
    u8 operand_float_flag;               /* 表明当前operand是浮点数 0:不是 1:是*/
    int err_code;                        /* 错误码 */

    u32 ui_operator_elm_id;              /* 记录当前输入的ui_operator_elm_id */
} calculator_handle_t;

/**********************
 * STATIC VARIABLES
 *********************/
static calculator_handle_t *p_calculator_handle;

/**********************
 * STATIC PROTOTYPES
 *********************/
static void calc_init(void);
static void calc_deinit(void);
static void calc_operand_input(char num);
static void calc_operator_input(u8 operator);
static void calc_equal(void);
static void calc_del(void);
static void calc_ac(void);

/************************************************
 *               计算器功能
 ***********************************************/
static int ui_calculator_window_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        calc_init();
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_RELEASE:
        calc_deinit();
        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_CALCULATOR)
.onchange = ui_calculator_window_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/* 触摸按键ui处理 */
static void ui_calculator_key(struct element *elm, u8 pic_index)
{
    struct ui_pic *bg_pic;
    switch (elm->id) {
    //数字
    case CAL_NUM_0:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_NUM_0_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_NUM_1:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_NUM_1_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_NUM_2:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_NUM_2_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_NUM_3:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_NUM_3_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_NUM_4:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_NUM_4_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_NUM_5:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_NUM_5_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_NUM_6:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_NUM_6_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_NUM_7:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_NUM_7_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_NUM_8:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_NUM_8_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_NUM_9:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_NUM_9_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    //四则运算
    case CAL_ADD:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_ADD_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_SUB:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_SUB_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_MUL:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_MUL_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_DIV:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_DIV_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_EQUAL:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_EQUAL_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    //功能按键
    case CAL_POINT:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_POINT_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_AC:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_AC_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    case CAL_DEL:
        ui_pic_set_image_index((struct ui_pic *)elm, pic_index);
        bg_pic = ui_pic_for_id(CAL_DEL_BG_PIC);
        ui_pic_set_image_index(bg_pic, pic_index);
        break;
    }
    struct element *window_elm = ui_core_get_element_by_id(ID_WINDOW_CALCULATOR);
    ui_core_redraw(window_elm);
}

/* 触摸按键响应 */
static int calculator_key(int key)
{
    switch (key) {
    //数字
    case CAL_NUM_0:
        log_debug("CAL_NUM_0");
        calc_operand_input('0');
        break;
    case CAL_NUM_1:
        log_debug("CAL_NUM_1");
        calc_operand_input('1');
        break;
    case CAL_NUM_2:
        log_debug("CAL_NUM_2");
        calc_operand_input('2');
        break;
    case CAL_NUM_3:
        log_debug("CAL_NUM_3");
        calc_operand_input('3');
        break;
    case CAL_NUM_4:
        log_debug("CAL_NUM_4");
        calc_operand_input('4');
        break;
    case CAL_NUM_5:
        log_debug("CAL_NUM_5");
        calc_operand_input('5');
        break;
    case CAL_NUM_6:
        log_debug("CAL_NUM_6");
        calc_operand_input('6');
        break;
    case CAL_NUM_7:
        log_debug("CAL_NUM_7");
        calc_operand_input('7');
        break;
    case CAL_NUM_8:
        log_debug("CAL_NUM_8");
        calc_operand_input('8');
        break;
    case CAL_NUM_9:
        log_debug("CAL_NUM_9");
        calc_operand_input('9');
        break;
    //四则运算
    case CAL_ADD:
        log_debug("CAL_ADD");
        calc_operator_input(OPERATOR_ADD);
        break;
    case CAL_SUB:
        log_debug("CAL_SUB");
        calc_operator_input(OPERATOR_SUB);
        break;
    case CAL_MUL:
        log_debug("CAL_MUL");
        calc_operator_input(OPERATOR_MUL);
        break;
    case CAL_DIV:
        log_debug("CAL_DIV");
        calc_operator_input(OPERATOR_DIV);
        break;
    case CAL_EQUAL:
        log_debug("CAL_EQUAL");
        calc_equal();
        break;
    //功能按键
    case CAL_POINT:
        log_debug("CAL_POINT");
        calc_operand_input('.');
        break;
    case CAL_AC:
        log_debug("CAL_AC");
        calc_ac();
        break;
    case CAL_DEL:
        log_debug("CAL_DEL");
        calc_del();
        break;
    }
    return 0;
}

static int ui_calculator_num_ontouch(void *ctr, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        log_info("<%s> down", __func__);
        struct element *operator_elm = ui_core_get_element_by_id(__this->ui_operator_elm_id);
        ui_calculator_key(operator_elm, 0);
        ui_calculator_key(elm, 1);
        return true;
    case ELM_EVENT_TOUCH_UP:
        log_info("<%s> wp", __func__);
        /* 为了触摸方便 就不判断UP事件前一个触摸事件是down事件 */
        ui_calculator_key(elm, 0);
        calculator_key(elm->id);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(CAL_NUM_0)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_NUM_1)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_NUM_2)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_NUM_3)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_NUM_4)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_NUM_5)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_NUM_6)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_NUM_7)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_NUM_8)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_NUM_9)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_POINT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};

REGISTER_UI_EVENT_HANDLER(CAL_EQUAL)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_AC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_DEL)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_num_ontouch,
};


static int ui_calculator_operator_ontouch(void *ctr, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        log_info("<%s> down", __func__);
        return true;
    case ELM_EVENT_TOUCH_UP:
        log_info("<%s> wp", __func__);
        /* 为了触摸方便 就不判断UP事件前一个触摸事件是down事件 */
        struct element *operator_elm = ui_core_get_element_by_id(__this->ui_operator_elm_id);
        ui_calculator_key(operator_elm, 0);
        __this->ui_operator_elm_id = elm->id;
        ui_calculator_key(elm, 1);
        calculator_key(elm->id);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(CAL_ADD)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_operator_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_SUB)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_operator_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_MUL)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_operator_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAL_DIV)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calculator_operator_ontouch,
};


/************************************************
 *                数字显示
 ***********************************************/
extern void *ui_core_load_widget_info(void *__head, u8 page);
static int calc_number_vsprintf(struct ui_number *number, struct ui_number_info *info, u16 *buf)
{
    u8 str[32] = {0};

    if (!number->num_str) {
        return 0;
    }

    strcpy((void *)str, (const char *)number->num_str);
    int len = strlen((const char *)str);

    ASSERT(len < 20);

    log_info("<%s> str:%s", __func__, str);
    int i;
    for (i = 0; i < len; i++) {
        if (str[i] == '-') {    /*'-'字符对应的图片*/
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
        } else if (str[i] == '.') {     /*'.'字符对应的图片*/
            if (info->delimiter[0] != 0xffff) {
                buf[i] = info->delimiter[0];
            } else {
                buf[i] = 0xff;
            }
        } else if (str[i] == 'e') {     /*'2'字符对应的图片*/
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
 * @note 最多13张
 */
void calc_number_update(struct ui_number *number)
{
    struct ui_number_info *info = ui_core_load_widget_info((void *)number->info, -1);

    calc_number_vsprintf(number, info, number->buf);

    if ((info->number[0] > 0) && (info->number[0] != 0xffff)) {
        text_element_set_text(&number->text, (char *)number->buf, UI_TEXT_ENCODE_IMAGE, number->text.elm.highlight ? number->hi_color : number->color);
    } else {
        log_error("ui number no picture");
        return;
    }
}

static int ui_calculator_number_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_number *number = (struct ui_number *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        struct unumber unum;
        unum.type = TYPE_STRING;
        unum.num_str = (u8 *)__this->num_show_str;
        ui_number_update(number, &unum);
        break;
    case ON_CHANGE_SHOW_PROBE:
        calc_number_update(number);
        return true;    /*接管事件,应用层处理number数据*/
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(CAL_RESULT)
.onchange = ui_calculator_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/************************************************
 *              显示相关API
 ***********************************************/

/**
 * @brief 刷新数字显示
 *
 * @param num_show_str 所指向buf的存储周期是这个控件的存在周期
 */
static void calc_result_number_show_update(char *num_show_str)
{
    struct unumber unum;
    static char number_str[NUM_SHOW_MAX + 1];

    memcpy(number_str, num_show_str, sizeof(number_str));
    unum.type = TYPE_STRING;
    unum.num_str = (u8 *)number_str;
    ui_number_update_by_id(CAL_RESULT, &unum);
}



/************************************************
 *              数学计算相关API
 ***********************************************/

/* 加法 */
static double add(double operand1, double operand2)
{
    return (operand1 + operand2);
}

/* 减法 */
static double subtract(double operand1, double operand2)
{
    return (operand1 - operand2);
}

/* 乘法 */
static double multiply(double operand1, double operand2)
{
    return (operand1 * operand2);
}

/* 除法 */
static double divide(double operand1, double operand2)
{
    if (operand2 == 0.0) {
        return NAN;
    }
    return (operand1 / operand2);
}

static double math_operate(double operand1, double operand2, u8 operator)
{
    double result;
    switch (operator) {
    case OPERATOR_ADD:
        result = add(operand1, operand2);
        break;
    case OPERATOR_SUB:
        result = subtract(operand1, operand2);
        break;
    case OPERATOR_MUL:
        result = multiply(operand1, operand2);
        break;
    case OPERATOR_DIV:
        result = divide(operand1, operand2);
        break;
defulet:
        result = NAN;
        break;
    }
    if (isnan(result)) {
        log_error("<%s> result is NAN ", __func__);
    } else {
        log_debug("operand1:%e, operand2:%e, operator:0x%x, result:%e", operand1, operand2, operator, result);
        log_debug("result:%f", result);
    }
    return result;
}

#if 0
void calc_math_test()
{
    math_operate(23.1, 2, OPERATOR_MUL);
    math_operate(23.1, 0.0, OPERATOR_DIV);
    math_operate(23.1, -2.0, OPERATOR_DIV);
    math_operate(43.1234555, 1.0, OPERATOR_DIV);
    math_operate(43.1234544, 1.0, OPERATOR_DIV);
    math_operate(2147483647, 2.0, OPERATOR_MUL);
}
#endif



/************************************************
 *              计算器功能相关API
 ***********************************************/

/*
 * 围绕计算operand1 operate operand2 来进行 比如:36 * 35
 * operator_input num_show_str 这两个输入值会刷新进入到operand operate
 */

/**
 * @brief 字符串转浮点数
 *
 * @param str 存储字符串数据的buf
 */
static double calc_atof(char *str)
{
    double s = 0.0;
    double d = 10.0;
    int jishu = 0;
    bool falg = false;
    while (*str == ' ') {
        str++;
    }
    if (*str == '-') { //记录数字正负
        falg = true;
        str++;
    }
    if (!(*str >= '0' && *str <= '9')) { //如果一开始非数字则退出，返回0.0
        return s;
    }
    while (*str >= '0' && *str <= '9' && *str != '.') { //计算小数点前整数部分
        s = s * 10.0 + *str - '0';
        str++;
    }
    if (*str == '.') { //以后为小数部分
        str++;
    }
    while (*str >= '0' && *str <= '9') { //计算小数部分
        s = s + (*str - '0') / d;
        d *= 10.0;
        str++;
    }
    s = s * (falg ? -1.0 : 1.0);
    return s;
}

static void calc_get_integer_decimal_digits(double num, int *integer_digits, int *decimal_digits)
{
    long long integer_part = (long long)num;

    /* Count the number of digits in the integer part */
    while (integer_part != 0) {
        integer_part /= 10;
        (*integer_digits)++;
    }

    /* Get the decimal part */
    double decimal_part = num - (long long)num;

    /* #<{(| Count the number of digits in the decimal part |)}># */
    /* decimal_part = fabs(decimal_part); // Handle negative decimal parts */
    /* long long tmp_decimal_part = (long long)(decimal_part * 10000000);// Adjust the threshold for precision */
    /* printf("<%s> decimal_part:%f tmp_decimal_part:%lld", __func__, decimal_part, tmp_decimal_part); */
    /* while (tmp_decimal_part != 0) { */
    /*     tmp_decimal_part /= 10; */
    /*     (*decimal_digits)++; */
    /* } */

    /* Count the number of digits in the decimal part */
    /*格式 "7.200000e-002" 因为精度问题,直接采用底层科学计算打印,来获取小数的位数*/
    char num_str[14] = {0};
    snprintf(num_str, sizeof(num_str), "%e", decimal_part);
    *decimal_digits = atoi(&num_str[10]);

    int index = strchr(num_str, '.') - num_str;
    int i = 0;
    for (i = 6; i > 0; --i) {
        if (!(num_str[index + i] == '0')) {
            break;
        }
    }
    *decimal_digits += i;
}

/**
 * @brief 按下"="号按键后
 */
static void calc_equal(void)
{
    char tmp_num_str[12 + 1 + 6 + 1]; /* 整数+小数点+小数+结束符*/
    u8 integer_digits; /* 整数位数 */
    u8 decimal_digits; /* 小数位数 */
    u8 tmp_part;   /* 小数部分 */
    u8 tmp_decimal_digits;

    if (!((__this->operator >= OPERATOR_ADD) && (__this->operator <= OPERATOR_DIV))) {
        log_error("<%s> operator err", __func__);
        return;
    }

    /* 进行计算 */
    __this->operand2 = calc_atof(__this->num_show_str);
    __this->operand1 = math_operate(__this->operand1, __this->operand2, __this->operator);
    __this->operand2 = NAN;
    __this->operator_input = OPERATOR_NONE;
    __this->operator = OPERATOR_NONE;
    __this->operand_float_flag = 0;
    memset(__this->num_show_str, 0, sizeof(__this->num_show_str));

    if (isnan(__this->operand1)) {
        __this->err_code = ERR_CALCULATE;
        ui_hide(CAL_RESULT);
        ui_show(CAL_ERROR_TEXT);
        return;
    }

    /* 将计算结果进行刷新显示 */
    if ((__this->operand1 - (long long)__this->operand1) == 0.0) {
        decimal_digits = 0;
    } else {
        decimal_digits = 6; /*默认显示后小数点后六位*/
    }

    log_debug("<%s> decimal_digits:%d", __func__, decimal_digits);
    if (decimal_digits == 0) {
        /*无小数显示处理*/

        /*不使用%d, %d的位数范围小于NUM_SHOW_MAX*/
        snprintf(tmp_num_str, sizeof(tmp_num_str), "%f", __this->operand1);
        integer_digits = strchr(tmp_num_str, '.') - tmp_num_str;
        if (integer_digits > NUM_SHOW_MAX) {
            integer_digits = NUM_SHOW_MAX;
        }
        memcpy(__this->num_show_str, tmp_num_str, integer_digits);
    } else {
        /*有小数显示处理*/
        int integer_part_digits = 0;
        int decimal_part_digits = 0;
        calc_get_integer_decimal_digits(__this->operand1, &integer_part_digits, &decimal_part_digits);
        if (__this->operand1 < 0.0) {
            ++integer_part_digits;
        }
        log_debug("operand1:%e, integer_part_digits:%d, decimal_part_digits:%d", __this->operand1, integer_part_digits, decimal_part_digits);
        if ((decimal_part_digits <= decimal_digits) && (integer_part_digits + 1 + decimal_part_digits) <= NUM_SHOW_MAX) {
            /* 数值个数小于NUM_SHOW_MAX 直接进行显示 */
            snprintf(__this->num_show_str, sizeof(__this->num_show_str), "%f", __this->operand1);
            tmp_part = strchr(__this->num_show_str, '.') - __this->num_show_str;
            /*清除多余的0,比如"0.072000"变为"0.072"*/
            for (int i = decimal_digits; i > 0; --i) {
                if (__this->num_show_str[tmp_part + i] == '0') {
                    __this->num_show_str[tmp_part + i] = 0;
                } else {
                    break;
                }
            }
        } else {
            /* 数值个数小于NUM_SHOW_MAX 转为科学计数法 再显示 */
            snprintf(tmp_num_str, sizeof(tmp_num_str), "%e", __this->operand1);
            /*"7.200000e-002" 转化为 "7.2e-2"*/
            log_debug("<%s> tmp_num_str:%s", __func__, tmp_num_str);
            tmp_part = strchr(tmp_num_str, 'e') - tmp_num_str;
            int aa = atoi(&tmp_num_str[tmp_part + 2]);
            snprintf(&tmp_num_str[tmp_part + 2], 4, "%d", aa);
            if (tmp_num_str[tmp_part + 1] == '+') {
                memmove(&tmp_num_str[tmp_part + 1], &tmp_num_str[tmp_part + 2], (sizeof(tmp_num_str) - (tmp_part + 2)));
            }
            int bb;
            for (bb = 0; bb < decimal_digits;) {
                if (tmp_num_str[tmp_part - 1 - bb] == '0') {
                    ++bb;
                } else {
                    break;
                }
            }
            if (tmp_num_str[tmp_part - 1 - bb] == '.') {
                ++bb;
            }
            memmove(&tmp_num_str[tmp_part - bb], &tmp_num_str[tmp_part], (sizeof(tmp_num_str) - tmp_part));
            memcpy(__this->num_show_str, tmp_num_str, sizeof(__this->num_show_str));
        }
    }

    log_debug("<%s> num_show_str:%s", __func__, __this->num_show_str);
    calc_result_number_show_update(__this->num_show_str);

    memset(__this->num_show_str, 0, sizeof(__this->num_show_str));
}

/**
 * @brief 运算数输入,记录到buf, 然后刷新显示
 *
 * @param str 计算器按下输入对应的字符
 */
static void calc_operand_input(char num)
{
    log_info("<%s> num:%c", __func__, num);
    u8 len;

    if (!((num >= '0' && num <= '9') || (num == '.'))) {
        log_error("<%s> input err", __func__);
        return;
    }


    log_debug("<%s> __this->operator_input:%d, __this->operator:%d", __func__, __this->operator_input, __this->operator);
    if (__this->operator_input != OPERATOR_NONE && __this->operator == OPERATOR_NONE) {
        /* 说明已有operand1 operator; 是在输入operand2,更新operator,刷新显示 */
        __this->operator = __this->operator_input;
        __this->operand_float_flag = 0;
        memset(__this->num_show_str, 0, sizeof(__this->num_show_str));
    }

    /*上次计算结果错误，再输入时候，进行显示刷新*/
    if (__this->err_code == ERR_CALCULATE) {
        __this->err_code = ERR_NONE;
        ui_hide(CAL_ERROR_TEXT);
        ui_show(CAL_RESULT);
    }

    len = strlen(__this->num_show_str);
    if (len >= NUM_SHOW_MAX) {
        log_error("<%s> len >= NUM_SHOW_MAX", __func__);
        return;
    }

    /*记录数据处理*/
    if (num == '0') {
        if (__this->num_show_str[0] == '0' && len == 1) {
            return;
        }
        __this->num_show_str[len] = num;
    } else if (num == '.') {
        if (__this->operand_float_flag == 1) {
            return;
        } else {
            if (len == 0) {
                __this->num_show_str[0] = '0';
                __this->num_show_str[1] = '.';
                __this->operand_float_flag = 1;
            } else {
                __this->num_show_str[len] = num;
                __this->operand_float_flag = 1;
            }
        }
    } else {
        if (__this->num_show_str[0] == '0' && __this->num_show_str[1] != '.') {
            __this->num_show_str[0] = num;
        } else {
            __this->num_show_str[len] = num;
        }
    }

    log_debug("<%s> num_show_str:%s", __func__, __this->num_show_str);
    calc_result_number_show_update(__this->num_show_str);
}

/**
 * @brief  operator输入处理
 *
 * @param  operator "+ - * / "
 */
static void calc_operator_input(u8 operator)
{
    u8 len;
    log_debug("<%s> operator:%d", __func__, operator);
    if (!((operator >= OPERATOR_ADD) && (operator <= OPERATOR_DIV))) {
        log_error("<%s> operator err", __func__);
        return;
    }

    if (isnan(__this->operand1) && isnan(__this->operand2)) {
        /* operand1无效 operand2无效 */
        __this->operator_input = operator;
        __this->operand1 = calc_atof(__this->num_show_str);
    } else if (!isnan(__this->operand1) && __this->operator_input == OPERATOR_NONE) {
        /* operand1有效 operator_input无效*/
        len = strlen(__this->num_show_str);
        if (len == 0) {
            /* 当前输入值无效, 记录operator, 作下次计算使用 */
            __this->operator_input = operator;
        } else {
            /* 当前输入值有效, 重新记录opreand1 开启新的计算*/
            __this->operand1 = calc_atof(__this->num_show_str);
            __this->operator_input = operator;
        }
    } else if (!isnan(__this->operand1) && __this->operator_input != OPERATOR_NONE) {
        /* operand1有效 operator_input有效 */
        if (__this->operator == OPERATOR_NONE) {
            /* 当前operator无效, 说明operand2未输入, 更新operator, 作下次计算使用 */
            __this->operator_input = operator;
        } else {
            /* 当前operator有效, 执行calc_equal(), 更新operator, */
            calc_equal();
            __this->operator_input = operator;
        }
    }

    log_debug("<%s> __this->operand1:%e", __func__, __this->operand1);
}

/**
 * @brief AC按键处理,恢复默认状态
 */
static void calc_ac(void)
{
    log_debug("<%s>", __func__);
    if (__this->err_code != ERR_NONE) {
        ui_hide(CAL_ERROR_TEXT);
        ui_show(CAL_RESULT);
    }

    __this->operand1 = NAN;
    __this->operand2 = NAN;
    __this->operator = OPERATOR_NONE;
    __this->operator_input = OPERATOR_NONE;
    __this->err_code = ERR_NONE;
    __this->operand_float_flag = 0;
    memset(__this->num_show_str, 0, sizeof(__this->num_show_str));
    __this->num_show_str[0] = '0';
    calc_result_number_show_update(__this->num_show_str);
}

/**
 * @brief del按键处理
 */
static void calc_del(void)
{
    u8 len;

    len = strlen(__this->num_show_str);
    if (__this->operator_input == OPERATOR_NONE) {
        if (len == 0) {
            /*该情况是按下"="按键 计算器默认状态, 直接再恢复默认状态就好*/
            calc_ac();
        } else if (len == 1) {
            if (__this->num_show_str[0] == '0') {
                return;
            } else {
                __this->num_show_str[len - 1] = 0;
            }
        } else {
            __this->num_show_str[len - 1] = 0;
        }
    } else {
        __this->operator_input = OPERATOR_NONE;
    }

    log_debug("<%s> num_show_str:%s", __func__, __this->num_show_str);
    calc_result_number_show_update(__this->num_show_str);
}

static void calc_init(void)
{
    __this = zalloc(sizeof(calculator_handle_t));
    if (!__this) {
        log_info("<%s> zalloc fail", __func__);
        return;
    }
    __this->operand1 = NAN;
    __this->operand2 = NAN;
    __this->num_show_str[0] = '0';  /*默认显示"0"*/
}

static void calc_deinit(void)
{
    if (__this) {
        free(__this);
        __this = NULL;
    }
}


#endif /* if TCFG_UI_ENABLE_CALCULATOR */
#endif /* #if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) */

