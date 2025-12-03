/**
 * @file lv_jljpeg.h
 *
 */

/*----------------------------------------------*/
/* JL HW JEPG CODEC                             */
/*----------------------------------------------*/
#ifndef LV_JLJPEG_H
#define LV_JLJPEG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_JLJPEG

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_jlhw_jpeg_init(void);

bool lv_jljpeg_check_magic(void *user_data);

void lv_jljpeg_get_img_mcu_info(void *user_data, uint16_t *mx, uint16_t *my, uint16_t *width, uint16_t *height);

lv_obj_t *lv_jljpeg_create(lv_obj_t *parent, const char *file);
void lv_jljpeg_destructor(lv_obj_t *obj);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_JLJPEG*/

#ifdef __cplusplus
}
#endif

#endif /* LV_JLJPEG_H */
