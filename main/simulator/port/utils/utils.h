#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "lvgl.h"

unsigned long get_millis(void);
void          free_lv_user_data(lv_obj_t *obj);

#endif