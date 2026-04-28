#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "AD.h"
#include "Key.h"
#include "LED.h"

uint16_t AD_POT, AD_LDR, AD_NTC;
float V_POT, V_LDR, V_NTC;
float LDR_Threshold = 1.8;
float NTC_Base = 1.35;  
uint8_t Last_LDR = 0;

int main(void)
{
    OLED_Init();
    AD_Init();
    Key_Init();
    LED_Init();
    
    // 关闭 JTAG，保留 SWD，解决 PB4 常亮问题（可选）
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    
    LED1_Off();   // 初始灭（新逻辑：常温灭）
    LED2_On();    // 光敏初始亮
    
    OLED_ShowString(1, 1, "Env Monitor");
    
    while(1)
    {
        AD_POT = AD_GetValue(ADC_Channel_0);
        AD_LDR = AD_GetValue(ADC_Channel_1);
        AD_NTC = AD_GetValue(ADC_Channel_2);
        
        V_POT = (float)AD_POT / 4095 * 3.3;
        V_LDR = (float)AD_LDR / 4095 * 3.3;
        V_NTC = (float)AD_NTC / 4095 * 3.3;
        
        // ---- 双按键调节 ----
        if(Key_Scan(KEY1_PIN))   // PA8 调光敏阈值
        {
            LDR_Threshold += 0.1f;
            if(LDR_Threshold > 3.0f) LDR_Threshold = 0.5f;
        }
        if(Key_Scan(KEY2_PIN))   // PA10 调热敏基准
        {
            NTC_Base += 0.1f;
            if(NTC_Base > 2.0f) NTC_Base = 1.0f;
        }
        
        // ---- 光敏 + LED2 ----
        uint8_t Now_LDR = (V_LDR > LDR_Threshold) ? 1 : 0;
        if(Now_LDR != Last_LDR)
        {
            Last_LDR = Now_LDR;
            if(Now_LDR)
            {
                OLED_ShowString(2, 1, "LDR: Dark  ");
                LED2_On();
            }
            else
            {
                OLED_ShowString(2, 1, "LDR: Light ");
                LED2_Off();
            }
        }
        OLED_ShowNum(2, 11, (uint16_t)LDR_Threshold, 1);
        OLED_ShowString(2, 12, ".");
        OLED_ShowNum(2, 13, (uint16_t)(LDR_Threshold*10)%10, 1);
        
        // ---- 热敏 + LED1（逻辑反转：常温灭，加热亮） ----
       static uint8_t NTC_Status = 1;
		if(V_NTC > NTC_Base + 0.03f){
			NTC_Status = 1;             // 电压高 = 常温
		}else if(V_NTC < NTC_Base - 0.03f){
			NTC_Status = 0;             // 电压低 = 加热
		}

		if(NTC_Status == 1){
			OLED_ShowString(3, 1, "NTC: Hot   ");   // 常温显示Hot
			LED1_Off();                              // 常温灭灯
		}else{
			OLED_ShowString(3, 1, "NTC: Norm  ");   // 加热显示Norm
			LED1_On();                               // 加热亮灯
		}
        OLED_ShowNum(3, 11, (uint16_t)NTC_Base, 1);
        OLED_ShowString(3, 12, ".");
        OLED_ShowNum(3, 13, (uint16_t)(NTC_Base*10)%10, 1);
        
        // ---- 电位器电压 ----
        OLED_ShowString(4, 1, "V:");
        OLED_ShowNum(4, 3, (uint16_t)V_POT, 1);
        OLED_ShowString(4, 4, ".");
        OLED_ShowNum(4, 5, (uint16_t)(V_POT*100)%100, 2);
        
        Delay_ms(100);
    }
}