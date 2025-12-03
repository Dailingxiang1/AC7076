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

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_FLASHLIGHT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_flashlight.data.bss")
#pragma data_seg(".ui_action_flashlight.data")
#pragma const_seg(".ui_action_flashlight.text.const")
#pragma code_seg(".ui_action_flashlight.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_FLASHLIGHT

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)

static int window_flashlight_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    int brightness_level;
    switch (event) {
    case ON_CHANGE_INIT:
        log_info("ID_WINDOW_FLASHLIGHT init!!!");
        brightness_level = MAX_LIGHTLEVEL;                   // 全亮屏
        ui_ajust_light(brightness_level * 2);
        break;
    case ON_CHANGE_RELEASE:
        log_info("ID_WINDOW_FLASHLIGHT release!!!");
        brightness_level = get_ui_sys_param(LightLevel);
        ui_ajust_light(brightness_level * 2);
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_FLASHLIGHT)
.onchange = window_flashlight_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif
#endif
