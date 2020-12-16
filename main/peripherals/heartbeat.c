/*
 * Modulo di gestione del LED_RUN. Contiene un task che fa lampeggiare il led ogni secondo
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "hardwareprofile.h"

/**
 * Task di inattivita', sempre viva. Accende e spegne il led di attivita'
 */
static void task_heartbeat() {
    TickType_t last;
    last = xTaskGetTickCount();
    while (1) {
        // ESP_LOGI("IDLE CHECK", "xPortGetFreeHeapSize: %d \t tasknum: %d", xPortGetFreeHeapSize(),
        // uxTaskGetNumberOfTasks());

        gpio_set_level(LED_RUN, 0);
        vTaskDelayUntil(&last, 1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_RUN, 1);
        vTaskDelayUntil(&last, 1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}


void heartbeat_init(int priority) {
    gpio_set_direction(LED_RUN, GPIO_MODE_OUTPUT);
    xTaskCreate(task_heartbeat, "idle", 2048, NULL, priority, NULL);
}