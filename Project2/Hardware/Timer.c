#include "Timer.h"
#include "Modbus.h"

volatile uint16_t timer_tick = 0;

// 接收临时缓冲区（定义在本文件，main.c 用 extern 引用）
volatile uint8_t  rx_buf[256];
volatile uint8_t  rx_index = 0;
volatile uint16_t last_rx_time = 0;

// 从 Modbus.c 引入
extern uint8_t  modbus_rx_buf[];
extern uint8_t  modbus_rx_len;
extern uint8_t  modbus_frame_ready;

void Timer_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef t;
    t.TIM_Prescaler = 72 - 1;
    t.TIM_Period = 1000 - 1;
    t.TIM_ClockDivision = TIM_CKD_DIV1;
    t.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &t);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef n;
    n.NVIC_IRQChannel = TIM2_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = 0;
    n.NVIC_IRQChannelSubPriority = 0;
    n.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&n);

    TIM_Cmd(TIM2, ENABLE);
}

// TIM2 中断：每1ms加1，并判断帧结束（3.5字符 ≈ 4ms）
void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        timer_tick++;

        if(rx_index > 0 && (timer_tick - last_rx_time) > 4)
        {
            for(uint8_t i = 0; i < rx_index; i++)
                modbus_rx_buf[i] = rx_buf[i];
            modbus_rx_len = rx_index;
            modbus_frame_ready = 1;
            rx_index = 0;
        }
    }
}