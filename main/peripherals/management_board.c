#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

#include "hardwareprofile.h"
#include "management_board.h"
#include "system.h"

struct task_message {
    size_t rele;
    int    level;
};


static void management_board_task(void *arg);
static void build_packet(uint8_t *buffer, uint8_t outputs);


static const char *        TAG = "Management";
static spi_device_handle_t spi;
static SemaphoreHandle_t   sem   = NULL;
static QueueHandle_t       queue = NULL;
static struct {
    uint8_t input;
    uint8_t cables[4];
    uint8_t output;
} state;
static int     newdata = 0;
static uint8_t received_packet[16];


void management_board_init(void) {
    sem   = xSemaphoreCreateMutex();
    queue = xQueueCreate(8, sizeof(struct task_message));

    spi_device_interface_config_t devcfg = {.clock_speed_hz = 2 * 1000 * 1000,
                                            .mode           = 0,
                                            .spics_io_num   = SPI_CS3,
                                            .queue_size     = 10,
                                            .pre_cb         = NULL,
                                            .post_cb        = NULL};

    // Attach the LCD to the SPI bus
    esp_err_t ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    assert(ret == ESP_OK);

    xTaskCreate(management_board_task, "Management board", 4096, NULL, 3, NULL);
}


void management_board_set_relay(uint8_t relay, int value) {
    struct task_message message = {.rele = relay, .level = !value};
    xQueueSend(queue, &message, 0);
}


uint8_t management_board_cable_anomaly(uint8_t cables) {
    xSemaphoreTake(sem, portMAX_DELAY);
    uint8_t anomalies = (state.cables[0] > 0) | ((state.cables[1] > 0) << 1) | ((state.cables[2] > 0) << 2) |
                        ((state.cables[3] > 0) << 3);
    xSemaphoreGive(sem);
    return anomalies & cables;
}


uint8_t management_board_power_supply_anomaly(void) {
    xSemaphoreTake(sem, portMAX_DELAY);
    uint8_t res = state.input;
    xSemaphoreGive(sem);
    return res;
}


void management_board_read_response(uint8_t *response) {
    xSemaphoreTake(sem, portMAX_DELAY);
    memcpy(response, received_packet, 14);
    newdata = 0;
    xSemaphoreGive(sem);
}


int management_board_new_data(void) {
    xSemaphoreTake(sem, portMAX_DELAY);
    int res = newdata;
    xSemaphoreGive(sem);
    return res;
}


static void build_packet(uint8_t *buffer, uint8_t outputs) {
    memset(buffer, 0x55, 4);
    buffer[4] = outputs;
    memset(&buffer[5], 0xAA, 14 - 5);
}


static int validate_packet(uint8_t *buffer) {
    if (buffer[0] != 0x55)
        return 0;
    if (buffer[1] != 0x55)
        return 0;
    if (buffer[2] != 0x55)
        return 0;
    if (buffer[3] != 0x55)
        return 0;
    if (buffer[13] != 0xAA)
        return 0;
    return 1;
}


static int update_state(void) {
    uint8_t buffer[16];
    uint8_t receive[16];
    xSemaphoreTake(sem, portMAX_DELAY);
    build_packet(buffer, state.output);
    xSemaphoreGive(sem);

    esp_err_t         ret;
    spi_transaction_t t = {.length    = 14 * 8,     // transaction length is in bits
                           .tx_buffer = buffer,
                           .rx_buffer = receive};

    system_spi_take();
    ret = spi_device_polling_transmit(spi, &t);     // Transmit!
    ESP_ERROR_CHECK(ret);
    system_spi_give();

    xSemaphoreTake(sem, portMAX_DELAY);
    memcpy(received_packet, receive, 14);
    newdata = 1;

    if (validate_packet(receive)) {
        state.input     = receive[4];
        state.cables[0] = receive[5];
        state.cables[1] = receive[6];
        state.cables[2] = receive[7];
        state.cables[3] = receive[8];
        xSemaphoreGive(sem);
        return 0;
    } else {
        // ESP_LOGW(TAG, "Error while polling underlaying management board!");
        // ESP_LOG_BUFFER_HEX(TAG, receive, 14);
        xSemaphoreGive(sem);
        return -1;
    }
}


static void management_board_task(void *arg) {
    struct task_message message;
    int                 power_supply_anomaly = -1;

    for (;;) {
        if (xQueueReceive(queue, &message, pdMS_TO_TICKS(500)) == pdTRUE) {
            xSemaphoreTake(sem, portMAX_DELAY);
            if (message.level)
                state.output |= 1 << message.rele;
            else
                state.output &= ~(1 << message.rele);
            xSemaphoreGive(sem);
        }

        if (update_state() == 0) {
            uint8_t anomaly = management_board_power_supply_anomaly();
            if (anomaly != power_supply_anomaly) {
                ESP_LOGI(TAG, "Anomalia sull'alimentazione");

                xSemaphoreTake(sem, portMAX_DELAY);
                if (anomaly)
                    state.output &= ~(1 << MANAGEMENT_BOARD_RELAY_POWER_SUPPLY_ANOMALY);
                else
                    state.output |= 1 << MANAGEMENT_BOARD_RELAY_POWER_SUPPLY_ANOMALY;
                xSemaphoreGive(sem);

                power_supply_anomaly = anomaly;
                update_state();
            }
        }
    }

    vTaskDelete(NULL);
}