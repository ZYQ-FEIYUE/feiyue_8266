#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "smart_config.h"
#include "http_server.h"
#include "mqtt_tcp.h"
#include "control_rgb.h"
#include "xpwm.h"
void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    pwm_init_data();
	light_driver_set_rgb_cycle(2);
    initialise_smartconfig_wifi();
    init_http_server();
    mqtt_tcp_init();
}