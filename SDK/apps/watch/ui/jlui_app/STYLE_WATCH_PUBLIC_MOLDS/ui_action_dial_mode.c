#include "app_config.h"
/* #include "app_task.h" */
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "rtc.h"

#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"
#include "ui_page_switch.h"
#include "btstack/avctp_user.h"
#include "jlui_app/ui_app_effect.h"
#include "gpu_task.h"
#include "avi_video.h"
#define LOG_TAG_CONST       UI_DIAL
#define LOG_TAG     		"[UI_DIAL]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_dial.data.bss")
#pragma data_seg(".ui_action_dial.data")
#pragma const_seg(".ui_action_dial.text.const")
#pragma code_seg(".ui_action_dial.text")
#endif


#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_DIAL_ENABLE


#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

static int watch_mode_sel_ontouch(void *ctr, struct element_touch_event *e)
{
    int ret;
    u8 watch_mode_type = 0;
    struct element *elm = (struct element *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        switch (elm->id) {
        case WATCH_MODE_SEL1:
            watch_mode_type = 1;
            break;
        case WATCH_MODE_SEL2:
            watch_mode_type = 0;
            break;
        }

        ret = syscfg_write(CFG_DIAL_TYPE_SEL, (u8 *)&watch_mode_type, 1);
        if (ret != 1) {
            printf("\n [ERROR] %s -[yuyu] %d\n", __FUNCTION__, __LINE__);
        }

        UI_SHOW_WINDOW(ID_WINDOW_DIAL);
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(WATCH_MODE_LAYOUT)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WATCH_MODE_SEL1)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch = watch_mode_sel_ontouch,
};
REGISTER_UI_EVENT_HANDLER(WATCH_MODE_SEL2)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch = watch_mode_sel_ontouch,
};
#endif
#endif
