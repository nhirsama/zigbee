#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <stdint.h>

#include "stm32f10x.h"

#ifdef __cplusplus
extern "C" {
#endif

#define int8 char
#define uint8 uint8_t
#define uint16 uint16_t

#define SENSOR_MAX_RX_BUFF_LEN 256u
#define SENSOR_MAX_PAYLOAD_LEN 64u

#ifndef SENSOR_FRAME_QUEUE_DEPTH
/*
 * Full UART frames are queued from the USART3 IDLE interrupt and consumed by
 * the main loop. Keep this deeper than one frame so LCD/SPI rendering or
 * WiFi/MQTT writes do not overwrite freshly received sensor data.
 */
#define SENSOR_FRAME_QUEUE_DEPTH 8u
#endif

#if SENSOR_FRAME_QUEUE_DEPTH == 0
#error "SENSOR_FRAME_QUEUE_DEPTH must be greater than 0"
#endif

#ifndef SENSOR_DEBUG_LOG
#define SENSOR_DEBUG_LOG 0u
#endif

/* USART3 gateway link:
 * - physical layer: 9600 8N1, no hardware flow control
 * - application layer: project-defined binary structure passthrough
 *
 * Legacy frames are also accepted for course examples:
 *   [t_val, h_val]
 *   [t_val, h_val, mode]
 *   [nwk_addr_lo, nwk_addr_hi, t_val, h_val]
 *   [nwk_addr_lo, nwk_addr_hi, t_val, h_val, mode]
 */
#define SENSOR_FRAME_MAGIC 0xA5u
#define SENSOR_FRAME_VERSION 0x01u
#define SENSOR_FRAME_HEADER_LEN 7u
#define SENSOR_FRAME_MIN_LEN 2u

typedef enum
{
    SENSOR_TYPE_ENV = 0x01u,
    SENSOR_TYPE_DOOR = 0x02u,
    SENSOR_TYPE_LIGHT = 0x03u,
    SENSOR_TYPE_CURTAIN = 0x04u,
    SENSOR_TYPE_ALARM = 0x05u
} sensor_payload_type_t;

typedef struct tagSensor_Nodex
{
    uint8_t t_val;
    uint8_t h_val;
    uint8_t mode;
} sensor_node_t;

extern sensor_node_t node_val;

typedef struct
{
    uint8_t framed;
    uint8_t version;
    uint8_t type;
    uint8_t seq;
    uint16_t nwk_addr;
    uint8_t payload_len;
    uint8_t checksum;
} sensor_frame_info_t;

extern sensor_frame_info_t sensor_last_frame;

typedef struct SENSOR_MESSAGE
{
    uint8_t rx_buff[SENSOR_MAX_RX_BUFF_LEN];
    uint32_t rx_count;
    uint8_t rx_over_flag;
} sensor_message_t;

extern sensor_message_t sensor_message;

typedef void (*sensor_update_callback_t)(const sensor_node_t *node,
                                         const uint8_t *raw,
                                         uint32_t raw_len,
                                         void *context);

void USART3_Init(void);
void USART3_SendByte(uint8_t data);
void USART3_SendArray(const uint8_t *data, uint16_t len);
void Sensor_SetUpdateCallback(sensor_update_callback_t callback, void *context);
void Sensor_ResetBuffer(void);
uint8_t Sensor_ParseBinaryFrame(const uint8_t *data,
                                uint32_t len,
                                sensor_node_t *node,
                                sensor_frame_info_t *info);
uint8_t Sensor_SendFrame(uint8_t type,
                         uint16_t nwk_addr,
                         const uint8_t *payload,
                         uint8_t payload_len);
uint8_t Sensor_SendNodeRaw(const sensor_node_t *node);
uint8_t Sensor_Poll(void);

/* Backward-compatible name kept for older app code. */
void USART3_Updata(void);

#ifdef __cplusplus
}
#endif

#endif /* _SENSOR_H_ */
