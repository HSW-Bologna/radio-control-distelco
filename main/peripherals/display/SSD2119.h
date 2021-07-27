/**
 * @file SSD1963.h
 *
 */

#ifndef SSD1963_H
#define SSD1963_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "lvgl.h"
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void ssd2119_init(void);
void ssd2119_test(uint16_t color);
void ssd2119_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void ssd2119_reconfigure(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SSD1963_H */
