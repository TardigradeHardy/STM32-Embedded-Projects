#include "RS485.h"

void RS485_Init(uint32_t baudrate)
{
    // 开启 USART3 和 GPIOB 时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef g;
    g.GPIO_Speed = GPIO_Speed_50MHz;

    // PB10: TX 复用推挽输出
    g.GPIO_Pin = GPIO_Pin_10;
    g.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &g);

    // PB11: RX 浮空输入
    g.GPIO_Pin = GPIO_Pin_11;
    g.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &g);

    // USART3 配置
    USART_InitTypeDef u;
    u.USART_BaudRate = baudrate;
    u.USART_WordLength = USART_WordLength_8b;
    u.USART_StopBits = USART_StopBits_1;
    u.USART_Parity = USART_Parity_No;
    u.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    u.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &u);

    // 开启接收中断
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    NVIC_InitTypeDef n;
    n.NVIC_IRQChannel = USART3_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = 1;
    n.NVIC_IRQChannelSubPriority = 1;
    n.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&n);

    USART_Cmd(USART3, ENABLE);
}

void RS485_SendByte(uint8_t byte)
{
    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, byte);
}

void RS485_SendBytes(uint8_t *buf, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        RS485_SendByte(buf[i]);
    }
    while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}