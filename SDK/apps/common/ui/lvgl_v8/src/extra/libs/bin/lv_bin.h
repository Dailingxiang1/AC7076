
/**
 * @file lv_bin.h
 *
 */

#ifndef LV_BIN_H_
#define LV_BIN_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
*      INCLUDES
*********************/

/*********************
 *      DEFINES
 *********************/
enum {
    LV_PACK_MODE_UNINIT = 0,
    LV_PACK_MODE_TIGHT,
    LV_PACK_MODE_UNTIGHT,
};
/**********************
 *      TYPEDEFS
 **********************/
/*******************JL************************/
typedef union _JLBinResourceHeaderNop {
    struct {
        uint32_t block : 2;
        uint32_t nop: 30;
    } image;
    uint32_t nop;
} JLBinResourceHeaderNop;
typedef struct {
    uint8_t res_type;
    uint8_t res_compress;
    uint16_t res_crc;
    uint32_t res_size;
    //uint32_t nop;
    JLBinResourceHeaderNop nop;
} JLBinResourceHeader;

typedef struct {
    char JL[2];
    uint16_t version;
    JLBinResourceHeader jl_header;
    void *resource;
} JLBinResource;

typedef struct {
    uint8_t first_byte;
    uint8_t second_byte;
} JL_get_type;

extern const char *const lv_src_base_file;
/*******************JL************************/
/**********************
 * GLOBAL PROTOTYPES
 **********************/

/*JL获取.bin文件位图数据接口*/
//get lv_img_dsc_t结构体数据
lv_res_t lv_get_img_dsc_bin(const char *src, lv_img_dsc_t *bin_dsc);
lv_res_t lv_get_img_dsc_bin_info(const char *src, lv_img_dsc_t *bin_dsc);
lv_res_t lv_get_JLBinResource(const char *src, JLBinResource *bin_res);
lv_img_src_t lv_get_img_src_type(const char *src);
uint8_t lv_get_compress_type(const void *src);
uint8_t lv_get_res_block(const void *src);
uint32_t lv_get_flash_src_addr(const char *src);
uint8_t lv_get_ui_pack_mode(void);
void lv_ui_src_pack_mode_init(void);
bool lv_get_jl_src_variable(const void *src);
/**********************
 *      MACROS
 **********************/




#ifdef __cplusplus
}
#endif

#endif
