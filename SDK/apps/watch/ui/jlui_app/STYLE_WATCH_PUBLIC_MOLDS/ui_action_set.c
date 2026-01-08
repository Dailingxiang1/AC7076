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
#include "system/includes.h"
#include "audio_config.h"
#include "asm/mcpwm.h"
#include "jlui_app/ui_sys_param.h"
#include "jlui_app/watch_syscfg_manage.h"
#include "font/language_list.h"
#include "bt_common.h"
#include "btstack/btstack_task.h"
#include "btstack/avctp_user.h"
#include "btstack/third_party/rcsp/btstack_rcsp_user.h"
#include "custom_cfg.h"
#include "syscfg_id.h"
#include "ui_page_switch.h"
#include "rtc.h"
#include "app_main.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_SET]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_set.data.bss")
#pragma data_seg(".ui_action_set.data")
#pragma const_seg(".ui_action_set.text.const")
#pragma code_seg(".ui_action_set.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_SYS_SET

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)


#define 	PAGE_SET						ID_WINDOW_SETTING
#define 	PAGE_UNDISTURB_MODE			    ID_WINDOW_UNDISTURB
#define 	PAGE_ABOUT						ID_WINDOW_ABOUT
#define 	PAGE_HEALTH_TIPS			    ID_WINDOW_HEALTH_TIPS
#define 	PAGE_SAVE_POWER_MODE			ID_WINDOW_LOW_POWER
#define 	PAGE_PASSWORD                   ID_WINDOW_SETTING_PASSWORD
#define     PAGE_APP_QR                     ID_WINDOW_APP_QRCODE
#define     PAGE_SHUTDOWN                   ID_WINDOW_SHUTDOWN
#define     PAGE_REBOOT                     ID_WINDOW_REBOOT
#define     PAGE_BACK2FACTORY               ID_WINDOW_BACK2FACTORY
#define     PAGE_BRIGHTNESS                 ID_WINDOW_SETTING_BRIGHTNESS
#define     PAGE_VOICE                      ID_WINDOW_SETTING_VOICE
#define     PAGE_DISCONN                    ID_WINDOW_CONN_NEW_PHONE
#define     PAGE_COMMON                     ID_WINDOW_SETTING_COMMON
#define     PAGE_COMPONENT                  ID_WINDOW_COMPONENT
#define     PAGE_LOW_POWER_TIPS             ID_WINDOW_LOW_POWER_TIPS
#define     PAGE_FINDMY                    	ID_WINDOW_FINDMY
#define     PAGE_BT_EMITTER                 ID_WINDOW_BT_EMITTER


#define SET_VLIST_LOOP_EN         1    // 列表循环

struct set_func_t {
    u8 is_24_hour;   // 24小时制
    u8 raise_hand_index;    // 抬手亮屏
    u8 screen_off_index;    // 熄屏表盘
    u8 bed_light_index;     // 床头灯
    u8 health_tip;      // 久坐提醒
    u8 low_power_mode;   // 省电模式
};
static struct set_func_t sec_func = {.is_24_hour = 1};

extern int UIInfo_w_vm_timer;

u32 ui_show_page_list[14] = {ID_WINDOW_DIAL};
int card_start_index = 0;
const u32 ui_page_list[] = {
    /* 0 */ID_WINDOW_HEART,
    /* 1 */ID_WINDOW_MUSIC_PLAYER,
    /* 2 */ID_WINDOW_BREATH_TRAIN,
    /* 3 */ID_WINDOW_SLEEP,
    /* 4 */ID_WINDOW_WEATHER,
    /* 5 */ID_WINDOW_CALENDAR,
    /* 6 */ID_WINDOW_CALCULATOR,
    /* 7 */ID_WINDOW_OXYGEN,
    ///* 8 */ID_WINDOW_BLOODPRESSURE,
    /* 9 */ID_WINDOW_HEAT,
    /* 10 */ID_WINDOW_COMPASS,
    ///* 11 */ID_WINDOW_TIMER,
    /* 12 */ID_WINDOW_STOPWATCH,
    /* 13 */ID_WINDOW_ALIPAY,
    /* 14 */ID_WINDOW_RUNLIGHT,
};

extern u8 is_bredr_close(void);
extern void bredr_conn_last_dev();
extern void bt_close_bredr();
extern u8 bt_ble_get_adv_enable(void);
void save_ui_info_to_vm();


u8 get_is_24_hour(void)
{
    return get_ui_sys_param(twenty_four_time);
}
void set_is_24_hour(int flag)
{
    set_ui_sys_param(twenty_four_time, flag);
}

u8 get_is_raise_hand(void)
{
    return get_ui_sys_param(raise_hand_screen_on);
}
void set_is_raise_hand(int flag)
{
    set_ui_sys_param(raise_hand_screen_on, flag);
}

u8 get_is_low_power_mode(void)
{
    return get_ui_sys_param(low_power_mode);
}
void set_is_low_power_mode(int flag)
{
    set_ui_sys_param(low_power_mode, flag);
}

static void get_sys_time(struct sys_time *time)
{
    rtc_read_time(time);
}

static void set_sys_time(struct sys_time *time)
{
    rtc_write_time(time);
}

/*进入省电模式*/
void enter_low_power_mode()
{
    u8 brightness_level = get_ui_sys_param(LightLevel);
    set_is_low_power_mode(1);
    set_ui_sys_param(LastLightLevel, brightness_level);
    set_ui_sys_param(LightLevel, MIN_LIGHTLEVEL);
    ui_ajust_light(MIN_LIGHTLEVEL * 2);

    u8 time_val = get_ui_sys_param(DarkTime);
    set_ui_sys_param(LastDarkTime, time_val);
    set_ui_sys_param(DarkTime, 0);
    ui_set_dark_time(get_ui_sys_param(DarkTime));

    save_ui_info_to_vm();

#if (BT_AI_SEL_PROTOCOL & RCSP_MODE_EN)
    log_info("%s [%d %d %d]", __func__, time_val, brightness_level, bt_rcsp_device_conn_num());

    /*与手机已经连接了，不去操作蓝牙了*/
    if (bt_rcsp_device_conn_num() > 0) {
        return;
    }

    if (bt_ble_get_adv_enable()) {
        ble_module_enable(0);
    }

#if TCFG_USER_BT_CLASSIC_ENABLE
    if (!is_bredr_close()) {
        bt_close_bredr();
    }
#endif
#endif

}

/*退出省电模式*/
void exit_low_power_mode()
{
    u8 brightness_level = get_ui_sys_param(LastLightLevel);
    set_is_low_power_mode(0);
    set_ui_sys_param(LightLevel, brightness_level);
    ui_ajust_light(brightness_level * 2);

    u8 time_val = get_ui_sys_param(LastDarkTime);
    set_ui_sys_param(DarkTime, time_val);
    ui_set_dark_time(get_ui_sys_param(DarkTime));

    save_ui_info_to_vm();

#if (BT_AI_SEL_PROTOCOL & RCSP_MODE_EN)
    log_info("%s [%d %d %d]", __func__, time_val, brightness_level, bt_rcsp_device_conn_num());
    /*与手机已经连接了，不去操作蓝牙了*/
    if (bt_rcsp_device_conn_num() > 0) {
        return;
    }

    if (!bt_ble_get_adv_enable()) {
        ble_module_enable(1);
    }
#endif
}

static void setting_write_UIInfo_to_vm(void *info)
{
    int ret = 0;
    if (UIInfo_w_vm_timer != 0) {
        sys_timer_del(UIInfo_w_vm_timer);
        UIInfo_w_vm_timer = 0;
    }
    ui_sys_param.valid = 1;
    if ((int)info == (int)SYSCFG_WRITE_ERASE_STATUS) {
        ui_sys_param.valid = 0;
    }
    ret = syscfg_write(VM_UI_SYS_INFO, &ui_sys_param, sizeof(ui_sys_param));
    if (ret != sizeof(ui_sys_param)) {
        printf("write ui_sysinfo VM err\n");
    }
}
void save_ui_info_to_vm()
{
    if (UIInfo_w_vm_timer == 0) {
        UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
    } else {
        sys_timer_re_run(UIInfo_w_vm_timer);
    }
}

int ui_core_get_rtc_time(struct ui_time *time)    // 用于显示rtc时间
{
    struct sys_time sys_time = {0};

    rtc_read_time(&sys_time);

    time->year = sys_time.year;
    time->month = sys_time.month;
    time->day = sys_time.day;
    if (!get_is_24_hour()) {
        if (sys_time.hour > 12) {
            ui_text_show_index_by_id(SETTING_TIME_AMPM_TEXT, 1);
            time->hour = sys_time.hour - 12;
        } else if (sys_time.hour == 12) {
            ui_text_show_index_by_id(SETTING_TIME_AMPM_TEXT, 1);
            time->hour = sys_time.hour;
        } else {
            ui_text_show_index_by_id(SETTING_TIME_AMPM_TEXT, 0);
            time->hour = sys_time.hour;
        }
    } else {
        time->hour = sys_time.hour;
    }
    time->min = sys_time.min;
    time->sec = sys_time.sec;

    return 0;
}

static void ui_default_param_set()
{
    u8 i, k;
    u8 card_set_num = 0;

    if (ui_page_list[0] != 0) {
        ui_sys_param.card_select[card_set_num++] = 0;
    }
    if (ui_page_list[3] != 0) {
        ui_sys_param.card_select[card_set_num++] = 3;
    }
    if (ui_page_list[4] != 0) {
        ui_sys_param.card_select[card_set_num++] = 4;
    }
    if (ui_page_list[5] != 0) {
        ui_sys_param.card_select[card_set_num++] = 5;
    }

    set_ui_sys_param(CardSetNum, card_set_num);
}


void ui_sysinfo_init()
{
    u8 card_set_num;
    int ret;
    ret = read_UIInfo_from_vm();
    if (ret < 0) {
        log_info("read_UIInfo_from_vm err");
        ui_default_param_set();
    }

    ui_moto_init(TCFG_MOTO_PWM_IO);
    ui_set_voice(get_ui_sys_param(LastSysVol));
    ui_ajust_light(get_ui_sys_param(LightLevel) * 2);
    ui_set_dark_time(get_ui_sys_param(DarkTime));
    card_set_num = get_ui_sys_param(CardSetNum);

    for (card_start_index = 0; card_start_index < sizeof(ui_show_page_list) / sizeof(ui_show_page_list[0]); card_start_index++) {
        if (ui_show_page_list[card_start_index] == 0) {
            break;
        }
    }

    if (card_set_num != 0) {
        for (int i = 0; i < card_set_num; i++) {
            ui_show_page_list[i + card_start_index] = ui_page_list[ui_sys_param.card_select[i]];
            log_info("page:0x%x %d", ui_show_page_list[i + card_start_index], ui_sys_param.card_select[i]);
            ui_page_add(ui_page_list[ui_sys_param.card_select[i]]);
        }
    }

    ui_page_list_update(ui_show_page_list, card_set_num + card_start_index);
    watch_set_style(get_ui_sys_param(curr_sel_dial));

}
static int setting_vlist_default_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static void key_redraw(void *ctrl)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    ui_core_redraw(grid);
}

static int default_vlist_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int index = ui_grid_get_hindex_dynamic(grid);
    struct rect r;

    switch (e->value) {
    case KEY_UI_PLUS:
        printf("@@@@@ plus\n");
        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        ui_grid_slide_with_callback_dynamic(grid, SCROLL_DIRECTION_UD, -(r.height + grid->y_interval), key_redraw);
        return true;
        break;
    case KEY_UI_MINUS:
        printf("@@@@@ minus\n");
        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        ui_grid_slide_with_callback_dynamic(grid, SCROLL_DIRECTION_UD, (r.height + grid->y_interval), key_redraw);
        return true;
        break;
    default:
        break;
    }

    return false;
}

//------------------------------设置主界面-------------------------------//
static u32 setpage_item_memory = 0;
static void setpage_item_set(struct ui_grid *grid, int item)
{
    if (item >= grid->avail_item_num - 1) {
        ui_grid_set_hi_index(grid, item - 1);
    } else {
        ui_grid_set_hi_index(grid, item + 1);
    }
}

static int set_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        set_ui_sys_param(ConnNewPhone, 0);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        setpage_item_set(grid, setpage_item_memory);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int set_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    static u8 touch_action = 0;
    int sel_item;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        setpage_item_memory = ui_grid_touch_item(grid);
        sel_item = ui_grid_touch_item(grid);
        if (sel_item < 0) {
            break;
        }
        switch (sel_item) {
        case 0:// 通用

            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_COMMON);

            break;
        case 1:// 时间设置
            ui_hide(SETTING_LAYOUT);
            ui_show(SETTING_TIME_SET_LAYOUT);
            break;
        case 2:// 显示与亮度
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_BRIGHTNESS);
            break;
        case 3:// 声音与振动
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_VOICE);
            break;
        case 4:// 勿扰模式
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_UNDISTURB_MODE);
            break;
        case 5:// 健康提醒
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_HEALTH_TIPS);
            break;
        case 6:// 省电模式
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_SAVE_POWER_MODE);
            break;
        case 7:// 密码
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_PASSWORD);
            break;
        case 8:// 组件
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_COMPONENT);
            break;
        case 9:// APP视图
            ui_hide(SETTING_LAYOUT);
            ui_show(SETTING_APP_SHOW_LAYOUT);
            break;
        case 10:// 蓝牙设置
            ui_hide(SETTING_LAYOUT);
            ui_show(SETTING_EDR_LAYOUT);
            break;
        case 11:// findmy
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_FINDMY);
            break;
        case 12:// 蓝牙耳机
#if TCFG_USER_EMITTER_ENABLE
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_BT_EMITTER);
#endif
            break;
        case 13:// 灵动岛

            break;
        case 14:// 转场动画
            ui_hide(SETTING_LAYOUT);
            ui_show(SETTING_ANIMATION_LAYOUT);
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
REGISTER_UI_EVENT_HANDLER(SETTING_VLIST)//设置-垂直列表
.onchange = set_onchange,
 .onkey = NULL,
  .ontouch = set_ontouch,
};

//-----------------------通用页面设置界面---------------------------//
static u32 commonpage_item_memory = 0;
static void commonpage_item_set(struct ui_grid *grid, int item)
{
    if (item >= grid->avail_item_num - 1) {
        ui_grid_set_hi_index(grid, item - 1);
    } else {
        ui_grid_set_hi_index(grid, item + 1);
    }
}

static int set_common_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        commonpage_item_set(grid, commonpage_item_memory);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
static int set_common_vlist_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    static u8 touch_action = 0;
    int sel_item;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        commonpage_item_memory = ui_grid_cur_item(grid);
        sel_item = ui_grid_cur_item(grid);
        if (sel_item < 0) {
            break;
        }
        switch (sel_item) {
        case 0:// 语言设置
            ui_hide(SETTING_COMMON_LAYOUT);
            ui_show(SETTING_LANGUAGE_LAYOUT);
            break;
        case 1:// APP二维码
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_APP_QR);
            break;
        case 2:// 关于
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_ABOUT);
            break;
        case 3:// 关机
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_SHUTDOWN);
            break;
        case 4:// 重启
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_REBOOT);
            break;
        case 5:// 连接新手机
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_DISCONN);
            break;
        case 6:// 恢复出厂
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_BACK2FACTORY);
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

REGISTER_UI_EVENT_HANDLER(SETTING_COMMON_VLIST)//通用-垂直列表
.onchange = set_common_vlist_onchange,
 .onkey = NULL,
  .ontouch = set_common_vlist_ontouch,
};

//-----------------------语言设置界面---------------------------//
static int set_language_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int ui_language = ui_language_get();

    switch (event) {
    case ON_CHANGE_INIT:
        // 获取当前系统语言
        if (ui_language == Chinese_Simplified) {
            ui_show(SETTING_LANGUAGE_CHINESE_PIC);
        } else if (ui_language == English) {
            ui_show(SETTING_LANGUAGE_ENGLISH_PIC);
        }
        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        break;
    }
    return 0;
}

static int set_language_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SETTING_LANGUAGE_LAYOUT);
        ui_show(SETTING_COMMON_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_LANGUAGE_LAYOUT)//通用-垂直列表
.onchange = set_language_layout_onchange,
 .onkey = NULL,
  .ontouch = set_language_layout_ontouch,
};

static int set_language_vlist_ontouch(void *ctr, struct element_touch_event *e)
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
        case 0:// 中文
            // 设置中文接口
            ui_hide(SETTING_LANGUAGE_ENGLISH_PIC);
            ui_show(SETTING_LANGUAGE_CHINESE_PIC);
            ui_language_set(Chinese_Simplified);
            break;
        case 1:// english
            // 设置english接口
            ui_hide(SETTING_LANGUAGE_CHINESE_PIC);
            ui_show(SETTING_LANGUAGE_ENGLISH_PIC);
            ui_language_set(English);
            break;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        return true;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_ENERGY:
        return true;
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_LANGUAGE_VLIST)//通用-垂直列表
.onchange = setting_vlist_default_onchange,
 .onkey = NULL,
  .ontouch = set_language_vlist_ontouch,
};

//-------------------------关于界面---------------------------------//
struct SYS_ABOUT_MESSAGE {
    const char *dev_name;
    char *dev_num;
    char model[10];
    char mac_addr[18];
    char ver[4 + 2];
    char serial_num[32];
};

struct SYS_ABOUT_MESSAGE sys_about_message = {
    .dev_num = "AC701N",
    .model =  "S1 PRO",
    /* .version = "master", */
    /* .serial_num = "ABC1234567890", */
};

static void bt_addr2string(const u8 *addr, char *buf)
{
    u8 len = 0;
    for (s8 i = 5; i >= 0; i--) {
        if ((addr[i] / 16) >= 10) {
            buf[len] = 'A' + addr[i] / 16 - 10;
        } else {
            buf[len] = '0' + addr[i] / 16;
        }
        if ((addr[i] % 16) >= 10) {
            buf[len + 1] = 'A' + addr[i] % 16 - 10;
        } else {
            buf[len + 1] = '0' + addr[i] % 16;
        }
        len += 2;
        buf[len] = ':';
        len += 1;
    }
    buf[len - 1] = '\0';
    log_info("%s", buf);
}

static void version_u16_2_string(char *str, int version)
{
    for (int i = 0; i < 4; i++) {
        u8 tmp = version & 0x0F;
        version = version >> 4;
        if (tmp > 9) {
            str[3 - i] = '-';
        } else {
            str[3 - i] = tmp + '0';
        }
        /* printf("ver:0x%x, tmp:0x%x, str:0x%x \n", version, tmp, str[i]); */
    }
}


static int text_about_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    u16 version = 0;
    u8 serial_len = 0;

    switch (event) {
    case ON_CHANGE_INIT:
        switch (text->elm.id) {
        case ABOUT_NAME_TEXT:
            sys_about_message.dev_name = bt_get_local_name();
            ui_text_set_text_attrs(text, sys_about_message.dev_name, strlen(sys_about_message.dev_name), FONT_ENCODE_UTF8, 0, FONT_DEFAULT);
            break;
        case ABOUT_ADDRESS_TEXT:
            bt_addr2string(bt_get_mac_addr(), sys_about_message.mac_addr);
            ui_text_set_text_attrs(text, sys_about_message.mac_addr, strlen(sys_about_message.mac_addr), FONT_ENCODE_UTF8, 0, FONT_DEFAULT);
            break;
        case ABOUT_VERSION_TEXT:
            version = get_vid_pid_ver_from_cfg_file(GET_VER_FROM_EX_CFG);
            memset(sys_about_message.ver, 0, sizeof(sys_about_message.ver));
            sys_about_message.ver[0] = 'v';
            version_u16_2_string(&sys_about_message.ver[1], version);
            ui_text_set_text_attrs(text, sys_about_message.ver, strlen(sys_about_message.ver), FONT_ENCODE_UTF8, 0, FONT_DEFAULT);
            break;
        case ABOUT_MODEL_TEXT:
            ui_text_set_text_attrs(text, sys_about_message.model, strlen(sys_about_message.model), FONT_ENCODE_UTF8, 0, FONT_DEFAULT);
            break;
        default:
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(ABOUT_NAME_TEXT)
.onchange = text_about_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ABOUT_ADDRESS_TEXT)
.onchange = text_about_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ABOUT_VERSION_TEXT)
.onchange = text_about_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ABOUT_MODEL_TEXT)
.onchange = text_about_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ABOUT_LIST)
.onchange =  setting_vlist_default_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


//-----------------------关机界面---------------------------//
static int shutdown_no_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SHUTDOWN_NO_BUTTON:
            //接口调用
            printf("shutdown no!\n");
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_COMMON);
            UI_WINDOW_BACK_DEL(PAGE_SHUTDOWN);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SHUTDOWN_NO_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = shutdown_no_ontouch,
};

static int shutdown_yes_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SHUTDOWN_YES_BUTTON:
            //接口调用
            printf("shutdown yes!\n");
            watch_reboot_or_shutdown(0, 0);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SHUTDOWN_YES_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = shutdown_yes_ontouch,
};

//-----------------------重启界面---------------------------//
static int reboot_no_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case REBOOT_NO_BUTTON:
            //接口调用
            printf("reboot no!\n");
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_COMMON);
            UI_WINDOW_BACK_DEL(PAGE_REBOOT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(REBOOT_NO_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = reboot_no_ontouch,
};

static int reboot_yes_touch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case REBOOT_YES_BUTTON:
            //接口调用
            printf("reboot yes!\n");
            app_var.goto_reboot_flag = 1;
            watch_reboot_or_shutdown(1, 0);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(REBOOT_YES_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = reboot_yes_touch,
};

//-----------------------连接新手机界面---------------------------//
#if TCFG_USER_BT_CLASSIC_ENABLE
static void ui_disconnect_phone_deal(void)
{
#ifdef CONFIG_APP_BT_ENABLE
    ble_app_disconnect(); // 断开BLE
#endif /*CONFIG_APP_BT_ENABLE*/
    if (bt_get_curr_channel_state() != 0) { // 断开经典蓝牙
        bt_cmd_prepare(USER_CTRL_A2DP_CMD_CLOSE, 0, NULL);
        bt_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
    }
    bt_cmd_prepare(USER_CTRL_DEL_LAST_REMOTE_INFO, 0, NULL);
}

static int disconn_no_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case DISCONN_NO_BUTTON:
            //接口调用
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_COMMON);
            UI_WINDOW_BACK_DEL(PAGE_DISCONN);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(DISCONN_NO_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = disconn_no_ontouch,
};


static int disconn_yes_touch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case DISCONN_YES_BUTTON:
            set_ui_sys_param(ConnNewPhone, 1);
            ui_disconnect_phone_deal();
            ui_hide(DISCONN_LAYOUT);
            ui_show(DISCONN_QR_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(DISCONN_YES_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = disconn_yes_touch,
};


static int disconn_qr_button_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case DISCONN_QR_BUTTON:
            //接口调用
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_APP_QR);
            UI_WINDOW_BACK_DEL(PAGE_DISCONN);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(DISCONN_QR_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = disconn_qr_button_ontouch,
};
#endif /* #if TCFG_USER_BT_CLASSIC_ENABLE */


//-----------------------恢复出厂界面---------------------------//
static int back2factory_no_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case BACK2FACTORY_NO_BUTTON:
            //接口调用
            printf("back2fac no!\n");
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_COMMON);
            UI_WINDOW_BACK_DEL(PAGE_BACK2FACTORY);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(BACK2FACTORY_NO_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = back2factory_no_ontouch,
};

static int back2factory_yes_touch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case BACK2FACTORY_YES_BUTTON:
            //接口调用
            printf("back2fac yes!\n");
            app_var.goto_reboot_flag = 1;
#if TCFG_USER_BT_CLASSIC_ENABLE
            ui_disconnect_phone_deal();
#endif
            restore_sys_settings();
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(BACK2FACTORY_YES_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = back2factory_yes_touch,
};

//-----------------------时间设置界面---------------------------//
struct set_time_t {
    u8 make_sure;
    u8 dynamic_day_num;
    u32 vlist_timer;
    struct sys_time curtime;

};
static struct set_time_t *set_time = NULL;

static int time_set_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SETTING_TIME_SET_LAYOUT);
        ui_show(SETTING_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_LAYOUT)//通用-垂直列表
.onchange = NULL,
 .onkey = NULL,
  .ontouch = time_set_layout_ontouch,
};

static int setting_24_hour_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 is_24_hour = get_ui_sys_param(twenty_four_time);

    switch (event) {
    case ON_CHANGE_INIT:
        switch (pic->elm.id) {
        case SETTING_TIME_SETTING_24_BUTTON:
            ui_pic_set_image_index(pic, is_24_hour);
            break;
        default:
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int setting_24_hour_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;
    struct sys_time show_time = {0};
    struct utime u_time = {0};
    u8 is_24_hour = get_ui_sys_param(twenty_four_time);

    get_sys_time(&show_time);

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SETTING_TIME_SETTING_24_BUTTON:
            is_24_hour = !is_24_hour;
            set_ui_sys_param(twenty_four_time, is_24_hour);
            if (UIInfo_w_vm_timer == 0) {
                UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
            } else {
                sys_timer_re_run(UIInfo_w_vm_timer);
            }
            ui_pic_show_image_by_id(SETTING_TIME_SETTING_24_BUTTON, is_24_hour);
            if (!is_24_hour) {
                if (show_time.hour >= 12) {
                    u_time.hour = show_time.hour - 12;
                    u_time.min = show_time.min;
                    u_time.sec = show_time.sec;
                    ui_time_update_by_id(SETTING_TIME_SETTING_CURTIME, &u_time);
                    ui_text_show_index_by_id(SETTING_TIME_AMPM_TEXT, 1);
                } else {
                    ui_text_show_index_by_id(SETTING_TIME_AMPM_TEXT, 0);
                }
            } else {
                u_time.hour = show_time.hour;
                u_time.min = show_time.min;
                u_time.sec = show_time.sec;
                ui_time_update_by_id(SETTING_TIME_SETTING_CURTIME, &u_time);
                ui_hide(SETTING_TIME_AMPM_TEXT);
            }
            // 24小时制
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SETTING_24_BUTTON)
.onchange = setting_24_hour_onchange,
 .onkey = NULL,
  .ontouch = setting_24_hour_ontouch,
};

static int setting_time_curtime_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_time *time = (struct ui_time *)_ctrl;
    struct sys_time get_time = {0};
    struct utime show_time = {0};

    get_sys_time(&get_time);

    show_time.hour = get_time.hour;
    show_time.min = get_time.min;
    show_time.sec = get_time.sec;

    switch (event) {
    case ON_CHANGE_INIT:
        if (!get_ui_sys_param(twenty_four_time)) {
            if (show_time.hour > 12) {
                show_time.hour -= 12;
            }
            ui_time_update(time, &show_time);
        } else {
            ui_time_update(time, &show_time);
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SETTING_CURTIME)
.onchange = setting_time_curtime_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int setting_time_ampm_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    struct sys_time show_time = {0};

    get_sys_time(&show_time);

    switch (event) {
    case ON_CHANGE_INIT:
        if (!get_ui_sys_param(twenty_four_time)) {
            ui_show(SETTING_TIME_AMPM_TEXT);
            if (show_time.hour >= 12 && show_time.hour < 24) {
                ui_text_set_index(text, 1);
            } else {
                ui_text_set_index(text, 0);
            }
        } else {
            ui_hide(SETTING_TIME_AMPM_TEXT);
        }
        break;
    case ON_CHANGE_SHOW_PROBE:
        if (show_time.hour >= 12 && show_time.hour < 24) {
            ui_text_set_index(text, 1);
        } else {
            ui_text_set_index(text, 0);
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_AMPM_TEXT)
.onchange = setting_time_ampm_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int setting_time_next_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SETTING_TIME_SETTING_NEXT_BUTTON:
            if (get_ui_sys_param(twenty_four_time)) {
                ui_hide(SETTING_TIME_SET_LAYOUT);
                ui_show(SETTING_TIME_SET_24_LAYOUT);
            } else {
                ui_hide(SETTING_TIME_SET_LAYOUT);
                ui_show(SETTING_TIME_SET_12_LAYOUT);
            }
            break;
        case SETTING_DATE_SETTING_NEXT_BUTTON:
            ui_hide(SETTING_TIME_SET_LAYOUT);
            ui_show(SETTING_DATE_SET_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SETTING_NEXT_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_time_next_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_SETTING_NEXT_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_time_next_ontouch,
};

static int time_set_24_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SETTING_TIME_SET_24_LAYOUT);
        ui_show(SETTING_TIME_SET_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_24_LAYOUT)//通用-垂直列表
.onchange = NULL,
 .onkey = NULL,
  .ontouch = time_set_24_layout_ontouch,
};

static int time_set_date_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SETTING_DATE_SET_LAYOUT);
        ui_show(SETTING_TIME_SET_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_DATE_SET_LAYOUT)//通用-垂直列表
.onchange = NULL,
 .onkey = NULL,
  .ontouch = time_set_date_layout_ontouch,
};

static int setting_time_24_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SETTING_TIME_SET_24_BACK_BUTTON:
            ui_hide(SETTING_TIME_SET_24_LAYOUT);
            ui_show(SETTING_TIME_SET_LAYOUT);
            break;
        case SETTING_DATE_BACK_BUTTON:
            ui_hide(SETTING_DATE_SET_LAYOUT);
            ui_show(SETTING_TIME_SET_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_24_BACK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_time_24_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_BACK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_time_24_ontouch,
};

static int set_time_hour_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_hour;

    switch (event) {
    case ON_CHANGE_INIT:

#if SET_VLIST_LOOP_EN
        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        get_sys_time(&(set_time->curtime));

        int base = 10000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 24;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        time_hour = set_time->curtime.hour;
        time_hour = time_hour % 24;

        base = (base / 2) * 24;

        ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

        base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_hour == 0) ? r.height + 5 : 0;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);
#else
        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        get_sys_time(&(set_time->curtime));
        row = 24;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        printf("dynamic_grid %d X %d\n", row, col);

        time_hour = set_time->curtime.hour;

        if (time_hour == 0) {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 0);
        } else if (time_hour == 23) {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 3);
        } else if (time_hour == 22) {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 2);
        } else {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 1);
        }
        base_index_once = (time_hour >= 1) ? (time_hour - 1) : 0;

        if (time_hour == 23) {
            base_index_once = time_hour - 3;
        } else if (time_hour == 22) {
            base_index_once = time_hour - 2;
        }

        ui_grid_set_base_dynamic(grid, base_index_once, 0);
#endif
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        if (set_time->make_sure) {
            if (grid->elm.id == SETTING_TIME_SET_24_HOUR_VLIST) {
                set_time->curtime.hour = ui_grid_get_hindex_dynamic(grid) % 24;
                set_sys_time(&(set_time->curtime));
            }
        }
        if (set_time) {
            free(set_time);
            set_time = NULL;
        }
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_24_HOUR_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_time_hour_onchange,
 .onkey = default_vlist_onkey,
  .ontouch = NULL,
};

static int set_time_min_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_min;

    switch (event) {
    case ON_CHANGE_INIT:
#if SET_VLIST_LOOP_EN
        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        get_sys_time(&(set_time->curtime));

        int base = 10000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 60;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        time_min = set_time->curtime.min;
        time_min = time_min % 60;

        base = (base / 2) * 60;

        ui_grid_set_hindex_dynamic(grid, time_min + base, true, 1);

        base_index_once = ((time_min >= 1) ? (time_min - 1) : 0) + base;

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_min == 0) ? r.height + 5 : 0;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);
#else
        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        get_sys_time(&(set_time->curtime));
        row = 60;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        time_min = set_time->curtime.min;

        if (time_min == 0) {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 0);
        } else if (time_min == 59) {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 3);
        } else if (time_min == 58) {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 2);
        } else {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 1);
        }

        base_index_once = (time_min >= 1) ? (time_min - 1) : 0;

        if (time_min == 59) {
            base_index_once = time_min - 3;
        } else if (time_min == 58) {
            base_index_once = time_min - 2;
        }

        ui_grid_set_base_dynamic(grid, base_index_once, 0);
#endif
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        if (set_time->make_sure) {
            if (grid->elm.id == SETTING_TIME_SET_24_MIN_VLIST) {
                set_time->curtime.min = ui_grid_get_hindex_dynamic(grid) % 60;
                set_sys_time(&(set_time->curtime));
            }
        }
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_24_MIN_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_time_min_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int set_time_ok_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_text *text = (struct ui_text *)ctr;

    u8 start_hour;
    u8 start_min;
    u8 end_hour;
    u8 end_min;
    u32 start_all_time;
    u32 end_all_time;
    u8 new_hour;
    u8 new_min;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        set_time->make_sure = 1;
        if (text->elm.id == SETTING_TIME_SET_24_SURE_BUTTON) {
            // 设置时间
            ui_hide(SETTING_TIME_SET_24_LAYOUT);
            ui_show(SETTING_TIME_SET_LAYOUT);
        }
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_24_SURE_BUTTON)//开始时间-文字控件(确定)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = set_time_ok_ontouch,
};

static int set_time_h_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_SHOW:
        //局部高亮demo，（不支持缩放）
        struct draw_context *dc = (struct draw_context *)arg;
        struct rect r_no_high_1 = {//非高亮区域1
            .left = 55,
            .top = 105,
            .width = 79,
            .height = 47,
        };
        struct rect r_no_high_2 = {//非高亮区域2
            .left = 55,
            .top = 105 + 2 * 47,
            .width = 79,
            .height = 47,
        };
        struct rect r_high = {//中间高亮区域
            .left = 55,
            .top = 105 + 1 * 47,
            .width = 79,
            .height = 47,
        };
        struct rect r_cover;
        switch (pic->elm.id) {
        //非高亮控件
        case SETTING_TIME_H00:
        case SETTING_TIME_H10:
        case SETTING_TIME_H20:
        case SETTING_TIME_H30:
        case SETTING_TIME_H01:
        case SETTING_TIME_H11:
        case SETTING_TIME_H21:
        case SETTING_TIME_H31:
            if (get_rect_cover(&dc->draw, &r_no_high_1, &r_cover)) {//取交集给draw
                memcpy(&dc->draw, &r_cover, sizeof(struct rect));
                return false;
            }
            if (get_rect_cover(&dc->draw, &r_no_high_2, &r_cover)) {//取交集给draw
                memcpy(&dc->draw, &r_cover, sizeof(struct rect));
                return false;
            }
            //高亮区域不显示
            dc->draw.width = 0;
            break;
        //高亮控件
        case SETTING_TIME_HL_H00:
        case SETTING_TIME_HL_H10:
        case SETTING_TIME_HL_H20:
        case SETTING_TIME_HL_H01:
        case SETTING_TIME_HL_H11:
        case SETTING_TIME_HL_H21:
            if (get_rect_cover(&dc->draw, &r_high, &r_cover)) {//取交集给draw
                memcpy(&dc->draw, &r_cover, sizeof(struct rect));
                return false;
            } else {
                //非高亮区域不显示
                dc->draw.width = 0;
            }
            break;
        }
        break;

    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 24;//这里必须求余数方式获取索引
        /* printf("tid %d\n", index); */
        if ((index < 0) || (index > 23)) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_TIME_H00:
        case SETTING_TIME_H10:
        case SETTING_TIME_H20:
        case SETTING_TIME_H30:
        case SETTING_TIME_HL_H00:
        case SETTING_TIME_HL_H10:
        case SETTING_TIME_HL_H20:
            index_buf = index / 10;
            break;
        case SETTING_TIME_H01:
        case SETTING_TIME_H11:
        case SETTING_TIME_H21:
        case SETTING_TIME_H31:
        case SETTING_TIME_HL_H01:
        case SETTING_TIME_HL_H11:
        case SETTING_TIME_HL_H21:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_H00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_H10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_H20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_H30)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_H01)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_H11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_H21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_H31)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_H00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_H10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_H20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_H01)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_H11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_H21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int set_time_m_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_SHOW:
        //局部高亮demo，（不支持缩放）
        struct draw_context *dc = (struct draw_context *)arg;
        struct rect r_no_high_1 = {//非高亮区域1
            .left = 189,
            .top = 105,
            .width = 79,
            .height = 47,
        };
        struct rect r_no_high_2 = {//非高亮区域2
            .left = 189,
            .top = 105 + 2 * 47,
            .width = 79,
            .height = 47,
        };
        struct rect r_high = {//中间高亮区域
            .left = 189,
            .top = 105 + 1 * 47,
            .width = 79,
            .height = 47,
        };
        struct rect r_cover;
        switch (pic->elm.id) {
        //非高亮控件
        case SETTING_TIME_M00:
        case SETTING_TIME_M10:
        case SETTING_TIME_M20:
        case SETTING_TIME_M30:
        case SETTING_TIME_M01:
        case SETTING_TIME_M11:
        case SETTING_TIME_M21:
        case SETTING_TIME_M31:
            if (get_rect_cover(&dc->draw, &r_no_high_1, &r_cover)) {//取交集给draw
                memcpy(&dc->draw, &r_cover, sizeof(struct rect));
                return false;
            }
            if (get_rect_cover(&dc->draw, &r_no_high_2, &r_cover)) {//取交集给draw
                memcpy(&dc->draw, &r_cover, sizeof(struct rect));
                return false;
            }
            //高亮区域不显示
            dc->draw.width = 0;
            break;
        //高亮控件
        case SETTING_TIME_HL_M00:
        case SETTING_TIME_HL_M10:
        case SETTING_TIME_HL_M20:
        case SETTING_TIME_HL_M01:
        case SETTING_TIME_HL_M11:
        case SETTING_TIME_HL_M21:
            if (get_rect_cover(&dc->draw, &r_high, &r_cover)) {//取交集给draw
                memcpy(&dc->draw, &r_cover, sizeof(struct rect));
                return false;
            } else {
                //非高亮区域不显示
                dc->draw.width = 0;
            }
            break;
        }
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 60;//这里必须求余数方式获取索引
        /* log_info("tid %d\n", index); */
        if ((index < 0) || (index > 59)) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_TIME_M00:
        case SETTING_TIME_M10:
        case SETTING_TIME_M20:
        case SETTING_TIME_M30:

        case SETTING_TIME_HL_M00:
        case SETTING_TIME_HL_M10:
        case SETTING_TIME_HL_M20:
            index_buf = index / 10;
            break;
        case SETTING_TIME_M01:
        case SETTING_TIME_M11:
        case SETTING_TIME_M21:
        case SETTING_TIME_M31:

        case SETTING_TIME_HL_M01:
        case SETTING_TIME_HL_M11:
        case SETTING_TIME_HL_M21:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_M00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_M10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_M20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_M30)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_M01)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_M11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_M21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_M31)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_M00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_M10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_M20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_M01)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_M11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_HL_M21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



static int time_set_12_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SETTING_TIME_SET_12_LAYOUT);
        ui_show(SETTING_TIME_SET_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_12_LAYOUT)//通用-垂直列表
.onchange = NULL,
 .onkey = NULL,
  .ontouch = time_set_12_layout_ontouch,
};

static int setting_time_12_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SETTING_TIME_SET_12_BACK_BUTTON:
            ui_hide(SETTING_TIME_SET_12_LAYOUT);
            ui_show(SETTING_TIME_SET_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_12_BACK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_time_12_ontouch,
};

static int set_time_12_hour_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    struct ui_grid *ampm_grid = NULL;
    int row, col;
    int base_index_once;
    int time_hour;
    int hour;
    int ampm_index;

    switch (event) {
    case ON_CHANGE_INIT:

#if SET_VLIST_LOOP_EN
        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        get_sys_time(&(set_time->curtime));

        int base = 10000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 12;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        time_hour = set_time->curtime.hour;
        time_hour = time_hour % 13;

        base = (base / 2) * 13;

        ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

        base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_hour == 0) ? r.height + 5 : 0;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);
#else
        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        get_sys_time(&(set_time->curtime));
        row = 12;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        printf("dynamic_grid %d X %d\n", row, col);

        time_hour = set_time->curtime.hour;

        if (time_hour == 0) {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 0);
        } else if (time_hour == 12) {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 3);
        } else if (time_hour == 11) {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 2);
        } else {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 1);
        }
        base_index_once = (time_hour >= 1) ? (time_hour - 1) : 0;

        if (time_hour == 12) {
            base_index_once = time_hour - 3;
        } else if (time_hour == 11) {
            base_index_once = time_hour - 2;
        }

        ui_grid_set_base_dynamic(grid, base_index_once, 0);
#endif
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        if (set_time->make_sure) {
            ampm_grid = ui_grid_for_id(SETTING_TIME_SET_AMPM_HOUR_VLIST);
            if (ampm_grid) {
                ampm_index = ui_grid_get_hindex_dynamic(ampm_grid) % 2;
            }
            if (grid->elm.id == SETTING_TIME_SET_12_HOUR_VLIST) {
                hour = ui_grid_get_hindex_dynamic(grid) % 13;
                set_time->curtime.hour = ampm_index * 12 + hour;
                set_sys_time(&(set_time->curtime));
            }
        }
        if (set_time) {
            free(set_time);
            set_time = NULL;
        }
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_12_HOUR_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_time_12_hour_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int set_time_12_min_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_min;

    switch (event) {
    case ON_CHANGE_INIT:
#if SET_VLIST_LOOP_EN
        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        get_sys_time(&(set_time->curtime));

        int base = 10000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 60;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        time_min = set_time->curtime.min;
        time_min = time_min % 60;

        base = (base / 2) * 60;

        ui_grid_set_hindex_dynamic(grid, time_min + base, true, 1);

        base_index_once = ((time_min >= 1) ? (time_min - 1) : 0) + base;

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_min == 0) ? r.height + 5 : 0;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);
#else
        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        get_sys_time(&(set_time->curtime));
        row = 60;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        time_min = set_time->curtime.min;

        if (time_min == 0) {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 0);
        } else if (time_min == 59) {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 3);
        } else if (time_min == 58) {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 2);
        } else {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 1);
        }

        base_index_once = (time_min >= 1) ? (time_min - 1) : 0;

        if (time_min == 59) {
            base_index_once = time_min - 3;
        } else if (time_min == 58) {
            base_index_once = time_min - 2;
        }

        ui_grid_set_base_dynamic(grid, base_index_once, 0);
#endif
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        if (set_time->make_sure) {
            if (grid->elm.id == SETTING_TIME_SET_12_MIN_VLIST) {
                set_time->curtime.min = ui_grid_get_hindex_dynamic(grid) % 60;
                set_sys_time(&(set_time->curtime));
            }
        }
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_12_MIN_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_time_12_min_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int set_time_ampm_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    struct ui_grid *ampm_grid = NULL;
    int row, col;
    int base_index_once;
    int time_hour;
    int hour;
    int ampm_index;

    switch (event) {
    case ON_CHANGE_INIT:

#if SET_VLIST_LOOP_EN

        int base = 10000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 2;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        base = (base / 2);

        ui_grid_set_hindex_dynamic(grid, base, true, 1);

        base_index_once = base;

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = r.height + 5;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);
#else
        row = 2;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        printf("dynamic_grid %d X %d\n", row, col);

        ui_grid_set_hindex_dynamic(grid, 0, true, 0);
        base_index_once = 0;


        ui_grid_set_base_dynamic(grid, base_index_once, 0);
#endif
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_AMPM_HOUR_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_time_ampm_vlist_onchange,
 .onkey = default_vlist_onkey,
  .ontouch = NULL,
};

static int set_time_12_ok_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_text *text = (struct ui_text *)ctr;

    u8 start_hour;
    u8 start_min;
    u8 end_hour;
    u8 end_min;
    u32 start_all_time;
    u32 end_all_time;
    u8 new_hour;
    u8 new_min;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        set_time->make_sure = 1;
        if (text->elm.id == SETTING_TIME_SET_12_SURE_BUTTON) {
            // 设置时间
            ui_hide(SETTING_TIME_SET_12_LAYOUT);
            ui_show(SETTING_TIME_SET_LAYOUT);
        }
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_SET_12_SURE_BUTTON)//开始时间-文字控件(确定)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = set_time_12_ok_ontouch,
};

static int set_time_h_12_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 13;//这里必须求余数方式获取索引
        /* printf("tid %d\n", index); */
        if ((index < 0) || (index > 13)) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_TIME_12_H00:
        case SETTING_TIME_12_H10:
        case SETTING_TIME_12_H20:
        case SETTING_TIME_12_H30:
            index_buf = index / 10;
            break;
        case SETTING_TIME_12_H01:
        case SETTING_TIME_12_H11:
        case SETTING_TIME_12_H21:
        case SETTING_TIME_12_H31:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_H00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_H10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_H20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_H30)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_h_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_H01)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_H11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_H21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_H31)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_h_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int set_time_m_12_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 60;//这里必须求余数方式获取索引
        /* log_info("tid %d\n", index); */
        if ((index < 0) || (index > 59)) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_TIME_12_M00:
        case SETTING_TIME_12_M10:
        case SETTING_TIME_12_M20:
        case SETTING_TIME_12_M30:
            index_buf = index / 10;
            break;
        case SETTING_TIME_12_M01:
        case SETTING_TIME_12_M11:
        case SETTING_TIME_12_M21:
        case SETTING_TIME_12_M31:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_M00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_M10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_M20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_M30)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_m_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_M01)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_M11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_M21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_12_M31)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = set_time_m_12_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int set_time_ampm_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 2;//这里必须求余数方式获取索引
        /* log_info("tid %d\n", index); */
        if ((index < 0) || (index > 59)) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_TIME_AMPM1:
        case SETTING_TIME_AMPM2:
        case SETTING_TIME_AMPM3:
        case SETTING_TIME_AMPM4:
            index_buf = index;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_AMPM1)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_ampm_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_AMPM2)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_ampm_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_AMPM3)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_ampm_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_TIME_AMPM4)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = set_time_ampm_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int setting_undisturb_vlist_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    static u8 touch_action = 0;
    int sel_item;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_ENERGY:
        return true;
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(UNDISTURB_MODE_VLIST)
.onchange = setting_vlist_default_onchange,
 .onkey = NULL,
  .ontouch = setting_undisturb_vlist_ontouch,
};

static int set_date_ok_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        set_time->make_sure = 1;
        if (text->elm.id == SETTING_DATE_SURE_BUTTON) {
            // 设置时间
            ui_hide(SETTING_DATE_SET_LAYOUT);
            ui_show(SETTING_TIME_SET_LAYOUT);
        }
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_SURE_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = set_date_ok_ontouch,
};

static int set_date_year_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    struct ui_grid *ampm_grid = NULL;
    int row, col;
    int base_index_once;
    int time_year;
    int year;
    u8 leap_year;

    switch (event) {
    case ON_CHANGE_INIT:

        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        get_sys_time(&(set_time->curtime));

        int base = 1000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 5000;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        time_year = set_time->curtime.year;

        ui_grid_set_hindex_dynamic(grid, time_year, true, 1);

        base_index_once = ((time_year >= 1) ? (time_year) : 0);

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = r.height + 5;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        if (set_time && set_time->make_sure) {
            if (grid->elm.id == SETTING_DATE_YEAR_VLIST) {
                year = ui_grid_get_hindex_dynamic(grid);
                set_time->curtime.year = year;
                set_sys_time(&(set_time->curtime));
            }
        }
        printf("@@@@@year %d\n", set_time->curtime.year);
        if (set_time) {
            free(set_time);
            set_time = NULL;
        }
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_YEAR_VLIST)
.onchange = set_date_year_vlist_onchange,
 .onkey = default_vlist_onkey,
  .ontouch = NULL,
};

static int set_date_y_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 10000;//这里必须求余数方式获取索引
        /* printf("tid %d\n", index); */
        if ((index < 0) || (index > 10000)) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_YEAR_Y00:
        case SETTING_YEAR_Y10:
        case SETTING_YEAR_Y20:
        case SETTING_YEAR_Y30:
            index_buf = index / 1000;
            break;
        case SETTING_YEAR_Y01:
        case SETTING_YEAR_Y11:
        case SETTING_YEAR_Y21:
        case SETTING_YEAR_Y31:
            index_buf = index / 100 % 10;
            break;
        case SETTING_YEAR_Y02:
        case SETTING_YEAR_Y12:
        case SETTING_YEAR_Y22:
        case SETTING_YEAR_Y32:
            index_buf = index / 10 % 10;
            break;
        case SETTING_YEAR_Y03:
        case SETTING_YEAR_Y13:
        case SETTING_YEAR_Y23:
        case SETTING_YEAR_Y33:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y00)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y01)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y02)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y03)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y10)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y11)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y12)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y13)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y20)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y21)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y22)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y23)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y30)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y31)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y32)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_YEAR_Y33)
.onchange = set_date_y_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static bool get_is_leap_year(u16 year)
{
    if (((year % 4) == 0) && ((year % 100) != 0)) {
        return 1;
    } else if ((year % 400) == 0) {
        return 1;
    }

    return 0;
}

static u8 vlist_create_day(struct sys_time *time)
{
    u8 month = time->month;
    u8 year = time->year;
    if (month == 2) {
        if (get_is_leap_year(year)) {
            return 29;
        } else {
            return 28;
        }
    } else if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 ||
               month == 10 || month == 12) {
        return 31;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }
    return 0;
}

static int set_date_day_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    struct rect r;
    int row, col;
    int base_index_once;
    int time_day;
    int day;

    switch (event) {
    case ON_CHANGE_INIT:
        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        get_sys_time(&(set_time->curtime));

        u8 create_day = vlist_create_day(&(set_time->curtime));
        set_time->dynamic_day_num = create_day;

        row = create_day;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        time_day = set_time->curtime.day;

        if (time_day == 1) {
            ui_grid_set_hindex_dynamic(grid, time_day - 1, true, 0);
        } else if (time_day == create_day) {
            ui_grid_set_hindex_dynamic(grid, time_day - 1, true, 3);
        } else if (time_day == create_day - 1) {
            ui_grid_set_hindex_dynamic(grid, time_day - 1, true, 2);
        } else {
            ui_grid_set_hindex_dynamic(grid, time_day - 1, true, 1);
        }

        base_index_once = (time_day >= 1) ? (time_day - 1) : 0;

        if (time_day == create_day) {
            base_index_once = time_day - 4;
        } else if (time_day == create_day - 1) {
            base_index_once = time_day - 3;
        } else {
            base_index_once = time_day - 2;
        }

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        if (time_day == create_day || time_day == create_day - 1) {
            ui_grid_set_base_dynamic(grid, base_index_once, -r.height - 5);
        } else if (time_day == 1) {
            ui_grid_set_base_dynamic(grid, base_index_once, r.height + 5);
            ui_grid_slide(grid, SCROLL_DIRECTION_UD, r.height + 5);
        } else {
            ui_grid_set_base_dynamic(grid, base_index_once, 0);
        }
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);

        break;
    case ON_CHANGE_RELEASE:
        if (set_time && set_time->make_sure) {
            if (grid->elm.id == SETTING_DATE_DAY_VLIST) {
                day = ui_grid_get_hindex_dynamic(grid) % 32;
                set_time->curtime.day = day + 1;
                set_sys_time(&(set_time->curtime));
            }
        }
        printf("@@@@@day %d\n", set_time->curtime.day);
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_DAY_VLIST)
.onchange = set_date_day_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int set_date_d_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 32 + 1;//这里必须求余数方式获取索引
        /* printf("tid %d\n", index); */
        if ((index < 0) || (index > 31)) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_DATE_D00:
        case SETTING_DATE_D10:
        case SETTING_DATE_D20:
        case SETTING_DATE_D30:
            index_buf = index / 10;
            break;
        case SETTING_DATE_D01:
        case SETTING_DATE_D11:
        case SETTING_DATE_D21:
        case SETTING_DATE_D31:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D00)
.onchange = set_date_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D01)
.onchange = set_date_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D10)
.onchange = set_date_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D11)
.onchange = set_date_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D20)
.onchange = set_date_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D21)
.onchange = set_date_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D30)
.onchange = set_date_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D31)
.onchange = set_date_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static void month_check(void *priv)
{
    struct ui_grid *grid_month = NULL;
    struct ui_grid *grid_day = NULL;
    struct ui_grid *grid_year = NULL;

    grid_month = ui_grid_for_id(SETTING_DATE_MONTH_VLIST);
    grid_day = ui_grid_for_id(SETTING_DATE_DAY_VLIST);
    grid_year = ui_grid_for_id(SETTING_DATE_YEAR_VLIST);

    int row, col;
    int month = ui_grid_cur_item_dynamic(grid_month);
    int is_leap_year = get_is_leap_year(ui_grid_cur_item_dynamic(grid_year));

    if (!is_leap_year) {    // 非闰年
        if (month == 1) {   // 2月
            if (set_time->dynamic_day_num == 31) {
                row = 3;
                col = 0;
                set_time->dynamic_day_num -= 3;
                ui_grid_del_dynamic(grid_day, &row, &col, 1);
            } else if (set_time->dynamic_day_num == 30) {
                row = 2;
                col = 0;
                set_time->dynamic_day_num -= 2;
                ui_grid_del_dynamic(grid_day, &row, &col, 1);
            } else if (set_time->dynamic_day_num == 29) {
                row = 1;
                col = 0;
                set_time->dynamic_day_num -= 1;
                ui_grid_del_dynamic(grid_day, &row, &col, 1);
            }
        } else if (month == 0 || month == 2 || month == 4 || month == 6 || month == 7 ||
                   month == 9 || month == 11) {
            if (set_time->dynamic_day_num == 30) {
                row = 1;
                col = 0;
                set_time->dynamic_day_num += 1;
                ui_grid_add_dynamic(grid_day, &row, &col, 1);
            } else if (set_time->dynamic_day_num == 28) {
                row = 3;
                col = 0;
                set_time->dynamic_day_num += 3;
                ui_grid_add_dynamic(grid_day, &row, &col, 1);
            }

        } else if (month == 3 || month == 5 || month == 8 || month == 10) {
            if (set_time->dynamic_day_num == 31) {
                row = 1;
                col = 0;
                set_time->dynamic_day_num -= 1;
                ui_grid_del_dynamic(grid_day, &row, &col, 1);
            } else if (set_time->dynamic_day_num == 28) {
                row = 2;
                col = 0;
                set_time->dynamic_day_num += 2;
                ui_grid_add_dynamic(grid_day, &row, &col, 1);
            }
        }
    } else {    // 闰年
        if (month == 1) {   // 2月
            if (set_time->dynamic_day_num == 31) {
                row = 2;
                col = 0;
                set_time->dynamic_day_num -= 2;
                ui_grid_del_dynamic(grid_day, &row, &col, 1);
            } else if (set_time->dynamic_day_num == 30) {
                row = 1;
                col = 0;
                set_time->dynamic_day_num -= 1;
                ui_grid_del_dynamic(grid_day, &row, &col, 1);
            } else if (set_time->dynamic_day_num == 28) {
                row = 1;
                col = 0;
                set_time->dynamic_day_num += 1;
                ui_grid_add_dynamic(grid_day, &row, &col, 1);
            }
        } else if (month == 0 || month == 2 || month == 4 || month == 6 || month == 7 ||
                   month == 9 || month == 11) {
            if (set_time->dynamic_day_num == 30) {
                row = 1;
                col = 0;
                set_time->dynamic_day_num += 1;
                ui_grid_add_dynamic(grid_day, &row, &col, 1);
            } else if (set_time->dynamic_day_num == 29) {
                row = 2;
                col = 0;
                set_time->dynamic_day_num += 2;
                ui_grid_add_dynamic(grid_day, &row, &col, 1);
            }

        } else if (month == 3 || month == 5 || month == 8 || month == 10) {
            if (set_time->dynamic_day_num == 31) {
                row = 1;
                col = 0;
                set_time->dynamic_day_num -= 1;
                ui_grid_del_dynamic(grid_day, &row, &col, 1);
            } else if (set_time->dynamic_day_num == 29) {
                row = 1;
                col = 0;
                set_time->dynamic_day_num += 1;
                ui_grid_add_dynamic(grid_day, &row, &col, 1);
            }
        }
    }
}

static int set_date_month_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    struct ui_grid *ampm_grid = NULL;
    struct rect r;
    int row, col;
    int base_index_once;
    int time_month;
    int month;

    switch (event) {
    case ON_CHANGE_INIT:
        if (!set_time) {
            set_time = zalloc(sizeof(struct set_time_t));
        }
        if (!set_time->vlist_timer) {
            set_time->vlist_timer = sys_timer_add(NULL, month_check, 1000);
        }
        get_sys_time(&(set_time->curtime));
        row = 12;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        time_month = set_time->curtime.month;

        if (time_month == 1) {
            ui_grid_set_hindex_dynamic(grid, time_month - 1, true, 0);
        } else if (time_month == 12) {
            ui_grid_set_hindex_dynamic(grid, time_month - 1, true, 3);
        } else if (time_month == 11) {
            ui_grid_set_hindex_dynamic(grid, time_month - 1, true, 2);
        } else {
            ui_grid_set_hindex_dynamic(grid, time_month - 1, true, 1);
        }

        base_index_once = (time_month >= 1) ? (time_month - 1) : 0;

        if (time_month == 12) {
            base_index_once = time_month - 4;
        } else if (time_month == 11) {
            base_index_once = time_month - 3;
        } else {
            base_index_once = time_month - 2;
        }

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        if (time_month == 11 || time_month == 12) {
            ui_grid_set_base_dynamic(grid, base_index_once, -r.height - 5);
        } else if (time_month == 1) {
            ui_grid_set_base_dynamic(grid, base_index_once, r.height + 5);
            ui_grid_slide(grid, SCROLL_DIRECTION_UD, r.height + 5);
        } else {
            ui_grid_set_base_dynamic(grid, base_index_once, 0);
        }
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);

        break;
    case ON_CHANGE_RELEASE:
        if (set_time && set_time->vlist_timer) {
            sys_timer_del(set_time->vlist_timer);
            set_time->vlist_timer = 0;
        }
        if (set_time && set_time->make_sure) {
            if (grid->elm.id == SETTING_DATE_MONTH_VLIST) {
                month = ui_grid_get_hindex_dynamic(grid) % 13;
                set_time->curtime.month = month + 1;
                set_sys_time(&(set_time->curtime));
            }
        }
        printf("@@@@@month %d\n", set_time->curtime.month);
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_DATE_MONTH_VLIST)
.onchange = set_date_month_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int set_date_m_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 13 + 1;//这里必须求余数方式获取索引
        /* printf("tid %d\n", index); */
        if ((index < 0) || (index > 12)) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_MONTH_M00:
        case SETTING_MONTH_M10:
        case SETTING_MONTH_M20:
        case SETTING_MONTH_M30:
            index_buf = index / 10;
            break;
        case SETTING_MONTH_M01:
        case SETTING_MONTH_M11:
        case SETTING_MONTH_M21:
        case SETTING_MONTH_M31:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SETTING_MONTH_M00)
.onchange = set_date_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_MONTH_M10)
.onchange = set_date_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_MONTH_M20)
.onchange = set_date_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_MONTH_M30)
.onchange = set_date_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_MONTH_M01)
.onchange = set_date_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_MONTH_M11)
.onchange = set_date_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_MONTH_M21)
.onchange = set_date_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_MONTH_M31)
.onchange = set_date_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


//-----------------------显示与亮度界面---------------------------//
struct brightness_info_t {
    u8 brightness_adjust;   // 判断旋钮是调节背光还是列表
};
static struct brightness_info_t *brightness_info = NULL;

static int brightness_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***brightness_onchange***\n");
        if (!brightness_info) {
            brightness_info = zalloc(sizeof(struct brightness_info_t));
        }
        break;
    case ON_CHANGE_RELEASE:
        if (brightness_info) {
            free(brightness_info);
            brightness_info = NULL;
        }
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_SETTING_BRIGHTNESS)
.onchange = brightness_page_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int set_brightness_layout_onkey(void *ctr, struct element_key_event *e)
{
    struct layout *layout = (struct layout *)ctr;
    int brightness_level;
    printf("@@@@@ %s\n", __func__);

    switch (e->value) {
    case KEY_UI_PLUS:
        printf("@@@@@ plus\n");
        if (brightness_info->brightness_adjust == 1) {
            if (get_ui_sys_param(LightLevel) < MAX_LIGHTLEVEL) {
                brightness_level = get_ui_sys_param(LightLevel);
                set_ui_sys_param(LightLevel, ++brightness_level);
                ui_pic_show_image_by_id(SETTING_BRIGHTNESS_LEVEL_PIC, brightness_level);
                ui_ajust_light(brightness_level * 2);
                if (UIInfo_w_vm_timer == 0) {
                    UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
                } else {
                    sys_timer_re_run(UIInfo_w_vm_timer);
                }
            }
            return true;
        } else {
            return false;
        }
        break;
    case KEY_UI_MINUS:
        printf("@@@@@ minus\n");
        if (brightness_info->brightness_adjust == 1) {
            if (get_ui_sys_param(LightLevel) > MIN_LIGHTLEVEL) {
                brightness_level = get_ui_sys_param(LightLevel);
                set_ui_sys_param(LightLevel, --brightness_level);
                ui_pic_show_image_by_id(SETTING_BRIGHTNESS_LEVEL_PIC, brightness_level);
                ui_ajust_light(brightness_level * 2);
                if (UIInfo_w_vm_timer == 0) {
                    UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
                } else {
                    sys_timer_re_run(UIInfo_w_vm_timer);
                }
            }
            return true;
        } else {
            return false;
        }
        break;
    default:
        break;
    }

    return false;
}

static int set_brightness_vlist_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    static u8 touch_action = 0;
    int sel_item;
    if (brightness_info) {
        brightness_info->brightness_adjust = 0;
    }

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
        case 0:
            brightness_info->brightness_adjust = 1;
            break;
        case 2:
            ui_hide(SETTING_BRIGHTNESS_LAYOUT);
            ui_show(SETTING_BRIGHTNESS_TIME_LAYOUT);
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

REGISTER_UI_EVENT_HANDLER(SETTING_BRIGHTNESS_VLIST)//通用-垂直列表
.onchange = setting_vlist_default_onchange,
 .onkey = set_brightness_layout_onkey,
  .ontouch = set_brightness_vlist_ontouch,
};

static int setting_brightness_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        switch (pic->elm.id) {
        case SETTING_BRIGHTNESS_LEVEL_PIC:
            ui_pic_set_image_index(pic, get_ui_sys_param(LightLevel));
            break;
        case SETTING_RAISE_HAND_BUTTON:
            ui_pic_set_image_index(pic, get_ui_sys_param(raise_hand_screen_on));
            break;
        case SETTING_SCREEN_OFF_DIAL_BUTTON:
            ui_pic_set_image_index(pic, get_ui_sys_param(screen_off_dial));
            break;
        case SETTING_BED_LIGHT_BUTTON:
            ui_pic_set_image_index(pic, get_ui_sys_param(bed_light));
            break;
        default:
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int setting_brightness_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;
    int brightness_level = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SETTING_BRIGHTNESS_MINUS_BUTTON:
            brightness_info->brightness_adjust = 1;
            if (get_ui_sys_param(LightLevel) > MIN_LIGHTLEVEL) {
                brightness_level = get_ui_sys_param(LightLevel);
                set_ui_sys_param(LightLevel, --brightness_level);
                ui_pic_show_image_by_id(SETTING_BRIGHTNESS_LEVEL_PIC, brightness_level);
                ui_ajust_light(brightness_level * 2);
                if (UIInfo_w_vm_timer == 0) {
                    UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
                } else {
                    sys_timer_re_run(UIInfo_w_vm_timer);
                }
                // 亮度减小接口
            }
            break;
        case SETTING_BRIGHTNESS_ADD_BUTTON:
            brightness_info->brightness_adjust = 1;
            if (get_ui_sys_param(LightLevel) < MAX_LIGHTLEVEL) {
                brightness_level = get_ui_sys_param(LightLevel);
                set_ui_sys_param(LightLevel, ++brightness_level);
                ui_pic_show_image_by_id(SETTING_BRIGHTNESS_LEVEL_PIC, brightness_level);
                ui_ajust_light(brightness_level * 2);
                if (UIInfo_w_vm_timer == 0) {
                    UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
                } else {
                    sys_timer_re_run(UIInfo_w_vm_timer);
                }
                // 亮度增大接口
            }
            break;
        case SETTING_RAISE_HAND_BUTTON:
            u8 raise_hand_index = get_ui_sys_param(raise_hand_screen_on);
            raise_hand_index = !raise_hand_index;
            ui_pic_show_image_by_id(SETTING_RAISE_HAND_BUTTON, raise_hand_index);
            set_ui_sys_param(raise_hand_screen_on, raise_hand_index);
            if (UIInfo_w_vm_timer == 0) {
                UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
            } else {
                sys_timer_re_run(UIInfo_w_vm_timer);
            }
            // 抬手亮屏
            break;
        case SETTING_SCREEN_OFF_DIAL_BUTTON:
            u8 screen_off_dial_index = get_ui_sys_param(screen_off_dial);
            screen_off_dial_index = !screen_off_dial_index;
            ui_pic_show_image_by_id(SETTING_SCREEN_OFF_DIAL_BUTTON, screen_off_dial_index);
            set_ui_sys_param(screen_off_dial, screen_off_dial_index);
            if (UIInfo_w_vm_timer == 0) {
                UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
            } else {
                sys_timer_re_run(UIInfo_w_vm_timer);
            }
            // 熄屏表盘
            break;
        case SETTING_BED_LIGHT_BUTTON:
            u8 bed_light_index = get_ui_sys_param(bed_light);
            bed_light_index = !bed_light_index;
            ui_pic_show_image_by_id(SETTING_BED_LIGHT_BUTTON, bed_light_index);
            set_ui_sys_param(bed_light, bed_light_index);
            if (UIInfo_w_vm_timer == 0) {
                UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
            } else {
                sys_timer_re_run(UIInfo_w_vm_timer);
            }
            // 床头灯
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_BRIGHTNESS_MINUS_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_brightness_ontouch,
};

REGISTER_UI_EVENT_HANDLER(SETTING_BRIGHTNESS_ADD_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_brightness_ontouch,
};

REGISTER_UI_EVENT_HANDLER(SETTING_BRIGHTNESS_LEVEL_PIC)
.onchange = setting_brightness_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(SETTING_RAISE_HAND_BUTTON)
.onchange = setting_brightness_onchange,
 .onkey = NULL,
  .ontouch = setting_brightness_ontouch,
};

REGISTER_UI_EVENT_HANDLER(SETTING_SCREEN_OFF_DIAL_BUTTON)
.onchange = setting_brightness_onchange,
 .onkey = NULL,
  .ontouch = setting_brightness_ontouch,
};

REGISTER_UI_EVENT_HANDLER(SETTING_BED_LIGHT_BUTTON)
.onchange = setting_brightness_onchange,
 .onkey = NULL,
  .ontouch = setting_brightness_ontouch,
};

static int setting_brightness_time_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SETTING_BRIGHTNESS_TIME_LAYOUT);
        ui_show(SETTING_BRIGHTNESS_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_BRIGHTNESS_TIME_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_brightness_time_layout_ontouch,
};

static int setting_brightness_time_sure_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        // 设置熄屏时长接口
        set_ui_sys_param(DarkTime, ui_grid_get_hindex(ui_grid_for_id(SETTING_SCREEN_OFF_TIME_VLIST)));
        ui_set_dark_time(get_ui_sys_param(DarkTime));
        if (UIInfo_w_vm_timer == 0) {
            UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
        } else {
            sys_timer_re_run(UIInfo_w_vm_timer);
        }
        ui_hide(SETTING_BRIGHTNESS_TIME_LAYOUT);
        ui_show(SETTING_BRIGHTNESS_LAYOUT);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_BRIGHTNESS_TIME_SURE_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_brightness_time_sure_ontouch,
};

static int setting_brightness_time_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    struct rect r;
    u8 dark_time = get_ui_sys_param(DarkTime);

    switch (event) {
    case ON_CHANGE_INIT:
        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        if (dark_time == 0) {
            ui_grid_slide(grid, SCROLL_DIRECTION_UD, (r.height + grid->y_interval));
        } else {
            ui_grid_slide(grid, SCROLL_DIRECTION_UD, (dark_time - 1) * (-(r.height + grid->y_interval)));
        }
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_SCREEN_OFF_TIME_VLIST)
.onchange = setting_brightness_time_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
//-----------------------声音与振动界面---------------------------//
static int set_voice_layout_onkey(void *ctr, struct element_key_event *e)
{
    struct layout *layout = (struct layout *)ctr;
    u8 sys_vol = get_ui_sys_param(LastSysVol);
    printf("@@@@@ %s\n", __func__);

    switch (e->value) {
    case KEY_UI_PLUS:
        printf("@@@@@ plus\n");
        if (sys_vol < 100) {
            ui_volume_up(20);
            sys_vol += 20;
            ui_pic_show_image_by_id(SETTING_VOICE_LEVEL_PIC, sys_vol / 20);
            set_ui_sys_param(LastSysVol, sys_vol);
            ui_set_voice_mute(0);
            if (UIInfo_w_vm_timer == 0) {
                UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
            } else {
                sys_timer_re_run(UIInfo_w_vm_timer);
            }
            ui_pic_show_image_by_id(SETTING_VOICE_MUTE_BUTTON, 0);
        } else if (sys_vol == 100) {
            ui_pic_show_image_by_id(SETTING_VOICE_LEVEL_PIC, sys_vol / 20);
            ui_set_voice_mute(0);
            ui_pic_show_image_by_id(SETTING_VOICE_MUTE_BUTTON, 0);
        }
        return true;
        break;
    case KEY_UI_MINUS:
        printf("@@@@@ minus\n");
        if (sys_vol > 0) {
            ui_volume_down(20);
            sys_vol -= 20;
            ui_pic_show_image_by_id(SETTING_VOICE_LEVEL_PIC, sys_vol / 20);
            set_ui_sys_param(LastSysVol, sys_vol);
            ui_set_voice_mute(0);
            if (UIInfo_w_vm_timer == 0) {
                UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
            } else {
                sys_timer_re_run(UIInfo_w_vm_timer);
            }
            if (sys_vol == 0) {
                ui_set_voice_mute(1);
                ui_pic_show_image_by_id(SETTING_VOICE_MUTE_BUTTON, 1);
            } else {
                ui_pic_show_image_by_id(SETTING_VOICE_MUTE_BUTTON, 0);
            }
        }
        return true;
        break;
    default:
        break;
    }

    return false;
}

static int set_voice_vibration_vlist_ontouch(void *ctr, struct element_touch_event *e)
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
        case 2:
            ui_hide(SETTING_VOICE_LAYOUT);
            ui_show(SETTING_VIBRATION_LAYOUT);
            break;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        return true;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_ENERGY:
        return true;
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_VOICE_VLIST)//通用-垂直列表
.onchange = setting_vlist_default_onchange,
 .onkey = set_voice_layout_onkey,
  .ontouch = set_voice_vibration_vlist_ontouch,
};

static int setting_voice_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 sys_vol;
    u8 mute_vol;

    switch (event) {
    case ON_CHANGE_INIT:
        switch (pic->elm.id) {
        case SETTING_VOICE_LEVEL_PIC:
            if (!ui_get_voice_mute()) {
                sys_vol = get_ui_sys_param(LastSysVol);
                ui_pic_set_image_index(pic, sys_vol / 20);
            }
            break;
        case SETTING_VOICE_MUTE_BUTTON:
            mute_vol = ui_get_voice_mute();
            ui_pic_set_image_index(pic, mute_vol);
            break;
        default:
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int setting_voice_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;
    u8 sys_vol = get_ui_sys_param(LastSysVol);
    u8 voice_mute_sel;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SETTING_VOICE_MINUS_BUTTON:
            if (sys_vol > 0) {
                ui_volume_down(20);
                sys_vol -= 20;
                ui_pic_show_image_by_id(SETTING_VOICE_LEVEL_PIC, sys_vol / 20);
                set_ui_sys_param(LastSysVol, sys_vol);
                ui_set_voice_mute(0);
                if (UIInfo_w_vm_timer == 0) {
                    UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
                } else {
                    sys_timer_re_run(UIInfo_w_vm_timer);
                }
                if (sys_vol == 0) {
                    ui_set_voice_mute(1);
                    ui_pic_show_image_by_id(SETTING_VOICE_MUTE_BUTTON, 1);
                }
            }
            break;
        case SETTING_VOICE_ADD_BUTTON:
            if (sys_vol < 100) {
                ui_volume_up(20);
                sys_vol += 20;
                ui_pic_show_image_by_id(SETTING_VOICE_LEVEL_PIC, sys_vol / 20);
                set_ui_sys_param(LastSysVol, sys_vol);
                ui_set_voice_mute(0);
                if (UIInfo_w_vm_timer == 0) {
                    UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
                } else {
                    sys_timer_re_run(UIInfo_w_vm_timer);
                }
                ui_pic_show_image_by_id(SETTING_VOICE_MUTE_BUTTON, 0);
            } else if (sys_vol == 100) {
                ui_pic_show_image_by_id(SETTING_VOICE_LEVEL_PIC, sys_vol / 20);
                ui_set_voice_mute(0);
                ui_pic_show_image_by_id(SETTING_VOICE_MUTE_BUTTON, 0);
            }
            break;
        case SETTING_VOICE_MUTE_BUTTON:
            voice_mute_sel = ui_get_voice_mute();
            voice_mute_sel = !voice_mute_sel;
            ui_pic_show_image_by_id(SETTING_VOICE_MUTE_BUTTON, !!voice_mute_sel);
            ui_set_voice_mute(voice_mute_sel);
            if (voice_mute_sel) {
                ui_pic_show_image_by_id(SETTING_VOICE_LEVEL_PIC, 0);
            } else {
                ui_pic_show_image_by_id(SETTING_VOICE_LEVEL_PIC, sys_vol / 20);
            }
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_VOICE_MINUS_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_voice_ontouch,
};

REGISTER_UI_EVENT_HANDLER(SETTING_VOICE_ADD_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_voice_ontouch,
};

REGISTER_UI_EVENT_HANDLER(SETTING_VOICE_LEVEL_PIC)
.onchange = setting_voice_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(SETTING_VOICE_MUTE_BUTTON)
.onchange = setting_voice_onchange,
 .onkey = NULL,
  .ontouch = setting_voice_ontouch,
};

static int setting_vibration_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    u8 moto_mode;

    switch (event) {
    case ON_CHANGE_INIT:
        moto_mode = get_ui_sys_param(MotoMode);
        if (moto_mode == TCFG_MOTO_PWM_L) {
            ui_show(SETTING_VIBRATION_MEDIUM_PIC);
        } else {
            ui_show(SETTING_VIBRATION_STRONG_PIC);
        }
        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        break;
    }
    return 0;
}

static int setting_vibration_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SETTING_VIBRATION_LAYOUT);
        ui_show(SETTING_VOICE_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_VIBRATION_LAYOUT)
.onchange = setting_vibration_layout_onchange,
 .onkey = NULL,
  .ontouch = setting_vibration_layout_ontouch,
};

static int setting_vibration_button_ontouch(void *ctr, struct element_touch_event *e)
{
    struct button *button = (struct button *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        switch (button->elm.id) {
        case SETTING_VIBRATION_MEDIUM_BUTTON:
            ui_moto_set_H_L(TCFG_MOTO_PWM_L);
            ui_hide(SETTING_VIBRATION_STRONG_PIC);
            ui_show(SETTING_VIBRATION_MEDIUM_PIC);
            UI_MOTO_RUN(2);
            break;
        case SETTING_VIBRATION_STRONG_BUTTON:
            ui_moto_set_H_L(TCFG_MOTO_PWM_H);
            ui_hide(SETTING_VIBRATION_MEDIUM_PIC);
            ui_show(SETTING_VIBRATION_STRONG_PIC);
            UI_MOTO_RUN(2);
            break;
        }
        return true;
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_VIBRATION_MEDIUM_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_vibration_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SETTING_VIBRATION_STRONG_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_vibration_button_ontouch,
};

static int setting_vibration_back_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SETTING_VIBRATION_BACK_PIC:
            ui_hide(SETTING_VIBRATION_LAYOUT);
            ui_show(SETTING_VOICE_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_VIBRATION_BACK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = setting_vibration_back_ontouch,
};

//-----------------------勿扰模式界面---------------------------//
struct undisturb_param_t {
    u8 make_sure;
};
static struct undisturb_param_t undisturb_param = {0};

void set_all_day_undisturb_sel(u8 sel)
{
    set_ui_sys_param(AllDayUndisturbEn, sel);
    if (sel == 1) {
        ui_moto_run(3);//静音
    } else {
        ui_moto_run(4);//允许震动
    }
}

u8 get_all_day_undisturb_sel()
{
    return get_ui_sys_param(AllDayUndisturbEn);
}

void set_time_undisturb_sel(u8 sel)
{
    set_ui_sys_param(TimeUndisturbEn, sel);
    if (sel == 1) {
        ui_moto_run(5);
    } else {
        ui_moto_run(6);
        set_ui_sys_param(UndisturbStimeH, 0);
        set_ui_sys_param(UndisturbStimeM, 0);
        set_ui_sys_param(UndisturbEtimeH, 0);
        set_ui_sys_param(UndisturbEtimeM, 0);
    }
}

u8 get_time_undisturb_sel()
{
    return get_ui_sys_param(TimeUndisturbEn);
}

static int undisturb_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 time_undisturb = get_time_undisturb_sel();
    u8 all_day_undisturb = get_all_day_undisturb_sel();

    switch (event) {
    case ON_CHANGE_INIT:
        switch (pic->elm.id) {
        case ALL_DAY_UNDISTURB_BUTTON:
            ui_pic_set_image_index(pic, all_day_undisturb);
            break;
        case TIME_UNDISTURB_MODE_BUTTON:
            ui_pic_set_image_index(pic, time_undisturb);
            if (time_undisturb) {
                ui_show(TIME_UNDISTURB_MODE_START_LAYOUT);
                ui_show(TIME_UNDISTURB_MODE_END_LAYOUT);
            }
            break;
        default:
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int undisturb_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;
    u8 all_day_undisturb_sel;
    u8 time_undisturb_sel;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case ALL_DAY_UNDISTURB_BUTTON:
            all_day_undisturb_sel = get_all_day_undisturb_sel();
            all_day_undisturb_sel = !all_day_undisturb_sel;
            ui_pic_show_image_by_id(ALL_DAY_UNDISTURB_BUTTON, !!all_day_undisturb_sel);


            if (all_day_undisturb_sel == 1) {
                set_time_undisturb_sel(0);
                ui_pic_show_image_by_id(TIME_UNDISTURB_MODE_BUTTON, 0);
                ui_hide(TIME_UNDISTURB_MODE_START_LAYOUT);
                ui_hide(TIME_UNDISTURB_MODE_END_LAYOUT);
            }
            set_all_day_undisturb_sel(all_day_undisturb_sel);
            break;
        case TIME_UNDISTURB_MODE_BUTTON:
            time_undisturb_sel = get_time_undisturb_sel();
            time_undisturb_sel = !time_undisturb_sel;
            ui_pic_show_image_by_id(TIME_UNDISTURB_MODE_BUTTON, !!time_undisturb_sel);

            if (time_undisturb_sel == 1) {
                set_all_day_undisturb_sel(0);
                ui_pic_show_image_by_id(ALL_DAY_UNDISTURB_BUTTON, 0);
                ui_show(TIME_UNDISTURB_MODE_START_LAYOUT);
                ui_show(TIME_UNDISTURB_MODE_END_LAYOUT);
            } else {
                ui_moto_run(7);
                ui_hide(TIME_UNDISTURB_MODE_START_LAYOUT);
                ui_hide(TIME_UNDISTURB_MODE_END_LAYOUT);
            }
            set_time_undisturb_sel(time_undisturb_sel);

            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(ALL_DAY_UNDISTURB_BUTTON)
.onchange = undisturb_onchange,
 .onkey = NULL,
  .ontouch = undisturb_ontouch,
};

REGISTER_UI_EVENT_HANDLER(TIME_UNDISTURB_MODE_BUTTON)
.onchange = undisturb_onchange,
 .onkey = NULL,
  .ontouch = undisturb_ontouch,
};


static int undisturb_time_ontouch(void *ctr, struct element_touch_event *e)
{
    struct button *button = (struct button *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (button->elm.id) {
        case TIME_UNDISTURB_MODE_START_BUTTON:
            ui_hide(UNDISTURB_MODE_LAYOUT);
            ui_show(UNDISTURB_MODE_SSETTING_LAYOUT);
            break;
        case TIME_UNDISTURB_MODE_END_BUTTON:
            ui_hide(UNDISTURB_MODE_LAYOUT);
            ui_show(UNDISTURB_MODE_ESETTING_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TIME_UNDISTURB_MODE_START_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = undisturb_time_ontouch,
};

REGISTER_UI_EVENT_HANDLER(TIME_UNDISTURB_MODE_END_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = undisturb_time_ontouch,
};

static int time_undisturb_set_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case UNDISTURB_MODE_SSETTING_BACK_BUTTON:
            ui_hide(UNDISTURB_MODE_SSETTING_LAYOUT);
            ui_show(UNDISTURB_MODE_LAYOUT);
            break;
        case UNDISTURB_MODE_ESETTING_BACK_BUTTON:
            ui_hide(UNDISTURB_MODE_ESETTING_LAYOUT);
            ui_show(UNDISTURB_MODE_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(UNDISTURB_MODE_ESETTING_BACK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = time_undisturb_set_ontouch,
};
REGISTER_UI_EVENT_HANDLER(UNDISTURB_MODE_SSETTING_BACK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = time_undisturb_set_ontouch,
};

static int list_time_hour_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_hour;

    switch (event) {
    case ON_CHANGE_INIT:
#if SET_VLIST_LOOP_EN

        int base = 10000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 24;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        if (grid->elm.id == UNDISTURB_MODE_SSETTING_HOUR_VLIST) {
            time_hour = get_ui_sys_param(UndisturbStimeH);
        } else {
            time_hour = get_ui_sys_param(UndisturbEtimeH);
        }
        time_hour = time_hour % 24;

        base = (base / 2) * 24;

        ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

        base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_hour == 0) ? r.height + 5 : 0;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);

#else


        row = 24;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);

        if (grid->elm.id == UNDISTURB_MODE_SSETTING_HOUR_VLIST) {
            time_hour = get_ui_sys_param(UndisturbStimeH);
        } else {
            time_hour = get_ui_sys_param(UndisturbEtimeH);
        }
        time_hour = time_hour % 24;

        if (time_hour == 0) {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 0);
        } else if (time_hour == 23) {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 3);
        } else if (time_hour == 22) {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 2);
        } else {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 1);
        }
        base_index_once = (time_hour >= 1) ? (time_hour - 1) : 0;

        if (time_hour == 23) {
            base_index_once = time_hour - 3;
        } else if (time_hour == 22) {
            base_index_once = time_hour - 2;
        }

        ui_grid_set_base_dynamic(grid, base_index_once, 0);
        /* if (!base_index_once) { */
        /* ui_grid_set_base_dynamic(grid, 23, 0); */
        /* } else { */
        /* ui_grid_set_base_dynamic(grid, base_index_once, 0); */
        /* } */
#endif
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        if (undisturb_param.make_sure) {
            if (grid->elm.id == UNDISTURB_MODE_SSETTING_HOUR_VLIST) {
                set_ui_sys_param(UndisturbStimeH, ui_grid_get_hindex_dynamic(grid));
            } else {
                set_ui_sys_param(UndisturbEtimeH, ui_grid_get_hindex_dynamic(grid));
            }
        }
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(UNDISTURB_MODE_SSETTING_HOUR_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = list_time_hour_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(UNDISTURB_MODE_ESETTING_HOUR_VLIST)//勿扰模式-结束时间-动态垂直列表
.onchange = list_time_hour_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int list_time_min_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_min;

    u8 start_hour;
    u8 start_min;
    u8 end_hour;
    u8 end_min;
    u32 start_all_time;
    u32 end_all_time;
    u8 new_hour;
    u8 new_min;

    switch (event) {
    case ON_CHANGE_INIT:

#if SET_VLIST_LOOP_EN

        int base = 10000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 60;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        if (grid->elm.id == UNDISTURB_MODE_SSETTING_MIN_VLIST) {
            time_min = get_ui_sys_param(UndisturbStimeM);
        } else {
            time_min = get_ui_sys_param(UndisturbEtimeM);
        }
        time_min = time_min % 60;

        base = (base / 2) * 60;

        ui_grid_set_hindex_dynamic(grid, time_min + base, true, 1);

        base_index_once = ((time_min >= 1) ? (time_min - 1) : 0) + base;

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_min == 0) ? r.height + 5 : 0;

        printf("first_move_step %d\n", first_move_step);
        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);

#else

        row = 60;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        if (grid->elm.id == UNDISTURB_MODE_SSETTING_MIN_VLIST) {
            time_min = get_ui_sys_param(UndisturbStimeM);
        } else {
            time_min = get_ui_sys_param(UndisturbEtimeM);
        }
        time_min = time_min % 60;

        if (time_min == 0) {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 0);
        } else if (time_min == 59) {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 3);
        } else if (time_min == 58) {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 2);
        } else {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 1);
        }

        base_index_once = (time_min >= 1) ? (time_min - 1) : 0;

        if (time_min == 59) {
            base_index_once = time_min - 3;
        } else if (time_min == 58) {
            base_index_once = time_min - 2;
        }

        ui_grid_set_base_dynamic(grid, base_index_once, 0);
        /* if (!base_index_once) { */
        /* ui_grid_set_base_dynamic(grid, 59, 0); */
        /* } else { */
        /* ui_grid_set_base_dynamic(grid, base_index_once, 0); */
        /* } */
#endif
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        if (undisturb_param.make_sure) {
            if (grid->elm.id == UNDISTURB_MODE_SSETTING_MIN_VLIST) {
                set_ui_sys_param(UndisturbStimeM, ui_grid_get_hindex_dynamic(grid));
                ui_moto_run(4);
            } else {
                set_ui_sys_param(UndisturbEtimeM, ui_grid_get_hindex_dynamic(grid));
                ui_moto_run(4);
            }
            start_hour = get_ui_sys_param(UndisturbStimeH);
            start_min  = get_ui_sys_param(UndisturbStimeM);
            end_hour   = get_ui_sys_param(UndisturbEtimeH);
            end_min    = get_ui_sys_param(UndisturbEtimeM);
            start_all_time = start_hour * 60 + start_min;
            end_all_time = end_hour * 60 + end_min;
            if (end_all_time - start_all_time < 5) {
                new_hour = (start_all_time + 5) / 60;
                new_min = (start_all_time + 5) % 60;
                set_ui_sys_param(UndisturbEtimeH, new_hour);
                set_ui_sys_param(UndisturbEtimeM, new_min);
                ui_moto_run(4);
            }
        }
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(UNDISTURB_MODE_SSETTING_MIN_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = list_time_min_onchange,
 .onkey = default_vlist_onkey,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(UNDISTURB_MODE_ESETTING_MIN_VLIST)//勿扰模式-结束时间-动态垂直列表
.onchange = list_time_min_onchange,
 .onkey = default_vlist_onkey,
  .ontouch = NULL,
};

static int text_time_ok_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_text *text = (struct ui_text *)ctr;

    u8 start_hour;
    u8 start_min;
    u8 end_hour;
    u8 end_min;
    u32 start_all_time;
    u32 end_all_time;
    u8 new_hour;
    u8 new_min;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        undisturb_param.make_sure = 1;
        if (text->elm.id == UNDISTURB_MODE_SSETTING_SURE_BUTTON) {
            ui_hide(UNDISTURB_MODE_SSETTING_LAYOUT);
            ui_show(UNDISTURB_MODE_LAYOUT);
        } else {
            ui_hide(UNDISTURB_MODE_ESETTING_LAYOUT);
            ui_show(UNDISTURB_MODE_LAYOUT);
        }
        start_hour = get_ui_sys_param(UndisturbStimeH);
        start_min  = get_ui_sys_param(UndisturbStimeM);
        end_hour   = get_ui_sys_param(UndisturbEtimeH);
        end_min    = get_ui_sys_param(UndisturbEtimeM);
        start_all_time = start_hour * 60 + start_min;
        end_all_time = end_hour * 60 + end_min;
        if (!(start_all_time == 0 && end_all_time == 0)) {
            if (end_all_time - start_all_time < 5) {
                new_hour = (start_all_time + 5) / 60;
                new_min = (start_all_time + 5) % 60;
                set_ui_sys_param(UndisturbEtimeH, new_hour);
                set_ui_sys_param(UndisturbEtimeM, new_min);
                printf("if min %d\n", new_min);
            }
        }
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(UNDISTURB_MODE_SSETTING_SURE_BUTTON)//开始时间-文字控件(确定)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = text_time_ok_ontouch,
};
REGISTER_UI_EVENT_HANDLER(UNDISTURB_MODE_ESETTING_SURE_BUTTON)//结束时间-文字控件(确定)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = text_time_ok_ontouch,
};

static int text_time_h_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 24;//这里必须求余数方式获取索引
        /* printf("tid %d\n", index); */
        if ((index < 0) || (index > 23)) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_STIME_H00:
        case SETTING_STIME_H10:
        case SETTING_STIME_H20:
        case SETTING_STIME_H30:
        case SETTING_ETIME_H00:
        case SETTING_ETIME_H10:
        case SETTING_ETIME_H20:
        case SETTING_ETIME_H30:
            index_buf = index / 10;
            break;
        case SETTING_STIME_H01:
        case SETTING_STIME_H11:
        case SETTING_STIME_H21:
        case SETTING_STIME_H31:
        case SETTING_ETIME_H01:
        case SETTING_ETIME_H11:
        case SETTING_ETIME_H21:
        case SETTING_ETIME_H31:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_H00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_H10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_H20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_H30)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_H00)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_H10)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_H20)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_H30)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_H01)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_H11)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_H21)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_H31)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_H01)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_H11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_H21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_H31)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int text_time_m_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 60;//这里必须求余数方式获取索引
        /* log_info("tid %d\n", index); */
        if ((index < 0) || (index > 59)) {
            break;
        }
        switch (pic->elm.id) {
        case SETTING_STIME_M00:
        case SETTING_STIME_M10:
        case SETTING_STIME_M20:
        case SETTING_STIME_M30:
        case SETTING_ETIME_M00:
        case SETTING_ETIME_M10:
        case SETTING_ETIME_M20:
        case SETTING_ETIME_M30:
            index_buf = index / 10;
            break;
        case SETTING_STIME_M01:
        case SETTING_STIME_M11:
        case SETTING_STIME_M21:
        case SETTING_STIME_M31:
        case SETTING_ETIME_M01:
        case SETTING_ETIME_M11:
        case SETTING_ETIME_M21:
        case SETTING_ETIME_M31:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_M00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_M10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_M20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_M30)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_M00)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_M10)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_M20)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_M30)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_M01)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_M11)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_M21)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_STIME_M31)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_M01)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_M11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_M21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ETIME_M31)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = text_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int time_undisturb_mode_stime_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_time *time = (struct ui_time *)_ctrl;
    struct utime t = {0};

    switch (event) {
    case ON_CHANGE_INIT:
        t.hour = get_ui_sys_param(UndisturbStimeH) % 24;
        t.min = get_ui_sys_param(UndisturbStimeM) % 60;
        ui_time_update(time, &t);
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(TIME_UNDISTURB_MODE_START_TIME)//勿扰模式-垂直列表-时间控件
.onchange = time_undisturb_mode_stime_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int time_undisturb_mode_etime_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_time *time = (struct ui_time *)_ctrl;
    struct utime t = {0};

    switch (event) {
    case ON_CHANGE_INIT:
        t.hour = get_ui_sys_param(UndisturbEtimeH) % 24;
        t.min = get_ui_sys_param(UndisturbEtimeM) % 60;
        ui_time_update(time, &t);
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(TIME_UNDISTURB_MODE_END_TIME)//勿扰模式-垂直列表-时间控件
.onchange = time_undisturb_mode_etime_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


//-----------------------健康提醒界面---------------------------//


static int health_tip_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        switch (pic->elm.id) {
        case HEALTH_BUTTON:
            ui_pic_set_image_index(pic, get_ui_sys_param(health_tips));
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int health_tip_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case HEALTH_BUTTON:
            u8 health_tip_index = get_ui_sys_param(health_tips);
            health_tip_index = !health_tip_index;
            ui_pic_show_image_by_id(HEALTH_BUTTON, health_tip_index);
            set_ui_sys_param(health_tips, health_tip_index);
            if (UIInfo_w_vm_timer == 0) {
                UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
            } else {
                sys_timer_re_run(UIInfo_w_vm_timer);
            }
            // 久坐提醒
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(HEALTH_BUTTON)
.onchange = health_tip_onchange,
 .onkey = NULL,
  .ontouch = health_tip_ontouch,
};

//-----------------------省电模式界面---------------------------//
static int low_power_mode_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        switch (pic->elm.id) {
        case LOW_POWER_BUTTON:
            ui_pic_set_image_index(pic, get_ui_sys_param(low_power_mode));
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int low_power_mode_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case LOW_POWER_BUTTON:
            u8 lp_mode_index = get_ui_sys_param(low_power_mode);
            lp_mode_index = !lp_mode_index;
            ui_pic_show_image_by_id(LOW_POWER_BUTTON, lp_mode_index);

            if (lp_mode_index) {
                enter_low_power_mode();
            } else {
                exit_low_power_mode();
            }
            // 省电模式
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(LOW_POWER_BUTTON)
.onchange = low_power_mode_onchange,
 .onkey = NULL,
  .ontouch = low_power_mode_ontouch,
};

//-----------------------设置密码界面---------------------------//
#define SET_PASSWORD         0
#define OLD_PASSWORD         1
#define NEW_PASSWORD         2
#define NEW_PASSWORD_TWICE   3

#define PASSWORD_TIME_MS     100
#define PASSWORD_TIME_COUNT  30

/* struct set_password_t { */
/*     u8 is_password_open; */
/*     u8 index; */
/*     u8 status; */
/*     u8 time_cnt; */
/*     u32 timer_id; */
/*     char final_password[5]; // 保存的密码 */
/*     char password[5];     // 用于键盘输入 */
/*     char new_password[5];     // 用于保存暂存的新密码 */
/* }; */

static struct password_t *password = NULL;

static void password_timer_show(void *priv)
{
    int layout = (int)priv;
    if (password->time_cnt) {
        password->time_cnt--;
    } else {
        if (password && password->timer_id) {
            sys_timer_del(password->timer_id);
            password->timer_id = 0;
            ui_hide(layout);
            ui_show(PASSWORD_LAYOUT);
        }
    }
}

static void ui_show_password(int index)
{
    struct ui_pic *pic = NULL;
    struct ui_text *text = NULL;
    if (index == 0) {
        pic = ui_pic_for_id(PASSWORD_1_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(PASSWORD_2_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(PASSWORD_3_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(PASSWORD_4_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
    }
    if (index == 1) {
        text = ui_text_for_id(PASSWORD_ENTER_TEXT);
        if (text) {
            text->elm.css.invisible = 1;
        }
        pic = ui_pic_for_id(PASSWORD_1_PIC);
        if (pic) {
            pic->elm.css.invisible = 0;
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(PASSWORD_2_PIC);
        if (pic) {
            pic->elm.css.invisible = 0;
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(PASSWORD_3_PIC);
        if (pic) {
            pic->elm.css.invisible = 0;
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(PASSWORD_4_PIC);
        if (pic) {
            pic->elm.css.invisible = 0;
            ui_pic_set_image_index(pic, 0);
        }
    } else if (index == 2) {
        pic = ui_pic_for_id(PASSWORD_1_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(PASSWORD_2_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(PASSWORD_3_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(PASSWORD_4_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
    } else if (index == 3) {
        pic = ui_pic_for_id(PASSWORD_1_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(PASSWORD_2_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(PASSWORD_3_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(PASSWORD_4_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
    } else if (index == 4) {
        pic = ui_pic_for_id(PASSWORD_1_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(PASSWORD_2_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(PASSWORD_3_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(PASSWORD_4_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
    }
}

static void set_password_add_number(char ch)
{
    if (password->index >= 4) {
        return;
    }
    password->password[password->index++] = ch;
    password->password[password->index] = '\0';
    ui_show_password(password->index);
    printf("add pass:%s\n", password->password);
}

static void set_password_remove_number()
{
    if (password->index <= 0) {
        return;
    }
    password->password[--password->index] = '\0';
    ui_show_password(password->index);
    printf("del pass:%s\n", password->password);
}

static void password_back_process()
{
    memset(password->password, 0, sizeof(password->password));
    password->index = 0;
    ui_hide(PASSWORD_ENTER_LAYOUT);
    ui_show(PASSWORD_LAYOUT);

    if (password->status != SET_PASSWORD) {
        password->status = OLD_PASSWORD;
    }
}

static void ui_hide_password(int index)
{
    struct ui_pic *pic = NULL;
    struct ui_text *text = NULL;
    pic = ui_pic_for_id(PASSWORD_1_PIC);
    if (pic) {
        pic->elm.css.invisible = 1;
    }
    pic = ui_pic_for_id(PASSWORD_2_PIC);
    if (pic) {
        pic->elm.css.invisible = 1;
    }
    pic = ui_pic_for_id(PASSWORD_3_PIC);
    if (pic) {
        pic->elm.css.invisible = 1;
    }
    pic = ui_pic_for_id(PASSWORD_4_PIC);
    if (pic) {
        pic->elm.css.invisible = 1;
    }
    text = ui_text_for_id(PASSWORD_ENTER_TEXT);
    text->elm.css.invisible = 0;
    ui_text_set_index(text, index);
    /* ui_text_show_index_by_id(PASSWORD_ENTER_TEXT, index); */
}

static void password_sure_process()
{
    if (password->status == SET_PASSWORD) {
        if (password->index < POWERON_PASSWORD_LEN - 1) {
            return;
        }
        syscfg_write(USER_PASSWORD, &(password->password), POWERON_PASSWORD_LEN);
        ui_hide(PASSWORD_ENTER_LAYOUT);
        ui_show(PASSWORD_SUCCESS_LAYOUT);
        if (password->timer_id == 0) {
            int layout = PASSWORD_SUCCESS_LAYOUT;
            password->time_cnt = PASSWORD_TIME_COUNT;
            password->timer_id = sys_timer_add((void *)PASSWORD_SUCCESS_LAYOUT, password_timer_show, PASSWORD_TIME_MS);
        }
        memcpy(password->final_password, password->password, sizeof(password->password));
        memset(password->password, 0, sizeof(password->password));
        password->index = 0;
        password->status = OLD_PASSWORD;
    } else if (password->status == OLD_PASSWORD) {
        if (!strcmp(password->password, password->final_password)) {   // 旧密码正确
            ui_hide_password(4);
            memset(password->password, 0, sizeof(password->password));
            password->index = 0;
            password->status = NEW_PASSWORD;
        } else {             // 旧密码错误
            ui_hide_password(2);
            memset(password->password, 0, sizeof(password->password));
            password->index = 0;
        }
    } else if (password->status == NEW_PASSWORD) {     // 输入新密码
        if (password->index < POWERON_PASSWORD_LEN - 1) {
            return;
        }
        ui_hide_password(3);
        memcpy(password->new_password, password->password, sizeof(password->password));
        memset(password->password, 0, sizeof(password->password));
        password->index = 0;
        password->status = NEW_PASSWORD_TWICE;
    } else if (password->status == NEW_PASSWORD_TWICE) {
        if (password->index < POWERON_PASSWORD_LEN - 1) {
            return;
        }
        if (!strcmp(password->password, password->new_password)) {   // 第二次输入新密码成功
            ui_hide(PASSWORD_ENTER_LAYOUT);
            ui_show(PASSWORD_CHANGE_SUCCESS_LAYOUT);
            if (password->timer_id == 0) {
                int layout = PASSWORD_CHANGE_SUCCESS_LAYOUT;
                password->time_cnt = PASSWORD_TIME_COUNT;
                password->timer_id = sys_timer_add((void *)PASSWORD_CHANGE_SUCCESS_LAYOUT, password_timer_show, PASSWORD_TIME_MS);
            }
            //memcpy(password.final_password, password.password, sizeof(password.password));
            syscfg_write(USER_PASSWORD, &(password->password), POWERON_PASSWORD_LEN);
            memset(password->password, 0, sizeof(password->password));
            password->index = 0;
            password->status = OLD_PASSWORD;
        } else {
            ui_hide_password(2);
            memset(password->password, 0, sizeof(password->password));
            password->index = 0;
        }
    }
}


static int set_password_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***set_password_onchange***\n");
        if (!password) {
            password = zalloc(sizeof(struct password_t));
        }
        break;
    case ON_CHANGE_RELEASE:
        char password_check[POWERON_PASSWORD_LEN] = {0};
        syscfg_read(USER_PASSWORD, password_check, POWERON_PASSWORD_LEN);
        if (strlen(password_check) == 0) {
            password->is_password_open = 0;
            syscfg_write(USER_PASSWORD_ON, &(password->is_password_open), 1);
        }

        if (password) {
            if (password->timer_id) {
                sys_timer_del(password->timer_id);
                password->timer_id = 0;
            }
            free(password);
            password = NULL;
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PAGE_PASSWORD)
.onchange = set_password_page_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int set_password_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        syscfg_read(USER_PASSWORD_ON, &(password->is_password_open), 1);
        if (password->is_password_open) {
            ui_show(PASSWORD_CHANGE_LAYOUT);
        } else {
            ui_hide(PASSWORD_CHANGE_LAYOUT);
        }
        syscfg_read(USER_PASSWORD, &(password->final_password), POWERON_PASSWORD_LEN);
        printf("password: %s\n", password->final_password);
        if (strlen(password->final_password) == 0) {
            password->status = SET_PASSWORD;
        } else {
            password->status = OLD_PASSWORD;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(PASSWORD_LAYOUT)
.onchange = set_password_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int set_password_button_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        switch (pic->elm.id) {
        case PASSWORD_SWITCH_BUTTON:
            syscfg_read(USER_PASSWORD_ON, &(password->is_password_open), 1);
            ui_pic_set_image_index(pic, password->is_password_open);
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int set_password_button_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case PASSWORD_SWITCH_BUTTON:
            password->is_password_open = !password->is_password_open;
            ui_pic_show_image_by_id(PASSWORD_SWITCH_BUTTON, password->is_password_open);
            syscfg_write(USER_PASSWORD_ON, &(password->is_password_open), 1);
            if (password->is_password_open) {
                ui_show(PASSWORD_CHANGE_LAYOUT);
            } else {
                set_need_password(0);
                ui_hide(PASSWORD_CHANGE_LAYOUT);
            }
            // 密码开关
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(PASSWORD_SWITCH_BUTTON)
.onchange = set_password_button_onchange,
 .onkey = NULL,
  .ontouch = set_password_button_ontouch,
};

static int set_password_change_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }
        ui_hide(PASSWORD_LAYOUT);
        ui_show(PASSWORD_ENTER_LAYOUT);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(PASSWORD_CHANGE_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = set_password_change_layout_ontouch,
};

static int set_password_enter_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        memset(password->password, 0, sizeof(password->password));
        ui_hide(PASSWORD_ENTER_LAYOUT);
        ui_show(PASSWORD_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(PASSWORD_ENTER_LAYOUT)//通用-垂直列表
.onchange = NULL,
 .onkey = NULL,
  .ontouch = set_password_enter_layout_ontouch,
};

static int set_password_enter_text_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        switch (text->elm.id) {
        case PASSWORD_ENTER_TEXT:
            if (password->status == SET_PASSWORD) {
                ui_text_set_index(text, 0);
            } else if (password->status == OLD_PASSWORD) {
                ui_text_set_index(text, 1);
            }
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

REGISTER_UI_EVENT_HANDLER(PASSWORD_ENTER_TEXT)//通用-垂直列表
.onchange = set_password_enter_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int password_enter_button_ontouch(void *ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctrl;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_UP:
        switch (elm->id) {
        case PASSWORD_BACK_BUTTON:
            printf("[5]DIAL_EXIT UP\n");
            password_back_process();
            break;
        case PASSWORD_DEL_BUTTON:
            printf("[5]DIAL_DEL UP\n");
            set_password_remove_number();
            break;
        case PASSWORD_0_BUTTON:
            printf("[5]DIAL_0 UP\n");
            set_password_add_number('0');
            break;
        case PASSWORD_1_BUTTON:
            printf("[5]DIAL_1 UP\n");
            set_password_add_number('1');
            break;
        case PASSWORD_2_BUTTON:
            printf("[5]DIAL_2 UP\n");
            set_password_add_number('2');
            break;
        case PASSWORD_3_BUTTON:
            printf("[5]DIAL_3 UP\n");
            set_password_add_number('3');
            break;
        case PASSWORD_4_BUTTON:
            printf("[5]DIAL_4 UP\n");
            set_password_add_number('4');
            break;
        case PASSWORD_5_BUTTON:
            printf("[5]DIAL_5 UP\n");
            set_password_add_number('5');
            break;
        case PASSWORD_6_BUTTON:
            printf("[5]DIAL_6 UP\n");
            set_password_add_number('6');
            break;
        case PASSWORD_7_BUTTON:
            printf("[5]DIAL_7 UP\n");
            set_password_add_number('7');
            break;
        case PASSWORD_8_BUTTON:
            printf("[5]DIAL_8 UP\n");
            set_password_add_number('8');
            break;
        case PASSWORD_9_BUTTON:
            printf("[5]DIAL_9 UP\n");
            set_password_add_number('9');
            break;
        case PASSWORD_SURE_BUTTON:
            printf("[5]DIAL_SURE UP\n");
            password_sure_process();
            break;
        default:
            break;
        }
        /* ui_core_highlight_element(elm, false); */
        ui_core_redraw(elm->parent);

        break;
    default:
        break;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(PASSWORD_BACK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};

REGISTER_UI_EVENT_HANDLER(PASSWORD_DEL_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_0_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_1_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_2_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_3_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_4_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_5_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_6_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_7_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_8_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_9_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PASSWORD_SURE_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};

//-----------------------组件----------------------------------//
static void component_add_func(int card_set_num, int index)
{
    ui_sys_param.card_select[card_set_num++] = index;
    set_ui_sys_param(CardSetNum, card_set_num);
}

static void component_del_func(int card_set_num, int index)
{
    int index_del = 0;
    for (int i = 0; i < card_set_num; i++) {
        if (ui_sys_param.card_select[i] == index) {
            index_del = i;
            break;
        }
    }
    for (int i = index_del; i < card_set_num - 1; i++) {
        if (i == card_set_num - 1) {
            break;
        } else {
            ui_sys_param.card_select[i] = ui_sys_param.card_select[i + 1];
        }
    }
    card_set_num--;
    set_ui_sys_param(CardSetNum, card_set_num);
}


static int component_page_ontouch(void *ctr, struct element_touch_event *e)
{
    struct window *window = (struct window *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        if (ui_return_prev_page_id() == ID_WINDOW_SETTING) {
            return false;
        } else {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(ID_WINDOW_DIAL);
            return true;
        }
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_COMPONENT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = component_page_ontouch,
};

static int component_add_button_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case COMPONENT_ADD_BUTTON:
            ui_hide(COMPONENT_ADD_LAYOUT);
            ui_show(COMPONENT_ADD_ITEM_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(COMPONENT_ADD_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = component_add_button_ontouch,
};

u8 vlist_card_index[15] = {0};
static int component_add_item_vlist_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (int)arg;
        if (ui_id2type(elm->id) == CTRL_TYPE_TEXT) {
            ui_text_set_index((struct ui_text *)elm, index);
        }

    }
    return false;
}

static int component_add_item_vlist_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        break;
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        int row = 13;
        int col = 1;
        ui_set_default_handler(elm, NULL, NULL, component_add_item_vlist_child_onchange);
        ui_grid_init_dynamic(grid, &row, &col);
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(COMPONENT_ADD_ITEM_VLIST)
.onchange = component_add_item_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int component_item_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        if ((index < 0) || (index > 12)) {
            break;
        }
        switch (pic->elm.id) {
        case COMPONENT_SEL0:
            vlist_card_index[0] = index;
            break;
        case COMPONENT_SEL1:
            vlist_card_index[1] = index;
            break;
        case COMPONENT_SEL2:
            vlist_card_index[2] = index;
            break;
        case COMPONENT_SEL3:
            vlist_card_index[3] = index;
            break;
        case COMPONENT_SEL4:
            vlist_card_index[4] = index;
            break;
        case COMPONENT_SEL5:
            vlist_card_index[5] = index;
            break;
        default:
            break;
            break;
        }

        if (!ui_page_search(ui_page_list[index])) {
            ui_pic_set_image_index(pic, 0);
        } else {
            ui_pic_set_image_index(pic, 1);
        }
    default:
        break;
    }
    return 0;
}

static int component_item_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;
    int window_id = 0;
    int index = 0;
    int item_sel = 0;
    int card_set_num = get_ui_sys_param(CardSetNum);

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case COMPONENT_SEL0:
            index = vlist_card_index[0];
            item_sel = 0;
            break;
        case COMPONENT_SEL1:
            index = vlist_card_index[1];
            item_sel = 1;
            break;
        case COMPONENT_SEL2:
            index = vlist_card_index[2];
            item_sel = 2;
            break;
        case COMPONENT_SEL3:
            index = vlist_card_index[3];
            item_sel = 3;
            break;
        case COMPONENT_SEL4:
            index = vlist_card_index[4];
            item_sel = 4;
            break;
        case COMPONENT_SEL5:
            index = vlist_card_index[5];
            item_sel = 5;
            break;
        default:
            return false;
        }

        window_id = ui_page_list[index];
        if (!ui_page_search(window_id)) {
            ui_page_add(window_id);
            component_add_func(card_set_num, index);
        } else {
            ui_page_del(window_id);
            component_del_func(card_set_num, index);
        }

        card_set_num = get_ui_sys_param(CardSetNum);

        for (u8 i = 0; i < card_set_num; i++) {
            ui_show_page_list[i + card_start_index] = ui_page_list[ui_sys_param.card_select[i]];
        }
        if (UIInfo_w_vm_timer == 0) {
            UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
        } else {
            sys_timer_re_run(UIInfo_w_vm_timer);
        }

        ui_grid_update_by_id_dynamic(COMPONENT_ADD_ITEM_VLIST, item_sel, true);

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(COMPONENT_SEL0)
.onchange = component_item_onchange,
 .onkey = NULL,
  .ontouch = component_item_ontouch,
};
REGISTER_UI_EVENT_HANDLER(COMPONENT_SEL1)
.onchange = component_item_onchange,
 .onkey = NULL,
  .ontouch = component_item_ontouch,
};
REGISTER_UI_EVENT_HANDLER(COMPONENT_SEL2)
.onchange = component_item_onchange,
 .onkey = NULL,
  .ontouch = component_item_ontouch,
};
REGISTER_UI_EVENT_HANDLER(COMPONENT_SEL3)
.onchange = component_item_onchange,
 .onkey = NULL,
  .ontouch = component_item_ontouch,
};
REGISTER_UI_EVENT_HANDLER(COMPONENT_SEL4)
.onchange = component_item_onchange,
 .onkey = NULL,
  .ontouch = component_item_ontouch,
};
REGISTER_UI_EVENT_HANDLER(COMPONENT_SEL5)
.onchange = component_item_onchange,
 .onkey = NULL,
  .ontouch = component_item_ontouch,
};

//-----------------------APP试图界面---------------------------//
void app_show_pic(u8 init, u8 mode)
{
    u8 menu_style;

    if (init == 0) {
        menu_style = mode;
    } else {
        menu_style = get_ui_sys_param(MenuStyle);
    }

    struct ui_pic *pic = NULL;
    pic = ui_pic_for_id(SETTING_APP_SHOW_PIC0);
    if (pic) {
        ui_pic_set_image_index(pic, 0);
    }
    pic = ui_pic_for_id(SETTING_APP_SHOW_PIC1);
    if (pic) {
        ui_pic_set_image_index(pic, 0);
    }
    pic = ui_pic_for_id(SETTING_APP_SHOW_PIC2);
    if (pic) {
        ui_pic_set_image_index(pic, 0);
    }

    if (menu_style == 0) {
        pic = ui_pic_for_id(SETTING_APP_SHOW_PIC0);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
    } else if (menu_style == 1) {
        pic = ui_pic_for_id(SETTING_APP_SHOW_PIC1);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
    } else {
        pic = ui_pic_for_id(SETTING_APP_SHOW_PIC2);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
    }
}

static int set_app_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SETTING_APP_SHOW_LAYOUT);
        ui_show(SETTING_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_APP_SHOW_LAYOUT)//通用-垂直列表
.onchange = NULL,
 .onkey = NULL,
  .ontouch = set_app_layout_ontouch,
};

static int set_app_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int item = 0;

    switch (event) {
    case ON_CHANGE_INIT:
        app_show_pic(1, 0);

        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        item = ui_show_menu_get_index(NULL);
        if (item < 0) {
            item = 0;
        }
        /* printf("\n item init:%d \n", item); */
        ui_grid_set_hi_index(grid, item);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int set_app_vlist_ontouch(void *ctr, struct element_touch_event *e)
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

        app_show_pic(0, sel_item);
        /* printf("\n item sw:%d \n", sel_item); */
        if (ui_show_menu_sw(sel_item)) {
            ui_grid_set_hi_index(grid, sel_item);
            ui_core_redraw(grid->elm.parent);
        }
        if (UIInfo_w_vm_timer == 0) {
            UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
        } else {
            sys_timer_re_run(UIInfo_w_vm_timer);
        }
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        return true;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_ENERGY:
        return true;
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_APP_SHOW_VLIST)//通用-垂直列表
.onchange =  set_app_vlist_onchange,
 .onkey = NULL,
  .ontouch = set_app_vlist_ontouch,
};

//-----------------------转场动画界面---------------------------//
static int set_animation_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SETTING_ANIMATION_LAYOUT);
        ui_show(SETTING_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_LAYOUT)//通用-垂直列表
.onchange = NULL,
 .onkey = NULL,
  .ontouch = set_animation_layout_ontouch,
};

static const int card_anim_pic[] = {
    SETTING_ANIMATION_PIC0,
    SETTING_ANIMATION_PIC1,
    SETTING_ANIMATION_PIC2,
    SETTING_ANIMATION_PIC3,
    SETTING_ANIMATION_PIC4,
    SETTING_ANIMATION_PIC5,
    SETTING_ANIMATION_PIC6,
    SETTING_ANIMATION_PIC7,
    SETTING_ANIMATION_PIC8,
    SETTING_ANIMATION_PIC9,
    SETTING_ANIMATION_PIC10,
    SETTING_ANIMATION_PIC11,
};

static void animation_pic_show()
{
    struct ui_pic *pic = NULL;
    u8 idx = get_ui_sys_param(move_mode);
    for (int i = 0; i < ARRAY_SIZE(card_anim_pic); i++) {
        pic = ui_pic_for_id(card_anim_pic[i]);
        if (pic) {
            if (idx == i) {
                ui_pic_set_image_index(pic, 1);
            } else {
                ui_pic_set_image_index(pic, 0);
            }
        }
    }
}

static int set_animation_vlist_ontouch(void *ctr, struct element_touch_event *e)
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
        if (ui_card_anim_sw(sel_item)) {
            animation_pic_show();
        }
        ui_core_redraw(grid->elm.parent);

        if (UIInfo_w_vm_timer == 0) {
            UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
        } else {
            sys_timer_re_run(UIInfo_w_vm_timer);
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

REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_VLIST)//通用-垂直列表
.onchange = setting_vlist_default_onchange,
 .onkey = NULL,
  .ontouch = set_animation_vlist_ontouch,
};

static int setting_animation_pic_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 idx = get_ui_sys_param(move_mode);
    if (idx < 0) {
        idx = 0;
    }

    switch (event) {
    case ON_CHANGE_INIT:
        for (int i = 0; i < ARRAY_SIZE(card_anim_pic); i++) {
            if (pic->elm.id == card_anim_pic[i]) {
                if (idx == i) {
                    ui_pic_set_image_index(pic, 1);
                } else {
                    ui_pic_set_image_index(pic, 0);
                }
            }
        }
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC0)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC1)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC2)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC3)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC4)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC5)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC6)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC7)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC8)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC9)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC10)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SETTING_ANIMATION_PIC11)//通用-垂直列表
.onchange = setting_animation_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

//-----------------------蓝牙设置---------------------------------//
#if TCFG_USER_BT_CLASSIC_ENABLE
static int ui_edr_button_handler(const char *type, u32 arg)
{
    log_info("%s arg:%d", __func__, arg);
    ui_pic_show_image_by_id(SETTING_EDR_BUTTON, arg);
    return true;
}

static int ui_ble_button_handler(const char *type, u32 arg)
{
    log_info("%s arg:%d", __func__, arg);
    ui_pic_show_image_by_id(SETTING_BLE_BUTTON, arg);
    return true;
}

static const struct uimsg_handl bt_button_msg_handler[] = {
    { "edr_button",          ui_edr_button_handler         },
    { "ble_button",          ui_ble_button_handler         },
    { NULL, NULL},      /* 必须以此结尾！ */
};

static int set_edr_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        ui_register_msg_handler(ID_WINDOW_SETTING, bt_button_msg_handler);
        break;
    case ON_CHANGE_RELEASE:
        ui_register_msg_handler(ID_WINDOW_SETTING, NULL);
        break;
    default:
        break;
    }
    return false;
}
static int set_edr_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SETTING_EDR_LAYOUT);
        ui_show(SETTING_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SETTING_EDR_LAYOUT)//通用-垂直列表
.onchange = set_edr_layout_onchange,
 .onkey = NULL,
  .ontouch = set_edr_layout_ontouch,
};

static int setting_edr_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        switch (pic->elm.id) {
        case SETTING_EDR_BUTTON:
            if (is_bredr_close()) {
                ui_pic_set_image_index(pic, 0);
            } else {
                ui_pic_set_image_index(pic, 1);
            }
            break;
        case SETTING_BLE_BUTTON:
#if (BT_AI_SEL_PROTOCOL & RCSP_MODE_EN)
            if (!bt_ble_get_adv_enable()) {
                ui_pic_set_image_index(pic, 0);
            } else {
                ui_pic_set_image_index(pic, 1);
            }
#endif
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int setting_edr_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SETTING_EDR_BUTTON:
            if (is_bredr_close()) {
                bredr_conn_last_dev();
                ui_pic_show_image_by_id(SETTING_EDR_BUTTON, 1);
            } else {
                bt_close_bredr();
                ui_pic_show_image_by_id(SETTING_EDR_BUTTON, 0);
            }
            break;
        case SETTING_BLE_BUTTON:
#if (BT_AI_SEL_PROTOCOL & RCSP_MODE_EN)
            /*与手机已经连接了，从机无广播了，无需操作*/
            if (bt_rcsp_device_conn_num() > 0) {
                break;
            }

            if (!bt_ble_get_adv_enable()) {
                ble_module_enable(1);
                ui_pic_show_image_by_id(SETTING_BLE_BUTTON, 1);
            } else {
                ble_module_enable(0);
                ui_pic_show_image_by_id(SETTING_BLE_BUTTON, 0);
            }
#endif
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_EDR_BUTTON)
.onchange = setting_edr_onchange,
 .onkey = NULL,
  .ontouch = setting_edr_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SETTING_BLE_BUTTON)
.onchange = setting_edr_onchange,
 .onkey = NULL,
  .ontouch = setting_edr_ontouch,
};
#endif /* #if TCFG_USER_BT_CLASSIC_ENABLE */

//-----------------------灵动岛开关界面---------------------------//
static int dynamic_island = 0;
static int dynamic_island_switch_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        switch (pic->elm.id) {
        case SETTING_DYNAMIC_ISLAND_BUTTON:
#if TCFG_UI_ENABLE_SMARTWIN
            extern u8 smartwin_if_enable(void);
            dynamic_island = smartwin_if_enable();
#endif
            ui_pic_set_image_index(pic, dynamic_island);
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int dynamic_island_switch_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case SETTING_DYNAMIC_ISLAND_BUTTON:
            dynamic_island = !dynamic_island;
#if TCFG_UI_ENABLE_SMARTWIN
            extern void smartwin_if_enable_set(bool enable);
            smartwin_if_enable_set(dynamic_island);
            watch_syscfg_write("sys_param", 0);			//设置完之后将数据写到vm
#endif
            ui_pic_show_image_by_id(SETTING_DYNAMIC_ISLAND_BUTTON, dynamic_island);
            // 灵动岛
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(SETTING_DYNAMIC_ISLAND_BUTTON)
.onchange = dynamic_island_switch_onchange,
 .onkey = NULL,
  .ontouch = dynamic_island_switch_ontouch,
};

//-------------------重启关机页面-------------------------//
static int restart_shutdown_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case RESTART_BUTTON:
            app_var.goto_reboot_flag = 1;
            watch_reboot_or_shutdown(1, 0);
            break;
        case SHUTDOWN_BUTTON:
            watch_reboot_or_shutdown(0, 0);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(RESTART_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = restart_shutdown_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SHUTDOWN_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = restart_shutdown_ontouch,
};

//------------------低电量提醒-------------------//
struct low_power_t {
    u32 low_power_timer;
};
static struct low_power_t *low_power = NULL;

static void low_power_to(void *priv)
{
    UI_WINDOW_PREEMPTION_POP(ID_WINDOW_LOW_POWER_TIPS);

    if (low_power && low_power->low_power_timer) {
        sys_timeout_del(low_power->low_power_timer);
        low_power->low_power_timer = 0;
    }
}

static int low_power_tips_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:
        if (!low_power) {
            low_power = zalloc(sizeof(struct low_power_t));
        }
        if (low_power && !low_power->low_power_timer) {
            low_power->low_power_timer = sys_timeout_add(NULL, low_power_to, 5000);
        }
        ui_auto_shut_down_enable();
        break;
    case ON_CHANGE_RELEASE:
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_LOW_POWER_TIPS);

        if (low_power && low_power->low_power_timer) {
            sys_timeout_del(low_power->low_power_timer);
            low_power->low_power_timer = 0;
        }
        if (low_power) {
            free(low_power);
            low_power = NULL;
        }

        break;
    default:
        break;
    }
    return 0;
}

static int low_power_tips_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (text->elm.id == LOW_POWER_TIPS_BUTTON) {
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_LOW_POWER_TIPS);
        }
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(LOW_POWER_TIPS_BUTTON)//开始时间-文字控件(确定)
.onchange = low_power_tips_onchange,
 .onkey = NULL,
  .ontouch = low_power_tips_ontouch,
};


//------------------蓝牙断开提醒-------------------//
struct bt_disconn_t {
    u32 bt_disconn_timer;
};

static struct bt_disconn_t *bt_disconn = NULL;

static void bt_disconn_to(void *priv)
{
    UI_WINDOW_PREEMPTION_POP(ID_WINDOW_BT_DISCONN_TIPS);

    if (bt_disconn && bt_disconn->bt_disconn_timer) {
        sys_timeout_del(bt_disconn->bt_disconn_timer);
        bt_disconn->bt_disconn_timer = 0;
    }
}

static int bt_diconn_tips_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:
        if (!bt_disconn) {
            bt_disconn = zalloc(sizeof(struct bt_disconn_t));
        }
        if (bt_disconn && !bt_disconn->bt_disconn_timer) {
            bt_disconn->bt_disconn_timer = sys_timeout_add(NULL, bt_disconn_to, 5000);
        }
        ui_auto_shut_down_enable();
        break;
    case ON_CHANGE_RELEASE:
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_BT_DISCONN_TIPS);

        if (bt_disconn && bt_disconn->bt_disconn_timer) {
            sys_timeout_del(bt_disconn->bt_disconn_timer);
            bt_disconn->bt_disconn_timer = 0;
        }
        if (bt_disconn) {
            free(bt_disconn);
            bt_disconn = NULL;
        }
        break;
    default:
        break;
    }
    return 0;
}

static int bt_diconn_tips_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (text->elm.id == BT_DISCONN_SURE_BUTTON) {
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_BT_DISCONN_TIPS);
        }
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(BT_DISCONN_SURE_BUTTON)//开始时间-文字控件(确定)
.onchange = bt_diconn_tips_onchange,
 .onkey = NULL,
  .ontouch = bt_diconn_tips_ontouch,
};


#endif
#endif
