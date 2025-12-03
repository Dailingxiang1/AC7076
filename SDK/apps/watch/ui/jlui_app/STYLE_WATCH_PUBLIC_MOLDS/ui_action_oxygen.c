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



#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_OXYGEN]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_oxygen.data.bss")
#pragma data_seg(".ui_action_oxygen.data")
#pragma const_seg(".ui_action_oxygen.text.const")
#pragma code_seg(".ui_action_oxygen.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_OXYGEN

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)

typedef struct {
    u32 sa_o2;	//血氧饱和度
    u32 sa_o2_last[7];	//最后7次的血氧饱和度
    u32 avghr;		//平均心率
    u32 avghr_week[7];
    u16 oxygen_timer;
    u16 test_timer;
} OXYGEN_UI_PARAM;

static OXYGEN_UI_PARAM *oxygen_ui_handler = NULL;

#define __this		oxygen_ui_handler


extern int gpu_fill_rect(struct draw_context *dc, int left, int top, int width, int height, u32 acolor);

/********************************************************外部可用接口********************************************************************/
void oxygen_sa_o2_set(u32 sa_o2)
{
    struct unumber numb;

    __this->sa_o2 = sa_o2;

}

void oxygen_avghr_set(u32 avghr)
{
    struct unumber numb;

    __this->avghr = avghr;
}

u32 oxygen_sa_o2_get()
{
    return __this->sa_o2;
}

u32 oxygen_avghr_get()
{
    return __this->avghr;
}

void oxygen_sa_o2_last_set(u32 index, u32 sa_o2)
{
    if (index < 7) {
        __this->sa_o2_last[index] = sa_o2;
    }
}

u32 oxygen_sa_o2_last_get(u32 index)
{
    if (index < 7) {
        return __this->sa_o2_last[index];
    }

    return 0;
}

void oxygen_avghr_week_set(u32 index, u32 rate)
{
    if (index < 7) {
        __this->avghr_week[index] = rate;
    }
}

u32 oxygen_avghr_week_get(u32 index)
{
    if (index < 7) {
        return __this->avghr_week[index];
    }

    return 0;
}

/****************************************************************************************************************************************/
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
            gpu_fill_rect(dc, rect.left + draw_x, rect.top + draw_y, draw_width, draw_height, 100 << 24 | 0xf800);
        }
    }
}

static void oxygen_parm_init(void)
{
    if (__this) {
        printf("size of parm:%lu\n", sizeof(OXYGEN_UI_PARAM));
        memset(__this, 0, sizeof(OXYGEN_UI_PARAM));
    }
}

static int oxygen_num_onchange(void *_ctrl, enum element_change_event event, void *arg)
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
        break;
    case ON_CHANGE_SHOW_POST:
        number = (struct ui_number *)elm;

        numb.type = TYPE_NUM;
        numb.numbs = 1;
        numb.number[0] = __this->sa_o2;

        ui_number_update(number, &numb);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(OXYGEN_NUM)
.onchange = oxygen_num_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int oxygen_avg_num_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct ui_number *number;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        putchar('&');
        break;
    case ON_CHANGE_FIRST_SHOW:
        putchar('@');
        break;
    case ON_CHANGE_SHOW_POST:
        number = (struct ui_number *)elm;

        numb.type = TYPE_NUM;
        numb.numbs = 1;
        numb.number[0] = __this->avghr;

        ui_number_update(number, &numb);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(OXYGEN_AVG_NUM)
.onchange = oxygen_avg_num_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int oxygen_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
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
        ui_core_get_element_abs_rect(elm, &rect);

        ui_custom_draw_clear(dc);

        ui_draw_barchart(dc, elm->id, __this->sa_o2_last, 7, 100, 0);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(OXYGEN_LAYOUT)
.onchange = oxygen_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static void test_timer_handler(void *priv);
static int oxygen_weekrate_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
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
        ui_core_get_element_abs_rect(elm, &rect);

        ui_custom_draw_clear(dc);

        ui_draw_barchart(dc, elm->id, __this->avghr_week, 7, 150, 60);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(OXYGEN_WEEKRATE_LAYOUT)
.onchange = oxygen_weekrate_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int oxygen_main_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(OXYGEN_MAIN_LAYOUT)
.onchange = oxygen_main_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int oxygen_main_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct ui_grid *grid = (struct ui_grid *)elm;
    static struct scroll_area area = {0, 0, 10000, 10000};

    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
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

REGISTER_UI_EVENT_HANDLER(OXYGEN_MAIN_VLIST)
.onchange = oxygen_main_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int window_oxygen_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        printf("ID_WINDOW_OXYGEN init!!!");
        if (!__this) {
            __this = malloc(sizeof(OXYGEN_UI_PARAM));
        }
        oxygen_parm_init();
        break;
    case ON_CHANGE_RELEASE:
        printf("ID_WINDOW_OXYGEN release!!!");
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

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_OXYGEN)
.onchange = window_oxygen_onchange,
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

    numb.type = TYPE_NUM;
    numb.numbs = 1;
    numb.number[0] = __this->sa_o2;

    ui_number_update_by_id(OXYGEN_NUM, &numb);

    numb.type = TYPE_NUM;
    numb.numbs = 1;
    numb.number[0] = __this->avghr;

    ui_number_update_by_id(OXYGEN_AVG_NUM, &numb);


    ui_core_redraw(ui_core_get_element_by_id(OXYGEN_LAYOUT));
    ui_core_redraw(ui_core_get_element_by_id(OXYGEN_WEEKRATE_LAYOUT));

    //测试心率显示
    rate = (oxygen_sa_o2_get() + 1) % 100;
    oxygen_sa_o2_set(rate);
    rate = (oxygen_avghr_get() + 1) % 150;
    rate = (rate < 60) ? 60 : rate;
    oxygen_avghr_set(rate);

    for (int i = 0; i < 7; i++) {
        oxygen_sa_o2_last_set(i, oxygen_sa_o2_get());
    }

    for (int i = 0; i < 7; i++) {
        oxygen_avghr_week_set(i, oxygen_avghr_get());
    }
}

static int oxygen_layer_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        if (__this->test_timer == 0) {
            __this->test_timer = sys_timer_add(elm, test_timer_handler, 1000);
        }
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

REGISTER_UI_EVENT_HANDLER(OXYGEN_LAYER)
.onchange = oxygen_layer_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/*****************************************************************************************************/





#endif /*if TCFG_UI_ENABLE_OXYGEN*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/

