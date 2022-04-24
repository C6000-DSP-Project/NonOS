// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO LED 软件模拟 PWM
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    核心板 LED 呼吸灯
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

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinMuxSet()
{
    // 配置相应的 GPIO 口功能为普通输入输出口
    // 核心板
    // GPIO6[13]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) & (~(SYSCFG_PINMUX13_PINMUX13_11_8))) |
                                                    ((SYSCFG_PINMUX13_PINMUX13_11_8_GPIO6_13 << SYSCFG_PINMUX13_PINMUX13_11_8_SHIFT));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinInit()
{
    // 配置 LED 对应管脚为输出管脚
    // OMAP-L138 及 TMS320C6748 共有 144 个 GPIO
    // GPIO0[0] 1-16
    // GPIO1[0] 17-32
    // GPIO2[0] 33-48
    // GPIO3[0] 49-64
    // GPIO4[0] 65-80
    // GPIO5[0] 81-96
    // GPIO6[0] 97-112
    // GPIO7[0] 113-128
    // GPIO8[0] 129-144

    // 核心板
    GPIODirModeSet(SOC_GPIO_0_REGS, 110, GPIO_DIR_OUTPUT);  // GPIO6[13] LED2
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
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    // 使能外设
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // 管脚复用配置
    GPIOBankPinMuxSet();

    // GPIO 管脚初始化
    GPIOBankPinInit();

    unsigned int SoftPWM = 4000, p, q;

    for(;;)
    {
        for(p = 0; p < SoftPWM; p++)
        {
            q = SoftPWM - p;

            GPIOPinWrite(SOC_GPIO_0_REGS, 110, GPIO_PIN_LOW);
            Delay(p);

            GPIOPinWrite(SOC_GPIO_0_REGS, 110, GPIO_PIN_HIGH);
            Delay(q);
        }

        for(p = SoftPWM; p > 0; p--)
        {
            q = SoftPWM - p;

            GPIOPinWrite(SOC_GPIO_0_REGS, 110, GPIO_PIN_LOW);
            Delay(p);

            GPIOPinWrite(SOC_GPIO_0_REGS, 110, GPIO_PIN_HIGH);
            Delay(q);
        }
    }
}
