/****************************************************************************/
/*                                                                          */
/*    新核科技(广州)有限公司                                                */
/*                                                                          */
/*    Copyright (C) 2022 CoreKernel Technology (Guangzhou) Co., Ltd         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*    WM8960 音频芯片初始化                                                 */
/*                                                                          */
/*    2022年03月30日                                                        */
/*                                                                          */
/****************************************************************************/
/*
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include "soc_C6748.h"

/****************************************************************************/
/*                                                                          */
/*              宏定义                                                      */
/*                                                                          */
/****************************************************************************/
#define ADDRESS   0x1A

/****************************************************************************/
/*                                                                          */
/*              函数声明                                                    */
/*                                                                          */
/****************************************************************************/
void I2CInit(unsigned int baseAddr, unsigned int slaveAddr);
void I2CRegWrite(unsigned int baseAddr, unsigned char regAddr, unsigned char regData);
unsigned char I2CRegRead(unsigned int baseAddr, unsigned char regAddr);

/****************************************************************************/
/*                                                                          */
/*              WM8960 写寄存器                                             */
/*                                                                          */
/****************************************************************************/
void WM8960WriteReg(unsigned char regAddr, unsigned short regData)
{
    // WM8960 寄存器7Bit 数据 9Bit
    I2CRegWrite(SOC_I2C_0_REGS, (regAddr << 1) | ((regData >> 8) & 0x01), regData & 0xFF);
}

/****************************************************************************/
/*                                                                          */
/*              WM8960 初始化                                               */
/*                                                                          */
/****************************************************************************/
void WM8960Init()
{
    // I2C 初始化
    I2CInit(SOC_I2C_0_REGS, ADDRESS);

    // 复位
    WM8960WriteReg(0x0F, 0x00);

    // 配置时钟
    // MCLK->div1->SYSCLK->DAC/ADC Sample Freq = SYSCLK /(1 * 256) = 11.2896 / 256 = 44.1KHz
    WM8960WriteReg(0x04, 0x05);

    // 配置 ADC/DAC
    WM8960WriteReg(0x05, 0x00);

    // 配置音频接口
    // I2S 16Bit 主模式
    WM8960WriteReg(0x07, 0x42);

    // 配置 BCLK
    // BCLKDIV[3:0] = 0100 BCLK Frequency = SYSCLK / 4 = 11.2896 / 4 = 2.8224MHz
    WM8960WriteReg(0x08, 0x1C4);

    // 配置 PLL
    WM8960WriteReg(0x34, 0x37);
    WM8960WriteReg(0x35, 0x86);
    WM8960WriteReg(0x36, 0xC2);
    WM8960WriteReg(0x37, 0x26);

    // PWM MGMT
    WM8960WriteReg(0x19, 1 << 7 | 1 << 6);
    WM8960WriteReg(0x1A, 0x1FD);
    WM8960WriteReg(0x2F, 1 << 3 | 1 << 2);

    // 配置 HP_L 及 HP_R 输出
    WM8960WriteReg(0x02, 0x6F | 0x0100);
    WM8960WriteReg(0x03, 0x6F | 0x0100);

    // 配置 SPK_RP 及 SPK_RN 输出
    WM8960WriteReg(0x28, 0x7F | 0x0100);
    WM8960WriteReg(0x29, 0x7F | 0x0100);

    // 使能输出
    WM8960WriteReg(0x31, 0xF7);

    // 配置 DAC 音量
    WM8960WriteReg(0x0A, 0xFF | 0x0100);
    WM8960WriteReg(0x0B, 0xFF | 0x0100);

    // 3D
//  I2CRegWrite(0x10, 0x1F);

    // 配置麦克风
    WM8960WriteReg(0x22, 1 << 8 | 1 << 7);
    WM8960WriteReg(0x25, 1 << 8 | 1 << 7);

    // 耳机插座切换使能
    WM8960WriteReg(0x18, 1 << 6);

    // 额外配置
    WM8960WriteReg(0x17, 0x01C3);
    WM8960WriteReg(0x30, 0x09);
}
