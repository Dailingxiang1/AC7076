/* Copyright(C) 2024, JIELI TECHNOLOGY, Inc.
 * All right reserved.
 */

/* ------------------------------------------------------------------------------------*/
/**
 * @file gpu_port.h
 *
 * @brief jlgpu 任务管理
 *
 * @author zhuhaifang@zh-jieli.com
 *
 * @version V1.0.0
 *
 * @date 2024-05-21
 */
/* ------------------------------------------------------------------------------------*/

/*
   一般流程：
 1. module_init	GPU 硬件模块初始化
 2. create_head 创建新的GPU任务链
 3. create_task 创建新的GPU任务
 4. 配置、运行任务链
 5. free_head 释放任务链
 6. module_free 关闭 GPU硬件模块
 注：配置参数中有“默认”标志的参数，可根据实际情况配置，也可使用默认值
 */


#ifndef __PUBLIC_GPU_PORT_H__
#define __PUBLIC_GPU_PORT_H__

#include "generic/rect.h"
#include "jlgpu_math.h"
#include "jlgpu_driver.h"
#include "res/resfile.h"
#include "gpu_format.h"

#define	JLGPU_ID_NONE	(-1)	// GPU任务或控件ID 无效或不存在
#define GPU_CACHE_RES_RESIDENT	(0XFF)//gpu_cache资源常驻

/* GPU task type defined */
typedef enum {
    GPU_TASK_FILL,	// color fill task
    GPU_TASK_TEXT,	// text task
    GPU_TASK_DRAW,	// custom draw task
    GPU_TASK_IMAGE,	// image task
    GPU_TASK_LINEAR,// linegrad task
    GPU_TASK_CUSTOM = 0x0100,	// custom GPU task mode
    GPU_TASK_TRANS  = 0X0200,	// affine_to_perspective
} TASK_TYPE;


/* The source of lut tab. */
enum {
    CLUT_TAB_NULL,
    CLUT_TAB_FROM_FLASH,	// lut tab from flash.
    CLUT_TAB_FROM_RAM,		// lut tab from ram
};


enum {
    GPU_SYNTHESIS_AND_DRAW,
    GPU_SYNTHESIS,
    GPU_DRAW,
    GPU_DRAW_EACH_GROUP,
};


/* MASK功能参数 */
struct mask_param {
    u8 big_end;			// 是否大端，1大端，0小端（默认）
    u8 mask_inv;		// 是否取反，1取反，0不取反（默认）
    int mask_stride;	// 行步进
    GPU_mask_format_t mask_format;	// MASK格式
    u8 *mask_data;		// mask数据，不支持压缩
    struct rect rect;	// mask绘制区域
};

/* 变换功能参数 */
struct tran_param {	// 变换参数（杰理UI框架必须用框架API配置）
    float ratio_w;	// 缩放参数，scale_en使能时有效
    float ratio_h;

    s16 rotate_cx;	// 旋转参数，rotate_en使能时有效
    s16 rotate_cy;
    s16 rotate_dx;
    s16 rotate_dy;
    float rotate_angle;

    u8 sample_mode;	// 插值方式：0最近邻，1双线性，3 3x3滤波，4 5x5滤波。注意：没有sample_mode=2的方式
    u8 shift_sel;	// 右移参数：sample_mode=0/1时，shift_sel=6，其它根据插值系数配置

    u8 coeff0;	// 高斯模糊参数，gaussian_en使能时有效
    u8 coeff1;
    u8 coeff2;
    u8 coeff3;
    u8 coeff4;
    u8 coeff5;
};

/* 填充功能参数 */
struct fill_param {
    union {
        struct mask_param mask;		// 填充+MASK
        struct tran_param tran;		// 填充+变换
    };
};

/* 渐变功能参数 */
struct linegrad_param {
    u8 lut_lvl;			// 颜色点个数，颜色表中颜色值的数量（2^(lut_lvl)+1）个颜色，与clut表颜色个数匹配
    u8 spread_mode;		// 渐变方式，0一次，1反射，2重复
    /*
    示例：在点0到点1之间，产生从2到0的渐变
    方式：  0一次          1反射          2重复		(spread_mode)
    点位置： 0 1            0 1            0 1      (x0,y0)(x1,y1)
    	 |222210000|    |101210121|    |210210210|
    	 |222210000|    |101210121|    |210210210|
    	 |222210000|    |101210121|    |210210210|
    */
    /* 一次渐变周期坐标区间，渐变通过clut表，从P0指向P1，向量方向即为渐变方向 */
    u16 x0;
    u16 y0;
    u16 x1;
    u16 y1;

    struct tran_param tran;			// 渐变+变换
};

/* 纹理功能参数 */
struct texture_param {
    u8 big_end;			// 是否大端，1大端，0小端（默认）
    u8 alpha_end;		// alpha在高/低位，1低位，0高位（默认）
    u8 rbs;				// R,B交换，1交换，0不交换（默认）
    u8 color_ext_mode;
    u8 not_compress;	// 数据不压缩，1不压缩，0压缩（默认）。注意：基础参数的image结构体要配置
    u8 adr_mode;		// 不压缩的数据排列方式，0按块排列，1按行排列（默认）
    u8 mmu_page_size;	// mmu tab页size，参考GPU_mmu_page_size_t，mmu_tab_base不为NULL时有效
    u8 *mmu_tab_base;	// mmu tab起始地址，不等于NULL时有效
    u8 *data;			// 纹理起始地址（SRAM/PSRAM/NORFLASH 地址）
    u8 *mmu_clut_tab;       // 只对mmu_tab_base非NULL时有效, (LX/ALXX/AX格式)纹理颜色表起始地址
    u32 mmu_clut_offset;    // 只对mmu_tab_base非NULL时有效, 在mmu_clut_tab[0]里的偏移地址
    struct rect crop;	// 裁剪区域，当crop_en为true时裁剪，否则为{left=0, top=0, width=img_w, height=img_h}

    union {
        struct mask_param mask;	// 纹理+MASK
        struct tran_param tran;	// 纹理+变换
    };
};

/* 基础功能参数 */
typedef struct gpu_task_param {
    TASK_TYPE task_type;// must, 任务类型
    u32 task_id;		// must, 任务ID
    u32 element_id;		// must, 控件ID
    s16 priority;		// optional, 优先级，可自行修改，若等于-1，内部会自动按链表顺序赋值
    u8 invisible;		// optional, 默认隐藏，true 隐藏，false 显示（默认）
    u8 global_alpha;	// optional, 全局alpha，默认128

    u8 blend_mode;		// 混合模式，默认GPU_BLEND_SRC_OVER，参照jlgpu_driver.h配置
    u8 premult;			// alpha预乘，true使能，false不使能（默认）
    u8 mask_en;			// MASK 使能，true使能（需配置mask参数），false不使能（默认）
    u8 perspective_en;	// 透视变换使能，true使能（需配置tran参数），false不使能（默认）

    u8 scale_en;		// 缩放使能，true使能（需配置tran参数），false不使能（默认）
    u8 rotate_en;		// 旋转使能，true使能（需配置tran参数），false不使能（默认）
    u8 crop_en;			// 裁剪使能，true使能（需配置纹理中crop参数），false不使能（默认）
    u8 draw_en;			// 自定义绘图使能，true使能（需配置自定义绘图参数），false不使能（默认）

    u8 color_key_en;	// 待完善
    u8 dither_en;
    u8 ext_mode;        // 拓展模式使能，true使能，false不使能（默认）
    u8 gaussian_en;		// 高斯模糊使能，true使能（需配置tran中coeff参数），false不使能（默认）

    u8 custom_color;	// 自定义颜色，true使能（自动将颜色表颜色改为下面参数颜色），flase不使能（默认）
    u8 has_clut;		// 颜色表参数，颜色表来源（CLUT_TAB_FROM_FLASH, CLUT_TAB_FROM_RAM, CLUT_TAB_NULL（默认））
    u8 clut_format;		// 颜色表格式，（ARGB8888（默认）, ARGB8565, RGB888, RGB565）
    u8 gpu_free_data;	// 由GPU释放纹理buf标志，true使能（GPU任务销毁时会释放纹理buf），false不使能（默认）

    int clut_size;		// 颜色表长度
    u8 *clut_tab;		// 颜色表地址，（has_clut != CLUT_TAB_NULL 时有效），注意：每次配置GPU任务时会释放，需重新malloc buf缓存

    /* 颜色值(ARGB8888)，纯色填充时，为填充颜色；custom_color使能时，为自定义颜色 */
    u8 alpha;			// 颜色，透明度（0~100）
    u8 red;				// 颜色，红（8bit）
    u8 green;			// 颜色，绿（8bit）
    u8 blue;			// 颜色，蓝（8bit）

    GPU_format_t format;	// must, 输入颜色格式
    gpu_matrix_t *matrix;	// optional, 叠加矩阵，一般指父控件矩阵，用于父子控件特效叠加，如果需自己管理矩阵，也可用作自定义矩阵

    struct rect area;			// must, 限制区域，相当于父控件区域，绘图最终必须落在area范围内
    struct rect draw;			// must, 绘制区域，当前任务要绘制到哪个区域，相对于0坐标计算
    float offset_x;
    float offset_y;
    struct image_file image;		// 从flash读取数据时的图片信息
    struct flash_file_info info;	// flash 信息

    /* 自定义绘图参数，draw_en使能时才会生效 */
    void (*cb_func);
    void (*cb_exit);
    void *priv;
    u32 priv_len;
    int draw_id;
    /************ 自定义绘图参数结束 ***********/

    /* 功能参数，根据不同的功能进行配置 */
    union {
        struct fill_param fill;			// 填充功能
        struct linegrad_param linegrad;	// 渐变功能
        struct texture_param texture;	// 纹理功能
    };
} JLGPUTaskParam_t, *pJLGPUTaskParam_t;


// GPU任务公共的基础参数初始化，默认设置，所有GPU任务默认设置都一致
#define jlgpu_task_basic_param_init(tsk_type, tsk_id, elm_id) \
	JLGPUTaskParam_t task_param = {0}; \
	task_param.task_type = (tsk_type); \
	task_param.task_id = (tsk_id); \
	task_param.element_id = (elm_id); \
	task_param.global_alpha = 128; \
	task_param.blend_mode = GPU_BLEND_SRC_OVER; \
	task_param.dither_en = 1; \
	task_param.premult = 1; \
	task_param.priority = -1; \


#define jlgpu_task_texture_big_endian_enable(big_endian) \
    task_param.texture.big_end = big_endian; //字库buf字节位序翻转

// 填充任务的参数初始化
#define jlgpu_task_fill_param_init(task_id, elm_id, a, r, g, b) \
	jlgpu_task_basic_param_init(GPU_TASK_FILL, task_id, elm_id); \
	task_param.alpha = (a); \
	task_param.red   = (r); \
	task_param.green = (g); \
	task_param.blue  = (b); \

// 渐变任务的参数初始化
#define jlgpu_task_linegrad_param_init(task_id, elm_id) \
	jlgpu_task_basic_param_init(GPU_TASK_LINEAR, task_id, elm_id); \

// 纹理任务的参数初始化
#define jlgpu_task_texture_param_init(task_id, elm_id) \
	jlgpu_task_basic_param_init(GPU_TASK_IMAGE, task_id, elm_id); \
	task_param.texture.mmu_page_size = MMU_PAGE_4K; \
	task_param.texture.adr_mode = 1; \


// 3x3高斯模糊参数配置

// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 1; \
// 	(tran)->coeff2 = 56; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;

// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 2; \
// 	(tran)->coeff2 = 52; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;

// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 3; \
// 	(tran)->coeff2 = 48; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;

// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 4; \
// 	(tran)->coeff2 = 44; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;

// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 5; \
// 	(tran)->coeff2 = 40; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;

// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 6; \
// 	(tran)->coeff2 = 36; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;


// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 7; \
// 	(tran)->coeff2 = 32; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;

// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 8; \
// 	(tran)->coeff2 = 28; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;

// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 9; \
// 	(tran)->coeff2 = 24; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;

#define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
	(param)->gaussian_en = 1; \
	(tran)->sample_mode = 3; \
	(tran)->shift_sel = 6; \
	(tran)->coeff0 = 1; \
	(tran)->coeff1 = 10; \
	(tran)->coeff2 = 20; \
	(tran)->coeff3 = 0; \
	(tran)->coeff4 = 0; \
	(tran)->coeff5 = 0;

// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 11; \
// 	(tran)->coeff2 = 16; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;

// #define jlgpu_task_param_3x3_gaussian_low_level(param, tran) \
// 	(param)->gaussian_en = 1; \
// 	(tran)->sample_mode = 3; \
// 	(tran)->shift_sel = 6; \
// 	(tran)->coeff0 = 1; \
// 	(tran)->coeff1 = 12; \
// 	(tran)->coeff2 = 12; \
// 	(tran)->coeff3 = 0; \
// 	(tran)->coeff4 = 0; \
// 	(tran)->coeff5 = 0;

#define jlgpu_task_param_3x3_gaussian_middle_level(param, tran) \
	(param)->gaussian_en = 1; \
	(tran)->sample_mode = 3; \
	(tran)->shift_sel = 5; \
	(tran)->coeff0 = 1; \
	(tran)->coeff1 = 5; \
	(tran)->coeff2 = 8; \
	(tran)->coeff3 = 0; \
	(tran)->coeff4 = 0; \
	(tran)->coeff5 = 0;

#define jlgpu_task_param_3x3_gaussian_high_level(param, tran) \
	(param)->gaussian_en = 1; \
	(tran)->sample_mode = 3; \
	(tran)->shift_sel = 4; \
	(tran)->coeff0 = 1; \
	(tran)->coeff1 = 2; \
	(tran)->coeff2 = 4; \
	(tran)->coeff3 = 0; \
	(tran)->coeff4 = 0; \
	(tran)->coeff5 = 0;

// 5x5高斯模糊参数配置
#define jlgpu_task_param_5x5_gaussian_level0(param, tran) \
	(param)->gaussian_en = 1; \
	(tran)->sample_mode = 4; \
	(tran)->shift_sel = 9; \
	(tran)->coeff0 = 1; \
	(tran)->coeff1 = 5; \
	(tran)->coeff2 = 24; \
	(tran)->coeff3 = 10; \
	(tran)->coeff4 = 62; \
	(tran)->coeff5 = 84;

#define jlgpu_task_param_5x5_gaussian_level1(param, tran) \
	(param)->gaussian_en = 1; \
	(tran)->sample_mode = 4; \
	(tran)->shift_sel = 8; \
	(tran)->coeff0 = 1; \
	(tran)->coeff1 = 3; \
	(tran)->coeff2 = 15; \
	(tran)->coeff3 = 6; \
	(tran)->coeff4 = 24; \
	(tran)->coeff5 = 48;

#define jlgpu_task_param_5x5_gaussian_level2(param, tran) \
	(param)->gaussian_en = 1; \
	(tran)->sample_mode = 4; \
	(tran)->shift_sel = 7; \
	(tran)->coeff0 = 1; \
	(tran)->coeff1 = 2; \
	(tran)->coeff2 = 8; \
	(tran)->coeff3 = 4; \
	(tran)->coeff4 = 11; \
	(tran)->coeff5 = 16;


#define jlgpu_task_param_5x5_gaussian_level3(param, tran) \
	(param)->gaussian_en = 1; \
	(tran)->sample_mode = 4; \
	(tran)->shift_sel = 6; \
	(tran)->coeff0 = 1; \
	(tran)->coeff1 = 1; \
	(tran)->coeff2 = 4; \
	(tran)->coeff3 = 2; \
	(tran)->coeff4 = 5; \
	(tran)->coeff5 = 8;


#define jlgpu_task_param_5x5_gaussian_level4(param, tran) \
	(param)->gaussian_en = 1; \
	(tran)->sample_mode = 4; \
	(tran)->shift_sel = 6; \
	(tran)->coeff0 = 2; \
	(tran)->coeff1 = 2; \
	(tran)->coeff2 = 3; \
	(tran)->coeff3 = 3; \
	(tran)->coeff4 = 3; \
	(tran)->coeff5 = 4;

#define jlgpu_task_param_5x5_gaussian_level5(param, tran) \
	(param)->gaussian_en = 1; \
	(tran)->sample_mode = 4; \
	(tran)->shift_sel = 10; \
	(tran)->coeff0 = 41; \
	(tran)->coeff1 = 41; \
	(tran)->coeff2 = 41; \
	(tran)->coeff3 = 41; \
	(tran)->coeff4 = 41; \
	(tran)->coeff5 = 40;


// 自定义绘图回调
typedef struct gpu_task_draw {
    void (*cb_func)(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv, void *matrix);
    void (*cb_exit)(void *draw_info, void *priv);
    void *priv;
    int priv_len;
    int draw_id;
    struct rect task_r;
} JLGPUTaskDraw_t, *pJLGPUTaskDraw_t;


/* ------------------------------------------------------------------------------------*/
/**
 * @brief rect_to_bbox rect结构体转gpu_boundbox_t结构体
 *
 * @Params rect rect结构体指针
 * @Params bbox bbox结构体指针
 *
 * @return void
 */
/* ------------------------------------------------------------------------------------*/
#define rect_to_bbox(rect, bbox) { \
	(bbox)->minx = (rect)->left; \
	(bbox)->maxx = (rect)->left + (rect)->width; \
	(bbox)->miny = (rect)->top; \
	(bbox)->maxy = (rect)->top + (rect)->height; \
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief bbox_to_rect gpu_boundbox_t结构体转rect结构体
 *
 * @Params bbox bbox结构体指针
 * @Params rect rect结构体指针
 *
 * @return void
 */
/* ------------------------------------------------------------------------------------*/
#define bbox_to_rect(bbox, rect) { \
	(rect)->left   = (bbox)->minx; \
	(rect)->width  = (bbox)->maxx - (bbox)->minx; \
	(rect)->top    = (bbox)->miny; \
	(rect)->height = (bbox)->maxy - (bbox)->miny; \
}


/* GPU task information structure defined.
   Cache required, resources must be saved. */
typedef struct gpu_task_info {
    u32 rev1: 16;		// 预留
    u32 need_free_data: 1;	// 是否由GPU释放数据buf
    u32 matrix_from_task: 1;	// 拷贝链表从gpu链表获取的矩阵
    u32 own_matrix: 1;	// 矩阵是否属于自己标志（1自己申请的矩阵，0继承的父控件矩阵）
    u32 enable: 1;		// enable
    u32 priority: 12;	// 优先级
    //4bytes
    u32 image_width: 11;	//
    u32 image_height: 11; //
    u32 group: 10;		//
    //8bytes
    u32 task_id;		// 任务ID
    u32 element_id;		// 控件ID
    //16bytes
    u8 *clut_tab;  		// 颜色表
    gpu_matrix_t *matrix;	// 当前任务的变换矩阵
    pJLGPUTaskDraw_t draw_info;	// custom draw
    //19bytes
} JLGPUTaskInfo_t, *pJLGPUTaskInfo_t;


/* GPU task unit structure defined. */
typedef struct gpu_task_unit {
    JLGPUTaskInfo_t info;	// GPU task information structure.
    u32 task_addr;			// GPU task instructions address.
} JLGPUTaskUnit_t, *pJLGPUTaskUnit_t;


/* GPU task chain head structure defined. */
typedef struct gpu_task_head {
    gpu_out_params_t out_param;	// GPU output instructions structure.
    struct rect win_rect;
    void *gpu_task_base_adr;	// First GPU task instructions address.
} JLGPUTaskHead_t, *pJLGPUTaskHead_t;

enum HEAD_STATUS {
    HEAD_STATUS_IDLE,
    HEAD_STATUS_READY,
};

typedef struct gpu_head_list {
    pJLGPUTaskHead_t head;
    enum HEAD_STATUS status;
    u8 index;
} JLGPUMultHeadList_t, *pJLGPUMultHeadList_t;

/* Multi task chain management unit structure defined. */
typedef struct gpu_mult_list {
    struct list_head list;	// list node.
    JLGPUMultHeadList_t head_list[3];
    pJLGPUMultHeadList_t curr_head;
    pJLGPUMultHeadList_t last_head;
    pJLGPUTaskHead_t head;	// Head pointer of GPU task chain.
    struct rect win_rect;
    struct rect gpu_rect;
    struct rect lcd_rect;
    u8 index;				// Index of GPU task chain, unique correspondence with head.
} JLGPUMultTaskList_t, *pJLGPUMultTaskList_t;



/* ================================= GPU 模块接口 ================================= */
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_module_init GPU模块初始化
 *
 * @Params malloc 内存申请回调
 * @Params free 内存释放回调
 * @Params screen_width 屏幕宽度
 * @Params screen_height 屏幕高度
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_module_init(
    void *(*malloc)(int size, u32 ram_type, u32 module_type),
    void (*free)(void *p, u32 ram_type, u32 module_type),
    u16 screen_width, u16 screen_height);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_module_free GPU模块关闭
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_module_free();



/* ================================= GPU 任务链接口 ================================= */
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_create_task_list_head 创建GPU任务链
 *
 * @return NULL 创建失败，其它 创建成功
 *
 * @note 链表头用malloc的buf缓存，释放时需用delete接口释放
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUTaskHead_t jlgpu_create_task_list_head();
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_free_all_task 释放指定任务链中的所有任务
 *
 * @Params head GPU链表头
 *
 * @note 只释放gpu任务，不释放链表头
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_free_all_task(pJLGPUTaskHead_t head);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_delete_task_list_head 删除GPU任务链
 *
 * @Params head GPU任务链头
 *
 * @note 会释放任务链中所有的GPU任务，并将任务链头释放掉
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_delete_task_list_head(pJLGPUTaskHead_t head);


pJLGPUTaskHead_t jlgpu_shadow_task_list_head(pJLGPUTaskHead_t head);	// 内部使用
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_set_task_list_out_format 设置GPU任务链输出格式
 *
 * @Params head GPU任务链头
 * @Params out_format GPU任务链输出格式
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_set_task_list_out_format(pJLGPUTaskHead_t head, GPU_out_format_t out_format);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_set_task_list_out_rbs 设置GPU任务链输出R、B是否交换
 *
 * @Params head GPU任务链头
 * @Params rbs R、B是否交换，0不交换(默认), 1交换
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_set_task_list_out_rbs(pJLGPUTaskHead_t head, u8 rbs);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_set_task_list_out_rgba 设置GPU任务链输出 alpha 位置
 *
 * @Params head GPU任务链头
 * @Params rgba alpha排列方式，0 alpha在高位(默认)，1 alpha在低位
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_set_task_list_out_rgba(pJLGPUTaskHead_t head, u8 rgba);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_set_task_list_out_endian 设置GPU任务链输出大小端
 *
 * @Params head GPU任务链头
 * @Params endian 大小端选择，0 小端(默认)，1 大端
 *
 * @note 只支持Bpp<8或RGB565
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_set_task_list_out_endian(pJLGPUTaskHead_t head, u8 endian);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_get_task_list_out_format 获取任务链输出格式
 *
 * @Params head 任务链表头
 *
 * @return 输出格式
 */
/* ------------------------------------------------------------------------------------*/
GPU_out_format_t jlgpu_get_task_list_out_format(pJLGPUTaskHead_t head);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_set_task_list_out_buf 设置GPU任务链绘制区域和buf
 *
 * @Params head GPU任务链头
 * @Params out_buf 输出buf指针
 * @Params rect 本次输出区域
 * @Params left 左右移动步进
 * @Params stride 移动时的行步进，left != 0时有效
 *
 * @note 这里的rect与out_buf大小对应，每次run任务链之前进行配置，rect为每个分块的区域，out_buf为每个分块的输出buf
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_set_task_list_out_buf(pJLGPUTaskHead_t head, u8 *out_buf, struct rect *rect, int left, int stride);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_list_run 启动GPU任务链合成绘制
 *
 * @Params head GPU任务链头
 *
 * @note 本函数会等GPU合成完毕才退出，退出时out_buf已经是GPU混合好的输出数据
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_task_list_run(pJLGPUTaskHead_t head);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_list_reorder GPU任务链按照priority重排序
 *
 * @Params head GPU任务链头
 *
 * priority默认按照GPU任务创建顺序递增，修改priority后需重排才能调整GPU混合顺序
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_task_list_reorder(pJLGPUTaskHead_t head);


/* ================================= 特效接口 ================================= */
// 创建&拷贝gpu任务链
pJLGPUTaskHead_t jlgpu_task_list_copy_create_with_backcolor(u32 backcolor, pJLGPUTaskHead_t head, gpu_matrix_t *matrix, int group);
// 拷贝gpu任务链
pJLGPUTaskHead_t jlgpu_task_list_copy_create(pJLGPUTaskHead_t new_head, pJLGPUTaskHead_t head, gpu_matrix_t *matrix, int group);
// 释放拷贝的gpu任务链
void jlgpu_task_list_copy_destroy(pJLGPUTaskHead_t head);
// 更新指定group的gpu任务矩阵
int jlgpu_task_list_copy_update_matrix_by_group(pJLGPUTaskHead_t new_head, pJLGPUTaskHead_t head, int group);
// 指定group的gpu任务进行矩阵相乘
void jlgpu_task_list_copy_mul_matrix_by_group(pJLGPUTaskHead_t head, gpu_matrix_t *matrix, int group);
// 显示/隐藏指定group的gpu任务
void jlgpu_task_list_copy_enable_by_group(pJLGPUTaskHead_t head, int group, int enable);
// 销毁指定group的gpu任务
void jlgpu_task_list_copy_destroy_by_group(pJLGPUTaskHead_t head, int group);
// 链表指令顺序重排
int jlgpu_task_list_copy_group_order_adjust(pJLGPUTaskHead_t head, u8 *order_tab, int order_tab_size);
// 链表指令透明度重置
int jlgpu_task_list_copy_group_global_alpha_reset(pJLGPUTaskHead_t head, u8 *order_tab, u8 *global_alpha_tab, int global_alpha_tab_size);

/* ================================= GPU 任务接口 ================================= */
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_create_task 创建GPU任务，并添加到head链表中
 *
 * @Params head GPU任务链头
 * @Params param GPU任务参数
 *
 * @return NULL 创建任务失败，其它 创建的GPU任务单元
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUTaskUnit_t jlgpu_create_task(pJLGPUTaskHead_t head, pJLGPUTaskParam_t param);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_update_task 更新GPU任务单元
 *
 * @Params taskp GPU任务单元
 * @Params param GPU任务参数
 *
 * @return GPU任务单元，传入的taskp
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUTaskUnit_t jlgpu_update_task(pJLGPUTaskUnit_t taskp, pJLGPUTaskParam_t param);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_delete_task 删除GPU任务单元
 *
 * @Params head GPU任务链头
 * @Params taskp GPU任务单元（待删除的单元）
 *
 * @return 0 删除成功，-1 删除失败（任务单元不在任务链中）
 */
/* ------------------------------------------------------------------------------------*/
int jlgpu_delete_task(pJLGPUTaskHead_t head, pJLGPUTaskUnit_t taskp);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_get_task_rect 获取GPU任务的绘制区域
 *
 * @Params taskp GPU任务单元
 * @Params rect GPU任务单元的绘制区域（输出）
 *
 * @note GPU绘制区域会有一定的映射，与配置进去的区域不一定相同
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_get_task_rect(pJLGPUTaskUnit_t taskp, struct rect *rect);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_get_win_rect 获取GPU绘制窗口区域
 *
 * @Params rect GPU窗口区域（输出）
 *
 * @note GPU模块初始化后才能获取，GPU模块会根据屏幕宽高映射到一定坐标
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_get_win_rect(struct rect *rect);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_set_win_rect 设置GPU绘制窗口区域
 *
 * @Params rect GPU窗口区域（输入）
 *
 * @note GPU模块初始化后，可以通过本接口移动窗口位置，满足需移动到0坐标的特效使用
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_set_win_rect(struct rect *rect);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_shift_rect 将rect方框区域映射到GPU绘制窗口内
 *
 * @Params rect 待映射的区域(输入&输出)
 *
 * @note 未创建GPU任务的方框区域(相对于0坐标)，可通过本接口映射到GPU绘制窗口内
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_shift_rect(struct rect *rect);



/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_new_mode 通过参数获取gpu任务类型
 *
 * @param param	gpu参数
 *
 * @return GPU任务类型
 */
/* ------------------------------------------------------------------------------------*/
u8 jlgpu_task_new_mode(pJLGPUTaskParam_t param);
/* ================================= 通过ID操作任务接口 ================================= */
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_get_task_by_id 通过ID在任务链中搜素任务
 *
 * @Params head GPU任务链头
 * @Params task_id GPU任务ID
 * @Params element_id 控件ID
 *
 * @return NULL 未找到GPU任务，其它 从GPU任务链中找到的任务单元
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUTaskUnit_t jlgpu_get_task_by_id(pJLGPUTaskHead_t head, u32 task_id, u32 element_id);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_enable_by_id 通过ID设置GPU任务的使能
 *
 * @Params head GPU任务链头
 * @Params task_id 任务ID
 * @Params element_id 控件ID
 * @Params enable 使能标志，1使能，0不使能
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_task_enable_by_id(pJLGPUTaskHead_t head, u32 task_id, u32 element_id, int enable);
void jlgpu_task_enable(pJLGPUTaskUnit_t taskp, int enable);
void jlgpu_task_layer_enable(pJLGPUTaskHead_t head, int enable);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_delete_by_id 通过ID删除GPU任务
 *
 * @Params head 任务链头
 * @Params task_id 任务ID
 * @Params element_id 控件ID
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_task_delete_by_id(pJLGPUTaskHead_t head, u32 task_id, u32 element_id);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_set_prior_by_id 通过ID设置GPU任务优先级
 *
 * @Params head 任务链头
 * @Params task_id 任务ID
 * @Params element_id 控件ID
 * @Params priority 优先级
 * @Params reorder 是否重新排序，true 是，false 否
 *
 * @note 如果需设置多个优先级，可在都设置完后再调用一次重新排序，不必每次都排序
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_task_set_prior_by_id(pJLGPUTaskHead_t head, u32 task_id, u32 element_id, u32 priority, int reorder);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_clean_up_by_id 清空指定控件ID下超过某索引的GPU任务
 *
 * @Params head 任务链头
 * @Params element_id 控件ID
 * @Params task_index 需要删除的GPU任务索引起始值
 *
 * @note 可将element_id理解为组，task_id理解为组内按顺序增长的索引，这里删除某组内超过task_index索引的GPU任务
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_task_clean_up_by_id(pJLGPUTaskHead_t head, u32 element_id, u8 task_index);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_clean_up_by_id 清空指定控件ID下超过某索引的GPU任务
 *
 * @Params head 任务链头
 * @Params element_id 控件ID
 * @Params task_index 需要删除的GPU任务索引起始值
 * @Params sync 同步回调接口(适用于双链表)
 *
 * @note 可将element_id理解为组，task_id理解为组内按顺序增长的索引，这里删除某组内超过task_index索引的GPU任务
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_task_clean_up_sync_by_id(pJLGPUTaskHead_t head, u32 element_id, u8 task_index, int (*wait_sync)(void));

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_find_task_by_id 通过ID查找匹配的GPU任务配置
 *
 * @Params head 任务链头
 * @Params task_id 任务ID
 * @Params element_id 控件ID
 *
 * @return NULL GPU任务单元不存在，其它 找到匹配的的GPU任务单元
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUTaskUnit_t jlgpu_find_task_by_id(pJLGPUTaskHead_t head, u32 task_id, u32 element_id);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_update_task_by_id 通过ID更新GPU任务配置
 *
 * @Params head 任务链头
 * @Params task_id 任务ID
 * @Params element_id 控件ID
 * @Params param 任务配置参数
 *
 * @return NULL 配置失败，其它 更新了配置的GPU任务单元
 *
 * @note 当任务链中不存在指定任务时，这里会重新创建，返回NULL表示创建失败
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUTaskUnit_t jlgpu_update_task_by_id(pJLGPUTaskHead_t head, u32 task_id, u32 element_id, pJLGPUTaskParam_t param);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_find_first_task 获取任务链的第一个任务
 *
 * @Params head 任务链头
 *
 * @return NULL 任务不存在，其它 任务链的第一个任务
 *
 * @note 无
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUTaskUnit_t jlgpu_task_find_first_task(pJLGPUTaskHead_t head);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_find_next_task 获取当前任务的下一个任务
 *
 * @Params curr_task 当前任务
 *
 * @return NULL 任务不存在，其它 当前任务的下一个任务
 *
 * @note 如果当前任务为所在任务链的最后一个任务，则返回NULL
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUTaskUnit_t jlgpu_find_next_task(pJLGPUTaskUnit_t curr_task);


/* ================================= 多任务链管理接口 ================================= */
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_mult_task_list_get_common_root 获取多任务链管理句柄
 *
 * @return 多任务链管理句柄
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUMultTaskList_t jlgpu_mult_task_list_get_common_root();

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_mult_task_list_check 检查多任务链管理句柄是否合法
 *
 * @Params mult_list 待检查的多任务链管理句柄
 *
 * @return 1 多任务链管理句柄合法，0 多任务链管理句柄不合法
 */
/* ------------------------------------------------------------------------------------*/
int jlgpu_mult_task_list_check(pJLGPUMultTaskList_t mult_list);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_mult_task_list_switch_root 切换多任务链管理句柄
 *
 * @Params mult_list 当前使用的多任务链管理句柄
 *
 * @return 影子组多任务链管理句柄
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUMultTaskList_t jlgpu_mult_task_list_switch_root(pJLGPUMultTaskList_t mult_list);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_mult_task_list_get_head_num 获取多任务链管理句柄管理的任务链条数
 *
 * @Params mult_list 多任务链管理句柄
 *
 * @return 多任务链管理句柄管理的任务链数量
 */
/* ------------------------------------------------------------------------------------*/
int jlgpu_mult_task_list_get_head_num(pJLGPUMultTaskList_t mult_list);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_mult_task_list_add 增加任务链到多任务链管理句柄管理
 *
 * @Params mult_list 多任务链管理句柄
 * @Params task_head 待加入管理的GPU任务链
 * @Params index GPU任务链索引
 *
 * @return 0添加成功，1添加失败
 */
/* ------------------------------------------------------------------------------------*/
int jlgpu_mult_task_list_add(pJLGPUMultTaskList_t mult_list, pJLGPUTaskHead_t task_head, u8 index);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_mult_task_list_del 从任务链管理句柄删除GPU任务链
 *
 * @Params mult_list 多任务链管理句柄
 * @Params task_head 待从多任务链管理句柄删除的任务链
 *
 * @return false 删除失败，true 删除成功
 *
 * @note 这里只会从多任务链管理句柄剔除任务链，并不会释放任务链本身。要释放任务链需用任务链清空接口
 */
/* ------------------------------------------------------------------------------------*/
int jlgpu_mult_task_list_del(pJLGPUMultTaskList_t mult_list, pJLGPUTaskHead_t task_head);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_mult_task_list_clr 清空多任务链管理句柄管理的任务链
 *
 * @Params mult_list 多任务链管理句柄
 *
 * @return 0 清空成功，1 清空失败
 *
 * @note 注意，这里会释放句柄中管理的所有任务链
 */
/* ------------------------------------------------------------------------------------*/
int jlgpu_mult_task_list_clr(pJLGPUMultTaskList_t mult_list);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_mult_task_list_sync_shadow 同步影子组多任务链管理句柄
 *
 * @Params mult_list 当前使用的多任务链管理句柄
 *
 * @return 影子组多任务链管理器句柄
 *
 * @notes 注意，这里会将当前使用的多任务链管理句柄拷贝一份放到影子组，GPU任务链使用的内存增长一倍
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUMultTaskList_t jlgpu_mult_task_list_sync_shadow(pJLGPUMultTaskList_t mult_list);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_mult_task_head_by_index 通过index获取GPU任务链
 *
 * @Params mult_list 多任务链管理句柄
 * @Params index 需获取的GPU任务链索引
 *
 * @return NULL 获取失败，其它 获取成功
 */
/* ------------------------------------------------------------------------------------*/
pJLGPUTaskHead_t jlgpu_mult_task_head_by_index(pJLGPUMultTaskList_t mult_list, int index);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_mult_task_head_modify_by_index 替换指定索引的GPU任务链
 *
 * @Params mult_list 多任务链管理句柄
 * @Params index GPU任务链索引
 * @Params new_head 新的GPU任务链
 *
 * @return 0 替换成功，-1 替换失败
 *
 * @note 注意，这里会将指定索引的GPU任务链替换，如果原GPU任务链未记录或未释放，可能出现内存泄露
 */
/* ------------------------------------------------------------------------------------*/
int jlgpu_mult_task_head_modify_by_index(pJLGPUMultTaskList_t mult_list, int index, pJLGPUTaskHead_t new_head);

int jlgpu_mult_task_head_set_rect_by_index(pJLGPUMultTaskList_t mult_list, int index, struct rect *page_r, struct rect *gpu_r, struct rect *lcd_r);

int jlgpu_mult_task_head_get_rect_by_index(pJLGPUMultTaskList_t mult_list, int index, struct rect *page_r, struct rect *gpu_r, struct rect *lcd_r);

pJLGPUTaskHead_t jlgpu_mult_task_head_idle_by_index(pJLGPUMultTaskList_t mult_list, u8 index, pJLGPUTaskHead_t curr_head);

int jlgpu_mult_task_list_get(pJLGPUMultTaskList_t mult_list, int *index_tab);

int jlgpu_mult_task_list_del_by_index(pJLGPUMultTaskList_t mult_list, u8 index);

void jlgpu_driver_fill_enchange_perspective(void *p, int clip_x_min, int clip_x_max, int clip_y_min, int clip_y_max);
void jlgpu_driver_texture_enchange_perspective(void *p, int clip_x_min, int clip_x_max, int clip_y_min, int clip_y_max);
void jlgpu_driver_set_clut_addr(void *p, u32 clut_addr);


/* ================================= 调试打印接口 ================================= */
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_dump_task_head 打印GPU任务链头配置
 *
 * @Params head GPU任务链头
 *
 * @note 这里只打印任务链头信息，不打印任务链中的任务信息
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_dump_task_head(pJLGPUTaskHead_t head);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_dump_task_info 打印GPU任务单元信息
 *
 * @Params task 任务单元指针
 * @Params count 当前任务个数
 *
 * @note count 参数在打印多个任务单元时用作区分，打印时会将count打印出来，本身不具有含义
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_dump_task_info(pJLGPUTaskUnit_t task, int count);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_dump_all_task 打印任务链中所有信息
 *
 * @Params head 任务链头
 * @Params func 调用本接口的函数，一般用 __func__ 即可
 * @Params line 调用本接口的行数，一般用 __LINE__ 即可
 *
 * @note 这里会将任务链头及任务链中所有任务信息都打印出来
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_dump_all_task(pJLGPUTaskHead_t head, const char *func, int line);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_dump_mult_list_node 打印GPU多任务链管理单元
 *
 * @Params mult_list GPU多任务链管理单元
 * @Params dump_task_unit 是否打印具体GPU任务信息
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_dump_mult_list_node(pJLGPUMultTaskList_t mult_list, u8 dump_task_unit);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_dump_mult_list 打印多任务链管理句柄
 *
 * @Params dump_task_unit 是否打印具体任务信息
 * 1 将所有任务链的任务单元信息打印；
 * 0 只打印多任务链管理有那些任务链，不打印具体任务信息；
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_dump_mult_list(u8 dump_task_unit);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_get_res_len 通过gpu任务获取资源大小
 *
 * @param task_addr	gpu访问的地址
 *
 * @return
 			> 0 资源长度
			0	获取不到长度（压缩方式不支持）
			-1	任务格式不匹配（非纹理）
 */
/* ------------------------------------------------------------------------------------*/
int jlgpu_task_get_res_len(void *task_addr);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_get_res_info
 *
 * @param task_addr	gpu任务地址
 * @param len		addr长度(bytes)
 					当资源为res_mmu时，返回res_mmu长度*4bytes
					当资源为非res_mmu时，返回资源实际长度bytes
 *
 * @return 	资源地址
 */
/* ------------------------------------------------------------------------------------*/
void *jlgpu_task_get_res_info(void *task_addr, int *len);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_task_get_mask_info
 *
 * @param task_addr	gpu任务地址
 * @param len		mask_len 长度
 *
 * @return 	mask地址
 */
/* ------------------------------------------------------------------------------------*/
void *jlgpu_task_get_mask_info(void *task_addr, int *len);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_get_matrix 从gpu任务里获取矩阵
 *
 * @Params task 任务单元指针
 * @Params matrix 矩阵指针
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_get_matrix(pJLGPUTaskUnit_t task, gpu_matrix_t *matrix);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_set_matrix 设置矩阵到gpu任务里
 *
 * @Params task 任务单元指针
 * @Params matrix 矩阵指针
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_set_matrix(pJLGPUTaskUnit_t task, gpu_matrix_t *matrix);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlgpu_set_texture_big_end 设置纹理的大小端排列(只支持bpp<8 or RGB565格式)
 *
 * @Params task 任务单元指针
 * @Params big_end 字节顺序
 */
/* ------------------------------------------------------------------------------------*/
void jlgpu_set_texture_big_end(pJLGPUTaskUnit_t task, int big_end);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_set_alloc_cb 注册cache申请数据的接口，不注册时默认使用psram
 *
 * @param malloc
 * @param free
 */
/* ------------------------------------------------------------------------------------*/
void gpu_input_stream_cache_set_alloc_cb(void *(*malloc)(int size, u32 ram_type, u32 module_type), void (*free)(void *buf, u32 ram_type, u32 module_type));
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_init 初始化gpu cache 模块
 *
 * @param read_cb			读数据回调
 * @param free_cache		释放cache数据回调
 */
/* ------------------------------------------------------------------------------------*/
void  gpu_input_stream_cache_init(void (*read_cb)(void *priv, void *dst, void *src, int len), int (*free_cache)(void *cache));
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_free 释放cache模块句柄，需要先清空数据再释放
 */
/* ------------------------------------------------------------------------------------*/
void  gpu_input_stream_cache_free();
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_add 加载数据
 *
 * @param priv		私有句柄，传文件句柄、资源地址等
 * @param data		需要拷贝的数据地址
 * @param len		拷贝数据长度
 * @param check		数据特征值，可以是crc、hash，或者其他规则
 * @param mmu_tb	是否使用flash_mmu
 * @param mmu_size	flash_mmu大小
 *
 * @param note		vaild =  BIT(1)|BIT(0)
 * @return
 */
/* ------------------------------------------------------------------------------------*/
void *gpu_input_stream_cache_add(void *priv, void *data, int len, u32 check, void *mmu_tb, int mmu_size);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_add_with_callback 支持数据私有回调
 *
 * @param priv
 * @param data
 * @param len
 * @param check
 * @param mmu_tb
 * @param mmu_size
 * @param read_cb
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
void *gpu_input_stream_cache_add_with_callback(void *priv, void *data, int len, u32 check, void *mmu_tb, int mmu_size, void (*read_cb)(void *priv, void *dst, void *src, int len));
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_del 清cache数据
 *
 * @param cache_addr	cache地址
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int   gpu_input_stream_cache_del(void *cache_addr);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_check 通过特征值check找资源地址
 *
 * @param check
 *
 * @return 返回cache资源地址，如果资源不存在则返回NULL
 */
/* ------------------------------------------------------------------------------------*/
void *gpu_input_stream_cache_check(u32 check);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_realloc 更新cache缓存buf的大小
 *
 * 注意：这里是重新申请一个指定大小的buf，原来的buf会被释放掉
 *
 * @Params size 新buf需要的大小
 * @Params check 校验值
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
void *gpu_input_stream_cache_realloc(int size, u32 check);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_invaild_all_by_index 将所有数据的index标记为无效（按bit管理）
 *
 * @param index 0~7		all data vaild &= ~BIT(index)
 *
 * @return

 */
/* ------------------------------------------------------------------------------------*/
int gpu_input_stream_cache_invaild_all_by_index(u8 index);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_vaild_value_set_by_index 设置有效值(按数值管理)vaild = 0xff不自减
 *
 * @param cache_addr
 * @param value 	有效值
 *
 * @return
 *
 * @note  gpu_input_stream_cache_vaild_set_by_index作废
 */
/* ------------------------------------------------------------------------------------*/
int gpu_input_stream_cache_vaild_value_set_by_index(u32 cache_addr, u8 value);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_vaild_sub 所有vaild自减（按数值管理）,vaild = 0xff不自减
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int gpu_input_stream_cache_vaild_sub();
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_vaild_add_by_addr 有效值自增（按数值管理）
 *
 * @param cache_addr
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int gpu_input_stream_cache_vaild_add_by_addr(u32 cache_addr);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_clr_all_invaild  将所有vaild 为0的数据清除
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int gpu_input_stream_cache_clr_all_invaild();
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_clr_non_resident 清空非常驻资源(有效值非0也会清除)
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int gpu_input_stream_cache_clr_non_resident();
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_clr_resident 清空常驻资源(有效值非0也会清除)
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int gpu_input_stream_cache_clr_resident();
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_reset 清空cache数据(常驻非常驻资源均清空)
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int gpu_input_stream_cache_reset();

/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_set_index
 * 设置资源归宿索引，用于多链表管理
 * 与 gpu_input_stream_cache_vaild_add_by_addr 联用
 *
 * @param cache_addr
 * @param index：0~255
 *
 * @return
 *
 * @default index = 0，没有设置时，资源索引为0
 */
/* ------------------------------------------------------------------------------------*/
int gpu_input_stream_cache_set_index(u32 cache_addr, u8 index);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_input_stream_cache_vaild_sub_by_index
 * 按资源index，将有效值-1，vaild= 0xff的资源不自减
 * 需要先设置资源索引 gpu_input_stream_cache_set_index
 *
 * @
 *
 * @param index: 0~255
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int gpu_input_stream_cache_vaild_sub_by_index(u8 index);


#endif






