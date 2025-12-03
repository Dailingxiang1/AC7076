#ifndef LV_FONT_UNIC_H
#define LV_FONT_UNIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
/*********************
 *      DEFINES
 *********************/

#define LV_FONT_UNIC_FILE_PATH  "storage/res_nor_mode/C/lvgl/F_UNIC.PIX"

struct font_file {
    char *name;
    FILE *fd;
};

struct font {
    struct font_file file;
    u16 nbytes;
    u8 size;
    u8 *pixelbuf;
};

enum BIT_DEPTH {
    BIT_DEPTH_1BPP,
    BIT_DEPTH_2BPP,
    BIT_DEPTH_4BPP,
    BIT_DEPTH_8BPP,
};

struct font_info {
    struct font pixel;			//像素
    u8 bigendian;				//大端模式(unicode编码)
    u16 default_code;   //字库文件中不存在待显示字符时的默认替换字符编码
};

typedef struct {
    u32 width : 6;
    u32 height : 7;
    s32 top : 8;
    u32 size : 11;
    s32 left : 7;
    u32 addr : 25;
    u8 advance_x;
} __attribute__((packed, aligned(1))) UnicInfo_new;

typedef struct {
    u8 width;
    u8 height;
    s8 left;
    s8 top;
    u8 advance_x;
} __attribute__((packed, aligned(1))) UnicInfo;

enum lv_unic_type {
    LV_UNIC_ERR,
    LV_UNIC_THAI,
    LV_UNIC_INDIC,
    LV_UNIC_ARABIC,
    LV_UNIC_HEBREW,
    LV_UNIC_TIBETAN,
    LV_UNIC_MYANMAR,
    LV_UNIC_BENGALI,
    LV_UNIC_KHMER,
    LV_UNIC_OTHER
};

enum lv_myanmar_type {
    lv_myanmar_top,
    lv_myanmar_below,
    lv_myanmar_103c,
    lv_myanmar_half,
    lv_myanmar_follow,
    lv_myanmar_other,
};

typedef enum {
    LV_THAI_BASE_CODE,
    LV_THAI_ABOVE_CODE,
    LV_THAI_BELOW_CODE,
    LV_THAI_PHONETICS_CODE,
    LV_THAI_OTHER_CODE
} LV_THAI_CODE_TYPE;

typedef enum {
    lv_khmer_topl,
    lv_khmer_tops,
    lv_khmer_belowl,
    lv_khmer_belows,
    lv_khmer_normol
} LV_KHMER_CODE_TYPE;


u8 lv_font_get_bpp();

bool lv_font_init(const char *filename);

void lv_font_release();

u16 lv_font_mixrightword_update(u8 *str, u16 len);

u8 *lv_font_letter_bitmap_get(u16 letter);

void lv_font_letter_info_get(u16 letter, u16 letter_next, u16 letter_prev, short *info);

u16 lv_utf8_to_utf16(u8 *utf8_buf, u16 utf8_len, u16 *utf16_buf, u8 bigendian);

u16 lv_utf16_to_utf8(u8 *utf16, u16 utf16_len, u8 *utf8, u8 bigendian);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_FONT_UNIC_H*/
