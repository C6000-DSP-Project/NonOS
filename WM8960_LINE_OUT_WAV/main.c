// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      WM8960 ��Ƶ WAV �ļ������EDMA3 ģʽ��
//
//      2022��03��30��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    WAV ��Ƶ�ļ����
 *
 *    - ϣ����Ĭ(bin wang)
 *    - bin@corekernel.net
 *
 *    ���� corekernel.net/.org/.cn
 *    ���� fpga.net.cn
 *
 */
#include "hw_types.h"
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "psc.h"
#include "mcasp.h"
#include "edma.h"
#include "edma_event.h"

#include "interrupt.h"

#include "uartStdio.h"

#include "McASPInit.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ȫ�ֱ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// 50M ��Ƶ����
unsigned char wav_sound[1024 * 1024 * 50];

unsigned int sample_rate, data_length, channel_num, slot_size, data_start;

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void WM8960Init();

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽŸ�������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinMuxSet()
{
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(00)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(00)) &
                                                    (~(SYSCFG_PINMUX0_PINMUX0_27_24  |
                                                       SYSCFG_PINMUX0_PINMUX0_23_20  |
                                                       SYSCFG_PINMUX0_PINMUX0_19_16  |
                                                       SYSCFG_PINMUX0_PINMUX0_15_12  |
                                                       SYSCFG_PINMUX0_PINMUX0_11_8   |
                                                       SYSCFG_PINMUX0_PINMUX0_7_4    |
                                                       SYSCFG_PINMUX0_PINMUX0_3_0))) |
                                                     ((SYSCFG_PINMUX0_PINMUX0_27_24_AMUTE0  << SYSCFG_PINMUX0_PINMUX0_27_24_SHIFT) |
                                                      (SYSCFG_PINMUX0_PINMUX0_23_20_AHCLKX0 << SYSCFG_PINMUX0_PINMUX0_23_20_SHIFT) |
                                                      (SYSCFG_PINMUX0_PINMUX0_19_16_AHCLKR0 << SYSCFG_PINMUX0_PINMUX0_19_16_SHIFT) |
                                                      (SYSCFG_PINMUX0_PINMUX0_15_12_AFSX0   << SYSCFG_PINMUX0_PINMUX0_15_12_SHIFT) |
                                                      (SYSCFG_PINMUX0_PINMUX0_11_8_AFSR0    << SYSCFG_PINMUX0_PINMUX0_11_8_SHIFT)  |
                                                      (SYSCFG_PINMUX0_PINMUX0_7_4_ACLKX0    << SYSCFG_PINMUX0_PINMUX0_7_4_SHIFT)   |
                                                      (SYSCFG_PINMUX0_PINMUX0_3_0_ACLKR0    << SYSCFG_PINMUX0_PINMUX0_3_0_SHIFT));

    // AXR[11] �� AXR[12]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(01)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(01)) &
                                                    (~(SYSCFG_PINMUX1_PINMUX1_19_16    |
                                                       SYSCFG_PINMUX1_PINMUX1_15_12))) |
                                                     ((SYSCFG_PINMUX1_PINMUX1_19_16_AXR0_11 << SYSCFG_PINMUX1_PINMUX1_19_16_SHIFT) |
                                                      (SYSCFG_PINMUX1_PINMUX1_15_12_AXR0_12 << SYSCFG_PINMUX1_PINMUX1_15_12_SHIFT));
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      McASP ��ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void I2SDMAParamInit()
{
    static struct EDMA3CCPaRAMEntry dmaPar[3];

    dmaPar[0].opt        = (unsigned int)(EDMA3CC_OPT_DAM  | (0x02 << 8u));           // Opt
    dmaPar[0].srcAddr    = (unsigned int)wav_sound + data_start;                      // Դ��ַ
    dmaPar[0].aCnt       = (unsigned short)(slot_size >> 3);                          // aCnt
    dmaPar[0].bCnt       = (unsigned short)65000;                                     // bCnt
    dmaPar[0].destAddr   = (unsigned int) SOC_MCASP_0_DATA_REGS,                      // Ŀ���ַ
    dmaPar[0].srcBIdx    = (short)(slot_size >> 3);                                   // Դ bIdx
    dmaPar[0].destBIdx   = (short)0x00;                                               // Ŀ�� bIdx
    dmaPar[0].linkAddr   = (unsigned short)(32u * 40u);                               // ���ӵ�ַ
    dmaPar[0].bCntReload = (unsigned short)65000;                                     // bCnt ��װֵ
    dmaPar[0].srcCIdx    = (short)(slot_size >> 3);                                   // Դ cIdx
    dmaPar[0].destCIdx   = (short)0x00;                                               // Ŀ�� cIdx
    dmaPar[0].cCnt       = (unsigned short)(data_length / (slot_size >> 3) / 65000);  // cCnt

    dmaPar[1].opt        = (unsigned int)(EDMA3CC_OPT_DAM  | (0x02 << 8u));           // Opt
    dmaPar[1].srcAddr    = (unsigned int)(dmaPar[0].srcAddr +
                             dmaPar[0].aCnt * dmaPar[0].bCnt * dmaPar[0].cCnt);       // Դ��ַ
    dmaPar[1].aCnt       = (unsigned short)(slot_size >> 3);                          // aCnt
    dmaPar[1].bCnt       = (unsigned short)(data_length / (slot_size >> 3) % 65000);  // bCnt
    dmaPar[1].destAddr   = (unsigned int) SOC_MCASP_0_DATA_REGS,                      // Ŀ���ַ
    dmaPar[1].srcBIdx    = (short)(slot_size >> 3);                                   // Դ bIdx
    dmaPar[1].destBIdx   = (short)0x00;                                               // Ŀ�� bIdx
    dmaPar[1].linkAddr   = (unsigned short)(32u * 41u);                               // ���ӵ�ַ
    dmaPar[1].bCntReload = (unsigned short)0;                                         // bCnt ��װֵ
    dmaPar[1].srcCIdx    = (short)0;                                                  // Դ cIdx
    dmaPar[1].destCIdx   = (short)0x00;                                               // Ŀ�� cIdx
    dmaPar[1].cCnt       = (unsigned short)(1u);                                      // cCnt

    dmaPar[2].opt        = (unsigned int)(EDMA3CC_OPT_DAM  | (0x02 << 8u));           // Opt
    dmaPar[2].srcAddr    = (unsigned int)wav_sound + data_start;                      // Դ��ַ
    dmaPar[2].aCnt       = (unsigned short)(slot_size >> 3);                          // aCnt
    dmaPar[2].bCnt       = (unsigned short)65000;                                     // bCnt
    dmaPar[2].destAddr   = (unsigned int) SOC_MCASP_0_DATA_REGS,                      // Ŀ���ַ
    dmaPar[2].srcBIdx    = (short)(slot_size >> 3);                                   // Դ bIdx
    dmaPar[2].destBIdx   = (short)0x00;                                               // Ŀ�� bIdx
    dmaPar[2].linkAddr   = (unsigned short)(32u * 40u);                               // ���ӵ�ַ
    dmaPar[2].bCntReload = (unsigned short)65000;                                     // bCnt ��װֵ
    dmaPar[2].srcCIdx    = (short)(slot_size >> 3);                                   // Դ cIdx
    dmaPar[2].destCIdx   = (short)0x00;                                               // Ŀ�� cIdx
    dmaPar[2].cCnt       = (unsigned short)(data_length / (slot_size >> 3) / 65000);  // cCnt

    // ��ʼ�� DMA ����
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_TX, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[0])));
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, 40, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[1])));
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, 41, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[2])));
}

static void McASPInit()
{
	// ʹ�� EDMA3
	PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_CC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);
	PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_TC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

	EDMA3Init(SOC_EDMA30CC_0_REGS, 0);

	// ���� EDMA ͨ��
	EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, EDMA3_CHA_MCASP0_TX, EDMA3_CHA_MCASP0_TX, 0);

	// ��ʼ�� DMA ����
	I2SDMAParamInit();

	// ��ʼ�� McASP Ϊ I2S ģʽ
	McASPI2SConfigure(MCASP_TX_MODE, slot_size, slot_size, channel_num, MCASP_MODE_DMA);

	// ʹ�� EDMA ����
	EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_TX, EDMA3_TRIG_MODE_EVENT);

	// ���� McASP ����
	I2SDataTxRxActivate(MCASP_TX_MODE);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      DSP �жϳ�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void InterruptInit(void)
{
	// ��ʼ�� DSP �жϿ�����
	IntDSPINTCInit();

	// ʹ�� DSP ȫ���ж�
	IntGlobalEnable();
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��ʱ���Ǿ�ȷ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void Delay(volatile unsigned int delay)
{
    while(delay--);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��ȡ EDMA3 �汾
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
unsigned int EDMAVersionGet()
{
    return 1;
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
int main()
{
    unsigned int i;

    UARTStdioInit();
    UARTPuts("CoreKernel WAV Audio Line Out...\r\n\r\n", -1);

    // GPIO �ܽŸ�������
    GPIOBankPinMuxSet();

    // ��ȡ������
    sample_rate = wav_sound[0x1B] << 24 | wav_sound[0x1A] | \
                    wav_sound[0x19] << 8 | wav_sound[0x18];

    // ��ȡͨ����
    channel_num = wav_sound[0x17] << 8 | wav_sound[0x16];

    // ��ȡ����λ��
    slot_size   = wav_sound[0x23] << 8 | wav_sound[0x22];

    // ɨ�衰data���ֶ�
    for(i = 35; i <= 0x100; i++)
    {
        if((wav_sound[i] == 'd') && (wav_sound[i+1] == 'a') &&
                (wav_sound[i+2] == 't') && (wav_sound[i+3] == 'a'))
            break;
    }
    i += 4;

    // ��ȡ��Ƶ���ݳ���
    data_length = wav_sound[i+3] << 24 | wav_sound[i+2] << 16 | \
                    wav_sound[i+1] << 8 | wav_sound[i];

    // ��Ƶ���ݵ�ƫ�Ƶ�ַ
    data_start = i + 4;

    UARTprintf("sample_rate=%d, channel_num=%d, slot_size=%d, data_length=%d\n",
            sample_rate, channel_num, slot_size, data_length);

    // DSP �жϳ�ʼ��
    InterruptInit();

    // WM8960 ��ʼ��
    WM8960Init();

    // McASP ��ʼ��
    McASPInit();

    Delay(0xFFFFF);

    for(;;)
    {

    }
}
