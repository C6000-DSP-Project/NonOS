/****************************************************************************/
/*                                                                          */
/*    �º˿Ƽ�(����)���޹�˾                                                */
/*                                                                          */
/*    Copyright (C) 2022 CoreKernel Technology (Guangzhou) Co., Ltd         */
/*                                                                          */
/****************************************************************************/
/*
 *    - ϣ����Ĭ(bin wang)
 *    - bin@corekernel.net
 *
 *    ���� corekernel.net/.org/.cn
 *    ���� fpga.net.cn
 *
 */
#ifndef MCASPINIT_H_
#define MCASPINIT_H_

#include "hw_mcasp.h"

// DMA ģʽѡ��
#define MCASP_MODE_DMA			0
#define MCASP_MODE_NON_DMA		1

// ���ͽ���ģʽѡ��
#define MCASP_TX_MODE          0x01
#define MCASP_RX_MODE          0x02
#define MCASP_BOTH_MODE        0x03

extern void OutputSample(unsigned int outData);
extern unsigned int InputSample(void);

extern void McASPI2SConfigure(unsigned char transmitMode,unsigned char wordSize,
		unsigned char slotSize, unsigned int slotNum, unsigned char modeDMA);
extern void McASPIntSetup(unsigned int cpuINT, void (*userISR)(void));
extern void I2SDataTxRxActivate(unsigned char transmitMode);

#endif
