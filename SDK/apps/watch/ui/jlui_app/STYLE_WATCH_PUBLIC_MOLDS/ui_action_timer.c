#include "app_config.h"
#include "jlui/ui.h"
#include "ui/ui_api.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "tone_player.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "res/resfile.h"
#include "jlui_app/res_config.h"
#include "ui_sys_param.h"
#include "app_tone.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_TIMER]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_timer.data.bss")
#pragma data_seg(".ui_action_timer.data")
#pragma const_seg(".ui_action_timer.text.const")
#pragma code_seg(".ui_action_timer.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_TIMER_ACTION

#define STYLE_NAME  JL

#define MAX_TIME  (60*60*24 - 1)

#define TIMER_INIT   0
#define TIMER_START  1
#define TIMER_STOP   2

#define TIMER_VLIST_LOOP_EN      1    // 列表循环

struct timer_param_t {
    int countdown_hour;         // 获取垂直列表的小时
    int countdown_min;          // 获取垂直列表的分钟
    int countdown_sec;         // 获取垂直列表的秒

    u32 timer_show_id;          // 用于显示的定时器id
    u32 timer_end_id;           // 结束界面定时器id

    int time_cnt;             // 用于结束界面的计时器
};
static struct timer_param_t *timer = NULL;

struct timer_background_param_t {
    u32 timer_background_prev_time;    // 退出到后台前的时间
    u32 countdown_num;         // 总倒计时时间 (s)
    u32 total_num;             // 总倒计时间 (s)
    u32 percent;               // 用于圆环的百分比
    u32 timer_countdown_num_id; // 总倒计时定时器id
    u32 step_1;
    u32 step_2;
    u32 time_1;
    u32 time_2;
    u32 cnt_time_1;
    u32 cnt_time_2;
    u32 cnt_1;
    u16 timer_end_id;
    u8 timer_status;           // 记录timer状态
    u8 timer_is_end;
    u8 timer_is_background;    // 记录timer是否在后台计时
};
static struct timer_background_param_t *timer_background = NULL;


static int timer_end_tone_play_handler(int param)
{
    play_tone_file(get_tone_files()->phone_in);
    return 0;
}

static void time_end_tone_play()
{
    int argv[3];
    argv[0] = (int)timer_end_tone_play_handler;
    argv[1] = 1;
    argv[2] = 0;
    os_taskq_post_type("app_core", Q_CALLBACK, 3, argv);
}

static int timer_ui_switch_init(int id)
{
    if (timer_background->timer_is_end == 1) {
        ui_hide(TIMER_LAYOUT);
        ui_show(TIMER_STOP_LAYOUT);
        return 0;
    }
    if (timer_background->timer_status == TIMER_START || timer_background->timer_status == TIMER_STOP) {
        ui_hide(TIMER_LAYOUT);
        ui_show(TIMER_START_LAYOUT);
    } else if (timer_background->timer_status == TIMER_INIT) {
        ui_hide(TIMER_START_LAYOUT);
        ui_show(TIMER_LAYOUT);
    }
    return 0;
}

static int timer_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***timer_onchange***\n");
        if (!timer) {
            timer = zalloc(sizeof(struct timer_param_t));
        }
        if (!timer_background) {
            timer_background = zalloc(sizeof(struct timer_background_param_t));
        }
        if (timer_background->timer_end_id) {
            sys_timeout_del(timer_background->timer_end_id);
            timer_background->timer_end_id = 0;
        }
        //timer_background->timer_is_background = 0;

        printf("@@@@@@page init\n");
        //ui_set_call(timer_ui_switch_init, 0);
        break;
    case ON_CHANGE_RELEASE:
        if (timer) {
            if (timer_background->timer_countdown_num_id) {
                if (timer_background->timer_status != TIMER_START) {
                    sys_timer_del(timer_background->timer_countdown_num_id);
                    timer_background->timer_countdown_num_id = 0;
                }
            }
            if (timer->timer_show_id) {
                sys_timer_del(timer->timer_show_id);
                timer->timer_show_id = 0;
            }
            if (timer->timer_end_id) {
                sys_timer_del(timer->timer_end_id);
                timer->timer_end_id = 0;
            }
            free(timer);
            timer = NULL;
        }
        if (timer_background && (timer_background->timer_status == TIMER_INIT)) {
            free(timer_background);
            timer_background = NULL;
        } else {
            timer_background->timer_is_background = 1;
        }
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_TIMER)
.onchange = timer_page_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int timer_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        if (timer_background->timer_status == TIMER_INIT) {
            layout->elm.css.invisible = 0;
        }
        break;
    case ON_CHANGE_INIT:
        if (timer_background) {
            timer_background->timer_is_background = 0;
        }
        memset(timer, 0, sizeof(struct timer_param_t));
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

REGISTER_UI_EVENT_HANDLER(TIMER_LAYOUT)//通用-垂直列表
.onchange = timer_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int timer_hour_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_hour = 0;

    switch (event) {
    case ON_CHANGE_INIT:
#if TIMER_VLIST_LOOP_EN

        int base = 10000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 24;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        base = (base / 2) * 24;

        ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

        base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;
        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_hour == 0) ? r.height + 5 : 0;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);

#else

        row = 24;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        printf("dynamic_grid %d X %d\n", row, col);

        if (time_hour == 0) {
            ui_grid_set_hindex_dynamic(grid, time_hour, true, 0);
        }
        base_index_once = (time_hour >= 1) ? (time_hour - 1) : 0;

        if (time_hour == 23) {
            base_index_once = time_hour - 3;
        } else if (time_hour == 22) {
            base_index_once = time_hour - 2;
        }

        ui_grid_set_base_dynamic(grid, base_index_once, 0);
#endif
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        /* if (grid->elm.id == TIMER_HOUR_VLIST) { */
        /*     if (timer) { */
        /*         timer->countdown_hour = ui_grid_get_hindex_dynamic(grid); */
        /*     } */
        /* } */
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(TIMER_HOUR_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = timer_hour_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int timer_min_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_min = 0;

    switch (event) {
    case ON_CHANGE_INIT:

#if TIMER_VLIST_LOOP_EN

        int base = 10000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 60;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        base = (base / 2) * 60;

        ui_grid_set_hindex_dynamic(grid, time_min + base, true, 1);

        base_index_once = ((time_min >= 1) ? (time_min - 1) : 0) + base;

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_min == 0) ? r.height + 5 : 0;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);

#else
        row = 60;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        if (time_min == 0) {
            ui_grid_set_hindex_dynamic(grid, time_min, true, 0);
        }

        base_index_once = (time_min >= 1) ? (time_min - 1) : 0;

        if (time_min == 59) {
            base_index_once = time_min - 3;
        } else if (time_min == 58) {
            base_index_once = time_min - 2;
        }

        ui_grid_set_base_dynamic(grid, base_index_once, 0);
#endif
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        /* if (grid->elm.id == TIMER_MIN_VLIST) { */
        /*     if (timer) { */
        /*         timer->countdown_min = ui_grid_get_hindex_dynamic(grid); */
        /*     } */
        /* } */
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(TIMER_MIN_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = timer_min_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static void key_redraw(void *ctrl)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    ui_core_redraw(grid);
}

static int timer_sec_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int index = ui_grid_get_hindex_dynamic(grid);
    struct rect r;

    switch (e->value) {
    case KEY_UI_PLUS:
        printf("@@@@@ plus\n");
        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        ui_grid_slide_with_callback_dynamic(grid, SCROLL_DIRECTION_UD, -(r.height + grid->y_interval), key_redraw);
        return true;
        break;
    case KEY_UI_MINUS:
        printf("@@@@@ minus\n");
        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        ui_grid_slide_with_callback_dynamic(grid, SCROLL_DIRECTION_UD, (r.height + grid->y_interval), key_redraw);
        return true;
        break;
    default:
        break;
    }

    return false;
}

static int timer_sec_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_sec = 0;

    switch (event) {
    case ON_CHANGE_INIT:

#if TIMER_VLIST_LOOP_EN

        int base = 10000;
        int first_move_step = 0;
        struct rect r;

        base_index_once = base * 60;

        row = base_index_once;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);

        base = (base / 2) * 60;

        ui_grid_set_hindex_dynamic(grid, time_sec + base, true, 1);

        base_index_once = ((time_sec >= 1) ? (time_sec - 1) : 0) + base;

        ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
        first_move_step = (time_sec == 0) ? r.height + 5 : 0;

        ui_grid_set_base_dynamic(grid, base_index_once, first_move_step);

#else
        row = 60;
        col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);


        if (time_sec == 0) {
            ui_grid_set_hindex_dynamic(grid, time_sec, true, 0);
        }

        base_index_once = (time_sec >= 1) ? (time_sec - 1) : 0;

        if (time_sec == 59) {
            base_index_once = time_sec - 3;
        } else if (time_sec == 58) {
            base_index_once = time_sec - 2;
        }

        ui_grid_set_base_dynamic(grid, base_index_once, 0);
#endif
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        /* if (grid->elm.id == TIMER_SEC_VLIST) { */
        /*     if (timer) { */
        /*         timer->countdown_sec = ui_grid_get_hindex_dynamic(grid); */
        /*     } */
        /* } */
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(TIMER_SEC_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = timer_sec_onchange,
 .onkey = timer_sec_onkey,
  .ontouch = NULL,
};

static int timer_h_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 24;//这里必须求余数方式获取索引
        /* printf("tid %d\n", index); */
        if ((index < 0) || (index > 23)) {
            break;
        }
        switch (pic->elm.id) {
        case TIMER_H_00:
        case TIMER_H_10:
        case TIMER_H_20:
        case TIMER_H_30:
            index_buf = index / 10;
            break;
        case TIMER_H_01:
        case TIMER_H_11:
        case TIMER_H_21:
        case TIMER_H_31:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_H_00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_H_10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_H_20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_H_01)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_H_11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_H_21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_H_31)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_H_30)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_h_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int timer_m_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 60;//这里必须求余数方式获取索引
        /* log_info("tid %d\n", index); */
        if ((index < 0) || (index > 59)) {
            break;
        }
        switch (pic->elm.id) {
        case TIMER_M_00:
        case TIMER_M_10:
        case TIMER_M_20:
        case TIMER_M_30:
            index_buf = index / 10;
            break;
        case TIMER_M_01:
        case TIMER_M_11:
        case TIMER_M_21:
        case TIMER_M_31:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_M_00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_M_10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_M_20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_M_01)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_M_11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_M_21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_M_30)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_M_31)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int timer_s_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u8 index_buf;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 60;//这里必须求余数方式获取索引
        /* log_info("tid %d\n", index); */
        if ((index < 0) || (index > 59)) {
            break;
        }
        switch (pic->elm.id) {
        case TIMER_S_00:
        case TIMER_S_10:
        case TIMER_S_20:
        case TIMER_S_30:
            index_buf = index / 10;
            break;
        case TIMER_S_01:
        case TIMER_S_11:
        case TIMER_S_21:
        case TIMER_S_31:
            index_buf = index % 10;
            break;
        }
        ui_pic_set_image_index(pic, index_buf);
        break;
    default:
        break;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_S_00)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_S_10)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_S_20)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_S_01)//勿扰模式-开始时间-动态垂直列表-文字控件
.onchange = timer_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_S_11)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_S_21)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_S_31)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(TIMER_S_30)//勿扰模式-结束时间-动态垂直列表-文字控件
.onchange = timer_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int timer_play_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    struct ui_grid *grid = NULL;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case TIMER_PLAY_BUTTON:
#if TIMER_VLIST_LOOP_EN
            grid = ui_grid_for_id(TIMER_HOUR_VLIST);
            if (grid) {
                timer->countdown_hour = ui_grid_get_hindex_dynamic(grid);
                timer->countdown_hour = timer->countdown_hour % 24;
            }
            grid = ui_grid_for_id(TIMER_MIN_VLIST);
            if (grid) {
                timer->countdown_min = ui_grid_get_hindex_dynamic(grid);
                timer->countdown_min = timer->countdown_min % 60;
            }
            grid = ui_grid_for_id(TIMER_SEC_VLIST);
            if (grid) {
                timer->countdown_sec = ui_grid_get_hindex_dynamic(grid);
                timer->countdown_sec = timer->countdown_sec % 60;
            }
#else
            grid = ui_grid_for_id(TIMER_HOUR_VLIST);
            if (grid) {
                timer->countdown_hour = ui_grid_get_hindex_dynamic(grid);
            }
            grid = ui_grid_for_id(TIMER_MIN_VLIST);
            if (grid) {
                timer->countdown_min = ui_grid_get_hindex_dynamic(grid);
            }
            grid = ui_grid_for_id(TIMER_SEC_VLIST);
            if (grid) {
                timer->countdown_sec = ui_grid_get_hindex_dynamic(grid);
            }

#endif

            timer_background->countdown_num = timer->countdown_hour * 3600 + timer->countdown_min * 60 + timer->countdown_sec;
            if (timer_background->countdown_num != 0) {
                ui_hide(TIMER_LAYOUT);
                ui_show(TIMER_START_LAYOUT);
            }
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_PLAY_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = timer_play_ontouch,
};

static void redraw_timer_num()
{
    struct utime time;
    time.hour = timer->countdown_hour;
    time.min = timer->countdown_min;
    time.sec = timer->countdown_sec;

    ui_time_update_by_id(TIMER_COUNTDOWN_NUM, &time);

    timer_background->countdown_num = timer->countdown_hour * 3600 + timer->countdown_min * 60 + timer->countdown_sec;

}



static void show_timer(void *priv)
{
    struct utime time;

    if (timer_background->countdown_num == 0) {
        ui_progress_set_persent_by_id(TIMER_PROGRESS, 0);
    } else {
        ui_progress_set_persent_by_id(TIMER_PROGRESS, timer_background->percent);
    }

    time.hour = timer_background->countdown_num / 3600;
    time.min = (timer_background->countdown_num % 3600) / 60;
    time.sec = ((timer_background->countdown_num % 3600) % 60);
    ui_time_update_by_id(TIMER_COUNTDOWN_NUM, &time);


    if (timer_background->countdown_num == 0) {
        ui_hide(TIMER_START_LAYOUT);
        ui_show(TIMER_STOP_LAYOUT);
        timer_background->percent = 100;
        time_end_tone_play();
    }
}


static void timer_end_show(void *priv)
{
    if (timer->time_cnt) {
        timer->time_cnt--;
    } else {
        if (timer->timer_end_id) {
            sys_timer_del(timer->timer_end_id);
            timer->timer_end_id = 0;
        }
        if (timer_background->timer_is_background == 1) {
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_TIMER);
            /* if (UI_WINDOW_PREEMPTION_CHECK()) { */
            /* return; */
            /* } */
            /* UI_HIDE_CURR_WINDOW(); */
            /* UI_SHOW_WINDOW(ID_WINDOW_DIAL); */
        } else {
            ui_hide(TIMER_STOP_LAYOUT);
            ui_show(TIMER_LAYOUT);
        }
        /* ui_hide(TIMER_STOP_LAYOUT); */
        /* ui_show(TIMER_LAYOUT); */

        UI_MOTO_RUN(0);
    }
}

static void background_timer_end_show()
{
    timer_background->timer_is_end = 1;
    /* UI_HIDE_CURR_WINDOW(); */
    /* UI_SHOW_WINDOW(ID_WINDOW_TIMER); */
    UI_WINDOW_PREEMPTION_POSH(ID_WINDOW_TIMER, NULL, NULL, UI_WINDOW_PREEMPTION_TYPE_TIMER_COUNTDOWN);

    if (timer_background->timer_countdown_num_id) {
        sys_timer_del(timer_background->timer_countdown_num_id);
        timer_background->timer_countdown_num_id = 0;
    }

    if (timer_background->timer_end_id) {
        sys_timeout_del(timer_background->timer_end_id);
        timer_background->timer_end_id = 0;
    }
}

static void cal_circle_percent()
{
    if (timer_background->total_num <= 100) {
        timer_background->step_1 = 100 / timer_background->total_num;
        timer_background->step_2 = timer_background->step_1 + 1;

        for (int i = 0; i <= timer_background->total_num; i++) {
            if (timer_background->step_1 * i + timer_background->step_2 * (timer_background->total_num - i) == 100) {
                timer_background->time_1 = i;
                timer_background->time_2 = timer_background->total_num - i;
                break;
            }
        }
    } else {
        timer_background->step_1 = (timer_background->total_num / 100) + 1;      // step_1秒动1格
        timer_background->step_2 = timer_background->step_1 - 1;
        for (int i = 0; i <= 100; i++) {
            if (timer_background->step_1 * i + timer_background->step_2 * (100 - i) == timer_background->total_num) {
                timer_background->time_1 = i;
                timer_background->time_2 = 100 - i;
                break;
            }
        }
    }
}

static void countdown_timer(void *priv)
{
    int step;
    int time_for_step;

    if (timer_background->total_num <= 100) {      // 倒计时小于100秒
        if (timer_background->cnt_time_1 < timer_background->time_1) {
            step = timer_background->step_1;
            timer_background->percent -= step;
            timer_background->cnt_time_1++;
        } else {
            step = timer_background->step_2;
            timer_background->percent -= step;
        }
    } else {                      // 倒计时大于100秒
        if (timer_background->cnt_1 < timer_background->time_1) {
            if (timer_background->cnt_time_1 < timer_background->step_1 - 1) {
                timer_background->cnt_time_1++;
            } else {
                timer_background->cnt_time_1 = 0;
                step = 1;
                timer_background->percent -= step;
                timer_background->cnt_1++;
            }
        } else {
            if (timer_background->cnt_time_2 < timer_background->step_2 - 1) {
                timer_background->cnt_time_2++;
            } else {
                timer_background->cnt_time_2 = 0;
                step = 1;
                timer_background->percent -= step;
            }
        }
    }

    if (timer_background->countdown_num < MAX_TIME) {
        timer_background->countdown_num--;
    } else {
        timer_background->countdown_num = MAX_TIME;
    }

    if (timer_background->timer_is_background == 1 && timer_background->countdown_num == 0) {
        background_timer_end_show();
        time_end_tone_play();
    }
}

static int timer_start_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;
    struct ui_pic *pic = NULL;
    struct ui_time *countdown_time = NULL;
    struct ui_progress *progress = NULL;
    struct utime time;

    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        if (timer_background->timer_status == TIMER_START || timer_background->timer_status == TIMER_STOP) {
            if (timer_background->countdown_num != 0) {
                layout->elm.css.invisible = 0;
            }
        }
        break;
    case ON_CHANGE_INIT:
        if (timer_background->timer_status == TIMER_INIT) {
            redraw_timer_num();                       // 计算倒计时的总时间
            timer_background->total_num = timer_background->countdown_num;
            if (timer_background->total_num == 0) {
                ui_hide(TIMER_START_LAYOUT);
                ui_show(TIMER_STOP_LAYOUT);
                return 0;
            }
            cal_circle_percent();
            ui_progress_set_persent_by_id(TIMER_PROGRESS, 100);

            if (!timer_background->timer_countdown_num_id) {
                timer_background->timer_countdown_num_id = sys_timer_add(NULL, countdown_timer, 1000);
            }
            if (!timer->timer_show_id) {
                timer->timer_show_id = sys_timer_add(NULL, show_timer, 500);
            }
            timer_background->timer_status = TIMER_START;
            timer_background->percent = 100;
        } else if (timer_background->timer_status == TIMER_STOP) {

            time.hour = timer_background->countdown_num / 3600;
            time.min = (timer_background->countdown_num % 3600) / 60;
            time.sec = ((timer_background->countdown_num % 3600) % 60);
            ui_time_update_by_id(TIMER_COUNTDOWN_NUM, &time);
            ui_progress_set_persent_by_id(TIMER_PROGRESS, timer_background->percent);
            pic = ui_pic_for_id(TIMER_START_STOP_BUTTON);
            if (pic) {
                ui_pic_set_image_index(pic, 1);
            }

            if (!timer->timer_show_id) {
                timer->timer_show_id = sys_timer_add(NULL, show_timer, 500);
            }
        } else if (timer_background->timer_status == TIMER_START) {
            time.hour = timer_background->countdown_num / 3600;
            time.min = (timer_background->countdown_num % 3600) / 60;
            time.sec = ((timer_background->countdown_num % 3600) % 60);

            countdown_time = ui_time_for_id(TIMER_COUNTDOWN_NUM);
            if (countdown_time) {
                ui_time_update(countdown_time, &time);
            }

            progress = ui_progress_for_id(TIMER_PROGRESS);
            if (progress) {
                ui_progress_set_persent(progress, timer_background->percent);
            }

            if (!timer->timer_show_id) {
                timer->timer_show_id = sys_timer_add(NULL, show_timer, 500);
            }
        }
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_RELEASE:
        if (timer_background->timer_status == TIMER_START) {
            timer_background->timer_background_prev_time = jiffies_msec();
        }

        if (timer && timer_background->timer_countdown_num_id && timer_background->timer_status == TIMER_STOP) {
            sys_timer_del(timer_background->timer_countdown_num_id);
            timer_background->timer_countdown_num_id = 0;
        }
        if (timer && timer->timer_show_id) {
            sys_timer_del(timer->timer_show_id);
            timer->timer_show_id = 0;
        }
        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return 0;
}

static int timer_start_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(TIMER_START_LAYOUT);
        ui_show(TIMER_LAYOUT);
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_START_LAYOUT)//通用-垂直列表
.onchange = timer_start_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/* static int timer_progress_onchange(void *_ctrl, enum element_change_event event, void *arg) */
/* { */
/*     struct ui_progress *progress = (struct ui_progress *)_ctrl; */
/*      */
/*     switch (event) { */
/*     case ON_CHANGE_INIT: */
/*         printf("@@@@@@@@@@@@@@@@@@%s\n", __func__); */
/*         ui_progress_set_persent(progress, 100); */
/*         break; */
/*     case ON_CHANGE_RELEASE: */
/*         break; */
/*     default: */
/*         break; */
/*     } */
/*     return 0; */
/* } */
/* REGISTER_UI_EVENT_HANDLER(TIMER_PROGRESS)//通用-垂直列表 */
/* .onchange = timer_progress_onchange, */
/*  .onkey = NULL, */
/*   .ontouch = NULL, */
/* }; */



static int timer_button_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case TIMER_START_STOP_BUTTON:
            if (timer_background->timer_status == TIMER_START) {
                if (timer_background->timer_countdown_num_id) {
                    sys_timer_del(timer_background->timer_countdown_num_id);
                    timer_background->timer_countdown_num_id = 0;
                }
                timer_background->timer_status = TIMER_STOP;
                ui_pic_show_image_by_id(TIMER_START_STOP_BUTTON, 1);
            } else if (timer_background->timer_status == TIMER_STOP) {
                if (!timer_background->timer_countdown_num_id) {
                    timer_background->timer_countdown_num_id = sys_timer_add(NULL, countdown_timer, 1000);
                }
                timer_background->timer_status = TIMER_START;
                ui_pic_show_image_by_id(TIMER_START_STOP_BUTTON, 0);
            }
            break;
        case TIMER_START_BACK_BUTTON:
            if (timer_background->timer_status == TIMER_START) {
                timer_background->timer_background_prev_time = jiffies_msec();
                if (!timer_background->timer_end_id && timer_background->countdown_num != 0) {
                    timer_background->timer_end_id = sys_timeout_add(NULL, background_timer_end_show, (timer_background->countdown_num) * 1000);
                }
            }

            if (timer && timer_background->timer_countdown_num_id && timer_background->timer_status == TIMER_STOP) {
                sys_timer_del(timer_background->timer_countdown_num_id);
                timer_background->timer_countdown_num_id = 0;
            }
            if (timer && timer->timer_show_id) {
                sys_timer_del(timer->timer_show_id);
                timer->timer_show_id = 0;
            }

            u8 menu_style = get_ui_sys_param(MenuStyle);
            if (ui_show_menu_sw(menu_style)) {
                ui_show_menu_force();
            }

            ui_auto_shut_down_enable();
            break;
        case TIMER_START_EXIT_BUTTON:
            if (timer_background->timer_countdown_num_id) {
                sys_timer_del(timer_background->timer_countdown_num_id);
                timer_background->timer_countdown_num_id = 0;
            }
            if (timer->timer_show_id) {
                sys_timer_del(timer->timer_show_id);
                timer->timer_show_id = 0;
            }
            timer_background->timer_status = TIMER_INIT;
            memset(timer_background, 0, sizeof(struct timer_background_param_t));
            memset(timer, 0, sizeof(struct timer_param_t));
            ui_hide(TIMER_START_LAYOUT);
            ui_show(TIMER_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(TIMER_START_STOP_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = timer_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TIMER_START_EXIT_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = timer_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TIMER_START_BACK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = timer_button_ontouch,
};



static int timer_end_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        if (timer_background->timer_is_end == 1) {
            layout->elm.css.invisible = 0;
        }
        break;
    case ON_CHANGE_INIT:
        key_ui_takeover(1);
        ui_auto_shut_down_disable();
        timer_background->timer_is_end = 0;
        timer_background->timer_status = TIMER_INIT;
        timer->time_cnt = 3;
        if (!timer->timer_end_id) {
            timer->timer_end_id = sys_timer_add(NULL, timer_end_show, 1000);
            printf("timer %d\n", timer->timer_end_id);
        }

        UI_MOTO_RUN(2);

        break;
    case ON_CHANGE_RELEASE:
        UI_WINDOW_PREEMPTION_DEL(ID_WINDOW_TIMER);   // 防止在组件滑动不能pop
        key_ui_takeover(0);
        ui_auto_shut_down_enable();
        tone_player_stop();
        break;
    default:
        break;
    }
    return 0;
}

static int timer_end_layout_onkey(void *ctr, struct element_key_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->value) {
    case KEY_UI_HOME:
        if (timer_background->timer_is_background == 1) {
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_TIMER);
            if (UI_WINDOW_PREEMPTION_CHECK()) {
                return false;
            }
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(ID_WINDOW_DIAL);
        } else {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(ID_WINDOW_DIAL);
        }
        return true;
        break;
    case KEY_UI_SHORTCUT:
        return true;
        break;
    default:
        break;
    }

    return false;
}

static int timer_end_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        if (timer_background->timer_is_background == 1) {
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_TIMER);
        } else {
            ui_hide(TIMER_STOP_LAYOUT);
            ui_show(TIMER_LAYOUT);
        }
        /* ui_hide(TIMER_STOP_LAYOUT); */
        /* ui_show(TIMER_LAYOUT); */

        if (timer && timer->timer_end_id) {
            sys_timer_del(timer->timer_end_id);
            timer->timer_end_id = 0;
        }

        UI_MOTO_RUN(0);
        ui_auto_shut_down_enable();
        return true;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        if (get_need_password() == 1) {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(ID_WINDOW_POWERON_PASSWORD);
            return true;
        }
        return true;
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_STOP_LAYOUT)//通用-垂直列表
.onchange = timer_end_layout_onchange,
 .onkey = timer_end_layout_onkey,
  .ontouch = timer_end_layout_ontouch,
};

#endif
#endif
