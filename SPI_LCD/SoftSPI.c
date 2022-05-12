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
#include "hw_types.h"
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "gpio.h"

#include "SoftSPI.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �궨��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
#define SoftSPI_SCLK_CLR() GPIOPinWrite(SOC_GPIO_0_REGS, 46, GPIO_PIN_LOW)  // GPIO2[13] SCL/SCLK ʱ��
#define SoftSPI_SCLK_SET() GPIOPinWrite(SOC_GPIO_0_REGS, 46, GPIO_PIN_HIGH)

#define SoftSPI_MOSI_CLR() GPIOPinWrite(SOC_GPIO_0_REGS, 43, GPIO_PIN_LOW)  // GPIO2[10] SDA/MOSI �������
#define SoftSPI_MOSI_SET() GPIOPinWrite(SOC_GPIO_0_REGS, 43, GPIO_PIN_HIGH)

#define SoftSPI_CS_CLR()   GPIOPinWrite(SOC_GPIO_0_REGS, 47, GPIO_PIN_LOW)  // GPIO2[14] CS Ƭѡ
#define SoftSPI_CS_SET()   GPIOPinWrite(SOC_GPIO_0_REGS, 47, GPIO_PIN_HIGH)

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

    // CS
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
    GPIODirModeSet(SOC_GPIO_0_REGS, 43, GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_0_REGS, 46, GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_0_REGS, 47, GPIO_DIR_OUTPUT);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��� SPI д����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void SoftSPIWrite(unsigned char data)
{	
    SoftSPI_CS_CLR();

    unsigned char i;
	for(i = 0; i < 8; i++)
	{			  
	    SoftSPI_SCLK_CLR();

		if(data & 0x80)
		{
		    SoftSPI_MOSI_SET();
		}
		else
		{
		    SoftSPI_MOSI_CLR();
		}

		SoftSPI_SCLK_SET();

		data <<= 1;
	}

	SoftSPI_CS_SET();
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
