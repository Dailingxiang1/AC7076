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
#include "health_manager/health_manager.h"
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
#if TCFG_UI_ENABLE_PULLUP_MENU

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

extern int music_is_play(void);
extern u8 alarm_get_active_index(void);
extern u8 rtc_calculate_week_val(struct sys_time *data_time);
extern int sidebar_button_onchange(void *ctr, enum element_change_event e, void *arg);
#define TEST_EN							1

#define TOTAL_OFFSET		 			(80)
#define CHILD_LAYOUT_HEIGHT				(140)
#define TOTAL_TOP_MAX					(50)
#define CHILD_LAYOUT_SCALE_BTM_BEGIN 	(2*CHILD_LAYOUT_HEIGHT  + TOTAL_OFFSET)
#define CHILD_LAYOUT_SCALE_BTM_END		(4*CHILD_LAYOUT_HEIGHT  + TOTAL_OFFSET)
#define CHILD_SCALE_MAX					(1.0f)
#define	CHILD_SCALE_MIN					(0.8f)
#define CHILD_SCALE_DT					(CHILD_SCALE_MAX-CHILD_SCALE_MIN)
#define	STATIC_TOP						(20)
#define ITEM_INTERVAL					(20)
#define ABS(x)          ((x)>0?(x):(-(x)))          //绝对值
#define SIDEBAR_PULLUP_MENU_LAYOUT	PULLUP_MENU_LAYOUT
#define SIDEBAR_PU_ACTIVE_DIST		PU_ACTIVE_DIST
#define SIDEBAR_PU_ACTIVE_KCAL		PU_ACTIVE_KCAL
#define SIDEBAR_PU_ACTIVE_LAYOUT	PU_ACTIVE_LAYOUT
#define SIDEBAR_PU_ACTIVE_RING		PU_ACTIVE_RING
#define SIDEBAR_PU_ACTIVE_RING_PIC  PU_ACTIVE_RING_PIC
#define SIDEBAR_PU_ACTIVE_STEPS		PU_ACTIVE_STEPS
#define SIDEBAR_PU_TIME_LAYOUT		PU_TIME_LAYOUT
#define SIDEBAR_PU_TIME_TEXT_DAY	PU_TIME_TEXT_DAY
#define SIDEBAR_PU_TIME_TEXT_WEEK	PU_TIME_TEXT_WEEK
#define SIDEBAR_PU_TIME_WATCH_0		PU_TIME_WATCH_0

struct ui_pullup_info {
    u32 init_count: 2;
    u32 steps: 20;
    u32 heart: 8;
    u8 music_pp: 1;
    u8 air_temperature_en: 1;

    u32 distance: 20;
    u8 alarm_min: 6;
    u8 alarm_hour: 5;
    u8 alarm_en: 1;

    u8 oxygen;
    s8 air_temperature;
    u16 kcal;

    u8 weather_type;
    u16 change_bit;
};
static ui_anim_t *pullup_anim_hd;
static int anim_last_step;
static struct ui_pullup_info *__info;

int ui_pullup_info_init();
int ui_pullup_info_create()
{
    if (!__info) {
        __info = zalloc(sizeof(struct ui_pullup_info));
        ui_pullup_info_init();
    }
    __info->init_count++;
    ASSERT(__info->init_count);/*防溢出*/
    return __info->init_count;
}

int ui_pullup_info_release()
{
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
int ui_pullup_info_init()
{
    if (!__info) {
        log_error("not init\n");
        return -1;
    }
    __info->music_pp = music_is_play();
    u8 alarm_index = alarm_get_active_index();
    T_ALARM alarm_info;
    alarm_get_info(&alarm_info, alarm_index - 1);
    __info->alarm_en = alarm_info.sw;
    __info->alarm_hour = alarm_info.time.hour;
    __info->alarm_min = alarm_info.time.min;
    /* printf("%s %d %d %d", __func__, __info->alarm_en, __info->alarm_hour, __info->alarm_min); */
    __info->distance = sport_health_get_value_daily_distance();
    __info->steps = sport_health_get_value_daily_steps();
    __info->kcal = sport_health_get_value_daily_calories();

#if TEST_EN
    __info->heart = 70 + rand32() % 40;
    __info->oxygen = 96 + rand32() % 4;
#endif
    int count = ui_small_file_weather_get_count();
    if (count) {
        struct weather_single_info weather_info;
        ui_small_file_weather_get_singel_info(&weather_info, (count - 1));

        __info->air_temperature_en = 1;
        __info->air_temperature = weather_info.temperature;
        __info->weather_type = weather_info.weather;
    }


    __info->change_bit = 0xffff;
    return 0;
}

struct child_menu_rect {
    struct list_head entry;
    struct rect rect;
};
struct list_head child_menu_head;

void child_menu_init()
{
    INIT_LIST_HEAD(&child_menu_head);
}

int child_menu_search(struct rect *rect)
{
    struct child_menu_rect *p;
    list_for_each_entry(p, &child_menu_head, entry) {
        if (!memcmp(&p->rect, rect, sizeof(struct rect))) {
            return 1;
        }
    }
    return 0;
}

int child_menu_add(struct rect *rect)
{
    if (child_menu_search(rect)) {
        return 0;
    }
    struct child_menu_rect *new = (struct child_menu_rect *)malloc(sizeof(struct child_menu_rect));
    memcpy(&new->rect, rect, sizeof(struct rect));
    list_add_tail(&new->entry, &child_menu_head);

    return 1;
}

void child_menu_uninit()
{
    struct child_menu_rect *p, *n;
    list_for_each_entry_safe(p, n, &child_menu_head, entry) {
        free(p);
    }
}

static int child_menu_layout_move(struct element *layout_elm, int y_move, int redraw)
{
    if ((layout_elm->css.top != 0) && redraw) {
        return 0;
    }
    int  ret = 1;
    struct element *child_elm, *n;
    int total_top = 0;
    int child_index = 0;
    int layout_height = 0;
    int curr_top = 0;
    int curr_btm = 0;

    child_menu_init();

    list_for_each_child_element_reverse(child_elm, n, layout_elm) {
        struct rect rect;
        ui_core_get_element_abs_rect(child_elm, &rect);
        int show = child_menu_add(&rect);
        if (child_index == 0) {

            /* printf("%s %d %d ", __func__, child_elm->css.top, - 6 * (child_elm->css.height + ITEM_INTERVAL) + STATIC_TOP); */
            if (abs(child_elm->css.top + y_move) > abs(- 6 * (child_elm->css.height + ITEM_INTERVAL) + STATIC_TOP)) {
                child_elm->css.top = (- 6 * (child_elm->css.height + ITEM_INTERVAL) +  STATIC_TOP) ;
            } else {
                child_elm->css.top += y_move;
            }
            if (child_elm->css.top > STATIC_TOP) {
                child_elm->css.top = STATIC_TOP;
                /* return 0; */
                ret = 0;
            }

            total_top =	child_elm->css.top;
            /* printf("%s %d %d %d \n", __func__, child_elm->css.top, - 6 * (child_elm->css.height + ITEM_INTERVAL), total_top); */
            layout_height = child_elm->css.height + ITEM_INTERVAL;
            if (child_elm->css.top > 0) {
                struct element *elm = ui_core_get_element_by_id(STYLE_DIAL_ID(WATCH));
                elm->css.invisible = false;
            }
        } else {
            curr_top = total_top + child_index * layout_height;
            curr_btm = curr_top + layout_height;
            if (curr_btm < CHILD_LAYOUT_SCALE_BTM_BEGIN) {
                child_elm->css.top = curr_top;
                /* child_elm->css.invisible = 0; */
                /* child_elm->css.height = layout_height; */
                /* ui_core_set_element_ratio(child_elm, 1.0f, 1.0f, 0); */
            } else if (curr_btm >= CHILD_LAYOUT_SCALE_BTM_BEGIN) {
                /* child_elm->css.invisible = 0; */
                float scale = CHILD_SCALE_MAX - CHILD_SCALE_DT * abs(curr_btm -  CHILD_LAYOUT_SCALE_BTM_BEGIN)	/ CHILD_LAYOUT_HEIGHT;
                if (scale < CHILD_SCALE_MIN) {
                    scale = CHILD_SCALE_MIN;
                }
                int scale_height = scale * layout_height;
                int y_offset = (layout_height -  scale_height) / 2;
                child_elm->css.top = CHILD_LAYOUT_SCALE_BTM_BEGIN - layout_height + y_offset;
                /* printf("%s %d %d %f \n", __func__, child_elm->css.top, total_top, scale); */
                ui_core_set_element_ratio(child_elm, scale, scale, 1);
            }
        }

        if (redraw) {
            if (show) {
                child_elm->css.invisible = false;
            } else {
                child_elm->css.invisible = true;
            }
        }
        child_index ++;
    }

    child_menu_uninit();

    if (redraw) {
        ui_core_redraw(layout_elm);
    }
    return ret;
}
static void ui_pullup_anim_cb(int var, int32_t v)
{
    struct element *elm = (struct element *)ui_core_get_element_by_id(var);
    if (!elm) {
        return;
    }
    int steps = v -  anim_last_step;
    child_menu_layout_move(elm, steps, 1);

    /* printf("%s %d %d %d", __func__, steps, v, anim_last_step); */
    anim_last_step = v;
}
static void pullup_anim_stop(void)
{
    ui_anim_del(SIDEBAR_PULLUP_MENU_LAYOUT, NULL);
}

static void pullup_energy_anim_start(struct element *elm, struct element_touch_event *e)
{
    int start_dist, end_dist;
    // 释放旧的
    pullup_anim_stop();
    // 动画参数计算
    int dist_y = e->pos.y >> 16;
    int energy_t0 = (e->pos.x + 1) & 0xffff; //防止div0
    float vy0 = ABS(2 * (float)dist_y / energy_t0); //垂直初速度
    int ydir = (e->pos.y >> 8) & 0xff;				//垂直滑动方向

    int run_time = vy0 * 30;						//惯性时间

    end_dist = 3 * run_time ;						//y方向的惯性距离
    end_dist = end_dist / (CHILD_LAYOUT_HEIGHT + ITEM_INTERVAL) * (CHILD_LAYOUT_HEIGHT + ITEM_INTERVAL);
    end_dist *= (ydir == 1) ? -1 : 1;
    struct element *child_elm, *n;
    int	top = 0;
    int move_steps = 0;
    int total_move_steps = 1000000;//给个足够大的数
    int total_move_top = 0;
    int total_move_btm = 0;
    int count = 0;
    list_for_each_child_element_reverse(child_elm, n, elm) {
        if (!count) {
            total_move_top =  STATIC_TOP - child_elm->css.top;
            total_move_btm = 6 * (child_elm->css.height + ITEM_INTERVAL) + STATIC_TOP - child_elm->css.top;
        }
        move_steps = -1 * (child_elm->css.top - STATIC_TOP);
        /* printf("%s move_steps:%d top:%d ", __func__, move_steps, child_elm->css.top); */
        if (abs(move_steps) < abs(total_move_steps)) {
            total_move_steps = move_steps;
        }
    }

    end_dist = total_move_steps  + end_dist;

    start_dist = 0;
    /* printf("%s end_dist:%d ydir:%d runtime:%d total_move_steps:%d top:%d btm:%d \n", __func__, end_dist, ydir, run_time, total_move_steps, total_move_top, total_move_btm); */
    end_dist = (end_dist < total_move_top) ? total_move_top : end_dist;
    end_dist = (end_dist > total_move_btm) ? total_move_btm : end_dist;
    if (total_move_steps) {
        ui_anim_init(pullup_anim_hd);
        ui_anim_set_var(pullup_anim_hd, elm->id);
        ui_anim_set_path_cb(pullup_anim_hd, ui_anim_path_ease_out);
        ui_anim_set_exec_cb(pullup_anim_hd,  ui_pullup_anim_cb);
        /* ui_anim_set_ready_cb(pullup_anim_hd, ui_grid_anim_ready_callback); */
        ui_anim_set_values(pullup_anim_hd, start_dist, end_dist);
        ui_anim_set_time(pullup_anim_hd, run_time);
        anim_last_step  = 0;
        ui_anim_start(pullup_anim_hd);
    }

}

static int pullup_menu_layout_ontouch(void *ctrl, struct element_touch_event *e)
{
    static s16 last_y_pos = -1;
    struct element *elm = (struct element *)ctrl;

    struct ui_watch *watch  = (struct ui_watch *)ui_core_get_element_by_id(STYLE_DIAL_ID(WATCH));
    if (e->event == ELM_EVENT_TOUCH_DOWN || e->event == ELM_EVENT_TOUCH_UP) {
        if (e->event == ELM_EVENT_TOUCH_DOWN || e->event == ELM_EVENT_TOUCH_MOVE || e->event == ELM_EVENT_TOUCH_UP) {
            if (watch && watch->handler && watch->handler->ontouch) {
                watch->handler->ontouch(watch, e);
            }
        }
    }
    switch (e->event) {
    case ELM_EVENT_TOUCH_ENERGY:
        pullup_energy_anim_start(elm, e);
        break;
    case ELM_EVENT_TOUCH_DOWN:
        last_y_pos = e->pos.y;

        return true;
        break;
    case ELM_EVENT_TOUCH_MOVE:
        int y_move = e->pos.y - last_y_pos;
        if (!child_menu_layout_move((struct element *)ctrl, y_move, 1)) {
            if (e->event == ELM_EVENT_TOUCH_DOWN || e->event == ELM_EVENT_TOUCH_MOVE || e->event == ELM_EVENT_TOUCH_UP) {
                if (watch && watch->handler && watch->handler->ontouch) {
                    watch->handler->ontouch(watch, e);
                }
            }
        }
        last_y_pos = e->pos.y;
        break;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            if (e->has_energy) {
                break;
            }
            struct element *child_elm, *n;
            int	top = 0;
            int move_steps = 0;
            int total_move_steps = 1000000;//给个足够大的数
            list_for_each_child_element_reverse(child_elm, n, elm) {
                move_steps = -1 * (child_elm->css.top - STATIC_TOP);
                /* printf("%s move_steps:%d top:%d ", __func__, move_steps, child_elm->css.top); */
                if (abs(move_steps) < abs(total_move_steps)) {
                    total_move_steps = move_steps;
                }
            }
            /* printf("%s steps:%d\n", __func__, total_move_steps); */
            if (total_move_steps) {
                ui_anim_del(elm->id, NULL);
                ui_anim_init(pullup_anim_hd);
                ui_anim_set_var(pullup_anim_hd, elm->id);
                ui_anim_set_path_cb(pullup_anim_hd, ui_anim_path_ease_out);
                ui_anim_set_exec_cb(pullup_anim_hd,  ui_pullup_anim_cb);
                /* ui_anim_set_ready_cb(pullup_anim_hd, ui_grid_anim_ready_callback); */
                ui_anim_set_values(pullup_anim_hd, 0, total_move_steps);
                ui_anim_set_time(pullup_anim_hd, 100);
                anim_last_step  = 0;
                ui_anim_start(pullup_anim_hd);
            }
        } else {
            int index = 0;
            struct element *child_elm, *n;
            list_for_each_child_element_reverse(child_elm, n, elm) {
                struct rect  child_rect;
                ui_core_get_element_abs_rect(child_elm, &child_rect);
                if (in_rect(&child_rect, &e->pos)) {
                    switch (index) {
                    case 0:
                        break;
                    case 1:
                        ui_card_disable();
                        UI_SHOW_WINDOW(ID_WINDOW_MOMENTUM);
                        break;
                    case 2 :
                        ui_card_disable();
                        UI_SHOW_WINDOW(ID_WINDOW_ALARM);
                        break;
                    case 3:
                        ui_card_disable();
                        UI_SHOW_WINDOW(ID_WINDOW_MUSIC_PLAYER);
                        break;
                    case 4:
                        ui_card_disable();
                        UI_SHOW_WINDOW(ID_WINDOW_WEATHER);
                        break;
                    case 5:
                        ui_card_disable();
                        UI_SHOW_WINDOW(ID_WINDOW_BLOODPRESSURE);
                        break;
                    case 6:
                        ui_card_disable();
                        UI_SHOW_WINDOW(ID_WINDOW_OXYGEN);
                        break;
                    case 7:
                        ui_show_menu_force();
                        break;

                    }

                    return true;
                }
                index ++;
            }
        }
        break;
    }
    return false;
}

static int  pullup_menu_layout_onchange(void *ctrl, enum element_change_event event, void *arg)
{

    sidebar_button_onchange(ctrl, event, arg);
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_INIT:
        ui_pullup_info_create();
        child_menu_layout_move(elm, 50, 0);
        if (!pullup_anim_hd) {
            pullup_anim_hd = zalloc(sizeof(ui_anim_t));
        }
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    case ON_CHANGE_RELEASE:
        if (pullup_anim_hd) {
            ui_anim_del(elm->id, NULL);
            free(pullup_anim_hd);
            pullup_anim_hd = NULL;
        }

        ui_pullup_info_release();
        break;
    default:
        return false;
    }
    return false;
}
static int pullup_menu_layout_onkey(void *ctrl, struct element_key_event *event)
{
    /* printf("%s %d",__func__,__LINE__); */
    int max_cnt = 4;
    switch (event->value) {
    case KEY_UI_MINUS:
        for (int i = 0; i < max_cnt; i++) {
            if (i == max_cnt - 1) {
                int steps = (CHILD_LAYOUT_HEIGHT + ITEM_INTERVAL) - i * ((CHILD_LAYOUT_HEIGHT + ITEM_INTERVAL) / max_cnt);
                child_menu_layout_move((struct element *)ctrl, steps, 1);
            } else {
                child_menu_layout_move((struct element *)ctrl, (CHILD_LAYOUT_HEIGHT + ITEM_INTERVAL) / max_cnt, 1);
                os_time_dly(2);
            }
        }
        break;
    case KEY_UI_PLUS:
        for (int i = 0; i < max_cnt; i++) {
            if (i == max_cnt - 1) {
                int steps = (CHILD_LAYOUT_HEIGHT + ITEM_INTERVAL) - i * ((CHILD_LAYOUT_HEIGHT + ITEM_INTERVAL) / max_cnt);
                child_menu_layout_move((struct element *)ctrl, -1 * steps, 1);
            } else {
                child_menu_layout_move((struct element *)ctrl, -1 * (CHILD_LAYOUT_HEIGHT + ITEM_INTERVAL) / max_cnt, 1);
                os_time_dly(2);
            }
        }

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SIDEBAR_PULLUP_MENU_LAYOUT)
.onchange = pullup_menu_layout_onchange,
 .onkey =  pullup_menu_layout_onkey,
  .ontouch = pullup_menu_layout_ontouch,
};
//=================================================================================================================

static int pullup_time_layout_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{

    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_SHOW_PROBE:
        struct sys_time time;
        rtc_read_time(&time);
        if (elm->id == SIDEBAR_PU_TIME_TEXT_DAY) {
            u8 index_buf[5] = {0};
            index_buf[0] = time.month / 10;
            index_buf[1] = time.month % 10;
            index_buf[2] = 11;
            index_buf[3] = time.day / 10;
            index_buf[4] = time.day % 10;
            ui_text_set_multi_text_index((struct ui_text *)elm, index_buf, 5);
        } else if (elm->id == SIDEBAR_PU_TIME_TEXT_WEEK) {
            u8 week  = rtc_calculate_week_val(&time);
            ui_text_set_index((struct ui_text *)elm, week);
        }
        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SIDEBAR_PU_TIME_TEXT_DAY)
.onchange = pullup_time_layout_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_PU_TIME_TEXT_WEEK)
.onchange = pullup_time_layout_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int ui_text_mulstr_buf_change_10(u8 *buf, int value)
{
    if (value >= 1000000000L) {
        return -1;
    }

    int tmp = value;
    int count = 0;
    for (count = 0; count < 10; count++) {
        tmp /= 10;
        if (!tmp) {
            count ++;
            break;
        }
    }
    tmp = value;
    for (int i = count - 1; i >= 0; i--) {
        buf[i] = tmp % 10;
        tmp /= 10;
    }
    return count;
}
static int pullup_active_layout_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{

    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_SHOW_PROBE:
        struct sys_time time;
        rtc_read_time(&time);
        if (elm->id == SIDEBAR_PU_ACTIVE_STEPS) {
            int steps =  sport_health_get_value_daily_steps();

            u8 index_buf[10] = {0};
            u8 index_len = ui_text_mulstr_buf_change_10(index_buf, steps);

            struct ui_text *text = (struct ui_text *)elm;
            text->_format =  UI_TEXT_ENCODE_MULSTR;
            ui_text_set_multi_text_index((struct ui_text *)elm, index_buf, index_len);
        } else if (elm->id == SIDEBAR_PU_ACTIVE_DIST) {
            int distance =  sport_health_get_value_daily_distance();

            u8 index_buf[10] = {0};
            u8 index_len = ui_text_mulstr_buf_change_10(index_buf, distance);

            struct ui_text *text = (struct ui_text *)elm;
            text->_format =  UI_TEXT_ENCODE_MULSTR;
            ui_text_set_multi_text_index((struct ui_text *)elm, index_buf, index_len);
        } else if (elm->id == SIDEBAR_PU_ACTIVE_KCAL) {
            int kcal = sport_health_get_value_daily_calories();

            u8 index_buf[10] = {0};
            u8 index_len = ui_text_mulstr_buf_change_10(index_buf, kcal);

            struct ui_text *text = (struct ui_text *)elm;
            text->_format =  UI_TEXT_ENCODE_MULSTR;
            ui_text_set_multi_text_index((struct ui_text *)elm, index_buf, index_len);
        }

        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SIDEBAR_PU_ACTIVE_STEPS)
.onchange = pullup_active_layout_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_PU_ACTIVE_DIST)
.onchange = pullup_active_layout_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SIDEBAR_PU_ACTIVE_KCAL)
.onchange = pullup_active_layout_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int pullup_alarm_layout_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    u8 alarm_text_buf[6];
    u8 alarm_text_buf_len;
    switch (event) {
    case ON_CHANGE_SHOW_PROBE:
        if (__info->alarm_en) {
            alarm_text_buf[0] = __info->alarm_hour / 10;
            alarm_text_buf[1] = __info->alarm_hour % 10;
            alarm_text_buf[2] = 10;
            alarm_text_buf[3] = __info->alarm_min / 10;
            alarm_text_buf[4] = __info->alarm_min % 10;
            alarm_text_buf_len = 5;
        } else {
            alarm_text_buf[0] = 11;
            alarm_text_buf_len = 1;
        }

        ui_text_set_multi_text_index((struct ui_text *)elm, alarm_text_buf, alarm_text_buf_len);
        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PU_ALARM_TIME)
.onchange =  pullup_alarm_layout_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int pullup_music_layout_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_SHOW_PROBE:
        ui_pic_set_image_index((struct ui_pic *)elm, music_is_play());
        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PU_MUSIC_PP)
.onchange =  pullup_music_layout_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int pullup_weather_layout_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_SHOW_PROBE:
        if (elm->id == PU_WEATHER_PIC) {
            ui_pic_set_image_index((struct ui_pic *)elm, __info->weather_type);
        }
        break;
    case ON_CHANGE_SHOW:
        if (elm->id == PU_WEATHER_TEMP) {
            struct ui_number *ui_num = (struct ui_number *)elm;
            struct ui_number_info *info = ui_core_load_widget_info((void *)ui_num->info, -1);
            int number_buffer_offset = 0;
            //负号
            if (__info->air_temperature < 0) {
                ui_num->buf[number_buffer_offset]	 = info->delimiter[1];
                number_buffer_offset++;
                __info->air_temperature *= -1;
            }
            int air_temp_10 = __info->air_temperature / 10;
            int air_temp_1  = __info->air_temperature % 10;
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
    case ON_CHANGE_RELEASE:

        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PU_WEATHER_PIC)
.onchange =  pullup_weather_layout_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(PU_WEATHER_TEMP)
.onchange =  pullup_weather_layout_child_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif// TCFG_UI_DRAW_DEMO
#endif// CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE







