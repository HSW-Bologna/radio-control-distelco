#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "lvgl.h"

#include "peripherals/heartbeat.h"
#include "peripherals/hardwareprofile.h"
#include "peripherals/ethernet.h"
#include "peripherals/system.h"
#include "controller/gui.h"
#include "peripherals/display/SSD2119.h"
#include "model/model.h"

static const char *TAG = "Main";

void app_main() {
    model_t model;

    ethernet_init();
    system_spi_init();
    heartbeat_init(2);

    controller_gui_init(&model);

    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        controller_manage_gui(&model);

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
