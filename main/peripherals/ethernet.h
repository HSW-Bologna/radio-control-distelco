#ifndef ETHERNET_H_INCLUDED
#define ETHERNET_H_INCLUDED

#include <stdint.h>
#include "freertos/FreeRTOS.h"

void     ethernet_init(uint32_t ip);
void     ethernet_set_ip(uint32_t ip);
int      ethernet_is_connected(TickType_t delay);
void     ethernet_set_callbacks(void (*discb)(void), void (*conncb)(void));
uint32_t ethernet_current_ip(void);

#endif