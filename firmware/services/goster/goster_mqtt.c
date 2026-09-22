#include "goster_mqtt.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static int goster_checked_snprintf(char *buf, size_t buf_size, const char *fmt, ...)
{
    int len;
    va_list args;

    if (buf == 0 || buf_size == 0 || fmt == 0)
    {
        return GOSTER_MQTT_ERR_PARAM;
    }

    va_start(args, fmt);
    len = vsnprintf(buf, buf_size, fmt, args);
    va_end(args);

    if (len < 0)
    {
        return GOSTER_MQTT_ERR_BUFFER;
    }
    if ((size_t)len >= buf_size)
    {
        buf[0] = '\0';
        return GOSTER_MQTT_ERR_BUFFER;
    }

    return len;
}

static int goster_json_token_is_safe(const char *token)
{
    const unsigned char *p;

    if (token == 0 || token[0] == '\0')
    {
        return 1;
    }

    p = (const unsigned char *)token;
    while (*p != '\0')
    {
        if (*p < 0x20u || *p == '"' || *p == '\\')
        {
            return 0;
        }
        p++;
    }

    return 1;
}

static const char *goster_optional_token(const char *token)
{
    if (token == 0 || token[0] == '\0')
    {
        return 0;
    }
    return token;
}

int goster_mqtt_format_topic(char *buf,
                             size_t buf_size,
                             const char *base_topic,
                             const char *device_uuid,
                             const char *kind)
{
    const char *base_start;
    const char *base_end;
    const char *uuid_start;
    const char *uuid_end;
    const char *kind_start;
    const char *kind_end;
    size_t base_len;
    size_t uuid_len;
    size_t kind_len;

    if (buf == 0 || buf_size == 0 || base_topic == 0 || device_uuid == 0 || kind == 0)
    {
        return GOSTER_MQTT_ERR_PARAM;
    }

    base_start = base_topic;
    while (*base_start == '/')
    {
        base_start++;
    }
    base_end = base_start + strlen(base_start);
    while (base_end > base_start && base_end[-1] == '/')
    {
        base_end--;
    }

    uuid_start = device_uuid;
    while (*uuid_start == '/')
    {
        uuid_start++;
    }
    uuid_end = uuid_start + strlen(uuid_start);
    while (uuid_end > uuid_start && uuid_end[-1] == '/')
    {
        uuid_end--;
    }

    kind_start = kind;
    while (*kind_start == '/')
    {
        kind_start++;
    }
    kind_end = kind_start + strlen(kind_start);
    while (kind_end > kind_start && kind_end[-1] == '/')
    {
        kind_end--;
    }

    base_len = (size_t)(base_end - base_start);
    uuid_len = (size_t)(uuid_end - uuid_start);
    kind_len = (size_t)(kind_end - kind_start);
    if (base_len == 0u || uuid_len == 0u || kind_len == 0u)
    {
        return GOSTER_MQTT_ERR_PARAM;
    }

    return goster_checked_snprintf(buf,
                                   buf_size,
                                   "%.*s/%.*s/%.*s",
                                   (int)base_len,
                                   base_start,
                                   (int)uuid_len,
                                   uuid_start,
                                   (int)kind_len,
                                   kind_start);
}

int goster_mqtt_format_sensor_payload(char *buf,
                                      size_t buf_size,
                                      uint32_t temperature,
                                      uint32_t humidity,
                                      uint32_t mode,
                                      uint32_t sensor_type,
                                      uint32_t nwk_addr,
                                      uint32_t timestamp_ms,
                                      const char *token)
{
    token = goster_optional_token(token);
    if (!goster_json_token_is_safe(token))
    {
        return GOSTER_MQTT_ERR_PARAM;
    }

    if (token != 0)
    {
        return goster_checked_snprintf(
            buf,
            buf_size,
            "{\"token\":\"%s\",\"device_type\":\"stm32_zigbee_gateway\","
            "\"sensor_type\":%lu,\"nwk_addr\":\"0x%04lX\",\"state\":%lu,\"uptime_ms\":%lu,"
            "\"metrics\":["
            "{\"name\":\"temperature\",\"value\":%lu,\"unit\":\"C\",\"legacy_metric_type\":1},"
            "{\"name\":\"humidity\",\"value\":%lu,\"unit\":\"%%\",\"legacy_metric_type\":2},"
            "{\"name\":\"mode\",\"value\":%lu,\"legacy_metric_type\":0}]}",
            token,
            (unsigned long)sensor_type,
            (unsigned long)(nwk_addr & 0xFFFFu),
            (unsigned long)mode,
            (unsigned long)timestamp_ms,
            (unsigned long)temperature,
            (unsigned long)humidity,
            (unsigned long)mode);
    }

    return goster_checked_snprintf(
        buf,
        buf_size,
        "{\"device_type\":\"stm32_zigbee_gateway\","
        "\"sensor_type\":%lu,\"nwk_addr\":\"0x%04lX\",\"state\":%lu,\"uptime_ms\":%lu,"
        "\"metrics\":["
        "{\"name\":\"temperature\",\"value\":%lu,\"unit\":\"C\",\"legacy_metric_type\":1},"
        "{\"name\":\"humidity\",\"value\":%lu,\"unit\":\"%%\",\"legacy_metric_type\":2},"
        "{\"name\":\"mode\",\"value\":%lu,\"legacy_metric_type\":0}]}",
        (unsigned long)sensor_type,
        (unsigned long)(nwk_addr & 0xFFFFu),
        (unsigned long)mode,
        (unsigned long)timestamp_ms,
        (unsigned long)temperature,
        (unsigned long)humidity,
        (unsigned long)mode);
}

int goster_mqtt_format_nfc_payload(char *buf,
                                   size_t buf_size,
                                   uint32_t contact,
                                   uint32_t param_a,
                                   uint32_t param_b,
                                   uint32_t mode,
                                   uint32_t sensor_type,
                                   uint32_t nwk_addr,
                                   uint32_t event_id,
                                   uint32_t timestamp_ms,
                                   const char *token)
{
    token = goster_optional_token(token);
    if (!goster_json_token_is_safe(token))
    {
        return GOSTER_MQTT_ERR_PARAM;
    }

    if (token != 0)
    {
        return goster_checked_snprintf(
            buf,
            buf_size,
            "{\"token\":\"%s\",\"device_type\":\"stm32_nfc_gateway\","
            "\"event\":\"nfc_contact\",\"sensor_type\":%lu,\"nwk_addr\":\"0x%04lX\","
            "\"state\":%lu,\"event_id\":%lu,\"uptime_ms\":%lu,"
            "\"metrics\":["
            "{\"name\":\"nfc_contact\",\"value\":%lu,\"legacy_metric_type\":0},"
            "{\"name\":\"param_a\",\"value\":%lu,\"legacy_metric_type\":0},"
            "{\"name\":\"param_b\",\"value\":%lu,\"legacy_metric_type\":0},"
            "{\"name\":\"mode\",\"value\":%lu,\"legacy_metric_type\":0}]}",
            token,
            (unsigned long)sensor_type,
            (unsigned long)(nwk_addr & 0xFFFFu),
            (unsigned long)contact,
            (unsigned long)event_id,
            (unsigned long)timestamp_ms,
            (unsigned long)contact,
            (unsigned long)param_a,
            (unsigned long)param_b,
            (unsigned long)mode);
    }

    return goster_checked_snprintf(
        buf,
        buf_size,
        "{\"device_type\":\"stm32_nfc_gateway\","
        "\"event\":\"nfc_contact\",\"sensor_type\":%lu,\"nwk_addr\":\"0x%04lX\","
        "\"state\":%lu,\"event_id\":%lu,\"uptime_ms\":%lu,"
        "\"metrics\":["
        "{\"name\":\"nfc_contact\",\"value\":%lu,\"legacy_metric_type\":0},"
        "{\"name\":\"param_a\",\"value\":%lu,\"legacy_metric_type\":0},"
        "{\"name\":\"param_b\",\"value\":%lu,\"legacy_metric_type\":0},"
        "{\"name\":\"mode\",\"value\":%lu,\"legacy_metric_type\":0}]}",
        (unsigned long)sensor_type,
        (unsigned long)(nwk_addr & 0xFFFFu),
        (unsigned long)contact,
        (unsigned long)event_id,
        (unsigned long)timestamp_ms,
        (unsigned long)contact,
        (unsigned long)param_a,
        (unsigned long)param_b,
        (unsigned long)mode);
}

int goster_mqtt_format_pair_payload(char *buf,
                                    size_t buf_size,
                                    uint32_t first,
                                    uint32_t second,
                                    uint32_t timestamp_ms,
                                    const char *token)
{
    token = goster_optional_token(token);
    if (!goster_json_token_is_safe(token))
    {
        return GOSTER_MQTT_ERR_PARAM;
    }

    if (token != 0)
    {
        return goster_checked_snprintf(
            buf,
            buf_size,
            "{\"token\":\"%s\",\"device_type\":\"stm32_gateway\","
            "\"uptime_ms\":%lu,\"metrics\":["
            "{\"name\":\"counter_a\",\"value\":%lu,\"legacy_metric_type\":0},"
            "{\"name\":\"counter_b\",\"value\":%lu,\"legacy_metric_type\":0}]}",
            token,
            (unsigned long)timestamp_ms,
            (unsigned long)first,
            (unsigned long)second);
    }

    return goster_checked_snprintf(
        buf,
        buf_size,
        "{\"device_type\":\"stm32_gateway\","
        "\"uptime_ms\":%lu,\"metrics\":["
        "{\"name\":\"counter_a\",\"value\":%lu,\"legacy_metric_type\":0},"
        "{\"name\":\"counter_b\",\"value\":%lu,\"legacy_metric_type\":0}]}",
        (unsigned long)timestamp_ms,
        (unsigned long)first,
        (unsigned long)second);
}

int goster_mqtt_format_heartbeat_payload(char *buf,
                                         size_t buf_size,
                                         uint32_t timestamp_ms,
                                         const char *token)
{
    token = goster_optional_token(token);
    if (!goster_json_token_is_safe(token))
    {
        return GOSTER_MQTT_ERR_PARAM;
    }

    if (token != 0)
    {
        return goster_checked_snprintf(buf,
                                       buf_size,
                                       "{\"token\":\"%s\",\"status\":\"online\",\"ts\":%lu}",
                                       token,
                                       (unsigned long)timestamp_ms);
    }

    return goster_checked_snprintf(buf,
                                   buf_size,
                                   "{\"status\":\"online\",\"ts\":%lu}",
                                   (unsigned long)timestamp_ms);
}
