/****************************************************************************/
/*                                                                          */
/*    新核科技(广州)有限公司                                                */
/*                                                                          */
/*    Copyright (C) 2022 CoreKernel Technology (Guangzhou) Co., Ltd         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*    LCD 触摸屏                                                            */
/*                                                                          */
/*    2022年03月27日                                                        */
/*                                                                          */
/****************************************************************************/
/*
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include <stdio.h>

#include "hw_types.h"
#include "hw_syscfg0_C6748.h"
#include "hw_psc_C6748.h"

#include "soc_C6748.h"
#include "psc.h"
#include "gpio.h"
#include "i2c.h"

#include "interrupt.h"

#include "Log.h"

#include "Touch.h"

/****************************************************************************/
/*                                                                          */
/*              宏定义                                                      */
/*                                                                          */
/****************************************************************************/
// GPIO 引脚
#define TouchResetPin          98
#define TouchIntPin            139

// I2C 设备地址
#define ADDRESS                0x5D

// 触控芯片寄存器
#define GT9XX_CFG_VER          0x8047     // 配置版本
#define GT9XX_PRODUCT_ID       0x8140     // 产品 ID 寄存器 0x8140(LSB) 0x8141 0x8142 0x8143(MSB)
#define GT9XX_C00RDINATE_INFO  0x814E     // 坐标信息

// 触控点数
#define TouchMax               5

/****************************************************************************/
/*                                                                          */
/*              全局变量                                                    */
/*                                                                          */
/****************************************************************************/
stTouchInfo TouchInfo;

/****************************************************************************/
/*                                                                          */
/*              GPIO 管脚复用配置                                           */
/*                                                                          */
/****************************************************************************/
static void GPIOBankPinMuxSet()
{
    // GPIO6[01] CTP_RESET
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(19)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(19)) & (~(SYSCFG_PINMUX19_PINMUX19_23_20))) |
                                                    (SYSCFG_PINMUX19_PINMUX19_23_20_GPIO6_1 << SYSCFG_PINMUX19_PINMUX19_23_20_SHIFT);

    // GPIO8[10] CTP_INT
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(18)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(18)) & (~(SYSCFG_PINMUX18_PINMUX18_31_28))) |
                                                    (SYSCFG_PINMUX18_PINMUX18_31_28_GPIO8_10 << SYSCFG_PINMUX18_PINMUX18_31_28_SHIFT);
}

/****************************************************************************/
/*                                                                          */
/*              复位                                                        */
/*                                                                          */
/****************************************************************************/
static void TouchReset()
{
    // 复位并配置 I2C 设备地址为 0x5D(写地址 0xBA 读地址 0xBB)
    GPIODirModeSet(SOC_GPIO_0_REGS, TouchResetPin, GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_0_REGS, TouchIntPin, GPIO_DIR_OUTPUT);

    GPIOPinWrite(SOC_GPIO_0_REGS, TouchResetPin, GPIO_PIN_HIGH);
    GPIOPinWrite(SOC_GPIO_0_REGS, TouchIntPin, GPIO_PIN_LOW);
    Delay(0x00FFFFFF);  // 超过 100us

    // 复位
    GPIOPinWrite(SOC_GPIO_0_REGS, TouchResetPin, GPIO_PIN_LOW);
    Delay(0x00FFFFFF);  // 超过 5ms

    GPIOPinWrite(SOC_GPIO_0_REGS, TouchResetPin, GPIO_PIN_HIGH);
    Delay(0x00FFFFFF);

    // 配置中断引脚为输入
    GPIODirModeSet(SOC_GPIO_0_REGS, TouchIntPin, GPIO_DIR_INPUT);
    Delay(0x00FFFFFF);

    // 配置中断触发方式
    GPIOIntTypeSet(SOC_GPIO_0_REGS, TouchIntPin, GPIO_INT_TYPE_RISEDGE);

    // 使能 GPIO BANK 中断
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 8);
}

/****************************************************************************/
/*                                                                          */
/*              获取触摸坐标                                                */
/*                                                                          */
/****************************************************************************/
void TouchXYGet()
{
    // 设置从设备地址
    I2CMasterSlaveAddrSet(SOC_I2C_0_REGS, ADDRESS);

    unsigned char val[TouchMax * 8 + 1];

    // 读取触摸状态
    val[0] = I2CHWRegRead(SOC_I2C_0_REGS, GT9XX_C00RDINATE_INFO);
    TouchInfo.Num = val[0] & 0x0F;

    if((TouchInfo.Num >= 1) && (TouchInfo.Num <= TouchMax))
    {
        // 读取触摸坐标
        int i;
        for(i = 0; i < 8 * TouchInfo.Num; i++)
        {
            val[i] = I2CHWRegRead(SOC_I2C_0_REGS, GT9XX_C00RDINATE_INFO + 1 + i);
        }

        // 写入坐标
        for(i = 0; i < TouchInfo.Num; i++)
        {
            TouchInfo.X[i] = (val[i * 8 + 2] << 8) | val[i * 8 + 1];
            TouchInfo.Y[i] = (val[i * 8 + 4] << 8) | val[i * 8 + 3];
        }

        TouchInfo.Flag = true;
    }
    else
    {
        TouchInfo.Flag = false;
    }

    // 清除标志位
    I2CHWRegWrite(SOC_I2C_0_REGS, GT9XX_C00RDINATE_INFO, 0);
}

/****************************************************************************/
/*                                                                          */
/*              触摸屏中断服务函数                                          */
/*                                                                          */
/****************************************************************************/
void TouchISR()
{
    GPIOBankIntDisable(SOC_GPIO_0_REGS, 8);

    if(GPIOPinIntStatus(SOC_GPIO_0_REGS, TouchIntPin) == GPIO_INT_PEND)
    {
        TouchXYGet();
    }

    GPIOPinIntClear(SOC_GPIO_0_REGS, TouchIntPin);

    GPIOBankIntEnable(SOC_GPIO_0_REGS, 8);
}

/****************************************************************************/
/*                                                                          */
/*              中断初始化                                   */
/*                                                                          */
/****************************************************************************/
static void InterruptInit()
{
    IntRegister(C674X_MASK_INT4, TouchISR);
    IntEventMap(C674X_MASK_INT4, SYS_INT_GPIO_B8INT);
    IntEnable(C674X_MASK_INT4);
}

/****************************************************************************/
/*                                                                          */
/*              触摸屏初始化                                                */
/*                                                                          */
/****************************************************************************/
void TouchInit()
{
    // 使能外设
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // 管脚复用配置
    GPIOBankPinMuxSet();

    // 复位并配置 I2C 设备地址
    TouchReset();

    // I2C 配置
    I2CInit(SOC_I2C_0_REGS, ADDRESS);

    // 中断初始化
    InterruptInit();

    // 读取触控芯片信息
    ConsoleWrite("\r\nTouch IC Info\r\n");
    unsigned char val[16];
    val[0] = I2CHWRegRead(SOC_I2C_0_REGS, GT9XX_CFG_VER);
    ConsoleWrite("Config Version: 0x%x\n", val[0]);

    int i;
    for(i = 0; i < 11; i++)
    {
        val[i] = I2CHWRegRead(SOC_I2C_0_REGS, GT9XX_PRODUCT_ID + i);
    }

    char str[64];
    sprintf(str, "Product ID: GT%c%c%c%c", val[0], val[1], val[2], val[3]);
    ConsoleWrite("%s\n", str);

    ConsoleWrite("Firmware Version: 0x%x\n", (val[5] << 8) | val[4]);
    ConsoleWrite("X/Y Coordinate Resolution: %dx%d\n", (val[7] << 8) | val[6], (val[9] << 8) | val[8]);
    ConsoleWrite("Vendor ID: 0x%x\r\n\r\n", val[10]);
}
