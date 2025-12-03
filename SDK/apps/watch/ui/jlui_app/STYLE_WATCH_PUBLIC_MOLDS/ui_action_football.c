#include "app_config.h"
#include "system/includes.h"
#include "asm/math_fast_function.h"
#include "ui_api.h"
#include "ui.h"
#include "ui_style.h"
#include "jlui_app/ui_menu_manage.h"
#include "app_task.h"
#include "system/timer.h"
#include "app_main.h"
#include "init.h"
#include "key_event_deal.h"
#include "data_weather_storage.h"
#include "res/font_ascii.h"
#include "gpu_port.h"
#include "football.h"
#include <math.h>

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-FOOTBALL]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_football.data.bss")
#pragma data_seg(".ui_action_football.data")
#pragma const_seg(".ui_action_football.text.const")
#pragma code_seg(".ui_action_football.text")
#endif


#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_FOOTBALL

#define STYLE_NAME  JL

#define SUPPORT_ICON_BACKGROUND_OVERLAY         0 //是否支持图标六边形背景叠加功能

extern struct ui_image_list *ui_pic_get_normal_image_list(struct ui_pic *pic);
extern struct ui_platform_api *ui_get_platform_api();

#if SUPPORT_ICON_BACKGROUND_OVERLAY
const static struct football_bg_rgb_t football_bg_rgb[] = { // 图标的底色rgb
    {255, 	255,	   255},     // ID_WINDOW_DIAL
    {36, 	139,	   234},       // ID_WINDOW_PHONE
    {60, 	225,	   59},      // ID_WINDOW_NOTICE
    {133, 	32,	       248},      // ID_WINDOW_MUSIC_PLAYER
    {255, 	129,	   25},       // ID_WINDOW_ALARM
    {37, 	140,	   235},     // ID_WINDOW_WEATHER
    {0, 	230,	   65},     // ID_WINDOW_OUTDOOR_SPORTS
    {251, 	27,	       37},      // ID_WINDOW_HEART
    {237, 	231,	   229},     // ID_WINDOW_OXYGEN
    {255, 	187,	   23},      // ID_WINDOW_BLOODPRESSURE
    {31, 	200,	   212},     // ID_WINDOW_BREATH_TRAIN
    {135, 	38,	       249},     // ID_WINDOW_SLEEP
    {255, 	47,	       47},     // ID_WINDOW_ECG
    {221, 	221,	   221},     // ID_WINDOW_CALCULATOR
    {255, 	187,	   23},      // ID_WINDOW_CALENDAR
    {253, 	18,	       79},     // ID_WINDOW_MENU_WOMEN_HEALTH
    {135, 	38,	       249},      // ID_WINDOW_TIMER
    {135, 	38,	       249},      // ID_WINDOW_STOPWATCH
    {250, 	180,	   9},     // ID_WINDOW_PHOTOGRAGH
    {210, 	209,	   208},     // ID_WINDOW_FINDPHONE
    {58, 	225,	   57},     // ID_WINDOW_STEP_CAL
    {251, 	178,	   7},     // ID_WINDOW_CUBE
    {255, 	6,	       61},     // ID_WINDOW_HEALTH_RING
    {35, 	139,	   234},     // ID_WINDOW_FACEBOOK
    {247, 	246,	   255},     // ID_WINDOW_TWITTER
    {56, 	224,	   56},     // ID_WINDOW_WAHTSAPP
    {30, 	137,	   234},     // ID_WINDOW_SETTING
    {0, 	0,	       0},       // ID_WINDOW_MOMENTUM
};

const static struct football_bg_rgb_t football_bg_3in1_rgb[60] = {
    /*     底色     */   /*    左上角    */   /*     右下角    */
    {0x71, 0x44, 0xff}, {0x63, 0x3E, 0xFF}, {0x8B, 0x50, 0xFF},//0
    {0xFF, 0x51, 0x58}, {0xFF, 0x44, 0x5B}, {0xFF, 0x68, 0x52},//1
    {0x11, 0xE1, 0xA3}, {0x01, 0xD0, 0xA7}, {0x2B, 0xFF, 0x9C},//2
    {0xFE, 0x62, 0x44}, {0xFE, 0x41, 0x3D}, {0xFF, 0x9E, 0x51},//3
    {0x71, 0x44, 0xff}, {0x63, 0x3E, 0xFF}, {0x8B, 0x50, 0xFF},//4
    {0x22, 0x86, 0xFE}, {0x13, 0x4C, 0xFD}, {0x3A, 0xE3, 0xFF},//5
    {0x11, 0xE2, 0xA3}, {0x01, 0xD1, 0xA8}, {0x2C, 0xFF, 0x9C},//6
    {0x52, 0xE3, 0x78}, {0x14, 0xD5, 0x7C}, {0xC4, 0xFD, 0x73},//7
    {0xFF, 0x45, 0x93}, {0xFF, 0x33, 0x66}, {0xFE, 0x64, 0xE2},//8
    {0x86, 0x41, 0xEB}, {0x3E, 0x35, 0xF2}, {0xFA, 0x55, 0xE0},//9
    {0xFE, 0x89, 0x25}, {0xFC, 0x69, 0x21}, {0xFF, 0xBF, 0x28},//10
    {0xB5, 0xC0, 0xEB}, {0xCA, 0xD5, 0xFF}, {0x93, 0x9F, 0xCB},//11
    {0x71, 0x44, 0xff}, {0x63, 0x3E, 0xFF}, {0x8B, 0x50, 0xFF},//12
    {0xFF, 0x51, 0x58}, {0xFF, 0x44, 0x5B}, {0xFF, 0x68, 0x52},//13
    {0x11, 0xE1, 0xA3}, {0x01, 0xD0, 0xA7}, {0x2B, 0xFF, 0x9C},//14
    {0xFE, 0x62, 0x44}, {0xFE, 0x41, 0x3D}, {0xFF, 0x9E, 0x51},//15
    {0x71, 0x44, 0xff}, {0x63, 0x3E, 0xFF}, {0x8B, 0x50, 0xFF},//16
    {0x22, 0x86, 0xFE}, {0x13, 0x4C, 0xFD}, {0x3A, 0xE3, 0xFF},//17
    {0x11, 0xE2, 0xA3}, {0x01, 0xD1, 0xA8}, {0x2C, 0xFF, 0x9C},//18
    {0x52, 0xE3, 0x78}, {0x14, 0xD5, 0x7C}, {0xC4, 0xFD, 0x73},//19
};
#endif

struct football_anim {
    ui_anim_t anim;
    float dist_x;
    float dist_y;
    int limit_v;	// 超过后按该值运行
    int limit_base;	// 最小值限制
    u8  negative_x;	// 负方向标记
    u8  negative_y;	// 负方向标记
    float hypotenuse; // 直角三角形斜边
    float sin_x;
};

struct football_priv {
    struct football_param football;
    struct element *elm;
    float x_diff; // x方向偏移
    float y_diff; // y方向偏移
    u8 hold_flag;
    u8 touch_down_flag;
    struct football_anim *p_anim;
};
static struct football_priv *__this = NULL;
static float last_pos[9] = {0};
/* #define abs(x)  ((x)>0?(x):-(x) ) */
/* #define abs(x)  __builtin_abs(x) */
#define abs(x)  __builtin_fabsf(x)

static void football_anim_exec_cb(int var, int32_t v)
{
    if (!__this) {
        return ;
    }
    if (!__this->p_anim) {
        return ;
    }
    /* log_info("\n\n v:%d \n", v); */
    if ((__this->p_anim->limit_v) && (v > __this->p_anim->limit_v)) {
        v = __this->p_anim->limit_v;
    }
    if (__this->p_anim->limit_base) {
        if (v > __this->p_anim->limit_base) {
            v -= __this->p_anim->limit_base;
        } else {
            v = 1;
        }
    }
    __this->p_anim->hypotenuse += v;
    float dist_x = __this->p_anim->sin_x * __this->p_anim->hypotenuse;
    float dist_y = complex_dqdt_float(__this->p_anim->hypotenuse, dist_x);
    float int_x = dist_x;
    float int_y = dist_y;
    if (__this->p_anim->negative_x) {
        int_x = -int_x;
    }
    if (__this->p_anim->negative_y) {
        int_y = -int_y;
    }
    __this->x_diff = int_x - __this->p_anim->dist_x;
    __this->y_diff = int_y - __this->p_anim->dist_y;
    __this->p_anim->dist_x = int_x;
    __this->p_anim->dist_y = int_y;

    /* log_info("dist:%d, %d, %d \n", (int)__this->p_anim->hypotenuse, int_x, int_y); */
    /* log_info("diff:%d, %d, v:%d \n", (int)(__this->x_diff * 100), (int)(__this->y_diff * 100), v); */

    ui_core_redraw(__this->elm);
}

static void football_anim_stop(void)
{
    if (!__this) {
        return ;
    }
    if (__this->p_anim) {
        ui_anim_del(FOOTBALL, NULL);
        free(__this->p_anim);
        __this->p_anim = NULL;
    }
}

static void football_anim_start(struct element_touch_event *e)
{
    int start_dist, end_dist;
    int run_time;

    // 释放旧的
    football_anim_stop();

    __this->p_anim = zalloc(sizeof(struct football_anim));
    ASSERT(__this->p_anim);

    // 动画参数计算
    __this->p_anim->dist_x = e->pos.x >> 16;
    __this->p_anim->dist_y = e->pos.y >> 16;
    float dist_x = abs(__this->p_anim->dist_x);
    float dist_y = abs(__this->p_anim->dist_y);
    __this->p_anim->hypotenuse = complex_abs_float(dist_x, dist_y); // 边长计算
    __this->p_anim->sin_x = dist_x / __this->p_anim->hypotenuse; //sinA
    int energy_t0 = (e->pos.x + 1) & 0xffff; //防止div0
    float vxy = __this->p_anim->hypotenuse / energy_t0;

    run_time = vxy * 800;

    log_info("x:%d, y:%d, en:%d \n", __this->p_anim->dist_x, __this->p_anim->dist_y, energy_t0);
    log_info("hypotenuse:%d, vxy:%d, run:%d \n", (int)__this->p_anim->hypotenuse, (int)vxy, run_time);

    if (run_time > 4000) {
        run_time = 4000;
    } else if (run_time < 1500) {
        run_time = 1500;
    }
    __this->p_anim->limit_base = 0;//4;
    end_dist = 0;
    start_dist = __this->p_anim->limit_base + run_time / 30;
    __this->p_anim->limit_base = __this->p_anim->limit_base * 2 / 3; // 后期匀速一段时间
    __this->p_anim->limit_v = start_dist * 2 / 3; // 前期匀速一段时间
    if (__this->p_anim->dist_x < 0) {
        __this->p_anim->negative_x = 1;
    }
    if (__this->p_anim->dist_y < 0) {
        __this->p_anim->negative_y = 1;
    }

    /*重新开始配置惯性*/
    ui_anim_init(&__this->p_anim->anim);
    ui_anim_set_var(&__this->p_anim->anim, FOOTBALL);
    ui_anim_set_path_cb(&__this->p_anim->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim->anim, football_anim_exec_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim->anim, start_dist, end_dist);		// 路径设置
    ui_anim_set_time(&__this->p_anim->anim, run_time);						// 运行时间设置
    ui_anim_start(&__this->p_anim->anim);
}
static void football_anim_enter_cb(int var, int32_t v)
{
    __this->x_diff = -v;
    ui_redraw(FOOTBALL);
}
static void football_anim_enter()
{
    football_anim_stop();

    __this->p_anim = zalloc(sizeof(struct football_anim));
    ASSERT(__this->p_anim);
    /*进入动效*/
    /*进入动画控制在1s内*/
    int run_time = 800;
    /*给定一个参数让球速缓慢衰减，可参考惯性改成目标距离的方式，让球停到指定的角度或者图标*/
    int start = 76;
    int end  = 0;
    /*重新开始配置惯性*/
    ui_anim_init(&__this->p_anim->anim);
    ui_anim_set_var(&__this->p_anim->anim, FOOTBALL);
    ui_anim_set_path_cb(&__this->p_anim->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim->anim,  football_anim_enter_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim->anim, start, end);		// 路径设置
    ui_anim_set_time(&__this->p_anim->anim, run_time);						// 运行时间设置
    ui_anim_start(&__this->p_anim->anim);
}

static void football_rdec_anim_start(int xoffset)
{
    football_anim_stop();

    __this->p_anim = zalloc(sizeof(struct football_anim));
    ASSERT(__this->p_anim);

    /*动画时间控制在1s内*/
    int run_time = 800;
    /*给定一个参数让球速缓慢衰减，可参考惯性改成目标距离的方式，让球停到指定的角度或者图标*/
    int start = xoffset;
    int end  = 0;
    /*重新开始配置惯性*/
    ui_anim_init(&__this->p_anim->anim);
    ui_anim_set_var(&__this->p_anim->anim, FOOTBALL);
    ui_anim_set_path_cb(&__this->p_anim->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim->anim,  football_anim_enter_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim->anim, start, end);		// 路径设置
    ui_anim_set_time(&__this->p_anim->anim, run_time);						// 运行时间设置
    ui_anim_start(&__this->p_anim->anim);
}

static int FOOTBALL_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctrl;
    struct draw_context *dc = (struct draw_context *)arg;

#if SUPPORT_ICON_BACKGROUND_OVERLAY
    struct ui_pic *pic_bg = NULL;
    pic_bg = ui_pic_for_id(FOOTBALL_BG);
    if (pic_bg) {
        struct draw_context dc_tmp_bg = {0};
        struct ui_image_list *image_list_bg = ui_pic_get_normal_image_list(pic_bg);
        if (image_list_bg) {
            struct rect rect_bg = {0};
            ui_core_get_draw_context(&dc_tmp_bg, &pic->elm, &rect_bg);
            int i;
            u16 *image_id_bg = (u16 *)image_list_bg->image;
            __this->football.image_bg_num = image_list_bg->num;
            if (__this->football.image_bg_num > 0) {
                if (!__this->football.image_bg) {
                    __this->football.image_bg = (struct ui_image_attrs *)malloc(__this->football.image_bg_num * sizeof(struct ui_image_attrs));
                    for (i = 0; i < image_list_bg->num; i++) {
                        platform_api->read_image_info(&dc_tmp_bg, image_id_bg[i], &__this->football.image_bg[i]);
                    }
                }

                if ((__this->football.image_bg_num == 1) || (__this->football.image_bg_num == 3)) {
                    int bg_rgb_num = (__this->football.image_bg_num == 3) ? __this->football.image_num * 3 : __this->football.image_num;
                    const struct football_bg_rgb_t *bg_rgb = (__this->football.image_bg_num == 3) ? football_bg_3in1_rgb : football_bg_rgb;
                    if (!__this->football.football_bg_rgb) {
                        __this->football.football_bg_rgb = (struct football_bg_rgb_t *)malloc(bg_rgb_num * sizeof(struct football_bg_rgb_t));
                        for (i = 0; i < bg_rgb_num; i++) {
                            __this->football.football_bg_rgb[i].red = bg_rgb[i].red;
                            __this->football.football_bg_rgb[i].green = bg_rgb[i].green;
                            __this->football.football_bg_rgb[i].blue = bg_rgb[i].blue;
                        }
                    }
                }
                set_football_bg_mask_en(1);
                football_set_mask_scale(1.05f);
            }
        }
    }
#endif

    switch (event) {
    case ON_CHANGE_INIT:
        __this = zalloc(sizeof(struct football_priv));
        ASSERT(__this);
        struct ui_image_list *image_list = ui_pic_get_normal_image_list(pic);
        ASSERT(image_list);
        struct ui_platform_api *platform_api = ui_get_platform_api();
        ASSERT(platform_api);

        struct draw_context dc_tmp = {0};
        struct rect rect = {0};
        ui_core_get_draw_context(&dc_tmp, &pic->elm, &rect);
        __this->football.image = (struct ui_image_attrs *)malloc(image_list->num * sizeof(struct ui_image_attrs));
        __this->football.image_num = image_list->num;

        int i;
        u16 *image_id = (u16 *)image_list->image;
        for (i = 0; i < image_list->num; i++) {
            platform_api->read_image_info(&dc_tmp, image_id[i], &__this->football.image[i]);
        }

        extern void jlgpu_get_win_rect(struct rect * rect);
        struct rect win_rect;
        jlgpu_get_win_rect(&win_rect);
        football_init(__this->football.image[0].width, __this->football.image[0].height, 1.1f, 4.0f, win_rect.left, win_rect.top, win_rect.width, win_rect.height);
        football_set_pos(last_pos);
        break;

    case ON_CHANGE_SHOW:
        ui_remove_backcolor(&pic->elm);//移除控件背景颜色
        ui_remove_backimage(&pic->elm);//移除控件背景图像
        ui_remove_border(&pic->elm);//移除控件边界
        ASSERT(dc->gpu_task_head);
        __this->football.head = dc->gpu_task_head;
        __this->football.element_id = pic->elm.id;
        __this->football.task_id = dc->elm_index;
        __this->football.prior = pic->elm.prior;
        football_draw(&__this->football, __this->x_diff, __this->y_diff);
        break;

    case ON_CHANGE_RELEASE:
        if (__this) {
            football_anim_stop();
            football_get_pos(last_pos);
            football_uinit();
            if (__this->football.image) {
                free(__this->football.image);
                __this->football.image = NULL;
            }
#if SUPPORT_ICON_BACKGROUND_OVERLAY
            if (__this->football.image_bg) {
                free(__this->football.image_bg);
                __this->football.image_bg = NULL;
            }
            if (__this->football.football_bg_rgb) {
                free(__this->football.football_bg_rgb);
                __this->football.football_bg_rgb = NULL;
            }
#endif
            free(__this);
            __this = NULL;
        }

        break;
    case ON_CHANGE_FIRST_SHOW:
        if (ui_menu_enter_anim_flag_get()) {
            ui_menu_enter_anim_disable();
            football_anim_enter();
        }

        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int FOOTBALL_onkey(void *ctrl, struct element_key_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctrl;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_UI_MINUS:
            football_rdec_anim_start(30);
            break;
        case KEY_UI_PLUS:
            football_rdec_anim_start(-30);
            break;
        default:
            return false;
        }
        break;
    default:
        break;
    }

    return true;
}


static int FOOTBALL_ontouch(void *ctrl, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctrl;
    struct draw_context *dc = pic->elm.dc;
    static struct position last_point = {0};

    switch (e->event) {
    case ELM_EVENT_TOUCH_ENERGY:
        if (!__this->touch_down_flag) {
            break;
        }
        __this->elm = &pic->elm;
        football_anim_start(e);
        break;

    case ELM_EVENT_TOUCH_DOWN:
        football_anim_stop();
        last_point.x = e->pos.x;
        last_point.y = e->pos.y;
        __this->hold_flag = 0;
        __this->touch_down_flag = 1;
        break;

    case ELM_EVENT_TOUCH_HOLD:
        __this->hold_flag = 1;
        break;

    case ELM_EVENT_TOUCH_MOVE:
        if (!__this->touch_down_flag) {
            break;
        }
        __this->x_diff = e->pos.x - last_point.x;
        __this->y_diff = e->pos.y - last_point.y;
        last_point.x = e->pos.x;
        last_point.y = e->pos.y;

        ui_core_redraw(&pic->elm);
        break;

    case ELM_EVENT_TOUCH_UP:
        if (__this->hold_flag) {
            break;
        }
        if (e->move_flag) {
            break;
        }
        if (e->has_energy) {
            break;
        }
        if (!__this->touch_down_flag) {
            break;
        }
        int index = football_get_face_index(e->pos.x, e->pos.y);
        if (index != -1) {
#if 0 // test
            struct unumber n;
            n.numbs = 1;
            n.type = TYPE_NUM;
            n.number[0] = index + 1;
            ui_core_set_disp_status_by_id(FACE_INDEX, 1);
            ui_number_update_by_id(FACE_INDEX, &n);
            os_time_dly(50);
            ui_hide(FACE_INDEX);
#else
            int window_id = ui_menu_map_by_sel(index, MENU_SEL_ID_APP_MENU);
            if (window_id > 0) {
                UI_HIDE_CURR_WINDOW();
                UI_SHOW_WINDOW(window_id);
            }
#endif
        }

        break;
    }
    return true;
}

#if defined(FOOTBALL)
REGISTER_UI_EVENT_HANDLER(FOOTBALL)
.onchange = FOOTBALL_onchange,
 .onkey = FOOTBALL_onkey,
  .ontouch = FOOTBALL_ontouch,
};
#endif

#endif /*if TCFG_UI_ENABLE_FOOTBALL*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/

