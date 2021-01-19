#ifndef MANAGEMENT_BOARD_H_INCLUDED
#define MANAGEMENT_BOARD_H_INCLUDED

#include <stdint.h>


void management_board_init(void);
void management_board_transaction(uint8_t cmd, uint8_t *receive);

#endif