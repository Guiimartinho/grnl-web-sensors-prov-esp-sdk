#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "esp_system.h"
#include "esp_task_wdt.h"

#include "cJSON.h"
#include "define.h"

static const char *TAG = "MQTT_INTERPRET";

extern esp_mqtt_client_handle_t mqtt_client;

cJSON *msgJSON;
cJSON *init;
cJSON *freq;
cJSON *pay_size;
cJSON *qualityof;
char* payload;

unsigned long lastMillis = 0;   // time since last publish
unsigned long Timeout = 10000;  // timeout is set to 10 seconds
int period = 1000;         //specify perdiod duration
int payload_size=0;
int qos =0;               //QOS level


char *gen_string(size_t length) {       
    
    char *varString = NULL;

    if (length) {
        varString = malloc(sizeof(char) * (length +1));

        if (varString) {            
            for (int n = 0;n < length;n++) {            
                varString[n] = 'S';
            }

            varString[length] = '\0';
        }
    }

    return varString;
}

uint32_t esp_log_timestamp(void)
{
//    if (unlikely(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)) {
//        return esp_log_early_timestamp();
//    }
    static uint32_t base = 0;
    if (base == 0 && xPortGetCoreID() == 0) {
        base = esp_log_early_timestamp();
    }
    TickType_t tick_count = xPortInIsrContext() ? xTaskGetTickCountFromISR() : xTaskGetTickCount();
    return base + tick_count * (1000 / configTICK_RATE_HZ);
}

void printJsonObject(cJSON *item)
{
    char *json_string = cJSON_Print(item);
    if (json_string) 
    {
        printf("%s\n", json_string);
        cJSON_free(json_string);
    }
}

void mqtt_receive(esp_mqtt_event_handle_t event)
{
	int msg_id;

	char message[12];
    cJSON_AddStringToObject(msgJSON, "sensor_data", "");
    cJSON_AddStringToObject(msgJSON, "roundtrip_time", "");
	
	ESP_LOGI(TAG, "mqtt_receive");
    if (strncmp((event->topic), "grnl/ack" , event->topic_len) == 0)
	{
		cJSON_DeleteItemFromObject(msgJSON, "roundtrip_time");
		cJSON_AddStringToObject(msgJSON, "roundtrip_time", itoa((esp_log_timestamp() - lastMillis),message,10));
		msg_id = esp_mqtt_client_publish(mqtt_client, "grnl/action", cJSON_Print(msgJSON), 0, 0, 0);
		//GotAck = 1; //enable ack bit to verify
		ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
		printf("Roundtrip=%lu\n", esp_log_timestamp() - lastMillis);
    }
	else if (strncmp((event->topic), "grnl/start" , event->topic_len) == 0)
	{
		ESP_LOGI(TAG, "Start_Code, msg_id=%d", event->msg_id);
		init = cJSON_Parse(event->data);
		freq = cJSON_GetObjectItemCaseSensitive(init, "frequency");
		pay_size = cJSON_GetObjectItemCaseSensitive(init, "payload");
		qualityof = cJSON_GetObjectItemCaseSensitive(init, "qos");
		period = freq->valueint;
		payload_size = pay_size->valueint;
		qos = qualityof->valueint;
		//payload = gen_string(payload_size);
		cJSON_AddStringToObject(msgJSON, "message", payload);
		//ESP_LOGI(TAG, "init data is %d %d %d %s", period, payload_size, qos,payload);
		ESP_LOGI(TAG, "init data is Freq: %d, pay_size: %d, qos: %d", period, payload_size, qos);
		//Start = 1; // enable Start/tansmission bit
		//ESP_LOGI(TAG, "transmit is %d", event->msg_id);

	}
	else if (strncmp((event->topic), "grnl/kill" , event->topic_len) == 0){
		ESP_LOGI(TAG, "Kill_Code, msg_id=%d", event->msg_id);
		//Start = 0; // disable start/tranmsission bit
		free(payload);
		vTaskDelay(100);
		esp_restart();
	}
	else if (strncmp((event->topic), "grnl/print" , event->topic_len) == 0){
		ESP_LOGI(TAG, "Print_Code, msg_id=%d", event->msg_id);

		cJSON *monitor_json = cJSON_Parse(event->data);
		char *string = cJSON_Print(monitor_json);

		ESP_LOGI(TAG, "Data cJSON converted:\n%s", string);
	}

}