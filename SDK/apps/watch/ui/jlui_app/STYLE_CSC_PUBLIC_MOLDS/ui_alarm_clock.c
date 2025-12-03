#include "app_config.h"
#include "ui/ui_api.h"
#include "alarm.h"
#include "time.h"
#include "rtc/rtc.h"
#include "watch_syscfg_manage.h"
#include "rcsp_rtc_func.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_ALARM_CLOCK]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_cs_about.data.bss")
#pragma data_seg(".ui_cs_about.data")
#pragma const_seg(".ui_cs_about.text.const")
#pragma code_seg(".ui_cs_about.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if (defined TCFG_UI_ALARM_CLOCK_ENABLE) && TCFG_UI_ALARM_CLOCK_ENABLE

#define STYLE_NAME  JL


struct alarm_clock {
    u8 total_alarm;
    int countdown_time; //sec
    u16 time_id;
};

struct alarm_clock *p_alarm_clock;
const u8 sel_time[] = {5, 10, 30, 60};
static u8 sel_time_index;


#define __this  p_alarm_clock
#define PRINTF_TIME(string, x)  log_info("PRINTF_TIME %s: (%d %d %d %d %d %d)", string, (x)->sec, (x)->min, (x)->hour, (x)->day, (x)->month, (x)->year)


/************************************************
 *ALARM_CLOCK_SET_LAYOUT相关逻辑处理
 ***********************************************/

static void ui_add_alarm_clock(u8 sel_time_index)
{
    if (sel_time_index >= ARRAY_SIZE(sel_time)) {
        log_error("%s index %s is error", __func__, sel_time_index);
        return;
    }

    T_ALARM tmp_alarm;
    /*如果闹钟是存在的，得先读取出来，因为name_len需要保留下来*/
    alarm_get_info(&tmp_alarm, 0);

    tmp_alarm.index = 0;
    tmp_alarm.sw = 1;
    tmp_alarm.mode = E_ALARM_MODE_ONCE;
    rtc_read_time(&tmp_alarm.time);

    struct tm tm_info = {0};
    tm_info.tm_sec = tmp_alarm.time.sec;
    tm_info.tm_min = tmp_alarm.time.min;
    tm_info.tm_hour = tmp_alarm.time.hour;
    tm_info.tm_mday = tmp_alarm.time.day;
    tm_info.tm_mon = tmp_alarm.time.month - 1;
    tm_info.tm_year = tmp_alarm.time.year - 1900;

    time_t rawtime = mktime(&tm_info);
    if (rawtime == -1) {
        log_error("%s mktime is error", __func__);
        return;
    }

    rawtime += sel_time[sel_time_index] * 60;
    struct tm *p_tm_new_info = localtime(&rawtime);
    if (!p_tm_new_info) {
        log_error("%s localtime is error", __func__);
        return;
    }

    tmp_alarm.time.sec = p_tm_new_info->tm_sec;
    tmp_alarm.time.min = p_tm_new_info->tm_min;
    tmp_alarm.time.hour = p_tm_new_info->tm_hour;
    tmp_alarm.time.day = p_tm_new_info->tm_mday;
    tmp_alarm.time.month = p_tm_new_info->tm_mon;
    tmp_alarm.time.year = p_tm_new_info->tm_year;

    /* log_debug_hexdump((u8 *)&tmp_alarm, sizeof(T_ALARM)); */
    alarm_add(&tmp_alarm, 0);
#if RCSP_APP_RTC_EN
    rcsp_update_alarm_info();
#endif
}


static int alarm_clock_next_prev_button_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }
        switch (elm->id) {
        case ALARM_CLOCK_NEXT_BUTTON_PIC:
            sel_time_index++;
            if (sel_time_index >= ARRAY_SIZE(sel_time)) {
                sel_time_index = ARRAY_SIZE(sel_time) - 1;
            }
            break;
        case ALARM_CLOCK_PREV_BUTTON_PIC:
            if (sel_time_index > 0) {
                sel_time_index--;
            }
            break;
        default:
            break;
        }
        ui_pic_show_image_by_id(ALARM_CLOCK_SETTING_TIME_PIC, sel_time_index);
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_CLOCK_NEXT_BUTTON_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alarm_clock_next_prev_button_pic_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ALARM_CLOCK_PREV_BUTTON_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alarm_clock_next_prev_button_pic_ontouch,
};

static int alarm_clock_set_button_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }

        ui_add_alarm_clock(sel_time_index);
        ui_hide(ALARM_CLOCK_SET_LAYOUT);
        ui_show(ALARM_CLOCK_RUNNING_LAYOUT);
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_CLOCK_SETTING_BUTTON_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alarm_clock_set_button_pic_ontouch,
};


static int alarm_clock_set_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        if (__this->total_alarm) {
            layout->elm.css.invisible = 1;
        } else {
            sel_time_index = 0;
            layout->elm.css.invisible = 0;
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_CLOCK_SET_LAYOUT)
.onchange = alarm_clock_set_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 *ALARM_CLOCK_RUNNING_LAYOUT相关逻辑处理
 ***********************************************/

static int alarm_clock_cancel_button_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            alarm_delete(0);
#if RCSP_APP_RTC_EN
            rcsp_update_alarm_info();
#endif
            ui_hide(ALARM_CLOCK_RUNNING_LAYOUT);
            ui_show(ALARM_CLOCK_SET_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;

}
REGISTER_UI_EVENT_HANDLER(ALARM_CLOCK_CANCEL_BUTTON_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alarm_clock_cancel_button_pic_ontouch,
};


static void show_countdown_time_tick_handler(void *p)
{
    struct utime ui_time_info = {0};

    __this->countdown_time--;
    if (__this->countdown_time < 0) {
        log_error("%s countdown_time is error", __func__);
        return;
    }

    /*按现在有需求，设置的闹钟时间不超过一天*/
    ui_time_info.hour = __this->countdown_time / 3600;
    int tmp_value = __this->countdown_time % 3600;
    ui_time_info.min = __this->countdown_time / 60;
    ui_time_info.sec = __this->countdown_time % 60;
    ui_time_info.day = 0;
    ui_time_info.month = 0;
    ui_time_info.year = 0;
    ui_time_update_by_id(ALARM_CLOCK_RUNNING_TIME, &ui_time_info);

    if (__this->countdown_time == 0) {
        if (__this->time_id) {
            sys_timer_del(__this->time_id);
            __this->time_id = 0;
        }
        return;
    }
}

static int alarm_clock_running_time_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_time *time = (struct ui_time *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        struct sys_time cur_rtc_time;
        struct sys_time alarm_time;
        rtc_read_time(&cur_rtc_time);
        rtc_read_alarm(&alarm_time);

        struct tm date1 = {0};
        struct tm date2 = {0};
        date1.tm_sec = cur_rtc_time.sec;
        date1.tm_min = cur_rtc_time.min;
        date1.tm_hour = cur_rtc_time.hour;
        date1.tm_mday = cur_rtc_time.day;
        date1.tm_mon = cur_rtc_time.month - 1;
        date1.tm_year = cur_rtc_time.year - 1900;

        date2.tm_sec = alarm_time.sec;
        date2.tm_min = alarm_time.min;
        date2.tm_hour = alarm_time.hour;
        date2.tm_mday = alarm_time.day;
        date2.tm_mon = alarm_time.month - 1;
        date2.tm_year = alarm_time.year - 1900;

        PRINTF_TIME("cur_rtc_time", &cur_rtc_time);
        PRINTF_TIME("alarm_time", &alarm_time);
        time_t time1 = mktime(&date1);
        time_t time2 = mktime(&date2);
        __this->countdown_time = time2 - time1;

        struct utime ui_time_info = {0};
        /*按现在有需求，设置的闹钟时间不超过一天*/
        ui_time_info.hour = __this->countdown_time / 3600;
        int tmp_value = __this->countdown_time % 3600;
        ui_time_info.min = tmp_value / 60;
        ui_time_info.sec = tmp_value % 60;
        ui_time_info.day = 0;
        ui_time_info.month = 0;
        ui_time_info.year = 0;
        log_debug("__this->countdown_time:%d", __this->countdown_time);
        PRINTF_TIME("ui_time_info", &ui_time_info);
        ui_time_update(time, &ui_time_info);

        __this->time_id = sys_timer_add(NULL, show_countdown_time_tick_handler, 1000);
        break;
    case ON_CHANGE_RELEASE_PROBE:
        if (__this->time_id) {
            sys_timer_del(__this->time_id);
            __this->time_id = 0;
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_CLOCK_RUNNING_TIME)
.onchange = alarm_clock_running_time_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int alarm_clock_running_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        if (__this->total_alarm) {
            layout->elm.css.invisible = 0;
        } else {
            layout->elm.css.invisible = 1;
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_CLOCK_RUNNING_LAYOUT)
.onchange = alarm_clock_running_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



static int ID_WINDOW_ALARM_CLOCK_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    int ret;
    switch (event) {
    case ON_CHANGE_INIT:
        __this = zalloc(sizeof(struct alarm_clock));
        if (!__this) {
            log_error("%s zalloc error", __func__);
            return true;
        }
        T_ALARM alarm0;
        ret = alarm_get_info(&alarm0, 0);
        if (ret == E_SUCCESS) {
            if (alarm0.sw == 1) {
                __this->total_alarm = 1;
            }
        } else {
            __this->total_alarm = 0;
        }
        break;
    case ON_CHANGE_RELEASE:
        if (__this) {
            free(__this);
            __this = NULL;
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_ALARM_CLOCK)
.onchange = ID_WINDOW_ALARM_CLOCK_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 *      闹钟响铃页面相关处理
 ***********************************************/

static int alarm_ring_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();
        break;
    default:
        return false;
    }
    return false;
}

static int alarm_ring_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_CLOCK_RING_LAYOUT)
.onchange = alarm_ring_layout_onchange,
 .onkey = NULL,
  .ontouch = alarm_ring_layout_ontouch,
};

static int alarm_clock_ring_stop_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (alarm_active_flag_get()) {
            alarm_stop(0);
            alarm_delete(0);
#if RCSP_APP_RTC_EN
            rcsp_update_alarm_info();
#endif
        }
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_ALARM_RINGING);
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_CLOCK_RING_STOP_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alarm_clock_ring_stop_pic_ontouch,
};

static int alarm_clock_ring_repeat_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (alarm_active_flag_get()) {
            alarm_stop(0);
        }
        ui_add_alarm_clock(sel_time_index);
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_ALARM_RINGING);
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_CLOCK_RING_REPEAT_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alarm_clock_ring_repeat_pic_ontouch,
};



/************************************************
 *           恢复出厂设置 清除数据
 ***********************************************/
static int watch_syscfg_write_alarm_to_vm(void *priv)
{
    if ((int)priv == (int)SYSCFG_WRITE_ERASE_STATUS) {
        alarm_delete_all();
    }
    return 0;
}

REGISTER_WATCH_SYSCFG(alarm_ops) = {
    .name = "alarm",
    .read = NULL,
    .write = watch_syscfg_write_alarm_to_vm,
};

#endif
#endif
