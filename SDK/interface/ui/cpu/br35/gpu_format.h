#ifndef __LIB_GPU_FORMAT_H__
#define __LIB_GPU_FORMAT_H__

int res_format_to_gpu(int res_format);
int gpu_format_to_res(int gpu_format);
int text_format_to_gpu(int res_format);
int gpu_format_to_text(int gpu_format);
int get_clut_format_tabsize(int res_format, int clut_format);
int gpu_format_to_lut_size(int gpu_format, int clut_format);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief texture_line_to_tile_base 纹理行排列转为块排列
 *
 * @Params dst 纹理输出(注意对齐)
 * @Params src 纹理输入
 * @Params src_w 纹理输入宽度
 * @Params src_h 纹理输入高度
 * @Params format 纹理格式，参考 GPU_format_t
 */
/* ------------------------------------------------------------------------------------*/
void texture_line_to_tile_base(void *dst, void *src, int src_w, int src_h, int format);

#endif


