/**
 * @file lv_listmenu.h
 *
 */

#ifndef LV_LISTMENU_H
#define LV_LISTMENU_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#if LV_USE_LISTMENU != 0
/*Testing of dependencies*/
#if LV_USE_IMG == 0
#error "lv_listmenu: lv_img is required. Enable it in lv_conf.h (LV_USE_IMG 1)"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief menu item transform callback
 * @param icon_pos menu item icon position
 * @param menu menu object
 * @param btn button object
 * @return transform zoom value
 */
typedef int16_t (*lv_listmenu_item_transform_cb_t)(lv_point_t *icon_pos, const lv_obj_t *menu, const lv_obj_t *btn);

typedef enum {
    LV_LISTMENU_STYLE_LIST, // 列表
    LV_LISTMENU_STYLE_ARC, // 弧形
    LV_LISTMENU_STYLE_GEAR, // 齿轮

    LV_LISTMENU_STYLE_CUSTOM = 0x10, // 自定义
} lv_listmenu_style_t;

typedef union {
    struct {
        lv_coord_t radius;
    } arc;
    struct {
        lv_coord_t radius;
    } gear;

    struct {
        lv_coord_t nop1;
        lv_coord_t nop2;
        lv_coord_t nop3;
        lv_coord_t nop4;
    } custom;
} lv_listmenu_style_param_t;

typedef struct {
    lv_obj_t obj;

    lv_listmenu_item_transform_cb_t item_transform_cb;
    lv_listmenu_style_t style;
    lv_listmenu_style_param_t style_param;

    struct {
        lv_coord_t ver_offset;
        lv_coord_t offset_min;
        lv_coord_t offset_max;
        lv_coord_t momentum;
        bool is_scroll;
        lv_timer_t *scroll_tmr;
    } scroll;

    lv_coord_t friction_factor;
    lv_coord_t snap_factor;
    lv_coord_t elastic_factor;
    lv_coord_t out_of_bound_factor;

    lv_coord_t top_margin;
    lv_coord_t bottom_margin;

    bool is_snap;
    bool _is_init;
} lv_listmenu_t;

typedef struct {
    lv_obj_t obj;
    lv_obj_t *img;
    lv_obj_t *label;
} lv_listmenu_btn_t;

extern const lv_obj_class_t lv_listmenu_class;
extern const lv_obj_class_t lv_listmenu_btn_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Create a new list menu
 * @param parent parent object
 * @return new created list menu object
 */
lv_obj_t *lv_listmenu_create(lv_obj_t *parent);

/**
 * @brief Add a button to the list menu
 * @param parent parent object
 * @param icon button icon
 * @param txt button text
 * @return new added button object
 */
lv_obj_t *lv_listmenu_add_btn(lv_obj_t *parent, const void *icon, const char *txt);

/*=====================
 * Setter functions
 *====================*/

/**
 * @brief Set the list menu style
 * @param obj list menu object
 * @param style style type
 * @param param style parameter
 */
void lv_listmenu_set_style(lv_obj_t *obj, lv_listmenu_style_t style, lv_listmenu_style_param_t *param);

/**
 * @brief Set the menu item transform callback
 * @param obj list menu object
 * @param item_transform_cb transform callback
 * @param param style parameter
 */
void lv_listmenu_set_item_transform_cb(lv_obj_t *obj, lv_listmenu_item_transform_cb_t item_transform_cb,
                                       lv_listmenu_style_param_t *param);

/**
 * @brief Set the scroll offset
 * @param obj list menu object
 * @param ver_offset vertical offset
 * @param anim whether to enable animation
 * @param speed animation speed
 */
void lv_listmenu_set_scroll_offset(lv_obj_t *obj, lv_coord_t ver_offset, bool anim, uint32_t speed);

/**
 * @brief Set the scroll to center
 * @param obj list menu object
 * @param anim whether to enable animation
 * @param speed animation speed
 */
void lv_listmenu_set_scroll_to_center(lv_obj_t *obj, bool anim, uint32_t speed);

/**
 * @brief Set the margin
 * @param obj list menu object
 * @param top_margin head margin
 * @param bottom_margin tail margin
 */
void lv_listmenu_set_margin(lv_obj_t *obj, lv_coord_t top_margin, lv_coord_t bottom_margin);

/**
 * @brief Enable or disable snap feature
 * @param obj list menu object
 * @param enabled whether to enable
 */
void lv_listmenu_set_snap_enabled(lv_obj_t *obj, bool enabled);

/**
 * @brief Set the elastic factor
 * @param obj list menu object
 * @param elastic_factor elastic factor (0-100, default value: 15)
 */
void lv_listmenu_set_elastic_factor(lv_obj_t *obj, lv_coord_t elastic_factor);

/**
 * @brief Set the out of bound factor
 * @param obj list menu object
 * @param out_of_bound_factor out of bound factor (0-100, default value: 60)
 */
void lv_listmenu_set_out_of_bound_factor(lv_obj_t *obj, lv_coord_t out_of_bound_factor);

/**
 * @brief Set the friction factor
 * @param obj list menu object
 * @param friction_factor friction factor (0-100, default value: 80)
 */
void lv_listmenu_set_friction_factor(lv_obj_t *obj, lv_coord_t friction_factor);

/**
 * @brief Set the snap factor
 * @param obj list menu object
 * @param snap_factor snap factor (0-100, default value: 40)
 */
void lv_listmenu_set_snap_factor(lv_obj_t *obj, lv_coord_t snap_factor);

/**
 * @brief Hide or show button
 * @param btn button object
 * @param hide whether to hide
 */
void lv_listmenu_btn_set_hide(lv_obj_t *btn, bool hide);

/**
 * @brief Set the button icon zoom
 * @param btn button object
 * @param zoom zoom value
 */
void lv_listmenu_btn_set_img_zoom(lv_obj_t *btn, uint16_t zoom);

/**
 * @brief Set the button text
 * @param btn button object
 * @param text text content
 */
void lv_listmenu_btn_set_text(lv_obj_t *btn, const char *text);

/*=====================
 * Getter functions
 *====================*/

/**
 * @brief Get the list menu style
 * @param obj list menu object
 * @return current style type
 */
lv_listmenu_style_t lv_listmenu_get_style(lv_obj_t *obj);

/**
 * @brief Get the list menu style parameter
 * @param obj list menu object
 * @return current style parameter
 */
lv_listmenu_style_param_t *lv_listmenu_get_style_param(lv_obj_t *obj);

/**
 * @brief Get the current center button
 * @param obj list menu object
 * @return current center button object
 */
lv_obj_t *lv_listmenu_get_center_btn(lv_obj_t *obj);

/**
 * @brief Get the scroll offset
 * @param obj list menu object
 * @return scroll offset
 */
lv_coord_t lv_listmenu_get_scroll_offset(lv_obj_t *obj);

/**
 * @brief Get the scroll to center
 * @param obj list menu object
 * @return scroll to center
 */
lv_coord_t lv_listmenu_get_scroll_to_center(lv_obj_t *obj);

/**
 * @brief Get the head margin
 * @param obj list menu object
 * @return head margin
 */
lv_coord_t lv_listmenu_get_top_margin(lv_obj_t *obj);

/**
 * @brief Get the tail margin
 * @param obj list menu object
 * @return tail margin
 */
lv_coord_t lv_listmenu_get_bottom_margin(lv_obj_t *obj);

/**
 * @brief Get the snap enabled
 * @param obj list menu object
 * @return snap enabled
 */
bool lv_listmenu_is_snap_enabled(lv_obj_t *obj);

/**
 * @brief Get the friction factor
 * @param obj list menu object
 * @return friction factor
 */
lv_coord_t lv_listmenu_get_friction_factor(lv_obj_t *obj);

/**
 * @brief Get the snap factor
 * @param obj list menu object
 * @return snap factor
 */
lv_coord_t lv_listmenu_get_snap_factor(lv_obj_t *obj);

/**
 * @brief Get the elastic factor
 * @param obj list menu object
 * @return elastic factor
 */
lv_coord_t lv_listmenu_get_elastic_factor(lv_obj_t *obj);

/**
 * @brief Get the out of bound factor
 * @param obj list menu object
 * @return out of bound factor
 */
lv_coord_t lv_listmenu_get_out_of_bound_factor(lv_obj_t *obj);

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_LISTMENU*/

#ifdef __cplusplus
}
#endif

#endif
