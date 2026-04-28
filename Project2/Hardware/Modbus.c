#include "Modbus.h"
#include "Modbus_Data.h"
#include "RS485.h"

uint8_t modbus_rx_buf[MODBUS_RX_BUF_SIZE];
uint8_t modbus_rx_len = 0;
uint8_t modbus_frame_ready = 0;

uint16_t Modbus_CRC16(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for(uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(uint8_t j = 0; j < 8; j++)
        {
            if(crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

void Modbus_Parse(void)
{
    uint8_t *frame = modbus_rx_buf;
    uint8_t  len   = modbus_rx_len;

    // 1. 最小帧长度检查（地址+功能码+CRC ≥ 4字节）
    if(len < 4) { modbus_frame_ready = 0; return; }

    // 2. 站号检查
    if(frame[0] != MODBUS_SLAVE_ID) { modbus_frame_ready = 0; return; }

    // 3. CRC16 校验
    uint16_t crc_recv = frame[len-2] | (frame[len-1] << 8);
    uint16_t crc_calc = Modbus_CRC16(frame, len-2);
    if(crc_recv != crc_calc) { modbus_frame_ready = 0; return; }

    // 4. 功能码分发
    switch(frame[1])
    {
        case 0x04: Modbus_Reply04(frame); break;
        default:   break;
    }

    modbus_frame_ready = 0;
    modbus_rx_len = 0;
}

void Modbus_Reply04(uint8_t *request)
{
    uint16_t start_addr = (request[2] << 8) | request[3];
    uint16_t quantity   = (request[4] << 8) | request[5];

    if(quantity < 1 || quantity > 10) return;

    uint8_t reply[64];
    uint8_t reply_len = 0;

    reply[reply_len++] = MODBUS_SLAVE_ID;
    reply[reply_len++] = 0x04;
    reply[reply_len++] = quantity * 2;

    for(uint16_t i = 0; i < quantity; i++)
    {
        uint16_t val = Modbus_GetInputReg(start_addr + i);
        reply[reply_len++] = (val >> 8) & 0xFF;
        reply[reply_len++] =  val       & 0xFF;
    }

    uint16_t crc = Modbus_CRC16(reply, reply_len);
    reply[reply_len++] = crc & 0xFF;
    reply[reply_len++] = (crc >> 8) & 0xFF;

    RS485_SendBytes(reply, reply_len);
}