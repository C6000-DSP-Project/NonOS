// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      AD7606 ADC EDMA 模式
//
//      2022年04月25日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    定时器启动 ADC 转换 转换完成后 BUSY 信号触发 EDMA3 读取数据
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
#include "timer.h"

#include "interrupt.h"

#include "uartStdio.h"

#include <stdio.h>

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ADC 数据
short ADCDataCH0[1024];
short ADCDataCH1[1024];
short ADCDataCH2[1024];
short ADCDataCH3[1024];
short ADCDataCH4[1024];
short ADCDataCH5[1024];
short ADCDataCH6[1024];
short ADCDataCH7[1024];

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      函数声明
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
extern void AD7606EDMA3Init();

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      电源时钟使能
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void PSCInit()
{
    // 使能 GPIO 模块
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // 使能 EMIFA 模块
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_EMIFA, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinMuxSet()
{
    // 配置相应的 GPIO 口功能为 EMIFA 端口
    // EMIFA D0-D15
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(9)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(9)) &
                                                   (~(SYSCFG_PINMUX9_PINMUX9_31_28  |
                                                      SYSCFG_PINMUX9_PINMUX9_27_24  |
                                                      SYSCFG_PINMUX9_PINMUX9_23_20  |
                                                      SYSCFG_PINMUX9_PINMUX9_19_16  |
                                                      SYSCFG_PINMUX9_PINMUX9_15_12  |
                                                      SYSCFG_PINMUX9_PINMUX9_11_8   |
                                                      SYSCFG_PINMUX9_PINMUX9_7_4    |
                                                      SYSCFG_PINMUX9_PINMUX9_3_0))) |
                                                    ((SYSCFG_PINMUX9_PINMUX9_31_28_EMA_D0 << SYSCFG_PINMUX9_PINMUX9_31_28_SHIFT) |
                                                     (SYSCFG_PINMUX9_PINMUX9_27_24_EMA_D1 << SYSCFG_PINMUX9_PINMUX9_27_24_SHIFT) |
                                                     (SYSCFG_PINMUX9_PINMUX9_23_20_EMA_D2 << SYSCFG_PINMUX9_PINMUX9_23_20_SHIFT) |
                                                     (SYSCFG_PINMUX9_PINMUX9_19_16_EMA_D3 << SYSCFG_PINMUX9_PINMUX9_19_16_SHIFT) |
                                                     (SYSCFG_PINMUX9_PINMUX9_15_12_EMA_D4 << SYSCFG_PINMUX9_PINMUX9_15_12_SHIFT) |
                                                     (SYSCFG_PINMUX9_PINMUX9_11_8_EMA_D5  << SYSCFG_PINMUX9_PINMUX9_11_8_SHIFT)  |
                                                     (SYSCFG_PINMUX9_PINMUX9_7_4_EMA_D6   << SYSCFG_PINMUX9_PINMUX9_7_4_SHIFT)   |
                                                     (SYSCFG_PINMUX9_PINMUX9_3_0_EMA_D7   << SYSCFG_PINMUX9_PINMUX9_3_0_SHIFT));

    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(8)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(8)) &
                                                   (~(SYSCFG_PINMUX8_PINMUX8_31_28  |
                                                      SYSCFG_PINMUX8_PINMUX8_27_24  |
                                                      SYSCFG_PINMUX8_PINMUX8_23_20  |
                                                      SYSCFG_PINMUX8_PINMUX8_19_16  |
                                                      SYSCFG_PINMUX8_PINMUX8_15_12  |
                                                      SYSCFG_PINMUX8_PINMUX8_11_8   |
                                                      SYSCFG_PINMUX8_PINMUX8_7_4    |
                                                      SYSCFG_PINMUX8_PINMUX8_3_0))) |
                                                    ((SYSCFG_PINMUX8_PINMUX8_31_28_EMA_D8  << SYSCFG_PINMUX8_PINMUX8_31_28_SHIFT) |
                                                     (SYSCFG_PINMUX8_PINMUX8_27_24_EMA_D9  << SYSCFG_PINMUX8_PINMUX8_27_24_SHIFT) |
                                                     (SYSCFG_PINMUX8_PINMUX8_23_20_EMA_D10 << SYSCFG_PINMUX8_PINMUX8_23_20_SHIFT) |
                                                     (SYSCFG_PINMUX8_PINMUX8_19_16_EMA_D11 << SYSCFG_PINMUX8_PINMUX8_19_16_SHIFT) |
                                                     (SYSCFG_PINMUX8_PINMUX8_15_12_EMA_D12 << SYSCFG_PINMUX8_PINMUX8_15_12_SHIFT) |
                                                     (SYSCFG_PINMUX8_PINMUX8_11_8_EMA_D13  << SYSCFG_PINMUX8_PINMUX8_11_8_SHIFT)  |
                                                     (SYSCFG_PINMUX8_PINMUX8_7_4_EMA_D14   << SYSCFG_PINMUX8_PINMUX8_7_4_SHIFT)   |
                                                     (SYSCFG_PINMUX8_PINMUX8_3_0_EMA_D15   << SYSCFG_PINMUX8_PINMUX8_3_0_SHIFT));

    // CS2
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) &
                                                   (~(SYSCFG_PINMUX7_PINMUX7_3_0))) |
                                                    ((SYSCFG_PINMUX7_PINMUX7_3_0_NEMA_CS2 << SYSCFG_PINMUX7_PINMUX7_3_0_SHIFT));

    // 配置相应的 GPIO 口功能为普通输入输出口
    // RESET
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                        (~(SYSCFG_PINMUX11_PINMUX11_15_12))) |
                                                         ((SYSCFG_PINMUX11_PINMUX11_15_12_GPIO5_12 << SYSCFG_PINMUX11_PINMUX11_15_12_SHIFT));

    // CONVSTA 或 CONVST
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                        (~(SYSCFG_PINMUX11_PINMUX11_11_8))) |
                                                         ((SYSCFG_PINMUX11_PINMUX11_11_8_GPIO5_13 << SYSCFG_PINMUX11_PINMUX11_11_8_SHIFT));

    // CONVSTB 或 WE
    // AD7606 CONVSTA 转换前 4 通道 CONVSTB 转换后 4 通道
    // AD7606B CONVST 转换所有通道 WE 用于写内部软件寄存器
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) &
                                                       (~(SYSCFG_PINMUX7_PINMUX7_19_16))) |
                                                        ((SYSCFG_PINMUX7_PINMUX7_19_16_GPIO3_11 << SYSCFG_PINMUX7_PINMUX7_19_16_SHIFT));

    // BUSY
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                        (~(SYSCFG_PINMUX11_PINMUX11_19_16))) |
                                                         ((SYSCFG_PINMUX11_PINMUX11_19_16_GPIO5_11 << SYSCFG_PINMUX11_PINMUX11_19_16_SHIFT));

    // FRSTDATA
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                        (~(SYSCFG_PINMUX11_PINMUX11_23_20))) |
                                                         ((SYSCFG_PINMUX11_PINMUX11_23_20_GPIO5_10 << SYSCFG_PINMUX11_PINMUX11_23_20_SHIFT));

    // RANGE 输入电压范围
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(6)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(6)) &
                                                       (~(SYSCFG_PINMUX7_PINMUX7_15_12))) |
                                                        ((SYSCFG_PINMUX7_PINMUX7_15_12_GPIO3_12 << SYSCFG_PINMUX7_PINMUX7_15_12_SHIFT));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinInit()
{
    // 设置 GPIO5[11] / BUSY 为输入模式
    GPIODirModeSet(SOC_GPIO_0_REGS, 92, GPIO_DIR_INPUT);

    // 配置中断触发方式
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 92, GPIO_INT_TYPE_RISEDGE);

    // 使能 GPIO BANK 中断
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 5);

    // 设置 GPIO5[12] / RESET 为输出模式
    GPIODirModeSet(SOC_GPIO_0_REGS, 93, GPIO_DIR_OUTPUT);
    GPIOPinWrite(SOC_GPIO_0_REGS, 93, GPIO_PIN_HIGH);

    // 设置 GPIO5[13] / CONVSTA 为输出模式
    GPIODirModeSet(SOC_GPIO_0_REGS, 94, GPIO_DIR_OUTPUT);
    GPIOPinWrite(SOC_GPIO_0_REGS, 94, GPIO_PIN_LOW);

    // 设置 GPIO3[11] / CONVSTB 为输出模式
    GPIODirModeSet(SOC_GPIO_0_REGS, 60, GPIO_DIR_OUTPUT);
    GPIOPinWrite(SOC_GPIO_0_REGS, 60, GPIO_PIN_LOW);

    // 设置 GPIO3[12] / RANGE 为输入模式
    GPIODirModeSet(SOC_GPIO_0_REGS, 61, GPIO_DIR_INPUT);
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
//      ADC 复位
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void ADCReset()
{
    GPIOPinWrite(SOC_GPIO_0_REGS, 93, GPIO_PIN_HIGH);
    Delay(0x1FFF);
    GPIOPinWrite(SOC_GPIO_0_REGS, 93, GPIO_PIN_LOW);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      ADC 启动
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void ADCStart()
{
    // CONVSTA
    GPIOPinWrite(SOC_GPIO_0_REGS, 94, GPIO_PIN_LOW);
    GPIOPinWrite(SOC_GPIO_0_REGS, 60, GPIO_PIN_LOW);

    // CONVSTB
    GPIOPinWrite(SOC_GPIO_0_REGS, 94, GPIO_PIN_HIGH);
    GPIOPinWrite(SOC_GPIO_0_REGS, 60, GPIO_PIN_HIGH);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      获取输入电压范围
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static unsigned char ADCRangeGet()
{
    // 1 -10V - +10V
    // 0 -5V - +5V
    return GPIOPinRead(SOC_GPIO_0_REGS, 61);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      EMIF 初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void EMIFInit()
{
    // 配置数据总线 16bit
    EMIFAAsyncDevDataBusWidthSelect(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_DATA_BUSWITTH_16BIT);

    // 选择 Normal 模式
    EMIFAAsyncDevOpModeSelect(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_ASYNC_INTERFACE_NORMAL_MODE);

    // 禁止WAIT引脚
    EMIFAExtendedWaitConfig(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_EXTENDED_WAIT_DISABLE);

    // 配置 W_SETUP/R_SETUP W_STROBE/R_STROBE W_HOLD/R_HOLD   TA 等参数
    EMIFAWaitTimingConfig(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_ASYNC_WAITTIME_CONFIG(4, 6, 4, 4, 6, 4, 0));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      中断服务函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
short EMIFData[8];
void TimerIsr()
{
    // 清除中断标志
    IntEventClear(SYS_INT_T64P2_TINTALL);

    // 清除定时器中断标志
    TimerIntStatusClear(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);

//    EMIFData[0] = ((short *)SOC_EMIFA_CS2_ADDR)[1];
//    EMIFData[1] = ((short *)SOC_EMIFA_CS2_ADDR)[2];
//    EMIFData[2] = ((short *)SOC_EMIFA_CS2_ADDR)[3];
//    EMIFData[3] = ((short *)SOC_EMIFA_CS2_ADDR)[4];
//    EMIFData[4] = ((short *)SOC_EMIFA_CS2_ADDR)[5];
//    EMIFData[5] = ((short *)SOC_EMIFA_CS2_ADDR)[6];
//    EMIFData[6] = ((short *)SOC_EMIFA_CS2_ADDR)[7];
//    EMIFData[7] = ((short *)SOC_EMIFA_CS2_ADDR)[8];

    // 启动 ADC
    ADCStart();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      定时器/计数器初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void TimerInit(unsigned int SamplingRate)
{
    // 采样率最高 200KSPS
    if(SamplingRate > 200000)
    {
        SamplingRate = 200000;
    }

    // 配置定时器/计数器 2 为 64 位模式
    TimerConfigure(SOC_TMR_2_REGS, TMR_CFG_64BIT_CLK_INT);

    // 设置周期
    TimerPeriodSet(SOC_TMR_2_REGS, TMR_TIMER12, 228000000 / SamplingRate);
    TimerPeriodSet(SOC_TMR_2_REGS, TMR_TIMER34, 0);

    // 使能定时器/计数器 2
    TimerEnable(SOC_TMR_2_REGS, TMR_TIMER12, TMR_ENABLE_CONT);

    // 使能定时器/计数器中断
    TimerIntEnable(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);
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
    IntRegister(C674X_MASK_INT4, TimerIsr);

    // 映射中断到 DSP 可屏蔽中断
    IntEventMap(C674X_MASK_INT4, SYS_INT_T64P2_TINTALL);

    // 使能 DSP 可屏蔽中断
    IntEnable(C674X_MASK_INT4);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    UARTStdioInit();
    UARTprintf("\r\nCoreKernel AD7606 EDMA Example...\r\n");

    // 使能外设
    PSCInit();

    // 管脚复用配置
    GPIOBankPinMuxSet();

    // GPIO 管脚初始化
    GPIOBankPinInit();

    // DSP 中断初始化
    InterruptInit();

    // EMIF 初始化
    EMIFInit();

    // 复位 ADC
    ADCReset();

    // EDMA3 初始化
    AD7606EDMA3Init();

    // 定时器初始化 200KHz 采样率
    TimerInit(200000);

    // 检测输入电压范围设置
    char ADCRange = ADCRangeGet();
    char *ADCRangStr[2] = {"-5V - +5V", "-10V - +10V"};
    UARTprintf("ADC Input Range %s\r\n", ADCRangStr[ADCRange]);

    for(;;)
    {

    }
}
