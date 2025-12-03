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
#define LOG_TAG     		"[UI_BLOODPRESS]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_bloodpress.data.bss")
#pragma data_seg(".ui_action_bloodpress.data")
#pragma const_seg(".ui_action_bloodpress.text.const")
#pragma code_seg(".ui_action_bloodpress.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_BLOODPRESS

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)

int ui_core_pagemove_flag();
typedef struct {
    u32 bloodpress_systolic;	//收缩压
    u32 bloodpress_diastolic;	//舒张压
    u32 systolic_new[7]; 		//最近七次的收缩压
    u32 diastolic_new[7];		//最近七次的舒张压
    u32 systolic_week[7];	//一周
    u32 diastolic_week[7];
    u32 bloodpress_24h[24];
    u16 test_timer;
} BLOODPRESS_UI_PARAM;

static BLOODPRESS_UI_PARAM *bloodpress_ui_handler = NULL;

#define __this		bloodpress_ui_handler
extern int gpu_fill_rect(struct draw_context *dc, int left, int top, int width, int height, u32 acolor);

static void draw_barchart_bar(struct draw_context *dc, u32 id, u32 data[], u32 data_num, u32 max, u32 min, int color)
{
    /* u32 data_num; */
    struct element *elm;
    struct rect rect;
    u32 draw_x, draw_y, draw_width, draw_height;
    u32 interval;	//柱状图之间的间隔
    u32 percent;

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
            ui_draw_bar(dc, rect.left + draw_x, rect.top + draw_y, draw_width, draw_height, color, 100, (i != (data_num - 1)));
        }
    }
}


static void bloodpress_rectbar_draw(struct draw_context *dc, int x, int y, int width, int height, int color, int percent, int backcolor)
{
    int bar_x, bar_y, bar_width, bar_height;
    int back_x, back_y, back_width, back_height;

    bar_x = x;
    bar_height = height * percent / 100;
    bar_y = y + (height - bar_height);
    bar_width = width;
    /* printf("bar:%d * %d * %d * %d\n", bar_x, bar_y, bar_width, bar_height); */
    if (bar_height != 0) {
        gpu_fill_rect(dc, bar_x, bar_y, bar_width, bar_height, 100 << 24 | color);
    }

    back_x = x;
    back_y = y;
    back_width = width;
    back_height = height - bar_height;
    /* printf("back:%d * %d * %d * %d\n", back_x, back_y, back_width, back_height); */
    if (back_height != 0) {
        gpu_fill_rect(dc, back_x, back_y, back_width, back_height, 100 << 24 | backcolor);
    }
}

static void bloodpress_roundbar_draw(struct draw_context *dc, int x, int y, int width, int height, int color, int percent, int backcolor, int wait_sync)
{
    int bar_x, bar_y, bar_width, bar_height;
    int back_x, back_y, back_width, back_height;

    bar_x = x;
    bar_height = height * percent / 100;
    bar_y = y + (height - bar_height);
    bar_width = width;
    /* printf("bar:%d * %d * %d * %d\n", bar_x, bar_y, bar_width, bar_height); */
    if (bar_height != 0) {
        ui_draw_bar(dc, bar_x, bar_y, bar_width, bar_height, color, 100, wait_sync);
    }
#if 0
    back_x = x;
    back_y = y;
    back_width = width;
    back_height = height - bar_height;
    /* printf("back:%d * %d * %d * %d\n", back_x, back_y, back_width, back_height); */
    if (back_height != 0) {
        ui_draw_bar(dc, back_x, back_y, back_width, back_height, backcolor, 100, wait_sync);
    }
#endif
}

//绘制最近七次的血压数据
static void bloodpress_bar_chart_draw(struct draw_context *dc, u32 id, u32 data_sysyt[7], u32 data_dias[7], u32 max, u32 min)
{
    struct element *elm;
    struct rect rect;
    u32 draw_x, draw_y, draw_width, draw_height;
    u32 interval;	//柱状图之间的间隔
    u32 percent;
    u32 data_num = 7;

    elm = ui_core_get_element_by_id(id);
    ui_core_get_element_abs_rect(elm, &rect);
    interval = (float)rect.width / (data_num * 8 - 5);
    /* printf("rect:%d * %d\n", rect.width, rect.height); */
    draw_width = (float)(rect.width - interval * (data_num + (data_num - 1) * 5)) / (data_num * 2);
    draw_y = 0;
    /* printf("interval:%d, draw_width:%d\n", interval, draw_width); */

    for (int i = 0; i < data_num; i++) {
        //绘制收缩压
        if (data_sysyt[i] <  min) {
            percent = 0;
        } else {
            percent = (data_sysyt[i] - min) * 100 / (max - min);
        }
        draw_x = i * (draw_width * 2 + interval * 6);
        /* printf("hkzdebug:draw:%d, %d, %d, %d\n", draw_width, draw_height, draw_x, draw_y); */

        bloodpress_rectbar_draw(dc, rect.left + draw_x, rect.top + draw_y, draw_width, rect.height, RGB565(254, 30, 122), percent, RGB565(67, 67, 67));

        //绘制舒张压
        if (data_dias[i] <  min) {
            percent = 0;
        } else {
            percent = (data_dias[i] - min) * 100 / (max - min);
        }

        draw_x = draw_x + interval + draw_width;

        bloodpress_rectbar_draw(dc, rect.left + draw_x, rect.top + draw_y, draw_width, rect.height, RGB565(255, 210, 0), percent, RGB565(67, 67, 67));
    }

}

//绘制平均血压柱状图
static void bloodpress_arg_barchart_draw(struct draw_context *dc, u32 id, u32 data_sysyt[7], u32 data_dias[7], u32 max, u32 min)
{
    struct element *elm;
    struct rect rect;
    u32 draw_x, draw_y, draw_width, draw_height;
    u32 interval;	//柱状图之间的间隔
    u32 percent;
    u32 data_num = 7;

    elm = ui_core_get_element_by_id(id);
    ui_core_get_element_abs_rect(elm, &rect);
    interval = (float)rect.width / (data_num * 3 - 1);
    /* printf("rect:%d * %d\n", rect.width, rect.height); */
    draw_width = (float)(rect.width - interval * (data_num - 1)) / (data_num * 2);
    draw_y = 0;
    /* printf("interval:%d, draw_width:%d\n", interval, draw_width); */

    for (int i = 0; i < data_num; i++) {
        //绘制收缩压
        if (data_sysyt[i] <  min) {
            percent = 0;
        } else {
            percent = (data_sysyt[i] - min) * 100 / (max - min);
        }
        draw_x = i * (draw_width * 2 + interval);
        /* printf("hkzdebug:draw:%d, %d, %d, %d\n", draw_width, draw_height, draw_x, draw_y); */

        bloodpress_roundbar_draw(dc, rect.left + draw_x, rect.top + draw_y, draw_width, rect.height, RGB565(254, 30, 122), percent, RGB565(0, 0, 0), (i != (data_num - 1)));
    }
    draw_width = (float)(rect.width - interval * (data_num - 1)) / (data_num * 2);
    for (int i = 0; i < data_num; i++) {
        //绘制舒张压
        if (data_dias[i] <  min) {
            percent = 0;
        } else {
            percent = (data_dias[i] - min) * 100 / (max - min);
        }
        draw_x = i * (draw_width * 2 + interval);
        draw_x = draw_x + draw_width;

        bloodpress_roundbar_draw(dc, rect.left + draw_x, rect.top + draw_y, draw_width, rect.height, RGB565(255, 210, 0), percent, RGB565(0, 0, 0), (i != (data_num - 1)));
    }
}

static void bloodpress_parm_init(void)
{
    if (__this) {
        /* printf("size of parm:%lu\n", sizeof(BLOODPRESS_UI_PARAM)); */
        memset(__this, 0, sizeof(BLOODPRESS_UI_PARAM));
    }
}

static int bloodpress_newly_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        /* printf("BLOODPRESS_NEWLY_LAYOUT init!!!"); */
        for (int i = 0; i < 7; i++) {
            __this->systolic_new[i] = 60 + i * 10;
            __this->diastolic_new[i] = 70 + i * 10;
        }
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:

        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_SHOW_POST:
        /* printf("BLOODPRESS_NEWLY_LAYOUT SHOW POST!!!"); */


        ui_custom_draw_clear(dc);

        bloodpress_bar_chart_draw(dc, elm->id, __this->systolic_new, __this->diastolic_new, 150, 60);
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(BLOODPRESS_NEWLY_LAYOUT)
.onchange = bloodpress_newly_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int bloodpress_24h_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        /* printf("BLOODPRESS_24H_LAYOUT init!!!"); */
        for (int i = 0; i < 24; i++) {
            __this->bloodpress_24h[i] = 60 + i * 3;
        }
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:

        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_SHOW_POST:
        /* printf("BLOODPRESS_24H_LAYOUT SHOW POST!!!"); */
        ui_custom_draw_clear(dc);
        ui_core_get_element_abs_rect(elm, &rect);
        if (!ui_core_pagemove_flag()) {
            draw_barchart_bar(dc, elm->id, __this->bloodpress_24h, 24, 150, 60, RGB565(255, 167, 24));
        }
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(BLOODPRESS_24H_LAYOUT)
.onchange = bloodpress_24h_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int bloodpress_arg_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        /* printf("BLOODPRESS_ARG_LAYOUT init!!!"); */
        for (int i = 0; i < 7; i++) {
            __this->systolic_week[i] = 60 + i * 10;
            __this->diastolic_week[i] = 70 + i * 10;
        }
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_SHOW_POST:
        /* printf("BLOODPRESS_ARG_LAYOUT SHOW POST!!!"); */

        ui_custom_draw_clear(dc);

        bloodpress_arg_barchart_draw(dc, elm->id, __this->systolic_week, __this->diastolic_week, 150, 60);
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(BLOODPRESS_ARG_LAYOUT)
.onchange = bloodpress_arg_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int bloodpress_main_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct ui_grid *grid = (struct ui_grid *)elm;

    switch (event) {
    case ON_CHANGE_INIT:
        elm->css.left = 0;
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
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

REGISTER_UI_EVENT_HANDLER(BLOODPRESS_MAIN_VLIST)
.onchange = bloodpress_main_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int window_bloodpress_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        /* printf("ID_WINDOW_BLOODPRESSURE init!!!"); */
        if (!__this) {
            __this = malloc(sizeof(BLOODPRESS_UI_PARAM));
        }
        bloodpress_parm_init();
        break;
    case ON_CHANGE_RELEASE:
        /* printf("ID_WINDOW_BLOODPRESSURE release!!!"); */
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

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_BLOODPRESSURE)
.onchange = window_bloodpress_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


#endif /*if TCFG_UI_ENABLE_BLOODPRESS*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/

