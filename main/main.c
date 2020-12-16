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

static const char *TAG = "Main";

void app_main() {
    ethernet_init();
    system_spi_init();
    heartbeat_init(2);

    controller_gui_init();

    ESP_LOGI(TAG, "Begin main loop");

    lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_t *lbl = lv_label_create(btn, NULL);
    lv_label_set_text(lbl, "Ciao");
    lv_obj_align(btn, NULL, LV_ALIGN_CENTER, 0, 0);

    for (;;) {
        lv_task_handler();
        //lv_tick_inc(5);

        /*lv_color_t buffer[30 * 30] = {0};
        memset(buffer, 0xFF, 30 * 30 * sizeof(lv_color_t));
        lv_area_t area = {.x1 = 10, .y1 = 10, .x2 = 30, .y2 = 30};

        ssd2119_flush(NULL, &area, buffer);*/

        // ssd2119_test(0xFFFF);

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
