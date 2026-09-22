#include "sensor.h"

#include <stdio.h>
#include <string.h>

sensor_message_t sensor_message = {0};
sensor_node_t node_val = {0};
sensor_frame_info_t sensor_last_frame = {0};

typedef struct
{
    uint8_t data[SENSOR_MAX_RX_BUFF_LEN];
    uint16_t len;
} sensor_rx_frame_t;

static sensor_rx_frame_t sensor_frame_queue[SENSOR_FRAME_QUEUE_DEPTH];
static volatile uint8_t sensor_frame_head = 0u;
static volatile uint8_t sensor_frame_tail = 0u;
static volatile uint8_t sensor_frame_count = 0u;
static volatile uint32_t sensor_frame_drop_count = 0u;
static sensor_update_callback_t sensor_update_callback = 0;
static void *sensor_update_context = 0;

static uint8_t Sensor_Checksum(const uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint8_t sum = 0u;

    if (data == 0) {
        return 0u;
    }

    for (i = 0u; i < len; i++) {
        sum = (uint8_t)(sum + data[i]);
    }

    return sum;
}

void USART3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStruct = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    Sensor_ResetBuffer();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &USART_InitStruct);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
    USART_Cmd(USART3, ENABLE);
}

void USART3_SendByte(uint8_t data)
{
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
    {
    }
    USART_SendData(USART3, data);
}

void USART3_SendArray(const uint8_t *data, uint16_t len)
{
    uint16_t i;

    if (data == 0 && len != 0u)
    {
        return;
    }

    for (i = 0u; i < len; i++)
    {
        USART3_SendByte(data[i]);
    }
}

void USART3_IRQHandler(void)
{
    volatile uint8_t clear;
    uint8_t data;

    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
    {
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        data = (uint8_t)USART_ReceiveData(USART3);
        if (sensor_message.rx_count < SENSOR_MAX_RX_BUFF_LEN)
        {
            sensor_message.rx_buff[sensor_message.rx_count++] = data;
        }
        else
        {
            sensor_message.rx_count = 0;
        }
    }

    if (USART_GetITStatus(USART3, USART_IT_IDLE) == SET)
    {
        clear = (uint8_t)USART3->SR;
        clear = (uint8_t)USART3->DR;
        (void)clear;
        if (sensor_message.rx_count > 0u)
        {
            uint8_t head = sensor_frame_head;
            uint32_t len = sensor_message.rx_count;
            if (len > SENSOR_MAX_RX_BUFF_LEN)
            {
                len = SENSOR_MAX_RX_BUFF_LEN;
            }

            /*
             * The display and WiFi paths are much slower than USART3. If the
             * frame queue is full, keep the newest sample by discarding the
             * oldest queued frame instead of dropping the just-arrived frame.
             */
            if (sensor_frame_count >= SENSOR_FRAME_QUEUE_DEPTH)
            {
                sensor_frame_drop_count++;
                sensor_frame_tail = (uint8_t)((sensor_frame_tail + 1u) % SENSOR_FRAME_QUEUE_DEPTH);
                sensor_frame_count = (uint8_t)(SENSOR_FRAME_QUEUE_DEPTH - 1u);
            }

            memcpy(sensor_frame_queue[head].data, sensor_message.rx_buff, len);
            sensor_frame_queue[head].len = (uint16_t)len;
            sensor_frame_head = (uint8_t)((head + 1u) % SENSOR_FRAME_QUEUE_DEPTH);
            sensor_frame_count++;
            sensor_message.rx_over_flag = 1u;
            sensor_message.rx_count = 0u;
        }
    }
}

void Sensor_SetUpdateCallback(sensor_update_callback_t callback, void *context)
{
    sensor_update_callback = callback;
    sensor_update_context = context;
}

void Sensor_ResetBuffer(void)
{
    __disable_irq();
    memset(&sensor_message, 0, sizeof(sensor_message));
    memset(sensor_frame_queue, 0, sizeof(sensor_frame_queue));
    sensor_frame_head = 0u;
    sensor_frame_tail = 0u;
    sensor_frame_count = 0u;
    sensor_frame_drop_count = 0u;
    __enable_irq();
}

uint8_t Sensor_ParseBinaryFrame(const uint8_t *data,
                                uint32_t len,
                                sensor_node_t *node,
                                sensor_frame_info_t *info)
{
    sensor_frame_info_t parsed;

    if (data == 0 || node == 0 || len < SENSOR_FRAME_MIN_LEN)
    {
        return 0u;
    }

    memset(&parsed, 0, sizeof(parsed));

    if (data[0] == SENSOR_FRAME_MAGIC)
    {
        uint8_t payload_len;
        uint32_t total_len;
        const uint8_t *payload;
        uint8_t checksum;

        if (len < (uint32_t)(SENSOR_FRAME_HEADER_LEN + 1u))
        {
            return 0u;
        }

        payload_len = data[6];
        if (payload_len > SENSOR_MAX_PAYLOAD_LEN)
        {
            return 0u;
        }

        total_len = (uint32_t)SENSOR_FRAME_HEADER_LEN + payload_len + 1u;
        if (len < total_len)
        {
            return 0u;
        }

        checksum = Sensor_Checksum(data, total_len - 1u);
        if (checksum != data[total_len - 1u])
        {
            return 0u;
        }

        parsed.framed = 1u;
        parsed.version = data[1];
        parsed.type = data[2];
        parsed.seq = data[3];
        parsed.nwk_addr = (uint16_t)data[4] | ((uint16_t)data[5] << 8);
        parsed.payload_len = payload_len;
        parsed.checksum = checksum;

        payload = &data[SENSOR_FRAME_HEADER_LEN];
        memset(node, 0, sizeof(*node));

        if (parsed.type == (uint8_t)SENSOR_TYPE_ENV)
        {
            if (payload_len < 2u)
            {
                return 0u;
            }
            node->t_val = payload[0];
            node->h_val = payload[1];
            node->mode = (payload_len > 2u) ? payload[2] : 0u;
        }
        else
        {
            node->t_val = (payload_len > 0u) ? payload[0] : 0u;
            node->h_val = (payload_len > 1u) ? payload[1] : 0u;
            node->mode = (payload_len > 2u) ? payload[2] : parsed.type;
        }

        if (info != 0)
        {
            *info = parsed;
        }
        return 1u;
    }

    /* Raw structure passthrough used by the course examples. */
    memset(node, 0, sizeof(*node));
    if (len == 2u)
    {
        node->t_val = data[0];
        node->h_val = data[1];
    }
    else if (len == 3u)
    {
        node->t_val = data[0];
        node->h_val = data[1];
        node->mode = data[2];
    }
    else if (len == 4u)
    {
        parsed.nwk_addr = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
        node->t_val = data[2];
        node->h_val = data[3];
    }
    else if (len == 5u)
    {
        parsed.nwk_addr = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
        node->t_val = data[2];
        node->h_val = data[3];
        node->mode = data[4];
    }
    else
    {
        /* Exact compatibility with the original STM32 code:
         * node_val = *(struct tagSensor_Nodex *)rx_buff;
         * Some teaching examples leave trailing bytes in the UART buffer.
         */
        node->t_val = data[0];
        node->h_val = data[1];
        node->mode = data[2];
    }

    parsed.framed = 0u;
    parsed.version = 0u;
    parsed.type = (uint8_t)SENSOR_TYPE_ENV;
    parsed.seq = 0u;
    parsed.payload_len = (len > 2u && len < 255u) ? (uint8_t)(len - 2u) : (uint8_t)len;

    if (info != 0)
    {
        *info = parsed;
    }

    return 1u;
}

uint8_t Sensor_SendFrame(uint8_t type,
                         uint16_t nwk_addr,
                         const uint8_t *payload,
                         uint8_t payload_len)
{
    static uint8_t tx_seq = 0u;
    uint8_t buf[SENSOR_FRAME_HEADER_LEN + SENSOR_MAX_PAYLOAD_LEN + 1u];
    uint32_t total_len;

    if (payload_len > SENSOR_MAX_PAYLOAD_LEN || (payload == 0 && payload_len != 0u))
    {
        return 0u;
    }

    buf[0] = SENSOR_FRAME_MAGIC;
    buf[1] = SENSOR_FRAME_VERSION;
    buf[2] = type;
    buf[3] = tx_seq++;
    buf[4] = (uint8_t)(nwk_addr & 0xFFu);
    buf[5] = (uint8_t)(nwk_addr >> 8);
    buf[6] = payload_len;
    if (payload_len > 0u)
    {
        memcpy(&buf[SENSOR_FRAME_HEADER_LEN], payload, payload_len);
    }

    total_len = (uint32_t)SENSOR_FRAME_HEADER_LEN + payload_len + 1u;
    buf[total_len - 1u] = Sensor_Checksum(buf, total_len - 1u);
    USART3_SendArray(buf, (uint16_t)total_len);

    return 1u;
}

uint8_t Sensor_SendNodeRaw(const sensor_node_t *node)
{
    if (node == 0)
    {
        return 0u;
    }

    USART3_SendArray((const uint8_t *)node, (uint16_t)sizeof(*node));
    return 1u;
}

uint8_t Sensor_Poll(void)
{
    static uint8_t frame[SENSOR_MAX_RX_BUFF_LEN];
    uint32_t frame_len;
    sensor_node_t parsed_node;
    sensor_frame_info_t parsed_info;

    if (sensor_message.rx_over_flag == 0u)
    {
        return 0;
    }

    __disable_irq();
    if (sensor_frame_count == 0u)
    {
        sensor_message.rx_over_flag = 0u;
        __enable_irq();
        return 0u;
    }

    frame_len = sensor_frame_queue[sensor_frame_tail].len;
    if (frame_len > SENSOR_MAX_RX_BUFF_LEN)
    {
        frame_len = SENSOR_MAX_RX_BUFF_LEN;
    }
    memcpy(frame, sensor_frame_queue[sensor_frame_tail].data, frame_len);
    sensor_frame_tail = (uint8_t)((sensor_frame_tail + 1u) % SENSOR_FRAME_QUEUE_DEPTH);
    sensor_frame_count--;
    sensor_message.rx_over_flag = (sensor_frame_count > 0u) ? 1u : 0u;
    __enable_irq();

    if (Sensor_ParseBinaryFrame(frame, frame_len, &parsed_node, &parsed_info) == 0u)
    {
#if SENSOR_DEBUG_LOG
        printf("sensor invalid frame len:%lu\r\n", (unsigned long)frame_len);
#endif
        return 0;
    }

    node_val = parsed_node;
    sensor_last_frame = parsed_info;

#if SENSOR_DEBUG_LOG
    printf("sensor t:%u h:%u mode:%u type:%u addr:%04X len:%u drop:%lu\r\n",
           (unsigned int)node_val.t_val,
           (unsigned int)node_val.h_val,
           (unsigned int)node_val.mode,
           (unsigned int)sensor_last_frame.type,
           (unsigned int)sensor_last_frame.nwk_addr,
           (unsigned int)sensor_last_frame.payload_len,
           (unsigned long)sensor_frame_drop_count);
#endif

    if (sensor_update_callback != 0)
    {
        sensor_update_callback(&node_val, frame, frame_len, sensor_update_context);
    }

    return 1;
}

void USART3_Updata(void)
{
    (void)Sensor_Poll();
}
