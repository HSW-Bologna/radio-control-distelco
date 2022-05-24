#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "spi_devices/io/MCP23x17/mcp23x17.h"
#include "spi_ports/esp-idf/esp_idf_spi_port.h"
#include "hardwareprofile.h"
#include "management_board.h"
#include "system.h"


#define LINEA_ANOMALIA_ALIMENTAZIONE MCP23X17_GPIO_9


struct task_message {
    size_t rele;
    int    level;
};


static void management_board_task(void *arg);
static void build_packet(uint8_t *buffer, uint8_t outputs);
static int  cs_control(int level);


static const char       *TAG = "Management";
spi_device_handle_t      spi;
static SemaphoreHandle_t sem    = NULL;
static QueueHandle_t     queue  = NULL;
static spi_driver_t      driver = {
         .spi_cs_control = cs_control,
         .spi_exchange   = esp_idf_spi_port_exchange,
         .user_data      = NULL,
};

static struct {
    uint8_t input;
    uint8_t cables[4];
    uint8_t error;
} state;
static int newdata = 0;


static int cs_control(int level) {
    vTaskDelay(2);
    gpio_set_level(SPI_CS3, level);
    vTaskDelay(2);
    return 0;
}


void management_board_init(void) {
    sem   = xSemaphoreCreateMutex();
    queue = xQueueCreate(8, sizeof(struct task_message));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 500 * 1000,
        .mode           = 0,
        .spics_io_num   = -1,
        .queue_size     = 10,
        .pre_cb         = NULL,
        .post_cb        = NULL,
    };

    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &devcfg, &spi));
    driver.user_data = spi;

    gpio_config_t conf = {
        .intr_type = GPIO_INTR_DISABLE, .mode = GPIO_MODE_OUTPUT, .pin_bit_mask = BIT64(SPI_CS4) | BIT64(SPI_CS3)};
    gpio_config(&conf);
    gpio_set_level(SPI_CS3, 1);
    gpio_set_level(SPI_CS4, 1);

    system_spi_take();
    mcp23x17_set_iocon_register(driver, MCP23X17_DEFAULT_ADDR, 0);
    mcp23x17_set_gpio_direction(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_9, MCP23X17_OUTPUT_MODE);
    mcp23x17_set_gpio_direction(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_10, MCP23X17_OUTPUT_MODE);
    mcp23x17_set_gpio_direction(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_11, MCP23X17_OUTPUT_MODE);
    mcp23x17_set_gpio_direction(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_12, MCP23X17_OUTPUT_MODE);
    mcp23x17_set_gpio_direction(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_1, MCP23X17_INPUT_MODE);
    mcp23x17_set_gpio_direction(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_2, MCP23X17_INPUT_MODE);

    mcp23x17_set_gpio_level(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_9, 0);
    mcp23x17_set_gpio_level(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_10, 0);
    mcp23x17_set_gpio_level(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_11, 0);
    mcp23x17_set_gpio_level(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_12, 0);
    system_spi_give();

    xTaskCreate(management_board_task, TAG, 4096, NULL, 3, NULL);
}


void management_board_set_relay(uint8_t relay, int value) {
    struct task_message message = {.level = !value};
    switch (relay) {
        case MANAGEMENT_BOARD_RELAY_1: message.rele = MCP23X17_GPIO_9; break;
        case MANAGEMENT_BOARD_RELAY_2: message.rele = MCP23X17_GPIO_10; break;
        case MANAGEMENT_BOARD_RELAY_3: message.rele = MCP23X17_GPIO_11; break;
        case MANAGEMENT_BOARD_RELAY_4: message.rele = MCP23X17_GPIO_12; break;
    }
    xQueueSend(queue, &message, 0);
}


int management_board_error(void) {
    xSemaphoreTake(sem, portMAX_DELAY);
    int res = state.error;
    xSemaphoreGive(sem);

    return res;
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


int management_board_new_data(void) {
    xSemaphoreTake(sem, portMAX_DELAY);
    int res = newdata;
    xSemaphoreGive(sem);
    return res;
}


static int update_state(void) {
    system_spi_take();

    state.cables[0]   = 0;
    state.cables[1]   = 0;
    state.cables[2]   = 0;
    state.cables[3]   = 0;
    int power_anomaly = 1;
    mcp23x17_get_gpio_level(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_1, &power_anomaly);
    int battery = 1;
    mcp23x17_get_gpio_level(driver, MCP23X17_DEFAULT_ADDR, MCP23X17_GPIO_2, &battery);
    state.input = (power_anomaly > 0) | ((battery > 0) << 1);
    state.error = 0;
    system_spi_give();
    return 0;
}


static void management_board_task(void *arg) {
    struct task_message message;
    int                 power_supply_anomaly = -1;



    for (;;) {
        if (xQueueReceive(queue, &message, pdMS_TO_TICKS(500)) == pdTRUE) {
            system_spi_take();
            mcp23x17_set_gpio_level(driver, MCP23X17_DEFAULT_ADDR, message.rele, message.level);
            system_spi_give();
        }

        update_state();
        uint8_t anomaly = management_board_power_supply_anomaly();
        if (anomaly != power_supply_anomaly) {
            ESP_LOGI(TAG, "Anomalia sull'alimentazione");
            mcp23x17_set_gpio_level(driver, MCP23X17_DEFAULT_ADDR, LINEA_ANOMALIA_ALIMENTAZIONE, !anomaly);
            power_supply_anomaly = anomaly;
        }
    }

    vTaskDelete(NULL);
}