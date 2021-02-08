#ifndef DEVICES_H_INCLUDED
#define DEVICES_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "model/model.h"
#include "network/device.h"



void connections_restart(model_t *model, QueueHandle_t queue);
void connections_clear(void);
void connections_init(void);

#endif