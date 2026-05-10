#include "Modbus.h"
#include "Modbus_Data.h"
#include "RS485.h"

uint8_t modbus_rx_buf[MODBUS_RX_BUF_SIZE];
uint8_t modbus_rx_len = 0;
uint8_t modbus_frame_ready = 0;

uint8_t modbus_reply_buf[256];
uint8_t modbus_reply_len = 0;

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

uint8_t Modbus_GetReplyByte(uint8_t idx)
{
    if(idx < modbus_reply_len)
        return modbus_reply_buf[idx];
    return 0;
}

void Modbus_ErrorReply(uint8_t *request, uint8_t exception_code)
{
    uint8_t reply[5];
    reply[0] = request[0];
    reply[1] = request[1] | 0x80;
    reply[2] = exception_code;
    uint16_t crc = Modbus_CRC16(reply, 3);
    reply[3] = crc & 0xFF;
    reply[4] = (crc >> 8) & 0xFF;

    RS485_SendBytes(reply, 5);

    for(uint8_t i = 0; i < 5; i++)
        modbus_reply_buf[i] = reply[i];
    modbus_reply_len = 5;
}

void Modbus_Parse(void)
{
    uint8_t *frame = modbus_rx_buf;
    uint8_t  len   = modbus_rx_len;

    if(len < 4) { modbus_frame_ready = 0; return; }

    if(frame[0] != MODBUS_SLAVE_ID) { modbus_frame_ready = 0; return; }

    uint16_t crc_recv = frame[len-2] | (frame[len-1] << 8);
    uint16_t crc_calc = Modbus_CRC16(frame, len-2);
    if(crc_recv != crc_calc) { modbus_frame_ready = 0; return; }

    uint8_t func = frame[1];

    // 请求数量检查
    uint16_t quantity = 0;
    if(func == 0x03 || func == 0x04)
    {
        quantity = (frame[4] << 8) | frame[5];
        if(quantity < 1 || quantity > 10)
        {
            Modbus_ErrorReply(frame, 0x03);
            modbus_frame_ready = 0;
            modbus_rx_len = 0;
            return;
        }
    }

    // 地址检查
    uint16_t start_addr = 0;
    if(func == 0x03 || func == 0x04)
    {
        start_addr = (frame[2] << 8) | frame[3];
        if(start_addr + quantity > 10)
        {
            Modbus_ErrorReply(frame, 0x02);
            modbus_frame_ready = 0;
            modbus_rx_len = 0;
            return;
        }
    }

    switch(func)
    {
        case 0x03: Modbus_Reply03(frame); break;
        case 0x04: Modbus_Reply04(frame); break;
        case 0x06: Modbus_Reply06(frame); break;
        case 0x10: Modbus_Reply10(frame); break;
        default:   Modbus_ErrorReply(frame, 0x01); break;
    }

    modbus_frame_ready = 0;
    modbus_rx_len = 0;
}

void Modbus_Reply03(uint8_t *request)
{
    uint16_t start_addr = (request[2] << 8) | request[3];
    uint16_t quantity   = (request[4] << 8) | request[5];

    uint8_t reply[64];
    uint8_t reply_len = 0;

    reply[reply_len++] = MODBUS_SLAVE_ID;
    reply[reply_len++] = 0x03;
    reply[reply_len++] = quantity * 2;

    for(uint16_t i = 0; i < quantity; i++)
    {
        uint16_t val = Modbus_GetHoldingReg(start_addr + i);
        reply[reply_len++] = (val >> 8) & 0xFF;
        reply[reply_len++] =  val       & 0xFF;
    }

    uint16_t crc = Modbus_CRC16(reply, reply_len);
    reply[reply_len++] = crc & 0xFF;
    reply[reply_len++] = (crc >> 8) & 0xFF;

    RS485_SendBytes(reply, reply_len);

    for(uint8_t i = 0; i < reply_len; i++)
        modbus_reply_buf[i] = reply[i];
    modbus_reply_len = reply_len;
}

void Modbus_Reply04(uint8_t *request)
{
    uint16_t start_addr = (request[2] << 8) | request[3];
    uint16_t quantity   = (request[4] << 8) | request[5];

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

    for(uint8_t i = 0; i < reply_len; i++)
        modbus_reply_buf[i] = reply[i];
    modbus_reply_len = reply_len;
}

void Modbus_Reply06(uint8_t *request)
{
    uint16_t addr = (request[2] << 8) | request[3];
    uint16_t val  = (request[4] << 8) | request[5];

    Modbus_SetHoldingReg(addr, val);

    uint16_t crc = Modbus_CRC16(request, 6);
    uint8_t reply[8];
    for(uint8_t i = 0; i < 6; i++) reply[i] = request[i];
    reply[6] = crc & 0xFF;
    reply[7] = (crc >> 8) & 0xFF;

    RS485_SendBytes(reply, 8);

    for(uint8_t i = 0; i < 8; i++)
        modbus_reply_buf[i] = reply[i];
    modbus_reply_len = 8;
}

void Modbus_Reply10(uint8_t *request)
{
    uint16_t start_addr = (request[2] << 8) | request[3];
    uint16_t quantity   = (request[4] << 8) | request[5];
    uint8_t  byte_count = request[6];

    for(uint16_t i = 0; i < quantity; i++)
    {
        uint16_t val = (request[7 + i*2] << 8) | request[8 + i*2];
        Modbus_SetHoldingReg(start_addr + i, val);
    }

    uint8_t reply[8];
    reply[0] = MODBUS_SLAVE_ID;
    reply[1] = 0x10;
    reply[2] = request[2];
    reply[3] = request[3];
    reply[4] = request[4];
    reply[5] = request[5];
    uint16_t crc = Modbus_CRC16(reply, 6);
    reply[6] = crc & 0xFF;
    reply[7] = (crc >> 8) & 0xFF;

    RS485_SendBytes(reply, 8);

    for(uint8_t i = 0; i < 8; i++)
        modbus_reply_buf[i] = reply[i];
    modbus_reply_len = 8;
}