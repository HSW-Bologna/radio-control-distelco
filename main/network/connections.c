#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "connections.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "network/device.h"
#include "model/model.h"


#define NUM_CONNECTIONS      (MAX_CHANNELS * (MAX_MASTERS_PER_CHANNEL + MAX_MINIONS_PER_CHANNEL))
#define DEVICE_TASK_PRIORITY 2



static void device_communication_task(void *args);
static void device_not_ok(QueueHandle_t queue, size_t channel, int master, size_t index);
static void device_update(QueueHandle_t queue, size_t channel, int master, size_t index, status2_packet_t *packet);


static const char *       TAG                      = "Devices";
static TaskHandle_t       tasks[NUM_CONNECTIONS]   = {NULL};
static EventGroupHandle_t stopped[NUM_CONNECTIONS] = {NULL};


void connections_init(void) {
    for (size_t i = 0; i < NUM_CONNECTIONS; i++) {
        stopped[i] = xEventGroupCreate();
        xEventGroupSetBits(stopped[i], 1);
    }
}


void connections_clear(void) {
    for (size_t i = 0; i < NUM_CONNECTIONS; i++) {
        if (tasks[i]) {
            xTaskNotify(tasks[i], 1, eIncrement);
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
            void **args = malloc(sizeof(void *) * 6);
            if (model_get_channel_master_ip(model, (uint32_t *)&args[0], c, m))
                continue;

            size_t index = c * (MAX_MASTERS_PER_CHANNEL + MAX_MINIONS_PER_CHANNEL) + m;

            args[1] = queue;
            args[2] = stopped[index];
            args[3] = (void *)(uintptr_t)c;
            args[4] = (void *)(uintptr_t)m;
            args[5] = (void *)(uintptr_t)1;

            xTaskCreate(device_communication_task, "Device master", 4096, args, DEVICE_TASK_PRIORITY, &tasks[index]);
        }

        for (size_t m = 0; m < model_get_minions_in_channel(model, c); m++) {
            void **args = malloc(sizeof(void *) * 6);
            if (model_get_channel_minion_ip(model, (uint32_t *)&args[0], c, m))
                continue;

            size_t index = c * (MAX_MASTERS_PER_CHANNEL + MAX_MINIONS_PER_CHANNEL) + MAX_MASTERS_PER_CHANNEL + m;

            args[1] = queue;
            args[2] = stopped[index];
            args[3] = (void *)(uintptr_t)c;
            args[4] = (void *)(uintptr_t)m;
            args[5] = (void *)(uintptr_t)0;

            xTaskCreate(device_communication_task, "Device minion", 4096, args, DEVICE_TASK_PRIORITY, &tasks[index]);
        }
    }
}


static void device_communication_task(void *args) {
    uint32_t notification = 0;
#define WAIT_OR_YIELD(ms)                                                                                              \
    xTaskNotifyWait(0, 0, &notification, pdMS_TO_TICKS(ms));                                                           \
    if (notification > 0)                                                                                              \
        break;


    void **            params  = args;
    uint32_t           ip      = (uint32_t)(uintptr_t)params[0];
    QueueHandle_t      queue   = params[1];
    EventGroupHandle_t group   = params[2];
    size_t             channel = (size_t)(uintptr_t)params[3];
    size_t             index   = (size_t)(uintptr_t)params[4];
    int                master  = (int)(uintptr_t)params[5];
    free(args);

    xEventGroupWaitBits(group, 1, 1, 1, portMAX_DELAY);
    ESP_LOGI(TAG, "Start communication task for ip %i.%i.%i.%i", IP_GET_PART_1(ip), IP_GET_PART_2(ip),
             IP_GET_PART_3(ip), IP_GET_PART_4(ip));

    for (;;) {
        WAIT_OR_YIELD(0);

        device_not_ok(queue, channel, master, index);
        int socket = device_connect(ip);
        if (socket < 0) {
            ESP_LOGE(TAG, "Error connecting to device");
            WAIT_OR_YIELD(2000);
            continue;
        }

        WAIT_OR_YIELD(500);

        for (;;) {
            status2_packet_t status;
            int              res = device_get_state(socket, &status);

            if (res < 0) {
                ESP_LOGE(TAG, "Error during communication with %s %i", master ? "master" : "slave", index);
                shutdown(socket, 0);
                close(socket);
                break;
            }

            // ESP_LOGI(TAG, "debug task for ip %i.%i.%i.%i %i", IP_GET_PART_1(ip), IP_GET_PART_2(ip),
            // IP_GET_PART_3(ip), IP_GET_PART_4(ip), status.remoteStatus[0]);
            device_update(queue, channel, master, index, &status);

            if (xTaskNotifyWait(0, 0, &notification, pdMS_TO_TICKS(4000)) == pdPASS) {
                ESP_LOGI(TAG, "Socket chiusa");
                shutdown(socket, 0);
                close(socket);
                break;
            }
        }
    }

    ESP_LOGI(TAG, "End communication task for ip %i.%i.%i.%i", IP_GET_PART_1(ip), IP_GET_PART_2(ip), IP_GET_PART_3(ip),
             IP_GET_PART_4(ip));
    xEventGroupSetBits(group, 1);
    vTaskDelete(NULL);
}


static void device_update(QueueHandle_t queue, size_t channel, int master, size_t index, status2_packet_t *packet) {
    int guasto = 0;
    for (size_t i = 0; i < MAX_REMOTE_ADDRESS; i++) {
        if (packet->remoteStatus[i] == RemoteLostConnection) {
            guasto = 1;
            break;
        }
    }

    device_update_t update = {
        .channel = channel,
        .master  = master,
        .index   = index,
        .info =
            {
                .guasto_radio   = guasto,
                .failed_tx      = packet->failed_tx,
                .guasto_antenna = packet->failed_antenna,
                .connected      = 1,
            },
    };
    strncpy(update.info.name, (const char *)packet->name, CONFIG_NAME_LEN);
    strncpy(update.info.fw_version, (const char *)packet->fw_version, CONFIG_NAME_LEN);
    memcpy(update.info.remotes, packet->remotes, sizeof(uint32_t) * MAX_REMOTE_ADDRESS);

    for (size_t i = 0; i < MAX_REMOTE_ADDRESS; i++)
        update.info.remoteStatus[i] = packet->remoteStatus[i];
    xQueueSend(queue, &update, portMAX_DELAY);
}


static void device_not_ok(QueueHandle_t queue, size_t channel, int master, size_t index) {
    device_update_t update = {
        .channel = channel,
        .master  = master,
        .index   = index,
        .info    = {.connected = 0},
    };
    xQueueSend(queue, &update, portMAX_DELAY);
}