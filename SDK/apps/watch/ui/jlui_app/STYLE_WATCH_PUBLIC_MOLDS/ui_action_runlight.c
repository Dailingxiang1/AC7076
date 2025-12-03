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
#define LOG_TAG     		"[UI_RUNLIGHT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_runlight.data.bss")
#pragma data_seg(".ui_action_runlight.data")
#pragma const_seg(".ui_action_runlight.text.const")
#pragma code_seg(".ui_action_runlight.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_RUNLIGHT

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)

typedef struct {
    bool runlight_con;	//0：关灯；1：开灯
    u16 test_timer;
} RUNLIGHT_UI_PARAM;

static RUNLIGHT_UI_PARAM *runlight_ui_handler = NULL;

#define __this		runlight_ui_handler

/*************************************************外部可用接口**************************************************************/
void runlight_open()
{
    if (__this->runlight_con != 1) {
        __this->runlight_con = 1;

        ui_hide(RUNLIGHT_CLOSE_LAYOUT);
        ui_show(RUNLIGHT_OPEN_LAYOUT);
    }
}

void runlight_close()
{
    if (__this->runlight_con) {
        __this->runlight_con = 0;

        ui_hide(RUNLIGHT_OPEN_LAYOUT);
        ui_show(RUNLIGHT_CLOSE_LAYOUT);
    }
}

bool runlight_con_get()
{
    return __this->runlight_con;
}
/***************************************************************************************************************************/
static void runlight_parm_init(void)
{
    if (__this) {
        printf("size of parm:%lu\n", sizeof(RUNLIGHT_UI_PARAM));
        memset(__this, 0, sizeof(RUNLIGHT_UI_PARAM));
    }
}

static int runlight_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        putchar('&');
        break;
    case ON_CHANGE_FIRST_SHOW:
        putchar('@');
        if (__this->runlight_con) {
            ui_hide(RUNLIGHT_CLOSE_LAYOUT);
            ui_show(RUNLIGHT_OPEN_LAYOUT);
        } else {
            ui_hide(RUNLIGHT_OPEN_LAYOUT);
            ui_show(RUNLIGHT_CLOSE_LAYOUT);
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

REGISTER_UI_EVENT_HANDLER(RUNLIGHT_LAYOUT)
.onchange = runlight_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int runlight_close_layout_ontouch(void *ctrl, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        ui_show(RUNLIGHT_OPEN_LAYOUT);
        ui_hide(RUNLIGHT_CLOSE_LAYOUT);
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(RUNLIGHT_CLOSE_LAYOUT)
.ontouch = runlight_close_layout_ontouch,
 .onkey = NULL,
  .onchange  = NULL,
};

static int runlight_open_layout_ontouch(void *ctrl, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        ui_show(RUNLIGHT_CLOSE_LAYOUT);
        ui_hide(RUNLIGHT_OPEN_LAYOUT);
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(RUNLIGHT_OPEN_LAYOUT)
.ontouch = runlight_open_layout_ontouch,
 .onkey = NULL,
  .onchange  = NULL,
};

static int window_runlight_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        printf("ID_WINDOW_RUNLIGHT init!!!");
        if (!__this) {
            __this = malloc(sizeof(RUNLIGHT_UI_PARAM));
        }
        runlight_parm_init();
        break;
    case ON_CHANGE_RELEASE:
        printf("ID_WINDOW_RUNLIGHT release!!!");
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

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_RUNLIGHT)
.onchange = window_runlight_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};





#endif /*if TCFG_UI_ENABLE_RUNLIGHT*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/



