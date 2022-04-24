// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      通用异步串口 2(DMA 模式)
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    串口收发
 *
 *    注意: EDMA3 仅能访问 DSP 全局 L2RAM 地址(0x11800000) 不能访问 DSP 本地 L2RAM 地址(0x00800000)
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
#include "uart.h"
#include "edma.h"
#include "edma_event.h"

#include "interrupt.h"

#include "string.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 时钟
#define SYSCLK_1_FREQ     (456000000)
#define SYSCLK_2_FREQ     (SYSCLK_1_FREQ / 2)

// EDMA3 参数
#define MAX_ACNT           1
#define MAX_CCNT           1
#define RX_BUFFER_SIZE     20

// EDMA3 通道
#define EVT_QUEUE_NUM      0

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
volatile unsigned int flag = 0;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      函数声明
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void (*cb_Fxn[EDMA3_NUM_TCC])(unsigned int tcc, unsigned int status);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinMuxSet()
{
    // 两线制串口 不使用流控
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(04)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(04)) &
                                                    (~(SYSCFG_PINMUX4_PINMUX4_23_20    |
                                                       SYSCFG_PINMUX4_PINMUX4_19_16))) |
                                                     ((SYSCFG_PINMUX4_PINMUX4_23_20_UART2_TXD << SYSCFG_PINMUX4_PINMUX4_23_20_SHIFT) |
                                                      (SYSCFG_PINMUX4_PINMUX4_19_16_UART2_RXD << SYSCFG_PINMUX4_PINMUX4_19_16_SHIFT));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      发送数据
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void UARTTransmitData(unsigned int tccNum, unsigned int chNum, volatile char *buffer, unsigned int buffLength)
{
    EDMA3CCPaRAMEntry paramSet;

    // 配置参数 RAM
    paramSet.srcAddr = (unsigned int)buffer;
    // 接收缓存寄存器 / 发送保持寄存器 地址
    paramSet.destAddr = SOC_UART_2_REGS;
    paramSet.aCnt = MAX_ACNT;
    paramSet.bCnt = (unsigned short)buffLength;
    paramSet.cCnt = MAX_CCNT;

    // 源索引自增系数 1 即一个字节
    paramSet.srcBIdx = (short)1u;

    // 目标索引自增系数
    paramSet.destBIdx = (short)0u;

    // 异步传输模式
    paramSet.srcCIdx = (short)0u;
    paramSet.destCIdx = (short)0u;
    paramSet.linkAddr = (unsigned short)0xFFFFu;
    paramSet.bCntReload = (unsigned short)0u;
    paramSet.opt = 0x00000000u;
    paramSet.opt |= (EDMA3CC_OPT_DAM );
    paramSet.opt |= ((tccNum << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    // 写参数 RAM
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, chNum, &paramSet);

    // 使能 EDMA3 通道
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

    // 使能串口 DMA
    UARTDMAEnable(SOC_UART_2_REGS, UART_RX_TRIG_LEVEL_1 | UART_DMAMODE | UART_FIFO_MODE);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      接收数据
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void UARTReceiveData(unsigned int tccNum, unsigned int chNum, volatile char *buffer)
{
    EDMA3CCPaRAMEntry paramSet;

    // 配置参数 RAM
    // 接收缓存寄存器 / 发送保持寄存器 地址
    paramSet.srcAddr = SOC_UART_2_REGS;
    paramSet.destAddr = (unsigned int)buffer;
    paramSet.aCnt = MAX_ACNT;
    paramSet.bCnt = RX_BUFFER_SIZE;
    paramSet.cCnt = MAX_CCNT;

    // 源索引自增系数
    paramSet.srcBIdx = 0;
    // 目标索引自增系数 1 即一个字节
    paramSet.destBIdx = 1;

    // 异步模式
    paramSet.srcCIdx = 0;
    paramSet.destCIdx = 0;
    paramSet.linkAddr = (unsigned short)0xFFFFu;
    paramSet.bCntReload = 0;
    paramSet.opt = 0x00000000u;
    paramSet.opt |= ((EDMA3CC_OPT_SAM) << EDMA3CC_OPT_SAM_SHIFT);
    paramSet.opt |= ((tccNum << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    // 写参数 RAM
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, chNum, &paramSet);

    // 使能 EDMA3 通道
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

    // 使能串口 DMA 模式
    UARTDMAEnable(SOC_UART_2_REGS, UART_RX_TRIG_LEVEL_1 | UART_DMAMODE | UART_FIFO_MODE);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      回调函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void EDMA3IntCallback(unsigned int tccNum, unsigned int status)
{
    UARTDMADisable(SOC_UART_2_REGS, (UART_RX_TRIG_LEVEL_1 | UART_FIFO_MODE));

    flag = 1;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      中断服务函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// StarterWare system_config.lib 驱动库内部已使用 interrupt 关键字修饰 此处仅为回调函数
void EDMA3ComplHandlerIsr(void)
{
    volatile unsigned int pendingIrqs;
    volatile unsigned int isIPR = 0;

    unsigned int indexl;
    unsigned int Cnt = 0;

    indexl = 1;

    IntEventClear(SYS_INT_EDMA3_0_CC0_INT1);

    isIPR = HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_S_IPR(1));
    if(isIPR)
    {
        while((Cnt < EDMA3CC_COMPL_HANDLER_RETRY_COUNT)&& (indexl != 0u))
        {
            indexl = 0u;
            pendingIrqs = HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_S_IPR(1));

            while(pendingIrqs)
            {
                if((pendingIrqs & 1u) == TRUE)
                {
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_S_ICR(1)) = (1u << indexl);

                    (*cb_Fxn[indexl])(indexl, EDMA3_XFER_COMPLETE);
                }
                ++indexl;
                pendingIrqs >>= 1u;
            }
            Cnt++;
        }
    }
}

void EDMA3CCErrHandlerIsr()
{
    volatile unsigned int pendingIrqs = 0;
    unsigned int regionNum = 0;
    unsigned int evtqueNum = 0;
    unsigned int index = 1;
    unsigned int Cnt = 0;

    IntEventClear(SYS_INT_EDMA3_0_CC0_ERRINT);

    if((HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_EMR) != 0 ) || \
       (HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_QEMR) != 0) || \
       (HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_CCERR) != 0))
    {
        while((Cnt < EDMA3CC_ERR_HANDLER_RETRY_COUNT) && (index != 0u))
        {
            index = 0u;
            pendingIrqs = HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_EMR);
            while(pendingIrqs)
            {
                if((pendingIrqs & 1u) == TRUE)
                {
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_EMCR) = (1u<<index);
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_S_SECR(regionNum)) = (1u << index);
                }

                ++index;
                pendingIrqs >>= 1u;
            }

            index = 0u;
            pendingIrqs = HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_QEMR);

            while(pendingIrqs)
            {
                if((pendingIrqs & 1u)==TRUE)
                {
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_QEMCR) = (1u << index);
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_S_QSECR(0)) = (1u << index);
                }

                ++index;
                pendingIrqs >>= 1u;
            }

            index = 0u;
            pendingIrqs = HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_CCERR);

            if(pendingIrqs != 0u)
            {
                for(evtqueNum = 0u; evtqueNum < EDMA3_0_NUM_EVTQUE; evtqueNum++)
                {
                    if((pendingIrqs & (1u << evtqueNum)) != 0u)
                    {
                        HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_CCERRCLR) = (1u << evtqueNum);
                    }
                 }

                 if ((pendingIrqs & (1 << EDMA3CC_CCERR_TCCERR_SHIFT)) != 0u)
                 {
                     HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_CCERRCLR) = \
                          (0x01u << EDMA3CC_CCERR_TCCERR_SHIFT);
                 }

                 ++index;
            }

            Cnt++;
        }
    }
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
    IntRegister(C674X_MASK_INT4, EDMA3ComplHandlerIsr);
    IntRegister(C674X_MASK_INT5, EDMA3CCErrHandlerIsr);

    // 映射中断到 DSP 可屏蔽中断
    IntEventMap(C674X_MASK_INT4, SYS_INT_EDMA3_0_CC0_INT1);
    IntEventMap(C674X_MASK_INT5, SYS_INT_EDMA3_0_CC0_ERRINT);

    // 使能 DSP 可屏蔽中断
    IntEnable(C674X_MASK_INT4);
    IntEnable(C674X_MASK_INT5);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      串口初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void UARTInit()
{
    // 配置 UART2 参数
    // 波特率 115200 数据位 8 停止位 1 无校验位
    UARTConfigSetExpClk(SOC_UART_2_REGS, SYSCLK_2_FREQ, BAUD_115200, UART_WORDL_8BITS, UART_OVER_SAMP_RATE_16);

    // 使能 UART2
    UARTEnable(SOC_UART_2_REGS);

    // 使能接收 / 发送 FIFO
    UARTFIFOEnable(SOC_UART_2_REGS);

    // 设置 FIFO 级别
    UARTFIFOLevelSet(SOC_UART_2_REGS, UART_RX_TRIG_LEVEL_1);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      获取 EDMA3 版本
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned int EDMAVersionGet()
{
    return 1;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    // 使能外设
    // UART2
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_UART2, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // EDMA3
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_CC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);  // EDMA3 CC0
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_TC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);  // EDMA3 TC0

    // 管脚复用配置
    GPIOBankPinMuxSet();

    // DSP 中断初始化
    InterruptInit();

    // EDMA3 初始化
    EDMA3Init(SOC_EDMA30CC_0_REGS, EVT_QUEUE_NUM);

    // 串口初始化
    UARTInit();

    // 申请发送通道
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, EDMA3_CHA_UART2_TX, EDMA3_CHA_UART2_TX, EVT_QUEUE_NUM);

    // 注册回调函数
    cb_Fxn[EDMA3_CHA_UART2_TX] = &EDMA3IntCallback;

    // 申请串口接收通道
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, EDMA3_CHA_UART2_RX, EDMA3_CHA_UART2_RX, EVT_QUEUE_NUM);

    // 注册回调函数
    cb_Fxn[EDMA3_CHA_UART2_RX] = &EDMA3IntCallback;

    // 发送数据
    char txData[] = "\r\nCoreKernel UART2 Example...\r\nPlease Enter 20 bytes from keyboard\r\n";
    char rxData[RX_BUFFER_SIZE];

    unsigned int len = 0;
    len = strlen((const char *)txData);
    UARTTransmitData(EDMA3_CHA_UART2_TX, EDMA3_CHA_UART2_TX, txData, len);

    // 等待从回调函数返回
    while(flag == 0);
    flag = 0;

    // 接收数据
    UARTReceiveData(EDMA3_CHA_UART2_RX, EDMA3_CHA_UART2_RX, rxData);

    // 等待从回调函数返回
    while(flag == 0);
    flag = 0;

    // 发送数据
    UARTTransmitData(EDMA3_CHA_UART2_TX, EDMA3_CHA_UART2_TX, rxData, RX_BUFFER_SIZE);

    // 等待从回调函数返回
    while(flag == 0);
    flag = 0;

    // 释放 EDMA3 通道(如果未释放 下一次运行程序无法申请通道)
    EDMA3FreeChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, EDMA3_CHA_UART2_TX, EDMA3_TRIG_MODE_EVENT, EDMA3_CHA_UART2_TX, EVT_QUEUE_NUM);
    EDMA3FreeChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, EDMA3_CHA_UART2_RX, EDMA3_TRIG_MODE_EVENT, EDMA3_CHA_UART2_RX, EVT_QUEUE_NUM);

    for(;;)
    {

    }
}
