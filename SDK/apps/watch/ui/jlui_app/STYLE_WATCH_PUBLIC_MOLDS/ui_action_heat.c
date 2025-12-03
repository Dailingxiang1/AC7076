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
#define LOG_TAG     		"[UI_HEAT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_heat.data.bss")
#pragma data_seg(".ui_action_heat.data")
#pragma const_seg(".ui_action_heat.text.const")
#pragma code_seg(".ui_action_heat.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_HEAT

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)


typedef struct {
    u32 heat_cur;		//当前压力值
    u32 avg_day[5];		//一天的压力值（分为0,6,12,18,24五个时间段）
    u16 test_timer;
} HEAT_UI_PARAM;

static HEAT_UI_PARAM *heat_ui_handler = NULL;

#define __this		heat_ui_handler

extern int gpu_fill_rect(struct draw_context *dc, int left, int top, int width, int height, u32 acolor);

/********************************************************外部可用接口********************************************************************/
void heat_heat_cur_set(u32 heat)
{
    struct unumber numb;

    __this->heat_cur = heat;
}

u32 heat_heat_cur_get()
{
    return __this->heat_cur;
}


void heat_avg_day_set(u32 index, u32 heat_avg)
{
    if (index < 5) {
        __this->avg_day[index] = heat_avg;
    }
}

u32 heat_avg_day_get(u32 index)
{
    if (index < 5) {
        return __this->avg_day[index];
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

static void heat_draw_barchart(struct draw_context *dc, u32 id, u32 data[], u32 data_num, u32 max, u32 min, int color)
{
    struct element *elm;
    struct rect rect;
    u32 draw_x, draw_y, draw_width, draw_height;
    u32 interval;	//柱状图之间的间隔

    elm = ui_core_get_element_by_id(id);
    ui_core_get_element_abs_rect(elm, &rect);
    interval = (float)rect.width / (data_num * 2 - 1);
    printf("rect:%d * %d\n", rect.width, rect.height);
    draw_width = (float)(rect.width - interval * (data_num - 1)) / data_num;

    for (int i = 0; i < data_num; i++) {
        if (data[i] <  min) {
            draw_height = 0;
        } else {
            draw_height = (float)rect.height / (max - min) * (data[i] - min);
        }
        draw_x = i * (draw_width + interval);
        draw_y = rect.height - draw_height;

        if (!(0 == draw_height || 0 == draw_width)) {
            gpu_fill_rect(dc, rect.left + draw_x, rect.top + draw_y, draw_width, draw_height, 100 << 24 | color);
        }
    }
}

static void heat_draw_barchart_bar(struct draw_context *dc, u32 id, u32 data[], u32 data_num, u32 max, u32 min, int color)
{
    struct element *elm;
    struct rect rect;
    u32 draw_x, draw_y, draw_width, draw_height;
    u32 interval;	//柱状图之间的间隔
    u32 percent;

    elm = ui_core_get_element_by_id(id);
    ui_core_get_element_abs_rect(elm, &rect);
    interval = (float)rect.width / (data_num * 2 - 1);
    printf("rect:%d * %d\n", rect.width, rect.height);
    draw_width = (float)(rect.width - interval * (data_num - 1)) / data_num;

    for (int i = 0; i < data_num; i++) {
        if (data[i] <  min) {
            draw_height = 0;
        } else {
            draw_height = (float)rect.height / (max - min) * (data[i] - min);
        }
        draw_x = i * (draw_width + interval);
        draw_y = rect.height - draw_height;
        percent = draw_height * 100 / rect.height;

        if (!(0 == draw_height || 0 == draw_width)) {
            ui_draw_bar(dc, rect.left + draw_x, rect.top + draw_y, draw_width, draw_height, color, 100, (i != (data_num - 1)));
        }
    }
}

static void heat_parm_init(void)
{
    if (__this) {
        printf("size of parm:%lu\n", sizeof(HEAT_UI_PARAM));
        memset(__this, 0, sizeof(HEAT_UI_PARAM));
    }
}

static int heat_num_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct rect rect;
    struct element_css *css;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_SHOW_PROBE:
        numb.type = TYPE_NUM;
        numb.numbs = 1;
        numb.number[0] = __this->heat_cur;

        ui_number_update((struct ui_number *)_ctrl, &numb);
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

REGISTER_UI_EVENT_HANDLER(HEAT_NUM)
.onchange = heat_num_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int heat_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
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
        ui_custom_draw_clear(dc);

        heat_draw_barchart(dc, elm->id, __this->avg_day, 5, 100, 0, RGB565(234, 59, 59));
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HEAT_LAYOUT)
.onchange = heat_layout_onchange,
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
        printf("ID_WINDOW_HAET init!!!");
        if (!__this) {
            __this = malloc(sizeof(HEAT_UI_PARAM));
        }
        heat_parm_init();
        break;
    case ON_CHANGE_RELEASE:
        printf("ID_WINDOW_HAET release!!!");
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

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_HEAT)
.onchange = window_heat_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/********************************************test*****************************************************/
static void test_timer_handler(void *priv)
{
    struct sys_time s_time;
    struct utime u_time;
    u32 rate;
    struct unumber numb;
    struct ui_number *number;

    rate = (heat_heat_cur_get() + 1) % 81;
    heat_heat_cur_set(rate);

    for (int i = 0; i < 5; i++) {
        heat_avg_day_set(i, heat_heat_cur_get());
    }

    numb.type = TYPE_NUM;
    numb.numbs = 1;
    numb.number[0] = __this->heat_cur;

    ui_number_update_by_id(HEAT_NUM, &numb);

}

static int heat_test_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    struct draw_context *dc = (struct draw_context *)arg;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        if (__this->test_timer == 0) {
            __this->test_timer = sys_timer_add(NULL, test_timer_handler, 2000);
        }
        break;
    case ON_CHANGE_SHOW_POST:
        printf("HEAT_LAYOUT show post!!!");
        break;
    case ON_CHANGE_RELEASE:
        printf("HEAT_LAYOUT relaese!!!");

        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(BASEFORM_633)
.onchange = heat_test_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/*****************************************************************************************************/


#endif /*if TCFG_UI_ENABLE_HEART*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/

