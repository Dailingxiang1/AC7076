#include "app_config.h"
/* #include "app_task.h" */
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
#include "ui_draw/ui_type.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_SLEEP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_sleep.data.bss")
#pragma data_seg(".ui_action_sleep.data")
#pragma const_seg(".ui_action_sleep.text.const")
#pragma code_seg(".ui_action_sleep.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_SLEEP

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)

typedef struct {
    u32 sleep_time;	//睡眠时间
    u32 deepsleep_time;	//深度睡眠时间
    u32 lightsleep_time;	//浅度睡眠时间
    u32 sleep_day[24]; 		//全天睡眠数据
    u32 sleep_avg_week[7];	//一周的平均睡眠数据
    u16 test_timer;
} SLEEP_UI_PARAM;

static SLEEP_UI_PARAM *sleep_ui_handler = NULL;

#define __this		sleep_ui_handler

/********************************************************************************************/
//设置一天24h每个小时的睡眠情况。0：无睡眠；1：浅度睡眠；2：深度睡眠
void sleep_day_set(u32 index, u32 data)
{
    if (index < 24) {
        __this->sleep_day[index] = data;
    }
}

u32 sleep_day_get(u32 index)
{
    if (index < 24) {
        return __this->sleep_day[index];
    }

    return 0;
}

void sleep_avg_week_set(u32 index, u32 data)
{
    if (index < 7) {
        __this->sleep_avg_week[index] = data;
    }
}

u32 sleep_avg_week_get(u32 index)
{
    if (index < 7) {
        return __this->sleep_avg_week[index];
    }

    return 0;
}

/********************************************************************************************/

extern int gpu_fill_rect(struct draw_context *dc, int left, int top, int width, int height, u32 acolor);

static void draw_barchart_bar(struct draw_context *dc, u32 id, u32 data[], u32 data_num, u32 max, u32 min, int color)
{
    /* u32 data_num; */
    struct element *elm;
    struct rect rect;
    u32 draw_x, draw_y, draw_width, draw_height;
    u32 interval;	//柱状图之间的间隔
    u32 percent;

    /* data_num = sizeof(data) / sizeof(data[0]); */
    /* printf("hkzdebug:data_num:%d, %d\n", data_num, sizeof(data)); */
    elm = ui_core_get_element_by_id(id);
    ui_core_get_element_abs_rect(elm, &rect);
    interval = (float)rect.width / (data_num * 2 - 1);
    printf("rect:%d * %d\n", rect.width, rect.height);
    draw_width = (float)(rect.width - interval * (data_num - 1)) / data_num;

    /* u32 rest = rect.width - draw_width * data_num - interval * data_num; */

    for (int i = 0; i < data_num; i++) {
        /* draw_width = (float)(rect.width - interval * (data_num - 1)) / data_num; */
        if (data[i] <  min) {
            draw_height = 0;
        } else {
            draw_height = (float)rect.height / (max - min) * (data[i] - min);
        }
        draw_x = i * (draw_width + interval);
        draw_y = rect.height - draw_height;
        percent = draw_height * 100 / rect.height;
        /* printf("percent:%d\n", percent); */
        /* printf("hkzdebug:draw:%d, %d, %d, %d\n", draw_width, draw_height, draw_x, draw_y); */

        if (!(0 == draw_height || 0 == draw_width)) {
            ui_draw_bar(dc, rect.left + draw_x, rect.top + draw_y, draw_width, draw_height, color, 100, (i != (data_num - 1)));
        }
    }
}

//绘制全天睡眠数据图
static void draw_sleep_oneday(struct draw_context *dc, u32 id, u32 sleep_oneday[24], u32 data_num)
{
    struct element *elm;
    struct rect rect;
    u32 draw_x, draw_y, draw_width, draw_height;
    u32 acolor;
    u32 interval;	//柱状图之间的间隔

    elm = ui_core_get_element_by_id(id);
    ui_core_get_element_abs_rect(elm, &rect);
    interval = 1;

    printf("rect:%d * %d\n", rect.width, rect.height);

    draw_width = (float)(rect.width - interval * (data_num - 1)) / data_num;
    draw_y = 0;
    for (int i = 0; i < data_num; i++) {
        draw_x = i * (draw_width + interval);
        if (sleep_oneday[i] == 0) {
            acolor = RGB565(67, 67, 67);
        } else if (sleep_oneday[i] == 1) {
            acolor = RGB565(0, 137, 233);
        } else {
            acolor = RGB565(0, 78, 255);
        }
        gpu_fill_rect(dc, rect.left + draw_x, rect.top + draw_y, draw_width, rect.height, 100 << 24 | acolor);
    }

}


//绘制一周的平均睡眠柱状图
static void draw_sleep_week(struct draw_context *dc, u32 id, u32 data[], u32 data_num, u32 max, u32 min, u32 acolor)
{
    /* u32 data_num; */
    struct element *elm;
    struct rect rect;
    u32 draw_x, draw_y, draw_width, draw_height;
    u32 interval;	//柱状图之间的间隔

    /* data_num = sizeof(data) / sizeof(data[0]); */
    /* printf("hkzdebug:data_num:%d, %d\n", data_num, sizeof(data)); */
    elm = ui_core_get_element_by_id(id);
    ui_core_get_element_abs_rect(elm, &rect);
    interval = (float)rect.width / (data_num * 2 - 1);
    printf("rect:%d * %d\n", rect.width, rect.height);
    draw_width = (float)(rect.width - interval * (data_num - 1)) / data_num;

    /* u32 rest = rect.width - draw_width * data_num - interval * data_num; */

    for (int i = 0; i < data_num; i++) {
        /* draw_width = (float)(rect.width - interval * (data_num - 1)) / data_num; */
        if (data[i] <  min) {
            draw_height = 0;
        } else {
            draw_height = (float)rect.height / (max - min) * (data[i] - min);
        }
        draw_x = i * (draw_width + interval);
        draw_y = rect.height - draw_height;
        /* printf("hkzdebug:draw:%d, %d, %d, %d\n", draw_width, draw_height, draw_x, draw_y); */

        if (!(0 == draw_height || 0 == draw_width)) {
            /* gpu_fill_rect(dc, rect.left + draw_x, rect.top + draw_y, draw_width, draw_height, 100 <<24 | acolor); */

            ui_draw_bar(dc, rect.left + draw_x, rect.top + draw_y, draw_width, draw_height, acolor, 100, (i != (data_num - 1)));
        }
    }
}

static void sleep_parm_init(void)
{
    if (__this) {
        printf("size of parm:%lu\n", sizeof(SLEEP_UI_PARAM));
        memset(__this, 0, sizeof(SLEEP_UI_PARAM));
    }
}

static int sleep_oneday_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    case ON_CHANGE_SHOW_POST:
        printf("SLEEP_ONEDAY_LAYOUT show post!!!");
        /* ui_core_get_element_abs_rect(elm, &rect); */

        ui_custom_draw_clear(dc);

        draw_sleep_oneday(dc, elm->id, __this->sleep_day, 24);
        /* gpu_fill_rect(dc, 0, 0, 10, 320, 100 << 24 | 0xf800); */
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SLEEP_ONEDAY_LAYOUT)
.onchange = sleep_oneday_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int sleep_avgweek_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        printf("SLEEP_AVGWEEK_LAYOUT show!!!");
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    case ON_CHANGE_SHOW_POST:
        printf("SLEEP_AVGWEEK_LAYOUT show post!!!");
        ui_core_get_element_abs_rect(elm, &rect);

        ui_custom_draw_clear(dc);

        /* ui_draw_barchart(dc, elm->id, __this->avghr_week, 7, 150, 60); */
        draw_sleep_week(dc, elm->id, __this->sleep_avg_week, 7, 12, 0, RGB565(253, 30, 122));
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SLEEP_AVGWEEK_LAYOUT)
.onchange = sleep_avgweek_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int sleep_main_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct ui_grid *grid = (struct ui_grid *)elm;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        /* ui_grid_flick_ctrl_close(grid, 1); */
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SLEEP_MAIN_VLIST)
.onchange = sleep_main_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int window_sleep_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        printf("ID_WINDOW_SLEEP init!!!");
        if (!__this) {
            __this = malloc(sizeof(SLEEP_UI_PARAM));
        }
        sleep_parm_init();
        break;
    case ON_CHANGE_RELEASE:
        printf("ID_WINDOW_SLEEP release!!!");
        if (__this) {
            if (__this->test_timer) {
                sys_timer_del(__this->test_timer);
                __this->test_timer = 0;
            }
            free(__this);
            __this = NULL;
        }
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_SLEEP)
.onchange = window_sleep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/********************************************test*****************************************************/
static void test_timer_handler(void *priv)
{
    struct sys_time s_time;
    struct utime u_time;
    u32 data;

    ui_core_redraw(ui_core_get_element_by_id(SLEEP_ONEDAY_LAYOUT));
    ui_core_redraw(ui_core_get_element_by_id(SLEEP_AVGWEEK_LAYOUT));
    /* ui_show(SLEEP_ONEDAY_LAYOUT); */
    /* ui_show(SLEEP_AVGWEEK_LAYOUT); */

    for (int i = 0; i < 24; i++) {
        data = (sleep_day_get(i) + 1) % 3;
        sleep_day_set(i, data);
    }

    for (int i = 0; i < 7; i++) {
        data = (sleep_avg_week_get(i) + 1) % 13;
        sleep_avg_week_set(i, data);
    }
}

static int sleep_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct draw_context *dc = (struct draw_context *)arg;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        if (__this->test_timer == 0) {
            __this->test_timer = sys_timer_add(NULL, test_timer_handler, 1000);
        }
        break;
    case ON_CHANGE_SHOW_POST:
        printf("HEARTRATE_LAYOUT show post!!!");
        /* ui_draw_bar(dc, 0, 0, 20, 300, 0xf800, 100); */
        break;
    case ON_CHANGE_RELEASE:
        printf("HEARTRATE_LAYOUT relaese!!!");

        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SLEEP_LAYOUT)
.onchange = sleep_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/*****************************************************************************************************/










#endif /*if TCFG_UI_ENABLE_SLEEP*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/

