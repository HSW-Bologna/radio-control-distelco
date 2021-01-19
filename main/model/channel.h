#ifndef CHANNEL_H_INCLUDED
#define CHANNEL_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>

#define MAX_MASTERS_PER_CHANNEL 2
#define MAX_MINIONS_PER_CHANNEL 4

#define CHANNEL_NAME_LEN 16


typedef struct {
    uint32_t addr;
    int      ok;
    char     name[CHANNEL_NAME_LEN];
} device_t;


typedef struct {
    device_t masters[MAX_MASTERS_PER_CHANNEL];
    device_t minions[MAX_MINIONS_PER_CHANNEL];

    size_t num_masters, num_minions;
} channel_t;

void channel_init(channel_t *channel);

int   channel_is_configured(channel_t *channel);
int   channel_is_ok(channel_t *channel);
int   channel_add_minion(channel_t *channel, uint32_t ip);
int   channel_add_master(channel_t *channel, uint32_t ip);
int   channel_get_minion_ip(channel_t *channel, uint32_t *ip, size_t m);
int   channel_get_master_ip(channel_t *channel, uint32_t *ip, size_t m);
char *channel_get_name(channel_t *channel);

int  channel_set_master_ip(channel_t *channel, uint32_t ip, size_t m);
int  channel_set_minion_ip(channel_t *channel, uint32_t ip, size_t m);
int  channel_remove_master(channel_t *channel, size_t i);
int  channel_remove_minion(channel_t *channel, size_t i);
void channel_set_master_ok(channel_t *channel, size_t m, int ok);
void channel_set_minion_ok(channel_t *channel, size_t m, int ok);
void channel_set_master_name(channel_t *channel, size_t m, char *name);
void channel_set_minion_name(channel_t *channel, size_t m, char *name);
void channel_reset(channel_t *channel);

#endif