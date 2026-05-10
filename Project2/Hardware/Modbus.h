#ifndef __MODBUS_H
#define __MODBUS_H
#include "stm32f10x.h"

#define MODBUS_SLAVE_ID    1
#define MODBUS_RX_BUF_SIZE 256

extern uint8_t  modbus_rx_buf[MODBUS_RX_BUF_SIZE];
extern uint8_t  modbus_rx_len;
extern uint8_t  modbus_frame_ready;

// 应答缓冲区（供主站测试模块读取）
extern uint8_t  modbus_reply_buf[256];
extern uint8_t  modbus_reply_len;

uint16_t Modbus_CRC16(uint8_t *data, uint16_t len);
void     Modbus_Parse(void);
uint8_t  Modbus_GetReplyByte(uint8_t idx);
void     Modbus_ErrorReply(uint8_t *request, uint8_t exception_code);

// 功能码处理函数
void     Modbus_Reply03(uint8_t *request);
void     Modbus_Reply04(uint8_t *request);
void     Modbus_Reply06(uint8_t *request);
void     Modbus_Reply10(uint8_t *request);

#endif