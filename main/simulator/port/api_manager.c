#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "model/model.h"


struct response_buffer {
    uint8_t *pointer;
    size_t   max;
};


typedef enum { MESSAGE_CODE_REENABLE } message_code_t;


struct message {
    message_code_t code;

    union {
        uint32_t ip;
    };
};


static void api_task(void *arg);

static const char * TAG   = "API";
static xQueueHandle queue = NULL;


void api_manager_init(void) {
    queue = xQueueCreate(8, sizeof(struct message));
    xTaskCreate(api_task, "Async API", 4096 * 2, NULL, uxTaskPriorityGet(NULL), NULL);
}


void api_manager_enable_sync(uint32_t ip, uint8_t enable) {
    ESP_LOGI(TAG, "Http request to %i.%i.%i.%i for abilitation: %i", IP_GET_PART_1(ip), IP_GET_PART_2(ip),
             IP_GET_PART_3(ip), IP_GET_PART_4(ip), enable);
}


void api_manager_enable_async(uint32_t ip) {
    struct message msg = {.code = MESSAGE_CODE_REENABLE, .ip = ip};
    xQueueSend(queue, &msg, 0);
}


static void api_task(void *arg) {
    (void)arg;

    for (;;) {
        struct message msg;
        if (xQueueReceive(queue, &msg, portMAX_DELAY) == pdTRUE) {
            switch (msg.code) {
                case MESSAGE_CODE_REENABLE:
                    api_manager_enable_sync(msg.ip, 1);
                    break;
            }
        }
    }

    vTaskDelete(NULL);
}