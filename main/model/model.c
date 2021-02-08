#include <assert.h>
#include <string.h>

#include "model.h"
#include "model/channel.h"


static uint32_t next_ip_addr(model_t *model, uint32_t ip);


void model_init(model_t *model) {
    model->ip_addr                = IP_ADDR(192, 168, 1, 10);
    model->to_save                = 0;
    model->cavi                   = 0;
    model->anomalie_cavi          = 0;
    model->network_config_changed = 0;
    model->connected              = 0;

    for (size_t i = 0; i < MAX_CHANNELS; i++)
        channel_init(&model->channels[i]);
}


void model_get_my_ip_parts(model_t *model, int *ip1, int *ip2, int *ip3, int *ip4) {
    *ip1 = IP_GET_PART_1(model->ip_addr);
    *ip2 = IP_GET_PART_2(model->ip_addr);
    *ip3 = IP_GET_PART_3(model->ip_addr);
    *ip4 = IP_GET_PART_4(model->ip_addr);
}


uint32_t model_get_my_ip(model_t *model) {
    return model->ip_addr;
}


void model_set_my_ip_parts(model_t *model, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4) {
    model->ip_addr                = IP_ADDR(ip1, ip2, ip3, ip4);
    model->to_save                = 1;
    model->network_config_changed = 1;
}


int model_is_channel_configured(model_t *model, size_t i) {
    assert(i < MAX_CHANNELS);
    return channel_is_configured(&model->channels[i]);
}


int model_set_channel_master_ip(model_t *model, uint32_t ip, size_t c, size_t m) {
    assert(c < MAX_CHANNELS);
    model->to_save                = 1;
    model->network_config_changed = 1;
    return channel_set_master_ip(&model->channels[c], ip, m);
}


int model_set_channel_minion_ip(model_t *model, uint32_t ip, size_t c, size_t m) {
    assert(c < MAX_CHANNELS);
    model->to_save                = 1;
    model->network_config_changed = 1;
    return channel_set_minion_ip(&model->channels[c], ip, m);
}


int model_get_channel_master_ip(model_t *model, uint32_t *ip, size_t c, size_t m) {
    assert(c < MAX_CHANNELS);
    return channel_get_master_ip(&model->channels[c], ip, m);
}


int model_get_channel_minion_ip(model_t *model, uint32_t *ip, size_t c, size_t m) {
    assert(c < MAX_CHANNELS);
    return channel_get_minion_ip(&model->channels[c], ip, m);
}


int model_get_channel_master_ip_parts(model_t *model, int *ip1, int *ip2, int *ip3, int *ip4, size_t c, size_t m) {
    assert(c < MAX_CHANNELS);
    uint32_t ip;

    if (channel_get_master_ip(&model->channels[c], &ip, m))
        return -1;

    *ip1 = IP_GET_PART_1(ip);
    *ip2 = IP_GET_PART_2(ip);
    *ip3 = IP_GET_PART_3(ip);
    *ip4 = IP_GET_PART_4(ip);

    return 0;
}



size_t model_get_masters_in_channel(model_t *model, size_t i) {
    assert(i < MAX_CHANNELS);
    return model->channels[i].num_masters;
}


size_t model_get_minions_in_channel(model_t *model, size_t i) {
    assert(i < MAX_CHANNELS);
    return model->channels[i].num_minions;
}


int model_add_master_to_channel(model_t *model, size_t i) {
    assert(i < MAX_CHANNELS);
    model->to_save                = 1;
    model->network_config_changed = 1;
    return channel_add_master(&model->channels[i], next_ip_addr(model, model->ip_addr));
}


int model_add_minion_to_channel(model_t *model, size_t i) {
    assert(i < MAX_CHANNELS);
    model->to_save                = 1;
    model->network_config_changed = 1;
    return channel_add_minion(&model->channels[i], next_ip_addr(model, model->ip_addr));
}


int model_remove_master_from_channel(model_t *model, size_t i, size_t m) {
    assert(i < MAX_CHANNELS);
    model->to_save                = 1;
    model->network_config_changed = 1;
    return channel_remove_master(&model->channels[i], m);
}


int model_remove_minion_from_channel(model_t *model, size_t i, size_t m) {
    assert(i < MAX_CHANNELS);
    model->to_save                = 1;
    model->network_config_changed = 1;
    return channel_remove_minion(&model->channels[i], m);
}


int model_ip_address_already_in_use(model_t *model, uint32_t ip) {
    if (model->ip_addr == ip)
        return 1;

    for (size_t i = 0; i < MAX_CHANNELS; i++) {
        if (!model_is_channel_configured(model, i))
            continue;

        for (size_t j = 0; j < model_get_masters_in_channel(model, j); j++) {
            uint32_t tmpip;
            if (model_get_channel_master_ip(model, &tmpip, i, j))
                continue;

            if (tmpip == ip)
                return 1;
        }

        for (size_t j = 0; j < model_get_minions_in_channel(model, j); j++) {
            uint32_t tmpip;
            if (model_get_channel_minion_ip(model, &tmpip, i, j))
                continue;

            if (tmpip == ip)
                return 1;
        }
    }

    return 0;
}


void model_channel_device_guasti(model_t *model, size_t c, device_type_t t, size_t m, int *radio, int *antenna) {
    assert(c < MAX_CHANNELS);
    channel_device_guasti(&model->channels[c], t, m, radio, antenna);
}


int model_is_channel_ok(model_t *model, size_t c) {
    assert(c < MAX_CHANNELS);
    return model->connected && channel_is_ok(&model->channels[c]);
}


int model_channel_guasto_radio(model_t *model, size_t c) {
    assert(c < MAX_CHANNELS);
    return channel_guasto_radio(&model->channels[c]);
}


int model_channel_guasto_antenna(model_t *model, size_t c) {
    assert(c < MAX_CHANNELS);
    return channel_guasto_antenna(&model->channels[c]);
}


void model_set_saved(model_t *model) {
    model->to_save = 0;
}


int model_is_to_save(model_t *model) {
    return model->to_save;
}


void model_network_config_clear(model_t *model) {
    model->network_config_changed = 0;
}


int model_is_network_config_changed(model_t *model) {
    return model->network_config_changed;
}


char *model_get_channel_name(model_t *model, size_t c) {
    assert(c < MAX_CHANNELS);
    return channel_get_name(&model->channels[c]);
}


void model_set_channel_master_name(model_t *model, size_t c, size_t m, char *name) {
    assert(c < MAX_CHANNELS);
    channel_set_master_name(&model->channels[c], m, name);
}


void model_set_channel_minion_name(model_t *model, size_t c, size_t m, char *name) {
    assert(c < MAX_CHANNELS);
    channel_set_minion_name(&model->channels[c], m, name);
}


void model_reset_channel_state(model_t *model, size_t c) {
    assert(c < MAX_CHANNELS);
    channel_reset(&model->channels[c]);
}


void model_set_cable(model_t *model, size_t cable, int enabled) {
    assert(cable < 4);
    model->to_save = 1;
    if (enabled)
        model->cavi |= 1 << cable;
    else
        model->cavi &= ~(1 << cable);
}


void model_set_cable_anomaly(model_t *model, uint8_t anomaly) {
    model->anomalie_cavi = anomaly;
}


int model_is_cable_enabled(model_t *model, size_t cable) {
    assert(cable < 4);
    return (model->cavi & (1 << cable)) > 0;
}


int model_is_cable_ok(model_t *model, size_t cable) {
    assert(cable < 4);
    return model_is_cable_enabled(model, cable) && ((model->anomalie_cavi & (1 << cable)) == 0);
}


uint8_t model_cables(model_t *model) {
    return model->cavi;
}


void model_update_channel(model_t *model, device_update_t update) {
    assert(update.channel < MAX_CHANNELS);
    if (update.info.connected) {
        channel_set_device_info(&model->channels[update.channel], update.master, update.index, update.info);
    } else {
        channel_set_device_connected(&model->channels[update.channel], update.master, update.index, 0);
    }
}


device_info_t model_get_channel_master_info(model_t *model, size_t c, size_t m) {
    assert(c < MAX_CHANNELS);
    return channel_get_device_info(&model->channels[c], DEVICE_TYPE_MASTER, m);
}


device_info_t model_get_channel_minion_info(model_t *model, size_t c, size_t m) {
    assert(c < MAX_CHANNELS);
    return channel_get_device_info(&model->channels[c], DEVICE_TYPE_MINION, m);
}


char *model_get_channel_minion_name(model_t *model, size_t channel, size_t m) {
    assert(channel < MAX_CHANNELS);
    return channel_get_minion_name(&model->channels[channel], m);
}


char *model_get_channel_master_name(model_t *model, size_t channel, size_t m) {
    assert(channel < MAX_CHANNELS);
    return channel_get_master_name(&model->channels[channel], m);
}


void model_set_spi_received(model_t *model, uint8_t *buffer) {
    memcpy(model->spi_received, buffer, 14);
}


void model_set_connected(model_t *model, int connected) {
    model->connected = connected;
}


int model_get_connected(model_t *model) {
    return model->connected;
}


static uint32_t next_ip_addr(model_t *model, uint32_t ip) {
    uint32_t new = ip;

    for (int i = 0; i < 256; i++) {
        uint32_t final = (IP_GET_PART_4(new) + 1) % 256;
        new            = IP_SET_PART_4(ip, final);

        if (!model_ip_address_already_in_use(model, new))
            return new;
    }

    return ip;
}