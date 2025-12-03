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
#include "custom_cfg.h"
#include "syscfg_id.h"
#include "ui_page_switch.h"
#include "rtc.h"
#include "events_adapter.h"
#include "user_cfg.h"
#include "app_main.h"
#include "smartbox_info_manager.h"

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

#if (defined (CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
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
#define SET_VLIST_LOOP_EN                   (1)    // 列表循环

extern int UIInfo_w_vm_timer;

struct set_func_t {
    u8 is_24_hour;   // 24小时制
    u8 raise_hand_index;    // 抬手亮屏
    u8 screen_off_index;    // 熄屏表盘
    u8 bed_light_index;     // 床头灯
    u8 health_tip;      // 久坐提醒
    u8 low_power_mode;   // 省电模式
};

int card_start_index = 0;

static struct set_func_t sec_func = {.is_24_hour = 1};
static bool page_90_top_update_flage = true;

#if (defined TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE) && TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE
u32 ui_show_page_list[50] = {};
#else
u32 ui_show_page_list[9] = {ID_WINDOW_DIAL};
#endif

const u32 ui_page_list[] = {
    ID_WINDOW_MUSIC_PLAYER,
    ID_WINDOW_VOLUME,
    ID_WINDOW_EARPHONE_DISNOISE,
    ID_WINDOW_EQUALIZER,
    // ID_WINDOW_TIMER ,
    ID_WINDOW_LCD_BRIGHTNESS,
    ID_WINDOW_LANGUAGE,
    // ID_WINDOW_BG_SELECT ,
    ID_WINDOW_LOCK_SELECT,
    ID_WINDOW_FIND_EARPHONE,
    ID_WINDOW_TIME_SETTING,
    ID_WINDOW_TIKTOK,
    ID_WINDOW_PHOTOGRAGH,
    ID_WINDOW_ALARM_CLOCK,
};

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
    ASSERT(time);
    rtc_read_time(time);
}

static void set_sys_time(struct sys_time *time)
{
    ASSERT(time);
    rtc_write_time(time);
}

void enter_low_power_mode()
{
    u8 brightness_level = MIN_LIGHTLEVEL;
    set_is_low_power_mode(1);
    //set_ui_sys_param(LightLevel, brightness_level);
    // ui_ajust_light(brightness_level * 2);
    ui_ajust_light(brightness_level);
}

void exit_low_power_mode()
{
    u8 brightness_level = get_ui_sys_param(LightLevel);
    set_is_low_power_mode(0);
    set_ui_sys_param(LightLevel, brightness_level);
    //ui_ajust_light(brightness_level * 2);
    ui_ajust_light(brightness_level);
}

void setting_write_UIInfo_to_vm(void *info)
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

static void ui_default_param_set()
{
    u8 i, k;
    u8 card_set_num = 0;

    for (i = 0; i < sizeof(ui_page_list) / sizeof(u32); i++) {
        if (ui_page_list[i] != 0) {
            printf("-----------------%s %d card_set_num %d\n", __func__, __LINE__, card_set_num);
            ui_sys_param.card_select[card_set_num++] = i;
        }
    }
    printf("-----------------%s %d card_set_num %d\n", __func__, __LINE__, card_set_num);

    set_ui_sys_param(CardSetNum, card_set_num);
}


void ui_sysinfo_init()
{
    u8 card_set_num;
    int ret;
    ret = read_UIInfo_from_vm();
    if (ret < 0) {
        log_info("read_UIInfo_from_vm err");
    }
    ui_default_param_set();

    ui_moto_init(TCFG_MOTO_PWM_IO);
    ui_set_voice(get_ui_sys_param(LastSysVol));
    //ui_ajust_light(get_ui_sys_param(LightLevel) * 2);
    ui_ajust_light(get_ui_sys_param(LightLevel));
    ui_set_dark_time(get_ui_sys_param(DarkTime));
    if (get_ui_sys_param(low_power_mode)) {
        enter_low_power_mode();
    }
    card_set_num = get_ui_sys_param(CardSetNum);

    g_printf("card_set_num %d", card_set_num);

    for (card_start_index = 0; card_start_index < sizeof(ui_show_page_list) / sizeof(ui_show_page_list[0]); card_start_index++) {
        if (ui_show_page_list[card_start_index] == 0) {
            break;
        }
    }

    if (card_set_num != 0) {
        for (int i = 0; i < card_set_num; i++) {
            ui_show_page_list[i + card_start_index] = ui_page_list[ui_sys_param.card_select[i]];
            printf("---------------------------------ui_page_list[%d] = 0x%x\n", ui_sys_param.card_select[i], ui_page_list[ui_sys_param.card_select[i]]);
            log_info("page:0x%x %d", ui_show_page_list[i + card_start_index], ui_sys_param.card_select[i]);
            ui_page_add(ui_page_list[ui_sys_param.card_select[i]]);
        }
    }

    ui_page_list_update(ui_show_page_list, card_set_num + card_start_index);
    watch_set_style(get_ui_sys_param(curr_sel_dial));

    select_strfile(0);
    u8 language = csc_get_ui_language_type();
    log_info("%s set language %d", __func__, language);
    ui_language_set(language);   //开机初始化语言
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
    ASSERT(grid);
    if (item >= grid->avail_item_num - 1) {
        ui_grid_set_hi_index(grid, item - 1);
    } else {
        ui_grid_set_hi_index(grid, item + 1);
    }
}

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
    ASSERT(addr && buf);
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
    ASSERT(str);
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





//-----------------------连接新手机界面---------------------------//
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

//-----------------------时间设置界面---------------------------//
struct set_time_t {
    u8 make_sure;
    u8 dynamic_day_num;
    u32 vlist_timer;
    struct sys_time curtime;

};
static struct set_time_t *set_time = NULL;

static int set_date_ok_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        set_time->make_sure = 1;
        // if (text->elm.id == SETTING_DATE_SURE_BUTTON) {
        //     // 设置时间
        //     ui_hide(SETTING_DATE_SET_LAYOUT);
        //     ui_show(SETTING_TIME_SET_LAYOUT);
        // }
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
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_SURE_BUTTON)
// .onchange = NULL,
//  .onkey = NULL,
//   .ontouch = set_date_ok_ontouch,
// };

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

        base_index_once = ((time_year >= 1) ? (time_year - 1) : 0);

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = r.height + 5;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        if (set_time && set_time->make_sure) {
            // if (grid->elm.id == SETTING_DATE_YEAR_VLIST) {
            //     year = ui_grid_get_hindex_dynamic(grid);
            //     set_time->curtime.year = year;
            //     set_sys_time(&(set_time->curtime));
            // }
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
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_YEAR_VLIST)
// .onchange = set_date_year_vlist_onchange,
//  .onkey = default_vlist_onkey,
//   .ontouch = NULL,
// };

static int set_date_y_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    /* u8 index_buf; */
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
        // switch (pic->elm.id) {
        // case SETTING_YEAR_Y00:
        // case SETTING_YEAR_Y10:
        // case SETTING_YEAR_Y20:
        // case SETTING_YEAR_Y30:
        //     index_buf = index / 1000;
        //     break;
        // case SETTING_YEAR_Y01:
        // case SETTING_YEAR_Y11:
        // case SETTING_YEAR_Y21:
        // case SETTING_YEAR_Y31:
        //     index_buf = index / 100 % 10;
        //     break;
        // case SETTING_YEAR_Y02:
        // case SETTING_YEAR_Y12:
        // case SETTING_YEAR_Y22:
        // case SETTING_YEAR_Y32:
        //     index_buf = index / 10 % 10;
        //     break;
        // case SETTING_YEAR_Y03:
        // case SETTING_YEAR_Y13:
        // case SETTING_YEAR_Y23:
        // case SETTING_YEAR_Y33:
        //     index_buf = index % 10;
        //     break;
        // }
        /* ui_pic_set_image_index(pic, index_buf); */
        break;
    default:
        break;
    }
    return FALSE;
}

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
    ASSERT(time);
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

        if (time_day == 0) {
            ui_grid_set_hindex_dynamic(grid, time_day, true, 0);
        } else if (time_day == 30) {
            ui_grid_set_hindex_dynamic(grid, time_day, true, 3);
        } else if (time_day == 29) {
            ui_grid_set_hindex_dynamic(grid, time_day, true, 2);
        } else {
            ui_grid_set_hindex_dynamic(grid, time_day, true, 1);
        }

        base_index_once = (time_day >= 1) ? (time_day - 1) : 0;

        if (time_day == 30) {
            base_index_once = time_day - 3;
        } else if (time_day == 29) {
            base_index_once = time_day - 2;
        }

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        ui_grid_set_base_dynamic(grid, base_index_once, r.height + 5);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);

        break;
    case ON_CHANGE_RELEASE:
        if (set_time && set_time->make_sure) {
            // if (grid->elm.id == SETTING_DATE_DAY_VLIST) {
            //     day = ui_grid_get_hindex_dynamic(grid) % 32;
            //     set_time->curtime.day = day + 1;
            //     set_sys_time(&(set_time->curtime));
            // }
        }
        printf("@@@@@day %d\n", set_time->curtime.day);
        break;
    default:
        break;
    }
    return 0;
}
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_DAY_VLIST)
// .onchange = set_date_day_vlist_onchange,
//  .onkey = NULL,
//   .ontouch = NULL,
// };

static int set_date_d_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf = 0;
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
            // case SETTING_DATE_D00:
            // case SETTING_DATE_D10:
            // case SETTING_DATE_D20:
            // case SETTING_DATE_D30:
            index_buf = index / 10;
            break;
            // case SETTING_DATE_D01:
            // case SETTING_DATE_D11:
            // case SETTING_DATE_D21:
            // case SETTING_DATE_D31:
            //     index_buf = index % 10;
            //     break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return false;
}
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D00)
// .onchange = set_date_d_onchange,
//  .onkey = NULL,
//   .ontouch = NULL,
// };
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D01)
// .onchange = set_date_d_onchange,
//  .onkey = NULL,
//   .ontouch = NULL,
// };
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D10)
// .onchange = set_date_d_onchange,
//  .onkey = NULL,
//   .ontouch = NULL,
// };
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D11)
// .onchange = set_date_d_onchange,
//  .onkey = NULL,
//   .ontouch = NULL,
// };
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D20)
// .onchange = set_date_d_onchange,
//  .onkey = NULL,
//   .ontouch = NULL,
// };
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D21)
// .onchange = set_date_d_onchange,
//  .onkey = NULL,
//   .ontouch = NULL,
// };
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D30)
// .onchange = set_date_d_onchange,
//  .onkey = NULL,
//   .ontouch = NULL,
// };
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_D31)
// .onchange = set_date_d_onchange,
//  .onkey = NULL,
//   .ontouch = NULL,
// };


static void month_check(void *priv)
{
    struct ui_grid *grid_month = NULL;
    struct ui_grid *grid_day = NULL;
    struct ui_grid *grid_year = NULL;

    // grid_month = ui_grid_for_id(SETTING_DATE_MONTH_VLIST);
    // grid_day = ui_grid_for_id(SETTING_DATE_DAY_VLIST);
    // grid_year = ui_grid_for_id(SETTING_DATE_YEAR_VLIST);
    ASSERT(grid_month && grid_day && grid_year);
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

        if (time_month == 0) {
            ui_grid_set_hindex_dynamic(grid, time_month, true, 0);
        } else if (time_month == 11) {
            ui_grid_set_hindex_dynamic(grid, time_month, true, 3);
        } else if (time_month == 10) {
            ui_grid_set_hindex_dynamic(grid, time_month, true, 2);
        } else {
            ui_grid_set_hindex_dynamic(grid, time_month, true, 1);
        }

        base_index_once = (time_month >= 1) ? (time_month - 1) : 0;

        if (time_month == 11) {
            base_index_once = time_month - 3;
        } else if (time_month == 10) {
            base_index_once = time_month - 2;
        }

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        ui_grid_set_base_dynamic(grid, base_index_once, r.height + 5);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);

        break;
    case ON_CHANGE_RELEASE:
        if (set_time && set_time->vlist_timer) {
            sys_timer_del(set_time->vlist_timer);
            set_time->vlist_timer = 0;
        }
        if (set_time && set_time->make_sure) {
            // if (grid->elm.id == SETTING_DATE_MONTH_VLIST) {
            //     month = ui_grid_get_hindex_dynamic(grid) % 13;
            //     set_time->curtime.month = month + 1;
            //     set_sys_time(&(set_time->curtime));
            // }
        }
        printf("@@@@@month %d\n", set_time->curtime.month);
        break;
    default:
        break;
    }
    return 0;
}
// REGISTER_UI_EVENT_HANDLER(SETTING_DATE_MONTH_VLIST)
// .onchange = set_date_month_vlist_onchange,
//  .onkey = NULL,
//   .ontouch = NULL,
// };

static int set_date_m_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    /* u8 index_buf; */
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
            // case SETTING_MONTH_M00:
            // case SETTING_MONTH_M10:
            // case SETTING_MONTH_M20:
            // case SETTING_MONTH_M30:
            //     index_buf = index / 10;
            //     break;
            // case SETTING_MONTH_M01:
            // case SETTING_MONTH_M11:
            // case SETTING_MONTH_M21:
            // case SETTING_MONTH_M31:
            //     index_buf = index % 10;
            //     break;
        }
        /* ui_pic_set_image_index(pic, index_buf); */
        break;
    default:
        break;
    }
    return FALSE;
}



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



//-----------------------亮度控制界面---------------------------//
static int brightness_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    int brightness_level = 0;
    int brightness_level_max = 0;
    int light_level = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }

        struct ui_pic *pic_brightness = (struct ui_pic *)ui_core_get_element_by_id(BRIGHTNESS_PIC);
        if (!pic_brightness) {
            break;
        }

        brightness_level = pic_brightness->index + 1;
        brightness_level_max = ui_pic_get_normal_image_number_by_id(pic_brightness->elm.id);

        switch (pic->elm.id) {
        case BRIGHTNESS_SUBSTRACT_BUTTON:
            if (brightness_level <= 1) {
                break;
            }
            --brightness_level;
            break;
        case BRIGHTNESS_ADD_BUTTON:
            if (brightness_level >= brightness_level_max) {
                break;
            }
            ++brightness_level;
            break;
        }

        light_level = (float)brightness_level / brightness_level_max * UI_LIGHT_LEVEL_MAX;
        set_ui_sys_param(LightLevel, light_level);
        sbox_backlight_level_set(light_level);

        ui_ajust_light(light_level);

        ui_pic_show_image_by_id(pic_brightness->elm.id, brightness_level - 1);

        if (UIInfo_w_vm_timer == 0) {
            UIInfo_w_vm_timer = sys_timer_add(NULL, setting_write_UIInfo_to_vm, 1000);
        } else {
            sys_timer_re_run(UIInfo_w_vm_timer);
        }

        log_info("%s light_level:%d brightness_level:%d", __func__, light_level, brightness_level);
        return true;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(BRIGHTNESS_ADD_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = brightness_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BRIGHTNESS_SUBSTRACT_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = brightness_ontouch,
};


static int ui_brightness_handler(const char *type, u32 arg)
{
    int brightness_level = get_ui_sys_param(LightLevel);
    float parent = (float)brightness_level / (float)UI_LIGHT_LEVEL_MAX;
    int image_number = ui_pic_get_normal_image_number_by_id(BRIGHTNESS_PIC);
    int index = image_number * parent + 0.5f;
    ui_pic_show_image_by_id(BRIGHTNESS_PIC, (index - 1));
    return true;
}
static const struct uimsg_handl ui_brightness_msg_handler[] = {
    { "ui_brightness",          ui_brightness_handler         },
    { NULL, NULL},      /* 必须以此结尾！ */
};

static int page_brightness_pic_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        int brightness_level = get_ui_sys_param(LightLevel);
        float parent = (float)brightness_level / (float)UI_LIGHT_LEVEL_MAX;
        int image_number = ui_pic_get_normal_image_number_by_id(pic->elm.id);
        int index = image_number * parent + 0.5f;
        ui_pic_set_image_index(pic, (index - 1));
        ui_register_msg_handler(ID_WINDOW_LCD_BRIGHTNESS, ui_brightness_msg_handler);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(BRIGHTNESS_PIC)
.onchange = page_brightness_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



//-----------------------抖音页面-------------------------//
#if (defined TCFG_UI_TIKTOK_ENABLE) && TCFG_UI_TIKTOK_ENABLE
#include "screen_trans/smartbox_user_app.h"

typedef enum {
    ICON_STATUS_DISCONNECT = 0,
    ICON_STATUS_CONNECT,
    ICON_STATUS_PRESSING,
} ICON_STATUS;

static int tiktok_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        if (sbox_ble_connect_flag_get()) {
            ui_pic_set_image_index(pic, 1);
        } else {
            ui_pic_set_image_index(pic, 0);
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}


static int tiktok_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    u8 data = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }

        if (!sbox_ble_connect_flag_get()) {
            return false;
        }

        switch (pic->elm.id) {
        case TIKTOK_PIC:
            ui_pic_show_image_by_id(TIKTOK_PIC, 1);
            os_time_dly(30);
            data = 4;
            custom_client_send_ctrl_tiktop(data);
            ui_pic_show_image_by_id(TIKTOK_PIC, 0);
            break;
        case TIKTOK_NEXT_PIC:
            data = 1;
            custom_client_send_ctrl_tiktop(data);
            break;
        case TIKTOK_PREVIOUS_PIC:
            data = 0;
            custom_client_send_ctrl_tiktop(data);
            break;
        default:
            break;
        }
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIKTOK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = tiktok_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TIKTOK_NEXT_PIC)
.onchange = tiktok_onchange,
 .onkey = NULL,
  .ontouch = tiktok_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TIKTOK_PREVIOUS_PIC)
.onchange = tiktok_onchange,
 .onkey = NULL,
  .ontouch = tiktok_ontouch,
};


void ui_tiktok_window_bt_status_update(void)
{
    struct ui_pic *pic = NULL;

    pic = (struct ui_pic *)ui_core_get_element_by_id(TIKTOK_PREVIOUS_PIC);
    if (pic) {
        ui_pic_set_image_index(pic, sbox_ble_connect_flag_get() ? ICON_STATUS_CONNECT : ICON_STATUS_DISCONNECT);
    }

    pic = (struct ui_pic *)ui_core_get_element_by_id(TIKTOK_NEXT_PIC);
    if (pic) {
        ui_pic_set_image_index(pic, sbox_ble_connect_flag_get() ? ICON_STATUS_CONNECT : ICON_STATUS_DISCONNECT);
    }
}

#endif //TCFG_UI_TIKTOK_ENABLE

//-------------------关机-重启-恢复出厂-船运模式页面-------------------------//
void ui_enter_ship_mode(void)
{
    log_info("shipping mode!\n");

    rtc_dev_deinit();     // 关闭rtc

    power_control(PCONTROL_SF_KEEP_LRC, 0);   // 关闭lrc

    power_control(PCONTROL_SF_VDDIO_KEEP, VDDIO_KEEP_TYPE_CLOSE);   // 关闭vddio

    watch_reboot_or_shutdown(0, 0);   // 软关机
}

static int shutdown_reboot_reset_ShippingMode_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }

        switch (pic->elm.id) {
        case SHUTDOWN_PIC:
            watch_reboot_or_shutdown(0, 0);
            break;
        case REBOOT_PIC:
            app_var.goto_reboot_flag = 1;
            watch_reboot_or_shutdown(1, 0);
            break;
        case RESET_PIC:
            app_var.goto_reboot_flag = 1;
            watch_reboot_or_shutdown(1, 1);
            break;
        case SHIPPING_MODE_PIC:
            ui_enter_ship_mode();
            break;
        default:
            break;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SHUTDOWN_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = shutdown_reboot_reset_ShippingMode_ontouch,
};
REGISTER_UI_EVENT_HANDLER(REBOOT_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = shutdown_reboot_reset_ShippingMode_ontouch,
};
REGISTER_UI_EVENT_HANDLER(RESET_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = shutdown_reboot_reset_ShippingMode_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SHIPPING_MODE_PIC)  // 船运模式
.onchange = NULL,
 .onkey = NULL,
  .ontouch = shutdown_reboot_reset_ShippingMode_ontouch,
};

#endif
#endif//CONFIG_UI_STYLE_JL_SCREEN_BOX_PUBLIC_MODLS_ENABLE
