#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "storage/configuration.h"
#include "model/model.h"
#include "gel/timer/timecheck.h"
#include "view/view.h"
#include "connections.h"
#include "network/api_manager.h"
#include "peripherals/ethernet.h"
#include "peripherals/storage.h"
#include "peripherals/management_board.h"
#include "utils/utils.h"
#include "modbus_server.h"


static void restart_connections(void);


static const char   *TAG               = "Controller";
static QueueHandle_t queue             = NULL;
static QueueHandle_t deviceq           = NULL;
static int           test              = 0;
static int           update_after_test = 0;


void controller_init(model_t *model) {
    queue   = xQueueCreate(1, sizeof(uint8_t));
    deviceq = xQueueCreate(MAX_CHANNELS * (MAX_MASTERS_PER_CHANNEL + MAX_MINIONS_PER_CHANNEL), sizeof(device_update_t));

    modbus_server_init();
    connections_init();
    storage_init();
    configuration_load(model);
    api_manager_init();
    ethernet_set_ip(model_get_my_ip(model));
    ethernet_set_callbacks(connections_clear, restart_connections);
    view_rebase_page(model, &page_main);
}


void controller_process_msg(view_controller_command_t msg, model_t *model) {
    switch (msg.code) {
        case VIEW_CONTROLLER_COMMAND_REENABLE_TX: api_manager_enable_async(msg.addr); break;

        case VIEW_CONTROLLER_COMMAND_CODE_SAVE_CONFIG:
            if (model_is_to_save(model)) {
                configuration_save(model);
                model_set_saved(model);
            }

            if (model_is_network_config_changed(model)) {
                ESP_LOGI(TAG, "Reset della configurazione di rete");
                ethernet_set_ip(model_get_my_ip(model));
                if (ethernet_is_connected(0)) {
                    connections_restart(model, deviceq);
                }
                model_network_config_clear(model);
            }
            break;

        case VIEW_CONTROLLER_COMMAND_TEST:
            test = msg.test;
            if (test) {
                management_board_set_relay(MANAGEMENT_BOARD_RELAY_1, 0);
                management_board_set_relay(MANAGEMENT_BOARD_RELAY_2, 0);
                management_board_set_relay(MANAGEMENT_BOARD_RELAY_3, 0);
                management_board_set_relay(MANAGEMENT_BOARD_RELAY_4, 0);
            } else {
                update_after_test = 1;
            }
            break;

        case VIEW_CONTROLLER_COMMAND_TEST_RELE: management_board_set_relay(msg.rele, msg.level); break;

        default: break;
    }
}


void controller_manage(model_t *model) {
    static unsigned long timestamp = 0;

    static uint8_t cable_anomaly = 0xFF;
    static uint8_t cables        = -1;

    if (is_expired(timestamp, get_millis(), 500)) {
        model_set_connected(model, ethernet_is_connected(0));
        modbus_server_update(model);
        timestamp = get_millis();
    }


    uint8_t msg;
    if (xQueueReceive(queue, &msg, 0) == pdTRUE) {
        connections_restart(model, deviceq);
    }

    int             update = 0;
    device_update_t message;
    while (xQueueReceive(deviceq, &message, 0) == pdTRUE) {
        update = 1;
        model_update_channel(model, message);
    }

    uint8_t anomaly = management_board_cable_anomaly(model_cables(model));
    if (anomaly != cable_anomaly || cables != model_cables(model) || update_after_test) {
        model_set_cable_anomaly(model, anomaly);
        if (!test) {
            management_board_set_relay(MANAGEMENT_BOARD_RELAY_CABLE_ANOMALY, anomaly);
        }
        cable_anomaly = anomaly;
        update        = 1;
        cables        = model_cables(model);
    }

    anomaly = management_board_power_supply_anomaly();
    if (anomaly != model->power_bits) {
        model->power_bits = anomaly;
        view_event((view_event_t){.code = VIEW_EVENT_CODE_POWER_STATE, .power_bits = anomaly});
    }

    if (update || update_after_test) {
        int guasto_antenna = 0;
        int guasto_radio   = 0;

        for (size_t i = 0; i < MAX_CHANNELS; i++) {
            if (!model_is_channel_configured(model, i))
                continue;

            guasto_antenna |= model_channel_guasto_antenna(model, i);
            guasto_radio |= model_channel_guasto_radio(model, i);
        }

        if (!test) {
            management_board_set_relay(MANAGEMENT_BOARD_RELAY_ANTENNA, guasto_antenna);
            management_board_set_relay(MANAGEMENT_BOARD_RELAY_RADIO, guasto_radio);
        }

        view_event((view_event_t){.code = VIEW_EVENT_CODE_MODEL_UPDATE});
    }


    if (management_board_new_data()) {
        model_set_errore_scheda_gestione(model, management_board_error());
        if (!update) {
            view_event((view_event_t){.code = VIEW_EVENT_CODE_MODEL_UPDATE});
        }
    }

    update_after_test = 0;
}


static void restart_connections(void) {
    uint8_t msg = 1;
    xQueueOverwrite(queue, &msg);
}
