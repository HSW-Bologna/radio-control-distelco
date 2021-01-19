#ifndef ESP_LOG_H_INCLUDED
#define ESP_LOG_H_INCLUDED

#include <stdio.h>

#define ESP_LOGI(format, ...) printf(format, __VA_ARGS__)
#define ESP_LOGE(format, ...) printf(format, __VA_ARGS__)

#endif