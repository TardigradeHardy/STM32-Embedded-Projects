#ifndef __MODBUS_H
#define __MODBUS_H
#include "stm32f10x.h"

#define MODBUS_SLAVE_ID    1
#define MODBUS_RX_BUF_SIZE 256

extern uint8_t  modbus_rx_buf[MODBUS_RX_BUF_SIZE];
extern uint8_t  modbus_rx_len;
extern uint8_t  modbus_frame_ready;

uint16_t Modbus_CRC16(uint8_t *data, uint16_t len);
void     Modbus_Parse(void);
void     Modbus_Reply04(uint8_t *request);

#endif