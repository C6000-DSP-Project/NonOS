// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      SPI LCD
//
//      2022年05月12日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    SPI LCD 显示(0.96 英寸 ST7735S 驱动芯片)
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

#include "interrupt.h"

#include "SoftSPI.h"

#include "LCD.h"
#include "Pic.h"

#include "delay.h"

#include <stdio.h>

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
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    // DSP 中断初始化
    InterruptInit();

    // 定时器延时
    SysDelayTimerSetup();

    // LCD 初始化
    LCDInit();
    LCDFill(0, 0, LCD_WIDTH, LCD_HEIGHT, WHITE);

    char str[64];
    float t = 0;

    LCDChineseDraw(5, 2, "希望缄默", LIGHTBLUE, WHITE, FONT16, 4, 16, 0);

    LCDLineDraw(5, 35, 40, 35, LGRAYBLUE);
    LCDCircleDraw(60, 28, 8, GREEN);
    LCDRectangleDraw(50, 38, 70, 48, MAGENTA);

    LCDStringDraw(5, 20, "Hello", BLACK, WHITE, 12, 0);
    LCDStringDraw(5, 40, "World!", BLACK, WHITE, 12, 0);

    LCDPictureDraw(80, 0, 80, 80, gImage_Rabbit);

    for(;;)
    {
        sprintf(str, "%.2f", t);
        LCDStringDraw(0, 50, str, RED, WHITE, 32, 0);
        t += 0.015;
    }
}
