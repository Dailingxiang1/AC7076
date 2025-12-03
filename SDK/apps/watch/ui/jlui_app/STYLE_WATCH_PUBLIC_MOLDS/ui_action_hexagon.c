#include "hexagon.h"
#include "ui_api.h"
#include "asm/math_fast_function.h"
#include "jlui/ui_page_switch.h"
#include "jlui/ui_measure.h"
#include "jlui_app/ui_resource.h"
#include "gpu_task.h"
#include "jlui_app/ui_style.h"
#include  "jlui_app/ui_app_effect.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_hexagon.data.bss")
#pragma data_seg(".ui_action_hexagon.data")
#pragma const_seg(".ui_action_hexagon.text.const")
#pragma code_seg(".ui_action_hexagon.text")
#endif

#define LOG_TAG_CONST       UI_TASK
#define LOG_TAG     		"[UI_TASK]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CHAR_ENABLE
#include "debug.h"

#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))

#define SCREEN_MASK								0//添加屏幕mask，预留
#define UI_EFFECT_REAL_TIME_RUN_CLOSE			0//关闭走时(省ram)
#define UI_EFFECT_FREE_CURR_PAGE_LIST			0//释放页面链表(省ram)


#if (POLYTOPE_CONFIG == POLYTOPE_SIX)
const u8 polytope = POLYTOPE_SIX;
#elif (POLYTOPE_CONFIG == POLYTOPE_EIGHT)
const u8 polytope = POLYTOPE_EIGHT;
#elif (POLYTOPE_CONFIG == POLYTOPE_ALL)
const u8 polytope = POLYTOPE_ALL;
#endif
struct hexagon_anim0 {
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

struct hexagon_anim1 {
    ui_anim_t anim;
    u8 status;			//1 run
};

struct hexagon_anim2 {
    ui_anim_t anim;
    u8 status;			//1 run
    int face_index;
};

struct hexagon_anim3 {
    ui_anim_t anim;
    int last_val;
};

struct hexagon_priv {
    struct hexagon_param hexagon;
    struct element *curr_elm;
    u8 hold_flag;
    u8 move_flag;
    u8 touch_flag;
    u8 polytope_type; // 多面体的数量配置，多个多面体共存时使用
    struct hexagon_anim0 *p_anim0;   // 转场动画 : 惯性动画
    struct hexagon_anim1 *p_anim1;   // 其他界面进入的转场动画前的入场动画
    struct hexagon_anim2 *p_anim2;   // 转场动画退出进入其他页面的离场动画
    struct hexagon_anim3 *p_anim3;   // 配合离场动画执行的角度修正
    int curr_win;
    s16 pos_x;
    s16 pos_y;
#if (POLYTOPE_CONFIG == POLYTOPE_SIX)
    u32 page_id[POLYTOPE_SIX];
    u8 reset_sep[POLYTOPE_SIX];
    u8 reset_sep_last[POLYTOPE_SIX];
#else //八面体或六面体和八面体共存
    u32 page_id[POLYTOPE_EIGHT];
    u8 reset_sep[POLYTOPE_EIGHT];
    u8 reset_sep_last[POLYTOPE_EIGHT];
#endif
    int init : 1;
    int ignore_first_move : 1;
    int cur_left;
    int dial_page_index;
    unsigned long last_time;
    s16 x_curr_angle;
    s16 y_curr_angle;
    //u8 x_dir;
    //u8 y_dir;
    int width_tab[8];
    int height_tab[8];
};

static struct hexagon_priv hexagon_priv_t = {0};
#define __this (&hexagon_priv_t)

// 说明：以下参数是经过调整的，不要修改
#define X_ANGLE_ADJ 0.0f
#define X_ANGLE 14.85f

#define VIEX_DISTANCE_MIN   4.7f
#define VIEX_DISTANCE_MAX   2.575f
#define VIEX_DISTANCE_DIFF  (VIEX_DISTANCE_MIN - VIEX_DISTANCE_MAX)
#define W_SCALE     1.285f
#define H_SCALE     1.195f
#define SCALE_ANIM_SCOPE    100.0f
#define ANGLE_ANIM_SCOPE_MAX 180

struct hexagon_face_preview {
    int page_id;
    int image_id;
};

// 以下页面id优先使用图片作为页面预览
static const struct hexagon_face_preview preview[] = {
    /* {ID_WINDOW_CALENDAR, PAGE66_b35e_CALENDAR}, */
    /* {ID_WINDOW_SLEEP, PAGE66_ebb4_SLEEP}, */
    {-1, PAGE66_eb12_PAGE_ADD},
#if SCREEN_MASK
    {-2, PAGE66_1259_MASK_SCREEN},
#endif
};

static int get_face_preview(int page_id)
{
    int i;
    for (i = 0; i < sizeof(preview) / sizeof(preview[0]); i++) {
        if (preview[i].page_id == page_id) {
            return preview[i].image_id;
        }
    }
    return -1;
}

int watch_unload_sidebar(struct element *elm);
int watch_load_sidebar(struct element *elm);
void hexagon_effect_init(struct element *curr_elm, bool list_flag);
void hexagon_effect_uninit(void);
int ui_hexagon_move(int curr_win, int xoffset, int yoffset, int mode);
int ui_hexagon_zoom_out(int curr_win, int v);
int ui_hexagon_zoom_in(int curr_win, int v);
int ui_page_num();
extern int window_init(int id);
extern void jlui_malloc_ram_info_dump();
extern struct ui_platform_api *ui_get_platform_api();

#define abs(x)    (((x) > 0) ? (x) : (-(x)))

static void hexagon_anim3_start(int angle_ofs, int run_time);



/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim0_exec_cb 惯性动画回调
 *
 * @param var
 * @param v
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim0_exec_cb(int var, int32_t v)
{
    if (!__this) {
        return ;
    }
    if (!__this->p_anim0) {
        return ;
    }
    /* log_info("\n\n v:%d \n", v); */
    if ((__this->p_anim0->limit_v) && (v > __this->p_anim0->limit_v)) {
        v = __this->p_anim0->limit_v;
    }
    if (v > __this->p_anim0->limit_base) {
        v -= __this->p_anim0->limit_base;
    } else {
        v = 1;
    }
    __this->p_anim0->hypotenuse += v;
    float dist_x = __this->p_anim0->sin_x * __this->p_anim0->hypotenuse;
    float dist_y = complex_dqdt_float(__this->p_anim0->hypotenuse, dist_x);
    float int_x = dist_x;
    float int_y = dist_y;
    if (__this->p_anim0->negative_x) {
        int_x = -int_x;
    }
    if (__this->p_anim0->negative_y) {
        int_y = -int_y;
    }

    int x_diff = int_x - __this->p_anim0->dist_x;
    int y_diff = int_y - __this->p_anim0->dist_y;
    __this->p_anim0->dist_x = int_x;
    __this->p_anim0->dist_y = int_y;

    /* log_info("dist:%d, %d, %d \n", (int)__this->p_anim0->hypotenuse, int_x, int_y); */
    /* printf("diff:%f, %f, v:%d \n", x_diff, y_diff, v); */

    ui_hexagon_move(__this->curr_win, x_diff, y_diff, ui_card_get_move_mode());
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim0_stop 结束惯性动结束惯性动画 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim0_stop(void)
{
    if (!__this) {
        return ;
    }
    if (__this->p_anim0) {
        ui_anim_del(FOOTBALL, NULL);
        free(__this->p_anim0);
        __this->p_anim0 = NULL;
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim0_start 开始惯性动画
 *
 * @param e
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim0_start(struct element_touch_event *e)
{
    int start_dist, end_dist;
    int run_time;

    // 释放旧的
    hexagon_anim0_stop();

    __this->p_anim0 = zalloc(sizeof(struct hexagon_anim0));
    ASSERT(__this->p_anim0);

    // 动画参数计算
    __this->p_anim0->dist_x = e->pos.x >> 16;
    __this->p_anim0->dist_y = e->pos.y >> 16;
    float dist_x = abs(__this->p_anim0->dist_x);
    float dist_y = abs(__this->p_anim0->dist_y);
    __this->p_anim0->hypotenuse = complex_abs_float(dist_x, dist_y); // 边长计算
    __this->p_anim0->sin_x = dist_x / __this->p_anim0->hypotenuse; //sinA
    int energy_t0 = (e->pos.x + 1) & 0xffff; //防止div0
    float vxy = __this->p_anim0->hypotenuse / energy_t0;

    run_time = vxy * 800;

    /* log_info("x:%d, y:%d, en:%d \n", __this->p_anim0->dist_x, __this->p_anim0->dist_y, energy_t0); */
    /* log_info("hypotenuse:%d, vxy:%d, run:%d \n", (int)__this->p_anim0->hypotenuse, (int)vxy, run_time); */

    if (run_time > 4000) {
        run_time = 4000;
    } else if (run_time < 1500) {
        run_time = 1500;
    }
    __this->p_anim0->limit_base = 4;
    end_dist = 0;
    start_dist = __this->p_anim0->limit_base + run_time / 30;
    __this->p_anim0->limit_base = __this->p_anim0->limit_base * 2 / 3; // 后期匀速一段时间
    __this->p_anim0->limit_v = start_dist * 2 / 3; // 前期匀速一段时间
    if (__this->p_anim0->dist_x < 0) {
        __this->p_anim0->negative_x = 1;
    }
    if (__this->p_anim0->dist_y < 0) {
        __this->p_anim0->negative_y = 1;
    }

    /*重新开始配置惯性*/
    ui_anim_init(&__this->p_anim0->anim);
    ui_anim_set_var(&__this->p_anim0->anim, FOOTBALL);
    ui_anim_set_path_cb(&__this->p_anim0->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim0->anim, hexagon_anim0_exec_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim0->anim, start_dist, end_dist);		// 路径设置
    ui_anim_set_time(&__this->p_anim0->anim, run_time);						// 运行时间设置
    ui_anim_start(&__this->p_anim0->anim);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim1_exec_cb 进入动画回调
 *
 * @param var
 * @param v
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim1_exec_cb(int var, int32_t v)
{
    if (!__this) {
        return ;
    }
    if (!__this->p_anim1) {
        return ;
    }

    //printf("[%s] v = %d.", __func__, v);
    ui_hexagon_zoom_out(__this->curr_win, v);
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim1_ready_cb 入场动画接入回调
 *
 * @param _anim
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim1_ready_cb(struct _ui_anim_t *_anim)
{
    __this->p_anim1->status = 0;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim1_stop 结束入场动画
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim1_stop(void)
{
    if (!__this) {
        return ;
    }
    if (__this->p_anim1) {
        ui_anim_del(FOOTBALL, NULL);
        free(__this->p_anim1);
        __this->p_anim1 = NULL;
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim1_start 开始入场动画
 *
 * @param curr_win
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim1_start(void)
{
    hexagon_anim1_stop();

    __this->p_anim1 = zalloc(sizeof(struct hexagon_anim1));
    ASSERT(__this->p_anim1);

    /*进入动效*/
    /*进入动画控制在1s内*/
    int run_time = 300;
    int start = 0;
    int end  = SCALE_ANIM_SCOPE;

    ui_anim_init(&__this->p_anim1->anim);
    ui_anim_set_var(&__this->p_anim1->anim, FOOTBALL);
    ui_anim_set_path_cb(&__this->p_anim1->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim1->anim, hexagon_anim1_exec_cb);		// 运行回调
    ui_anim_set_ready_cb(&__this->p_anim1->anim, hexagon_anim1_ready_cb);	//结束回调
    ui_anim_set_values(&__this->p_anim1->anim, start, end);		            // 路径设置
    ui_anim_set_time(&__this->p_anim1->anim, run_time);						// 运行时间设置
    __this->p_anim1->status = 1;
    __this->x_curr_angle = 0;
    __this->y_curr_angle = 0;

    ui_anim_start(&__this->p_anim1->anim);
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim1_is_playing 入场动画是否播放
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int hexagon_anim1_is_playing()
{
    if (__this && __this->p_anim1 && __this->p_anim1->status) {
        return true;
    } else {
        return false;
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim2_exec_cb 离场动画回调
 *
 * @param var
 * @param v
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim2_exec_cb(int var, int32_t v)
{
    if (!__this) {
        return ;
    }

    if (!__this->p_anim2) {
        return ;
    }

    //printf("[%s] v = %d.", __func__, v);
    ui_hexagon_zoom_in(__this->curr_win, v);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim2_ready_cb 离场动画结束回调
 *
 * @param _anim
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim2_ready_cb(struct _ui_anim_t *_anim)
{
    int select_page = __this->page_id[__this->p_anim2->face_index];

    //printf("*********************** face_index = %d; select_page = 0x%x.", __this->p_anim2->face_index, select_page);
    //printf("__this->x_curr_angle = %d; __this->y_curr_angle = %d.", __this->x_curr_angle, __this->y_curr_angle);

    if (select_page == -1) {
        select_page = ID_WINDOW_COMPONENT;
        printf(" map to add componenet");
    }

    hexagon_effect_uninit();

    int curr_page = __this->curr_win;
    ui_hide(curr_page);
    ui_show(select_page);
#if 0
    struct element *curr_elm = ui_core_get_element_nowarning_by_id(curr_page);

    if (select_page != curr_page) {
        ui_hide(curr_page);
        ui_show(select_page);
    } else {
        if (curr_elm->id == ID_WINDOW_DIAL) {
            struct element *watch_elm = ui_core_get_element_by_id(DIAL_WATCH);
            if (watch_elm) {
                watch_load_sidebar(watch_elm);
            }
        }
        ui_core_redraw(curr_elm);
    }
#endif
    // __this->p_anim2 可能已经被释放掉了
    if (__this && __this->p_anim2) {
        __this->p_anim2->status = 0;
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim2_stop 结束离场动画
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim2_stop(void)
{
    if (!__this) {
        return ;
    }
    if (__this->p_anim2) {
        ui_anim_del(FOOTBALL, NULL);
        free(__this->p_anim2);
        __this->p_anim2 = NULL;
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim2_start
 *
 * @Params face_index   离场切换的页面
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim2_start(int face_index)
{
    hexagon_anim2_stop();

    __this->p_anim2 = zalloc(sizeof(struct hexagon_anim2));
    ASSERT(__this->p_anim2);
    __this->p_anim2->face_index = face_index;
    int face_x_angle;
    int face_y_angle;
    //printf("__this->x_curr_angle = %d; __this->y_curr_angle = %d.", __this->x_curr_angle, __this->y_curr_angle);
    hexagon_get_face_angle(__this->p_anim2->face_index, &face_x_angle, &face_y_angle);
    face_x_angle = face_index * 120;
    if (__this->x_curr_angle >= 0) {
        __this->x_curr_angle = __this->x_curr_angle % 720;
    } else {
        __this->x_curr_angle = __this->x_curr_angle % -720 + 720;

    }

    int face_x_ofs = __this->x_curr_angle - face_x_angle;
    if (face_x_ofs >= 360) {   // 角度变换不会大于 360
        face_x_ofs = face_x_ofs - 720;
    } else if (face_x_ofs <= -360) {
        face_x_ofs = face_x_ofs + 720;
    }

    // 忽略 face_y_angle
    //int face_y_ofs = 0;

    //printf("__this->x_curr_angle = %d; __this->y_curr_angle = %d.", __this->x_curr_angle, __this->y_curr_angle);
    //printf("face_x_angle = %d; face_y_angle = %d.", face_x_angle, face_y_angle);
    //printf("face_x_ofs = %d; face_y_ofs = %d.", face_x_ofs, face_y_ofs);

    // 先启动旋转, 注意放大动画的时间要比旋转动画的时间长
    //__this->x_dir = face_x_ofs > 0 ? 1 : 0;
    hexagon_anim3_start(face_x_ofs, ANGLE_ANIM_SCOPE_MAX);

    /*进入动效*/
    /*进入动画控制在1s内*/
    int run_time = 300;
    int start = SCALE_ANIM_SCOPE;
    int end  = 0;
    /*重新开始配置惯性*/
    ui_anim_init(&__this->p_anim2->anim);
    ui_anim_set_var(&__this->p_anim2->anim, FOOTBALL);
    ui_anim_set_path_cb(&__this->p_anim2->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim2->anim, hexagon_anim2_exec_cb);		// 运行回调
    ui_anim_set_ready_cb(&__this->p_anim2->anim, hexagon_anim2_ready_cb);	//结束回调
    ui_anim_set_values(&__this->p_anim2->anim, start, end);		            // 路径设置
    ui_anim_set_time(&__this->p_anim2->anim, run_time);						// 运行时间设置
    __this->p_anim2->status = 1;
    //__this->x_curr_angle = 0; // 不需要恢复为 0
    //__this->y_curr_angle = 0;

    ui_anim_start(&__this->p_anim2->anim);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim2_is_playing 离场动画是否播放
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int hexagon_anim2_is_playing()
{
    if (__this && __this->p_anim2 && __this->p_anim2->status) {
        return true;
    } else {
        return false;
    }
}


/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim3_exec_cb 匀速平移回调
 *
 * @param var
 * @param v
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim3_exec_cb(int var, int32_t v)
{
    if (!__this) {
        return ;
    }
    if (!__this->p_anim3) {
        return ;
    }

    //printf("__this->x_curr_angle = %d.", __this->x_curr_angle);
    //printf("[%s] v = %d.", __func__, v);
    //ui_hexagon_move(__this->curr_win, (v - __this->p_anim3->last_val), 0, ui_card_get_move_mode());
    __this->x_curr_angle -= (v - __this->p_anim3->last_val);
    // __this->y_curr_angle 忽略
    __this->p_anim3->last_val = v;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim3_stop 结束匀速平移动画 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim3_stop(void)
{
    if (!__this) {
        return ;
    }
    if (__this->p_anim3) {
        ui_anim_del(FOOTBALL, NULL);
        free(__this->p_anim3);
        __this->p_anim3 = NULL;
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_anim3_start 开始匀速平移动画
 *
 * @param e
 */
/* ------------------------------------------------------------------------------------*/
static void hexagon_anim3_start(int angle_ofs, int run_time)
{
    int start_dist, end_dist;

    //printf("__this->x_curr_angle = %d, angle_ofs = %d.", __this->x_curr_angle, angle_ofs);
    //printf("__this->x_dir = %d.", __this->x_dir);

    // 释放旧的
    hexagon_anim3_stop();

    __this->p_anim3 = zalloc(sizeof(struct hexagon_anim3));
    ASSERT(__this->p_anim3);

    start_dist = 0;
    end_dist = angle_ofs;

    int time_div = run_time / ANGLE_ANIM_SCOPE_MAX;

    if (angle_ofs > 0) {
        run_time = time_div * end_dist;
    } else {
        run_time = time_div * -end_dist;
    }

    //printf("time_div = %d; run_time = %d; end_dist = %d.", time_div, run_time, end_dist);

    /*开始配置动画信息*/
    ui_anim_init(&__this->p_anim3->anim);
    ui_anim_set_var(&__this->p_anim3->anim, FOOTBALL);
    ui_anim_set_path_cb(&__this->p_anim3->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim3->anim, hexagon_anim3_exec_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim3->anim, start_dist, end_dist);		// 路径设置
    ui_anim_set_time(&__this->p_anim3->anim, run_time);						// 运行时间设置
    __this->p_anim3->last_val = 0;
    ui_anim_start(&__this->p_anim3->anim);
}


pJLGPUTaskHead_t jlgpu_create_task_list_by_image(pJLGPUTaskHead_t head, struct ui_image_attrs *image_attr, int xoffset, int yoffset);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_update_task_list_order 重排
 *
 * @param reset_sep
 */
/* ------------------------------------------------------------------------------------*/
static void jlgpu_update_task_list_order(u8 *reset_sep)
{
    //printf("last sep ===");
    //for (int i = 0; i < 6; i++) {
    //    printf("reset_sep[%d] = %d.", i, reset_sep[i]);
    //}

    for (int i = 0; i < __this->polytope_type; i++) {
        if (__this->reset_sep_last[i] != reset_sep[i]) {
            goto update_list;   // 贴图顺序发生了改变，需要更新链表
        }
    }

    return; // 跑到这里意味着链表不需要更新

update_list:
#if (POLYTOPE_CONFIG == POLYTOPE_SIX)
    u8 order_tab[] = {0, reset_sep[0], reset_sep[1], reset_sep[2], reset_sep[3], reset_sep[4], reset_sep[5]};
    u16 order_tab_size = sizeof(order_tab);
#elif (POLYTOPE_CONFIG == POLYTOPE_EIGHT)
    u8 order_tab[] = {0, reset_sep[0], reset_sep[1], reset_sep[2], reset_sep[3], reset_sep[4], reset_sep[5], reset_sep[6], reset_sep[7]};
    u16 order_tab_size = sizeof(order_tab);
#elif (POLYTOPE_CONFIG == POLYTOPE_ALL)
    u8 order_tab0[] = {0, reset_sep[0], reset_sep[1], reset_sep[2], reset_sep[3], reset_sep[4], reset_sep[5]};
    u8 order_tab1[] = {0, reset_sep[0], reset_sep[1], reset_sep[2], reset_sep[3], reset_sep[4], reset_sep[5], reset_sep[6], reset_sep[7]};
    u8 *order_tab = NULL;
    u16 order_tab_size = 0;
    if (__this->polytope_type == POLYTOPE_SIX) {
        order_tab = order_tab0;
        order_tab_size = sizeof(order_tab0);
    } else if (__this->polytope_type == POLYTOPE_EIGHT) {
        order_tab = order_tab1;
        order_tab_size = sizeof(order_tab1);
    } else {
        printf("__this->polytope_type config err\n");
        return;
    }
#endif
    jlgpu_task_list_copy_group_order_adjust(__this->hexagon.head, order_tab, order_tab_size / sizeof(order_tab[0]));

    // 暂存上一次的输出
    for (int i = 0; i < __this->polytope_type; i++) {
        __this->reset_sep_last[i] = reset_sep[i];
    }

    return;
}

/* 动态设置多面体的数量，在配置成多面体共存是启用 */
static u8 custom_polytope_type = POLYTOPE_SIX;
void hexagon_custom_polytope_type_set(u8 type)
{
    custom_polytope_type = type;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_effect_init 3d灯笼初始化
 *
 * @param curr_elm
 * @param list_flag
 */
/* ------------------------------------------------------------------------------------*/
void hexagon_effect_init(struct element *curr_elm, bool list_flag)
{
    if (__this->init) {
        return;
    }
    if (polytope == POLYTOPE_SIX) {
        __this->polytope_type = POLYTOPE_SIX;
    } else if (polytope == POLYTOPE_EIGHT) {
        __this->polytope_type = POLYTOPE_EIGHT;
    } else if (polytope == POLYTOPE_ALL) {
        __this->polytope_type = custom_polytope_type; //用户自己定义，默认6面体
        hexagon_polytope_type_set(__this->polytope_type);
    } else {
        printf("polytope config err\n");
        return;
    }

    struct ui_platform_api *platform_api = ui_get_platform_api();
    ASSERT(platform_api);
    struct draw_context dc_tmp = {0};
    struct rect rect = {0};

    ui_core_get_draw_context(&dc_tmp, curr_elm, &rect);

    jlgpu_scheduler_set_redraw_mode(curr_elm->dc, GPU_SYNC_REDRAW);


    if (curr_elm->id == ID_WINDOW_DIAL) {
        watch_unload_sidebar(curr_elm);
    }

    curr_elm->dc->refresh = false;
    ui_core_redraw(curr_elm);
    curr_elm->dc->refresh = true;

    pJLGPUTaskHead_t head = curr_elm->dc->gpu_task_head;
#if SCREEN_MASK
    curr_elm->dc->draw_state = GPU_DRAW_EACH_GROUP;
#endif
    extern void jlgpu_get_win_rect(struct rect * rect);
    struct rect win_rect;
    jlgpu_get_win_rect(&win_rect);

    //默认值
    for (int i = 0; i < __this->polytope_type; i++) {
        __this->width_tab[i] = win_rect.width;
        __this->height_tab[i] = win_rect.height;
    }

    int image_id = get_face_preview(curr_elm->id);
    if (image_id != -1) {
        struct ui_image_attrs image_attr;
        dc_tmp.prj = 0;
        dc_tmp.page = (image_id >> 16) & 0xff;
        platform_api->read_image_info(&dc_tmp, image_id & 0xffff, &image_attr);
        pJLGPUTaskHead_t new_head = jlgpu_create_task_list_by_image(head, &image_attr, win_rect.left, win_rect.top);
        __this->hexagon.head = jlgpu_task_list_copy_create(NULL, new_head, NULL, 1);
        __this->width_tab[0] = image_attr.width;
        __this->height_tab[0] = image_attr.height;
        jlgpu_delete_task_list_head(new_head);
    } else {

        __this->hexagon.head = jlgpu_task_list_copy_create(NULL, head, NULL, 1);
    }
#if UI_EFFECT_FREE_CURR_PAGE_LIST
    jlgpu_free_all_task(head);
    head->gpu_task_base_adr = NULL;
#endif
#if SCREEN_MASK
    {
        image_id = get_face_preview(-2);
        if (image_id != -1) {
            struct ui_image_attrs image_attr;
            dc_tmp.prj = 0;
            dc_tmp.page = (image_id >> 16) & 0xff;
            platform_api->read_image_info(&dc_tmp, image_id & 0xffff, &image_attr);
            pJLGPUTaskHead_t new_head = jlgpu_create_task_list_by_image_set_blend_mode(head, &image_attr, win_rect.left, win_rect.top, GPU_BLEND_DST_OUT);
            jlgpu_task_list_copy_create(__this->hexagon.head, new_head, NULL, 1);
            jlgpu_delete_task_list_head(new_head);
        }
    }
#endif
    int next_win;
    int page_index = 0;
    __this->page_id[page_index++] = curr_elm->id;
    next_win = curr_elm->id;
    int page_num = get_ui_page_list_total_num() - 1;
    while (page_num--) {
        next_win = ui_page_next(next_win);
        __this->page_id[page_index++] = next_win;
        if (page_index == __this->polytope_type) {
            break;
        }
    }
    for (; page_index < __this->polytope_type; page_index++) {
        __this->page_id[page_index] = -1;
    }

    int win;
    struct element *win_elm;
    for (int i = 1; i < __this->polytope_type; i++) {
        win = __this->page_id[i];
        image_id = get_face_preview(win);
        if (image_id != -1) {
            struct ui_image_attrs image_attr;
            dc_tmp.prj = 0;
            dc_tmp.page = (image_id >> 16) & 0xff;
            platform_api->read_image_info(&dc_tmp, image_id & 0xffff, &image_attr);
            pJLGPUTaskHead_t new_head = jlgpu_create_task_list_by_image(head, &image_attr, win_rect.left, win_rect.top);
            jlgpu_task_list_copy_create(__this->hexagon.head, new_head, NULL, i + 1);
            __this->width_tab[i] = image_attr.width;
            __this->height_tab[i] = image_attr.height;
            jlgpu_delete_task_list_head(new_head);
        } else {
            if (win != DIAL_PAGE_0) {
                win_elm = ui_core_get_element_nowarning_by_id(win);
                if (!win_elm) {
                    int ret = window_init(win);
                    ASSERT(!ret);
                    if (!ret) {
                        win_elm = ui_core_get_element_nowarning_by_id(win);
                        win_elm->dc->refresh = false;
                        ui_core_show(win_elm, true);
                        jlgpu_task_list_copy_create(__this->hexagon.head, win_elm->dc->gpu_task_head, NULL, i + 1);
                        ui_hide(win);
                    }
                } else {
                    jlgpu_task_list_copy_create(__this->hexagon.head, win_elm->dc->gpu_task_head, NULL, i + 1);
                }
            }
        }
#if SCREEN_MASK
        {
            image_id = get_face_preview(-2);
            if (image_id != -1) {
                struct ui_image_attrs image_attr;
                dc_tmp.prj = 0;
                dc_tmp.page = (image_id >> 16) & 0xff;
                platform_api->read_image_info(&dc_tmp, image_id & 0xffff, &image_attr);
                pJLGPUTaskHead_t new_head = jlgpu_create_task_list_by_image_set_blend_mode(head, &image_attr, win_rect.left, win_rect.top, GPU_BLEND_DST_OUT);
                jlgpu_task_list_copy_create(__this->hexagon.head, new_head, NULL, i + 1);
                jlgpu_delete_task_list_head(new_head);
            }
        }
#endif
    }

    __this->dial_page_index = -1;
    for (int i = 1; i < __this->polytope_type; i++) {
        win = __this->page_id[i];
        if (win == DIAL_PAGE_0) {
            __this->dial_page_index = i;
            win_elm = ui_core_get_element_nowarning_by_id(win);
            if (!win_elm) {
                if (!window_init(win)) {
                    win_elm = ui_core_get_element_nowarning_by_id(win);
                    if (win_elm) {
                        win_elm->dc->refresh = false;
                        ASSERT(win_elm->dc);
                        watch_unload_sidebar(win_elm);
                        ui_core_show(win_elm, true);
                        jlgpu_task_list_copy_create(__this->hexagon.head, win_elm->dc->gpu_task_head, NULL, i + 1);
#if (UI_EFFECT_REAL_TIME_RUN_CLOSE)
                        ui_hide(win);
                        __this->dial_page_index = -1;
#endif
                    }
                }
            } else {
                jlgpu_task_list_copy_create(__this->hexagon.head, win_elm->dc->gpu_task_head, NULL, i + 1);
            }
        }
    }

    hexagon_init(__this->width_tab, __this->height_tab, ((float)win_rect.height / (float)win_rect.width) * W_SCALE, H_SCALE, VIEX_DISTANCE_MIN, win_rect.left, win_rect.top, win_rect.width, win_rect.height, (X_ANGLE - X_ANGLE_ADJ));

    curr_elm->dc->gpu_task_dont_sort = 1;

    int ret = jlgpu_mult_task_head_modify_by_index(curr_elm->dc->gpu_mult_list, curr_elm->dc->index, __this->hexagon.head);
    ASSERT(!ret);

    __this->curr_elm = curr_elm;
    if (list_flag == false) {
        for (int i = 0; i < __this->polytope_type; i++) {
            __this->reset_sep[i] = __this->reset_sep_last[i] = i + 1;
        }
    }

    __this->init = 1;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_effect_update 特效更新，用于刷新时间
 *
 * @param timeout
 */
/* ------------------------------------------------------------------------------------*/
void hexagon_effect_update(int timeout)
{
    int i;
    int dial_page_index = -1;

    if (!__this->init) {
        return;
    }

    for (i = 0; i < __this->polytope_type; i++) {
        if (__this->page_id[i] == DIAL_PAGE_0) {
            dial_page_index = i;
            break;
        }
    }
    if (dial_page_index != -1) {
        struct element *win_elm = ui_core_get_element_nowarning_by_id(DIAL_PAGE_0);
        if (win_elm) {
            int ret = jlgpu_task_list_copy_update_matrix_by_group(__this->hexagon.head, win_elm->dc->gpu_task_head, dial_page_index + 1);
            if (ret) {
                jlgpu_task_list_copy_destroy_by_group(__this->hexagon.head, dial_page_index + 1);
                ASSERT(win_elm->dc);
                jlgpu_task_list_copy_create(__this->hexagon.head, win_elm->dc->gpu_task_head, NULL, dial_page_index + 1);
            }
        }

        // 更新模型的显示, 播放动画时不更新

        hexagon_draw(&__this->hexagon, __this->x_curr_angle, __this->y_curr_angle, X_ANGLE, (X_ANGLE - X_ANGLE_ADJ), __this->reset_sep, 1);
        struct element *curr_elm = __this->curr_elm;
        struct rect lcdrect;
        lcdrect.left = 0;
        lcdrect.top = 0;
        ASSERT(curr_elm->dc);
        lcdrect.width = curr_elm->dc->width;
        lcdrect.height = curr_elm->dc->height;
        extern struct ui_platform_api *ui_get_platform_api();
        struct ui_platform_api *platform_api = ui_get_platform_api();

        unsigned long curr_time = jiffies_msec();
        int diff_time = jiffies_offset_to_msec(__this->last_time, curr_time) / 10;
        if (((__this->touch_flag || __this->p_anim0 || __this->p_anim1 || __this->p_anim2 || __this->p_anim3) && \
             (diff_time >= (timeout * 9 / 10))) || \
            (!__this->touch_flag && !__this->p_anim0 && !__this->p_anim1 && !__this->p_anim2 && !__this->p_anim3)) {
            __this->last_time = curr_time;
            if (platform_api->put_draw_context) {
                struct rect rect_orig;
                memcpy(&rect_orig, &curr_elm->dc->rect_orig, sizeof(struct rect));
                memcpy(&curr_elm->dc->rect_orig, &lcdrect, sizeof(struct rect));
                if (ui_page_num() > 1) {
                    platform_api->put_draw_context(curr_elm->dc);
                } else {
                    platform_api->put_draw_context(curr_elm->dc);
                }
                memcpy(&curr_elm->dc->rect_orig, &rect_orig, sizeof(struct rect));
            }
        }
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief hexagon_effect_uninit 反初始化
 */
/* ------------------------------------------------------------------------------------*/
void hexagon_effect_uninit(void)
{
    if (__this->init) {
        hexagon_anim0_stop();
        hexagon_anim1_stop();
        hexagon_anim2_stop();
        hexagon_anim3_stop();

        if (__this->dial_page_index != -1) {
            ui_hide(__this->page_id[__this->dial_page_index]);
            __this->dial_page_index = -1;
        }

        struct element *curr_elm = __this->curr_elm;
        jlgpu_task_list_copy_destroy(__this->hexagon.head);
        hexagon_uninit();
        int ret = jlgpu_mult_task_head_modify_by_index(curr_elm->dc->gpu_mult_list, curr_elm->dc->index, curr_elm->dc->gpu_task_head);
        ASSERT(!ret);
        curr_elm->dc->gpu_task_dont_sort = 0;
#if UI_EFFECT_FREE_CURR_PAGE_LIST
        ui_core_redraw(curr_elm);
#endif
        jlgpu_scheduler_set_redraw_mode(curr_elm->dc, GPU_ASYN_REDRAW);

        ui_page_set_busy(0);
        __this->init = 0;
        __this->cur_left = 0;
        __this->hold_flag = 0;
        /* if (curr_elm->id == ID_WINDOW_DIAL) { */
        /* struct element *watch_elm = ui_core_get_element_by_id(DIAL_WATCH); */
        /* if (watch_elm) { */
        /* watch_load_sidebar(watch_elm); */
        /* } */
        /* } */

    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_hexagon_status 特效执行状态
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_hexagon_status()
{
    if (hexagon_anim1_is_playing() || hexagon_anim2_is_playing()) {
        return 2;//入场动画/退场动画
    } else {
        return __this->init;
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_hexagon_move 滑动接口
 *
 * @param curr_win
 * @param xoffset
 * @param yoffset
 * @param mode
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_hexagon_move(int curr_win, int xoffset, int yoffset, int mode)
{
    struct element *curr_elm = NULL;
    struct rect rect;
    struct rect lcdrect;
    int page_width;
    int page_height;

    ASSERT(ui_page_num() <= 2);
    curr_elm = ui_core_get_element_nowarning_by_id(curr_win);
    if (!curr_elm) {
        printf("curr_win : 0x%x, curr_elm : 0x%x\n", curr_win, (u32)curr_elm);
        extern void list_all_page();
        list_all_page();
        return -1;
    }
    page_width = curr_elm->dc->width;
    page_height = curr_elm->dc->height;

    rect.left = 0;
    rect.top = 0;
    rect.width = page_width;
    rect.height = page_height;

    lcdrect.left = 0;
    lcdrect.top = 0;
    lcdrect.width = page_width;
    lcdrect.height = page_height;

    __this->cur_left += xoffset;
    __this->cur_left = (__this->cur_left >= page_width) ? (__this->cur_left = page_width) : __this->cur_left;
    __this->cur_left = (__this->cur_left <= -page_width) ? (__this->cur_left = -page_width) : __this->cur_left;

    if ((__this->cur_left != 0) && (__this->init == 0)) {
        hexagon_effect_init(curr_elm, false);
    }

    ui_page_set_busy(1);

    if (__this->init) {
        __this->x_curr_angle -= xoffset;
        //__this->y_curr_angle -= yoffset;
        //__this->x_dir = xoffset > 0 ? 1 : 0;
        //__this->y_dir;
        hexagon_draw(&__this->hexagon, __this->x_curr_angle, __this->y_curr_angle, X_ANGLE_ADJ, 0.0f, __this->reset_sep, 0); // 这个 draw 不会真正绘制，只是为了更新 list
        jlgpu_update_task_list_order(__this->reset_sep); // 这里根据页面绘制顺序更新链表
        hexagon_draw(&__this->hexagon, __this->x_curr_angle, __this->y_curr_angle, X_ANGLE, (X_ANGLE - X_ANGLE_ADJ), __this->reset_sep, 1);
    }

    rect.left = 0;
    memcpy(&curr_elm->dc->page_r, &rect, sizeof(struct rect));
    get_rect_cover(&rect, &lcdrect, &curr_elm->dc->lcd_r);
    memcpy(&curr_elm->dc->gpu_r, &curr_elm->dc->lcd_r, sizeof(struct rect));
    curr_elm->dc->gpu_r.left = curr_elm->dc->lcd_r.left - curr_elm->dc->page_r.left;
    extern struct ui_platform_api *ui_get_platform_api();
    struct ui_platform_api *platform_api = ui_get_platform_api();
    __this->last_time = jiffies_msec();
    if (platform_api->put_draw_context) {
        struct rect rect_orig;
        memcpy(&rect_orig, &curr_elm->dc->rect_orig, sizeof(struct rect));
        memcpy(&curr_elm->dc->rect_orig, &lcdrect, sizeof(struct rect));
        platform_api->put_draw_context(curr_elm->dc);
        memcpy(&curr_elm->dc->rect_orig, &rect_orig, sizeof(struct rect));
    }

    return 0;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_hexagon_zoom_out 入场动画: 缩小
 *
 * @param curr_win
 * @param v
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_hexagon_zoom_out(int curr_win, int v)
{
    struct element *curr_elm = NULL;
    struct rect rect;
    struct rect lcdrect;
    int page_width;
    int page_height;

    ASSERT(ui_page_num() <= 2);
    curr_elm = ui_core_get_element_nowarning_by_id(curr_win);
    if (!curr_elm) {
        printf("curr_win : 0x%x, curr_elm : 0x%x\n", curr_win, (u32)curr_elm);
        extern void list_all_page();
        list_all_page();
        return -1;
    }

    struct rect win_rect;
    jlgpu_get_win_rect(&win_rect);

    page_width = curr_elm->dc->width;
    page_height = curr_elm->dc->height;

    rect.left = 0;
    rect.top = 0;
    rect.width = page_width;
    rect.height = page_height;

    lcdrect.left = 0;
    lcdrect.top = 0;
    lcdrect.width = page_width;
    lcdrect.height = page_height;


    if (__this->init == 0) {
        hexagon_effect_init(curr_elm, false);
    }

    hexagon_uninit();
    hexagon_init(__this->width_tab, __this->height_tab, ((float)win_rect.height / (float)win_rect.width) * W_SCALE, H_SCALE, (VIEX_DISTANCE_MAX + ((VIEX_DISTANCE_DIFF * v) / SCALE_ANIM_SCOPE)), win_rect.left, win_rect.top, win_rect.width, win_rect.height, (X_ANGLE - X_ANGLE_ADJ));

    ui_page_set_busy(1);

    if (__this->init) {
        hexagon_draw(&__this->hexagon, __this->x_curr_angle, __this->y_curr_angle, X_ANGLE_ADJ, 0.0f, __this->reset_sep, 0); // 这个 draw 不会真正绘制，只是为了更新 list
        jlgpu_update_task_list_order(__this->reset_sep); // 这里根据页面绘制顺序更新链表
        float x_angle_val = ((X_ANGLE - X_ANGLE_ADJ) * v) / SCALE_ANIM_SCOPE;
        hexagon_draw(&__this->hexagon, __this->x_curr_angle, __this->y_curr_angle, (x_angle_val + X_ANGLE_ADJ), x_angle_val, __this->reset_sep, 1);
    }

    rect.left = 0;
    memcpy(&curr_elm->dc->page_r, &rect, sizeof(struct rect));
    get_rect_cover(&rect, &lcdrect, &curr_elm->dc->lcd_r);
    memcpy(&curr_elm->dc->gpu_r, &curr_elm->dc->lcd_r, sizeof(struct rect));
    curr_elm->dc->gpu_r.left = curr_elm->dc->lcd_r.left - curr_elm->dc->page_r.left;
    extern struct ui_platform_api *ui_get_platform_api();
    struct ui_platform_api *platform_api = ui_get_platform_api();
    __this->last_time = jiffies_msec();
    if (platform_api->put_draw_context) {
        struct rect rect_orig;
        memcpy(&rect_orig, &curr_elm->dc->rect_orig, sizeof(struct rect));
        memcpy(&curr_elm->dc->rect_orig, &lcdrect, sizeof(struct rect));
        platform_api->put_draw_context(curr_elm->dc);
        memcpy(&curr_elm->dc->rect_orig, &rect_orig, sizeof(struct rect));
    }

    return 0;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_hexagon_zoom_in 离场动画: 放大
 *
 * @param curr_win
 * @param v
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_hexagon_zoom_in(int curr_win, int v)
{
    struct element *curr_elm = NULL;
    struct rect rect;
    struct rect lcdrect;
    int page_width;
    int page_height;

    ASSERT(ui_page_num() <= 2);
    curr_elm = ui_core_get_element_nowarning_by_id(curr_win);
    if (!curr_elm) {
        printf("curr_win : 0x%x, curr_elm : 0x%x\n", curr_win, (u32)curr_elm);
        extern void list_all_page();
        list_all_page();
        return -1;
    }

    struct rect win_rect;
    jlgpu_get_win_rect(&win_rect);

    page_width = curr_elm->dc->width;
    page_height = curr_elm->dc->height;

    rect.left = 0;
    rect.top = 0;
    rect.width = page_width;
    rect.height = page_height;

    lcdrect.left = 0;
    lcdrect.top = 0;
    lcdrect.width = page_width;
    lcdrect.height = page_height;

    if (__this->init == 0) {
        printf("%s error!! __this->init == 0.\n", __func__);
        return -1;
    }

    hexagon_uninit();
    hexagon_init(__this->width_tab, __this->height_tab, ((float)win_rect.height / (float)win_rect.width) * W_SCALE, H_SCALE, (VIEX_DISTANCE_MAX + ((VIEX_DISTANCE_DIFF * v) / SCALE_ANIM_SCOPE)), win_rect.left, win_rect.top, win_rect.width, win_rect.height, (X_ANGLE - X_ANGLE_ADJ));

    ui_page_set_busy(1);

    hexagon_draw(&__this->hexagon, __this->x_curr_angle, __this->y_curr_angle, X_ANGLE_ADJ, 0.0f, __this->reset_sep, 0); // 这个 draw 不会真正绘制，只是为了更新 list
    jlgpu_update_task_list_order(__this->reset_sep); // 这里根据页面绘制顺序更新链表

    float x_angle_val = ((X_ANGLE - X_ANGLE_ADJ) * v) / SCALE_ANIM_SCOPE;
    hexagon_draw(&__this->hexagon, __this->x_curr_angle, __this->y_curr_angle, (x_angle_val + X_ANGLE_ADJ), x_angle_val, __this->reset_sep, 1);

    rect.left = 0;
    memcpy(&curr_elm->dc->page_r, &rect, sizeof(struct rect));
    get_rect_cover(&rect, &lcdrect, &curr_elm->dc->lcd_r);
    memcpy(&curr_elm->dc->gpu_r, &curr_elm->dc->lcd_r, sizeof(struct rect));
    curr_elm->dc->gpu_r.left = curr_elm->dc->lcd_r.left - curr_elm->dc->page_r.left;
    extern struct ui_platform_api *ui_get_platform_api();
    struct ui_platform_api *platform_api = ui_get_platform_api();
    __this->last_time = jiffies_msec();
    if (platform_api->put_draw_context) {
        struct rect rect_orig;
        memcpy(&rect_orig, &curr_elm->dc->rect_orig, sizeof(struct rect));
        memcpy(&curr_elm->dc->rect_orig, &lcdrect, sizeof(struct rect));
        platform_api->put_draw_context(curr_elm->dc);
        memcpy(&curr_elm->dc->rect_orig, &rect_orig, sizeof(struct rect));
    }

    return 0;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_hexagon_ontouch 触摸
 *
 * @param e
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_hexagon_ontouch(struct element_touch_event *e)
{
    int xoffset, yoffset;
    /* printf("e->event = 0x%02x. move_flag = %d; hold_flag = %d.", e->event, __this->move_flag, __this->hold_flag); */
    if (hexagon_anim1_is_playing() || hexagon_anim2_is_playing()) { //入场和离场动画不允许打断
        return 0;
    }
    switch (e->event) {
    case ELM_EVENT_TOUCH_ENERGY:
        if (__this && __this->init) {
            if ((ui_get_current_window_id() == ID_WINDOW_DIAL) && (!__this->init)) {
                __this->curr_win = ui_get_current_window_id();      // 主界面切换到转场特效: 使用惯性动画
                __this->x_curr_angle = 0;
                __this->y_curr_angle = 0;
                hexagon_anim0_start(e);
            } else if (__this->init) {
                hexagon_anim0_start(e); // 转场特效触发惯性动画
            }
            return true;
        }
        break;
    case ELM_EVENT_TOUCH_DOWN:
        hexagon_anim0_stop();
        //入场动画不允许打断
        /* hexagon_anim1_stop(); */
        __this->pos_x = e->pos.x;
        __this->pos_y = e->pos.y;
        __this->hold_flag = 0;
        __this->move_flag = 0;
        __this->touch_flag = 1;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        __this->hold_flag = 1;
#if 0
        if (ui_get_current_window_id() != ID_WINDOW_DIAL && (ui_page_num() == 1)) { //长按进入特效， 主界面不生效
            __this->curr_win = ui_get_current_window_id();
            hexagon_anim1_start();
        }
#endif
        break;
    case ELM_EVENT_TOUCH_MOVE:
#if 0
        if (__this->hold_flag) {
            break;
        }
#endif

        xoffset = e->pos.x - __this->pos_x;
        yoffset = e->pos.y - __this->pos_y;
        if (!xoffset && !yoffset) {
            break;
        }
        __this->pos_x = e->pos.x;
        __this->pos_y = e->pos.y;
#if 0
        ui_hexagon_move(__this->curr_win, xoffset, yoffset, ui_card_get_move_mode());
#else
        if (/*(ui_get_current_window_id() != ID_WINDOW_DIAL) &&*/ (ui_page_num() == 1) && (!__this->init)) {
            __this->curr_win = ui_get_current_window_id();  // 非主页切换到转场特效: 使用缩小动画
            hexagon_anim1_start();
        } else {
            if (__this && __this->init && __this->ignore_first_move) {
                ui_hexagon_move(__this->curr_win, xoffset, yoffset, ui_card_get_move_mode());
            }
            __this->ignore_first_move = 1;
        }

#endif
        __this->move_flag = 1;
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        __this->ignore_first_move = 0;
        __this->touch_flag = 0;
        if (__this->hold_flag) {
            __this->hold_flag = 0;
            break;
        }
        if (__this->move_flag) {
            __this->move_flag = 0;
            break;
        }
        if (e->has_energy) {
            break;
        }
        if (!__this->init) {
            break;
        }
        int face_index = hexagon_get_face_index(e->pos.x, e->pos.y);
        //printf("touch face %d\n", face_index);
        if (face_index != -1) {
            hexagon_anim2_start(face_index);    // 转场特效离场
        }
        return true;
        break;
    }
    return 0;
}
REGISTER_UI_EFFECT_MODULE(PAGE_MOVE_MODE_HEXAGON)
.ontouch =  ui_hexagon_ontouch,
 .get_status = ui_hexagon_status,
  .uninit = hexagon_effect_uninit,
};
#endif
