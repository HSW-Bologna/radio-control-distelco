#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include "channel.h"

#define MAX_CHANNELS 5
#define MAX_CABLES   4

#define IP_PART_4(ip4) ((ip4 & 0xFF) << 24)
#define IP_PART_3(ip3) ((ip3 & 0xFF) << 16)
#define IP_PART_2(ip2) ((ip2 & 0xFF) << 8)
#define IP_PART_1(ip1) (ip1 & 0xFF)

#define IP_GET_PART_4(ip) ((ip >> 24) & 0xFF)
#define IP_GET_PART_3(ip) ((ip >> 16) & 0xFF)
#define IP_GET_PART_2(ip) ((ip >> 8) & 0xFF)
#define IP_GET_PART_1(ip) (ip & 0xFF)

#define IP_SET_PART_1(ip, y) ((ip & ~(IP_PART_1(0xFF))) | IP_PART_1(y))
#define IP_SET_PART_2(ip, y) ((ip & ~(IP_PART_2(0xFF))) | IP_PART_2(y))
#define IP_SET_PART_3(ip, y) ((ip & ~(IP_PART_3(0xFF))) | IP_PART_3(y))
#define IP_SET_PART_4(ip, y) ((ip & ~(IP_PART_4(0xFF))) | IP_PART_4(y))

#define IP_ADDR(ip1, ip2, ip3, ip4) (IP_PART_1(ip1) | IP_PART_2(ip2) | IP_PART_3(ip3) | IP_PART_4(ip4))


typedef struct {
    channel_t channels[MAX_CHANNELS];
    uint32_t  ip_addr;
    uint8_t   cavi;
    int       connected;
    uint8_t   password[4];

    uint8_t power_bits;
    uint8_t anomalie_cavi;
    uint8_t errore_scheda_gestione;
    int     to_save;
    int     network_config_changed;
} model_t;

void model_init(model_t *model);

uint32_t model_get_my_ip(model_t *model);
void     model_get_my_ip_parts(model_t *model, int *ip1, int *ip2, int *ip3, int *ip4);
int      model_is_channel_configured(model_t *model, size_t i);
size_t   model_get_minions_in_channel(model_t *model, size_t i);
size_t   model_get_masters_in_channel(model_t *model, size_t i);
int      model_get_channel_minion_ip(model_t *model, uint32_t *ip, size_t c, size_t m);
int      model_get_channel_master_ip(model_t *model, uint32_t *ip, size_t c, size_t m);
int      model_is_channel_ok(model_t *model, size_t c);
int      model_is_to_save(model_t *model);
int      model_is_network_config_changed(model_t *model);
char *   model_get_channel_name(model_t *model, size_t c);
char *   model_get_channel_master_name(model_t *model, size_t channel, size_t m);
char *   model_get_channel_minion_name(model_t *model, size_t channel, size_t m);
int      model_channel_guasto_radio(model_t *model, size_t c);
int      model_channel_guasto_antenna(model_t *model, size_t c);
int      model_is_cable_enabled(model_t *model, size_t cable);
int      model_is_cable_ok(model_t *model, size_t cable);
void     model_channel_device_guasti(model_t *model, size_t c, device_type_t t, size_t m, int *radio, int *antenna);
uint8_t  model_cables(model_t *model);

device_info_t model_get_channel_master_info(model_t *model, size_t c, size_t m);
device_info_t model_get_channel_minion_info(model_t *model, size_t c, size_t m);
int           model_get_connected(model_t *model);
int           model_get_errore_scheda_gestione(model_t *model);
int           model_password_enabled(model_t *model);
void          model_set_password(model_t *model, int pw1, int pw2, int pw3, int pw4);

int  model_add_master_to_channel(model_t *model, size_t i);
int  model_add_minion_to_channel(model_t *model, size_t i);
int  model_set_channel_master_ip(model_t *model, uint32_t ip, size_t c, size_t m);
int  model_set_channel_minion_ip(model_t *model, uint32_t ip, size_t c, size_t m);
int  model_get_channel_master_ip_parts(model_t *model, int *ip1, int *ip2, int *ip3, int *ip4, size_t c, size_t m);
int  model_remove_master_from_channel(model_t *model, size_t i, size_t m);
int  model_remove_minion_from_channel(model_t *model, size_t i, size_t m);
void model_set_saved(model_t *model);
void model_set_my_ip_parts(model_t *model, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4);
void model_network_config_clear(model_t *model);
void model_set_channel_minion_name(model_t *model, size_t c, size_t m, char *name);
void model_set_channel_master_name(model_t *model, size_t c, size_t m, char *name);
void model_reset_channel_state(model_t *model, size_t c);
void model_set_cable(model_t *model, size_t cable, int enabled);
void model_set_cable_anomaly(model_t *model, uint8_t anomaly);
void model_update_channel(model_t *model, device_update_t update);
void model_set_connected(model_t *model, int connected);
void model_set_errore_scheda_gestione(model_t *model, int errore);

#endif