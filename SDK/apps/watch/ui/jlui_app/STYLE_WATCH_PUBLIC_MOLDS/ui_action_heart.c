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
#include "health_manager/health_manager.h"



#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_HEART]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_heart.data.bss")
#pragma data_seg(".ui_action_heart.data")
#pragma const_seg(".ui_action_heart.text.const")
#pragma code_seg(".ui_action_heart.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_HEART

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)


typedef struct {
    u32 heart_rate;
    u32 heart_rate_24h[24];
    u32 avghr;		//平均心率
    u32 avghr_week[7];
    u16 test_timer;
} HEARTRATE_UI_PARAM;

static HEARTRATE_UI_PARAM *heart_rate_ui_handler = NULL;

#define __this		heart_rate_ui_handler

extern int gpu_fill_rect(struct draw_context *dc, int left, int top, int width, int height, u32 acolor);

/********************************************************外部可用接口********************************************************************/
void heartrate_heart_rate_set(u32 heartrate)
{
    struct unumber numb;

    __this->heart_rate = heartrate;
}

void heartrate_avghr_set(u32 avghr)
{
    struct unumber numb;

    __this->avghr = avghr;
}

u32 heartrate_heart_rate_get()
{
    return __this->heart_rate;
}

u32 heartrate_avghr_get()
{
    return __this->avghr;
}

void heartrate_heart_rate_24h_set(u32 index, u32 rate)
{
    if (index < 24) {
        __this->heart_rate_24h[index] = rate;
    }
}

u32 heartrate_heart_rate_24h_get(u32 index)
{
    if (index < 24) {
        return __this->heart_rate_24h[index];
    }

    return 0;
}

void heartrate_avghr_week_set(u32 index, u32 rate)
{
    if (index < 7) {
        __this->avghr_week[index] = rate;
    }
}

u32 heartrate_avghr_week_get(u32 index)
{
    if (index < 7) {
        return __this->avghr_week[index];
    }

    return 0;
}

/****************************************************************************************************************************************/

/**
 * @file ui_action_heat.c
 * @brief 获取列表指定子控件的id
 * @author hukaize@zh-jieli.com
 * @version
 * @date 2024-05-13
 */
static u32 ui_list_get_child_id(u32 list_id, u32 index)
{
    struct ui_grid *grid;
    struct element *elm;

    grid = (struct ui_grid *)ui_core_get_element_by_id(list_id);
    elm = (struct element *)(&grid->item[index]);

    return elm->id;
}

static u32 ui_list_get_child_num(u32 list_id)
{
    struct ui_grid *grid;
    struct element *elm;
    u32 row_num;

    grid = (struct ui_grid *)ui_core_get_element_by_id(list_id);
    row_num = grid->row_num;

    return row_num;
}

static u32 ui_if_hignlight(u32 id)
{
    struct element *elm;

    elm = ui_core_get_element_by_id(id);
    return elm->highlight;
}


/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_draw_barchart 在指定控件绘制柱状图
 *
 * @Params id 控件的id
 * @Params data 绘制的数据
 */
/* ------------------------------------------------------------------------------------*/
static void ui_draw_barchart(struct draw_context *dc, u32 id, u32 data[], u32 data_num, u32 max, u32 min)
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
    /* printf("rect:%d * %d\n", rect.width, rect.height); */
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
            gpu_fill_rect(dc, rect.left + draw_x, rect.top + draw_y, draw_width, draw_height, 100 << 24 | 0xf800);
        }
    }
}

void ui_draw_barchart_bar(struct draw_context *dc, u32 id, u32 data[], u32 data_num, u32 max, u32 min, int color)
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
    /* printf("rect:%d * %d\n", rect.width, rect.height); */
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
            ui_draw_bar(dc, rect.left + draw_x, rect.top + draw_y, draw_width, draw_height, color, 100, 0);
        }
    }
}

static void heat_parm_init(void)
{
    if (__this) {
        /* printf("size of parm:%lu\n", sizeof(HEARTRATE_UI_PARAM)); */
        memset(__this, 0, sizeof(HEARTRATE_UI_PARAM));
    }
}

static int heartrate_num_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct rect rect;
    struct element_css *css;
    struct ui_number *number;


    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        number = (struct ui_number *)elm;

        numb.type = TYPE_NUM;
        numb.numbs = 1;
        numb.number[0] = __this->heart_rate;

        ui_number_update(number, &numb);
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

REGISTER_UI_EVENT_HANDLER(HEATRRATE_NUM)
.onchange = heartrate_num_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int heartrate_avg_num_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct ui_number *number;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        number = (struct ui_number *)elm;

        numb.type = TYPE_NUM;
        numb.numbs = 1;
        numb.number[0] = __this->avghr;

        ui_number_update(number, &numb);
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

REGISTER_UI_EVENT_HANDLER(HEARTRATE_AVG_NUM)
.onchange = heartrate_avg_num_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int heartrate_dayrate_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;
    struct ui_number *number;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        /* struct rect rect; */
        /* ui_core_get_element_abs_rect(elm, &rect); */
        /* struct cg_point inputPoints[] = {{0, 10}, {10, 30}, {20, 20}, {30, 50}, {40, 20}, {50, 25}, {60, 50}, {70, 30}, {80, 60}, {90, 0}, {100, 20}, {110, 30}, {120, 50}, {130, 45}, {140, 50}, {150, 40}, {160, 45}, {170, 50}, {180, 40}, {190, 30}, {200, 40}, {210, 50}, {220, 50}}; */
        /* int point_num = sizeof(inputPoints) / sizeof(inputPoints[0]); */
        /* ui_draw_curve_grad(dc, rect.left, rect.top, rect.width, rect.height, inputPoints, point_num, 0, 0xf800); */
        break;
    case ON_CHANGE_FIRST_SHOW:

        break;
    case ON_CHANGE_SHOW_POST:
        /* ui_core_get_element_abs_rect(elm, &rect); */

        ui_custom_draw_clear(dc);

        ui_draw_barchart(dc, elm->id, __this->heart_rate_24h, 24, 150, 60);
        /* gpu_fill_rect(dc, 0, 0, 10, 320, 100 << 24 | 0xf800); */
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HEARTRATE_DAYRATE_LAYOUT)
.onchange = heartrate_dayrate_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static void test_timer_handler(void *priv);
static int heartrate_weekrate_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
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
        /* printf("HEARTRATE_WEEKRATE_LAYOUT show!!!"); */
        break;
    case ON_CHANGE_FIRST_SHOW:
        /* if(__this->test_timer == 0){ */
        /* __this->test_timer = sys_timer_add(NULL, test_timer_handler, 1000); */
        /* } */

        break;
    case ON_CHANGE_SHOW_POST:
        /* printf("HEARTRATE_WEEKRATE_LAYOUT show post!!!"); */
        ui_core_get_element_abs_rect(elm, &rect);

        ui_custom_draw_clear(dc);

        ui_draw_barchart(dc, elm->id, __this->avghr_week, 7, 150, 60);
        /* ui_draw_barchart_bar(dc, elm->id, __this->avghr_week, 7, 150, 60, 0xf800); */
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(HEARTRATE_WEEKRATE_LAYOUT)
.onchange = heartrate_weekrate_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int heartrate_main_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct ui_grid *grid = (struct ui_grid *)elm;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
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

REGISTER_UI_EVENT_HANDLER(HEARTRATE_MAIN_VLIST)
.onchange = heartrate_main_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int window_heat_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        /* printf("ID_WINDOW_HAET init!!!"); */
        if (!__this) {
            __this = malloc(sizeof(HEARTRATE_UI_PARAM));
        }
        heat_parm_init();
        break;
    case ON_CHANGE_RELEASE:
        /* printf("ID_WINDOW_HAET release!!!"); */

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

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_HEART)
.onchange = window_heat_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/********************************************test*****************************************************/
static void test_timer_handler(void *priv)
{
//更新数据
    u32 rate;
    rate = sport_health_heart_rate_get_cur();
    heartrate_heart_rate_set(rate);
    rate = sport_health_heart_rate_get_avg();
    heartrate_avghr_set(rate);
    //测试心率显示
    /* rate = (heartrate_heart_rate_get() + 1) % 150; */
    /* rate = (rate < 60) ? 60 : rate; */

    /* rate = (heartrate_avghr_get() + 1) % 150; */
    /* rate = (rate < 60) ? 60 : rate; */
    /* for (int i = 0; i < 24; i++) { */
    /* heartrate_heart_rate_24h_set(i, heartrate_heart_rate_get()); */
    /* } */

    /* for (int i = 0; i < 7; i++) { */
    /* heartrate_avghr_week_set(i, heartrate_avghr_get()); */
    /* } */
//更新ui
    struct sys_time s_time;
    struct utime u_time;
    struct unumber numb;
    struct ui_number *number;

    numb.type = TYPE_NUM;
    numb.numbs = 1;
    numb.number[0] = __this->heart_rate;

    ui_number_update_by_id(HEATRRATE_NUM, &numb);

    numb.type = TYPE_NUM;
    numb.numbs = 1;
    numb.number[0] = __this->avghr;

    ui_number_update_by_id(HEARTRATE_AVG_NUM, &numb);

    ui_core_redraw(ui_core_get_element_by_id(HEARTRATE_DAYRATE_LAYOUT));
    ui_core_redraw(ui_core_get_element_by_id(HEARTRATE_WEEKRATE_LAYOUT));

}

static int window_heart_onchange(void *_ctrl, enum element_change_event event, void *arg)
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
        sport_health_heart_rate_module_enable();
        sport_health_heart_rate_module_clr();
        if (__this->test_timer == 0) {
            __this->test_timer = sys_timer_add(NULL, test_timer_handler, 2000);
        }
        break;
    case ON_CHANGE_SHOW_POST:
        /* printf("HEARTRATE_LAYOUT show post!!!"); */
        /* ui_draw_bar(dc, 0, 0, 20, 300, 0xf800, 100); */
        break;
    case ON_CHANGE_RELEASE:
        /* printf("HEARTRATE_LAYOUT relaese!!!"); */
        sport_health_heart_rate_module_disable();
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(HEARTRATE_LAYOUT)
.onchange = window_heart_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/*****************************************************************************************************/


#endif /*if TCFG_UI_ENABLE_HEART*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/

