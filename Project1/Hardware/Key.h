#ifndef __KEY_H
#define __KEY_H
#include "stm32f10x.h"
#include "Delay.h"

#define KEY1_PIN  GPIO_Pin_8
#define KEY2_PIN  GPIO_Pin_10
#define KEY_PORT  GPIOA

void Key_Init(void);
uint8_t Key_Scan(uint16_t pin);

#endif