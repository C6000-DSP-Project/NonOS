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
#include "emifa.h"
#include "timer.h"

#include "interrupt.h"

#include "uartStdio.h"

#include <stdio.h>

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ȫ�ֱ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ADC ����
short ADCDataCH0[1024];
short ADCDataCH1[1024];
short ADCDataCH2[1024];
short ADCDataCH3[1024];
short ADCDataCH4[1024];
short ADCDataCH5[1024];
short ADCDataCH6[1024];
short ADCDataCH7[1024];

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
extern void AD7606EDMA3Init();

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��Դʱ��ʹ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void PSCInit()
{
    // ʹ�� GPIO ģ��
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // ʹ�� EMIFA ģ��
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_EMIFA, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽŸ�������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinMuxSet()
{
    // ������Ӧ�� GPIO �ڹ���Ϊ EMIFA �˿�
    // EMIFA D0-D15
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(9)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(9)) &
                                                   (~(SYSCFG_PINMUX9_PINMUX9_31_28  |
                                                      SYSCFG_PINMUX9_PINMUX9_27_24  |
                                                      SYSCFG_PINMUX9_PINMUX9_23_20  |
                                                      SYSCFG_PINMUX9_PINMUX9_19_16  |
                                                      SYSCFG_PINMUX9_PINMUX9_15_12  |
                                                      SYSCFG_PINMUX9_PINMUX9_11_8   |
                                                      SYSCFG_PINMUX9_PINMUX9_7_4    |
                                                      SYSCFG_PINMUX9_PINMUX9_3_0))) |
                                                    ((SYSCFG_PINMUX9_PINMUX9_31_28_EMA_D0 << SYSCFG_PINMUX9_PINMUX9_31_28_SHIFT) |
                                                     (SYSCFG_PINMUX9_PINMUX9_27_24_EMA_D1 << SYSCFG_PINMUX9_PINMUX9_27_24_SHIFT) |
                                                     (SYSCFG_PINMUX9_PINMUX9_23_20_EMA_D2 << SYSCFG_PINMUX9_PINMUX9_23_20_SHIFT) |
                                                     (SYSCFG_PINMUX9_PINMUX9_19_16_EMA_D3 << SYSCFG_PINMUX9_PINMUX9_19_16_SHIFT) |
                                                     (SYSCFG_PINMUX9_PINMUX9_15_12_EMA_D4 << SYSCFG_PINMUX9_PINMUX9_15_12_SHIFT) |
                                                     (SYSCFG_PINMUX9_PINMUX9_11_8_EMA_D5  << SYSCFG_PINMUX9_PINMUX9_11_8_SHIFT)  |
                                                     (SYSCFG_PINMUX9_PINMUX9_7_4_EMA_D6   << SYSCFG_PINMUX9_PINMUX9_7_4_SHIFT)   |
                                                     (SYSCFG_PINMUX9_PINMUX9_3_0_EMA_D7   << SYSCFG_PINMUX9_PINMUX9_3_0_SHIFT));

    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(8)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(8)) &
                                                   (~(SYSCFG_PINMUX8_PINMUX8_31_28  |
                                                      SYSCFG_PINMUX8_PINMUX8_27_24  |
                                                      SYSCFG_PINMUX8_PINMUX8_23_20  |
                                                      SYSCFG_PINMUX8_PINMUX8_19_16  |
                                                      SYSCFG_PINMUX8_PINMUX8_15_12  |
                                                      SYSCFG_PINMUX8_PINMUX8_11_8   |
                                                      SYSCFG_PINMUX8_PINMUX8_7_4    |
                                                      SYSCFG_PINMUX8_PINMUX8_3_0))) |
                                                    ((SYSCFG_PINMUX8_PINMUX8_31_28_EMA_D8  << SYSCFG_PINMUX8_PINMUX8_31_28_SHIFT) |
                                                     (SYSCFG_PINMUX8_PINMUX8_27_24_EMA_D9  << SYSCFG_PINMUX8_PINMUX8_27_24_SHIFT) |
                                                     (SYSCFG_PINMUX8_PINMUX8_23_20_EMA_D10 << SYSCFG_PINMUX8_PINMUX8_23_20_SHIFT) |
                                                     (SYSCFG_PINMUX8_PINMUX8_19_16_EMA_D11 << SYSCFG_PINMUX8_PINMUX8_19_16_SHIFT) |
                                                     (SYSCFG_PINMUX8_PINMUX8_15_12_EMA_D12 << SYSCFG_PINMUX8_PINMUX8_15_12_SHIFT) |
                                                     (SYSCFG_PINMUX8_PINMUX8_11_8_EMA_D13  << SYSCFG_PINMUX8_PINMUX8_11_8_SHIFT)  |
                                                     (SYSCFG_PINMUX8_PINMUX8_7_4_EMA_D14   << SYSCFG_PINMUX8_PINMUX8_7_4_SHIFT)   |
                                                     (SYSCFG_PINMUX8_PINMUX8_3_0_EMA_D15   << SYSCFG_PINMUX8_PINMUX8_3_0_SHIFT));

    // CS2
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) &
                                                   (~(SYSCFG_PINMUX7_PINMUX7_3_0))) |
                                                    ((SYSCFG_PINMUX7_PINMUX7_3_0_NEMA_CS2 << SYSCFG_PINMUX7_PINMUX7_3_0_SHIFT));

    // ������Ӧ�� GPIO �ڹ���Ϊ��ͨ���������
    // RESET
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                        (~(SYSCFG_PINMUX11_PINMUX11_15_12))) |
                                                         ((SYSCFG_PINMUX11_PINMUX11_15_12_GPIO5_12 << SYSCFG_PINMUX11_PINMUX11_15_12_SHIFT));

    // CONVSTA �� CONVST
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                        (~(SYSCFG_PINMUX11_PINMUX11_11_8))) |
                                                         ((SYSCFG_PINMUX11_PINMUX11_11_8_GPIO5_13 << SYSCFG_PINMUX11_PINMUX11_11_8_SHIFT));

    // CONVSTB �� WE
    // AD7606 CONVSTA ת��ǰ 4 ͨ�� CONVSTB ת���� 4 ͨ��
    // AD7606B CONVST ת������ͨ�� WE ����д�ڲ�����Ĵ���
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) &
                                                       (~(SYSCFG_PINMUX7_PINMUX7_19_16))) |
                                                        ((SYSCFG_PINMUX7_PINMUX7_19_16_GPIO3_11 << SYSCFG_PINMUX7_PINMUX7_19_16_SHIFT));

    // BUSY
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                        (~(SYSCFG_PINMUX11_PINMUX11_19_16))) |
                                                         ((SYSCFG_PINMUX11_PINMUX11_19_16_GPIO5_11 << SYSCFG_PINMUX11_PINMUX11_19_16_SHIFT));

    // FRSTDATA
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                        (~(SYSCFG_PINMUX11_PINMUX11_23_20))) |
                                                         ((SYSCFG_PINMUX11_PINMUX11_23_20_GPIO5_10 << SYSCFG_PINMUX11_PINMUX11_23_20_SHIFT));

    // RANGE �����ѹ��Χ
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(6)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(6)) &
                                                       (~(SYSCFG_PINMUX7_PINMUX7_15_12))) |
                                                        ((SYSCFG_PINMUX7_PINMUX7_15_12_GPIO3_12 << SYSCFG_PINMUX7_PINMUX7_15_12_SHIFT));
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽų�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinInit()
{
    // ���� GPIO5[11] / BUSY Ϊ����ģʽ
    GPIODirModeSet(SOC_GPIO_0_REGS, 92, GPIO_DIR_INPUT);

    // �����жϴ�����ʽ
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 92, GPIO_INT_TYPE_RISEDGE);

    // ʹ�� GPIO BANK �ж�
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 5);

    // ���� GPIO5[12] / RESET Ϊ���ģʽ
    GPIODirModeSet(SOC_GPIO_0_REGS, 93, GPIO_DIR_OUTPUT);
    GPIOPinWrite(SOC_GPIO_0_REGS, 93, GPIO_PIN_HIGH);

    // ���� GPIO5[13] / CONVSTA Ϊ���ģʽ
    GPIODirModeSet(SOC_GPIO_0_REGS, 94, GPIO_DIR_OUTPUT);
    GPIOPinWrite(SOC_GPIO_0_REGS, 94, GPIO_PIN_LOW);

    // ���� GPIO3[11] / CONVSTB Ϊ���ģʽ
    GPIODirModeSet(SOC_GPIO_0_REGS, 60, GPIO_DIR_OUTPUT);
    GPIOPinWrite(SOC_GPIO_0_REGS, 60, GPIO_PIN_LOW);

    // ���� GPIO3[12] / RANGE Ϊ����ģʽ
    GPIODirModeSet(SOC_GPIO_0_REGS, 61, GPIO_DIR_INPUT);
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
//      ADC ��λ
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void ADCReset()
{
    GPIOPinWrite(SOC_GPIO_0_REGS, 93, GPIO_PIN_HIGH);
    Delay(0x1FFF);
    GPIOPinWrite(SOC_GPIO_0_REGS, 93, GPIO_PIN_LOW);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ADC ����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void ADCStart()
{
    // CONVSTA
    GPIOPinWrite(SOC_GPIO_0_REGS, 94, GPIO_PIN_LOW);
    GPIOPinWrite(SOC_GPIO_0_REGS, 60, GPIO_PIN_LOW);

    // CONVSTB
    GPIOPinWrite(SOC_GPIO_0_REGS, 94, GPIO_PIN_HIGH);
    GPIOPinWrite(SOC_GPIO_0_REGS, 60, GPIO_PIN_HIGH);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��ȡ�����ѹ��Χ
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static unsigned char ADCRangeGet()
{
    // 1 -10V - +10V
    // 0 -5V - +5V
    return GPIOPinRead(SOC_GPIO_0_REGS, 61);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      EMIF ��ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void EMIFInit()
{
    // ������������ 16bit
    EMIFAAsyncDevDataBusWidthSelect(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_DATA_BUSWITTH_16BIT);

    // ѡ�� Normal ģʽ
    EMIFAAsyncDevOpModeSelect(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_ASYNC_INTERFACE_NORMAL_MODE);

    // ��ֹWAIT����
    EMIFAExtendedWaitConfig(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_EXTENDED_WAIT_DISABLE);

    // ���� W_SETUP/R_SETUP W_STROBE/R_STROBE W_HOLD/R_HOLD   TA �Ȳ���
    EMIFAWaitTimingConfig(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_ASYNC_WAITTIME_CONFIG(4, 6, 4, 4, 6, 4, 0));
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �жϷ�����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
short EMIFData[8];
void TimerIsr()
{
    // ����жϱ�־
    IntEventClear(SYS_INT_T64P2_TINTALL);

    // �����ʱ���жϱ�־
    TimerIntStatusClear(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);

//    EMIFData[0] = ((short *)SOC_EMIFA_CS2_ADDR)[1];
//    EMIFData[1] = ((short *)SOC_EMIFA_CS2_ADDR)[2];
//    EMIFData[2] = ((short *)SOC_EMIFA_CS2_ADDR)[3];
//    EMIFData[3] = ((short *)SOC_EMIFA_CS2_ADDR)[4];
//    EMIFData[4] = ((short *)SOC_EMIFA_CS2_ADDR)[5];
//    EMIFData[5] = ((short *)SOC_EMIFA_CS2_ADDR)[6];
//    EMIFData[6] = ((short *)SOC_EMIFA_CS2_ADDR)[7];
//    EMIFData[7] = ((short *)SOC_EMIFA_CS2_ADDR)[8];

    // ���� ADC
    ADCStart();
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��ʱ��/��������ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void TimerInit(unsigned int SamplingRate)
{
    // ��������� 200KSPS
    if(SamplingRate > 200000)
    {
        SamplingRate = 200000;
    }

    // ���ö�ʱ��/������ 2 Ϊ 64 λģʽ
    TimerConfigure(SOC_TMR_2_REGS, TMR_CFG_64BIT_CLK_INT);

    // ��������
    TimerPeriodSet(SOC_TMR_2_REGS, TMR_TIMER12, 228000000 / SamplingRate);
    TimerPeriodSet(SOC_TMR_2_REGS, TMR_TIMER34, 0);

    // ʹ�ܶ�ʱ��/������ 2
    TimerEnable(SOC_TMR_2_REGS, TMR_TIMER12, TMR_ENABLE_CONT);

    // ʹ�ܶ�ʱ��/�������ж�
    TimerIntEnable(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);
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
    IntRegister(C674X_MASK_INT4, TimerIsr);

    // ӳ���жϵ� DSP �������ж�
    IntEventMap(C674X_MASK_INT4, SYS_INT_T64P2_TINTALL);

    // ʹ�� DSP �������ж�
    IntEnable(C674X_MASK_INT4);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void main()
{
    UARTStdioInit();
    UARTprintf("\r\nCoreKernel AD7606 EDMA Example...\r\n");

    // ʹ������
    PSCInit();

    // �ܽŸ�������
    GPIOBankPinMuxSet();

    // GPIO �ܽų�ʼ��
    GPIOBankPinInit();

    // DSP �жϳ�ʼ��
    InterruptInit();

    // EMIF ��ʼ��
    EMIFInit();

    // ��λ ADC
    ADCReset();

    // EDMA3 ��ʼ��
    AD7606EDMA3Init();

    // ��ʱ����ʼ�� 200KHz ������
    TimerInit(200000);

    // ��������ѹ��Χ����
    char ADCRange = ADCRangeGet();
    char *ADCRangStr[2] = {"-5V - +5V", "-10V - +10V"};
    UARTprintf("ADC Input Range %s\r\n", ADCRangStr[ADCRange]);

    for(;;)
    {

    }
}
