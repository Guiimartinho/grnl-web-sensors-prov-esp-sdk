#include <stdio.h>

/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_AP_PASS			"12345678"
#define WIFI_AP_MAX_STA_CONN	4

static const char *TAG = "wifi ap";

static void wifi_ap_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

static void get_device_service_name(char *ssid_with_mac, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "GRNL_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(ssid_with_mac, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}
static struct netif *esp_netif[TCPIP_ADAPTER_IF_MAX];
static tcpip_adapter_ip_info_t esp_ip[TCPIP_ADAPTER_IF_MAX];
static tcpip_adapter_ip_info_t esp_ip_old[TCPIP_ADAPTER_IF_MAX];

void wifi_ap_init()
{
	ESP_LOGI(TAG, "Starting WiFi AP");

//    tcpip_adapter_init();

//    ESP_ERROR_CHECK(esp_netif_init());

    tcpip_adapter_init();

    //memset(esp_ip, 0, sizeof(tcpip_adapter_ip_info_t)*TCPIP_ADAPTER_IF_MAX);
    //memset(esp_ip_old, 0, sizeof(tcpip_adapter_ip_info_t)*TCPIP_ADAPTER_IF_MAX);
    //
    //IP4_ADDR(&esp_ip[TCPIP_ADAPTER_IF_AP].ip, 192, 168 , 4, 1);
    //IP4_ADDR(&esp_ip[TCPIP_ADAPTER_IF_AP].gw, 192, 168 , 4, 1);
    //IP4_ADDR(&esp_ip[TCPIP_ADAPTER_IF_AP].netmask, 255, 255 , 255, 0);

    //esp_netif_create_default_wifi_ap();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_ap_event_handler, NULL));

    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = "",
            .ssid_len = 0,
            .ssid_hidden = false,
            //.channel = 1,
            .password = WIFI_AP_PASS,
            .max_connection = WIFI_AP_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .beacon_interval = 400
        }
    };

    char wifi_ap_name[12];
    get_device_service_name(wifi_ap_name, sizeof(wifi_ap_name));
    memcpy(&wifi_ap_config.ap.ssid, &wifi_ap_name, strlen(wifi_ap_name));
    wifi_ap_config.ap.ssid_len = (uint8_t)strlen(wifi_ap_name);


	if (strlen(WIFI_AP_PASS) == 0) {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi init ap finished. SSID:%s password:%s",
             wifi_ap_name, WIFI_AP_PASS);
}
