#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lv_ui_core.data.bss")
#pragma data_seg(".lv_ui_core.data")
#pragma const_seg(".lv_ui_core.text.const")
#pragma code_seg(".lv_ui_core.text")
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "../../lvgl.h"
#endif

#ifndef LV_FONT_MONTSERRAT_NONE
#define LV_FONT_MONTSERRAT_NONE 1
#endif

#if LV_FONT_MONTSERRAT_NONE


static bool _lv_font_get_glyph_dsc_fmt_txt(const lv_font_t *font, lv_font_glyph_dsc_t *dsc_out, uint32_t unicode_letter,
        uint32_t unicode_letter_next)
{
    return false;
}

static const uint8_t *_lv_font_get_bitmap_fmt_txt(const lv_font_t *font, uint32_t unicode_letter)
{
    return NULL;
}

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t lv_font_montserrat_none = {
#else
lv_font_t lv_font_montserrat_none = {
#endif
    .get_glyph_dsc = _lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = _lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = NULL  /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};

#endif /*#if LV_FONT_MONTSERRAT_NONE*/

