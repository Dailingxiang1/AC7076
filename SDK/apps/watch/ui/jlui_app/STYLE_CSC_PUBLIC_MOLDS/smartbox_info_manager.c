/**
 * @brief 彩屏仓相关数据信息
 *
 */
#include "app_config.h"
#include "smartbox_info_manager.h"
#include "events_adapter.h"
#include "btstack/le/att.h"
#include "ui/ui_api.h"
#include "ui_sys_param.h"
#include "jlui/font/language_list.h"
#include "utils/syscfg_id.h"
#include "system/init.h"
#include "bt_common.h"

#define LOG_TAG     		"[SBOX_INFO]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE

static struct s_box_info box_info = {
    .phone_state        = 0,
    .earphone_state     = 0,
    .l_ear_bat          = 0,
    .r_ear_bat          = 0,
    .l_inbox            = OUT_OF_BOX,     //左耳是否入仓
    .r_inbox            = OUT_OF_BOX,
    .box_bat            = -1,
    .music_state        = 2, // 默认配置为暂停状态
    .earphone_vol       = 0,
    .anc_mode           = ANC_MODE_OFF,
    .eq_mode            = 0,
    .ui_bl_levl         = 5,
    .l_charging         = 0,
    .r_charging         = 0,
    .box_charging       = 0,
    .l_finding          = 0,
    .r_finding          = 0,
    .box_clid_status    = LID_CLOSE,
    .language           = Chinese_Simplified,
    .low_power          = 0,
    .phone_call_state   = 0,
    .phone_call_mute    = PHONE_CTRL_MUTE_OFF,
    .local_page         = 0,
    .emitter_status     = 0, // 默认配置为手机音乐 0：手机音乐 1：SD音乐
    .box_touch_switch = 1,
    .box_voice_switch = 0,

};

static custom_edr_info s_emitter_info;

struct s_box_info *getbox()
{
    return &box_info;
}

#define ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE 0x0009

u8 sbox_bt_app_flag_get(void)
{
    int cang_ble_state = att_get_ccc_config(ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE);
    box_info.app_state = cang_ble_state;
    return box_info.app_state;
}
u8 sbox_get_box_charging(void)
{
    return box_info.box_charging;
}

void sbox_bt_connect_flag_set(u8 bt_connect_flag)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_info("%s rets:%x bt_connect_flag:%d", __func__, rets, bt_connect_flag);
    box_info.phone_state = bt_connect_flag;
    UI_MSG_POST("bt_status");
}
u8 sbox_bt_connect_flag_get(void)
{
    return box_info.phone_state;
}

void sbox_ble_connect_flag_set(u8 ble_connect_flag)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_info("%s rets:%x ble_connect_flag:%d", __func__, rets, ble_connect_flag);
    box_info.earphone_state = ble_connect_flag;
    sbox_left_finding_set(FALSE);
    sbox_right_finding_set(FALSE);
    UI_MSG_POST("bt_status");
}
u8 sbox_ble_connect_flag_get(void)
{
    return box_info.earphone_state;
}
void sbox_battery_left_set(u8 battery_left)
{
    if (battery_left > 100 || battery_left < 0) {
        return;
    }
    box_info.l_ear_bat = battery_left;
    UI_MSG_POST("topbar_battery");
}
u8 sbox_battery_left_get(void)
{
    return box_info.l_ear_bat;
}
void sbox_battery_right_set(u8 battery_right)
{
    if (battery_right > 100 || battery_right < 0) {
        return;
    }
    box_info.r_ear_bat = battery_right;
    UI_MSG_POST("topbar_battery");
}
u8 sbox_battery_right_get(void)
{
    return box_info.r_ear_bat;
}
void sbox_battery_box_set(u8 battery_box)
{
    box_info.box_bat = battery_box;
    UI_MSG_POST("topbar_battery");

#if (defined TCFG_CSC_BT_APP) && TCFG_CSC_BT_APP
    void screen_bt_ble_rcsp_send_info(void *p);
    int msg[2];
    msg[0] = (int)screen_bt_ble_rcsp_send_info;
    msg[1] = 0;
    os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
#endif
}
u8 sbox_battery_box_get(void)
{
    return box_info.box_bat;
}
void sbox_language_ui_set(u8 language)
{
    box_info.language = language;
}
u8 sbox_language_ui_get(void)
{
    return box_info.language;
}
void sbox_music_ui_state_set(u8 music)
{
    box_info.music_state = music;
    // UI_MSG_POST("music_status:pp=%4",music);
#if(defined TCFG_UI_MUSIC_CTRL_ENABLE && TCFG_UI_MUSIC_CTRL_ENABLE)
    extern void ui_music_update(char *type, int arg);
    ui_music_update("pp", box_info.music_state);
#endif


}
u8 sbox_music_ui_state_get(void) // 0:index 0
{
    return box_info.music_state;
}

void sbox_volume_set(u8 volume)
{
    box_info.earphone_vol = volume;
    s16 volume_vm = volume;
    syscfg_write(CFG_MUSIC_VOL, &volume_vm, sizeof(volume_vm));
    UI_MSG_POST("volume");
}
u8 sbox_volume_get(void)
{
    return box_info.earphone_vol;
}

// 1 隐藏 2来电 3通话中 4 拨打
void sbox_call_state_set(u8 flag)
{
    box_info.phone_call_state = flag;
}
u8 sbox_call_state_get(void)
{
    return box_info.phone_call_state;
}

u8 sbox_equalizer_mode_get(void)
{
    return box_info.eq_mode;
}
void sbox_equalizer_mode_set(u8 equalizer_mode)
{
    box_info.eq_mode = equalizer_mode;
    UI_MSG_POST("EQ_UPDATA");
}

void sbox_eq_gain_set(u8 *gain)
{
    memcpy(box_info.eq_gain, gain, 10);
}

u8 *sbox_eq_gain_get(u8 *gain)
{
    return box_info.eq_gain;
}
void sbox_key_info_set(u8 *mysetting)
{
    memcpy(box_info.key_info, mysetting, 8);
}

u8 *sbox_key_info_get(void)
{
    return box_info.key_info;
}


void sbox_backlight_level_set(u8 backlight_level)
{
    box_info.ui_bl_levl = backlight_level;
}
u8 sbox_backlight_level_get(void)
{
    box_info.ui_bl_levl = get_ui_sys_param(LightLevel);
    return box_info.ui_bl_levl;
}

void sbox_left_charging_set(u8 charging)
{
    box_info.l_charging = charging;
    UI_MSG_POST("topbar_battery");
}
u8 sbox_left_charging_get(void)
{
    return box_info.l_charging;
}

void sbox_right_charging_set(u8 charging)
{
    box_info.r_charging = charging;
    UI_MSG_POST("topbar_battery");
}
u8 sbox_right_charging_get(void)
{
    return box_info.r_charging;
}

void sbox_box_charging_set(u8 charging)
{
    box_info.box_charging = charging;
    UI_MSG_POST("topbar_battery");
}
u8 sbox_box_charging_get(void)
{
    return box_info.box_charging;
}

void sbox_left_finding_set(u8 finding)
{
    box_info.l_finding = finding;
}
u8 sbox_left_finding_get(void)
{
    return box_info.l_finding;
}

void sbox_right_finding_set(u8 finding)
{
    box_info.r_finding = finding;
}
u8 sbox_right_finding_get(void)
{
    return box_info.r_finding;
}

void sbox_box_clid_status_set(u8 status)
{
    box_info.box_clid_status = status;
}
u8 sbox_box_clid_status_get(void)
{
    return box_info.box_clid_status;
}

void sbox_low_power_set(u8 low_power)
{
    box_info.low_power = low_power;
}
u8 sbox_low_power_get(void)
{
    return box_info.low_power;
}

void sbox_phone_call_state_set(u8 state)
{
    box_info.phone_call_state = state;
}
u8 sbox_phone_call_state_get(void)
{
    return box_info.phone_call_state;
}

void sbox_phone_call_mute_set(u8 mute)
{
    box_info.phone_call_mute = mute;
}
u8 sbox_phone_call_mute_get(void)
{
    return box_info.phone_call_mute;
}

void sbox_box_year_set(u8 year)
{
    box_info.phone_time.year = year;
}
u8 sbox_box_year_get(void)
{
    return box_info.phone_time.year;
}

void sbox_box_month_set(u8 month)
{
    box_info.phone_time.month = month;
}
u8 sbox_box_month_get(void)
{
    return box_info.phone_time.month;
}
void sbox_box_day_set(u8 day)
{
    box_info.phone_time.day = day;
}
u8 sbox_box_day_get(void)
{
    return box_info.phone_time.day;
}
void sbox_box_hour_set(u8 hour)
{
    box_info.phone_time.hour = hour;
}

bool wallpaper_flag = false;
void app_lock_set_flag(bool flag)
{
    wallpaper_flag = flag;
}

bool app_lock_get_flag(void)
{
    return wallpaper_flag;
}

void set_touch_switch_flag(bool flag)
{
    box_info.box_touch_switch = flag;
}

bool get_touch_switch_flag(void)
{
    return box_info.box_touch_switch;
}

void set_voice_switch_flag(bool flag)
{
    box_info.box_voice_switch = flag;
}

bool get_voice_switch_flag(void)
{
    return box_info.box_voice_switch;
}


u8 sbox_box_hour_get(void)
{
    return box_info.phone_time.hour;
}

void sbox_box_minute_set(u8 minute)
{
    box_info.phone_time.min = minute;
}
u8 sbox_box_minute_get(void)
{
    return box_info.phone_time.min;
}
void sbox_box_second_set(u8 sec)
{
    box_info.phone_time.sec = sec;
}
u8 sbox_box_second_get(void)
{
    return box_info.phone_time.sec;
}

u8 sbox_box_time_get(struct sys_time *time)
{
    if (time == NULL) {
        log_error("time is null, please check! func: %s, line: %d", __func__, __LINE__);
        return -1;
    }

    time->year = box_info.phone_time.year;
    time->month = box_info.phone_time.month;
    time->day = box_info.phone_time.day;
    time->hour = box_info.phone_time.hour;
    time->min = box_info.phone_time.min;
    time->sec = box_info.phone_time.sec;
    return 0;
}

void sbox_phone_time_set(u16 year, u8 month, u8 day, u8 hour, u8 min, u8 sec)
{
    box_info.phone_time.year = year;
    box_info.phone_time.month = month;
    box_info.phone_time.day = day;
    box_info.phone_time.hour = hour;
    box_info.phone_time.min = min;
    box_info.phone_time.sec = sec;
}

void sbox_local_page_set(u8 page)
{
    box_info.local_page = page;
}
u8 sbox_local_page_get(void)
{
    return box_info.local_page;
}


void sbox_anc_mode_set(u8 mode)
{
    box_info.anc_mode = mode;
    UI_MSG_POST("denoise");
}

u8 sbox_anc_mode_get(void)
{
    return box_info.anc_mode;
}

void sbox_emitter_state_set(u8 state)
{
    box_info.emitter_status = state;
}

u8 sbox_emitter_state_get(void)
{
    return box_info.emitter_status;
}

void sbox_emitter_addr_set(void)
{
    memcpy(s_emitter_info.emitter_addr, (void *)bt_get_mac_addr(), 6);
}

u8 *sbox_emitter_addr_get(void)
{
    return s_emitter_info.emitter_addr;
}

void sbox_r_inbox_set(u8 state)
{
    box_info.r_inbox = state;
}

u8 sbox_r_inbox_get(void)
{
    return box_info.r_inbox;
}

void sbox_l_inbox_set(u8 state)
{
    box_info.l_inbox = state;
}

u8 sbox_l_inbox_get(void)
{
    return box_info.l_inbox;
}

void ear_inbox_state_deal(u8 status)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_info("%s rets:%x status:%d", __func__, rets, status);
    switch (status) {
    case 1:
        box_info.l_inbox = IN_BOX;
        break;
    case 2:                                         //右耳入仓
        box_info.r_inbox = IN_BOX;
        break;
    case 3:
        box_info.l_inbox = OUT_OF_BOX;
        break;
    case 4:
        box_info.r_inbox = OUT_OF_BOX;
        break;

    default:
        break;
    }
    log_info("ear_inbox_state_deal l:%d r:%d\n", box_info.l_inbox, box_info.r_inbox);
    // if( box_info.l_inbox){
    //     // ui_earin_enter();
    //     b_printf("ui_earin_enter()");
    // }

    //screen_pop_up_callback_enter_filter();
}


static int sbox_info_init(void)
{
    log_info("%s", __func__);
    int ret = 0;
    s16 volume_vm = 0;

    ret = syscfg_read(CFG_MUSIC_VOL, &volume_vm, sizeof(volume_vm));
    if (ret != sizeof(volume_vm)) {
        volume_vm = 0;
    }

    box_info.earphone_vol = volume_vm;
    return 0;
}

late_initcall(sbox_info_init);


#endif //#if TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE

