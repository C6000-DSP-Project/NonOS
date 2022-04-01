/****************************************************************************/
/*                                                                          */
/*    �º˿Ƽ�(����)���޹�˾                                                */
/*                                                                          */
/*    Copyright (C) 2022 CoreKernel Technology (Guangzhou) Co., Ltd         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*    WM8960 ��ƵоƬ��ʼ��                                                 */
/*                                                                          */
/*    2022��03��30��                                                        */
/*                                                                          */
/****************************************************************************/
/*
 *    - ϣ����Ĭ(bin wang)
 *    - bin@corekernel.net
 *
 *    ���� corekernel.net/.org/.cn
 *    ���� fpga.net.cn
 *
 */
#include "soc_C6748.h"

/****************************************************************************/
/*                                                                          */
/*              �궨��                                                      */
/*                                                                          */
/****************************************************************************/
#define ADDRESS   0x1A

/****************************************************************************/
/*                                                                          */
/*              ��������                                                    */
/*                                                                          */
/****************************************************************************/
void I2CInit(unsigned int baseAddr, unsigned int slaveAddr);
void I2CRegWrite(unsigned int baseAddr, unsigned char regAddr, unsigned char regData);
unsigned char I2CRegRead(unsigned int baseAddr, unsigned char regAddr);

/****************************************************************************/
/*                                                                          */
/*              WM8960 д�Ĵ���                                             */
/*                                                                          */
/****************************************************************************/
void WM8960WriteReg(unsigned char regAddr, unsigned short regData)
{
    // WM8960 �Ĵ���7Bit ���� 9Bit
    I2CRegWrite(SOC_I2C_0_REGS, (regAddr << 1) | ((regData >> 8) & 0x01), regData & 0xFF);
}

/****************************************************************************/
/*                                                                          */
/*              WM8960 ��ʼ��                                               */
/*                                                                          */
/****************************************************************************/
void WM8960Init()
{
    // I2C ��ʼ��
    I2CInit(SOC_I2C_0_REGS, ADDRESS);

    // ��λ
    WM8960WriteReg(0x0F, 0x00);

    // ����ʱ��
    // MCLK->div1->SYSCLK->DAC/ADC Sample Freq = SYSCLK /(1 * 256) = 11.2896 / 256 = 44.1KHz
    WM8960WriteReg(0x04, 0x05);

    // ���� ADC/DAC
    WM8960WriteReg(0x05, 0x00);

    // ������Ƶ�ӿ�
    // I2S 16Bit ��ģʽ
    WM8960WriteReg(0x07, 0x42);

    // ���� BCLK
    // BCLKDIV[3:0] = 0100 BCLK Frequency = SYSCLK / 4 = 11.2896 / 4 = 2.8224MHz
    WM8960WriteReg(0x08, 0x1C4);

    // ���� PLL
    WM8960WriteReg(0x34, 0x37);
    WM8960WriteReg(0x35, 0x86);
    WM8960WriteReg(0x36, 0xC2);
    WM8960WriteReg(0x37, 0x26);

    // PWM MGMT
    WM8960WriteReg(0x19, 1 << 7 | 1 << 6);
    WM8960WriteReg(0x1A, 0x1FD);
    WM8960WriteReg(0x2F, 1 << 3 | 1 << 2);

    // ���� HP_L �� HP_R ���
    WM8960WriteReg(0x02, 0x6F | 0x0100);
    WM8960WriteReg(0x03, 0x6F | 0x0100);

    // ���� SPK_RP �� SPK_RN ���
    WM8960WriteReg(0x28, 0x7F | 0x0100);
    WM8960WriteReg(0x29, 0x7F | 0x0100);

    // ʹ�����
    WM8960WriteReg(0x31, 0xF7);

    // ���� DAC ����
    WM8960WriteReg(0x0A, 0xFF | 0x0100);
    WM8960WriteReg(0x0B, 0xFF | 0x0100);

    // 3D
//  I2CRegWrite(0x10, 0x1F);

    // ������˷�
    WM8960WriteReg(0x22, 1 << 8 | 1 << 7);
    WM8960WriteReg(0x25, 1 << 8 | 1 << 7);

    // ���������л�ʹ��
    WM8960WriteReg(0x18, 1 << 6);

    // ��������
    WM8960WriteReg(0x17, 0x01C3);
    WM8960WriteReg(0x30, 0x09);
}
