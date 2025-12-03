#ifndef __JLGPU_DRIVER_H__
#define __JLGPU_DRIVER_H__


#define GPU_PLATFORM_PC	0




#if GPU_PLATFORM_PC

#include <stdint.h>


#else

#include "asm/cpu.h"
#include "system/includes.h"
#include "jlgpu_math.h"

#endif


#define GPU_COLOR_RAMP_SPREAD_PAD         0x00000000U
#define GPU_COLOR_RAMP_SPREAD_REFLECT     0x00000001U
#define GPU_COLOR_RAMP_SPREAD_REPEAT      0x00000002U

#define GPU_BLEND_SRC             0x00000000U
#define GPU_BLEND_SRC_OVER        0x00000001U
#define GPU_BLEND_DST_OUT         0x00000002U
#define GPU_BLEND_DST_IN          0x00000003U
#define GPU_BLEND_MAX             0x00000004U
#define GPU_BLEND_DST_OVER        0x00000005U
#define GPU_BLEND_SRC_OUT         0x00000006U
#define GPU_BLEND_SRC_IN          0x00000007U

#define GPU_FILL                    0x00000000U
#define GPU_FILL_MASK               0x00000001U
#define GPU_FILL_AFFINE             0x00000002U
#define GPU_LINEGRAD                0x00000002U
#define GPU_FILL_PERSPECTIVE        0x00000003U
#define GPU_LINEGRAD_PERSPECTIVE    0x00000003U
#define GPU_TEXTURE                 0x00000004U
#define GPU_TEXTURE_MASK            0x00000005U
#define GPU_TEXTURE_AFFINE          0x00000006U
#define GPU_TEXTURE_PERSPECTIVE     0x00000007U

#define LINEGRAD(mode)				((mode) << 4)	// 线性渐变左移4bit，用于和FILL区分开

#define GPU_OUTPUT_WIN_MAX			0xFFFF	// uint16_t
#define GPU_BOUND_BOX_MAX			0x07FF	// uint32_t 11bit

#define GPU_API_CODE				AT(.gpu_api.text.cache.L2)

typedef enum {
    GPU_FORMAT_ARGB8888 = 0x00,
    GPU_FORMAT_ARGB8565 = 0x01, //out
    GPU_FORMAT_ARGB1555 = 0x02,
    GPU_FORMAT_ARGB4444 = 0x03,
    GPU_FORMAT_RGB888 = 0x04,
    GPU_FORMAT_RGB565 = 0x05, //out

    GPU_FORMAT_YUV422_BT601 = 0x06,
    GPU_FORMAT_YUV422_BT709 = 0x07,

    GPU_FORMAT_AL88 = 0x08,
    GPU_FORMAT_AL44 = 0x10,
    GPU_FORMAT_AL22 = 0x11,
    GPU_FORMAT_L8 = 0x12,
    GPU_FORMAT_L4 = 0x13,
    GPU_FORMAT_L2 = 0x14,
    GPU_FORMAT_L1 = 0x15,
    GPU_FORMAT_A8 = 0x16,//out
    GPU_FORMAT_A4 = 0x17,//out,
    GPU_FORMAT_A2 = 0x18,//out,
    GPU_FORMAT_A1 = 0x19,//out,
} GPU_format_t;

typedef enum {
    GPU_MASK_FORMAT_A8 = 0x00,
    GPU_MASK_FORMAT_A4 = 0x01,
    GPU_MASK_FORMAT_A2 = 0x02,
    GPU_MASK_FORMAT_A1 = 0x03,
} GPU_mask_format_t;

typedef enum {
    GPU_OUT_FORMAT_ARGB8565 = 0x01, //out
    GPU_OUT_FORMAT_RGB565 = 0x05, //out
    GPU_OUT_FORMAT_A8 = 0x16,//out
    GPU_OUT_FORMAT_A4 = 0x17,//out,
    GPU_OUT_FORMAT_A2 = 0x18,//out,
    GPU_OUT_FORMAT_A1 = 0x19,//out,
} GPU_out_format_t;

typedef enum {
    GPU_CLUT_FORMAT_ARGB8888 = 0x00,
    GPU_CLUT_FORMAT_ARGB8565 = 0x01,
    GPU_CLUT_FORMAT_RGB888 = 0x02,
    GPU_CLUT_FORMAT_RGB565 = 0x03,
} GPU_clut_format_t;

// mmu table page size symbol define.
typedef enum {
    MMU_PAGE_4K = 0x00,
    MMU_PAGE_16K = 0x01,
    MMU_PAGE_32K = 0x02,
    MMU_PAGE_64K = 0x03,
} GPU_mmu_page_size_t;

// 状态定义
typedef enum {
    GPU_STA_IDLE = 0,
    GPU_STA_RUNNING,
    GPU_STA_BREAK,
    GPU_STA_PAUSE,
    GPU_STA_TIMEOUT,
} GPU_State;

typedef struct {
    uint8_t *data;
    uint32_t stride;
    uint8_t format;
    uint8_t rbs;
    uint8_t rgba;
    uint8_t endian;
    uint16_t win_x_min;
    uint16_t win_x_max;
    uint16_t win_y_min;
    uint16_t win_y_max;
    uint16_t offset_left;
} gpu_out_params_t;

typedef struct {
    uint8_t format;
    uint8_t compress_mode;
    uint8_t adr_mode;
    uint8_t big_end;
    uint8_t alpha_end;
    uint8_t rbs;
    uint8_t color_ext_mode;
    uint16_t stride;
    uint8_t *data;
    uint32_t compress_size;
} gpu_texture_params_t;


typedef struct {
    uint8_t *clut;
    uint8_t layer_en;
    uint8_t pad_mode;
    uint8_t trans_mode;
    uint8_t mask_en;
    uint8_t blend_mode;
    uint8_t colorkey_en;
    uint8_t dither_en;
    uint8_t clut_format;
    uint8_t premult;
    uint8_t ext_mode;
    uint8_t breakpoint_en;
    uint16_t act_x_min;
    uint16_t act_x_max;
    uint16_t act_y_min;
    uint16_t act_y_max;
    uint8_t global_alpha;
    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    //linegrad
    uint8_t spread_mode;
    uint8_t lut_lvl;
} gpu_basic_params_t;


typedef struct {
    uint8_t sample_mode;
    uint8_t shift_sel;
    float M00;
    float M01;
    float M02;
    float M10;
    float M11;
    float M12;
    float M20;
    float M21;
    float M22;
    uint8_t coeff0;
    uint8_t coeff1;
    uint8_t coeff2;
    uint8_t coeff3;
    uint8_t coeff4;
    uint8_t coeff5;
    uint16_t fg_x_min;
    uint16_t fg_x_max;
    uint16_t fg_y_min;
    uint16_t fg_y_max;
} gpu_transform_params_t;

typedef struct {
    uint8_t *data;
    uint32_t stride;
    uint8_t mask_inv;
    uint8_t mask_big_end;
    uint8_t mask_format;
    uint16_t mask_x_min;
    uint16_t mask_x_max;
    uint16_t mask_y_min;
    uint16_t mask_y_max;
} gpu_mask_params_t; //add

void gpu_set_sfr(uint8_t *param, uint8_t *out_data);
void gpu_set_out_layer(gpu_out_params_t *param);
void gpu_set_texture_mmu(uint8_t *mmu_tb_base, uint8_t mmu_en, uint8_t mmu_page_size, uint8_t mode);
int gpu_init();
int gpu_free();
int gpu_run();
int gpu_wait_done();

void gpu_reset_all_regs(uint8_t mode);
uint32_t gpu_get_format_bpp(uint32_t format);
uint32_t gpu_get_mask_format_bpp(uint32_t format);
void gpu_computeLinearParameters(int x0, int y0, int x1, int y1, gpu_transform_params_t *transform_params);
void gpu_fill(gpu_basic_params_t *basic_params);
void gpu_fill_mask(gpu_basic_params_t *basic_params, gpu_mask_params_t *mask_params);
void gpu_fill_affine(gpu_basic_params_t *basic_params, gpu_transform_params_t *transform_params);
void gpu_lineargrad(gpu_basic_params_t *basic_params, gpu_transform_params_t *transform_params);
void gpu_fill_perspective(gpu_basic_params_t *basic_params, gpu_transform_params_t *transform_params);
void gpu_lineargrad_perspective(gpu_basic_params_t *basic_params, gpu_transform_params_t *transform_params);
void gpu_texture(gpu_basic_params_t *basic_params, gpu_texture_params_t *texture_params);
void gpu_texture_mask(gpu_basic_params_t *basic_params, gpu_texture_params_t *texture_params, gpu_mask_params_t *mask_params);
void gpu_texture_affine(gpu_basic_params_t *basic_params, gpu_texture_params_t *texture_params, gpu_transform_params_t *transform_params);
void gpu_texture_perspective(gpu_basic_params_t *basic_params, gpu_texture_params_t *texture_params, gpu_transform_params_t *transform_params);

void *gpu_get_current_task();
void *gpu_create_task(uint8_t mode);
void gpu_free_all_tasks();
void gpu_dumcp_all_tasks();

void gpu_fill(gpu_basic_params_t *basic_params);

void gpu_set_out_layer(gpu_out_params_t *param);

// 获取状态
GPU_State gpu_get_state(void);
int gpu_get_break_state(void);
//清除状态
int gpu_clr_break_state(u8 state);
int gpu_clr_break_state_all();
int gpu_isr_sem_clr();
uint32_t gpu_pack_pixel(uint8_t a, uint8_t r, uint8_t g, uint8_t b, uint32_t format);


typedef struct {
    int x, y, w, h;
} gpu_rectangle_t;


void gpu_rectangle_intersect(gpu_rectangle_t *dst, gpu_rectangle_t *src0, gpu_rectangle_t *src1);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_get_len 根据类型获取 GPU 任务所需的 BUF 长度
 *
 * @Params mode GPU 任务类型
 *
 * @return GPU 任务所需的 BUF 长度
 */
/* ------------------------------------------------------------------------------------*/
uint32_t gpu_task_get_len(uint8_t mode);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_add_to_list GPU 任务追加到任务链末尾
 *
 * @Params base_task GPU 任务链起始地址
 * @Params new_task 待追加的 GPU 任务
 *
 * @return 待追加的 GPU 任务
 */
/* ------------------------------------------------------------------------------------*/
void *gpu_task_add_to_list(void *base_task, void *new_task);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_add_to_next 在指定任务下追加下一个任务
 *
 * @Params curr_task 指定任务
 * @Params next_task 下一个任务
 *
 * @return 下一个任务
 */
/* ------------------------------------------------------------------------------------*/
void *gpu_task_add_to_next(void *curr_task, void *next_task);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_have_next 判断指定任务是否有下一个任务
 *
 * @Params curr_task 指定任务
 *
 * @return 0 无下一个任务，1 有下一个任务
 */
/* ------------------------------------------------------------------------------------*/
int gpu_task_have_next(void *curr_task);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_get_next 获取指定任务的下一个任务
 *
 * @Params curr_task 指定任务
 *
 * @return NULL 指定任务无下一个任务，其他 指定任务的下一个任务
 */
/* ------------------------------------------------------------------------------------*/
void *gpu_task_get_next(void *curr_task);



/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_del_next 删除指定任务的下一个任务
 *
 * @Params curr_task 指定任务
 *
 * @return NULL 指定任务无下一个任务，其他 指定任务原来的下一个任务
 */
/* ------------------------------------------------------------------------------------*/
void *gpu_task_del_next(void *curr_task);



/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_mode 设置指定任务的类型
 *
 * @Params taskp 任务指针
 * @Params mode 任务类型
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_mode(void *taskp, uint8_t mode);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_get_mode 获取指定任务的类型
 *
 * @Params task_p 任务指针
 *
 * @return 任务类型
 */
/* ------------------------------------------------------------------------------------*/
uint8_t gpu_task_get_mode(void *task_p);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_enable 设置GPU任务使能开关
 *
 * @Params task_p 任务指针
 * @Params enable  1 使能，0 不使能
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_enable(void *task_p, u32 enable);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_get_enable 获取指定任务的使能标志
 *
 * @Params task_p 任务指针
 *
 * @return 1 任务使能，0 任务未使能
 */
/* ------------------------------------------------------------------------------------*/
int gpu_task_get_enable(void *task_p);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_breakpoint_enable 设置断点使能
 *
 * @param task_p
 * @param enable
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_breakpoint_enable(void *task_p, u32 enable);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_get_bbox 获取GPU任务的绘图区域
 *
 * @Params task_p 任务指针
 * @Params xmin x最小值，输出
 * @Params xmax x最大值，输出
 * @Params ymin y最小值，输出
 * @Params ymax y最大值，输出
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_get_bbox(void *task_p, u16 *xmin, u16 *xmax, u16 *ymin, u16 *ymax);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_fill 设置 GPU_FILL 任务参数
 *
 * @Params task_p 任务指针
 * @Params basic_params gpu_basic_params_t
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_fill(void *task_p, gpu_basic_params_t *basic_params);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_fill_mask 设置 GPU_FILL_MASK 任务参数
 *
 * @Params task_p 任务指针
 * @Params basic_params gpu_basic_params_t
 * @Params mask_params gpu_mask_params_t
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_fill_mask(void *task_p, gpu_basic_params_t *basic_params, gpu_mask_params_t *mask_params);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_fill_affine 设置 GPU_FILL_AFFINE 任务参数
 *
 * @Params task_p 任务指针
 * @Params basic_params gpu_basic_params_t
 * @Params transform_params gpu_transform_params_t
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_fill_affine(void *task_p, gpu_basic_params_t *basic_params, gpu_transform_params_t *transform_params);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_fill_perspective 设置 GPU_FILL_PERSPECTIVE 任务参数
 *
 * @Params task_p 任务指针
 * @Params basic_params gpu_basic_params_t
 * @Params transform_params gpu_transform_params_t
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_fill_perspective(void *task_p, gpu_basic_params_t *basic_params, gpu_transform_params_t *transform_params);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_lineargrad 设置 GPU_LINEGRAD 任务参数
 *
 * @Params task_p 任务指针
 * @Params basic_params gpu_basic_params_t
 * @Params transform_params gpu_transform_params_t
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_lineargrad(void *task_p, gpu_basic_params_t *basic_params, gpu_transform_params_t *transform_params);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_lineargrad_perspective 设置 GPU_LINEGRAD_PERSPECTIVE 任务参数
 *
 * @Params task_p 任务指针
 * @Params basic_params gpu_basic_params_t
 * @Params transform_params gpu_transform_params_t
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_lineargrad_perspective(void *task_p, gpu_basic_params_t *basic_params, gpu_transform_params_t *transform_params);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_texture 设置 GPU_TEXTURE 任务参数
 *
 * @Params task_p 任务指针
 * @Params basic_params gpu_basic_params_t
 * @Params texture_params gpu_texture_params_t
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_texture(void *task_p, gpu_basic_params_t *basic_params, gpu_texture_params_t *texture_params);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_texture_mask 设置 GPU_TEXTURE_MASK 任务参数
 *
 * @Params task_p 任务指针
 * @Params basic_params gpu_basic_params_t
 * @Params texture_params gpu_texture_params_t
 * @Params mask_params gpu_mask_params_t
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_texture_mask(void *task_p, gpu_basic_params_t *basic_params, gpu_texture_params_t *texture_params, gpu_mask_params_t *mask_params);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_texture_affine 设置 GPU_TEXTURE_AFFINE 任务参数
 *
 * @Params task_p 任务指针
 * @Params basic_params gpu_basic_params_t
 * @Params texture_params gpu_texture_params_t
 * @Params transform_params gpu_transform_params_t
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_texture_affine(void *task_p, gpu_basic_params_t *basic_params, gpu_texture_params_t *texture_params, gpu_transform_params_t *transform_params);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_texture_perspective 设置 GPU_TEXTURE_PERSPECTIVE 任务参数
 *
 * @Params task_p 任务指针
 * @Params basic_params gpu_basic_params_t
 * @Params texture_params gpu_texture_params_t
 * @Params transform_params gpu_transform_params_t
 */
/* ------------------------------------------------------------------------------------*/
void gpu_task_set_texture_perspective(void *task_p, gpu_basic_params_t *basic_params, gpu_texture_params_t *texture_params, gpu_transform_params_t *transform_params);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_set_texture_mmu 设置 GPU_TEXTURE 任务的MMU(文件table表)
 *
 * @Params task_p 任务指针
 * @Params mmu_tb_base mmu 起始地址，相当于第一个 page 的偏移地址
 * @Params mmu_en mmu 使能标志：1 使能，0 不使能
 * @Params mmu_page_size mmu page 大小，参考 GPU_mmu_page_size_t
 *
 * @return int 0 设置成功，-1 设置失败，任务没有 mmu 命令（任务类型错误）
 */
/* ------------------------------------------------------------------------------------*/
int gpu_task_set_texture_mmu(void *task_p, uint8_t *mmu_tb_base, uint8_t mmu_en, uint8_t mmu_page_size);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_get_next_task_by_reg 从GPU模块寄存器获取下一个GPU任务(非必要情况禁止使用)
 *
 * @return 下一个GPU任务指针
 */
/* ------------------------------------------------------------------------------------*/
void *gpu_get_next_task_by_reg();


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_get_curr_task_by_reg 从GPU模块寄存器获取当前的GPU任务(非必要情况禁止使用)
 *
 * @return 当前的CPU任务指针
 */
/* ------------------------------------------------------------------------------------*/
void *gpu_get_curr_task_by_reg();


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_task_list_run 运行 GPU 任务链
 *
 * @Params base_task_addr GPU 任务链首地址
 *
 * @return 0 运行成功，其他 运行失败
 */
/* ------------------------------------------------------------------------------------*/
int gpu_task_list_run(void *base_task_addr);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief gpu_dump_task_list 打印任务链中所有任务信息，用于 debug
 *
 * @Params base_task 任务链起始地址
 */
/* ------------------------------------------------------------------------------------*/
void gpu_dump_task_list(void *base_task);


void jlgpu_driver_get_matrix(void *p, gpu_matrix_t *matrix);
void jlgpu_driver_set_matrix(void *p, gpu_matrix_t *matrix);
void jlgpu_driver_set_bbox(void *p, gpu_boundbox_t *bbox);
void jlgpu_driver_get_bbox(void *p, gpu_boundbox_t *bbox);
uint8_t jlgpu_driver_mode(void *p);
void jlgpu_driver_set_next_inst_addr(void *p, u32 addr);
void jlgpu_driver_set_trans_mode(void *p, int trans_mode);
void jlgpu_driver_get_clip(void *p, int *clip_x_min, int *clip_x_max, int *clip_y_min, int *clip_y_max, int width, int height);
void jlgpu_driver_set_clip(void *p, int *clip_x_min, int *clip_x_max, int *clip_y_min, int *clip_y_max);
void jlgpu_driver_set_breakpoint(void *p, u32 enable);
void jlgpu_driver_get_texture_addr(void *p, u32 *mmu_en, u32 *mmu_tb_base, u32 *tex_base_adr, u32 *rle_limit_adr, u32 *tex_stride);
void jlgpu_driver_set_texture_addr(void *p, u32 *mmu_en, u32 *mmu_tb_base, u32 *tex_base_adr, u32 *rle_limit_adr, u32 *tex_stride);
int jlgpu_driver_get_format(void *p);
int jlgpu_driver_get_clut_format(void *p);
void *jlgpu_driver_get_clut_addr(void *p);
void jlgpu_driver_set_texture_big_end(void *p, int big_end);
#endif




