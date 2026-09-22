#include "main.h"

#include "app_config.h"
#include "app_ui.h"
#include "beep.h"
#include "bemfa_wifi.h"
#include "goster_mqtt.h"
#include "key.h"
#include "lcd.h"
#include "led.h"
#include "mqtt_client.h"
#include "mqtt_wifi_transport.h"
#include "sensor.h"
#include "usart1.h"
#include "wifi.h"

#define APP_USES_MQTT ((APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_MQTT) || \
                       (APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT))

#if APP_USES_MQTT
#define APP_BOOT_TOTAL_STEPS 11u
#else
#define APP_BOOT_TOTAL_STEPS 9u
#endif

static uint8_t led_flag = 0;
#if APP_COUNTER_TELEMETRY_ENABLE
static uint32_t telemetry_a = 0;
static uint32_t telemetry_b = 0;
#endif
static uint8_t boot_done = 0;
static sensor_node_t app_pending_sensor = {0};
static sensor_frame_info_t app_pending_sensor_frame = {0};
static uint8_t app_sensor_has_value = 0u;
static uint8_t app_sensor_ui_dirty = 0u;
static uint8_t app_sensor_publish_dirty = 0u;
static uint32_t app_sensor_last_ui_ms = 0u;
static uint32_t app_sensor_last_publish_ms = 0u;
static uint8_t app_nfc_contact_state = 0u;
static uint8_t app_nfc_event_pending = 0u;
static uint32_t app_nfc_event_seq = 0u;
static sensor_node_t app_nfc_event_sensor = {0};
static sensor_frame_info_t app_nfc_event_frame = {0};

#if APP_USES_MQTT
static mqtt_client_t app_mqtt;
static uint8_t app_mqtt_tx_buf[512];
static uint8_t app_mqtt_rx_buf[512];
static uint8_t app_mqtt_connected = 0u;
static uint32_t app_mqtt_last_ping_ms = 0u;
static uint32_t app_mqtt_last_reconnect_ms = 0u;
#endif

#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
static uint32_t app_goster_last_heartbeat_ms = 0u;
#endif

static void App_BootStep(const char *label) {
    if (boot_done < APP_BOOT_TOTAL_STEPS) {
        boot_done++;
    }
    AppUI_BootProgress(boot_done, APP_BOOT_TOTAL_STEPS, label);
}

static void App_BootFail(const char *label) {
    AppUI_BootProgress(boot_done, APP_BOOT_TOTAL_STEPS, label);
}

#if APP_USES_MQTT
static const char *App_MqttOptionalText(const char *value) {
    if (value == 0 || value[0] == '\0') {
        return 0;
    }
    return value;
}

static void App_MqttOnMessage(const char *topic,
                              uint16_t topic_len,
                              const uint8_t *payload,
                              uint16_t payload_len,
                              uint8_t qos,
                              uint8_t retained,
                              void *user_context) {
    (void) user_context;
    (void) qos;
    (void) retained;

    printf("mqtt rx topic:%.*s payload:%.*s\r\n",
           (int)topic_len,
           (topic != 0) ? topic : "",
           (int)payload_len,
           (payload != 0) ? (const char *)payload : "");
}

static void App_MqttMarkDisconnected(const char *label, int rc) {
    app_mqtt_connected = 0u;
    app_mqtt.connected = 0u;
    AppUI_SetNetworkStatus(0u);
    if (label != 0) {
        printf("%s:%d\r\n", label, rc);
    }
}

static int App_MqttPublishTextTo(const char *topic, const char *payload) {
    if (app_mqtt_connected == 0u || payload == 0) {
        return MQTT_CLIENT_ERR_TRANSPORT;
    }

    return mqtt_client_publish_string(&app_mqtt,
                                      topic,
                                      payload,
                                      (uint8_t)(
#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
                                          APP_GOSTER_MQTT_QOS
#else
                                          APP_MQTT_QOS
#endif
                                      ),
                                      0u);
}

#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
static const char *App_GosterPayloadToken(void) {
#if APP_GOSTER_INCLUDE_TOKEN_IN_PAYLOAD
    return APP_GOSTER_DEVICE_TOKEN;
#else
    return 0;
#endif
}

static int App_GosterPublishKind(const char *kind, const char *payload) {
    char topic[96];
    int len;

    len = goster_mqtt_format_topic(topic,
                                   sizeof(topic),
                                   APP_GOSTER_MQTT_BASE_TOPIC,
                                   APP_GOSTER_DEVICE_UUID,
                                   kind);
    if (len < 0) {
        printf("goster topic format failed:%d\r\n", len);
        return MQTT_CLIENT_ERR_PARAM;
    }

    return App_MqttPublishTextTo(topic, payload);
}

static int App_GosterPublishHeartbeat(uint32_t now_ms) {
    char payload[160];
    int len;
    int rc;

    len = goster_mqtt_format_heartbeat_payload(payload,
                                               sizeof(payload),
                                               now_ms,
                                               App_GosterPayloadToken());
    if (len < 0) {
        printf("goster heartbeat format failed:%d\r\n", len);
        return MQTT_CLIENT_ERR_BUFFER;
    }
    rc = App_GosterPublishKind("heartbeat", payload);
#if APP_MQTT_PUBLISH_DEBUG_LOG
    printf("goster heartbeat publish rc:%d\r\n", rc);
#endif
    return rc;
}
#else
static int App_MqttPublishText(const char *payload) {
    return App_MqttPublishTextTo(APP_MQTT_TOPIC, payload);
}
#endif
#endif

static uint8_t App_IsNfcContact(const sensor_node_t *node) {
    return (node != 0 && node->t_val == 1u && node->h_val == 1u) ? 1u : 0u;
}

static int App_CloudPublishSensor(const sensor_node_t *node,
                                  const sensor_frame_info_t *frame_info) {
    if (node == 0 || AppUI_IsPublishEnabled() == 0u) {
        return 0;
    }

#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
    {
        char payload[320];
        int len;
        int rc;
        const sensor_frame_info_t *info = (frame_info != 0) ? frame_info : &sensor_last_frame;
        len = goster_mqtt_format_sensor_payload(payload,
                                                sizeof(payload),
                                                node->t_val,
                                                node->h_val,
                                                node->mode,
                                                info->type,
                                                info->nwk_addr,
                                                Delay_GetMillis(),
                                                App_GosterPayloadToken());
        if (len < 0) {
            printf("goster sensor format failed:%d\r\n", len);
            return MQTT_CLIENT_ERR_BUFFER;
        }
        rc = App_GosterPublishKind("telemetry", payload);
#if APP_MQTT_PUBLISH_DEBUG_LOG
        printf("goster sensor publish rc:%d\r\n", rc);
#endif
        if (rc != MQTT_CLIENT_OK) {
            App_MqttMarkDisconnected("goster sensor publish failed", rc);
        }
        return rc;
    }
#elif APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_MQTT
    {
        char payload[40];
        snprintf(payload,
                 sizeof(payload),
                 "#%u#%u#%u",
                 (unsigned int)node->t_val,
                 (unsigned int)node->h_val,
                 (unsigned int)node->mode);
        return App_MqttPublishText(payload);
    }
#else
    return bemfa_wifi_publish_sensor(APP_BEMFA_UID,
                                     APP_BEMFA_TOPIC,
                                     node->t_val,
                                     node->h_val,
                                     node->mode);
#endif
}

static int App_CloudPublishNfcEvent(const sensor_node_t *node,
                                    const sensor_frame_info_t *frame_info,
                                    uint32_t event_id) {
    uint8_t contact;

    if (node == 0 || AppUI_IsPublishEnabled() == 0u) {
        return 0;
    }

    contact = App_IsNfcContact(node);

#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
    {
        char payload[448];
        int len;
        int rc;
        const sensor_frame_info_t *info = (frame_info != 0) ? frame_info : &sensor_last_frame;
        len = goster_mqtt_format_nfc_payload(payload,
                                             sizeof(payload),
                                             contact,
                                             node->t_val,
                                             node->h_val,
                                             node->mode,
                                             info->type,
                                             info->nwk_addr,
                                             event_id,
                                             Delay_GetMillis(),
                                             App_GosterPayloadToken());
        if (len < 0) {
            printf("goster nfc format failed:%d\r\n", len);
            return MQTT_CLIENT_ERR_BUFFER;
        }
        rc = App_GosterPublishKind("telemetry", payload);
#if APP_MQTT_PUBLISH_DEBUG_LOG
        printf("goster nfc publish rc:%d\r\n", rc);
#endif
        if (rc != MQTT_CLIENT_OK) {
            App_MqttMarkDisconnected("goster nfc publish failed", rc);
        }
        return rc;
    }
#elif APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_MQTT
    {
        char payload[96];
        snprintf(payload,
                 sizeof(payload),
                 "{\"event\":\"nfc_contact\",\"state\":%u,\"p1\":%u,\"p2\":%u,\"id\":%lu}",
                 (unsigned int)contact,
                 (unsigned int)node->t_val,
                 (unsigned int)node->h_val,
                 (unsigned long)event_id);
        return App_MqttPublishText(payload);
    }
#else
    {
        char payload[32];
        snprintf(payload,
                 sizeof(payload),
                 "#NFC#%u#%u#%u",
                 (unsigned int)contact,
                 (unsigned int)node->t_val,
                 (unsigned int)node->h_val);
        return bemfa_wifi_publish(APP_BEMFA_UID, APP_BEMFA_TOPIC, payload);
    }
#endif
}

#if APP_COUNTER_TELEMETRY_ENABLE
static void App_CloudPublishPair(uint32_t first, uint32_t second) {
    if (AppUI_IsPublishEnabled() == 0u) {
        return;
    }

#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
    {
        char payload[224];
        int len;
        int rc;
        len = goster_mqtt_format_pair_payload(payload,
                                              sizeof(payload),
                                              first,
                                              second,
                                              Delay_GetMillis(),
                                              App_GosterPayloadToken());
        if (len < 0) {
            printf("goster pair format failed:%d\r\n", len);
            return;
        }
        rc = App_GosterPublishKind("telemetry", payload);
        if (rc != MQTT_CLIENT_OK) {
            App_MqttMarkDisconnected("goster pair publish failed", rc);
        }
    }
#elif APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_MQTT
    {
        char payload[40];
        snprintf(payload,
                 sizeof(payload),
                 "#%lu#%lu",
                 (unsigned long)first,
                 (unsigned long)second);
        (void) App_MqttPublishText(payload);
    }
#else
    (void) bemfa_wifi_publish_pair(APP_BEMFA_UID,
                                   APP_BEMFA_TOPIC,
                                   first,
                                   second);
#endif
}
#endif

static void App_OnSensorUpdate(const sensor_node_t *node,
                               const uint8_t *raw,
                               uint32_t raw_len,
                               void *context) {
    uint8_t nfc_contact;

    (void) raw;
    (void) raw_len;
    (void) context;

    if (node == NULL) {
        return;
    }

    nfc_contact = App_IsNfcContact(node);
    if (nfc_contact != 0u && app_nfc_contact_state == 0u) {
        app_nfc_event_seq++;
        if (app_nfc_event_seq == 0u) {
            app_nfc_event_seq = 1u;
        }
        app_nfc_event_sensor = *node;
        app_nfc_event_frame = sensor_last_frame;
        app_nfc_event_pending = 1u;
        AppUI_BeepShortTwice();
    }
    app_nfc_contact_state = nfc_contact;

    app_pending_sensor = *node;
    app_pending_sensor_frame = sensor_last_frame;
    app_sensor_has_value = 1u;
    app_sensor_ui_dirty = 1u;
    app_sensor_publish_dirty = 1u;
}

static void App_ProcessSensorTasks(uint32_t now_ms) {
    int rc;

    if (app_sensor_has_value == 0u) {
        return;
    }

    if (app_sensor_ui_dirty != 0u &&
        (app_sensor_last_ui_ms == 0u ||
         (uint32_t)(now_ms - app_sensor_last_ui_ms) >= APP_SENSOR_UI_INTERVAL_MS)) {
        AppUI_SetSensor(&app_pending_sensor);
        app_sensor_ui_dirty = 0u;
        app_sensor_last_ui_ms = now_ms;

        /*
         * LCD/SPI drawing and MQTT/WiFi publish are both comparatively slow.
         * Do only one slow sensor-side task per main-loop pass so the next
         * iteration can return to Sensor_Poll() before starting the publish.
         */
        return;
    }

    if (AppUI_IsPublishEnabled() == 0u) {
        return;
    }

#if APP_USES_MQTT
    if (app_mqtt_connected == 0u) {
        return;
    }
#endif

    if (app_nfc_event_pending != 0u) {
        rc = App_CloudPublishNfcEvent(&app_nfc_event_sensor,
                                      &app_nfc_event_frame,
                                      app_nfc_event_seq);
        if (rc == 0) {
            app_nfc_event_pending = 0u;
            app_sensor_last_publish_ms = now_ms;
            AppUI_NotePublish(1u, 1u);
        }
        return;
    }

    if (app_sensor_publish_dirty == 0u ||
        (app_sensor_last_publish_ms != 0u &&
         (uint32_t)(now_ms - app_sensor_last_publish_ms) < APP_SENSOR_PUBLISH_INTERVAL_MS)) {
        return;
    }

    app_sensor_last_publish_ms = now_ms;
    rc = App_CloudPublishSensor(&app_pending_sensor, &app_pending_sensor_frame);
    if (rc == 0) {
        app_sensor_publish_dirty = 0u;
        AppUI_NotePublish(app_pending_sensor.t_val, app_pending_sensor.h_val);
    }
}

static void App_InitHardware(void) {
    JTAG_SWD_Config();
    SysTick_Config(72000);
    LCD_Init();
    AppUI_Init();
    AppUI_BootBegin();
    App_BootStep("LCD ready");

    LED_Config();
    App_BootStep("LED ready");

    BEEP_Config();
    App_BootStep("BEEP ready");

    KEY_Config();
    App_BootStep("KEY ready");

    USART1_Config();
    App_BootStep("USART1 ready");

    USART3_Init();
    App_BootStep("USART3 ready");
}

#if APP_USES_MQTT
static void App_MqttFillConfig(mqtt_client_config_t *config,
                               mqtt_client_transport_t *transport) {
    memset(config, 0, sizeof(*config));
#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
    config->client_id = APP_GOSTER_DEVICE_UUID;
    config->username = App_MqttOptionalText(APP_GOSTER_MQTT_USERNAME);
    config->password = App_MqttOptionalText(APP_GOSTER_DEVICE_TOKEN);
#else
    config->client_id = APP_MQTT_CLIENT_ID;
    config->username = App_MqttOptionalText(APP_MQTT_USERNAME);
    config->password = App_MqttOptionalText(APP_MQTT_PASSWORD);
#endif
    config->keep_alive_sec = MQTT_CLIENT_DEFAULT_KEEPALIVE_SEC;
    config->clean_session = 1u;
    config->tx_buf = app_mqtt_tx_buf;
    config->tx_buf_size = sizeof(app_mqtt_tx_buf);
    config->rx_buf = app_mqtt_rx_buf;
    config->rx_buf_size = sizeof(app_mqtt_rx_buf);
    config->command_timeout_ms = 5000u;
    config->message_callback = App_MqttOnMessage;
    config->user_context = 0;

    *transport = mqtt_wifi_get_transport();
}

static int App_MqttSubscribeDownlink(void) {
    int rc;

#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
    {
        char downlink_topic[96];
        int topic_len;

        topic_len = goster_mqtt_format_topic(downlink_topic,
                                             sizeof(downlink_topic),
                                             APP_GOSTER_MQTT_BASE_TOPIC,
                                             APP_GOSTER_DEVICE_UUID,
                                             "downlink");
        if (topic_len < 0) {
            printf("goster downlink topic failed:%d\r\n", topic_len);
            return MQTT_CLIENT_ERR_PARAM;
        }
        rc = mqtt_client_subscribe(&app_mqtt, downlink_topic, APP_GOSTER_MQTT_QOS);
    }
#else
    rc = mqtt_client_subscribe(&app_mqtt, APP_MQTT_SUB_TOPIC, APP_MQTT_QOS);
#endif

    return rc;
}

static int App_MqttConnectStack(uint8_t reconnect_wifi, uint8_t show_boot_progress) {
    int rc;
    mqtt_client_config_t config;
    mqtt_client_transport_t transport;

    App_MqttMarkDisconnected(0, 0);

    if (reconnect_wifi != 0u) {
        rc = mqtt_wifi_connect_ap(APP_WIFI_SSID, APP_WIFI_PASSWORD, 5000u);
        if (rc != MQTT_WIFI_OK) {
            printf("wifi init failed:%d\r\n", rc);
            if (show_boot_progress != 0u) {
                App_BootFail("WiFi failed");
            }
            return rc;
        }
        if (show_boot_progress != 0u) {
            App_BootStep("WiFi ready");
        }
    }

    rc = mqtt_wifi_open_tcp_transparent(
#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
        APP_GOSTER_MQTT_HOST,
        APP_GOSTER_MQTT_PORT,
#else
        APP_MQTT_HOST,
        APP_MQTT_PORT,
#endif
        3000u);
    if (rc != MQTT_WIFI_OK) {
        printf("mqtt tcp failed:%d\r\n", rc);
        if (show_boot_progress != 0u) {
            App_BootFail("MQTT TCP fail");
        }
        return MQTT_CLIENT_ERR_TRANSPORT;
    }
    if (show_boot_progress != 0u) {
        App_BootStep("MQTT TCP");
    }

    App_MqttFillConfig(&config, &transport);
    rc = mqtt_client_init(&app_mqtt, &config, &transport);
    if (rc != MQTT_CLIENT_OK) {
        printf("mqtt init failed:%d\r\n", rc);
        if (show_boot_progress != 0u) {
            App_BootFail("MQTT init fail");
        }
        return rc;
    }
    if (show_boot_progress != 0u) {
        App_BootStep("MQTT init");
    }

    rc = mqtt_client_connect(&app_mqtt);
    if (rc != MQTT_CLIENT_OK) {
        printf("mqtt connect failed:%d\r\n", rc);
        if (show_boot_progress != 0u) {
            App_BootFail("MQTT conn fail");
        }
        return rc;
    }
    app_mqtt_connected = 1u;
    app_mqtt_last_ping_ms = Delay_GetMillis();
    AppUI_SetNetworkStatus(1u);
    if (show_boot_progress != 0u) {
        App_BootStep("MQTT conn");
    }

    rc = App_MqttSubscribeDownlink();
    if (rc != MQTT_CLIENT_OK) {
        printf("mqtt sub failed:%d\r\n", rc);
        App_MqttMarkDisconnected("mqtt sub failed", rc);
        if (show_boot_progress != 0u) {
            App_BootFail("MQTT sub fail");
        }
        return rc;
    }
    if (show_boot_progress != 0u) {
        App_BootStep("MQTT ready");
    }

#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
    app_goster_last_heartbeat_ms = Delay_GetMillis();
    rc = App_GosterPublishHeartbeat(app_goster_last_heartbeat_ms);
    if (rc != MQTT_CLIENT_OK) {
        App_MqttMarkDisconnected("goster heartbeat publish failed", rc);
        return rc;
    }
#endif

    printf("mqtt connected\r\n");
    return MQTT_CLIENT_OK;
}

static void App_MqttTryReconnect(uint32_t now_ms) {
    int rc;

    if ((uint32_t)(now_ms - app_mqtt_last_reconnect_ms) < APP_MQTT_RECONNECT_INTERVAL_MS) {
        return;
    }
    app_mqtt_last_reconnect_ms = now_ms;

    printf("mqtt reconnecting\r\n");
    rc = App_MqttConnectStack(0u, 0u);
    if (rc != MQTT_CLIENT_OK) {
        printf("mqtt reconnect tcp failed:%d\r\n", rc);
        rc = App_MqttConnectStack(1u, 0u);
    }
    if (rc != MQTT_CLIENT_OK) {
        printf("mqtt reconnect failed:%d\r\n", rc);
    }
}
#endif

static void App_InitNetwork(void) {
#if APP_USES_MQTT
    int rc;
#else
    uint8_t wifi_rc;
    int bemfa_rc;
#endif

    AppUI_SetNetworkStatus(0u);

#if APP_USES_MQTT
    rc = App_MqttConnectStack(1u, 1u);
    if (rc != MQTT_CLIENT_OK) {
        return;
    }
#else
    wifi_rc = WIFI_InitWithCredentials(APP_WIFI_SSID, APP_WIFI_PASSWORD);
    if (wifi_rc != 0u) {
        printf("wifi init failed:%u\r\n", (unsigned int) wifi_rc);
        App_BootFail("WiFi failed");
        return;
    }
    App_BootStep("WiFi ready");

    bemfa_rc = bemfa_wifi_open_stream(APP_BEMFA_HOST, APP_BEMFA_PORT, 2000u);
    if (bemfa_rc != BEMFA_OK) {
        printf("bemfa open failed:%d\r\n", bemfa_rc);
        App_BootFail("Bemfa TCP fail");
        return;
    }
    App_BootStep("Bemfa TCP");

    (void) bemfa_wifi_subscribe(APP_BEMFA_UID, APP_BEMFA_TOPIC);
    App_BootStep("Bemfa ready");
    AppUI_SetNetworkStatus(1u);
#endif
}

#if APP_USES_MQTT
static void App_MqttMaintain(uint32_t now_ms) {
    int rc;

    if (app_mqtt_connected == 0u) {
        App_MqttTryReconnect(now_ms);
        return;
    }

    rc = mqtt_client_poll(&app_mqtt, 1u);
    if (rc != MQTT_CLIENT_OK && rc != MQTT_CLIENT_ERR_TIMEOUT) {
        App_MqttMarkDisconnected("mqtt poll failed", rc);
        return;
    }

    if ((uint32_t)(now_ms - app_mqtt_last_ping_ms) > 30000u) {
        rc = mqtt_client_ping(&app_mqtt);
        if (rc != MQTT_CLIENT_OK) {
            App_MqttMarkDisconnected("mqtt ping failed", rc);
            return;
        }
        app_mqtt_last_ping_ms = now_ms;
    }

#if APP_CLOUD_BACKEND == APP_CLOUD_BACKEND_GOSTER_MQTT
    if ((uint32_t)(now_ms - app_goster_last_heartbeat_ms) > APP_GOSTER_HEARTBEAT_INTERVAL_MS) {
        rc = App_GosterPublishHeartbeat(now_ms);
        app_goster_last_heartbeat_ms = now_ms;
        if (rc != MQTT_CLIENT_OK) {
            App_MqttMarkDisconnected("goster heartbeat publish failed", rc);
            return;
        }
    }
#endif
}
#endif

int main(void) {
    uint8_t ui_ready = 0u;

    App_InitHardware();
    Sensor_SetUpdateCallback(App_OnSensorUpdate, 0);
    printf("system boot\r\n");
    App_InitNetwork();

    while (1) {
        uint8_t key_code;
        uint8_t poll_budget;
        uint32_t now_ms;

        if (ui_ready == 0u) {
            AppUI_ShowControlInterface();
            ui_ready = 1u;
        }

        for (poll_budget = 0u; poll_budget < APP_SENSOR_POLL_BUDGET; poll_budget++) {
            if (Sensor_Poll() == 0u) {
                break;
            }
        }

        now_ms = Delay_GetMillis();
        App_ProcessSensorTasks(now_ms);

        if (bafa_runtime > APP_TELEMETRY_INTERVAL_MS) {
#if APP_COUNTER_TELEMETRY_ENABLE
            if (AppUI_IsPublishEnabled() != 0u) {
                App_CloudPublishPair(telemetry_a, telemetry_b);
                AppUI_NotePublish(telemetry_a, telemetry_b);
            }
            telemetry_a++;
            telemetry_b++;
#endif
            bafa_runtime = 0;
        }

        if (AppUI_IsLed1BlinkEnabled() == 0u) {
            LED1_OFF;
            led_flag = 0u;
            led_runtime = 0u;
        } else if (led_runtime > APP_LED_BLINK_INTERVAL_MS) {
            if (led_flag == 0u) {
                LED1_ON;
            } else {
                LED1_OFF;
            }
            led_flag = (uint8_t) !led_flag;
            led_runtime = 0;
        }

#if APP_USES_MQTT
        App_MqttMaintain(now_ms);
#endif
        AppUI_Tick(now_ms);
        key_code = KEY_Sanf();
        if (key_code != 0u) {
            AppUI_HandleKey(key_code, Delay_GetMillis());
        }
    }
}
