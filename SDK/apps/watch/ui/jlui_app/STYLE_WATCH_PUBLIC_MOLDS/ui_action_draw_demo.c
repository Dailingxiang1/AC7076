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
#define LOG_TAG     		"[UI_DRAW_DEMO]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_draw_demo.data.bss")
#pragma data_seg(".ui_action_draw_demo.data")
#pragma const_seg(".ui_action_draw_demo.text.const")
#pragma code_seg(".ui_action_draw_demo.text")
#endif

#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_DRAW_DEMO_ENABLE

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

#define DRAW_DEMO_LAYOUT   0

static int draw_demo_layout_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        /*不一定有init probe*/
        break;
    case ON_CHANGE_INIT:
        /*初始化事件*/
        break;
    case ON_CHANGE_SHOW_PROBE:
        /*dc创建前的事件，可以在此处修改与element相关的属性*/
        break;
    case ON_CHANGE_SHOW:
        /*在show事件之后，将使用dc指针的信息绘制控件背景色、背景图片、及图片、文本*/
        dc = (struct draw_context *)arg;
        log_info("elm_id:0x%x dc_elm_id:0x%x", elm->id, dc->elm->id);
        break;
    case ON_CHANGE_SHOW_POST:
        /*一般在此处创建自定义绘图信息，叠加到控件之上*/
        dc = (struct draw_context *)arg;
        log_info("elm_id:0x%x dc_elm_id:0x%x", elm->id, dc->elm->id);
        break;
    case ON_CHANGE_HIDE:
        /*隐藏，释放时产生*/
        break;
    case ON_CHANGE_RELEASE_PROBE:
        /*释放*/
        break;
    case ON_CHANGE_RELEASE:
        /*释放*/
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*第一次显示 show三个事件之后post出*/
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        /*第一次显示 show三个事件之后post出*/
        break;
    case ON_CHANGE_HIGHLIGHT:
        /*高亮切换事件，不是所有控件都有*/
        int is_highlight = (int)arg;
        break;
    case ON_CHANGE_UPDATE_ITEM:
        /*动态列表事件*/
        int index = (int)arg;
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(DRAW_DEMO_LAYOUT)
.onchange =  draw_demo_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif// TCFG_UI_DRAW_DEMO
#endif// CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
