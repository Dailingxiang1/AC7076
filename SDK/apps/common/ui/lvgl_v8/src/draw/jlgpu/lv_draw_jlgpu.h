/**
 * @file lv_draw_jlgpu.h
 *
 */


#ifndef LV_DRAW_JLGPU_H
#define LV_DRAW_JLGPU_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../../lv_conf_internal.h"

#if LV_USE_GPU2D_JL
#include "../sw/lv_draw_sw.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef lv_draw_sw_ctx_t lv_draw_jlgpu_ctx_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_jlgpu_ctx_init(struct _lv_disp_drv_t *drv, lv_draw_ctx_t *draw_ctx);

void lv_draw_jlgpu_ctx_deinit(struct _lv_disp_drv_t *drv, lv_draw_ctx_t *draw_ctx);

void lv_draw_jlgpu_set_trans_area(lv_area_t *_area);
/**********************
 *      MACROS
 **********************/
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
