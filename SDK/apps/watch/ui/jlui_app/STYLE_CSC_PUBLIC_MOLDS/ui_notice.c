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

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_MSG_NOTICE

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)


// 获取信息条数
extern int ui_small_file_message_get_count(void);

// 通过索引获取信息内容
extern int small_file_message_read_by_index(small_file_message_t *message, int index);

// 通过索引删除信息内容
extern int small_file_delete_by_index(u8 small_file_type, int index);

// 获取文本宽、高
extern u16 ui_get_text_width_and_height(u8 encode, u8 *str, u16 strlen, u16 elm_width, u16 elm_height, u8 flags, char *value_type);

int notice_detail_control_onchange(void *_ctrl, enum element_change_event event, void *arg);
void notice_dialog_show();
void notice_dialog_hide(int anim);


/* 消息缓存buf数量，与列表项数量一致 */
#define MESSAGE_STORE_BUF_NUM       3

/* 消息弹窗显示时间，超过这个时间会自动隐藏(ms) */
#define DIALOG_SHOW_TIME_MS         2000

/* 消息弹窗弹出、隐藏动画时间 */
#define DIALOG_ANIM_RUN_TIME        150

/* 消息列表各项内控件ID，需要跟随滑动更新消息内容 */
// 图标
#define	SIDEBAR_PIC_APP_ICON0		NOTICE_LIST_ITEM_TYPE_PIC0
#define	SIDEBAR_PIC_APP_ICON1		NOTICE_LIST_ITEM_TYPE_PIC1
#define	SIDEBAR_PIC_APP_ICON2		NOTICE_LIST_ITEM_TYPE_PIC2
// 标题
#define SIDEBAR_TEXT_TITLE0			NOTICE_LIST_ITEM_TYPE_TEXT0
#define SIDEBAR_TEXT_TITLE1     	NOTICE_LIST_ITEM_TYPE_TEXT1
#define SIDEBAR_TEXT_TITLE2     	NOTICE_LIST_ITEM_TYPE_TEXT2
// 消息
#define SIDEBAR_TEXT_CONTENT0		NOTICE_LIST_ITEM_CONTENT_TEXT0
#define SIDEBAR_TEXT_CONTENT1		NOTICE_LIST_ITEM_CONTENT_TEXT1
#define SIDEBAR_TEXT_CONTENT2		NOTICE_LIST_ITEM_CONTENT_TEXT2
// 时间
#define SIDEBAR_NOTICE_TIME0		NOTICE_LIST_ITEM_TIME0
#define SIDEBAR_NOTICE_TIME1		NOTICE_LIST_ITEM_TIME1
#define SIDEBAR_NOTICE_TIME2		NOTICE_LIST_ITEM_TIME2

// 删除图标ID
#define SIDEBAR_TEXT_CLEAR_ALL		NOTICE_LIST_CLEAR_ALL_PIC
#define	SIDEBAR_TEXT_CLEAR_CURR		NOTICE_DETAIL_CLEAR_PIC

// 消息详情页面各控件ID
#define	SIDEBAR_LAYOUT_MESSAGE_DETAIL 		NOTICE_DETAIL_LAYOUT            // 详情布局
#define	SIDEBAR_TEXT_TITLE_DETAIL			NOTICE_DETAIL_TITLE_TEXT        // 详情标题
#define SIDEBAR_TEXT_CONTENT_DETAIL			NOTICE_DETAIL_CONTEXT_TEXT      // 详情内容
#define SIDEBAR_PIC_APP_ICON				NOTICE_DETAIL_TYPE_PIC          // 详情图标

// 弹窗页面各控件ID
#define SIDEBAR_DIALOG_APP_ICON             NOTICE_DIALOG_PIC
#define SIDEBAR_DIALOG_TITLE                NOTICE_DIALOG_TITLE
#define SIDEBAR_DIALOG_CONTENT              NOTICE_DIALOG_CONTENT
#define SIDEBAR_DIALOG_TIME                 NOTICE_DIALOG_TIME

// 各布局ID
#define SIDEBAR_NOTICE_MAIN_LAYOUT          NOTICE_LAYOUT                   // 消息界面主布局
#define SIDEBAR_NOTICE_LIST_LAYOUT			NOTICE_LIST_LAYOUT              // 有消息布局
#define SIDEBAR_NOTICE_NO_MSG_LAYOUT		NOTICE_NO_MSG_LAYOUT            // 无消息布局
#define SIDEBAR_DIALOG_LAYOUT               NOTICE_DIALOG                   // 新消息弹窗布局
#define SIDEBAR_NOTICE_LIST					NOTICE_LIST                     // 消息列表


// 消息内容缓存，有消息布局显示时创建，隐藏时释放
struct ui_message_info {
    u8 init_count;              /*初始化计数值，用于特效切换时保障数据安全*/
    u8 select_index;			/*消息记录*/
    u8 in_bottom;               /*在最底下标志*/
    u8 mem_item;                /*记忆项数，进入消息详情后返回时，用于记忆位置*/
    u8 in_dialog;               /*是否在弹窗中标志*/
    u8 from_dialog;             /*消息详情界面是否从弹窗跳进去*/
    int dialog_timeout;         /*超时定时器ID*/
    ui_anim_t dialog_anim;      /*弹窗动画*/
    small_file_message_t store_buffer[MESSAGE_STORE_BUF_NUM];
};
static struct ui_message_info *__info;

// 从菜单进入标志，从菜单页面进入时为true，从表盘界面进入为false
static u8	create_control_by_menu = 0;

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
    //printf("<%s>  rets:%x\n", __func__,  rets);
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
    //printf("<%s> rets:%x\n", __func__, rets);
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
    // printf("@@@@@ %s(), index: %d, store_buf_index: %d\n", __func__, index, store_buf_index);
    if (!__info) {
        /* 如果 __info 指针未初始化，退出 */
        log_error("<%s>ui message info not init\n", __func__);
        return -1;
    }
    if (index >= ui_small_file_message_get_count()) {
        /* 如果传入信息索引大于 flash 存储的信息条数，退出 */
        log_error("<%s>index over.index:%d total:%d\n", __func__, index, ui_small_file_message_get_count());
        return -2;
    }

    /* 倒序读取 */
    int read_index = ui_small_file_message_get_count() - 1 - index;
    // printf("@@@ %s() index:%d store:%d read:%d", __func__, index, store_buf_index, read_index);

    /* 将指定索引的信息读到 store_buf 中缓存，用于UI显示 */
    small_file_message_t *temp_message_buf;
    temp_message_buf = &__info->store_buffer[store_buf_index];
    memset(temp_message_buf, 0, sizeof(small_file_message_t));
    int ret =  small_file_message_read_by_index(temp_message_buf, read_index);
    if (ret == false) {
        /* 读取信息失败 */
        log_error("<%s> read error index:%d read_index:%d\n", __func__, index, read_index);
    }
    return 0;
}

/* 消息列表初始化 */
static int vlist_notice_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    // printf("@@@@@ %s(), event: %d\n", __func__, event);
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
            row = ui_small_file_message_get_count();
            col = 1;
        }
        if (create_control_by_menu_get()) {
            ui_set_default_handler(&grid->elm, NULL, NULL, notice_detail_control_onchange);
        }
        // printf("@@@@@@ row: %d, col: %d\n", row, col);
        ui_grid_init_dynamic(grid, &row, &col);
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);

        ui_grid_set_energy_target_line(grid, 74);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE1);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*第一次显示时，把列表项拉到0位置*/
        if (grid->item) {
            struct element *item = &grid->item[0].elm;
            struct rect item_rect;
            ui_core_get_element_abs_rect(item, &item_rect);
            int item_h = item_rect.height + grid->y_interval;
            int msg_number = ui_small_file_message_get_count();

            for (int i = 0; i < grid->avail_item_num; i++) {
                item = &grid->item[i].elm;
                item->css.top = i * item_h;
                if (i >= msg_number) {
                    item->css.invisible = true;
                }
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

static int vlist_notice_ontouch(void *ctrl, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    //printf("@@@ %s(), event: %d\n", __func__, e->event);

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        /* 如果已经到列表最底下了，继续向上滑动时需隐藏列表 */
        if (ui_grid_cur_item_dynamic(grid) == grid->dynamic->max_row_index) {
            __info->in_bottom = 0x0f;
        }
        break;
    case ELM_EVENT_TOUCH_MOVE:
        if (((__info->in_bottom == 0x0f) && (e->yoffset < 0)) || (__info->in_bottom == 0xff)) {
            if (__info->in_bottom == 0x0f) {
                e->event = ELM_EVENT_TOUCH_DOWN;
                __info->in_bottom |= 0xf0;
            }
            /* 继续向上滑动时隐藏列表，把消息发给表盘界面处理即可，利用表盘界面隐藏下拉菜单的逻辑 */
            struct element *element = ui_core_get_element_by_id(STYLE_DIAL_ID(WATCH));
            if (element && element->handler && element->handler->ontouch) {
                element->handler->ontouch(element, e);
                return true;
            }
        }
        break;
    case ELM_EVENT_TOUCH_UP:
        if (__info->in_bottom == 0xff) {
            /* 如果是隐藏动作，需要把 touch up 事件也发给表盘处理，才会自动处理回弹、惯性等动作 */
            struct element *element = ui_core_get_element_by_id(STYLE_DIAL_ID(WATCH));
            if (element && element->handler && element->handler->ontouch) {
                element->handler->ontouch(element, e);
            }
        }
        __info->in_bottom = 0;
        if (e->move_flag) {
            break;
        }

        /* 获取被点击的消息索引，并将该消息放到 store_buf 的 0 位置 */
        int sel_item = ui_grid_cur_item_dynamic(grid);
        // printf("@@@ %s(), sel_item: %d\n", __func__, sel_item);
        if (sel_item >= 0) {
            __info->select_index = sel_item;
            message_data_analysis(__info->select_index, 0);
        }

        /* 隐藏消息列表页面，显示消息详情页面 */
        ui_hide(SIDEBAR_NOTICE_LIST_LAYOUT);
        ui_show(SIDEBAR_LAYOUT_MESSAGE_DETAIL);

        /* 这里不是从菜单页面进入消息详情，因此关闭卡片使能，禁止右划返回上一个页面 */
        if (!create_control_by_menu_get()) {
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
    log_info("%s total_item_height:%d", __func__, total_item_height);
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
    log_info("%s %d %d ", __func__, __LINE__, event->value);
    if (is_ui_sidebar_show_top() || create_control_by_menu) {
        vlist_notice_user_onkey(ctrl, event);
        //发给底层
        /* return false; */
    }
    // if (is_ui_sidebar_show_bottom()) {
    //     struct element *element = ui_core_get_element_by_id(PULLUP_MENU_LAYOUT);
    //     if (element && element->handler && element->handler->onkey) {
    //         element->handler->onkey(element, event);
    //     }
    // }
    return true;
}
REGISTER_UI_EVENT_HANDLER(SIDEBAR_NOTICE_LIST)
.onchange = vlist_notice_onchange,
 .onkey =  NULL,//vlist_notice_onkey,
  .ontouch = vlist_notice_ontouch,
};

// 消息界面初始化回调，这个函数在表盘文件调用
int notice_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        ui_message_info_create();
        break;
    case ON_CHANGE_SHOW:
        struct draw_context *dc = (struct draw_context *)arg;
        if (__info->in_dialog) {
            dc->alpha = 100;
        } else {
            dc->alpha = 0;
        }
        break;
    case ON_CHANGE_RELEASE:
        ui_message_info_release();
        break;
    default:
        break;
    }
    return 0;
}

/* 布局初始化，如果有消息就显示有消息界面，否则显示无消息界面 */
static int notice_child_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        if (elm->id == SIDEBAR_NOTICE_LIST_LAYOUT) {
            if (ui_small_file_message_get_count()) {
                elm->css.invisible = 0;
            } else {
                elm->css.invisible = 1;
            }
        } else if (elm->id == SIDEBAR_NOTICE_NO_MSG_LAYOUT) {
            if (!ui_small_file_message_get_count()) {
                elm->css.invisible = 0;
            } else {
                elm->css.invisible = 1;
            }
        } else if (elm->id == SIDEBAR_DIALOG_LAYOUT) {
            elm->css.invisible = true;
        }
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SIDEBAR_NOTICE_LIST_LAYOUT)
.onchange =  notice_child_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_NOTICE_NO_MSG_LAYOUT)
.onchange =  notice_child_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_DIALOG_LAYOUT)
.onchange =  notice_child_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/* 消息详情布局触摸回调 */
static int message_detail_layout_ontouch(void *ctrl, struct element_touch_event *e)
{
    //printf("@@@ %s(), event: %d\n", __func__, e->event);
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_R_MOVE:
        struct rect rect = {0};
        ui_core_get_element_abs_rect((struct element *)ctrl, &rect);
        if (rect.top < -(rect.height / 2)) {
            return true;
        }

        /*从弹窗进来，右划后隐藏消息窗口*/
        if (__info->from_dialog) {
            struct element *main_elm = ui_core_get_element_by_id(SIDEBAR_NOTICE_MAIN_LAYOUT);
            if (main_elm) {
                main_elm->css.top = -main_elm->css.height;
                ui_core_redraw(main_elm);
            }
            __info->from_dialog = false;
        }

        ui_hide(SIDEBAR_LAYOUT_MESSAGE_DETAIL);
        ui_show(SIDEBAR_NOTICE_LIST_LAYOUT);
        if (!create_control_by_menu_get()) {
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

/* 更新消息列表显示的内容 */
static void notice_update_vlist_content(struct element *elm, int index)
{
    //printf("@@@@@ %s(), %d, index: %d, id: 0x%x\n", __func__, __LINE__, index, elm->id);
    ASSERT(elm, "notice vlist control elm is NULL!");

    /*
    注意：
        这里使用控件的数据源进行区分，UI工程中消息列表各项的控件数据源应该与代码设计保持一致。
        如：图标控件数据源为 pic0, pic1, pic2, ...；时间控件数据源为 time0, time1, time2, ...
    */

    u8 type = ui_id2type(elm->id);

    switch (type) {
    case CTRL_TYPE_PIC: {
        struct ui_pic *pic = (struct ui_pic *)elm;
        const char target[] = "pic";
        if (strstr(pic->source, target)) {
            // 根据数据源判断显示哪个 store_buf 缓存中的图片索引
            int sel = pic->source[strlen(target)] - '0';
            ASSERT(sel < MESSAGE_STORE_BUF_NUM);    // 这里防止UI工程设计错误，数据源给错的话，计算出来超过store_buf缓存数量，会导致访问内存越界
            if (pic->elm.id != SIDEBAR_PIC_APP_ICON) {
                message_data_analysis(index, sel);      // 更新store_buf缓存的消息内容
            }
            int app_index = __info->store_buffer[sel].app_identifier;
            // printf("app_index: %d\n", app_index);
            ui_pic_set_image_index(pic, app_index);
        }
    }
    break;
    case CTRL_TYPE_TIME: {
        struct ui_time *time = (struct ui_time *)elm;
        const char target[] = "time";
        if (strstr(time->source, target)) {
            // 根据数据源判断显示哪个 store_buf 缓存中的时间
            int sel = time->source[strlen(target)] - '0';
            ASSERT(sel < MESSAGE_STORE_BUF_NUM);    // 这里防止UI工程设计错误，数据源给错的话，计算出来超过store_buf缓存数量，会导致访问内存越界
            u32 timestamp = __info->store_buffer[sel].timestamp;
            struct sys_time ntime;
            __int2time(timestamp, &ntime);
            // printf("time: %d-%d-%d %d:%d:%d\n", ntime.year, ntime.month, ntime.day, ntime.hour, ntime.min, ntime.sec);
            ui_time_update(time, (struct utime *)&ntime);
        }
    }
    break;
    case CTRL_TYPE_TEXT: {
        struct ui_text *text = (struct ui_text *)elm;
        const char target0[] = "title";
        const char target1[] = "text";
        if (strstr(text->source, target0)) {
            // 根据数据源判断显示哪个 store_buf 缓存中的标题
            int sel = text->source[strlen(target0)] - '0';
            ASSERT(sel < MESSAGE_STORE_BUF_NUM);    // 这里防止UI工程设计错误，数据源给错的话，计算出来超过store_buf缓存数量，会导致访问内存越界
            // printf("title: %s\n", __info->store_buffer[sel].title);
            ui_text_set_text_attrs(
                text,
                (const char *)__info->store_buffer[sel].title,
                strlen((const char *)__info->store_buffer[sel].title),
                FONT_ENCODE_UTF8,
                1,
                FONT_DEFAULT | FONT_SHOW_SCROLL
            );
        } else if (strstr(text->source, target1)) {
            // 根据数据源判断显示哪个 store_buf 缓存中的消息内容
            int sel = text->source[strlen(target1)] - '0';
            ASSERT(sel < MESSAGE_STORE_BUF_NUM);    // 这里防止UI工程设计错误，数据源给错的话，计算出来超过store_buf缓存数量，会导致访问内存越界
            // printf("text: %s\n", __info->store_buffer[sel].content);
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
}

int notice_detail_layout_control_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    if (event != ON_CHANGE_SHOW_PROBE) {
        return false;
    }
    notice_update_vlist_content((struct element *)_ctrl, (int)arg);
    return FALSE;
}

/* 消息详情各控件 */
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

/* 消息弹窗各控件 */
REGISTER_UI_EVENT_HANDLER(SIDEBAR_DIALOG_APP_ICON)
.onchange =  notice_detail_layout_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_DIALOG_TITLE)
.onchange =  notice_detail_layout_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_DIALOG_CONTENT)
.onchange =  notice_detail_layout_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_DIALOG_TIME)
.onchange =  notice_detail_layout_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/* 更新消息内容 */
int notice_detail_control_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    if (event != ON_CHANGE_UPDATE_ITEM) {
        return false;
    }
    // printf("@@@@@ %s(), %d, arg: %d\n", __func__, __LINE__, arg);

    notice_update_vlist_content((struct element *)_ctrl, (int)arg);
    return false;
}
REGISTER_UI_EVENT_HANDLER(SIDEBAR_PIC_APP_ICON0)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_PIC_APP_ICON1)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_PIC_APP_ICON2)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_TEXT_TITLE0)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_TEXT_TITLE1)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_TEXT_TITLE2)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_TEXT_CONTENT0)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_TEXT_CONTENT1)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_TEXT_CONTENT2)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_NOTICE_TIME0)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_NOTICE_TIME1)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_NOTICE_TIME2)
.onchange =  notice_detail_control_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

// 放到ui_action_dial.c文件注册为系统事件回调
int notice_message_status_handler(const char *type, u32 arg)
{
    // printf("@@@ %s(), type: %s, arg: %d\n", __func__, type, arg);
    struct ui_grid *grid = (struct ui_grid *)ui_core_get_element_by_id(SIDEBAR_NOTICE_LIST);

    /* 滚动到第 0 项 */
    if (grid) {
        struct element *elm = &grid->item[0].elm;
        struct rect item_rect;
        ui_core_get_element_abs_rect(elm, &item_rect);
        int item_h = item_rect.height + grid->y_interval;

        for (int item = 0; item < grid->avail_item_num; item++) {
            elm = &grid->item[item].elm;
            elm->css.top = item * item_h;
        }

        int cur_index = ui_grid_cur_item_dynamic(grid);
        int move_step = item_h * cur_index;
        // printf("item h: %d, cur_index: %d, move_step: %d\n", item_h, cur_index, move_step);
        ui_grid_slide(grid, SCROLL_DIRECTION_UD, move_step);
        ui_core_redraw(grid);
    }

    /* 响应事件 */
    if (type && (!strcmp(type, "event"))) {
        int row = 1;
        int col = 0;
        switch (arg) {
        case 1: /* add */
            if (ui_small_file_message_get_count() == 1) {       // 原来无消息，现在增加到一条消息
                ui_hide(SIDEBAR_NOTICE_NO_MSG_LAYOUT);
                ui_show(SIDEBAR_NOTICE_LIST_LAYOUT);
            } else {
                if (grid) {
                    ui_grid_add_dynamic(grid, &row, &col, true);
                }
            }
            break;
        case 2: /* del */
            if (ui_small_file_message_get_count() > 0) {
                if (grid) {
                    ui_grid_del_dynamic(grid, &row, &col, true);
                }
            } else if (ui_small_file_message_get_count() == 0) {    // 删到0条消息时，跳转到无消息界面
                ui_hide(SIDEBAR_NOTICE_LIST_LAYOUT);
                ui_show(SIDEBAR_NOTICE_NO_MSG_LAYOUT);
            }
            break;
        case 3: /* reset grid */
            if (grid) {
                for (int i = 0; i < grid->avail_item_num - 1; i++) {
                    ui_grid_update_by_id_dynamic(SIDEBAR_NOTICE_LIST, i, false);
                }
                ui_grid_update_by_id_dynamic(SIDEBAR_NOTICE_LIST, grid->avail_item_num - 1, true);  // 最后一项刷新，避免过度刷新
            }
            break;
        }
    }
    return 0;
}

//===============================================================================================================//
// SIDEBAR_NOTICE_MAIN_LAYOUT
static void notice_dialog_anim_cb(int var, int v)
{
    struct element *main_elm = ui_core_get_element_by_id(SIDEBAR_NOTICE_MAIN_LAYOUT);
    if (main_elm) {
        main_elm->css.top = v;
        // ui_core_redraw(main_elm);
    }
}

static void dialog_timeout_cb(void *p)
{
    __info->dialog_timeout = 0;
    notice_dialog_hide(true);
}

static void notice_dialog_anim_ready(struct _ui_anim_t *p)
{
    if (__info->in_dialog && (!__info->dialog_timeout)) {
        __info->dialog_timeout = sys_timeout_add(NULL, dialog_timeout_cb, DIALOG_SHOW_TIME_MS);
    } else if (!__info->in_dialog) {
        /*删除动画*/
        ui_anim_del(SIDEBAR_DIALOG_LAYOUT, NULL);

        /* 隐藏掉弹窗布局 */
        ui_hide(SIDEBAR_DIALOG_LAYOUT);

        /* 如果有消息，显示消息列表布局，否则显示无消息布局 */
        if (ui_small_file_message_get_count()) {
            ui_show(SIDEBAR_NOTICE_LIST_LAYOUT);
        } else {
            ui_show(SIDEBAR_NOTICE_NO_MSG_LAYOUT);
        }
    }
}

/* 消息弹窗显示 */
void notice_dialog_show()
{
    struct element *main_elm = ui_core_get_element_by_id(SIDEBAR_NOTICE_MAIN_LAYOUT);
    if (main_elm) {
        struct rect main_rect, dialog_rect;
        ui_core_get_element_abs_rect(main_elm, &main_rect);

        if (main_rect.top == -main_rect.height) {
            ui_hide(SIDEBAR_NOTICE_LIST_LAYOUT);
            ui_hide(SIDEBAR_NOTICE_NO_MSG_LAYOUT);
            ui_show(SIDEBAR_DIALOG_LAYOUT);

            struct element *dialog_elm = ui_core_get_element_by_id(SIDEBAR_DIALOG_LAYOUT);
            ASSERT(dialog_elm);
            ui_core_get_element_abs_rect(dialog_elm, &dialog_rect);

            __info->in_dialog = true;

            ui_anim_init(&__info->dialog_anim);
            ui_anim_set_var(&__info->dialog_anim, SIDEBAR_DIALOG_LAYOUT);
            ui_anim_set_path_cb(&__info->dialog_anim, ui_anim_path_ease_out);
            ui_anim_set_exec_cb(&__info->dialog_anim, notice_dialog_anim_cb);
            ui_anim_set_ready_cb(&__info->dialog_anim, notice_dialog_anim_ready);
            ui_anim_set_values(&__info->dialog_anim, -main_rect.height, -main_rect.height + dialog_rect.height);
            ui_anim_set_time(&__info->dialog_anim, DIALOG_ANIM_RUN_TIME);
            ui_anim_start(&__info->dialog_anim);
        }
    }
}

/* 消息弹窗隐藏 */
void notice_dialog_hide(int anim)
{
    // 如果弹窗没显示，没必要跑隐藏回调
    if (!__info->in_dialog) {
        return;
    }

    if (__info->dialog_timeout) {
        sys_timeout_del(__info->dialog_timeout);
        __info->dialog_timeout = 0;
    }

    __info->in_dialog = false;

    struct element *main_elm = ui_core_get_element_by_id(SIDEBAR_NOTICE_MAIN_LAYOUT);
    if (main_elm) {
        struct rect main_rect;
        ui_core_get_element_abs_rect(main_elm, &main_rect);

        if ((anim) && (main_rect.top > -main_rect.height)) {
            ui_anim_init(&__info->dialog_anim);
            ui_anim_set_var(&__info->dialog_anim, SIDEBAR_DIALOG_LAYOUT);
            ui_anim_set_path_cb(&__info->dialog_anim, ui_anim_path_ease_out);
            ui_anim_set_exec_cb(&__info->dialog_anim, notice_dialog_anim_cb);
            ui_anim_set_ready_cb(&__info->dialog_anim, notice_dialog_anim_ready);
            ui_anim_set_values(&__info->dialog_anim, main_rect.top, -main_rect.height);
            ui_anim_set_time(&__info->dialog_anim, DIALOG_ANIM_RUN_TIME);
            ui_anim_start(&__info->dialog_anim);
        } else {
            main_elm->css.top = -main_elm->css.height;
            // ui_core_redraw(main_elm);
        }
    }
}

static int notice_dialog_layout_ontouch(void *ctrl, struct element_touch_event *e)
{
    // printf("@@@ %s(), event: %d\n", __func__, e->event);
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            /*删掉隐藏定时器*/
            if (__info->dialog_timeout) {
                sys_timeout_del(__info->dialog_timeout);
                __info->dialog_timeout = 0;
            }

            /*删除动画*/
            ui_anim_del(SIDEBAR_DIALOG_LAYOUT, NULL);

            /* 跳转到消息详情界面 */
            ui_hide(SIDEBAR_DIALOG_LAYOUT);
            ui_show(SIDEBAR_LAYOUT_MESSAGE_DETAIL);
            __info->in_dialog = false;
            __info->from_dialog = true;

            /*把消息界面拉到屏幕内*/
            struct element *main_elm = ui_core_get_element_by_id(SIDEBAR_NOTICE_MAIN_LAYOUT);
            if (main_elm) {
                main_elm->css.top = 0;
                // ui_core_redraw(main_elm);
            }
        }
        break;
    default:
        return false;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(NOTICE_DIALOG_BG)     // 这里用背景图片控制，目的是把点击范围限制到背景图片区域内
.onchange =  NULL,
 .onkey = NULL,
  .ontouch = notice_dialog_layout_ontouch,
};

//===============================================================================================================//
static int text_clear_ontouch(void *ctrl, struct element_touch_event *e)
{
    // printf("@@@ %s(), event: %d\n", __func__, e->event);
    struct element *elm  = (struct element *)ctrl;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return 0;
        }
        if (elm->id == SIDEBAR_TEXT_CLEAR_ALL) {
            ui_hide(SIDEBAR_NOTICE_LIST_LAYOUT);
            /* 清空所有消息 */
            while (ui_small_file_message_get_count()) {
                small_file_delete_by_index(F_TYPE_MESSAGE, 0);
            }
            ui_show(SIDEBAR_NOTICE_NO_MSG_LAYOUT);
        } else {
            ui_hide(SIDEBAR_LAYOUT_MESSAGE_DETAIL);
            /* 删除当前消息 */
            int del_index = ui_small_file_message_get_count() - 1 - __info->select_index;
            ASSERT(del_index >= 0, "total:%d del:%d sel:%d", ui_small_file_message_get_count(), del_index, __info->select_index);
            small_file_delete_by_index(F_TYPE_MESSAGE, del_index);
            if (ui_small_file_message_get_count()) {
                ui_show(SIDEBAR_NOTICE_LIST_LAYOUT);
            } else {
                ui_show(SIDEBAR_NOTICE_NO_MSG_LAYOUT);
            }
            if (!create_control_by_menu_get()) {
                ui_card_enable();
            }
        }
        break;
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


#endif
#endif//CONFIG_UI_STYLE_JL_SCREEN_BOX_PUBLIC_MODLS_ENABLE
