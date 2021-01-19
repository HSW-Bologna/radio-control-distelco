#ifndef __TSC2046_H__
#define __TSC2046_H__


#include <stdint.h>
#include <stdbool.h>
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define XPT2046_AVG     4
#define XPT2046_XY_SWAP 1
#define XPT2046_X_MIN   150
#define XPT2046_Y_MIN   150
#define XPT2046_X_MAX   3950
#define XPT2046_Y_MAX   3800
#define XPT2046_X_INV   1
#define XPT2046_Y_INV   0


/*
 *  Funzione che inserisce in `data` le coordinate del tocco. E' thread safe
 * con la funzione `xpt2046_read`
 */
bool tsc2046_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

/*
 *  Inizializza il dispositivo spi
 */
void tsc2046_init();

#endif