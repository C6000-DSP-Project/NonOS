// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      通用异步串口 2(中断模式)
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
//      中断服务函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// StarterWare system_config.lib 驱动库内部已使用 interrupt 关键字修饰 此处仅为回调函数
void UARTIsr()
{
    static unsigned int length = sizeof(txData);
    static unsigned int count = 0;
    unsigned char rxData = 0;
    unsigned int int_id = 0;

    // 确定中断源
    int_id = UARTIntStatus(SOC_UART_2_REGS);

    // 清除 UART2 系统中断
    IntEventClear(SYS_INT_UART2_INT);

    // 发送中断
    if(UART_INTID_TX_EMPTY == int_id)
    {
        if(0 < length)
        {
            // 写一个字节到 THR
            UARTCharPutNonBlocking(SOC_UART_2_REGS, txData[count]);
            length--;
            count++;
        }
        if(0 == length)
        {
            // 禁用发送中断
            UARTIntDisable(SOC_UART_2_REGS, UART_INT_TX_EMPTY);
        }
     }

    // 接收中断
    if(UART_INTID_RX_DATA == int_id)
    {
        // 回显字符
        rxData = UARTCharGetNonBlocking(SOC_UART_2_REGS);
        UARTCharPutNonBlocking(SOC_UART_2_REGS, rxData);
    }

    // 接收错误
    if(UART_INTID_RX_LINE_STAT == int_id)
    {
        while(UARTRxErrorGet(SOC_UART_2_REGS))
        {
            // 从 RBR 读一个字节
            UARTCharGetNonBlocking(SOC_UART_2_REGS);
        }
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      DSP 中断初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void InterruptInit(void)
{
    // 初始化 DSP 中断控制器
    IntDSPINTCInit();

    // 使能 DSP 全局中断
    IntGlobalEnable();

    // 注册中断服务函数
    IntRegister(C674X_MASK_INT4, UARTIsr);

    // 映射中断到 DSP 可屏蔽中断
    IntEventMap(C674X_MASK_INT4, SYS_INT_UART2_INT);

    // 使能 DSP 可屏蔽中断
    IntEnable(C674X_MASK_INT4);
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

    // 使能接收 / 发送 FIFO
    UARTFIFOEnable(SOC_UART_2_REGS);

    // 设置 FIFO 级别
    UARTFIFOLevelSet(SOC_UART_2_REGS, UART_RX_TRIG_LEVEL_1);

    // 使能串口中断
    UARTIntEnable(SOC_UART_2_REGS, UART_INT_LINE_STAT | UART_INT_TX_EMPTY | UART_INT_RXDATA_CTI);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    // 使能外设
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_UART2, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // 管脚复用配置
    GPIOBankPinMuxSet();

    // DSP 中断初始化
    InterruptInit();

    // 串口初始化(需要已初始化 DSP 中断才能正常发送)
    UARTInit();

    for(;;)
    {

    }
}
