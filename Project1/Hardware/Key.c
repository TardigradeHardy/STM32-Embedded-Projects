#include "Key.h"

void Key_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;   // 下拉输入，按键接VCC
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);
}

uint8_t Key_Scan(uint16_t pin)
{
    if(GPIO_ReadInputDataBit(KEY_PORT, pin) == 1)
    {
        Delay_ms(20);
        if(GPIO_ReadInputDataBit(KEY_PORT, pin) == 1)
        {
            while(GPIO_ReadInputDataBit(KEY_PORT, pin) == 1); // 等待释放
            return 1;
        }
    }
    return 0;
}