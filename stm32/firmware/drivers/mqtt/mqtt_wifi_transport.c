#include "mqtt_wifi_transport.h"

#include <stdio.h>
#include <string.h>

#include "delay.h"
#include "wifi.h"

int mqtt_wifi_connect_ap(const char *ssid, const char *password, uint32_t timeout_ms)
{
    (void)timeout_ms;

    if (ssid == 0 || password == 0)
    {
        return MQTT_WIFI_ERR_PARAM;
    }

    usart2_config(115200);
    Delay_ms(1000);

    if (STA_ModeConnect(ssid, password) != 0)
    {
        return MQTT_WIFI_ERR_WIFI_AP;
    }

    return MQTT_WIFI_OK;
}

int mqtt_wifi_open_tcp_transparent(const char *host, uint16_t port, uint32_t timeout_ms)
{
    uint8_t rc;

    if (host == 0 || host[0] == '\0' || port == 0)
    {
        return MQTT_WIFI_ERR_PARAM;
    }
    if (timeout_ms == 0)
    {
        timeout_ms = MQTT_CLIENT_DEFAULT_TIMEOUT_MS;
    }

    rc = WIFI_OpenTcpTransparent(host, port, timeout_ms);
    if (rc == 0)
    {
        return MQTT_WIFI_OK;
    }
    if (rc == 2)
    {
        return MQTT_WIFI_ERR_SOCKET;
    }
    if (rc == 3)
    {
        return MQTT_WIFI_ERR_TRANSPARENT;
    }
    return MQTT_WIFI_ERR_PARAM;
}

int mqtt_wifi_start(const char *ssid,
                    const char *password,
                    const char *host,
                    uint16_t port,
                    uint32_t timeout_ms)
{
    int rc;

    rc = mqtt_wifi_connect_ap(ssid, password, timeout_ms);
    if (rc != MQTT_WIFI_OK)
    {
        return rc;
    }

    return mqtt_wifi_open_tcp_transparent(host, port, timeout_ms);
}

mqtt_client_transport_t mqtt_wifi_get_transport(void)
{
    mqtt_client_transport_t transport;

    transport.send = mqtt_wifi_send;
    transport.recv = mqtt_wifi_recv;

    return transport;
}

int mqtt_wifi_send(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    (void)timeout_ms;

    if (data == 0 && len != 0)
    {
        return MQTT_WIFI_ERR_PARAM;
    }

    if (len != 0)
    {
        wifi_SendArray(data, len);
    }

    return len;
}

int mqtt_wifi_recv(uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    if (data == 0 && len != 0)
    {
        return MQTT_WIFI_ERR_PARAM;
    }

    return wifi_ReadBytes(data, len, timeout_ms);
}
