
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/event_groups.h"
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "esp_netif.h"
#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/sockets.h>
#include <ping/ping.h>


#include "lwip/apps/netbiosns.h"

#include "wifi.h"
#include "sim800.h"


#include "esp_task_wdt.h"

#include "mdns.h"
#include "webserver.h"
#include "define.h"

#include "storage.h"
#include "ota.h"
#include "mqtt_service.h"
#include "ntp_client.h"

//#include "temperature.h"

#define CONFIG_ESP_WIFI_SOFTAP_SUPPORT 1

/* FreeRTOS event group to signal when we are connected*/
EventGroupHandle_t status_event_group;

static const char *TAG = "main";

char DEVICE_ID[13];

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int boot_count = 0;

cJSON *root;

static void initialise_mdns(void)
{
    //initialize mDNS
	ESP_ERROR_CHECK(mdns_init());

	//set mDNS hostname (required if you want to advertise services)
    mdns_hostname_set(CONFIG_EXAMPLE_MDNS_HOST_NAME);

	ESP_LOGI(TAG, "mdns hostname set to: [%s]", CONFIG_EXAMPLE_MDNS_HOST_NAME);
    //mdns_instance_name_set(MDNS_INSTANCE);

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "grnl"},
        {"path", "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add("grnl", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}

void user_printf(char c)
{
   // do nothing here
}

void hw_wdt_disable(){
  *((volatile uint32_t*) 0x60000900) &= ~(1); // Hardware WDT OFF
}

static void logip_task(void *pvParameter)
{
	tcpip_adapter_ip_info_t ip;
	memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
	while (1)
    {
        vTaskDelay(10000 / portTICK_PERIOD_MS);

		memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
        if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip) == 0) {
            ESP_LOGI(TAG, "TCPIP_ADAPTER_IF_STA ~~~~~~~~~~~");
            ESP_LOGI(TAG, "IP:"IPSTR, IP2STR(&ip.ip));
            ESP_LOGI(TAG, "MASK:"IPSTR, IP2STR(&ip.netmask));
            ESP_LOGI(TAG, "GW:"IPSTR, IP2STR(&ip.gw));
            ESP_LOGI(TAG, "~~~~~~~~~~~");
        }

		memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
		if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip) == 0) {
           ESP_LOGI(TAG, "TCPIP_ADAPTER_IF_AP ~~~~~~~~~~~");
           ESP_LOGI(TAG, "IP:"IPSTR, IP2STR(&ip.ip));
           ESP_LOGI(TAG, "MASK:"IPSTR, IP2STR(&ip.netmask));
           ESP_LOGI(TAG, "GW:"IPSTR, IP2STR(&ip.gw));
           ESP_LOGI(TAG, "~~~~~~~~~~~");
       }
	   
	   memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
       if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip) == 0) {
           ESP_LOGI(TAG, "TCPIP_ADAPTER_IF_ETH ~~~~~~~~~~~");
           ESP_LOGI(TAG, "IP:"IPSTR, IP2STR(&ip.ip));
           ESP_LOGI(TAG, "MASK:"IPSTR, IP2STR(&ip.netmask));
           ESP_LOGI(TAG, "GW:"IPSTR, IP2STR(&ip.gw));
           ESP_LOGI(TAG, "~~~~~~~~~~~");
       }
	   memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
       if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_TEST, &ip) == 0) {
           ESP_LOGI(TAG, "TCPIP_ADAPTER_IF_TEST ~~~~~~~~~~~");
           ESP_LOGI(TAG, "IP:"IPSTR, IP2STR(&ip.ip));
           ESP_LOGI(TAG, "MASK:"IPSTR, IP2STR(&ip.netmask));
           ESP_LOGI(TAG, "GW:"IPSTR, IP2STR(&ip.gw));
           ESP_LOGI(TAG, "~~~~~~~~~~~");
       }
		ESP_LOGI(TAG, "Free heap size: %d\n", esp_get_free_heap_size());
    }
}
        


int app_main(void)
{
	++boot_count;
    ESP_LOGI(TAG, "GREENOLE Initializing...");

    ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Boot count: %d", boot_count);

	uint8_t mac[6];
	esp_read_mac(mac, ESP_MAC_WIFI_STA);
	sprintf(DEVICE_ID, "%02X%02X%02X%02X%02X%02X", MAC2STR(mac));
	ESP_LOGI(TAG, "Device ID: %s", DEVICE_ID);

	status_event_group = xEventGroupCreate();

	tcpip_adapter_init();

	// Initialize NVS needed by Wi-Fi
	ESP_ERROR_CHECK(nvs_flash_init());
	// Initialize TCP/IP network interface
	ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
	/* Initialize file storage */
    ESP_ERROR_CHECK(init_spiffs());


	//esp_log_set_putchar(&user_printf);

	esp_task_wdt_reset();
	hw_wdt_disable();
	//Initialize mDNS
	initialise_mdns();
	netbiosns_init();
	netbiosns_set_name(CONFIG_EXAMPLE_MDNS_HOST_NAME);

	tcpip_adapter_dns_info_t dns;
    dns.ip.u_addr.ip4.addr = 0x08080808;
    dns.ip.type = IPADDR_TYPE_V4;

	//xTaskCreate(&logip_task, "logIP", 512, NULL, tskIDLE_PRIORITY+8, NULL);


    ESP_ERROR_CHECK(tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_ETH, TCPIP_ADAPTER_DNS_MAIN, &dns));
    ESP_ERROR_CHECK(tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, TCPIP_ADAPTER_DNS_MAIN, &dns));
    ESP_ERROR_CHECK(tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_TEST, TCPIP_ADAPTER_DNS_MAIN, &dns));
	wifi_ap_init();
	//wifi_init_sta();

	start_server("/html");

	//esp_task_wdt_reset();

	ppposInit();

	//Aguarda uma conexão válida para prosseguir
	xEventGroupWaitBits(status_event_group, WIFI_CONNECTED_BIT | GSM_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
	ping_init();
	sntp_client_init();
	

	//Aguarda a sincronização NTP para prosseguir
	xEventGroupWaitBits(status_event_group, SNTP_SYNCHRONIZED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	mqtt_app_start();
	//vTaskDelay(100);
	

	ESP_LOGI(TAG, "Free heap size: %d\n", esp_get_free_heap_size());

	//temperature_init();

    while(1) 
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        esp_task_wdt_reset();

		ESP_LOGI(TAG, "Free heap size: %d\n", esp_get_free_heap_size());
    }
}