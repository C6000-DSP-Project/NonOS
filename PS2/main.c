// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      PS2 游戏手柄控制
//
//      2022年05月09日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    PS2 游戏手柄控制(软件模拟 SPI 协议)
 *
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include <stdio.h>

#include "PS2.h"

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
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    // PS2 初始化
    PS2Init();

    unsigned short key;
    char *PS2KEYStr[] = {"NONE", "SELECT", "L3", "R3", "START", "UP", "RIGHT", "DOWN", "LEFT",
                                  "L2", "R2", "L1", "R1", "GREEN", "RED", "BLUE", "PINK"};

    for(;;)
    {
        // 获取键值
        key = PS2KEYData();
        printf("PS2 %s KEY PRESSED %5d %5d %5d %5d\r\n", PS2KEYStr[key], PS2AnologData(PSS_LX), PS2AnologData(PSS_LY), PS2AnologData(PSS_RX), PS2AnologData(PSS_RY));

        // 震动
        if(key == PSB_L1)
        {
            PS2Vibration(0, 255);
            Delay(0x00FFFFFF);
        }
        else if(key == PSB_R1)
        {
            PS2Vibration(255, 0);
            Delay(0x00FFFFFF);
        }

        // 延时(非精确)
        Delay(0x000FFFFF);
    }
}
