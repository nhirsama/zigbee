#ifndef _WIFI_H_
#define _WIFI_H_

#include <stdint.h>
#include "stm32f10x.h"

#ifdef __cplusplus
extern "C" {
#endif

#define uint8 uint8_t
#define WIFI_RX_BUFFER_SIZE 1024u
#define WIFI_DEFAULT_BAUDRATE 115200u

#ifndef MY_SSID
#define MY_SSID ""
#endif

#ifndef MY_PASSWORD
#define MY_PASSWORD ""
#endif

typedef struct
{
    u8 RX_buff[WIFI_RX_BUFFER_SIZE];
    u16 RX_count;
    vu8 WIFI_RecFlag;
} WIFI_DataTypedef;

extern WIFI_DataTypedef WIFI_Data;

void WIFI_Init(void);
uint8_t WIFI_InitWithCredentials(const char *ssid, const char *password);
void usart2_config(uint32_t brr);

void wifi_SendData(uint8_t data);
void wifi_SendStr(const char *p);
void wifi_SendArray(const uint8_t *arr, uint16_t data_length);
uint16_t wifi_ReadBytes(uint8_t *buf, uint16_t len, uint32_t timeout_ms);

char *FindStr(char *dest, const char *src, uint32_t timeout);
void Exit_Scotch(void);
uint8_t SendCmd_RecAck(const char *cmd, const char *ack, uint32_t timeout, uint8_t check_cnt);
uint8_t STA_ModeConnect(const char *ssid, const char *password);
uint8_t WIFI_OpenTcpTransparent(const char *host, uint16_t port, uint32_t timeout_ms);
void WIFI_DataStruct_Clear(void);

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_H_ */
