#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"

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

#include "esp_log.h"
#include "mqtt_service.h"
#include "cJSON.h"

#include "define.h"
#include "temperature.h"

static const char *TAG = "MODULE_TEMPERATURE";

//#define TEMPERATURE_DATA "{temp : %d}"
#define MIN_TEMP 20

//void publish_telemetry_event(iotc_context_handle_t context_handle,
//                             iotc_timed_task_handle_t timed_task, void *user_data)
//{
//    //IOTC_UNUSED(timed_task);
//    //IOTC_UNUSED(user_data);
//
//    char *publish_topic = NULL;
//    asprintf(&publish_topic, TOPIC_EVT, DEVICE_ID);
//    char *publish_message = NULL;
//    asprintf(&publish_message, TEMPERATURE_DATA, MIN_TEMP + rand() % 10);
//    ESP_LOGI(TAG, "publishing msg \"%s\" to topic: \"%s\"", publish_message, publish_topic);
//
//    esp_mqtt_client_publish(mqtt_client, publish_topic, publish_message, 0, 1, 0);
//
//    //iotc_publish(context_handle, publish_topic, publish_message,
//    //             iotc_example_qos,
//    //             /*callback=*/NULL, /*user_data=*/NULL);
//    free(publish_topic);
//    free(publish_message);
//}

#define MSG_OFFSET  0       //Segundos
#define MSG_SEND_TIME 15    //Segundos


static void temperarure_task(void *pvParameter)
{

	int msg_id, count = 20;
	volatile time_t MSG_last_time = 0;
    cJSON *tempJSON = cJSON_CreateObject();
    cJSON *tempe = cJSON_CreateNumber(count);

    cJSON_AddNumberToObject(tempJSON, "temperature", -100);

	while (1)
    {
		
        time_t now = time(NULL) - MSG_OFFSET;
        if((MSG_SEND_TIME != 0) && ((!(now % MSG_SEND_TIME) && now != MSG_last_time/* && difftime(now, MSG_last_time) >= MSG_SEND_TIME */) || !MSG_last_time))
        {

            MSG_last_time = now;
            int temp = MIN_TEMP + rand() % 10;
            
            cJSON_SetIntValue(cJSON_GetObjectItem(tempJSON, "temperature"), temp);
            char *buff = cJSON_PrintUnformatted(tempJSON);
            //cJSON_Minify(buff);
            ESP_LOGI(TAG, "Message %s", buff);
            MQTT_publish(TOPIC_EVT, buff);
            free(buff);
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
            

}



void temperature_init(void)
{
    xTaskCreate(&temperarure_task, "temperarure_task", 2048, NULL, tskIDLE_PRIORITY+5, NULL);
}


