#include "esp_log.h"


static const char *TAG = "Display";


void display_set_backlight(int percentage) {
    ESP_LOGI(TAG, "Setting backlight to %i%%", percentage);
}