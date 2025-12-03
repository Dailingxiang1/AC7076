#ifndef LV_C2_H_
#define LV_C2_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_GPU_COMPRESS



/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/


typedef struct c2_info {
    uint32_t width;
    uint32_t height;
    uint16_t format;
// 0 : ARGB8888, 图像格式
// 1 : ARGB8565
// 2 : ARGB1555
// 3 : ARGB4444
// 4 : RGB888
// 5 : RGB565
// 6 : BT601
// 7 : BT709
// 8 : AL88
// 16 : AL44
// 17 : AL22
// 18 : L8
// 19 : L4
// 20 : L2
// 21 : L1
// 22 : A8
// 23 : A4
// 24 : A2
// 25 : A1
    uint32_t img_width;
    uint32_t img_height;
} c2_info_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
uint16_t lv_get_c2_stride(c2_info_t *c2_info);
void lv_c2_decompress_in_frame(uint8_t *input, uint8_t **output);
void lv_c2_decompress_read_line(uint8_t *input, uint8_t *output, int x, int y, int len);
/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
}
#endif

#endif



