// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO KEY ����
//
//      2022��04��24��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    �����ж�
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

#include "interrupt.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽŸ�������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinMuxSet()
{
    // ������Ӧ�� GPIO �ڹ���Ϊ��ͨ���������
    // �װ� LED
    // GPIO2[15]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(05)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(05)) & (~(SYSCFG_PINMUX5_PINMUX5_3_0))) |
                                                    ((SYSCFG_PINMUX5_PINMUX5_3_0_GPIO2_15 << SYSCFG_PINMUX5_PINMUX5_3_0_SHIFT));

    // GPIO4[00]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(10)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(10)) & (~(SYSCFG_PINMUX10_PINMUX10_31_28))) |
                                                    ((SYSCFG_PINMUX10_PINMUX10_31_28_GPIO4_0 << SYSCFG_PINMUX10_PINMUX10_31_28_SHIFT));

    // ����
    // GPIO0[8]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(00)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(00)) & (~(SYSCFG_PINMUX0_PINMUX0_31_28))) |
                                                    ((SYSCFG_PINMUX0_PINMUX0_31_28_GPIO0_8 << SYSCFG_PINMUX0_PINMUX0_31_28_SHIFT));

    // GPIO8[12]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(18)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(18)) & (~(SYSCFG_PINMUX18_PINMUX18_23_20))) |
                                                    ((SYSCFG_PINMUX18_PINMUX18_23_20_GPIO8_12 << SYSCFG_PINMUX18_PINMUX18_23_20_SHIFT));
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽų�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinInit()
{
    // �װ� LED
    GPIODirModeSet(SOC_GPIO_0_REGS, 48, GPIO_DIR_OUTPUT);               // GPIO2[15] LED4
    GPIODirModeSet(SOC_GPIO_0_REGS, 65, GPIO_DIR_OUTPUT);               // GPIO4[00] LED3

    // ����
    // �����жϴ�����ʽ
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 9, GPIO_INT_TYPE_FALLEDGE);         // SW6 �½���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 141, GPIO_INT_TYPE_FALLEDGE);       // SW4 �����ؼ��½���

    // ʹ�� GPIO BANK �ж�
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 0);                              // GPIO BANK0
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 8);                              // GPIO BANK8
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
//      �жϷ�����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// StarterWare system_config.lib �������ڲ���ʹ�� interrupt �ؼ������� �˴���Ϊ�ص�����
void KEY4Isr()
{
    // ����ж�״̬
    IntEventClear(SYS_INT_GPIO_B8INT);

    if(GPIOPinIntStatus(SOC_GPIO_0_REGS, 141) == GPIO_INT_PEND)
    {
        GPIOPinWrite(SOC_GPIO_0_REGS, 65, GPIO_PIN_HIGH);
        Delay(0x00FFFFFF);  // �˴���Ϊ��ʾ���� �������жϷ�����ʹ���������

        GPIOPinWrite(SOC_GPIO_0_REGS, 65, GPIO_PIN_LOW);
        Delay(0x00FFFFFF);
    }

    // ��� GPIO �ж�״̬
    GPIOPinIntClear(SOC_GPIO_0_REGS, 141);
}

void KEY6Isr()
{
    // ����ж�״̬
    IntEventClear(SYS_INT_GPIO_B0INT);

    if(GPIOPinIntStatus(SOC_GPIO_0_REGS, 9) == GPIO_INT_PEND)
    {
        // ��˸ LED
        GPIOPinWrite(SOC_GPIO_0_REGS, 48, GPIO_PIN_HIGH);
        Delay(0x00FFFFFF);  // �˴���Ϊ��ʾ���� �������жϷ�����ʹ���������

        GPIOPinWrite(SOC_GPIO_0_REGS, 48, GPIO_PIN_LOW);
        Delay(0x00FFFFFF);
    }

    // ��� GPIO �ж�״̬
    GPIOPinIntClear(SOC_GPIO_0_REGS, 9);
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
    IntRegister(C674X_MASK_INT4, KEY4Isr);
    IntRegister(C674X_MASK_INT5, KEY6Isr);

    // ӳ���жϵ� DSP �������ж�
    IntEventMap(C674X_MASK_INT4, SYS_INT_GPIO_B8INT);
    IntEventMap(C674X_MASK_INT5, SYS_INT_GPIO_B0INT);

    // ʹ�� DSP �������ж�
    IntEnable(C674X_MASK_INT4);
    IntEnable(C674X_MASK_INT5);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void main()
{
    // ʹ������
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // �ܽŸ�������
    GPIOBankPinMuxSet();

    // GPIO �ܽų�ʼ��
    GPIOBankPinInit();

    // DSP �жϳ�ʼ��
    InterruptInit();

    for(;;)
    {

    }
}
