#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".battery_level.data.bss")
#pragma data_seg(".battery_level.data")
#pragma const_seg(".battery_level.text.const")
#pragma code_seg(".battery_level.text")
#endif
#include "system/includes.h"
#include "battery_manager.h"
#include "app_power_manage.h"
#include "app_main.h"
#include "app_config.h"
#include "app_action.h"
#include "asm/charge.h"
#include "app_tone.h"
#include "gpadc.h"
#include "btstack/avctp_user.h"
#include "user_cfg.h"
#include "bt_tws.h"
#include "idle.h"
#include "ui/ui_api.h"
#include "rcsp.h"
#include "power/power_manage.h"
#include "rtc.h"
#include "timestamp/timestamp.h"
#if RCSP_ADV_EN
#include "ble_rcsp_adv.h"
#endif

#define LOG_TAG             "[BATTERY]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

enum {
    VBAT_NORMAL = 0,
    VBAT_WARNING,
    VBAT_LOWPOWER,
} VBAT_STATUS;

#define VBAT_DETECT_CNT         3 //每次更新电池电量采集的次数
#define VBAT_DETECT_TIME        10L //每次采集的时间间隔
#define VBAT_UPDATE_SLOW_TIME   60000L //慢周期更新电池电量
#define VBAT_UPDATE_FAST_TIME   10000L //快周期更新电池电量

#define VBAT_PERCENT_MODE_OPEN_CIRCUIT_VOLTAGE			0		//开路电压法
#define VBAT_PERCENT_MODE_CURRENT_INTEGRATING_METHOD	1		//安时积分法
#define VBAT_PERCENT_MODE_SEL   VBAT_PERCENT_MODE_OPEN_CIRCUIT_VOLTAGE
/* #define VBAT_PERCENT_MODE_SEL  VBAT_PERCENT_MODE_CURRENT_INTEGRATING_METHOD */


struct battery_curve {
    u8 percent;
    u16 voltage;
};

union battery_data {
    u32 raw_data;
    struct {
        u8 reserved;
        u16 voltage;
        u8 percent;
    } __attribute__((packed)) data;
};

#define REAL_CHANEG_VAL			15				//参考值20
#define REAL_CHANGE_PER			85				//真实电流占比
#define PRED_CHANGE_PER			15				//模拟电流占比
#define BATTERY_CHARGE_K 		(1.0f)			//模拟充电电流系数
#define BATTERY_CAPACITY    	(200.0f) 		//电池总容量，单位mAh
#define BATTERY_RESISTOR    	(0.15f)   		//电池阻值
#define VBAT_UPDATE_SEC			8				//刷新间隔
#define BATTERY_CHANGE_SAVE		5				//电量变化超过5%时存储到vm
#define RECHECK_HOUR			24				//关机超过24小时，开机时校准
#define RECHECK_SOC				10				//误差超过10%电量，开机时校准

#define abs(x) ((x)>0?(x):-(x))
static float current_soc = 0;
static u16 last_present_k = 0;
static u32 is_sleep_cnt = 0;

static u16 vbat_slow_timer = 0;
static u16 vbat_fast_timer = 0;
static u16 lowpower_timer = 0;
static u8 old_battery_level = 9;
static u16 cur_battery_voltage = 0;
static u8 cur_battery_level = 0;
static u8 old_battery_percent = 0;
static u8 cur_battery_percent = 0;
static u8 tws_sibling_bat_level = 0xff;
static u8 tws_sibling_bat_percent_level = 0xff;
static u8 cur_bat_st = VBAT_NORMAL;
static u8 battery_curve_max;
static struct battery_curve *battery_curve_p;

void vbat_check(void *priv);
void clr_wdt(void);
u8 get_charge_full_flag(void);
#if TCFG_USER_TWS_ENABLE
u8 get_tws_sibling_bat_level(void)
{
    return tws_sibling_bat_level & 0x7f;
}

u8 get_tws_sibling_bat_persent(void)
{
    return tws_sibling_bat_percent_level;
}

void app_power_set_tws_sibling_bat_level(u8 vbat, u8 percent)
{
    tws_sibling_bat_level = vbat;
    tws_sibling_bat_percent_level = percent;
    /*
     ** 发出电量同步事件进行进一步处理
     **/
    batmgr_send_msg(POWER_EVENT_SYNC_TWS_VBAT_LEVEL, 0);

    log_info("set_sibling_bat_level: %d, %d\n", vbat, percent);
}


static void set_tws_sibling_bat_level(void *_data, u16 len, bool rx)
{
    u8 *data = (u8 *)_data;

    if (rx) {
        app_power_set_tws_sibling_bat_level(data[0], data[1]);
    }
}

REGISTER_TWS_FUNC_STUB(vbat_sync_stub) = {
    .func_id = TWS_FUNC_ID_VBAT_SYNC,
    .func    = set_tws_sibling_bat_level,
};

void tws_sync_bat_level(void)
{
#if TCFG_BT_DISPLAY_BAT_ENABLE
    u8 battery_level = cur_battery_level;
#if CONFIG_DISPLAY_DETAIL_BAT
    u8 percent_level = get_vbat_percent();
#else
    u8 percent_level = get_self_battery_level() * 10 + 10;
#endif
    if (get_charge_online_flag()) {
        percent_level |= BIT(7);
    }

    u8 data[2];
    data[0] = battery_level;
    data[1] = percent_level;
    tws_api_send_data_to_sibling(data, 2, TWS_FUNC_ID_VBAT_SYNC);

    log_info("tws_sync_bat_level: %d,%d\n", battery_level, percent_level);
#endif
}
#endif


static void power_warning_timer(void *p)
{
    batmgr_send_msg(POWER_EVENT_POWER_WARNING, 0);
}

static int app_power_event_handler(int *msg)
{
    int ret = false;

#if(TCFG_SYS_LVD_EN == 1)
    switch (msg[0]) {
    case POWER_EVENT_POWER_NORMAL:
        break;
    case POWER_EVENT_POWER_WARNING:
        play_tone_file(get_tone_files()->low_power);
#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
        UI_WINDOW_PREEMPTION_POSH(ID_WINDOW_LOW_POWER_TIPS, NULL, NULL, UI_WINDOW_PREEMPTION_TYPE_CHARGE);     // 低电提醒页面
#endif
        if (lowpower_timer == 0) {
            lowpower_timer = sys_timer_add(NULL, power_warning_timer, LOW_POWER_WARN_TIME);
        }
        break;
    case POWER_EVENT_POWER_LOW:
        r_printf(" POWER_EVENT_POWER_LOW");
        vbat_timer_delete();
        if (lowpower_timer) {
            sys_timer_del(lowpower_timer);
            lowpower_timer = 0 ;
        }
#if TCFG_APP_BT_EN
#if RCSP_ADV_EN
        adv_tws_both_in_charge_box(1);
#endif
        if (!app_in_mode(APP_MODE_IDLE)) {
            sys_enter_soft_poweroff(POWEROFF_NORMAL);
        } else {
            power_set_soft_poweroff();
        }
#else
        app_goto_mode(APP_MODE_IDLE, IDLE_MODE_PLAY_POWEROFF);
#endif
        break;
#if TCFG_APP_BT_EN
    case POWER_EVENT_SYNC_TWS_VBAT_LEVEL:
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            bt_cmd_prepare(USER_CTRL_HFP_CMD_UPDATE_BATTARY, 0, NULL);
        }
        break;
    case POWER_EVENT_POWER_CHANGE:
        log_info("POWER_EVENT_POWER_CHANGE\n");

        if (!app_in_mode(APP_MODE_BT)) {
            break;
        }
#if TCFG_USER_TWS_ENABLE
        if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
            if (tws_api_get_tws_state()&TWS_STA_ESCO_OPEN) {
                break;
            }
            tws_sync_bat_level();
        }
#endif
        bt_cmd_prepare(USER_CTRL_HFP_CMD_UPDATE_BATTARY, 0, NULL);
#endif
        RCSP_UPDATE(COMMON_FUNCTION, BIT(RCSP_DEVICE_STATUS_ATTR_TYPE_BATTERY));
        break;
    case POWER_EVENT_POWER_CHARGE:
        if (lowpower_timer) {
            sys_timer_del(lowpower_timer);
            lowpower_timer = 0 ;
        }
        break;
#if TCFG_CHARGE_ENABLE
    case CHARGE_EVENT_LDO5V_OFF:
        //充电拔出时重新初始化检测定时器
        vbat_check_init();
        break;
#endif
    default:
        break;
    }
#endif

    return ret;
}
APP_MSG_HANDLER(bat_level_msg_entry) = {
    .owner      = 0xff,
    .from       = MSG_FROM_BATTERY,
    .handler    = app_power_event_handler,
};

static u16 get_vbat_voltage(void)
{
    return gpadc_battery_get_voltage();
}

static u16 battery_calc_voltage(u16 percent)
{
    u8 i;
    u16 max, min, div_voltage, tmp_voltage;
    if (battery_curve_p == NULL) {
        printf("%s battery_curve not init!!!\n", __func__);
        return 0;
    }

    for (i = 0; i < (battery_curve_max - 1); i++) {
        if (percent <= battery_curve_p[i].percent) {
            printf("%s(1) %d", __func__, battery_curve_p[i].voltage);
            return battery_curve_p[i].voltage;
        }
        if (percent >= battery_curve_p[i + 1].percent) {
            continue;
        }

        div_voltage = battery_curve_p[i + 1].voltage - battery_curve_p[i].voltage;
        min = battery_curve_p[i].percent;
        max = battery_curve_p[i + 1].percent;
        tmp_voltage = battery_curve_p[i].voltage;
        tmp_voltage += ((percent - min) * div_voltage / (max - min));
        printf("%s(2) %d percent:%d div:%d min:%d max:%d tvp:%d", __func__, tmp_voltage, percent, div_voltage, min, max, battery_curve_p[i].voltage);
        return tmp_voltage;
    }
    printf("%s(3) %d", __func__, battery_curve_p[battery_curve_max - 1].voltage);
    return battery_curve_p[battery_curve_max - 1].voltage;
}

static u16 battery_calc_percent(u16 bat_val)
{
    u8 i, tmp_percent;
    u16 max, min, div_percent;
    if (battery_curve_p == NULL) {
        log_error("battery_curve not init!!!\n");
        return 0;
    }

    for (i = 0; i < (battery_curve_max - 1); i++) {
        if (bat_val <= battery_curve_p[i].voltage) {
            return battery_curve_p[i].percent;
        }
        if (bat_val >= battery_curve_p[i + 1].voltage) {
            continue;
        }
        div_percent = battery_curve_p[i + 1].percent - battery_curve_p[i].percent;
        min = battery_curve_p[i].voltage;
        max = battery_curve_p[i + 1].voltage;
        tmp_percent = battery_curve_p[i].percent;
        tmp_percent += (bat_val - min) * div_percent / (max - min);
        return tmp_percent;
    }
    return battery_curve_p[battery_curve_max - 1].percent;
}

u16 get_vbat_value(void)
{
    return cur_battery_voltage;
}

u8 get_vbat_percent(void)
{
    return cur_battery_percent;
}

bool get_vbat_need_shutdown(void)
{
    if ((cur_battery_voltage <= app_var.poweroff_tone_v) || adc_check_vbat_lowpower()) {
        return TRUE;
    }
    return FALSE;
}

//将当前电量转换为1~9级发送给手机同步电量
u8  battery_value_to_phone_level(void)
{
    u8  battery_level = 0;
    u8 vbat_percent = get_vbat_percent();
    if (vbat_percent < 5) { //小于5%电量等级为0，显示10%
        return 0;
    }
    battery_level = (vbat_percent - 5) / 10;
    return battery_level;
}

//获取自身的电量
u8  get_self_battery_level(void)
{
    return cur_battery_level;
}

#if TCFG_USER_TWS_ENABLE
u8 get_cur_battery_level(void)
{
    u8 bat_lev = tws_sibling_bat_level & (~BIT(7));
    if (bat_lev == 0x7f) {
        return cur_battery_level;
    }

#if (CONFIG_DISPLAY_TWS_BAT_TYPE == CONFIG_DISPLAY_TWS_BAT_LOWER)
    return cur_battery_level < bat_lev ? cur_battery_level : bat_lev;
#elif (CONFIG_DISPLAY_TWS_BAT_TYPE == CONFIG_DISPLAY_TWS_BAT_HIGHER)
    return cur_battery_level < bat_lev ? bat_lev : cur_battery_level;
#elif (CONFIG_DISPLAY_TWS_BAT_TYPE == CONFIG_DISPLAY_TWS_BAT_LEFT)
    return tws_api_get_local_channel() == 'L' ? cur_battery_level : bat_lev;
#elif (CONFIG_DISPLAY_TWS_BAT_TYPE == CONFIG_DISPLAY_TWS_BAT_RIGHT)
    return tws_api_get_local_channel() == 'R' ? cur_battery_level : bat_lev;
#else
    return cur_battery_level;
#endif
}

#else

u8 get_cur_battery_level(void)
{
    return cur_battery_level;
}
#endif


struct battery_info {
    u32 time_stamp;
    u16 percent;
};
int read_bat_info_vm(struct battery_info *info)
{
    int ret = syscfg_read(VM_BATTERY_INFO, info, sizeof(struct battery_info));
    if (ret != sizeof(struct battery_info)) {
        return -1;
    }
    return 0;
}
int bat_info_need_recheck(u16 vpercent)
{
    struct battery_info info;
    int ret = read_bat_info_vm(&info);
    if (ret) {
        printf("%s vm not find", __func__);
        return true;
    }
    last_present_k =  vpercent;

    struct sys_time time;
    rtc_read_time(&time);
    u32 timestamp = timestamp_mytime_2_utc_sec(&time);
    if (timestamp - info.time_stamp >  RECHECK_HOUR	* 60 * 60) { //超过半小时
        printf("%s time need reset", __func__);
        return true;
    }
    if (abs(vpercent - info.percent) > RECHECK_SOC) { //误差超过10%
        printf("%s vbat percent more than 10per", __func__);
        return true;
    }
    cur_battery_percent = info.percent;
    printf("%s used vm info:%d\n ", __func__, info.percent);
    return false;
}
void update_bat_info_vm()
{
    struct sys_time time;
    rtc_read_time(&time);
    u32 timestamp = timestamp_mytime_2_utc_sec(&time);
    struct battery_info info = {
        .time_stamp = timestamp,
        .percent = cur_battery_percent,
    };
    syscfg_write(VM_BATTERY_INFO, &info, sizeof(struct battery_info));
}
void bat_info_storage_task_sync()
{
    if (abs(cur_battery_percent - last_present_k) >= BATTERY_CHANGE_SAVE) {
        last_present_k = cur_battery_percent;
        int argv[3];
        argv[0] = (int)update_bat_info_vm;
        argv[1] = 0;
        int ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, argv);
        if (ret) {
            log_error("vbat change post ret:%d \n", ret);
        }
    }
}

void vbat_sys_sleep_check()
{
    int is_low_power = !low_power_sys_not_idle_cnt();
    if (is_low_power) {
        ++is_sleep_cnt;
    } else {
        is_sleep_cnt = 0;
    }
}



float vbat_predicted_current(u16 voltage, u16 soc)
{
    u16 ocv = battery_calc_voltage(soc);
    printf("%s ocv:%d soc:%d vol:%d ", __func__, ocv, soc, voltage);
    return (float)(voltage - ocv) / BATTERY_RESISTOR;//I mA
}

void vbat_calculate_loop()
{
    // 模拟每 8 秒更新一次电池电压 curr_present_k
    int curr_present_k;
    //获取vbat电压
    u16 current_vbat = get_vbat_voltage() ;//mv
    u16 ocv2soc = battery_calc_percent(current_vbat);
    float I = 0.0f;
    //长时间休眠且非充电状态直接校准
    if ((is_sleep_cnt > 20000) && !LVCMP_DET_GET()) {
        is_sleep_cnt = 0;
        if (current_vbat >= battery_calc_voltage(100)) {
            current_soc = 100;
        } else if (current_vbat <= battery_calc_voltage(0)) {
            current_soc = 0;
        } else {
            current_soc = ocv2soc;
        }
    } else {// 根据电池状态进行放电或充电运算
        if (!LVCMP_DET_GET()) { // 放电
            //模拟充电电量ma
            I = vbat_predicted_current(current_vbat, current_soc);
            //转mAh积分->百分比
            current_soc += (float)I * VBAT_UPDATE_SEC * 100 / 3600 / BATTERY_CAPACITY;
        } else { // 充电状态
            //模拟充电电量,使用电流镜时，建议按比例获取电流
            float I1 = vbat_predicted_current(current_vbat, current_soc) * BATTERY_CHARGE_K;
            float I2 = adc_get_voltage(AD_CH_PMU_PROGI) * REAL_CHANEG_VAL / 100;
            I = (REAL_CHANGE_PER * I2 + PRED_CHANGE_PER	 * I1) / 100;
            printf("change:I1 :%f I2:%f I:%f ", I1, I2, I);
            //转mAh积分->百分比
            current_soc += (float)I * VBAT_UPDATE_SEC * 100 / 3600 / BATTERY_CAPACITY;
            if (!get_charge_full_flag()) { //未充满
                current_soc = (current_soc > 99.99f) ? 99.99f : current_soc;
            } else {
                current_soc += 1;//快速逼近100%
            }
        }
    }
    //soc边界处理
    if (current_soc < 0) {
        current_soc = 0;
    } else if (current_soc > 100) {
        current_soc = 100;
    }
    //soc2percent
    curr_present_k = current_soc;

    // 防止电压回弹
    if (get_charge_online_flag()) {
        //充电过程中电量下降使用之前电量
        if (curr_present_k < last_present_k) {
            curr_present_k = last_present_k;
        }
    } else {
        //未充电过程中电量上升使用之前电量
        if (curr_present_k >= last_present_k) {
            curr_present_k = last_present_k;
        }
    }

    cur_battery_percent = curr_present_k;
    printf("%s  V:%d(mV) I:%f(mA) SOC:%d(per%f) OCV2SOC:%d(per) last:%d(per) charge_flag:%d is_sleep_cnt:%d",
           __func__, current_vbat, I, cur_battery_percent, current_soc, ocv2soc, last_present_k, get_charge_online_flag(), is_sleep_cnt);
    bat_info_storage_task_sync();
}

void vbat_check_slow(void *priv)
{
    if (vbat_fast_timer == 0) {
        vbat_fast_timer = sys_s_hi_timer_add(NULL, vbat_check, VBAT_DETECT_TIME);
    }
    if (get_charge_online_flag()) {
        sys_s_hi_timer_modify(vbat_slow_timer, VBAT_UPDATE_SLOW_TIME);
    } else {
        sys_s_hi_timer_modify(vbat_slow_timer, VBAT_UPDATE_FAST_TIME);
    }
}

void vbat_check_init(void)
{
    u8 tmp[128] = {0};
    int i;
    u16 battery_0, battery_100;
    union battery_data battery_data_t;

    //初始化电池曲线
    if (battery_curve_p == NULL) {
        memset(tmp, 0x00, sizeof(tmp));
        int ret = syscfg_read(CFG_BATTERY_CURVE_ID, tmp, sizeof(tmp));
        if (ret > 0) {
            battery_curve_max = ret / sizeof(battery_data_t);
        } else {
            battery_curve_max = 2;
        }
        battery_curve_p = malloc(battery_curve_max * sizeof(struct battery_curve));
        ASSERT(battery_curve_p, "malloc battery_curve err!");
        if (ret < 0) {
            log_error("battery curve id, ret: %d\n", ret);
            battery_0 = app_var.poweroff_tone_v;
#if TCFG_CHARGE_ENABLE
            //防止部分电池充不了这么高电量，充满显示未满的情况
            battery_100 = (get_charge_full_value() - 100);
#else
            battery_100 = 4100;
#endif
            battery_curve_p[0].percent = 0;
            battery_curve_p[0].voltage = battery_0;
            battery_curve_p[1].percent = 100;
            battery_curve_p[1].voltage = battery_100;
            log_info("percent: %d, voltage: %d mV", 0, battery_curve_p[0].voltage);
            log_info("percent: %d, voltage: %d mV", 100, battery_curve_p[1].voltage);
        } else {
            for (i = 0; i < battery_curve_max; i++) {
                memcpy(&battery_data_t.raw_data,
                       &tmp[i * sizeof(battery_data_t)], sizeof(battery_data_t));
                battery_curve_p[i].percent = battery_data_t.data.percent;
                battery_curve_p[i].voltage = battery_data_t.data.voltage;
                printf("percent: %d, voltage: %d mV\n",
                       battery_curve_p[i].percent, battery_curve_p[i].voltage);
            }
        }
        //初始化相关变量
        cur_battery_voltage = get_vbat_voltage();

        u16 tmp_percent = battery_calc_percent(cur_battery_voltage);
        int check_ret = bat_info_need_recheck(tmp_percent);
        if (check_ret == true) {
            cur_battery_percent = tmp_percent;
            current_soc  = cur_battery_percent;
            printf("%s %d", __func__, cur_battery_percent);
            cur_battery_level = battery_value_to_phone_level();
        }
    }
#if (VBAT_PERCENT_MODE_SEL == VBAT_PERCENT_MODE_CURRENT_INTEGRATING_METHOD)
    sys_s_hi_timer_add(NULL, vbat_calculate_loop, VBAT_UPDATE_SEC * 1000);
#endif
    if (vbat_slow_timer == 0) {
        vbat_slow_timer = sys_s_hi_timer_add(NULL, vbat_check_slow, VBAT_UPDATE_FAST_TIME);
    } else {
        sys_s_hi_timer_modify(vbat_slow_timer, VBAT_UPDATE_FAST_TIME);
    }

    if (vbat_fast_timer == 0) {
        vbat_fast_timer = sys_s_hi_timer_add(NULL, vbat_check, VBAT_DETECT_TIME);
    }
}

void vbat_timer_delete(void)
{
    if (vbat_slow_timer) {
        sys_s_hi_timer_del(vbat_slow_timer);
        vbat_slow_timer = 0;
    }
    if (vbat_fast_timer) {
        sys_s_hi_timer_del(vbat_fast_timer);
        vbat_fast_timer = 0;
    }
}

void vbat_check(void *priv)
{
    static u8 unit_cnt = 0;
    static u8 low_voice_cnt = 0;
    static u8 low_power_cnt = 0;
    static u8 power_normal_cnt = 0;
    static u8 charge_online_flag = 0;
    static u8 low_voice_first_flag = 1;//进入低电后先提醒一次
    static u32 bat_voltage = 0;
    u16 tmp_percent;

    bat_voltage += get_vbat_voltage();
    unit_cnt++;
    if (unit_cnt < VBAT_DETECT_CNT) {
        return;
    }
    unit_cnt = 0;

    //更新电池电压,以及电池百分比,还有电池等级
    cur_battery_voltage = bat_voltage / VBAT_DETECT_CNT;
    bat_voltage = 0;
    tmp_percent = battery_calc_percent(cur_battery_voltage);
#if (VBAT_PERCENT_MODE_SEL == VBAT_PERCENT_MODE_OPEN_CIRCUIT_VOLTAGE)
    if (get_charge_online_flag()) {
        if (tmp_percent > cur_battery_percent) {
            cur_battery_percent++;
        }
    } else {
        if (tmp_percent < cur_battery_percent) {
            cur_battery_percent--;
        }
    }
    bat_info_storage_task_sync();
#endif
    cur_battery_level = battery_value_to_phone_level();

    printf("cur_voltage: %d mV, tmp_percent: %d, cur_percent: %d, cur_level: %d\n",
           cur_battery_voltage, tmp_percent, cur_battery_percent, cur_battery_level);

    if (get_charge_online_flag() == 0) {
        if (adc_check_vbat_lowpower() ||
            (cur_battery_voltage <= app_var.poweroff_tone_v)) { //低电关机
            low_power_cnt++;
            low_voice_cnt = 0;
            power_normal_cnt = 0;
            cur_bat_st = VBAT_LOWPOWER;
            if (low_power_cnt > 6) {
                log_info("\n*******Low Power,enter softpoweroff********\n");
                low_power_cnt = 0;
                batmgr_send_msg(POWER_EVENT_POWER_LOW, 0);
                sys_s_hi_timer_del(vbat_fast_timer);
                vbat_fast_timer = 0;
            }
        } else if (cur_battery_voltage <= app_var.warning_tone_v) { //低电提醒
            low_voice_cnt ++;
            low_power_cnt = 0;
            power_normal_cnt = 0;
            cur_bat_st = VBAT_WARNING;
            if ((low_voice_first_flag && low_voice_cnt > 1) || //第一次进低电10s后报一次
                (!low_voice_first_flag && low_voice_cnt >= 5)) {
                low_voice_first_flag = 0;
                low_voice_cnt = 0;
#if(TCFG_SYS_LVD_EN == 1)
                if (!lowpower_timer) {
                    log_info("\n**Low Power,Please Charge Soon!!!**\n");
                    batmgr_send_msg(POWER_EVENT_POWER_WARNING, 0);
                }
#endif
            }
        } else {
            power_normal_cnt++;
            low_voice_cnt = 0;
            low_power_cnt = 0;
            if (power_normal_cnt > 2) {
                if (cur_bat_st != VBAT_NORMAL) {
                    log_info("[Noraml power]\n");
                    cur_bat_st = VBAT_NORMAL;
                    batmgr_send_msg(POWER_EVENT_POWER_NORMAL, 0);
                }
            }
        }
    } else {
        batmgr_send_msg(POWER_EVENT_POWER_CHARGE, 0);
    }

    if (cur_bat_st != VBAT_LOWPOWER) {
        sys_s_hi_timer_del(vbat_fast_timer);
        vbat_fast_timer = 0;
        //电量等级变化,或者在仓状态变化,交换电量
        if ((cur_battery_level != old_battery_level) ||
            (charge_online_flag != get_charge_online_flag()) ||
            (cur_battery_percent != old_battery_percent)) {
            batmgr_send_msg(POWER_EVENT_POWER_CHANGE, 0);
        }
        charge_online_flag =  get_charge_online_flag();
        old_battery_level = cur_battery_level;
        old_battery_percent = cur_battery_percent;
    }
    UI_MSG_POST("batcharge:process=%4",  cur_battery_percent);

}

bool vbat_is_low_power(void)
{
    return (cur_bat_st != VBAT_NORMAL);
}

void check_power_on_voltage(void)
{
#if(TCFG_SYS_LVD_EN == 1)

    u16 val = 0;
    u8 normal_power_cnt = 0;
    u8 low_power_cnt = 0;

    while (1) {
        clr_wdt();
        val = get_vbat_voltage();
        printf("vbat: %d\n", val);
        if ((val < app_var.poweroff_tone_v) || adc_check_vbat_lowpower()) {
            low_power_cnt++;
            normal_power_cnt = 0;
            if (low_power_cnt > 10) {
                /* ui_update_status(STATUS_POWERON_LOWPOWER); */
                os_time_dly(100);
                log_info("power on low power , enter softpoweroff!\n");
                power_set_soft_poweroff();
            }
        } else {
            normal_power_cnt++;
            low_power_cnt = 0;
            if (normal_power_cnt > 10) {
                vbat_check_init();
                return;
            }
        }
    }
#endif
}
u8 check_vbat_low_power(void)
{
    int ret = 0;
#if(TCFG_SYS_LVD_EN == 1)
    u16 val = 0;
    u8 normal_power_cnt = 0;
    u8 low_power_cnt = 0;

    while (1) {
        clr_wdt();
        val = get_vbat_voltage();
        printf("vbat: %d\n", val);
        if ((val < app_var.poweroff_tone_v) || adc_check_vbat_lowpower()) {
            low_power_cnt++;
            normal_power_cnt = 0;
            if (low_power_cnt > 10) {
                /* ui_update_status(STATUS_POWERON_LOWPOWER); */
                os_time_dly(100);
                ret = 1;
                /* printf("%s low_power",__func__); */
                return ret;
            }
        } else {
            normal_power_cnt++;
            low_power_cnt = 0;
            if (normal_power_cnt > 10) {
                /* printf("%s power_on",__func__); */
                ret = 0;
                return ret;
            }
        }
    }
#endif
    return ret;
}
