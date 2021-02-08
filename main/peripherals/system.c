/*
 * Modulo di inizializzazione del sistema.
 * Contiene le funzioni di configurazione (specifica) di I2C ed SPI
 */
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "hardwareprofile.h"
#include "system.h"


static xSemaphoreHandle sem = NULL;

void system_spi_init(void) {
    sem = xSemaphoreCreateMutex();

    esp_err_t        ret;
    spi_bus_config_t buscfg = {.miso_io_num     = SPI_DIN,
                               .mosi_io_num     = SPI_DOUT,
                               .sclk_io_num     = SPI_CLK,
                               .quadwp_io_num   = -1,
                               .quadhd_io_num   = -1,
                               .max_transfer_sz = 4094};
    // Initialize the SPI bus
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);

    //gpio_set_drive_capability(SPI_DOUT, GPIO_DRIVE_CAP_0);
    //gpio_set_drive_capability(SPI_CLK, GPIO_DRIVE_CAP_0);
}


void system_spi_take(void) {
    xSemaphoreTake(sem, portMAX_DELAY);
    // vTaskDelay(pdMS_TO_TICKS(20));
}


void system_spi_give(void) {
    xSemaphoreGive(sem);
}