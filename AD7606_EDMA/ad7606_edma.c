// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      AD7606 ADC EDMA ģʽ
//
//      2022��04��25��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    ��ʱ������ ADC ת�� ת����ɺ� BUSY �źŴ��� EDMA3 ��ȡ����
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

#include "edma.h"
#include "edma_event.h"

#include "interrupt.h"
#include "dspcache.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �궨��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
#define MAX_ACOUNT        (2u)          // 16bit
#define MAX_BCOUNT        (8u)          // �ɼ� 8 ��DACͨ��
#define MAX_CCOUNT        (1024u)       // ÿ�� ADC ͨ���ɼ� 1024 ������

#define MAX_BUFFER_SIZE   (MAX_ACOUNT * MAX_BCOUNT * MAX_CCOUNT)

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ȫ�ֱ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// DMA ���ݻ�����
char buf1[MAX_BUFFER_SIZE];
char buf2[MAX_BUFFER_SIZE];

// ADC ����
extern short ADCDataCH0[MAX_CCOUNT];
extern short ADCDataCH1[MAX_CCOUNT];
extern short ADCDataCH2[MAX_CCOUNT];
extern short ADCDataCH3[MAX_CCOUNT];
extern short ADCDataCH4[MAX_CCOUNT];
extern short ADCDataCH5[MAX_CCOUNT];
extern short ADCDataCH6[MAX_CCOUNT];
extern short ADCDataCH7[MAX_CCOUNT];


// �û��������
unsigned int EVT_QUEUE_NUM = 0; // ʹ�õ��¼�����

// DMA ͨ�����ò���
static struct EDMA3CCPaRAMEntry dmaPar[3] =
{
    {
        (unsigned int)(1 << 2 | 1 << 20 | (EDMA3_CHA_GPIO_BNKINT5 << EDMA3CC_OPT_TCC_SHIFT)),  // Opt
        (unsigned int)SOC_EMIFA_CS2_ADDR,					                                   // Դ��ַ
        (unsigned short)(MAX_ACOUNT),                                                          // aCnt
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt
        (unsigned int)buf1,                                                                    // Ŀ���ַ
        (short)(MAX_ACOUNT),                                                                   // Դ����
        (short)(MAX_ACOUNT * MAX_CCOUNT),                                                      // Ŀ������
        (unsigned short)(32u * 40u),                                                           // ���ӵ�ַ
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt ��װֵ
        (short)(MAX_ACOUNT * MAX_BCOUNT),                                                      // Դ cIdx
        (short)(MAX_ACOUNT),                                                                   // Ŀ�� cIdx
        (unsigned short)MAX_CCOUNT                                                             // cCnt
    },
    {
        (unsigned int)(1 << 2 | 1 << 20 | (EDMA3_CHA_GPIO_BNKINT5 << EDMA3CC_OPT_TCC_SHIFT)),  // Opt
        (unsigned int)SOC_EMIFA_CS2_ADDR,                                                      // Դ��ַ
        (unsigned short)(MAX_ACOUNT),                                                          // aCnt
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt
        (unsigned int)buf2,                                                                    // Ŀ���ַ
        (short)(MAX_ACOUNT),                                                                   // Դ bIdx
        (short)(MAX_ACOUNT * MAX_CCOUNT),                                                      // Ŀ�� bIdx
        (unsigned short)(32u * 41u),                                                           // ���ӵ�ַ
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt ��װֵ
        (short)(MAX_ACOUNT * MAX_BCOUNT),                                                      // Դ cIdx
        (short)(MAX_ACOUNT),                                                                   // Ŀ�� cIdx
        (unsigned short)MAX_CCOUNT                                                             // cCnt
    },
    {
        (unsigned int)(1 << 2 | 1 << 20 | (EDMA3_CHA_GPIO_BNKINT5 << EDMA3CC_OPT_TCC_SHIFT)),  // Opt
        (unsigned int)SOC_EMIFA_CS2_ADDR,                                                      // Դ��ַ
        (unsigned short)(MAX_ACOUNT),                                                          // aCnt
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt
        (unsigned int)buf1,                                                                    // Ŀ���ַ
        (short)(MAX_ACOUNT),                                                                   // Դ bIdx
        (short)(MAX_ACOUNT * MAX_CCOUNT),                                                      // Ŀ�� bIdx
        (unsigned short)(32u * 40u),                                                           // ���ӵ�ַ
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt ��װֵ
        (short)(MAX_ACOUNT * MAX_BCOUNT),                                                      // Դ cIdx
        (short)(MAX_ACOUNT),                                                                   // Ŀ�� cIdx
        (unsigned short)MAX_CCOUNT                                                             // cCnt
    }
};

// DMA �������״̬
volatile int irqRaised;

unsigned int pingpong = 1;

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void (*cb_Fxn[EDMA3_NUM_TCC])(unsigned int tcc, unsigned int status, void *appData);

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��Դʱ��ʹ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void PSCInit()
{
    // ʹ�� EDMA3CC_0
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_CC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // ʹ�� EDMA3TC_0
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_TC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �ص�����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void EDMA3IntCallback(unsigned int tccNum, unsigned int status, void *appData)
{
    (void)tccNum;
    (void)appData;

    if(EDMA3_XFER_COMPLETE == status)
    {
        if(pingpong)
        {
            CacheInv((unsigned int)buf1, MAX_BUFFER_SIZE);

        	memcpy(ADCDataCH0, &buf1[MAX_ACOUNT * MAX_CCOUNT * 0], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH1, &buf1[MAX_ACOUNT * MAX_CCOUNT * 1], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH2, &buf1[MAX_ACOUNT * MAX_CCOUNT * 2], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH3, &buf1[MAX_ACOUNT * MAX_CCOUNT * 3], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH4, &buf1[MAX_ACOUNT * MAX_CCOUNT * 4], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH5, &buf1[MAX_ACOUNT * MAX_CCOUNT * 5], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH6, &buf1[MAX_ACOUNT * MAX_CCOUNT * 6], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH7, &buf1[MAX_ACOUNT * MAX_CCOUNT * 7], MAX_ACOUNT * MAX_CCOUNT);
        }
        else
        {
            CacheInv((unsigned int)buf1, MAX_BUFFER_SIZE);

        	memcpy(ADCDataCH0, &buf2[MAX_ACOUNT * MAX_CCOUNT * 0], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH1, &buf2[MAX_ACOUNT * MAX_CCOUNT * 1], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH2, &buf2[MAX_ACOUNT * MAX_CCOUNT * 2], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH3, &buf2[MAX_ACOUNT * MAX_CCOUNT * 3], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH4, &buf2[MAX_ACOUNT * MAX_CCOUNT * 4], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH5, &buf2[MAX_ACOUNT * MAX_CCOUNT * 5], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH6, &buf2[MAX_ACOUNT * MAX_CCOUNT * 6], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH7, &buf2[MAX_ACOUNT * MAX_CCOUNT * 7], MAX_ACOUNT * MAX_CCOUNT);
        }
        pingpong = !pingpong;
    }
    else if(EDMA3_CC_DMA_EVT_MISS == status)
    {
		// ���䵼�� DMA �¼���ʧ����
		irqRaised = -1;
    }

    else if(EDMA3_CC_QDMA_EVT_MISS == status)
    {
    	// ���䵼�� QDMA �¼���ʧ����
		irqRaised = -2;
    }

    irqRaised = 1;
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �жϷ�����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void EDMA3CCComplIsr()
{
    volatile unsigned int pendingIrqs;
    volatile unsigned int isIPR = 0;

    unsigned int indexl;
    unsigned int Cnt = 0;
    indexl = 1u;

    IntEventClear(SYS_INT_EDMA3_0_CC0_INT1);

    isIPR = EDMA3GetIntrStatus(SOC_EDMA30CC_0_REGS);
    if(isIPR)
    {
        while((Cnt < EDMA3CC_COMPL_HANDLER_RETRY_COUNT)&& (indexl != 0u))
		{
			indexl = 0u;
			pendingIrqs = EDMA3GetIntrStatus(SOC_EDMA30CC_0_REGS);
			while (pendingIrqs)
			{
				if(TRUE == (pendingIrqs & 1u))
				{
					// ���û��ָ���ص����� ���޷���� IPR ��Ӧλ
					// д ICR ��� IPR ��Ӧλ
					EDMA3ClrIntr(SOC_EDMA30CC_0_REGS, indexl);
					(*cb_Fxn[indexl])(indexl, EDMA3_XFER_COMPLETE, NULL);
				}

				++indexl;
				pendingIrqs >>= 1u;
			}

			Cnt++;
		}
    }
}

void EDMA3CCErrIsr()
{
    volatile unsigned int pendingIrqs;
    unsigned int Cnt = 0u;
    unsigned int index;
    unsigned int evtqueNum = 0;  // �¼�������Ŀ

    pendingIrqs = 0u;
    index = 1u;

    IntEventClear(SYS_INT_EDMA3_0_CC0_ERRINT);

    if((EDMA3GetErrIntrStatus(SOC_EDMA30CC_0_REGS) != 0 )
        || (EDMA3QdmaGetErrIntrStatus(SOC_EDMA30CC_0_REGS) != 0)
        || (EDMA3GetCCErrStatus(SOC_EDMA30CC_0_REGS) != 0))
    {
        // ѭ�� EDMA3CC_ERR_HANDLER_RETRY_COUNT ��
    	// ֱ��û�еȴ��е��ж�ʱ��ֹ
        while((Cnt < EDMA3CC_ERR_HANDLER_RETRY_COUNT) && (index != 0u))
        {
            index = 0u;
            pendingIrqs = EDMA3GetErrIntrStatus(SOC_EDMA30CC_0_REGS);

            while(pendingIrqs)
            {
				// ִ�����еȴ��е��ж�
				if(TRUE == (pendingIrqs & 1u))
				{
					// ��� SER
					EDMA3ClrMissEvt(SOC_EDMA30CC_0_REGS, index);
				}
					++index;
					pendingIrqs >>= 1u;
			}

			index = 0u;
			pendingIrqs = EDMA3QdmaGetErrIntrStatus(SOC_EDMA30CC_0_REGS);

			while (pendingIrqs)
			{
				// ִ�����еȴ��е��ж�
				if(TRUE == (pendingIrqs & 1u))
				{
					// ��� SER
					EDMA3QdmaClrMissEvt(SOC_EDMA30CC_0_REGS, index);
				}
				++index;
				pendingIrqs >>= 1u;
			}

			index = 0u;
			pendingIrqs = EDMA3GetCCErrStatus(SOC_EDMA30CC_0_REGS);

			if(pendingIrqs != 0u)
			{
				// ִ�����еȴ��е� CC �����ж�
				// �¼����� ������ڴ���
				for(evtqueNum = 0u; evtqueNum < SOC_EDMA3_NUM_EVQUE; evtqueNum++)
				{
					if((pendingIrqs & (1u << evtqueNum)) != 0u)
					{
						// ��������ж�
						EDMA3ClrCCErr(SOC_EDMA30CC_0_REGS, (1u << evtqueNum));
					}
				}

				// ������ɴ���
				if((pendingIrqs & (1 << EDMA3CC_CCERR_TCCERR_SHIFT)) != 0u)
				{
					EDMA3ClrCCErr(SOC_EDMA30CC_0_REGS, (0x01u << EDMA3CC_CCERR_TCCERR_SHIFT));
				}

				++index;
			}

			Cnt++;
        }
    }
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      EDMA3 �жϳ�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void EDMA3InterruptInit()
{
    // ��������ж�
    IntRegister(C674X_MASK_INT7, EDMA3CCComplIsr);
    IntEventMap(C674X_MASK_INT7, SYS_INT_EDMA3_0_CC0_INT1);
    IntEnable(C674X_MASK_INT7);

    // ��������ж�
    IntRegister(C674X_MASK_INT8, EDMA3CCErrIsr);
    IntEventMap(C674X_MASK_INT8, SYS_INT_EDMA3_0_CC0_ERRINT);
    IntEnable(C674X_MASK_INT8);
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
//      EDMA3 ��ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void AD7606EDMA3Init()
{
    // ʹ������
    PSCInit();

    // EDMA3 �жϳ�ʼ��
    EDMA3InterruptInit();

    // EDMA3 ��ʼ��
    EDMA3Init(SOC_EDMA30CC_0_REGS, EVT_QUEUE_NUM);

    // ���� DMA ͨ��
    unsigned int retVal = 0u;
    retVal = EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
                                 EDMA3_CHA_GPIO_BNKINT5, EDMA3_CHA_GPIO_BNKINT5, EVT_QUEUE_NUM);

    // ע��ص�����
    cb_Fxn[EDMA3_CHA_GPIO_BNKINT5] = &EDMA3IntCallback;

    if(retVal == TRUE)
    {
        // ��ʼ�� DMA ����
        EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, EDMA3_CHA_GPIO_BNKINT5, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[0])));
        EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, 40, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[1])));
        EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, 41, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[2])));

        // ʹ�ܴ���
        EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_GPIO_BNKINT5, EDMA3_TRIG_MODE_EVENT);
    }
}
