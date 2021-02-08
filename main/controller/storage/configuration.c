#include <stdio.h>
#include "esp_log.h"

#include "peripherals/storage.h"
#include "model/model.h"


#define KEY_IP_ADDR     "IPADDR"
#define KEY_CABLES      "CABLES"
#define KEY_CHANNEL_FMT "CHANNEL%i"


static const char *TAG = "Configuration";


void configuration_load(model_t *model) {
    load_uint32_option(&model->ip_addr, KEY_IP_ADDR);
    load_uint8_option(&model->cavi, KEY_CABLES);

    for (size_t i = 0; i < MAX_CHANNELS; i++) {
        char key[16] = {0};
        snprintf(key, 16, KEY_CHANNEL_FMT, i);
        ESP_LOGI(TAG, "Loading %s", key);
        load_blob_option(&model->channels[i], sizeof(channel_t), key);
    }
}


void configuration_save(model_t *model) {
    save_uint32_option(&model->ip_addr, KEY_IP_ADDR);
    save_uint8_option(&model->cavi, KEY_CABLES);

    for (size_t i = 0; i < MAX_CHANNELS; i++) {
        char key[16] = {0};
        snprintf(key, 16, KEY_CHANNEL_FMT, i);
        save_blob_option(&model->channels[i], sizeof(channel_t), key);
    }
}