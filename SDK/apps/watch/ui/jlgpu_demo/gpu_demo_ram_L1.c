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

#if defined GPU_DEMO_RAM_L1_ENABLE && GPU_DEMO_RAM_L1_ENABLE

struct gpu_port_demo_priv {
    struct lcd_interface *lcd;
    struct lcd_info info;

    pJLGPUTaskHead_t gpu_task_head;
    uint8_t *buf1;
    uint8_t *buf2;
    /* uint8_t cur_buf; */
};

static struct gpu_port_demo_priv priv = {0};

#define __this		(&priv)

struct img_info_user {
    //图片基本参数
    int width;		//图片宽度
    int height;		//图片高度
    int left;
    int top;

    u8 img_data_from;	//图像数据的来源（ram/flash）
    u8 *data;		//存放图像数据的地址
    int format;		//图像的格式
    //(demo中目前支持的图片格式：GPU_FORMAT_RGB565, GPU_FORMAT_ARGB8565, GPU_FORMAT_L1)

    //旋转参数
    u8 rotate_en;	//旋转使能
    short rotate_cx;	//图片旋转中心(相对于图片的坐标，不是相对于屏幕)
    short rotate_cy;
    short rotate_dx;	//旋转后图片中心点的坐标（相对于屏幕）
    short rotate_dy;
    float rotate_angle;	//旋转的角度

    //缩放参数
    u8 scale_en;
    float ratio_w;		//图片宽的缩放倍数
    float ratio_h;		//图片高的缩放倍数

    //带有颜色表的图片格式的参数
    s16 has_clut;		//是否有颜色表。0：不带颜色表，1：从flash中获取颜色表，2直接传参颜色表的地址
    s16 clut_format;	//颜色表的格式(ARGB8888, ARGB8565, RGB888, RGB565),当前demo仅支持ARGB8888
    u8 *clut_tab;		//颜色表的地址
    u32 clut_tab_acolor;//颜色表的颜色

};

typedef enum {
    IMAGE_FROM_RAM = 0,
    IMAGE_FROM_FLASH,
} IMAGE_FROM;

static void *gpu_port_demo_malloc(int size, u32 ram_type, u32 module_type)
{
    void *buf = (void *)malloc(size);

    return buf;
}

static void gpu_port_demo_free(void *buf, u32 ram_type, u32 module_type)
{
    free(buf);
}

static void gpu_port_demo_lcd_init(void)
{
    __this->lcd = lcd_get_hdl();
    ASSERT(__this->lcd);
    ASSERT(__this->lcd->init);
    ASSERT(__this->lcd->get_screen_info);
    ASSERT(__this->lcd->buffer_malloc);
    ASSERT(__this->lcd->buffer_free);
    ASSERT(__this->lcd->draw);
    ASSERT(__this->lcd->set_draw_area);
    ASSERT(__this->lcd->backlight_ctrl);

    if (__this->lcd->init) {
        extern const struct ui_devices_cfg ui_cfg_data;
        __this->lcd->init((void *)&ui_cfg_data);
    }

    if (__this->lcd->get_screen_info) {
        __this->lcd->get_screen_info(&__this->info);
    }

    if (__this->lcd->clear_screen) {
        __this->lcd->clear_screen(0x000000, 0, __this->info.width - 1, 0, __this->info.height - 1);
    }


    if (__this->lcd->backlight_ctrl) {
        __this->lcd->backlight_ctrl(100);
    }

    mem_var_init(3 * 1024, false);
    ui_resfile_info_init(gpu_port_demo_malloc, gpu_port_demo_free);		// UI 资源管理
}

//初始化gpu_port模块接口
static void gpu_port_demo_port_init(void)
{
    jlgpu_module_init(gpu_port_demo_malloc, gpu_port_demo_free, __this->info.width, __this->info.height);
}

void gpu_port_demo_task_init(int gpu_out_format)
{
    //创建任务链
    __this->gpu_task_head = jlgpu_create_task_list_head();
    if (!__this->gpu_task_head) {
        printf("<gpu_port_demo> gpu create task list head fail!!! ");
        return;
    }

    //设置任务链输出的格式
    jlgpu_set_task_list_out_format(__this->gpu_task_head, gpu_out_format);

}

void gpu_port_demo_task_free(void)
{
    if (__this->gpu_task_head) {
        //删除任务链
        jlgpu_delete_task_list_head(__this->gpu_task_head);
        __this->gpu_task_head = NULL;
    }
}

/* L1格式的图像 */
static u8 icon_36x36_L1[188] ALIGNED(4) = {
    0x00, 0x00, 0x00, 0x00, 	//Color of index 0
    0xff, 0xff, 0x00, 0xe4, 	//Color of index 1

    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x00, 0x00,
    0x00, 0x03, 0xfc, 0x00, 0x00,
    0x00, 0x07, 0xff, 0x00, 0x00,
    0x00, 0x1f, 0xff, 0x00, 0x00,
    0x00, 0x1f, 0x9f, 0x80, 0x00,
    0x00, 0x1e, 0x07, 0x80, 0x00,
    0x00, 0x3c, 0x03, 0xc0, 0x00,
    0x00, 0x3c, 0x03, 0xc0, 0x00,
    0x00, 0x3c, 0x03, 0xc0, 0x00,
    0x00, 0x3c, 0x03, 0xc0, 0x00,
    0x00, 0x3c, 0x03, 0xc0, 0x00,
    0x00, 0x3c, 0x03, 0xc0, 0x00,
    0x00, 0x3c, 0x03, 0xc0, 0x00,
    0x00, 0x3c, 0x03, 0xc0, 0x00,
    0x00, 0xff, 0xff, 0xf8, 0x00,
    0x03, 0xff, 0xff, 0xf8, 0x00,
    0x03, 0xff, 0xff, 0xfc, 0x00,
    0x03, 0xff, 0xff, 0xfc, 0x00,
    0x03, 0xff, 0xff, 0xfc, 0x00,
    0x03, 0xff, 0xff, 0xfc, 0x00,
    0x03, 0xff, 0xdf, 0xfc, 0x00,
    0x03, 0xff, 0x0f, 0xfc, 0x00,
    0x03, 0xf8, 0x01, 0xfc, 0x00,
    0x03, 0xf8, 0x01, 0xfc, 0x00,
    0x03, 0xff, 0x0f, 0xfc, 0x00,
    0x03, 0xff, 0xbf, 0xfc, 0x00,
    0x03, 0xff, 0xff, 0xfc, 0x00,
    0x03, 0xff, 0xff, 0xfc, 0x00,
    0x03, 0xff, 0xff, 0xfc, 0x00,
    0x03, 0xff, 0xff, 0xfc, 0x00,
    0x03, 0xff, 0xff, 0xfc, 0x00,
    0x00, 0xff, 0xff, 0xf0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
};

//创建绘制背景颜色任务
static void gpu_port_demo_create_bg_task(struct rect draw_rect, u32 color)
{
    u8 a = (color >> 24) & 0xff;
    u8 r = (color >> 16) & 0xff;
    u8 g = (color >> 8) & 0xff;
    u8 b = (color) & 0xff;

    //创建gpu绘制任务
    jlgpu_task_fill_param_init(0, 0, a, r, g, b);
    memcpy(&task_param.draw, &draw_rect, sizeof(struct rect));
    jlgpu_get_win_rect(&task_param.area);	// 限制区域是整个GPU窗口区域

    printf("<%s>----param->head:0x%p\n", __func__, __this->gpu_task_head);

    jlgpu_create_task(__this->gpu_task_head, &task_param);
}

//创建绘制图片的任务
static void gpu_port_demo_create_img_task(struct img_info_user *img)
{
    //创建gpu绘制任务
    jlgpu_task_texture_param_init(0, 0);
    task_param.draw.width = img->width;
    task_param.draw.height = img->height;
    task_param.draw.left = img->left;
    task_param.draw.top = img->top;
    jlgpu_get_win_rect(&task_param.area);	// 限制区域是整个GPU窗口区域
    task_param.task_type = GPU_TASK_IMAGE;
    task_param.texture.data = img->data;
    task_param.format = img->format;
    //配置图片的大小
    task_param.image.width = img->width;
    task_param.image.height = img->height;
    //配置颜色表
    task_param.clut_tab = img->clut_tab;
    task_param.alpha = (img->clut_tab_acolor >> 24) & 0xff;
    task_param.red   = (img->clut_tab_acolor >> 16) & 0xff;
    task_param.green = (img->clut_tab_acolor >> 8) & 0xff;
    task_param.blue  = (img->clut_tab_acolor) & 0xff;
    if (!task_param.clut_tab) {
        printf("malloc clut_tab buf faild!\n");
        ASSERT(task_param.clut_tab != NULL, "Insufficient memory!");
    }
    task_param.clut_format = img->clut_format;
    task_param.alpha = 0x00;	// L1格式没有透明度
    task_param.has_clut = img->has_clut;


    //配置图片旋转参数
    if (img->rotate_en) {
        task_param.rotate_en = 1;
        task_param.texture.tran.rotate_cx = img->rotate_cx;	//图片旋转中心(这里的坐标是相对于图片而不是相对于屏幕)
        task_param.texture.tran.rotate_cy = img->rotate_cy;
        task_param.texture.tran.rotate_dx = img->rotate_dx;	//旋转之后中心点的坐标
        task_param.texture.tran.rotate_dy = img->rotate_dy;
        task_param.texture.tran.rotate_angle = img->rotate_angle;
    }

    //配置图片缩放参数
    if (img->scale_en) {
        task_param.scale_en = 1;
        task_param.texture.tran.ratio_w = img->ratio_w;		//宽的缩放倍数
        task_param.texture.tran.ratio_h = img->ratio_h;		//高的缩放倍数
    }
    jlgpu_create_task(__this->gpu_task_head, &task_param);
    /* taskp = jlgpu_create_task(__this->gpu_task_head, &task_param); */
    /* void *task_addr = jlgpu_get_task_addr(taskp); */
    /* extern void gpu_texture_taskq_debug(void *task_q); */
    /* gpu_texture_taskq_debug(task_addr); */
}

void gpu_port_demo_draw_image(struct img_info_user *img, u32 background)
{
    //首先判断图片的基本参数配置是否合法
    if (!img->width || !img->height || !img->data) {
        ASSERT(0, "The params of img is error!!!");
    }

    //输出配置
    int width = __this->info.width;
    int height = __this->info.height;
    int out_format = GPU_FORMAT_RGB565;
    int out_bpp = gpu_get_format_bpp(out_format) / 8;
    int out_stride = width * out_bpp;
    int block_height = 16;		//分块输出到屏幕上，每块的大小（width * block_height）

    //申请输出的缓存空间
    __this->buf1 = (uint8_t *)zalloc(block_height * out_stride);
    /* __this->buf2 = (uint8_t *)zalloc((height % block_height) * out_stride); */

    struct rect draw_task_rect = {0};
    //创建绘制背景颜色的任务
    if (background) {
        draw_task_rect.left = 0;
        draw_task_rect.top = 0;
        draw_task_rect.width = __this->info.width;
        draw_task_rect.height = __this->info.height;
        gpu_port_demo_create_bg_task(draw_task_rect, background);
    }

    //创建绘制图片的任务
    gpu_port_demo_create_img_task(img);

    //设置输出buf并且显示到屏幕上
    int block_num = 0;
    block_num = (height % block_height) ? (height / block_height + 1) : (height / block_height);
    //设置推屏区域
    lcd_set_draw_area(0, width - 1, 0, height - 1);

    struct rect draw_rect = {0};
    int draw_height;
    for (int i = 0; i < block_num; i++) {
        if ((i + 1) * block_height < height) {
            draw_height = block_height;
        } else {
            draw_height = height - (i * block_height);
        }

        draw_rect.left	 = 0;
        draw_rect.top	 = i * block_height;
        draw_rect.width  = width;
        draw_rect.height = draw_height;

        memset(__this->buf1, 0, draw_rect.width * draw_rect.height * 2);

        jlgpu_set_task_list_out_buf(__this->gpu_task_head,  __this->buf1, &draw_rect, 0, 0);
        jlgpu_task_list_run(__this->gpu_task_head);
        /* jlgpu_dump_task_head(__this->gpu_task_head); */


        if (!i) {
            /* 开始推屏时使用 kistart */
            lcd_draw_kistart(__this->buf1, draw_rect.left, draw_rect.left + draw_rect.width - 1, draw_rect.top, draw_rect.top + draw_rect.height - 1);
            lcd_wait_busy();
        } else {
            /* 之后推屏时使用 continue 续传 */
            lcd_draw_continue(__this->buf1, draw_rect.left, draw_rect.left + draw_rect.width - 1, draw_rect.top, draw_rect.top + draw_rect.height - 1);
            lcd_wait_busy();
        }
    }

    //将申请的内存进行释放
    /* if(img->clut_tab){ */
    /* free(img->clut_tab); */
    /* img->clut_tab = NULL; */
    /* } */
    if (__this->buf1) {
        free(__this->buf1);
        __this->buf1 = NULL;
    }
    if (__this->buf2) {
        free(__this->buf2);
        __this->buf2 = NULL;
    }
}


void gpu_demo_L1(void *p)
{
    gpu_port_demo_lcd_init();		//初始化lcd
    gpu_port_demo_port_init();		//初始化gpu_port接口
    struct img_info_user img_user = {0};

    img_user.clut_tab = (u8 *)zalloc(gpu_format_to_lut_size(GPU_FORMAT_L1, GPU_CLUT_FORMAT_ARGB8888));
    img_user.clut_tab[4] = icon_36x36_L1[4];
    img_user.clut_tab[5] = icon_36x36_L1[5];
    img_user.clut_tab[6] = icon_36x36_L1[6];
    img_user.clut_tab[7] = icon_36x36_L1[7];
    img_user.width = 36;
    img_user.height = 36;
    img_user.left = 100;
    img_user.top = 100;
    img_user.img_data_from = IMAGE_FROM_RAM;
    img_user.data = icon_36x36_L1 + 8;
    img_user.format = GPU_FORMAT_L1;
    img_user.has_clut = CLUT_TAB_FROM_RAM;
    img_user.clut_format = GPU_CLUT_FORMAT_ARGB8888;
    img_user.clut_tab_acolor = 0xe400ffff;
    img_user.rotate_en = 1;
    img_user.rotate_cx = 18;	//图片旋转中心(这里的坐标是相对于图片而不是相对于屏幕)
    img_user.rotate_cy = 18;
    img_user.rotate_dx = 118;	//旋转之后中心点的坐标
    img_user.rotate_dy = 118;
    img_user.rotate_angle = 30;
    img_user.scale_en = 1;
    img_user.ratio_w = 2;
    img_user.ratio_h = 2;

    gpu_port_demo_task_init(GPU_FORMAT_RGB565);
    gpu_port_demo_draw_image(&img_user, 0xff00ff00);
    gpu_port_demo_task_free();	//任务链用完之后删除任务链，同时创建的任务也会清除

    if (img_user.clut_tab) {
        free(img_user.clut_tab);
        img_user.clut_tab = NULL;
    }

    while (1) {
        os_time_dly(100);
    }
}


int gpu_demo_L1_task(void)
{
    printf(">>>>>>>>> %s()", __func__);
    int err;
    err = os_task_create(gpu_demo_L1, NULL, 10, 1024 * 5, 512, "gpu_demo_L1");
    if (err != OS_NO_ERR) {
        printf("gpu task creat fail %x\n", err);
    }
    return 0;
}

#endif
