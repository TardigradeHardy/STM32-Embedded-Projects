#ifndef __MASTER_TEST_H
#define __MASTER_TEST_H
#include "stm32f10x.h"

void MasterTest_Init(void);
void MasterTest_Run(void);          // 执行一轮测试
uint8_t MasterTest_GetResult(void); // 0=全部通过, 1=有失败
void MasterTest_ShowResult(void);   // 在OLED上显示结果

#endif