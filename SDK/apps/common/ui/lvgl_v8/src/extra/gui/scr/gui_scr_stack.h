/**
 * @file gui_scr_stack.h
 *
 */
#ifndef GUI_SCR_STACK_H
#define GUI_SCR_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#if LV_USE_GUI_SCR_STACK
/*********************
 *      INCLUDES
 *********************/
#include "gui_scr.h"

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
 * @brief initialize the screen stack
 *
 */
void gui_scr_stack_init(void);

/**
 * @brief push the screen to the stack
 *
 * @param screen screen object
 */
void gui_scr_stack_push(gui_scr_t *screen);

/**
 * @brief pop the screen from the stack
 *
 * @return gui_scr_t* screen object
 */
gui_scr_t *gui_scr_stack_pop(void);

/**
 * @brief get the top screen from the stack
 *
 * @return gui_scr_t* screen object
 */
gui_scr_t *gui_scr_stack_top(void);

/**
 * @brief clear the screen stack
 *
 */
void gui_scr_stack_clear(void);

/*=====================
 * Getter functions
 *====================*/

/**
  * @brief get the length of the screen stack
  *
  * @return uint32_t screen stack length
  */
uint32_t gui_scr_stack_get_len(void);

/**
 * @brief check the screen stack is empty
 *
 * @return true screen stack is empty
 * @return false screen stack is not empty
 */
bool gui_scr_stack_is_empty(void);

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GUI_SCR_STACK_H*/
