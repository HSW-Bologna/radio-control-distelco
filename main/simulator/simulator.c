#include <unistd.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"

#include "utils/utils.h"
#include "view/view.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "display/monitor.h"
#include "indev/mouse.h"


void test(void *arg) {
    for (;;) {
        vTaskDelay(1000);
        printf("Hello\n");
    }
}

void app_main(void *arg) {
    (void)arg;
    model_t model = {0};

    monitor_init();
    mouse_init();

    model_init(&model);
    view_init(&model, monitor_flush, mouse_read);
    controller_init(&model);

    for (;;) {
        controller_manage_gui(&model);
        controller_manage(&model);

        model_set_connected(&model, 1);
        device_info_t info1 = {.connected    = 1,
                               .name         = "72.232!",
                               .remoteStatus = {1, 1, 0, 0},
                               .remotes      = {IP_ADDR(192, 168, 1, 12), IP_ADDR(192, 168, 1, 13), 0, 0}};
        model_update_channel(&model, (device_update_t){.channel = 0, .master = 1, .index = 0, .info = info1});
        device_info_t minion1 = {.connected = 1, .name = "MCH 1"};
        model_update_channel(&model, (device_update_t){.channel = 0, .master = 0, .index = 0, .info = minion1});
        device_info_t minion2 = {.connected = 1, .name = "MCH 2"};
        model_update_channel(&model, (device_update_t){.channel = 0, .master = 0, .index = 1, .info = minion2});

        device_info_t info2 = {.connected = 1, .name = "VVF 41"};
        model_update_channel(&model, (device_update_t){.channel = 1, .master = 1, .index = 0, .info = info2});

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}