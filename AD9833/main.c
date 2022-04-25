// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      AD9833 DDS
//
//      2022年04月25日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    DDS 直接数字频率生成
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
    // GPIO3[08] SCLK
    // GPIO3[09] SDATA
    // GPIO3[10] FSYNC
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) &
                                                       (~(SYSCFG_PINMUX7_PINMUX7_31_28    |
                                                          SYSCFG_PINMUX7_PINMUX7_27_24    |
                                                          SYSCFG_PINMUX7_PINMUX7_23_20))) |
                                                        ((SYSCFG_PINMUX7_PINMUX7_31_28_GPIO3_8  << SYSCFG_PINMUX7_PINMUX7_31_28_SHIFT) |
                                                         (SYSCFG_PINMUX7_PINMUX7_27_24_GPIO3_9 << SYSCFG_PINMUX7_PINMUX7_27_24_SHIFT)  |
                                                         (SYSCFG_PINMUX7_PINMUX7_23_20_GPIO3_10 << SYSCFG_PINMUX7_PINMUX7_23_20_SHIFT));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinInit()
{
    GPIODirModeSet(SOC_GPIO_0_REGS, 57, GPIO_DIR_OUTPUT);  // GPIO3[08] SCLK
    GPIODirModeSet(SOC_GPIO_0_REGS, 58, GPIO_DIR_OUTPUT);  // GPIO3[09] SDATA
    GPIODirModeSet(SOC_GPIO_0_REGS, 59, GPIO_DIR_OUTPUT);  // GPIO3[10] FSYNC
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
//      软件模拟 SPI
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 引脚定义
#define SCLK_SET      GPIOPinWrite(SOC_GPIO_0_REGS, 57, GPIO_PIN_HIGH)     // GPIO3[08]
#define SCLK_CLR      GPIOPinWrite(SOC_GPIO_0_REGS, 57, GPIO_PIN_LOW)      // GPIO3[08]
#define SCLK_TOGGLE   {                                                        \
                            Delay(100);                                        \
                            GPIOPinWrite(SOC_GPIO_0_REGS, 57, GPIO_PIN_LOW);   \
                            Delay(100);                                        \
                            GPIOPinWrite(SOC_GPIO_0_REGS, 57, GPIO_PIN_HIGH);  \
                       }                                                    // GPIO3[08]

#define SDATA_SET     GPIOPinWrite(SOC_GPIO_0_REGS, 58, GPIO_PIN_HIGH)     // GPIO3[09]
#define SDATA_CLR     GPIOPinWrite(SOC_GPIO_0_REGS, 58, GPIO_PIN_LOW)      // GPIO3[09]

#define FSYNC_SET     GPIOPinWrite(SOC_GPIO_0_REGS, 59, GPIO_PIN_HIGH)     // GPIO3[10]
#define FSYNC_CLR     GPIOPinWrite(SOC_GPIO_0_REGS, 59, GPIO_PIN_LOW)      // GPIO3[10]

void SoftSPIWrite(unsigned short data)
{
    SCLK_SET;

    FSYNC_SET;
    FSYNC_CLR;

    unsigned char i;
    for(i = 16; i > 0; i--)
    {
        (data >> (i - 1)) & 0x01 ? SDATA_SET : SDATA_CLR;
        SCLK_TOGGLE;
    }

    FSYNC_SET;
    Delay(1000);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      AD9833 配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
#define AD9833_MCLK             (25000000)
#define AD9833_CTRLB28          (1 << 13)
#define AD9833_CTRLRESET        (1 << 8)

#define AD9833_OUTREG0          (0)
#define AD9833_OUTREG1          (1)

#define AD9833_SIN              (0)  // 正弦波
#define AD9833_TRI              (1)  // 三角波
#define AD9833_SQUARE           (3)  // 方波

void AD9833Output(unsigned char mode, unsigned char regsel, unsigned int freq, unsigned short phase)
{
    // 复位
    SoftSPIWrite(AD9833_CTRLRESET);

    // 配置频率/相位
    // 写入 28Bits 频率寄存器
    SoftSPIWrite(AD9833_CTRLB28 | AD9833_CTRLRESET);

    // 频率换算
    float FregReg = freq * 1.0 * (2 << 27) / AD9833_MCLK;

    if(regsel == 0)
    {
        // 频率
        SoftSPIWrite(((unsigned int)FregReg & 0x0003FFF) | 0x4000);
        SoftSPIWrite((((unsigned int)FregReg & 0xFFFC000) >> 14) | 0x4000);

        // 相位
        SoftSPIWrite((phase & 0xFFF) | 0xC000);
    }
    else
    {
        // 频率
        SoftSPIWrite(((unsigned int)freq & 0x0003FFF) | 0x8000);
        SoftSPIWrite((((unsigned int)freq & 0xFFFC000) >> 14) | 0x8000);

        // 相位
        SoftSPIWrite((phase & 0xFFF) | 0xE000);
    }

    // 配置波形
    switch(mode)
    {
        case 1:     // 三角波
            SoftSPIWrite(0x2002); break;

        case 2:     // 方波(输入时钟分频) DAC 数据 MSB / 2
            SoftSPIWrite(0x2020); break;

        case 3:     // 方波(输入时钟分频) DAC 数据 MSB
            SoftSPIWrite(0x2028); break;

        default:    // 正弦波
            SoftSPIWrite(0x2000);
            break;
    }
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

    // 输出波形 400KHz 三角波
    AD9833Output(AD9833_TRI, AD9833_OUTREG0, 400000, 0);

    for(;;)
    {

    }
}
