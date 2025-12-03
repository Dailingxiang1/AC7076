/** * @file ui_action_alarm_clock.c
 * @brief 闹钟功能实现
 */
#include "app_config.h"
#include "system/timer.h"
#include "key_event_deal.h"
#include "app_mode_manager/app_mode_manager.h"
#include "app_main.h"
#include "alarm.h"
#include "rtc.h"
#include "ui.h"
#include "ui_api.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/result_pic_index.h"
#include "jlui_app/watch_syscfg_manage.h"
#include "ui_draw/ui_type.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-ALARM-CLOCK]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_alarm_clock.data.bss")
#pragma data_seg(".ui_action_alarm_clock.data")
#pragma const_seg(".ui_action_alarm_clock.text.const")
#pragma code_seg(".ui_action_alarm_clock.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_ALARM

#define STYLE_NAME  JL

/**********************
 *      DEFINES
 *********************/
#define __this  (p_alarm_clock_handle)
#define INVALID_VALUE   0xff

/**********************
 *      TYPEDEFS
 *********************/
typedef struct alarm_clock_handle {
    u8 alarm_clock_add_or_edit; /*0:是在添加新闹钟信息 1:是在编辑已有闹钟信息*/
    u8 cur_time_index;          /*当前操作的闹钟顺序,按时间顺序*/
    u8 cur_week;                /*当前操作闹钟的week配置*/
    struct sys_time cur_time;   /*当前操作闹钟的time配置*/
    u32 min_grid_highlight_elm_id;
    u32 hour_grid_highlight_elm_id;
} alarm_clock_handle_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ui_alarm_info_init(void);
static void ui_alarm_info_deinit(void);
static u8 ui_alarm_get_count(void);
static void ui_alarm_get_alarm_bt_time_index(u8 time_index, PT_ALARM alarm);
static void ui_alarm_save_cur_alarm(void);
static void ui_alarm_update_cur_alarm_info(u8 time_index);
static u8 ui_alarm_get_cur_week(void);
static void ui_alarm_set_cur_week(u8 week);
static void ui_alarm_get_cur_time(struct sys_time *time);
static void ui_alarm_set_cur_time(struct sys_time *time);
static void ui_alarm_set_add_or_edit_falg(u8 flag);
static u8 ui_alarm_is_add_or_edit(void);
static void ui_alarm_del_cur_alarm(void);
static void ui_alarm_switch(u8 time_index, u8 is_on);
static u8 ui_text_parse_alarm_week(u8 week, u8 *index_buf);

/**********************
 *  STATIC VARIABLES
 *********************/
static alarm_clock_handle_t *p_alarm_clock_handle;
static u8 week_tmp;


/************************************************
 *              初始化相关数据
 ***********************************************/
static int ui_alarm_clock_window_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        ui_alarm_info_init();
        break;
    case ON_CHANGE_RELEASE:
        ui_alarm_info_deinit();
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_ALARM)
.onchange = ui_alarm_clock_window_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 *              布局之间跳转管理
 ***********************************************/
/*添加闹钟布局(ALARM_ADD_LAYOUT) "返回图片" 点击跳转*/
static int ui_alarm_add_layout_back_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_hide(ALARM_ADD_LAYOUT);
            ui_show(ALARM_DISPLAY_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_ADD_LAYOUT_BACK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_add_layout_back_pic_ontouch,
};

/*添加闹钟布局(ALARM_ADD_LAYOUT) 右滑跳转*/
static int ui_alarm_add_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(ALARM_ADD_LAYOUT);
        ui_show(ALARM_DISPLAY_LAYOUT);
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_ADD_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_add_layout_ontouch,
};

/*添加闹钟布局(ALARM_ADD_LAYOUT) "进入时间设置图片" 点击跳转*/
static int ui_alarm_add_time_set_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_hide(ALARM_ADD_LAYOUT);
            ui_show(ALARM_TIME_SET_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_ADD_TIME_SET_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_add_time_set_pic_ontouch,
};

/*添加闹钟布局(ALARM_ADD_LAYOUT) "进入响铃设置图片" 点击跳转*/
static int ui_alarm_add_ring_set_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_hide(ALARM_ADD_LAYOUT);
            ui_show(ALARM_RING_SET_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_ADD_RING_SET_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_add_ring_set_pic_ontouch,
};

/*编辑闹钟布局(ALARM_EDIT_LAYOUT) "返回图片" 点击跳转*/
static int ui_alarm_edit_layout_back_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_alarm_save_cur_alarm();
            ui_hide(ALARM_EDIT_LAYOUT);
            ui_show(ALARM_DISPLAY_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_EDIT_LAYOUT_BACK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_edit_layout_back_pic_ontouch,
};

/*编辑闹钟布局(ALARM_EDIT_LAYOUT) 右滑跳转*/
static int ui_alarm_edit_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_alarm_save_cur_alarm();
        ui_hide(ALARM_EDIT_LAYOUT);
        ui_show(ALARM_DISPLAY_LAYOUT);
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_EDIT_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_edit_layout_ontouch,
};

/*编辑闹钟布局(ALARM_EDIT_LAYOUT) "进入时间设置图片" 点击跳转*/
static int ui_alarm_edit_time_set_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_hide(ALARM_EDIT_LAYOUT);
            ui_show(ALARM_TIME_SET_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_EDIT_TIME_SET_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_edit_time_set_pic_ontouch,
};

/*编辑闹钟布局(ALARM_EDIT_LAYOUT) "进入响铃设置图片" 点击跳转*/
static int ui_alarm_edit_ring_set_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_hide(ALARM_EDIT_LAYOUT);
            ui_show(ALARM_RING_SET_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_EDIT_RING_SET_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_edit_ring_set_pic_ontouch,
};

/*时间设置布局(ALARM_TIME_SET_LAYOUT) "返回图片" 点击跳转*/
static int ui_alarm_time_set_layout_back_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_hide(ALARM_TIME_SET_LAYOUT);
            if (ui_alarm_is_add_or_edit() == 1) {
                ui_show(ALARM_EDIT_LAYOUT);
            } else {
                ui_show(ALARM_ADD_LAYOUT);
            }
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_TIME_SET_LAYOUT_BACK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_time_set_layout_back_pic_ontouch,
};

/*时间设置布局(ALARM_TIME_SET_LAYOUT) 右滑跳转*/
static int ui_alarm_time_set_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(ALARM_TIME_SET_LAYOUT);
        if (ui_alarm_is_add_or_edit() == 1) {
            ui_show(ALARM_EDIT_LAYOUT);
        } else {
            ui_show(ALARM_ADD_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_TIME_SET_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_time_set_layout_ontouch,
};

/*响铃设置布局(ALARM_RING_SET_LAYOUT) "返回图片" 点击跳转*/
static int ui_alarm_ring_set_layout_back_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_hide(ALARM_RING_SET_LAYOUT);
            if (ui_alarm_is_add_or_edit() == 1) {
                ui_show(ALARM_EDIT_LAYOUT);
            } else {
                ui_show(ALARM_ADD_LAYOUT);
            }
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_RING_SET_LAYOUT_BACK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_ring_set_layout_back_pic_ontouch,
};

/*响铃设置布局(ALARM_RING_SET_LAYOUT) 右滑跳转*/
static int ui_alarm_ring_set_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(ALARM_RING_SET_LAYOUT);
        if (ui_alarm_is_add_or_edit() == 1) {
            ui_show(ALARM_EDIT_LAYOUT);
        } else {
            ui_show(ALARM_ADD_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_RING_SET_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_ring_set_layout_ontouch,
};



/************************************************
 *      显示闹钟信息布局处理
 *      ALARM_DISPLAY_LAYOUT
 ***********************************************/
static int ui_alarm_exist_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (ui_alarm_get_count()) {
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
REGISTER_UI_EVENT_HANDLER(ALARM_EXIST_LAYOUT)
.onchange = ui_alarm_exist_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_alarm_not_exist_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (ui_alarm_get_count()) {
            layout->elm.css.invisible = 1;
        } else {
            layout->elm.css.invisible = 0;
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_NOT_EXIST_LAYOUT)
.onchange = ui_alarm_not_exist_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/* "添加闹钟"文字控件 处理*/
static int ui_alarm_add_text_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            u8 sel_index = ui_alarm_get_count();    /*添加闹钟，直接按最后个闹钟添加*/
            ui_alarm_set_add_or_edit_falg(0);
            ui_alarm_update_cur_alarm_info(sel_index);
            ui_hide(ALARM_DISPLAY_LAYOUT);
            if (ui_alarm_get_count() < M_MAX_ALARM_NUMS) {
                ui_show(ALARM_ADD_LAYOUT);
            } else {
                ui_show(ALARM_QUANTITY_EXCEED_LAYOUT);
            }
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_ADD_TEXT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_add_text_ontouch,
};


/*根据ui工程布局 时间控件最先遍历*/
static int alarm_info_grid_child_cb(void *_ctrl, int id, int type, int index)
{
    static T_ALARM tmp_alarm;
    switch (type) {
    case CTRL_TYPE_TIME:
        ui_alarm_get_alarm_bt_time_index(index, &tmp_alarm);
        log_debug("<%s> index:%d hour:%d, min:%d", __func__, index, tmp_alarm.time.hour, tmp_alarm.time.min);
        struct utime time = {0};
        time.hour = tmp_alarm.time.hour;
        time.min = tmp_alarm.time.min;
        ui_time_update((struct ui_time *)_ctrl, &time);
        break;
    case CTRL_TYPE_PIC:
        log_debug("<%s> tmp_alarm.sw:%d", __func__, tmp_alarm.sw);
        ui_pic_set_image_index((struct ui_pic *)_ctrl, !!tmp_alarm.sw);
        break;
    case CTRL_TYPE_TEXT:
        log_debug("<%s> tmp_alarm.mode:0x%x", __func__, tmp_alarm.mode);
        u8 index_buf[11] = {0}; /*最多显示 "一，二，三，四，五，六"*/
        u8 index_num =  ui_text_parse_alarm_week(tmp_alarm.mode, index_buf);
        log_debug_hexdump(index_buf, index_num);
        ui_text_set_multi_text_index((struct ui_text *)_ctrl, index_buf, index_num);
        break;
    default:
        break;
    }
    return 0;
}

static int iterate_alarm_info_grid_child_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    int type = ui_id2type(elm->id);
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (u32)arg;
        alarm_info_grid_child_cb(elm, elm->id, type, index);
    }
    return false;
}



static int ui_alarm_info_grid_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;

    switch (event) {
    case ON_CHANGE_INIT:
        row = ui_alarm_get_count();
        col = 1;
        ui_set_default_handler(&grid->elm, NULL, NULL, iterate_alarm_info_grid_child_onchange);
        ui_grid_init_dynamic(grid, &row, &col);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE1);
        ui_grid_set_energy_target_line(grid, 102);
        break;
    case ON_CHANGE_RELEASE:
        ui_set_default_handler(&grid->elm, NULL, NULL, NULL);
        break;
    default:
        break;
    }
    return false;
}

static int ui_alarm_info_grid_ontouch(void *ctr, struct element_touch_event *e)
{
    T_ALARM tmp_alarm;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }
        int sel_index =  ui_grid_cur_item_dynamic(grid);
        log_debug("%s sel:%d touch:%d", __func__, sel_index, grid->touch_index);
        if (grid->touch_index == -1) {
            return false;
        }
        /*开关*/
        struct element *elm, *child;
        struct rect child_r;
        list_for_each_child_element(elm, &grid->elm) {
            list_for_each_child_element(child, elm) {
                if (ui_id2type(child->id) == CTRL_TYPE_PIC) {
                    ui_core_get_element_abs_rect(child, &child_r);
                    if (in_rect(&child_r, &e->pos)) {
                        ui_alarm_switch(sel_index, 0);
                        ui_alarm_get_alarm_bt_time_index(sel_index, &tmp_alarm);
                        ui_pic_show_image_by_id(child->id, tmp_alarm.sw);
                        return false;
                    }
                }
            }
        }

        /* 编辑*/
        ui_alarm_set_add_or_edit_falg(1);
        ui_alarm_update_cur_alarm_info(sel_index);
        ui_hide(ALARM_DISPLAY_LAYOUT);
        ui_show(ALARM_EDIT_LAYOUT);
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_INFO_GRID)
.onchange = ui_alarm_info_grid_onchange,
 .onkey = NULL,
  .ontouch = ui_alarm_info_grid_ontouch,
};



/************************************************
 *      添加闹钟超出数量限制提示布局处理
 *      ALARM_QUANTITY_EXCEED_LAYOUT
 ***********************************************/
/* "我知道了"文字控件 处理 */
static int ui_alarm_quantity_exceed_confirm_text_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_hide(ALARM_QUANTITY_EXCEED_LAYOUT);
            ui_show(ALARM_DISPLAY_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_QUANTITY_EXCEED_CONFIRM_TEXT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_quantity_exceed_confirm_text_ontouch,
};



/************************************************
 *      显示闹钟信息布局处理
 *      ALARM_EDIT_LAYOUT
 ***********************************************/
static int ui_alarm_edit_time_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_time *time = (struct ui_time *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        struct utime utime = {0};
        struct sys_time tmp_time;
        ui_alarm_get_cur_time(&tmp_time);
        utime.hour = tmp_time.hour;
        utime.min = tmp_time.min;
        ui_time_update(time, &utime);
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(ALARM_EDIT_TIME)
.onchange = ui_alarm_edit_time_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/* "删除闹钟"文字控件 处理*/
static int ui_alarm_del_text_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_alarm_del_cur_alarm();
            ui_hide(ALARM_EDIT_LAYOUT);
            ui_show(ALARM_DISPLAY_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_EDIT_DEL_TEXT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_del_text_ontouch,
};

static int ui_alarm_edit_week_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        u8 cur_week = ui_alarm_get_cur_week();
        log_debug("<%s> cur_week:0x%x", __func__, cur_week);
        u8 index_buf[11] = {0}; /*最多显示 "一，二，三，四，五，六"*/
        u8 index_num =  ui_text_parse_alarm_week(cur_week, index_buf);
        log_debug_hexdump(index_buf, index_num);
        ui_text_set_multi_text_index(text, index_buf, index_num);
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_EDIT_WEEK_TEXT)
.onchange = ui_alarm_edit_week_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 *      添加闹钟布局显示处理
 *      ALARM_ADD_LAYOUT
 ***********************************************/
/*添加闹钟时间显示*/
static int ui_alarm_add_time_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_time *time = (struct ui_time *)_ctrl;
    struct sys_time tmp_time;
    struct utime utime;
    switch (event) {
    case ON_CHANGE_INIT:
        ui_alarm_get_cur_time(&tmp_time);
        utime.hour = tmp_time.hour;
        utime.min = tmp_time.min;
        ui_time_update(time, &utime);
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(ALARM_ADD_TIME)
.onchange = ui_alarm_add_time_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/*点击保存闹钟*/
static int ui_alarm_add_confirm_text_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_alarm_save_cur_alarm();
            ui_hide(ALARM_ADD_LAYOUT);
            ui_show(ALARM_DISPLAY_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_ADD_CONFIRM_TEXT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_add_confirm_text_ontouch,
};

static int ui_alarm_add_week_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        u8 cur_week = ui_alarm_get_cur_week();
        log_debug("<%s> cur_week:0x%x", __func__, cur_week);
        u8 index_buf[11] = {0}; /*最多显示 "一，二，三，四，五，六"*/
        u8 index_num =  ui_text_parse_alarm_week(cur_week, index_buf);
        log_debug_hexdump(index_buf, index_num);
        ui_text_set_multi_text_index(text, index_buf, index_num);
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_ADD_WEEK_TEXT)
.onchange = ui_alarm_add_week_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/************************************************
 *      闹钟时间设置
 *      ALARM_TIME_SET_LAYOUT
 ***********************************************/

/****************** 小时设置 ******************/
static int alarm_set_hour_number_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    int index;
    switch (event) {
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        int time = index % 24;
        struct unumber num;
        num.type = TYPE_NUM;
        num.numbs = 1;
        num.number[0] = time;
        ui_number_update(number, &num);
        break;
    case ON_CHANGE_HIGHLIGHT:
        __this->hour_grid_highlight_elm_id = number->text.elm.id;
        break;
    case ON_CHANGE_SHOW:
        struct draw_context *dc = (struct draw_context *)arg;
        if (number->text.elm.id == __this->hour_grid_highlight_elm_id) {
            dc->custom_color = 0;
        } else {
            dc->custom_color = BIT(UI_CUSTOM_COLOR_BIT_IMAGE);
            dc->custom_argb8888 = 0xff777777;
        }
        break;
    default:
        break;
    }
    return 0;
}

REGISTER_UI_EVENT_HANDLER(ALARM_SET_HOUR_1_NUMBER)
.onchange = alarm_set_hour_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ALARM_SET_HOUR_2_NUMBER)
.onchange = alarm_set_hour_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ALARM_SET_HOUR_3_NUMBER)
.onchange = alarm_set_hour_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ALARM_SET_HOUR_4_NUMBER)
.onchange = alarm_set_hour_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int alarm_hour_set_grid_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_hour = 0;
    int base = 10000 * 24;
    int first_move_step = 0;
    struct rect r;
    struct sys_time tmp_time;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_alarm_get_cur_time(&tmp_time);
        time_hour = tmp_time.hour;
        row = base;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);

        base = base / 2;
        ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

        base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;
        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_hour == 0) ? r.height + 5 : 0;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    default:
        break;
    }
    return 0;
}

REGISTER_UI_EVENT_HANDLER(ALARM_HOUR_SET_GRID)
.onchange = alarm_hour_set_grid_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/****************** 分钟设置 ******************/
static int alarm_set_min_number_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    int index;
    switch (event) {
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        int time = index % 60;
        struct unumber num;
        num.type = TYPE_NUM;
        num.numbs = 1;
        num.number[0] = time;
        ui_number_update(number, &num);
        break;
    case ON_CHANGE_HIGHLIGHT:
        __this->min_grid_highlight_elm_id = number->text.elm.id;
        break;
    case ON_CHANGE_SHOW:
        struct draw_context *dc = (struct draw_context *)arg;
        if (number->text.elm.id == __this->min_grid_highlight_elm_id) {
            dc->custom_color = 0;
        } else {
            dc->custom_color = BIT(UI_CUSTOM_COLOR_BIT_IMAGE);
            dc->custom_argb8888 = 0xff777777;

        }
        break;
    default:
        break;
    }
    return 0;
}

REGISTER_UI_EVENT_HANDLER(ALARM_SET_MIN_1_NUMBER)
.onchange = alarm_set_min_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ALARM_SET_MIN_2_NUMBER)
.onchange = alarm_set_min_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ALARM_SET_MIN_3_NUMBER)
.onchange = alarm_set_min_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ALARM_SET_MIN_4_NUMBER)
.onchange = alarm_set_min_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int alarm_min_set_grid_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_min = 0;
    int base = 10000 * 60;
    int first_move_step = 0;
    struct rect r;
    struct sys_time tmp_time;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_alarm_get_cur_time(&tmp_time);
        time_min = tmp_time.min;
        row = base;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);

        base = base / 2;
        ui_grid_set_hindex_dynamic(grid, time_min + base, true, 1);

        base_index_once = ((time_min >= 1) ? (time_min - 1) : 0) + base;
        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_min == 0) ? r.height + 5 : 0;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
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
        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        ui_grid_slide_with_callback_dynamic(grid, SCROLL_DIRECTION_UD, -(r.height + grid->y_interval), key_redraw);
        return true;
        break;
    case KEY_UI_MINUS:
        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        ui_grid_slide_with_callback_dynamic(grid, SCROLL_DIRECTION_UD, (r.height + grid->y_interval), key_redraw);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(ALARM_MIN_SET_GRID)
.onchange = alarm_min_set_grid_onchange,
 .onkey = default_vlist_onkey,
  .ontouch = NULL,
};


static int ui_alarm_set_confirm_text_ontouch(void *ctr, struct element_touch_event *e)
{
    int hour, min;
    struct ui_grid *grid = NULL;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag != 0) {
            break;
        }

        grid = ui_grid_for_id(ALARM_HOUR_SET_GRID);
        hour = ui_grid_get_hindex_dynamic(grid);
        hour = hour % 24;
        grid = ui_grid_for_id(ALARM_MIN_SET_GRID);
        min = ui_grid_get_hindex_dynamic(grid);
        min = min % 60;
        __this->cur_time.hour = hour;
        __this->cur_time.min = min;

        ui_hide(ALARM_TIME_SET_LAYOUT);
        if (ui_alarm_is_add_or_edit() == 1) {
            ui_show(ALARM_EDIT_LAYOUT);
        } else {
            ui_show(ALARM_ADD_LAYOUT);
        }
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_TIME_SET_CONFIRM_TEXT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_set_confirm_text_ontouch,
};

/************************************************
 *      闹钟响铃设置
 *      ALARM_RING_SET_LAYOUT
 ***********************************************/
static int ui_alarm_week_pic_onchange(void *ctrl, enum element_change_event e, void *arge)
{
    struct ui_pic *pic = (struct ui_pic *) ctrl;

    switch (e) {
    case ON_CHANGE_INIT:
        week_tmp = ui_alarm_get_cur_week();
        switch (pic->elm.id) {
        case ALARM_MONDAY_PIC:
            if (week_tmp & E_ALARM_MODE_EVERY_MONDAY) {
                ui_pic_set_image_index(pic, 1);
            }
            break;
        case ALARM_TUEASDAY_PIC:
            if (week_tmp & E_ALARM_MODE_EVERY_TUESDAY) {
                ui_pic_set_image_index(pic, 1);
            }
            break;
        case ALARM_WEDNESDAYS_PIC:
            if (week_tmp & E_ALARM_MODE_EVERY_WEDNESDAY) {
                ui_pic_set_image_index(pic, 1);
            }
            break;
        case ALARM_THURSDAY_PIC:
            if (week_tmp & E_ALARM_MODE_EVERY_THURSDAY) {
                ui_pic_set_image_index(pic, 1);
            }
            break;
        case ALARM_FRIDAY_PIC:
            if (week_tmp & E_ALARM_MODE_EVERY_FRIDAY) {
                ui_pic_set_image_index(pic, 1);
            }
            break;
        case ALARM_SATURDAY_PIC:
            if (week_tmp & E_ALARM_MODE_EVERY_SATURDAY) {
                ui_pic_set_image_index(pic, 1);
            }
            break;
        case ALARM_SUNDAY_PIC:
            if (week_tmp & E_ALARM_MODE_EVERY_SUNDAY) {
                ui_pic_set_image_index(pic, 1);
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
    return false;
}

static int ui_alarm_week_pic_ontouch(void *ctrl, struct element_touch_event *e)
{
    struct ui_pic *_pic = (struct ui_pic *) ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            switch (_pic->elm.id) {
            case ALARM_MONDAY_PIC:
                if (week_tmp & E_ALARM_MODE_EVERY_MONDAY) {
                    week_tmp &= ~E_ALARM_MODE_EVERY_MONDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 0);
                } else if (!(week_tmp & E_ALARM_MODE_EVERY_MONDAY)) {
                    week_tmp |= E_ALARM_MODE_EVERY_MONDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 1);
                }
                break;
            case ALARM_TUEASDAY_PIC:
                if (week_tmp & E_ALARM_MODE_EVERY_TUESDAY) {
                    week_tmp &= ~E_ALARM_MODE_EVERY_TUESDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 0);
                } else if (!(week_tmp & E_ALARM_MODE_EVERY_TUESDAY)) {
                    week_tmp |= E_ALARM_MODE_EVERY_TUESDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 1);
                }
                break;
            case ALARM_WEDNESDAYS_PIC:
                if (week_tmp & E_ALARM_MODE_EVERY_WEDNESDAY) {
                    week_tmp &= ~E_ALARM_MODE_EVERY_WEDNESDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 0);
                } else if (!(week_tmp & E_ALARM_MODE_EVERY_WEDNESDAY)) {
                    week_tmp |= E_ALARM_MODE_EVERY_WEDNESDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 1);
                }
                break;
            case ALARM_THURSDAY_PIC:
                if (week_tmp & E_ALARM_MODE_EVERY_THURSDAY) {
                    week_tmp &= ~E_ALARM_MODE_EVERY_THURSDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 0);
                } else if (!(week_tmp & E_ALARM_MODE_EVERY_THURSDAY)) {
                    week_tmp |= E_ALARM_MODE_EVERY_THURSDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 1);
                }
                break;
            case ALARM_FRIDAY_PIC:
                if (week_tmp & E_ALARM_MODE_EVERY_FRIDAY) {
                    week_tmp &= ~E_ALARM_MODE_EVERY_FRIDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 0);
                } else if (!(week_tmp & E_ALARM_MODE_EVERY_FRIDAY)) {
                    week_tmp |= E_ALARM_MODE_EVERY_FRIDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 1);
                }
                break;
            case ALARM_SATURDAY_PIC:
                if (week_tmp & E_ALARM_MODE_EVERY_SATURDAY) {
                    week_tmp &= ~E_ALARM_MODE_EVERY_SATURDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 0);
                } else if (!(week_tmp & E_ALARM_MODE_EVERY_SATURDAY)) {
                    week_tmp |= E_ALARM_MODE_EVERY_SATURDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 1);
                }
                break;
            case ALARM_SUNDAY_PIC:
                if (week_tmp & E_ALARM_MODE_EVERY_SUNDAY) {
                    week_tmp &= ~E_ALARM_MODE_EVERY_SUNDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 0);
                } else if (!(week_tmp & E_ALARM_MODE_EVERY_SUNDAY)) {
                    week_tmp |= E_ALARM_MODE_EVERY_SUNDAY;
                    ui_pic_show_image_by_id(_pic->elm.id, 1);
                }
                break;
            default:
                break;
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_SUNDAY_PIC)
.ontouch = ui_alarm_week_pic_ontouch,
 .onkey = NULL,
  .onchange  = ui_alarm_week_pic_onchange,
};

REGISTER_UI_EVENT_HANDLER(ALARM_MONDAY_PIC)
.ontouch = ui_alarm_week_pic_ontouch,
 .onkey = NULL,
  .onchange  = ui_alarm_week_pic_onchange,
};

REGISTER_UI_EVENT_HANDLER(ALARM_TUEASDAY_PIC)
.ontouch = ui_alarm_week_pic_ontouch,
 .onkey = NULL,
  .onchange  = ui_alarm_week_pic_onchange,
};

REGISTER_UI_EVENT_HANDLER(ALARM_WEDNESDAYS_PIC)
.ontouch = ui_alarm_week_pic_ontouch,
 .onkey = NULL,
  .onchange  = ui_alarm_week_pic_onchange,
};

REGISTER_UI_EVENT_HANDLER(ALARM_THURSDAY_PIC)
.ontouch = ui_alarm_week_pic_ontouch,
 .onkey = NULL,
  .onchange  = ui_alarm_week_pic_onchange,
};

REGISTER_UI_EVENT_HANDLER(ALARM_FRIDAY_PIC)
.ontouch = ui_alarm_week_pic_ontouch,
 .onkey = NULL,
  .onchange  = ui_alarm_week_pic_onchange,
};

REGISTER_UI_EVENT_HANDLER(ALARM_SATURDAY_PIC)
.ontouch = ui_alarm_week_pic_ontouch,
 .onkey = NULL,
  .onchange  = ui_alarm_week_pic_onchange,
};

/*"保存"文字控件 点击保存响铃设置*/
static int ui_alarm_ring_set_confirm_text_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_alarm_set_cur_week(week_tmp);
            ui_hide(ALARM_RING_SET_LAYOUT);
            if (ui_alarm_is_add_or_edit() == 1) {
                ui_show(ALARM_EDIT_LAYOUT);
            } else {
                ui_show(ALARM_ADD_LAYOUT);
            }
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_RING_SET_CONFIRM_TEXT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_ring_set_confirm_text_ontouch,
};



/************************************************
 *      闹钟响铃页面
 ***********************************************/

static int ui_alarm_ringing_layout_onchange(void *ctr, enum element_change_event e, void *arg)
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

static int ui_alarm_ringing_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_RINGING_LAYOUT)
.onchange = ui_alarm_ringing_layout_onchange,
 .onkey = NULL,
  .ontouch = ui_alarm_ringing_layout_ontouch,
};

static int ui_alarm_ringing_close_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (alarm_active_flag_get()) {
            alarm_stop(0);
        }
        if (get_need_password() == 1) {
            /* UI_HIDE_CURR_WINDOW(); */
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_ALARM_RINGING);
            UI_SHOW_WINDOW(ID_WINDOW_POWERON_PASSWORD);
        } else {
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_ALARM_RINGING);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_RINGING_CLOSE_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_ringing_close_pic_ontouch,
};

static int ui_alarm_ringing_remind_me_later_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (alarm_active_flag_get()) {
            alarm_stop(1);
        }
        alarm_snooze();
        if (get_need_password() == 1) {
            /* UI_HIDE_CURR_WINDOW(); */
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_ALARM_RINGING);
            UI_SHOW_WINDOW(ID_WINDOW_POWERON_PASSWORD);
        } else {
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_ALARM_RINGING);
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALARM_RINGING_REMIND_ME_LATER_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_alarm_ringing_remind_me_later_pic_ontouch,
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



/************************************************
 *             闹钟相关工具函数
 ***********************************************/

static void ui_alarm_info_init(void)
{
    __this = zalloc(sizeof(alarm_clock_handle_t));
    if (!__this) {
        log_info("<%s> zalloc fail", __func__);
        return;
    }
}

static void ui_alarm_info_deinit(void)
{

    if (__this) {
        free(__this);
        __this = NULL;
    }
}

/**
 * @brief 获取当前闹钟数量
 */
static u8 ui_alarm_get_count(void)
{
    /* alarm_print_all_info(); */
    return  alarm_get_total();
}

/**
 * @brief 通过时间顺序的index,获取alarm数据
 *
 * @param time_index 按时间顺序获取，范围:0 -- (M_MAX_ALARM_NUMS-1)
                     假设设置了3个闹钟 0:最先操作的闹钟 2:最后操作的闹钟
 * @param alarm 存储读取出来alarm数据指针
 */
static void ui_alarm_get_alarm_bt_time_index(u8 time_index, PT_ALARM alarm)
{
    u8 table_index;
    if ((!(time_index < M_MAX_ALARM_NUMS)) || (!alarm)) {
        return;
    }
    get_alarm_number2table(time_index, &table_index);
    if (table_index == INVALID_VALUE) {
        log_error("<%s> table_index err", __func__);
        return;
    }

    alarm_get_info(alarm, table_index);
}

/**
 * @brief 保存当前操作的闹钟信息
 */
static void ui_alarm_save_cur_alarm(void)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_debug("<%s> rets:%x\n", __func__, rets);

    u8 table_index;
    get_alarm_number2table(__this->cur_time_index, &table_index); /*根据闹钟操作顺序获取对应alarm_tab中的alarm*/
    T_ALARM tmp_alarm;
    /*如果闹钟是存在的，得先读取出来，因为name_len需要保留下来*/
    alarm_get_info(&tmp_alarm, table_index);
    for (int index = 0; index < table_index; index++) {
        T_ALARM tmp_alarm;
        alarm_get_info(&tmp_alarm, index);
        // 当闹钟列表有相同设置的闹钟，直接开启之前的闹钟，不重复添加
        if ((tmp_alarm.mode == __this->cur_week) && (tmp_alarm.time.hour == __this->cur_time.hour) && (tmp_alarm.time.min == __this->cur_time.min)) {
            log_debug("curr alarm already in the alarm list!!\n");
            ui_alarm_switch(index, 1);
            return;
        }
    }

    tmp_alarm.index = table_index;
    tmp_alarm.sw = 1;
    tmp_alarm.mode = __this->cur_week;
    tmp_alarm.time = __this->cur_time;
    /* set_alarm.name_len 保留原来的数据 */

    log_debug("time_index:%d table_index:%d", __this->cur_time_index, table_index);
    log_debug_hexdump((u8 *)&tmp_alarm, sizeof(T_ALARM));
    alarm_add(&tmp_alarm, table_index);
}
/**
 * @brief 开关当前闹钟
 *
 * @param time_index 按时间顺序获取，范围:0 -- (M_MAX_ALARM_NUMS-1)
                     假设设置了3个闹钟 0:最先操作的闹钟 2:最后操作的闹钟
          is_on      是否打开闹铃，1：打开闹钟   0：切换闹钟状态
 */
static void ui_alarm_switch(u8 time_index, u8 is_on)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_debug("<%s> rets:%x\n", __func__, rets);

    u8 table_index;
    get_alarm_number2table(time_index, &table_index); /*根据闹钟操作顺序获取对应alarm_tab中的alarm*/
    if (table_index == INVALID_VALUE) {
        log_error("<%s> table_index err", __func__);
        return;
    }
    T_ALARM tmp_alarm;
    alarm_get_info(&tmp_alarm, table_index);

    if (is_on) {
        tmp_alarm.sw = 1;
    } else {
        tmp_alarm.sw = !tmp_alarm.sw;
    }

    log_debug("time_index:%d table_index:%d", time_index, table_index);
    log_debug_hexdump((u8 *)&tmp_alarm, sizeof(T_ALARM));

    alarm_add(&tmp_alarm, table_index);
}

/**
 * @brief 删除当前操作的闹钟
 */
static void ui_alarm_del_cur_alarm(void)
{
    u8 table_index;
    get_alarm_number2table(__this->cur_time_index, &table_index); /*根据闹钟操作顺序获取对应alarm_tab中的alarm*/
    log_debug("<%s> cur_time_index:%d  table_index:%d", __func__, __this->cur_time_index, table_index);
    alarm_delete(table_index);
}

/**
 * @brief 更新当前操作的闹钟信息
 *
 * @param index 按时间顺序获取，范围:0 -- (M_MAX_ALARM_NUMS-1)
                假设设置了3个闹钟 0:最先操作的闹钟 2:最后操作的闹钟
 */
static void ui_alarm_update_cur_alarm_info(u8 time_index)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_debug("<%s> rets:%x\n", __func__, rets);

    u8 table_index;
    T_ALARM tmp_alarm;

    if (!(time_index < M_MAX_ALARM_NUMS)) {
        return;
    }

    get_alarm_number2table(time_index, &table_index);
    alarm_get_info(&tmp_alarm, table_index);
    log_debug("time_index:%d table_index:%d", time_index, table_index);
    log_debug_hexdump((u8 *)&tmp_alarm, sizeof(T_ALARM));

    __this->cur_time_index = time_index;
    __this->cur_week = tmp_alarm.mode;
    if (ui_alarm_is_add_or_edit() == 1) {
        __this->cur_time = tmp_alarm.time;
    } else {
        rtc_read_time(&__this->cur_time);
    }
}

/**
 * @brief 获取当前闹钟week配置
 */
static u8 ui_alarm_get_cur_week(void)
{
    return __this->cur_week;
}

/**
 * @brief 设置当前闹钟week配置
 */
static void ui_alarm_set_cur_week(u8 week)
{
    log_debug("<%s> week:%x", __func__, week);
    __this->cur_week = week;
}

/**
 * @brief 获取当前闹钟time配置
 */
static void ui_alarm_get_cur_time(struct sys_time *time)
{
    if (!time) {
        return;
    }

    memcpy(time, &__this->cur_time, sizeof(struct sys_time));
}

/**
 * @brief 设置当前闹钟time配置
 */
static void ui_alarm_set_cur_time(struct sys_time *time)
{
    if (!time) {
        return;
    }
    memcpy(&__this->cur_time, time, sizeof(struct sys_time));
}

/**
 * @brief 设置当前闹钟 是新添加的闹钟 or 是已存在的闹钟,进行修改
 *
 * @param index flag 0:是在添加新闹钟信息 1:是在编辑已有闹钟信息
 */
static void ui_alarm_set_add_or_edit_falg(u8 flag)
{
    __this->alarm_clock_add_or_edit = flag;
}

/**
 * @brief 0:是在添加新闹钟信息 1:是在编辑已有闹钟信息
 */
static u8 ui_alarm_is_add_or_edit(void)
{
    return __this->alarm_clock_add_or_edit;
}

/**
 * @brief 解析对应mulstr文字控件 显示week信息
 *
 * @param week
 * @param index_buf 存储excel文本id的buf
 * @return u8 要去拼接excel文本id数量
 */
static u8 ui_text_parse_alarm_week(u8 week, u8 *index_buf)
{
    u8 count = 0;
    if (week == E_ALARM_MODE_EVERY_DAY || week == 0xfe) {
        index_buf[0] = 10;  /* 显示"每天" */
        count = 1;
    } else if (week == E_ALARM_MODE_ONCE) {
        index_buf[0] = 8;   /* 显示"单次" */
        count = 1;
    } else if (week == 0x3e) {
        index_buf[0] = 9;   /* 显示"工作日" */
        count = 1;
    } else {
        for (u8 i = 1 ; i < 8 ; i++) {
            if (week & BIT(i)) {
                index_buf[count] = i - 1; /* 显示"一"之类的 */
                index_buf[count + 1] = 7; /* 显示"，"*/
                count += 2;
            }
        }
        count -= 1; /* 显示"一，二，三", 最后逗号不显示*/
    }
    return count;
}

#endif /* if TCFG_UI_ENABLE_ALARM */
#endif /* #if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) */

