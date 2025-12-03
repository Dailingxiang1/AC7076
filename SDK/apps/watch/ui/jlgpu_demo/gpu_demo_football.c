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

#if defined GPU_DEMO_FOOTBALL_ENABLE && GPU_DEMO_FOOTBALL_ENABLE

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

struct football_info_user {
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

#define  PAGE0_c186_MENU_29SET                  0x000001 //  images\menu_29set.png
#define  PAGE0_56cc_MENU_16SIRI                 0x000002 //  images\menu_16siri.png
#define  PAGE0_e4e2_MENU_1CLOCK                 0x000003 //  images\menu_1clock.png
#define  PAGE0_eb8d_MENU_4MUSIC                 0x000004 //  images\menu_4music.png
#define  PAGE0_2e24_MENU_14SLEEP                0x000005 //  images\menu_14sleep.png
#define  PAGE0_1ee8_MENU_26TIMER                0x000006 //  images\menu_26timer.png
#define  PAGE0_f432_MENU_33PRUNE                0x000007 //  images\menu_33Prune.png
#define  PAGE0_a990_MENU_24ALIPAY               0x000008 //  images\menu_24alipay.png
#define  PAGE0_f02e_MENU_6WEATHER               0x000009 //  images\menu_6weather.png
#define  PAGE0_3efa_MENU_22COMPASS              0x00000a //  images\menu_22compass.png
#define  PAGE0_bffe_MENU_23FIND_MY              0x00000b //  images\menu_23Find_my.png
#define  PAGE0_66f5_MENU_30RESTART              0x00000c //  images\menu_30restart.png
#define  PAGE0_ca8e_MENU_18CALENDAR             0x00000d //  images\menu_18calendar.png
#define  PAGE0_e1ba_MENU_20PRESSURE             0x00000e //  images\menu_20pressure.png
#define  PAGE0_e9bf_MENU_2TELEPHONE             0x00000f //  images\menu_2telephone.png
#define  PAGE0_bdf6_MENU_34PASSWORD             0x000010 //  images\menu_34password.png
#define  PAGE0_16f7_MENU_80ACTIVITY             0x000011 //  images\menu_80activity.png
#define  PAGE0_d12b_MENU_13BREATHING            0x000012 //  images\menu_13breathing.png
#define  PAGE0_79bc_MENU_21ALTIMETER            0x000013 //  images\menu_21altimeter.png
#define  PAGE0_651f_MENU_27STOPWATCH            0x000014 //  images\menu_27Stopwatch.png
#define  PAGE0_b338_MENU_10HEART_RATE           0x000015 //  images\menu_10heart_rate.png
#define  PAGE0_cfa8_MENU_17CALCULATOR           0x000016 //  images\menu_17calculator.png
#define  PAGE0_dede_MENU_28PHOTOGRAPH           0x000017 //  images\menu_28photograph.png
#define  PAGE0_7f29_MENU_3INFORMATION           0x000018 //  images\menu_3information.png
#define  PAGE0_f695_MENU_5ALARM_CLOCK           0x000019 //  images\menu_5alarm_clock.png
#define  PAGE0_cf1d_MENU_7BEDSIDE_LAMP          0x00001a //  images\menu_7bedside_lamp.png
#define  PAGE0_f6d4_MENU_11BLOOD_OXYGEN         0x00001b //  images\menu_11Blood_oxygen.png
#define  PAGE0_c5ef_MENU_9INDOOR_SPORTS         0x00001c //  images\menu_9indoor_sports.png
#define  PAGE0_35c8_MENU_32AMBIENT_NOISE        0x00001d //  images\menu_32ambient_noise.png
#define  PAGE0_6a9f_MENU_8OUTDOOR_SPORTS        0x00001e //  images\menu_8outdoor_sports.png
#define  PAGE0_0aa3_MENU_12BLOOD_PRESSURE       0x00001f //  images\menu_12blood_pressure.png
#define  PAGE0_32d6_MENU_15HEALTH_REMINDER      0x000020 //  images\menu_15Health_reminder.png
#define  PAGE0_e103_MENU_25SOUND_RECORDING      0x000021 //  images\menu_25sound_recording.png
#define  PAGE0_e0fb_MENU_19FEMALE_ASSISTANT     0x000022 //  images\menu_19Female_assistant.png
#define  PAGE0_76b1_MENU_31FACTORY_RECOVERY     0x000023 //  images\menu_31factory_recovery.png
#define  PAGE0_9f4c_MENU_35POWER_SAVINGMODE     0x000024 //  images\menu_35Power_savingmode.png
#define  PAGE0_fbab_MENU_36NIGHT_RUNNINGLAMP    0x000025 //  images\menu_36Night_runninglamp.png

#define FOOTBALL_IMAGE_NUM		37
u32 img_id_tab[FOOTBALL_IMAGE_NUM] = {
    PAGE0_c186_MENU_29SET,
    PAGE0_56cc_MENU_16SIRI,
    PAGE0_e4e2_MENU_1CLOCK,
    PAGE0_eb8d_MENU_4MUSIC,
    PAGE0_2e24_MENU_14SLEEP,
    PAGE0_1ee8_MENU_26TIMER,
    PAGE0_f432_MENU_33PRUNE,
    PAGE0_a990_MENU_24ALIPAY,
    PAGE0_f02e_MENU_6WEATHER,
    PAGE0_3efa_MENU_22COMPASS,
    PAGE0_bffe_MENU_23FIND_MY,
    PAGE0_66f5_MENU_30RESTART,
    PAGE0_ca8e_MENU_18CALENDAR,
    PAGE0_e1ba_MENU_20PRESSURE,
    PAGE0_e9bf_MENU_2TELEPHONE,
    PAGE0_bdf6_MENU_34PASSWORD,
    PAGE0_16f7_MENU_80ACTIVITY,
    PAGE0_d12b_MENU_13BREATHING,
    PAGE0_79bc_MENU_21ALTIMETER,
    PAGE0_651f_MENU_27STOPWATCH,
    PAGE0_b338_MENU_10HEART_RATE,
    PAGE0_cfa8_MENU_17CALCULATOR,
    PAGE0_dede_MENU_28PHOTOGRAPH,
    PAGE0_7f29_MENU_3INFORMATION,
    PAGE0_f695_MENU_5ALARM_CLOCK,
    PAGE0_cf1d_MENU_7BEDSIDE_LAMP,
    PAGE0_f6d4_MENU_11BLOOD_OXYGEN,
    PAGE0_c5ef_MENU_9INDOOR_SPORTS,
    PAGE0_35c8_MENU_32AMBIENT_NOISE,
    PAGE0_6a9f_MENU_8OUTDOOR_SPORTS,
    PAGE0_0aa3_MENU_12BLOOD_PRESSURE,
    PAGE0_32d6_MENU_15HEALTH_REMINDER,
    PAGE0_e103_MENU_25SOUND_RECORDING,
    PAGE0_e0fb_MENU_19FEMALE_ASSISTANT,
    PAGE0_76b1_MENU_31FACTORY_RECOVERY,
    PAGE0_9f4c_MENU_35POWER_SAVINGMODE,
    PAGE0_fbab_MENU_36NIGHT_RUNNINGLAMP,
};

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

static void gpu_port_demo_football_init(struct football_param *football, struct football_info_user *football_user)
{
    football->image = (struct ui_image_attrs *)malloc(football_user->image_num * sizeof(struct ui_image_attrs));
    football->image_num = football_user->image_num;

    char *img_path = football_user->path;

    //打开资源文件
    UI_RESFILE *fp =  res_fopen(img_path, "r");
    if (!fp) {
        printf("%s fp is null", __func__);
        ASSERT(0);
    }
    //获取文件的flash table
    struct flash_file_info file_info;
    ui_res_flash_info_get(&file_info, img_path, "res", true);
    jlgpu_task_texture_param_init(0, 0);
    //获取图片信息
    for (int i = 0; i < football->image_num; i++) {
        printf("id:%x\n", img_id_tab[i]);
        jlui_res_get_image_info(0, (img_id_tab[i] & 0xffff), fp, &task_param.image, &task_param.info);
        ui_res_get_image_flash_info(&task_param.info, &file_info, &task_param.image);
        printf("<%s>--------format:%d, offset:0x%x, size:%d * %d\n", __func__, task_param.image.format, task_param.image.offset, task_param.image.width, task_param.image.height);

        football->image[i].width = task_param.image.width;
        football->image[i].height = task_param.image.height;
        football->image[i].format = task_param.image.format;
        football->image[i].clut_format = task_param.image.clut_format;
        football->image[i].compress = task_param.image.compress;
        football->image[i].has_clut = task_param.image.has_clut;
        football->image[i].tab = (u8 *)task_param.info.tab;
        football->image[i].data = (u8 *)task_param.info.offset;
        football->image[i].len = task_param.image.len;
        football->image[i].gpu_format = res_format_to_gpu(task_param.image.format);
        printf("<%s>--------format:%d, offset:0x%x\n", __func__, task_param.image.format, task_param.info.offset);
    }

    struct rect win_rect;
    jlgpu_get_win_rect(&win_rect);

    football_init(football->image[0].width, football->image[0].height, 1.1f, 4.0f, win_rect.left, win_rect.top, win_rect.width, win_rect.height);
}


static void gpu_port_demo_create_football_task(struct football_info_user *football_user)
{
    struct football_param football;

    football.image = (struct ui_image_attrs *)malloc(football_user->image_num * sizeof(struct ui_image_attrs));
    football.image_num = football_user->image_num;

    char *img_path = football_user->path;

    //打开资源文件
    UI_RESFILE *fp =  res_fopen(img_path, "r");
    if (!fp) {
        printf("%s fp is null", __func__);
        ASSERT(0);
    }
    //获取文件的flash table
    struct flash_file_info file_info;
    ui_res_flash_info_get(&file_info, img_path, "res", true);
    jlgpu_task_texture_param_init(0, 0);
    //获取图片信息
    for (int i = 0; i < football.image_num; i++) {
        printf("id:%x\n", img_id_tab[i]);
        jlui_res_get_image_info(0, (img_id_tab[i] & 0xffff), fp, &task_param.image, &task_param.info);
        ui_res_get_image_flash_info(&task_param.info, &file_info, &task_param.image);
        printf("<%s>--------format:%d, offset:0x%x, size:%d * %d\n", __func__, task_param.image.format, task_param.image.offset, task_param.image.width, task_param.image.height);

        football.image[i].width = task_param.image.width;
        football.image[i].height = task_param.image.height;
        football.image[i].format = task_param.image.format;
        football.image[i].clut_format = task_param.image.clut_format;
        football.image[i].compress = task_param.image.compress;
        football.image[i].has_clut = task_param.image.has_clut;
        football.image[i].tab = (u8 *)task_param.info.tab;
        football.image[i].data = (u8 *)task_param.info.offset;
        football.image[i].len = task_param.image.len;
        football.image[i].gpu_format = res_format_to_gpu(task_param.image.format);
        printf("<%s>--------format:%d, offset:0x%x\n", __func__, task_param.image.format, task_param.info.offset);
    }

    struct rect win_rect;
    jlgpu_get_win_rect(&win_rect);

    football_init(football.image[0].width, football.image[0].height, 1.1f, 4.0f, win_rect.left, win_rect.top, win_rect.width, win_rect.height);

    football.head = __this->gpu_task_head;		//设置任务链表头
    football.task_id = 1;		//设置任务id，否则默认为零，会覆盖之前创建的任务
    football_draw(&football, football_user->x_diff, football_user->y_diff);

    if (fp) {
        res_fclose(fp);
    }
    if (football.image) {
        free(football.image);
    }
}

void gpu_port_demo_draw_football(struct football_param *football, float x_diff, float y_diff, u32 background)
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
        gpu_port_demo_create_bg_task(draw_task_rect, background);
    }

    //创建绘制图片的任务
    football->head = __this->gpu_task_head;		//设置任务链表头
    football->task_id = 1;		//设置任务id，否则默认为零，会覆盖之前创建的任务
    football->mask_en = 0;
    football_draw(football, x_diff, y_diff);


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

void gpu_demo_football(void *p)
{
    gpu_port_demo_lcd_init();		//初始化lcd
    gpu_port_demo_port_init();		//初始化gpu_port接口
    struct football_info_user football_user = {0};
    float x_diff = 0.0;
    float y_diff = 0.0;

    football_user.path = "storage/virfat_flash/C/ui/ui.res";
    football_user.image_num = FOOTBALL_IMAGE_NUM;
    football_user.image_id_tab = img_id_tab;
    football_user.x_diff = 0;
    football_user.y_diff = 0;

    struct football_param football;
    gpu_port_demo_football_init(&football, &football_user);

    while (1) {
        gpu_port_demo_task_init(GPU_FORMAT_RGB565);
        gpu_port_demo_draw_football(&football, x_diff, y_diff, 0xff00ff00);
        gpu_port_demo_task_free();	//任务链用完之后删除任务链，同时创建的任务也会清除
        x_diff += 0.1;
        y_diff += 0.1;

        os_time_dly(50);
    }
}


int gpu_demo_football_task(void)
{
    printf(">>>>>>>>> %s()", __func__);
    int err;
    err = os_task_create(gpu_demo_football, NULL, 10, 1024 * 5, 512, "gpu_demo_football");
    if (err != OS_NO_ERR) {
        printf("gpu task creat fail %x\n", err);
    }
    return 0;
}

#endif
