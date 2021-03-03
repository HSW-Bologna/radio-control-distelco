#include <string.h>
#include <assert.h>

#include "channel.h"
#include "model/model.h"


static device_t *get_device(channel_t *channel, device_type_t t, size_t m);


void channel_init(channel_t *channel) {
    channel->num_masters = 0;
    channel->num_minions = 0;
    channel_reset(channel);
}


int channel_is_configured(channel_t *channel) {
    return channel->num_masters > 0 || channel->num_minions > 0;
}


int channel_get_master_ip(channel_t *channel, uint32_t *ip, size_t m) {
    assert(m < MAX_MASTERS_PER_CHANNEL);
    if (m >= channel->num_masters)
        return -1;

    *ip = channel->masters[m].addr;
    return 0;
}


int channel_get_minion_ip(channel_t *channel, uint32_t *ip, size_t m) {
    assert(m < MAX_MASTERS_PER_CHANNEL);
    if (m >= channel->num_minions)
        return -1;

    *ip = channel->minions[m].addr;
    return 0;
}


int channel_set_master_ip(channel_t *channel, uint32_t ip, size_t m) {
    assert(m < MAX_MASTERS_PER_CHANNEL);
    if (m >= channel->num_masters)
        return -1;

    channel->masters[m].addr = ip;
    return 0;
}


int channel_set_minion_ip(channel_t *channel, uint32_t ip, size_t m) {
    assert(m < MAX_MASTERS_PER_CHANNEL);
    if (m >= channel->num_minions)
        return -1;

    channel->minions[m].addr = ip;
    return 0;
}


int channel_add_master(channel_t *channel, uint32_t ip) {
    if (channel->num_masters < MAX_MASTERS_PER_CHANNEL) {
        int index                    = channel->num_masters;
        channel->masters[index].addr = ip;
        channel->num_masters++;
        return index;
    } else {
        return -1;
    }
}


int channel_add_minion(channel_t *channel, uint32_t ip) {
    if (channel->num_minions < MAX_MINIONS_PER_CHANNEL) {
        int index                    = channel->num_minions;
        channel->minions[index].addr = ip;
        channel->num_minions++;
        return index;
    } else {
        return -1;
    }
}


int channel_remove_master(channel_t *channel, size_t i) {
    if (i >= channel->num_masters)
        return -1;

    for (size_t j = i; j < channel->num_masters - 1; j++)
        channel->masters[j] = channel->masters[j + 1];

    channel->num_masters--;
    return 0;
}


int channel_remove_minion(channel_t *channel, size_t i) {
    if (i >= channel->num_minions)
        return -1;

    for (size_t j = i; j < channel->num_minions - 1; j++)
        channel->minions[j] = channel->minions[j + 1];

    channel->num_minions--;
    return 0;
}

void channel_set_master_guasto_antenna(channel_t *channel, size_t m, int guasto) {
    assert(m < channel->num_masters);
    channel->masters[m].info.guasto_antenna = guasto;
}


void channel_set_minion_guasto_antenna(channel_t *channel, size_t m, int guasto) {
    assert(m < channel->num_minions);
    channel->minions[m].info.guasto_antenna = guasto;
}


void channel_set_master_guasto_radio(channel_t *channel, size_t m, int guasto) {
    assert(m < channel->num_masters);
    channel->masters[m].info.guasto_radio = guasto;
}


void channel_set_minion_guasto_radio(channel_t *channel, size_t m, int guasto) {
    assert(m < channel->num_minions);
    channel->minions[m].info.guasto_radio = guasto;
}


int channel_is_ok(channel_t *channel) {
    for (size_t i = 0; i < channel->num_masters; i++) {
        if (!channel->masters[i].info.connected)
            return 0;
        if (channel->masters[i].info.guasto_radio)
            return 0;
        if (channel->masters[i].info.guasto_antenna)
            return 0;
        if (channel->masters[i].info.failed_tx)
            return 0;
        if (channel->masters[i].info.tx_disabled)
            return 0;
    }

    for (size_t i = 0; i < channel->num_minions; i++) {
        if (!channel->minions[i].info.connected)
            return 0;
        if (channel->minions[i].info.guasto_radio)
            return 0;
        if (channel->minions[i].info.guasto_antenna)
            return 0;
        if (channel->minions[i].info.failed_tx)
            return 0;
        if (channel->minions[i].info.tx_disabled)
            return 0;
    }

    return 1;
}


int channel_guasto_radio(channel_t *channel) {
    for (size_t i = 0; i < channel->num_masters; i++) {
        if (channel->masters[i].info.guasto_radio)
            return 1;
    }

    for (size_t i = 0; i < channel->num_minions; i++) {
        if (channel->minions[i].info.guasto_radio)
            return 1;
    }

    return 0;
}


int channel_guasto_antenna(channel_t *channel) {
    for (size_t i = 0; i < channel->num_masters; i++) {
        if (channel->masters[i].info.guasto_antenna)
            return 1;
        if (channel->masters[i].info.failed_tx)
            return 1;
    }

    for (size_t i = 0; i < channel->num_minions; i++) {
        if (channel->minions[i].info.guasto_antenna)
            return 1;
        if (channel->minions[i].info.failed_tx)
            return 1;
    }

    return 0;
}



char *channel_get_name(channel_t *channel) {
    if (channel->num_masters > 0)
        return channel->masters[0].info.name;
    else if (channel->num_minions > 0)
        return channel->minions[0].info.name;
    else
        return NULL;
}


char *channel_get_master_name(channel_t *channel, size_t m) {
    assert(m < channel->num_masters);
    return channel->masters[m].info.name;
}


char *channel_get_minion_name(channel_t *channel, size_t m) {
    assert(m < channel->num_minions);
    return channel->minions[m].info.name;
}


void channel_set_master_name(channel_t *channel, size_t m, char *name) {
    assert(m < channel->num_masters);
    strncpy(channel->masters[m].info.name, name, CHANNEL_NAME_LEN);
    channel->masters[m].info.name[CHANNEL_NAME_LEN - 1] = '\0';
}


void channel_reset(channel_t *channel) {
    for (size_t i = 0; i < channel->num_masters; i++) {
        memset(&channel->masters[i].info, 0, sizeof(device_info_t));
    }
    for (size_t i = 0; i < channel->num_minions; i++) {
        memset(&channel->minions[i].info, 0, sizeof(device_info_t));
    }
}


void channel_set_minion_name(channel_t *channel, size_t m, char *name) {
    assert(m < channel->num_minions);
    strncpy(channel->minions[m].info.name, name, CHANNEL_NAME_LEN);
    channel->masters[m].info.name[CHANNEL_NAME_LEN - 1] = '\0';
}


void channel_device_guasti(channel_t *channel, device_type_t t, size_t m, int *radio, int *antenna) {
    switch (t) {
        case DEVICE_TYPE_MASTER:
            assert(m < MAX_MASTERS_PER_CHANNEL);
            *radio   = channel->masters[m].info.guasto_radio;
            *antenna = channel->masters[m].info.guasto_antenna;
            break;

        case DEVICE_TYPE_MINION:
            assert(m < MAX_MINIONS_PER_CHANNEL);
            *radio   = channel->minions[m].info.guasto_radio;
            *antenna = channel->minions[m].info.guasto_antenna;
            break;

        default:
            assert(0);
            break;
    }
}


void channel_set_device_connected(channel_t *channel, device_type_t t, size_t m, int connected) {
    get_device(channel, t, m)->info.connected = connected;
}


device_info_t channel_get_device_info(channel_t *channel, device_type_t t, size_t m) {
    return get_device(channel, t, m)->info;
}


void channel_set_device_info(channel_t *channel, device_type_t t, size_t m, device_info_t info) {
    get_device(channel, t, m)->info = info;
}


static device_t *get_device(channel_t *channel, device_type_t t, size_t m) {
    switch (t) {
        case DEVICE_TYPE_MASTER:
            assert(m < MAX_MASTERS_PER_CHANNEL);
            return &channel->masters[m];

        case DEVICE_TYPE_MINION:
            assert(m < MAX_MINIONS_PER_CHANNEL);
            return &channel->minions[m];

        default:
            assert(0);
            break;
    }

    return NULL;
}