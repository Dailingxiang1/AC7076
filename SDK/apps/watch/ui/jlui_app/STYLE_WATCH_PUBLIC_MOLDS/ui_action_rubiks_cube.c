#include "app_config.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"
#include "rubiks_cube.h"
#include "gpu_port.h"
#include "gpu_task.h"
#include "ui_expand/ui_color.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_RUBIKS_CUBE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_rubiks_cube.data.bss")
#pragma data_seg(".ui_action_rubiks_cube.data")
#pragma const_seg(".ui_action_rubiks_cube.text.const")
#pragma code_seg(".ui_action_rubiks_cube.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_RUBIKS_CUBE

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)


/* 魔方游戏使用图片 */
#define RUBIKS_CUBE_USED_PIC		0


#if (defined RUBIKS_CUBE_USED_PIC && RUBIKS_CUBE_USED_PIC)
struct pic_map_def {
    u32 color;
    u32 image;
};

static struct pic_map_def pic_map[] = {
    {0x000000, 0x490001}, // 图片分辨率 160x120
    {0x00aa00, 0x490002},
    {0xaa00aa, 0x490003},
    {0xaa0000, 0x490004},
    {0x0000aa, 0x490005},
    {0xaa5500, 0x490006},
    {0xffff55, 0x490007},
};

static inline u32 get_pic_id(u32 color)
{
    for (int i = 0; i < ARRAY_SIZE(pic_map); i++) {
        if (color == pic_map[i].color) {
            return pic_map[i].image;
        }
    }
    return (u32)(-1);
}
#endif


typedef struct {
    uint16_t minx;
    uint16_t maxx;
    uint16_t miny;
    uint16_t maxy;
} boundbox_t;


struct rubiks_cube_t {
    u16 cube_timer;
    u16 task_id;
    struct element *elm;
    void *task_head;
    void *cube_head;
};
struct rubiks_cube_t *rubiks_cube = NULL;


static inline void *jlgpu_get_task_addr(pJLGPUTaskUnit_t taskp)
{
    return (taskp) ? ((void *)&taskp->task_addr) : (NULL);
}


/* 配置GPU任务 */
static void config_gpu_task(boundbox_t *bbox, int fg_w, int fg_h, u8 color[4], float mat[9])
{
    /* 如果魔方链表未初始化，则初始化链表 */
    if (!rubiks_cube->cube_head) {
        rubiks_cube->cube_head = jlgpu_create_task_list_head();
    }

    u32 task_id = rubiks_cube->task_id;
    u32 elm_id = rubiks_cube->elm->id;

#if (defined RUBIKS_CUBE_USED_PIC && RUBIKS_CUBE_USED_PIC)
    /* 使用图片显示魔方 */
    struct ui_image_attrs image_attr;
    struct draw_context dc_tmp;
    u32 image_id = get_pic_id((u32)TO_RGB888(color[2], color[1], color[0]));
    if (image_id == (u32) - 1) {
        ASSERT(0, "error, image id err: 0x%x\n", image_id);
    }
    dc_tmp.prj = 0;
    dc_tmp.page = (image_id >> 16) & 0xff;
    platform_api->read_image_info(&dc_tmp, image_id & 0xffff, &image_attr);
    /* printf("@@@@@ image w: %d, h: %d, fmt: %d, clut_fmt: %d, compress: %d, has_clut: %d, len: %d, gpu_format: %d, clut_tab: 0x%x\n", image_attr.width, image_attr.height, image_attr.format, image_attr.clut_format, image_attr.compress, image_attr.has_clut, image_attr.len, image_attr.gpu_format, (u32)image_attr.clut_tab); */

    jlgpu_task_texture_param_init(task_id, elm_id);
    gpu_matrix_t M;
    gpu_matrix_set_identity(&M);
    task_param.matrix = &M;
    task_param.perspective_en = true;
    pJLGPUTaskUnit_t taskp = jlgpu_create_task(rubiks_cube->cube_head, &task_param);

    gpu_basic_params_t basic_param = {0};
    gpu_texture_params_t texture_param = {0};
    gpu_transform_params_t transform_param = {0};
    basic_param.layer_en = 1;
    basic_param.global_alpha = 128;
    basic_param.act_x_min = bbox->minx; // 60x60
    basic_param.act_x_max = bbox->maxx;
    basic_param.act_y_min = bbox->miny;
    basic_param.act_y_max = bbox->maxy;

    basic_param.blend_mode = 1;
    basic_param.blue = color[0];
    basic_param.green = color[1];
    basic_param.red = color[2];
    basic_param.alpha = 0xff;
    basic_param.clut_format = image_attr.clut_format;
    if (image_attr.has_clut == CLUT_TAB_FROM_FLASH) {
        int clut_size = get_clut_format_tabsize(image_attr.format, image_attr.clut_format);
        u32 clut_addr = (u32)image_attr.data - clut_size;

        if ((clut_addr % 4) == 0) {
            basic_param.clut = (u8 *)clut_addr;
        } else {
            taskp->info.clut_tab = malloc(clut_size);
            memcpy(taskp->info.clut_tab, (void *)clut_addr, clut_size);
            basic_param.clut = (u8 *)taskp->info.clut_tab;
        }
    }

    texture_param.data = image_attr.data;
    texture_param.format = image_attr.gpu_format;
    texture_param.big_end = 0;
    texture_param.alpha_end = 0;
    texture_param.rbs = 0;
    texture_param.color_ext_mode = 0;

    int stride = 0;
    u8 adr_mode = 0;
    u8 compress = 0;
    int compress_size = 0;
    struct image_file image;
    image.format = image_attr.format;
    image.width = image_attr.width;
    image.height = image_attr.height;
    image.compress = image_attr.compress;
    image.has_clut = image_attr.has_clut;
    image.len = image_attr.len;
    stride = res_get_image_stride(texture_param.format, &image, &adr_mode, &compress, &compress_size);
    texture_param.stride = stride;
    texture_param.adr_mode = adr_mode;
    texture_param.compress_size = compress_size;
    texture_param.compress_mode = compress;
    /* printf("@@@@@ stride: %d, adr_mode: %d, compress: %d, compress_size: %d\n", stride, adr_mode, compress, compress_size); */
    /* printf("fg_w: %d, fg_h: %d\n", fg_w, fg_h); */

    transform_param.fg_x_min = 0;
    transform_param.fg_x_max = fg_w; //  原图大小 160x120
    transform_param.fg_y_min = 0;
    transform_param.fg_y_max = fg_h;
    transform_param.sample_mode = 1;
    transform_param.shift_sel = 6;

    transform_param.M00 = mat[0];
    transform_param.M01 = mat[1];
    transform_param.M02 = mat[2];
    transform_param.M10 = mat[3];
    transform_param.M11 = mat[4];
    transform_param.M12 = mat[5];
    transform_param.M20 = mat[6];
    transform_param.M21 = mat[7];
    transform_param.M22 = mat[8];

    gpu_task_set_texture_perspective(jlgpu_get_task_addr(taskp), &basic_param, &texture_param, &transform_param);
    rubiks_cube->task_id += 1;
#else

    /* 使用填充显示魔方 */
    jlgpu_task_fill_param_init(task_id, elm_id, 100, color[2], color[1], color[0]);
    gpu_matrix_t M;
    gpu_matrix_set_identity(&M);
    task_param.matrix = &M;
    task_param.perspective_en = true;
    pJLGPUTaskUnit_t taskp = jlgpu_create_task(rubiks_cube->cube_head, &task_param);

    gpu_basic_params_t basic_param;
    gpu_transform_params_t transform_param;
    memset(&basic_param, 0, sizeof(basic_param));
    memset(&transform_param, 0, sizeof(transform_param));
    basic_param.layer_en = 1;
    basic_param.global_alpha = 128;
    basic_param.act_x_min = bbox->minx; // 60x60
    basic_param.act_x_max = bbox->maxx;
    basic_param.act_y_min = bbox->miny;
    basic_param.act_y_max = bbox->maxy;

    basic_param.blend_mode = 1;
    basic_param.blue = color[0];
    basic_param.green = color[1];
    basic_param.red = color[2];
    /*basic_param.alpha = color[3];*/
    basic_param.alpha = 255;

    transform_param.fg_x_min = 0;
    transform_param.fg_x_max = fg_w;
    transform_param.fg_y_min = 0;
    transform_param.fg_y_max = fg_h;
    transform_param.sample_mode = 1;
    transform_param.shift_sel = 6;

    transform_param.M00 = mat[0];
    transform_param.M01 = mat[1];
    transform_param.M02 = mat[2];
    transform_param.M10 = mat[3];
    transform_param.M11 = mat[4];
    transform_param.M12 = mat[5];
    transform_param.M20 = mat[6];
    transform_param.M21 = mat[7];
    transform_param.M22 = mat[8];

    gpu_task_set_fill_perspective(jlgpu_get_task_addr(taskp), &basic_param, &transform_param);
    rubiks_cube->task_id += 1;
#endif
}

int rubiks_cube_destroy_gpu_list(void *head)
{
    int argv[3] = {0};
    int retry = 3;
    argv[0] = (int)jlgpu_task_list_copy_destroy;
    argv[1] = 1;
    argv[2] = (int)head;

__try_again:
    int ret = os_taskq_post_type(GPU_TASK_NAME, Q_CALLBACK, 3, argv);
    if (ret) {
        if (retry) {
            os_time_dly(1);
            retry--;
            goto __try_again;
        }
        printf("%s post ret:%d retry:%d\n", __func__, ret, retry);
    }
    return ret;
}

/* 刷新屏幕 */
static void draw_rubiks_cube()
{
    struct element *curr_elm = ui_core_get_element_by_id(ui_get_current_window_id());
    /* rubiks_cube->task_head = jlgpu_shadow_task_list_head(curr_elm->dc->gpu_task_head); // 复制一下当前页面的GPU任务 */
    rubiks_cube->task_head = jlgpu_task_list_copy_create(NULL, curr_elm->dc->gpu_task_head, NULL, 0); // 复制一下当前页面的GPU任务
    rubiks_cube->task_head = jlgpu_task_list_copy_create(rubiks_cube->task_head, rubiks_cube->cube_head, NULL, 0);
    int ret = jlgpu_mult_task_head_modify_by_index(curr_elm->dc->gpu_mult_list, curr_elm->dc->index, rubiks_cube->task_head);

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

    /* 合成完毕清空魔方任务链 */
    rubiks_cube_destroy_gpu_list(rubiks_cube->task_head);
    jlgpu_delete_task_list_head(rubiks_cube->cube_head);
    rubiks_cube->task_id = 1;
    rubiks_cube->task_head = NULL;
    rubiks_cube->cube_head = NULL;
}

void ui_rubiks_cube_init(void *priv)
{
    struct rect win_rect;
    jlgpu_get_win_rect(&win_rect);

    struct element *cur_elm = ui_core_get_element_by_id(ui_get_current_window_id());
    ui_core_show(cur_elm, true);
    rubiks_cube->elm = cur_elm; // 记录当前elm句柄
    jlgpu_scheduler_set_redraw_mode(cur_elm->dc, GPU_SYNC_REDRAW); // 设置为同步刷新

    win_rect.left = 0;
    win_rect.top = 0;
    jlgpu_set_win_rect(&win_rect);

    rubiks_cube_init(win_rect.width, win_rect.height, config_gpu_task, draw_rubiks_cube);
    /* rubiks_cube_init(win_rect.width, win_rect.height, NULL, NULL); // 使用原来底层逻辑 */

    if (rubiks_cube && rubiks_cube->cube_timer) {
        sys_timeout_del(rubiks_cube->cube_timer);
        rubiks_cube->cube_timer = 0;
    }
}

void ui_rubiks_cube_uninit()
{
    struct rect win_rect;
    jlgpu_get_win_rect(&win_rect);

    win_rect.left = (2047 - win_rect.width) / 2;
    win_rect.top = (2047 - win_rect.height) / 2;
    jlgpu_set_win_rect(&win_rect);

    struct element *elm = rubiks_cube->elm;
    int ret = jlgpu_mult_task_head_modify_by_index(elm->dc->gpu_mult_list, elm->dc->index, elm->dc->gpu_task_head);
    ASSERT(!ret);
    jlgpu_scheduler_set_redraw_mode(elm->dc, GPU_ASYN_REDRAW);

    if (rubiks_cube->task_head) {
        rubiks_cube_destroy_gpu_list(rubiks_cube->task_head);
    }
    if (rubiks_cube->cube_head) {
        jlgpu_delete_task_list_head(rubiks_cube->cube_head);
    }

    rubiks_cube_uninit();
}

static int layout_rubiks_cube_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:
        //特效不执行
        if (!rubiks_cube) {
            rubiks_cube = zalloc(sizeof(struct rubiks_cube_t));
        }
        log_info("ID_WINDOW_RUBIKS_CUBE init!!!");
        if (rubiks_cube && !rubiks_cube->cube_timer) {
            rubiks_cube->cube_timer = sys_timeout_add(NULL, ui_rubiks_cube_init, 200);
        }
        rubiks_cube->elm = (struct element *)_ctrl;
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_RELEASE:
        log_info("ID_WINDOW_RUBIKS_CUBE release!!!");
        if (rubiks_cube && !rubiks_cube->cube_timer) {
            ui_rubiks_cube_uninit();
        }
        if (rubiks_cube && rubiks_cube->cube_timer) {
            sys_timeout_del(rubiks_cube->cube_timer);
            rubiks_cube->cube_timer = 0;
        }

        if (rubiks_cube) {
            free(rubiks_cube);
            rubiks_cube = NULL;
        }

        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return false;
}

static int layout_rubiks_cube_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        OnMove(e->pos.x, e->pos.y);
        break;
    case ELM_EVENT_TOUCH_UP:
        OnMouseUp(e->pos.x, e->pos.y);
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        break;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_RUBIKS_CUBE)
.onchange = layout_rubiks_cube_onchange,
 .onkey = NULL,
  .ontouch = layout_rubiks_cube_ontouch,
};


#endif
#endif
