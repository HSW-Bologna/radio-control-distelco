#include <stdio.h>
#include <string.h>
#include "esp_log.h"

#include "peripherals/storage.h"
#include "model/model.h"


#define KEY_IP_ADDR                "IPADDR"
#define KEY_CABLES                 "CABLES"
#define KEY_PW                     "PASSWORD"
#define KEY_CHANNEL_MASTER_NUM_FMT "C%iMASNUM"
#define KEY_CHANNEL_MINION_NUM_FMT "C%iMINNUM"
#define KEY_CHANNEL_MASTER_ADD_FMT "C%iMAS%iADD"
#define KEY_CHANNEL_MINION_ADD_FMT "C%iMIN%iADD"


static const char *TAG = "Configuration";


void configuration_load(model_t *model) {
    load_uint32_option(&model->ip_addr, KEY_IP_ADDR);
    load_uint8_option(&model->cavi, KEY_CABLES);
    load_blob_option(&model->password, sizeof(model->password), KEY_PW);

    for (size_t i = 0; i < MAX_CHANNELS; i++) {
        ESP_LOGI(TAG, "Loading channel %i", i);
        char key[16] = {0};

        snprintf(key, 15, KEY_CHANNEL_MASTER_NUM_FMT, i);
        load_uint8_option(&model->channels[i].num_masters, key);

        memset(key, 0, 16);
        snprintf(key, 15, KEY_CHANNEL_MINION_NUM_FMT, i);
        load_uint8_option(&model->channels[i].num_minions, key);

        for (size_t j = 0; j < model_get_masters_in_channel(model, i); j++) {
            memset(key, 0, 16);
            snprintf(key, 015, KEY_CHANNEL_MASTER_ADD_FMT, i, j);
            load_uint32_option(&model->channels[i].masters[j].addr, key);
        }

        for (size_t j = 0; j < model_get_minions_in_channel(model, i); j++) {
            memset(key, 0, 16);
            snprintf(key, 015, KEY_CHANNEL_MINION_ADD_FMT, i, j);
            load_uint32_option(&model->channels[i].minions[j].addr, key);
        }
    }
}


void configuration_save(model_t *model) {
    save_uint32_option(&model->ip_addr, KEY_IP_ADDR);
    save_uint8_option(&model->cavi, KEY_CABLES);
    save_blob_option(&model->password, sizeof(model->password), KEY_PW);

    for (size_t i = 0; i < MAX_CHANNELS; i++) {
        ESP_LOGI(TAG, "Saving channel %i", i);
        char key[16] = {0};

        snprintf(key, 15, KEY_CHANNEL_MASTER_NUM_FMT, i);
        save_uint8_option(&model->channels[i].num_masters, key);

        memset(key, 0, 16);
        snprintf(key, 15, KEY_CHANNEL_MINION_NUM_FMT, i);
        save_uint8_option(&model->channels[i].num_minions, key);

        for (size_t j = 0; j < model_get_masters_in_channel(model, i); j++) {
            memset(key, 0, 16);
            snprintf(key, 015, KEY_CHANNEL_MASTER_ADD_FMT, i, j);
            save_uint32_option(&model->channels[i].masters[j].addr, key);
        }

        for (size_t j = 0; j < model_get_minions_in_channel(model, i); j++) {
            memset(key, 0, 16);
            snprintf(key, 015, KEY_CHANNEL_MINION_ADD_FMT, i, j);
            save_uint32_option(&model->channels[i].minions[j].addr, key);
        }
    }
}