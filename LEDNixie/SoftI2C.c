// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      软件模拟 I2C
//
//      2022年03月16日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
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
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 引脚定义
#define SCL_SET       {GPIOPinWrite(SOC_GPIO_0_REGS, 96, GPIO_PIN_HIGH);}      // GPIO5[15] SCL 96 脚
#define SCL_CLR       {GPIOPinWrite(SOC_GPIO_0_REGS, 96, GPIO_PIN_LOW);}       // GPIO5[15] SCL 96 脚
#define SCL_D_OUT     {GPIODirModeSet(SOC_GPIO_0_REGS, 96, GPIO_DIR_OUTPUT);}  // 设置 SCL 为输出方向

#define SDA_SET       {GPIOPinWrite(SOC_GPIO_0_REGS, 95, GPIO_PIN_HIGH);}      // GPIO5[14] SDA 95 脚
#define SDA_CLR       {GPIOPinWrite(SOC_GPIO_0_REGS, 95, GPIO_PIN_LOW);}       // GPIO5[14] SDA 95 脚
#define SDA_IN        (GPIOPinRead(SOC_GPIO_0_REGS, 95))                       // GPIO5[14] SDA 95 脚
#define SDA_D_OUT     {GPIODirModeSet(SOC_GPIO_0_REGS, 95, GPIO_DIR_OUTPUT);}  // 设置 SDA 为输出方向
#define SDA_D_IN      {GPIODirModeSet(SOC_GPIO_0_REGS, 95, GPIO_DIR_INPUT);}   // 设置 SDA 为输入方向

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinMuxSet()
{
    // 配置相应的 GPIO 口功能为普通输入输出口
    // GPIO5[15] SCL
    // GPIO5[14] SDA
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                  (~(SYSCFG_PINMUX11_PINMUX11_7_4    |
                                                     SYSCFG_PINMUX11_PINMUX11_3_0))) |
                                                   ((SYSCFG_PINMUX11_PINMUX11_7_4_GPIO5_14 << SYSCFG_PINMUX11_PINMUX11_7_4_SHIFT) |
                                                    (SYSCFG_PINMUX11_PINMUX11_3_0_GPIO5_15 << SYSCFG_PINMUX11_PINMUX11_3_0_SHIFT));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      延时（非精确）
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void Delay(volatile unsigned int delay)
{
    while(delay--);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      软件模拟 I2C
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 起始信号
void SoftI2CStart()
{
    SDA_SET;                        // 发送起始信号
    SDA_D_OUT;                      // 设置 SDA 为输出方向
    SCL_SET;
    SCL_D_OUT;                      // 设置 SCL 为输出方向
    Delay(5000);
    SDA_CLR;                        // 发送起始信号
    Delay(5000);
    SCL_CLR;                        // 准备发送或接收数据
    Delay(5000);
}

// 结束信号
void SoftI2CStop()
{
    SDA_CLR;
    SDA_D_OUT;                      // 设置 SDA 为输出方向
    Delay(5000);
    SCL_SET;
    Delay(5000);
    SDA_SET;                        // 发送结束信号
    Delay(5000);
    SDA_D_IN;                       // 设置 SDA 为输入方向
    Delay(5000);
}

// 写字节数据
void SoftI2CWrite(unsigned char ch)
{
    SDA_D_OUT;                      // 设置 SDA 为输出方向

    unsigned char i;
    for(i = 0; i < 8; i++)          // 输出 8 位数据
    {
        if(ch & 0x80)
        {
            SDA_SET;
        }
        else
        {
            SDA_CLR;
        }

        Delay(5000);
        SCL_SET;
        ch <<= 1;
        Delay(5000);
        SCL_CLR;
        Delay(5000);
    }

    SDA_D_IN;                      // 设置 SDA 为输入方向
    SDA_SET;
    Delay(5000);
    SCL_SET;                       // 接收应答
    Delay(5000);
    SCL_CLR;
    Delay(5000);
}

// 读字节数据
unsigned char SoftI2CRead()
{

    SDA_SET;
    SDA_D_IN;                      // 设置 SDA 为输入方向

    unsigned char ch, i;
    ch = 0;

    for(i = 0; i < 8; i++)        // 输入 8 位数据
    {
        SCL_SET;
        Delay(5000);

        ch <<= 1;
        if(SDA_IN)
            ch++;                 // 输入 1 位

        SCL_CLR;
        Delay(5000);
    }

    SDA_SET;
    Delay(5000);
    SCL_SET;                     // 发出无效应答
    Delay(5000);
    SCL_CLR;
    Delay(5000);

    return(ch);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void SoftI2CInit()
{
    // 使能外设
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // 管脚复用配置
    GPIOBankPinMuxSet();
}
