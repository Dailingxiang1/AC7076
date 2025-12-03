/**
 * @file lv_polygonmenu.h
 *
 */
#ifndef LV_POLYGONMENU_H
#define LV_POLYGONMENU_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include <math.h>

#if LV_USE_POLYGONMENU != 0

/*Testing of dependencies*/
#if LV_USE_IMG == 0
#error "lv_polygonmenu: lv_img is required. Enable it in lv_conf.h (LV_USE_IMG 1)"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef int16_t (*lv_polygonmenu_item_transform_cb_t)(lv_point_t *icon_pos, const lv_obj_t *menu,
        const lv_obj_t *btn);

typedef lv_obj_t *(*lv_polygonmenu_get_setup_scr_cb_t)(int32_t scr_id);
typedef bool (*lv_polygonmenu_is_scr_act_cb_t)(int32_t scr_id);
typedef void (*lv_polygonmenu_load_scr_cb_t)(int32_t src_id);

typedef struct {
    lv_polygonmenu_get_setup_scr_cb_t get_setup_scr_cb;
    lv_polygonmenu_is_scr_act_cb_t is_scr_act_cb;
    lv_polygonmenu_load_scr_cb_t load_scr_cb;
} lv_polygonmenu_scr_cb;

typedef enum {
    LV_POLYGONMENU_SHAPE_SQUARE = 0,
    LV_POLYGONMENU_SHAPE_HEXAGON = 1,
} lv_polygonmenu_shape_t;


typedef union {
    struct {
        uint16_t sphere_r_corner;      // 球面转换角大小
        uint16_t sphere_radius;        // 球体半径
        float sphere_distortion_coeff; // 畸变系数
    } star;

    struct {
        lv_coord_t nop1;
        lv_coord_t nop2;
        lv_coord_t nop3;
        lv_coord_t nop4;
    } custom;
} lv_polygonmenu_style_param_t;


typedef struct {
    lv_obj_t obj;
    lv_polygonmenu_shape_t shape;

    lv_polygonmenu_item_transform_cb_t item_transform_cb;
    lv_polygonmenu_style_param_t style_param;
    int32_t icon_min_size; // 转换后，隐藏大小小于该值的icon

    lv_polygonmenu_scr_cb scr_cb;

    // 内部属性
    int32_t icon_size;
    int32_t icon_width;
    int32_t icon_height;

    uint8_t status;
    uint8_t prev_status;
    uint16_t max_row_icon_count;
    int32_t offset_x;
    int32_t offset_y;
    lv_timer_t *move_tmr;
    lv_timer_t *temp_status_report_tmr;
    void *move_tmr_data;

} lv_polygonmenu_t;
extern const lv_obj_class_t lv_polygonmenu_class;


typedef struct {
    lv_img_t obj;
    bool is_hide; // 是否隐藏
    int32_t scr_id; //屏幕id

    // 内部属性
    int32_t offset_x;
    int32_t offset_y;
} lv_polygonmenu_btn_t;
extern const lv_obj_class_t lv_polygonmenu_btn_class;


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Create an polygonmenu object
 * @param parent pointer to the parent object
 * @return lv_obj_t* pointer to the created polygonmenu object
 */
lv_obj_t *lv_polygonmenu_create(lv_obj_t *parent);

/**
 * @brief Add an image button to the polygonmenu
 * @param menu pointer to the polygonmenu object
 * @param icon path to the image source
 * @return lv_obj_t* pointer to the created button object
 */
lv_obj_t *lv_polygonmenu_add_btn(lv_obj_t *menu, const void *icon);

void lv_polygonmenu_zoom_in(lv_obj_t *menu_obj);

void lv_polygonmenu_zoom_out(lv_obj_t *menu_obj);

/**
 * @brief Load scr from polygonmenu_btn.
 * @param menu pointer to the polygonmenu_btn object
 */
void lv_polygonmenu_btn_load_scr(lv_obj_t *btn_obj);

/**
 * @brief Return to the screen where the menu belongs
 * @param menu pointer to the polygonmenu object
 */
void lv_polygonmenu_return(lv_obj_t *menu_obj);


/*=====================
 * Setter functions
 *====================*/

void lv_polygonmenu_btn_set_hide(lv_obj_t *btn_obj, bool hide);

void lv_polygonmenu_set_shape(lv_obj_t *menu_obj, lv_polygonmenu_shape_t shape);

void lv_polygonmenu_set_icon_min_size(lv_obj_t *menu_obj, int32_t icon_min_size);

void lv_polygonmenu_set_item_transform_cb(lv_obj_t *menu_obj, lv_polygonmenu_item_transform_cb_t cb);

void lv_polygonmenu_set_style_param(lv_obj_t *menu_obj, lv_polygonmenu_style_param_t *style_param);

void lv_polygonmenu_set_scr_cb(lv_obj_t *menu_obj, lv_polygonmenu_scr_cb scr_cb);

void lv_polygonmenu_btn_set_scr_id(lv_obj_t *btn_obj, int32_t scr_id);

/*=====================
 * Getter functions
 *====================*/

bool lv_polygonmenu_btn_is_hide(lv_obj_t *btn_obj);

lv_polygonmenu_shape_t lv_polygonmenu_get_shape(lv_obj_t *menu_obj);

int32_t lv_polygonmenu_get_icon_min_size(lv_obj_t *menu_obj);

lv_polygonmenu_item_transform_cb_t lv_polygonmenu_get_item_transform_cb(lv_obj_t *menu_obj);

lv_polygonmenu_style_param_t *lv_polygonmenu_get_style_param(lv_obj_t *menu_obj);

lv_polygonmenu_scr_cb lv_polygonmenu_get_scr_cb(lv_obj_t *menu_obj);

int32_t lv_polygonmenu_btn_get_scr_id(lv_obj_t *btn_obj);


/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_POLYGONMENU*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_POLYGONMENU_H*/
