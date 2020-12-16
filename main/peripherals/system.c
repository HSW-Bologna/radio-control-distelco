/*
 * Modulo di inizializzazione del sistema.
 * Contiene le funzioni di configurazione (specifica) di I2C ed SPI
 */
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "hardwareprofile.h"
#include "system.h"

void system_spi_init(void) {
    esp_err_t        ret;
    spi_bus_config_t buscfg = {.miso_io_num     = SPI_DIN,
                               .mosi_io_num     = SPI_DOUT,
                               .sclk_io_num     = SPI_CLK,
                               .quadwp_io_num   = -1,
                               .quadhd_io_num   = -1,
                               .max_transfer_sz = 128};
    // Initialize the SPI bus
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
}