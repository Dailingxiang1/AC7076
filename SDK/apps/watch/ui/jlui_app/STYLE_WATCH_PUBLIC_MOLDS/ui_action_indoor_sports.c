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

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-INDOORSPORTS]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_indoorsports.data.bss")
#pragma data_seg(".ui_action_indoorsports.data")
#pragma const_seg(".ui_action_indoorsports.text.const")
#pragma code_seg(".ui_action_indoorsports.text")
#endif

#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_ENABLE_INDOORSPORTS

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

#define abs(x)  ((x)>0?(x):-(x))

int ui_show_main(int id);
int ui_hide_curr_main();

static struct indoor_sports_type {
    s8 index;
    struct element *target;
} *__this = NULL;


static int indoor_sports_init(int p)
{
    struct rect item_child_rect;
    struct element *elm;

    elm = ui_core_get_element_by_id(INDOOR_SPORT_LIST);
    ui_grid_energy_auto_center((struct ui_grid *)elm, 1);
    ui_grid_set_energy_target_line((struct ui_grid *)elm, 232);
    return 0;
}


static int32_t remap(int32_t x, int32_t min_in, int32_t max_in, int32_t min_out, int32_t max_out)
{
    if (max_in >= min_in && x >= max_in) {
        return max_out;
    }
    if (max_in >= min_in && x <= min_in) {
        return min_out;
    }

    if (max_in <= min_in && x <= max_in) {
        return max_out;
    }
    if (max_in <= min_in && x >= min_in) {
        return min_out;
    }

    int32_t delta_in = max_in - min_in;
    int32_t delta_out = max_out - min_out;

    return ((x - min_in) * delta_out) / delta_in + min_out;
}


static int indoor_vlist_default_onchange(void *_ctrl, enum element_change_event event, void *arg)
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

static int indoor_sports_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int center, ratio;
    struct rect item_rect, item_child_rect;
    struct element *elm, *item_elm, *item_child_elm;

    elm = ui_core_get_element_by_id(INDOOR_SPORT_LIST);
    switch (e) {
    case ON_CHANGE_INIT:
        break;

    case ON_CHANGE_FIRST_SHOW:
        ui_set_call(indoor_sports_init, 0);
        break;

    case ON_CHANGE_RELEASE:
        break;

    case ON_CHANGE_SHOW_COMPLETED:
        //ui_grid_slide(elm, SCROLL_DIRECTION_UD, -59);
        break;

    case ON_CHANGE_SHOW_POST:
        list_for_each_child_element(item_elm, elm) {
            ui_core_get_element_abs_rect(item_elm, &item_rect);
            center = (item_rect.top + item_rect.height / 2);
            ratio = remap(abs(center - 232), 156, 0, 85, 100);

            //printf("left = %d, top = %d, w = %d, h = %d, center = %d, %d, ratio = %d, %d, %d\n", item_rect.left, item_rect.top, item_rect.width, item_rect.height, center, center - 232, ratio1, ratio2, item_elm->css.top);
            list_for_each_child_element(item_child_elm, item_elm) {
                ui_core_get_element_abs_rect(item_child_elm, &item_child_rect);
                switch (ui_id2type(item_child_elm->id)) {
                case CTRL_TYPE_PIC:
                    ui_core_get_element_abs_rect(item_child_elm, &item_child_rect);
                    ui_core_set_element_ratio(item_child_elm, (float)ratio / 100, (float)ratio / 100, true);
                    /* item_child_elm->css.ratio.en = 1; */
                    /* item_child_elm->css.ratio.ratio_w = (float)ratio / 100; */
                    /* item_child_elm->css.ratio.ratio_h = (float)ratio / 100; */
                    break;

                case CTRL_TYPE_TEXT:
                    break;

                default:
                    break;
                }
            }
        }

        break;

    default:
        return FALSE;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(INDOOR_SPORT_LAYOUT)
.onchange = indoor_sports_layout_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};


static int indoor_sports_list_ontouch(void *ctr, struct element_touch_event *e)
{
    u8 idx = 0;
    struct rect item_rect, child_rect;
    struct element *item_elm, *child;
    struct element *elm = (struct element *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        __this->index = ui_grid_touch_item((struct ui_grid *)elm);
        if (__this->index < 0) {
            break;
        }

        list_for_each_child_element(item_elm, elm) {
            if (idx++ == __this->index) {
                list_for_each_child_element(child, item_elm) {
                    ui_core_get_element_abs_rect(child, &child_rect);
                    if (child_rect.width < 40 && \
                        e->pos.x >= child_rect.left && e->pos.x <= (child_rect.left + child_rect.width) && \
                        e->pos.y >= child_rect.top && e->pos.y <= (child_rect.top + child_rect.height)) {
                        __this->target = child;
                        return FALSE;
                    }
                }
            }
        }

        break;

    case ELM_EVENT_TOUCH_UP:
        if (ui_grid_touch_item((struct ui_grid *)elm) < 0 || \
            __this->index != ui_grid_touch_item((struct ui_grid *)elm)) {
            break;
        }

        if (__this->target) {
            ui_core_get_element_abs_rect(__this->target, &child_rect);
            if (e->pos.x >= child_rect.left && e->pos.x <= (child_rect.left + child_rect.width) && \
                e->pos.y >= child_rect.top && e->pos.y <= (child_rect.top + child_rect.height)) {
                void sporting_target_record(u32 priv);
                sporting_target_record(ID_WINDOW_INDOOR_SPORTS);
                ui_hide_curr_main();
                ui_show_main(ID_WINDOW_SPORT_TARGET);
                UI_WINDOW_BACK_DEL(ID_WINDOW_INDOOR_SPORTS);
                __this->target = NULL;
                break;
            }
        }
        int sport_type  = sport_type_map_get_type(SPORT_MODE_INDOOR, __this->index);
        sport_health_set_sport_type(sport_type);
        sport_health_ctrl_sport_start();
        sport_info_sync_start_exercise_rcsp(NULL, 0);
        ui_hide_curr_main();
        ui_show_main(ID_WINDOW_SPORTING);
        UI_WINDOW_BACK_DEL(ID_WINDOW_INDOOR_SPORTS);
        break;

    case ELM_EVENT_TOUCH_MOVE:
        __this->index = -1;
        __this->target = NULL;
        break;

    default:
        break;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(INDOOR_SPORT_LIST)
.onchange = indoor_vlist_default_onchange,
 .onkey    = NULL,
  .ontouch  = indoor_sports_list_ontouch,
};


static int indoor_sports_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        if (!__this) {
            __this = zalloc(sizeof(struct indoor_sports_type));
        }
        break;

    case ON_CHANGE_FIRST_SHOW:
        ui_set_call(indoor_sports_init, 0);
        break;

    case ON_CHANGE_RELEASE:
        if (__this) {
            free(__this);
            __this = NULL;
        }
        break;

    case ON_CHANGE_SHOW_COMPLETED:
        break;

    default:
        return FALSE;
    }
    return FALSE;
}


static int indoor_sports_page_ontouch(void *elm, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide_curr_main();
        ui_show_main(ID_WINDOW_MENU_LIST);
        UI_WINDOW_BACK_DEL(ID_WINDOW_INDOOR_SPORTS);
        return TRUE;


    default:
        break;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(ID_WINDOW_INDOOR_SPORTS)
.onchange = indoor_sports_page_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};


#endif/*#if TCFG_UI_ENABLE_INDOORSPORTS*/
#endif/*#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE*/



