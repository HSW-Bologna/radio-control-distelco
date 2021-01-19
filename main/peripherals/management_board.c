#include <string.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

#include "hardwareprofile.h"


static const char *        TAG = "Management";
static spi_device_handle_t spi;


void management_board_init(void) {
    spi_device_interface_config_t devcfg = {.clock_speed_hz = 10 * 1000,     // Clock out at 40 MHz
                                            .mode           = 0,             // SPI mode 0
                                            .spics_io_num   = SPI_CS3,       // CS pin
                                            .queue_size     = 10,
                                            .pre_cb         = NULL,
                                            .post_cb        = NULL};

    // Attach the LCD to the SPI bus
    esp_err_t ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    assert(ret == ESP_OK);
}


void management_board_transaction(uint8_t cmd, uint8_t *receive) {
    static int blink    = 1;
    uint8_t    send[10] = {0};
    memset(send, blink ? 0x55 : 0xAA, 10);
    send[0] = cmd;
    blink   = !blink;

    esp_err_t ret;

    spi_transaction_t t = {.length    = 10 * 8,     // transaction length is in bits
                           .tx_buffer = send,
                           .rx_buffer = receive};

    ret = spi_device_polling_transmit(spi, &t);     // Transmit!
    ESP_ERROR_CHECK(ret);
}