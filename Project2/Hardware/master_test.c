#include "master_test.h"
#include "Modbus.h"
#include "OLED.h"
#include "Delay.h"

typedef struct {
    uint8_t  request[8];
    uint8_t  req_len;
    uint8_t  expected_prefix[6];
    uint8_t  prefix_len;
    char     desc[24];
    uint8_t  should_pass;         // 1=期望通过, 0=期望返回异常
} TestCase;

static const TestCase cases[] = {
    // 用例1：正常读3个寄存器
    {
        {0x01, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00}, 6,
        {0x01, 0x04, 0x06}, 3,
        "Read 3 input regs", 1
    },
    // 用例2：非法功能码
    {
        {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00}, 6,
        {0x01, 0x83, 0x01}, 3,
        "Illegal function", 0
    },
    // 用例3：地址超出范围
    {
        {0x01, 0x04, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00}, 6,
        {0x01, 0x84, 0x02}, 3,
        "Illegal address", 0
    },
    // 用例4：请求数量为0
    {
        {0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 6,
        {0x01, 0x84, 0x03}, 3,
        "Quantity zero", 0
    },
};

static uint8_t test_pass_count = 0;
static uint8_t test_fail_count = 0;

void MasterTest_Init(void)
{
    test_pass_count = 0;
    test_fail_count = 0;
}

void MasterTest_Run(void)
{
    test_pass_count = 0;
    test_fail_count = 0;

    uint8_t case_count = sizeof(cases) / sizeof(cases[0]);

    for(uint8_t i = 0; i < case_count; i++)
    {
        // 计算请求帧CRC并填入
        uint16_t crc = Modbus_CRC16((uint8_t*)cases[i].request, cases[i].req_len - 2);
        uint8_t req[8];
        for(uint8_t j = 0; j < cases[i].req_len; j++) req[j] = cases[i].request[j];
        req[cases[i].req_len - 2] = crc & 0xFF;
        req[cases[i].req_len - 1] = (crc >> 8) & 0xFF;

        // 将请求帧注入从站解析
        for(uint8_t j = 0; j < cases[i].req_len; j++)
            modbus_rx_buf[j] = req[j];
        modbus_rx_len = cases[i].req_len;

        Modbus_Parse();

        // 检查应答头部
        uint8_t ok = 1;
        for(uint8_t j = 0; j < cases[i].prefix_len; j++)
        {
            if(Modbus_GetReplyByte(j) != cases[i].expected_prefix[j])
            {
                ok = 0;
                break;
            }
        }

        if(ok) test_pass_count++;
        else   test_fail_count++;

        Delay_ms(10);
    }
}

uint8_t MasterTest_GetResult(void)
{
    return (test_fail_count == 0) ? 0 : 1;
}

void MasterTest_ShowResult(void)
{
    uint8_t total = test_pass_count + test_fail_count;
    OLED_ShowString(1, 1, "Master Test     ");
    OLED_ShowString(2, 1, "Pass:");
    OLED_ShowNum(2, 6, test_pass_count, 1);
    OLED_ShowString(2, 7, "/");
    OLED_ShowNum(2, 8, total, 1);

    if(test_fail_count == 0)
        OLED_ShowString(3, 1, "Result: ALL OK  ");
    else
    {
        OLED_ShowString(3, 1, "Result: FAIL    ");
        OLED_ShowNum(4, 1, test_fail_count, 1);
        OLED_ShowString(4, 3, "failed        ");
    }
}