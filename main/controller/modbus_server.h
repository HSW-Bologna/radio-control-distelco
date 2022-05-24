#ifndef MODBUS_SERVER_H_INCLUDED
#define MODBUS_SERVER_H_INCLUDED

#include "model/model.h"


void modbus_server_init(void);
void modbus_server_update(model_t *pmodel);


#endif