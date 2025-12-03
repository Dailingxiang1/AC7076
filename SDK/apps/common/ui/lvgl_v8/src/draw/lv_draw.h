/**
 * @file lv_draw.h
 *
 */

#ifndef LV_DRAW_H
#define LV_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_conf_internal.h"

#include "../misc/lv_style.h"
#include "../misc/lv_txt.h"
#include "lv_img_decoder.h"
#include "lv_img_cache.h"

#include "lv_draw_rect.h"
#include "lv_draw_label.h"
#include "lv_draw_img.h"
#include "lv_draw_line.h"
#include "lv_draw_triangle.h"
#include "lv_draw_arc.h"
#include "lv_draw_mask.h"
#include "lv_draw_transform.h"
#include "lv_draw_layer.h"

// #if LV_USE_GPU_LIST_DRAW
#define BOOL_DEFINE_CONFLICT
#include "gpu_port.h"		// module head file
// #endif
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
#if LV_USE_GPU_LIST_DRAW

typedef struct lv_gpu_task_param {
    pJLGPUTaskParam_t param;
} lv_gpu_task_param_t;

typedef struct gpu_list_attach_buf gpu_list_attach_buf_t;
struct gpu_list_attach_buf {
    gpu_list_attach_buf_t *next;  // 指向下一个节点
    uint8_t *data;      // 分配的内存
    uint32_t data_size_bytes;      // 分配的内存大小
};

#endif


typedef struct {
    void *user_data;
} lv_draw_mask_t;

typedef struct _lv_draw_layer_ctx_t {
    lv_area_t area_full;
    lv_area_t area_act;
    lv_coord_t max_row_with_alpha;
    lv_coord_t max_row_with_no_alpha;
    void *buf;
    struct {
        const lv_area_t *clip_area;
        lv_area_t *buf_area;
        void *buf;
        bool screen_transp;
    } original;
} lv_draw_layer_ctx_t;

typedef struct _lv_draw_ctx_t  {
    /**
     *  Pointer to a buffer to draw into
     */
    void *buf;

    /**
     * The position and size of `buf` (absolute coordinates)
     */
    lv_area_t *buf_area;

    /**
     * The current clip area with absolute coordinates, always the same or smaller than `buf_area`
     */
    const lv_area_t *clip_area;

    void (*init_buf)(struct _lv_draw_ctx_t *draw_ctx);

    void (*draw_rect)(struct _lv_draw_ctx_t *draw_ctx, const lv_draw_rect_dsc_t *dsc, const lv_area_t *coords);

    void (*draw_arc)(struct _lv_draw_ctx_t *draw_ctx, const lv_draw_arc_dsc_t *dsc, const lv_point_t *center,
                     uint16_t radius,  uint16_t start_angle, uint16_t end_angle);

    void (*draw_img_decoded)(struct _lv_draw_ctx_t *draw_ctx, const lv_draw_img_dsc_t *dsc,
                             const lv_area_t *coords, const uint8_t *map_p, lv_img_cf_t color_format);

    lv_res_t (*draw_img)(struct _lv_draw_ctx_t *draw_ctx, const lv_draw_img_dsc_t *draw_dsc,
                         const lv_area_t *coords, const void *src);

    void (*draw_letter)(struct _lv_draw_ctx_t *draw_ctx, const lv_draw_label_dsc_t *dsc,  const lv_point_t *pos_p,
                        uint32_t letter);

    void (*draw_line)(struct _lv_draw_ctx_t *draw_ctx, const lv_draw_line_dsc_t *dsc, const lv_point_t *point1,
                      const lv_point_t *point2);

    void (*draw_polygon)(struct _lv_draw_ctx_t *draw_ctx, const lv_draw_rect_dsc_t *draw_dsc,
                         const lv_point_t *points, uint16_t point_cnt);

    /**
     * Get an area of a transformed image (zoomed and/or rotated)
     * @param draw_ctx      pointer to a draw context
     * @param dest_area     get this area of the result image. It assumes that the original image is placed to the 0;0 position.
     * @param src_buf       the source image
     * @param src_w         width of the source image in [px]
     * @param src_h         height of the source image in [px]
     * @param src_stride    the stride in [px].
     * @param draw_dsc      an `lv_draw_img_dsc_t` descriptor containing the transformation parameters
     * @param cf            the color format of `src_buf`
     * @param cbuf          place the colors of the pixels on `dest_area` here in RGB format
     * @param abuf          place the opacity of the pixels on `dest_area` here
     */
    void (*draw_transform)(struct _lv_draw_ctx_t *draw_ctx, const lv_area_t *dest_area, const void *src_buf,
                           lv_coord_t src_w, lv_coord_t src_h, lv_coord_t src_stride,
                           const lv_draw_img_dsc_t *draw_dsc, lv_img_cf_t cf, lv_color_t *cbuf, lv_opa_t *abuf);

    /**
     * Replace the buffer with a rect without decoration like radius or borders
     */
    void (*draw_bg)(struct _lv_draw_ctx_t *draw_ctx, const lv_draw_rect_dsc_t *draw_dsc, const lv_area_t *coords);

    /**
     * Wait until all background operations are finished. (E.g. GPU operations)
     */
    void (*wait_for_finish)(struct _lv_draw_ctx_t *draw_ctx);

    /**
     * Copy an area from buffer to an other
     * @param draw_ctx      pointer to a draw context
     * @param dest_buf      copy the buffer into this buffer
     * @param dest_stride   the width of the dest_buf in pixels
     * @param dest_area     the destination area
     * @param src_buf       copy from this buffer
     * @param src_stride    the width of src_buf in pixels
     * @param src_area      the source area.
     *
     * @note dest_area and src_area must have the same width and height
     *       but can have different x and y position.
     * @note dest_area and src_area must be clipped to the real dimensions of the buffers
     */
    void (*buffer_copy)(struct _lv_draw_ctx_t *draw_ctx, void *dest_buf, lv_coord_t dest_stride,
                        const lv_area_t *dest_area,
                        void *src_buf, lv_coord_t src_stride, const lv_area_t *src_area);

    /**
     * Initialize a new layer context.
     * The original buffer and area data are already saved from `draw_ctx` to `layer_ctx`
     * @param draw_ctx      pointer to the current draw context
     * @param layer_area    the coordinates of the layer
     * @param flags         OR-ed flags from @lv_draw_layer_flags_t
     * @return              pointer to the layer context, or NULL on error
     */
    struct _lv_draw_layer_ctx_t *(*layer_init)(struct _lv_draw_ctx_t *draw_ctx, struct _lv_draw_layer_ctx_t *layer_ctx,
            lv_draw_layer_flags_t flags);

    /**
     * Adjust the layer_ctx and/or draw_ctx based on the `layer_ctx->area_act`.
     * It's called only if flags has `LV_DRAW_LAYER_FLAG_CAN_SUBDIVIDE`
     * @param draw_ctx      pointer to the current draw context
     * @param layer_ctx     pointer to a layer context
     * @param flags         OR-ed flags from @lv_draw_layer_flags_t
     */
    void (*layer_adjust)(struct _lv_draw_ctx_t *draw_ctx, struct _lv_draw_layer_ctx_t *layer_ctx,
                         lv_draw_layer_flags_t flags);

    /**
     * Blend a rendered layer to `layer_ctx->area_act`
     * @param draw_ctx      pointer to the current draw context
     * @param layer_ctx     pointer to a layer context
     * @param draw_dsc      pointer to an image draw descriptor
     */
    void (*layer_blend)(struct _lv_draw_ctx_t *draw_ctx, struct _lv_draw_layer_ctx_t *layer_ctx,
                        const lv_draw_img_dsc_t *draw_dsc);

    /**
     * Destroy a layer context. The original buffer and area data of the `draw_ctx` will be restored
     * and the `layer_ctx` itself will be freed automatically.
     * @param draw_ctx      pointer to the current draw context
     * @param layer_ctx     pointer to a layer context
     */
    void (*layer_destroy)(struct _lv_draw_ctx_t *draw_ctx, lv_draw_layer_ctx_t *layer_ctx);

    /**
     * Size of a layer context in bytes.
     */
    size_t layer_instance_size;

#if LV_USE_USER_DATA
    void *user_data;
#endif

    //资源压缩类型
    uint8_t compress_type;
    //资源分块类型
    uint8_t res_block;

#if LV_USE_GPU_LIST_DRAW
    pJLGPUTaskHead_t gpu_task_head;	//GPU任务链头指针
    uint8_t gpu_task_node_num;      //GPU任务链节点数
    uint8_t gpu_task_list_task_id;  //GPU任务链节点id
    uint8_t suport_list_format;     //GPU任务链格式支持标志

    const lv_area_t *final_draw_area;   // 最终绘制的区域，GPU绘制动作有效时才有意义
    void *attachments;  // 当前绘制的附件资源：图片资源等
    gpu_list_attach_buf_t *gpu_list_attach_buf_head;
#endif

#if LV_FONT_MONTSERRAT_JL
    unsigned short letter_prev;
    unsigned short letter_next;
#endif
} lv_draw_ctx_t;





/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_init(void);

void lv_draw_wait_for_finish(lv_draw_ctx_t *draw_ctx);


#if LV_USE_GPU_LIST_DRAW

//创建任务链表:1+2
void lv_gpu_task_list_create(struct _lv_draw_ctx_t *draw_ctx, GPU_out_format_t gpu_out_format);

//1.初始化GPU任务管理模块
void lv_gpu_task_list_port_init(u16 gpu_win_width, u16 gpu_win_height);

//2.创建GPU任务链，并设置GPU任务链输出格式
void lv_gpu_task_list_init(struct _lv_draw_ctx_t *draw_ctx, GPU_out_format_t gpu_out_format);

//3.设置GPU任务链输出buf和绘制区域
void lv_gpu_task_list_set_out_buf(struct _lv_draw_ctx_t *draw_ctx, u8 *out_buf, struct rect *rect, int left, int stride);

//4.添加GPU任务链表节点
pJLGPUTaskUnit_t lv_gpu_add_task_node(struct _lv_draw_ctx_t *draw_ctx, lv_gpu_task_param_t src_param, TASK_TYPE task_type);

//5.执行gpu链表启动合成
void lv_gpu_task_list_run(struct _lv_draw_ctx_t *draw_ctx);

//6.打断GPU链表,将当前已经收集的 task 绘制完成
void lv_gpu_task_list_run_break(struct _lv_draw_ctx_t *draw_ctx);

//7.释放GPU任务链头以及GPU任务
void lv_gpu_task_list_free(struct _lv_draw_ctx_t *draw_ctx);

//添加附加资源节点
uint8_t *lv_gpu_attach_buf_add_node(struct _lv_draw_ctx_t *draw_ctx, uint8_t *src_buf, uint32_t buf_size);

//释放全部附加资源节点
void lv_gpu_attach_buf_list_free(struct _lv_draw_ctx_t *draw_ctx);


bool lv_jl_gpu2p5d_check_draw_ctx_buf(uint8_t *buf);

// void lv_gpu_task_list_blend(lv_disp_t *disp);
void lv_gpu_task_list_blend(void *_disp);

#endif

/**********************
 *  GLOBAL VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   POST INCLUDES
 *********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_H*/
