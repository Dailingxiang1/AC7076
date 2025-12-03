#include "app_config.h"
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
#include "rtc.h"
#include "poweroff.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_ENGINEERING_MODE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_engineering_mode.data.bss")
#pragma data_seg(".ui_action_engineering_mode.data")
#pragma const_seg(".ui_action_engineering_mode.text.const")
#pragma code_seg(".ui_action_engineering_mode.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_ENGINEERING_MODE

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)

static void set_shipping_mode()
{
    log_info("shipping mode!\n");

    rtc_dev_deinit();     // 关闭rtc

    power_control(PCONTROL_SF_KEEP_LRC, 0);   // 关闭lrc

    power_control(PCONTROL_SF_VDDIO_KEEP, VDDIO_KEEP_TYPE_CLOSE);   // 关闭vddio

    sys_enter_soft_poweroff(POWEROFF_NORMAL);     // 软关机
}

static int engineering_mode_vlist_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    static u8 touch_action = 0;
    int sel_item;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        sel_item = ui_grid_cur_item(grid);
        if (sel_item < 0) {
            break;
        }
        switch (sel_item) {
        case 0:// 船运模式
            set_shipping_mode();
            break;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(ENGINEERING_MODE_VLIST)//通用-垂直列表
.onchange = NULL,
 .onkey = NULL,
  .ontouch = engineering_mode_vlist_ontouch,
};

#endif
#endif
