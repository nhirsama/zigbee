#include "goster_mqtt.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    char buf[384];
    int len;

    len = goster_mqtt_format_topic(buf, sizeof(buf), "/goster/v1/", "/dev-1/", "/telemetry/");
    assert(len > 0);
    assert(strcmp(buf, "goster/v1/dev-1/telemetry") == 0);

    len = goster_mqtt_format_sensor_payload(buf, sizeof(buf), 23u, 56u, 1u, 1u, 0x1234u, 1700000000u, 0);
    assert(len > 0);
    assert(strcmp(buf,
                  "{\"device_type\":\"stm32_zigbee_gateway\","
                  "\"sensor_type\":1,\"nwk_addr\":\"0x1234\",\"state\":1,\"uptime_ms\":1700000000,"
                  "\"metrics\":["
                  "{\"name\":\"temperature\",\"value\":23,\"unit\":\"C\",\"legacy_metric_type\":1},"
                  "{\"name\":\"humidity\",\"value\":56,\"unit\":\"%\",\"legacy_metric_type\":2},"
                  "{\"name\":\"mode\",\"value\":1,\"legacy_metric_type\":0}]}") == 0);

    len = goster_mqtt_format_pair_payload(buf, sizeof(buf), 10u, 20u, 1700000001u, "tok-1");
    assert(len > 0);
    assert(strcmp(buf,
                  "{\"token\":\"tok-1\",\"device_type\":\"stm32_gateway\","
                  "\"uptime_ms\":1700000001,\"metrics\":["
                  "{\"name\":\"counter_a\",\"value\":10,\"legacy_metric_type\":0},"
                  "{\"name\":\"counter_b\",\"value\":20,\"legacy_metric_type\":0}]}") == 0);

    len = goster_mqtt_format_nfc_payload(buf, sizeof(buf), 1u, 1u, 1u, 0u, 1u, 0x1234u, 7u, 1700000002u, 0);
    assert(len > 0);
    assert(strcmp(buf,
                  "{\"device_type\":\"stm32_nfc_gateway\","
                  "\"event\":\"nfc_contact\",\"sensor_type\":1,\"nwk_addr\":\"0x1234\","
                  "\"state\":1,\"event_id\":7,\"uptime_ms\":1700000002,"
                  "\"metrics\":["
                  "{\"name\":\"nfc_contact\",\"value\":1,\"legacy_metric_type\":0},"
                  "{\"name\":\"param_a\",\"value\":1,\"legacy_metric_type\":0},"
                  "{\"name\":\"param_b\",\"value\":1,\"legacy_metric_type\":0},"
                  "{\"name\":\"mode\",\"value\":0,\"legacy_metric_type\":0}]}") == 0);

    len = goster_mqtt_format_heartbeat_payload(buf, sizeof(buf), 1700000002u, 0);
    assert(len > 0);
    assert(strcmp(buf, "{\"status\":\"online\",\"ts\":1700000002}") == 0);

    assert(goster_mqtt_format_topic(buf, sizeof(buf), "", "dev-1", "telemetry") == GOSTER_MQTT_ERR_PARAM);
    assert(goster_mqtt_format_topic(buf, 8u, "goster/v1", "dev-1", "telemetry") == GOSTER_MQTT_ERR_BUFFER);
    assert(goster_mqtt_format_heartbeat_payload(buf, sizeof(buf), 1u, "bad\"token") == GOSTER_MQTT_ERR_PARAM);

    puts("goster_mqtt tests passed");
    return 0;
}
