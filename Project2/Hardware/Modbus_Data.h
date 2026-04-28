#ifndef __MODBUS_DATA_H
#define __MODBUS_DATA_H
#include "stm32f10x.h"

void     Modbus_UpdateInputRegs(void);
uint16_t Modbus_GetInputReg(uint16_t addr);

#endif