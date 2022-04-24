// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      PLL
//
//      2022年03月27日
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
#include "hw_pllc_C6748.h"

#include "soc_C6748.h"

#include "PLL.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned int OSCINFreq = 24;  // 24MHz 输入时钟

PLLClock pllcfg;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      PLL 时钟
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void PLLClockGet()
{
    unsigned int PLLCTL;
    unsigned int PLLPreDiv, PLLM, PLLPostDiv;
    unsigned int PLLDiv1, PLLDiv2, PLLDiv3, PLLDiv4, PLLDiv6, PLLDiv7;
    unsigned int PLLOut;

    // PLLC1
    PLLCTL = HWREG(SOC_PLLC_1_REGS + PLLC_PLLCTL);
    PLLM = HWREG(SOC_PLLC_1_REGS + PLLC_PLLM);
    PLLPostDiv = HWREG(SOC_PLLC_1_REGS + PLLC_POSTDIV);

    PLLDiv1 = HWREG(SOC_PLLC_1_REGS + PLLC_PLLDIV1);
    PLLDiv2 = HWREG(SOC_PLLC_1_REGS + PLLC_PLLDIV2);
    PLLDiv3 = HWREG(SOC_PLLC_1_REGS + PLLC_PLLDIV3);

    // PLL1 输出时钟
    if(PLLCTL & 0x01)          // PLL 使能
    {
        PLLOut = OSCINFreq * ((PLLM & 0x1F) + 1);

        if(PLLPostDiv & (1 << 15))
        {
            PLLOut /= ((PLLPostDiv & 0x1F) + 1);
        }
    }
    else                       // PLL 旁路
    {
        PLLOut = OSCINFreq;
    }

    pllcfg.PLL1_SYSCLK1 = (PLLDiv1 & (1 << 15)) ? PLLOut / ((PLLDiv1 & 0x1F) + 1) : PLLOut;
    pllcfg.PLL1_SYSCLK2 = (PLLDiv2 & (1 << 15)) ? PLLOut / ((PLLDiv2 & 0x1F) + 1) : PLLOut;
    pllcfg.PLL1_SYSCLK3 = (PLLDiv3 & (1 << 15)) ? PLLOut / ((PLLDiv3 & 0x1F) + 1) : PLLOut;

    // PLLC0
    PLLCTL = HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL);
    PLLPreDiv = HWREG(SOC_PLLC_0_REGS + PLLC_PREDIV);
    PLLM = HWREG(SOC_PLLC_0_REGS + PLLC_PLLM);
    PLLPostDiv = HWREG(SOC_PLLC_0_REGS + PLLC_POSTDIV);

    PLLDiv1 = HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV1);
    PLLDiv2 = HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV2);
    PLLDiv3 = HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV3);
    PLLDiv4 = HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV4);
    PLLDiv6 = HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV5);
    PLLDiv7 = HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV7);

    // PLL0 输出时钟
    if(PLLCTL & 0x01)          // PLL 使能
    {
        if(PLLPreDiv & (1 << 15))
        {
            PLLOut = OSCINFreq / ((PLLPreDiv & 0x1F) + 1);
        }
        else
        {
            PLLOut = OSCINFreq;
        }

        PLLOut *= ((PLLM & 0x1F) + 1);

        if(PLLPostDiv & (1 << 15))
        {
            PLLOut /= ((PLLPostDiv & 0x1F) + 1);
        }
    }
    else                       // PLL 旁路
    {
        if(PLLCTL & (1 << 9))  // 使用 PLL1_SYSCLK3 作为时钟源
        {
            PLLOut = pllcfg.PLL1_SYSCLK3;
        }
        else                   // 使用外部时钟源
        {
            PLLOut = OSCINFreq;
        }
    }

    pllcfg.PLL0_AUXCLK = OSCINFreq;

    pllcfg.PLL0_SYSCLK1 = (PLLDiv1 & (1 << 15)) ? PLLOut / ((PLLDiv1 & 0x1F) + 1) : PLLOut;
    pllcfg.PLL0_SYSCLK2 = (PLLDiv2 & (1 << 15)) ? PLLOut / ((PLLDiv2 & 0x1F) + 1) : PLLOut;
    pllcfg.PLL0_SYSCLK3 = (PLLDiv3 & (1 << 15)) ? PLLOut / ((PLLDiv3 & 0x1F) + 1) : PLLOut;
    pllcfg.PLL0_SYSCLK4 = (PLLDiv4 & (1 << 15)) ? PLLOut / ((PLLDiv4 & 0x1F) + 1) : PLLOut;
    pllcfg.PLL0_SYSCLK6 = (PLLDiv6 & (1 << 15)) ? PLLOut / ((PLLDiv6 & 0x1F) + 1) : PLLOut;
    pllcfg.PLL0_SYSCLK7 = (PLLDiv7 & (1 << 15)) ? PLLOut / ((PLLDiv7 & 0x1F) + 1) : PLLOut;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      PLL 时钟输出
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void PLL0ClockOut(PLL0OBSCLK source, unsigned char div)
{
    // GPIO6[14] CLKOUT
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) & (~(SYSCFG_PINMUX13_PINMUX13_7_4))) |
                                                    (SYSCFG_PINMUX13_PINMUX13_7_4_OBSCLK0 << SYSCFG_PINMUX13_PINMUX13_7_4_SHIFT);

    volatile unsigned int *OCSEL = (volatile unsigned int *)(SOC_PLLC_0_REGS + PLLC_OCSEL);
    volatile unsigned int *OCDIV = (volatile unsigned int *)(SOC_PLLC_0_REGS + PLLC_OSCDIV);
    volatile unsigned int *CKEN  = (volatile unsigned int *)(SOC_PLLC_0_REGS + PLLC_CKEN);

    *CKEN  = 0x00000003;
    *OCSEL = source;
    if(div != 0)
    {
        *OCDIV = div;
        *OCDIV |= 1 << 15;
    }
    else
    {
        *OCDIV = 0x00008000;
    }
}

void PLL1ClockOut(PLL1OBSCLK source, unsigned char div)
{
    // GPIO6[14] CLKOUT
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) & (~(SYSCFG_PINMUX13_PINMUX13_7_4))) |
                                                    (SYSCFG_PINMUX13_PINMUX13_7_4_OBSCLK0 << SYSCFG_PINMUX13_PINMUX13_7_4_SHIFT);

    PLL0ClockOut(PLLC1_OBSCLK, 0);

    volatile unsigned int *OCSEL = (volatile unsigned int *)(SOC_PLLC_1_REGS + PLLC_OCSEL);
    volatile unsigned int *OCDIV = (volatile unsigned int *)(SOC_PLLC_1_REGS + PLLC_OSCDIV);
    volatile unsigned int *CKEN  = (volatile unsigned int *)(SOC_PLLC_1_REGS + PLLC_CKEN);

    *CKEN  = 0x00000003;
    *OCSEL = source;
    if(div != 0)
    {
        *OCDIV = div;
        *OCDIV |= 1 << 15;
    }
    else
    {
        *OCDIV = 0x00008000;
    }
}
