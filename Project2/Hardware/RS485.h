#ifndef __RS485_H
#define __RS485_H
#include "stm32f10x.h"

void RS485_Init(uint32_t baudrate);
void RS485_SendByte(uint8_t byte);
void RS485_SendBytes(uint8_t *buf, uint16_t len);

#endif