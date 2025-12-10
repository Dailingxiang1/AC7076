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

#include "jlgpu_math.h"		// matrix
#include "jlgpu_driver.h"	// gpu 驱动

#include "gpu_port.h"	// GPU 接口，依赖于 jlgpu_driver.h
#include "gpu_task.h"

#include "jljpeg_decode.h"
#include "jpeg_stream.h"
/*
 * 描述：UVC显示模块
 * 使用uvc模块需要开启PSRAN模块，
 * TCFG_PSRAM_DEV_ENABLE 1 // PSRAM 使能
 * TCFG_PSRAM_SIZE 2*1024*1024 // PSRAM容量
*/
#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_HEAT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_uvc_show.data.bss")
#pragma data_seg(".ui_action_uvc_show.data")
#pragma const_seg(".ui_action_uvc_show.text.const")
#pragma code_seg(".ui_action_uvc_show.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))

#ifndef UVC_SHOW
#define UVC_SHOW LAYOUT_UVC_SHOW
#endif

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)



#define UVC_JPG_WIDTH	640
#define UVC_JPG_HEIGHT	480


#if TCFG_UI_UVC_SHOW_ENABLE
extern  int jluvc_set_refresh_cb(void (*cb)(void));
static void jpeg_dec_flush(void)
{
    struct element *elm;
    elm = ui_core_get_element_by_id(UVC_SHOW);
    if (elm != NULL) {
        printf(">>>>>>elm != NULL");
        ui_redraw(UVC_SHOW);
    }
}

void ui_uvc_ui_reflush(void)
{
#if ((defined TCFG_UI_ENABLE) && TCFG_UI_ENABLE)
    int msg[3] = {0};
    msg[0] = (int) jpeg_dec_flush; // 刷新
    msg[1] = 1;
    msg[2] = 0;
    int ret = os_taskq_post_type("ui", Q_CALLBACK, 3, msg);
#endif
}

static int uvc_layout_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_auto_shut_down_disable();
        jljpeg_stream_init();
        jluvc_set_refresh_cb(ui_uvc_ui_reflush);
        break;
    case ON_CHANGE_SHOW_POST:
        /*一般在此处创建自定义绘图信息，叠加到控件之上*/
        if (!jljpeg_stream_src_data_get()) {
            break;
        }
        dc = (struct draw_context *)arg;
        /* ui_custom_draw_clear(dc); */
        /* put_buf(jljpeg_stream_src_data_get(),128); */
        jlgpu_scheduler_wait_sync();
        jlgpu_task_clean_up_by_id(dc->gpu_task_head, dc->elm->id, 0x1);
        jpeg_image_ram(dc, 0, 0, UVC_JPG_WIDTH	, UVC_JPG_HEIGHT,
                       jljpeg_stream_src_data_get(),
                       jljpeg_stream_src_data_len_get());
        break;
    case ON_CHANGE_RELEASE:
        jljpeg_stream_deinit();
        ui_auto_shut_down_enable();
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(UVC_SHOW)
.onchange =  uvc_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



#endif /*if TCFG_UI_UVC_SHOW_ENABLE*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/

