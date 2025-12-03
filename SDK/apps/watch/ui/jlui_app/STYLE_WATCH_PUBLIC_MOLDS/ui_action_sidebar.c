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
#include "data_storage/data_weather_storage.h"
#include "alarm.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_DRAW_DEMO]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_draw_demo.data.bss")
#pragma data_seg(".ui_action_draw_demo.data")
#pragma const_seg(".ui_action_draw_demo.text.const")
#pragma code_seg(".ui_action_draw_demo.text")
#endif

#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_ENABLE_LEFT_MENU
#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)
#define SIDEBAR_SIDEBAR_AIRTEMP_NUM  	 SIDEBAR_AIRTEMP_NUM
#define SIDEBAR_SIDEBAR_APPREC_0		 SIDEBAR_APPREC_0
#define SIDEBAR_SIDEBAR_APPREC_1		 SIDEBAR_APPREC_1
#define SIDEBAR_SIDEBAR_APPREC_2		 SIDEBAR_APPREC_2
#define SIDEBAR_SIDEBAR_APPREC_3		 SIDEBAR_APPREC_3
#define SIDEBAR_SIDEBAR_PAY_PIC			 SIDEBAR_PAY_PIC
#define SIDEBAR_SIDEBAR_TIME_TIME		 SIDEBAR_TIME_TIME
#define SIDEBAR_SIDEBAR_WEATHER_PIC		 SIDEBAR_WEATHER_PIC
#define SIDEBAR_SIDEBAR_BAT_PIC		 	 SIDEBAR_BAT_PIC
#define SIDEBAR_SIDEBAR_BAT_NUM			 SIDEBAR_BAT_NUM


u8 get_vbat_percent(void);
u32 ui_menu_app_record_get_sel(u32 index);
u32 ui_menu_app_record_window(u32 index);

static u8 __rtc_calculate_week_val(struct sys_time *data_time)
{
    struct sys_time t_time;
    u32 century, val, year;

    memcpy(&t_time, data_time, sizeof(struct sys_time));
    if (t_time.month < 3) {
        t_time.month = t_time.month + 12;
        t_time.year--;
    }
    year = t_time.year % 100;
    century = t_time.year / 100;
    val = year + (year / 4) + (century / 4) + (26 * (t_time.month + 1) / 10) + t_time.day;
    val = val - century * 2 - 1;

    return (u8)(val % 7);
}
static int sidebar_left_chile_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    struct sys_time time;
    int index;
    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW_PROBE: {
        switch (elm->id) {
        case SIDEBAR_SIDEBAR_TIME_TIME:
            rtc_read_time(&time);
            ui_time_update((struct ui_time *)elm, (struct utime *)&time);
            break;
        case SIDEBAR_SIDEBAR_WEATHER_PIC:
            int count = ui_small_file_weather_get_count();
            if (!count) {
                break;
            }
            struct weather_single_info weather_info;
            ui_small_file_weather_get_singel_info(&weather_info, count - 1);
            ui_pic_set_image_index((struct ui_pic *)elm, weather_info.weather);
            ui_core_set_element_ratio(elm, (float)60.f / 100, (float)60.f / 100, 1);
            break;
        case SIDEBAR_SIDEBAR_APPREC_0:
            ui_core_set_element_ratio(elm, (float)60.f / 80, (float)60.f / 80, 1);
            ui_core_set_element_ratio_change_rect(elm, 1);
            index = ui_menu_app_record_get_sel(0);
            ui_pic_set_image_index((struct ui_pic *)elm, index);
            break;
        case SIDEBAR_SIDEBAR_APPREC_1:
            ui_core_set_element_ratio(elm, (float)60.f / 80, (float)60.f / 80, 1);

            ui_core_set_element_ratio_change_rect(elm, 1);
            index = ui_menu_app_record_get_sel(1);
            ui_pic_set_image_index((struct ui_pic *)elm, index);
            break;
        case SIDEBAR_SIDEBAR_APPREC_2:
            ui_core_set_element_ratio(elm, (float)60.f / 80, (float)60.f / 80, 1);

            ui_core_set_element_ratio_change_rect(elm, 1);
            index = ui_menu_app_record_get_sel(2);
            ui_pic_set_image_index((struct ui_pic *)elm, index);
            break;
        case SIDEBAR_SIDEBAR_APPREC_3:
            ui_core_set_element_ratio(elm, (float)60.f / 80, (float)60.f / 80, 1);

            ui_core_set_element_ratio_change_rect(elm, 1);
            index = ui_menu_app_record_get_sel(3);
            ui_pic_set_image_index((struct ui_pic *)elm, index);
            break;
        case SIDEBAR_SIDEBAR_BAT_PIC:
            u8 vbat_percent =  get_vbat_percent();
            u8 vbat_level  = 0;
            if (vbat_percent >= 80) {
                vbat_level  = 4;
            } else if (vbat_percent >= 60) {
                vbat_level = 3;
            } else if (vbat_percent >= 40) {
                vbat_level = 2;
            } else if (vbat_percent >= 20) {
                vbat_level = 1;
            }
            ui_pic_set_image_index((struct ui_pic *)elm, vbat_level);
            break;
        case SIDEBAR_SIDEBAR_BAT_NUM:
            struct unumber vbat_num = {
                .type = TYPE_NUM,
                .numbs = 1,
                .number[0] = get_vbat_percent(),
            };
            ui_number_update((struct ui_number *)elm, &vbat_num);
            break;
        }
    }
    break;
    case  ON_CHANGE_SHOW:
        switch (elm->id) {
        case SIDEBAR_SIDEBAR_AIRTEMP_NUM: {
            int count = ui_small_file_weather_get_count();
            if (!count) {
                break;
            }
            struct weather_single_info weather_info;
            ui_small_file_weather_get_singel_info(&weather_info, count - 1);
            struct ui_number *ui_num = (struct ui_number *)elm;
            struct ui_number_info *info = ui_core_load_widget_info((void *)ui_num->info, -1);
            int number_buffer_offset = 0;
            //负号
            if (weather_info.temperature < 0) {
                ui_num->buf[number_buffer_offset]	 = info->delimiter[1];
                number_buffer_offset++;
                weather_info.temperature *= -1;
            }
            int air_temp_10 = weather_info.temperature / 10;
            int air_temp_1  = weather_info.temperature % 10;
            /* printf("%s %d %d",__func__,air_temp_10,air_temp_1); */
            //十位
            if (air_temp_10 != 0) {
                ui_num->buf[number_buffer_offset] = info->number[air_temp_10];
                number_buffer_offset++;
            }
            //个位
            ui_num->buf[number_buffer_offset] = info->number[air_temp_1];
            number_buffer_offset++;
            //单位
            ui_num->buf[number_buffer_offset]	 = info->delimiter[0];
            number_buffer_offset++;
            //结尾
            ui_num->buf[number_buffer_offset] = 0xffff;
            number_buffer_offset++;
            /* put_buf((u8*)ui_num->buf,2*number_buffer_offset); */
        }
        break;
        }
        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        return false;
    }
    return false;
}
static int sidebar_left_child_ontouch(void *ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }
        switch (elm->id) {
        case SIDEBAR_SIDEBAR_APPREC_0: {
            u32 win = ui_menu_app_record_window(0);
            if (win > 0) {
                UI_SHOW_WINDOW(win);
            }
        }
        break;
        case SIDEBAR_SIDEBAR_APPREC_1: {
            u32 win = ui_menu_app_record_window(1);
            if (win > 0) {
                UI_SHOW_WINDOW(win);
            }
        }
        break;
        case SIDEBAR_SIDEBAR_APPREC_2: {
            u32 win = ui_menu_app_record_window(2);
            if (win > 0) {
                UI_SHOW_WINDOW(win);
            }
        }
        break;
        case SIDEBAR_SIDEBAR_APPREC_3: {
            u32 win = ui_menu_app_record_window(3);
            if (win > 0) {
                UI_SHOW_WINDOW(win);
            }
        }
        break;
        case SIDEBAR_SIDEBAR_PAY_PIC:
            ui_card_disable();
            UI_SHOW_WINDOW(ID_WINDOW_ALIPAY);
            break;
        case SIDEBAR_SIDEBAR_WEATHER_PIC:
            ui_card_disable();
            UI_SHOW_WINDOW(ID_WINDOW_WEATHER);
            break;
        }
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SIDEBAR_SIDEBAR_APPREC_0)
.onchange =  sidebar_left_chile_onchange,
 .onkey = NULL,
  .ontouch =  sidebar_left_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_SIDEBAR_APPREC_1)
.onchange =  sidebar_left_chile_onchange,
 .onkey = NULL,
  .ontouch =  sidebar_left_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_SIDEBAR_APPREC_2)
.onchange =  sidebar_left_chile_onchange,
 .onkey = NULL,
  .ontouch =  sidebar_left_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_SIDEBAR_APPREC_3)
.onchange =  sidebar_left_chile_onchange,
 .onkey = NULL,
  .ontouch =  sidebar_left_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_SIDEBAR_PAY_PIC)
.onchange =  sidebar_left_chile_onchange,
 .onkey = NULL,
  .ontouch =  sidebar_left_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_SIDEBAR_BAT_PIC)
.onchange =  sidebar_left_chile_onchange,
 .onkey = NULL,
  .ontouch =   NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_SIDEBAR_TIME_TIME)
.onchange =  sidebar_left_chile_onchange,
 .onkey = NULL,
  .ontouch =    NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_SIDEBAR_AIRTEMP_NUM)
.onchange =  sidebar_left_chile_onchange,
 .onkey = NULL,
  .ontouch =  NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_SIDEBAR_WEATHER_PIC)
.onchange =  sidebar_left_chile_onchange,
 .onkey = NULL,
  .ontouch =    sidebar_left_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_SIDEBAR_BAT_NUM)
.onchange =  sidebar_left_chile_onchange,
 .onkey = NULL,
  .ontouch =  NULL,
};
#endif
#endif
