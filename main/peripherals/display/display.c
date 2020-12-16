#include <stdint.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "../hardwareprofile.h"
#include "display.h"

static spi_device_handle_t display_spi;

// data = 1, command = 0
void display_set_data_command(uint8_t dc) {
    gpio_set_level(LCD_DATA_COMMAND, dc);
}

void display_set_reset(uint8_t res) {
    gpio_set_level(LCD_RESET, res);
}

static void display_backlight_init() {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,        // resolution of PWM duty
        .freq_hz         = 1000,                    // frequency of PWM signal
        .speed_mode      = LEDC_LOW_SPEED_MODE,     // timer mode
        .timer_num       = LEDC_TIMER_2,            // timer index
        .clk_cfg         = LEDC_AUTO_CLK,           // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {.channel    = LEDC_CHANNEL_1,
                                          .duty       = 0xFF,
                                          .gpio_num   = LCD_RETRO,
                                          .speed_mode = LEDC_LOW_SPEED_MODE,
                                          .hpoint     = 0,
                                          .timer_sel  = LEDC_TIMER_2};
    ledc_channel_config(&ledc_channel);
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    display_set_backlight(100);
}


void display_init(void) {
    esp_err_t ret;

    gpio_config_t io_conf;
    io_conf.intr_type    = GPIO_INTR_DISABLE;
    io_conf.mode         = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (BIT64(LCD_RESET) | BIT64(LCD_DATA_COMMAND));
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en   = 0;
    gpio_config(&io_conf);

    spi_device_interface_config_t devcfg = {.clock_speed_hz = 15 * 1000 * 1000,     // Clock out at 40 MHz
                                            .mode           = 0,                   // SPI mode 0
                                            .spics_io_num   = LCD_CS,              // CS pin
                                            .queue_size     = 10,
                                            .pre_cb         = NULL,
                                            .post_cb        = NULL};

    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &display_spi);
    assert(ret == ESP_OK);

    display_backlight_init();
}

void display_send_data(uint8_t *data, unsigned int length) {
    esp_err_t ret;
    if (length == 0)
        return;     // no need to send anything

    spi_transaction_t t = {.length    = length * 8,     // transaction length is in bits
                           .tx_buffer = data,
                           .rx_buffer = NULL};

    ret = spi_device_polling_transmit(display_spi, &t);     // Transmit!
    ESP_ERROR_CHECK(ret);
}

void display_set_backlight(int percentage) {
    int duty = (percentage * 0xFF) / 100;
    // gpio_set_level(RETRO, percentage > 0);
    if (duty != ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1)) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    }
}