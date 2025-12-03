
/**
 * @file lv_c1.h
 *
 */

#ifndef LV_C1_H_
#define LV_C1_H_

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
enum {
    ARGB8888 = 0,
    ARGB8565,
    ARGB1555,
    ARGB4444,
    RGB888,
    RGB565,
    AL88,
    AL44,
    L8,
    A8,
};

typedef struct c1_info {
    uint32_t width;
    uint32_t height;
    uint16_t format;    //可配置项
    // 0 : ARGB8888,
    // 1 : ARGB8565
    // 2 : ARGB1555
    // 3 : ARGB4444
    // 4 : RGB888
    // 5 : RGB565
    // 8 : AL88
    // 9 : AL44
    // 10 : L8
    // 11 : A8

    uint16_t block;  //可配置项
    // 0 : block_64x1
    // 1 : block_8x8
    // 2 : block_256x1
    // 3 : block_16x16

    uint16_t lut_format; //可配置项
    // 0 : ARGB8888
    // 1 : ARGB8565
    // 2 : RGB888
    // 3 : RGB565
    uint16_t nop; //可配置项

    uint32_t stride;		//图像每行像素字节
    uint32_t lut_addr;		//颜色表地址
    uint32_t lut_len;		//颜色表大小
    uint32_t addr;			//压缩数据地址
    uint32_t len;			//压缩数据长度
} c1_info_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_c1_init(void);
uint16_t lv_get_c1_stride(c1_info_t *c1_info, uint8_t *adr_mode);
/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
}
#endif

#endif
