#include <stdio.h>

#include "simulator_controller.h"
#include "view/view.h"
#include "simulator_storage.h"
#include "gui.h"
#include "network/device.h"


void controller_init(model_t *model) {
    gui_init();
    storage_load_model(model);
    // cman_update(model);
}


void controller_process_msg(view_controller_command_t msg, model_t *model) {
    switch (msg.code) {
        case VIEW_CONTROLLER_COMMAND_CODE_SAVE_CONFIG:
            storage_save_model(model);
            break;

        default:
            break;
    }
}