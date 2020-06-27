#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "xpwm.h"
#include "cJSON.h"
#include "mqtt_tcp.h"
static void post_data_to_clouds();
int parse_data(char *data)
{
    ////首先整体判断是否为一个json格式的数据
    cJSON *pJsonRoot = cJSON_Parse(data);

    //如果是否json格式数据
    if (pJsonRoot == NULL) {
        return 0;
    }

    cJSON *pChange = cJSON_GetObjectItem(pJsonRoot, "change");  //得到字段
    cJSON *pValue = cJSON_GetObjectItem(pJsonRoot, "value");

    //判断字段是否pChange格式
    if (pChange && pValue) {
        //判断字段是否string类型
        if (cJSON_IsString(pChange))
            printf("get pChange:%s \n", pChange->valuestring);
        else
            return 0; 

        //获取最新的状态
        if (strcmp(pChange->valuestring, "query") == 0) {
            post_data_to_clouds();
        }
        //收到服务器的开关灯指令
        else if (strcmp(pChange->valuestring, "power") == 0) {
            //开灯
            if (strcmp(pValue->valuestring, "true") == 0) {
                light_driver_set_rgb(0, 255, 0);
            }
            //关灯
            else {
                light_driver_set_rgb(0, 0, 0);
            }
        }
        //收到服务器的调节亮度灯指令
        else if (strcmp(pChange->valuestring, "pwm") == 0) {
            light_driver_set_rgb(cJSON_GetArrayItem(pValue, 0)->valueint, cJSON_GetArrayItem(pValue, 1)->valueint, cJSON_GetArrayItem(pValue, 2)->valueint);
        }
        //每次下发成功控制都要主动上报给服务器
        // post_data_to_clouds();
    }
    else
        printf("get pChange failed \n");
    //删除json，释放内存
    cJSON_Delete(pJsonRoot);
    return 1;
}
/**
 * @description: 上报数据给服务器
 * @param {type}
 * @return:
 */
static void post_data_to_clouds()
{
    int get_ip[4];
    uint8_t device_mac[6];
    char device_str_mac[12];
    tcpip_adapter_ip_info_t ip_info;
    //设备ip
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
    get_ip[0] = (ip_info.ip.addr & 0xff);
    get_ip[1] = (ip_info.ip.addr >> 8 & 0xff);
    get_ip[2] = (ip_info.ip.addr >> 16 & 0xff);
    get_ip[3] = (ip_info.ip.addr >> 24 & 0xff);
    //设备mac
    esp_efuse_mac_get_default(device_mac);
    for (uint8_t i = 0; i < 6; i++) {
        sprintf(device_str_mac + i * 2, "%02x", device_mac[i]);
    }

	cJSON *pRoot = cJSON_CreateObject();
    //ip
    cJSON_AddItemToObject(pRoot, "deviceIp", cJSON_CreateIntArray(get_ip, 4));

    //设备mac
    cJSON_AddStringToObject(pRoot, "deviceMac", device_str_mac);

	// //获取当前的rgb输出百分比
	uint8_t red = 0, green = 0, blue = 0;
	light_driver_get_rgb(&red, &green, &blue);

	//是否为0,否则就是开灯状态！
	if (red == 0 && green == 0 && blue == 0)
		cJSON_AddBoolToObject(pRoot, "power", false);
	else
		cJSON_AddBoolToObject(pRoot, "power",true );

	//上报pwm百分比，作为参数给服务器
	cJSON_AddNumberToObject(pRoot, "Red", red);
	cJSON_AddNumberToObject(pRoot, "Green", green);
	cJSON_AddNumberToObject(pRoot, "Blue", blue);

	//格式化为字符串
	char *s = cJSON_Print(pRoot);
	//发布消息
    esp_mqtt_publish(s, strlen(s));

	//删除json结构体，释放内存
	cJSON_free((void *)s);
	cJSON_Delete(pRoot);
}
