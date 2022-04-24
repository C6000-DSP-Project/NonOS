// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      I2C API �ӿڣ���ѯģʽ��
//
//      2022��03��30��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    - ϣ����Ĭ(bin wang)
 *    - bin@corekernel.net
 *
 *    ���� corekernel.net/.org/.cn
 *    ���� fpga.net.cn
 *
 */
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "psc.h"
#include "mcasp.h"

#include "interrupt.h"

#include "McASPInit.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �궨��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// McASP ����ͨ��
#define MCASP_XSER_RX                         (12u)

// McASP ����ͨ��
#define MCASP_XSER_TX                         (11u)

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      McASP ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void OutputSample(unsigned int outData)
{
	McASPTxBufWrite(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_TX, outData);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      McASP ����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
unsigned int InputSample(void)
{
	return (McASPRxBufRead(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_RX));
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      McASP ����ͨ����ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void McASPI2SRxConfigure(unsigned char wordSize,unsigned char slotSize, unsigned int slotNum, unsigned char modeDMA)
{
	// ��λ
	McASPRxReset(SOC_MCASP_0_CTRL_REGS);

	switch(modeDMA)
	{
		case MCASP_MODE_DMA:
			// ʹ�� FIFO
			McASPReadFifoEnable(SOC_MCASP_0_FIFO_REGS, 1, 1);

			// ���ý��� word �� slot �Ĵ�С
			McASPRxFmtI2SSet(SOC_MCASP_0_CTRL_REGS, wordSize, slotSize, MCASP_RX_MODE_DMA);
			break;
		case MCASP_MODE_NON_DMA:
			// ���ý��� word �� slot �Ĵ�С
			McASPRxFmtI2SSet(SOC_MCASP_0_CTRL_REGS, wordSize, slotSize, MCASP_RX_MODE_NON_DMA);
			break;
	}

	// ��ʼ��֡ͬ����TDM ��ʽʹ�� slot ����������֡ͬ���źŵ�������
	McASPRxFrameSyncCfg(SOC_MCASP_0_CTRL_REGS, slotNum, MCASP_RX_FS_WIDTH_WORD, MCASP_RX_FS_EXT_BEGIN_ON_RIS_EDGE);

	// ��ʼ������ʱ�ӣ�ʹ���ⲿʱ�ӣ�ʱ����������Ч
	McASPRxClkCfg(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLK_EXTERNAL, 0, 0);
	McASPRxClkPolaritySet(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLK_POL_RIS_EDGE);
	McASPRxClkCheckConfig(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLKCHCK_DIV32, 0x00, 0xFF);

	// ʹ�ܷ��ͽ���ͬ��
//	McASPTxRxClkSyncEnable(SOC_MCASP_0_CTRL_REGS);
	McASPTxClkCfg(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLK_EXTERNAL, 0, 0);

	// ʹ�� ���� slot
	McASPRxTimeSlotSet(SOC_MCASP_0_CTRL_REGS, (1 << slotNum)-1);

	// ���ô�����������12ͨ������
	McASPSerializerRxSet(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_RX);

	// ��ʼ�� McASP ���ţ������������������
	McASPPinMcASPSet(SOC_MCASP_0_CTRL_REGS, 0xFFFFFFFF);
	McASPPinDirInputSet(SOC_MCASP_0_CTRL_REGS, MCASP_PIN_AFSX
											   | MCASP_PIN_ACLKX
											   | MCASP_PIN_AXR(MCASP_XSER_RX));
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      McASP ����ͨ����ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void McASPI2STxConfigure(unsigned char wordSize,unsigned char slotSize, unsigned int slotNum, unsigned char modeDMA)
{
	// ��λ
	McASPTxReset(SOC_MCASP_0_CTRL_REGS);

	switch(modeDMA)
	{
		case MCASP_MODE_DMA:
			// ʹ�� FIFO
			McASPWriteFifoEnable(SOC_MCASP_0_FIFO_REGS, 1, 1);

			// ���÷��� word �� slot �Ĵ�С
			McASPTxFmtI2SSet(SOC_MCASP_0_CTRL_REGS, wordSize, slotSize,
								MCASP_TX_MODE_DMA);
			break;
		case MCASP_MODE_NON_DMA:
			// ���÷��� word �� slot �Ĵ�С
			McASPTxFmtI2SSet(SOC_MCASP_0_CTRL_REGS, wordSize, slotSize,
								MCASP_TX_MODE_NON_DMA);
			break;
	}

	// ��ʼ��֡ͬ����TDM ��ʽʹ�� slot ����������֡ͬ���źŵ�������
	McASPTxFrameSyncCfg(SOC_MCASP_0_CTRL_REGS, slotNum, MCASP_TX_FS_WIDTH_WORD,
						MCASP_TX_FS_EXT_BEGIN_ON_RIS_EDGE);

	// ��ʼ������ʱ�ӣ�ʹ���ⲿʱ�ӣ�ʱ����������Ч
	McASPTxClkCfg(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_EXTERNAL, 0, 0);
	McASPTxClkPolaritySet(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_POL_FALL_EDGE);
	McASPTxClkCheckConfig(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLKCHCK_DIV32, 0x00, 0xFF);

	// ʹ�ܷ��ͽ���ͬ��
//	McASPTxRxClkSyncEnable(SOC_MCASP_0_CTRL_REGS);

	// ʹ�� ���� slot
	McASPTxTimeSlotSet(SOC_MCASP_0_CTRL_REGS, (1 << slotNum) - 1);

	// ���ô����� ���� 11 ͨ������
	McASPSerializerTxSet(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_TX);

	// ��ʼ�� McASP ���ţ������������������
	McASPPinMcASPSet(SOC_MCASP_0_CTRL_REGS, 0xFFFFFFFF);
	McASPPinDirOutputSet(SOC_MCASP_0_CTRL_REGS, MCASP_PIN_AHCLKX | MCASP_PIN_AXR(MCASP_XSER_TX));
    McASPPinDirInputSet(SOC_MCASP_0_CTRL_REGS, MCASP_PIN_AFSX | MCASP_PIN_ACLKX);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      McASP I2C ģʽ����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void McASPI2SConfigure(unsigned char transmitMode, unsigned char wordSize,
		unsigned char slotSize, unsigned int slotNum, unsigned char modeDMA)
{
	// ʹ�� McASP ģ�� PSC
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

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      McASP �жϳ�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void McASPIntInit(unsigned int cpuINT, void (*userISR)(void))
{
	IntRegister(cpuINT, userISR);
	IntEventMap(cpuINT, SYS_INT_MCASP0_INT);
	IntEnable(cpuINT);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ���� I2S ���ͽ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void I2SDataTxRxActivate(unsigned char transmitMode)
{
	if(transmitMode & MCASP_TX_MODE)
	{
		// ����ʹ���ⲿʱ��
		McASPTxClkStart(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_EXTERNAL);

		// ����������
		McASPTxSerActivate(SOC_MCASP_0_CTRL_REGS);

		// ʹ��״̬��
		McASPTxEnable(SOC_MCASP_0_CTRL_REGS);

		// ��������
		McASPTxBufWrite(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_TX, 0);
	}

	if(transmitMode & MCASP_RX_MODE)
	{
		// ����ʹ���ⲿʱ��
		McASPRxClkStart(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_EXTERNAL);

		// ����������
		McASPRxSerActivate(SOC_MCASP_0_CTRL_REGS);

		// ʹ��״̬��
		McASPRxEnable(SOC_MCASP_0_CTRL_REGS);
	}
}
