/**
 * @Brief : 电话功能,通讯录显示,通话记录显示
 */
#include "system/includes.h"
#include "app_task.h"
#include "system/timer.h"
#include "data_storage.h"
#include "app_common.h"
#include "timestamp.h"
#include "rtc.h"
#include "ui.h"
#include "ui_api.h"
#include "jlui_app/ui_style.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-PHONE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_phone.data.bss")
#pragma data_seg(".ui_action_phone.data")
#pragma const_seg(".ui_action_phone.text.const")
#pragma code_seg(".ui_action_phone.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_PHONE_ACTION

#define STYLE_NAME  JL

/**********************
 *      DEFINES
 *********************/
#define __this  (p_phone_handle)

/**********************
 *      TYPEDEFS
 *********************/
typedef struct phone_handle {
    u8 *phonebook_storage_buf;
    u8 *call_log_storage_buf;
} phone_handle_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ui_phone_set_last_layout(u32 layout_id);

/**********************
 *  STATIC VARIABLES
 *********************/
static phone_handle_t *p_phone_handle;
static u32 last_layout_id = PHONE_ITEM_LIST_LAYOUT;



/************************************************
 *       进行电话页面 进行相关初始化
 ***********************************************/
static int ui_phone_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT_PROBE:
        __this = (phone_handle_t *)zalloc(sizeof(phone_handle_t));
        if (!__this) {
            log_error("[%s] line:%d zalloc fail", __func__, __LINE__);
            break;
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

REGISTER_UI_EVENT_HANDLER(PHONE_LAYOUT)
.onchange = ui_phone_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 *              通讯录显示处理
 ***********************************************/
static int ui_phone_phonebook_exist_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (ui_small_file_phonebook_get_count()) {
            layout->elm.css.invisible = 0;
        } else {
            layout->elm.css.invisible = 1;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PHONE_PHONEBOOK_EXIST_LAYOUT)
.onchange = ui_phone_phonebook_exist_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_phone_phonebook_not_exist_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (ui_small_file_phonebook_get_count()) {
            layout->elm.css.invisible = 1;
        } else {
            layout->elm.css.invisible = 0;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PHONE_PHONEBOOK_NOT_EXIST_LAYOUT)
.onchange = ui_phone_phonebook_not_exist_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int phonebook_grid_child_cb(void *_ctrl, int id, int type, int index)
{
    static small_file_phonebook_t *phonebook = NULL;


    switch (type) {
    case CTRL_TYPE_TEXT:
        struct ui_text *text = (struct ui_text *)_ctrl;
        phonebook = (small_file_phonebook_t *)&__this->phonebook_storage_buf[index * SMALL_FILE_PHONEBOOK_SIZE];
        ui_small_file_phonebook_read_by_index(phonebook, index);
        /* log_info("phonebook->name:%s, phonebook->number:%s", phonebook->name, phonebook->number); */
        ui_text_set_text_attrs(text, (const char *)phonebook->name, strlen(phonebook->name), FONT_ENCODE_UTF8, FONT_ENDIAN_SMALL, FONT_DEFAULT | FONT_SHOW_SCROLL);
        break;
    case CTRL_TYPE_NUMBER:
        struct ui_number *number = (struct ui_number *)_ctrl;
        struct unumber num;

        num.type = TYPE_STRING;
        num.num_str = (u8 *)phonebook->number;
        ui_number_update(number, &num);
        break;
    default:
        break;
    }
    return 0;
}

static int iterate_phonebook_grid_child_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    int type = ui_id2type(elm->id);
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (u32)arg;
        phonebook_grid_child_cb(elm, elm->id, type, index);
    }
    return 0;
}

static int ui_phone_phonebook_list_grid_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        int row = ui_small_file_phonebook_get_count();
        int col = 1;
        __this->phonebook_storage_buf = zalloc(row * SMALL_FILE_PHONEBOOK_SIZE);
        if (!__this->phonebook_storage_buf) {
            log_error("<%s> zalloc fail", __func__);
            break;
        }
        ui_set_default_handler(&grid->elm, NULL, NULL, iterate_phonebook_grid_child_onchange);
        ui_grid_init_dynamic(grid, &row, &col);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        break;
    case ON_CHANGE_RELEASE_PROBE:
        ui_set_default_handler(&grid->elm, NULL, NULL, NULL);
        if (__this->phonebook_storage_buf) {
            free(__this->phonebook_storage_buf);
            __this->phonebook_storage_buf = NULL;
        }
        break;
    default:
        break;
    }
    return false;
}

static int ui_phone_phonebook_list_grid_ontouch(void *ctr, struct element_touch_event *e)
{
    int sel_item;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    small_file_phonebook_t phonebook;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag == 1) {
            break;
        }
        sel_item = ui_grid_cur_item_dynamic(grid);
        ui_small_file_phonebook_read_by_index(&phonebook, sel_item);
        call_dial_number(strlen(phonebook.number), phonebook.number);
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PHONE_PHONEBOOK_LIST_GRID)
.onchange = ui_phone_phonebook_list_grid_onchange,
 .onkey = NULL,
  .ontouch = ui_phone_phonebook_list_grid_ontouch,
};



/************************************************
 *              通话记录显示处理
 ***********************************************/
static int ui_call_log_exist_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (ui_small_file_call_log_get_count()) {
            layout->elm.css.invisible = 0;
        } else {
            layout->elm.css.invisible = 1;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PHONE_CALL_LOG_EXIST_LAYOUT)
.onchange = ui_call_log_exist_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_call_log_not_exist_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (ui_small_file_call_log_get_count()) {
            layout->elm.css.invisible = 1;
        } else {
            layout->elm.css.invisible = 0;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PHONE_CALL_LOG_NOT_EXIST_LAYOUT)
.onchange = ui_call_log_not_exist_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int call_log_grid_child_cb(void *_ctrl, int id, int type, int index)
{
    static small_file_call_log_t *call_log = NULL;
    switch (type) {
    case CTRL_TYPE_TEXT:
        struct ui_text *text = (struct ui_text *)_ctrl;
        call_log = (small_file_call_log_t *)&__this->call_log_storage_buf[index * SMALL_FILE_CALL_LOG_SIZE];
        index = ui_small_file_call_log_get_count() - index - 1;
        ui_small_file_call_log_read_by_index(call_log, index);
        /* log_info_hexdump((u8 *)call_log, SMALL_FILE_CALL_LOG_SIZE); */
        /* log_debug("index:%d, call_log->date:%d", index, call_log->utc_time); */
        if (strlen(call_log->name) != 0) {
            text->elm.css.invisible = 0;
            ui_text_set_text_attrs(text, (const char *)call_log->name, strlen(call_log->name), FONT_ENCODE_UTF8, FONT_ENDIAN_SMALL, FONT_DEFAULT | FONT_SHOW_SCROLL);
        } else {
            text->elm.css.invisible = 1;
        }
        break;
    case CTRL_TYPE_NUMBER:
        struct ui_number *number = (struct ui_number *)_ctrl;
        struct unumber num;
        if (strlen(call_log->name) != 0) {
            number->text.elm.css.invisible = 1;
        } else {
            number->text.elm.css.invisible = 0;
            num.type = TYPE_STRING;
            num.num_str = (u8 *)call_log->number;
            ui_number_update(number, &num);
        }
        break;
    case CTRL_TYPE_TIME:
        struct ui_time *time = (struct ui_time *)_ctrl;
        struct utime ui_time = {0};
        struct sys_time call_log_time;
        struct sys_time cur_time;
        timestamp_utc_sec_2_mytime(call_log->utc_time, &call_log_time);
        ui_time.year = call_log_time.year;
        ui_time.month = call_log_time.month;
        ui_time.day = call_log_time.day;
        ui_time.hour = call_log_time.hour;
        ui_time.min = call_log_time.min;
        ui_time.sec = call_log_time.sec;
        ui_time_update(time, &ui_time);
        rtc_read_time(&cur_time);

        if (!strcmp(time->source, "date")) {
            if (cur_time.year == call_log_time.year && cur_time.month == call_log_time.month && \
                cur_time.day == call_log_time.day) {
                time->text.elm.css.invisible = 1;
            } else {
                time->text.elm.css.invisible = 0;
            }
        } else if (!strcmp(time->source, "time")) {
            if (cur_time.year == call_log_time.year && cur_time.month == call_log_time.month && \
                cur_time.day == call_log_time.day) {
                time->text.elm.css.invisible = 0;
            } else {
                time->text.elm.css.invisible = 1;
            }

        }
        break;
    case CTRL_TYPE_PIC:
        struct ui_pic *pic = (struct ui_pic *)_ctrl;
        ui_pic_set_image_index(pic, call_log->type);
        break;
    default:
        break;
    }
    return 0;
}

static int iterate_call_log_grid_child_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    int type = ui_id2type(elm->id);
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (u32)arg;
        call_log_grid_child_cb(elm, elm->id, type, index);
    }
    return 0;
}

static int ui_phone_call_log_list_grid_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        int row = ui_small_file_call_log_get_count();
        int col = 1;
        __this->call_log_storage_buf = zalloc(row * SMALL_FILE_CALL_LOG_SIZE);
        if (!__this->call_log_storage_buf) {
            log_error("<%s> zalloc fail", __func__);
            break;
        }
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        ui_set_default_handler(&grid->elm, NULL, NULL, iterate_call_log_grid_child_onchange);
        ui_grid_init_dynamic(grid, &row, &col);
        break;
    case ON_CHANGE_RELEASE_PROBE:
        if (__this->call_log_storage_buf) {
            free(__this->call_log_storage_buf);
            __this->call_log_storage_buf = NULL;
        }
        ui_set_default_handler(&grid->elm, NULL, NULL, NULL);
        break;
    default:
        break;
    }
    return false;
}

static int ui_phone_call_log_grid_ontouch(void *ctr, struct element_touch_event *e)
{
    int sel_item;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    small_file_call_log_t call_log;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag == 1) {
            break;
        }
        sel_item = ui_grid_cur_item_dynamic(grid);
        ui_small_file_call_log_read_by_index(&call_log, grid->dynamic->drow_num - sel_item - 1);
        call_dial_number(strlen(call_log.number), call_log.number);
        small_file_call_log_set_number(call_log.number, strlen(call_log.number) + 1); //strlen不计算‘\0’所以要+1；
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PHONE_CALL_LOG_LIST_GRID)
.onchange = ui_phone_call_log_list_grid_onchange,
 .onkey = NULL,
  .ontouch = ui_phone_call_log_grid_ontouch,
};



/************************************************
 *         刚进入页面,确定显示哪个布局
 ***********************************************/

/**
 * @brief 判断是否为最后次显示的布局
 */
static int is_last_layout(u32 layout_id)
{
    if (layout_id == last_layout_id) {
        return true;
    }
    return false;
}

/**
 * @brief 记录拨号跳转时，所在的布局，结束通话时候返回显示该布局
 */
static void ui_phone_set_last_layout(u32 layout_id)
{
    last_layout_id = layout_id;
}


static int ui_phone_call_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT_PROBE:
        if (is_last_layout(layout->elm.id) == true) {
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
static int ui_phone_call_log_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(layout->elm.id);
        ui_phone_set_last_layout(PHONE_ITEM_LIST_LAYOUT);
        ui_show(PHONE_ITEM_LIST_LAYOUT);
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_LOG_LAYOUT)
.onchange = ui_phone_call_layout_onchange,
 .onkey = NULL,
  .ontouch = ui_phone_call_log_layout_ontouch,
};


static int ui_phoneook_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT_PROBE:
        if (is_last_layout(layout->elm.id) == true) {
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
static int ui_phone_phonebook_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(layout->elm.id);
        ui_phone_set_last_layout(PHONE_ITEM_LIST_LAYOUT);
        ui_show(PHONE_ITEM_LIST_LAYOUT);
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_PHONEBOOK_LAYOUT)
.onchange = ui_phoneook_layout_onchange,
 .onkey = NULL,
  .ontouch = ui_phone_phonebook_ontouch,
};

static int ui_phone_item_list_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT_PROBE:
        if (is_last_layout(layout->elm.id) == true) {
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

REGISTER_UI_EVENT_HANDLER(PHONE_ITEM_LIST_LAYOUT)
.onchange = ui_phone_item_list_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/************************************************
 *          电话 UI页面之前跳转逻辑
 ***********************************************/
static void phone_item_list_grid_up_handler(struct ui_grid *grid, u8 sel_item)
{
    switch (sel_item) {
    case 0: /*最近通话*/
        ui_hide(PHONE_ITEM_LIST_LAYOUT);
        ui_phone_set_last_layout(PHONE_CALL_LOG_LAYOUT);
        ui_show(PHONE_CALL_LOG_LAYOUT);
        break;
    case 1: /*通讯录*/
        ui_hide(PHONE_ITEM_LIST_LAYOUT);
        ui_phone_set_last_layout(PHONE_PHONEBOOK_LAYOUT);
        ui_show(PHONE_PHONEBOOK_LAYOUT);
        break;
    case 2: /*拨号键盘*/
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_PHONE_KEYPAD);
        break;
    default:
        break;
    }
}

static int ui_phone_item_list_grid_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item;
    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
    case ELM_EVENT_TOUCH_ENERGY:
        return true;    /*该列表不需要滑动,拦截消息*/
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            sel_item = ui_grid_cur_item(grid);
            phone_item_list_grid_up_handler(grid, sel_item);
        }
        return true;
    default:
        break;
    }
    return false;
}
static int vlist_default_onchange(void *_ctrl, enum element_change_event event, void *arg)
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
REGISTER_UI_EVENT_HANDLER(PHONE_ITEM_LIST_GRID)
.onchange = vlist_default_onchange,
 .onkey = NULL,
  .ontouch = ui_phone_item_list_grid_ontouch,
};

static int ui_phone_back_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    int elm_type = 0;
    struct element *p, *elm;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            elm = ui_core_get_element_by_id(PHONE_LAYOUT);
            /*找到正在显示的layout，然后隐藏->显示对应页layout*/
            list_for_each_child_element(p, elm) {
                elm_type = ui_id2type(p->id);
                if ((elm_type == CTRL_TYPE_LAYOUT) && (!p->css.invisible)) {
                    ui_hide(p->id);
                    ui_phone_set_last_layout(PHONE_ITEM_LIST_LAYOUT);
                    ui_show(PHONE_ITEM_LIST_LAYOUT);
                }
            }
        }
        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PHONE_PHONEBOOK_EXIST_BACK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_phone_back_pic_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_PHONEBOOK_NOT_EXIST_BACK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_phone_back_pic_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_LOG_EXIST_BACK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_phone_back_pic_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PHONE_CALL_LOG_NOT_EXIST_BACK_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_phone_back_pic_ontouch,
};

#endif /* if TCFG_UI_ENABLE_PHONE_ACTION */
#endif /* #if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) */

