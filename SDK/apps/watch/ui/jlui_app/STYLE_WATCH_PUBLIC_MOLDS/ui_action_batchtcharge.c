#include "app_config.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "app_mode_manager/app_mode_manager.h"
#include "app_task.h"
#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-BATCHARGE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_batcharge.data.bss")
#pragma data_seg(".ui_action_batcharge.data")
#pragma const_seg(".ui_action_batcharge.text.const")
#pragma code_seg(".ui_action_batcharge.text")
#endif


#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_ENABLE_BATCHARGE

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

#define BAT_CHARGE_AUTO_EXIT	5//S
extern void set_lcd_keep_open_flag(u8 flag);
extern u8 get_vbat_percent(void);
extern u8 get_charge_online_flag(void);
extern u8 get_charge_full_flag(void);
static u16 timer_id = 0;

static u8 power_off_charge_flag = 0;
void power_off_charge_flag_set(int flag)
{
    power_off_charge_flag = flag;
}
u8 power_off_charge_flag_get()
{
    return power_off_charge_flag;
}

static int batcharge_handler(const char *type, u32 arg)
{
    static char process_record = -1;
    struct unumber num;
    if (type && !strcmp(type, "process")) {
        printf("%s val:%dprocess_record:%d ", __func__, arg, process_record);

        if (process_record != arg) {
            struct ui_text *charge_text = (struct ui_text *)ui_core_get_element_by_id(TEXT_CHARGE_STATUS);
            if (charge_text) {
                int charge_online =  !!get_charge_online_flag();	//充电判断
                int full_flag  =  !!get_charge_full_flag();			//充满判断
                int charge_text_index  = full_flag + charge_online;
                ui_text_set_index(charge_text, charge_text_index);
            }

            process_record = arg;
            ui_progress_set_persent_by_id(BATCHARGE_PROGRESS_VALUE, arg);
            num.type = TYPE_NUM;
            num.numbs = 1;
            num.number[0] = arg;
            ui_number_update_by_id(BATCHARGE_PERSENT_VALUE, &num);
        }
    }

    return 0;
}


static const struct uimsg_handl ui_msg_handler[] = {
    { "batcharge",        batcharge_handler     },
    { NULL, NULL},
};


static int batcharge_init(int p)
{
    int bat_percent  =  get_vbat_percent();
    struct ui_progress *progress = (struct ui_progress *) ui_core_get_element_by_id(BATCHARGE_PROGRESS_VALUE);
    if (progress) {
        ui_progress_set_persent(progress, bat_percent);
    }
    struct ui_number *widget_num = (struct ui_number *)ui_core_get_element_by_id(BATCHARGE_PERSENT_VALUE);
    if (widget_num) {
        struct unumber num;
        num.type = TYPE_NUM;
        num.numbs = 1;
        num.number[0] = bat_percent;
        ui_number_update(widget_num, &num);
    }
    struct ui_text *charge_text = (struct ui_text *)ui_core_get_element_by_id(TEXT_CHARGE_STATUS);
    if (charge_text) {
        int charge_online =  !!get_charge_online_flag();	//充电判断
        int full_flag  =  !!get_charge_full_flag();			//充满判断
        int charge_text_index  = full_flag + charge_online;
        ui_text_set_index(charge_text, charge_text_index);
    }

    ui_progress_set_persent_by_id(BATCHARGE_PROGRESS_BACKUP, 100);
    return 0;
}

void ui_return_page_hump(void);
static void bat_charge_auto_exit_deal(void *p)
{
    if (!timer_id) {
        return;
    }
    //5s后回表盘，关机充电不允许退出
    if (timer_id) {
        sys_timeout_del(timer_id);
        timer_id = 0;
    }

    printf("%s ret:0x%x prev:0x%x", __func__, ui_return_page_id(), ui_return_prev_page_id());
    ui_return_page_hump();
    //有页面回页面，没有回表盘
    if (!ui_return_page_id() ||  ui_return_prev_page_id() == ID_WINDOW_BATCHARGE) {
        UI_SHOW_WINDOW(ID_WINDOW_DIAL);
    } else {
        ui_return_page_pop(2);
    }
}

static int batcharge_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        /* if (!ui_auto_shut_down_disable()) { */
        /* set_lcd_keep_open_flag(1); */
        /* } */
        ui_register_msg_handler(ui_get_current_window_id(), ui_msg_handler);
        //息屏插入
        ui_auto_shut_down_enable();
        //更新定时器
        ui_auto_shut_down_re_run();
        if (app_in_mode(APP_MODE_IDLE)) {

        } else {

            if (!timer_id) {
                timer_id = sys_timeout_add(NULL, bat_charge_auto_exit_deal, BAT_CHARGE_AUTO_EXIT * 1000);
            }
        }
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_set_call(batcharge_init, 0);
        break;

    case ON_CHANGE_RELEASE:
        /* set_lcd_keep_open_flag(0); */
        if (timer_id) {
            sys_timeout_del(timer_id);
            timer_id = 0;
        }
        /* ui_auto_shut_down_enable(); */
        break;

    default:
        return false;
    }
    return false;
}
static int  batcharge_ontouch(void *_ctrl, struct element_touch_event *e)
{
    if (app_in_mode(APP_MODE_IDLE)) {
        return true;
    } else {
        return false;
    }
}



REGISTER_UI_EVENT_HANDLER(BATCHARGE_LAYOUT)
.onchange = batcharge_onchange,
 .onkey    = NULL,
  .ontouch  =   batcharge_ontouch,
};


#endif/*#if TCFG_UI_ENABLE_BATCHARGE*/
#endif/*#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE*/



