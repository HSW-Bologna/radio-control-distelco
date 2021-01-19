#ifndef DEVICES_H_INCLUDED
#define DEVICES_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "model/model.h"
#include "network/device.h"



typedef struct {
    int    master;
    size_t channel;
    size_t index;
    int    ok;
    char   name[CONFIG_NAME_LEN];
} device_update_t;


void connections_restart(model_t *model, QueueHandle_t queue);
void connections_clear(void);
void connections_init(void);

#endif