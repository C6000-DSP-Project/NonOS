// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      温湿度传感器(I2C 查询模式)
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    获取温湿度值
 *
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include "soc_C6748.h"

#include "i2c.h"

#include "uartStdio.h"

#include <stdio.h>

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
extern volatile unsigned char I2CData[10];

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      函数声明
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
extern void I2CInit(unsigned int baseAddr, unsigned int slaveAddr);
extern void I2CRcvBlocking(unsigned int baseAddr, unsigned int dataCnt);
extern void I2CSendBlocking(unsigned int baseAddr, unsigned int dataCnt);

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
//      CRC 校验
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CRC8 多项式为 x^8 + x^5 + x^4 + 1
// 校验码为 0x31

unsigned char CRC8(const unsigned char *data, int len)
{
     unsigned char crc = 0xFF;

     int i, j;
     for(i = 0; i < len; i++)
     {
        crc ^= *data++;

        for(j = 0; j < 8; j++)
        {
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
        }
     }

     return crc;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      获取温湿度值
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void TempSensorGet(float *t, float *rh)
{
    // 设置从设备地址
    I2CMasterSlaveAddrSet(SOC_I2C_0_REGS, 0x44);

    // 访问结果寄存器
    I2CData[0] = 0xE0;
    I2CData[1] = 0x00;
    I2CSendBlocking(SOC_I2C_0_REGS, 2);

    // 读取数据
    I2CRcvBlocking(SOC_I2C_0_REGS, 6);

    // 换算
    unsigned short val;

    // 温度
    val = (I2CData[0] << 8) | I2CData[1];
    *t = -45 + 175 * (val * 1.0 / 65535);

    // 湿度
    val = (I2CData[3] << 8) | I2CData[4];
    *rh = 100 * (val * 1.0 / 65535);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      温湿度传感器初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void TempSensorInit()
{
    // I2C 配置
    I2CInit(SOC_I2C_0_REGS, 0x44);

    // 周期模式（1秒）
    I2CData[0] = 0x21;
    I2CData[1] = 0x30;
    I2CSendBlocking(SOC_I2C_0_REGS, 2);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    UARTStdioInit();
    UARTprintf("\r\nCoreKernel SHT30 Example...\r\n");

    // 温湿度传感器初始化
    TempSensorInit();

    char str[64];
    float t, rh;

    for(;;)
    {
        // 延时(非精确)
        Delay(0x0FFFFFFF);

        // 温度/湿度(测量间隔需要超过 1s)
        TempSensorGet(&t, &rh);
        sprintf(str, "Temperature %2.2f°C", t);
        UARTprintf("%s\r\n", str);

        sprintf(str, "Humidity %2.2f%%", rh);
        UARTprintf("%s\r\n\r\n", str);
    }
}
