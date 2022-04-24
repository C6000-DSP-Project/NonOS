// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      EEPROM(I2C 查询模式)
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    EEPROM 读写
 *
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include "soc_C6748.h"

#include "i2c.h"

#include "uartStdio.h"

#include <string.h>

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/// EEPROM 设备地址
#define I2C_ADDR   0x50

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
extern unsigned char I2CData[10];

static volatile unsigned int AckRolling = 0;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      函数声明
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
extern void I2CInit(unsigned int baseAddr, unsigned int slaveAddr);
extern void I2CRcvBlocking(unsigned int baseAddr, unsigned int dataCnt);
extern void I2CSendBlocking(unsigned int baseAddr, unsigned int dataCnt);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      延时（非精确）
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void Delay(volatile unsigned int delay)
{
    while(delay--);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      EEPROM 写字节
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void EEPROMByteWrite(unsigned int address, unsigned char data)
{
    I2CData[0] = address;
    I2CData[1] = data;

    I2CSendBlocking(SOC_I2C_0_REGS, 2);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      EEPROM 读字节
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned char EEPROMByteRead(unsigned int address)
{
    I2CData[0] = address;

    I2CSendBlocking(SOC_I2C_0_REGS, 1);
    I2CRcvBlocking(SOC_I2C_0_REGS, 1);

    return I2CData[0];
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      EEPROM 写多个字节
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void EEPROMPageWrite(unsigned int address, unsigned char *data, unsigned int size)
{
    I2CData[0] = address;

    int i;
    for(i = 0; i < size; i++)
    {
        I2CData[i + 1] = data[i];
    }

    I2CSendBlocking(SOC_I2C_0_REGS, size + 1);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      EEPROM 读多个字节
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void EEPROMPageRead(unsigned int address, unsigned char *data, unsigned int size)
{
    I2CData[0] = address;

    I2CSendBlocking(SOC_I2C_0_REGS, 1);
    I2CRcvBlocking(SOC_I2C_0_REGS, size);

    int i;
    for(i = 0; i < size; i++)
    {
        data[i] = I2CData[i];
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      EEPROM 获取当前地址
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned char EEPROMCurrentAddressGet()
{
    I2CRcvBlocking(SOC_I2C_0_REGS, 1);

    return I2CData[0];
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      EEPROM 写状态轮询
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void EEPROMAckPolling()
{
    do
    {
        AckRolling = 0;
        Delay(0xFFFFF);

        I2CData[0] = I2C_ADDR;
        I2CSendBlocking(SOC_I2C_0_REGS, 1);

        Delay(0xFFFFF);
    } while(AckRolling == 1);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    // 串口初始化
    UARTStdioInit();

    UARTprintf("\r\nCoreKernel EEPROM Example...\r\n");

    // I2C 初始化
    I2CInit(SOC_I2C_0_REGS, I2C_ADDR);

    int i, result;
    unsigned char buf_send[8];
    unsigned char buf_recv[8];

    // 写一个字节
    UARTprintf("Write one byte to address 0x0, value is 0x55.\r\n");
    EEPROMByteWrite(0, 0x55);

    // 等待写完成
    EEPROMAckPolling();

    // 读一个字节
    memset(buf_recv, 0, 8);
    UARTprintf("Read one byte at a address 0x0, the value is ");
    buf_recv[0] = EEPROMByteRead(0);
    UARTprintf("0x%x.\r\n\r\n", buf_recv[0]);

    // 获取当前地址值
    buf_recv[0] = EEPROMCurrentAddressGet();
    UARTprintf("Read one byte at current address 0x0, the value is 0x%x\r\n\r\n", buf_recv[0]);

    // 连续写指定长度
    for(i = 0; i < 8; i++)
    {
        buf_send[i] = i;
    }

    UARTprintf("Write one page (8 bytes) to address 0.\r\n");
    EEPROMPageWrite(0x00, buf_send, 8);

    // 连续读指定长度
    memset(buf_recv, 0 , 8);
    UARTprintf("Read one page (8 bytes) at address 0.\r\n");
    EEPROMPageRead(0x00, buf_recv, 8);

    // 校验
    result = 1;
    for(i = 0; i < 8; i++)
    {
        if(buf_send[i] != buf_recv[i])
        {
            result = 0;
            break;
        }
    }

    UARTprintf("Verify %s.\r\n", result ? "successfully" : "failed");

    for(;;)
    {

    }
}
