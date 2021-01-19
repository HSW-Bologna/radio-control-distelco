
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
#include "peripherals/management_board.h"
#include "model/model.h"
#include "esp_ping.h"
#include "ping/ping.h"

#define MAGIC "DST-ROIP-AXDDRFS"
#define PORT  8082

/**
 * Elenco valori del campo <command>
 */
enum {
    command_status = 0,
    command_save   = 1,
    command_set_ip,
    command_set_sysmode,
    command_set_name,
    command_set_in1_gain,
    command_set_in2_gain,
    command_set_out1_gain,
    command_set_out2_gain,
    command_set_remotes,
    command_set_single_remote,
} command_packet_commands;

/**
 * struttura dati del messaggio
 *
 */
typedef struct __attribute__((__packed__)) {
    uint8_t sync[16];     // 16 byte sync = "DST-ROIP-AXDDRFS"
    uint8_t version;
    uint8_t command;
} command_packet_t;

static const char *TAG = "Main";


void app_main() {
    model_t model;

    system_spi_init();
    heartbeat_init(2);
    management_board_init();

    ethernet_init(model_get_my_ip(&model));

    model_init(&model);
    controller_init(&model);
    controller_gui_init(&model);

    // xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Begin main loop");

    for (;;) {
        controller_manage_gui(&model);
        controller_manage(&model);

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
