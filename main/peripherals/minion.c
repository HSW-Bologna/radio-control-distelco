#include <stdint.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "hardwareprofile.h"


spi_device_handle_t handle;


void minion_init(void) {
    esp_err_t ret;

    spi_device_interface_config_t devcfg = {.clock_speed_hz = 1 * 1000 * 1000,     // Clock out at 40 MHz
                                            .mode           = 0,                   // SPI mode 0
                                            .spics_io_num   = SPI_CS2,             // CS pin
                                            .queue_size     = 10,
                                            .pre_cb         = NULL,
                                            .post_cb        = NULL};

    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &handle);
    assert(ret == ESP_OK);
}
