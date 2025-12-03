/* Copyright(C) 2010- , JIELI TECHNOLOGY, Inc.
 * All right reserved.
 */

/* ------------------------------------------------------------------------------------*/
/**
 * @file gpu_demo_task.c
 *
 * @brief GPU 系统任务，GPU demo调度与推屏逻辑
 *
 * @author zhuhaifang@zh-jieli.com
 *
 * @version V1.0.0
 *
 * @date 2024-09-13
 */
/* ------------------------------------------------------------------------------------*/

#include "typedef.h"
#include "rect.h"
#include "res/resfile.h"

#include "dbi.h"
#include "jlgpu_math.h"		// matrix
#include "jlgpu_driver.h"	// gpu driver
#include "ui_resource.h"	// JL UI resource
#include "ui_expand/ui_expand.h"	// macro defined

#include "gpu_port.h"		// module head file
#include "gpu_draw.h"		// custom draw head file

#include "ui_expand/buffer_manager.h"

#include "ui_core.h"
#include "ui_measure.h"

#include "ui/lcd/lcd_drive.h"
#include "res/mem_var.h"
#include "football.h"

#include "gpu_demo.h"

#if defined GPU_DEMO_TASK_MAIN && GPU_DEMO_TASK_MAIN


/* GPU DEMO 背景颜色 */
#define GPU_DEMO_BACKGROUND		0xff404040


struct gpu_port_demo_priv {
    struct lcd_interface *lcd;		// LCD 设备控制API
    struct lcd_info info;			// LCD 设备信息

    pJLGPUTaskHead_t gpu_task_head;	// GPU任务链头指针

    void *buffer_hdl;	// 显存buffer管理句柄
    void *buffer_adr;	// 当前使用的显存buffer

    OS_SEM buffer_sem;	// buffer使用信号量
};
static struct gpu_port_demo_priv priv = {0};
#define __this		(&priv)


static void *gpu_port_demo_malloc(int size, u32 ram_type, u32 module_type)
{
    void *buf = (void *)malloc(size);
    return buf;
}

static void gpu_port_demo_free(void *buf, u32 ram_type, u32 module_type)
{
    free(buf);
}


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_port_demo_create_task 创建GPU DEMO任务
 *
 * @Params head GPU任务链头
 */
/* ------------------------------------------------------------------------------------*/
void gpu_port_demo_create_task(pJLGPUTaskHead_t head)
{
#if defined GPU_DEMO_TASK_FILL && GPU_DEMO_TASK_FILL
    extern void gpu_demo_create_fill_task(pJLGPUTaskHead_t head);
    gpu_demo_create_fill_task(head);
#elif defined GPU_DEMO_TASK_FILL_MASK && GPU_DEMO_TASK_FILL_MASK
    extern void gpu_demo_create_fill_mask(pJLGPUTaskHead_t head);
    gpu_demo_create_fill_mask(head);
#elif defined GPU_DEMO_TASK_FILL_AFFINE && GPU_DEMO_TASK_FILL_AFFINE
    extern void gpu_demo_create_fill_affine(pJLGPUTaskHead_t head);
    gpu_demo_create_fill_affine(head);
#elif defined GPU_DEMO_TASK_LINEGRAD && GPU_DEMO_TASK_LINEGRAD
    extern void gpu_demo_create_linegrad_task(pJLGPUTaskHead_t head);
    gpu_demo_create_linegrad_task(head);
#endif
#if defined GPU_DEMO_COLOR_SWAP_ENABLE && GPU_DEMO_COLOR_SWAP_ENABLE
    extern void gpu_demo_color_swap(pJLGPUTaskHead_t head);
    gpu_demo_color_swap(head);
#endif
#if defined GPU_DEMO_BLEND_ENABLE	&& GPU_DEMO_BLEND_ENABLE
    extern void gpu_demo_blend_mode(pJLGPUTaskHead_t head);
    gpu_demo_blend_mode(head);
#endif
#if defined GPU_DEMO_ALPHA_PREMULT_ENABLE	&& GPU_DEMO_ALPHA_PREMULT_ENABLE
    extern void gpu_demo_alpha_premult_test(pJLGPUTaskHead_t head);
    gpu_demo_alpha_premult_test(head);
#endif
#if defined GPU_DEMO_TASK_TEXTURE && GPU_DEMO_TASK_TEXTURE
    void  gpu_demo_texture_test(pJLGPUTaskHead_t head);
    gpu_demo_texture_test(head);
#endif
#if defined GPU_DEMO_OUTPUT_FORMAT && GPU_DEMO_OUTPUT_FORMAT
    void gpu_demo_output_mode(pJLGPUTaskHead_t head);
    gpu_demo_output_mode(head);
#endif
#if defined GPU_DEMO_MASK_SWAP&& GPU_DEMO_MASK_SWAP
    void gpu_demo_mask_swap_test(pJLGPUTaskHead_t head);
    gpu_demo_mask_swap_test(head);
#endif
#if defined GPU_DEMO_CUSTOM_DRAW && GPU_DEMO_CUSTOM_DRAW
    void gpu_demo_custom_draw_demo(pJLGPUTaskHead_t head);
    gpu_demo_custom_draw_demo(head);
#endif
#if defined JPEG_DEMO_ENABLE && JPEG_DEMO_ENABLE
    void jpeg_demo_test(pJLGPUTaskHead_t head);
    jpeg_demo_test(head);
#endif
#if (defined GPU_DEMO_WITH_PSRAM && GPU_DEMO_WITH_PSRAM)
    extern void gpu_demo_with_psram(pJLGPUTaskHead_t head);
    gpu_demo_with_psram(head);
#endif
}


/* LCD 屏幕初始化 */
static int lcd_flush_finish_cb(void *priv);
static void gpu_port_demo_lcd_init(void)
{
    /* 获取LCD设备句柄 */
    __this->lcd = lcd_get_hdl();

    /* 判断操作LCD必须的API是否存在 */
    ASSERT(__this->lcd);
    ASSERT(__this->lcd->init);
    ASSERT(__this->lcd->get_screen_info);
    ASSERT(__this->lcd->buffer_malloc);
    ASSERT(__this->lcd->buffer_free);
    ASSERT(__this->lcd->draw);
    ASSERT(__this->lcd->set_draw_area);
    ASSERT(__this->lcd->backlight_ctrl);

    /* LCD屏幕初始化 */
    if (__this->lcd->init) {
        extern const struct ui_devices_cfg ui_cfg_data;
        __this->lcd->init((void *)&ui_cfg_data);
    }

    /* 获取LCD屏幕信息，如屏幕宽、高等 */
    if (__this->lcd->get_screen_info) {
        __this->lcd->get_screen_info(&__this->info);
    }

    /* 清空屏幕显示，这里是用DBI模块的硬件纯色推屏功能，没有GPU参与，与LCD debug功能一致。 */
    if (__this->lcd->clear_screen) {
        __this->lcd->clear_screen(0x000000, 0, __this->info.width - 1, 0, __this->info.height - 1);
    }

    /* 设置LCD背光亮度等级为 100% */
    if (__this->lcd->backlight_ctrl) {
        __this->lcd->backlight_ctrl(100);
    }

    /* 创建buffer信号量 */
    os_sem_create(&__this->buffer_sem, 1);

    /* 设置DBI模块中断回调 */
    lcd_draw_set_callback(lcd_flush_finish_cb);

    /* mem 加速缓存，用于减少读flash次数 */
    mem_var_init(3 * 1024, false);
    ui_resfile_info_init(gpu_port_demo_malloc, gpu_port_demo_free);		// UI 资源管理
}

static void gpu_port_demo_port_init(void)
{
    /* GPU模块初始化 */
    jlgpu_module_init(gpu_port_demo_malloc, gpu_port_demo_free, __this->info.width, __this->info.height);
}

static void gpu_port_demo_task_init(int gpu_out_format)
{
    /* 创建一条GPU任务链 */
    __this->gpu_task_head = jlgpu_create_task_list_head();
    if (!__this->gpu_task_head) {
        printf("<gpu_port_demo> gpu create task list head fail!!! ");
        return;
    }

    /* 设置任务链输出的格式 */
    jlgpu_set_task_list_out_format(__this->gpu_task_head, gpu_out_format);
}

static void gpu_port_demo_task_free(void)
{
    if (__this->gpu_task_head) {
        /* 释放GPU任务链，会将任务链中的GPU任务也一并释放 */
        jlgpu_delete_task_list_head(__this->gpu_task_head);
        __this->gpu_task_head = NULL;
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_port_demo_create_bg_task 创建GPU任务链背景任务
 *
 * @Params draw_rect GPU背景任务绘制区域
 * @Params color GPU背景任务颜色
 *
 * @notes GPU背景任务本质为一个填充任务，非必须存在，但若无背景任务，LCD无数据区可能显示
 * 为乱码内容，因此一般需有一个背景任务将无数据区域填充为纯色。
 */
/* ------------------------------------------------------------------------------------*/
static void gpu_port_demo_create_bg_task(struct rect draw_rect, u32 color)
{
    /* 填充颜色拆分为ARGB8888分量 */
    u8 a = (color >> 24) & 0xff;
    u8 r = (color >> 16) & 0xff;
    u8 g = (color >> 8) & 0xff;
    u8 b = (color) & 0xff;

    /* GPU 填充任务配置参数初始化 */
    jlgpu_task_fill_param_init(0, 0, a, r, g, b);

    /* 设置GPU填充区域，默认为整个屏幕区域 */
    memcpy(&task_param.draw, &draw_rect, sizeof(struct rect));

    /* 设置GPU填充任务绘制限制区域，GPU实际绘制为task_param.draw 与 task_param.area 重叠区域，这里也设置为全屏区域 */
    jlgpu_get_win_rect(&task_param.area);

    /* 创建GPU任务，并添加到gpu_task_head任务链中 */
    jlgpu_update_task_by_id(__this->gpu_task_head, 0, 0, &task_param);
}

static void lcd_wait_te()
{
    /*
    TE同步，等待TE需使用TE中断回调，使用信号量进行同步。为避免函数定义冲突，这里不设置TE等待具体函数
    可参考 gpu_task.c 文件中：lcd_wait_te(); lcd_te_sync_int_handler()和lcd_te_sem信号量相关逻辑。
     */
}

/* DBI模块中断回调函数，当DBI模块推完一帧数据，会起来一次中断 */
/* static void lcd_flush_finish_cb(int err) */
static int lcd_flush_finish_cb(void *p)
{
    /* 设置推完的buf为空闲状态 */
    set_buffer_idle(__this->buffer_hdl, __this->buffer_adr);

    /* 发送信号量，允许下一个推屏，lcd_flush_buffer_start会等这个post */
    os_sem_post(&__this->buffer_sem);
    return 0;
}


/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_flush_buffer_start 启动DBI模块推屏
 *
 * @Params buf_hdl buffer管理句柄
 * @Params buf_adr buffer地址/待推屏的buf地址
 * @Params rect 推屏的区域，与buf_adr指向的地址大小相同
 * @Params is_continue 是否续传，每次推屏第一个分块为0，其它分块为1，直到本次推屏结束
 */
/* ------------------------------------------------------------------------------------*/
static void lcd_flush_buffer_start(void *buf_hdl, void *buf_adr, struct rect *rect, int is_continue)
{
    /* 将推屏的rect区域转为start和end区域格式 */
    int x_start = rect->left;
    int x_end   = x_start + rect->width - 1;
    int y_start = rect->top;
    int y_end   = y_start + rect->height - 1;

    /* 等待上一次DBI模块推完，并起中断。在DBI中断会post这个信号量 */
    if (os_sem_pend(&__this->buffer_sem, 100)) {
        /* 如果等待DBI模块推完数据超时，这里抛出Error打印 */
        printf("Error: wait DBI interrupt time out!\n");
    }

    /* 把准备要推的buffer设置为使用状态 */
    set_buffer_used(__this->buffer_hdl, buf_adr);

    /* 把准备要推的buffer放到全局变量记录，等DBI模块推完后再中断中要用 */
    __this->buffer_adr = buf_adr;

    if (!is_continue) {
        /* 仅第一个分块需考虑TE同步 */
        lcd_wait_te();

        /* 第一个分块用kistart接口进行推屏 */
        lcd_draw_kistart(buf_adr, x_start, x_end, y_start, y_end);	// DBI kistart
    } else {
        /* 其它分块用continue接口进行续传即可 */
        lcd_draw_continue(buf_adr, x_start, x_end, y_start, y_end);	// DBI 续传
    }
}


void gpu_port_demo_start_draw(struct rect draw, pJLGPUTaskHead_t head)
{
    u8 *buf;
    u32 len;

    /* 通过LCD的显存申请接口，申请显存buf，用于GPU硬件的输出缓存buf。这个接口会通过LCD屏驱配置的显存buf分块和数量进行申请。 */
    __this->lcd->buffer_malloc(&buf, &len);
    printf("@@@ disp buf init, buf: 0x%p, len: %d\n", buf, len);

    /* 初始化显存buf管理模块，把刚刚申请的显存buf，和LCD屏驱配置的buf数量配置给buffer管理模块，创建buffer管理句柄。 */
    buffer_manager_init(__this->buffer_hdl, buf, len, __this->info.buf_num);

    /* 获取显存buf的分块行数，这里通过LCD驱动配置的显存buf长度反算，因为显存分块高度并没有被记录，但会反应在显存buf长度上。 */
    int lines = len / __this->info.buf_num / __this->info.stride;
    printf("@@@ lines: %d\n", lines);

    /* 将要绘制的宽、高、分块高度用其它变量缓存，非必须，只是为方便使用。 */
    int width  = draw.width;
    int height = draw.height;
    int block_height = lines;

    /* 计算分块数量，如果分块高度不能被全屏高度整除，最后一块不足一个分块高度，也要算做一个分块。 */
    int block_number = (height % block_height) ? ((height / block_height) + 1) : (height / block_height);
    printf("@@@ block_number: %d\n", block_number);

    /* 设置LCD推屏区域。注意，这里是start和end的参数，而且刷新屏幕前设置一次要刷新的整个区域即可。 */
    lcd_set_draw_area(draw.left, (draw.width - 1), draw.top, (draw.height - 1));

    int draw_height;
    struct rect draw_rect = {0};
    for (int i = 0; i < block_number; i++) {

        /* 计算当前需要合成的块高度，考虑最后一个块可能不足一个分块的高度，因此每次合成前需计算本次要合成的块高度。 */
        if ((i + 1) * block_height < height) {
            draw_height = block_height;
        } else {
            /* 最后一个分块时，一般不满足预设高度，需计算实际高度才能推屏 */
            draw_height = height - (i * block_height);
        }

        /* 本次循环需要GPU合成的区域 */
        draw_rect.left	 = draw.left + 0;
        draw_rect.top	 = draw.top + i * block_height;
        draw_rect.width  = width;
        draw_rect.height = draw_height;
        printf("@@@ cur rect[%d, %d, %d, %d]\n", draw_rect.left, draw_rect.top, draw_rect.width, draw_rect.height);

        /* 获取空闲的显存buf */
        u8 *idle_buf = NULL;
        u32 time_msec1 = jiffies_msec();
        do {
            /* 尝试获取空闲的显存buf */
            idle_buf = get_idle_buffer_to_lock(__this->buffer_hdl);
            u32 time_msec2 = jiffies_msec();
            if (time_msec2 - time_msec1 > 500) {
                /* 如果超过500ms没获取到空闲buf，则打印buffer管理句柄管理的所有buf状态 */
                buffer_manager_status_dump(__this->buffer_hdl);
            }
        } while (!idle_buf);
        memset(idle_buf, 0x00, draw_rect.width * draw_rect.height * 2);

        /* 设置GPU任务链输出buf和绘制区域 */
        jlgpu_set_task_list_out_buf(head, idle_buf, &draw_rect, 0, 0);
        /* jlgpu_dump_all_task(head, __func__, __LINE__); */
        /* 启动GPU合成 */
        jlgpu_task_list_run(head);

        /* 设置GPU输出的buf为等待状态，等待DBI模块推屏，到这里buf已经是GPU合成完毕的输出了。 */
        set_buffer_pend(__this->buffer_hdl, idle_buf);

        /* 启动LCD推屏 */
        lcd_flush_buffer_start(__this->buffer_hdl, idle_buf, &draw_rect, i);
    }
}

void gpu_demo_task(void *p)
{
    /* 初始化LCD设备 */
    gpu_port_demo_lcd_init();

    /* 初始化GPU任务管理模块 */
    gpu_port_demo_port_init();

    /* 创建GPU任务链，并设置GPU任务链输出格式为RGB565 */
    gpu_port_demo_task_init(GPU_FORMAT_RGB565);

    /* 创建GPU背景任务 */
    struct rect bg_rect;
    bg_rect.left   = 0;
    bg_rect.top    = 0;
    bg_rect.width  = __this->info.width;	// 背景任务绘制区域为全屏
    bg_rect.height = __this->info.height;
    gpu_port_demo_create_bg_task(bg_rect, GPU_DEMO_BACKGROUND);

    /* 创建GPU demo任务 */
    gpu_port_demo_create_task(__this->gpu_task_head);

    /* 启动GPU合成并输出到LCD屏幕，绘制范围也为全屏 */
    gpu_port_demo_start_draw(bg_rect, __this->gpu_task_head);

    /* 释放GPU任务链和任务链中的GPU任务 */
    gpu_port_demo_task_free();

    /* GPU demo 系统任务空转 */
    os_time_dly(100);
    while (1) {
        os_time_dly(100);
    }
}

/* 创建GPU demo系统任务 */
int gpu_demo_task_main(void)
{
    printf(">>>>>>>>> %s()", __func__);
    int err;
    err = os_task_create(gpu_demo_task, NULL, 10, 1024 * 5, 512, "gpu_demo_task");
    if (err != OS_NO_ERR) {
        printf("gpu task creat fail %x\n", err);
    }
    return 0;
}
static u8 gpu_idle_query(void)
{
    return 0;
}
REGISTER_LP_TARGET(gpu_demo_lp_target) = {
    .name = "gpu_demo",
    .is_idle =  gpu_idle_query,
};


#endif


