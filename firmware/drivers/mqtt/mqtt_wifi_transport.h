#ifndef _MQTT_WIFI_TRANSPORT_H_
#define _MQTT_WIFI_TRANSPORT_H_

#include <stdint.h>
#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MQTT_WIFI_OK                 0
#define MQTT_WIFI_ERR_PARAM         -1
#define MQTT_WIFI_ERR_WIFI_AP       -2
#define MQTT_WIFI_ERR_SOCKET        -3
#define MQTT_WIFI_ERR_TRANSPARENT   -4

/* Initialize USART2 and connect to WiFi AP. */
int mqtt_wifi_connect_ap(const char *ssid, const char *password, uint32_t timeout_ms);

/* Open TCP socket and enter transparent mode. host is MQTT broker IP/domain, usually port 1883. */
int mqtt_wifi_open_tcp_transparent(const char *host, uint16_t port, uint32_t timeout_ms);

/* Connect WiFi AP and MQTT broker TCP transparent mode in one step. */
int mqtt_wifi_start(const char *ssid,
                    const char *password,
                    const char *host,
                    uint16_t port,
                    uint32_t timeout_ms);

/* Transport callbacks for mqtt_client_t. */
mqtt_client_transport_t mqtt_wifi_get_transport(void);

int mqtt_wifi_send(const uint8_t *data, uint16_t len, uint32_t timeout_ms);
int mqtt_wifi_recv(uint8_t *data, uint16_t len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* _MQTT_WIFI_TRANSPORT_H_ */
