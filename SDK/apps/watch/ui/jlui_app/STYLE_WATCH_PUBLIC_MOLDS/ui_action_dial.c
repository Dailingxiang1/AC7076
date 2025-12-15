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
#include "ui_page_switch.h"
#include "btstack/avctp_user.h"
#include "jlui_app/ui_app_effect.h"
#include "gpu_task.h"
#include "avi_video.h"
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


extern u8 get_avi_pause_status();
extern int ui_in_effect();
extern void *get_aviplay_handle();
extern void *get_avi_player_st_handle();
extern u16 avi_get_avi_playtimer_id();
extern void avi_set_avi_playtimer_id(u16 timer_id);
extern void avi_pause(void);

#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_DIAL_ENABLE


#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

#define SILIDE_TOUCH_FULL_SCREEN						1	//优化上下侧边栏滑动，跟手时不超过屏幕50%收回侧边栏，惯性时依据惯性方向滑动
#define	SCREEN_WIDTH									320
#define SCREEN_HEIGHT									386

#define DIAL_SIDEBAR_MAX_THRESHOLD      				(SCREEN_HEIGHT * 70 / 100)//区域判断
#define DIAL_SIDEBAR_MIN_THRESHOLD      				(SCREEN_HEIGHT * 30 / 100)//区域判断
#define DIAL_SIDEBAR_RUN_MAX_THRESHOLD  				(SCREEN_HEIGHT * 85 / 100)//滑动距离判断
#define DIAL_SIDEBAR_RUN_MIN_THRESHOLD  				(SCREEN_HEIGHT * 15 / 100)//滑动距离判断
#define DIAL_SIDEBAR_LEFT_MAX_THRESHOLD      			(SCREEN_WIDTH  * 70 / 100)//区域判断
#define DIAL_SIDEBAR_LEFT_MIN_THRESHOLD      			(SCREEN_WIDTH  * 100 / 100)//区域判断
#define DIAL_SIDEBAR_LEFT_RUN_MAX_THRESHOLD  			(SCREEN_WIDTH  * 85 / 100)//滑动距离判断
#define DIAL_SIDEBAR_LEFT_RUN_MIN_THRESHOLD  			(SCREEN_WIDTH  * 15 / 100)//滑动距离判断
#define DIAL_SIDEBAR_STEP               				(SCREEN_HEIGHT * 20 / 100)//滑动步进
#define DIAL_SIDEBAR_RUN_HALF_THRESHOLD					(SCREEN_HEIGHT * 50 / 100)//
#define ALPHA_MIN										(70)
#define ALPHA_MAX										(100)

#if SILIDE_TOUCH_FULL_SCREEN
#define DIAL_SIDEBAR_START_THRESHOLD      				(SCREEN_WIDTH  * 5 / 100)
#else
#define DIAL_SIDEBAR_START_THRESHOLD      				(SCREEN_WIDTH  * 15 / 100)
#endif

#define DIAL_SIDEBAR_TOP_LAYOUT							NOTICE_LAYOUT
#define DIAL_SIDEBAR_TOP_PAGE							PAGE_60
#define DIAL_SIDEBAR_BUTTON_LAYOUT						PULLUP_MENU_LAYOUT
#define DIAL_SIDEBAR_BUTTON_PAGE						PAGE_61
#define	DIAL_SIDEBAR_LEFT_LAYOUT						SIDEBAR
#define DIAL_SIDEBAR_LEFT_PAGE							PAGE_58
#define SIDEBAR_TAB_PATH								MODE_PATH"JL/JL.tab"
#define MAX_GRID_ITEM       (6)


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
u8 video_play_mode = 0;

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

#define abs(x)  ((x)>0?(x):-(x) )
static VIEWFILE *view_file = NULL;
static struct sidebar_priv sidebar = {0};
static u16 watch_show_timer = 0;

static u16 timer_interval = 0;
static u16 watch_msec = 0;	// 表盘毫秒计数
static struct watch_param dial_param;

extern MV_DRAW *avi_player;
extern int avi_playtimer;
extern void __jpeg_draw_cb_gpu(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv, void *matrix);
extern int dial_sel_state();
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
        /* printf("%04d-%02d-%02d %02d:%02d:%02d\n", time.year, time.month, time.day, time.hour, time.min, time.sec); */
        ui_watch_set_time((struct ui_watch *)ui_core_get_element_by_id(id), time.hour % 12, time.min, time.sec, watch_msec);
    } else {
        /* printf("%s %s", __func__, watch->source); */
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
    struct unumber num;
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
static int pic_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_SHOW_PROBE:
        if (!strcmp(pic->source, "watwek")) {
            struct sys_time time_t;
            rtc_read_time(&time_t);
            int week_index  = rtc_calculate_week_val(&time_t);
            ui_pic_set_image_index(pic, week_index); //0周日
        }
        break;
    case ON_CHANGE_RELEASE:
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
    notice_detail_control_onchange(elm, event, arg);
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
        /* printf("%s mode:%d hd:0x%x",__func__,ui_card_get_move_mode(),(u32)effmod_hd); */
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
        extern void dial_sel_update(int update);
        dial_sel_update(1);
        if (ui_card_get_move_mode() == PAGE_MOVE_MODE_CUBE) {
            cube_effect_update(timer_interval);
        } else if (ui_card_get_move_mode() == PAGE_MOVE_MODE_HEXAGON) {
            hexagon_effect_update(timer_interval);
        } else if (ui_card_get_move_mode() == PAGE_MOVE_MODE_REFLECTION) {
            reflection_effect_update(timer_interval);
        } else if (ui_card_get_move_mode() == PAGE_MOVE_MODE_CUBE_REFLECTION) {
            cube_reflection_effect_update(timer_interval);
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
int watch_unload_sidebar(struct element *elm)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    printf("<%s> rets:%x\n", __func__, rets);
    if (ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT)) {
        ui_hide_set(DIAL_SIDEBAR_LEFT_LAYOUT, HIDE_WITHOUT_REDRAW);
        delete_control_by_id(DIAL_SIDEBAR_LEFT_LAYOUT);
    }
    if (ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT)) {
        ui_hide_set(DIAL_SIDEBAR_TOP_LAYOUT, HIDE_WITHOUT_REDRAW);
        delete_control_by_id(DIAL_SIDEBAR_TOP_LAYOUT);
    }
    if (ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT)) {
        ui_hide_set(DIAL_SIDEBAR_BUTTON_LAYOUT, HIDE_WITHOUT_REDRAW);
        delete_control_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT);
    }
    sidebar.sidebar_root = 0;
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
    printf("<%s> rets:%x\n", __func__, rets);
    if (ui_cube_status() && (ui_card_get_move_mode() == PAGE_MOVE_MODE_CUBE)) {
        return 0;
    }
    if (ui_hexagon_status() && (ui_card_get_move_mode() == PAGE_MOVE_MODE_HEXAGON)) {
        return 0;
    }
    if (ui_reflection_status() && (ui_card_get_move_mode() == PAGE_MOVE_MODE_REFLECTION)) {
        return 0;
    }
    if (ui_cube_reflection_status() && (ui_card_get_move_mode() == PAGE_MOVE_MODE_CUBE_REFLECTION)) {
        return 0;
    }

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
    if ((!(sidebar.sidebar_root & BIT(0))) && root && create_control_by_id(SIDEBAR_TAB_PATH, DIAL_SIDEBAR_LEFT_PAGE,  DIAL_SIDEBAR_LEFT_LAYOUT, root)) {
        sidebar.sidebar_root |= BIT(0);
    } else {
        log_error("left_sidebar load fail! \n");
    }

    if ((!(sidebar.sidebar_root & BIT(1))) && root && create_control_by_id(SIDEBAR_TAB_PATH, DIAL_SIDEBAR_TOP_PAGE, DIAL_SIDEBAR_TOP_LAYOUT, root)) {
        sidebar.sidebar_root |= BIT(1);
    } else {
        log_error("sidebar_menu load fail! \n");
    }

    if ((!(sidebar.sidebar_root & BIT(2))) && root && create_control_by_id(SIDEBAR_TAB_PATH, DIAL_SIDEBAR_BUTTON_PAGE, DIAL_SIDEBAR_BUTTON_LAYOUT, root)) {
        sidebar.sidebar_root |= BIT(2);
    } else {
        log_error("sidebar_layout load fail! \n");
    }

    if (sidebar.sidebar_root) {
        log_info("sidebar load succ:%x\n", sidebar.sidebar_root);
        /* ui_register_msg_handler(DIAL_PAGE_0, ui_pd_menu_msg_handler);//注册消息交互的回调 */
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

extern char *watch_avi_get_related_path(u8 cur_watch);
static int watch_dial_bgp_init(struct watch_param *param)
{
    watch_dial_bgp_deinit(param);
    u32 flag;
    char *bg_path;
    u8 watch_mode_type = 0;
#if TCFG_VIDEO_DIAL_ENABLE
    AVI_PARAM avi_dial_param;
    syscfg_read(CFG_DIAL_TYPE_SEL, (u8 *)&watch_mode_type, 1);
    if (watch_mode_type == 1) {

        bg_path = watch_avi_get_related_path(watch_get_style());

        log_debug("cur watch style %d, bgp_path:%s\n\n\n\n\n", watch_get_style(), bg_path);
        /* bg_path=NULL; */

        if (bg_path) {
            avi_dial_param.is_dial = 1;
            avi_dial_param.is_audio_mute = 0;
            set_avi_play_mode(AVI_PLAY_LOOP);
            animig_open((char *)bg_path, avi_dial_param, 3); // 缓存3帧可以达到流畅播放30FPS
            avi_set_avi_playtimer_id(0);
            if (ui_in_effect() || dial_sel_state()) {
                if (get_aviplay_handle() != NULL) {
                    avi_pause();
                }
            }
        }

    } else
#endif
    {
        bg_path = watch_bgp_get_related_path(watch_get_style());
        if (bg_path) {
            param->view_file = res_fopen(bg_path, "r");
            if (!param->view_file) {
                log_warn("bgp_not_find:%s\n", bg_path);
                return -1;
            }
            if (UI_DATA_STORE_IN_NORFLASH) {
                ui_res_flash_info_get(&param->view_file_info, bg_path, "res", true);
            }

            res_fread(param->view_file, &flag, sizeof(flag));
            log_debug("flag : 0x%x\n", flag);
        }
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
#if TCFG_VIDEO_DIAL_ENABLE
    else if (get_aviplay_handle()) {
        watch->elm.css.background_image = 1;
        watch->elm.css.background_color = 0;
    }
#endif
    return 0;
}



/*****************************************************************/
/* 匀速表盘设置，默认是跳秒也就是不打开，如果打开 slow_sec = 1,
 * 定时器间隔不能修改，必须是166ms定时，因为 360度 / 60 = 6度每秒，然后
 * 1000ms / 6度 = 166ms,也就是每转动1度需要166ms */
/*****************************************************************/
void stop_watch_timer()
{
    if (watch_show_timer) {
        sys_timer_del(watch_show_timer);
        watch_show_timer = 0;
    }
}

void start_watch_timer()
{
    if (!watch_show_timer) {
        watch_show_timer = sys_timer_add(NULL, WATCH_timer, timer_interval);
    }
}
#if TCFG_VIDEO_DIAL_ENABLE
static void avi_timer(void *p)
{

    avi_set_avi_playtimer_id(0);
    // printf("timer");
    ui_redraw((int)p);
    wdt_clear();
    jlgpu_scheduler_wait_sync();
    wdt_clear();
}
#endif
static int WATCH_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_watch *watch = (struct ui_watch *)ctr;
    struct draw_context *dc = (struct draw_context *)arg;
    struct element *elm = (struct element *)ctr;

    struct sys_time time;
    /* int timer_interval = 0; */
    u8 slow_sec = 0;
#if TCFG_VIDEO_DIAL_ENABLE
    u16 avi_playtimer ;
    static u8 watch_mode_type = 0;
#endif
    /* printf("%s %d", __func__, e); */
    switch (e) {
    case ON_CHANGE_INIT:
        create_control_by_menu_set(0);

        ui_return_page_effect_enable(0, 0, 0, false);
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

        /* if (!watch_show_timer) { */
        /* watch_show_timer = sys_timer_add(NULL, WATCH_timer, timer_interval); */
        /* } */
        start_watch_timer();

        ui_page_move_en_callback(is_ui_page_move_enable);
        watch_load_sidebar(elm);
        watch_dial_bgp_init(&dial_param);
#if TCFG_VIDEO_DIAL_ENABLE
        syscfg_read(CFG_DIAL_TYPE_SEL, (u8 *)&watch_mode_type, 1);
#endif
        ui_auto_shut_down_enable();
        break;
#if TCFG_VIDEO_DIAL_ENABLE
    case ON_CHANGE_SHOW_POST:
        if (watch_mode_type == 1) {

            if (ui_in_effect() || ui_page_get_busy() || dial_sel_state()) {
                char *bg_path = watch_avi_get_related_path(watch_get_style());
                if (bg_path) {
                    extern int jpeg_image_file_psram(struct draw_context * dc, int left, int top, int width, int height, char *path, int path_len, int scale_en, float scale_f);
                    jpeg_image_file_psram(dc, 0, 0, 320, 384, bg_path, strlen(bg_path), 0, 1.0f);
                }
            }
        }

        // 保护措施，防止avi资源释放后访问空指针
        if (get_avi_pause_status()) {
            // printf("\n\n avi_player is already free!!!!!!!\n");
            return true;
            break;
        }
        avi_playtimer = avi_get_avi_playtimer_id();
#if SIDEBAR_ENABLE
        if (sidebar.sidebar_status == SIDEBAR_HIDE) {
            // printf("__func = %s, line = %d\n",__func__,__LINE__);
            avi_playtimer = sys_timeout_add((void *)elm->id, (avi_timer), 10); // 强制满帧刷新
        }
#else
        if (avi_playtimer == 0) {
            avi_playtimer = sys_timeout_add((void *)elm->id, (avi_timer), 10); // 强制满帧刷新
            avi_set_avi_playtimer_id(avi_playtimer);
        }
#endif
        u32 avip = (u32)(get_avi_player_st_handle());
        // printf("player->st %x %d %d ", avip, avi_get_width(player->st), avi_get_height(player->st));
        ui_draw(dc,
                NULL,
                0,
                0,
                avi_get_width((void *)avip),
                avi_get_height((void *)avip),
                __jpeg_draw_cb_gpu,
                &avip,
                sizeof(avip),
                1);
        break;
#endif
    case ON_CHANGE_SHOW:
        watch_dial_bgp_show(&dial_param, watch, dc);
        if (sidebar.sidebar_status == SIDEBAR_HIDE) {
            ui_clear_gpu_task(dc, ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT));
            ui_clear_gpu_task(dc, ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT));
            ui_clear_gpu_task(dc, ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT));
        }

        break;
    case ON_CHANGE_RELEASE:
        sidebar.sidebar_root = 0;
        /* if (watch_show_timer) { */
        /* sys_timer_del(watch_show_timer); */
        /* watch_show_timer = 0; */
        /* } */
        stop_watch_timer();
        ui_set_default_handler(elm, NULL, NULL, NULL);
#if TCFG_VIDEO_DIAL_ENABLE
        avi_play_shutdown();
#endif
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
    int ret = 1;
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
    /* printf("%s %d %d", __func__, x_offset, css_left); */
    css = ui_core_get_element_css(elm);
    css->left += css_left;
    percent = (css->left + css->width) * 100 / css->width;
    percent = (percent > 100) ? 100 : percent;
    percent = (percent < 0) ? 0 : percent;

    if (css->left > 0) {
        css->left = 0;
        /* ui_core_redraw_area(&rect); */
        /* ui_core_redraw(elm->parent); */
        /* return 0; */
        ret = 0;
    }

    if (css->left < -css->width) {
        css->left = -css->width;
        /* ui_core_redraw_area(&rect); */
        /* ui_core_redraw(elm->parent); */
        /* return 0; */
        ret = 0;
    }
    /* elm->css.alpha = percent; */
    elm->css.alpha = ALPHA_MIN + (ALPHA_MAX - ALPHA_MIN) * percent / 100;
    /* ui_effect_set_alpha(elm,elm->css.alpha,1,0); */
    /* ui_core_redraw_area(&rect); */
    ui_core_redraw(elm->parent);

    return ret;
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

    /* elm->css.alpha = percent; */
    elm->css.alpha = ALPHA_MIN + (ALPHA_MAX - ALPHA_MIN) * percent / 100;
    /* ui_effect_set_alpha(elm,elm->css.alpha,1,0); */
    ui_core_redraw_area(&rect);
    ui_core_redraw(elm->parent);

    return ret;
}

u8 is_ui_page_move_enable(void)
{
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
    /* printf("%s event:%d", __func__, e->event); */
    struct rect rect;
    struct element_css *css_top;
    struct element_css *css_bottom;
    struct element_css *css_left;

    extern int dial_sel_touch(struct element * elm, struct element_touch_event * e);
    if (dial_sel_touch((struct element *)_ctrl, e)) {
        return true;
    }

    if ((!sidebar.sidebar_root) && (e->event == ELM_EVENT_TOUCH_HOLD)) {
        return false;
    }

    switch (e->event) {
    case ELM_EVENT_TOUCH_ENERGY:
#if SILIDE_TOUCH_FULL_SCREEN
        int xdir = e->pos.y & 0xff;						//水平滑动方向
        int ydir = (e->pos.y >> 8) & 0xff;				//垂直滑动方向
        printf("%s xidr:%x ydir:%x status:%d", __func__, xdir, ydir, sidebar.sidebar_status);
        if (sidebar.move_dir == SLIDER_MOVE_UD) {
            if (ydir == 2) {
                sidebar.step = DIAL_SIDEBAR_STEP;
            } else {
                sidebar.step = -DIAL_SIDEBAR_STEP;
            }
        }
#endif
        break;
    case ELM_EVENT_TOUCH_DOWN:
        if (get_need_password() == 1) {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(ID_WINDOW_POWERON_PASSWORD);
            return true;
        }
#if TCFG_VIDEO_DIAL_ENABLE
        if (get_aviplay_handle() != NULL) {
            extern void avi_pause(void);
            avi_pause();
        }
#endif


        /* printf("%s %d move_dir:%d sidebar_status:%d stop:%d\n", __func__,__LINE__, sidebar.move_dir, sidebar.sidebar_status,sidebar.stop); */
        if (sidebar.stop == false) {
            return true;
        }
        memcpy(&sidebar.pos, &e->pos, sizeof(struct position));
        sidebar.first_x_offset = e->pos.x;
        sidebar.first_y_offset = e->pos.y;
        sidebar.move_dir = SLIDER_MOVE_NONE;
        sidebar.move_pos.x = 0;
        sidebar.move_pos.y = 0;
        struct element *elm_left =  ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT);
        struct element *elm_top = ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT);
        struct element *elm_btm = ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT);
#if TCFG_UI_ENABLE_LEFT_MENU
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
        if (elm_top && elm_btm) {
            css_bottom = ui_core_get_element_css(elm_btm);
            css_top = ui_core_get_element_css(elm_top);
            if ((css_top->top == -css_top->height) &&
                (css_bottom->top == css_bottom->height)) {
                sidebar.sidebar_status = SIDEBAR_HIDE;
            }
        }
#endif

        sidebar.stop = true;
        sidebar.step = 0;

        /* printf("%s %d move_dir:%d sidebar_status:%d stop:%d\n", __func__,__LINE__, sidebar.move_dir, sidebar.sidebar_status,sidebar.stop); */
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
#if SILIDE_TOUCH_FULL_SCREEN
        if (sidebar.move_dir == SLIDER_MOVE_UD) {
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {
                if (sidebar.move_pos.y > 0) {
                    sidebar.sidebar_status = SIDEBAR_SHOW_TOP;
                } else if (sidebar.move_pos.x  < 0) {
                    sidebar.sidebar_status = SIDEBAR_SHOW_BOTTOM;
                }
            }
        }
#endif
        if (sidebar.move_dir == SLIDER_MOVE_LF) {
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {
                if (sidebar.first_x_offset <= DIAL_SIDEBAR_LEFT_MIN_THRESHOLD) {

                    sidebar.stop = false;
                    ui_x_move(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), sidebar.x_offset);
                }
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_LEFT) {

                sidebar.stop = false;
                ui_x_move(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), sidebar.x_offset);
            }
        } else if (sidebar.move_dir == SLIDER_MOVE_UD) {
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {
                if (sidebar.first_y_offset <= DIAL_SIDEBAR_MIN_THRESHOLD) {

                    sidebar.stop = false;
                    ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), sidebar.y_offset, 1);
                } else if (sidebar.first_y_offset >= DIAL_SIDEBAR_MAX_THRESHOLD) {

                    sidebar.stop = false;
                    ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), sidebar.y_offset, 0);
                }
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_TOP) {

                sidebar.stop = false;
                ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), sidebar.y_offset, 1);
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_BOTTOM) {
                sidebar.stop = false;

                ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), sidebar.y_offset, 0);
            }
        }
        /* printf("%s %d move_dir:%d sidebar_status:%d stop:%d\n", __func__,__LINE__, sidebar.move_dir, sidebar.sidebar_status,sidebar.stop); */
        break;
    case ELM_EVENT_TOUCH_HOLD:
        printf("%s %d hold", __func__, __LINE__);
        UI_SHOW_WINDOW(ID_WINDOW_DIAL_SEL);
        return true;
    case ELM_EVENT_TOUCH_UP:
        //已经在滑动中的不处理
#if TCFG_VIDEO_DIAL_ENABLE
        if (get_aviplay_handle() != NULL) {
            extern void avi_resume();
            avi_resume();
        }
#endif

        if ((sidebar.sidebar_status == SIDEBAR_SHOWING_TOP) ||
            (sidebar.sidebar_status == SIDEBAR_SHOWING_BOTTOM) ||
            (sidebar.sidebar_status == SIDEBAR_SHOWING_LEFT)) {
            return true;
        }

        /* printf("%s %d move_dir:%d sidebar_status:%d stop:%d\n", __func__,__LINE__, sidebar.move_dir, sidebar.sidebar_status,sidebar.stop); */
        if (sidebar.move_dir == SLIDER_MOVE_LF) {
            sidebar.x_offset = e->pos.x - sidebar.pos.x;
            memcpy(&sidebar.pos, &e->pos, sizeof(struct position));
            /*刷新最后一次*/
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {
                if (sidebar.first_x_offset <= DIAL_SIDEBAR_LEFT_MIN_THRESHOLD) {
                    ui_x_move(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), sidebar.x_offset);
                }
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_LEFT) {
                ui_x_move(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), sidebar.x_offset);
            }

            ui_core_get_element_abs_rect(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), &rect);
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
                    ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), sidebar.y_offset, 1);
                } else if (sidebar.first_y_offset >= DIAL_SIDEBAR_MAX_THRESHOLD) {
                    ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), sidebar.y_offset, 0);
                }
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_TOP) {
                ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), sidebar.y_offset, 1);
            } else if (sidebar.sidebar_status == SIDEBAR_SHOW_BOTTOM) {
                ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), sidebar.y_offset, 0);
            }
            if (sidebar.sidebar_status == SIDEBAR_HIDE) {
                if (sidebar.first_y_offset <= DIAL_SIDEBAR_MIN_THRESHOLD) {
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
                } else if (sidebar.first_y_offset >= DIAL_SIDEBAR_MAX_THRESHOLD) {
                    ui_core_get_element_abs_rect(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), &rect);
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
                ui_core_get_element_abs_rect(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), &rect);
#if SILIDE_TOUCH_FULL_SCREEN
                if (rect.top + rect.height <= DIAL_SIDEBAR_RUN_HALF_THRESHOLD) {
                    sidebar.step = -DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_TOP;
                } else {
                    sidebar.step = DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_TOP;
                }
#else
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
                ui_core_get_element_abs_rect(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), &rect);
#if SILIDE_TOUCH_FULL_SCREEN
                if (rect.top >= DIAL_SIDEBAR_RUN_HALF_THRESHOLD) {
                    sidebar.step = DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_BOTTOM;
                } else {
                    sidebar.step = -DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_BOTTOM;
                }
#else
                if (rect.top >= DIAL_SIDEBAR_RUN_MIN_THRESHOLD) {
                    sidebar.step = DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_BOTTOM;
                } else {
                    sidebar.step = -DIAL_SIDEBAR_STEP;
                    sidebar.stop = false;
                    sidebar.sidebar_status = SIDEBAR_SHOWING_BOTTOM;
                }
#endif
            }
        }
        return true;
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
    case ELM_EVENT_TOUCH_L_MOVE:
        return true;
        break;

    default:
        break;
    }

    return false;
}


static int watch_onkey(void *ctrl, struct element_key_event *event)
{
    extern int dial_sel_onkey(struct element * elm, struct element_key_event * event);
    if (dial_sel_onkey(ctrl, event)) {
        return true;
    }
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
 .onkey =  watch_onkey,
  .ontouch = WATCH_ontouch,
};


static void SIDEBAR_timer(void *priv)
{
    if (!sidebar.timer) {
        return ;
    }
    if (!sidebar.stop && sidebar.step) {
        if (sidebar.sidebar_status == SIDEBAR_SHOWING_LEFT) {
            if (!ui_x_move(ui_core_get_element_by_id(DIAL_SIDEBAR_LEFT_LAYOUT), sidebar.step)) {
                if (sidebar.step > 0) {
                    sidebar.sidebar_status = SIDEBAR_SHOW_LEFT;
                } else if (sidebar.step < 0) {
                    sidebar.sidebar_status = SIDEBAR_HIDE;
                }
                sidebar.stop = true;
                sidebar.step = 0;
            }
        }
        if (sidebar.sidebar_status == SIDEBAR_SHOWING_TOP) {
            if (!ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_TOP_LAYOUT), sidebar.step, 1)) {
                if (sidebar.step > 0) {
                    sidebar.sidebar_status = SIDEBAR_SHOW_TOP;
                } else if (sidebar.step < 0) {
                    sidebar.sidebar_status = SIDEBAR_HIDE;
                }
                sidebar.stop = true;
                sidebar.step = 0;
            }
        } else if (sidebar.sidebar_status == SIDEBAR_SHOWING_BOTTOM) {
            if (!ui_y_move(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), sidebar.step, 0)) {
                if (sidebar.step > 0) {
                    sidebar.sidebar_status = SIDEBAR_HIDE;
                    /* ui_core_element_on_focus(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), 0); */
                } else if (sidebar.step < 0) {
                    sidebar.sidebar_status = SIDEBAR_SHOW_BOTTOM;
                    /* ui_core_element_on_focus(ui_core_get_element_by_id(DIAL_SIDEBAR_BUTTON_LAYOUT), 1); */
                    struct element *elm = ui_core_get_element_by_id(STYLE_DIAL_ID(WATCH));
                    elm->css.invisible = true;
                }
                sidebar.stop = true;
                sidebar.step = 0;
            }
        }
    }
}

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
        create_control_by_menu_set(0);
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
REGISTER_UI_EVENT_HANDLER(DIAL_SIDEBAR_LEFT_LAYOUT)
.onchange =  sidebar_left_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


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
extern void rcsp_extra_flash_opt_dial_nodify(void);
extern u8 call_ctrl_get_status(void);
static int dial_sel_list_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }
        int touch_index  = grid->touch_index;
        if (touch_index >= 0) {
            watch_set_style(touch_index);
            UI_SHOW_WINDOW(ID_WINDOW_DIAL);
            if (call_ctrl_get_status() == BT_CALL_HANGUP) {

                rcsp_extra_flash_opt_dial_nodify();
            }
        }
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
    case ELM_EVENT_TOUCH_L_MOVE:
        return true;
        break;
    default:
        break;
    }
    return false;
}

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
        struct rect item_r;
        ui_core_get_element_abs_rect((struct element *)&grid->item[0], &item_r);
        int item_width  = item_r.width + grid->x_interval;
        ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, -1 * watch_get_style()*item_width, NULL);
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_RELEASE:
        if (jlgpu_scheduler_wait_sync() == -OS_TIMEOUT) {
            log_error("<%s> ON_CHANGE_RELEASE error!", __func__);
        }
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
REGISTER_UI_EVENT_HANDLER(DIAL_SEL_LIST)
.onchange =  dial_sel_list_onchange,
 .onkey = NULL,
  .ontouch =  dial_sel_list_ontouch,
};
static int dial_sel_list_item_pic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct element *elm = (struct element *)ctr;
    struct draw_context *dc = (struct draw_context *)arg;
    int  sel_item;
    switch (e) {
    case ON_CHANGE_SHOW:

        if (elm->id == DIAL_SEL_LIST_PIC0) {
            sel_item = 0;
        } else if (elm->id == DIAL_SEL_LIST_PIC1) {
            sel_item = 1;
        } else if (elm->id == DIAL_SEL_LIST_PIC2) {
            sel_item = 2;
        } else if (elm->id == DIAL_SEL_LIST_PIC3) {
            sel_item = 3;
        } else if (elm->id == DIAL_SEL_LIST_PIC4) {
            sel_item = 4;
        } else if (elm->id == DIAL_SEL_LIST_PIC5) {
            sel_item = 5;
        } else {
            printf("select preview err0 %x\n", elm->id);
            return FALSE;
        }
        if ((sel_item >= watch_get_items_num()) || (view_file[sel_item].file == NULL)) {
            printf("select preview err1 %x, %d\n", elm->id, sel_item);
            return FALSE;
        }
        elm->css.background_image = 1;
        dc->preview.file = view_file[sel_item].file;
        dc->preview.file_info = &view_file[sel_item].info;
        dc->preview.id = 1;
        dc->preview.page = 0;
        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(DIAL_SEL_LIST_PIC0)
.onchange =  dial_sel_list_item_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(DIAL_SEL_LIST_PIC1)
.onchange =  dial_sel_list_item_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(DIAL_SEL_LIST_PIC2)
.onchange =  dial_sel_list_item_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(DIAL_SEL_LIST_PIC3)
.onchange =  dial_sel_list_item_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(DIAL_SEL_LIST_PIC4)
.onchange =  dial_sel_list_item_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(DIAL_SEL_LIST_PIC5)
.onchange =  dial_sel_list_item_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif /* #if TCFG_UI_DIAL_ENABLE */
#endif /* #ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE */

