// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      I2C API 接口（查询模式）
//
//      2022年03月30日
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
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "psc.h"
#include "mcasp.h"

#include "interrupt.h"

#include "McASPInit.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// McASP 接收通道
#define MCASP_XSER_RX                         (12u)

// McASP 发送通道
#define MCASP_XSER_TX                         (11u)

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      McASP 输出
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void OutputSample(unsigned int outData)
{
	McASPTxBufWrite(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_TX, outData);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      McASP 输入
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned int InputSample(void)
{
	return (McASPRxBufRead(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_RX));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      McASP 接收通道初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void McASPI2SRxConfigure(unsigned char wordSize,unsigned char slotSize, unsigned int slotNum, unsigned char modeDMA)
{
	// 复位
	McASPRxReset(SOC_MCASP_0_CTRL_REGS);

	switch(modeDMA)
	{
		case MCASP_MODE_DMA:
			// 使能 FIFO
			McASPReadFifoEnable(SOC_MCASP_0_FIFO_REGS, 1, 1);

			// 设置接收 word 和 slot 的大小
			McASPRxFmtI2SSet(SOC_MCASP_0_CTRL_REGS, wordSize, slotSize, MCASP_RX_MODE_DMA);
			break;
		case MCASP_MODE_NON_DMA:
			// 设置接收 word 和 slot 的大小
			McASPRxFmtI2SSet(SOC_MCASP_0_CTRL_REGS, wordSize, slotSize, MCASP_RX_MODE_NON_DMA);
			break;
	}

	// 初始化帧同步，TDM 格式使用 slot 个数，对齐帧同步信号的上升沿
	McASPRxFrameSyncCfg(SOC_MCASP_0_CTRL_REGS, slotNum, MCASP_RX_FS_WIDTH_WORD, MCASP_RX_FS_EXT_BEGIN_ON_RIS_EDGE);

	// 初始化接收时钟，使用外部时钟，时钟上升沿有效
	McASPRxClkCfg(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLK_EXTERNAL, 0, 0);
	McASPRxClkPolaritySet(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLK_POL_RIS_EDGE);
	McASPRxClkCheckConfig(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLKCHCK_DIV32, 0x00, 0xFF);

	// 使能发送接收同步
//	McASPTxRxClkSyncEnable(SOC_MCASP_0_CTRL_REGS);
	McASPTxClkCfg(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLK_EXTERNAL, 0, 0);

	// 使能 接收 slot
	McASPRxTimeSlotSet(SOC_MCASP_0_CTRL_REGS, (1 << slotNum)-1);

	// 设置串行器，设置12通道接收
	McASPSerializerRxSet(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_RX);

	// 初始化 McASP 引脚，和引脚输入输出方向
	McASPPinMcASPSet(SOC_MCASP_0_CTRL_REGS, 0xFFFFFFFF);
	McASPPinDirInputSet(SOC_MCASP_0_CTRL_REGS, MCASP_PIN_AFSX
											   | MCASP_PIN_ACLKX
											   | MCASP_PIN_AXR(MCASP_XSER_RX));
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      McASP 发送通道初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void McASPI2STxConfigure(unsigned char wordSize,unsigned char slotSize, unsigned int slotNum, unsigned char modeDMA)
{
	// 复位
	McASPTxReset(SOC_MCASP_0_CTRL_REGS);

	switch(modeDMA)
	{
		case MCASP_MODE_DMA:
			// 使能 FIFO
			McASPWriteFifoEnable(SOC_MCASP_0_FIFO_REGS, 1, 1);

			// 设置发送 word 和 slot 的大小
			McASPTxFmtI2SSet(SOC_MCASP_0_CTRL_REGS, wordSize, slotSize,
								MCASP_TX_MODE_DMA);
			break;
		case MCASP_MODE_NON_DMA:
			// 设置发送 word 和 slot 的大小
			McASPTxFmtI2SSet(SOC_MCASP_0_CTRL_REGS, wordSize, slotSize,
								MCASP_TX_MODE_NON_DMA);
			break;
	}

	// 初始化帧同步，TDM 格式使用 slot 个数，对齐帧同步信号的上升沿
	McASPTxFrameSyncCfg(SOC_MCASP_0_CTRL_REGS, slotNum, MCASP_TX_FS_WIDTH_WORD,
						MCASP_TX_FS_EXT_BEGIN_ON_RIS_EDGE);

	// 初始化发送时钟，使用外部时钟，时钟上升沿有效
	McASPTxClkCfg(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_EXTERNAL, 0, 0);
	McASPTxClkPolaritySet(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_POL_FALL_EDGE);
	McASPTxClkCheckConfig(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLKCHCK_DIV32, 0x00, 0xFF);

	// 使能发送接收同步
//	McASPTxRxClkSyncEnable(SOC_MCASP_0_CTRL_REGS);

	// 使能 发送 slot
	McASPTxTimeSlotSet(SOC_MCASP_0_CTRL_REGS, (1 << slotNum) - 1);

	// 设置串行器 设置 11 通道发送
	McASPSerializerTxSet(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_TX);

	// 初始化 McASP 引脚，和引脚输入输出方向
	McASPPinMcASPSet(SOC_MCASP_0_CTRL_REGS, 0xFFFFFFFF);
	McASPPinDirOutputSet(SOC_MCASP_0_CTRL_REGS, MCASP_PIN_AHCLKX | MCASP_PIN_AXR(MCASP_XSER_TX));
    McASPPinDirInputSet(SOC_MCASP_0_CTRL_REGS, MCASP_PIN_AFSX | MCASP_PIN_ACLKX);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      McASP I2C 模式配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void McASPI2SConfigure(unsigned char transmitMode, unsigned char wordSize,
		unsigned char slotSize, unsigned int slotNum, unsigned char modeDMA)
{
	// 使能 McASP 模块 PSC
	PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_MCASP0, PSC_POWERDOMAIN_ALWAYS_ON,
			 PSC_MDCTL_NEXT_ENABLE);

	if(transmitMode & MCASP_TX_MODE)
	{
		McASPI2STxConfigure(wordSize, slotSize, slotNum,  modeDMA);
	}

	if(transmitMode & MCASP_RX_MODE)
	{
		McASPI2SRxConfigure(wordSize, slotSize, slotNum,  modeDMA);
	}
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      McASP 中断初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void McASPIntInit(unsigned int cpuINT, void (*userISR)(void))
{
	IntRegister(cpuINT, userISR);
	IntEventMap(cpuINT, SYS_INT_MCASP0_INT);
	IntEnable(cpuINT);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      激活 I2S 发送接收
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void I2SDataTxRxActivate(unsigned char transmitMode)
{
	if(transmitMode & MCASP_TX_MODE)
	{
		// 启动使用外部时钟
		McASPTxClkStart(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_EXTERNAL);

		// 启动串行器
		McASPTxSerActivate(SOC_MCASP_0_CTRL_REGS);

		// 使能状态机
		McASPTxEnable(SOC_MCASP_0_CTRL_REGS);

		// 发送数据
		McASPTxBufWrite(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_TX, 0);
	}

	if(transmitMode & MCASP_RX_MODE)
	{
		// 启动使用外部时钟
		McASPRxClkStart(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_EXTERNAL);

		// 启动串行器
		McASPRxSerActivate(SOC_MCASP_0_CTRL_REGS);

		// 使能状态机
		McASPRxEnable(SOC_MCASP_0_CTRL_REGS);
	}
}
