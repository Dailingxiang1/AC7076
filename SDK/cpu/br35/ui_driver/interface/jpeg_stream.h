#ifndef __jpeg_stream__
#define __jpeg_stream__

#include "app_config.h"
int jljpeg_stream_init();
int jljpeg_stream_deinit();

void jljpeg_stream_src_data_lock();
void jljpeg_stream_src_data_unlock();
int jljpeg_stream_src_data_lock_check();

int jljpeg_stream_src_data_copy(u8 *buf, int size);
int jljpeg_stream_src_data_len_get();
u8 *jljpeg_stream_src_data_get();

int jljpeg_stream_src_data_save_to_file(s8 *filename);

void jpeg_image_ram(struct draw_context *dc, int left, int top, int width, int height, u8 *addr, int len, int scale_en, float scale_f);
void jpeg_image_file(struct draw_context *dc, int left, int top, int width, int height, u8 *path, int path_len, int scale_en, float scale_f);
int jpeg_image_file_psram(struct draw_context *dc, int left, int top, int width, int height, char *path, int path_len, int scale_en, float scale_f);
#endif// __jpeg_stream__
