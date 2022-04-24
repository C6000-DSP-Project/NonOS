// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      I2C API 接口（查询模式）
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include "hw_types.h"
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "i2c.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
volatile unsigned char I2CData[10];

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinMuxSet()
{
    // I2C0
    #define PINMUX4_I2C0_SDA_ENABLE    (SYSCFG_PINMUX4_PINMUX4_15_12_I2C0_SDA << SYSCFG_PINMUX4_PINMUX4_15_12_SHIFT)

    #define PINMUX4_I2C0_SCL_ENABLE    (SYSCFG_PINMUX4_PINMUX4_11_8_I2C0_SCL << SYSCFG_PINMUX4_PINMUX4_11_8_SHIFT)

    unsigned int savePinMux = 0;

    savePinMux = HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) & \
                       ~(SYSCFG_PINMUX4_PINMUX4_15_12 | \
                         SYSCFG_PINMUX4_PINMUX4_11_8);

    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) = (PINMUX4_I2C0_SDA_ENABLE | PINMUX4_I2C0_SCL_ENABLE | savePinMux);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      I2C 发送数据
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void I2CSendBlocking(unsigned int baseAddr, unsigned int dataCnt)
{
    I2CSetDataCount(baseAddr, dataCnt);

    I2CMasterControl(baseAddr, I2C_CFG_MST_TX | I2C_CFG_STOP);

    I2CMasterIntEnableEx(baseAddr, I2C_INT_TRANSMIT_READY | I2C_INT_STOP_CONDITION | I2C_INT_NO_ACK);

    I2CMasterStart(baseAddr);

    unsigned short i;
    for(i = 0; i < dataCnt; i++)
    {
        while(!(I2CMasterIntStatus(baseAddr) & 0x00000010));
        I2CMasterDataPut(SOC_I2C_0_REGS, I2CData[i]);
    }

    while(I2CMasterIntStatus(baseAddr) & 0x00001000);

    I2CMasterStop(SOC_I2C_0_REGS);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      I2C 接收数据
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void I2CRcvBlocking(unsigned int baseAddr, unsigned int dataCnt)
{
    I2CSetDataCount(baseAddr, dataCnt);

    I2CMasterControl(baseAddr, I2C_CFG_MST_RX | I2C_CFG_STOP);

    I2CMasterIntEnableEx(baseAddr, I2C_INT_DATA_READY | I2C_INT_STOP_CONDITION | I2C_INT_NO_ACK);

    I2CMasterStart(baseAddr);

    unsigned short i;
    for(i = 0; i < dataCnt; i++)
    {
        while(!(I2CMasterIntStatus(baseAddr) & 0x00000008));
        I2CData[i] = I2CMasterDataGet(SOC_I2C_0_REGS);
    }

    while(I2CMasterIntStatus(baseAddr) & 0x00001000);

    I2CMasterStop(SOC_I2C_0_REGS);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      I2C 写寄存器
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 8Bit 地址
void I2CRegWrite(unsigned int baseAddr, unsigned char regAddr, unsigned char regData)
{
    // 发送寄存器地址和数据
    I2CData[0] = regAddr;
    I2CData[1] = regData;

    I2CSendBlocking(baseAddr, 2);
}

// 16Bit 地址
void I2CHWRegWrite(unsigned int baseAddr, unsigned short regAddr, unsigned char regData)
{
    // 发送寄存器地址和数据
    I2CData[0] = regAddr >> 8;
    I2CData[1] = regAddr & 0xFF;
    I2CData[2] = regData;

    I2CSendBlocking(baseAddr, 3);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      I2C 读寄存器
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 8Bit 地址
unsigned char I2CRegRead(unsigned int baseAddr, unsigned char regAddr)
{
    // 发送寄存器地址
    I2CData[0] = regAddr;
    I2CSendBlocking(baseAddr, 1);

    // 等待传输完成
    unsigned short i = 50000;
    while(i--);

    // 接收寄存器返回数据
    I2CRcvBlocking(baseAddr, 1);

    return I2CData[0];
}

// 16Bit 地址
unsigned char I2CHWRegRead(unsigned int baseAddr, unsigned short regAddr)
{
    // 发送寄存器地址
    I2CData[0] = regAddr >> 8;
    I2CData[1] = regAddr & 0xFF;
    I2CSendBlocking(baseAddr, 2);

    // 等待传输完成
    unsigned short i = 50000;
    while(i--);

    // 接收寄存器返回数据
    I2CRcvBlocking(baseAddr, 1);

    return I2CData[0];
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      I2C 初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void I2CInit(unsigned int baseAddr, unsigned int slaveAddr)
{
    // 管脚复用配置
    GPIOBankPinMuxSet();

    // 禁用
    I2CMasterDisable(baseAddr);

    // 配置总线速度为 100KHz
    I2CMasterInitExpClk(baseAddr, 24000000, 8000000, 100000);

    // 设置从设备地址
    I2CMasterSlaveAddrSet(baseAddr, slaveAddr);

    // IIC 使能
    I2CMasterEnable(baseAddr);
}
