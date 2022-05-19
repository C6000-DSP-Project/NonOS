// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��� SPI
//
//      2022��05��12��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    GPIO ģ�� SPI ʱ��
 *
 *    - ϣ����Ĭ(bin wang)
 *    - bin@corekernel.net
 *
 *    ���� corekernel.net/.org/.cn
 *    ���� fpga.net.cn
 *
 */
#include "SoftSPI.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽŸ�������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinMuxSet()
{
    // SPI1 CLK SIMO SOMI
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) & (~(SYSCFG_PINMUX5_PINMUX5_23_20 | SYSCFG_PINMUX5_PINMUX5_19_16 | SYSCFG_PINMUX5_PINMUX5_15_12 | SYSCFG_PINMUX5_PINMUX5_11_8))) |
                                                   ((SYSCFG_PINMUX5_PINMUX5_11_8_GPIO2_13  << SYSCFG_PINMUX5_PINMUX5_11_8_SHIFT) |
                                                    (SYSCFG_PINMUX5_PINMUX5_23_20_GPIO2_10 << SYSCFG_PINMUX5_PINMUX5_23_20_SHIFT) |
                                                    (SYSCFG_PINMUX5_PINMUX5_19_16_GPIO2_11 << SYSCFG_PINMUX5_PINMUX5_19_16_SHIFT));

    // CS0
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) & (~(SYSCFG_PINMUX5_PINMUX5_7_4))) |
                                                   (SYSCFG_PINMUX5_PINMUX5_7_4_GPIO2_14 << SYSCFG_PINMUX5_PINMUX5_7_4_SHIFT);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽų�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinInit()
{
    GPIODirModeSet(SOC_GPIO_0_REGS, 47, GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_0_REGS, 43, GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_0_REGS, 44, GPIO_DIR_INPUT);
    GPIODirModeSet(SOC_GPIO_0_REGS, 46, GPIO_DIR_OUTPUT);

    GPIOPinWrite(SOC_GPIO_0_REGS, 47, GPIO_PIN_HIGH);
    GPIOPinWrite(SOC_GPIO_0_REGS, 43, GPIO_PIN_HIGH);
    GPIOPinWrite(SOC_GPIO_0_REGS, 44, GPIO_PIN_HIGH);
    GPIOPinWrite(SOC_GPIO_0_REGS, 46, GPIO_PIN_HIGH);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��� SPI д����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void SoftSPIReadWrite(unsigned char txdata, unsigned char *rxdata)
{	
    unsigned short i;
	for(i = 1; i < 0x100; i <<= 1)
	{
        if(txdata != 0)
        {
            if(i & txdata)
            {
                SoftSPI_MOSI_SET();
            }
            else
            {
                SoftSPI_MOSI_CLR();
            }
        }

        SoftSPI_SCLK_CLR();

        if(rxdata != NULL && SoftSPI_MISO_GET())
        {
            *rxdata |= i;
        }

        SoftSPI_SCLK_SET();
	}
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��� SPI ��ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void SoftSPIInit()
{
    // �ܽŸ�������
    GPIOBankPinMuxSet();

    // GPIO �ܽų�ʼ��
    GPIOBankPinInit();
}
