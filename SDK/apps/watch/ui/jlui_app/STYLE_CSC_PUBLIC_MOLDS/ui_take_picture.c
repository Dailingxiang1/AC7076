#include "app_config.h"
#include "ui/ui_api.h"
#include "events_adapter.h"
#include "smartbox_info_manager.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_CS_ABOUT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_takke_picture.data.bss")
#pragma data_seg(".ui_takke_picture.data")
#pragma const_seg(".ui_takke_picture.text.const")
#pragma code_seg(".ui_takke_picture.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if(defined TCFG_UI_PHOTOGRAHT_ENABLE && TCFG_UI_PHOTOGRAHT_ENABLE)


#define STYLE_NAME  JL

/************************************************************
 * 点击拍照处理
 **********************************************************/
static int ui_photogragh_click_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;    /*PIC类型控件，需要拦截down事件，才会有up*/
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            if (sbox_bt_connect_flag_get() == BT_CONNECTED) {
                custom_client_send_ctrl_photo(1);
            } else {
                log_warn("func:%s , ble is disconnected\n", __func__);
            }
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHOTOGRAPH_CLICK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_photogragh_click_pic_ontouch,
};

#endif  // #if(defined TCFG_UI_PHOTOGRAHT_ENABLE && TCFG_UI_PHOTOGRAHT_ENABLE)
#endif
