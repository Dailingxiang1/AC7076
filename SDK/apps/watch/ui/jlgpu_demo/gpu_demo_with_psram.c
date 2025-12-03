/**
 * @file gpu_demo_out_psram.c
 *
 * @brief GPU与PSRAM联动示例代码
 *
 * 1. 本示例包含两个demo:
 *  	demo 1为创建GPU任务链，并将合成内容输出到PSRAM;
 * 		demo 2为将demo 1输出的内容作为图片，将它进行旋转缩放后合成并输出到屏幕。
 * 2. 阅读本示例，从 gpu_demo_with_psram 函数开始阅读
 * 3. 本示例中，没有对PSRAM缓存buf进行释放，实际使用中需要开发者自行合理使用PSRAM内存
 *
 * @author
 *
 * @version V1.0.0
 *
 * @date 2025-02-28
 */

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

#include "ui_core.h"
#include "ui_measure.h"

#include "ui/lcd/lcd_drive.h"
#include "res/mem_var.h"
#include "football.h"

#include "gpu_demo.h"

#if (defined GPU_DEMO_WITH_PSRAM && GPU_DEMO_WITH_PSRAM && TCFG_PSRAM_DEV_ENABLE)

/* 任务ID定义 */
#define GPU_DEMO_TASK_ID	1

/* 控件DI定义 */
#define GPU_DEMO_ELEMENT	1

/* 填充的宽度 */
#define GPU_DEMO_FILL_W		100

/* 填充的高度 */
#define GPU_DEMO_FILL_H		100

/* GPU输出到PSRAM的数据格式，当前一个GPU任务链输出作为后一个任务的纹理输出时，等于纹理任务的纹理格式 */
#define GPU_DEMO_FILL_FMT	GPU_OUT_FORMAT_RGB565

/* 透明度定义，0~100 */
#define GPU_FILL_ALPHA		50

/* 颜色值定义，RGB888 */
#define GPU_FILL_COLOR		0x00ff00



/* DEMO 1. 创建一条任务链，配置一个填充任务，并将该任务链的合成结果输出到PSRAM. */
void gpu_out_to_psram(void *psram_buf, int w, int h)
{
    /* 创建GPU任务链 */
    pJLGPUTaskHead_t head = jlgpu_create_task_list_head();

    /* 设置GPU任务链输出格式 */
    jlgpu_set_task_list_out_format(head, GPU_DEMO_FILL_FMT);

    /* 填充颜色ARGB8888 */
    u8 a = GPU_FILL_ALPHA;
    u8 r = (GPU_FILL_COLOR >> 16) & 0xff;
    u8 g = (GPU_FILL_COLOR >> 8) & 0xff;
    u8 b = (GPU_FILL_COLOR) & 0xff;

    /* 创建task_param参数结构体，并设置任务ID，控件ID，填充颜色 */
    jlgpu_task_fill_param_init(GPU_DEMO_TASK_ID, GPU_DEMO_ELEMENT, a, r, g, b);

    /* 配置填充位置区域 */
    task_param.draw.left   = 0;
    task_param.draw.top    = 0;
    task_param.draw.width  = w;
    task_param.draw.height = h;

    /* 配置填充的父控件区域为整个屏幕 */
    jlgpu_get_win_rect(&task_param.area);

    /* 创建GPU任务，并添加到head任务链中 */
    jlgpu_update_task_by_id(head, GPU_DEMO_TASK_ID, GPU_DEMO_ELEMENT, &task_param);

    /* 设置GPU任务链输出buf */
    struct rect out_rect;
    out_rect.left = 0;
    out_rect.top = 0;
    out_rect.width = w;
    out_rect.height = h;
    jlgpu_set_task_list_out_buf(head, psram_buf, &out_rect, 0, 0);

    /* 启动GPU任务链合成 */
    jlgpu_task_list_run(head);

    /* 删除GPU任务链 */
    jlgpu_delete_task_list_head(head);
}

/* DEMO 2. 调用DEMO 1合成一个纯色图片并放到PSRAM，然后把这张图片作为输出，并合成输出到屏幕 */
void gpu_demo_with_psram(pJLGPUTaskHead_t head)
{
    /* 申请PSRAM缓存buf，大小根据DEMO 1的图片输出格式配置 */
    int psram_buf_size = GPU_DEMO_FILL_W * GPU_DEMO_FILL_H * 2;
    void *psram_buf_addr = malloc_psram(psram_buf_size);
    memset(psram_buf_addr, 0x00, psram_buf_size); // 清空buf，否则GPU合成时会把原先乱的数据当背景数据

    /* 调用DEMO 1, 创建GPU填充任务，并启动合成输出到PSRAM */
    gpu_out_to_psram(psram_buf_addr, GPU_DEMO_FILL_W, GPU_DEMO_FILL_W);

    /* 创建GPU图片任务 */
    jlgpu_task_texture_param_init(GPU_DEMO_TASK_ID, GPU_DEMO_ELEMENT);

    /* 配置图片格式和宽高 */
    task_param.format = GPU_FORMAT_RGB565;
    task_param.image.width = GPU_DEMO_FILL_W;
    task_param.image.height = GPU_DEMO_FILL_H;

    /* 配置图片绘制区域和数据地址 */
    task_param.draw.left = 100;
    task_param.draw.top = 100;
    task_param.draw.width = task_param.image.width;
    task_param.draw.height = task_param.image.height;
    jlgpu_get_win_rect(&task_param.area);
    task_param.texture.not_compress = true;
    task_param.texture.data = psram_buf_addr;

    /* 设置图片高放大两倍 */
    task_param.scale_en = true;
    task_param.texture.tran.ratio_w = 1.0;
    task_param.texture.tran.ratio_h = 2.0;

    /* 设置图片旋转45度 */
    task_param.rotate_en = true;
    task_param.texture.tran.rotate_cx = GPU_DEMO_FILL_W / 2;
    task_param.texture.tran.rotate_cy = GPU_DEMO_FILL_H / 2;
    task_param.texture.tran.rotate_dx = (320 - GPU_DEMO_FILL_W) / 2;
    task_param.texture.tran.rotate_dy = (385 - GPU_DEMO_FILL_H) / 2;
    task_param.texture.tran.rotate_angle = 45;

    /* 把图片任务添加到demo的任务链中 */
    jlgpu_update_task_by_id(head, task_param.task_id, task_param.element_id, &task_param);
}



#endif




