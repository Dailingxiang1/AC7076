#include "app_config.h"
#include "jlui/ui.h"
#include "ui/ui_api.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "res/resfile.h"
#include "jlui_app/res_config.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_STOPWATCH]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_stopwatch.data.bss")
#pragma data_seg(".ui_action_stopwatch.data")
#pragma const_seg(".ui_action_stopwatch.text.const")
#pragma code_seg(".ui_action_stopwatch.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_STOPWATCH

#define STYLE_NAME  JL


#define STOPWATCH_INIT       0
#define STOPWATCH_START      1
#define STOPWATCH_PAUSE      2

#define STOPWATCH_COUNT_MAX			(1000*60*100 - 1)

struct stopwatch_param_t {
    u8 stopwatch_status;
    u8 split_time;
    u16 stopwatch_reflash_id;
    u32 time_start;
    u32 time_curr;
    u32 time_used;
    u32 time_period;
    u32 time_total;
    u32 time_lap_start;
    u32 time_lap_stop;
    u32 time_lap_period;
    u8 split_list[10];
    struct utime time_list[10];
};
static struct stopwatch_param_t *stopwatch = NULL;


static void add_time_list()
{
    u16 hour = 0;
    u16 min = 0;
    u8 sec = 0;
    u8 tem = 0;

    stopwatch->time_lap_period = stopwatch->time_lap_stop - stopwatch->time_lap_start;


    hour = stopwatch->time_lap_period / 3600000;
    min = (stopwatch->time_lap_period - 3600000 * hour) / 60000;
    sec = (stopwatch->time_lap_period - 3600000 * hour - 60000 * min) / 1000;
    tem = (stopwatch->time_lap_period % 1000) / 10;

    if (hour > 0) {
        stopwatch->time_list[stopwatch->split_time].hour = hour;
        stopwatch->time_list[stopwatch->split_time].min = min;
        stopwatch->time_list[stopwatch->split_time].sec = sec;
        stopwatch->split_list[stopwatch->split_time] = 1;
    } else {
        stopwatch->time_list[stopwatch->split_time].hour = min;
        stopwatch->time_list[stopwatch->split_time].min = sec;
        stopwatch->time_list[stopwatch->split_time].sec = tem;
        stopwatch->split_list[stopwatch->split_time] = 0;
    }
}

void stopwatch_num_reflash(void *priv)
{
    u8 hour = 0;
    u8 min = 0;
    u8 sec = 0;
    u8 tem = 0;
    struct unumber num;

    if (stopwatch->stopwatch_status == STOPWATCH_START) {
        stopwatch->time_curr = jiffies_msec();
        stopwatch->time_used = stopwatch->time_curr - stopwatch->time_start;     //做差计算得到秒表运行时间
        stopwatch->time_total = stopwatch->time_used + stopwatch->time_period;

        hour = stopwatch->time_total / 3600000;
        min = (stopwatch->time_total - 3600000 * hour) / 60000;
        sec = (stopwatch->time_total - 3600000 * hour - 60000 * min) / 1000;
        tem = (stopwatch->time_total % 1000) / 10;

        if (hour > 0) {
            ui_pic_show_image_by_id(STOPWATCH_SPLIT, 1);

            num.type = TYPE_NUM;
            num.numbs = 1;
            num.number[0] = sec;
            ui_number_update((struct ui_number *)ui_core_get_element_by_id(STOPWATCH_DATA_MS), &num);
            num.number[0] = min;
            ui_number_update((struct ui_number *)ui_core_get_element_by_id(STOPWATCH_DATA_S), &num);
            num.number[0] = hour;
            ui_number_update((struct ui_number *)ui_core_get_element_by_id(STOPWATCH_DATA_M), &num);

            struct element *elm = ui_core_get_element_by_id(STOPWATCH_DATA_MS);
            ui_core_redraw(elm->parent);
        } else {
            ui_pic_show_image_by_id(STOPWATCH_SPLIT, 0);

            num.type = TYPE_NUM;
            num.numbs = 1;
            num.number[0] = tem;
            ui_number_update((struct ui_number *)ui_core_get_element_by_id(STOPWATCH_DATA_MS), &num);
            num.number[0] = sec;
            ui_number_update((struct ui_number *)ui_core_get_element_by_id(STOPWATCH_DATA_S), &num);
            num.number[0] = min;
            ui_number_update((struct ui_number *)ui_core_get_element_by_id(STOPWATCH_DATA_M), &num);

            struct element *elm = ui_core_get_element_by_id(STOPWATCH_DATA_MS);
            ui_core_redraw(elm->parent);
        }


    } else if (stopwatch->stopwatch_status == STOPWATCH_INIT) {
        ui_pic_show_image_by_id(STOPWATCH_SPLIT, 0);

        num.type = TYPE_NUM;
        num.numbs = 1;
        num.number[0] = tem;
        ui_number_update((struct ui_number *)ui_core_get_element_by_id(STOPWATCH_DATA_MS), &num);
        num.number[0] = sec;
        ui_number_update((struct ui_number *)ui_core_get_element_by_id(STOPWATCH_DATA_S), &num);
        num.number[0] = min;
        ui_number_update((struct ui_number *)ui_core_get_element_by_id(STOPWATCH_DATA_M), &num);

        struct element *elm = ui_core_get_element_by_id(STOPWATCH_DATA_MS);
        ui_core_redraw(elm->parent);

    }
}


static int stopwatch_page_onkey(void *ctr, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_UI_SHORTCUT:
        if (stopwatch->stopwatch_status == STOPWATCH_START) {
            stopwatch->time_period += stopwatch->time_used;          // 暂停前的一段时间
            stopwatch->time_used = 0;

            stopwatch->stopwatch_status = STOPWATCH_PAUSE;

            ui_pic_show_image_by_id(STOPWATCH_PP_BUTTON, 0);
            ui_pic_show_image_by_id(STOPWATCH_SPLIT_BUTTON, 2);
        } else if (stopwatch->stopwatch_status == STOPWATCH_PAUSE) {
            stopwatch->time_start = jiffies_msec();           // 暂停后再按下开始计时，获取当前时间
            stopwatch->stopwatch_status = STOPWATCH_START;

            ui_pic_show_image_by_id(STOPWATCH_PP_BUTTON, 1);
            ui_pic_show_image_by_id(STOPWATCH_SPLIT_BUTTON, 1);
        } else {
            stopwatch->stopwatch_status = STOPWATCH_START;

            stopwatch->time_start = jiffies_msec();           // 按下开始计时，获取当前时间
            stopwatch->time_lap_start = 0;

            ui_pic_show_image_by_id(STOPWATCH_PP_BUTTON, 1);
            ui_pic_show_image_by_id(STOPWATCH_SPLIT_BUTTON, 1);
        }
        return true;
        break;
    case KEY_UI_HOME:
        int row, col;
        struct rect r;
        struct ui_grid *grid = NULL;

        if (stopwatch->stopwatch_status == STOPWATCH_START) {                // 分段计时
            if (stopwatch->split_time == 10) {
                return false;
            }
            stopwatch->time_lap_stop = stopwatch->time_total;
            add_time_list();
            stopwatch->time_lap_start = stopwatch->time_lap_stop;
            stopwatch->split_time++;
            if (stopwatch->split_time == 1) {
                row = 1;
                col = 1;
            } else {
                row = 1;
                col = 0;
            }

            ui_grid_add_dynamic_by_id(STOPWATCH_TIME_VLIST, &row, &col, 0);

            grid = ui_grid_for_id(STOPWATCH_TIME_VLIST);
            ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
            ui_grid_slide(grid, SCROLL_DIRECTION_UD, (stopwatch->split_time - 3) * (-(r.height + grid->y_interval)));
        } else {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(ID_WINDOW_DIAL);
        }
        return true;
        break;
    default:
        break;
    }

    return false;
}

static int stopwatch_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        key_ui_takeover(1);
        puts("\n***stopwatch_onchange***\n");
        stopwatch = zalloc(sizeof(struct stopwatch_param_t));
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_RELEASE:
        key_ui_takeover(0);
        if (stopwatch) {
            if (stopwatch->stopwatch_reflash_id) {
                sys_timer_del(stopwatch->stopwatch_reflash_id);
                stopwatch->stopwatch_reflash_id = 0;
            }
            free(stopwatch);
            stopwatch = NULL;
        }
        ui_auto_shut_down_enable();
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_STOPWATCH)
.onchange = stopwatch_page_onchange,
 .onkey = stopwatch_page_onkey,
  .ontouch = NULL,
};

static int stopwatch_num_onchange(void *ctrl, enum element_change_event e, void *arge)
{
    struct ui_number *num = (struct ui_number *) ctrl;
    switch (e) {
    case ON_CHANGE_INIT:
        if (stopwatch->stopwatch_reflash_id == 0) {
            stopwatch->stopwatch_reflash_id = sys_timer_add(NULL, stopwatch_num_reflash, 90);
        }

        break;
    case ON_CHANGE_RELEASE:
        if (stopwatch && stopwatch->stopwatch_reflash_id) {
            sys_timer_del(stopwatch->stopwatch_reflash_id);
            stopwatch->stopwatch_reflash_id = 0;
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(STOPWATCH_LAYOUT)
.onchange = stopwatch_num_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



static int stopwatch_pp_button_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    struct ui_grid *grid = NULL;
    static u8 touch_action = 0;
    int row, col;
    struct rect r;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case STOPWATCH_PP_BUTTON:
            if (stopwatch->stopwatch_status == STOPWATCH_INIT) {             // 开始计时

                stopwatch->stopwatch_status = STOPWATCH_START;

                stopwatch->time_start = jiffies_msec();           // 按下开始计时，获取当前时间
                stopwatch->time_lap_start = 0;

                ui_pic_show_image_by_id(STOPWATCH_PP_BUTTON, 1);
                ui_pic_show_image_by_id(STOPWATCH_SPLIT_BUTTON, 1);

            } else if (stopwatch->stopwatch_status == STOPWATCH_START) {     // 暂停计时

                stopwatch->time_period += stopwatch->time_used;          // 暂停前的一段时间
                stopwatch->time_used = 0;

                stopwatch->stopwatch_status = STOPWATCH_PAUSE;

                ui_pic_show_image_by_id(STOPWATCH_PP_BUTTON, 0);
                ui_pic_show_image_by_id(STOPWATCH_SPLIT_BUTTON, 2);
            } else if (stopwatch->stopwatch_status == STOPWATCH_PAUSE) {     // 暂停后重新计时

                stopwatch->time_start = jiffies_msec();           // 暂停后再按下开始计时，获取当前时间
                stopwatch->stopwatch_status = STOPWATCH_START;

                ui_pic_show_image_by_id(STOPWATCH_PP_BUTTON, 1);
                ui_pic_show_image_by_id(STOPWATCH_SPLIT_BUTTON, 1);
            }
            break;
        case STOPWATCH_SPLIT_BUTTON:
            if (stopwatch->stopwatch_status == STOPWATCH_START) {                // 分段计时
                if (stopwatch->split_time == 10) {
                    return false;
                }
                stopwatch->time_lap_stop = stopwatch->time_total;
                add_time_list();
                stopwatch->time_lap_start = stopwatch->time_lap_stop;
                stopwatch->split_time++;
                if (stopwatch->split_time == 1) {
                    row = 1;
                    col = 1;
                } else {
                    row = 1;
                    col = 0;
                }

                ui_grid_add_dynamic_by_id(STOPWATCH_TIME_VLIST, &row, &col, 0);

                grid = ui_grid_for_id(STOPWATCH_TIME_VLIST);
                ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
                ui_grid_slide(grid, SCROLL_DIRECTION_UD, (stopwatch->split_time - 3) * (-(r.height + grid->y_interval)));

            } else if (stopwatch->stopwatch_status == STOPWATCH_PAUSE) {         // 重置计时器
                if (stopwatch->split_time != 0) {
                    row = stopwatch->split_time;
                    col = 1;
                } else {
                    row = 0;
                    col = 0;
                }
                ui_grid_del_dynamic_by_id(STOPWATCH_TIME_VLIST, &row, &col, 0);
                stopwatch->stopwatch_status = STOPWATCH_INIT;
                stopwatch->split_time = 0;
                memset(stopwatch->time_list, 0, sizeof(stopwatch->time_list));
                memset(stopwatch->split_list, 0, sizeof(stopwatch->split_list));

                stopwatch->time_start = 0;
                stopwatch->time_curr = 0;
                stopwatch->time_used = 0;
                stopwatch->time_period = 0;
                stopwatch->time_total = 0;

                stopwatch->time_lap_start = 0;
                stopwatch->time_lap_stop = 0;

                ui_pic_show_image_by_id(STOPWATCH_PP_BUTTON, 0);
                ui_pic_show_image_by_id(STOPWATCH_SPLIT_BUTTON, 0);
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

REGISTER_UI_EVENT_HANDLER(STOPWATCH_PP_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = stopwatch_pp_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_SPLIT_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = stopwatch_pp_button_ontouch,
};


static int vlist_time_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    struct element_css *css;
    int row, col;

    switch (event) {
    case ON_CHANGE_INIT:
        if (stopwatch->split_time == 0) {
            row = stopwatch->split_time;
            col = 0;
        } else {
            row = stopwatch->split_time;
            col = 1;
        }

        ui_grid_init_dynamic(grid, &row, &col);
        log_info("dynamic_grid %d X %d\n", row, col);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE1);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_VLIST)
.onchange = vlist_time_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int stopwatch_text_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 10;
        printf("text index %d\n", index);
        switch (text->elm.id) {
        case STOPWATCH_TEXT1:
        case STOPWATCH_TEXT2:
        case STOPWATCH_TEXT3:
        case STOPWATCH_TEXT4:
            break;
        default:
            return FALSE;
        }
        ui_text_set_index(text, index);
        break;
    default:
        break;
    }

    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TEXT1)
.onchange = stopwatch_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TEXT2)
.onchange = stopwatch_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TEXT3)
.onchange = stopwatch_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TEXT4)
.onchange = stopwatch_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int stopwatch_time_ms_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *ui_num = (struct ui_number *)_ctrl;
    struct ui_grid *grid = NULL;
    struct unumber num;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 10;
        printf("time index %d\n", index);
        switch (ui_num->text.elm.id) {
        case STOPWATCH_TIME_MS1:
        case STOPWATCH_TIME_MS2:
        case STOPWATCH_TIME_MS3:
        case STOPWATCH_TIME_MS4:
            break;
        default:
            return FALSE;
        }
        //ui_time_update(time, &stopwatch->time_list[index]);

        num.type = TYPE_NUM;
        num.numbs = 1;
        num.number[0] = stopwatch->time_list[index].sec;
        ui_number_update(ui_num, &num);

        break;
    default:
        break;
    }

    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_MS1)
.onchange = stopwatch_time_ms_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_MS2)
.onchange = stopwatch_time_ms_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_MS3)
.onchange = stopwatch_time_ms_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_MS4)
.onchange = stopwatch_time_ms_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int stopwatch_time_s_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *ui_num = (struct ui_number *)_ctrl;
    struct ui_grid *grid = NULL;
    struct unumber num;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 10;
        printf("time index %d\n", index);
        switch (ui_num->text.elm.id) {
        case STOPWATCH_TIME_S1:
        case STOPWATCH_TIME_S2:
        case STOPWATCH_TIME_S3:
        case STOPWATCH_TIME_S4:
            break;
        default:
            return FALSE;
        }
        //ui_time_update(time, &stopwatch->time_list[index]);

        num.type = TYPE_NUM;
        num.numbs = 1;
        num.number[0] = stopwatch->time_list[index].min;
        ui_number_update(ui_num, &num);

        break;
    default:
        break;
    }

    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_S1)
.onchange = stopwatch_time_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_S2)
.onchange = stopwatch_time_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_S3)
.onchange = stopwatch_time_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_S4)
.onchange = stopwatch_time_s_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int stopwatch_time_m_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *ui_num = (struct ui_number *)_ctrl;
    struct ui_grid *grid = NULL;
    struct unumber num;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 10;
        printf("time index %d\n", index);
        switch (ui_num->text.elm.id) {
        case STOPWATCH_TIME_M1:
        case STOPWATCH_TIME_M2:
        case STOPWATCH_TIME_M3:
        case STOPWATCH_TIME_M4:
            break;
        default:
            return FALSE;
        }
        //ui_time_update(time, &stopwatch->time_list[index]);

        num.type = TYPE_NUM;
        num.numbs = 1;
        num.number[0] = stopwatch->time_list[index].hour;
        ui_number_update(ui_num, &num);

        break;
    default:
        break;
    }

    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_M1)
.onchange = stopwatch_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_M2)
.onchange = stopwatch_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_M3)
.onchange = stopwatch_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_M4)
.onchange = stopwatch_time_m_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int stopwatch_split_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    struct ui_grid *grid = NULL;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:
        index = (u32)arg;
        index = index % 10;
        printf("time index %d\n", index);
        switch (pic->elm.id) {
        case STOPWATCH_TIME_SPLIT1:
        case STOPWATCH_TIME_SPLIT2:
        case STOPWATCH_TIME_SPLIT3:
        case STOPWATCH_TIME_SPLIT4:
            break;
        default:
            return FALSE;
        }
        //ui_time_update(time, &stopwatch->time_list[index]);

        ui_pic_set_image_index(pic, stopwatch->split_list[index]);

        break;
    default:
        break;
    }

    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_SPLIT1)
.onchange = stopwatch_split_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_SPLIT2)
.onchange = stopwatch_split_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_SPLIT3)
.onchange = stopwatch_split_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(STOPWATCH_TIME_SPLIT4)
.onchange = stopwatch_split_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif
#endif
