#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lv_ui_core.data.bss")
#pragma data_seg(".lv_ui_core.data")
#pragma const_seg(".lv_ui_core.text.const")
#pragma code_seg(".lv_ui_core.text")
#endif
#include "../../lv_examples.h"
#if LV_USE_BMP && LV_BUILD_EXAMPLES

/**
 * Open a BMP file from a file
 */
void lv_example_bmp_1(void)
{
    lv_obj_t *img = lv_img_create(lv_scr_act());
    /* Assuming a File system is attached to letter 'A'
     * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
#if LV_COLOR_DEPTH == 32
    lv_img_set_src(img, "A:lvgl/examples/libs/bmp/example_32bit.bmp");
#elif LV_COLOR_DEPTH == 16
    lv_img_set_src(img, "A:lvgl/examples/libs/bmp/example_16bit.bmp");
#endif
    lv_obj_center(img);

}

#endif
