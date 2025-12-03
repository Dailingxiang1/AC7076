#include "app_config.h"
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
#include "data_storage/data_storage.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_notice.data.bss")
#pragma data_seg(".ui_action_notice.data")
#pragma const_seg(".ui_action_notice.text.const")
#pragma code_seg(".ui_action_notice.text")
#endif// SUPPORT_MS_EXTENSIONS

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-ACTION]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) && TCFG_UI_ENABLE_NOTICE

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)
extern int ui_small_file_message_get_count(void);
extern int small_file_message_read_by_index(small_file_message_t *message, int index);
extern int small_file_delete_by_index(u8 small_file_type, int index);
extern u16 ui_get_text_width_and_height(u8 encode, u8 *str, u16 strlen, u16 elm_width, u16 elm_height, u8 flags, char *value_type);
int notice_detail_control_onchange(void *_ctrl, enum element_change_event event, void *arg);
#define MESSAGE_MAX_NUM     		10          /**< 支持存储的最大消息数量 */
#define MESSAGE_APPIDENTIFIER_LEN   1   		/**< 通知消息app标识符数据长度 0:通用 1:短信 2:微信 3:QQ 4:钉钉 其他:未定义 */
/* #define MESSAGE_PACKAGENAME_LEN    	32   		[>*< 通知消息包名数据长度 <] */
/* #define MESSAGE_TITLE_LEN   		37          [>*< 通知消息标题数据长度 <] */
/* #define MESSAGE_CONTENT_LEN 		441         [>*< 通知消息内容数据长度 <] */
/* #define MESSAGE_TIMESTAMP_LEN  		4        	[>*< 通知消息时间戳数据长度 <] */

#define SIDEBAR_TEXT_TITLE0			NOTICE_LIST_ITEM_TYPE_TEXT0
#define SIDEBAR_TEXT_TITLE1     	NOTICE_LIST_ITEM_TYPE_TEXT1
#define SIDEBAR_TEXT_TITLE2     	NOTICE_LIST_ITEM_TYPE_TEXT2
#define SIDEBAR_TEXT_CONTENT0		NOTICE_LIST_ITEM_CONTEXT_TEXT0
#define SIDEBAR_TEXT_CONTENT1		NOTICE_LIST_ITEM_CONTEXT_TEXT1
#define SIDEBAR_TEXT_CONTENT2		NOTICE_LIST_ITEM_CONTEXT_TEXT2
#define	SIDEBAR_PIC_APP_ICON0		NOTICE_LIST_ITEM_TYPE_PIC0
#define	SIDEBAR_PIC_APP_ICON1		NOTICE_LIST_ITEM_TYPE_PIC1
#define	SIDEBAR_PIC_APP_ICON2		NOTICE_LIST_ITEM_TYPE_PIC2
#define SIDEBAR_NOTICE_TIME0		NOTICE_LIST_ITEM_TIME0
#define SIDEBAR_NOTICE_TIME1		NOTICE_LIST_ITEM_TIME1
#define SIDEBAR_NOTICE_TIME2		NOTICE_LIST_ITEM_TIME2

#define SIDEBAR_TEXT_CLEAR_ALL		NOTICE_LIST_CLEAR_ALL_PIC
#define	SIDEBAR_TEXT_CLEAR_CURR		NOTICE_DETAIL_CLEAR_PIC

#define	SIDEBAR_LAYOUT_MESSAGE_DETAIL 		NOTICE_DETAIL_LAYOUT
#define	SIDEBAR_TEXT_TITLE_DETAIL			NOTICE_DETAIL_TITLE_TEXT
#define SIDEBAR_TEXT_CONTENT_DETAIL			NOTICE_DETAIL_CONTEXT_TEXT
#define SIDEBAR_PIC_APP_ICON				NOTICE_DETAIL_TYPE_PIC

#define SIDEBAR_NOTICE_LIST					NOTICE_LIST
#define SIDEBAR_NOTICE_LIST_LAYOUT			NOTICE_LIST_LAYOUT
#define SIDEBAR_NOTICE_NO_MSG_LAYOUT		NOTICE_NO_MSG_LAYOUT
#define SIDEBAR_VLIST_NOTICE			    SIDEBAR_NOTICE_LIST
struct ui_message_info {
    u8 init_count: 2;			/*初始化计数值，用于特效切换时保障数据安全*/
    u8 select_index: 6;			/*消息记录*/
    small_file_message_t store_buffer[3];
};
static struct ui_message_info *__info;
static u8	create_control_by_menu = 0;
int notice_status_handler(const char *type, u32 arg)
{
    int row, col;
    struct element_css *css;
    struct element *elm;
    struct ui_grid *grid = ui_grid_for_id(SIDEBAR_VLIST_NOTICE);
    printf("%s type:%s arg:%d", __func__, type, arg);
    if (type && (!strcmp(type, "event"))) {
        switch (arg) {
        case 1: /* add */
            log_info("add message.....");
            log_info("message_num:%d",  ui_small_file_message_get_count());
            if (!grid) {
                if (ui_core_get_element_by_id(SIDEBAR_TEXT_CONTENT_DETAIL)) {
                    //如果当前处于消息详情状态，那么不做处理
                } else {
                    ui_hide(NOTICE_NO_MSG_LAYOUT);
                    ui_show(NOTICE_LIST_LAYOUT);
                    grid = ui_grid_for_id(SIDEBAR_VLIST_NOTICE);
                    grid->item[0].elm.css.top = 0;
                }
                break;
            }
            row = 1;
            col = 0;

            ui_grid_add_dynamic_by_id(SIDEBAR_VLIST_NOTICE, &row, &col, create_control_by_menu);
            break;
        case 2: /* del */
            log_info("del message.....");
            log_info("message_num:%d",  ui_small_file_message_get_count());
            if (ui_small_file_message_get_count() == 0) {
                ui_hide(NOTICE_LIST_LAYOUT);
                ui_show(NOTICE_NO_MSG_LAYOUT);
                break;
            }
            row = 1;
            col = 0;
            ui_grid_del_dynamic_by_id(SIDEBAR_VLIST_NOTICE, &row, &col, create_control_by_menu);
            break;
        case 3:
            if (grid) {
                log_info("redraw grid:%d.....", grid->avail_item_num);
                for (int index = 0; index < grid->avail_item_num; index++) {
                    list_for_each_child_element(elm, &grid->item[index].elm) {
                        if (elm->handler && elm->handler->onchange) {
                            void *p = (void *)index;
                            elm->handler->onchange(elm, ON_CHANGE_UPDATE_ITEM, p);
                        }
                    }
                }
            }
            break;
        }
    }

    return 0;
}


static const struct uimsg_handl ui_menu_notice_msg_handler[] = {
    { "message_status",   notice_status_handler     },
    { NULL, NULL},      /* 必须以此结尾！ */
};
/* ------------------------------------------------------------------------------------*/
/**
 * @brief create_control_by_menu_get
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
u8 create_control_by_menu_get()
{
    return create_control_by_menu;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief create_control_by_menu_set
 *
 * @param en
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
u8 create_control_by_menu_set(u8 en)
{
    create_control_by_menu = !!en;
    return create_control_by_menu;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_message_info_create
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_message_info_create()
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    printf("<%s>  rets:%x\n", __func__,  rets);
    if (!__info) {
        __info = zalloc(sizeof(struct ui_message_info));
    }
    __info->init_count++;
    ASSERT(__info->init_count);/*防溢出*/
    return __info->init_count;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_message_info_release
 *
 * @return true 释放内存
 */
/* ------------------------------------------------------------------------------------*/
int ui_message_info_release()
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    printf("<%s> rets:%x\n", __func__, rets);
    if (!__info) {
        return false;
    } else {
        __info->init_count--;
        if (!__info->init_count) {
            free(__info);
            __info = NULL;
            return true;
        }
    }
    return false;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief message_data_analysis
 *
 * @param index				message index;
 * @param store_buf_index	storage  index;
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
u8 message_data_analysis(int index, u8 store_buf_index)
{
    if (!__info) {
        log_error("<%s>ui message info not init\n", __func__);
        return -1;
    }
    if (index >= ui_small_file_message_get_count()) {
        log_error("<%s>index over.index:%d total:%d\n", __func__, index, ui_small_file_message_get_count());
        return -2;
    }
    /*倒叙读取*/
    int read_index = ui_small_file_message_get_count() - 1 - index;
    /* printf("%s index:%d store:%d read:%d", __func__, index, store_buf_index, read_index); */
    small_file_message_t *temp_message_buf;
    temp_message_buf = &__info->store_buffer[store_buf_index];
    memset(temp_message_buf, 0, sizeof(small_file_message_t));
    int ret =  small_file_message_read_by_index(temp_message_buf, read_index);
    if (ret == false) {
        log_error("<%s> read error index:%d read_index:%d\n", __func__, index, read_index);
    }
    return 0;
}

static int vlist_notice_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct scroll_area area = {0, 0, 10000, 10000};
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_on_focus(grid);
        if (ui_small_file_message_get_count() == 0) {
            row = 0;
            col = 0;
        } else {
            row = ui_small_file_message_get_count() ;
            col = 1;
        }
        if (create_control_by_menu_get()) {
            ui_set_default_handler(&grid->elm, NULL, NULL, notice_detail_control_onchange);
        }
        ui_grid_init_dynamic(grid, &row, &col);
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);

        ui_grid_set_energy_target_line(grid, 74);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE1);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        break;
    }
    return 0;
}

static int vlist_notice_ontouch(void *ctrl, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }
        int sel_item = ui_grid_cur_item_dynamic(grid);
        if (sel_item >= 0) {
            __info->select_index = sel_item;
            message_data_analysis(__info->select_index, 0);
        }

        ui_hide(NOTICE_LIST_LAYOUT);
        ui_show(NOTICE_DETAIL_LAYOUT);
        if (!create_control_by_menu) {
            ui_card_disable();
        }
        break;
    }
    return false;
}
u8 is_ui_sidebar_show_top();
u8 is_ui_sidebar_show_bottom();
u8 is_ui_page_move_enable(void);
static int vlist_notice_user_onkey(void *ctrl, struct element_key_event *event)
{
    /* printf("%s %d",__func__,__LINE__); */
    struct rect r;
    struct element *elm = (struct element *)ctrl;
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
    int total_item_height  = grid->y_interval + r.height;
    printf("%s total_item_height:%d", __func__, total_item_height);
    int max_cnt = 4;
    switch (event->value) {
    case KEY_UI_MINUS:

        for (int i = 0; i < max_cnt; i++) {
            int single_step  = total_item_height / max_cnt;
            int step = (i == max_cnt - 1) ? (total_item_height - i * single_step) : single_step;
            ui_grid_slide(grid, SCROLL_DIRECTION_UD, step);
            os_time_dly(2);
        }

        break;
    case KEY_UI_PLUS:
        for (int i = 0; i < max_cnt; i++) {
            int single_step  = total_item_height / max_cnt;
            int step = (i == max_cnt - 1) ? (total_item_height - i * single_step) : single_step;
            step *= -1;
            ui_grid_slide(grid, SCROLL_DIRECTION_UD, step);
            os_time_dly(2);
        }

        break;
    }
    return false;
}
static int vlist_notice_onkey(void *ctrl, struct element_key_event *event)
{
    printf("%s %d %d ", __func__, __LINE__, event->value);
    if (is_ui_sidebar_show_top() || create_control_by_menu) {
        vlist_notice_user_onkey(ctrl, event);
        //发给底层
        /* return false; */
    }
    if (is_ui_sidebar_show_bottom()) {
        struct element *element = ui_core_get_element_by_id(PULLUP_MENU_LAYOUT);
        if (element && element->handler && element->handler->onkey) {
            element->handler->onkey(element, event);
        }
    }
    if (is_ui_page_move_enable()) {
        /* printf("%s %d",__func__,__LINE__); */
        struct element *element = ui_core_get_element_by_id(DIAL_WATCH);
        if (element && element->handler && element->handler->onkey) {
            element->handler->onkey(element, event);
        }
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(SIDEBAR_NOTICE_LIST)
.onchange = vlist_notice_onchange,
 .onkey =  vlist_notice_onkey,
  .ontouch = vlist_notice_ontouch,
};

int notice_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        ui_message_info_create();
        ui_register_msg_handler(ui_get_current_window_id(), ui_menu_notice_msg_handler);//注册消息交互的回调
        break;
    case ON_CHANGE_RELEASE:
        ui_message_info_release();
        break;
    default:
        break;
    }
    return 0;
}
static int notice_child_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        if (elm->id == NOTICE_LIST_LAYOUT) {
            if (ui_small_file_message_get_count()) {
                elm->css.invisible = 0;
            } else {
                elm->css.invisible = 1;
            }
        } else if (elm->id == NOTICE_NO_MSG_LAYOUT) {
            if (!ui_small_file_message_get_count()) {
                elm->css.invisible = 0;
            } else {
                elm->css.invisible = 1;
            }
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_LAYOUT)
.onchange =  notice_child_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(NOTICE_NO_MSG_LAYOUT)
.onchange =  notice_child_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




static int message_detail_layout_ontouch(void *ctrl, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        struct rect rect = {0};
        ui_core_get_element_abs_rect((struct element *)ctrl, &rect);
        if (rect.top < -(rect.height / 2)) {
            return true;
        }
        ui_hide(NOTICE_DETAIL_LAYOUT);
        ui_show(NOTICE_LIST_LAYOUT);
        if (!create_control_by_menu) {
            ui_card_enable();
        }
        return true;
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SIDEBAR_LAYOUT_MESSAGE_DETAIL)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch = message_detail_layout_ontouch,
};
//===============================================================================================================//
static int __int2time(int int_t, struct sys_time *time)
{
    u32 t ;
    t = ((int_t & 0xff000000) >> 24) | \
        ((int_t & 0x00ff0000) >> 8) | \
        ((int_t & 0x0000ff00) << 8) | \
        ((int_t & 0x000000ff) << 24);
    time->sec = t & 0x3f;
    time->min = (t >> 6) & 0x3f;
    time->hour = (t >> 12) & 0x1f;
    time->day = (t >> 17) & 0x1f;
    time->month = (t >> 22) & 0xf;
    time->year = ((t >> 26) & 0x3f) + 2010;
    /* printf("time : %d-%d-%d,%d:%d:%d\n", time->year, time->month, time->day, time->hour, time->min, time->sec); */
    return 0;
}
int notice_detail_layout_control_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    if (event != ON_CHANGE_SHOW_PROBE) {
        return false;
    }
    struct element *elm = (struct element *)_ctrl;
    u8 type = ui_id2type(elm->id);
    int index = (int)arg;
    switch (type) {
    case CTRL_TYPE_PIC: {
        struct ui_pic *pic = (struct ui_pic *)elm;
        const char target[] = "pic";

        printf("%s %d ", __func__, __LINE__);
        if (strstr(pic->source, target)) {

            printf("%s %d", __func__, __LINE__);
            int sel = pic->source[strlen(target)] - '0';
            /* message_data_analysis(index, sel);			//在最底层的控件load数据 */
            int app_index = __info->store_buffer[sel].app_identifier;
            ui_pic_set_image_index(pic, app_index);
        }
    }
    break;
    case CTRL_TYPE_TIME: {
        struct ui_time *time = (struct ui_time *)elm;
        const char target[] = "time";
        if (strstr(time->source, target)) {
            int sel = time->source[strlen(target)] - '0';
            u32 timestamp = __info->store_buffer[sel].timestamp;
            printf("%s %d", __func__, timestamp);
            struct sys_time ntime;
            __int2time(timestamp, &ntime);
            ui_time_update(time, (struct utime *)&ntime);
        }
    }
    break;
    case CTRL_TYPE_TEXT: {
        struct ui_text *text = (struct ui_text *)elm;
        const char target0[] = "title";
        const char target1[] = "text";
        printf("%s %d", __func__, __LINE__);
        if (strstr(text->source, target0)) {

            int sel = text->source[strlen(target0)] - '0';

            printf("%s %d sel:%d ", __func__, __LINE__, sel);
            /* put_buf(__info->store_buffer[sel].title, */
            /* strlen((const char *)__info->store_buffer[sel].title)); */

            ui_text_set_text_attrs(
                text,
                (const char *)__info->store_buffer[sel].title,
                strlen((const char *)__info->store_buffer[sel].title),
                FONT_ENCODE_UTF8,
                1,
                FONT_DEFAULT | FONT_SHOW_SCROLL
            );
        } else if (strstr(text->source, target1)) {

            int sel = text->source[strlen(target1)] - '0';
            printf("%s %d sel:%d ", __func__, __LINE__, sel);
            /* put_buf(__info->store_buffer[sel].title, */
            /* strlen((const char *)__info->store_buffer[sel].title)); */
            ui_text_set_text_attrs(
                text,
                (const char *)__info->store_buffer[sel].content,
                strlen((const char *)__info->store_buffer[sel].content),
                FONT_ENCODE_UTF8,
                1,
                FONT_DEFAULT | FONT_SHOW_MULTI_LINE
            );
        }
    }
    break;
    }
    return FALSE;
}

int notice_detail_control_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    if (event != ON_CHANGE_UPDATE_ITEM) {
        return false;
    }
    struct element *elm = (struct element *)_ctrl;
    u8 type = ui_id2type(elm->id);
    int index = (int)arg;
    /* printf("%s type:%d index:%d inv:%d prior:%d elm:0x%x", __func__, type, index, elm->css.invisible, elm->prior, elm->id); */
    switch (type) {
    case CTRL_TYPE_PIC: {
        struct ui_pic *pic = (struct ui_pic *)elm;
        const char target[] = "pic";

        if (strstr(pic->source, target)) {
            int sel = pic->source[strlen(target)] - '0';
            message_data_analysis(index, sel);			//在最底层的控件load数据
            int app_index = __info->store_buffer[sel].app_identifier;
            ui_pic_set_image_index(pic, app_index);
        }
    }
    break;
    case CTRL_TYPE_TIME: {
        struct ui_time *time = (struct ui_time *)elm;
        const char target[] = "time";
        if (strstr(time->source, target)) {
            int sel = time->source[strlen(target)] - '0';
            u32 timestamp = __info->store_buffer[sel].timestamp;
            printf("%s TimeStamp %x", __func__, timestamp);
            struct sys_time ntime;
            __int2time(timestamp, &ntime);
            ui_time_update(time, (struct utime *)&ntime);
        }
    }
    break;
    case CTRL_TYPE_TEXT: {
        struct ui_text *text = (struct ui_text *)elm;
        const char target0[] = "title";
        const char target1[] = "text";
        if (strstr(text->source, target0)) {

            int sel = text->source[strlen(target0)] - '0';
            /* put_buf(__info->store_buffer[sel].title, */
            /* strlen((const char *)__info->store_buffer[sel].title)); */
            ui_text_set_text_attrs(
                text,
                (const char *)__info->store_buffer[sel].title,
                strlen((const char *)__info->store_buffer[sel].title),
                FONT_ENCODE_UTF8,
                1,
                FONT_DEFAULT | FONT_SHOW_SCROLL
            );
        } else if (strstr(text->source, target1)) {

            int sel = text->source[strlen(target1)] - '0';
            /* put_buf(__info->store_buffer[sel].title, */
            /* strlen((const char *)__info->store_buffer[sel].title)); */
            ui_text_set_text_attrs(
                text,
                (const char *)__info->store_buffer[sel].content,
                strlen((const char *)__info->store_buffer[sel].content),
                FONT_ENCODE_UTF8,
                1,
                FONT_DEFAULT | FONT_SHOW_MULTI_LINE
            );
        }
    }
    break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SIDEBAR_TEXT_TITLE_DETAIL)
.onchange =  notice_detail_layout_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_TEXT_CONTENT_DETAIL)
.onchange =  notice_detail_layout_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_PIC_APP_ICON)
.onchange =  notice_detail_layout_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#if 0
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_TYPE_TEXT0)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_TYPE_PIC0)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_CONTENT_TEXT0)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_TIME0)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_TYPE_TEXT1)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_TYPE_PIC1)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_CONTENT_TEXT1)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_TIME1)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_TYPE_TEXT2)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_TYPE_PIC2)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_CONTENT_TEXT2)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(NOTICE_LIST_ITEM_TIME2)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif
//===============================================================================================================//
static int text_clear_ontouch(void *ctrl, struct element_touch_event *e)
{
    struct element *elm  = (struct element *)ctrl;

    struct ui_watch *watch  = (struct ui_watch *)ui_core_get_element_by_id(STYLE_DIAL_ID(WATCH));
    if (e->event == ELM_EVENT_TOUCH_DOWN || e->event == ELM_EVENT_TOUCH_MOVE || e->event == ELM_EVENT_TOUCH_UP) {
        if (watch && watch->handler && watch->handler->ontouch) {
            watch->handler->ontouch(watch, e);
        }
    }
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return 0;
        }
        if (elm->id == SIDEBAR_TEXT_CLEAR_ALL) {
            ui_hide(NOTICE_LIST_LAYOUT);
            while (ui_small_file_message_get_count()) {
                small_file_delete_by_index(F_TYPE_MESSAGE, 0);
            }
            ui_show(NOTICE_NO_MSG_LAYOUT);
        } else {
            ui_hide(NOTICE_DETAIL_LAYOUT);
            int del_index = ui_small_file_message_get_count() - 1 - __info->select_index;
            ASSERT(del_index >= 0, "total:%d del:%d sel:%d", ui_small_file_message_get_count(), del_index, __info->select_index);
            small_file_delete_by_index(F_TYPE_MESSAGE, del_index);
            if (ui_small_file_message_get_count()) {
                ui_show(NOTICE_LIST_LAYOUT);
            } else {
                ui_show(NOTICE_NO_MSG_LAYOUT);
            }
            if (!create_control_by_menu) {
                ui_card_enable();
            }
        }
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_MOVE:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SIDEBAR_TEXT_CLEAR_ALL)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = text_clear_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_TEXT_CLEAR_CURR)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = text_clear_ontouch,
};

#else

__attribute__((weak)) int notice_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    return 0;
}
__attribute__((weak)) int notice_detail_control_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    return 0;
}
__attribute__((weak)) u8 create_control_by_menu_get()
{
    return 0;
}
__attribute__((weak)) u8 create_control_by_menu_set(u8 en)
{
    return 0;
}

#endif
