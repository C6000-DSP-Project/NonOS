/****************************************************************************/
/*                                                                          */
/*    新核科技(广州)有限公司                                                */
/*                                                                          */
/*    Copyright (C) 2022 CoreKernel Technology (Guangzhou) Co., Ltd         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*    WM8960 音频输出（EDMA3 模式）                                         */
/*                                                                          */
/*    2022年03月30日                                                        */
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
#include "mcasp.h"
#include "edma.h"
#include "edma_event.h"
#include "uartStdio.h"

#include "interrupt.h"

#include "McASPInit.h"

/****************************************************************************/
/*                                                                          */
/*              宏定义                                                      */
/*                                                                          */
/****************************************************************************/
// I2S 使用2个 slot
#define I2S_SLOTS                             (2u)

// 接收 每个 slot 大小
#define SLOT_SIZE                             (16u)

// 接收数据 word 大小. Word size <= Slot size
#define WORD_SIZE                             (16u)

//每个采样点的字节数
#define BYTES_PER_SAMPLE                      ((SLOT_SIZE >> 3))

// AIC3106 音频芯片的 I2C 从地址
#define I2C_SLAVE_CODEC_AIC31                 (0x18u)

/* Definitions for sample tone */
#define TONE_START_ADDR               ((unsigned int)toneRaw)
#define TONE_NUM_BYTES                (sizeof(toneRaw))
#define TONE_END_ADDR                 (TONE_START_ADDR + TONE_NUM_BYTES - 1)
#define PARAM1_NUM_SAMPLES_L          ((unsigned int)(TONE_NUM_BYTES  \
                                       / (WORD_SIZE >> 3)))
#define PARAM1_BCNT                   (65000)
#define PARAM1_CCNT                   1//((unsigned int) \
                                       (PARAM1_NUM_SAMPLES_L / PARAM1_BCNT))
#define PARAM2_START_ADDR             (TONE_START_ADDR + (PARAM1_CCNT * \
                                       (WORD_SIZE >> 3) * PARAM1_BCNT))
#define PARAM2_BCNT                   49698//(((TONE_END_ADDR - PARAM2_START_ADDR) \
                                        / (WORD_SIZE >> 3)) + 1)
#define PARAM1_INTCODE                (0)
#define PARAM2_INTCODE                (1)

/****************************************************************************/
/*                                                                          */
/*              全局变量                                                    */
/*                                                                          */
/****************************************************************************/
extern unsigned short toneRaw[229396/2];
extern unsigned char toneRaw1[];

static struct EDMA3CCPaRAMEntry dmaPar[3] = {
       {
           (unsigned int)(EDMA3CC_OPT_DAM  | (0x02 << 8u)),	// Opt
           (unsigned int)TONE_START_ADDR,						// 源地址
           (unsigned short)BYTES_PER_SAMPLE,				    // aCnt
           (unsigned short)PARAM1_BCNT,						// bCnt
           (unsigned int) SOC_MCASP_0_DATA_REGS,				// 目标地址
           (short) BYTES_PER_SAMPLE,						    // 源 bIdx
           (short)0x00,											// 目标 bIdx
           (unsigned short)(32u * 40u),						// 链接地址
           (unsigned short)PARAM1_BCNT,						// bCnt 重装值
           (short)BYTES_PER_SAMPLE,						        // 源 cIdx
           (short)0x00,											// 目标 cIdx
           (unsigned short)PARAM1_CCNT							// cCnt
       },
       {
           (unsigned int)(EDMA3CC_OPT_DAM  | (0x02 << 8u)),	// Opt
           (unsigned int)(PARAM2_START_ADDR),					// 源地址
           (unsigned short)BYTES_PER_SAMPLE,				    // aCnt
           (unsigned short)PARAM2_BCNT,						// bCnt
           (unsigned int) SOC_MCASP_0_DATA_REGS,				// 目标地址
           (short)BYTES_PER_SAMPLE,						        // 源 bIdx
           (short)0x00,											// 目标 bIdx
           (unsigned short)(32u * 41u),						// 链接地址
           (unsigned short)0,									// bCnt 重装值
           (short)0x00,											// 源 cIdx
           (short)0x00,											// 目标 cIdx
           (unsigned short)(1u)								// cCnt
       },
       {
           (unsigned int)(EDMA3CC_OPT_DAM  | (0x02 << 8u)),	// Opt
           (unsigned int)TONE_START_ADDR,						// 源地址
           (unsigned short)BYTES_PER_SAMPLE,				    // aCnt
           (unsigned short)PARAM1_BCNT,						// bCnt
           (unsigned int) SOC_MCASP_0_DATA_REGS,				// 目标地址
           (short) BYTES_PER_SAMPLE,						    // 源 bIdx
           (short)0x00,											// 目标 bIdx
           (unsigned short)(32u * 40u),						// 链接地址
           (unsigned short)PARAM1_BCNT,						// bCnt 重装值
           (short)BYTES_PER_SAMPLE,						        // 源 cIdx
           (short)0x00,											// 目标 cIdx
           (unsigned short)PARAM1_CCNT							// cCnt
       }
};

/****************************************************************************/
/*                                                                          */
/*              函数声明                                                    */
/*                                                                          */
/****************************************************************************/
void WM8960Init();

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
/*              管脚复用配置                                                */
/*                                                                          */
/****************************************************************************/
#define PINMUX0_MCASP0_ACLKR_ENABLE    (SYSCFG_PINMUX0_PINMUX0_3_0_ACLKR0 << \
                                       SYSCFG_PINMUX0_PINMUX0_3_0_SHIFT)

#define PINMUX0_MCASP0_ACLKX_ENABLE    (SYSCFG_PINMUX0_PINMUX0_7_4_ACLKX0 << \
                                       SYSCFG_PINMUX0_PINMUX0_7_4_SHIFT)

#define PINMUX0_MCASP0_AFSR_ENABLE     (SYSCFG_PINMUX0_PINMUX0_11_8_AFSR0 << \
                                       SYSCFG_PINMUX0_PINMUX0_11_8_SHIFT)

#define PINMUX0_MCASP0_AFSX_ENABLE     (SYSCFG_PINMUX0_PINMUX0_15_12_AFSX0 << \
                                       SYSCFG_PINMUX0_PINMUX0_15_12_SHIFT)

#define PINMUX0_MCASP0_AHCLKR_ENABLE   (SYSCFG_PINMUX0_PINMUX0_19_16_AHCLKR0 << \
                                       SYSCFG_PINMUX0_PINMUX0_19_16_SHIFT)

#define PINMUX0_MCASP0_AHCLKX_ENABLE   (SYSCFG_PINMUX0_PINMUX0_23_20_AHCLKX0 << \
                                       SYSCFG_PINMUX0_PINMUX0_23_20_SHIFT)

#define PINMUX0_MCASP0_AMUTE_ENABLE    (SYSCFG_PINMUX0_PINMUX0_27_24_AMUTE0 << \
                                       SYSCFG_PINMUX0_PINMUX0_27_24_SHIFT)

//AXR[11] 和 AXR[12]
#define PINMUX1_MCASP0_AXR11_ENABLE    (SYSCFG_PINMUX1_PINMUX1_19_16_AXR0_11 <<\
                                       SYSCFG_PINMUX1_PINMUX1_19_16_SHIFT)

#define PINMUX1_MCASP0_AXR12_ENABLE    (SYSCFG_PINMUX1_PINMUX1_15_12_AXR0_12 << \
                                       SYSCFG_PINMUX1_PINMUX1_15_12_SHIFT)

void McASPPinMuxSetup(void)
{
    unsigned int savePinMux = 0;

    savePinMux = HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) & \
                       ~(SYSCFG_PINMUX0_PINMUX0_27_24 | \
                         SYSCFG_PINMUX0_PINMUX0_23_20 | \
                         SYSCFG_PINMUX0_PINMUX0_19_16 | \
                         SYSCFG_PINMUX0_PINMUX0_15_12 | \
                         SYSCFG_PINMUX0_PINMUX0_11_8 | \
                         SYSCFG_PINMUX0_PINMUX0_7_4 | \
                         SYSCFG_PINMUX0_PINMUX0_3_0);

    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) = \
         (PINMUX0_MCASP0_AMUTE_ENABLE | PINMUX0_MCASP0_AHCLKX_ENABLE | \
          PINMUX0_MCASP0_AHCLKR_ENABLE | PINMUX0_MCASP0_AFSX_ENABLE | \
          PINMUX0_MCASP0_AFSR_ENABLE | PINMUX0_MCASP0_ACLKX_ENABLE | \
          PINMUX0_MCASP0_ACLKR_ENABLE | savePinMux);

    savePinMux = HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) & \
                       ~(SYSCFG_PINMUX1_PINMUX1_19_16 | \
                         SYSCFG_PINMUX1_PINMUX1_15_12);

    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) = \
         (PINMUX1_MCASP0_AXR11_ENABLE | \
          PINMUX1_MCASP0_AXR12_ENABLE | \
          savePinMux);
}

/****************************************************************************/
/*                                                                          */
/*              McASP 初始化                                                */
/*                                                                          */
/****************************************************************************/
static void I2SDMAParamInit(void)
{
    // 初始化 DMA 参数
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_TX, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[0])));
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, 40, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[1])));
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, 41, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[2])));
}

static void InitMcaspEdma(void)
{
    // McASP 模块引脚配置
    McASPPinMuxSetup();

    // 使能 EDMA3 PSC
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_CC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_TC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    EDMA3Init(SOC_EDMA30CC_0_REGS, 0);

    // 申请 EDMA 通道，通道 0 用于接收
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, EDMA3_CHA_MCASP0_TX, EDMA3_CHA_MCASP0_TX, 0);

    // 初始化 DMA 参数
    I2SDMAParamInit();

    // 初始化 McASP 为 I2S 模式 只发送
    McASPI2SConfigure(MCASP_TX_MODE, WORD_SIZE, SLOT_SIZE, I2S_SLOTS, MCASP_MODE_DMA);

    // 使能 EDMA 传输
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_TX, EDMA3_TRIG_MODE_EVENT);

    // 启动 McASP 发送
    I2SDataTxRxActivate(MCASP_TX_MODE);
}

/****************************************************************************/
/*                                                                          */
/*              DSP 中断初始化                                              */
/*                                                                          */
/****************************************************************************/
static void InterruptInit(void)
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
    UARTPuts("Audio Line Out...\r\n\r\n", -1);

    unsigned int i;
    for(i = 0; i < 229396 / 2; i++)
    {
        toneRaw[i] = (toneRaw1[i * 2]) | toneRaw1[i * 2 + 1] << 8;
    }

    // DSP 中断初始化
    InterruptInit();

    // 初始化 WM8960 音频芯片
    WM8960Init();

    // 初始化 McASP EDMA
    InitMcaspEdma();

    Delay(0xFFFFF);

    for(;;)
    {

    }
}
