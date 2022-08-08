/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/apps/sntp.h"
#include "define.h"

static const char *TAG = "NTP";

#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_CUSTOM
void sntp_sync_time(struct timeval *tv)
{
   settimeofday(tv, NULL);
   ESP_LOGI(TAG, "Time is synchronized from custom code");
   sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}
#endif

void time_sync_notification_cb(struct timeval *tv)
{
	ESP_LOGI(TAG, "Notification of a time synchronization event");

	uint64_t utcMs = ((uint64_t)tv->tv_sec) * 1000 + tv->tv_usec;
	ESP_LOGI(TAG, "UTC time synchronized: %llu", utcMs);

	// wait for time to be set
	time_t now = 0;
	struct tm timeinfo = { 0 };
//	int retry = 0;
//	const int retry_count = 10;
//	while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
//		ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
//		vTaskDelay(2000 / portTICK_PERIOD_MS);
//	}

	time(&now);
	localtime_r(&now, &timeinfo);

	xEventGroupSetBits(status_event_group, SNTP_SYNCHRONIZED_BIT);
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}

static void obtain_time(void)
{
	/**
	* NTP server address could be aquired via DHCP,
	* see LWIP_DHCP_GET_NTP_SRV menuconfig option
	*/
#ifdef LWIP_DHCP_GET_NTP_SRV
	sntp_servermode_dhcp(1);
#endif
	
	// Initialize SNTP time synchronization
	initialize_sntp();

	// wait for time to be set
	time_t now = 0;
	struct tm timeinfo = { 0 };
//	int retry = 0;
//	const int retry_count = 10;
//	while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
//		ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
//		vTaskDelay(2000 / portTICK_PERIOD_MS);
//	}


	ESP_LOGI(TAG, "Waiting for SNTP synchronize...");
	// Wait for SNTP got response with current time
	xEventGroupWaitBits(status_event_group, SNTP_SYNCHRONIZED_BIT, 0, 1, portMAX_DELAY);
	ESP_LOGI(TAG, "SNTP synchronized");

	time(&now);
	localtime_r(&now, &timeinfo);
}

static void sntp_task(void *arg)
{

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2021 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
		time(&now);
    }
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    else {
        // add 500 ms error to the current system time.
        // Only to demonstrate a work of adjusting method!
        {
            ESP_LOGI(TAG, "Add a error for test adjtime");
            struct timeval tv_now;
            gettimeofday(&tv_now, NULL);
            int64_t cpu_time = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
            int64_t error_time = cpu_time + 500 * 1000L;
            struct timeval tv_error = { .tv_sec = error_time / 1000000L, .tv_usec = error_time % 1000000L };
            settimeofday(&tv_error, NULL);
        }

        ESP_LOGI(TAG, "Time was set, now just adjusting it. Use SMOOTH SYNC method.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
#endif

	char strftime_buf[64];

	// Set timezone to China Standard Time
	setenv("TZ", "GMTGMT+3", 1);
	tzset();

	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	ESP_LOGI(TAG, "The current GMT date/time is: %s", strftime_buf);

	if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) {
		struct timeval outdelta;
		while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {
			adjtime(NULL, &outdelta);
			ESP_LOGI(TAG, "Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us",
						(long)outdelta.tv_sec,
						outdelta.tv_usec/1000,
						outdelta.tv_usec%1000);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
		}
	}
   while (1) {
       // update 'now' variable with current time
       time(&now);
       localtime_r(&now, &timeinfo);
       if (timeinfo.tm_year < (2022 - 1900)) {
           ESP_LOGE(TAG, "The current date/time error");
       } else {
           strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
           ESP_LOGI(TAG, "The current date/time in Brasilia is: %s", strftime_buf);
       }
       ESP_LOGI(TAG, "Free heap size: %d\n", esp_get_free_heap_size());
       vTaskDelay(60000 / portTICK_RATE_MS);
   }
}


void sntp_client_init(void)
{
	// SNTP service uses LwIP, please allocate large stack space.
	xTaskCreate(sntp_task, "sntp_task", 2048, NULL, 10, NULL);
}
