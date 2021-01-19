#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "connections.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "network/device.h"
#include "model/model.h"


#define NUM_CONNECTIONS      (MAX_CHANNELS * (MAX_MASTERS_PER_CHANNEL + MAX_MINIONS_PER_CHANNEL))
#define DEVICE_TASK_PRIORITY 2



static void master_communication_task(void *args);
static void minion_communication_task(void *args);
static void master_not_ok(QueueHandle_t queue, size_t channel, size_t master);
static void master_ok(QueueHandle_t queue, size_t channel, size_t master, char *name);
static void minion_not_ok(QueueHandle_t queue, size_t channel, size_t minion);
static void minion_ok(QueueHandle_t queue, size_t channel, size_t minion, char *name);


static const char * TAG                    = "Devices";
static TaskHandle_t tasks[NUM_CONNECTIONS] = {NULL};


void connections_init(void) {}


void connections_clear(void) {
    for (size_t i = 0; i < NUM_CONNECTIONS; i++) {
        if (tasks[i]) {
            vTaskDelete(tasks[i]);
            tasks[i] = NULL;
        }
    }
}


void connections_restart(model_t *model, QueueHandle_t queue) {
    connections_clear();


    for (size_t c = 0; c < MAX_CHANNELS; c++) {
        if (!model_is_channel_configured(model, c))
            continue;

        model_reset_channel_state(model, c);

        for (size_t m = 0; m < model_get_masters_in_channel(model, c); m++) {
            void **args = malloc(sizeof(void *) * 4);
            if (model_get_channel_master_ip(model, (uint32_t *)&args[0], c, m))
                continue;
            args[1] = queue;
            args[2] = (void *)(uintptr_t)c;
            args[3] = (void *)(uintptr_t)m;

            size_t index = c * (MAX_MASTERS_PER_CHANNEL + MAX_MINIONS_PER_CHANNEL) + m;
            xTaskCreate(master_communication_task, "Device master", 4096, args, DEVICE_TASK_PRIORITY, &tasks[index]);
        }

        for (size_t m = 0; m < model_get_minions_in_channel(model, c); m++) {
            void **args = malloc(sizeof(void *) * 4);
            if (model_get_channel_minion_ip(model, (uint32_t *)&args[0], c, m))
                continue;
            args[1] = queue;
            args[2] = (void *)(uintptr_t)c;
            args[3] = (void *)(uintptr_t)m;

            size_t index = c * (MAX_MASTERS_PER_CHANNEL + MAX_MINIONS_PER_CHANNEL) + MAX_MASTERS_PER_CHANNEL + m;
            xTaskCreate(minion_communication_task, "Device minion", 4096, args, DEVICE_TASK_PRIORITY, &tasks[index]);
        }
    }
}


static void master_communication_task(void *args) {
    void **       params  = args;
    uint32_t      ip      = (uint32_t)(uintptr_t)params[0];
    QueueHandle_t queue   = params[1];
    size_t        channel = (size_t)(uintptr_t)params[2];
    size_t        index   = (size_t)(uintptr_t)params[3];
    free(args);

    for (;;) {
        master_not_ok(queue, channel, index);
        int socket = device_connect(ip);
        if (socket < 0) {
            ESP_LOGE(TAG, "Error connecting to device");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
        for (;;) {
            status_packet_t status;
            int             res = device_get_state(socket, &status);

            if (res < 0) {
                ESP_LOGE(TAG, "Error during communication");
                shutdown(socket, 0);
                close(socket);
                break;
            }

            int ok = 1;
            for (size_t i = 0; i < MAX_REMOTE_ADDRESS; i++) {
                if (status.remoteStatus[i] == RemoteLostConnection) {
                    ok = 0;
                    break;
                }
            }

            ok ? master_ok(queue, channel, index, (char *)status.name) : master_not_ok(queue, channel, index);
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    vTaskDelete(NULL);
}


static void minion_communication_task(void *args) {
    void **       params  = args;
    uint32_t      ip      = (uint32_t)(uintptr_t)params[0];
    QueueHandle_t queue   = params[1];
    size_t        channel = (size_t)(uintptr_t)params[2];
    size_t        index   = (size_t)(uintptr_t)params[3];
    free(args);

    for (;;) {
        minion_not_ok(queue, channel, index);
        int socket = device_connect(ip);
        if (socket < 0) {
            ESP_LOGE(TAG, "Error connecting to device");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
        for (;;) {
            status_packet_t status;
            int             res = device_get_state(socket, &status);

            if (res < 0) {
                ESP_LOGE(TAG, "Error during communication");
                shutdown(socket, 0);
                close(socket);
                break;
            }

            int ok = 1;
            for (size_t i = 0; i < MAX_REMOTE_ADDRESS; i++) {
                if (status.remoteStatus[i] == RemoteLostConnection) {
                    ok = 0;
                    break;
                }
            }

            ok ? minion_ok(queue, channel, index, (char *)status.name) : minion_not_ok(queue, channel, index);
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    vTaskDelete(NULL);
}


static void master_not_ok(QueueHandle_t queue, size_t channel, size_t master) {
    device_update_t update = {.channel = channel, .master = 1, .index = master, .ok = 0};
    xQueueSend(queue, &update, portMAX_DELAY);
}


static void master_ok(QueueHandle_t queue, size_t channel, size_t master, char *name) {
    device_update_t update = {.channel = channel, .master = 1, .index = master, .ok = 1};
    strncpy(update.name, name, CONFIG_NAME_LEN);
    update.name[CONFIG_NAME_LEN - 1] = '\0';
    xQueueSend(queue, &update, portMAX_DELAY);
}


static void minion_not_ok(QueueHandle_t queue, size_t channel, size_t minion) {
    device_update_t update = {.channel = channel, .master = 0, .index = minion, .ok = 0};
    xQueueSend(queue, &update, portMAX_DELAY);
}


static void minion_ok(QueueHandle_t queue, size_t channel, size_t minion, char *name) {
    device_update_t update = {.channel = channel, .master = 0, .index = minion, .ok = 1};
    strncpy(update.name, name, CONFIG_NAME_LEN);
    update.name[CONFIG_NAME_LEN - 1] = '\0';
    xQueueSend(queue, &update, portMAX_DELAY);
}