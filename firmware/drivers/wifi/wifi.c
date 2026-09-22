#include "wifi.h"

#include <stdio.h>
#include <string.h>

#include "delay.h"

WIFI_DataTypedef WIFI_Data = {0};

void usart2_config(uint32_t brr)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    USART_InitTypeDef USART_InitStruct = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitStruct.USART_BaudRate = brr;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStruct);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

void WIFI_Init(void)
{
    (void)WIFI_InitWithCredentials(MY_SSID, MY_PASSWORD);
}

uint8_t WIFI_InitWithCredentials(const char *ssid, const char *password)
{
    if (ssid == NULL || password == NULL || ssid[0] == '\0')
    {
        return 1;
    }

    usart2_config(WIFI_DEFAULT_BAUDRATE);
    Delay_ms(1000);
    return STA_ModeConnect(ssid, password);
}

void wifi_SendData(uint8_t data)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
    {
    }
    USART_SendData(USART2, data);
}

void wifi_SendStr(const char *p)
{
    if (p == NULL)
    {
        return;
    }

    while (*p != '\0')
    {
        wifi_SendData((uint8_t)*p);
        p++;
    }
}

void wifi_SendArray(const uint8_t *arr, uint16_t data_length)
{
    uint16_t i = 0;

    if (arr == NULL && data_length != 0u)
    {
        return;
    }

    while (data_length--)
    {
        wifi_SendData(arr[i++]);
    }
}

uint16_t wifi_ReadBytes(uint8_t *buf, uint16_t len, uint32_t timeout_ms)
{
    uint16_t copied = 0;
    uint32_t waited = 0;

    if (buf == NULL || len == 0u)
    {
        return 0;
    }

    while (copied < len)
    {
        uint16_t available;
        uint16_t take;

        __disable_irq();
        available = WIFI_Data.RX_count;
        if (available > 0u)
        {
            take = (uint16_t)(len - copied);
            if (take > available)
            {
                take = available;
            }

            memcpy(buf + copied, WIFI_Data.RX_buff, take);
            if (available > take)
            {
                memmove(WIFI_Data.RX_buff, WIFI_Data.RX_buff + take, available - take);
            }
            WIFI_Data.RX_count = (uint16_t)(available - take);
            if (WIFI_Data.RX_count == 0u)
            {
                WIFI_Data.WIFI_RecFlag = 0;
            }
            else if (WIFI_Data.RX_count < sizeof(WIFI_Data.RX_buff))
            {
                WIFI_Data.RX_buff[WIFI_Data.RX_count] = 0;
            }
        }
        else
        {
            take = 0;
        }
        __enable_irq();

        if (take > 0u)
        {
            copied = (uint16_t)(copied + take);
            continue;
        }

        if (timeout_ms == 0u || waited >= timeout_ms)
        {
            break;
        }

        Delay_ms(1);
        waited++;
    }

    return copied;
}

void USART2_IRQHandler(void)
{
    uint8_t data;

    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        data = (uint8_t)USART_ReceiveData(USART2);
        USART1->DR = data;

        if (WIFI_Data.RX_count < sizeof(WIFI_Data.RX_buff))
        {
            WIFI_Data.RX_buff[WIFI_Data.RX_count++] = data;
            if (WIFI_Data.RX_count < sizeof(WIFI_Data.RX_buff))
            {
                WIFI_Data.RX_buff[WIFI_Data.RX_count] = 0;
            }
        }
        else
        {
            WIFI_Data.RX_count = 0;
        }
    }

    if (USART_GetITStatus(USART2, USART_IT_IDLE) == SET)
    {
        (void)USART2->SR;
        (void)USART2->DR;
        WIFI_Data.WIFI_RecFlag = 1;
    }
}

char *FindStr(char *dest, const char *src, uint32_t timeout)
{
    if (dest == NULL || src == NULL)
    {
        return NULL;
    }

    while (timeout-- && (strstr(dest, src) == NULL))
    {
        Delay_ms(1);
    }
    return strstr(dest, src);
}

uint8_t SendCmd_RecAck(const char *cmd, const char *ack, uint32_t timeout, uint8_t check_cnt)
{
    uint32_t len = (cmd != NULL) ? strlen(cmd) : 0u;

    WIFI_DataStruct_Clear();
    do
    {
        if (cmd != NULL)
        {
            wifi_SendArray((const uint8_t *)cmd, (uint16_t)len);
            wifi_SendStr("\r\n");
        }

        if (ack == NULL)
        {
            return 0;
        }

        if (FindStr((char *)WIFI_Data.RX_buff, ack, timeout) != NULL)
        {
            return 0;
        }
    } while (check_cnt--);

    return 1;
}

uint8_t STA_ModeConnect(const char *ssid, const char *password)
{
    char sendarr[256] = {0};

    if (ssid == NULL || password == NULL)
    {
        return 1;
    }

    Exit_Scotch();
    if (SendCmd_RecAck("AT+RST", "OK", 8000, 3) != 0u)
    {
        printf("wifi reset failed\r\n");
        return 1;
    }

    printf("wifi reset ok\r\n");
    Delay_ms(8000);

    if (SendCmd_RecAck("AT+WMODE=1,1", "OK", 500, 2) != 0u)
    {
        printf("wifi STA mode failed\r\n");
        return 3;
    }

    printf("wifi STA mode ok\r\n");
    sprintf(sendarr, "AT+WJAP=\"%s\",\"%s\"", ssid, password);
    if (SendCmd_RecAck(sendarr, "+EVENT:WIFI_GOT_IP", 10000, 2) != 0u)
    {
        printf("wifi join AP failed\r\n");
        return 4;
    }

    printf("wifi join AP ok\r\n");
    return 0;
}

uint8_t WIFI_OpenTcpTransparent(const char *host, uint16_t port, uint32_t timeout_ms)
{
    char sendarr[160] = {0};

    if (host == NULL || host[0] == '\0' || port == 0u)
    {
        return 1;
    }

    if (timeout_ms == 0u)
    {
        timeout_ms = 2000u;
    }

    Exit_Scotch();
    WIFI_DataStruct_Clear();

    sprintf(sendarr, "AT+SOCKET=4,\"%s\",%u", host, (unsigned int)port);
    if (SendCmd_RecAck(sendarr, "OK", timeout_ms, 2) != 0u)
    {
        printf("tcp socket open failed\r\n");
        return 2;
    }

    if (SendCmd_RecAck("AT+SOCKETTT", ">", timeout_ms, 2) != 0u)
    {
        printf("tcp transparent mode failed\r\n");
        return 3;
    }

    WIFI_DataStruct_Clear();
    return 0;
}

void Exit_Scotch(void)
{
    wifi_SendStr("+++\r\n");
    Delay_ms(500);
}

void WIFI_DataStruct_Clear(void)
{
    memset(&WIFI_Data, 0, sizeof(WIFI_Data));
}
