// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO KEYBOARD
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    键盘
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

#include "interrupt.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned char KEYNum = 0;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinMuxSet()
{
    // 配置相应的 GPIO 口功能为普通输入输出口
    // 底板 LED
    // GPIO2[15]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(05)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(05)) & (~(SYSCFG_PINMUX5_PINMUX5_3_0))) |
                                                    ((SYSCFG_PINMUX5_PINMUX5_3_0_GPIO2_15 << SYSCFG_PINMUX5_PINMUX5_3_0_SHIFT));

    // GPIO4[00]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(10)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(10)) & (~(SYSCFG_PINMUX10_PINMUX10_31_28))) |
                                                    ((SYSCFG_PINMUX10_PINMUX10_31_28_GPIO4_0 << SYSCFG_PINMUX10_PINMUX10_31_28_SHIFT));

    // 按键
    // GPIO5[0] - GPIO5[7]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(12)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(12)) &
                                                  (~(SYSCFG_PINMUX12_PINMUX12_31_28 |
                                                     SYSCFG_PINMUX12_PINMUX12_27_24 |
                                                     SYSCFG_PINMUX12_PINMUX12_23_20 |
                                                     SYSCFG_PINMUX12_PINMUX12_19_16 |
                                                     SYSCFG_PINMUX12_PINMUX12_15_12 |
                                                     SYSCFG_PINMUX12_PINMUX12_11_8 |
                                                     SYSCFG_PINMUX12_PINMUX12_7_4 |
                                                     SYSCFG_PINMUX12_PINMUX12_3_0))) |
                                                   ((SYSCFG_PINMUX12_PINMUX12_31_28_GPIO5_0 << SYSCFG_PINMUX12_PINMUX12_31_28_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_27_24_GPIO5_1 << SYSCFG_PINMUX12_PINMUX12_27_24_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_23_20_GPIO5_2 << SYSCFG_PINMUX12_PINMUX12_23_20_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_19_16_GPIO5_3 << SYSCFG_PINMUX12_PINMUX12_19_16_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_15_12_GPIO5_4 << SYSCFG_PINMUX12_PINMUX12_15_12_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_11_8_GPIO5_5  << SYSCFG_PINMUX12_PINMUX12_11_8_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_7_4_GPIO5_6   << SYSCFG_PINMUX12_PINMUX12_7_4_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_3_0_GPIO5_7   << SYSCFG_PINMUX12_PINMUX12_3_0_SHIFT));

    // GPIO5[8] - GPIO5[9]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                  (~(SYSCFG_PINMUX11_PINMUX11_31_28 |
                                                     SYSCFG_PINMUX11_PINMUX11_27_24))) |
                                                   ((SYSCFG_PINMUX11_PINMUX11_31_28_GPIO5_8 << SYSCFG_PINMUX11_PINMUX11_31_28_SHIFT) |
                                                    (SYSCFG_PINMUX11_PINMUX11_27_24_GPIO5_9 << SYSCFG_PINMUX11_PINMUX11_27_24_SHIFT));

    // GPIO0[8] 功能键
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) &
                                                  (~(SYSCFG_PINMUX0_PINMUX0_31_28))) |
                                                   ((SYSCFG_PINMUX0_PINMUX0_31_28_GPIO0_8 << SYSCFG_PINMUX0_PINMUX0_31_28_SHIFT));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinInit()
{
    // 底板 LED
    GPIODirModeSet(SOC_GPIO_0_REGS, 48, GPIO_DIR_OUTPUT);   // GPIO2[15] LED4
    GPIODirModeSet(SOC_GPIO_0_REGS, 65, GPIO_DIR_OUTPUT);   // GPIO4[00] LED3

    // 按键
    // 配置中断触发方式
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 81, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[00] 下降沿
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 82, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[01] 下降沿
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 83, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[02] 下降沿
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 84, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[03] 下降沿
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 85, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[04] 下降沿
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 86, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[05] 下降沿
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 87, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[06] 下降沿
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 88, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[07] 下降沿
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 89, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[08] 下降沿
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 90, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[09] 下降沿

    GPIOIntTypeSet(SOC_GPIO_0_REGS, 9, GPIO_INT_TYPE_FALLEDGE);         // GPIO0[08] 下降沿

    // 使能 GPIO BANK 中断
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 0);                              // GPIO BANK0
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 5);                              // GPIO BANK5
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
void KEYNumIsr()
{
    // 清除中断状态
    IntEventClear(SYS_INT_GPIO_B5INT);

    unsigned int KEYInt = (HWREG(SOC_GPIO_0_REGS + GPIO_INTSTAT(2)) & 0x03FF0000) >> 16;

    switch(KEYInt)
    {
        case 0x001:  // 按键 0
            KEYNum = 0;
            break;

        case 0x002:  // 按键 1
            KEYNum = 1;
            break;

        case 0x004:  // 按键 2
            KEYNum = 2;
            break;

        case 0x008:  // 按键 3
            KEYNum = 3;
            break;

        case 0x010:  // 按键 4
            KEYNum = 4;
            break;

        case 0x020:  // 按键 5
            KEYNum = 5;
            break;

        case 0x040:  // 按键 6
            KEYNum = 6;
            break;

        case 0x080:  // 按键 7
            KEYNum = 7;
            break;

        case 0x100:  // 按键 8
            KEYNum = 8;
            break;

        case 0x200:  // 按键 9
            KEYNum = 9;
            break;

        default:
            break;
    }

    // 闪烁 LED
    unsigned char i;
    for(i = 0; i < KEYNum; i++)
    {
        GPIOPinWrite(SOC_GPIO_0_REGS, 48, GPIO_PIN_HIGH);
        Delay(0x00FFFFFF);  // 此处仅为演示方便 请勿在中断服务函数使用类似语句

        GPIOPinWrite(SOC_GPIO_0_REGS, 48, GPIO_PIN_LOW);
        Delay(0x00FFFFFF);
    }

    // 清除 GPIO 中断状态
    GPIOPinIntClear(SOC_GPIO_0_REGS, 81 + KEYNum);
}

void KEYFUNIsr()
{
    // 清除中断状态
    IntEventClear(SYS_INT_GPIO_B0INT);

    if(GPIOPinIntStatus(SOC_GPIO_0_REGS, 9) == GPIO_INT_PEND)
    {
        // 闪烁 LED
        GPIOPinWrite(SOC_GPIO_0_REGS, 65, GPIO_PIN_HIGH);
        Delay(0x00FFFFFF);  // 此处仅为演示方便 请勿在中断服务函数使用类似语句
        GPIOPinWrite(SOC_GPIO_0_REGS, 65, GPIO_PIN_LOW);
    }

    // 清除 GPIO 中断状态
    GPIOPinIntClear(SOC_GPIO_0_REGS, 9);
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
    IntRegister(C674X_MASK_INT4, KEYNumIsr);
    IntRegister(C674X_MASK_INT5, KEYFUNIsr);

    // 映射中断到 DSP 可屏蔽中断
    IntEventMap(C674X_MASK_INT4, SYS_INT_GPIO_B5INT);
    IntEventMap(C674X_MASK_INT5, SYS_INT_GPIO_B0INT);

    // 使能 DSP 可屏蔽中断
    IntEnable(C674X_MASK_INT4);
    IntEnable(C674X_MASK_INT5);
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

    // DSP 中断初始化
    InterruptInit();

    for(;;)
    {

    }
}
