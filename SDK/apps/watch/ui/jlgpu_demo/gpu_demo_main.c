#include "typedef.h"
#include "rect.h"
#include "res/resfile.h"

#include "dbi.h"
#include "jlgpu_math.h"		// matrix
#include "jlgpu_driver.h"	// gpu driver
#include "ui_resource.h"	// JL UI resource
#include "ui_expand/ui_expand.h"	// macro defined

#include "gpu_port.h"		// module head file
#include "gpu_draw.h"		// custom draw head file

#include "ui_core.h"
#include "ui_measure.h"

#include "ui/lcd/lcd_drive.h"
#include "res/mem_var.h"
#include "football.h"

#include "gpu_demo.h"

#if defined GPU_DEMO_ENABLE && GPU_DEMO_ENABLE

void gpu_demo_run(void)
{
#if GPU_DEMO_RAM_565_ENABLE
    extern int gpu_demo_ram_565_task(void);
    gpu_demo_ram_565_task();
#elif GPU_DEMO_FLASH_565_ENABLE
    extern int gpu_demo_flash_565_task(void);
    gpu_demo_flash_565_task();
#elif GPU_DEMO_RAM_L1_ENABLE
    extern int gpu_demo_L1_task(void);
    gpu_demo_L1_task();
#elif GPU_DEMO_RES_ENABLE
    extern int gpu_demo_res_task(void);
    gpu_demo_res_task();
#elif GPU_DEMO_COMPRESS_ENABLE
    extern int gpu_demo_compress_task(void);
    gpu_demo_compress_task();
#elif GPU_DEMO_FOOTBALL_ENABLE
    extern int gpu_demo_football_task(void);
    gpu_demo_football_task();
#elif DEMO_LCD_DUAL_BUF_ENABLE
    extern int demo_lcd_dual_buf_task(void);
    demo_lcd_dual_buf_task();
#elif GPU_DEMO_PERSPECTIVE_ENABLE
    extern int gpu_demo_perspective_task(void);
    gpu_demo_perspective_task();
#elif GPU_DEMO_TASK_MAIN
    extern int gpu_demo_task_main(void);
    gpu_demo_task_main();

#elif GPU_DEMO_TEXTURE_INPUT_ENABLE
    extern int gpu_demo_texture_input_task(void);
    gpu_demo_texture_input_task();
#elif GPU_DEMO_TEXTURE_AFFINE_ENABLE
    extern int gpu_demo_texture_affine_task(void);
    gpu_demo_texture_affine_task();
#elif GPU_DEMO_TEXTURE_PERSPECTIVE_ENABLE
    extern int gpu_demo_texture_perspective_task(void);
    gpu_demo_texture_perspective_task();
#elif GPU_DEMO_TEXTURE_CROP_ENABLE
    extern int gpu_demo_texture_crop_task(void);
    gpu_demo_texture_crop_task();
#elif GPU_DEMO_TEXTURE_MASK_ENABLE
    extern int gpu_demo_texture_mask_task(void);
    gpu_demo_texture_mask_task();
#elif GPU_DEMO_TEXTURE_EDGE_ENABLE
    extern int gpu_demo_texture_edge_task(void);
    gpu_demo_texture_edge_task();
#elif GPU_DEMO_DITHER_ENABLE
    extern int gpu_demo_dither_task(void);
    gpu_demo_dither_task();
#elif GPU_DEMO_AFFINE_ENABLE
    extern int gpu_demo_affine(void);
    gpu_demo_affine();
#elif GPU_DEMO_SCALE_INHERIT_ENABLE
    extern int gpu_demo_scale_inherit_task(void);
    gpu_demo_scale_inherit_task();
#elif GPU_DEMO_GAUSSIANBLUR
    extern int gpu_demo_gaussianblur_task(void);
    gpu_demo_gaussianblur_task();
#endif
}


#endif

