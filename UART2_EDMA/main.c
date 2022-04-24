// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ͨ���첽���� 2(DMA ģʽ)
//
//      2022��04��24��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    �����շ�
 *
 *    ע��: EDMA3 ���ܷ��� DSP ȫ�� L2RAM ��ַ(0x11800000) ���ܷ��� DSP ���� L2RAM ��ַ(0x00800000)
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
#include "gpio.h"
#include "uart.h"
#include "edma.h"
#include "edma_event.h"

#include "interrupt.h"

#include "string.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �궨��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ʱ��
#define SYSCLK_1_FREQ     (456000000)
#define SYSCLK_2_FREQ     (SYSCLK_1_FREQ / 2)

// EDMA3 ����
#define MAX_ACNT           1
#define MAX_CCNT           1
#define RX_BUFFER_SIZE     20

// EDMA3 ͨ��
#define EVT_QUEUE_NUM      0

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ȫ�ֱ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
volatile unsigned int flag = 0;

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void (*cb_Fxn[EDMA3_NUM_TCC])(unsigned int tcc, unsigned int status);

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽŸ�������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinMuxSet()
{
    // �����ƴ��� ��ʹ������
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(04)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(04)) &
                                                    (~(SYSCFG_PINMUX4_PINMUX4_23_20    |
                                                       SYSCFG_PINMUX4_PINMUX4_19_16))) |
                                                     ((SYSCFG_PINMUX4_PINMUX4_23_20_UART2_TXD << SYSCFG_PINMUX4_PINMUX4_23_20_SHIFT) |
                                                      (SYSCFG_PINMUX4_PINMUX4_19_16_UART2_RXD << SYSCFG_PINMUX4_PINMUX4_19_16_SHIFT));
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void UARTTransmitData(unsigned int tccNum, unsigned int chNum, volatile char *buffer, unsigned int buffLength)
{
    EDMA3CCPaRAMEntry paramSet;

    // ���ò��� RAM
    paramSet.srcAddr = (unsigned int)buffer;
    // ���ջ���Ĵ��� / ���ͱ��ּĴ��� ��ַ
    paramSet.destAddr = SOC_UART_2_REGS;
    paramSet.aCnt = MAX_ACNT;
    paramSet.bCnt = (unsigned short)buffLength;
    paramSet.cCnt = MAX_CCNT;

    // Դ��������ϵ�� 1 ��һ���ֽ�
    paramSet.srcBIdx = (short)1u;

    // Ŀ����������ϵ��
    paramSet.destBIdx = (short)0u;

    // �첽����ģʽ
    paramSet.srcCIdx = (short)0u;
    paramSet.destCIdx = (short)0u;
    paramSet.linkAddr = (unsigned short)0xFFFFu;
    paramSet.bCntReload = (unsigned short)0u;
    paramSet.opt = 0x00000000u;
    paramSet.opt |= (EDMA3CC_OPT_DAM );
    paramSet.opt |= ((tccNum << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    // д���� RAM
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, chNum, &paramSet);

    // ʹ�� EDMA3 ͨ��
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

    // ʹ�ܴ��� DMA
    UARTDMAEnable(SOC_UART_2_REGS, UART_RX_TRIG_LEVEL_1 | UART_DMAMODE | UART_FIFO_MODE);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void UARTReceiveData(unsigned int tccNum, unsigned int chNum, volatile char *buffer)
{
    EDMA3CCPaRAMEntry paramSet;

    // ���ò��� RAM
    // ���ջ���Ĵ��� / ���ͱ��ּĴ��� ��ַ
    paramSet.srcAddr = SOC_UART_2_REGS;
    paramSet.destAddr = (unsigned int)buffer;
    paramSet.aCnt = MAX_ACNT;
    paramSet.bCnt = RX_BUFFER_SIZE;
    paramSet.cCnt = MAX_CCNT;

    // Դ��������ϵ��
    paramSet.srcBIdx = 0;
    // Ŀ����������ϵ�� 1 ��һ���ֽ�
    paramSet.destBIdx = 1;

    // �첽ģʽ
    paramSet.srcCIdx = 0;
    paramSet.destCIdx = 0;
    paramSet.linkAddr = (unsigned short)0xFFFFu;
    paramSet.bCntReload = 0;
    paramSet.opt = 0x00000000u;
    paramSet.opt |= ((EDMA3CC_OPT_SAM) << EDMA3CC_OPT_SAM_SHIFT);
    paramSet.opt |= ((tccNum << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    // д���� RAM
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, chNum, &paramSet);

    // ʹ�� EDMA3 ͨ��
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

    // ʹ�ܴ��� DMA ģʽ
    UARTDMAEnable(SOC_UART_2_REGS, UART_RX_TRIG_LEVEL_1 | UART_DMAMODE | UART_FIFO_MODE);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �ص�����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void EDMA3IntCallback(unsigned int tccNum, unsigned int status)
{
    UARTDMADisable(SOC_UART_2_REGS, (UART_RX_TRIG_LEVEL_1 | UART_FIFO_MODE));

    flag = 1;
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �жϷ�����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// StarterWare system_config.lib �������ڲ���ʹ�� interrupt �ؼ������� �˴���Ϊ�ص�����
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

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      DSP �жϳ�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void InterruptInit(void)
{
    // ��ʼ�� DSP �жϿ�����
    IntDSPINTCInit();

    // ʹ�� DSP ȫ���ж�
    IntGlobalEnable();

    // ע���жϷ�����
    IntRegister(C674X_MASK_INT4, EDMA3ComplHandlerIsr);
    IntRegister(C674X_MASK_INT5, EDMA3CCErrHandlerIsr);

    // ӳ���жϵ� DSP �������ж�
    IntEventMap(C674X_MASK_INT4, SYS_INT_EDMA3_0_CC0_INT1);
    IntEventMap(C674X_MASK_INT5, SYS_INT_EDMA3_0_CC0_ERRINT);

    // ʹ�� DSP �������ж�
    IntEnable(C674X_MASK_INT4);
    IntEnable(C674X_MASK_INT5);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ���ڳ�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void UARTInit()
{
    // ���� UART2 ����
    // ������ 115200 ����λ 8 ֹͣλ 1 ��У��λ
    UARTConfigSetExpClk(SOC_UART_2_REGS, SYSCLK_2_FREQ, BAUD_115200, UART_WORDL_8BITS, UART_OVER_SAMP_RATE_16);

    // ʹ�� UART2
    UARTEnable(SOC_UART_2_REGS);

    // ʹ�ܽ��� / ���� FIFO
    UARTFIFOEnable(SOC_UART_2_REGS);

    // ���� FIFO ����
    UARTFIFOLevelSet(SOC_UART_2_REGS, UART_RX_TRIG_LEVEL_1);
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
void main()
{
    // ʹ������
    // UART2
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_UART2, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // EDMA3
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_CC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);  // EDMA3 CC0
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_TC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);  // EDMA3 TC0

    // �ܽŸ�������
    GPIOBankPinMuxSet();

    // DSP �жϳ�ʼ��
    InterruptInit();

    // EDMA3 ��ʼ��
    EDMA3Init(SOC_EDMA30CC_0_REGS, EVT_QUEUE_NUM);

    // ���ڳ�ʼ��
    UARTInit();

    // ���뷢��ͨ��
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, EDMA3_CHA_UART2_TX, EDMA3_CHA_UART2_TX, EVT_QUEUE_NUM);

    // ע��ص�����
    cb_Fxn[EDMA3_CHA_UART2_TX] = &EDMA3IntCallback;

    // ���봮�ڽ���ͨ��
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, EDMA3_CHA_UART2_RX, EDMA3_CHA_UART2_RX, EVT_QUEUE_NUM);

    // ע��ص�����
    cb_Fxn[EDMA3_CHA_UART2_RX] = &EDMA3IntCallback;

    // ��������
    char txData[] = "\r\nCoreKernel UART2 Example...\r\nPlease Enter 20 bytes from keyboard\r\n";
    char rxData[RX_BUFFER_SIZE];

    unsigned int len = 0;
    len = strlen((const char *)txData);
    UARTTransmitData(EDMA3_CHA_UART2_TX, EDMA3_CHA_UART2_TX, txData, len);

    // �ȴ��ӻص���������
    while(flag == 0);
    flag = 0;

    // ��������
    UARTReceiveData(EDMA3_CHA_UART2_RX, EDMA3_CHA_UART2_RX, rxData);

    // �ȴ��ӻص���������
    while(flag == 0);
    flag = 0;

    // ��������
    UARTTransmitData(EDMA3_CHA_UART2_TX, EDMA3_CHA_UART2_TX, rxData, RX_BUFFER_SIZE);

    // �ȴ��ӻص���������
    while(flag == 0);
    flag = 0;

    // �ͷ� EDMA3 ͨ��(���δ�ͷ� ��һ�����г����޷�����ͨ��)
    EDMA3FreeChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, EDMA3_CHA_UART2_TX, EDMA3_TRIG_MODE_EVENT, EDMA3_CHA_UART2_TX, EVT_QUEUE_NUM);
    EDMA3FreeChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, EDMA3_CHA_UART2_RX, EDMA3_TRIG_MODE_EVENT, EDMA3_CHA_UART2_RX, EVT_QUEUE_NUM);

    for(;;)
    {

    }
}
