#include "app_config.h"
/* #include "app_task.h" */
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"

#include "res/resfile.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"
#include "jlui_app/ui_menu_manage.h"
#include "jlui_app/ui_app_effect.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_MENU]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_menu_manage.data.bss")
#pragma data_seg(".ui_menu_manage.data")
#pragma const_seg(".ui_menu_manage.text.const")
#pragma code_seg(".ui_menu_manage.text")
#endif


#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
extern void ui_card_disable();
extern u8 create_control_by_menu_set(u8 en);

#define MENU_SEL_USE_ID				0
#define MENU_APP_RECORD_NUM			4
struct app_rec {
    u8 style;
    u8 sel;
};
static  struct app_rec app_rec_tbl[MENU_APP_RECORD_NUM] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
typedef struct menu_map {
#if  MENU_SEL_USE_ID
    u32 sel[MENU_SEL_ID_MAX];
#else
    u8 sel[MENU_SEL_ID_MAX];
#endif
    u32 window;
} MENU_MAP;

static u8 menu_enter_anim_en  = 0;

const static MENU_MAP menu_map[] = {

    {0, 	0,	 	ID_WINDOW_DIAL					},
    {1, 	1,	 	ID_WINDOW_PHONE					},
    {2, 	2,	 	ID_WINDOW_NOTICE				},
    {3, 	3,	 	ID_WINDOW_MUSIC_PLAYER			},
    {4, 	4,	 	ID_WINDOW_ALARM					},
    {5, 	5,	 	ID_WINDOW_WEATHER				},
    {6, 	6,	 	ID_WINDOW_RUNLIGHT              },
    {7, 	7,	 	ID_WINDOW_SPORT_INTENSITY       },
    {8, 	8,	 	ID_WINDOW_OUTDOOR_SPORTS        },
    {9, 	9,	 	ID_WINDOW_INDOOR_SPORTS 		},

    {10, 	10,	 	ID_WINDOW_HEART					},
    {11, 	11,	 	ID_WINDOW_OXYGEN	            },
    {12, 	12,	 	ID_WINDOW_BLOODPRESSURE         },
    {13, 	13,	 	ID_WINDOW_BREATH_TRAIN          },
    {14, 	14,	 	ID_WINDOW_SLEEP		            },
    {15, 	15,	 	ID_WINDOW_SLEEPAID	            },
    {16, 	16,	 	ID_WINDOW_SIRI                  },
    {17, 	17,	 	ID_WINDOW_CALCULATOR			},
    {18, 	18,	 	ID_WINDOW_CALENDAR				},
    {19, 	19,	 	ID_WINDOW_MENU_WOMEN_HEALTH	    },

    {20, 	20,	 	ID_WINDOW_HEAT				    },
    {21, 	21,	 	ID_WINDOW_COMPASS               },
    {22, 	22,	 	ID_WINDOW_ALIPAY   		        },
    {23, 	23,	 	ID_WINDOW_TIMER                 },
    {24, 	24,	 	ID_WINDOW_STOPWATCH             },
    {25, 	25,	 	ID_WINDOW_PHOTOGRAGH			},
    {26, 	26,	 	ID_WINDOW_SETTING               },
    {27, 	27,	 	ID_WINDOW_RUBIKS_CUBE           },
    {28, 	28,	 	ID_WINDOW_APP_IFLYTEK			 	},
    {29, 	29,	 	ID_WINDOW_AI_DIAL           },
    {30, 	30,	 	ID_WINDOW_NET_IFLY               },
    /* {31, 	31,	 	ID_WINDOW_MOMENTUM			 	}, */
    {31, 	31,	 	ID_WINDOW_CAMERA			 	},
    /* {29, 	29,	 	29, 	}, */
    /* {30, 	30,	 	30, 	}, */
};

u32 ui_menu_app_record_set(u32 sel, u32 menu_type)
{
    int rec_swap  = -1;
    for (int i = MENU_APP_RECORD_NUM - 1; i >= 0; i--) {
        /* printf("rec_sel:%d sel:%d recstyle:%d type:%d i:%d", app_rec_tbl[i].sel, sel, app_rec_tbl[i].style, menu_type, i); */
        if ((app_rec_tbl[i].sel == sel) && (app_rec_tbl[i].style == menu_type)) {
            rec_swap = i;
        }
    }
    if (rec_swap != -1) {
        for (int i = rec_swap; i > 0; i--) {
            app_rec_tbl[i].sel = app_rec_tbl[i - 1].sel;
            app_rec_tbl[i].style = app_rec_tbl[i - 1].style;
        }
    } else {
        for (int i = MENU_APP_RECORD_NUM - 1; i > 0; i--) {
            app_rec_tbl[i].sel = app_rec_tbl[i - 1].sel;
            app_rec_tbl[i].style = app_rec_tbl[i - 1].style;
        }
    }
    app_rec_tbl[0].sel  = sel;
    app_rec_tbl[0].style = menu_type;
    /* printf("%s sel:%d type:%d\n", __func__, sel, menu_type); */
    return 0;
}
u32 ui_menu_app_record_get_sel(u32 index)
{
    /* printf("%s index:%d sel:%d\n", __func__, index, app_rec_tbl[index].sel); */
    return app_rec_tbl[index].sel;
}
u32 ui_menu_app_record_window(u32 index)
{
    u32 sel =  ui_menu_app_record_get_sel(index);
    /* printf("%s index:%d sel:%d\n",__func__,index ,app_rec_tbl[index].sel); */
    for (int i = 0; i < (sizeof(menu_map) / sizeof(menu_map[0])); i++) {
        if (menu_map[i].sel[MENU_APP_RECORD] == sel) {
            /* printf("__%s__sel:%d mapsel:%d win:%x", __func__, sel, menu_map[i].sel[MENU_APP_RECORD], menu_map[i].window); */
#if TCFG_UI_ENABLE_NOTICE
            if (menu_map[i].window == ID_WINDOW_NOTICE) {
                create_control_by_menu_set(1);
            }
#endif
            return menu_map[i].window;
        }
    }
    return 0;
}
u32 ui_menu_map_by_sel(u32 sel, u32 menu_type)
{

    for (int i = 0; i < (sizeof(menu_map) / sizeof(menu_map[0])); i++) {
        if (menu_map[i].sel[menu_type] == sel) {
            /* printf("__%s__sel:%d mapsel:%d type:%d win:%x", __func__, sel, menu_map[i].sel[menu_type], menu_type, menu_map[i].window); */
            ui_menu_app_record_set(menu_map[i].sel[MENU_APP_RECORD], menu_type);
            ui_card_disable();
#if TCFG_UI_ENABLE_NOTICE
            if (menu_map[i].window == ID_WINDOW_NOTICE) {
                create_control_by_menu_set(1);
            }
#endif
            return menu_map[i].window;
        }
    }
    return 0;
}

u32 ui_menu_enter_anim_enable()
{
    menu_enter_anim_en  = 1;
    return menu_enter_anim_en;
}
u32 ui_menu_enter_anim_disable()
{
    menu_enter_anim_en  = 0;
    return menu_enter_anim_en;
}
u32 ui_menu_enter_anim_flag_get()
{
    return menu_enter_anim_en;
}





#endif// CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
