#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"
#include "mqtt_tcp.h"
#include "esp_wifi.h"
#include "control_rgb.h"
static const char *TAG = "MQTT_EXAMPLE";
#define MQTT_DEVICE_SUB "device/"
#define MQTT_DEVICE_PUB "AP/"
char SUB_ALL[20];
char PUB_ALL[20];
uint8_t mac[6];
esp_mqtt_client_handle_t client;
extern SemaphoreHandle_t xsendAndroidSemaphore;
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    // esp_mqtt_client_handle_t client = event->client;
    client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, SUB_ALL, 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            // msg_id = esp_mqtt_client_publish(client, MQTT_DEVICE_PUB, "data", 0, 0, 0);
            // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
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
            parse_data(event->data);
            // ESP_LOGI(TAG, "Received %d bytes: %s", event->data_len,  event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}
void esp_mqtt_publish(char *data, uint8_t len)
{
    
    int msg_id;
    msg_id = esp_mqtt_client_publish(client, PUB_ALL, (char *)data, len, 1, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}
static void mqtt_app_start(void)
{
    char str_mac[12];
    wifi_config_t wifi_config;
    esp_efuse_mac_get_default(mac);
    for (uint8_t i = 0; i < 6; i++) {
        sprintf(str_mac + i * 2, "%02x", mac[i]);
    }
    sprintf(SUB_ALL, "%s%s", MQTT_DEVICE_SUB, str_mac);     //得到订阅topic
    ESP_LOGI(TAG, "SUB_topic:%s", SUB_ALL);
    esp_wifi_get_config(ESP_IF_WIFI_STA,&wifi_config);      //发布当前连接的路由器的topic
    for (uint8_t i = 0; i < 6; i++) {
        sprintf(str_mac + i * 2, "%02x", wifi_config.sta.bssid[i]);
    }
    sprintf(PUB_ALL, "%s%s", MQTT_DEVICE_PUB, str_mac);     //得到发布topic
    ESP_LOGI(TAG, "APSSID:%s", str_mac);
    ESP_LOGI(TAG, "PUB_topic:%s", PUB_ALL);

    esp_mqtt_client_config_t mqtt_cfg = {
        .host = "rau32dc.mqtt.iot.gz.baidubce.com",
        .port = 1883,
        .username = "rau32dc/esp8266",
        .password = "UkfO2sZs5rUKZrG4",
        .client_id = str_mac,
        .disable_clean_session = 0,
        .keepalive = 300,
        .event_handle = mqtt_event_handler,
        // .user_context = (void *)your_context
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

void mqtt_tcp_init()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    mqtt_app_start();
}
