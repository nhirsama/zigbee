#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* Central project configuration.
 *
 * Keep board/app secrets and cloud endpoints out of low-level drivers.
 * Values can also be overridden from Makefile/CFLAGS with -DAPP_WIFI_SSID=...
 */

#ifndef APP_WIFI_SSID
#define APP_WIFI_SSID "802.11 AP"
#endif

#ifndef APP_WIFI_PASSWORD
#define APP_WIFI_PASSWORD "YOUR_PASSWORD"
#endif

#ifndef APP_BEMFA_HOST
#define APP_BEMFA_HOST "bemfa.com"
#endif

#ifndef APP_BEMFA_PORT
#define APP_BEMFA_PORT 8344u
#endif

#ifndef APP_BEMFA_UID
#define APP_BEMFA_UID "YOUR_BEMFA_UID"
#endif

#ifndef APP_BEMFA_TOPIC
#define APP_BEMFA_TOPIC "zigbee"
#endif

#define APP_CLOUD_BACKEND_BEMFA 0u
#define APP_CLOUD_BACKEND_MQTT  1u
#define APP_CLOUD_BACKEND_GOSTER_MQTT 2u

#ifndef APP_CLOUD_BACKEND
#define APP_CLOUD_BACKEND APP_CLOUD_BACKEND_BEMFA
#endif

#ifndef APP_MQTT_HOST
#define APP_MQTT_HOST "broker.emqx.io"
#endif

#ifndef APP_MQTT_PORT
#define APP_MQTT_PORT 1883u
#endif

#ifndef APP_MQTT_CLIENT_ID
#define APP_MQTT_CLIENT_ID "stm32-zigbee-screen"
#endif

#ifndef APP_MQTT_USERNAME
#define APP_MQTT_USERNAME ""
#endif

#ifndef APP_MQTT_PASSWORD
#define APP_MQTT_PASSWORD ""
#endif

#ifndef APP_MQTT_TOPIC
#define APP_MQTT_TOPIC "zigbee"
#endif

#ifndef APP_MQTT_SUB_TOPIC
#define APP_MQTT_SUB_TOPIC APP_MQTT_TOPIC "/cmd"
#endif

#ifndef APP_MQTT_QOS
#define APP_MQTT_QOS 0u
#endif

/* Goster-IoT protocol-ingress MQTT adapter.
 *
 * Goster embedded MQTT auth expects:
 *   client_id = device UUID
 *   username  = device UUID or any non-empty value
 *   password  = device token
 * Publish:
 *   goster/v1/{uuid}/telemetry
 *   goster/v1/{uuid}/heartbeat
 * Subscribe:
 *   goster/v1/{uuid}/downlink
 *
 * Set APP_CLOUD_BACKEND=APP_CLOUD_BACKEND_GOSTER_MQTT to enable it.
 */
#ifndef APP_GOSTER_MQTT_HOST
#define APP_GOSTER_MQTT_HOST APP_MQTT_HOST
#endif

#ifndef APP_GOSTER_MQTT_PORT
#define APP_GOSTER_MQTT_PORT 1883u
#endif

#ifndef APP_GOSTER_MQTT_BASE_TOPIC
#define APP_GOSTER_MQTT_BASE_TOPIC "goster/v1"
#endif

#ifndef APP_GOSTER_DEVICE_UUID
#define APP_GOSTER_DEVICE_UUID APP_MQTT_CLIENT_ID
#endif

#ifndef APP_GOSTER_DEVICE_TOKEN
#define APP_GOSTER_DEVICE_TOKEN APP_MQTT_PASSWORD
#endif

#ifndef APP_GOSTER_MQTT_USERNAME
#define APP_GOSTER_MQTT_USERNAME APP_GOSTER_DEVICE_UUID
#endif

#ifndef APP_GOSTER_MQTT_QOS
#define APP_GOSTER_MQTT_QOS 0u
#endif

/* For Goster external MQTT mode without broker-side auth, set this to 1 so
 * protocol-ingress can authenticate each payload by token. Keep 0 for the
 * embedded broker to avoid storing the token in raw event payloads.
 */
#ifndef APP_GOSTER_INCLUDE_TOKEN_IN_PAYLOAD
#define APP_GOSTER_INCLUDE_TOKEN_IN_PAYLOAD 0u
#endif

#ifndef APP_GOSTER_HEARTBEAT_INTERVAL_MS
#define APP_GOSTER_HEARTBEAT_INTERVAL_MS 30000u
#endif

#ifndef APP_MQTT_RECONNECT_INTERVAL_MS
#define APP_MQTT_RECONNECT_INTERVAL_MS 5000u
#endif

#ifndef APP_SENSOR_UI_INTERVAL_MS
#define APP_SENSOR_UI_INTERVAL_MS 250u
#endif

#ifndef APP_SENSOR_PUBLISH_INTERVAL_MS
#define APP_SENSOR_PUBLISH_INTERVAL_MS 1000u
#endif

#ifndef APP_SENSOR_POLL_BUDGET
#define APP_SENSOR_POLL_BUDGET 4u
#endif

#ifndef APP_MQTT_PUBLISH_DEBUG_LOG
#define APP_MQTT_PUBLISH_DEBUG_LOG 0u
#endif

#ifndef APP_COUNTER_TELEMETRY_ENABLE
#define APP_COUNTER_TELEMETRY_ENABLE 0u
#endif

#ifndef APP_TELEMETRY_INTERVAL_MS
#define APP_TELEMETRY_INTERVAL_MS 2000u
#endif

#ifndef APP_LED_BLINK_INTERVAL_MS
#define APP_LED_BLINK_INTERVAL_MS 500u
#endif

#endif /* APP_CONFIG_H */
