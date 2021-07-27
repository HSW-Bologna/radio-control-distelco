#include <stdint.h>
#include <string.h>
#include <stdio.h>

void management_board_init(void) {}

void management_board_set_relay(uint8_t relay, int value) {
    printf("Impostazione rele %i livello %i\n", relay, value);
}

uint8_t management_board_cable_anomaly(uint8_t cables) {
    return 0;
}

void management_board_read_response(uint8_t *cables) {}

int management_board_new_data(void) {
    return 0;
}

int management_board_error(void) {
    return 0;
}

uint8_t management_board_power_supply_anomaly(void) {
    return 0;
}