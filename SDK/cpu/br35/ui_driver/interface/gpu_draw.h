
#ifndef __PUBLIC_GPU_DRAW_H__
#define __PUBLIC_GPU_DRAW_H__


void jlui_gpu_blend_init();
int jdec_to_jlgpu_texture_blend(u8 *dst_buf, struct rect *dst_rect, u8 *src_buf, struct rect *src_rect, struct rect *draw_rect, int src_stride, int format, void *p, int line_surpule, struct rect *cover);

#endif
