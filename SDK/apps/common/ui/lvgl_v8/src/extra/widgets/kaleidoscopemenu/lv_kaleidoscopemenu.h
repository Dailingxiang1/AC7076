/**
 * @file lv_kaleidoscopemenu.h
 *
 */
#ifndef LV_KALEIDOSCOPEMENU_H
#define LV_KALEIDOSCOPEMENU_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include <math.h>

#if LV_USE_KALEIDOSCOPEMENU != 0

/*Testing of dependencies*/
#if LV_USE_IMG == 0
#error "lv_kaleidoscopemenu: lv_img is required. Enable it in lv_conf.h (LV_USE_IMG 1)"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t obj;
    uint16_t icon_size;
    uint16_t ring_interval; // 用于调整相邻的各圈的间隔

    // single_move的大小依赖于icon_size和ring_interval，不能直接修改
    uint16_t icon_single_move;   // 单次移动距离

    int16_t roffset;    //移动距离
    int8_t cnt;         //圈数切换的参数
    int8_t last_dir;    //最后一次移动的方向
    uint16_t last_move_distance;
    int8_t energy_cnt;  //惯性计数
    uint16_t key_elm_index;  //最靠近屏幕右侧边界的图标下标
    lv_timer_t *touch_release_timer;  // 惯性定时器
} lv_kaleidoscopemenu_t;
extern const lv_obj_class_t lv_kaleidoscopemenu_class;


typedef struct {
    lv_img_t obj;
    bool _hide;
} lv_kaleidoscopemenu_btn_t;
extern const lv_obj_class_t lv_kaleidoscopemenu_btn_class;


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Create an kaleidoscope menu object
 * @param parent pointer to the parent object
 * @return lv_obj_t* pointer to the created kaleidoscope menu object
 */
lv_obj_t *lv_kaleidoscopemenu_create(lv_obj_t *parent);

/**
 * @brief Add an image button to the kaleidoscopemenu menu
 * @param menu pointer to the kaleidoscopemenu menu object
 * @param icon path to the image source
 * @return lv_obj_t* pointer to the created button object
 */
lv_obj_t *lv_kaleidoscopemenu_add_btn(lv_obj_t *menu, const void *icon);

/*=====================
 * Setter functions
 *====================*/

/**
 * @brief Set the size of the kaleidoscopemenu btn.
 * @param obj pointer to the kaleidoscope menu object
 * @param icon_size size of the kaleidoscopemenu btn
 */
void lv_kaleidoscopemenu_set_icon_size(lv_obj_t *obj, uint16_t icon_size);

/**
 * @brief Set the icon interval of the kaleidoscopemenu btn ring.
 * @param obj pointer to the kaleidoscope menu object
 * @param ring_interval the interval of the kaleidoscopemenu btn ring
 */
void lv_kaleidoscopemenu_set_ring_interval(lv_obj_t *obj, uint16_t ring_interval);

/**
 * @brief Set the is_hide status of the kaleidoscopemenu btn icon
 * @param obj pointer to the kaleidoscope menu object
 * @param hide is_hide status of the kaleidoscopemenu btn icon
 */
void lv_kaleidoscopemenu_btn_set_hide(lv_obj_t *obj, bool hide);

/*=====================
 * Getter functions
 *====================*/

/**
 * @brief Get the size of the kaleidoscopemenu btn
 * @param obj pointer to the kaleidoscope menu object
 * @return the size of the kaleidoscopemenu btn
 */
uint16_t lv_kaleidoscopemenu_get_icon_size(lv_obj_t *obj);

/**
 * @brief Get the interval of the kaleidoscopemenu btn ring
 * @param obj pointer to the kaleidoscope menu object
 * @return the interval of the kaleidoscopemenu btn ring
 */
uint16_t lv_kaleidoscopemenu_get_ring_interval(lv_obj_t *obj);

/**
 * @brief Get the is_hide status of the kaleidoscopemenu btn icon
 * @param obj pointer to the kaleidoscope menu object
 * @return is_hide status of the kaleidoscopemenu btn icon
 */
bool lv_kaleidoscopemenu_btn_get_hide(lv_obj_t *obj);

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_KALEIDOSCOPEMENU*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_KALEIDOSCOPEMENU_H*/
