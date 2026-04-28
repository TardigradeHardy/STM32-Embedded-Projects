#ifndef __SERIAL_H
#define __SERIAL_H
#include "stm32f10x.h"
#include <stdio.h>

void Serial_Init(void);
void Serial_SendByte(uint8_t byte);
void Serial_SendString(uint8_t *str);
int fputc(int ch, FILE *f);

#endif