#ifndef BEMFA_WIFI_H
#define BEMFA_WIFI_H

#include <stdint.h>

#include "bemfa_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

int bemfa_wifi_open_stream(const char *host, uint16_t port, uint32_t timeout_ms);
int bemfa_wifi_subscribe(const char *uid, const char *topic);
int bemfa_wifi_publish(const char *uid, const char *topic, const char *message);
int bemfa_wifi_publish_pair(const char *uid, const char *topic, uint32_t first, uint32_t second);
int bemfa_wifi_publish_sensor(const char *uid,
                              const char *topic,
                              int temperature,
                              int humidity,
                              int mode);

#ifdef __cplusplus
}
#endif

#endif /* BEMFA_WIFI_H */
