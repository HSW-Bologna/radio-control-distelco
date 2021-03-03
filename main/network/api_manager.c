#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "tcpip_adapter.h"
#include "esp_http_client.h"

#include "model/model.h"


struct response_buffer {
    uint8_t *pointer;
    size_t   max;
};


typedef enum { MESSAGE_CODE_REENABLE } message_code_t;


struct message {
    message_code_t code;

    union {
        uint32_t ip;
    };
};


static esp_err_t _http_event_handler(esp_http_client_event_t *evt);
static void      api_task(void *arg);


static const char * TAG   = "API";
static xQueueHandle queue = NULL;


void api_manager_init(void) {
    queue = xQueueCreate(8, sizeof(struct message));
    xTaskCreate(api_task, "Async API", 4096*2, NULL, uxTaskPriorityGet(NULL), NULL);
}


void api_manager_enable_sync(uint32_t ip, uint8_t enable) {
    uint8_t                buffer[2048] = {0};
    struct response_buffer response     = {.pointer = buffer, .max = 2048};

    char host[32] = {0};
    snprintf(host, 32, "%i.%i.%i.%i", IP_GET_PART_1(ip), IP_GET_PART_2(ip), IP_GET_PART_3(ip), IP_GET_PART_4(ip));

    esp_http_client_config_t config = {
        .timeout_ms    = 4000,
        .host          = host,
        .path          = "/commands",
        .query         = enable ? "transmit=enable" : "transmit=disable",
        .event_handler = _http_event_handler,
        .user_data     = &response,     // Pass address of local buffer to get response
        //.disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d", esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}


void api_manager_enable_async(uint32_t ip) {
    struct message msg = {.code = MESSAGE_CODE_REENABLE, .ip = ip};
    xQueueSend(queue, &msg, 0);
}


static void api_task(void *arg) {
    (void)arg;

    for (;;) {
        struct message msg;
        if (xQueueReceive(queue, &msg, portMAX_DELAY) == pdTRUE) {
            switch (msg.code) {
                case MESSAGE_CODE_REENABLE:
                    api_manager_enable_sync(msg.ip, 1);
                    break;
            }
        }
    }

    vTaskDelete(NULL);
}


static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    static char *output_buffer;     // Buffer to store response of http request from event handler
    static int   output_len;        // Stores number of bytes read
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary
             * data. However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    struct response_buffer *response = evt->user_data;
                    if (response->max > output_len + evt->data_len) {
                        memcpy(response->pointer + output_len, evt->data, evt->data_len);
                    } else {
                        ESP_LOGE(TAG, "Error: not enough space for response");
                    }
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *)malloc(esp_http_client_get_content_length(evt->client));
                        output_len    = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            struct response_buffer *response = evt->user_data;
            response->max                    = output_len;
            output_len                       = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}