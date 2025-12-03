#include "app_config.h"
/* #include "app_task.h" */
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "rtc.h"

#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"
#include "ui_page_switch.h"
#include "btstack/avctp_user.h"
#include "jlui_app/ui_app_effect.h"
#include "gpu_task.h"
#include "ui_expand/ui_scrollview.h"
#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_DIAL]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_dial.data.bss")
#pragma data_seg(".ui_action_dial.data")
#pragma const_seg(".ui_action_dial.text.const")
#pragma code_seg(".ui_action_dial.text")
#endif


#ifdef CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE
#if TCFG_UI_DIAL_SEL_ENABLE


/* 表盘预览图缩放后的倍率 */
#define DIAL_PREVIEW_RATIO		0.7f

/* 表盘缩略图缩小后的间隙 */
#define DIAL_PREVIEW_SPACE		0

/* 缩放动画效果过渡时间 ms */
#define DIAL_ZOOM_ANIM_TIME		300

/* 遮罩图片ID，从result_pic_index.h获取 */
#define DIAL_PREVIEW_MASK		PAGE57_b4c0_DIAL_MASK

/* 编码器旋钮时步进量 */
#define DIAL_KEY_MOVE_STEP		(20)

/* 遮罩任务ID，自由配置，不与页面中其它任务ID冲突即可 */
#define DIAL_GPU_TASK_ID		(JLGPU_ID_NONE - 10)


/* 表盘选择状态 */
typedef enum {
    DIAL_SEL_NULL, // 没在表盘选择功能
    DIAL_SEL_INIT, // 进入表盘选择功能
    DIAL_SEL_MOVE, // 正在表盘选择功能
    DIAL_SEL_FREE, // 退出表盘选择功能
} dial_sel_t;


/* 表盘选择私有参数 */
struct dial_sel_priv {
    u8 last_event; // TP的上一次事件
    u8 curr_watch; // 当前的表盘风格
    u8 watch_num; // 表盘数量
    dial_sel_t state; // 表盘选择状态
    ui_anim_t anim; // 表盘选择动画
    struct ui_image_attrs mask; // 遮罩图片属性
    struct rect elm_rect; // 表盘大小
    pJLGPUTaskHead_t head; // 表盘选择GPU任务链头
    int x_start; // 表盘预览图列表X坐标起始位置
    int y_start;
    int ratio_w; // 表盘预览图缩小后的宽高
    int ratio_h;
    float zoom; // 缩放倍数，进入、退出动画使用
    void *mask_data; // mask图片数据缓存地址
    ui_scrollview_t scroll; // 滚动视图，用于惯性等
};
/* static struct dial_sel_priv dial_sel = {0}; */
static struct dial_sel_priv *dial_sel = NULL;
#define __this	(dial_sel)


/* 加载和释放侧边栏 */
extern int watch_unload_sidebar(struct element *elm);
extern int watch_load_sidebar(struct element *elm);
extern int window_init(int id);
/* 表盘刷新定时器 */
extern void start_watch_timer();
extern void stop_watch_timer();

/* 表盘数据 */
extern int watch_get_items_num();
extern char *watch_get_item(int style);
extern void watch_set_need_reload(u8 need);
/* 缓存mmu开关 */
extern void mmu_tab_cache_start();
extern void mmu_tab_cache_clear();


int dial_sel_state()
{
    return (__this) ? ((__this->state == DIAL_SEL_NULL) ? 0 : 1) : 0;
}

/* 获取mask_id图片属性 */
static void get_mask_image_attr(struct element *elm, u32 mask_id, struct ui_image_attrs *attr)
{
    if (mask_id == -1) {
        return;
    }

    struct rect rect = {0};
    struct draw_context dc;
    ui_core_get_draw_context(&dc, elm, &rect);

    dc.prj = 0;
    dc.page = (mask_id >> 16) & 0xff;
    platform_api->read_image_info(&dc, (mask_id & 0xffff), attr);

#if TCFG_NANDFLASH_DEV_ENABLE
    /* nandflash时需要将数据缓存到PSRAM，但加载表盘时会释放当前页面，故需要将mask数据另外缓存，自己管理 */
    if (attr->data && !__this->mask_data) {
        int tab_size = get_clut_format_tabsize(attr->format, attr->clut_format);
        int data_len = attr->len + tab_size;
        void *data = (void *)((u32)attr->data - tab_size);
        __this->mask_data = malloc_psram(data_len);
        memcpy(__this->mask_data, data, data_len);
        attr->data = (u8 *)((u32)__this->mask_data + tab_size);
    }
#endif
}

/* 向任务链中添加mask图片任务 */
static void add_mask_to_gpu_list(pJLGPUTaskHead_t head, struct ui_image_attrs *mask_attr)
{
    if (!mask_attr || !mask_attr->width || !mask_attr->height) {
        return;
    }

    u32 task_id = DIAL_GPU_TASK_ID;
    u32 elm_id = DIAL_GPU_TASK_ID;
    jlgpu_task_texture_param_init(task_id, elm_id);

    /* 绘制区域 */
    task_param.draw.left = 0;
    task_param.draw.top = 0;
    task_param.draw.width = mask_attr->width;
    task_param.draw.height = mask_attr->height;
    jlgpu_get_win_rect(&task_param.area);

    /* 纹理指令 */
    task_param.format         = mask_attr->gpu_format;
    task_param.clut_format    = mask_attr->clut_format;
    task_param.has_clut       = mask_attr->has_clut;
    task_param.image.format   = mask_attr->format;
    task_param.image.width    = mask_attr->width;
    task_param.image.height   = mask_attr->height;
    task_param.image.compress = mask_attr->compress;
    task_param.image.has_clut = mask_attr->has_clut;
    task_param.image.len      = mask_attr->len;
    task_param.texture.data   = mask_attr->data;
    task_param.texture.mmu_tab_base = mask_attr->tab;

    jlgpu_update_task_by_id(head, task_id, elm_id, &task_param);
}

/* 加载所有的表盘GPU任务链 */
static void dial_sel_load_gpu_task_list(u32 win_id, struct ui_image_attrs *mask_attr)
{
    char *item;
    struct element *win_elm;
    watch_set_need_reload(0);
    /* 加载除当前表盘外的其它表盘 */
    for (int i = 0; i < __this->watch_num; i++) {
        if (i == __this->curr_watch) {
            continue; // 跳过当前表盘
        }

        item = watch_get_item(i); // 表盘路径
        ui_set_sty_path_by_pj_id(1, NULL);
        ui_set_sty_path_by_pj_id(1, (u8 *)item);
        window_init(win_id);
        win_elm = ui_core_get_element_by_id(win_id);
        ASSERT(win_elm);
        win_elm->dc->refresh = false;
        ui_core_show(win_elm, true);
        watch_unload_sidebar(win_elm);	// 释放掉侧边栏
        pJLGPUTaskHead_t head = win_elm->dc->gpu_task_head;
        add_mask_to_gpu_list(head, mask_attr);
        __this->head = jlgpu_task_list_copy_create(__this->head, head, NULL, i);
        ui_hide(win_id);
    }

    /* 加载当前的表盘 */
    item = watch_get_item(__this->curr_watch);
    ui_set_sty_path_by_pj_id(1, NULL);
    ui_set_sty_path_by_pj_id(1, (u8 *)item);

    ui_show(win_id);
    win_elm = ui_core_get_element_by_id(win_id);
    ASSERT(win_elm);
    ASSERT(win_elm->dc);
    jlgpu_scheduler_set_redraw_mode(win_elm->dc, GPU_SYNC_REDRAW); // 设置为同步刷新
    win_elm->dc->refresh = false;
    ui_core_show(win_elm, true); // 一定要在设置同步刷新之后调用
    watch_unload_sidebar(win_elm);
    pJLGPUTaskHead_t head = win_elm->dc->gpu_task_head;
    add_mask_to_gpu_list(head, mask_attr);
    __this->head = jlgpu_task_list_copy_create(__this->head, head, NULL, __this->curr_watch);
    int ret = jlgpu_mult_task_head_modify_by_index(win_elm->dc->gpu_mult_list, win_elm->dc->index, __this->head);
    ASSERT(!ret);
}

void dial_sel_update(int update_sec)
{
    if (!dial_sel_state()) {
        return;
    }
    if (__this->state == DIAL_SEL_MOVE && update_sec) {
        struct element *dial_elm = ui_core_get_element_nowarning_by_id(ID_WINDOW_DIAL);
        if (dial_elm) {
            int ret = jlgpu_task_list_copy_update_matrix_by_group(__this->head, dial_elm->dc->gpu_task_head, __this->curr_watch);
            if (ret) {
                jlgpu_task_list_copy_destroy_by_group(__this->head, __this->curr_watch);
                ASSERT(dial_elm->dc);
                jlgpu_task_list_copy_create(__this->head, dial_elm->dc->gpu_task_head, NULL, __this->curr_watch);
            }
        }
    }
    gpu_matrix_t matrix;
    for (int i = 0; i < __this->watch_num; i++) {
        gpu_matrix_set_identity(&matrix);
        gpu_matrix_translate(&matrix, -(__this->x_start + i * __this->ratio_w), -__this->y_start); // 按顺序排列
        gpu_matrix_scale(&matrix, __this->zoom, __this->zoom); // 一样的缩放等级
        jlgpu_task_list_copy_mul_matrix_by_group(__this->head, &matrix, i); // 跟新GPU任务链的矩阵
    }

    struct element *curr_elm = ui_core_get_element_by_id(ui_get_current_window_id());
    ASSERT(curr_elm);
    struct rect lcdrect;
    lcdrect.left = 0;
    lcdrect.top = 0;
    ASSERT(curr_elm->dc);
    lcdrect.width = curr_elm->dc->width;
    lcdrect.height = curr_elm->dc->height;
    extern struct ui_platform_api *ui_get_platform_api();
    struct ui_platform_api *platform_api = ui_get_platform_api();
    if (platform_api->put_draw_context) {
        struct rect rect_orig;
        memcpy(&rect_orig, &curr_elm->dc->rect_orig, sizeof(struct rect));
        memcpy(&curr_elm->dc->rect_orig, &lcdrect, sizeof(struct rect));
        curr_elm->dc->refresh = true;
        platform_api->put_draw_context(curr_elm->dc);
        memcpy(&curr_elm->dc->rect_orig, &rect_orig, sizeof(struct rect));
    }
}

static void enter_anim_cb(int var, int32_t v)
{
    float ratio = (float)v / 100; // 计算缩放倍数
    __this->ratio_w = __this->elm_rect.width * ratio; // 计算页面缩放后的宽度
    __this->ratio_h = __this->elm_rect.height * ratio; // 计算页面缩放后的高度
    __this->x_start = (2048 - __this->ratio_w) / 2 - (__this->curr_watch * __this->ratio_w); // 计算缩放后的x起始坐标
    __this->y_start = (2048 - __this->ratio_h) / 2; // 计算缩放后的y起始坐标
    __this->zoom = 1.0f / ratio; // 计算矩阵缩放系数
    dial_sel_update(0);
}

static int dial_sel_scroll(ui_scrollview_t *s, void *priv, int pos)
{
    if (!__this) {
        return -1;
    }
    if (__this->state != DIAL_SEL_MOVE) {
        return -1;
    }
    __this->x_start = pos;
    dial_sel_update(0);
    return 0;
}

static void enter_anim_ready_cb(struct _ui_anim_t *p)
{
    /* 进入动画结束后，初始化滚动模型 */
    ui_scrollview_init(&__this->scroll, NULL, __this->x_start, dial_sel_scroll);

    int max_pos = 1024 - __this->ratio_w / 2;
    int min_pos = max_pos - (__this->watch_num - 1) * __this->ratio_w;
    ui_scrollview_set_scroll_area(&__this->scroll, min_pos, max_pos); // 配置滚动区域
    ui_scrollview_set_align_by_gap(&__this->scroll, __this->ratio_w); // 设置对齐方式为等间距，并设置间距
    ui_scrollview_set_bounces(&__this->scroll, __this->ratio_w / 2); // 设置回弹距离

    __this->state = DIAL_SEL_MOVE;
}


/* 进入表盘选择 */
void dial_sel_enter(void *p)
{
    if (__this->state != DIAL_SEL_INIT) {
        return;
    }
    ui_hide(ui_get_current_window_id()); // 隐藏当前页面

#if (!TCFG_NANDFLASH_DEV_ENABLE)
    mmu_tab_cache_start(); // 缓存mmu_tab功能启动
#endif

    dial_sel_load_gpu_task_list(ID_WINDOW_DIAL, &(__this->mask)); // 加载所有表盘任务链

    ui_anim_init(&__this->anim);
    ui_anim_set_var(&__this->anim, ID_WINDOW_DIAL);
    ui_anim_set_path_cb(&__this->anim, ui_anim_path_ease_out);
    ui_anim_set_exec_cb(&__this->anim, enter_anim_cb);
    ui_anim_set_values(&__this->anim, 1.0f * 100, DIAL_PREVIEW_RATIO * 100);
    ui_anim_set_time(&__this->anim, DIAL_ZOOM_ANIM_TIME);
    ui_anim_set_ready_cb(&__this->anim, enter_anim_ready_cb);
    ui_anim_start(&__this->anim);
}


static void dial_sel_init(struct element *elm)
{
    if (!__this) {
        __this = (struct dial_sel_priv *)malloc(sizeof(struct dial_sel_priv));
        memset(__this, 0x00, sizeof(struct dial_sel_priv));
    }
    if (__this->state != DIAL_SEL_NULL) {
        return;
    }
    stop_watch_timer();
    __this->curr_watch = watch_get_style();
    __this->watch_num = watch_get_items_num();
    __this->state = DIAL_SEL_INIT;

    /* 记录当前页面大小 */
    struct element *win_elm = ui_core_get_element_by_id(ui_get_current_window_id());
    if (win_elm) {
        ui_core_get_element_abs_rect(win_elm, &(__this->elm_rect));
    } else {
        ui_core_get_element_abs_rect(elm, &(__this->elm_rect));
    }
    get_mask_image_attr(elm, DIAL_PREVIEW_MASK, &(__this->mask)); // 加载mask图片信息
    sys_timeout_add(NULL, dial_sel_enter, 100); // 本页面需要退出才能加载其它表盘，因此用timeout来错开时间回调
}

static void leave_anim_ready_cb(struct _ui_anim_t *p)
{
    if (!__this) {
        return;
    }
    ui_anim_del(ID_WINDOW_DIAL, NULL);
    ui_scrollview_stop(&__this->scroll);
    jlgpu_scheduler_wait_sync();
    u32 curr_page = ui_get_current_window_id();
    if (curr_page && (curr_page != -1)) {
        struct element *elm = ui_core_get_element_by_id(curr_page);
        if (elm) {
            int ret = jlgpu_mult_task_head_modify_by_index(elm->dc->gpu_mult_list, elm->dc->index, elm->dc->gpu_task_head);
            ASSERT(!ret);
            jlgpu_scheduler_set_redraw_mode(elm->dc, GPU_ASYN_REDRAW); // 恢复为异步刷新

        }
    }
    ui_hide(curr_page);
    jlgpu_task_list_copy_destroy(__this->head); // 销毁当前GPU任务链
    __this->head = NULL;
#if (!TCFG_NANDFLASH_DEV_ENABLE)
    mmu_tab_cache_clear(); // 清空mmu缓存
#endif
    if (__this->mask_data) {
        free_psram(__this->mask_data);
        __this->mask_data = NULL;
    }
    if (p) {
        watch_set_style(__this->curr_watch); // 设置表盘风格
        ui_show(curr_page);
    }

    __this->state = DIAL_SEL_NULL;
    __this->head = NULL;

    free(__this);
    __this = NULL;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief dial_sel_exit 弹窗打断
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int dial_sel_exit()
{
    if (dial_sel_state()) {
        leave_anim_ready_cb(NULL);
    }
    return 0;
}
/* 退出表盘选择 */
void dial_sel_leave(void *p)
{
    if (__this->state != DIAL_SEL_FREE) {
        return;
    }

    ui_anim_init(&__this->anim);
    ui_anim_set_var(&__this->anim, ID_WINDOW_DIAL);
    ui_anim_set_path_cb(&__this->anim, ui_anim_path_ease_out);
    ui_anim_set_exec_cb(&__this->anim, enter_anim_cb);
    ui_anim_set_values(&__this->anim, DIAL_PREVIEW_RATIO * 100, 1.0f * 100);
    ui_anim_set_time(&__this->anim, DIAL_ZOOM_ANIM_TIME);
    ui_anim_set_ready_cb(&__this->anim, leave_anim_ready_cb);
    ui_anim_start(&__this->anim);
}

void dial_sel_free(struct element *elm, int touch_x, int touch_y)
{
    if ((touch_x == -1) && (touch_y == -1)) {
        //按键退出
        leave_anim_ready_cb(&__this->anim);
        return;
    }
    /* 根据点击点判断选择的表盘是哪个 */
    int win_left = (2048 - __this->elm_rect.width) / 2;
    int sel_index = ((touch_x + win_left) - __this->x_start) / __this->ratio_w;
    if (sel_index <= 0) {
        sel_index = 0;
    } else if (sel_index >= __this->watch_num) {
        sel_index = __this->watch_num - 1;
    }
    ui_scrollview_free(&__this->scroll);

    __this->curr_watch = sel_index;
    __this->state = DIAL_SEL_FREE;
    sys_timeout_add(NULL, dial_sel_leave, 100); // 启动退出动画
}

int dial_sel_touch(struct element *elm, struct element_touch_event *e)
{
    if ((dial_sel_state()) || (e->event == ELM_EVENT_TOUCH_HOLD)) {
        switch (e->event) {
        case ELM_EVENT_TOUCH_DOWN:
            break;
        case ELM_EVENT_TOUCH_MOVE:
            ui_scrollview_move_offset(&__this->scroll, e->xoffset); // 划屏滚动
            break;
        case ELM_EVENT_TOUCH_HOLD:
            dial_sel_init(elm); // 进入表盘选择
            break;
        case ELM_EVENT_TOUCH_UP:
            if (__this->last_event == ELM_EVENT_TOUCH_DOWN) {
                dial_sel_free(elm, e->pos.x, e->pos.y); // 点击后抬起，选中表盘，退出表盘选择
            } else {
                ui_scrollview_auto_align(&__this->scroll); // 划屏后抬起，自动对齐表盘预览图显示
            }
            break;
        case ELM_EVENT_TOUCH_ENERGY:
            ui_scrollview_move_velocity(&__this->scroll, e->pos.x >> 16); // 划屏惯性
            break;
        default:
            break;
        }
        __this->last_event = e->event; // 记录上一次tp事件消息，用于判断抬起时选中或对齐

        return true;
    } else {
        return false;
    }
}

int dial_sel_onkey(struct element *elm, struct element_key_event *event)
{
    if (dial_sel_state()) {
        if (event->value == KEY_UI_MINUS) {
            ui_scrollview_move_accrue(&__this->scroll, DIAL_KEY_MOVE_STEP);
        } else if (event->value == KEY_UI_PLUS) {
            ui_scrollview_move_accrue(&__this->scroll, -DIAL_KEY_MOVE_STEP);
        }
        return true;
    } else {
        return false;
    }
}


#endif // !TCFG_UI_DIAL_SEL_ENABLE

#endif // !CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE


