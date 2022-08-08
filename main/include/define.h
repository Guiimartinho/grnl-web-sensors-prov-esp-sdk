
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1
#define GSM_CONNECTED_BIT       BIT2
#define GSM_DISCONNECTED_BIT    BIT3
#define GSM_MODEM_STOP_BIT      BIT4
#define SNTP_SYNCHRONIZED_BIT   BIT5
#define MQTT_CONNECTED_BIT      BIT6
#define MQTT_DISCONNECTED_BIT   BIT7
#define MQTT_GOT_DATA_BIT       BIT8

#define TOPIC_MASTER    "grnl/%s/"
#define TOPIC_CMD       "grnl/%s/commands"
#define TOPIC_CFG       "grnl/%s/config"
#define TOPIC_EVT       "grnl/%s/event"
#define TOPIC_STE       "grnl/%s/state"
#define TOPIC_SYS       "grnl/%s/sys"
#define TOPIC_DATA      "grnl/%s/data"
#define TOPIC_OTA       "grnl/%s/ota"

extern char DEVICE_ID[13];       //esp_efuse_mac_get_default(uint8_t *mac); MAC2STR(ap_mac)

#include "freertos/event_groups.h"
#include "cJSON.h"


extern EventGroupHandle_t status_event_group;
extern cJSON *root;


