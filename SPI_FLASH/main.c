// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      SPI NOR FLASH
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    SPI FLASH 读写
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

#include "spi.h"

#include "interrupt.h"

#include "uartStdio.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SPI 管脚配置
#define SPI_CS                  (1 << 0)

// FLASH 地址
#define SPI_FLASH_ADDR_MSB1     0x0A
#define SPI_FLASH_ADDR_MSB0     0x00
#define SPI_FLASH_ADDR_LSB      0x00

// FLASH 命令
#define SPI_FLASH_SECTOR_ERASE  0xD8  // 扇区擦除命令
#define SPI_FLASH_PAGE_WRITE    0x02  // 页写入命令
#define SPI_FLASH_STATUS_RX     0x05  // 读状态寄存器命令
#define SPI_FLASH_WRITE_EN      0x06  // 写使能命令
#define SPI_FLASH_READ          0x03  // 读命令
#define WRITE_IN_PROGRESS       0x01  // 写操作执行中

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
extern volatile unsigned int spi_flag;

extern unsigned int tx_len;
extern unsigned int rx_len;
extern unsigned char tx_data[260];
extern unsigned char rx_data[260];

unsigned char vrf_data[260];

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      函数声明
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void SPIInit();
void SpiTransfer();
void SPIIsr();

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      读状态寄存器
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void StatusGet()
{
    tx_data[0] = SPI_FLASH_STATUS_RX;
    tx_len = rx_len = 2;
    SPIDat1Config(SOC_SPI_1_REGS, (SPI_CSHOLD | SPI_DATA_FORMAT0), SPI_CS);
    SpiTransfer();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      写使能
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void WriteEnable()
{
    tx_data[0] = SPI_FLASH_WRITE_EN;
    tx_len = rx_len = 1;
    SPIDat1Config(SOC_SPI_1_REGS, (SPI_CSHOLD | SPI_DATA_FORMAT0), SPI_CS);
    SpiTransfer();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      忙检测
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void IsFlashBusy()
{
    do{
         StatusGet();

      }while(rx_data[1] & WRITE_IN_PROGRESS);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      扇区擦除
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void SectorErase()
{
    tx_data[0] =  SPI_FLASH_SECTOR_ERASE;
    tx_data[1] =  SPI_FLASH_ADDR_MSB1;
    tx_data[2] =  SPI_FLASH_ADDR_MSB0;
    tx_data[3] =  SPI_FLASH_ADDR_LSB;

    tx_len = rx_len = 4;
    SPIDat1Config(SOC_SPI_1_REGS, (SPI_CSHOLD | SPI_DATA_FORMAT0), SPI_CS);
    SpiTransfer();

    IsFlashBusy();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      写扇区
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void WriteFlash()
{
    unsigned int index;

    tx_data[0] =  SPI_FLASH_PAGE_WRITE;
    tx_data[1] =  SPI_FLASH_ADDR_MSB1;
    tx_data[2] =  SPI_FLASH_ADDR_MSB0;
    tx_data[3] =  SPI_FLASH_ADDR_LSB;

    // 准备要写入的数据
    for (index = 4; index < 260; index++)
    {
        tx_data[index] =  index;
    }

    for(index = 4; index < 260; index++)
    {
         vrf_data[index] = index;
    }

    tx_len = rx_len = index;
    SPIDat1Config(SOC_SPI_1_REGS, (SPI_CSHOLD | SPI_DATA_FORMAT0), SPI_CS);
    SpiTransfer();

    IsFlashBusy();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      读数据
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void ReadFlash()
{
    unsigned int index;

    tx_data[0] =  SPI_FLASH_READ;
    tx_data[1] =  SPI_FLASH_ADDR_MSB1;
    tx_data[2] =  SPI_FLASH_ADDR_MSB0;
    tx_data[3] =  SPI_FLASH_ADDR_LSB;

    // 情况变量
    for (index = 4; index < 260; index++)
    {
        tx_data[index] =  0;
    }

    tx_len = rx_len = index;
    SPIDat1Config(SOC_SPI_1_REGS, (SPI_CSHOLD | SPI_DATA_FORMAT0), SPI_CS);
    SpiTransfer();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      数据校验
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
int VerifyData()
{
    unsigned int index;

    for(index = 4; index < 260; index++)
    {
        if(vrf_data[index] != rx_data[index])
        {
            UARTPuts("\r\n", -1);
            UARTPuts("VerifyData: Comparing the data written to and read", -1);
            UARTPuts(" from Flash.\r\nThe two data blocks are unequal.", -1);
            UARTPuts(" Mismatch found at index ", -1);
            UARTPutNum((int)index - 3);
            UARTPuts("\r\n", -1);
            UARTPuts("Verify Failed.\r\n", -1);
            return 0;
        }
    }

    if (index == 260)
    {
        UARTPuts("\r\nThe data in the Flash and the one written ", -1);
        UARTPuts("to it are equal.\r\n", -1);
        UARTPuts("Verify successfully.\r\n", -1);
        return 1;
    }

    return 0;
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
    IntRegister(C674X_MASK_INT4, SPIIsr);

    // 映射中断到 DSP 可屏蔽中断
    IntEventMap(C674X_MASK_INT4, SYS_INT_SPI1_INT);

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
    // 串口初始化
    UARTStdioInit();

    UARTprintf("\r\nCoreKernel SPI FLASH Example...\r\n");

    // SPI 初始化
    SPIInit(SPI_CS);

    // DSP 中断初始化
    InterruptInit();

    // 写使能
    WriteEnable();

    UARTPuts("Do you want to erase a sector of the flash before writing to it ?.", -1);
    UARTPuts("\r\nInput y(Y)/n(N) to proceed.\r\n", -1);

    char ch = UARTGetc();
    UARTPutc(ch);

    if((ch == 'y') || (ch == 'Y'))
    {
        UARTPuts("\r\n", -1);

        // 擦除 Flash
        SectorErase();
    }

    // 写 Flash
    WriteEnable();
    WriteFlash();

    // 读 Flash
    ReadFlash();

    // 数据校验
    VerifyData();

    for(;;)
    {

    }
}
