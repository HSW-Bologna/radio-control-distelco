#ifndef ESP_LOG_H_INCLUDED
#define ESP_LOG_H_INCLUDED

#include <stdio.h>

#define ESP_LOGI(tag, format, ...) printf(format "\n", ##__VA_ARGS__)
#define ESP_LOGE(tag, format, ...) printf(format "\n", ##__VA_ARGS__)

#endif