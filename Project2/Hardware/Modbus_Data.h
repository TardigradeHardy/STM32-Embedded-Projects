#ifndef __MODBUS_DATA_H
#define __MODBUS_DATA_H
#include "stm32f10x.h"

void     Modbus_UpdateInputRegs(void);
uint16_t Modbus_GetInputReg(uint16_t addr);

// 保持寄存器接口（0x03读，0x06/0x10写）
uint16_t Modbus_GetHoldingReg(uint16_t addr);
void     Modbus_SetHoldingReg(uint16_t addr, uint16_t val);

#endif