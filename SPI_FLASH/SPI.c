// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      SPI API 接口（中断模式）
//
//      2022年04月20日
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
#include "hw_psc_C6748.h"

#include "soc_C6748.h"

#include "psc.h"
#include "spi.h"

#include "interrupt.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
volatile unsigned int spi_flag = 1;

unsigned int tx_len;
unsigned int rx_len;
unsigned char tx_data[260];
unsigned char rx_data[260];
unsigned char *p_tx;
unsigned char *p_rx;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void SPI1CSPinMuxSet(unsigned int csPinNum)
{
     switch(csPinNum)
     {
          case 0:
              HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) & (~(SYSCFG_PINMUX5_PINMUX5_7_4))) |
                                                             (SYSCFG_PINMUX5_PINMUX5_7_4_NSPI1_SCS0 << SYSCFG_PINMUX5_PINMUX5_7_4_SHIFT);

              break;

          case 1:
              HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) & (~(SYSCFG_PINMUX5_PINMUX5_3_0))) |
                                                             (SYSCFG_PINMUX5_PINMUX5_3_0_NSPI1_SCS1 << SYSCFG_PINMUX5_PINMUX5_3_0_SHIFT);

              break;

          case 2:
              HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) & (~(SYSCFG_PINMUX4_PINMUX4_31_28))) |
                                                             (SYSCFG_PINMUX4_PINMUX4_31_28_NSPI1_SCS2 << SYSCFG_PINMUX4_PINMUX4_31_28_SHIFT);

              break;

          case 3:
              HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) & (~(SYSCFG_PINMUX4_PINMUX4_27_24))) |
                                                             (SYSCFG_PINMUX4_PINMUX4_27_24_NSPI1_SCS3 << SYSCFG_PINMUX4_PINMUX4_27_24_SHIFT);

              break;

          case 4:
              HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) & (~(SYSCFG_PINMUX4_PINMUX4_23_20))) |
                                                             (SYSCFG_PINMUX4_PINMUX4_23_20_NSPI1_SCS4 << SYSCFG_PINMUX4_PINMUX4_23_20_SHIFT);

              break;

          case 5:
              HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) & (~(SYSCFG_PINMUX4_PINMUX4_19_16))) |
                                                             (SYSCFG_PINMUX4_PINMUX4_19_16_NSPI1_SCS5 << SYSCFG_PINMUX4_PINMUX4_19_16_SHIFT);

              break;

          case 6:
              HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) & (~(SYSCFG_PINMUX4_PINMUX4_15_12))) |
                                                             (SYSCFG_PINMUX4_PINMUX4_15_12_NSPI1_SCS6 << SYSCFG_PINMUX4_PINMUX4_15_12_SHIFT);

              break;

          case 7:
              HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) & (~(SYSCFG_PINMUX4_PINMUX4_11_8))) |
                                                             (SYSCFG_PINMUX4_PINMUX4_11_8_NSPI1_SCS7 << SYSCFG_PINMUX4_PINMUX4_11_8_SHIFT);

              break;

          default:
              break;
     }
}

static void GPIOBankPinMuxSet()
{
    // SPI1 CLK SIMO SOMI
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) & (~(SYSCFG_PINMUX5_PINMUX5_23_20 | SYSCFG_PINMUX5_PINMUX5_19_16 | SYSCFG_PINMUX5_PINMUX5_15_12 | SYSCFG_PINMUX5_PINMUX5_11_8))) |
                                                   ((SYSCFG_PINMUX5_PINMUX5_11_8_SPI1_CLK << SYSCFG_PINMUX5_PINMUX5_11_8_SHIFT) |
                                                    (SYSCFG_PINMUX5_PINMUX5_23_20_SPI1_SIMO0 << SYSCFG_PINMUX5_PINMUX5_23_20_SHIFT) |
                                                    (SYSCFG_PINMUX5_PINMUX5_19_16_SPI1_SOMI0 << SYSCFG_PINMUX5_PINMUX5_19_16_SHIFT));

    // SPI1 CS
    SPI1CSPinMuxSet(0);  // FLASH
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      中断服务函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void SPIIsr()
{
    IntEventClear(SYS_INT_SPI1_INT);

    unsigned int intCode = 0;
    intCode = SPIInterruptVectorGet(SOC_SPI_1_REGS);

    while(intCode)
    {
        if(intCode == SPI_TX_BUF_EMPTY)
        {
            tx_len--;
            SPITransmitData1(SOC_SPI_1_REGS, *p_tx);
            p_tx++;
            if (!tx_len)
            {
                SPIIntDisable(SOC_SPI_1_REGS, SPI_TRANSMIT_INT);
            }
        }

        if(intCode == SPI_RECV_FULL)
        {
            rx_len--;
            *p_rx = (char)SPIDataReceive(SOC_SPI_1_REGS);
            p_rx++;
            if (!rx_len)
            {
                spi_flag = 0;
                SPIIntDisable(SOC_SPI_1_REGS, SPI_RECV_INT);
            }
        }

        intCode = SPIInterruptVectorGet(SOC_SPI_1_REGS);
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      SPI 传输
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void SpiTransfer()
{
    p_tx = &tx_data[0];
    p_rx = &rx_data[0];
    SPIIntEnable(SOC_SPI_1_REGS, (SPI_RECV_INT | SPI_TRANSMIT_INT));
    while(spi_flag);
    spi_flag = 1;

    SPIDat1Config(SOC_SPI_1_REGS, SPI_DATA_FORMAT0, (1 << 0));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      SPI 数据格式配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void SPIDataFormatConfig(unsigned int dataFormat)
{
    // 配置 SPI 时钟
    SPIConfigClkFormat(SOC_SPI_1_REGS, (SPI_CLK_POL_HIGH | SPI_CLK_INPHASE), dataFormat);

    // 配置 SPI 发送时 MSB 优先
    SPIShiftMsbFirst(SOC_SPI_1_REGS, dataFormat);

    // 设置字符长度
    SPICharLengthSet(SOC_SPI_1_REGS, 0x08, dataFormat);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      SPI 初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void SPIInit(unsigned char cs)
{
    // 使能外设
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_SPI1, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // 管脚复用配置
    GPIOBankPinMuxSet();

    // SPI 复位
    SPIReset(SOC_SPI_1_REGS);
    SPIOutOfReset(SOC_SPI_1_REGS);

    // SPI 模式
    SPIModeConfigure(SOC_SPI_1_REGS, SPI_MASTER_MODE);

    // SPI 时钟
    SPIClkConfigure(SOC_SPI_1_REGS, 228000000, 20000000, SPI_DATA_FORMAT0);

    // SPI 引脚配置
    unsigned int  val = 0x00000E01;
    SPIPinControl(SOC_SPI_1_REGS, 0, 0, &val);

    SPIDefaultCSSet(SOC_SPI_1_REGS, cs);

    // 配置 SPI 数据格式
    SPIDataFormatConfig(SPI_DATA_FORMAT0);

    // 映射中断到 INT1
    SPIIntLevelSet(SOC_SPI_1_REGS, SPI_RECV_INTLVL | SPI_TRANSMIT_INTLVL);

    // 使能 SPI
    SPIEnable(SOC_SPI_1_REGS);
}
