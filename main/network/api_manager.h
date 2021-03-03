#ifndef API_MANAGER_H_INCLUDED
#define API_MANAGER_H_INCLUDED

#include <stdint.h>

void api_manager_enable_sync(uint32_t ip, uint8_t enable);
void api_manager_enable_async(uint32_t ip);
void api_manager_init(void);

#endif