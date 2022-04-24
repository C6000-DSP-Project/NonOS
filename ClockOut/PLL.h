// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
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
#ifndef PLL_H_
#define PLL_H_

// PLL
typedef struct
{
    unsigned int PLL0_SYSCLK1;  // DSP
    unsigned int PLL0_SYSCLK2;  // ARM RAM/ROM, DSP ports, Shared RAM, UART0, EDMA, SPI0, MMC/SD0/1, VPIF, LCDC, SATA, uPP, DDR2/mDDR (bus ports), USB2.0, HPI, PRU
    unsigned int PLL0_SYSCLK3;  // EMIFA
    unsigned int PLL0_SYSCLK4;  // SYSCFG, GPIO, PLL0/1, PSC, I2C1, EMAC/MDIO, USB1.1, ARM INTC
    unsigned int PLL0_SYSCLK5;  // 未使用
    unsigned int PLL0_SYSCLK6;  // ARM
    unsigned int PLL0_SYSCLK7;  // EMAC RMII Clock
    unsigned int PLL0_AUXCLK;   // PLL 旁路时钟(24MHz)
                                 // I2C0, Timer64P0/P1, RTC, USB2.0 PHY, McASP0 Serial Clock
    unsigned int PLL0_OBSCLK;   // 时钟输出

    unsigned int PLL1_SYSCLK1;  // DDR2/mDDR PHY
    unsigned int PLL1_SYSCLK2;  // ECAP0/1/2, UART1/2, Timer64P2/3, eHRPWM0/1, McBSP0/1, McASP0, SPI1 (默认使用 PLL0_SYSCLK2)
    unsigned int PLL1_SYSCLK3;  // PLL0 输入参考时钟
} PLLClock;

// PLL 时钟输出
typedef enum
{
    OSCIN = 0x14,
    PLL0_SYSCLK1 = 0x17,
    PLL0_SYSCLK2,
    PLL0_SYSCLK3,
    PLL0_SYSCLK4,
    PLL0_SYSCLK5,
    PLL0_SYSCLK6,
    PLL0_SYSCLK7,
    PLLC1_OBSCLK
} PLL0OBSCLK;

typedef enum
{
    PLL1_SYSCLK1 = 0x17,
    PLL1_SYSCLK2,
    PLL1_SYSCLK3
} PLL1OBSCLK;

void PLL0ClockOut(PLL0OBSCLK source, unsigned char div);

#endif
