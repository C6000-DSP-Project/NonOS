// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      AD5676R DAC
//
//      2022年04月25日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    模拟信号输出
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
#include "emifa.h"

#include "interrupt.h"

#include "uartStdio.h"

#include <stdio.h>
#include <math.h>

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SPI 管脚配置
#define SPI_CS                  (1 << 2)

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      函数声明
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void SPIInit(unsigned char cs);
void SPIWrite(unsigned char cs, unsigned char *data, unsigned int len);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      电源时钟使能
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void PSCInit()
{
    // 使能 GPIO 模块
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinMuxSet()
{
    // 配置相应的 GPIO 口功能为普通输入输出口
    // RESET
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) &
                                                       (~(SYSCFG_PINMUX7_PINMUX7_7_4))) |
                                                        ((SYSCFG_PINMUX7_PINMUX7_7_4_GPIO3_14 << SYSCFG_PINMUX7_PINMUX7_7_4_SHIFT));

    // LDAC
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) &
                                                       (~(SYSCFG_PINMUX0_PINMUX0_3_0))) |
                                                        ((SYSCFG_PINMUX0_PINMUX0_3_0_GPIO0_15 << SYSCFG_PINMUX0_PINMUX0_3_0_SHIFT));

    // RANGE 输出电压范围
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) &
                                                       (~(SYSCFG_PINMUX7_PINMUX7_11_8))) |
                                                        ((SYSCFG_PINMUX7_PINMUX7_11_8_GPIO3_13 << SYSCFG_PINMUX7_PINMUX7_11_8_SHIFT));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinInit()
{
    // 设置 GPIO3[14] / RESET 为输出模式
    GPIODirModeSet(SOC_GPIO_0_REGS, 63, GPIO_DIR_OUTPUT);

    // 设置 LDAC 为输出模式
    GPIODirModeSet(SOC_GPIO_0_REGS, 16, GPIO_DIR_OUTPUT);
    GPIOPinWrite(SOC_GPIO_0_REGS, 16, GPIO_PIN_HIGH);

    // 设置 GPIO3[13] / RANGE 为输入模式
    GPIODirModeSet(SOC_GPIO_0_REGS, 62, GPIO_DIR_INPUT);
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
//      DAC 复位
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void DACReset()
{
    GPIOPinWrite(SOC_GPIO_0_REGS, 63, GPIO_PIN_LOW);
    Delay(0x1FFF);
    GPIOPinWrite(SOC_GPIO_0_REGS, 63, GPIO_PIN_HIGH);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      DAC 初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
#define AD5676CMD_WRITE  (1 << 20)             // 0001B
#define AD5676CMD_WRUP   (3 << 20)             // 0011B
#define AD5676CMD_PWDN   (4 << 20)             // 0100B
#define AD5676CMD_RESET  (6 << 20)             // 0110B

#define AD5676CH0        (0)                   // 通道 0
#define AD5676CH1        (1 << 16)             // 通道 1
#define AD5676CH2        (2 << 16)             // 通道 2
#define AD5676CH3        (3 << 16)             // 通道 3
#define AD5676CH4        (4 << 16)             // 通道 4
#define AD5676CH5        (5 << 16)             // 通道 5
#define AD5676CH6        (6 << 16)             // 通道 6
#define AD5676CH7        (7 << 16)             // 通道 7

void AD5676RInit()
{
    unsigned int data;

    // 软件复位
    data = AD5676CMD_RESET;
    SPIWrite(SPI_CS, (unsigned char *)&data, 3);

    // 打开所有通道
    data = AD5676CMD_PWDN;
    SPIWrite(SPI_CS, (unsigned char *)&data, 3);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      获取输出电压范围
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static unsigned char DACRangeGet()
{
    // 1 5V
    // 0 2.5V
    return GPIOPinRead(SOC_GPIO_0_REGS, 62);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
#define pi  3.1415926535897932384626433832795  // π
#define Fs  1000.0                             // 采样频率

void main()
{
    UARTStdioInit();
    UARTprintf("\r\nCoreKernel AD5676 Example...\r\n");

    // 使能外设
    PSCInit();

    // 管脚复用配置
    GPIOBankPinMuxSet();

    // GPIO 管脚初始化
    GPIOBankPinInit();

    // SPI 初始化
    SPIInit(SPI_CS);

    // 复位 DAC
    DACReset();

    // DAC 初始化
    AD5676RInit();

    // 检测输出电压范围设置
    char DACRange = DACRangeGet();
    char *DACRangStr[2] = {"2.5V", "5V"};
    UARTprintf("DAC Output Range %s\r\n", DACRangStr[DACRange]);

    // 产生函数
    // 方波(50 点每周期 20 周期)
    unsigned short SquareWave[1000];
    unsigned int i, j;

    for(i = 0; i < 20; i++)
    {
        for(j = 0; j < 25; j++)
        {
            SquareWave[i * 50 + j] = 65535;
            SquareWave[i * 50 + j + 25] = 0;
        }
    }

    unsigned int data;

    for(;;)
    {
        // 拉低 LDAC 管脚
//      GPIOPinWrite(SOC_GPIO_0_REGS, 16, GPIO_PIN_LOW);

        for(i = 0; i < 1000; i++)
        {
            data = AD5676CMD_WRITE | AD5676CH0 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH1 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH2 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH3 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH4 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH5 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH6 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH7 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);
        }
    }
}
