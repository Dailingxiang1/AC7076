#include "cube.h"
#include "ui_api.h"
#include "asm/math_fast_function.h"
#include "jlui/ui_page_switch.h"
#include "jlui/ui_measure.h"
#include "jlui_app/ui_resource.h"
#include "gpu_task.h"
#include "jlui_app/ui_style.h"
#include  "jlui_app/ui_app_effect.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_cube.data.bss")
#pragma data_seg(".ui_action_cube.data")
#pragma const_seg(".ui_action_cube.text.const")
#pragma code_seg(".ui_action_cube.text")
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

struct cube_anim {
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

struct cube_priv {
    struct cube_param cube;
    struct element *curr_elm;
    pJLGPUTaskHead_t head;
    u8 hold_flag;
    u8 move_flag;
    u8 touch_flag;
    struct cube_anim *p_anim;
    int curr_win;
    s16 pos_x;
    s16 pos_y;
    u32 page_id[6];
    int init;
    int cur_left;
    int dial_page_index;
    unsigned long last_time;
    float x_diff;
    float y_diff;
    int width_tab[6];
    int height_tab[6];
    u32 psram_tab[6];
};
struct cube_face_preview {
    int page_id;
    int image_id;
};
#define UI_CUBE_MAP_ENABLE		1	//页面重排序
#if UI_CUBE_MAP_ENABLE//当前页面在最前，左向右滑动显示前一个页面+当前页面。右向左滑显示当前页面+后一个页面
/*src：
  0上 1下 2后 3前 4右 5左
  map：
  3前 4右 2后 5左 0上 1下*/
const u8 group_map[6] = {3, 4, 2, 5, 0, 1};
const u8 group_remap[6] = {4, 5, 2, 0, 1, 3};
#else
const u8 group_map[6] = {0, 1, 2, 3, 4, 5};
const u8 group_remap[6] = {0, 1, 2, 3, 4, 5};
#endif

int watch_unload_sidebar(struct element *elm);
int watch_load_sidebar(struct element *elm);
int ui_cube_move(int curr_win, int xoffset, int yoffset, int mode);
int ui_page_num();
extern int window_init(int id);
extern struct ui_platform_api *ui_get_platform_api();
extern pJLGPUTaskHead_t jlgpu_create_task_list_by_image(pJLGPUTaskHead_t head, struct ui_image_attrs *image_attr, int xoffset, int yoffset);


#define abs(x)    (((x) > 0) ? (x) : (-(x)))


static struct cube_priv cube_priv_t = {0};
#define __this (&cube_priv_t)


// 以下页面id优先使用图片作为页面预览
static const struct cube_face_preview preview[] = {
    /* {ID_WINDOW_CALENDAR, PAGE66_b35e_CALENDAR}, */
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

static void cube_anim_exec_cb(int var, int32_t v)
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
    if (v > __this->p_anim->limit_base) {
        v -= __this->p_anim->limit_base;
    } else {
        v = 1;
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
    /* printf("diff:%f, %f, v:%d \n", __this->x_diff, __this->y_diff, v); */

    ui_cube_move(__this->curr_win,  __this->x_diff, __this->y_diff, ui_card_get_move_mode());
}

static void cube_anim_stop(void)
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

static void cube_anim_start(struct element_touch_event *e)
{
    int start_dist, end_dist;
    int run_time;

    // 释放旧的
    cube_anim_stop();

    __this->p_anim = zalloc(sizeof(struct cube_anim));
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

    /* log_info("x:%d, y:%d, en:%d \n", __this->p_anim->dist_x, __this->p_anim->dist_y, energy_t0); */
    /* log_info("hypotenuse:%d, vxy:%d, run:%d \n", (int)__this->p_anim->hypotenuse, (int)vxy, run_time); */

    if (run_time > 4000) {
        run_time = 4000;
    } else if (run_time < 1500) {
        run_time = 1500;
    }
    __this->p_anim->limit_base = 4;
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
    ui_anim_set_exec_cb(&__this->p_anim->anim, cube_anim_exec_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim->anim, start_dist, end_dist);		// 路径设置
    ui_anim_set_time(&__this->p_anim->anim, run_time);						// 运行时间设置
    ui_anim_start(&__this->p_anim->anim);
}

static void cube_anim_enter_cb(int var, int32_t v)
{
    __this->x_diff = -v;
    ui_cube_move(__this->curr_win,  __this->x_diff, __this->y_diff, ui_card_get_move_mode());
}

static void cube_anim_enter()
{
    cube_anim_stop();

    __this->p_anim = zalloc(sizeof(struct cube_anim));
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
    ui_anim_set_exec_cb(&__this->p_anim->anim,  cube_anim_enter_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim->anim, start, end);		// 路径设置
    ui_anim_set_time(&__this->p_anim->anim, run_time);						// 运行时间设置
    ui_anim_start(&__this->p_anim->anim);
}

void cube_effect_init(struct element *curr_elm, struct rect *lcd_rect)
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
    ui_core_show(curr_elm, true);	// 防止影子任务链还未创建，拿到空GPU任务链
    curr_elm->dc->refresh = true;
    pJLGPUTaskHead_t head = curr_elm->dc->gpu_task_head;

    extern void jlgpu_get_win_rect(struct rect * rect);
    struct rect win_rect;
    jlgpu_get_win_rect(&win_rect);

    //默认值
    for (int i = 0; i < 6; i++) {
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
        __this->cube.head = jlgpu_task_list_copy_create(NULL, new_head, NULL,  group_map[0] + 1);
        __this->width_tab[0] = image_attr.width;
        __this->height_tab[0] = image_attr.height;
        jlgpu_delete_task_list_head(new_head);
    } else {
        __this->cube.head = jlgpu_task_list_copy_create(NULL, head, NULL, group_map[0] + 1);
    }
#if UI_EFFECT_FREE_CURR_PAGE_LIST
    jlgpu_free_all_task(head);
    head->gpu_task_base_adr = NULL;
#endif
#if (!UI_CUBE_MAP_ENABLE)
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
#else
    int page_index = 0;
    //curr
    __this->page_id[page_index++] = curr_elm->id;
    //other
    int next_win = curr_elm->id;
    int prev_win = curr_elm->id;
    int page_num = get_ui_page_list_total_num() - 1;
    page_num = (page_num > 4) ? 4 : page_num;
    int prev_num = 1;//往前滑动可以获取到前一个页面，也可以取page_num/2之类的数值
    int next_num = page_num - prev_num;
    //next
    while (next_num--) {
        next_win = ui_page_next(next_win);
        __this->page_id[page_index++] = next_win;
    }
    //prev
    while (prev_num--) {
        prev_win = ui_page_prev(prev_win);
        __this->page_id[page_index++] = prev_win;
    }
    //”+“
    for (; page_index < 6; page_index++) {
        __this->page_id[page_index] = -1;
    }
#endif
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
            jlgpu_task_list_copy_create(__this->cube.head, new_head, NULL, group_map[i] + 1);
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
#if 0//(TCFG_PSRAM_DEV_ENABLE && TCFG_NANDFLASH_DEV_ENABLE)

                        /* 获取页面的宽、高 */
                        struct rect win_rect;
                        ui_core_get_element_abs_rect(win_elm, &win_rect);

                        /* 启动页面合成，并输出到PSRAM */
                        extern void *jlgpu_task_list_dump_to_psram(pJLGPUTaskHead_t head, int w, int h, void *buf_hdl, int disp_buf_size);
                        __this->psram_tab[i] = (u32)jlgpu_task_list_dump_to_psram(win_elm->dc->gpu_task_head, win_rect.width, win_rect.height, win_elm->dc->buf, win_elm->dc->len);

                        /* 将页面帧数据作为图片，创建GPU任务链 */
                        extern pJLGPUTaskHead_t jlgpu_create_task_list_by_psram_img(void *psram_img, int format, int img_w, int img_h);
                        pJLGPUTaskHead_t new_head = jlgpu_create_task_list_by_psram_img((void *)__this->psram_tab[i], GPU_FORMAT_RGB565, win_rect.width, win_rect.height);

                        /* 将页面帧的任务链添加到特效任务链中 */
                        jlgpu_task_list_copy_create(__this->cube.head, new_head, NULL,  group_map[i] + 1);
                        __this->width_tab[i] = win_rect.width;
                        __this->height_tab[i] = win_rect.height;

                        /* 清空页面帧任务链 */
                        jlgpu_delete_task_list_head(new_head);
#else
                        jlgpu_task_list_copy_create(__this->cube.head, win_elm->dc->gpu_task_head, NULL,  group_map[i] + 1);
                        __this->width_tab[i] = win_rect.width;
                        __this->height_tab[i] = win_rect.height;
#endif
                        ui_hide(win);
                    }
                }
            }
        }
    }

    // 释放当前页面，降低RAM峰值
    struct element *p, *n;
    if (curr_elm->id != DIAL_PAGE_0) {
        list_for_each_child_element(p, curr_elm) {
            if (ui_id2type(p->id) == CTRL_TYPE_LAYER) {
                list_for_each_child_element(n, p) {
                    if (ui_id2type(n->id) == CTRL_TYPE_LAYOUT) {
                        n->hide_action = HIDE_WITHOUT_REDRAW;
                        ui_hide(n->id);
                    }
                }
            }
        }
    }

    __this->dial_page_index = -1;
    __this->init = 1;
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
                        jlgpu_task_list_copy_create(__this->cube.head, win_elm->dc->gpu_task_head, NULL, group_map[i] + 1);
#if (UI_EFFECT_REAL_TIME_RUN_CLOSE)
                        ui_hide(win);
                        __this->dial_page_index = -1;
#endif
                    }
                }
            }
        }
    }

    cube_init(__this->width_tab, __this->height_tab, 1.01f, 1.01f, 4.5f, win_rect.left, win_rect.top, win_rect.width, win_rect.height);
    curr_elm->dc->gpu_task_dont_sort = 1;

    int ret = jlgpu_mult_task_head_modify_by_index(curr_elm->dc->gpu_mult_list, curr_elm->dc->index, __this->cube.head);
    ASSERT(!ret);

    __this->curr_elm = curr_elm;
}

void cube_effect_update(int timeout)
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
            int ret = jlgpu_task_list_copy_update_matrix_by_group(__this->cube.head, win_elm->dc->gpu_task_head,   group_map[dial_page_index] + 1);
            if (ret) {
                jlgpu_task_list_copy_destroy_by_group(__this->cube.head, group_map[dial_page_index] + 1);
                ASSERT(win_elm->dc);
                jlgpu_task_list_copy_create(__this->cube.head, win_elm->dc->gpu_task_head, NULL,  group_map[dial_page_index] + 1);
            }
        }

        __this->x_diff = 0;
        __this->y_diff = 0;
        cube_draw(&__this->cube, __this->x_diff, __this->y_diff);
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
        if (((__this->touch_flag || __this->p_anim) && (diff_time >= (timeout * 9 / 10))) || (!__this->touch_flag && !__this->p_anim)) {
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

void cube_effect_uninit()
{
    if (__this->init) {
        cube_anim_stop();

        if (__this->dial_page_index != -1) {
            ui_hide(__this->page_id[__this->dial_page_index]);
            __this->dial_page_index = -1;
        }

        jlgpu_task_list_copy_destroy(__this->cube.head);
        cube_uninit();
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
        __this->cur_left = 0;

#if (TCFG_PSRAM_DEV_ENABLE && TCFG_NANDFLASH_DEV_ENABLE)
        /* 释放页面帧缓存 */
        for (int i = 0; i < 6; i++) {
            if (__this->psram_tab[i]) {
                free_psram((void *)__this->psram_tab[i]);
                __this->psram_tab[i] = 0;
            }
        }
#endif

        /* if (curr_elm->id == ID_WINDOW_DIAL) { */
        /* struct element *watch_elm = ui_core_get_element_by_id(DIAL_WATCH); */
        /* if (watch_elm) { */
        /* watch_load_sidebar(watch_elm); */
        /* } */
        /* } */
    }
}

int ui_cube_status()
{
    return __this->init;
}

int ui_cube_move(int curr_win, int xoffset, int yoffset, int mode)
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

    if (__this->cur_left != 0) {
        __this->curr_win = curr_win;
        cube_effect_init(curr_elm, &lcdrect);
    }

    ui_page_set_busy(1);

    if (__this->init) {
        __this->x_diff = xoffset;
        __this->y_diff = yoffset;
        cube_draw(&__this->cube, __this->x_diff, __this->y_diff);
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

int ui_cube_ontouch(struct element_touch_event *e)
{
    int xoffset, yoffset;
    /* printf("%s %d", __func__, e->event); */
    switch (e->event) {
    case ELM_EVENT_TOUCH_ENERGY:
        if (!__this->init) {
            break;
        }
        cube_anim_start(e);
        return true;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        cube_anim_stop();
        __this->pos_x = e->pos.x;
        __this->pos_y = e->pos.y;
        __this->hold_flag = 0;
        __this->move_flag = 0;
        __this->touch_flag = 1;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        __this->hold_flag = 1;
        break;
    case ELM_EVENT_TOUCH_MOVE:
        xoffset = e->pos.x - __this->pos_x;
        yoffset = e->pos.y - __this->pos_y;
        if (!xoffset && !yoffset) {
            break;
        }

        __this->pos_x = e->pos.x;
        __this->pos_y = e->pos.y;
        ui_cube_move(ui_get_current_window_id(), xoffset, yoffset, ui_card_get_move_mode());
        __this->move_flag = 1;
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        __this->touch_flag = 0;
        if (__this->hold_flag) {
            break;
        }
        if (__this->move_flag) {
            break;
        }
        if (e->has_energy) {
            break;
        }
        if (!__this->init) {
            break;
        }
        int face_index = cube_get_face_index(e->pos.x, e->pos.y);
        /* printf("touch face %d\n", face_index); */
        if (face_index != -1) {
            int face_index_map =  face_index;
            face_index = group_remap[face_index];
            printf("%s face_index:%d map %d", __func__, face_index_map, face_index);
            int select_page = __this->page_id[face_index];
            if (select_page == -1) {
                select_page = ID_WINDOW_COMPONENT;
                printf(" map to add componenet");
            }
            cube_effect_uninit();
            int curr_page = ui_get_current_window_id();
            /* struct element *curr_elm = ui_core_get_element_nowarning_by_id(curr_page); */

            /* if (__this->dial_page_index != -1) { */
            /*     ui_hide(__this->page_id[__this->dial_page_index]); */
            /* } */

            ui_hide(curr_page);
            ui_show(select_page);
#if 0
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

            return true;
        }
        break;
    }
    return 0;
}

extern int lcd_get_screen_width();
extern int lcd_get_screen_height();
#define TP_FPS			60
#define ENERGY_T_MS			(1000000/TP_FPS/1000)
void ui_cube_rdec(int xoffset)
{
    int x = lcd_get_screen_width() / 2;
    int y = lcd_get_screen_height() / 2;
    u8 xdir;
    u16 x_offset = (u16)xoffset;
    struct element_touch_event t = {0};

    cube_anim_stop();
    __this->pos_x = x;
    __this->pos_y = y;
    __this->hold_flag = 0;
    __this->move_flag = 0;
    __this->touch_flag = 0;

    xdir = (xoffset < 0) ? 1 : 2;
    t.event = ELM_EVENT_TOUCH_ENERGY;
    t.pos.x = (x_offset << 16) | (ENERGY_T_MS & 0xffff);
    t.pos.y = (0 << 16) | (2 << 8) | (xdir & 0xff);
    t.has_energy = 1;
    cube_anim_start(&t);
}

REGISTER_UI_EFFECT_MODULE(PAGE_MOVE_MODE_CUBE)
.ontouch =  ui_cube_ontouch,
 .get_status = ui_cube_status,
  .uninit = cube_effect_uninit,
};

#endif
