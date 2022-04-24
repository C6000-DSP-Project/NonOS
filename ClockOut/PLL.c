// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      PLL
//
//      2022��03��27��
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
#include "hw_types.h"

#include "hw_syscfg0_C6748.h"
#include "hw_pllc_C6748.h"

#include "soc_C6748.h"

#include "PLL.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ȫ�ֱ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
unsigned int OSCINFreq = 24;  // 24MHz ����ʱ��

PLLClock pllcfg;

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      PLL ʱ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
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

    // PLL1 ���ʱ��
    if(PLLCTL & 0x01)          // PLL ʹ��
    {
        PLLOut = OSCINFreq * ((PLLM & 0x1F) + 1);

        if(PLLPostDiv & (1 << 15))
        {
            PLLOut /= ((PLLPostDiv & 0x1F) + 1);
        }
    }
    else                       // PLL ��·
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

    // PLL0 ���ʱ��
    if(PLLCTL & 0x01)          // PLL ʹ��
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
    else                       // PLL ��·
    {
        if(PLLCTL & (1 << 9))  // ʹ�� PLL1_SYSCLK3 ��Ϊʱ��Դ
        {
            PLLOut = pllcfg.PLL1_SYSCLK3;
        }
        else                   // ʹ���ⲿʱ��Դ
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

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      PLL ʱ�����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
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
