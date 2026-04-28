#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "RS485.h"
#include "Modbus.h"
#include "Modbus_Data.h"
#include "Timer.h"

// 外部变量声明（定义在 Modbus.c 中）
extern uint8_t  modbus_frame_ready;
extern uint8_t  modbus_rx_buf[];
extern uint8_t  modbus_rx_len;

// 外部变量声明（定义在 Timer.c 中）
extern volatile uint16_t timer_tick;
extern volatile uint8_t  rx_buf[256];
extern volatile uint8_t  rx_index;
extern volatile uint16_t last_rx_time;

int main(void)
{
    OLED_Init();
    AD_Init();
    LED_Init();
    RS485_Init(9600);
    Timer_Init();

    OLED_ShowString(1, 1, "Modbus RTU");
    OLED_ShowString(2, 1, "Addr:1 9600");

    while(1)
    {
        // 实时更新传感器数据到Modbus寄存器
        Modbus_UpdateInputRegs();

        // 如果收到完整Modbus帧，自动解析并应答
        if(modbus_frame_ready)
        {
            LED1_On();
            Modbus_Parse();
            LED1_Off();
        }

        // 本地显示前两路传感器数据（毫伏）
        OLED_ShowString(3, 1, "P:");
        OLED_ShowNum(3, 3, Modbus_GetInputReg(0), 4);
        OLED_ShowString(4, 1, "L:");
        OLED_ShowNum(4, 3, Modbus_GetInputReg(1), 4);

        Delay_ms(200);
    }
}

// USART3 接收中断：逐字节存入临时缓冲区
void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        rx_buf[rx_index++] = USART_ReceiveData(USART3);
        if(rx_index >= MODBUS_RX_BUF_SIZE) rx_index = 0;
        last_rx_time = timer_tick;
    }
}