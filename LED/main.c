// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO LED
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    核心板与底板 GPIO LED 闪烁
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
//      延时（指令方式）
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void Delay(volatile unsigned int delay)
{
    while(delay--);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinMuxSet()
{
    // 配置相应的 GPIO 口功能为普通输入输出口
    // 核心板
    // GPIO6[12]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) & (~(SYSCFG_PINMUX13_PINMUX13_15_12))) |
                                                    ((SYSCFG_PINMUX13_PINMUX13_15_12_GPIO6_12 << SYSCFG_PINMUX13_PINMUX13_15_12_SHIFT));

    // GPIO6[13]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) & (~(SYSCFG_PINMUX13_PINMUX13_11_8))) |
                                                    ((SYSCFG_PINMUX13_PINMUX13_11_8_GPIO6_13 << SYSCFG_PINMUX13_PINMUX13_11_8_SHIFT));

    // 底板
    // GPIO2[15]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(05)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(05)) & (~(SYSCFG_PINMUX5_PINMUX5_3_0))) |
                                                    ((SYSCFG_PINMUX5_PINMUX5_3_0_GPIO2_15 << SYSCFG_PINMUX5_PINMUX5_3_0_SHIFT));

    // GPIO4[00]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(10)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(10)) & (~(SYSCFG_PINMUX10_PINMUX10_31_28))) |
                                                    ((SYSCFG_PINMUX10_PINMUX10_31_28_GPIO4_0 << SYSCFG_PINMUX10_PINMUX10_31_28_SHIFT));
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
    GPIODirModeSet(SOC_GPIO_0_REGS, 109, GPIO_DIR_OUTPUT);  // GPIO6[12] LED3
    GPIODirModeSet(SOC_GPIO_0_REGS, 110, GPIO_DIR_OUTPUT);  // GPIO6[13] LED2

    // 底板
    GPIODirModeSet(SOC_GPIO_0_REGS, 48, GPIO_DIR_OUTPUT);   // GPIO2[15] LED4
    GPIODirModeSet(SOC_GPIO_0_REGS, 65, GPIO_DIR_OUTPUT);   // GPIO4[00] LED3
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

    for(;;)
    {
        // 延时(非精确)
        Delay(0x00FFFFFF);
        GPIOPinWrite(SOC_GPIO_0_REGS, 109, GPIO_PIN_LOW);  // GPIO6[12] LED3
        GPIOPinWrite(SOC_GPIO_0_REGS, 110, GPIO_PIN_LOW);  // GPIO6[13] LED2

        GPIOPinWrite(SOC_GPIO_0_REGS, 48, GPIO_PIN_LOW);   // GPIO2[15] LED4
        GPIOPinWrite(SOC_GPIO_0_REGS, 65, GPIO_PIN_LOW);   // GPIO4[00] LED3

        // 延时(非精确)
        Delay(0x00FFFFFF);
        GPIOPinWrite(SOC_GPIO_0_REGS, 109, GPIO_PIN_HIGH); // GPIO6[12] LED3
        GPIOPinWrite(SOC_GPIO_0_REGS, 110, GPIO_PIN_HIGH); // GPIO6[13] LED2

        GPIOPinWrite(SOC_GPIO_0_REGS, 48, GPIO_PIN_HIGH);  // GPIO2[15] LED4
        GPIOPinWrite(SOC_GPIO_0_REGS, 65, GPIO_PIN_HIGH);  // GPIO4[00] LED3
    }
}
