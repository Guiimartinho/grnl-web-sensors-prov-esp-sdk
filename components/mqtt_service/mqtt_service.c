/* MQTT (over TCP) Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
//#include "protocol_examples_common.h"
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_timer.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "esp_system.h"
#include "esp_task_wdt.h"

#include "mqtt_service.h"
#include "cJSON.h"
#include "define.h"

static const char *TAG = "MQTT";

#define CONFIG_MQTT_PROTOCOL_311

esp_mqtt_client_handle_t mqtt_client;
static EventGroupHandle_t mqtt_event_group;


extern void mqtt_receive(esp_mqtt_event_handle_t event);

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
//    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        char *publish_topic = NULL;
        asprintf(&publish_topic, TOPIC_CMD, DEVICE_ID);
        msg_id = esp_mqtt_client_subscribe(mqtt_client, publish_topic, 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        free(publish_topic);

        //msg_id = esp_mqtt_client_publish(client, TOPIC_SYS, "{\"variable\":\"system_init\"}", 0, 1, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        //msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        //msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        //msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        //ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        esp_mqtt_client_reconnect(mqtt_client);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        ESP_LOGI(TAG, "Free memory after recieving: %d bytes", esp_get_free_heap_size());
        mqtt_receive(event);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        //if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
        //    ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
        //    ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
        //    ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
        //             strerror(event->error_handle->esp_transport_sock_errno));
        //} else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
        //    ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        //} else {
        //    ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        //}
        break;
    case MQTT_EVENT_ANY:
        ESP_LOGI(TAG, "MQTT_EVENT_ANY");
        break;

//    case MQTT_EVENT_DELETED:
//        ESP_LOGI(TAG, "MQTT_EVENT_DELETED");
//        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}





static void mqtt_task(void *pvParameter)
{
	int msg_id;
//  int count = 20;
//	char data[255];

    cJSON *msgJSON = cJSON_CreateObject();

    cJSON_AddStringToObject(msgJSON, "variable", "system_init");

    char *publish_topic = NULL;
    asprintf(&publish_topic, TOPIC_SYS, DEVICE_ID);
    char* buff = cJSON_PrintUnformatted(msgJSON);
    msg_id = esp_mqtt_client_publish(mqtt_client, publish_topic, buff, 0, 1, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    free(publish_topic);
    free(buff);
    cJSON_Delete(msgJSON);
    
	while (1)
    {
		
		
//        time_t now = time(NULL) + MSG_OFFSET;
//        if(MSG_SEND_TIME && MSG_last_time != (now / MSG_SEND_TIME))
//        {
//            MSG_last_time = (now / MSG_SEND_TIME);
//
//		    sprintf(data, "{\"variable\":\"temperatura\",\"value\":%d}]", count);
//		    //ESP_LOGI(TAG, "Message: %s", &data);
//		    msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_DATA, data, 0, 1, 0);
//		    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
//		    if(count++ > 37)
//                count = 20;
//        }
        






//		msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_DATA, itoa(count, data, 10), 0, 1, 0);
//		ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
//
//		sprintf(data, "[{\"variable\":\"time\",\"value\":\"%d\"}]", (int32_t)esp_timer_get_time() / 1000000);
//		//ESP_LOGI(TAG, "Message: %s", data);
//		msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_DATA, data, 0, 1, 0);
//		ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

		//int rssi = _get_wifi_rssi();
		//msg_id = esp_mqtt_client_publish(mqtt_client, "$stats/rssi", rssi, 0, 1, 0);
		//ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
		
		// Translate to "signal" percentage, assuming RSSI range of (-100,-50)
		//msg_id = esp_mqtt_client_publish(mqtt_client, "$stats/signal", _clamp((rssi + 100) * 2, 0, 1, 0);
		//ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
		
		//msg_id = esp_mqtt_client_publish(mqtt_client, "/stats/freeheap", esp_get_free_heap_size(), 0, 1, 0);
		//ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
        

    }
}

void MQTT_publish(char *topic, char *publish_message)
{
    int msg_id;
    char *publish_topic = NULL;
    asprintf(&publish_topic, topic, DEVICE_ID);
    msg_id = esp_mqtt_client_publish(mqtt_client, publish_topic, publish_message, 0, 1, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    ESP_LOGI(TAG, "publishing msg \"%s\" to topic: \"%s\"", publish_message, publish_topic);
    free(publish_topic);
}


void mqtt_app_start(void)
{

    ESP_LOGI(TAG, "Starting MQTT");

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    
    mqtt_event_group = xEventGroupCreate();

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://mqtt.tago.io",//CONFIG_BROKER_URL,
        .transport = MQTT_TRANSPORT_UNKNOWN,
        .port = 1883,
        .username = "GRNL_HTTPS", //CONFIG_BROKER_USERNAME
        .password = "2881435a-b2a7-48ef-861f-7ace1d655a5d", //CONFIG_BROKER_PASSWORD
        //.lwt_topic = "/grnl",
        .client_id = DEVICE_ID,
        .protocol_ver = MQTT_PROTOCOL_V_3_1_1,
        .keepalive = 1000,
    };

    ESP_LOGI(TAG, "Broker: %s", mqtt_cfg.uri);

     mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, false, true, portMAX_DELAY);

    xTaskCreate(&mqtt_task, "mqtt_task", 4096, NULL, tskIDLE_PRIORITY+6, NULL);

}
