// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO KEYBOARD
//
//      2022��04��24��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    ����
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
//      ȫ�ֱ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
unsigned char KEYNum = 0;

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
    // GPIO5[0] - GPIO5[7]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(12)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(12)) &
                                                  (~(SYSCFG_PINMUX12_PINMUX12_31_28 |
                                                     SYSCFG_PINMUX12_PINMUX12_27_24 |
                                                     SYSCFG_PINMUX12_PINMUX12_23_20 |
                                                     SYSCFG_PINMUX12_PINMUX12_19_16 |
                                                     SYSCFG_PINMUX12_PINMUX12_15_12 |
                                                     SYSCFG_PINMUX12_PINMUX12_11_8 |
                                                     SYSCFG_PINMUX12_PINMUX12_7_4 |
                                                     SYSCFG_PINMUX12_PINMUX12_3_0))) |
                                                   ((SYSCFG_PINMUX12_PINMUX12_31_28_GPIO5_0 << SYSCFG_PINMUX12_PINMUX12_31_28_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_27_24_GPIO5_1 << SYSCFG_PINMUX12_PINMUX12_27_24_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_23_20_GPIO5_2 << SYSCFG_PINMUX12_PINMUX12_23_20_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_19_16_GPIO5_3 << SYSCFG_PINMUX12_PINMUX12_19_16_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_15_12_GPIO5_4 << SYSCFG_PINMUX12_PINMUX12_15_12_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_11_8_GPIO5_5  << SYSCFG_PINMUX12_PINMUX12_11_8_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_7_4_GPIO5_6   << SYSCFG_PINMUX12_PINMUX12_7_4_SHIFT) |
                                                    (SYSCFG_PINMUX12_PINMUX12_3_0_GPIO5_7   << SYSCFG_PINMUX12_PINMUX12_3_0_SHIFT));

    // GPIO5[8] - GPIO5[9]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) &
                                                  (~(SYSCFG_PINMUX11_PINMUX11_31_28 |
                                                     SYSCFG_PINMUX11_PINMUX11_27_24))) |
                                                   ((SYSCFG_PINMUX11_PINMUX11_31_28_GPIO5_8 << SYSCFG_PINMUX11_PINMUX11_31_28_SHIFT) |
                                                    (SYSCFG_PINMUX11_PINMUX11_27_24_GPIO5_9 << SYSCFG_PINMUX11_PINMUX11_27_24_SHIFT));

    // GPIO0[8] ���ܼ�
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) &
                                                  (~(SYSCFG_PINMUX0_PINMUX0_31_28))) |
                                                   ((SYSCFG_PINMUX0_PINMUX0_31_28_GPIO0_8 << SYSCFG_PINMUX0_PINMUX0_31_28_SHIFT));
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽų�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinInit()
{
    // �װ� LED
    GPIODirModeSet(SOC_GPIO_0_REGS, 48, GPIO_DIR_OUTPUT);   // GPIO2[15] LED4
    GPIODirModeSet(SOC_GPIO_0_REGS, 65, GPIO_DIR_OUTPUT);   // GPIO4[00] LED3

    // ����
    // �����жϴ�����ʽ
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 81, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[00] �½���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 82, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[01] �½���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 83, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[02] �½���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 84, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[03] �½���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 85, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[04] �½���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 86, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[05] �½���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 87, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[06] �½���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 88, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[07] �½���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 89, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[08] �½���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 90, GPIO_INT_TYPE_FALLEDGE);        // GPIO5[09] �½���

    GPIOIntTypeSet(SOC_GPIO_0_REGS, 9, GPIO_INT_TYPE_FALLEDGE);         // GPIO0[08] �½���

    // ʹ�� GPIO BANK �ж�
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 0);                              // GPIO BANK0
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 5);                              // GPIO BANK5
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
void KEYNumIsr()
{
    // ����ж�״̬
    IntEventClear(SYS_INT_GPIO_B5INT);

    unsigned int KEYInt = (HWREG(SOC_GPIO_0_REGS + GPIO_INTSTAT(2)) & 0x03FF0000) >> 16;

    switch(KEYInt)
    {
        case 0x001:  // ���� 0
            KEYNum = 0;
            break;

        case 0x002:  // ���� 1
            KEYNum = 1;
            break;

        case 0x004:  // ���� 2
            KEYNum = 2;
            break;

        case 0x008:  // ���� 3
            KEYNum = 3;
            break;

        case 0x010:  // ���� 4
            KEYNum = 4;
            break;

        case 0x020:  // ���� 5
            KEYNum = 5;
            break;

        case 0x040:  // ���� 6
            KEYNum = 6;
            break;

        case 0x080:  // ���� 7
            KEYNum = 7;
            break;

        case 0x100:  // ���� 8
            KEYNum = 8;
            break;

        case 0x200:  // ���� 9
            KEYNum = 9;
            break;

        default:
            break;
    }

    // ��˸ LED
    unsigned char i;
    for(i = 0; i < KEYNum; i++)
    {
        GPIOPinWrite(SOC_GPIO_0_REGS, 48, GPIO_PIN_HIGH);
        Delay(0x00FFFFFF);  // �˴���Ϊ��ʾ���� �������жϷ�����ʹ���������

        GPIOPinWrite(SOC_GPIO_0_REGS, 48, GPIO_PIN_LOW);
        Delay(0x00FFFFFF);
    }

    // ��� GPIO �ж�״̬
    GPIOPinIntClear(SOC_GPIO_0_REGS, 81 + KEYNum);
}

void KEYFUNIsr()
{
    // ����ж�״̬
    IntEventClear(SYS_INT_GPIO_B0INT);

    if(GPIOPinIntStatus(SOC_GPIO_0_REGS, 9) == GPIO_INT_PEND)
    {
        // ��˸ LED
        GPIOPinWrite(SOC_GPIO_0_REGS, 65, GPIO_PIN_HIGH);
        Delay(0x00FFFFFF);  // �˴���Ϊ��ʾ���� �������жϷ�����ʹ���������
        GPIOPinWrite(SOC_GPIO_0_REGS, 65, GPIO_PIN_LOW);
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
    IntRegister(C674X_MASK_INT4, KEYNumIsr);
    IntRegister(C674X_MASK_INT5, KEYFUNIsr);

    // ӳ���жϵ� DSP �������ж�
    IntEventMap(C674X_MASK_INT4, SYS_INT_GPIO_B5INT);
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
