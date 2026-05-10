#include "Modbus_Data.h"
#include "AD.h"

static uint16_t input_regs[10] = {0};

// 保持寄存器（可读写参数）
static uint16_t holding_regs[10] = {
    1,    // [0] 从站地址
    0,    // [1] 波特率索引：0=9600
    180,  // [2] LDR阈值*100（默认1.80V）
    135,  // [3] NTC基准*100（默认1.35V）
    0,0,0,0,0,0
};

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

uint16_t Modbus_GetHoldingReg(uint16_t addr)
{
    if(addr < 10) return holding_regs[addr];
    return 0;
}

void Modbus_SetHoldingReg(uint16_t addr, uint16_t val)
{
    if(addr < 10) holding_regs[addr] = val;
}