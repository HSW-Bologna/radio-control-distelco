#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <assert.h>
#include "app_config.h"
#include "esp_log.h"
#include "lightmodbus/base.h"
#include "lightmodbus/lightmodbus.h"
#include "lightmodbus/slave.h"
#include "lightmodbus/slave_func.h"
#include "model/model.h"


static ModbusError register_callback(const ModbusSlave *status, const ModbusRegisterCallbackArgs *args,
                                     ModbusRegisterCallbackResult *result);
static ModbusError exception_callback(const ModbusSlave *slave, uint8_t function, ModbusExceptionCode code);
static void        modbus_server_task(void *args);


static const char *TAG = "Modbus server";


xSemaphoreHandle sem = NULL;
struct {
    uint16_t errors;
    uint16_t channels[MAX_CHANNELS];
} modbus_server_state = {0};


void modbus_server_init(void) {
    static StaticSemaphore_t static_sem;
    sem = xSemaphoreCreateMutexStatic(&static_sem);

    static StackType_t  task_stack[4096] = {0};
    static StaticTask_t static_task;
    xTaskCreateStatic(modbus_server_task, TAG, sizeof(task_stack) / sizeof(StackType_t), NULL, 1, task_stack,
                      &static_task);
}


void modbus_server_update(model_t *pmodel) {
    for (size_t i = 0; i < 4; i++) {
        int ok = model_is_cable_ok(pmodel, i);
        xSemaphoreTake(sem, portMAX_DELAY);
        modbus_server_state.errors = (modbus_server_state.errors & (~(1 << i))) | ((ok > 0) << i);
        xSemaphoreGive(sem);
    }

    for (size_t c = 0; c < MAX_CHANNELS; c++) {
        size_t shift = 8 + c;
        int    ok    = model_is_channel_ok(pmodel, c);

        xSemaphoreTake(sem, portMAX_DELAY);
        modbus_server_state.errors      = (modbus_server_state.errors & (~(1 << shift))) | ((ok == 0) << shift);
        modbus_server_state.channels[c] = 0;

        for (size_t i = 0; i < model_get_masters_in_channel(pmodel, c); i++) {
            device_info_t info              = model_get_channel_master_info(pmodel, c, i);
            modbus_server_state.channels[c] = (channel_device_info_ok(&info) == 0) << i;
        }

        for (size_t i = 0; i < model_get_minions_in_channel(pmodel, c); i++) {
            device_info_t info              = model_get_channel_minion_info(pmodel, c, i);
            modbus_server_state.channels[c] = (channel_device_info_ok(&info) == 0) << (8 + i);
        }
        xSemaphoreGive(sem);
    }
}


static void modbus_server_task(void *args) {
    (void)args;
    ModbusSlave     minion;
    ModbusErrorInfo err;

    err = modbusSlaveInit(&minion,
                          register_callback,                  // Callback for register operations
                          exception_callback,                 // Callback for handling slave exceptions (optional)
                          modbusDefaultAllocator,             // Memory allocator for allocating responses
                          modbusSlaveDefaultFunctions,        // Set of supported functions
                          modbusSlaveDefaultFunctionCount     // Number of supported functions
    );

    // Check for errors
    assert(modbusIsOk(err) && "modbusSlaveInit() failed");

    int                sockfd, connfd;
    unsigned int       len = 0;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        ESP_LOGE(TAG, "socket creation failed: %s", strerror(errno));
        vTaskDelete(NULL);
    } else {
        ESP_LOGI(TAG, "Socket successfully created...");
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(MODBUS_SERVER_PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
        ESP_LOGE(TAG, "socket bind failed: %s", strerror(errno));
        vTaskDelete(NULL);
    } else {
        ESP_LOGI(TAG, "Socket successfully bound...");
    }

    for (;;) {
        // Now server is ready to listen and verification
        if ((listen(sockfd, 1)) != 0) {
            ESP_LOGE(TAG, "Listen failed: %s", strerror(errno));
            break;
        } else {
            ESP_LOGI(TAG, "Server listening...");
        }
        len = sizeof(cli);

        // Accept the data packet from client and verification
        connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
        if (connfd < 0) {
            ESP_LOGE(TAG, "Server accept failed: %s", strerror(errno));
            break;
        } else {
            ESP_LOGI(TAG, "server accepted a client...");
        }

        // Service loop
        for (;;) {
            uint8_t buffer[256] = {0};

            int len = read(connfd, buffer, sizeof(buffer));

            if (len < 0) {
                ESP_LOGE(TAG, "Error while reading from socket: %s", strerror(errno));
                break;
            }

            err = modbusParseRequestTCP(&minion, buffer, (uint16_t)len);

            if (modbusIsOk(err)) {
                int rlen = modbusSlaveGetResponseLength(&minion);
                if (rlen > 0) {
                    if (write(connfd, modbusSlaveGetResponse(&minion), rlen) != rlen) {
                        ESP_LOGE(TAG, "Error while writing to socket: %s", strerror(errno));
                        break;
                    }
                } else {
                    ESP_LOGI(TAG, "Empty response");
                }
            } else if (err.error != MODBUS_ERROR_ADDRESS) {
                ESP_LOGW(TAG, "Invalid request with source %i and error %i", err.source, err.error);
                ESP_LOG_BUFFER_HEX(TAG, buffer, len);
            }
        }
    }

    // After chatting close the socket
    close(sockfd);

    vTaskDelete(NULL);
}


static ModbusError register_callback(const ModbusSlave *status, const ModbusRegisterCallbackArgs *args,
                                     ModbusRegisterCallbackResult *result) {

    // ESP_LOGI(TAG, "%i %i %i %i", args->query, args->type, args->index, args->value);
    switch (args->query) {
        // R/W access check
        case MODBUS_REGQ_R_CHECK:
        case MODBUS_REGQ_W_CHECK:
            switch (args->type) {
                case MODBUS_INPUT_REGISTER: result->exceptionCode = MODBUS_EXCEP_NONE; break;
                default: result->exceptionCode = MODBUS_EXCEP_ILLEGAL_FUNCTION; break;
            }
            break;

        // Read register
        case MODBUS_REGQ_R:
            switch (args->type) {
                case MODBUS_INPUT_REGISTER: {
                    result->exceptionCode = MODBUS_EXCEP_NONE;
                    xSemaphoreTake(sem, portMAX_DELAY);
                    switch (args->index) {
                        case 0: result->value = modbus_server_state.errors; break;

                        case 1 ... 5: result->value = modbus_server_state.channels[args->index - 1]; break;

                        default: result->exceptionCode = MODBUS_EXCEP_ILLEGAL_ADDRESS; break;
                    }
                    xSemaphoreGive(sem);
                    break;
                }
                default: result->exceptionCode = MODBUS_EXCEP_ILLEGAL_FUNCTION; break;
            }
            break;

        // Write register
        case MODBUS_REGQ_W:
            switch (args->type) {
                case MODBUS_HOLDING_REGISTER: {
                    result->exceptionCode = MODBUS_EXCEP_ILLEGAL_ADDRESS;
                    break;
                }
                case MODBUS_COIL: result->exceptionCode = MODBUS_EXCEP_ILLEGAL_FUNCTION; break;
                case MODBUS_INPUT_REGISTER: result->exceptionCode = MODBUS_EXCEP_ILLEGAL_FUNCTION; break;
                default: break;
            }
            break;
    }

    // Always return MODBUS_OK
    return MODBUS_OK;
}


static ModbusError exception_callback(const ModbusSlave *slave, uint8_t function, ModbusExceptionCode code) {
    ESP_LOGI(TAG, "Slave reports an exception %d (function %d)", code, function);

    // Always return MODBUS_OK
    return MODBUS_OK;
}