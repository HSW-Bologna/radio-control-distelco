#ifndef CHANNEL_H_INCLUDED
#define CHANNEL_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>

#define MAX_MASTERS_PER_CHANNEL 2
#define MAX_MINIONS_PER_CHANNEL 4

#define CHANNEL_NAME_LEN 16
#define CONFIG_NAME_LEN  16


typedef enum {
    RemoteNotPresent = 0,
    RemoteConnected,
    RemoteLostConnection,
} incoming_stream_status_t;



typedef enum {
    DEVICE_TYPE_MASTER = 1,
    DEVICE_TYPE_MINION = 0,
} device_type_t;


typedef struct {
    int      guasto_radio;
    int      guasto_antenna;
    int      failed_tx;
    int      connected;
    char     name[CHANNEL_NAME_LEN];
    uint32_t remotes[MAX_MINIONS_PER_CHANNEL];

    incoming_stream_status_t remoteStatus[MAX_MINIONS_PER_CHANNEL];

    char fw_version[CONFIG_NAME_LEN];
} device_info_t;


typedef struct {
    int    master;
    size_t channel;
    size_t index;

    device_info_t info;
} device_update_t;



typedef struct {
    uint32_t addr;

    device_info_t info;
} device_t;


typedef struct {
    device_t masters[MAX_MASTERS_PER_CHANNEL];
    device_t minions[MAX_MINIONS_PER_CHANNEL];

    size_t num_masters, num_minions;
} channel_t;

void channel_init(channel_t *channel);

int   channel_is_configured(channel_t *channel);
int   channel_add_minion(channel_t *channel, uint32_t ip);
int   channel_add_master(channel_t *channel, uint32_t ip);
int   channel_get_minion_ip(channel_t *channel, uint32_t *ip, size_t m);
int   channel_get_master_ip(channel_t *channel, uint32_t *ip, size_t m);
char *channel_get_name(channel_t *channel);
char *channel_get_master_name(channel_t *channel, size_t m);
char *channel_get_minion_name(channel_t *channel, size_t m);
int   channel_is_ok(channel_t *channel);
int   channel_guasto_radio(channel_t *channel);
int   channel_guasto_antenna(channel_t *channel);
void  channel_device_guasti(channel_t *channel, device_type_t t, size_t m, int *radio, int *antenna);

int  channel_set_master_ip(channel_t *channel, uint32_t ip, size_t m);
int  channel_set_minion_ip(channel_t *channel, uint32_t ip, size_t m);
int  channel_remove_master(channel_t *channel, size_t i);
int  channel_remove_minion(channel_t *channel, size_t i);
void channel_set_master_name(channel_t *channel, size_t m, char *name);
void channel_set_minion_name(channel_t *channel, size_t m, char *name);
void channel_reset(channel_t *channel);
void channel_set_device_connected(channel_t *channel, device_type_t t, size_t m, int connected);

device_info_t channel_get_device_info(channel_t *channel, device_type_t t, size_t m);
void          channel_set_device_info(channel_t *channel, device_type_t t, size_t m, device_info_t info);

#endif