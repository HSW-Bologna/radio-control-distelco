#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "storage/storage.h"
#include "storage/configuration.h"
#include "model/model.h"
#include "view/view.h"
#include "network/connections.h"
#include "peripherals/ethernet.h"
#include "peripherals/management_board.h"


static void restart_connections(void);


static const char *  TAG     = "Controller";
static QueueHandle_t queue   = NULL;
static QueueHandle_t deviceq = NULL;


void controller_init(model_t *model) {
    queue   = xQueueCreate(1, sizeof(uint8_t));
    deviceq = xQueueCreate(MAX_CHANNELS * (MAX_MASTERS_PER_CHANNEL + MAX_MINIONS_PER_CHANNEL), sizeof(device_update_t));

    connections_init();
    storage_init();
    configuration_load(model);
    ethernet_set_ip(model_get_my_ip(model));
    ethernet_set_callbacks(connections_clear, restart_connections);
}


void controller_process_msg(view_controller_command_t msg, model_t *model) {
    switch (msg.code) {
        case VIEW_CONTROLLER_COMMAND_MANAGEMENT_SEND: {
            view_event_t event;
            event.code = VIEW_EVENT_CODE_TEST_MANAGEMENT_RESPONSE;
            management_board_transaction(msg.data_reg, event.management_registers);
            view_event(event);
            break;
        }

        case VIEW_CONTROLLER_COMMAND_CODE_SAVE_CONFIG:
            if (model_is_to_save(model)) {
                configuration_save(model);
                model_set_saved(model);
            }

            if (model_is_network_config_changed(model)) {
                ethernet_set_ip(model_get_my_ip(model));
                if (ethernet_is_connected(0))
                    connections_restart(model, deviceq);
                model_network_config_clear(model);
            }
            break;

        default:
            break;
    }
}


void controller_manage(model_t *model) {
    uint8_t msg;
    if (xQueueReceive(queue, &msg, 0) == pdTRUE) {
        connections_restart(model, deviceq);
    }

    int             update_ui = 0;
    device_update_t update;
    while (xQueueReceive(deviceq, &update, 0) == pdTRUE) {
        update_ui = 1;
        if (update.master) {
            model_channel_master_set_ok(model, update.channel, update.index, update.ok);
            if (update.ok)
                model_set_channel_master_name(model, update.channel, update.index, update.name);
        } else {
            model_channel_minion_set_ok(model, update.channel, update.index, update.ok);
            if (update.ok)
                model_set_channel_minion_name(model, update.channel, update.index, update.name);
        }
    }

    if (update_ui) {
        view_event((view_event_t){.code = VIEW_EVENT_CODE_MODEL_UPDATE});
    }
}


static void restart_connections(void) {
    uint8_t msg = 1;
    xQueueOverwrite(queue, &msg);
}