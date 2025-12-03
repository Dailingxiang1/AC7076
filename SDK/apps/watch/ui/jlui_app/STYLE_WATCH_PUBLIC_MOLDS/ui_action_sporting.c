#include "app_config.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "health_manager/health_manager.h"
#include "sport_info_sync.h"
#include "rcsp_manage.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-SPORTING]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_sporting.data.bss")
#pragma data_seg(".ui_action_sporting.data")
#pragma const_seg(".ui_action_sporting.text.const")
#pragma code_seg(".ui_action_sporting.text")
#endif


#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_ENABLE_SPORTING


#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)


static struct sporting_type {
    int timer;
    u32 base;
    u8 base_enable;
    u8 is_lock;
} *__this = NULL;

int ui_show_main(int id);
int ui_hide_curr_main();


static void sport_redraw(void *ctrl)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    ui_core_redraw(grid);
}

int sport_pause_handler(const char *type, u32 arg)
{
    struct ui_grid *grid = NULL;
    grid = ui_grid_for_id(SPORTING_LIST);
    ui_auto_shut_down_re_run();

    __this->base_enable = !__this->base_enable;

    if (ui_grid_cur_item(grid) == 1) {
        ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, 320, sport_redraw);
        ui_pic_show_image_by_id(SPORTING_CTL, 0);
    } else if (ui_grid_cur_item(grid) == 0) {
        ui_pic_show_image_by_id(SPORTING_CTL, 1);

    }

    return 0;
}

int sport_continue_handler(const char *type, u32 arg)
{
    struct ui_grid *grid = NULL;
    grid = ui_grid_for_id(SPORTING_LIST);
    ui_auto_shut_down_re_run();

    __this->base_enable = !__this->base_enable;

    if (ui_grid_cur_item(grid) == 0) {
        ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, -320, sport_redraw);
    }

    return 0;
}

static int sporting_lock_ontouch(void *elm, struct element_touch_event *e)
{
    static u32 idx = 0;
    struct element *list_elm = ui_core_get_element_by_id(SPORTING_LIST);
    struct ui_grid *grid = ui_grid_for_id(SPORTING_LIST);

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        if (++idx % 2) {
            ui_pic_show_image_by_id(SPORTING_LOCK, 1);
            __this->is_lock = 1;
        } else {
            ui_pic_show_image_by_id(SPORTING_LOCK, 0);
            __this->is_lock = 0;
        }
        return TRUE;
        break;

    default:
        break;
    }
    return FALSE;
}


static int sporting_ctl_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_SHOW:
        ui_pic_set_image_index((struct ui_pic *)ctr, !__this->base_enable);
        break;

    default:
        return FALSE;
    }
    return FALSE;
}


static int sporting_ctl_ontouch(void *elm, struct element_touch_event *e)
{
    struct ui_grid *grid = NULL;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        grid = ui_grid_for_id(SPORTING_LIST);
        __this->base_enable = !__this->base_enable;
        ui_pic_show_image_by_id(SPORTING_CTL, !__this->base_enable);
        if (__this->base_enable) {
            ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, -320, sport_redraw);
            sport_health_ctrl_sport_continue();
            sport_info_sync_keep_exercise_rcsp();
        } else {
            sport_health_ctrl_sport_pause();
            sport_info_sync_pause_exercise_rcsp();
        }
        break;

    default:
        break;
    }
    return FALSE;
}


static int sporting_exit_ontouch(void *elm, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        ui_auto_goto_dial_enable();       // 开启自动回表盘功能
        sport_health_ctrl_sport_stop();
        sport_info_sync_end_exercise_by_fw();
        os_time_dly(1);
        ui_hide_curr_main();
        ui_show_main(ID_WINDOW_SPORT_RESULT);
        UI_WINDOW_BACK_DEL(ID_WINDOW_SPORTING);

        /* if (__this && __this->timer) {      // 只有在手动停止运动才释放 */
        /* sys_timer_del(__this->timer); */
        /* } */

        /* if (__this) { */
        /* free(__this); */
        /* __this = NULL; */
        /* } */
        break;

    default:
        break;
    }
    return FALSE;
}


static int sporting_init(int p)
{
    struct element *elm;

    elm = ui_core_get_element_by_id(SPORTING_LIST);
    ui_grid_energy_auto_center((struct ui_grid *)elm, 1);
    ui_grid_set_energy_target_line((struct ui_grid *)elm, 160);
    return 0;
}


static int sporting_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct element *elm;
    elm = ui_core_get_element_by_id(SPORTING_LIST);
    switch (e) {
    case ON_CHANGE_INIT:
        break;

    case ON_CHANGE_FIRST_SHOW:
        //ui_set_call(sporting_init, 0);
        break;

    case ON_CHANGE_RELEASE:
        break;

    case ON_CHANGE_SHOW_COMPLETED:
        //ui_grid_slide((struct ui_grid *)elm, SCROLL_DIRECTION_LR, 320);
        //ui_grid_set_hi_index((struct ui_grid *)elm, 1);
        break;

    case ON_CHANGE_SHOW_POST:
        break;

    default:
        return FALSE;
    }
    return FALSE;
}

static int show_sport_data(int flag)
{
    struct unumber num;
    struct ui_number *number = NULL;
    struct utime t = {0};
    struct ui_time *time = NULL;

    num.type = TYPE_NUM;
    num.numbs = 2;
    num.number[0] = 66;//percent;
    number = ui_number_for_id(SPORTING_BPM);
    if (number) {
        ui_number_update(number, &num);
    }
    num.number[0] =  sport_health_get_sport_calories();
    number = ui_number_for_id(SPORTING_KCAL);
    if (number) {
        ui_number_update(number, &num);
    }
    num.number[0] = sport_health_get_sport_steps();
    number = ui_number_for_id(SPORTING_STEPS);
    if (number) {
        ui_number_update(number, &num);
    }
    int sport_distance = sport_health_get_sport_distance();
    num.number[0] = sport_distance / 100;
    num.number[1] = sport_distance % 100;
    number = ui_number_for_id(SPORTING_KM);
    if (number) {
        ui_number_update(number, &num);
    }

    __this->base = sport_health_get_sport_time();
    t.hour = __this->base / 3600;
    t.min  = (__this->base % 3600) / 60;
    t.sec  = (__this->base % 3600) % 60;
    time = ui_time_for_id(SPORTING_TIME);
    if (time) {
        ui_time_update(time, &t);
    }
    ui_core_redraw(time->text.elm.parent);

    return 0;
}

static void sporting_timer(void *priv)
{
    if (!__this) {
        return;
    }

    if (__this->base_enable) {
        if (!get_screen_saver_status()) {
            show_sport_data(0);
        }
        int get_sport_target_value(u8 id);
        if (get_sport_target_value(1) && __this->base > (get_sport_target_value(1) * 60)) {
            ui_hide_curr_main();
            ui_show_main(ID_WINDOW_SPORT_RESULT);
            UI_WINDOW_BACK_DEL(ID_WINDOW_SPORTING);
        }
    }
}


static int sporting_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        key_ui_takeover(1);
        ui_auto_shut_down_enable();
        ui_auto_goto_dial_disable();       // 关闭自动回表盘功能
        extern const struct uimsg_handl ui_msg_handler[];
        ui_register_msg_handler(ID_WINDOW_SPORTING, ui_msg_handler);
        if (!__this) {
            __this = zalloc(sizeof(struct sporting_type));
        }

        if (!__this->timer) {
            if (sport_health_get_sport_status() == RCSP_SPORT_STATUS_PAUSE) {
                __this->base_enable = FALSE;
            } else {
                __this->base_enable = TRUE;
            }
            __this->timer = sys_timer_add(NULL, sporting_timer, 1000);
        }

        ui_set_call(show_sport_data, 0);

        break;

    case ON_CHANGE_FIRST_SHOW:
        break;

    case ON_CHANGE_RELEASE:
        key_ui_takeover(0);
        if (__this && __this->timer) {
            sys_timer_del(__this->timer);
        }

        if (__this) {
            free(__this);
            __this = NULL;
        }
        break;

    default:
        return FALSE;
    }
    return FALSE;
}


static int ui_sporting_page_onkey(void *ctr, struct element_key_event *e)
{
    struct layer *layer = (struct layer *)ctr;
    struct ui_grid *grid = NULL;
    switch (e->value) {
    case KEY_UI_HOME:
        grid = ui_grid_for_id(SPORTING_LIST);
        __this->base_enable = !__this->base_enable;
        ui_pic_show_image_by_id(SPORTING_CTL, !__this->base_enable);

        if (__this->base_enable) {
            sport_health_ctrl_sport_continue();
            sport_info_sync_keep_exercise_rcsp();
            if (ui_grid_cur_item(grid) == 0) {
                ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, -320, sport_redraw);
            }
        } else {
            sport_health_ctrl_sport_pause();
            sport_info_sync_pause_exercise_rcsp();
            if (ui_grid_cur_item(grid) == 1) {
                ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, 320, sport_redraw);
            } else if (ui_grid_cur_item(grid) == 2) {
                ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, 640, sport_redraw);
            }
        }
        return true;
    case KEY_UI_SHORTCUT:
        grid = ui_grid_for_id(SPORTING_LIST);
        __this->base_enable = !__this->base_enable;
        ui_pic_show_image_by_id(SPORTING_CTL, !__this->base_enable);

        if (__this->base_enable) {
            sport_health_ctrl_sport_continue();
            sport_info_sync_keep_exercise_rcsp();
            if (ui_grid_cur_item(grid) == 0) {
                ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, -320, sport_redraw);
            } else if (ui_grid_cur_item(grid) == 2) {
                ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, 320, sport_redraw);
            }
        } else {
            sport_health_ctrl_sport_pause();
            sport_info_sync_pause_exercise_rcsp();
            if (ui_grid_cur_item(grid) == 1) {
                ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, 320, sport_redraw);
            } else if (ui_grid_cur_item(grid) == 2) {
                ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, 640, sport_redraw);
            }
        }
        return true;
    default:
        break;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(SPORTING_LOCK)
.onchange = NULL,
 .onkey    = NULL,
  .ontouch  = sporting_lock_ontouch,
};


REGISTER_UI_EVENT_HANDLER(SPORTING_CTL)
.onchange = sporting_ctl_onchange,
 .onkey    = NULL,
  .ontouch  = sporting_ctl_ontouch,
};


REGISTER_UI_EVENT_HANDLER(SPORTING_EXIT)
.onchange = NULL,
 .onkey    = NULL,
  .ontouch  = sporting_exit_ontouch,
};


REGISTER_UI_EVENT_HANDLER(SPORTING_LAYOUT)
.onchange = sporting_layout_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};



REGISTER_UI_EVENT_HANDLER(ID_WINDOW_SPORTING)
.onchange = sporting_page_onchange,
 .onkey    = ui_sporting_page_onkey,
  .ontouch  = NULL,
};




static int sporting_list_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT_PROBE:
        ui_grid_set_item_num(grid, 3);//修改列表条目数量,不能超过列表的最大数量
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_LR);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE1);
        break;

    case ON_CHANGE_INIT:
        ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_LR, -grid->item[1].elm.css.left, NULL);

        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_LR);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        break;

    default:
        break;
    }
    return FALSE;
}




static int sporting_list_ontouch(void *elm, struct element_touch_event *e)
{
    if (__this->is_lock) {         // 锁定屏幕
        return true;
    }

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
        return TRUE;


    default:
        break;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(SPORTING_LIST)
.onchange = sporting_list_onchange,
 .onkey    = NULL,
  .ontouch  = sporting_list_ontouch,
};


#endif/*#if TCFG_UI_ENABLE_SPORTING*/
#endif/*#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE*/


