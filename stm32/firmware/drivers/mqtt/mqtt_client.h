#ifndef _MQTT_CLIENT_H_
#define _MQTT_CLIENT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MQTT_CLIENT_DEFAULT_TIMEOUT_MS      5000u
#define MQTT_CLIENT_DEFAULT_KEEPALIVE_SEC   60u
#define MQTT_CLIENT_MIN_PACKET_ID           1u

typedef enum
{
    MQTT_CLIENT_OK = 0,
    MQTT_CLIENT_ERR_PARAM = -1,
    MQTT_CLIENT_ERR_BUFFER = -2,
    MQTT_CLIENT_ERR_TRANSPORT = -3,
    MQTT_CLIENT_ERR_TIMEOUT = -4,
    MQTT_CLIENT_ERR_PROTOCOL = -5,
    MQTT_CLIENT_ERR_CONNACK = -6,
    MQTT_CLIENT_ERR_UNSUPPORTED = -7
} mqtt_client_result_t;

typedef int (*mqtt_client_send_fn)(const uint8_t *data, uint16_t len, uint32_t timeout_ms);
typedef int (*mqtt_client_recv_fn)(uint8_t *data, uint16_t len, uint32_t timeout_ms);

typedef struct
{
    mqtt_client_send_fn send;
    mqtt_client_recv_fn recv;
} mqtt_client_transport_t;

typedef void (*mqtt_client_message_callback_t)(const char *topic,
                                                uint16_t topic_len,
                                                const uint8_t *payload,
                                                uint16_t payload_len,
                                                uint8_t qos,
                                                uint8_t retained,
                                                void *user_context);

typedef struct
{
    const char *client_id;
    const char *username;
    const char *password;
    uint16_t keep_alive_sec;
    uint8_t clean_session;

    uint8_t *tx_buf;
    uint16_t tx_buf_size;
    uint8_t *rx_buf;
    uint16_t rx_buf_size;

    uint32_t command_timeout_ms;
    mqtt_client_message_callback_t message_callback;
    void *user_context;
} mqtt_client_config_t;

typedef struct
{
    mqtt_client_config_t config;
    mqtt_client_transport_t transport;
    uint16_t next_packet_id;
    uint8_t connected;
} mqtt_client_t;

int mqtt_client_init(mqtt_client_t *client,
                     const mqtt_client_config_t *config,
                     const mqtt_client_transport_t *transport);

int mqtt_client_connect(mqtt_client_t *client);
int mqtt_client_disconnect(mqtt_client_t *client);
int mqtt_client_ping(mqtt_client_t *client);

int mqtt_client_publish(mqtt_client_t *client,
                        const char *topic,
                        const uint8_t *payload,
                        uint16_t payload_len,
                        uint8_t qos,
                        uint8_t retained);

int mqtt_client_publish_string(mqtt_client_t *client,
                               const char *topic,
                               const char *payload,
                               uint8_t qos,
                               uint8_t retained);

int mqtt_client_subscribe(mqtt_client_t *client, const char *topic_filter, uint8_t qos);
int mqtt_client_unsubscribe(mqtt_client_t *client, const char *topic_filter);

/*
 * Read and handle one MQTT packet:
 * - PUBLISH calls message_callback.
 * - QoS1 PUBLISH is acknowledged with PUBACK.
 * - No data before timeout returns MQTT_CLIENT_ERR_TIMEOUT.
 */
int mqtt_client_poll(mqtt_client_t *client, uint32_t timeout_ms);

const char *mqtt_client_strerror(int code);

#ifdef __cplusplus
}
#endif

#endif /* _MQTT_CLIENT_H_ */
