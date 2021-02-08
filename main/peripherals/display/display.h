#ifndef __DISPLAY_MONO_H__
#define __DISPLAY_MONO_H__

#include <stdint.h>

#define MAX_TRANSFER_SIZE 4000

void display_set_data_command(uint8_t dc);
void display_set_reset(uint8_t res);
void display_init();
void display_send_data_command(uint8_t *data, unsigned int length, uint8_t mode);
void display_set_backlight(int percentage);

#endif