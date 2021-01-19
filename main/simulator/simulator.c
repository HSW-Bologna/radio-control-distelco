#include <unistd.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"

#include "utils/utils.h"
#include "view/view.h"
#include "simulator_controller.h"
#include "gui.h"
#include "simulator_storage.h"


void test(void *arg) {
    for (;;) {
        vTaskDelay(1000);
        printf("Hello\n");
    }
}

void app_main(void *arg) {
    (void)arg;
    model_t model = {0};

    model_init(&model);
    controller_init(&model);
    view_init(&model);

    for (;;) {
        controller_manage_gui(&model);
        usleep(10000UL);
    }

    vTaskDelete(NULL);
}