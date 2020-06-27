#ifndef _mqtt_tcp_H
#define _mqtt_tcp_H

#ifdef __cplusplus
extern "C"
{
#endif
void mqtt_tcp_init();
void esp_mqtt_publish(char *data, uint8_t len);
#ifdef __cplusplus
}
#endif

#endif