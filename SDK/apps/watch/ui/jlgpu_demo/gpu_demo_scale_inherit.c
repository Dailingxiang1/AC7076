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

#include "gpu_demo.h"

#if defined GPU_DEMO_SCALE_INHERIT_ENABLE && GPU_DEMO_SCALE_INHERIT_ENABLE

struct gpu_port_demo_priv {
    struct lcd_interface *lcd;
    struct lcd_info info;

    pJLGPUTaskHead_t gpu_task_head;
    uint8_t *buf1;
    uint8_t *buf2;
    /* uint8_t cur_buf; */
};

struct img_info_user {
    //图片基本参数
    int width;		//图片宽度
    int height;		//图片高度
    int left;
    int top;

    u8 img_data_from;	//图像数据的来源（ram/flash）
    u8 *data;		//存放图像数据的地址
    int format;		//图像的格式
    int len;
    int compress_mode;		//压缩方式
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

static struct gpu_port_demo_priv priv = {0};

#define __this		(&priv)

struct scale_inherit_info_user {
    char *path;			//资源文件的路径
    u32 image_num;		//足球表面图片的数量
    u32 *image_id_tab;	//图片的id
    float x_diff;
    float y_diff;
};

typedef enum {
    IMAGE_FROM_RAM = 0,
    IMAGE_FROM_FLASH,
} IMAGE_FROM;


#define  PAGE0_e060_H4_00                       0x000001 //  images\h4_00.png
#define  PAGE0_e0f6_M4_00                       0x000002 //  images\m4_00.png
#define  PAGE0_e382_S4_00                       0x000003 //  images\s4_00.png
#define  PAGE0_5fab_FACE_BG_01                  0x000004 //  images\face_bg_01.png

u32 scale_inherit_img_id_tab[] = {
    PAGE0_e060_H4_00,
    PAGE0_e0f6_M4_00,
    PAGE0_e382_S4_00,
    PAGE0_5fab_FACE_BG_01,
};

int find_index_by_id(u32 image_id)
{
    int i;
    for (i = 0; i < sizeof(scale_inherit_img_id_tab) / sizeof(scale_inherit_img_id_tab[0]); i++) {
        if (scale_inherit_img_id_tab[i] == image_id) {
            return i;
        }
    }
    return -1;
}

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
static pJLGPUTaskUnit_t gpu_port_demo_create_img_task(struct img_info_user *img, gpu_matrix_t *matrix)
{
    //创建gpu绘制任务
    jlgpu_task_texture_param_init(0, 0);
    task_param.draw.width = img->width;
    task_param.draw.height = img->height;
    task_param.draw.left = img->left;
    task_param.draw.top = img->top;
    jlgpu_get_win_rect(&task_param.area);	// 限制区域是整个GPU窗口区域
    task_param.texture.data = img->data;
    task_param.format = img->format;
    //配置图片的大小
    task_param.image.width = img->width;
    task_param.image.height = img->height;
    task_param.image.len = img->len;
    task_param.image.compress = img->compress_mode;
    task_param.matrix = matrix;

    if (task_param.format == GPU_FORMAT_L1) {
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

    }

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
    pJLGPUTaskUnit_t taskp = jlgpu_create_task(__this->gpu_task_head, &task_param);
    /* taskp = jlgpu_create_task(__this->gpu_task_head, &task_param); */
    /* void *task_addr = jlgpu_get_task_addr(taskp); */
    /* extern void gpu_texture_taskq_debug(void *task_q); */
    /* gpu_texture_taskq_debug(task_addr); */
    return taskp;
}

static u32 get_image_addr_in_flash(FILE *file)
{
    struct vfs_attr attr = {0};
    fget_attrs(file, &attr);
    u32 file_base_addr = attr.sclust;		//文件在分区中的地址
    printf("<%s>--------file_base_addr:0x%x\n", __func__, file_base_addr);
    file_base_addr += TCFG_MODE_INSERT_FLASH_BASE;

    file_base_addr = sdfile_flash_addr2cpu_addr(file_base_addr);
    printf("<%s>--------file_base_addr:0x%x\n", __func__, file_base_addr);
    return file_base_addr;
}

void jlgpu_create_image_task(pJLGPUTaskHead_t head, struct ui_image_attrs *image_attr, gpu_matrix_t *matrix)
{
    JLGPUTaskParam_t task_param = {0};
    task_param.task_type = GPU_TASK_IMAGE;

    //基础指令参数
    gpu_boundbox_t bbox;
    bbox.minx = 0;
    bbox.miny = 0;
    bbox.maxx = image_attr->width;
    bbox.maxy = image_attr->height;
    gpu_matrix_get_boundbox(matrix, &bbox, &bbox);
    task_param.draw.left   = bbox.minx;
    task_param.draw.width  = bbox.maxx - bbox.minx;
    task_param.draw.top    = bbox.miny;
    task_param.draw.height = bbox.maxy - bbox.minx;
    task_param.global_alpha = 128;
    task_param.blend_mode = 1;
    task_param.premult = 1;

    //纹理指令
    task_param.format = image_attr->gpu_format;
    task_param.clut_format = image_attr->clut_format;
    task_param.image.format = image_attr->format;
    task_param.image.width = image_attr->width;
    task_param.image.height = image_attr->height;
    task_param.image.compress = image_attr->compress;
    task_param.image.has_clut = image_attr->has_clut;
    task_param.has_clut = image_attr->has_clut;
    task_param.image.len = image_attr->len;
    task_param.texture.data = image_attr->data;
    task_param.texture.mmu_tab_base = image_attr->tab;

    //透视指令
    task_param.perspective_en = 1;
    task_param.texture.crop.left   = 0;
    task_param.texture.crop.width  = image_attr->width;
    task_param.texture.crop.top    = 0;
    task_param.texture.crop.height = image_attr->height;

    task_param.matrix = matrix;

    pJLGPUTaskUnit_t taskp;

    taskp = jlgpu_create_task(head, &task_param);
    taskp->info.image_width = image_attr->width;
    taskp->info.image_height = image_attr->height;
}


static void gpu_port_demo_create_scale_inherit_task(struct scale_inherit_info_user *scale_inherit_user)
{
    struct ui_image_attrs *image;
    int image_num;

    image = (struct ui_image_attrs *)malloc(scale_inherit_user->image_num * sizeof(struct ui_image_attrs));
    image_num = scale_inherit_user->image_num;
    char *img_path = scale_inherit_user->path;

    //打开资源文件
    UI_RESFILE *fp =  res_fopen(img_path, "r");
    if (!fp) {
        printf("%s fp is null", __func__);
        ASSERT(0);
    }
    //获取文件的flash table
    struct flash_file_info file_info;
    ui_res_flash_info_get(&file_info, img_path, "res", true);

    //获取图片信息
    for (int i = 0; i < image_num; i++) {
        JLGPUTaskParam_t task_param = {0};
        printf("id:%x\n", scale_inherit_img_id_tab[i]);
        jlui_res_get_image_info(0, (scale_inherit_img_id_tab[i] & 0xffff), fp, &task_param.image, &task_param.info);
        ui_res_get_image_flash_info(&task_param.info, &file_info, &task_param.image);
        printf("<%s>--------format:%d, offset:0x%x, size:%d * %d\n", __func__, task_param.image.format, task_param.image.offset, task_param.image.width, task_param.image.height);

        image[i].width = task_param.image.width;
        image[i].height = task_param.image.height;
        image[i].format = task_param.image.format;
        image[i].clut_format = task_param.image.clut_format;
        image[i].compress = task_param.image.compress;
        image[i].has_clut = task_param.image.has_clut;
        image[i].tab = (u8 *)task_param.info.tab;
        image[i].data = (u8 *)task_param.info.offset;
        image[i].len = task_param.image.len;
        image[i].gpu_format = res_format_to_gpu(task_param.image.format);
        printf("<%s>--------format:%d, offset:0x%x\n", __func__, task_param.image.format, task_param.info.offset);
    }

    int index = -1;
    // 表盘背景
    pJLGPUTaskUnit_t taskp;
    index = find_index_by_id(PAGE0_5fab_FACE_BG_01);
    if (index != -1) {
        struct img_info_user img_user = {0};
        img_user.width = image[index].width;
        img_user.height = image[index].height;
        img_user.left = 0;
        img_user.top = 0;
        img_user.img_data_from = IMAGE_FROM_FLASH;
        img_user.format = image[index].format;
        img_user.compress_mode = image[index].compress;
        img_user.data = ((u32 *)image[index].tab)[0] + image[index].data;
        img_user.len = image[index].len;

        img_user.scale_en = 1;
        img_user.ratio_w = 0.8;
        img_user.ratio_h = 0.8;

        taskp = gpu_port_demo_create_img_task(&img_user, NULL);
    }
    //时针
    index = find_index_by_id(PAGE0_e060_H4_00);
    if (index != -1) {
        struct img_info_user img_user = {0};
        img_user.width = image[index].width;
        img_user.height = image[index].height;
        img_user.left = 0;
        img_user.top = 0;
        img_user.img_data_from = IMAGE_FROM_FLASH;
        img_user.format = image[index].format;
        img_user.compress_mode = image[index].compress;
        img_user.data = ((u32 *)image[index].tab)[0] + image[index].data;
        img_user.len = image[index].len;
        img_user.rotate_en = 1;
        img_user.rotate_cx = 9;	//图片旋转中心(这里的坐标是相对于图片而不是相对于屏幕)
        img_user.rotate_cy = 81;
        img_user.rotate_dx = 160;	//旋转之后中心点的坐标
        img_user.rotate_dy = 193;
        img_user.rotate_angle = 30;
        gpu_port_demo_create_img_task(&img_user, taskp ? taskp->info.matrix : NULL);
    }
    //分针
    index = find_index_by_id(PAGE0_e0f6_M4_00);
    if (index != -1) {
        struct img_info_user img_user = {0};
        img_user.width = image[index].width;
        img_user.height = image[index].height;
        img_user.left = 0;
        img_user.top = 0;
        img_user.img_data_from = IMAGE_FROM_FLASH;
        img_user.format = image[index].format;
        img_user.compress_mode = image[index].compress;
        img_user.data = ((u32 *)image[index].tab)[0] + image[index].data;
        img_user.len = image[index].len;
        img_user.rotate_en = 1;
        img_user.rotate_cx = 7;	//图片旋转中心(这里的坐标是相对于图片而不是相对于屏幕)
        img_user.rotate_cy = 93;
        img_user.rotate_dx = 160;	//旋转之后中心点的坐标
        img_user.rotate_dy = 193;
        img_user.rotate_angle = 45;
        gpu_port_demo_create_img_task(&img_user, taskp ? taskp->info.matrix : NULL);
    }
    //秒针
    index = find_index_by_id(PAGE0_e382_S4_00);
    if (index != -1) {
        struct img_info_user img_user = {0};
        img_user.width = image[index].width;
        img_user.height = image[index].height;
        img_user.left = 0;
        img_user.top = 0;
        img_user.img_data_from = IMAGE_FROM_FLASH;
        img_user.format = image[index].format;
        img_user.compress_mode = image[index].compress;
        img_user.data = ((u32 *)image[index].tab)[0] + image[index].data;
        img_user.len = image[3].len;
        img_user.rotate_en = 1;
        img_user.rotate_cx = 7;	//图片旋转中心(这里的坐标是相对于图片而不是相对于屏幕)
        img_user.rotate_cy = 128;
        img_user.rotate_dx = 160;	//旋转之后中心点的坐标
        img_user.rotate_dy = 193;
        img_user.rotate_angle = 125;
        gpu_port_demo_create_img_task(&img_user, taskp ? taskp->info.matrix : NULL);
    }

    if (fp) {
        res_fclose(fp);
    }
    if (image) {
        free(image);
    }
}

void gpu_port_demo_draw_scale_inherit(struct scale_inherit_info_user *scale_inherit_user, u32 background)
{
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
        gpu_port_demo_create_bg_task(draw_task_rect, /*background*/100 << 24 | 0x000000);
    }

    //创建绘制图片的任务
    gpu_port_demo_create_scale_inherit_task(scale_inherit_user);

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

void gpu_demo_scale_inherit(void *p)
{
    gpu_port_demo_lcd_init();		//初始化lcd
    gpu_port_demo_port_init();		//初始化gpu_port接口
    struct scale_inherit_info_user scale_inherit_user = {0};

    scale_inherit_user.path = "storage/virfat_flash/C/ui/ui.res";
    scale_inherit_user.image_num = sizeof(scale_inherit_img_id_tab) / sizeof(scale_inherit_img_id_tab[0]);
    scale_inherit_user.image_id_tab = scale_inherit_img_id_tab;
    scale_inherit_user.x_diff = 0;
    scale_inherit_user.y_diff = 0;

    gpu_port_demo_task_init(GPU_FORMAT_RGB565);
    gpu_port_demo_draw_scale_inherit(&scale_inherit_user, 0x64000000);//ARGB8888格式 alpha范围 : 0~100
    gpu_port_demo_task_free();	//任务链用完之后删除任务链，同时创建的任务也会清除

    while (1) {
        os_time_dly(100);
    }
}


int gpu_demo_scale_inherit_task(void)
{
    printf(">>>>>>>>> %s()", __func__);
    int err;
    err = os_task_create(gpu_demo_scale_inherit, NULL, 10, 1024 * 5, 512, "gpu_demo_scale_inherit");
    if (err != OS_NO_ERR) {
        printf("gpu task creat fail %x\n", err);
    }
    return 0;
}

#endif


