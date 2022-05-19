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
 *    - ϣ����Ĭ(bin wang)
 *    - bin@corekernel.net
 *
 *    ���� corekernel.net/.org/.cn
 *    ���� fpga.net.cn
 *
 */
#ifndef SOFTSPI_H
#define SOFTSPI_H

#include "hw_types.h"
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "gpio.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void Delay(volatile unsigned int delay);
void SoftSPIInit();
void SoftSPIReadWrite(unsigned char txdata, unsigned char *rxdata);

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �궨��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
#define SoftSPI_SCLK_CLR() GPIOPinWrite(SOC_GPIO_0_REGS, 46, GPIO_PIN_LOW); Delay(0x000FFF);  // GPIO2[13] SCLK ʱ��
#define SoftSPI_SCLK_SET() GPIOPinWrite(SOC_GPIO_0_REGS, 46, GPIO_PIN_HIGH); Delay(0x000FFF);

#define SoftSPI_MOSI_CLR() GPIOPinWrite(SOC_GPIO_0_REGS, 43, GPIO_PIN_LOW); Delay(0x000FFF);  // GPIO2[10] MOSI �������
#define SoftSPI_MOSI_SET() GPIOPinWrite(SOC_GPIO_0_REGS, 43, GPIO_PIN_HIGH); Delay(0x000FFF);

#define SoftSPI_MISO_GET() GPIOPinRead(SOC_GPIO_0_REGS, 44)                                   // GPIO2[11] MISO ����

#define SoftSPI_CS_CLR()   GPIOPinWrite(SOC_GPIO_0_REGS, 47, GPIO_PIN_LOW); Delay(0x000FFF);  // GPIO2[14] CS0 Ƭѡ
#define SoftSPI_CS_SET()   GPIOPinWrite(SOC_GPIO_0_REGS, 47, GPIO_PIN_HIGH); Delay(0x000FFF);

#endif
