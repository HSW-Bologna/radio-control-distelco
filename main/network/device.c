#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "esp_log.h"

#include "model/model.h"
#include "device.h"


#define MAGIC "DST-ROIP-AXDDRFS"
#define PORT  8082

/**
 * Elenco valori del campo <command>
 */
enum {
    command_status = 0,
    command_save   = 1,
    command_set_ip,
    command_set_sysmode,
    command_set_name,
    command_set_in1_gain,
    command_set_in2_gain,
    command_set_out1_gain,
    command_set_out2_gain,
    command_set_remotes,
    command_set_single_remote,
    command_status2,
} command_packet_commands;

/**
 * struttura dati del messaggio
 *
 */
typedef struct __attribute__((__packed__)) {
    uint8_t sync[16];     // 16 byte sync = "DST-ROIP-AXDDRFS"
    uint8_t version;
    uint8_t command;
} command_packet_t;

static void flush_rx(int socket);

static const char *TAG = "Device";


int device_get_state(int socket, status2_packet_t *status) {
    flush_rx(socket);

    command_packet_t payload    = {.sync = MAGIC, .version = 1, .command = command_status2};
    uint8_t          buffer[40] = {0};
    memcpy(buffer, &payload, sizeof(command_packet_t));
    int err = send(socket, buffer, 40, 0);
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: %s (%i)", strerror(errno), errno);
        return -1;
    }

    uint8_t *receive   = (uint8_t *)status;
    size_t   bytesread = 0;
    size_t   toreceive = sizeof(status2_packet_t);

    while (bytesread < toreceive) {
        int res = recv(socket, &receive[bytesread], toreceive - bytesread, 0);

        if (res > 0) {
            bytesread += res;
        } else {
            ESP_LOGE(TAG, "Timeout after %i bytes!", bytesread);
            return -1;
        }
    }

    return 0;
}


int device_connect(uint32_t ip) {
#define TEARDOWN()                                                                                                     \
    shutdown(sock, 0);                                                                                                 \
    close(sock);

    int addr_family = 0;
    int ip_protocol = 0;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = ip;
    dest_addr.sin_family      = AF_INET;
    dest_addr.sin_port        = htons(PORT);
    addr_family               = AF_INET;
    ip_protocol               = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: %s", strerror(errno));
        return -1;
    }

    ESP_LOGI(TAG, "Socket created, connecting to %d.%d.%d.%d:%d", IP_GET_PART_1(ip), IP_GET_PART_2(ip),
             IP_GET_PART_3(ip), IP_GET_PART_4(ip), PORT);

    // Set non-blocking
    long arg;
    if ((arg = fcntl(sock, F_GETFL, NULL)) < 0) {
        ESP_LOGE(TAG, "Error fcntl(..., F_GETFL) (%s)", strerror(errno));
        TEARDOWN();
        return -1;
    }
    arg |= O_NONBLOCK;
    if (fcntl(sock, F_SETFL, arg) < 0) {
        ESP_LOGE(TAG, "Error fcntl(..., F_SETFL) (%s)", strerror(errno));
        TEARDOWN();
        return -1;
    }
    // Trying to connect with timeout
    int res = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (res < 0) {
        if (errno == EINPROGRESS) {
            ESP_LOGI(TAG, "EINPROGRESS in connect() - selecting");
            do {
                struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
                fd_set         myset;
                FD_ZERO(&myset);
                FD_SET(sock, &myset);

                res = select(sock + 1, NULL, &myset, NULL, &tv);
                if (res < 0 && errno != EINTR) {
                    ESP_LOGE(TAG, "Error connecting %d - %s", errno, strerror(errno));
                    TEARDOWN();
                    return -1;
                } else if (res > 0) {
                    // Socket selected for write
                    socklen_t lon = sizeof(int);
                    int       valopt;
                    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)(&valopt), &lon) < 0) {
                        ESP_LOGE(TAG, "Error in getsockopt() %d - %s", errno, strerror(errno));
                        TEARDOWN();
                        return -1;
                    }
                    // Check the value returned...
                    if (valopt) {
                        ESP_LOGE(TAG, "Error in delayed connection() %d - %s", valopt, strerror(valopt));
                        TEARDOWN();
                        return -1;
                    }
                    break;
                } else {
                    ESP_LOGE(TAG, "Timeout in select() - Cancelling!");
                    TEARDOWN();
                    return -1;
                }
            } while (1);
        } else {
            ESP_LOGE(TAG, "Error connecting %d - %s", errno, strerror(errno));
            TEARDOWN();
            return -1;
        }
    }

    // Set to blocking mode again...
    if ((arg = fcntl(sock, F_GETFL, NULL)) < 0) {
        ESP_LOGE(TAG, "Error fcntl(..., F_GETFL) (%s)", strerror(errno));
        TEARDOWN();
        return -1;
    }
    arg &= (~O_NONBLOCK);
    if (fcntl(sock, F_SETFL, arg) < 0) {
        ESP_LOGE(TAG, "Error fcntl(..., F_SETFL) (%s)", strerror(errno));
        TEARDOWN();
        return -1;
    }

    struct timeval tout = {.tv_sec = 0, .tv_usec = 300 * 1000UL};
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout))) {
        ESP_LOGE(TAG, "Error setsockopt (%s)", strerror(errno));
        TEARDOWN();
        return -1;
    }

    ESP_LOGI(TAG, "Successfully connected");
    return sock;
}


static void flush_rx(int socket) {
    uint8_t data;
    while (recv(socket, &data, 1, MSG_DONTWAIT) == 1) {}
}