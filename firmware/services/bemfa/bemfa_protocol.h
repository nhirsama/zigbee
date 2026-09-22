#ifndef BEMFA_PROTOCOL_H
#define BEMFA_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BEMFA_OK                  0
#define BEMFA_ERR_PARAM         -1
#define BEMFA_ERR_BUFFER        -2
#define BEMFA_ERR_SOCKET        -3
#define BEMFA_ERR_TRANSPARENT   -4

int bemfa_format_subscribe(char *buf,
                           size_t buf_size,
                           const char *uid,
                           const char *topic);

int bemfa_format_publish(char *buf,
                         size_t buf_size,
                         const char *uid,
                         const char *topic,
                         const char *message);

int bemfa_format_publish_pair(char *buf,
                              size_t buf_size,
                              const char *uid,
                              const char *topic,
                              uint32_t first,
                              uint32_t second);

int bemfa_format_publish_sensor(char *buf,
                                size_t buf_size,
                                const char *uid,
                                const char *topic,
                                int temperature,
                                int humidity,
                                int mode);

#ifdef __cplusplus
}
#endif

#endif /* BEMFA_PROTOCOL_H */
