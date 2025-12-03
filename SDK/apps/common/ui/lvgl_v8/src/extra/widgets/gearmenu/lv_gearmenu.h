/**
 * @file lv_gearmenu.h
 *
 */

#ifndef LV_GEARMENU_H
#define LV_GEARMENU_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#if LV_USE_GEARMENU != 0

/*Testing of dependencies*/
#if LV_USE_IMG == 0
#error "lv_gearmenu: lv_img is required. Enable it in lv_conf.h (LV_USE_IMG 1)"
#endif
#if LV_USE_LABEL == 0
#error "lv_gearmenu: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1)"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t obj;
    lv_coord_t angle_start; // 可视区域起始角度
    lv_coord_t angle_end; // 可视区域结束角度
    lv_coord_t angle; //当前角度
    lv_coord_t angle_step; // 菜单项之间的角度差
    lv_coord_t radius; // 菜单半径
    lv_coord_t px_per_angle; // 每度对应的像素
    lv_coord_t bounce_anim_time; // 回弹动画时间，单位ms
    lv_point_t last_point; // 上次滚动位置
    lv_point_t center; // 菜单中心坐标
    bool is_clockwise; // 顺时针
    bool is_loop_mode; // 循环模式
    bool _is_init; // 是否初始化
} lv_gearmenu_t;

typedef struct {
    lv_obj_t obj;
    lv_obj_t *img;
    lv_obj_t *label;
    bool is_hide;
} lv_gearmenu_btn_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a gear menu object
 * @param parent pointer to the parent object
 * @return lv_obj_t* pointer to the created gear menu object
 */
lv_obj_t *lv_gearmenu_create(lv_obj_t *parent);

/**
 * Add a button to the gear menu
 * @param gear_menu pointer to the gear menu object
 * @param icon pointer to the image source
 * @param txt text to display on the button
 * @return lv_obj_t* pointer to the created button object
 */
lv_obj_t *lv_gearmenu_add_btn(lv_obj_t *gear_menu, const void *icon, const char *txt);

/**
 * change the current angle to the closest angle in integral multiple of the closet angle step
 * @param gear_menu pointer to the gear menu object
 */
void lv_gearmenu_rotate_to_the_closest_angle(lv_obj_t *obj);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the start angle of the gear menu
 * @param obj pointer to the gear menu object
 * @param start start angle of the gear menu
 */
void lv_gearmenu_set_start_angle(lv_obj_t *obj, lv_coord_t start);

/**
 * Set the end angle of the gear menu
 * @param obj pointer to the gear menu object
 * @param end end angle of the gear menu
 */
void lv_gearmenu_set_end_angle(lv_obj_t *obj, lv_coord_t end);

/**
 * Set the start and end angles of the gear menu
 * @param obj pointer to the gear menu object
 * @param start start angle of the gear menu
 * @param end end angle of the gear menu
 */
void lv_gearmenu_set_angles(lv_obj_t *obj, lv_coord_t start, lv_coord_t end);

/**
 * Set the radius of the gear menu
 * @param obj pointer to the gear menu object
 * @param radius radius of the gear menu
 */
void lv_gearmenu_set_radius(lv_obj_t *obj, lv_coord_t radius);

/**
 * Set the angle step of the gear menu
 * @param obj pointer to the gear menu object
 * @param angle_step angle step of the gear menu
 */
void lv_gearmenu_set_angle_step(lv_obj_t *obj, lv_coord_t angle_step);

/**
 * Set the pixels per angle of the gear menu
 * @param obj pointer to the gear menu object
 * @param px_per_angle pixels per angle of the gear menu
 */
void lv_gearmenu_set_px_per_angle(lv_obj_t *obj, lv_coord_t px_per_angle);

/**
 * Set the center x of the gear menu
 * @param obj pointer to the gear menu object
 * @param x center x of the gear menu
 */
void lv_gearmenu_set_center_x(lv_obj_t *obj, lv_coord_t x);

/**
 * Set the center y of the gear menu
 * @param obj pointer to the gear menu object
 * @param y center y of the gear menu
 */
void lv_gearmenu_set_center_y(lv_obj_t *obj, lv_coord_t y);

/**
 * Set the center of the gear menu
 * @param obj pointer to the gear menu object
 * @param x center x of the gear menu
 * @param y center y of the gear menu
 */
void lv_gearmenu_set_center(lv_obj_t *obj, lv_coord_t x, lv_coord_t y);

/**
 * Set the gear menu to be clockwise or counterclockwise
 * @param obj pointer to the gear menu object
 * @param is_clockwise true to set the gear menu to be clockwise
 */
void lv_gearmenu_set_clockwise(lv_obj_t *obj, bool is_clockwise);

/**
 * Set the button to the center of the gear menu
 * @param obj pointer to the gear menu object
 * @param btn pointer to the button object
 */
void lv_gearmenu_set_btn_to_center(lv_obj_t *obj, lv_obj_t *btn);

/**
 * Set the image of the gear menu button
 * @param btn pointer to the button object
 * @param icon pointer to the image source
 */
void lv_gearmenu_btn_set_img(lv_obj_t *btn, const void *icon);

/**
 * Set the text of the gear menu button
 * @param btn pointer to the button object
 * @param txt text to display on the button
 */
void lv_gearmenu_btn_set_text(lv_obj_t *btn, const char *txt);

/**
 * Set the zoom of the gear menu button image
 * @param btn pointer to the button object
 * @param zoom zoom of the button image
 */
void lv_gearmenu_btn_set_img_zoom(lv_obj_t *btn, uint16_t zoom);

/**
 * Set the gear menu button to be hidden or not
 * @param btn pointer to the button object
 * @param is_hide true to set the button to be hidden
 */
void lv_gearmenu_btn_set_hide(lv_obj_t *btn, bool is_hide);

/**
 * Set the current angle of the gear menu
 * @param obj pointer to the gear menu object
 * @param angle angle of the gear menu
 */
void lv_gearmenu_set_angle(lv_obj_t *obj, lv_coord_t angle);

/**
 * Set the bounce animation time of the gear menu
 * @param obj pointer to the gear menu object
 * @param bounce_anim_time the bounce animation time of the gear menu
 */
void lv_gearmenu_set_bounce_anim_time(lv_obj_t *obj, lv_coord_t bounce_anim_time);

/**
 * Set the loop mode of the gear menu
 * @param obj pointer to the gear menu object
 * @param is_loop_mode true to enable the loop mode
 */
void lv_gearmenu_set_loop_mode(lv_obj_t *obj, bool is_loop_mode);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the start angle of the gear menu
 * @param obj pointer to the gear menu object
 * @return lv_coord_t start angle of the gear menu
 */
lv_coord_t lv_gearmenu_get_start_angle(lv_obj_t *obj);

/**
 * Get the end angle of the gear menu
 * @param obj pointer to the gear menu object
 * @return lv_coord_t end angle of the gear menu
 */
lv_coord_t lv_gearmenu_get_end_angle(lv_obj_t *obj);

/**
 * Get the radius of the gear menu
 * @param obj pointer to the gear menu object
 * @return lv_coord_t radius of the gear menu
 */
lv_coord_t lv_gearmenu_get_radius(lv_obj_t *obj);

/**
 * Get the angle step of the gear menu
 * @param obj pointer to the gear menu object
 * @return lv_coord_t angle step of the gear menu
 */
lv_coord_t lv_gearmenu_get_angle_step(lv_obj_t *obj);

/**
 * Get the pixels per angle of the gear menu
 * @param obj pointer to the gear menu object
 * @return lv_coord_t pixels per angle of the gear menu
 */
lv_coord_t lv_gearmenu_get_px_per_angle(lv_obj_t *obj);

/**
 * Get the hide status of the gear menu button
 * @param btn pointer to the gear menu button object
 * @return true if the button is hidden
 * @return false if the button is not hidden
 */
bool lv_gearmenu_btn_get_hide(lv_obj_t *btn);

/**
 * Get the current angle of the gear menu
 * @param obj pointer to the gear menu object
 * @return lv_coord_t angle of the gear menu
 */
lv_coord_t lv_gearmenu_get_angle(lv_obj_t *obj);

/**
 * Get the bounce animation time of the gear menu
 * @param obj pointer to the gear menu object
 * @return lv_coord_t bounce animation time of the gear menu
 */
lv_coord_t lv_gearmenu_get_bounce_anim_time(lv_obj_t *obj);

/**
 * Get the loop mode of the gear menu
 * @param obj pointer to the gear menu object
 * @return whether the loop mode is enabled
 */
bool lv_gearmenu_get_loop_mode(lv_obj_t *obj);

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_GEARMENU*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_GEARMENU_H*/

