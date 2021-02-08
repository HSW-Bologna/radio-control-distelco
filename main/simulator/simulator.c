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

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}