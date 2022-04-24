// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      通用异步串口 2(查询模式)
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    串口收发
 *
 *    注意: UART0, EDMA, SPI0, MMC/SD0/1, VPIF, LCDC, SATA, uPP, DDR2/mDDR(总线), USB2.0, HPI, PRU
 *         时钟源为 PLL0_SYSCLK2 默认频率为 CPU 频率 1/2
 *         ECAP0/1/2, UART1/2, Timer64P2/3, eHRPWM0/1, McBSP0/1, McASP0, SPI1
 *         这些外设时钟源可以在 PLL0_SYSCLK2(默认)和 PLL1_SYSCLK2 中选择
 *
 *         修改 System Configuration (SYSCFG) Module 寄存器 Chip Configuration 3 Register (CFGCHIP3)
 *         ASYNC3_CLKSRC 域
 *         （默认值） 0 使用 PLL0_SYSCLK2
 *                 1 使用 PLL1_SYSCLK2
 *
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

#include "psc.h"
#include "gpio.h"
#include "uart.h"

#include "interrupt.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 时钟
#define SYSCLK_1_FREQ     (456000000)
#define SYSCLK_2_FREQ     (SYSCLK_1_FREQ / 2)

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
char txData[] = "\r\nCoreKernel UART2 Example...\r\n";

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinMuxSet()
{
    // 两线制串口 不使用流控
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(04)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(04)) &
                                                    (~(SYSCFG_PINMUX4_PINMUX4_23_20    |
                                                       SYSCFG_PINMUX4_PINMUX4_19_16))) |
                                                     ((SYSCFG_PINMUX4_PINMUX4_23_20_UART2_TXD << SYSCFG_PINMUX4_PINMUX4_23_20_SHIFT) |
                                                      (SYSCFG_PINMUX4_PINMUX4_19_16_UART2_RXD << SYSCFG_PINMUX4_PINMUX4_19_16_SHIFT));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      串口初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void UARTInit()
{
    // 配置 UART2 参数
    // 波特率 115200 数据位 8 停止位 1 无校验位
    UARTConfigSetExpClk(SOC_UART_2_REGS, SYSCLK_2_FREQ, BAUD_115200, UART_WORDL_8BITS, UART_OVER_SAMP_RATE_16);

    // 使能 UART2
    UARTEnable(SOC_UART_2_REGS);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    // 使能外设
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_UART1, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // 管脚复用配置
    GPIOBankPinMuxSet();

    // 串口初始化(需要已初始化 DSP 中断才能正常发送)
    UARTInit();

    // 发送字符串
    unsigned char i;
    for(i = 0; i < sizeof(txData); i++)
    {
        UARTCharPut(SOC_UART_2_REGS, txData[i]);
    }

    unsigned char ch;

    for(;;)
    {
        ch = UARTCharGet(SOC_UART_2_REGS);
        UARTCharPut(SOC_UART_2_REGS, ch);
    }
}
