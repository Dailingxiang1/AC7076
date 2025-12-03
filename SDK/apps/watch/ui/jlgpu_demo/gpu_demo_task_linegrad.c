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


#if defined GPU_DEMO_TASK_LINEGRAD && GPU_DEMO_TASK_LINEGRAD

/* 创建GPU线性渐变任务 */
void gpu_demo_create_linegrad_task(pJLGPUTaskHead_t head)
{
    /* 渐变颜色值，ARGB8888 */
    u32 color0 = TO_ARGB8888(0xff, 0xff, 0x00, 0x00);
    u32 color1 = TO_ARGB8888(0xff, 0x00, 0xff, 0x00);
    u32 color2 = TO_ARGB8888(0xff, 0x00, 0x00, 0xff);

    /* 申请颜色表buf，每次配置都需要申请，GPU任务用完会释放 */
    u32 *clut_data = malloc(sizeof(u32) * 3);

    /* 配置渐变颜色值 */
    clut_data[0] = color0;
    clut_data[1] = color1;
    clut_data[2] = color2;

    /* 渐变颜色表的颜色格式 */
    int clut_format = GPU_CLUT_FORMAT_ARGB8888;

    /* 创建渐变任务的参数结构体，这里任务ID和控件ID为任意值 */
    jlgpu_task_linegrad_param_init(9, 9);

    /* 渐变区域大小 */
    int image_w = 100;
    int image_h = 100;

    /* 获取全屏区域，作为area区域 */
    jlgpu_get_win_rect(&task_param.area);

    /* 配置绘制区域，位置为屏幕中间 */
    task_param.draw.left = (task_param.area.width - image_w) / 2;
    task_param.draw.top = (task_param.area.height - image_h) / 2;
    task_param.draw.width = image_w;
    task_param.draw.height = image_h;

    /* 配置颜色表 */
    task_param.has_clut = CLUT_TAB_FROM_RAM;	// 颜色表来自RAM
    task_param.clut_format = clut_format;		// 颜色表颜色格式
    task_param.clut_tab = (u8 *)clut_data;		// 颜色表数据地址

    /* 配置渐变点数和渐变模式，参考gpu_port.h注释 */
    task_param.linegrad.lut_lvl = 1;		// 渐变色点个数（2^(lut_lvl)+1）个点
    task_param.linegrad.spread_mode = 1;	// 渐变模式

    /* 配置渐变点坐标，参考gpu_port.h注释 */
    task_param.linegrad.x0 = 10;
    task_param.linegrad.y0 = 0;
    task_param.linegrad.x1 = 30;
    task_param.linegrad.y1 = 0;

    /* 注意：这个foramt需要给底层用于颜色表计算，需配置为GPU_FORMAT_A8 */
    task_param.format = GPU_FORMAT_A8;

    task_param.image.width = image_w;
    task_param.image.height = image_h;

    /* 创建GPU任务 */
    jlgpu_update_task_by_id(head, task_param.task_id, task_param.element_id, &task_param);
}

#endif




