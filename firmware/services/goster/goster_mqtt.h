#ifndef GOSTER_MQTT_H
#define GOSTER_MQTT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GOSTER_MQTT_OK           0
#define GOSTER_MQTT_ERR_PARAM   -1
#define GOSTER_MQTT_ERR_BUFFER  -2

int goster_mqtt_format_topic(char *buf,
                             size_t buf_size,
                             const char *base_topic,
                             const char *device_uuid,
                             const char *kind);

int goster_mqtt_format_sensor_payload(char *buf,
                                      size_t buf_size,
                                      uint32_t temperature,
                                      uint32_t humidity,
                                      uint32_t mode,
                                      uint32_t sensor_type,
                                      uint32_t nwk_addr,
                                      uint32_t timestamp_ms,
                                      const char *token);

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
                                   const char *token);

int goster_mqtt_format_pair_payload(char *buf,
                                    size_t buf_size,
                                    uint32_t first,
                                    uint32_t second,
                                    uint32_t timestamp_ms,
                                    const char *token);

int goster_mqtt_format_heartbeat_payload(char *buf,
                                         size_t buf_size,
                                         uint32_t timestamp_ms,
                                         const char *token);

#ifdef __cplusplus
}
#endif

#endif /* GOSTER_MQTT_H */
