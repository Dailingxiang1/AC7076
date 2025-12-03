/**
 * @file lv_imgmask.h
 *
 */

#ifndef LV_IMGMASK_H
#define LV_IMGMASK_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#if LV_USE_IMGMASK != 0

/*Testing of dependencies*/
#if LV_USE_IMG == 0
#error "lv_imgmask: lv_img is required. Enable it in lv_conf.h (LV_USE_IMG 1)"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_img_t img;
    const void *mask;
    lv_opa_t *mask_map;
    lv_point_t mask_offset;
    lv_point_t mask_pivot;
    uint16_t angle;
    uint16_t zoom;
    lv_draw_mask_map_param_t mask_map_param;
    int16_t mask_id;
    uint8_t mask_type;
} lv_imgmask_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a image mask object
 * @param parent pointer to the parent object
 * @return lv_obj_t* pointer to the created image mask object
 */
lv_obj_t *lv_imgmask_create(lv_obj_t *parent);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the mask of an image mask object
 * @param obj pointer to an image mask object
 * @param mask pointer to a mask image (a variable, a file)
 */
void lv_imgmask_set_mask(lv_obj_t *obj, const void *mask);

/**
 * Set the offset of the mask image on the x axis
 * so the mask will be displayed from the new origin.
 * @param obj pointer to an image mask object
 * @param x offset on the x axis
 */
void lv_imgmask_set_mask_offset_x(lv_obj_t *obj, lv_coord_t x);

/**
 * Set the offset of the mask image on the y axis
 * so the mask will be displayed from the new origin.
 * @param obj pointer to an image mask object
 * @param y offset on the y axis
 */
void lv_imgmask_set_mask_offset_y(lv_obj_t *obj, lv_coord_t y);

/**
 * Set the angle of the mask image
 * The mask image will be rotated around the set pivot set by `lv_imgmask_set_mask_pivot()`
 * Note that indexed and alpha only images can't be transformed.
 * @param obj pointer to an image mask object
 * @param angle angle of the mask image
 */
void lv_imgmask_set_mask_angle(lv_obj_t *obj, uint16_t angle);

/**
 * Set the zoom of the mask image
 * @param obj pointer to an image mask object
 * @param zoom zoom of the mask image
 * @example 256 or LV_ZOOM_IMG_NONE for no zoom
 * @example <256: scale down
 * @example >256 scale up
 * @example 128 half size
 * @example 512 double size
 */
void lv_imgmask_set_mask_zoom(lv_obj_t *obj, uint16_t zoom);

/**
 * Set the pivot of the mask image
 * The mask image will be rotated around this point.
 * @param obj pointer to an image mask object
 * @param x rotation center x of the mask image
 * @param y rotation center y of the mask image
 */
void lv_imgmask_set_mask_pivot(lv_obj_t *obj, lv_coord_t x, lv_coord_t y);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the mask of an image mask object
 * @param obj pointer to an image mask object
 * @return const void* pointer to the mask image(file name or variable)
 */
const void *lv_imgmask_get_mask(lv_obj_t *obj);

/**
 * Get the offset's x attribute of the mask image object.
 * @param obj pointer to an image mask object
 * @return lv_coord_t offset X value.
 */
lv_coord_t lv_imgmask_get_mask_offset_x(lv_obj_t *obj);

/**
 * Get the offset's y attribute of the mask image object.
 * @param obj pointer to an image mask object
 * @return lv_coord_t offset Y value.
 */
lv_coord_t lv_imgmask_get_mask_offset_y(lv_obj_t *obj);

/**
 * Get the angle of the mask image object.
 * @param obj pointer to an image mask object
 * @return uint16_t angle value.
 */
uint16_t lv_imgmask_get_mask_angle(lv_obj_t *obj);

/**
 * Get the zoom of the mask image object.
 * @param obj pointer to an image mask object
 * @return uint16_t zoom value.
 */
uint16_t lv_imgmask_get_mask_zoom(lv_obj_t *obj);

/**
 * Get the pivot of the mask image object.
 * @param obj pointer to an image mask object
 * @param pivot pointer to a point to store the pivot
 */
void lv_imgmask_get_mask_pivot(lv_obj_t *obj, lv_point_t *pivot);

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_IMGMASK*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_IMGMASK_H*/
