#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lv_ui_core.data.bss")
#pragma data_seg(".lv_ui_core.data")
#pragma const_seg(".lv_ui_core.text.const")
#pragma code_seg(".lv_ui_core.text")
#endif
#include "lvgl.h"

void lvgl_font_loader_test(void)
{
    lv_obj_t *container = lv_obj_create(lv_scr_act());

    lv_obj_set_size(container, LV_PCT(100), LV_PCT(100));
    lv_obj_align(container,  LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *label = lv_label_create(container);
    lv_obj_set_size(label, LV_PCT(100), LV_PCT(100));
    lv_obj_align(label,  LV_ALIGN_CENTER, 100, 100);

    extern lv_font_t *lv_font_load_bin(const char *font_name);
    lv_font_t *font_bin = lv_font_load_bin("font6.bin");
    lv_obj_set_style_text_font(label, font_bin, LV_PART_MAIN);//

    lv_label_set_text(label, "lv_load_from_flash_test");

    /*extern void lv_font_free_bin(lv_font_t *font); */
    /*lv_font_free_bin(font_bin);//free font*/
}
