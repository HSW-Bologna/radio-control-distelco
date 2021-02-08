/* Ethernet Basic Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "portmacro.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdint.h>

#define EVENT_GOT_IP 1


void ethernet_init(uint32_t ip) {}


void ethernet_set_ip(uint32_t ip) {}



int ethernet_is_connected(TickType_t delay) {
    return 1;
}


uint32_t ethernet_current_ip(void) {
    return 0;
}


void ethernet_set_callbacks(void (*discb)(void), void (*conncb)(void)) {
    conncb();
}