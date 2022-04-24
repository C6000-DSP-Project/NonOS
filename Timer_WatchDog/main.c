// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      看门狗定时器
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    看门狗定时器(仅支持定时器 1)
 *    超时后会对 DSP 执行 POR 复位 请烧写到 FLASH 通过自启动验证程序现象
 *
 *    注意: I2C0, Timer64P0/P1, RTC, USB2.0 PHY, McASP0 串行时钟
 *         时钟源为 PLL 旁路时钟 即输入 24MHz 时钟
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
#include "timer.h"

#include "uartStdio.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 看门狗定时器超时时间
// 5 秒
#define TMR_PERIOD_LSB32  (5 * 24 * 1000 * 1000)  // 低 32 位
#define TMR_PERIOD_MSB32  (0)                     // 高 32 位

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      看门狗定时器初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void TimerInit()
{
    // 配置 定时器/计数器 1 为看门狗定时器
    TimerConfigure(SOC_TMR_1_REGS, TMR_CFG_64BIT_WATCHDOG);

    // 设置周期 64位
    TimerPeriodSet(SOC_TMR_1_REGS, TMR_TIMER12, TMR_PERIOD_LSB32);
    TimerPeriodSet(SOC_TMR_1_REGS, TMR_TIMER34, TMR_PERIOD_MSB32);

    // 使能看门狗定时器
    TimerWatchdogActivate(SOC_TMR_1_REGS);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    // 串口初始化
    UARTStdioInit();

    // 定时器/计数器初始化
    TimerInit();

    // 打印串口终端信息
    UARTPuts("System is Reset!\r\n\r\n", -2);
    UARTPuts("CoreKernel WatchDog Example...\r\n", -2);
    UARTPuts("If not any character inputs in every 5 seconds, DSP will reset...\r\n", -2);

    for(;;)
    {
        // 等待输入字符
        UARTPutc(UARTGetc());

        // 复位看门狗定时器 “喂狗”
        TimerWatchdogReactivate(SOC_TMR_1_REGS);
    }
}
