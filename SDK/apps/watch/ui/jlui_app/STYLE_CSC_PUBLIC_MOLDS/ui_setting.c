#include "app_config.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "smartbox_user_app.h"
#include "events_adapter.h"
// #include "message/message_vm_cfg.h"
#include "poweroff.h"
#include "ui_sys_param.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_setting.data.bss")
#pragma data_seg(".ui_setting.data")
#pragma const_seg(".ui_setting.text.const")
#pragma code_seg(".ui_setting.text")
#endif


#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_SETTING]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE
#if (defined TCFG_UI_SETTING_ENABLE && TCFG_UI_SETTING_ENABLE)


#define STYLE_NAME  JL
#if ((defined SET_MENU && SET_MENU) && (defined TCFG_UI_SECOND_MENU && TCFG_UI_SECOND_MENU))

/***********************************************************************
 *  二级菜单列表
 **********************************************************************/

typedef struct screen_menu_map {
    u8 index;       // 触摸按下的索引
    u32 window;     // 对应的窗口ID
} SCREEN_MENU_MAP;

static s16 grid_item0_top;

bool get_touch_switch_flag(void);


//二级菜单
const static SCREEN_MENU_MAP second_level_menu_map[] = {
#if(defined ID_WINDOW_LCD_BRIGHTNESS && ID_WINDOW_LCD_BRIGHTNESS)
    {0,     ID_WINDOW_LCD_BRIGHTNESS 			},
#endif
#if(defined ID_WINDOW_LCD_LIGHT_TIME && ID_WINDOW_LCD_LIGHT_TIME)
    {1,     ID_WINDOW_LCD_LIGHT_TIME 		        },
#endif
#if(defined ID_WINDOW_TOUCH_AWAKE && ID_WINDOW_TOUCH_AWAKE)
    {2,     ID_WINDOW_TOUCH_AWAKE   			    },
#endif
#if(defined ID_WINDOW_TIME_SETTING && ID_WINDOW_TIME_SETTING)
    {3,     ID_WINDOW_TIME_SETTING 			},
#endif
#if(defined ID_WINDOW_ABOUT && ID_WINDOW_ABOUT)
    {4,     ID_WINDOW_ABOUT 	     		},
#endif
#if(defined ID_WINDOW_RESTART_SHUTDOWN && ID_WINDOW_RESTART_SHUTDOWN)
    {5,     ID_WINDOW_RESTART_SHUTDOWN 		        },
#endif
};


// 二级菜单列表选择函数
static u32 ui_second_level_menu_select(u32 index)
{
    for (int i = 0; i < (sizeof(second_level_menu_map) / sizeof(second_level_menu_map[0])); i++) {
        if (second_level_menu_map[i].index == index) {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(second_level_menu_map[i].window);
        }
    }
    return 0;
}

static int ui_second_level_menu_child_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct element *elm = (struct element *)ctr;
    int type = ui_id2type(elm->id);
    switch (type) {
    case CTRL_TYPE_TEXT:
        struct ui_text *text = (struct ui_text *)elm;
        if (!strcmp(text->source, "Tout")) {
            const char text_tip[4][5] = {"5S", "10S", "30S", "60S"};
            log_debug("func:%s , text->source:%s ,index:%d\n", __func__, text->source, get_ui_sys_param(DarkTime));
            log_debug("func:%s , &text_tip[get_ui_sys_param(DarkTime)]:%s \n", __func__, &text_tip[get_ui_sys_param(DarkTime)]);
            int dark_time_index = get_ui_sys_param(DarkTime);
            if (dark_time_index >= 0 && dark_time_index < 4) {
                ui_text_set_text_attrs(text, (const char *)&text_tip[dark_time_index], strlen((const char *)&text_tip[dark_time_index]), FONT_ENCODE_UTF8, 0, FONT_DEFAULT);
            }
        }
        break;
    case CTRL_TYPE_PIC:
        struct ui_pic *pic = (struct ui_pic *)elm;
        // log_info("func:%s , pic->source:%s ,index:%d\n" ,__func__, pic->source, get_touch_switch_flag());
        if (!strcmp(pic->source, "touch")) {
            ui_pic_set_image_index(pic, get_touch_switch_flag());
        }
        break;
    default:
        break;
    }
    return 0;
}


// 二级菜单列表触摸逻辑函数
static int menu_grid_vlist_ontouch(void *ctrl, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    struct element *elm = (struct element *)ctrl;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:

        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            int touch_index = ui_grid_touch_item((struct ui_grid *)elm);
            ui_second_level_menu_select(touch_index);
        }
        break;
    default:
        break;
    }
    return false;
}

// 二级菜单列表初始化函数
static int menu_grid_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        log_info("func:%s , grid_item0_top:%d, \n\t\tfile:%s\n", __func__, grid_item0_top, __FILE__);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        ui_grid_slide_with_callback(grid, SCROLL_DIRECTION_UD, grid_item0_top, NULL);
        ui_set_default_handler(&grid->elm, NULL, NULL, ui_second_level_menu_child_onchange);
        break;
    case ON_CHANGE_RELEASE:
        grid_item0_top = grid->item[0].elm.css.top;
        break;
    default:
        break;
    }
    return 0;
}


REGISTER_UI_EVENT_HANDLER(SET_MENU)
.onchange = menu_grid_vlist_onchange,
 .onkey    = NULL,
  .ontouch  = menu_grid_vlist_ontouch,
};

#endif /*#ifdef SET_MENU*/

#endif/*#if TCFG_UI_ENABLE_SETING*/
#endif

