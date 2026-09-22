#include "bemfa_wifi.h"

#include <stddef.h>

#include "wifi.h"

#define BEMFA_TX_BUFFER_SIZE 160u

static int bemfa_wifi_send_formatted(int format_result, const char *buf)
{
    if (format_result < 0)
    {
        return format_result;
    }

    wifi_SendStr(buf);
    return BEMFA_OK;
}

int bemfa_wifi_open_stream(const char *host, uint16_t port, uint32_t timeout_ms)
{
    uint8_t rc;

    rc = WIFI_OpenTcpTransparent(host, port, timeout_ms);
    if (rc == 0)
    {
        return BEMFA_OK;
    }
    if (rc == 2)
    {
        return BEMFA_ERR_SOCKET;
    }
    if (rc == 3)
    {
        return BEMFA_ERR_TRANSPARENT;
    }
    return BEMFA_ERR_PARAM;
}

int bemfa_wifi_subscribe(const char *uid, const char *topic)
{
    char buf[BEMFA_TX_BUFFER_SIZE];

    return bemfa_wifi_send_formatted(
        bemfa_format_subscribe(buf, sizeof(buf), uid, topic),
        buf);
}

int bemfa_wifi_publish(const char *uid, const char *topic, const char *message)
{
    char buf[BEMFA_TX_BUFFER_SIZE];

    return bemfa_wifi_send_formatted(
        bemfa_format_publish(buf, sizeof(buf), uid, topic, message),
        buf);
}

int bemfa_wifi_publish_pair(const char *uid, const char *topic, uint32_t first, uint32_t second)
{
    char buf[BEMFA_TX_BUFFER_SIZE];

    return bemfa_wifi_send_formatted(
        bemfa_format_publish_pair(buf, sizeof(buf), uid, topic, first, second),
        buf);
}

int bemfa_wifi_publish_sensor(const char *uid,
                              const char *topic,
                              int temperature,
                              int humidity,
                              int mode)
{
    char buf[BEMFA_TX_BUFFER_SIZE];

    return bemfa_wifi_send_formatted(
        bemfa_format_publish_sensor(buf,
                                    sizeof(buf),
                                    uid,
                                    topic,
                                    temperature,
                                    humidity,
                                    mode),
        buf);
}
