// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      RTC 实时时钟
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    RTC 设置和显示时间
 *
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include "hw_types.h"

#include "soc_C6748.h"

#include "rtc.h"

#include "interrupt.h"

#include "uartStdio.h"

#include <stdio.h>
#include <time.h>

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct tm RTCTime;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      BCD/十进制数转换
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
int bcd2dec(int bcd)
{
    return (bcd - (bcd >> 4) * 6);
}

int dec2bcd(int dec)
{
    return (dec + (dec / 10) * 6);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      设置日期时间
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void RTCSet()
{
    // 设置日期
    RTCYearSet(SOC_RTC_0_REGS, dec2bcd(RTCTime.tm_year));
    RTCMonthSet(SOC_RTC_0_REGS, dec2bcd(RTCTime.tm_mon));
    RTCDayOfMonthSet(SOC_RTC_0_REGS, dec2bcd(RTCTime.tm_mday));
    RTCDayOfTheWeekSet(SOC_RTC_0_REGS, RTCTime.tm_wday);   // 星期

    // 设置时间
    RTCHourSet(SOC_RTC_0_REGS, dec2bcd(RTCTime.tm_hour));
    RTCMinuteSet(SOC_RTC_0_REGS, dec2bcd(RTCTime.tm_min));
    RTCSecondSet(SOC_RTC_0_REGS, dec2bcd(RTCTime.tm_sec));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      延时（非精确）
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void Delay(volatile unsigned int delay)
{
    while(delay--);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      中断服务函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// StarterWare system_config.lib 驱动库内部已使用 interrupt 关键字修饰 此处仅为回调函数
void RTCIsr()
{
    // 清除中断状态
    IntEventClear(SYS_INT_RTC_IRQS);

    // 获取日期
    RTCTime.tm_year = bcd2dec(RTCYearGet(SOC_RTC_0_REGS));
    RTCTime.tm_mon = bcd2dec(RTCMonthGet(SOC_RTC_0_REGS));
    RTCTime.tm_mday = bcd2dec(RTCDayOfMonthGet(SOC_RTC_0_REGS));
    RTCTime.tm_wday = RTCDayOfTheWeekGet(SOC_RTC_0_REGS);  // 星期

    // 获取时间
    RTCTime.tm_hour = bcd2dec(RTCHourGet(SOC_RTC_0_REGS));
    RTCTime.tm_min = bcd2dec(RTCMinuteGet(SOC_RTC_0_REGS));
    RTCTime.tm_sec = bcd2dec(RTCSecondGet(SOC_RTC_0_REGS));
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
    IntRegister(C674X_MASK_INT4, RTCIsr);

    // 映射中断到 DSP 可屏蔽中断
    IntEventMap(C674X_MASK_INT4, SYS_INT_RTC_IRQS);

    // 使能 DSP 可屏蔽中断
    IntEnable(C674X_MASK_INT4);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      RTC 初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void RTCInit()
{
    // 禁用 RTC 寄存器写保护
    RTCWriteProtectDisable(SOC_RTC_0_REGS);

    // 软件复位并使能 RTC
    RTCEnable(SOC_RTC_0_REGS);

    // 延时 最小3倍 32KH 时钟周期
    Delay(0xFFFF);

    // 使能 32KHz 计数器
    RTCRun(SOC_RTC_0_REGS);

    // 使能实时时钟中断 每秒产生一次中断
    RTCIntTimerEnable(SOC_RTC_0_REGS, RTC_INT_EVERY_SECOND);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      版本识别
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned int RtcVersionGet()
{
    // 返回 1 表示 AM1808
    // AM1808 启动后复位 RTC

    return 0;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    UARTStdioInit();
    UARTprintf("\r\nCoreKernel RTC Example...\r\n");

    // RTC 初始化
    RTCInit();

    // DSP 中断初始化
    InterruptInit();

    // 设置时间
    UARTPuts("Do you want to Set Date/Time?(y or n) ", -1);
    char ch = UARTGetc();
    UARTPutc(ch);
    UARTprintf("\r\n");

    if((ch == 'y') || (ch == 'Y'))
    {
        UARTPuts("Please input as 20220222 222222.\r\n", -1);

        char str[64];
        UARTGets(str, -1);

        sscanf(str, "%4d%2d%2d %2d%2d%2d", &RTCTime.tm_year, &RTCTime.tm_mon, &RTCTime.tm_mday, &RTCTime.tm_hour, &RTCTime.tm_min, &RTCTime.tm_sec);

        // 计算星期
        // 使用基姆拉尔森计算公式
        int y = RTCTime.tm_year, m = RTCTime.tm_mon, d = RTCTime.tm_mday;
        if(RTCTime.tm_mon == 1 || RTCTime.tm_mon==2)
        {
            m = (m == 1 ? 13 : 14);
            y = y - 1;
        }

        RTCTime.tm_wday = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400 + 1) % 7;

        RTCTime.tm_year += 20;
        RTCSet();

        UARTprintf("\r\nOK.\r\n");
    }

    for(;;)
    {
        // 延时(非精确)
        Delay(0x0FFFFFFF);

        // 输出时间
        UARTprintf("%s\r", asctime(&RTCTime));
    }
}
