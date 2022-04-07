/****************************************************************************/
/*                                                                          */
/*              定时器 / 计数器测试                                         */
/*                                                                          */
/*              2014年06月01日                                              */
/*                                                                          */
/****************************************************************************/
// 注意：DSP ports, Shared RAM, UART0, EDMA, SPI0, MMC/SDs,
//       VPIF, LCDC, SATA, uPP, DDR2/mDDR (bus ports), USB2.0, HPI, PRU
//       这些外设使用的时钟来源为 PLL0_SYSCLK2 默认频率为 CPU 频率的二分之一
//       但是，ECAPs, UART1/2, Timer64P2/3, eHRPWMs,McBSPs, McASP0, SPI1
//       这些外设的时钟来源可以在 PLL0_SYSCLK2 和 PLL1_SYSCLK2 中选择
//       通过修改 System Configuration (SYSCFG) Module
//       寄存器 Chip Configuration 3 Register (CFGCHIP3) 第四位 ASYNC3_CLKSRC
//       配置时钟来源
//       （默认值） 0 来源于 PLL0_SYSCLK2
//                  1 来源于 PLL1_SYSCLK2
//       如果不是为了降低功耗，不建议修改这个值，它会影响所有相关外设的时钟频率

#include <stdio.h>

#include "hw_types.h"
#include "hw_syscfg0_C6748.h"
#include "soc_C6748.h"

#include "timer.h"

#include "interrupt.h"

/****************************************************************************/
/*                                                                          */
/*              宏定义                                                      */
/*                                                                          */
/****************************************************************************/
// 64位 定时器 / 计数器周期
// 定时时间  1ms
#define TMR_PERIOD_LSB32  (228 * 1000)  // 低32位
#define TMR_PERIOD_MSB32  (0)           // 高32位 0

/****************************************************************************/
/*                                                                          */
/*              中断服务函数                                                */
/*                                                                          */
/****************************************************************************/
void TimerIsr(void)
{
    // 禁用定时器 / 计数器中断
    TimerIntDisable(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);

    // 清除中断标志
    IntEventClear(SYS_INT_T64P2_TINTALL);
    TimerIntStatusClear(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);

    lv_tick_inc(1);

    // 使能 定时器 / 计数器 中断
    TimerIntEnable(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);
}

/****************************************************************************/
/*                                                                          */
/*              中断初始化                                   */
/*                                                                          */
/****************************************************************************/
static void InterruptInit()
{
    // 注册中断服务函数
    IntRegister(C674X_MASK_INT6, TimerIsr);

    // 映射中断到 DSP 可屏蔽中断
    IntEventMap(C674X_MASK_INT6, SYS_INT_T64P2_TINTALL);

    // 使能 DSP 可屏蔽中断
    IntEnable(C674X_MASK_INT6);

    // 使能 定时器 / 计数器 中断
    TimerIntEnable(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);
}

/****************************************************************************/
/*                                                                          */
/*              定时器初始化                                                  */
/*                                                                          */
/****************************************************************************/
void TimerInit()
{
    // 定时器 / 计数器初始化
    // 配置 定时器 / 计数器 2 为 64 位模式
    TimerConfigure(SOC_TMR_2_REGS, TMR_CFG_64BIT_CLK_INT);

    // 设置周期
    TimerPeriodSet(SOC_TMR_2_REGS, TMR_TIMER12, TMR_PERIOD_LSB32);
    TimerPeriodSet(SOC_TMR_2_REGS, TMR_TIMER34, TMR_PERIOD_MSB32);

    // 使能 定时器 / 计数器 2
    TimerEnable(SOC_TMR_2_REGS, TMR_TIMER12, TMR_ENABLE_CONT);

    // DSP 中断初始化
    InterruptInit();
}
