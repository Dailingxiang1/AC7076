#include "typedef.h"
#include "rect.h"
#include "res/resfile.h"

#include "dbi.h"
#include "jlgpu_math.h"
#include "jlgpu_driver.h"
#include "ui_resource.h"	// RES格式与GPU格式互转接口
#include "ui_expand/ui_expand.h"	// 使用一些宏定义

#include "gpu_draw.h"
#include "gpu_port.h"
#include "ui_draw/ui_basic.h"
#include <math.h>

#include "jlui_app/ui_style.h"

#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE) || defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))


/* debug 打印配置 */
#define LOG_TAG_CONST		JL_GPU
#define LOG_TAG             "[JL GPU]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"
extern int ui_disp_out_stride_get();
extern int lcd_get_screen_width();
extern int lcd_get_screen_height();
static void jlui_blend_by_gpu(u8 *mask_buf, struct rect *mask_r, u8 *disp_buf, struct rect *disp_r, int color);


static const struct gpu_blend_interface gpu_draw_interface = {
    .gpu_blend = jlui_blend_by_gpu,

};

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlui_blend_by_gpu gpu绘图
 *
 * @param mask_buf		非压缩A8数据（前景）
 * @param mask_r		A8数据绘制区域
 * @param disp_buf		底图，一般是屏驱buf（后景）
 * @param disp_r		底图区域
 * @param color			A8颜色，这里传565
 */
/* ------------------------------------------------------------------------------------*/
AT_UI_RAM
void jlui_blend_mask(u8 *mask_buf, struct rect *mask_r, u8 *disp_buf, struct rect *disp_r, struct rect *draw_r, u32 color, float ratio_w, float ratio_h, int in_format)
{

    int r = ((color >> 11) & 0x1f) << 3;
    int g = ((color >> 5) & 0x3f) << 2;
    int b = ((color) & 0x1f) << 3;
    /* printf("%s (%d %d %d %d) (%d %d %d %d) %x %x (%d %d %d)", __func__, */
    /* mask_r->left, mask_r->top, mask_r->width, mask_r->height, */
    /* disp_r->left, disp_r->top, disp_r->width, disp_r->height, */
    /* (int)mask_buf, (int)disp_buf, r, g, b); */

    gpu_out_params_t out_param = { 0 };
    gpu_basic_params_t basic_param = { 0 };
    gpu_transform_params_t transform_param = {0};

    out_param.data = (uint8_t *)disp_buf;
    out_param.win_x_min = disp_r->left;
    out_param.win_x_max = disp_r->left + disp_r->width ;
    out_param.win_y_min = disp_r->top;
    out_param.win_y_max = disp_r->top +  disp_r->height ;
    out_param.stride = lcd_get_screen_width() * 2; //disp_r->width * 2;
    out_param.format = GPU_FORMAT_RGB565;

    /*********针对draw_r的区域超出屏幕区域的情况*****/
    struct rect lcd_rect;
    lcd_rect.left = 0;
    lcd_rect.top = 0;
    lcd_rect.width = lcd_get_screen_width();
    lcd_rect.height = lcd_get_screen_height();
    struct rect draw_rect;
    get_rect_cover(&lcd_rect, draw_r, &draw_rect);
    /************************************************/

    /* basic_param.act_x_min = mask_r->left ; */
    /* basic_param.act_x_max = mask_r->left + mask_r->width; */
    /* basic_param.act_y_min = mask_r->top ; */
    /* basic_param.act_y_max = mask_r->top + mask_r->height; */
    basic_param.act_x_min = draw_rect.left ;
    basic_param.act_x_max = draw_rect.left + draw_rect.width ;
    basic_param.act_y_min = draw_rect.top ;
    basic_param.act_y_max = draw_rect.top + draw_rect.height;
    /* printf("basic:{%d %d %d %d}", basic_param.act_x_min, basic_param.act_x_max, basic_param.act_y_min, basic_param.act_y_max); */
    basic_param.alpha = 255;
    basic_param.red = r;
    basic_param.green = g;
    basic_param.blue = b;
    basic_param.layer_en = 1;
    basic_param.blend_mode = 1;
    basic_param.global_alpha = 128; //范围:0-128
    basic_param.premult  = 0;

    gpu_texture_params_t texture_param = { 0 };
    texture_param.adr_mode = 1;
    texture_param.data = mask_buf;
    texture_param.format = in_format;
    texture_param.compress_mode = 0;
    texture_param.stride = (mask_r->width * gpu_get_format_bpp(in_format) + 7) / 8 ;
    texture_param.compress_size = mask_r->width;
    texture_param.big_end = 1;

    float zoom_w = (float)1 / ratio_w;
    float zoom_h = (float)1 / ratio_h;
    gpu_matrix_t matrix;
    gpu_matrix_set_identity(&matrix);
    float tran_x = (float)draw_r->left;
    float tran_y = (float)draw_r->top;

    gpu_matrix_translate(&matrix, -tran_x, -tran_y);
    gpu_matrix_scale(&matrix, zoom_w, zoom_h);

    /* printf("%s %f %f %f %f\n", __func__, zoom_w, zoom_h, tran_x, tran_y); */
    transform_param.M00 = matrix.m[0][0];
    transform_param.M01 = matrix.m[0][1];
    transform_param.M02 = matrix.m[0][2] - 0.5f * transform_param.M00 - 0.5f * transform_param.M01 + 0.5f;
    transform_param.M10 = matrix.m[1][0];
    transform_param.M11 = matrix.m[1][1];
    transform_param.M12 = matrix.m[1][2] - 0.5f * transform_param.M10 - 0.5f * transform_param.M11 + 0.5f;
    transform_param.fg_x_min = 0;
    transform_param.fg_x_max = mask_r->width;
    transform_param.fg_y_min = 0;
    transform_param.fg_y_max = mask_r->height;
    transform_param.sample_mode = 0;
    transform_param.shift_sel = 6;
    uint8_t mode = GPU_TEXTURE_AFFINE;
    gpu_reset_all_regs(mode);
    gpu_texture_affine(&basic_param, &texture_param, &transform_param);
    gpu_set_out_layer(&out_param);
    gpu_isr_sem_clr();
    gpu_run();
    gpu_wait_done();

    /* printf("%s %d", __func__, __LINE__); */

}



/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlui_blend_by_gpu gpu绘图
 *
 * @param mask_buf		非压缩A8数据（前景）
 * @param mask_r		A8数据绘制区域
 * @param disp_buf		底图，一般是屏驱buf（后景）
 * @param disp_r		底图区域
 * @param color			A8颜色，这里传565
 */
/* ------------------------------------------------------------------------------------*/
extern const int JLUI_CUSTOM_DRAW_CACHE;
AT_UI_RAM
static void jlui_blend_by_gpu(u8 *mask_buf, struct rect *mask_r, u8 *disp_buf, struct rect *disp_r, int color)
{
    if (JLUI_CUSTOM_DRAW_CACHE) {
        /* memcpy(disp_buf, mask_buf, disp_r->width * disp_r->height); // 一定是A8格式，直接copy */
        return;
    }

    u8 r = ((color >> 11) & 0x1f) << 3;
    u8 g = ((color >> 5) & 0x3f) << 2;
    u8 b = ((color) & 0x1f) << 3;
    u8 alpha = ((u32)color) >> 24;
    /* printf("%s (%d %d %d %d) (%d %d %d %d) %x %x (%d %d %d)", __func__, */
    /* mask_r->left, mask_r->top, mask_r->width, mask_r->height, */
    /* disp_r->left, disp_r->top, disp_r->width, disp_r->height, */
    /* (int)mask_buf, (int)disp_buf, r, g, b); */
    /* put_buf(mask_buf,320*mask_r->height); */

    gpu_out_params_t out_param = { 0 };
    gpu_basic_params_t basic_param = { 0 };


    out_param.data = (uint8_t *)disp_buf;
    out_param.win_x_min = 0;
    out_param.win_x_max = disp_r->width;
    out_param.win_y_min = 0;
    out_param.win_y_max = disp_r->height;
    out_param.stride = ui_disp_out_stride_get(); // disp_r->width*2;//lcd_get_screen_width() * 2;
    out_param.format = GPU_FORMAT_RGB565;

    basic_param.act_x_min = mask_r->left;
    basic_param.act_x_max = mask_r->left + mask_r->width;
    basic_param.act_y_min = mask_r->top;
    basic_param.act_y_max = mask_r->top + mask_r->height;

    basic_param.alpha = 255;
    basic_param.red = r;
    basic_param.green = g;
    basic_param.blue = b;
    basic_param.layer_en = 1;
    basic_param.blend_mode = 1;
    basic_param.global_alpha = alpha * 128 / 255; //范围:0-128
#if 0
    uint8_t mode =  GPU_TEXTURE;
    gpu_texture_params_t texture_param = { 0 };
    texture_param.adr_mode = 1;
    texture_param.data = mask_buf;
    texture_param.format = GPU_FORMAT_A8;
    texture_param.compress_mode = 0;
    texture_param.stride = (mask_r->width * gpu_get_format_bpp(GPU_FORMAT_A8) + 7) / 8 ;
    texture_param.compress_size = mask_r->width;
    texture_param.big_end = 1;


    gpu_reset_all_regs(mode);
    gpu_texture(&basic_param, &texture_param);
    gpu_set_out_layer(&out_param);
    gpu_isr_sem_clr();
    gpu_run();
    gpu_wait_done();
#else
    uint8_t mode =  GPU_FILL_MASK;
    gpu_mask_params_t mask_param = {0};
    mask_param.data = mask_buf;
    mask_param.mask_big_end = 0;
    mask_param.mask_format = GPU_MASK_FORMAT_A8;
    mask_param.mask_x_min = 0;
    mask_param.mask_x_max = mask_r->width;
    mask_param.mask_y_min = 0;
    mask_param.mask_y_max = mask_r->height;
    mask_param.stride = (mask_r->width * gpu_get_format_bpp(GPU_FORMAT_A8) + 7) / 8 ;

    gpu_reset_all_regs(mode);
    gpu_fill_mask(&basic_param, &mask_param);
    gpu_set_out_layer(&out_param);
    gpu_isr_sem_clr();
    gpu_run();
    gpu_wait_done();
#endif

}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jdec_to_jlgpu_texture_blend 硬件 GPU2D YUV422 格式 UYVY 转 RGB565
 *
 * @param dst_buf		输出buffer，一般为推屏buffer
 * @param dst_rect		输出buffer位置
 * @param src_buf		jpeg图像数据
 * @param src_rect		图像数据位置（相对于jpeg）
 * @param draw_rect		绘制区域
 * @param stride		行步进
 * @param format		预留
 * @param p				私有句柄，预留给变换传参

 * @return 0 linebuf已完全绘制 1linebuf未完全绘制
 * @return linebuf使用后剩余行数
 */
/* ------------------------------------------------------------------------------------*/
#define GPU_OFFSET 800
void dump_matrix(gpu_matrix_t *matrix);
int jdec_to_jlgpu_texture_blend(u8 *dst_buf, struct rect *dst_rect, u8 *src_buf, struct rect *src_rect, struct rect *draw_rect, int src_stride, int format, void *p, int line_surpule, struct rect *cover_rect)
{
    //缩放矩阵
    gpu_matrix_t tmp_matrix;
    gpu_matrix_t *matrix = NULL;
    if (p) {
        matrix = (gpu_matrix_t *)&tmp_matrix;
        memcpy(matrix, p, sizeof(gpu_matrix_t));
    } else {
        matrix = (gpu_matrix_t *)&tmp_matrix;
        gpu_matrix_set_identity(matrix);
    }
    //缩放系数
    float ratio_w = 1.0f, ratio_h = 1.0f; //倒数
    if (matrix) {
        ratio_w = matrix->m[0][0];
        ratio_h = matrix->m[1][1];
    }
    //区域计算
    /* DUMP_RECT(__func__, __LINE__, "dst_rect", dst_rect);			//disp_buf区域 */
    /* DUMP_RECT(__func__, __LINE__, "src_rect", src_rect);			//jpeg解码区域（缩放前，局部） */
    /* DUMP_RECT(__func__, __LINE__, "draw_rect", draw_rect);		//jpeg显示区域（缩放后,局部） */
    /* DUMP_RECT(__func__, __LINE__, "cover_rect", cover_rect);		//jpeg限制区域 （全局） */

    //计算缩放后的尺寸
    struct rect jpeg_dec_scale_r;
    jpeg_dec_scale_r.left = src_rect->left / ratio_w;
    jpeg_dec_scale_r.top = ((float)src_rect->top / ratio_h);
    jpeg_dec_scale_r.width = src_rect->width / ratio_w;
    jpeg_dec_scale_r.height = (int)(src_rect->top + src_rect->height) / ratio_h - jpeg_dec_scale_r.top;
    /* DUMP_RECT(__func__, __LINE__, "jpeg_dec_scale_r", (struct rect *)(&jpeg_dec_scale_r));	//缩放后的区域 */

    //偏移到显示位置
    struct rect jpeg_dec_draw;
    jpeg_dec_draw.left = draw_rect->left + jpeg_dec_scale_r.left;
    jpeg_dec_draw.top = draw_rect->top + jpeg_dec_scale_r.top;
    jpeg_dec_draw.width = jpeg_dec_scale_r.width;
    jpeg_dec_draw.height = jpeg_dec_scale_r.height;
    /* DUMP_RECT(__func__, __LINE__, "jpeg_dec_draw", (struct rect *)(&jpeg_dec_draw));	//缩放后在屏幕显示的区域 */

    //与推屏区域取交集
    struct rect jpeg_dec_draw_cover_t;
    if (!get_rect_cover(dst_rect, &jpeg_dec_draw, &jpeg_dec_draw_cover_t)) {
        return 0 ;
    }
    //对显示区域取交集
    struct rect jpeg_dec_draw_cover;
    get_rect_cover(&jpeg_dec_draw_cover_t, cover_rect, &jpeg_dec_draw_cover);
    /* DUMP_RECT(__func__, (jpeg_dec_draw_cover.top + jpeg_dec_draw_cover.height), "jpeg_dec_draw_cover", (struct rect *)(&jpeg_dec_draw_cover)); //与推屏buf的交集 */

    //输出配置
    int out_stride  = ui_disp_out_stride_get(); //RGB565
    gpu_out_params_t out_param = {0};
    gpu_basic_params_t basic_param = {0};
    gpu_texture_params_t texture_param = {0};
    gpu_transform_params_t transform_param = {0};
    uint8_t adr_mode = 1;
    uint8_t blend_mode = GPU_BLEND_SRC;
    uint8_t mode = GPU_TEXTURE;
    if (matrix) {//缩放
        mode = GPU_TEXTURE_AFFINE;
        out_param.data = (uint8_t *)dst_buf;
        out_param.win_x_min = GPU_OFFSET + dst_rect->left;
        out_param.win_x_max = GPU_OFFSET + dst_rect->left + dst_rect->width;
        out_param.win_y_min = GPU_OFFSET + dst_rect->top;
        out_param.win_y_max = GPU_OFFSET + dst_rect->top + dst_rect->height;
    } else {
        out_param.data = (uint8_t *)dst_buf + (jpeg_dec_draw_cover.top - dst_rect->top) * out_stride;
        out_param.win_x_min = GPU_OFFSET + dst_rect->left;
        out_param.win_x_max = GPU_OFFSET + dst_rect->left + dst_rect->width;
        out_param.win_y_min = GPU_OFFSET + jpeg_dec_draw_cover.top;
        out_param.win_y_max = GPU_OFFSET + jpeg_dec_draw_cover.top + jpeg_dec_draw_cover.height;
    }

    /* printf("@@@ win [%d, %d, %d, %d] h:%d \n", out_param.win_x_min, out_param.win_y_min, out_param.win_x_max - out_param.win_x_min, out_param.win_y_max - out_param.win_y_min, jpeg_dec_draw_cover.height); */
    out_param.stride = out_stride;
    out_param.format = GPU_FORMAT_RGB565;
    if (matrix) {
        basic_param.act_x_min = GPU_OFFSET + jpeg_dec_draw_cover.left;
        basic_param.act_x_max = GPU_OFFSET + jpeg_dec_draw_cover.left + jpeg_dec_draw_cover.width;
        basic_param.act_y_min = GPU_OFFSET + jpeg_dec_draw_cover.top;
        basic_param.act_y_max = GPU_OFFSET + jpeg_dec_draw_cover.top + jpeg_dec_draw_cover.height;
    } else {
        basic_param.act_x_min = GPU_OFFSET + draw_rect->left;
        basic_param.act_x_max = GPU_OFFSET + draw_rect->left + draw_rect->width;
        basic_param.act_y_min = GPU_OFFSET + jpeg_dec_draw.top;
        basic_param.act_y_max = GPU_OFFSET + jpeg_dec_draw.top + jpeg_dec_draw.height;
    }

    /* printf("@@@ box [%d, %d, %d, %d]\n", basic_param.act_x_min, basic_param.act_y_min, basic_param.act_x_max - basic_param.act_x_min, basic_param.act_y_max - basic_param.act_y_min); */
    basic_param.global_alpha = 128;
    basic_param.blend_mode = blend_mode;
    basic_param.premult = 0;
    basic_param.layer_en = 1;
    basic_param.ext_mode = 1;

    texture_param.adr_mode = adr_mode;
    texture_param.rbs = 0;  // must 0; YUV422 => UYVY
    texture_param.data = (uint8_t *)src_buf ;
    texture_param.format = GPU_FORMAT_YUV422_BT601;
    texture_param.compress_mode = 0;
    texture_param.stride = src_stride;
    texture_param.compress_size = src_rect->width;
    gpu_reset_all_regs(mode);

    if (matrix) {
        gpu_matrix_set_identity(matrix);
        gpu_matrix_translate(matrix, -GPU_OFFSET, -GPU_OFFSET);

        struct rect real_scale_r;
        memcpy(&real_scale_r, &jpeg_dec_draw, sizeof(struct rect));
        real_scale_r.height = (int)(src_rect->height) / ratio_h;
        struct rect real_scale_cover_t;
        get_rect_cover(dst_rect, &real_scale_r, &real_scale_cover_t);
        struct rect real_scale_cover;
        get_rect_cover(&real_scale_cover_t, cover_rect, &real_scale_cover);
        if ((jpeg_dec_draw_cover.top + jpeg_dec_draw_cover.height) == (real_scale_cover.top + real_scale_cover.height)) {
            gpu_matrix_translate(matrix, -draw_rect->left, - jpeg_dec_draw.top);
        } else	{
            //浮点补偿
            gpu_matrix_translate(matrix, -draw_rect->left, -(((float)src_rect->top / ratio_h) + draw_rect->top));
        }
        /* printf("%s matrix_offset:%f %f",__func__,matrix->m[0][2],matrix->m[1][2]); */
        gpu_matrix_scale(matrix, ratio_w, ratio_h);
        transform_param.M00 = matrix->m[0][0];
        transform_param.M01 = matrix->m[0][1];
        transform_param.M02 = matrix->m[0][2] - 0.5f * transform_param.M00 - 0.5f * transform_param.M01 + 0.5f;
        transform_param.M10 = matrix->m[1][0];
        transform_param.M11 = matrix->m[1][1];
        transform_param.M12 = matrix->m[1][2]/* - 0.5f * transform_param.M10 - 0.5f * transform_param.M11 + 0.5f*/;
        transform_param.fg_x_min = 0;
        transform_param.fg_x_max = src_rect->width;
        transform_param.fg_y_min = 0;
        transform_param.fg_y_max = src_rect->height;
        transform_param.sample_mode = 0;
        transform_param.shift_sel = 6;
        gpu_texture_affine(&basic_param, &texture_param, &transform_param);
    } else {
        gpu_texture(&basic_param, &texture_param);
    }
    gpu_set_out_layer(&out_param);
    gpu_isr_sem_clr();
    gpu_run();
    gpu_wait_done();

    return 0;
}

void jlui_argb8565_to_rgb565(u8 *out_buf, struct rect *out_r, u8 *in_buf, struct rect *in_r)
{
    gpu_out_params_t out_param = { 0 };
    gpu_basic_params_t basic_param = { 0 };
    gpu_transform_params_t transform_param = {0};
    //out info
    out_param.data = (uint8_t *)out_buf;
    out_param.win_x_min = 0;
    out_param.win_x_max = out_r->width ;
    out_param.win_y_min = 0;
    out_param.win_y_max = out_r->height ;
    out_param.format = GPU_FORMAT_RGB565;
    out_param.stride = (out_r->width * gpu_get_format_bpp(out_param.format) + 7) / 8;
    //in info
    basic_param.act_x_min = 0;
    basic_param.act_x_max = in_r->width;
    basic_param.act_y_min = 0;
    basic_param.act_y_max = in_r->height;
    basic_param.alpha = 0xff;
    basic_param.layer_en = 1;
    basic_param.blend_mode = 1;
    basic_param.global_alpha = 128; //范围:0-128
    basic_param.premult  = 1;
    //in texture
    gpu_texture_params_t texture_param = { 0 };
    texture_param.adr_mode = 1;
    texture_param.data = in_buf;
    texture_param.format = GPU_FORMAT_ARGB8565;
    texture_param.compress_mode = 0;
    texture_param.stride = (in_r->width * gpu_get_format_bpp(texture_param.format) + 7) / 8 ;
    texture_param.compress_size = in_r->width;

    uint8_t mode = GPU_TEXTURE;
    gpu_reset_all_regs(mode);
    gpu_texture(&basic_param, &texture_param);
    gpu_set_out_layer(&out_param);
    gpu_isr_sem_clr();
    gpu_run();
    gpu_wait_done();
}





void jlui_gpu_blend_init()
{
    gpu_blend_init((struct gpu_blend_interface *)&gpu_draw_interface);
}


#endif

