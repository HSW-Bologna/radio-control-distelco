#ifndef MANAGEMENT_BOARD_H_INCLUDED
#define MANAGEMENT_BOARD_H_INCLUDED

#include <stdint.h>


typedef enum {
    MANAGEMENT_BOARD_RELAY_4                    = 0,
    MANAGEMENT_BOARD_RELAY_3                    = 1,
    MANAGEMENT_BOARD_RELAY_2                    = 2,
    MANAGEMENT_BOARD_RELAY_1                    = 3,
    MANAGEMENT_BOARD_RELAY_CABLE_ANOMALY        = 3,
    MANAGEMENT_BOARD_RELAY_ANTENNA              = 2,
    MANAGEMENT_BOARD_RELAY_RADIO                = 1,
    MANAGEMENT_BOARD_RELAY_POWER_SUPPLY_ANOMALY = 0,
} management_board_relay_t;


void    management_board_init(void);
void    management_board_set_relay(uint8_t relay, int value);
uint8_t management_board_power_supply_anomaly(void);
uint8_t management_board_cable_anomaly(uint8_t cables);
void    management_board_read_response(uint8_t *response);
int     management_board_new_data(void);

#endif