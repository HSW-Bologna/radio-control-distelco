#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include "view/view.h"
#include "model/model.h"

void controller_process_msg(view_controller_command_t msg, model_t *model);
void controller_init(model_t *model);

#endif