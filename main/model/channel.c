#include <string.h>
#include <assert.h>

#include "channel.h"
#include "model/model.h"


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


void channel_set_master_ok(channel_t *channel, size_t m, int ok) {
    assert(m < channel->num_masters);
    channel->masters[m].ok = ok;
}


void channel_set_minion_ok(channel_t *channel, size_t m, int ok) {
    assert(m < channel->num_minions);
    channel->minions[m].ok = ok;
}


int channel_is_ok(channel_t *channel) {
    for (size_t i = 0; i < channel->num_masters; i++) {
        if (!channel->masters[i].ok)
            return 0;
    }

    for (size_t i = 0; i < channel->num_minions; i++) {
        if (!channel->minions[i].ok)
            return 0;
    }

    return 1;
}


char *channel_get_name(channel_t *channel) {
    if (channel->num_masters > 0)
        return channel->masters[0].name;
    else if (channel->num_minions > 0)
        return channel->minions[0].name;
    else
        return NULL;
}


void channel_set_master_name(channel_t *channel, size_t m, char *name) {
    assert(m < channel->num_masters);
    strncpy(channel->masters[m].name, name, CHANNEL_NAME_LEN);
    channel->masters[m].name[CHANNEL_NAME_LEN - 1] = '\0';
}


void channel_reset(channel_t *channel) {
    for (size_t i = 0; i < channel->num_masters; i++) {
        channel->masters[i].ok      = 0;
        channel->masters[i].name[0] = '\0';
    }
    for (size_t i = 0; i < channel->num_minions; i++) {
        channel->minions[i].ok      = 0;
        channel->minions[i].name[0] = '\0';
    }
}


void channel_set_minion_name(channel_t *channel, size_t m, char *name) {
    assert(m < channel->num_minions);
    strncpy(channel->minions[m].name, name, CHANNEL_NAME_LEN);
    channel->masters[m].name[CHANNEL_NAME_LEN - 1] = '\0';
}