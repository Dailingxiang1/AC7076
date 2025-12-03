/* Copyright(C)
 * not free
 * All right reserved
 *
 * @file ui_action_calendar.c
 * @brief 日历显示 中国公历(格里历)

         日历显示示例
    日 一 二 三 四 五 六
          01 02 03 04 05
    06 07 08 09 10 11 12
    13 14 15 16 17 18 19
    20 21 22 23 24 25 26
    27 28 29 30 31

*/
#include "app_config.h"
#include "rtc.h"
#include "ui.h"
#include "ui_api.h"
#include "jlui_app/result_pic_index.h"
#include "jlui_app/ui_style.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-CALENDAR]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_calendar.data.bss")
#pragma data_seg(".ui_action_calendar.data")
#pragma const_seg(".ui_action_calendar.text.const")
#pragma code_seg(".ui_action_calendar.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_CALENDAR

#define STYLE_NAME  JL

/**********************
 *      DEFINES
 *********************/
#define __this (p_ui_calendar_hander)

#define CALENDAR_COL 7  /*日历显示7列*/
#define CALENDAR_ROW 6  /*日历显示6行*/
#define CALENDAR_ITEM_WIDTH     38  /*日历显示每一项宽度*/
#define CALENDAR_ITEM_HEIGHT    38  /*日历显示每一项高度*/
#define CALENDAR_ITEM_INTERVAL  1   /*日历显示每一项间距*/

/**********************
 *      TYPEDEFS
 *********************/
struct ui_calendar {
    int year;       /*计算使用*/
    int month;      /*计算使用*/
    int day;        /*计算使用*/
    int week;       /*计算使用*/
    int cur_year;   /*当前日期*/
    int cur_month;  /*当前日期*/
    int cur_day;    /*当前日期*/
    struct image_file num_pic_file; /*数字图片信息*/
    u32 num_pic_id_array[10];   /*自定义绘图所需的图片id*/
    u32 today_pic_id;           /*自定义绘图所需的图片id*/
};

/**********************
 * GLOBAL PROTOTYPES
 *********************/
extern int cal_days(int year, int month);

/**********************
 * PROTOTYPES
 *********************/
static void calendar_get_time_by_index(int *year, int *month, int *day, int index);
static void calendar_init(void);
static void calendar_month_prev(void);
static void calendar_month_next(void);
static void calendar_draw(struct draw_context *dc);

extern struct ui_image_list *ui_pic_get_normal_image_list(struct ui_pic *pic);

/**********************
 * STATIC VARIABLES
 *********************/
static struct ui_calendar *p_ui_calendar_hander;



/************************************************
 *              日历ui显示流程
 ***********************************************/

static int ui_calendar_show_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        __this = zalloc(sizeof(struct ui_calendar));
        if (!__this) {
            log_error("<%s> zalloc fail", __func__);
            break;
        }

        struct ui_image_list *image_info;
        u32 pic_id;
        image_info = ui_pic_get_normal_image_list((struct ui_pic *)ui_core_get_element_by_id(CALENDAR_PIC));
        /*UI工程中图片控件的图片列表所放置的图片*/
        if (image_info->num != 11) {
            log_error("<%s> pic iamge num %d error", __func__, image_info->num);
        }
        pic_id = (layout->elm.page << 16) | (image_info->image[0] & 0xffff);
        __this->today_pic_id = pic_id;
        for (int i = 1; i < image_info->num; i++) {
            pic_id = (layout->elm.page << 16) | (image_info->image[i] & 0xffff);
            log_debug("i:%d,pic_id:%x", i, pic_id);
            __this->num_pic_id_array[i - 1] = pic_id;
        }

        calendar_init();
        open_image_by_id(0, NULL, &__this->num_pic_file, __this->num_pic_id_array[0] & 0xffff, __this->num_pic_id_array[0] >> 16);
        break;
    case ON_CHANGE_SHOW_POST:
        ui_custom_draw_clear((struct draw_context *)arg);
        calendar_draw((struct draw_context *)arg);
        break;
    case ON_CHANGE_RELEASE:
        if (__this) {
            free(__this);
            __this = NULL;
        }
        break;
    default:
        return false;
    }
    return false;
}

static int ui_calendar_show_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct element *elm_window;
    struct layout *layout = (struct layout *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        calendar_month_next();
        elm_window = ui_core_get_element_by_id(ID_WINDOW_CALENDAR);
        ui_core_redraw(elm_window);
        return true;
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        calendar_month_prev();
        elm_window = ui_core_get_element_by_id(ID_WINDOW_CALENDAR);
        ui_core_redraw(elm_window);
        return true;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(CALENDAR_SHOW_LAYOUT)
.onchange = ui_calendar_show_layout_onchange,
 .onkey = NULL,
  .ontouch = ui_calendar_show_layout_ontouch,
};

static void calendar_draw(struct draw_context *dc)
{
    struct rect rect;
    int year, month, day;
    int x, y;
    int col, row;
    int tens_digit, ones_digit;
    int tens_digit_pic_width, ones_digit_pic_width;

    ui_core_get_element_abs_rect(dc->elm, &rect);

    for (int  i = 0; i < (CALENDAR_COL * CALENDAR_ROW); i++) {
        year = __this->year;
        month = __this->month;
        day = __this->day;
        calendar_get_time_by_index(&year, &month, &day, i);
        log_debug("index:%d date:%04d-%02d-%02d", i, year, month, day);

        col = i % CALENDAR_COL;
        row = i / CALENDAR_COL;
        /*以所在布局为基点，算后面的偏移*/
        x = rect.left;
        y = rect.top;

        if ((day >= 0) && (day <= 9) && (month == __this->month)) {

            x += (CALENDAR_ITEM_WIDTH + CALENDAR_ITEM_INTERVAL) * col;
            y += (CALENDAR_ITEM_HEIGHT + CALENDAR_ITEM_INTERVAL) * row;
            if ((day == __this->cur_day) && (month == __this->cur_month) && (year == __this->cur_year)) {
                ui_draw_image(dc, __this->today_pic_id >> 16, __this->today_pic_id & 0xffff, x, y);
            }
            x += (CALENDAR_ITEM_WIDTH - __this->num_pic_file.width) / 2;
            y += (CALENDAR_ITEM_HEIGHT - __this->num_pic_file.height) / 2;
            log_debug("day:(%d %d)", x, y);
            ui_draw_image(dc, __this->num_pic_id_array[day] >> 16, __this->num_pic_id_array[day] & 0xffff, x, y);

        } else if ((day >= 10) && (day <= 31) && (month == __this->month)) {

            tens_digit = day / 10;
            ones_digit = day % 10;

            tens_digit_pic_width = __this->num_pic_file.width;
            ones_digit_pic_width = __this->num_pic_file.width;
            y += (CALENDAR_ITEM_HEIGHT + CALENDAR_ITEM_INTERVAL) * row;
            x += (CALENDAR_ITEM_WIDTH + CALENDAR_ITEM_INTERVAL) * col;
            if ((day == __this->cur_day) && (month == __this->cur_month && (year == __this->cur_year))) {
                ui_draw_image(dc, __this->today_pic_id >> 16, __this->today_pic_id & 0xffff, x, y);
            }
            y += (CALENDAR_ITEM_HEIGHT - __this->num_pic_file.height) / 2;
            x += (CALENDAR_ITEM_WIDTH - (tens_digit_pic_width + ones_digit_pic_width)) / 2;
            log_debug("tens_width:(%d, %d)", x, y);
            ui_draw_image(dc, __this->num_pic_id_array[tens_digit] >> 16, __this->num_pic_id_array[tens_digit] & 0xffff, x, y);

            x += tens_digit_pic_width;
            log_debug("ones_width:(%d, %d)", x, y);
            ui_draw_image(dc, __this->num_pic_id_array[ones_digit] >> 16, __this->num_pic_id_array[ones_digit] & 0xffff, x, y);

        }
    }
}

static int ui_calendar_date_number_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_number *number = (struct ui_number *)ctr;
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        struct unumber num;
        num.type = TYPE_NUM;
        num.numbs = 2;
        num.number[0] = __this->month;
        num.number[1] = __this->year;
        ui_number_update(number, &num);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(CALENDAR_DATE_NUMBER)
.onchange = ui_calendar_date_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_calendar_prev_next_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    struct element *elm_window;
    struct element *elm_grid;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag == 0) {
            switch (pic->elm.id) {
            case CALENDAR_PREV_PIC:
                calendar_month_prev();
                elm_window = ui_core_get_element_by_id(ID_WINDOW_CALENDAR);
                ui_core_redraw(elm_window);
                break;
            case CALENDAR_NEXT_PIC:
                calendar_month_next();
                elm_window = ui_core_get_element_by_id(ID_WINDOW_CALENDAR);
                ui_core_redraw(elm_window);
                break;
            default:
                break;

            }
        }
        break;
    default:
        return false;
        break;
    }
    return true;//接管消息
}
REGISTER_UI_EVENT_HANDLER(CALENDAR_NEXT_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calendar_prev_next_pic_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CALENDAR_PREV_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_calendar_prev_next_pic_ontouch,
};



/************************************************
 *              日期相关API
 ***********************************************/

/* 蔡勒公式:日期转星期 */
static int zeller_week(int year, int month, int day)
{
    int m = month;
    int d = day;

    if (month <= 2) { /*对小于2的月份进行修正*/
        year--;
        m = month + 12;
    }

    int y = year % 100;
    int c = year / 100;

    int w = (y + y / 4 + c / 4 - 2 * c + (13 * (m + 1) / 5) + d - 1) % 7;
    if (w < 0) { /*修正计算结果是负数的情况*/
        w += 7;
    }

    return w;
}

/* check if a given year is a leap year */
static int is_leap_year(int year)
{
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return 1; // Leap year
    } else {
        return 0; // Not a leap year
    }
}

/* get the number of days in a given month of a year */
static int days_in_month(int month, int year)
{
    if (month == 2) {
        return is_leap_year(year) ? 29 : 28;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    } else {
        return 31;
    }
}

/* adjust a date by a certain number of days */
static void adjust_date_by_days(int *year, int *month, int *day, int diffdays)
{
    while (diffdays != 0) {
        if (diffdays > 0) {
            // Increment the date
            int daysInMonthCurrent = days_in_month(*month, *year);
            if (*day + diffdays <= daysInMonthCurrent) {
                *day += diffdays;
                diffdays = 0;
            } else {
                diffdays -= (daysInMonthCurrent - *day + 1);
                *day = 1;
                if (*month == 12) {
                    *month = 1;
                    (*year)++;
                } else {
                    (*month)++;
                }
            }
        } else {
            // Decrement the date
            diffdays++;
            if (*day == 1) {
                if (*month == 1) {
                    (*year)--;
                    *month = 12;
                    *day = 31;
                } else {
                    (*month)--;
                    *day = days_in_month(*month, *year);
                }
            } else {
                (*day)--;
            }
        }
    }
}



/* ------------------------------------------------------------------------------------*/
/**
 * @brief 计算某日期按index对应的年月日
 *
 * @Params year
 * @Params month
 * @Params day
 * @Params index 比如日历显示7*6表格，index对应每个项的序号，从0开始
 */
/* ------------------------------------------------------------------------------------*/
static void calendar_get_time_by_index(int *year, int *month, int *day, int index)
{
    if (!__this) {
        return;
    }
    adjust_date_by_days(year, month, day, (index - __this->week));
}

static void calendar_init(void)
{
    struct sys_time cur_time;
    rtc_read_time(&cur_time);
    __this->year = cur_time.year;
    __this->month = cur_time.month;
    __this->day = 1; /*日历以1号开始显示*/
    __this->cur_year = cur_time.year;
    __this->cur_month = cur_time.month;
    __this->cur_day = cur_time.day;
    int week = zeller_week(__this->year, __this->month, __this->day);
    __this->week = week;
    log_info("<%s> %04d-%02d-%02d %d", __func__, __this->year, __this->month, __this->day, week);
}

static void calendar_month_prev(void)
{
    if ((__this->month - 1) < 1) {
        __this->month = 12;
        --__this->year;
    } else {
        __this->month -= 1;
    }
    int week = zeller_week(__this->year, __this->month, __this->day);
    __this->week = week;
    log_info("<%s> %04d-%02d-%02d %d", __func__, __this->year, __this->month, __this->day, week);
}

static void calendar_month_next(void)
{
    if ((__this->month + 1) > 12) {
        __this->month = 1;
        ++__this->year;
    } else {
        __this->month += 1;
    }
    int week = zeller_week(__this->year, __this->month, __this->day);
    __this->week = week;
    log_info("<%s> %04d-%02d-%02d %d", __func__, __this->year, __this->month, __this->day, week);
}

#endif /* if TCFG_UI_ENABLE_CALENDAR */
#endif /* #if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) */
