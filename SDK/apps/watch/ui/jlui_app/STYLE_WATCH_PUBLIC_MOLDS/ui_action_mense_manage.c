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
#include "mense_manage/mense_manage.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_MENSE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_mense_manage.data.bss")
#pragma data_seg(".ui_action_mense_manage.data")
#pragma const_seg(".ui_action_mense_manage.text.const")
#pragma code_seg(".ui_action_mense_manage.text")
#endif

#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_MENSE_MANAGE_ENABLE


#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

enum {
    LAYOUT_CUR,
    LAYOUT_SET_NOTICE,
    LAYOUT_SET_CYCLE,
    LAYOUT_SET_DAY,
    LAYOUT_SET_DATE,
    LAYOUT_MAIN_SEL,
    LAYOUT_NOP,
    LAYOUT_WARNING,
    LAYOUT_MAX,
};
enum {
    DAY_SET_RESP_MENSE,		//经期提醒
    DAY_SET_RESP_OVL,		//排卵期提醒
    DAY_SET_RESP_EASY,		//易孕期提醒
    DAY_SET_CYCLE_MENSE,	//经期时间
    DAY_SET_CYCLE_PHY,		//生理周期时间
};
struct ui_women_health {
    u32 curr_layout_id;
    u8 layout_mode;
    u8 set_day_mode;
    u8 rcp_mode;
    struct sys_time sel_time;
};
static struct ui_women_health __info = {
    .layout_mode = LAYOUT_MAIN_SEL,
    .curr_layout_id =  LAYOUT_WOMEN_HEALTH_SEL,
};
#define __this (&__info)

void rtc_read_time(struct sys_time *time);
int ui_women_health_layout_switch(u8 layout_mode)
{
    if (!__this) {
        log_error("%s not init", __func__);
        return -1;
    }
    if (layout_mode >= LAYOUT_MAX) {
        log_error("%s layout_mode error:%d", __func__, layout_mode);
        return -1;
    }
    if (layout_mode == __this->layout_mode) {
        log_warn("%s layout_mode:%d", __func__, layout_mode);
        return -1;
    }
    if (__this->curr_layout_id) {
        ui_hide(__this->curr_layout_id);
    }
    switch (layout_mode) {
    case LAYOUT_CUR :
        __this->curr_layout_id = LAYOUT_WOMEN_HEALTH_CUR;
        break;
    case LAYOUT_SET_NOTICE:
        __this->curr_layout_id = LAYOUT_WOMEN_HEALTH_SET_NOTICE;
        break;
    case LAYOUT_SET_CYCLE:
        __this->curr_layout_id = LAYOUT_WOMEN_HEALTH_SET_CYCLE;
        break;
    case LAYOUT_SET_DAY:
        __this->curr_layout_id = LAYOUT_WOMEN_HEALTH_SET_DAY;
        break;
    case LAYOUT_SET_DATE:
        __this->curr_layout_id = LAYOUT_WOMEN_HEALTH_SET_DATE;
        break;
    case LAYOUT_MAIN_SEL:
        __this->curr_layout_id = LAYOUT_WOMEN_HEALTH_SEL;
        break;
    case LAYOUT_NOP:
        __this->curr_layout_id = LAYOUT_WOMEN_HEALTH_NOP;
        break;
    }
    __this->layout_mode = layout_mode;
    ui_show(__this->curr_layout_id);
    return 0;
}

/**********************************************************************************
			设置页面
 **********************************************************************************/
static int  women_health_day_sel_list_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct ui_number *num  = (struct ui_number *)elm;
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (int)arg;
        if (ui_id2type(elm->id) == CTRL_TYPE_NUMBER) {
            struct unumber unum;
            unum.type = 0;
            unum.numbs = 1;
            unum.number[0] = index;
            ui_number_update(num, &unum);
        }
    } else if (event == ON_CHANGE_HIGHLIGHT) {
        int highlight = (int)arg;

        printf("%s elm:0x%x high:%d\n ", __func__, elm->id, highlight);
        if (ui_id2type(elm->id) == CTRL_TYPE_NUMBER) {
            if (!strcmp(num->source, "highl")) {
                elm->css.invisible = !highlight;
            } else if (!strcmp(num->source, "normal")) {
                elm->css.invisible = highlight;
            }
        }
    }
    return false;
}
static int women_health_day_sel_list_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct ui_grid *grid = (struct ui_grid *)elm;
    switch (event) {
    case ON_CHANGE_INIT:
        int row = 5;
        if (__this->set_day_mode == DAY_SET_CYCLE_PHY) {
            row = 60;
        } else if (__this->set_day_mode == DAY_SET_CYCLE_MENSE) {
            row = 15;
        }
        int col  = 1;
        ui_set_default_handler(elm, NULL, NULL, women_health_day_sel_list_child_onchange);
        ui_grid_init_dynamic(grid, &row, &col);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        int highlight_index = 0;
        struct element *item_elm, *child_elm;
        for (int i = 0; i < grid->ctrl_num; i++) {
            item_elm = &(grid->item[i].elm);
            if (!item_elm) {
                break;
            }
            list_for_each_child_element(child_elm, item_elm) {
                int is_highlight = (i == highlight_index);
                printf("%s elm:0x%x high:%d\n ", __func__, child_elm->id, (i == highlight_index));
                child_elm->highlight = !is_highlight;
                ui_core_highlight_element(child_elm, is_highlight);
            }
        }
        break;
    case ON_CHANGE_RELEASE:
        ui_set_default_handler(elm, NULL, NULL, NULL);
        break;
    default:
        return false;
    }
    return false;
}

static int women_health_day_sel_ontouch(void *ctrl, struct element_touch_event *e)
{

    struct element *elm = (struct element *)ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        struct ui_grid *grid  = (struct ui_grid *)ui_core_get_element_by_id(WOMEN_HEALTH_DAY_SEL_LIST);
        if (!grid) {
            break;
        }
        u8 return_layout_mode = 0;
        PERSONAL_MENSE info;
        mense_personal_info_get(&info);
        if (__this->set_day_mode == DAY_SET_CYCLE_PHY) {
            info.physiological_days = ui_grid_cur_item_dynamic(grid);
            return_layout_mode = LAYOUT_SET_CYCLE;
        } else if (__this->set_day_mode == DAY_SET_CYCLE_MENSE) {
            info.mense_period_days = ui_grid_cur_item_dynamic(grid);
            return_layout_mode = LAYOUT_SET_CYCLE;
        } else if (__this->set_day_mode == DAY_SET_RESP_MENSE) {
            info.resp.mense_prev_day = ui_grid_cur_item_dynamic(grid);
            return_layout_mode = LAYOUT_SET_NOTICE;
        } else if (__this->set_day_mode == DAY_SET_RESP_OVL) {
            info.resp.ovulation_prev_day = ui_grid_cur_item_dynamic(grid);
            return_layout_mode = LAYOUT_SET_NOTICE;
        } else if (__this->set_day_mode == DAY_SET_RESP_EASY) {
            info.resp.easy_prev_day = ui_grid_cur_item_dynamic(grid);
            return_layout_mode = LAYOUT_SET_NOTICE;
        }
        mense_personal_info_set(&info);
        ui_women_health_layout_switch(return_layout_mode);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_DAY_SEL_SW)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = women_health_day_sel_ontouch,
};

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_DAY_SEL_LIST)
.onchange = women_health_day_sel_list_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#define RGB565(r,g,b) (((((u8)r)>>3)<<11)|((((u8)g)>>2)<<5)|(((u8)b)>>3))
static int women_health_date_sel_list_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct ui_number *num  = (struct ui_number *)elm;
    /* printf("%s event:%d arg:0x%x", __func__, event, (u32)arg); */
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (int)arg;
        int basic = 1;
        if (elm->parent->parent->id == WOMEN_HEALTH_SET_DATE_LIST_YEAR) {
            basic = 2000;
        }
        if (ui_id2type(elm->id) == CTRL_TYPE_NUMBER) {
            struct unumber unum;
            unum.type = 0;
            unum.numbs = 1;
            unum.number[0] = index + basic;
            ui_number_update(num, &unum);
        }
    }	else if (event == ON_CHANGE_SHOW) {
        if (ui_id2type(elm->id) == CTRL_TYPE_NUMBER) {
            struct draw_context *dc = (struct draw_context *)arg;

            dc->custom_color = BIT(UI_CUSTOM_COLOR_BIT_IMAGE);
            if (elm->highlight) {
                dc->custom_argb8888 = 0xffffffff;
            } else {

                dc->custom_argb8888 = 0xff808080;
            }
        }

    }
    return false;
}
static int women_health_date_sel_list_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct ui_grid *grid = (struct ui_grid *)elm;
    switch (event) {
    case ON_CHANGE_INIT:
        int base_index_once = 0;
        int row = 0;
        int col = 1;
        if (WOMEN_HEALTH_SET_DATE_LIST_YEAR == elm->id) {
            row = 200;
        } else if (WOMEN_HEALTH_SET_DATE_LIST_MONTH == elm->id) {
            row = 12;
        } else if (WOMEN_HEALTH_SET_DATE_LIST_DAY == elm->id) {
            row = 31;
        }
        ui_set_default_handler(elm, NULL, NULL, women_health_date_sel_list_child_onchange);
        ui_grid_init_dynamic(grid, &row, &col);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_SHOW:
        /* struct ui_grid *list_year = (struct ui_grid*)ui_core_get_element_by_id(WOMEN_HEALTH_SET_DATE_LIST_YEAR); */
        /* struct ui_grid *list_month = (struct ui_grid*)ui_core_get_element_by_id(WOMEN_HEALTH_SET_DATE_LIST_MONTH); */
        /* struct ui_grid *list_day = (struct ui_grid*)ui_core_get_element_by_id(WOMEN_HEALTH_SET_DATE_LIST_DAY); */
        /* int year = ui_grid_cur_item_dynamic(list_year) + 2000; */
        /* int month = ui_grid_cur_item_dynamic(list_month) + 1; */
        /* int day = ui_grid_cur_item_dynamic(list_day) + 1; */
        break;
    case ON_CHANGE_RELEASE:
        ui_set_default_handler(elm, NULL, NULL, NULL);
        break;
    default:
        return false;
    }
    return false;
}

static int women_health_date_sel_ontouch(void *ctrl, struct element_touch_event *e)
{

    struct element *elm = (struct element *)ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        PERSONAL_MENSE info;
        mense_personal_info_get(&info);
        struct ui_grid *list_year = (struct ui_grid *)ui_core_get_element_by_id(WOMEN_HEALTH_SET_DATE_LIST_YEAR);
        struct ui_grid *list_month = (struct ui_grid *)ui_core_get_element_by_id(WOMEN_HEALTH_SET_DATE_LIST_MONTH);
        struct ui_grid *list_day = (struct ui_grid *)ui_core_get_element_by_id(WOMEN_HEALTH_SET_DATE_LIST_DAY);
        if (list_year && list_month && list_day) {
            int year = ui_grid_cur_item_dynamic(list_year) + 2000;
            int month = ui_grid_cur_item_dynamic(list_month) + 1;
            int day = ui_grid_cur_item_dynamic(list_day) + 1;
            info.menstruation_sday.year = year;
            info.menstruation_sday.month = month;
            info.menstruation_sday.day = day;
            mense_personal_info_set(&info);
            ui_women_health_layout_switch(LAYOUT_SET_CYCLE);
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_DATE_SEL_SW)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = women_health_date_sel_ontouch,
};

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_DATE_LIST_YEAR)
.onchange = women_health_date_sel_list_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_DATE_LIST_MONTH)
.onchange = women_health_date_sel_list_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_DATE_LIST_DAY)
.onchange = women_health_date_sel_list_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/**********************************************************************************
			周期页面
 **********************************************************************************/
static int list_women_health_cycle_ctrl_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    switch (event) {
    case ON_CHANGE_SHOW:
        PERSONAL_MENSE info;
        mense_personal_info_get(&info);
        u8 index_buf[3] = {0, 10, 10};

        int index_len = 2;
        switch (elm->id) {
        case WOMEN_HEALTH_SET_CYCLE_TEXT_0:
            if (info.mense_period_days >= 10) {
                index_buf[0] = info.mense_period_days / 10;
                index_buf[1] = info.mense_period_days % 10;
                index_len ++;
            } else {
                index_buf[0] = info.mense_period_days;
            }
            ui_text_set_multi_text_index((struct ui_text *)elm, index_buf, index_len);
            break;
        case WOMEN_HEALTH_SET_CYCLE_TEXT_1:
            if (info.physiological_days >= 10) {
                index_buf[0] = info.physiological_days / 10;
                index_buf[1] = info.physiological_days % 10;
                index_len ++;
            } else {
                index_buf[0] = info.physiological_days;
            }
            ui_text_set_multi_text_index((struct ui_text *)elm, index_buf, index_len);
            break;
        }
        break;
    default:
        return false;
    }
    return false;
}


static int women_health_cycle_ctrl_child_ontouch(void *ctrl, struct element_touch_event *e)
{

    struct element *elm = (struct element *)ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        switch (elm->id) {
        case WOMEN_HEALTH_SET_CYCLE_BG0:
            PERSONAL_MENSE info;
            __this->set_day_mode = DAY_SET_CYCLE_MENSE;
            ui_women_health_layout_switch(LAYOUT_SET_DAY);
            break;
        case WOMEN_HEALTH_SET_CYCLE_BG1:
            __this->set_day_mode = DAY_SET_CYCLE_PHY;
            ui_women_health_layout_switch(LAYOUT_SET_DAY);
            break;
        case WOMEN_HEALTH_SET_CYCLE_BG2:
            ui_women_health_layout_switch(LAYOUT_SET_DATE);
            break;
        }
        return true;
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_CYCLE_BG0)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =  women_health_cycle_ctrl_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_CYCLE_BG1)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =  women_health_cycle_ctrl_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_CYCLE_BG2)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =  women_health_cycle_ctrl_child_ontouch,
};

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_CYCLE_TEXT_0)
.onchange =  list_women_health_cycle_ctrl_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_CYCLE_TEXT_1)
.onchange =  list_women_health_cycle_ctrl_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/**********************************************************************************
			通知页面
 **********************************************************************************/
static int list_women_health_notice_ctrl_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    switch (event) {
    case ON_CHANGE_SHOW:
        PERSONAL_MENSE info;
        mense_personal_info_get(&info);
        u8 index_buf[2] = {0, 10};
        switch (elm->id) {
        case WOMEN_HEALTH_SET_NOTICE_PICSW:
            ui_pic_set_image_index((struct ui_pic *)elm, info.resp.enable);
            break;
        case WOMEN_HEALTH_SET_NOTICE_TEXT_0:
            index_buf[0] = info.resp.mense_prev_day;
            ui_text_set_multi_text_index((struct ui_text *)elm, index_buf, 2);
            break;
        case WOMEN_HEALTH_SET_NOTICE_TEXT_1:
            index_buf[0] = info.resp.ovulation_prev_day;
            ui_text_set_multi_text_index((struct ui_text *)elm, index_buf, 2);
            break;
        case WOMEN_HEALTH_SET_NOTICE_TEXT_2:
            index_buf[0] = info.resp.easy_prev_day;
            ui_text_set_multi_text_index((struct ui_text *)elm, index_buf, 2);
            break;
        }
        break;
    default:
        return false;
    }
    return false;
}


static int women_health_notice_ctrl_child_ontouch(void *ctrl, struct element_touch_event *e)
{

    struct element *elm = (struct element *)ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        switch (elm->id) {
        case WOMEN_HEALTH_SET_NOTICE_BG0:
            PERSONAL_MENSE info;
            mense_personal_info_get(&info);
            info.resp.enable = !info.resp.enable;
            mense_personal_info_set(&info);
            ui_core_redraw(elm);
            break;
        case WOMEN_HEALTH_SET_NOTICE_BG1:
            __this->set_day_mode = DAY_SET_RESP_MENSE;
            ui_women_health_layout_switch(LAYOUT_SET_DAY);
            break;
        case WOMEN_HEALTH_SET_NOTICE_BG2:
            __this->set_day_mode = DAY_SET_RESP_OVL;
            ui_women_health_layout_switch(LAYOUT_SET_DAY);
            break;
        case WOMEN_HEALTH_SET_NOTICE_BG3:
            __this->set_day_mode = DAY_SET_RESP_EASY;
            ui_women_health_layout_switch(LAYOUT_SET_DAY);
            break;
        }
        return true;
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_NOTICE_BG0)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =  women_health_notice_ctrl_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_NOTICE_BG1)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =  women_health_notice_ctrl_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_NOTICE_BG2)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =  women_health_notice_ctrl_child_ontouch,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_NOTICE_BG3)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =  women_health_notice_ctrl_child_ontouch,
};

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_NOTICE_PICSW)
.onchange =  list_women_health_notice_ctrl_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_NOTICE_TEXT_0)
.onchange =  list_women_health_notice_ctrl_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_NOTICE_TEXT_1)
.onchange =  list_women_health_notice_ctrl_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_SET_NOTICE_TEXT_2)
.onchange =  list_women_health_notice_ctrl_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};





/**********************************************************************************
			当前&详情页面
**********************************************************************************/
static int list_women_health_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    switch (event) {
    case ON_CHANGE_SHOW:

        struct sys_time time;
        MENSE_INFO day_info;
        int curr_status =  mense_now_status_get();

        switch (elm->id) {
        case WOMEN_HEALTH_CUR_PIC_STATE:
            ui_pic_set_image_index((struct ui_pic *)elm, mense_status_map_show_type(curr_status));
            break;
        case WOMEN_HEALTH_CUR_TEXT_STATE:
            ui_text_set_index((struct ui_text *)elm, mense_status_map_show_type(curr_status));
            break;
        case WOMEN_HEALTH_CUR_PIC_STATE_NEXT:
            int next_type = mense_next_status_get(curr_status);
            ui_pic_set_image_index((struct ui_pic *)elm, mense_status_map_show_type(next_type));
            ui_core_set_element_ratio(elm, 0.3333f, 0.3333f, 1);
            break;
        case WOMEN_HEALTH_CUR_TEXT_DAY:
            struct sys_time today_t;
            struct sys_time status_t;
            rtc_read_time(&today_t);
            mense_status_begin_time_get(curr_status, &status_t);
            int day = mense_time_day_len(&status_t, &today_t);
            day++;//
            u8 buf[4];
            u8 buf_len;
            buf[0] = 10;
            if (day / 10) {
                buf[1] = day / 10;
                buf[2] = day % 10;
                buf[3] = 11;
                buf_len = 4;
            } else {
                buf[1] = day % 10;
                buf[2] = 11;
                buf_len = 3;
            }
            ui_text_set_multi_text_index((struct ui_text *)elm, buf, buf_len);
            break;
        case WOMEN_HEALTH_CUR_TIME_BEGIN:
            mense_status_begin_time_get(curr_status, &time);
            ui_time_update((struct ui_time *)elm, (struct utime *)&time);
            break;
        case WOMEN_HEALTH_CUR_TIME_END:
            mense_status_end_time_get(curr_status, &time);
            ui_time_update((struct ui_time *)elm, (struct utime *)&time);
            break;
        case WOMEN_HEALTH_NEXT_TIME_BEGIN:
            mense_status_begin_time_get(mense_next_status_get(curr_status), &time);
            ui_time_update((struct ui_time *)elm, (struct utime *)&time);
            break;
        case WOMEN_HEALTH_NEXT_TIME_END:
            mense_status_end_time_get(mense_next_status_get(curr_status), &time);
            ui_time_update((struct ui_time *)elm, (struct utime *)&time);
            break;
        }
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_CUR_PIC_STATE)
.onchange =  list_women_health_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_CUR_TEXT_STATE)
.onchange =  list_women_health_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_CUR_PIC_STATE_NEXT)
.onchange =  list_women_health_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_CUR_TEXT_DAY)
.onchange =  list_women_health_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_CUR_TIME_BEGIN)
.onchange =  list_women_health_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_CUR_TIME_END)
.onchange =  list_women_health_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_NEXT_TIME_BEGIN)
.onchange =  list_women_health_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_NEXT_TIME_END)
.onchange =  list_women_health_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



static int list_women_health_layout_return_ontouch(void *ctr, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        switch (elm->id) {
        case LAYOUT_WOMEN_HEALTH_CUR:
        case LAYOUT_WOMEN_HEALTH_SET_CYCLE:
        case LAYOUT_WOMEN_HEALTH_SET_NOTICE:
            ui_women_health_layout_switch(LAYOUT_MAIN_SEL);
            break;
        case LAYOUT_WOMEN_HEALTH_SET_DATE:
        case LAYOUT_WOMEN_HEALTH_SET_DAY:
            u8 return_layout_mode = 0;
            if (__this->set_day_mode == DAY_SET_CYCLE_PHY) {
                return_layout_mode = LAYOUT_SET_CYCLE;
            } else if (__this->set_day_mode == DAY_SET_CYCLE_MENSE) {
                return_layout_mode = LAYOUT_SET_CYCLE;
            } else if (__this->set_day_mode == DAY_SET_RESP_MENSE) {
                return_layout_mode = LAYOUT_SET_NOTICE;
            } else if (__this->set_day_mode == DAY_SET_RESP_OVL) {
                return_layout_mode = LAYOUT_SET_NOTICE;
            } else if (__this->set_day_mode == DAY_SET_RESP_EASY) {
                return_layout_mode = LAYOUT_SET_NOTICE;
            }
            ui_women_health_layout_switch(return_layout_mode);

            break;
        }
        return true;
        break;
    default:
        break;
    }
    return false;
}

static int list_women_health_cur_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct ui_grid *grid = (struct ui_grid *)elm;
    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, 2);
        ui_grid_energy_auto_center(grid, 1);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);

        /*初始化事件*/
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(LIST_WOMEN_HEALTH_CUR)
.onchange =  list_women_health_cur_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(LAYOUT_WOMEN_HEALTH_CUR)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =   list_women_health_layout_return_ontouch,
};
REGISTER_UI_EVENT_HANDLER(LAYOUT_WOMEN_HEALTH_SET_NOTICE)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =   list_women_health_layout_return_ontouch,
};
REGISTER_UI_EVENT_HANDLER(LAYOUT_WOMEN_HEALTH_SET_CYCLE)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =   list_women_health_layout_return_ontouch,
};
REGISTER_UI_EVENT_HANDLER(LAYOUT_WOMEN_HEALTH_SET_DATE)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =   list_women_health_layout_return_ontouch,
};
REGISTER_UI_EVENT_HANDLER(LAYOUT_WOMEN_HEALTH_SET_DAY)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =   list_women_health_layout_return_ontouch,
};

/**********************************************************************************
			选择页面
**********************************************************************************/
static int  list_women_health_sel_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_ENERGY:
    case ELM_EVENT_TOUCH_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        int touch_index = ui_grid_touch_item(grid);
        if (e->move_flag) {
            break;
        }
        /* printf("%s move:%d touch_index:%d", __func__, e->move_flag, touch_index); */
        switch (touch_index) {
        case 0:
            ui_women_health_layout_switch(LAYOUT_CUR);
            break;
        case 1:
            ui_women_health_layout_switch(LAYOUT_SET_NOTICE);
            break;
        case 2:
            ui_women_health_layout_switch(LAYOUT_SET_CYCLE);
            break;
        }
        break;

    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LIST_WOMEN_HEALTH_SEL)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = list_women_health_sel_ontouch,
};


static int mense_win_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct ui_grid *grid = (struct ui_grid *)elm;
    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        __info.layout_mode =  LAYOUT_MAIN_SEL;
        __info.curr_layout_id =   LAYOUT_WOMEN_HEALTH_SEL;
        break;
    default:
        return false;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(ID_WINDOW_MENU_WOMEN_HEALTH)
.onchange =   mense_win_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
//==========================提醒===============================

static int  women_health_warning_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    switch (event) {
    case ON_CHANGE_SHOW_PROBE:
        int resp_type = mense_resp_status_get();	//显示用的状态
        int resp_real_status = mense_next_status_get(mense_now_status_get());//细分状态
        if (elm->id == WOMEN_HEALTH_WARNING_STA_PIC) {
            ui_pic_set_image_index((struct ui_pic *)elm, resp_type);
        } else if (elm->id == WOMEN_HEALTH_WARNING_BEGIN_TIME) {
            struct sys_time time;
            mense_status_begin_time_get(resp_real_status, &time);
            ui_time_update((struct ui_time *)elm, (struct utime *)&time);
        } else if (elm->id == WOMEN_HEALTH_WARNING_END_TIME) {
            struct sys_time time;
            mense_status_end_time_get(resp_real_status, &time);
            ui_time_update((struct ui_time *)elm, (struct utime *)&time);
        }
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_WARNING_STA_PIC)
.onchange = women_health_warning_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_WARNING_BEGIN_TIME)
.onchange = women_health_warning_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(WOMEN_HEALTH_WARNING_END_TIME)
.onchange = women_health_warning_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif// TCFG_UI_DRAW_DEMO
#endif// CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE

