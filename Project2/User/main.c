#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "RS485.h"
#include "Modbus.h"
#include "Modbus_Data.h"
#include "Timer.h"
#include "master_test.h"

extern uint8_t  modbus_frame_ready;
extern uint8_t  modbus_rx_buf[];
extern uint8_t  modbus_rx_len;

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
    MasterTest_Init();

    OLED_ShowString(1, 1, "Modbus RTU v2.0");
    OLED_ShowString(2, 1, "Addr:1 9600bps");

    // 上电自动执行主站自检
    MasterTest_Run();
    MasterTest_ShowResult();
    Delay_ms(2000);

    // 切换到从站运行界面
    OLED_Clear();
    OLED_ShowString(1, 1, "Modbus Slave");
    OLED_ShowString(2, 1, "Waiting...");

    while(1)
    {
        Modbus_UpdateInputRegs();

        if(modbus_frame_ready)
        {
            LED1_On();
            Modbus_Parse();
            LED1_Off();
        }

        OLED_ShowString(3, 1, "P:");
        OLED_ShowNum(3, 3, Modbus_GetInputReg(0), 4);
        OLED_ShowString(3, 8, "mV");
        OLED_ShowString(4, 1, "L:");
        OLED_ShowNum(4, 3, Modbus_GetInputReg(1), 4);
        OLED_ShowString(4, 8, "mV");

        Delay_ms(200);
    }
}

void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        rx_buf[rx_index++] = USART_ReceiveData(USART3);
        if(rx_index >= MODBUS_RX_BUF_SIZE) rx_index = 0;
        last_rx_time = timer_tick;
    }
}