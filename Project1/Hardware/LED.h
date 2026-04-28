#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

// 引脚定义
#define LED1_Pin   GPIO_Pin_14
#define LED2_Pin   GPIO_Pin_12
#define LED_GPIO   GPIOB

// 函数声明
void LED_Init(void);
void LED1_On(void);
void LED1_Off(void);
void LED2_On(void);
void LED2_Off(void);

#endif