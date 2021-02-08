#ifndef CONNECTION_MANAGER_H_INCLUDED
#define CONNECTION_MANAGER_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>

#include "model/model.h"

#define CONFIG_VERSION_LEN 16
#define MAX_REMOTE_ADDRESS 4


/**
 * Structure of the status packet sent over TCP connection
 * All fields are in network order
 */
typedef struct __attribute__((__packed__)) {
    uint8_t  sync[16];                  // 16 byte sync = "DST-ROIP-AXDDRFS"
    uint8_t  version;                   // 1 for now
    uint8_t  system_mode;               // from config system-mode
    uint8_t  name[CONFIG_NAME_LEN];     // string to identify the unit
    uint8_t  system_status[16];         // string with state machine info
    uint8_t  radio_rx;
    uint8_t  radio_tx;
    uint16_t audio_in1_rms;       // percentuale rispetto a sinusoide 1Vpp x10
    uint16_t audio_in1_peak;      // percentuale rispetto a clipping x10
    uint16_t audio_in2_rms;       // percentuale rispetto a sinusoide 1Vpp x10
    uint16_t audio_in2_peak;      // percentuale rispetto a clipping x10
    uint16_t audio_out1_rms;      // percentuale rispetto a sinusoide 1Vpp x10
    uint16_t audio_out1_peak;     // percentuale rispetto a clipping x10
    uint16_t audio_out2_rms;      // percentuale rispetto a sinusoide 1Vpp x10
    uint16_t audio_out2_peak;     // percentuale rispetto a clipping x10
    uint8_t  audio_in1_gain;
    uint8_t  audio_in2_gain;
    uint8_t  audio_out1_gain;
    uint8_t  audio_out2_gain;
    uint32_t remotes[MAX_REMOTE_ADDRESS];
    uint8_t  remoteStatus[MAX_REMOTE_ADDRESS];
} status_packet_t;


typedef struct __attribute__((__packed__)) {
    uint8_t  sync[16];                  // 16 byte sync = "DST-ROIP-AXDDRFS"
    uint8_t  version;                   // 1 for now
    uint8_t  system_mode;               // SYSTEM_MODE_MASTER or SYSTEM_MODE_SLAVE
    uint8_t  name[CONFIG_NAME_LEN];     // string to identify the unit
    uint8_t  fw_version[CONFIG_VERSION_LEN];
    uint8_t  radio_rx;     // 1 if receiving
    uint8_t  radio_tx;     // 1 if transmitting
    uint32_t remotes[MAX_REMOTE_ADDRESS];
    uint8_t  remoteStatus[MAX_REMOTE_ADDRESS];
    uint8_t  failed_tx;     // 1 if problem detected during last transmission
    uint8_t  failed_antenna;
    uint8_t  missing_rx;     // 1 if no rx // in last 24h
} status2_packet_t;


int device_get_state(int socket, status2_packet_t *status);
int device_connect(uint32_t ip);

#endif