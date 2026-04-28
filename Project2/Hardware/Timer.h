#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f10x.h"

void Timer_Init(void);
extern volatile uint16_t timer_tick;

#endif