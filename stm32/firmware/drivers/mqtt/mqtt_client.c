#include "mqtt_client.h"

#include <stddef.h>
#include <string.h>

#include "MQTTPacket.h"

static mqtt_client_t *s_read_client = 0;
static uint32_t s_read_timeout_ms = 0;

static int mqtt_paho_read_fn(unsigned char *buf, int len)
{
    int rc;

    if (s_read_client == 0 || buf == 0 || len <= 0 || len > 65535)
    {
        return -1;
    }

    rc = s_read_client->transport.recv((uint8_t *)buf, (uint16_t)len, s_read_timeout_ms);
    return (rc == len) ? rc : -1;
}

static uint32_t mqtt_client_timeout(const mqtt_client_t *client)
{
    if (client->config.command_timeout_ms == 0)
    {
        return MQTT_CLIENT_DEFAULT_TIMEOUT_MS;
    }
    return client->config.command_timeout_ms;
}

static uint16_t mqtt_client_next_packet_id(mqtt_client_t *client)
{
    uint16_t packet_id = client->next_packet_id;

    if (packet_id == 0)
    {
        packet_id = MQTT_CLIENT_MIN_PACKET_ID;
    }

    client->next_packet_id = (uint16_t)(packet_id + 1u);
    if (client->next_packet_id == 0)
    {
        client->next_packet_id = MQTT_CLIENT_MIN_PACKET_ID;
    }

    return packet_id;
}

static int mqtt_client_send_packet(mqtt_client_t *client, int len)
{
    int rc;

    if (len <= 0 || len > client->config.tx_buf_size)
    {
        return MQTT_CLIENT_ERR_BUFFER;
    }

    rc = client->transport.send(client->config.tx_buf, (uint16_t)len, mqtt_client_timeout(client));
    if (rc != len)
    {
        return MQTT_CLIENT_ERR_TRANSPORT;
    }

    return MQTT_CLIENT_OK;
}

static int mqtt_client_read_packet(mqtt_client_t *client, uint32_t timeout_ms)
{
    int packet_type;

    if (timeout_ms == 0)
    {
        timeout_ms = mqtt_client_timeout(client);
    }

    s_read_client = client;
    s_read_timeout_ms = timeout_ms;
    packet_type = MQTTPacket_read(client->config.rx_buf,
                                  client->config.rx_buf_size,
                                  mqtt_paho_read_fn);
    s_read_client = 0;
    s_read_timeout_ms = 0;

    if (packet_type <= 0)
    {
        return MQTT_CLIENT_ERR_TIMEOUT;
    }

    return packet_type;
}

static int mqtt_client_process_publish_packet(mqtt_client_t *client)
{
    unsigned char dup = 0;
    int qos = 0;
    unsigned char retained = 0;
    unsigned short packet_id = 0;
    MQTTString topic_name = MQTTString_initializer;
    unsigned char *payload = 0;
    int payload_len = 0;

    if (MQTTDeserialize_publish(&dup,
                                &qos,
                                &retained,
                                &packet_id,
                                &topic_name,
                                &payload,
                                &payload_len,
                                client->config.rx_buf,
                                client->config.rx_buf_size) != 1)
    {
        return MQTT_CLIENT_ERR_PROTOCOL;
    }

    if (client->config.message_callback != 0)
    {
        const char *topic_ptr = topic_name.cstring;
        uint16_t topic_len;

        if (topic_ptr != 0)
        {
            topic_len = (uint16_t)strlen(topic_ptr);
        }
        else
        {
            topic_ptr = topic_name.lenstring.data;
            topic_len = (uint16_t)topic_name.lenstring.len;
        }

        client->config.message_callback(topic_ptr,
                                        topic_len,
                                        payload,
                                        (uint16_t)payload_len,
                                        (uint8_t)qos,
                                        retained,
                                        client->config.user_context);
    }

    if (qos == 1)
    {
        int len = MQTTSerialize_puback(client->config.tx_buf,
                                       client->config.tx_buf_size,
                                       packet_id);
        return mqtt_client_send_packet(client, len);
    }
    if (qos > 1)
    {
        return MQTT_CLIENT_ERR_UNSUPPORTED;
    }

    return MQTT_CLIENT_OK;
}

static int mqtt_client_wait_for_packet(mqtt_client_t *client, int expected_type, uint32_t timeout_ms)
{
    int packet_type;
    int rc;

    for (;;)
    {
        packet_type = mqtt_client_read_packet(client, timeout_ms);
        if (packet_type < 0)
        {
            return packet_type;
        }
        if (packet_type == expected_type)
        {
            return packet_type;
        }
        if (packet_type == PUBLISH)
        {
            rc = mqtt_client_process_publish_packet(client);
            if (rc != MQTT_CLIENT_OK)
            {
                return rc;
            }
            continue;
        }

        return MQTT_CLIENT_ERR_PROTOCOL;
    }
}

int mqtt_client_init(mqtt_client_t *client,
                     const mqtt_client_config_t *config,
                     const mqtt_client_transport_t *transport)
{
    if (client == 0 || config == 0 || transport == 0 ||
        transport->send == 0 || transport->recv == 0 ||
        config->client_id == 0 || config->tx_buf == 0 || config->rx_buf == 0 ||
        config->tx_buf_size == 0 || config->rx_buf_size == 0)
    {
        return MQTT_CLIENT_ERR_PARAM;
    }

    memset(client, 0, sizeof(*client));
    client->config = *config;
    client->transport = *transport;
    client->next_packet_id = MQTT_CLIENT_MIN_PACKET_ID;
    client->connected = 0;

    if (client->config.keep_alive_sec == 0)
    {
        client->config.keep_alive_sec = MQTT_CLIENT_DEFAULT_KEEPALIVE_SEC;
    }
    if (client->config.command_timeout_ms == 0)
    {
        client->config.command_timeout_ms = MQTT_CLIENT_DEFAULT_TIMEOUT_MS;
    }

    return MQTT_CLIENT_OK;
}

int mqtt_client_connect(mqtt_client_t *client)
{
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    unsigned char session_present = 0;
    unsigned char connack_rc = 0;
    int len;
    int packet_type;

    if (client == 0)
    {
        return MQTT_CLIENT_ERR_PARAM;
    }

    data.MQTTVersion = 4; /* MQTT 3.1.1 */
    data.clientID.cstring = (char *)client->config.client_id;
    data.keepAliveInterval = client->config.keep_alive_sec;
    data.cleansession = client->config.clean_session ? 1u : 0u;

    if (client->config.username != 0)
    {
        data.username.cstring = (char *)client->config.username;
    }
    if (client->config.password != 0)
    {
        data.password.cstring = (char *)client->config.password;
    }

    len = MQTTSerialize_connect(client->config.tx_buf,
                                client->config.tx_buf_size,
                                &data);
    if (mqtt_client_send_packet(client, len) != MQTT_CLIENT_OK)
    {
        return MQTT_CLIENT_ERR_TRANSPORT;
    }

    packet_type = mqtt_client_wait_for_packet(client, CONNACK, mqtt_client_timeout(client));
    if (packet_type != CONNACK)
    {
        return packet_type < 0 ? packet_type : MQTT_CLIENT_ERR_PROTOCOL;
    }

    if (MQTTDeserialize_connack(&session_present,
                                &connack_rc,
                                client->config.rx_buf,
                                client->config.rx_buf_size) != 1)
    {
        return MQTT_CLIENT_ERR_PROTOCOL;
    }

    if (connack_rc != MQTT_CONNECTION_ACCEPTED)
    {
        return MQTT_CLIENT_ERR_CONNACK;
    }

    client->connected = 1;
    return MQTT_CLIENT_OK;
}

int mqtt_client_disconnect(mqtt_client_t *client)
{
    int len;
    int rc;

    if (client == 0)
    {
        return MQTT_CLIENT_ERR_PARAM;
    }

    len = MQTTSerialize_disconnect(client->config.tx_buf, client->config.tx_buf_size);
    rc = mqtt_client_send_packet(client, len);
    if (rc == MQTT_CLIENT_OK)
    {
        client->connected = 0;
    }

    return rc;
}

int mqtt_client_ping(mqtt_client_t *client)
{
    int len;
    int packet_type;
    int rc;

    if (client == 0)
    {
        return MQTT_CLIENT_ERR_PARAM;
    }

    len = MQTTSerialize_pingreq(client->config.tx_buf, client->config.tx_buf_size);
    rc = mqtt_client_send_packet(client, len);
    if (rc != MQTT_CLIENT_OK)
    {
        return rc;
    }

    packet_type = mqtt_client_wait_for_packet(client, PINGRESP, mqtt_client_timeout(client));
    if (packet_type != PINGRESP)
    {
        return packet_type < 0 ? packet_type : MQTT_CLIENT_ERR_PROTOCOL;
    }

    return MQTT_CLIENT_OK;
}

int mqtt_client_publish(mqtt_client_t *client,
                        const char *topic,
                        const uint8_t *payload,
                        uint16_t payload_len,
                        uint8_t qos,
                        uint8_t retained)
{
    MQTTString topic_name = MQTTString_initializer;
    unsigned short packet_id = 0;
    int len;
    int rc;

    if (client == 0 || topic == 0 || (payload == 0 && payload_len != 0))
    {
        return MQTT_CLIENT_ERR_PARAM;
    }
    if (qos > 1)
    {
        return MQTT_CLIENT_ERR_UNSUPPORTED;
    }

    topic_name.cstring = (char *)topic;
    if (qos > 0)
    {
        packet_id = mqtt_client_next_packet_id(client);
    }

    len = MQTTSerialize_publish(client->config.tx_buf,
                                client->config.tx_buf_size,
                                0,
                                qos,
                                retained ? 1u : 0u,
                                packet_id,
                                topic_name,
                                (unsigned char *)payload,
                                payload_len);

    rc = mqtt_client_send_packet(client, len);
    if (rc != MQTT_CLIENT_OK || qos == 0)
    {
        return rc;
    }

    rc = mqtt_client_wait_for_packet(client, PUBACK, mqtt_client_timeout(client));
    if (rc != PUBACK)
    {
        return rc < 0 ? rc : MQTT_CLIENT_ERR_PROTOCOL;
    }

    {
        unsigned char ack_type = 0;
        unsigned char dup = 0;
        unsigned short ack_packet_id = 0;

        if (MQTTDeserialize_ack(&ack_type,
                                &dup,
                                &ack_packet_id,
                                client->config.rx_buf,
                                client->config.rx_buf_size) != 1 ||
            ack_type != PUBACK || ack_packet_id != packet_id)
        {
            return MQTT_CLIENT_ERR_PROTOCOL;
        }
    }

    return MQTT_CLIENT_OK;
}

int mqtt_client_publish_string(mqtt_client_t *client,
                               const char *topic,
                               const char *payload,
                               uint8_t qos,
                               uint8_t retained)
{
    if (payload == 0)
    {
        payload = "";
    }

    return mqtt_client_publish(client,
                               topic,
                               (const uint8_t *)payload,
                               (uint16_t)strlen(payload),
                               qos,
                               retained);
}

int mqtt_client_subscribe(mqtt_client_t *client, const char *topic_filter, uint8_t qos)
{
    MQTTString topic_filters[1] = { MQTTString_initializer };
    int requested_qos[1];
    int granted_qos[1] = { 0 };
    int granted_count = 0;
    unsigned short packet_id;
    unsigned short suback_packet_id = 0;
    int len;
    int rc;

    if (client == 0 || topic_filter == 0)
    {
        return MQTT_CLIENT_ERR_PARAM;
    }
    if (qos > 1)
    {
        return MQTT_CLIENT_ERR_UNSUPPORTED;
    }

    packet_id = mqtt_client_next_packet_id(client);
    topic_filters[0].cstring = (char *)topic_filter;
    requested_qos[0] = qos;

    len = MQTTSerialize_subscribe(client->config.tx_buf,
                                  client->config.tx_buf_size,
                                  0,
                                  packet_id,
                                  1,
                                  topic_filters,
                                  requested_qos);
    rc = mqtt_client_send_packet(client, len);
    if (rc != MQTT_CLIENT_OK)
    {
        return rc;
    }

    rc = mqtt_client_wait_for_packet(client, SUBACK, mqtt_client_timeout(client));
    if (rc != SUBACK)
    {
        return rc < 0 ? rc : MQTT_CLIENT_ERR_PROTOCOL;
    }

    if (MQTTDeserialize_suback(&suback_packet_id,
                               1,
                               &granted_count,
                               granted_qos,
                               client->config.rx_buf,
                               client->config.rx_buf_size) != 1 ||
        suback_packet_id != packet_id || granted_count != 1 || granted_qos[0] == 0x80)
    {
        return MQTT_CLIENT_ERR_PROTOCOL;
    }

    return MQTT_CLIENT_OK;
}

int mqtt_client_unsubscribe(mqtt_client_t *client, const char *topic_filter)
{
    MQTTString topic_filters[1] = { MQTTString_initializer };
    unsigned short packet_id;
    unsigned short unsuback_packet_id = 0;
    int len;
    int rc;

    if (client == 0 || topic_filter == 0)
    {
        return MQTT_CLIENT_ERR_PARAM;
    }

    packet_id = mqtt_client_next_packet_id(client);
    topic_filters[0].cstring = (char *)topic_filter;

    len = MQTTSerialize_unsubscribe(client->config.tx_buf,
                                    client->config.tx_buf_size,
                                    0,
                                    packet_id,
                                    1,
                                    topic_filters);
    rc = mqtt_client_send_packet(client, len);
    if (rc != MQTT_CLIENT_OK)
    {
        return rc;
    }

    rc = mqtt_client_wait_for_packet(client, UNSUBACK, mqtt_client_timeout(client));
    if (rc != UNSUBACK)
    {
        return rc < 0 ? rc : MQTT_CLIENT_ERR_PROTOCOL;
    }

    if (MQTTDeserialize_unsuback(&unsuback_packet_id,
                                 client->config.rx_buf,
                                 client->config.rx_buf_size) != 1 ||
        unsuback_packet_id != packet_id)
    {
        return MQTT_CLIENT_ERR_PROTOCOL;
    }

    return MQTT_CLIENT_OK;
}

int mqtt_client_poll(mqtt_client_t *client, uint32_t timeout_ms)
{
    int packet_type;

    if (client == 0)
    {
        return MQTT_CLIENT_ERR_PARAM;
    }

    packet_type = mqtt_client_read_packet(client, timeout_ms);
    if (packet_type < 0)
    {
        return packet_type;
    }

    if (packet_type == PUBLISH)
    {
        return mqtt_client_process_publish_packet(client);
    }

    return MQTT_CLIENT_OK;
}

const char *mqtt_client_strerror(int code)
{
    switch (code)
    {
        case MQTT_CLIENT_OK: return "ok";
        case MQTT_CLIENT_ERR_PARAM: return "bad parameter";
        case MQTT_CLIENT_ERR_BUFFER: return "buffer too short";
        case MQTT_CLIENT_ERR_TRANSPORT: return "transport error";
        case MQTT_CLIENT_ERR_TIMEOUT: return "timeout";
        case MQTT_CLIENT_ERR_PROTOCOL: return "protocol error";
        case MQTT_CLIENT_ERR_CONNACK: return "broker refused connection";
        case MQTT_CLIENT_ERR_UNSUPPORTED: return "unsupported feature";
        default: return "unknown";
    }
}
