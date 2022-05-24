
#include <stdlib.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <arpa/inet.h>
#include "lvgl.h"

#include "peripherals/heartbeat.h"
#include "peripherals/hardwareprofile.h"
#include "peripherals/ethernet.h"
#include "peripherals/system.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "peripherals/display/SSD2119.h"
#include "peripherals/display/display.h"
#include "peripherals/display/tsc2046.h"
#include "peripherals/management_board.h"
#include "model/model.h"
#include "esp_ping.h"
#include "ping/ping.h"


static const char *TAG = "Main";


void app_main() {
    model_t model;

    system_spi_init();
    heartbeat_init(2);
    management_board_init();
    display_init();
    ssd2119_init();
    tsc2046_init();

    model_init(&model);
    ethernet_init(model_get_my_ip(&model));
    view_init(&model, ssd2119_flush, tsc2046_touch_read);
    controller_init(&model);

    ESP_LOGI(TAG, "Begin main loop");

    for (;;) {
        controller_manage_gui(&model);
        controller_manage(&model);

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
