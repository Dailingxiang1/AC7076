/* Copyright(C)
 * not free
 * All right reserved
 *
 * @file ui_example_animation.c
 * @brief JL_UI 动画使用示例
 * @author
 * @version
 * @date 2024-05-21
 */

#include "app_config.h"
#include "key_event_deal.h"
#include "ui.h"
#include "ui_api.h"
#include "ui_style.h"


#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-ANIM]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_animation_demo.data.bss")
#pragma data_seg(".ui_action_animation_demo.data")
#pragma const_seg(".ui_action_animation_demo.text.const")
#pragma code_seg(".ui_action_animation_demo.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ANIM_DEMO_ENABLE


#define STYLE_NAME  JL


#define CSS(x,X)        ((x)*10000/(X))             //绝对坐标转相对坐标


/*动画演示所需控件*/
#define UI_ANIM_EXAMPLE_LAYER   ANIM_EXAMPLE_LAYER

#define UI_ANIM_START_LAYOUT    ANIM_START_LAYOUT
#define UI_ANIM_BUTTON_PIC      ANIM_BUTTON_PIC
#define UI_ANIM_HELLO_TEXT      ANIM_HELLO_TEXT

#define UI_ANIM_PLAYBACK_LAYOUT ANIM_PLAYBACK_LAYOUT
#define UI_ANIM_1_LAYOUT        ANIM_1_LAYOUT




/************************************************
 *  动画 例子1
 ***********************************************/

static bool anim_button_flag;
void anim_x_cb(int var, int32_t v)
{
    /* printf("[%s] v:%d", __func__, v); */
    struct element *elm = ui_core_get_element_by_id(var);
    struct rect *screen_rect = ui_core_get_screen_draw_rect();

    /*重绘前需判断该控件是否存在*/
    if (elm == NULL) {
        return;
    }

    elm->css.left = CSS(v, screen_rect->width);
    ui_core_redraw(elm->parent);
}

static void anim_button_hander(bool flag)
{
    struct rect rect;
    struct element *elm = ui_core_get_element_by_id(UI_ANIM_HELLO_TEXT);
    if (elm == NULL) {
        return;
    }

    ui_core_get_element_abs_rect(elm, &rect);

    if (flag == true) {
        ui_anim_t a;
        ui_anim_init(&a);
        ui_anim_set_var(&a, UI_ANIM_HELLO_TEXT);
        ui_anim_set_values(&a, rect.left, 100);
        ui_anim_set_time(&a, 500);
        ui_anim_set_exec_cb(&a, anim_x_cb);
        ui_anim_set_path_cb(&a, ui_anim_path_overshoot);
        ui_anim_start(&a);
    } else {
        ui_anim_t a;
        ui_anim_init(&a);
        ui_anim_set_var(&a, UI_ANIM_HELLO_TEXT);
        ui_anim_set_values(&a, rect.left, -rect.width);
        ui_anim_set_time(&a, 500);
        ui_anim_set_exec_cb(&a, anim_x_cb);
        ui_anim_set_path_cb(&a, ui_anim_path_ease_in);
        ui_anim_start(&a);
    }
}


static int ui_anim_button_pic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        anim_button_flag = true;
        ui_pic_set_image_index(pic, anim_button_flag);
        break;
    case ON_CHANGE_RELEASE:
        ui_anim_del(UI_ANIM_HELLO_TEXT, anim_x_cb);
        break;
    default:
        return false;
    }
    return false;
}

static int ui_anim_button_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag == 0) {
            anim_button_flag = !anim_button_flag;
            ui_pic_set_image_index(pic, anim_button_flag);
            anim_button_hander(anim_button_flag);
        }
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(UI_ANIM_BUTTON_PIC)
.onchange = ui_anim_button_pic_onchange,
 .onkey = NULL,
  .ontouch = ui_anim_button_pic_ontouch,
};



/************************************************
 *  动画 例子2
 ***********************************************/

static void anim_layout_size_cb(int var, int32_t v)
{
    struct element *elm = ui_core_get_element_by_id(var);
    struct rect *screen_rect = ui_core_get_screen_draw_rect();

    if (elm == NULL) {
        return;
    }

    elm->css.width = CSS(v, screen_rect->width);
    elm->css.height = CSS(v, screen_rect->height);
    /*anim_x_cb(...) 执行了重绘，这里就不执行*/
}

static int ui_anim_1_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    struct rect rect;
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        ui_core_get_element_abs_rect(&layout->elm, &rect);
        ui_anim_t a;
        ui_anim_init(&a);
        ui_anim_set_var(&a, layout->elm.id);
        ui_anim_set_values(&a, rect.width, rect.width * 10);
        ui_anim_set_time(&a, 1000);
        ui_anim_set_playback_delay(&a, 100);
        ui_anim_set_playback_time(&a, 300);
        ui_anim_set_repeat_delay(&a, 500);
        ui_anim_set_repeat_count(&a, UI_ANIM_REPEAT_INFINITE);
        ui_anim_set_path_cb(&a, ui_anim_path_ease_in_out);

        ui_anim_set_exec_cb(&a, anim_layout_size_cb);
        ui_anim_start(&a);
        ui_anim_set_exec_cb(&a, anim_x_cb);
        ui_anim_set_values(&a, 10, 240);
        ui_anim_start(&a);
        break;
    case ON_CHANGE_RELEASE:
        ui_anim_del(layout->elm.id, anim_layout_size_cb);
        ui_anim_del(layout->elm.id, anim_x_cb);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(UI_ANIM_1_LAYOUT)
.onchange = ui_anim_1_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 * 1.相关初始化 2.按键切换动画演示
 ****************************************************/

static bool show_sub_layout_flag;
static int ui_anim_example_layer_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layer *layer = (struct layer *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        show_sub_layout_flag = 0;
        ui_auto_shut_down_disable();
        key_ui_takeover(1);
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();
        key_ui_takeover(0);
        break;
    default:
        return false;
    }
    return false;
}

static int ui_anim_example_layer_onkey(void *ctr, struct element_key_event *e)
{
    struct layer *layer = (struct layer *)ctr;
    switch (e->value) {
    case APP_MSG_JL_UI_HOME:
        show_sub_layout_flag = !show_sub_layout_flag;
        if (show_sub_layout_flag) {
            ui_hide(UI_ANIM_START_LAYOUT);
            ui_show(UI_ANIM_PLAYBACK_LAYOUT);
        } else {
            ui_hide(UI_ANIM_PLAYBACK_LAYOUT);
            ui_show(UI_ANIM_START_LAYOUT);
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(UI_ANIM_EXAMPLE_LAYER)
.onchange = ui_anim_example_layer_onchange,
 .onkey = ui_anim_example_layer_onkey,
  .ontouch = NULL,
};


#endif /* if TCFG_UI_ANIM_DEMO_ENABLE */
#endif /* #if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) */

