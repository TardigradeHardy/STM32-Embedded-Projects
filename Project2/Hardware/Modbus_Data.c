#include "Modbus_Data.h"
#include "AD.h"

static uint16_t input_regs[10] = {0};

void Modbus_UpdateInputRegs(void)
{
    uint16_t ad0 = AD_GetValue(ADC_Channel_0);
    uint16_t ad1 = AD_GetValue(ADC_Channel_1);
    uint16_t ad2 = AD_GetValue(ADC_Channel_2);

    input_regs[0] = (uint16_t)((float)ad0 / 4095.0f * 3.3f * 1000);
    input_regs[1] = (uint16_t)((float)ad1 / 4095.0f * 3.3f * 1000);
    input_regs[2] = (uint16_t)((float)ad2 / 4095.0f * 3.3f * 1000);
}

uint16_t Modbus_GetInputReg(uint16_t addr)
{
    if(addr < 10) return input_regs[addr];
    return 0;
}