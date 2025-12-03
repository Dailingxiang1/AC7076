#include "cube_reflection.h"
#include "ui_api.h"
#include "asm/math_fast_function.h"
#include "jlui/ui_page_switch.h"
#include "jlui/ui_measure.h"
#include "jlui_app/ui_resource.h"
#include "gpu_task.h"
#include <math.h>
#include "jlui_app/ui_style.h"
#include  "jlui_app/ui_app_effect.h"
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_cube_cube_reflection.data.bss")
#pragma data_seg(".ui_action_cube_cube_reflection.data")
#pragma const_seg(".ui_action_cube_cube_reflection.text.const")
#pragma code_seg(".ui_action_cube_cube_reflection.text")
#endif

#define LOG_TAG_CONST       UI_TASK
#define LOG_TAG     		"[UI_TASK]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CHAR_ENABLE
#include "debug.h"

#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))

#define UI_EFFECT_REAL_TIME_RUN_CLOSE			0//关闭走时(省ram)
#define UI_EFFECT_FREE_CURR_PAGE_LIST			0//释放页面链表(省ram)

struct cube_reflection_anim0 {
    ui_anim_t anim;
    int last_val;
    u8 is_draw;     // 0 no draw; 1 draw
};

struct cube_reflection_anim1 {
    ui_anim_t anim;
    u8 status;			//1 run
};

struct cube_reflection_anim2 {
    ui_anim_t anim;
    u8 status;			//1 run
    int face_index;
};

struct cube_reflection_priv {
    struct cube_reflection_param cube_reflection;
    struct element *curr_elm;
    u8 hold_flag;
    u8 move_flag;
    u8 touch_flag;
    u8 admission_flag;
    struct cube_reflection_anim0 *p_anim0;   // 转场动画: 匀速转动, 用于修正面朝屏幕的面的角度
    struct cube_reflection_anim1 *p_anim1;   // 其他界面进入的转场动画前的入场动画
    struct cube_reflection_anim2 *p_anim2;   // 转场动画退出进入其他页面的离场动画
    int curr_win;
    s16 pos_x;
    s16 pos_y;
    u32 page_id[6];
    int init : 1;
    int ignore_first_move : 1;
    int cur_left;
    int dial_page_index;
    unsigned long last_time;
    s16 x_curr_angle;
    s16 y_curr_angle;
    u8 x_dir;
    //u8 y_dir;
    int width_tab[12];
    int height_tab[12];
};

static struct cube_reflection_priv cube_reflection_priv_t = {0};
#define __this (&cube_reflection_priv_t)

// 说明：以下参数是经过调整的，不要修改
#define VIEX_DISTANCE_MIN   4.375f
#define VIEX_DISTANCE_MAX   3.285f   // 值最小, 模型最大
#define VIEX_DISTANCE_DIFF  (VIEX_DISTANCE_MIN - VIEX_DISTANCE_MAX)

#define W_SCALE     0.885f
#define H_SCALE     0.825f
#define SCALE_ANIM_SCOPE        100.0f
#define ANGLE_ANIM_SCOPE_MAX    120 // 由于修正作用, 最大旋转角度为 120°

struct cube_reflection_face_preview {
    int page_id;
    int image_id;
};

// 以下页面id优先使用图片作为页面预览
static const struct cube_reflection_face_preview preview[] = {
    {ID_WINDOW_CALENDAR, PAGE66_b35e_CALENDAR},
    /* {ID_WINDOW_SLEEP, PAGE66_ebb4_SLEEP}, */
    {-1, PAGE66_eb12_PAGE_ADD},
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
void cube_reflection_effect_init(struct element *curr_elm);
void cube_reflection_effect_uninit(void);
int ui_cube_reflection_move(int curr_win, int xoffset, int yoffset, int mode);
int ui_cube_reflection_zoom_out(int curr_win, int v);
int ui_cube_reflection_zoom_in(int curr_win, int v);
int ui_page_num();
extern int window_init(int id);
extern struct ui_platform_api *ui_get_platform_api();

#define abs(x)    (((x) > 0) ? (x) : (-(x)))

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_anim0_exec_cb 匀速平移回调
 *
 * @param var
 * @param v
 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim0_exec_cb(int var, int32_t v)
{
    if (!__this) {
        return ;
    }
    if (!__this->p_anim0) {
        return ;
    }

    //printf("__this->x_curr_angle = %d.", __this->x_curr_angle);
    //printf("[%s] v = %d.", __func__, v);
    if (__this->p_anim0->is_draw) {
        ui_cube_reflection_move(__this->curr_win, -(v - __this->p_anim0->last_val), 0, ui_card_get_move_mode());
    } else {
        __this->x_curr_angle += (v - __this->p_anim0->last_val);
    }

    __this->p_anim0->last_val = v;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_anim0_stop 结束匀速平移动画 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim0_stop(void)
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
 * @brief cube_reflection_anim0_start 开始匀速平移动画
 *
 * @param e
 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim0_start(int angle_ofs, int run_time, u8 is_draw)
{
    int start_dist, end_dist;

    //printf("__this->x_curr_angle = %d, angle_ofs = %d.", __this->x_curr_angle, angle_ofs);
    //printf("__this->x_dir = %d.", __this->x_dir);

    // 释放旧的
    cube_reflection_anim0_stop();

    __this->p_anim0 = zalloc(sizeof(struct cube_reflection_anim0));
    ASSERT(__this->p_anim0);
    __this->p_anim0->is_draw = is_draw;

    start_dist = 0;

    int time_div = run_time / ANGLE_ANIM_SCOPE_MAX;

    if (angle_ofs > 0) {
        if (__this->x_dir == 0) {
            end_dist = ANGLE_ANIM_SCOPE_MAX - angle_ofs;
            run_time = time_div * end_dist;
        } else {
            end_dist = -angle_ofs;
            run_time = time_div * -end_dist;
        }
    } else {
        if (__this->x_dir == 0) {
            end_dist = -angle_ofs;
            run_time = time_div * end_dist;
        } else {
            end_dist = - (ANGLE_ANIM_SCOPE_MAX + angle_ofs);
            run_time = time_div * -end_dist;
        }
    }

    //printf("end_dist = %d.", end_dist);

    /*开始配置动画信息*/
    ui_anim_init(&__this->p_anim0->anim);
    ui_anim_set_var(&__this->p_anim0->anim, FOOTBALL);
    ui_anim_set_path_cb(&__this->p_anim0->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim0->anim, cube_reflection_anim0_exec_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim0->anim, start_dist, end_dist);		// 路径设置
    ui_anim_set_time(&__this->p_anim0->anim, run_time);						// 运行时间设置
    __this->p_anim0->last_val = 0;
    ui_anim_start(&__this->p_anim0->anim);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_anim1_exec_cb 进入动画回调
 *
 * @param var
 * @param v
 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim1_exec_cb(int var, int32_t v)
{
    if (!__this) {
        return ;
    }
    if (!__this->p_anim1) {
        return ;
    }

    //printf("[%s] v = %d.", __func__, v);
    ui_cube_reflection_zoom_out(__this->curr_win, v);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_anim1_ready_cb 入场动画结束回调
 *
 * @param _anim
 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim1_ready_cb(struct _ui_anim_t *_anim)
{
    __this->p_anim1->status = 0;

    __this->admission_flag = 1;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_anim1_stop 结束入场动画
 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim1_stop(void)
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
 * @brief cube_reflection_anim1_start 开始入场动画
 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim1_start(void)
{
    cube_reflection_anim1_stop();

    __this->p_anim1 = zalloc(sizeof(struct cube_reflection_anim1));
    ASSERT(__this->p_anim1);

    /*进入动效*/
    /*进入动画控制在1s内*/
    int run_time = 300;
    int start = 0;
    int end  = SCALE_ANIM_SCOPE;

    ui_anim_init(&__this->p_anim1->anim);
    ui_anim_set_var(&__this->p_anim1->anim, FOOTBALL);
    ui_anim_set_path_cb(&__this->p_anim1->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim1->anim, cube_reflection_anim1_exec_cb);		// 运行回调
    ui_anim_set_ready_cb(&__this->p_anim1->anim, cube_reflection_anim1_ready_cb);	//结束回调
    ui_anim_set_values(&__this->p_anim1->anim, start, end);		            // 路径设置
    ui_anim_set_time(&__this->p_anim1->anim, run_time);						// 运行时间设置
    __this->p_anim1->status = 1;
    __this->x_curr_angle = 0;
    __this->y_curr_angle = 0;

    ui_anim_start(&__this->p_anim1->anim);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_anim1_is_playing 入场动画是否播放
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cube_reflection_anim1_is_playing()
{
    if (__this && __this->p_anim1 && __this->p_anim1->status) {
        return true;
    } else {
        return false;
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_anim2_exec_cb 离场动画回调
 *
 * @param var
 * @param v
 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim2_exec_cb(int var, int32_t v)
{
    if (!__this) {
        return ;
    }

    if (!__this->p_anim2) {
        return ;
    }

    //printf("[%s] v = %d.", __func__, v);
    ui_cube_reflection_zoom_in(__this->curr_win, v);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_anim2_ready_cb 离场动画结束回调
 *
 * @param _anim
 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim2_ready_cb(struct _ui_anim_t *_anim)
{
    int select_page = __this->page_id[__this->p_anim2->face_index];

    //printf("*********************** face_index = %d; select_page = 0x%x.", __this->p_anim2->face_index, select_page);
    //printf("__this->x_curr_angle = %d; __this->y_curr_angle = %d.", __this->x_curr_angle, __this->y_curr_angle);

    if (select_page == -1) {
        select_page = ID_WINDOW_COMPONENT;
        printf(" map to add componenet");
    }

    cube_reflection_effect_uninit();

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
 * @brief cube_reflection_anim2_stop 结束离场动画
 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim2_stop(void)
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
 * @brief cube_reflection_anim2_start
 *
 * @Params face_index   离场切换的页面
 */
/* ------------------------------------------------------------------------------------*/
static void cube_reflection_anim2_start(int face_index)
{
    cube_reflection_anim2_stop();

    __this->p_anim2 = zalloc(sizeof(struct cube_reflection_anim2));
    ASSERT(__this->p_anim2);
    __this->p_anim2->face_index = face_index;
    int face_x_angle;
    int face_y_angle;
    //printf("__this->x_curr_angle = %d; __this->y_curr_angle = %d.", __this->x_curr_angle, __this->y_curr_angle);
    // 720 (360 * 2)    x 2 是因为模型计算角度时乘了 0.5
    cube_reflection_get_face_angle(__this->p_anim2->face_index, &face_x_angle, &face_y_angle);
    if (__this->x_curr_angle >= 0) {
        __this->x_curr_angle = __this->x_curr_angle % 720;
    } else {
        __this->x_curr_angle = __this->x_curr_angle % -720 + 720;

    }

    int face_x_ofs = __this->x_curr_angle - face_x_angle;
    if (face_x_ofs >= 360) {   // 角度变换不会大于 360  (180 * 2)
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
    __this->x_dir = face_x_ofs > 0 ? 1 : 0;
    cube_reflection_anim0_start(face_x_ofs, ANGLE_ANIM_SCOPE_MAX, 0);

    /*进入动效*/
    /*进入动画控制在1s内*/
    int run_time = 300;
    int start = SCALE_ANIM_SCOPE;
    int end  = 0;
    /*重新开始配置惯性*/
    ui_anim_init(&__this->p_anim2->anim);
    ui_anim_set_var(&__this->p_anim2->anim, FOOTBALL);
    ui_anim_set_path_cb(&__this->p_anim2->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim2->anim, cube_reflection_anim2_exec_cb);		// 运行回调
    ui_anim_set_ready_cb(&__this->p_anim2->anim, cube_reflection_anim2_ready_cb);	//结束回调
    ui_anim_set_values(&__this->p_anim2->anim, start, end);		            // 路径设置
    ui_anim_set_time(&__this->p_anim2->anim, run_time);						// 运行时间设置
    __this->p_anim2->status = 1;
    //__this->x_curr_angle = 0; // 不需要恢复为 0
    //__this->y_curr_angle = 0;

    ui_anim_start(&__this->p_anim2->anim);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_anim2_is_playing 离场动画是否播放
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cube_reflection_anim2_is_playing()
{
    if (__this && __this->p_anim2 && __this->p_anim2->status) {
        return true;
    } else {
        return false;
    }
}


pJLGPUTaskHead_t jlgpu_create_task_list_by_image(pJLGPUTaskHead_t head, struct ui_image_attrs *image_attr, int xoffset, int yoffset);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_update_task_list_global_alpha 重置 global_alpha
 *
 * @param reset_sep
 */
/* ------------------------------------------------------------------------------------*/
static void jlgpu_update_task_list_global_alpha(void)
{
    u8 order_tab[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    u8 global_alpha_tab[1 + 6 + 6] = {32, 32, 32, 32, 32, 32, 32, 128, 128, 128, 128, 128, 128};    // 阴影部分透明处理

    jlgpu_task_list_copy_group_global_alpha_reset(__this->cube_reflection.head, order_tab, global_alpha_tab, 7);    // 1 + 6

    return;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_effect_init 3d灯笼 + 倒影初始化
 *
 * @param curr_elm
 */
/* ------------------------------------------------------------------------------------*/
void cube_reflection_effect_init(struct element *curr_elm)
{
    if (__this->init) {
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

    extern void jlgpu_get_win_rect(struct rect * rect);
    struct rect win_rect;
    jlgpu_get_win_rect(&win_rect);

    //默认值
    for (int i = 0; i < 12; i++) {
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
        __this->cube_reflection.head = jlgpu_task_list_copy_create(NULL, new_head, NULL, 1);
        jlgpu_delete_task_list_head(new_head);
    } else {
        __this->cube_reflection.head = jlgpu_task_list_copy_create(NULL, head, NULL, 1);
    }

    jlgpu_task_list_copy_create(__this->cube_reflection.head, head, NULL, 1 + 6);
#if UI_EFFECT_FREE_CURR_PAGE_LIST
    jlgpu_free_all_task(head);
    head->gpu_task_base_adr = NULL;
#endif
    int next_win;
    int page_index = 0;
    __this->page_id[page_index++] = curr_elm->id;
    next_win = curr_elm->id;
    int page_num = get_ui_page_list_total_num() - 1;
    while (page_num--) {
        next_win = ui_page_next(next_win);
        __this->page_id[page_index++] = next_win;
        if (page_index == 6) {
            break;
        }
    }
    for (; page_index < 6; page_index++) {
        __this->page_id[page_index] = -1;
    }

    int win;
    struct element *win_elm;
    for (int i = 1; i < 6; i++) {
        win = __this->page_id[i];
        image_id = get_face_preview(win);
        if (image_id != -1) {
            struct ui_image_attrs image_attr;
            dc_tmp.prj = 0;
            dc_tmp.page = (image_id >> 16) & 0xff;
            platform_api->read_image_info(&dc_tmp, image_id & 0xffff, &image_attr);
            pJLGPUTaskHead_t new_head = jlgpu_create_task_list_by_image(head, &image_attr, win_rect.left, win_rect.top);
            jlgpu_task_list_copy_create(__this->cube_reflection.head, new_head, NULL, i + 1);
            jlgpu_task_list_copy_create(__this->cube_reflection.head, new_head, NULL, i + 1 + 6);
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
                        jlgpu_task_list_copy_create(__this->cube_reflection.head, win_elm->dc->gpu_task_head, NULL, i + 1);
                        jlgpu_task_list_copy_create(__this->cube_reflection.head, win_elm->dc->gpu_task_head, NULL, i + 1 + 6);
                        ui_hide(win);
                    }
                } else {
                    jlgpu_task_list_copy_create(__this->cube_reflection.head, win_elm->dc->gpu_task_head, NULL, i + 1);
                    jlgpu_task_list_copy_create(__this->cube_reflection.head, win_elm->dc->gpu_task_head, NULL, i + 1 + 6);
                }
            }
        }
    }

    __this->dial_page_index = -1;
    for (int i = 1; i < 6; i++) {
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
                        ui_core_show(win_elm, true);
                        watch_unload_sidebar(win_elm);	// 释放掉侧边栏
                        jlgpu_task_list_copy_create(__this->cube_reflection.head, win_elm->dc->gpu_task_head, NULL, i + 1);
                        jlgpu_task_list_copy_create(__this->cube_reflection.head, win_elm->dc->gpu_task_head, NULL, i + 1 + 6);
#if (UI_EFFECT_REAL_TIME_RUN_CLOSE)
                        ui_hide(win);
                        __this->dial_page_index = -1;
#endif
                    }
                }
            } else {
                jlgpu_task_list_copy_create(__this->cube_reflection.head, win_elm->dc->gpu_task_head, NULL, i + 1);
                jlgpu_task_list_copy_create(__this->cube_reflection.head, win_elm->dc->gpu_task_head, NULL, i + 1 + 6);
            }
        }
    }

    cube_reflection_init(__this->width_tab, __this->height_tab, ((float)win_rect.height / (float)win_rect.width) * W_SCALE, H_SCALE, VIEX_DISTANCE_MIN, VIEX_DISTANCE_MAX, VIEX_DISTANCE_MIN, win_rect.left, win_rect.top, win_rect.width, win_rect.height);

    curr_elm->dc->gpu_task_dont_sort = 1;

    int ret = jlgpu_mult_task_head_modify_by_index(curr_elm->dc->gpu_mult_list, curr_elm->dc->index, __this->cube_reflection.head);
    ASSERT(!ret);

    __this->curr_elm = curr_elm;

    __this->init = 1;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cube_reflection_effect_update 特效更新，用于刷新时间
 *
 * @param timeout
 */
/* ------------------------------------------------------------------------------------*/
void cube_reflection_effect_update(int timeout)
{
    int i;
    int dial_page_index = -1;

    if (!__this->init) {
        return;
    }

    for (i = 0; i < 6; i++) {
        if (__this->page_id[i] == DIAL_PAGE_0) {
            dial_page_index = i;
            break;
        }
    }
    if (dial_page_index != -1) {
        struct element *win_elm = ui_core_get_element_nowarning_by_id(DIAL_PAGE_0);
        if (win_elm) {
            int ret = jlgpu_task_list_copy_update_matrix_by_group(__this->cube_reflection.head, win_elm->dc->gpu_task_head, dial_page_index + 1);
            if (ret) {
                jlgpu_task_list_copy_destroy_by_group(__this->cube_reflection.head, dial_page_index + 1);
                ASSERT(win_elm->dc);
                jlgpu_task_list_copy_create(__this->cube_reflection.head, win_elm->dc->gpu_task_head, NULL, dial_page_index + 1);
            }

            ret = jlgpu_task_list_copy_update_matrix_by_group(__this->cube_reflection.head, win_elm->dc->gpu_task_head, dial_page_index + 1 + 6);
            if (ret) {
                jlgpu_task_list_copy_destroy_by_group(__this->cube_reflection.head, dial_page_index + 1 + 6);
                ASSERT(win_elm->dc);
                jlgpu_task_list_copy_create(__this->cube_reflection.head, win_elm->dc->gpu_task_head, NULL, dial_page_index + 1 + 6);
            }
        }

        //printf("__this->x_curr_angle = %d; __this->y_curr_angle = %d.", __this->x_curr_angle, __this->y_curr_angle);
        // 更新模型的显示, 播放动画时不更新
        if ((!cube_reflection_anim1_is_playing()) && (!cube_reflection_anim2_is_playing())) {
            cube_reflection_draw(&__this->cube_reflection, __this->x_curr_angle, __this->y_curr_angle);
        }

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
        if (((__this->touch_flag || __this->p_anim0 || __this->p_anim1 || __this->p_anim2) && \
             (diff_time >= (timeout * 9 / 10))) || \
            (!__this->touch_flag && !__this->p_anim0 && !__this->p_anim1 && !__this->p_anim2)) {
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
 * @brief cube_reflection_effect_uninit 反初始化
 */
/* ------------------------------------------------------------------------------------*/
void cube_reflection_effect_uninit(void)
{
    if (__this->init) {
        cube_reflection_anim0_stop();
        cube_reflection_anim1_stop();
        cube_reflection_anim2_stop();

        if (__this->dial_page_index != -1) {
            ui_hide(__this->page_id[__this->dial_page_index]);
            __this->dial_page_index = -1;
        }

        jlgpu_task_list_copy_destroy(__this->cube_reflection.head);
        cube_reflection_uninit();
        struct element *curr_elm = __this->curr_elm;
        int ret = jlgpu_mult_task_head_modify_by_index(curr_elm->dc->gpu_mult_list, curr_elm->dc->index, curr_elm->dc->gpu_task_head);
        ASSERT(!ret);
        curr_elm->dc->gpu_task_dont_sort = 0;
#if UI_EFFECT_FREE_CURR_PAGE_LIST
        ui_core_redraw(curr_elm);
#endif
        jlgpu_scheduler_set_redraw_mode(curr_elm->dc, GPU_ASYN_REDRAW);

        ui_page_set_busy(0);
        __this->init = 0;
        __this->admission_flag = 0;
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
 * @brief ui_cube_reflection_status 特效执行状态
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_cube_reflection_status()
{
    if (cube_reflection_anim1_is_playing() || cube_reflection_anim2_is_playing()) { //入场和离场动画不允许打断

        return 2;
    }
    return __this->init;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_cube_reflection_move 滑动接口
 *
 * @param curr_win
 * @param xoffset
 * @param yoffset
 * @param mode
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_cube_reflection_move(int curr_win, int xoffset, int yoffset, int mode)
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
        cube_reflection_effect_init(curr_elm);
        jlgpu_update_task_list_global_alpha();
    }

    ui_page_set_busy(1);

    if (__this->init) {
        __this->x_curr_angle -= xoffset;
        //__this->y_curr_angle -= yoffset;
        __this->x_dir = xoffset > 0 ? 1 : 0;
        //__this->y_dir;
        cube_reflection_draw(&__this->cube_reflection, __this->x_curr_angle, __this->y_curr_angle);
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
 * @brief ui_cube_reflection_zoom_out 入场动画: 缩小
 *
 * @param curr_win
 * @param v
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_cube_reflection_zoom_out(int curr_win, int v)
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

    if ((__this->init == 0) && ((__this->hold_flag == 1) || (__this->admission_flag == 0))) {
        cube_reflection_effect_init(curr_elm);
        jlgpu_update_task_list_global_alpha();
    }

    cube_reflection_uninit();
    cube_reflection_init(__this->width_tab, __this->height_tab, ((float)win_rect.height / (float)win_rect.width) * W_SCALE, H_SCALE, (VIEX_DISTANCE_MAX + ((VIEX_DISTANCE_DIFF * v) / 100.0f)), VIEX_DISTANCE_MAX, VIEX_DISTANCE_MIN, win_rect.left, win_rect.top, win_rect.width, win_rect.height);

    ui_page_set_busy(1);

    if (__this->init) {
        cube_reflection_draw(&__this->cube_reflection, __this->x_curr_angle, __this->y_curr_angle);
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
 * @brief ui_cube_reflection_zoom_in 离场动画: 放大
 *
 * @param curr_win
 * @param v
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_cube_reflection_zoom_in(int curr_win, int v)
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

    cube_reflection_uninit();
    cube_reflection_init(__this->width_tab, __this->height_tab, ((float)win_rect.height / (float)win_rect.width) * W_SCALE, H_SCALE, (VIEX_DISTANCE_MAX + ((VIEX_DISTANCE_DIFF * v) / 100.0f)), VIEX_DISTANCE_MAX, VIEX_DISTANCE_MIN, win_rect.left, win_rect.top, win_rect.width, win_rect.height);

    ui_page_set_busy(1);

    cube_reflection_draw(&__this->cube_reflection, __this->x_curr_angle, __this->y_curr_angle);

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
 * @brief ui_cube_reflection_ontouch 触摸
 *
 * @param e
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int ui_cube_reflection_ontouch(struct element_touch_event *e)
{
    int xoffset, yoffset;
    /* printf("%s e->event = 0x%02x. move_flag = %d; hold_flag = %d.",__func__, e->event, __this->move_flag, __this->hold_flag); */
    if (cube_reflection_anim1_is_playing() || cube_reflection_anim2_is_playing()) { //入场和离场动画不允许打断

        return true;
    }

    switch (e->event) {
    case ELM_EVENT_TOUCH_ENERGY:
        if (!__this->init) {
            break;
        }
        if (__this->admission_flag == 0) {
            __this->curr_win = ui_get_current_window_id();
            cube_reflection_anim1_start();

            return true;
        }
        break;
    case ELM_EVENT_TOUCH_DOWN:
        cube_reflection_anim0_stop();
        //入场动画不允许打断
        /* cube_reflection_anim1_stop(); */
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
            cube_reflection_anim1_start();
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
#if 0
        if (__this->admission_flag) {   // 确保已经进入特效画面
            __this->pos_x = e->pos.x;
            __this->pos_y = e->pos.y;
            float xoffset_adj = (float)xoffset * 0.55f;  // 跟手动作角度减速, 不然角度大时速度和动画差异太大
            xoffset = round(xoffset_adj);
            ui_cube_reflection_move(__this->curr_win, xoffset, yoffset, ui_card_get_move_mode());
            __this->move_flag = 1;
        }
#else
        if (/*(ui_get_current_window_id() != ID_WINDOW_DIAL) &&*/ (ui_page_num() == 1) && (!__this->init)) {
            __this->curr_win = ui_get_current_window_id();  // 非主页切换到转场特效: 使用缩小动画
            cube_reflection_anim1_start();
        } else {
            if (__this->admission_flag && __this->ignore_first_move) {   // 确保已经进入特效画面
                float xoffset_adj = (float)xoffset * 0.55f;  // 跟手动作角度减速, 不然角度大时速度和动画差异太大
                xoffset = round(xoffset_adj);
                ui_cube_reflection_move(__this->curr_win, xoffset, yoffset, ui_card_get_move_mode());
                __this->move_flag = 1;
            }
            __this->ignore_first_move = 1;
            __this->pos_x = e->pos.x;
            __this->pos_y = e->pos.y;
        }
#endif

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
            //printf("__this->x_curr_angle = %d; __this->y_curr_angle = %d.", __this->x_curr_angle, __this->y_curr_angle);
            //printf("__this->pos_x = %d; __this->pos_y = %d.", __this->pos_x, __this->pos_y);
            // 移动事件之后松开触碰, 通过匀速的平移动画修正最终的模型角度
            int angle_ofs = __this->x_curr_angle % ANGLE_ANIM_SCOPE_MAX;
            if (angle_ofs != 0) {
                //printf("angle_ofs = %d.", angle_ofs);
                cube_reflection_anim0_start(angle_ofs, ANGLE_ANIM_SCOPE_MAX * 5, 1);
            }
            __this->move_flag = 0;

            break;
        }
        if (e->has_energy) {
            break;
        }
        if (!__this->init) {
            break;
        }

        int face_index = cube_reflection_get_face_index(e->pos.x, e->pos.y);
        //printf("touch face %d\n", face_index);
        if (face_index != -1) {
            cube_reflection_anim2_start(face_index);    // 转场特效离场
        }

        return true;
        break;
    }
    return 0;
}
REGISTER_UI_EFFECT_MODULE(PAGE_MOVE_MODE_CUBE_REFLECTION)
.ontouch = ui_cube_reflection_ontouch,
 .get_status = ui_cube_reflection_status,
  .uninit = cube_reflection_effect_uninit,
};
#endif

