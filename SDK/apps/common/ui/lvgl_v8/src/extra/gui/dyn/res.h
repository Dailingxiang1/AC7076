/**
 *
 * @brief
 */

#ifndef RES_H
#define RES_H

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
typedef struct {
    int32_t id;  // 页面ID
    lv_ll_t res_ll; // 资源列表
} LvPageResList; //页面资源列表


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Initialize the resource list
 */
void res_list_init();

/**
 * @brief Parse the page resource
 * @param buf The buffer of the dyn file
 */
void res_page_parse(char *buf);

/**
 * @brief Parse the resource
 * @param buf The buffer of the resource
 * @return The resource(lv_img_dsc_t or lv_font_t)
 */
void *res_parse(char *buf);

/**
 * @brief Get the page resource list
 * @param page_id The page id
 * @return The page resource list
 */
LvPageResList *res_list_get_page(int32_t page_id);

/**
 * @brief Check if the page resource list exists
 * @param page_id The page id
 * @return true if the page resource list exists, false otherwise
 */
bool res_list_has_page(int32_t page_id);

/**
 * @brief Get the resource
 * @param page_id The page id
 * @param res_id The resource id
 * @return The resource(lv_img_dsc_t or lv_font_t)
 */
void *res_get(int32_t page_id, int32_t res_id);

/**
 * @brief Free the page resource
 * @param page_id The page id
 */
void res_page_free(int32_t page_id);

/**
 * @brief Free all the resources
 */
void res_free_all();

#endif /*LV_USE_GUI_PARSE_DYN*/

#ifdef __cplusplus
}
#endif

#endif /*RES_H*/
