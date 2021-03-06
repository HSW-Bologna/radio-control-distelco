/* Ethernet Basic Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "tcpip_adapter.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "ethernet.h"


#define EVENT_GOT_IP 1


static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);


static const char        *TAG = "ethernet";
static EventGroupHandle_t group;
static SemaphoreHandle_t  sem;
static void (*connected_cb)(void)    = NULL;
static void (*disconnected_cb)(void) = NULL;
static uint32_t         current_ip   = 0;
static esp_eth_handle_t eth_handle   = NULL;


void ethernet_init(uint32_t ip) {
    esp_err_t ret = ESP_OK;

    group = xEventGroupCreate();
    sem   = xSemaphoreCreateMutex();
    ESP_LOGI(TAG, "Data structures initialized: %u", (unsigned int)(void *)group);

    tcpip_adapter_init();
    ESP_LOGI(TAG, "adapter initialized");
    ret = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);
    ESP_LOGI(TAG, "dhcp client stop RESULT: %d", ret);
    // ethernet_set_ip(ip);

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(tcpip_adapter_set_default_eth_handlers());
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));
    ESP_LOGI(TAG, "handlers set");

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr         = CONFIG_EXAMPLE_ETH_PHY_ADDR;
    phy_config.reset_gpio_num   = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO;
#if CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
    mac_config.smi_mdc_gpio_num  = CONFIG_EXAMPLE_ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;
    esp_eth_mac_t *mac           = esp_eth_mac_new_esp32(&mac_config);
#if CONFIG_EXAMPLE_ETH_PHY_IP101
    esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_RTL8201
    esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_LAN8720
    esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_DP83848
    esp_eth_phy_t *phy = esp_eth_phy_new_dp83848(&phy_config);
#endif
#elif CONFIG_EXAMPLE_USE_DM9051
    gpio_install_isr_service(0);
    spi_device_handle_t spi_handle = NULL;
    spi_bus_config_t    buscfg     = {
               .miso_io_num   = CONFIG_EXAMPLE_DM9051_MISO_GPIO,
               .mosi_io_num   = CONFIG_EXAMPLE_DM9051_MOSI_GPIO,
               .sclk_io_num   = CONFIG_EXAMPLE_DM9051_SCLK_GPIO,
               .quadwp_io_num = -1,
               .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(CONFIG_EXAMPLE_DM9051_SPI_HOST, &buscfg, 1));
    spi_device_interface_config_t devcfg = {.command_bits   = 1,
                                            .address_bits   = 7,
                                            .mode           = 0,
                                            .clock_speed_hz = CONFIG_EXAMPLE_DM9051_SPI_CLOCK_MHZ * 1000 * 1000,
                                            .spics_io_num   = CONFIG_EXAMPLE_DM9051_CS_GPIO,
                                            .queue_size     = 20};
    ESP_ERROR_CHECK(spi_bus_add_device(CONFIG_EXAMPLE_DM9051_SPI_HOST, &devcfg, &spi_handle));
    /* dm9051 ethernet driver is based on spi driver */
    eth_dm9051_config_t dm9051_config = ETH_DM9051_DEFAULT_CONFIG(spi_handle);
    dm9051_config.int_gpio_num        = CONFIG_EXAMPLE_DM9051_INT_GPIO;
    esp_eth_mac_t *mac                = esp_eth_mac_new_dm9051(&dm9051_config, &mac_config);
    esp_eth_phy_t *phy                = esp_eth_phy_new_dm9051(&phy_config);
#endif
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_LOGI(TAG, "about to install");
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    ESP_LOGI(TAG, "Eth driver installed");
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    ESP_LOGI(TAG, "Eth driver started");
}


void ethernet_set_ip(uint32_t ip) {
    if (ethernet_is_connected(0) && ethernet_current_ip() == ip) {
        // If the IP is the same do nothing
        ESP_LOGI(TAG, "No change in ethernet config");
        return;
    }

    ESP_ERROR_CHECK(esp_eth_stop(eth_handle));

    esp_err_t ret = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);
    ESP_LOGI(TAG, "dhcp client stop RESULT: %d", ret);

    ip4_addr_t printip;
    printip.addr = ip;
    ESP_LOGI(TAG, "Changing ip addr to:" IPSTR, IP2STR(&printip));

    tcpip_adapter_ip_info_t ipInfo;

    // myIp -> structure that save your static ip settings
    ipInfo.ip.addr = ip;
    inet_pton(AF_INET, "0.0.0.0", &ipInfo.gw);
    inet_pton(AF_INET, "255.255.255.0", &ipInfo.netmask);

    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &ipInfo));
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}


/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
        case ETHERNET_EVENT_CONNECTED:
            esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
            ESP_LOGI(TAG, "Ethernet Link Up");
            ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2],
                     mac_addr[3], mac_addr[4], mac_addr[5]);
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Ethernet Link Down");
            xEventGroupClearBits(group, EVENT_GOT_IP);
            xSemaphoreTake(sem, portMAX_DELAY);
            if (disconnected_cb)
                disconnected_cb();
            xSemaphoreGive(sem);
            break;
        case ETHERNET_EVENT_START: ESP_LOGI(TAG, "Ethernet Started"); break;
        case ETHERNET_EVENT_STOP: ESP_LOGI(TAG, "Ethernet Stopped"); break;
        default: ESP_LOGI(TAG, "Unknown ehternet event %i", event_id); break;
    }
}


int ethernet_is_connected(TickType_t delay) {
    BaseType_t res = xEventGroupWaitBits(group, EVENT_GOT_IP, 0, 1, delay);
    return res == pdTRUE;
}


uint32_t ethernet_current_ip(void) {
    xSemaphoreTake(sem, portMAX_DELAY);
    uint32_t res = current_ip;
    xSemaphoreGive(sem);
    return res;
}


void ethernet_set_callbacks(void (*discb)(void), void (*conncb)(void)) {
    xSemaphoreTake(sem, portMAX_DELAY);
    disconnected_cb = discb;
    connected_cb    = conncb;
    xSemaphoreGive(sem);
}


/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ip_event_got_ip_t             *event   = (ip_event_got_ip_t *)event_data;
    const tcpip_adapter_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");

    xEventGroupSetBits(group, EVENT_GOT_IP);
    xSemaphoreTake(sem, portMAX_DELAY);
    current_ip = ip_info->ip.addr;
    if (connected_cb)
        connected_cb();
    xSemaphoreGive(sem);
}