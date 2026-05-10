#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "AD.h"
#include "Key.h"
#include "LED.h"
#include "log.h"
#include "filter.h"

uint16_t AD_POT, AD_LDR, AD_NTC;
float V_POT, V_LDR, V_NTC;
float LDR_Threshold = 1.8;
float NTC_Base = 1.35;
uint8_t Last_LDR = 0;       // 光敏上一次状态（0=Light, 1=Dark）

int main(void)
{
    // ===== 初始化阶段 =====
    OLED_Init();
    AD_Init();
    Key_Init();
    LED_Init();
    Log_Init();
    Filter_Init();          // 会预采样填满窗口

    Log_SetLevel(LOG_INFO); // 默认INFO等级，调试时可改LOG_DEBUG

    // 关闭JTAG，保留SWD（避免PB4受影响）
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    LED1_Off();             // 常温灭
    LED2_On();              // 光敏初始亮

    Log(LOG_INFO, "Env Monitor started");
    Log(LOG_INFO, "LDR_TH=%.1fV NTC_BASE=%.1fV", LDR_Threshold, NTC_Base);

    // ① 启动自检日志：预采样完成后立即读一次三个通道
    Log(LOG_INFO, "Self-test: CH0=%d CH1=%d CH2=%d",
        AD_GetValue_Filtered(ADC_Channel_0),
        AD_GetValue_Filtered(ADC_Channel_1),
        AD_GetValue_Filtered(ADC_Channel_2));

    OLED_ShowString(1, 1, "Env Monitor");

    // ② 使能独立看门狗（超时2秒，避免OLED初始化等长耗时误复位）
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);   // 40kHz / 64 = 625Hz
    IWDG_SetReload(1250);                   // 1250 / 625 = 2秒
    IWDG_ReloadCounter();                   // 喂一次狗
    IWDG_Enable();
    Log(LOG_INFO, "IWDG enabled, timeout 2s");

    // ===== 主循环 =====
    while(1)
    {
        Log_Tick(100);  // 更新时间戳，100ms周期

        // 滤波后的ADC值
        AD_POT = AD_GetValue_Filtered(ADC_Channel_0);
        AD_LDR = AD_GetValue_Filtered(ADC_Channel_1);
        AD_NTC = AD_GetValue_Filtered(ADC_Channel_2);

        // DEBUG等级输出原始值（默认不显示，改日志等级为LOG_DEBUG可见）
        Log(LOG_DEBUG, "ADC RAW: %d %d %d",
            AD_GetValue(ADC_Channel_0),
            AD_GetValue(ADC_Channel_1),
            AD_GetValue(ADC_Channel_2));
        Log(LOG_DEBUG, "ADC FILTERED: %d %d %d", AD_POT, AD_LDR, AD_NTC);

        V_POT = (float)AD_POT / 4095 * 3.3;
        V_LDR = (float)AD_LDR / 4095 * 3.3;
        V_NTC = (float)AD_NTC / 4095 * 3.3;

        // ---- 按键调节 ----
        if(Key_Scan(KEY1_PIN))
        {
            LDR_Threshold += 0.1f;
            if(LDR_Threshold > 3.0f) LDR_Threshold = 0.5f;
            Log(LOG_INFO, "LDR threshold set to %.1fV", LDR_Threshold);
        }
        if(Key_Scan(KEY2_PIN))
        {
            NTC_Base += 0.1f;
            if(NTC_Base > 2.0f) NTC_Base = 1.0f;
            Log(LOG_INFO, "NTC base set to %.1fV", NTC_Base);
        }

        // ---- 光敏 + LED2 ----
        uint8_t Now_LDR = (V_LDR > LDR_Threshold) ? 1 : 0;
        if(Now_LDR != Last_LDR)     // 变化触发，避免OLED重复刷新闪烁
        {
            Last_LDR = Now_LDR;
            if(Now_LDR)
            {
                OLED_ShowString(2, 1, "LDR: Dark  ");
                LED2_On();
                Log(LOG_WARN, "LDR DARK: %.2fV > %.2fV", V_LDR, LDR_Threshold);
            }
            else
            {
                OLED_ShowString(2, 1, "LDR: Light ");
                LED2_Off();
                Log(LOG_INFO, "LDR normal: %.2fV", V_LDR);
            }
        }
        OLED_ShowNum(2, 11, (uint16_t)LDR_Threshold, 1);
        OLED_ShowString(2, 12, ".");
        OLED_ShowNum(2, 13, (uint16_t)(LDR_Threshold*10)%10, 1);

        // ---- 热敏 + LED1 ----
        // ③ 用static变量实现滞回状态保持，消除临界抖动
        static uint8_t NTC_Status = 1;   // 1=常温(Hot), 0=加热(Norm)
        if(V_NTC > NTC_Base + 0.03f){
            if(NTC_Status != 1) {
                NTC_Status = 1;
                Log(LOG_INFO, "NTC normal: %.2fV", V_NTC);
            }
        }else if(V_NTC < NTC_Base - 0.03f){
            if(NTC_Status != 0) {
                NTC_Status = 0;
                Log(LOG_WARN, "NTC HEATING: %.2fV < %.2fV", V_NTC, NTC_Base);
            }
        }

        if(NTC_Status == 1){
            OLED_ShowString(3, 1, "NTC: Hot   ");
            LED1_Off();
        }else{
            OLED_ShowString(3, 1, "NTC: Norm  ");
            LED1_On();
        }
        OLED_ShowNum(3, 11, (uint16_t)NTC_Base, 1);
        OLED_ShowString(3, 12, ".");
        OLED_ShowNum(3, 13, (uint16_t)(NTC_Base*10)%10, 1);

        // ---- 电位器电压 ----
        OLED_ShowString(4, 1, "V:");
        OLED_ShowNum(4, 3, (uint16_t)V_POT, 1);
        OLED_ShowString(4, 4, ".");
        OLED_ShowNum(4, 5, (uint16_t)(V_POT*100)%100, 2);

        // 喂狗，确保主循环正常运行时不会复位
        IWDG_ReloadCounter();
        Delay_ms(100);
    }
}