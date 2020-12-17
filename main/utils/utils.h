#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

#define get_millis() ((uint32_t)xTaskGetTickCount() * portTICK_PERIOD_MS)
void free_lv_user_data(lv_obj_t *obj);

#endif