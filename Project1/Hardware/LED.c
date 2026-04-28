#include "LED.h"

// 双LED初始化：推挽输出 + 默认高电平(上电亮)
void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = LED1_Pin | LED2_Pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_GPIO, &GPIO_InitStruct);
	
	// 默认高电平 → 上电两个灯都亮
	GPIO_SetBits(LED_GPIO, LED1_Pin | LED2_Pin);
}

// LED1(PB14) 亮
void LED1_On(void)
{
	GPIO_SetBits(LED_GPIO, LED1_Pin);
}

// LED1(PB14) 灭
void LED1_Off(void)
{
	GPIO_ResetBits(LED_GPIO, LED1_Pin);
}

// LED2(PB12) 亮
void LED2_On(void)
{
	GPIO_SetBits(LED_GPIO, LED2_Pin);
}

// LED2(PB12) 灭
void LED2_Off(void)
{
	GPIO_ResetBits(LED_GPIO, LED2_Pin);
}