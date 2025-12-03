#include "app_config.h"
/* #include "app_task.h" */
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "rtc.h"

#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"
#include "ui_page_switch.h"
#include "btstack/avctp_user.h"
#include "jlui_app/ui_app_effect.h"
#include "events_adapter.h"
#include "ui_bg_manage.h"
#include "smartbox_info_manager.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_DIAL]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_dial.data.bss")
#pragma data_seg(".ui_action_dial.data")
#pragma const_seg(".ui_action_dial.text.const")
#pragma code_seg(".ui_action_dial.text")
#endif

#if TCFG_UI_ENABLE
#ifdef CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE
#if TCFG_UI_DIAL_ENABLE


#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)


#define	SCREEN_WIDTH									320
#define SCREEN_HEIGHT									172
#define DIAL_SIDEBAR_START_THRESHOLD      				(SCREEN_WIDTH  * 15 / 100)
#define DIAL_SIDEBAR_MAX_THRESHOLD      				(SCREEN_HEIGHT * 70 / 100)//区域判断
#define DIAL_SIDEBAR_MIN_THRESHOLD      				(SCREEN_HEIGHT * 30 / 100)//区域判断
#define DIAL_SIDEBAR_RUN_MAX_THRESHOLD  				(SCREEN_HEIGHT * 85 / 100)//滑动距离判断
#define DIAL_SIDEBAR_RUN_MIN_THRESHOLD  				(SCREEN_HEIGHT * 15 / 100)//滑动距离判断
#define DIAL_SIDEBAR_LEFT_MAX_THRESHOLD      			(SCREEN_WIDTH  * 70 / 100)//区域判断
#define DIAL_SIDEBAR_LEFT_MIN_THRESHOLD      			(SCREEN_WIDTH  * 100 / 100)//区域判断
#define DIAL_SIDEBAR_LEFT_RUN_MAX_THRESHOLD  			(SCREEN_WIDTH  * 85 / 100)//滑动距离判断
#define DIAL_SIDEBAR_LEFT_RUN_MIN_THRESHOLD  			(SCREEN_WIDTH  * 15 / 100)//滑动距离判断
#define DIAL_SIDEBAR_STEP               				(SCREEN_HEIGHT * 20 / 100)//滑动步进
#if ((defined(TCFG_UI_MSG_NOTICE) && TCFG_UI_MSG_NOTICE) && (defined(NOTICE_LAYOUT) && NOTICE_LAYOUT))
#define DIAL_SIDEBAR_TOP_LAYOUT							NOTICE_LAYOUT
#endif
#define DIAL_SIDEBAR_TOP_PAGE							ID_WINDOW_NOTICE
// #define DIAL_SIDEBAR_BUTTON_LAYOUT						PULLUP_MENU_LAYOUT
#define DIAL_SIDEBAR_BUTTON_PAGE						PAGE_NULL
// #define	DIAL_SIDEBAR_LEFT_LAYOUT						SIDEBAR
#define DIAL_SIDEBAR_LEFT_PAGE							PAGE_NULL
#define SIDEBAR_TAB_PATH								MODE_PATH"JL/JL.tab"
#define MAX_GRID_ITEM       (6)
#define WATCH_ITEMS_LIMIT   (10)

#if (defined TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE) && TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE
static void watch_enter_anim_cb(int var, int v)
{
    struct element *elm = ui_core_get_element_by_id(var);
    if (!elm) {
        return;
    }
    elm->css.left = v;
    // ui_core_set_element_ratio_en(elm, 1);
    // ui_core_set_element_ratio(elm, 2.0, 2.0, 1);


    ui_core_redraw(elm);
    // ui_core_redraw(elm->parent);
}
static void watch_enter_anim_cb2(int var, int v)
{
    struct element *elm = ui_core_get_element_by_id(var);
    if (!elm) {
        return;
    }

    if (v == 256) {
        ui_core_set_element_ratio_en(elm, 0);
    } else {
        ui_core_set_element_ratio(elm, (float)v / 256.0, (float)v / 256.0, 1);
    }


    ui_core_redraw(elm);
}
#endif

void ui_card_enable();
void ui_card_set_move_mode(u8 mode);
extern void ui_page_move_en_callback(u8(*move_en_query)(void));
u8 is_ui_page_move_enable(void);
void ui_send_event(u16 event, u32 val);
void ui_core_redraw_area(struct rect *rect);
int watch_get_items_num();
char *watch_get_item(int style);
u8 create_control_by_menu_set(u8 en);
u8 create_control_by_menu_get();
int notice_detail_control_onchange(void *_ctrl, enum element_change_event event, void *arg);
u8 rtc_calculate_week_val(struct sys_time *data_time);
int notice_layout_onchange(void *_ctrl, enum element_change_event event, void *arg);
char *watch_bgp_get_related_path(u8 cur_watch);
int watch_bgp_set_related(char *bgp, u8 cur_watch, u8 del);
extern struct ui_platform_api *ui_get_platform_api();
extern int window_init(int id);

enum {
    SLIDER_MOVE_NONE,
    SLIDER_MOVE_UD,
    SLIDER_MOVE_LF,
};

enum SIDERBAR_STATUS {
    SIDEBAR_HIDE,
    SIDEBAR_MOVE_X,
    SIDEBAR_MOVE_Y,
    SIDEBAR_SHOWING_TOP,
    SIDEBAR_SHOWING_BOTTOM,
    SIDEBAR_SHOWING_LEFT,
    SIDEBAR_SHOW_TOP,
    SIDEBAR_SHOW_BOTTOM,
    SIDEBAR_SHOW_LEFT,
};
typedef struct viewfile {
    UI_RESFILE *file;
    struct flash_file_info info;
} VIEWFILE;
struct sidebar_priv {
    u8 sidebar_root;
    u8 stop;
    u8 move_dir;
    s16 first_x_offset;
    s16 x_offset;
    s16 first_y_offset;
    s16 y_offset;
    s16 step;
    u16 timer;
    struct position pos;		//单次触点
    struct position move_pos;	//累进滑动
    enum SIDERBAR_STATUS sidebar_status;
};

struct watch_param {
    int curr_watch_style;
    UI_RESFILE *view_file;
    struct flash_file_info view_file_info;
    int find_animation;
    int picture_num;
    int curr_picture;
};

struct dial_preview_t {
    int xoffset;
    int pos_x;
    int pos_x_total;

    pJLGPUTaskHead_t new_head;
    gpu_matrix_t matrix;
    struct element *this_curr_elm;

    struct draw_context new_dc_ontouch;
    struct draw_context new_dc_anim;

    int anim_tmp;
    ui_anim_t anim;

    u8 dial_sel_item;
    u8 is_dial_preview;
    u8 dial_num;
    u8 dial_index[WATCH_ITEMS_LIMIT];
};
static struct dial_preview_t *dial_preview = NULL;

#define abs(x)  ((x)>0?(x):-(x) )
static VIEWFILE *view_file = NULL;
static struct sidebar_priv sidebar = {0};
static u16 watch_show_timer = 0;

static u16 timer_interval = 0;
static u16 watch_msec = 0;	// 表盘毫秒计数
static struct watch_param dial_param;
static int watch_refresh(int id, struct ui_watch *watch)
{
    if (timer_interval != 500) {
        watch_msec += timer_interval;
        if (watch_msec >= 1000) {
            watch_msec -= 1000;
        }
    } else {
        watch_msec = 0;
    }

    if (!strcmp(watch->source, "rtc")) {
        struct sys_time time;
        rtc_read_time(&time);
        /* log_info("%04d-%02d-%02d %02d:%02d:%02d\n", time.year, time.month, time.day, time.hour, time.min, time.sec); */
        ui_watch_set_time((struct ui_watch *)ui_core_get_element_by_id(id), time.hour % 12, time.min, time.sec, watch_msec);
    } else {
        /* log_info("%s %s", __func__, watch->source); */
    }
    return 0;
}

static int progress_refresh(int id, struct ui_progress *progress)
{
    return 0;
}
static int multiprogress_refresh(int id, struct ui_multiprogress *multiprogress)
{
    return 0;
}
static int text_refresh(int id, struct ui_text *text)
{
    return 0;
}
static int number_refresh(int id, struct ui_number *number)
{
    return 0;
}
static int time_refresh(int id, struct ui_time *time)
{
    return 0;
}
static int progress_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_progress *progress = (struct ui_progress *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    }

    return 0;
}

static int multiprogress_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_multiprogress *multiprogress = (struct ui_multiprogress *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    }
    return 0;
}

static int text_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    }
    return 0;
}

static int time_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_time *time = (struct ui_time *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    }
    return 0;
}

static int number_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    }
    return 0;
}

static int progress_onkey(void *_ctrl, struct element_key_event *event)
{
    struct ui_progress *progress = (struct ui_progress *)_ctrl;
    switch (event->value) {
    case KEY_UP:
        break;
    case KEY_DOWN:
        break;
    case KEY_LEFT:
        break;
    case KEY_RIGHT:
        break;
    }
    return 0;
}

static int multiprogress_onkey(void *_ctrl, struct element_key_event *event)
{
    struct ui_multiprogress *multiprogress = (struct ui_multiprogress *)_ctrl;
    switch (event->value) {
    case KEY_UP:
        break;
    case KEY_DOWN:
        break;
    case KEY_LEFT:
        break;
    case KEY_RIGHT:
        break;
    }
    return 0;
}

static int time_onkey(void *_ctrl, struct element_key_event *event)
{
    struct ui_time *time = (struct ui_time *)_ctrl;
    switch (event->value) {
    case KEY_UP:
        break;
    case KEY_DOWN:
        break;
    case KEY_LEFT:
        break;
    case KEY_RIGHT:
        break;
    }
    return 0;
}

static int text_onkey(void *_ctrl, struct element_key_event *event)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    switch (event->value) {
    case KEY_UP:
        break;
    case KEY_DOWN:
        break;
    case KEY_LEFT:
        break;
    case KEY_RIGHT:
        break;
    }
    return 0;
}

static int number_onkey(void *_ctrl, struct element_key_event *event)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    switch (event->value) {
    case KEY_UP:
        break;
    case KEY_DOWN:
        break;
    case KEY_LEFT:
        break;
    case KEY_RIGHT:
        break;
    }
    return 0;
}

static int multiprogress_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_multiprogress *multiprogress = (struct ui_multiprogress *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int progress_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_progress *progress = (struct ui_progress *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int text_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
static int watch_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_watch *watch = (struct ui_watch *)_ctrl;

    switch (event) {
#if (defined TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE) && TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE
    case ON_CHANGE_SHOW_POST:
        ui_core_set_element_rotate(watch, 0, 0, 100, 100, 45.0, 1);
        break;
#endif
    case ON_CHANGE_SHOW_PROBE:
        if (!strcmp(watch->source, "rtc")) {
            struct sys_time time_t;
            rtc_read_time(&time_t);

            watch->hour = time_t.hour;
            watch->min = time_t.min;
            watch->sec = time_t.sec;
            ui_watch_update(watch, 0);
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int time_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_time *time = (struct ui_time *)_ctrl;

    switch (event) {
#if (defined TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE) && TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE
    case ON_CHANGE_FIRST_SHOW:
        if (!strcmp(time->source, "none")) {
            r_printf("%s %d", __func__, __LINE__);
            int start_pos  = time->text.elm.css.left - 200;
            int end_pos =  time->text.elm.css.left;
            ui_anim_t a = {0};
            ui_anim_init(&a);
            ui_anim_set_var(&a, time->text.elm.id);
            ui_anim_set_path_cb(&a, ui_anim_path_overshoot);
            ui_anim_set_exec_cb(&a, watch_enter_anim_cb);
            ui_anim_set_values(&a, start_pos, end_pos);
            ui_anim_set_time(&a, 300);
            ui_anim_start(&a);
        } else if (!strcmp(time->source, "none1")) {
            r_printf("%s %d", __func__, __LINE__);
            int start_pos  = time->text.elm.css.left + 200;
            int end_pos =  time->text.elm.css.left;
            ui_anim_t a = {0};
            ui_anim_init(&a);
            ui_anim_set_var(&a, time->text.elm.id);
            ui_anim_set_path_cb(&a, ui_anim_path_overshoot);
            ui_anim_set_exec_cb(&a, watch_enter_anim_cb);
            ui_anim_set_values(&a, start_pos, end_pos);
            ui_anim_set_time(&a, 300);
            ui_anim_start(&a);
        }
        break;
#endif
    case ON_CHANGE_SHOW_PROBE:
        if ((!strcmp(time->source, "rtc")) || (!strcmp(time->source, "watdat"))) {
            struct sys_time time_t;
            struct utime ui_time = {0};

            rtc_read_time(&time_t);
            ui_time.year  = time_t.year;
            ui_time.month = time_t.month;
            ui_time.day  = time_t.day;
            ui_time.hour = time_t.hour;
            ui_time.min = time_t.min;
            ui_time.sec = time_t.sec;
            ui_time_update(time, &ui_time);
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int number_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    static char str[8] = {0};
    static char str1[8] = {0};
    struct unumber num;
    switch (event) {
    case ON_CHANGE_INIT:
        int l_bat ;
        int r_bat ;
        l_bat = sbox_battery_left_get();
        r_bat = sbox_battery_right_get();
        log_info("%s %d   %s  %d", __func__, l_bat, number->source, strcmp(number->source, "left_ear"));
        if (!strncmp(number->source, "left_ear", 8)) {
            log_info("%s %d", __func__, l_bat);
            sprintf(str, "%d:", l_bat);
            struct unumber n;
            n.type = TYPE_STRING;
            n.num_str = (u8 *)str;
            ui_number_update(number, &n);
        } else if (!strncmp(number->source, "rightear", 8)) {
            sprintf(str1, "%d:", r_bat);
            struct unumber n;
            n.type = TYPE_STRING;
            n.num_str = (u8 *)str1;
            ui_number_update(number, &n);
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }

    return 0;
}

#if TCFG_DIAL_STYLE_AUTUMN_LEAVES
#define MAPLE_PIC1_INDEX         0x000001
#define MAPLE_PIC2_INDEX         0x000002
#define RATIO_INDEX              10.0f
#define MAPLE_RED_NUM            10         // 红色叶子数量
#define MAPLE_YELLOW_NUM         10         // 黄色叶子数量
#define MAPLE_MAX_RATIO          7          // 叶子最大缩放大小,需要除以10,如7实际代表0.7
#define MAPLE_MAX_X_SPEED        3          // 叶子最大x坐标移动速度
#define MAPLE_MAX_Y_SPEED        3          // 叶子最大y坐标移动速度
#define MAPLE_MAX_ROTATE_SPEED   10         // 叶子最大旋转速度

struct maple_t {
    u32 maple_red_timer;
    u32 maple_yellow_timer;

    int maple_red_x[MAPLE_RED_NUM];
    int maple_red_y[MAPLE_RED_NUM];
    int maple_red_speed_x[MAPLE_RED_NUM];
    int maple_red_speed_y[MAPLE_RED_NUM];
    int maple_red_rotate_angle[MAPLE_RED_NUM];
    float maple_red_ratio[MAPLE_RED_NUM];

    int maple_yellow_x[MAPLE_YELLOW_NUM];
    int maple_yellow_y[MAPLE_YELLOW_NUM];
    int maple_yellow_speed_x[MAPLE_YELLOW_NUM];
    int maple_yellow_speed_y[MAPLE_YELLOW_NUM];
    int maple_yellow_rotate_angle[MAPLE_YELLOW_NUM];
    float maple_yellow_ratio[MAPLE_YELLOW_NUM];

    struct ui_image_attrs maple_red_image_attr;
    struct ui_image_attrs maple_yellow_image_attr;
    struct rect win;
};
static struct maple_t *maple = NULL;

static void maple_red_cb(void *priv)
{
    struct element *elm = (struct element *)priv;
    ui_core_redraw(elm);
}

static void maple_yellow_cb(void *priv)
{
    struct element *elm = (struct element *)priv;
    ui_core_redraw(elm);
}

_NOINLINE_
static void maple_red_init(struct ui_pic *pic)
{
    if (!maple) {
        maple = zalloc(sizeof(struct maple_t));
        jlgpu_get_win_rect(&(maple->win));
    }
    struct draw_context dc_tmp = {0};
    struct rect rect = {0};
    ui_core_get_draw_context(&dc_tmp, &(pic->elm), &rect);
    dc_tmp.prj = 1;
    dc_tmp.page = (MAPLE_PIC1_INDEX >> 16) & 0xff;
    struct ui_platform_api *platform_api = ui_get_platform_api();
    platform_api->read_image_info(&dc_tmp, MAPLE_PIC1_INDEX & 0xffff, &(maple->maple_red_image_attr));
    for (int i = 0; i < MAPLE_RED_NUM; i++) {
        maple->maple_red_x[i] = rand32() % maple->win.width + 1;
        maple->maple_red_y[i] = rand32() % maple->win.height + 1;
        maple->maple_red_speed_x[i] = rand32() % MAPLE_MAX_X_SPEED + 1;
        maple->maple_red_speed_y[i] = rand32() % MAPLE_MAX_Y_SPEED + 1;
        maple->maple_red_ratio[i] = (float)(rand32() % MAPLE_MAX_RATIO + 1) / RATIO_INDEX;
        maple->maple_red_rotate_angle[i] = rand32() % MAPLE_MAX_ROTATE_SPEED + 1;
    }
}

_NOINLINE_
static void maple_yel_init(struct ui_pic *pic)
{
    if (!maple) {
        maple = zalloc(sizeof(struct maple_t));
        jlgpu_get_win_rect(&(maple->win));
    }
    struct draw_context dc_tmp = {0};
    struct rect rect = {0};
    ui_core_get_draw_context(&dc_tmp, &(pic->elm), &rect);
    dc_tmp.prj = 1;
    dc_tmp.page = (MAPLE_PIC2_INDEX >> 16) & 0xff;
    struct ui_platform_api *platform_api = ui_get_platform_api();
    platform_api->read_image_info(&dc_tmp, MAPLE_PIC2_INDEX & 0xffff, &(maple->maple_yellow_image_attr));
    for (int i = 0; i < MAPLE_YELLOW_NUM; i++) {
        maple->maple_yellow_x[i] = rand32() % maple->win.width + 1;
        maple->maple_yellow_y[i] = rand32() % maple->win.height + 1;
        maple->maple_yellow_speed_x[i] = rand32() % MAPLE_MAX_X_SPEED + 1;
        maple->maple_yellow_speed_y[i] = rand32() % MAPLE_MAX_Y_SPEED + 1;
        maple->maple_yellow_ratio[i] = (float)(rand32() % MAPLE_MAX_RATIO + 1) / RATIO_INDEX;
        maple->maple_yellow_rotate_angle[i] = rand32() % MAPLE_MAX_ROTATE_SPEED + 1;
    }
}

_NOINLINE_
static void maple_deinit()
{
    if (maple && maple->maple_red_timer) {
        sys_timer_del(maple->maple_red_timer);
        maple->maple_red_timer = 0;
    }
    if (maple && maple->maple_yellow_timer) {
        sys_timer_del(maple->maple_yellow_timer);
        maple->maple_yellow_timer = 0;
    }
    if (maple) {
        free(maple);
        maple = NULL;
    }
}

_NOINLINE_
static void maple_red_show(struct ui_pic *pic, void *arg)
{
    if (maple) {
        for (int i = 0; i < MAPLE_RED_NUM; i++) {
            ui_draw_image((struct draw_context *)arg, MAPLE_PIC1_INDEX >> 16, MAPLE_PIC1_INDEX & 0xffff, maple->maple_red_x[i], maple->maple_red_y[i], 1, maple->maple_red_ratio[i], maple->maple_red_ratio[i], 1, maple->maple_red_image_attr.width / 2 + maple->maple_red_x[i], maple->maple_red_image_attr.height / 2 + maple->maple_red_y[i], maple->maple_red_image_attr.width / 2, maple->maple_red_image_attr.height / 2, maple->maple_red_rotate_angle[i]);

            maple->maple_red_x[i] = maple->maple_red_x[i] + maple->maple_red_speed_x[i];
            if (maple->maple_red_x[i] >= maple->win.width) {
                maple->maple_red_x[i] = 0;
                maple->maple_red_y[i] = rand32() % maple->win.height + 1;
                maple->maple_red_speed_x[i] = rand32() % MAPLE_MAX_X_SPEED + 1;
                maple->maple_red_speed_y[i] = rand32() % MAPLE_MAX_Y_SPEED + 1;
                maple->maple_red_ratio[i] = (float)(rand32() % MAPLE_MAX_RATIO + 1) / RATIO_INDEX;
            }
            maple->maple_red_y[i] = maple->maple_red_y[i] + maple->maple_red_speed_y[i];
            if (maple->maple_red_y[i] >= maple->win.height) {
                maple->maple_red_y[i] = 0;
                maple->maple_red_x[i] = rand32() % maple->win.width + 1;
                maple->maple_red_speed_x[i] = rand32() % MAPLE_MAX_X_SPEED + 1;
                maple->maple_red_speed_y[i] = rand32() % MAPLE_MAX_Y_SPEED + 1;
                maple->maple_red_ratio[i] = (float)(rand32() % MAPLE_MAX_RATIO + 1) / RATIO_INDEX;
            }
            maple->maple_red_rotate_angle[i] += rand32() % MAPLE_MAX_ROTATE_SPEED + 1;
        }
        if (!maple->maple_red_timer) {
            maple->maple_red_timer = sys_timer_add(&(pic->elm), maple_red_cb, 50);
        }

    } else {
        if (!maple) {
            maple = zalloc(sizeof(struct maple_t));
            jlgpu_get_win_rect(&(maple->win));
        }
        struct draw_context dc_tmp = {0};
        struct rect rect = {0};
        ui_core_get_draw_context(&dc_tmp, &(pic->elm), &rect);
        dc_tmp.prj = 1;
        dc_tmp.page = (MAPLE_PIC1_INDEX >> 16) & 0xff;
        struct ui_platform_api *platform_api = ui_get_platform_api();
        platform_api->read_image_info(&dc_tmp, MAPLE_PIC1_INDEX & 0xffff, &(maple->maple_red_image_attr));
        for (int i = 0; i < MAPLE_RED_NUM; i++) {
            maple->maple_red_x[i] = rand32() % maple->win.width + 1;
            maple->maple_red_y[i] = rand32() % maple->win.height + 1;
            maple->maple_red_speed_x[i] = rand32() % MAPLE_MAX_X_SPEED + 1;
            maple->maple_red_speed_y[i] = rand32() % MAPLE_MAX_Y_SPEED + 1;
            maple->maple_red_ratio[i] = (float)(rand32() % MAPLE_MAX_RATIO + 1) / RATIO_INDEX;
            maple->maple_red_rotate_angle[i] = rand32() % MAPLE_MAX_ROTATE_SPEED + 1;
        }

        memset(&dc_tmp, 0, sizeof(struct draw_context));
        memset(&rect, 0, sizeof(struct rect));

        ui_core_get_draw_context(&dc_tmp, &(pic->elm), &rect);
        dc_tmp.prj = 1;
        dc_tmp.page = (MAPLE_PIC2_INDEX >> 16) & 0xff;
        platform_api->read_image_info(&dc_tmp, MAPLE_PIC2_INDEX & 0xffff, &(maple->maple_yellow_image_attr));
        for (int i = 0; i < MAPLE_YELLOW_NUM; i++) {
            maple->maple_yellow_x[i] = rand32() % maple->win.width + 1;
            maple->maple_yellow_y[i] = rand32() % maple->win.height + 1;
            maple->maple_yellow_speed_x[i] = rand32() % MAPLE_MAX_X_SPEED + 1;
            maple->maple_yellow_speed_y[i] = rand32() % MAPLE_MAX_Y_SPEED + 1;
            maple->maple_yellow_ratio[i] = (float)(rand32() % MAPLE_MAX_RATIO + 1) / RATIO_INDEX;
            maple->maple_yellow_rotate_angle[i] = rand32() % MAPLE_MAX_ROTATE_SPEED + 1;
        }

    }
}

_NOINLINE_
static void maple_yel_show(struct ui_pic *pic, void *arg)
{
    if (maple) {
        for (int i = 0; i < MAPLE_YELLOW_NUM; i++) {
            ui_draw_image((struct draw_context *)arg, MAPLE_PIC2_INDEX >> 16, MAPLE_PIC2_INDEX & 0xffff, maple->maple_yellow_x[i], maple->maple_yellow_y[i], 1, maple->maple_yellow_ratio[i], maple->maple_yellow_ratio[i], 1, maple->maple_yellow_image_attr.width / 2 + maple->maple_yellow_x[i], maple->maple_yellow_image_attr.height / 2 + maple->maple_yellow_y[i], maple->maple_yellow_image_attr.width / 2, maple->maple_yellow_image_attr.height / 2, maple->maple_yellow_rotate_angle[i]);

            maple->maple_yellow_x[i] = maple->maple_yellow_x[i] + maple->maple_yellow_speed_x[i];
            if (maple->maple_yellow_x[i] >= maple->win.width) {
                maple->maple_yellow_x[i] = 0;
                maple->maple_yellow_y[i] = rand32() % maple->win.height + 1;
                maple->maple_yellow_speed_x[i] = rand32() % MAPLE_MAX_X_SPEED + 1;
                maple->maple_yellow_speed_y[i] = rand32() % MAPLE_MAX_Y_SPEED + 1;
                maple->maple_yellow_ratio[i] = (float)(rand32() % MAPLE_MAX_RATIO + 1) / RATIO_INDEX;
            }
            maple->maple_yellow_y[i] = maple->maple_yellow_y[i] + maple->maple_yellow_speed_y[i];
            if (maple->maple_yellow_y[i] >= maple->win.height) {
                maple->maple_yellow_y[i] = 0;
                maple->maple_yellow_x[i] = rand32() % maple->win.width + 1;
                maple->maple_yellow_speed_x[i] = rand32() % MAPLE_MAX_X_SPEED + 1;
                maple->maple_yellow_speed_y[i] = rand32() % MAPLE_MAX_Y_SPEED + 1;
                maple->maple_yellow_ratio[i] = (float)(rand32() % MAPLE_MAX_RATIO + 1) / RATIO_INDEX;
            }
            maple->maple_yellow_rotate_angle[i] += rand32() % MAPLE_MAX_ROTATE_SPEED + 1;
        }
        if (!maple->maple_yellow_timer) {
            maple->maple_yellow_timer = sys_timer_add(&(pic->elm), maple_yellow_cb, 50);
        }

    } else {
        if (!maple) {
            maple = zalloc(sizeof(struct maple_t));
            jlgpu_get_win_rect(&(maple->win));
        }
        struct draw_context dc_tmp = {0};
        struct rect rect = {0};
        ui_core_get_draw_context(&dc_tmp, &(pic->elm), &rect);
        dc_tmp.prj = 1;
        dc_tmp.page = (MAPLE_PIC1_INDEX >> 16) & 0xff;
        struct ui_platform_api *platform_api = ui_get_platform_api();
        platform_api->read_image_info(&dc_tmp, MAPLE_PIC1_INDEX & 0xffff, &(maple->maple_red_image_attr));
        for (int i = 0; i < MAPLE_RED_NUM; i++) {
            maple->maple_red_x[i] = rand32() % maple->win.width + 1;
            maple->maple_red_y[i] = rand32() % maple->win.height + 1;
            maple->maple_red_speed_x[i] = rand32() % MAPLE_MAX_X_SPEED + 1;
            maple->maple_red_speed_y[i] = rand32() % MAPLE_MAX_Y_SPEED + 1;
            maple->maple_red_ratio[i] = (float)(rand32() % MAPLE_MAX_RATIO + 1) / RATIO_INDEX;
            maple->maple_red_rotate_angle[i] = rand32() % MAPLE_MAX_ROTATE_SPEED + 1;
        }

        memset(&dc_tmp, 0, sizeof(struct draw_context));
        memset(&rect, 0, sizeof(struct rect));

        ui_core_get_draw_context(&dc_tmp, &(pic->elm), &rect);
        dc_tmp.prj = 1;
        dc_tmp.page = (MAPLE_PIC2_INDEX >> 16) & 0xff;
        platform_api->read_image_info(&dc_tmp, MAPLE_PIC2_INDEX & 0xffff, &(maple->maple_yellow_image_attr));
        for (int i = 0; i < MAPLE_YELLOW_NUM; i++) {
            maple->maple_yellow_x[i] = rand32() % maple->win.width + 1;
            maple->maple_yellow_y[i] = rand32() % maple->win.height + 1;
            maple->maple_yellow_speed_x[i] = rand32() % MAPLE_MAX_X_SPEED + 1;
            maple->maple_yellow_speed_y[i] = rand32() % MAPLE_MAX_Y_SPEED + 1;
            maple->maple_yellow_ratio[i] = (float)(rand32() % MAPLE_MAX_RATIO + 1) / RATIO_INDEX;
            maple->maple_yellow_rotate_angle[i] = rand32() % MAPLE_MAX_ROTATE_SPEED + 1;
        }

    }
}
#endif

static int pic_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        if (!strcmp(pic->source, "mp_red")) {
#if TCFG_DIAL_STYLE_AUTUMN_LEAVES
            maple_red_init(pic);
#endif
        }
        if (!strcmp(pic->source, "mp_yel")) {
#if TCFG_DIAL_STYLE_AUTUMN_LEAVES
            maple_yel_init(pic);
#endif
        }
        break;
#if (defined TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE) && TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE
//居中再启动动画
    case ON_CHANGE_FIRST_SHOW:
        if (!strcmp(pic->source, "watwek")) {
            r_printf("%s %d", __func__, __LINE__);
            int start_pos  = pic->elm.css.left + 200;
            int end_pos =  pic->elm.css.left;
            ui_anim_t a = {0};
            ui_anim_init(&a);
            ui_anim_set_var(&a, pic->elm.id);
            ui_anim_set_path_cb(&a, ui_anim_path_overshoot);
            ui_anim_set_exec_cb(&a, watch_enter_anim_cb);
            ui_anim_set_values(&a, start_pos, end_pos);
            ui_anim_set_time(&a, 300);
            ui_anim_start(&a);
        }
        break;
#endif
    case ON_CHANGE_SHOW_PROBE:
        if (!strcmp(pic->source, "watwek")) {
            struct sys_time time_t;
            rtc_read_time(&time_t);
            int week_index  = rtc_calculate_week_val(&time_t);
            ui_pic_set_image_index(pic, week_index); //0周日
        }
        break;
    case ON_CHANGE_SHOW_POST:
        if (!strcmp(pic->source, "mp_red")) {
#if TCFG_DIAL_STYLE_AUTUMN_LEAVES
            maple_red_show(pic, arg);
#endif
        }
        if (!strcmp(pic->source, "mp_yel")) {
#if TCFG_DIAL_STYLE_AUTUMN_LEAVES
            maple_yel_show(pic, arg);
#endif
        }
        break;
    case ON_CHANGE_RELEASE:
        if (!strcmp(pic->source, "mp_yel") || !strcmp(pic->source, "mp_red")) {
#if TCFG_DIAL_STYLE_AUTUMN_LEAVES
            maple_deinit();
#endif
        }
        break;
    default:
        break;
    }

    return 0;
}
static int watch_child_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)_ctrl;

    int type = ui_id2type(elm->id);
    // if (dial_dynamic_manager_ontouch(_ctrl, e)) {
    //     return true;
    // }
    switch (type) {
    case CTRL_TYPE_PROGRESS:
        progress_ontouch(_ctrl, e);
        break;
    case CTRL_TYPE_MULTIPROGRESS:
        multiprogress_ontouch(_ctrl, e);
        break;
    case CTRL_TYPE_TEXT:
        text_ontouch(_ctrl, e);
        break;
    case CTRL_TYPE_NUMBER:
        number_ontouch(_ctrl, e);
        break;
    case CTRL_TYPE_TIME:
        time_ontouch(_ctrl, e);
        break;
    default:
        break;
    }

    return 0;
}

static int watch_child_onkey(void *_ctrl, struct element_key_event *e)
{
    struct element *elm = (struct element *)_ctrl;
    int type = ui_id2type(elm->id);
    // if (dial_dynamic_manager_onkey(_ctrl, e)) {
    //     return true;
    // }
    switch (type) {
    case CTRL_TYPE_PROGRESS:
        progress_onkey(_ctrl, e);
        break;
    case CTRL_TYPE_MULTIPROGRESS:
        multiprogress_onkey(_ctrl, e);
        break;
    case CTRL_TYPE_TEXT:
        text_onkey(_ctrl, e);
        break;
    case CTRL_TYPE_NUMBER:
        number_onkey(_ctrl, e);
        break;
    case CTRL_TYPE_TIME:
        time_onkey(_ctrl, e);
        break;
    }

    return 0;
}

static int watch_child_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    int type = ui_id2type(elm->id);
#if ((defined(TCFG_UI_MSG_NOTICE) && TCFG_UI_MSG_NOTICE) && (defined(NOTICE_LAYOUT) && NOTICE_LAYOUT))
    notice_detail_control_onchange(elm, event, arg);
#endif
    // if (dial_dynamic_manager_onchange(_ctrl, event, arg)) {
    //     return true;
    // }
    switch (type) {
    case CTRL_TYPE_PROGRESS:
        progress_onchange(_ctrl, event, arg);
        break;
    case CTRL_TYPE_MULTIPROGRESS:
        multiprogress_onchange(_ctrl, event, arg);
        break;
    case CTRL_TYPE_TEXT:
        text_onchange(_ctrl, event, arg);
        break;
    case CTRL_TYPE_NUMBER:
        number_onchange(_ctrl, event, arg);
        break;
    case CTRL_TYPE_TIME:
        time_onchange(_ctrl, event, arg);
        break;
    case CTRL_TYPE_WATCH:
        watch_onchange(_ctrl, event, arg);
        break;
    case CTRL_TYPE_PIC:
        pic_onchange(_ctrl, event, arg);
        break;
    default:
        break;
    }

    return 0;
}

int watch_child_cb(void *_ctrl, int id, int type)
{
    switch (type) {
    case CTRL_TYPE_WATCH:
        watch_refresh(id, (struct ui_watch *)_ctrl);
        break;
    case CTRL_TYPE_PROGRESS:
        progress_refresh(id, (struct ui_progress *)_ctrl);
        break;
    case CTRL_TYPE_MULTIPROGRESS:
        multiprogress_refresh(id, (struct ui_multiprogress *)_ctrl);
        break;
    case CTRL_TYPE_TEXT:
        text_refresh(id, (struct ui_text *)_ctrl);
        break;
    case CTRL_TYPE_NUMBER:
        number_refresh(id, (struct ui_number *)_ctrl);
        break;
    case CTRL_TYPE_TIME:
        time_refresh(id, (struct ui_time *)_ctrl);
        break;
    case CTRL_TYPE_LAYOUT:
        struct element *p;
        list_for_each_child_element(p, (struct element *)_ctrl) {
            if (watch_child_cb(p, p->id, ui_id2type(p->id))) {
                break;
            }
        }
        break;
    }

    return 0;
}


static void WATCH_timer(void *priv)
{
    if (!watch_show_timer) {
        return;
    }

    ui_get_child_by_id(STYLE_DIAL_ID(WATCH), watch_child_cb);

    struct element *elm = ui_core_get_element_by_id(STYLE_DIAL_ID(WATCH));

    if (elm) {
        if (!elm->dc) {
            ui_core_get_dc(elm);
        }
        struct ui_effect_module *effmod_hd = ui_effect_get_handle_by_style(ui_card_get_move_mode());
        /* log_info("%s mode:%d hd:0x%x",__func__,ui_card_get_move_mode(),(u32)effmod_hd); */
        if (effmod_hd && effmod_hd->get_status) {
            if (effmod_hd->get_status() == 2) {
                return;
            }
        }

        ui_core_redraw(elm);
        extern void cube_effect_update(int timeout);
        extern void hexagon_effect_update(int timeout);
        extern void reflection_effect_update(int timeout);
        extern void cube_reflection_effect_update(int timeout);
        if (ui_card_get_move_mode() == PAGE_MOVE_MODE_CUBE) {
#if TCFG_UI_MOVE_MODE_CUBE
            cube_effect_update(timer_interval);
#endif
        } else if (ui_card_get_move_mode() == PAGE_MOVE_MODE_HEXAGON) {
#if TCFG_UI_MOVE_MODE_HEXAGON
            hexagon_effect_update(timer_interval);
#endif
        } else if (ui_card_get_move_mode() == PAGE_MOVE_MODE_REFLECTION) {
#if TCFG_UI_MOVE_MODE_REFLECTION
            reflection_effect_update(timer_interval);
#endif
        } else if (ui_card_get_move_mode() == PAGE_MOVE_MODE_CUBE_REFLECTION) {
#if TCFG_UI_MOVE_MODE_CUBE_REFLECTION
            cube_reflection_effect_update(timer_interval);
#endif
        }
    }
}
void ui_clear_gpu_task(struct draw_context *dc, struct element *elm)
{
    if (!elm) {
        return;
    }
    struct element *child_elm;
    list_for_each_child_element(child_elm, elm) {
        ui_clear_gpu_task(dc, child_elm);
    }
    jlgpu_task_delete_by_id((pJLGPUTaskHead_t)dc->gpu_task_head, JLGPU_ID_NONE, elm->id);
}
#if ((defined(TCFG_UI_MSG_NOTICE) && TCFG_UI_MSG_NOTICE) && (defined(NOTICE_LAYOUT) && NOTICE_LAYOUT))

extern int notice_message_status_handler(const char *type, u32 arg);
static int sidebar_message_status_handler(const char *type, u32 arg)
{
    if (sidebar.sidebar_status != SIDEBAR_SHOW_TOP) {
        extern void notice_dialog_show();
        notice_dialog_show();
    } else {
        notice_message_status_handler(type, arg);
    }
    return 0;
}
static const struct uimsg_handl ui_pd_menu_msg_handler[] = {
    {"message_status", sidebar_message_status_handler},
    {NULL, NULL},   // 必须以此结尾
};
#endif
int watch_unload_sidebar(struct element *elm)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    printf("<%s> rets:%x\n", __func__, rets);
    // if (ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT)) {
    //     ui_hide_set(DIAL_SIDEBAR_LEFT_LAYOUT, HIDE_WITHOUT_REDRAW);
    //     delete_control_by_id(DIAL_SIDEBAR_LEFT_LAYOUT);
    // }
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
    if (ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT)) {
        ui_hide_set(DIAL_SIDEBAR_TOP_LAYOUT, HIDE_WITHOUT_REDRAW);
        delete_control_by_id(DIAL_SIDEBAR_TOP_LAYOUT);
    }
#endif
    // if (ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT)) {
    //     ui_hide_set(DIAL_SIDEBAR_BUTTON_LAYOUT, HIDE_WITHOUT_REDRAW);
    //     delete_control_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT);
    // }
    return 0;
}

int ui_cube_status();
int ui_hexagon_status();
int ui_reflection_status();
int ui_cube_reflection_status();
int watch_load_sidebar(struct element *elm)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_info("<%s> rets:%x\n", __func__, rets);
#if TCFG_UI_MOVE_MODE_CUBE
    if (ui_cube_status() && (ui_card_get_move_mode() == PAGE_MOVE_MODE_CUBE)) {
        return 0;
    }
#endif
#if TCFG_UI_MOVE_MODE_HEXAGON
    if (ui_hexagon_status() && (ui_card_get_move_mode() == PAGE_MOVE_MODE_HEXAGON)) {
        return 0;
    }
#endif
#if TCFG_UI_MOVE_MODE_REFLECTION
    if (ui_reflection_status() && (ui_card_get_move_mode() == PAGE_MOVE_MODE_REFLECTION)) {
        return 0;
    }
#endif
#if TCFG_UI_MOVE_MODE_CUBE_REFLECTION
    if (ui_cube_reflection_status() && (ui_card_get_move_mode() == PAGE_MOVE_MODE_CUBE_REFLECTION)) {
        return 0;
    }
#endif
    int root = 0;
    if (elm && elm->parent) {
        elm = elm->parent;
        if (elm->parent) {
            root = elm->parent->id;
        }
    }
    if (!root) {
        log_error("%s root error\n", __func__);
        return -1;
    }
    /* sidebar.sidebar_root = 0; */
    // if ((!(sidebar.sidebar_root & BIT(0))) && root && create_control_by_id(SIDEBAR_TAB_PATH, DIAL_SIDEBAR_LEFT_PAGE,  DIAL_SIDEBAR_LEFT_LAYOUT, root)) {
    //     sidebar.sidebar_root |= BIT(0);
    // } else {
    //     log_error("left_sidebar load fail! \n");
    // }
    mem_stats();
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
    if ((!(sidebar.sidebar_root & BIT(1))) && root && create_control_by_id(SIDEBAR_TAB_PATH, DIAL_SIDEBAR_TOP_PAGE, DIAL_SIDEBAR_TOP_LAYOUT, root)) {
        sidebar.sidebar_root |= BIT(1);
    } else {
        log_error("sidebar_menu load fail! \n");
    }
#endif

    // if ((!(sidebar.sidebar_root & BIT(2))) && root && create_control_by_id(SIDEBAR_TAB_PATH, DIAL_SIDEBAR_BUTTON_PAGE, DIAL_SIDEBAR_BUTTON_LAYOUT, root)) {
    //     sidebar.sidebar_root |= BIT(2);
    // } else {
    //     log_error("sidebar_layout load fail! \n");
    // }

    if (sidebar.sidebar_root) {
#if ((defined(TCFG_UI_MSG_NOTICE) && TCFG_UI_MSG_NOTICE) && (defined(NOTICE_LAYOUT) && NOTICE_LAYOUT))

        log_info("@@@@@@@@@@@@@ sidebar load succ:%x, register msg handler!\n", sidebar.sidebar_root);
        printf("@@@@@ win id: 0x%x\n", STYLE_DIAL_ID(WATCH));
        ui_register_msg_handler(DIAL_PAGE_0, ui_pd_menu_msg_handler);   // 注册消息交互的回调
#endif
    }
    return 0;
}

static int watch_dial_bgp_init(struct watch_param *param)
{
    if (dial_preview && dial_preview->is_dial_preview) {     // 表盘预览不加载bgp
        return 0;
    }
    u32 flag;
    char *bg_path;
    bg_path = watch_bgp_get_related_path(watch_get_style());
    log_debug("cur watch style %d, bgp_path:%s\n\n\n\n\n", watch_get_style(), bg_path);
    if (bg_path) {
        param->view_file = res_fopen(bg_path, "r");
        if (!param->view_file) {
            log_warn("bgp_not_find:%s\n", bg_path);
            return -1;
        }
        if (UI_DATA_STORE_IN_NORFLASH) {
            ui_res_flash_info_get(&param->view_file_info, bg_path, "res", false);
        }

        res_fread(param->view_file, &flag, sizeof(flag));
        log_debug("flag : 0x%x\n", flag);
    }
    return 0;
}

static int watch_dial_bgp_show(struct watch_param *param, struct ui_watch *watch, struct draw_context *dc)
{
    if (param->view_file) {
        watch->elm.css.background_image = 1;
        dc->preview.file = param->view_file;
        dc->preview.file_info = &param->view_file_info;
        dc->preview.id = 1;
        dc->preview.page = 0;
    }
    return 0;
}

static int watch_dial_bgp_deinit(struct watch_param *param)
{
    if (param->view_file) {
        ui_res_flash_info_free(&param->view_file_info, "res");
        res_fclose(param->view_file);
        param->view_file = NULL;
    }
    return 0;
}

/*****************************************************************/
/* 匀速表盘设置，默认是跳秒也就是不打开，如果打开 slow_sec = 1,
 * 定时器间隔不能修改，必须是166ms定时，因为 360度 / 60 = 6度每秒，然后
 * 1000ms / 6度 = 166ms,也就是每转动1度需要166ms */
/*****************************************************************/
static int WATCH_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_watch *watch = (struct ui_watch *)ctr;
    struct draw_context *dc = (struct draw_context *)arg;
    struct element *elm = (struct element *)ctr;

    struct sys_time time;
    /* int timer_interval = 0; */
    u8 slow_sec = 0;
    slow_sec = 1;
    /* printf("%s %d", __func__, e); */
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        if (ui_card_get_status()) {
            break;
        }
        // struct element * curr_elm = ui_core_get_element_by_id(DIAL_PAGE_0);
        // r_printf("%s %d %d %d ", __func__, __LINE__, curr_elm->css.left, ui_card_get_status());
        // int start_pos  = 512;
        // int end_pos =  256;
        // ui_anim_t a={0};
        // ui_anim_init(&a);
        // ui_anim_set_var(&a, elm->parent->id);
        // ui_anim_set_path_cb(&a, ui_anim_path_ease_out);
        // ui_anim_set_exec_cb(&a, watch_enter_anim_cb2);
        // ui_anim_set_values(&a, start_pos, end_pos);
        // ui_anim_set_time(&a, 80);
        // ui_anim_start(&a);
        break;
    case ON_CHANGE_INIT:
#if ((defined(TCFG_UI_MSG_NOTICE) && TCFG_UI_MSG_NOTICE) && (defined(NOTICE_LAYOUT) && NOTICE_LAYOUT))
        create_control_by_menu_set(0);
#endif
        ui_card_enable();
        rtc_read_time(&time);
        ui_watch_set_time(watch, time.hour % 12, time.min, time.sec, 0);
        ui_set_default_handler(elm, watch_child_ontouch, watch_child_onkey, watch_child_onchange);
        /* slow_sec = 1; */
        /* ui_watch_slow_sec_by_id(STYLE_DIAL_ID(WATCH), slow_sec); */

        if (slow_sec == 1) {
            timer_interval = 166;
        } else {
            timer_interval = 500;
        }

        if (!watch_show_timer) {
            watch_show_timer = sys_timer_add(NULL, WATCH_timer, timer_interval);
        }

        ui_page_move_en_callback(is_ui_page_move_enable);
        watch_load_sidebar(elm);
        watch_dial_bgp_init(&dial_param);
        ui_auto_shut_down_enable();
        break;
    case ON_CHANGE_SHOW:
#if ((defined TCFG_UI_BG_ENABLE && TCFG_UI_BG_ENABLE) && \
            (defined TCFG_UI_COMMON_BACKGROUND_SWITCH_ENABLE && TCFG_UI_COMMON_BACKGROUND_SWITCH_ENABLE))
        csbg_show(&watch->elm, dc, CSBG_TYPE_LOCKSCREEN_WALLPAPER);
#else
        watch_dial_bgp_show(&dial_param, watch, dc);
#endif
        if (sidebar.sidebar_status == SIDEBAR_HIDE) {
            // ui_clear_gpu_task(dc, ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT));
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
            ui_clear_gpu_task(dc, ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT));
#endif
            // ui_clear_gpu_task(dc, ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT));
        }

        break;
    case ON_CHANGE_RELEASE:
        sidebar.sidebar_root = 0;
        if (watch_show_timer) {
            sys_timer_del(watch_show_timer);
            watch_show_timer = 0;
        }
        ui_set_default_handler(elm, NULL, NULL, NULL);
        watch_unload_sidebar(elm);
        watch_dial_bgp_deinit(&dial_param);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}


static int ui_x_move(struct element *elm, int x_offset)
{
    struct element_css *parent_css;
    struct element_css *css;
    struct rect rect;
    struct rect parent_rect;
    int css_left;
    int percent;
    if (!elm) {
        return 0;
    }

    ui_core_get_element_abs_rect(elm->parent, &parent_rect);
    ui_core_get_element_abs_rect(elm, &rect);
    rect.left = 0;

    css_left = x_offset;// * 10000 / parent_rect.width;
    log_info("%s %d %d", __func__, x_offset, css_left);
    css = ui_core_get_element_css(elm);
    css->left += css_left;
    percent = (css->left + css->width) * 100 / css->width;
    percent = (percent > 100) ? 100 : percent;
    percent = (percent < 0) ? 0 : percent;

    if (css->left > 0) {
        css->left = 0;
        /* ui_core_redraw_area(&rect); */
        ui_core_redraw(elm->parent);
        return 0;
    }

    if (css->left < -css->width) {
        css->left = -css->width;
        /* ui_core_redraw_area(&rect); */
        ui_core_redraw(elm->parent);
        return 0;
    }
    elm->css.alpha = percent;

    /* ui_core_redraw_area(&rect); */
    ui_core_redraw(elm->parent);

    return 1;
}

static int ui_y_move(struct element *elm, int y_offset, u8 dir)
{
    struct element_css *parent_css;
    struct element_css *css;
    struct rect rect;
    struct rect parent_rect;
    int css_top;
    u32 percent;
    int ret = 1;

    if (!elm) {
        return 0;
    }

    ui_core_get_element_abs_rect(elm->parent, &parent_rect);
    ui_core_get_element_abs_rect(elm, &rect);
    rect.top = 0;

    css_top = y_offset ;//* 10000 / parent_rect.height;

    css = ui_core_get_element_css(elm);
    css->top += css_top;
    if (dir) {
        if (css->top > 0) {
            css->top = 0;
            ret = 0;
        } else if (css->top < -css->height) {
            css->top = -css->height;
            ret = 0;
        }
        percent = (css->top + css->height) * 100 / css->height;

    } else {
        if (css->top < 0) {
            css->top = 0;
            ret = 0;
        } else if (css->top > css->height) {
            css->top = css->height;
            ret = 0;
        }
        percent = (css->height - css->top) * 100 / css->height;

    }

    elm->css.alpha = percent;
    ui_core_redraw_area(&rect);
    ui_core_redraw(elm->parent);

    return ret;
}

u8 is_ui_page_move_enable(void)
{
#if (defined TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE) && TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE
    return 1;
#endif
    /* printf("%s stop:%d statsu:%d ", __func__, sidebar.stop, sidebar.sidebar_status); */
    return (sidebar.stop && (sidebar.sidebar_status == SIDEBAR_HIDE));
}
u8 is_ui_sidebar_show_top()
{
    return (sidebar.sidebar_status == SIDEBAR_SHOW_TOP);
}
u8 is_ui_sidebar_show_bottom()
{
    return (sidebar.sidebar_status == SIDEBAR_SHOW_BOTTOM);
}


static int WATCH_ontouch(void *_ctrl, struct element_touch_event *e)
{
    log_debug("%s event:%d", __func__, e->event);
    struct rect rect;
    struct element_css *css_top;
    struct element_css *css_bottom;
    struct element_css *css_left;

    if ((!sidebar.sidebar_root) && (e->event == ELM_EVENT_TOUCH_HOLD)) {
        return false;
    }

    // if (dial_dynamic_manager_ontouch(_ctrl, e)) {
    //     /*log_debug("%s %d",__func__,__LINE__); */
    //     return true;
    // }
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        /*如果有消息弹窗，这里要弹出消息弹窗*/
#if ((defined(TCFG_UI_MSG_NOTICE) && TCFG_UI_MSG_NOTICE) && (defined(NOTICE_LAYOUT) && NOTICE_LAYOUT))
        extern void notice_dialog_hide(int anim);
        notice_dialog_hide(true);
#endif

        if (get_need_password() == 1) {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(ID_WINDOW_POWERON_PASSWORD);
            return true;
        }

        /* log_info("%s %d move_dir:%d sidebar_status:%d stop:%d\n", __func__,__LINE__, sidebar.move_dir, sidebar.sidebar_status,sidebar.stop); */
        if (sidebar.stop == false) {
            return true;
        }
        memcpy(&sidebar.pos, &e->pos, sizeof(struct position));
        sidebar.first_x_offset = e->pos.x;
        sidebar.first_y_offset = e->pos.y;
        sidebar.move_dir = SLIDER_MOVE_NONE;
        sidebar.move_pos.x = 0;
        sidebar.move_pos.y = 0;
        //struct element *elm_left =  ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT);
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
        struct element *elm_top = ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT);
#endif
        //struct element *elm_btm = ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT);
#if TCFG_UI_ENABLE_LEFT_MENU
        struct element *elm_btm = ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT);
        struct element *elm_left =  ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT);
        if (elm_left && elm_top && elm_btm) {
            css_left = ui_core_get_element_css(elm_left);
            css_bottom = ui_core_get_element_css(elm_btm);
            css_top = ui_core_get_element_css(elm_top);
            if ((css_left->left == -css_left->width) &&
                (css_top->top == -css_top->height) &&
                (css_bottom->top == css_bottom->height)) {
                sidebar.sidebar_status = SIDEBAR_HIDE;
            }
        }
#else
        // if (elm_top && elm_btm) {
        //     css_bottom = ui_core_get_element_css(elm_btm);
        //     css_top = ui_core_get_element_css(elm_top);
        //     if ((css_top->top == -css_top->height) &&
        //         (css_bottom->top == css_bottom->height)) {
        //         sidebar.sidebar_status = SIDEBAR_HIDE;
        //     }
        // }
#endif

        sidebar.stop = true;
        sidebar.step = 0;

        /* log_info("%s %d move_dir:%d sidebar_status:%d stop:%d\n", __func__,__LINE__, sidebar.move_dir, sidebar.sidebar_status,sidebar.stop); */
        return true;

    case ELM_EVENT_TOUCH_MOVE:

        sidebar.x_offset = e->pos.x - sidebar.pos.x;
        sidebar.y_offset = e->pos.y - sidebar.pos.y;

        sidebar.move_pos.x += sidebar.x_offset;
        sidebar.move_pos.y += sidebar.y_offset;
        //判断滑动方向
        if (sidebar.move_dir == SLIDER_MOVE_NONE) {
            if ((abs(sidebar.move_pos.x) > DIAL_SIDEBAR_START_THRESHOLD) || (abs(sidebar.move_pos.y) > DIAL_SIDEBAR_START_THRESHOLD)) {
                //取分量大的一方作为当前滑动方向
                sidebar.move_dir = (abs(sidebar.move_pos.x) > abs(sidebar.move_pos.y)) ? SLIDER_MOVE_LF : SLIDER_MOVE_UD;
            }
        }
        memcpy(&sidebar.pos, &e->pos, sizeof(struct position));
        if (sidebar.move_dir == SLIDER_MOVE_LF) {
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {
                if (sidebar.first_x_offset <= DIAL_SIDEBAR_LEFT_MIN_THRESHOLD) {

                    sidebar.stop = false;
                    // ui_x_move(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), sidebar.x_offset);
                }
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_LEFT) {

                sidebar.stop = false;
                // ui_x_move(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), sidebar.x_offset);
            }
        } else if (sidebar.move_dir == SLIDER_MOVE_UD) {
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {
                if (sidebar.first_y_offset <= DIAL_SIDEBAR_MIN_THRESHOLD) {

                    sidebar.stop = false;
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
                    ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), sidebar.y_offset, 1);
#endif
                } else if (sidebar.first_y_offset >= DIAL_SIDEBAR_MAX_THRESHOLD) {

                    sidebar.stop = false;
                    // ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), sidebar.y_offset, 0);
                }
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_TOP) {

                sidebar.stop = false;
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
                ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), sidebar.y_offset, 1);
#endif
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_BOTTOM) {
                sidebar.stop = false;

                // ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), sidebar.y_offset, 0);
            }
        }

        /* log_info("%s %d move_dir:%d sidebar_status:%d stop:%d\n", __func__,__LINE__, sidebar.move_dir, sidebar.sidebar_status,sidebar.stop); */
        break;
    case ELM_EVENT_TOUCH_HOLD:
        log_info("%s %d hold", __func__, __LINE__);
        UI_SHOW_WINDOW(ID_WINDOW_LOCK_SELECT);
        return true;
    case ELM_EVENT_TOUCH_UP:
        //已经在滑动中的不处理
        if ((sidebar.sidebar_status == SIDEBAR_SHOWING_TOP) ||
            (sidebar.sidebar_status == SIDEBAR_SHOWING_BOTTOM) ||
            (sidebar.sidebar_status == SIDEBAR_SHOWING_LEFT)) {
            return true;
        }

        /* log_info("%s %d move_dir:%d sidebar_status:%d stop:%d\n", __func__,__LINE__, sidebar.move_dir, sidebar.sidebar_status,sidebar.stop); */
        if (sidebar.move_dir == SLIDER_MOVE_LF) {
            sidebar.x_offset = e->pos.x - sidebar.pos.x;
            memcpy(&sidebar.pos, &e->pos, sizeof(struct position));
            /*刷新最后一次*/
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {
                if (sidebar.first_x_offset <= DIAL_SIDEBAR_LEFT_MIN_THRESHOLD) {
                    //ui_x_move(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), sidebar.x_offset);
                }
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_LEFT) {
                //ui_x_move(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), sidebar.x_offset);
            }

            //ui_core_get_element_abs_rect(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), &rect);
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {//表盘显示状态
                if (rect.left + rect.width >= DIAL_SIDEBAR_LEFT_RUN_MIN_THRESHOLD) {
                    sidebar.step = DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_LEFT;
                } else {//否则收回
                    sidebar.step = -DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_LEFT;
                }
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_LEFT) {//左侧边栏显示状态
                if (rect.left + rect.width <= DIAL_SIDEBAR_LEFT_RUN_MAX_THRESHOLD) {
                    sidebar.step = -DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_LEFT;
                } else {
                    sidebar.step = DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_LEFT;
                }
            }
        } else if (sidebar.move_dir == SLIDER_MOVE_UD) {
            sidebar.y_offset = e->pos.y - sidebar.pos.y;
            memcpy(&sidebar.pos, &e->pos, sizeof(struct position));
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {
                if (sidebar.first_y_offset <= DIAL_SIDEBAR_MIN_THRESHOLD) {
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
                    ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), sidebar.y_offset, 1);
#endif
                } else if (sidebar.first_y_offset >= DIAL_SIDEBAR_MAX_THRESHOLD) {
                    //ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), sidebar.y_offset, 0);
                }
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_TOP) {
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
                ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), sidebar.y_offset, 1);
#endif
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_BOTTOM) {
                //ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), sidebar.y_offset, 0);
            }
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {
                if (sidebar.first_y_offset <= DIAL_SIDEBAR_MIN_THRESHOLD) {
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
                    ui_core_get_element_abs_rect(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), &rect);
                    if (rect.top + rect.height >= DIAL_SIDEBAR_RUN_MIN_THRESHOLD) {
                        sidebar.step = DIAL_SIDEBAR_STEP;
                        sidebar.stop = false;
                        sidebar.sidebar_status = SIDEBAR_SHOWING_TOP;
                    } else {
                        sidebar.step = -DIAL_SIDEBAR_STEP;
                        sidebar.stop = false;
                        sidebar.sidebar_status = SIDEBAR_SHOWING_TOP;
                    }
#endif
                } else if (sidebar.first_y_offset >= DIAL_SIDEBAR_MAX_THRESHOLD) {
                    //ui_core_get_element_abs_rect(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), &rect);
                    if (rect.top <= DIAL_SIDEBAR_RUN_MAX_THRESHOLD) {
                        sidebar.step = -DIAL_SIDEBAR_STEP;
                        sidebar.stop = false;
                        sidebar.sidebar_status = SIDEBAR_SHOWING_BOTTOM;
                    } else {
                        sidebar.step = DIAL_SIDEBAR_STEP;
                        sidebar.stop = false;
                        sidebar.sidebar_status = SIDEBAR_SHOWING_BOTTOM;
                    }
                } else {
                    sidebar.stop = true;
                }
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_TOP) {
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
                ui_core_get_element_abs_rect(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), &rect);
                if (rect.top + rect.height <= DIAL_SIDEBAR_RUN_MAX_THRESHOLD) {
                    sidebar.step = -DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_TOP;
                } else {
                    sidebar.step = DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_TOP;
                }
#endif
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_BOTTOM) {
                //ui_core_get_element_abs_rect(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), &rect);
                if (rect.top >= DIAL_SIDEBAR_MIN_THRESHOLD) {
                    sidebar.step = DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_BOTTOM;
                } else {
                    sidebar.step = -DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_BOTTOM;
                }
            }
        }
        return true;
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        // case ELM_EVENT_TOUCH_L_MOVE:
        // rlmove_flag = 1;
        extern void cpc_go_to_history_card();
        cpc_go_to_history_card();
        if (sidebar.sidebar_status == SIDEBAR_SHOW_TOP) {
            return true;
        }
        // g_printf("%s %d",__func__,__LINE__);
        // UI_WINDOW_BACK_SHOW(2);
        return true;
        break;

    default:
        break;
    }

    return false;
}


static int watch_onkey(void *ctrl, struct element_key_event *event)
{
    static u32 last_time = 0;
    switch (event->value) {
    case KEY_UI_MINUS: {
        u32 curr_msec = jiffies_msec();
        u32 offset = jiffies_msec2offset(last_time, curr_msec);
        if (offset < 60) {
            return false;
        }
        int item_num = watch_get_items_num();
        int sel_item = watch_get_style();
        sel_item --;
        if (sel_item < 0) {
            sel_item = item_num - 1;
        }
        watch_set_style(sel_item);
        UI_SHOW_WINDOW(ID_WINDOW_DIAL);
    }
    break;
    case KEY_UI_PLUS: {
        u32 curr_msec = jiffies_msec();
        u32 offset = jiffies_msec2offset(last_time, curr_msec);
        if (offset < 60) {
            return false;
        }
        int item_num = watch_get_items_num();
        int sel_item = watch_get_style();
        sel_item ++;
        if (sel_item >= item_num) {
            sel_item = 0;
        }
        watch_set_style(sel_item);
        UI_SHOW_WINDOW(ID_WINDOW_DIAL);
    }
    break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(STYLE_DIAL_ID(WATCH))
.onchange = WATCH_onchange,
 .onkey = NULL,
  .ontouch = WATCH_ontouch,
};


static void SIDEBAR_timer(void *priv)
{
    if (!sidebar.timer) {
        return ;
    }
    if (!sidebar.stop && sidebar.step) {
        if (sidebar.sidebar_status == SIDEBAR_SHOWING_LEFT) {
            // if (!ui_x_move(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), sidebar.step)) {
            //     if (sidebar.step > 0) {
            //         sidebar.sidebar_status = SIDEBAR_SHOW_LEFT;
            //     } else if (sidebar.step < 0) {
            //         sidebar.sidebar_status = SIDEBAR_HIDE;
            //     }
            //     sidebar.stop = true;
            //     sidebar.step = 0;
            // }
        }
        if (sidebar.sidebar_status == SIDEBAR_SHOWING_TOP) {
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
            if (!ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), sidebar.step, 1)) {
                if (sidebar.step > 0) {
                    sidebar.sidebar_status = SIDEBAR_SHOW_TOP;
                } else if (sidebar.step < 0) {
                    sidebar.sidebar_status = SIDEBAR_HIDE;
                }
                sidebar.stop = true;
                sidebar.step = 0;
            }
#endif
        } else if (sidebar.sidebar_status == SIDEBAR_SHOWING_BOTTOM) {
            // if (!ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), sidebar.step, 0)) {
            //     if (sidebar.step > 0) {
            //         sidebar.sidebar_status = SIDEBAR_HIDE;
            //         /* ui_core_element_on_focus(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), 0); */
            //     } else if (sidebar.step < 0) {
            //         sidebar.sidebar_status = SIDEBAR_SHOW_BOTTOM;
            //         /* ui_core_element_on_focus(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), 1); */
            //     }
            //     sidebar.stop = true;
            //     sidebar.step = 0;
            // }
        }
    }
}
#if(defined(DIAL_SIDEBAR_TOP_LAYOUT) && DIAL_SIDEBAR_TOP_LAYOUT)
static int sidebar_top_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    struct element_css *css;
    notice_layout_onchange(ctr, e, arg);
    switch (e) {
    case ON_CHANGE_INIT:
        if (!create_control_by_menu_get()) {
            css = ui_core_get_element_css(&layout->elm);
            css->top = -css->height;
            if (!sidebar.timer) {
                sidebar.timer = sys_timer_add(NULL, SIDEBAR_timer, 50);
            }
            sidebar.stop = true;
        }

        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_RELEASE:
        sidebar.sidebar_status = SIDEBAR_HIDE;
        if (sidebar.timer) {
            sys_timer_del(sidebar.timer);
            sidebar.timer = 0;
        }

        break;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(DIAL_SIDEBAR_TOP_LAYOUT)
.onchange = sidebar_top_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif


static int sidebar_left_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    struct element_css *css;

    switch (e) {
    case ON_CHANGE_INIT:
        css = ui_core_get_element_css(&layout->elm);
        css->left = -css->width;
        if (!sidebar.timer) {
            sidebar.timer = sys_timer_add(NULL, SIDEBAR_timer, 50);
        }
        sidebar.stop = true;
        break;
    case ON_CHANGE_SHOW:
        css = ui_core_get_element_css(&layout->elm);
        break;
    case ON_CHANGE_RELEASE:
        sidebar.sidebar_status = SIDEBAR_HIDE;
        if (sidebar.timer) {
            sys_timer_del(sidebar.timer);
            sidebar.timer = 0;
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
//REGISTER_UI_EVENT_HANDLER(DIAL_SIDEBAR_LEFT_LAYOUT)
//.onchange =  sidebar_left_onchange,
// .onkey = NULL,
//  .ontouch = NULL,
//};


int sidebar_button_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    struct element_css *css;
    switch (e) {
    case ON_CHANGE_INIT:
        css = ui_core_get_element_css(&layout->elm);
        css->top = css->height;
        if (!sidebar.timer) {
            sidebar.timer = sys_timer_add(NULL, SIDEBAR_timer, 50);
        }
        sidebar.stop = true;
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_RELEASE:
        sidebar.sidebar_status = SIDEBAR_HIDE;
        if (sidebar.timer) {
            sys_timer_del(sidebar.timer);
            sidebar.timer = 0;
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
/* REGISTER_UI_EVENT_HANDLER(DIAL_SIDEBAR_BUTTON_LAYOUT) */
/* .onchange = sidebar_button_onchange, */
/* .onkey = NULL, */
/* .ontouch = NULL, */
/* }; */

#define ENERGY_A			(-0.04)   	//负加速度
#define ENERGY_DIR_UL		1
#define ENERGY_VAL		    2



static void ui_dial_anim_cb(int var, int32_t v)
{
    static u32 msec_last  = 0;
    int steps = v - dial_preview->anim_tmp;
    int d_time = jiffies_msec() - msec_last;
    msec_last = jiffies_msec();
    log_debug("<<<<%s>>>> %d %d %d %d", __func__, steps, v, dial_preview->anim_tmp, d_time);
    dial_preview->anim_tmp = v;

    u32 curr_win = UI_GET_WINDOW_ID();
    struct element *curr_elm = ui_core_get_element_nowarning_by_id(curr_win);
    ASSERT(curr_elm);

    struct rect rect;
    jlgpu_get_win_rect(&rect);

    int jlgpu_scheduler_wait_sync();
    jlgpu_scheduler_wait_sync();

    for (int i = 0; i < dial_preview->dial_num; i++) {
        gpu_matrix_set_identity(&dial_preview->matrix);
        gpu_matrix_translate(&dial_preview->matrix, -rect.left, -rect.top);
        gpu_matrix_translate(&dial_preview->matrix, -(rect.width * i), 0);
        gpu_matrix_translate(&dial_preview->matrix, -dial_preview->pos_x_total, 0);
        gpu_matrix_translate(&dial_preview->matrix, -steps, 0);
        jlgpu_task_list_copy_mul_matrix_by_group(dial_preview->new_head, &dial_preview->matrix, i + 1);
    }

    memcpy(&dial_preview->new_dc_anim, curr_elm->dc, sizeof(struct draw_context));
    jlgpu_mult_task_head_modify_by_index(dial_preview->new_dc_anim.gpu_mult_list, dial_preview->new_dc_anim.index, dial_preview->new_head);

    dial_preview->pos_x_total += steps;

    struct rect lcdrect;

    lcdrect.left = 0;
    lcdrect.top = 0;
    lcdrect.width = curr_elm->dc->width;
    lcdrect.height = curr_elm->dc->height;

    struct ui_platform_api *platform_api = ui_get_platform_api();
    if (platform_api->put_draw_context) {
        struct rect rect_orig;
        memcpy(&rect_orig, &dial_preview->new_dc_anim.rect_orig, sizeof(struct rect));
        memcpy(&dial_preview->new_dc_anim.rect_orig, &lcdrect, sizeof(struct rect));
        platform_api->put_draw_context(&dial_preview->new_dc_anim);
        memcpy(&dial_preview->new_dc_anim.rect_orig, &rect_orig, sizeof(struct rect));
    }
}

static void ui_dial_anim_ready_callback(struct _ui_anim_t *_anim)
{
    int var = _anim->var;
}


// static int ui_static_dial_auto_center_and_flick()
// {
//     struct rect item_r;
//     struct rect grid_r;
//     jlgpu_get_win_rect(&grid_r);
//     /*抬手居中*/
//     int target_line = (grid_r.width / 2);

//     int check_mid = 0;

//     int x_interval = 0;
//     int y_interval = 0;
//     int item_mid;
//     int i;
//     int record_i;
//     int end_dist = 0;
//     int xoffset_static = dial_preview->pos_x_total;
//     /*判断最近的一项*/
//     for (i = 0; i < dial_preview->dial_num; i++) {
//         jlgpu_get_win_rect(&item_r);
//         item_r.top = 0;
//         if (i == 0) {
//             item_r.left = xoffset_static;
//             xoffset_static = item_r.left;
//         } else {
//             item_r.left = xoffset_static + item_r.width;
//             xoffset_static = item_r.left;
//         }
//         item_mid = item_r.left + item_r.width / 2;
//         if ((abs(item_mid - target_line) < abs(check_mid - target_line)) || (i == 0)) {
//             check_mid = item_mid;
//             record_i = i;
//         }
//     }
//     log_debug("<<<%s>>> i:%d check_mid %d target_line:%d", __func__, record_i, check_mid, target_line);
//     end_dist = -1 * (check_mid - target_line);
//     int center_i = -1;
//     if (end_dist) {
//         int end_time = end_dist * 2;//ms
//         dial_preview->anim_tmp = 0;
//         ui_anim_del(DIAL_SEL_LIST, NULL);
//         ui_anim_init(&dial_preview->anim);
//         ui_anim_set_var(&dial_preview->anim, DIAL_SEL_LIST);
//         ui_anim_set_path_cb(&dial_preview->anim, ui_anim_path_ease_out);
//         ui_anim_set_exec_cb(&dial_preview->anim, ui_dial_anim_cb);
//         ui_anim_set_values(&dial_preview->anim, 0, end_dist);
//         ui_anim_set_time(&dial_preview->anim, abs(end_time));
//         ui_anim_start(&dial_preview->anim);
//     }
//     return true;
// }

// static int ui_dial_anim_touch_deal(struct element_touch_event *e)
// {
//     struct rect item_r;
//     if (e->event == ELM_EVENT_TOUCH_ENERGY) {
//         //拆分惯性参数
//         int dist_x = e->pos.x >> 16;
//         int dist_y = e->pos.y >> 16;
//         int energy_t0 = (e->pos.x + 1) & 0xffff; //防止div0
//         float vx0 = abs(2 * (float)dist_x / energy_t0);	//水平初速度
//         float vy0 = abs(2 * (float)dist_y / energy_t0); //垂直初速度
//         int xdir = e->pos.y & 0xff;						//水平滑动方向
//         int ydir = (e->pos.y >> 8) & 0xff;				//垂直滑动方向
//         float eng_a  = ENERGY_A;						//负加速度

//         int end_dist = 0;								//惯性移动距离
//         int end_time = 0;								//惯性时间
//         float v0 = 0;									//速度
//         int dir = 0;									//方向
//         int max_dist = 0;								//最大惯性移动距离
//         int center_index = -1;
//         struct rect grid_r;
//         jlgpu_get_win_rect(&grid_r);

//         int target_line = (grid_r.width / 2);

//         v0 = vx0;
//         dir = (xdir == ENERGY_DIR_UL) ? -1 : 1;

//         end_dist = abs(v0 * v0 / eng_a / 2);
//         log_debug("end_dist:%d dir:%d", end_dist, dir);

//         int check_mid = 0;
//         int item_mid = 0;							//项的中线
//         int item_dist = 0;							//当前项与目标线的距离
//         int i;
//         int record_i;
//         int x_interval = 0;
//         int y_interval = 0;
//         jlgpu_get_win_rect(&item_r);
//         int item_width_height = (x_interval + item_r.width);
//         end_dist = dir * end_dist;
//         int xoffset_static = dial_preview->pos_x_total;
//         //遍历所有项 查找最靠近目标线的项
//         for (i = 0; i < dial_preview->dial_num ; i++) {
//             jlgpu_get_win_rect(&item_r);
//             item_r.top = 0;
//             if (i == 0) {
//                 item_r.left = xoffset_static;
//                 xoffset_static = item_r.left;
//             } else {
//                 item_r.left = xoffset_static + item_r.width;
//                 xoffset_static = item_r.left;
//             }
//             item_mid = item_r.left + item_r.width / 2 ;
//             log_debug("<<<%s>>>%d itemmid:%d check:%d target:%d (%d<%d)", __func__, i, item_mid, check_mid, target_line, abs(item_mid - target_line + end_dist), abs(check_mid - target_line + end_dist));
//             if ((abs(item_mid - target_line + end_dist) < abs(check_mid - target_line + end_dist)) || (i == 0)) {
//                 log_debug("%s %d %d", __func__, __LINE__, i);
//                 check_mid = item_mid;
//                 record_i = i;
//             }
//         }
//         end_dist = check_mid  - target_line;
//         dir = end_dist > 0 ? -1 : 1;
//         end_dist = abs(end_dist);
//         /*前面的距离都是绝对值*/
//         end_dist *= dir;
//         /*计算回弹所需要的时间 4是经验值*/
//         end_time = ENERGY_VAL * root_float(abs(2 * end_dist / eng_a));

//         log_debug("%s v0%f dir:%d enga:%f dist:%d max_dist:%d  time:%d",
//                   __func__, v0, dir, eng_a, end_dist, max_dist, end_time);
//         /*清除记录参数*/
//         dial_preview->anim_tmp = 0;
//         ui_anim_del(DIAL_SEL_LIST, NULL);
//         /*重新开始配置惯性*/
//         ui_anim_init(&dial_preview->anim);
//         ui_anim_set_var(&dial_preview->anim, DIAL_SEL_LIST);

//         ui_anim_set_path_cb(&dial_preview->anim,  ui_anim_path_ease_out);
//         ui_anim_set_exec_cb(&dial_preview->anim, ui_dial_anim_cb);
//         ui_anim_set_ready_cb(&dial_preview->anim, ui_dial_anim_ready_callback);

//         ui_anim_set_values(&dial_preview->anim, 0, end_dist);
//         ui_anim_set_time(&dial_preview->anim, end_time);
//         ui_anim_start(&dial_preview->anim);

//     } else if (e->event == ELM_EVENT_TOUCH_UP) {
//         return ui_static_dial_auto_center_and_flick();
//     }
//     return 0;
// }


extern void rcsp_extra_flash_opt_dial_nodify(void);
extern u8 call_ctrl_get_status(void);

static int dial_view_init(int p)
{

    struct draw_context *new_dc = zalloc(sizeof(struct draw_context));
    ASSERT(new_dc);

    dial_preview->dial_num = watch_get_items_num();
    u32 win[dial_preview->dial_num];
    char *watch_name[dial_preview->dial_num];
    for (int i = 0; i < dial_preview->dial_num; i++) {
        win[i] = ID_WINDOW_DIAL;
        watch_name[i] = zalloc(sizeof(char) * 100);
        ASSERT(watch_name[i]);
    }
    u8 curr_style = watch_get_style();
    int name_len = strlen(watch_get_item(curr_style));
    strcpy(watch_name[0], watch_get_item(curr_style));
    dial_preview->dial_index[0] = curr_style;

    u8 dial_name_position = 1;
    for (int i = 0; i < dial_preview->dial_num; i++) {
        if (curr_style == i) {
            continue;
        }
        name_len = strlen(watch_get_item(i));
        strcpy(watch_name[dial_name_position], watch_get_item(i));
        dial_preview->dial_index[dial_name_position] = i;
        dial_name_position++;
    }


    struct element *win_elm;
    u32 curr_win = UI_GET_WINDOW_ID();
    struct element *curr_elm = ui_core_get_element_nowarning_by_id(curr_win);
    dial_preview->this_curr_elm = curr_elm;
    pJLGPUTaskHead_t head = curr_elm->dc->gpu_task_head;
    dial_preview->new_head = jlgpu_task_list_copy_create(NULL, head, NULL, 0);

    memcpy(new_dc, curr_elm->dc, sizeof(struct draw_context));

    for (int i = 0; i < dial_preview->dial_num; i++) {
        ui_set_sty_path_by_pj_id(1, NULL);
        ui_set_sty_path_by_pj_id(1, (u8 *)watch_name[i]);
        win_elm = ui_core_get_element_nowarning_by_id(win[i]);
        if (!win_elm) {
            int ret = window_init(win[i]);
            ASSERT(!ret);
            if (!ret) {
                win_elm = ui_core_get_element_nowarning_by_id(win[i]);
                struct element *curr_elm_layer;
                list_for_each_child_element(curr_elm_layer, win_elm) {
                    ui_core_set_element_ratio(curr_elm_layer, 0.7f, 0.7f, 1);
                }
                win_elm->dc->refresh = false;
                ui_core_show(win_elm, true);
                jlgpu_task_list_copy_create(dial_preview->new_head, win_elm->dc->gpu_task_head, NULL, i + 1);
                ui_hide(win[i]);
            }
        } else {
            jlgpu_task_list_copy_create(dial_preview->new_head, win_elm->dc->gpu_task_head, NULL, i + 1);

        }
    }
    struct rect rect;
    jlgpu_get_win_rect(&rect);

    // 第一组
    for (int i = 0; i < dial_preview->dial_num; i++) {
        gpu_matrix_set_identity(&dial_preview->matrix);
        gpu_matrix_translate(&dial_preview->matrix, -rect.left, -rect.top);
        gpu_matrix_translate(&dial_preview->matrix, (-rect.width * i), 0);
        jlgpu_task_list_copy_mul_matrix_by_group(dial_preview->new_head, &dial_preview->matrix, i + 1);
    }
    int ret = jlgpu_mult_task_head_modify_by_index(new_dc->gpu_mult_list, new_dc->index, dial_preview->new_head);

    struct rect lcdrect;

    lcdrect.left = 0;
    lcdrect.top = 0;
    lcdrect.width = curr_elm->dc->width;
    lcdrect.height = curr_elm->dc->height;

    struct ui_platform_api *platform_api = ui_get_platform_api();
    if (platform_api->put_draw_context) {
        struct rect rect_orig;
        memcpy(&rect_orig, &new_dc->rect_orig, sizeof(struct rect));
        memcpy(&new_dc->rect_orig, &lcdrect, sizeof(struct rect));
        platform_api->put_draw_context(new_dc);
        memcpy(&new_dc->rect_orig, &rect_orig, sizeof(struct rect));
    }

    for (int i = 0; i < dial_preview->dial_num; i++) {
        free(watch_name[i]);
    }

    free(new_dc);

    return 0;
}

// static int dial_sel_list_onchange(void *_ctrl, enum element_change_event event, void *arg)
// {
//     switch (event) {
//     case ON_CHANGE_INIT:
//         if (!dial_preview) {
//             dial_preview = zalloc(sizeof(struct dial_preview_t));
//         }
//         dial_preview->is_dial_preview = 1;

//         ui_set_call(dial_view_init, 0);
//         break;
//     case ON_CHANGE_RELEASE:
//         if (jlgpu_scheduler_wait_sync() == -OS_TIMEOUT) {
//             log_error("<%s> ON_CHANGE_RELEASE error!", __func__);
//         }
//         dial_preview->is_dial_preview = 0;
//         jlgpu_task_list_copy_destroy(dial_preview->new_head);
//         struct element *curr_elm = dial_preview->this_curr_elm;
//         int ret = jlgpu_mult_task_head_modify_by_index(curr_elm->dc->gpu_mult_list, curr_elm->dc->index, curr_elm->dc->gpu_task_head);
//         ASSERT(!ret);
//         jlgpu_scheduler_set_redraw_mode(curr_elm->dc, GPU_ASYN_REDRAW);

//         char *watch_str = watch_get_item(dial_preview->dial_index[dial_preview->dial_sel_item]);
//         ui_set_sty_path_by_pj_id(1, NULL);
//         ui_set_sty_path_by_pj_id(1, (u8 *)watch_str);
//         ui_res_switch_watch_phy_addr(watch_str);

//         if (dial_preview) {
//             free(dial_preview);
//             dial_preview = NULL;
//         }
//         break;
//     default:
//         break;
//     }
//     return 0;
// }


// static int dial_sel_list_ontouch(void *ctr, struct element_touch_event *e)
// {
//     struct layout *layout = (struct layout *)ctr;
//     static u8 touch_action = 0;
//     u32 curr_win = UI_GET_WINDOW_ID();
//     struct element *curr_elm = ui_core_get_element_nowarning_by_id(curr_win);
//     struct rect rect;
//     jlgpu_get_win_rect(&rect);

//     ui_dial_anim_touch_deal(e);

//     switch (e->event) {
//     case ELM_EVENT_TOUCH_ENERGY:
//         return true;
//         break;
//     case ELM_EVENT_TOUCH_UP:
//         if (touch_action != 1) {
//             return true;
//         }

//         if (((-dial_preview->pos_x_total) % rect.width) != 0) {
//             return true;
//         }
//         dial_preview->dial_sel_item = (-(dial_preview->pos_x_total)) / rect.width;
//         watch_set_style(dial_preview->dial_index[dial_preview->dial_sel_item]);

//         UI_SHOW_WINDOW(ID_WINDOW_DIAL);
//         if (call_ctrl_get_status() == BT_CALL_HANGUP) {
// #if TCFG_DEV_MANAGER_ENABLE && RCSP_FILE_OPT
//             rcsp_extra_flash_opt_dial_nodify();
// #endif
//         }

//         return true;
//         break;
//     case ELM_EVENT_TOUCH_DOWN:
//         ui_anim_del(DIAL_SEL_LIST, NULL);
//         touch_action = 1;
//         dial_preview->pos_x = e->pos.x;
//         break;
//     case ELM_EVENT_TOUCH_MOVE:
//         jlgpu_scheduler_wait_sync();

//         touch_action = 2;

//         dial_preview->xoffset = e->pos.x - dial_preview->pos_x;
//         if (!dial_preview->xoffset) {
//             break;
//         }
//         dial_preview->pos_x = e->pos.x;

//         dial_preview->pos_x_total += dial_preview->xoffset;
//         if (dial_preview->pos_x_total + dial_preview->xoffset < -(rect.width * (dial_preview->dial_num - 1))) {
//             dial_preview->pos_x_total = -(rect.width * (dial_preview->dial_num - 1));
//         }
//         if (dial_preview->pos_x_total + dial_preview->xoffset > 0) {
//             dial_preview->pos_x_total = 0;
//         }

//         for (int i = 0; i < dial_preview->dial_num; i++) {
//             gpu_matrix_set_identity(&dial_preview->matrix);
//             gpu_matrix_translate(&dial_preview->matrix, -rect.left, -rect.top);
//             gpu_matrix_translate(&dial_preview->matrix, -(dial_preview->pos_x_total) - (rect.width * i), 0);
//             jlgpu_task_list_copy_mul_matrix_by_group(dial_preview->new_head, &dial_preview->matrix, i + 1);
//         }

//         memcpy(&dial_preview->new_dc_ontouch, curr_elm->dc, sizeof(struct draw_context));
//         jlgpu_mult_task_head_modify_by_index(dial_preview->new_dc_ontouch.gpu_mult_list, dial_preview->new_dc_ontouch.index, dial_preview->new_head);

//         struct rect lcdrect;

//         lcdrect.left = 0;
//         lcdrect.top = 0;
//         lcdrect.width = curr_elm->dc->width;
//         lcdrect.height = curr_elm->dc->height;

//         struct ui_platform_api *platform_api = ui_get_platform_api();
//         if (platform_api->put_draw_context) {
//             struct rect rect_orig;
//             memcpy(&rect_orig, &dial_preview->new_dc_ontouch.rect_orig, sizeof(struct rect));
//             memcpy(&dial_preview->new_dc_ontouch.rect_orig, &lcdrect, sizeof(struct rect));
//             platform_api->put_draw_context(&dial_preview->new_dc_ontouch);
//             memcpy(&dial_preview->new_dc_ontouch.rect_orig, &rect_orig, sizeof(struct rect));
//         }
//         return true;
//         break;
//     case ELM_EVENT_TOUCH_R_MOVE:
//     case ELM_EVENT_TOUCH_L_MOVE:
//         return true;
//         break;
//     default:
//         break;
//     }
//     return true;
// }

static int dial_sel_list_onchange(void *ctr, enum element_change_event e, void *arg)
{
    char *sty_suffix = ".sty";
    char *view_suffix = ".view";
    char *watch_item;
    u32 tmp_strlen;
    u32 sty_strlen;
    char tmp_name[100];
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int items_num = watch_get_items_num();
    switch (e) {
    case ON_CHANGE_INIT_PROBE:

        items_num = (items_num >= MAX_GRID_ITEM) ? MAX_GRID_ITEM : items_num;

        /* printf("grid num %d\n", items_num); */
        ui_grid_set_item_num(grid, items_num);//修改列表条目数量,不能超过列表的最大数量
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_LR);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        if (view_file) {
            free(view_file);
        }
        view_file = (VIEWFILE *) zalloc(sizeof(VIEWFILE) * items_num);
        ASSERT(view_file);

        sty_strlen = strlen(sty_suffix);
        for (int sel_item = 0; sel_item < items_num; sel_item++) {
            watch_item = watch_get_item(sel_item);
            if (watch_item == NULL) {
                break;
            }
            tmp_strlen = strlen(watch_item);
            strcpy(tmp_name, watch_item);
            strcpy(&tmp_name[tmp_strlen - sty_strlen], view_suffix);
            tmp_name[tmp_strlen - sty_strlen + strlen(view_suffix)] = '\0';
            /* printf("prew name %s\n", tmp_name); */
            view_file[sel_item].file = res_fopen(tmp_name, "r");
            if (!view_file[sel_item].file) {
                printf("open_prewfile fail %s\n", tmp_name);
                return FALSE;
            }
            if (ui_res_flash_info_get(&view_file[sel_item].info, tmp_name, "res", true)) {
                printf("get_prewfile tab fail %s\n", tmp_name);
                return false;
            }
            /* printf("prew out\n"); */
        }
        break;
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_LR);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        // ui_grid_flick_ctrl_close(grid, 1);
        struct rect item_r;
        ui_core_get_element_abs_rect((struct element *)&grid->item[0], &item_r);
        int item_width  = item_r.width + grid->x_interval;
        ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, -1 * watch_get_style()*item_width, NULL);
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_RELEASE:

        items_num = (items_num >= MAX_GRID_ITEM) ? MAX_GRID_ITEM : items_num;
        if (view_file) {
            for (int  sel_item = 0; sel_item < items_num; sel_item++) {
                if (view_file[sel_item].file) {
                    res_fclose(view_file[sel_item].file);
                    ui_res_flash_info_free(&view_file[sel_item].info, "res");
                    view_file[sel_item].file = NULL;
                }
            }
            free(view_file);
            view_file = NULL;
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

#endif /* #if TCFG_UI_DIAL_ENABLE */
#endif

#endif
