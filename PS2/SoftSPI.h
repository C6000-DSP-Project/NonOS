// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      软件 SPI
//
//      2022年05月12日
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
#ifndef SOFTSPI_H
#define SOFTSPI_H

#include "hw_types.h"
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "gpio.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      函数声明
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void Delay(volatile unsigned int delay);
void SoftSPIInit();
void SoftSPIReadWrite(unsigned char txdata, unsigned char *rxdata);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
#define SoftSPI_SCLK_CLR() GPIOPinWrite(SOC_GPIO_0_REGS, 46, GPIO_PIN_LOW); Delay(0x000FFF);  // GPIO2[13] SCLK 时钟
#define SoftSPI_SCLK_SET() GPIOPinWrite(SOC_GPIO_0_REGS, 46, GPIO_PIN_HIGH); Delay(0x000FFF);

#define SoftSPI_MOSI_CLR() GPIOPinWrite(SOC_GPIO_0_REGS, 43, GPIO_PIN_LOW); Delay(0x000FFF);  // GPIO2[10] MOSI 数据输出
#define SoftSPI_MOSI_SET() GPIOPinWrite(SOC_GPIO_0_REGS, 43, GPIO_PIN_HIGH); Delay(0x000FFF);

#define SoftSPI_MISO_GET() GPIOPinRead(SOC_GPIO_0_REGS, 44)                                   // GPIO2[11] MISO 数据

#define SoftSPI_CS_CLR()   GPIOPinWrite(SOC_GPIO_0_REGS, 47, GPIO_PIN_LOW); Delay(0x000FFF);  // GPIO2[14] CS0 片选
#define SoftSPI_CS_SET()   GPIOPinWrite(SOC_GPIO_0_REGS, 47, GPIO_PIN_HIGH); Delay(0x000FFF);

#endif
