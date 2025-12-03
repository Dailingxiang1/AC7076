/**
 * @file bind.h
 *
 */

#ifndef BIND_H
#define BIND_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "gui_parse_dyn.h"
#include "../../../core/lv_disp.h"
#include "lvgl.h"

#if LV_USE_GUI_PARSE_DYN != 0

typedef lv_event_cb_t (*gui_get_event_cb_t)(int32_t id);
typedef int32_t (*gui_bind_send_cb_t)(int32_t msg_id, void *value, int32_t len);
typedef void *(*gui_bind_get_cb_t)(int32_t msg_id);
typedef lv_subject_t *(*gui_bind_get_subject_cb_t)(int32_t id);
typedef void *(*gui_bind_get_data_cb_t)();

/*********************
 *      DEFINES
 *********************/

/**********************
 *     TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Set the control bind msg
 * @param buf The buffer of the control data
 * @param size The size of the control data
 * @param obj The object of the control
 * @param page_id The page id of the control
 */
void bind_set(char *buf, int32_t size, lv_obj_t *obj, int32_t page_id);

/**
 * @brief Set the bind send cb
 * @param cb
 */
void bind_set_send_cb(gui_bind_send_cb_t cb);

/**
 * @brief Set the bind get cb
 * @param cb
 */
void bind_set_get_cb(gui_bind_get_cb_t cb);

/**
 * @brief Set the bind get subject cb
 * @param cb
 */
void bind_set_get_subject_cb(gui_bind_get_subject_cb_t cb);

/**
 * @brief Set the bind get data cb
 * @param cb
 */
void bind_set_get_data_cb(gui_bind_get_data_cb_t cb);

#endif /*LV_USE_GUI_PARSE_DYN*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*BIND_H*/
