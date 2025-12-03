#include "app_config.h"
#include "jlui_app/ui_style.h"
#include "jlui/ui.h"
#include "ui/ui_api.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "res/resfile.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "font/language_list.h"
#include "jlui/ui_measure.h"
#include "jlui_app/ui_sys_param.h"
#include "btstack/avctp_user.h"
#include "asm/math_fast_function.h"
#include "font/font_textout.h"
#include "jlui/ui_effect.h"
#include "app_common.h"
#include "health_manager/health_manager.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_COMPASS]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_compass.data.bss")
#pragma data_seg(".ui_action_compass.data")
#pragma const_seg(".ui_action_compass.text.const")
#pragma code_seg(".ui_action_compass.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_COMPASS

#define STYLE_NAME JL
static int watch_bt_compass_timer = 0;

static void BT_COMPASS_timer(void *priv)
{
    if (!watch_bt_compass_timer) {
        return;
    }
    static int bk_angle = 0;
    static int ind_angle = 0;
    int sign = rand32() % 2;
    static char degree_str[4];

#if TCFG_SPORT_HEALTH_ENABLE
    sport_health_manager_value_get(SHM_MOD_COMPASS, SHM_GET_TYPE_REAL_VALUE, &bk_angle);
    bk_angle = ((360 - bk_angle) + 270) % 360;
#else
    bk_angle = sign ? 360 - (rand32() % 180) : rand32() % 180;
#endif

    ui_io_set(IO_FRAME, HIGH);
    ui_compass_set_angle_by_id(BT_COMPASS, bk_angle, ind_angle);
    ui_io_set(IO_FRAME, LOW);

    sprintf(degree_str, "%d", bk_angle);
    ui_text_set_textu_by_id(COMPASS_DEGREE_TEXT, degree_str, strlen(degree_str), FONT_DEFAULT);
}

static int BT_COMPASS_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_compass *compass = (struct ui_compass *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        if (!watch_bt_compass_timer) {
#if TCFG_SPORT_HEALTH_ENABLE
            sport_health_manager_msg_post(SHM_MOD_DET_WRIST, SHM_CMD_DISABLE, 0, 1);
            sport_health_manager_msg_post(SHM_MOD_COMPASS,   SHM_CMD_ENABLE,  0, 1);
#endif
            watch_bt_compass_timer = sys_timer_add(NULL, BT_COMPASS_timer, 100);
        }
        break;
    case ON_CHANGE_RELEASE:
        if (watch_bt_compass_timer) {
#if TCFG_SPORT_HEALTH_ENABLE
            sport_health_manager_msg_post(SHM_MOD_DET_WRIST, SHM_CMD_ENABLE,  0, 1);
            sport_health_manager_msg_post(SHM_MOD_COMPASS,   SHM_CMD_DISABLE, 0, 1);
#endif
            sys_timer_del(watch_bt_compass_timer);
            watch_bt_compass_timer = 0;
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(BT_COMPASS)
.onchange = BT_COMPASS_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif
#endif
