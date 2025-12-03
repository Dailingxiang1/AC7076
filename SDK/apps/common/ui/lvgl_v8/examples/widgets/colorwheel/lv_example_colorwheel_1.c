#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lv_ui_core.data.bss")
#pragma data_seg(".lv_ui_core.data")
#pragma const_seg(".lv_ui_core.text.const")
#pragma code_seg(".lv_ui_core.text")
#endif
#include "../../lv_examples.h"
#if LV_USE_COLORWHEEL && LV_BUILD_EXAMPLES

void lv_example_colorwheel_1(void)
{
    lv_obj_t *cw;

    cw = lv_colorwheel_create(lv_scr_act(), true);
    lv_obj_set_size(cw, 200, 200);
    lv_obj_center(cw);
}

#endif
