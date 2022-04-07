/****************************************************************************/
/*                                                                          */
/*    新核科技(广州)有限公司                                                */
/*                                                                          */
/*    Copyright (C) 2022 CoreKernel Technology (Guangzhou) Co., Ltd         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*    LCD LVGL 图形库                                         */
/*                                                                          */
/*    2022年04月08日                                                        */
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
#include "hw_types.h"
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "psc.h"

#include "interrupt.h"

#include "uartStdio.h"

#include "LCD.h"
#include "Touch.h"

#include "LVGL/lvgl.h"
#include "LVGL/porting/lv_port_disp_c674x.h"
#include "LVGL/porting/lv_port_indev_c674x.h"

/****************************************************************************/
/*                                                                          */
/*              函数声明                                                    */
/*                                                                          */
/****************************************************************************/
void TimerInit();

/****************************************************************************/
/*                                                                          */
/*              延时（指令方式）                                            */
/*                                                                          */
/****************************************************************************/
void Delay(volatile unsigned int delay)
{
    while(delay--);
}

/****************************************************************************/
/*                                                                          */
/*              DSP 中断初始化                                              */
/*                                                                          */
/****************************************************************************/
static void InterruptInit()
{
    // 初始化 DSP 中断控制器
    IntDSPINTCInit();

    // 使能 DSP 全局中断
    IntGlobalEnable();
}

/****************************************************************************/
/*                                                                          */
/*              主函数                                                      */
/*                                                                          */
/****************************************************************************/
int main()
{
    UARTStdioInit();
    UARTPuts("LCD LVGL...\r\n\r\n", -1);

    // DSP 中断初始化
    InterruptInit();

    // LCD 初始化
    LCDInit();

    // 触摸屏初始化
    TouchInit();

    // 定时器初始化
    TimerInit();

    // LVGL 图形库
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    lv_example_get_started_1();
//        lv_demo_benchmark();

    for(;;)
    {
        lv_timer_handler();
        Delay(0xFFFFF);
    }
}
