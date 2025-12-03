#include "app_config.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"

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


#define abs(x)  ((x)>0?(x):-(x))

int ui_show_main(int id);
int ui_hide_curr_main();

static u8 g_target_index = 0;
static u32 g_target_value[3] = {0};
void sporting_target_index_set(u8 index)
{
    g_target_index = index;
}


int get_sport_target_value(u8 id)
{
    return g_target_value[g_target_index];
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


static int sporting_target_set_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct rect item_rect, r;
    u8 item_idx = 0, time_idx = 0, item_num[12];
    int center, ratio, num, num_init[3] = {1, 5, 100}, num_interval[3] = {1, 5, 100};
    struct element *elm, *item_elm, *item_child_elm, *time_elm[2];
    u32 list[3] = {SPORTING_TARGET_KM_LIST, SPORTING_TARGET_TIME_LIST, SPORTING_TARGET_CAL_LIST};

    elm = ui_core_get_element_by_id(list[g_target_index]);
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_energy_auto_center((struct ui_grid *)elm, 1);
        ui_grid_set_energy_target_line((struct ui_grid *)elm, 187);

        if (g_target_value[g_target_index]) {
            struct ui_grid *grid = ui_grid_for_id(list[g_target_index]);
            u8 index = (g_target_value[g_target_index] - num_init[g_target_index]) / num_interval[g_target_index];
            ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
            ui_grid_slide(grid, SCROLL_DIRECTION_UD, (index - 1) * (-(r.height + grid->y_interval)));
        }

        list_for_each_child_element(item_elm, elm) {
            time_idx = 0;
            num = num_init[g_target_index] + item_idx * num_interval[g_target_index];
            list_for_each_child_element(item_child_elm, item_elm) {
                if (num / 100) {
                    switch (time_idx) {
                    case 0:
                        ui_pic_set_image_index((struct ui_pic *)item_child_elm, num / 100);
                        break;

                    case 1:
                        ui_pic_set_image_index((struct ui_pic *)item_child_elm, num % 100 / 10);
                        break;

                    case 2:
                        ui_pic_set_image_index((struct ui_pic *)item_child_elm, num % 100 % 10);
                        break;

                    default:
                        break;
                    }
                } else {
                    switch (time_idx) {
                    case 0:
                        ui_pic_set_image_index((struct ui_pic *)item_child_elm, num / 10);
                        break;

                    case 1:
                        ui_pic_set_image_index((struct ui_pic *)item_child_elm, num % 10);
                        break;

                    default:
                        break;
                    }
                }
                time_idx++;
            }
            item_idx++;
        }

        //ui_text_set_index((struct ui_text *)(ui_core_get_element_by_id(SPORTING_TARGET_TITLE)), g_target_index);
        //ui_text_set_index((struct ui_text *)(ui_core_get_element_by_id(SPORTING_TARGET_UNIT)), g_target_index);
        //for (u8 i = 0; i < ARRAY_SIZE(list); i++) {
        //    elm = ui_core_get_element_by_id(list[i]);
        //    elm->css.invisible = (i == g_target_index) ? 0 : 1;
        //}
        break;

    case ON_CHANGE_RELEASE:
        break;

    case ON_CHANGE_SHOW_COMPLETED:
        break;

    case ON_CHANGE_SHOW_POST:
        list_for_each_child_element(item_elm, elm) {
            ui_core_get_element_abs_rect(item_elm, &item_rect);
            center = (item_rect.top + item_rect.height / 2);
            ratio = remap(abs(center - 187), 56, 0, 65, 100);
            list_for_each_child_element(item_child_elm, item_elm) {
                ui_core_set_element_ratio(item_child_elm, (float)ratio / 100, (float)ratio / 100, true);
                /* item_child_elm->css.ratio.en = 1; */
                /* item_child_elm->css.ratio.ratio_w = (float)ratio / 100; */
                /* item_child_elm->css.ratio.ratio_h = (float)ratio / 100; */
            }
        }
        break;

    default:
        return FALSE;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(SPORTING_TARGET_SET_KM_LAYOUT)
.onchange = sporting_target_set_layout_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};
REGISTER_UI_EVENT_HANDLER(SPORTING_TARGET_SET_TIME_LAYOUT)
.onchange = sporting_target_set_layout_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};
REGISTER_UI_EVENT_HANDLER(SPORTING_TARGET_SET_CAL_LAYOUT)
.onchange = sporting_target_set_layout_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};


static int sporting_target_set_ontouch(void *ctr, struct element_touch_event *e)
{
    static struct element *target;
    struct element *elm = (struct element *)ctr;
    u32 list[3] = {SPORTING_TARGET_KM_LIST, SPORTING_TARGET_TIME_LIST, SPORTING_TARGET_CAL_LIST};

    int num_init[3] = {1, 5, 100}, num_interval[3] = {1, 5, 100};
    struct element *grid = ui_core_get_element_by_id(list[g_target_index]);

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        target = elm;
        break;

    case ELM_EVENT_TOUCH_UP:
        if (target && target == elm) {
            g_target_value[g_target_index] = num_init[g_target_index] + ui_grid_get_hindex((struct ui_grid *)grid) * num_interval[g_target_index];
            target = NULL;
            ui_hide_curr_main();
            ui_show_main(ID_WINDOW_SPORT_TARGET);
            if (g_target_index == 0) {
                UI_WINDOW_BACK_DEL(ID_WINDOW_SPORT_TARGET_KM_SET);
            } else if (g_target_index == 1) {
                UI_WINDOW_BACK_DEL(ID_WINDOW_SPORT_TARGET_TIME_SET);
            } else {
                UI_WINDOW_BACK_DEL(ID_WINDOW_SPORT_TARGET_CAL_SET);
            }
        }
        break;

    case ELM_EVENT_TOUCH_MOVE:
        target = NULL;
        break;

    default:
        break;
    }
    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(SPORTING_SET_KM_BUTTON)
.onchange = NULL,
 .onkey    = NULL,
  .ontouch  = sporting_target_set_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SPORTING_SET_TIME_BUTTON)
.onchange = NULL,
 .onkey    = NULL,
  .ontouch  = sporting_target_set_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SPORTING_SET_CAL_BUTTON)
.onchange = NULL,
 .onkey    = NULL,
  .ontouch  = sporting_target_set_ontouch,
};


#endif/*#if TCFG_UI_ENABLE_SPORTING*/
#endif/*#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE*/



