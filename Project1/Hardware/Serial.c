#include "Serial.h"

/**
  * 函数：串口初始化（USART1，PA9-TX、PA10-RX）
  * 参数：无
  * 返回：无
  */
void Serial_Init(void)
{
    // 1. 开启时钟（USART1 + GPIOA）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	
    // 2. 配置TX引脚（PA9 - 复用推挽输出）、RX引脚（PA10 - 浮空输入）
    GPIO_InitTypeDef GPIO_InitStructure;
    // TX引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // RX引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    // 3. 配置USART参数（波特率115200，8N1）
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200; // 固定波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // 收发模式
    USART_Init(USART1, &USART_InitStructure);
	
    // 4. 使能USART1
    USART_Cmd(USART1, ENABLE);
}

/**
  * 函数：发送单个字节
  */
void Serial_SendByte(uint8_t byte)
{
    USART_SendData(USART1, byte);
    // 等待发送完成（避免乱码）
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

/**
  * 函数：发送字符串
  */
void Serial_SendString(uint8_t *str)
{
    uint16_t i = 0;
    while(str[i] != '\0')
    {
        Serial_SendByte(str[i]);
        i++;
    }
}

/**
  * 函数：printf重定向（让printf通过串口发送）
  */
int fputc(int ch, FILE *f)
{
    Serial_SendByte(ch);
    return ch;
}