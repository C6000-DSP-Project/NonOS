// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ͨ���첽���� 2(�ж�ģʽ)
//
//      2022��04��24��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    �����շ�
 *
 *    ע��: UART0, EDMA, SPI0, MMC/SD0/1, VPIF, LCDC, SATA, uPP, DDR2/mDDR(����), USB2.0, HPI, PRU
 *         ʱ��ԴΪ PLL0_SYSCLK2 Ĭ��Ƶ��Ϊ CPU Ƶ�� 1/2
 *         ECAP0/1/2, UART1/2, Timer64P2/3, eHRPWM0/1, McBSP0/1, McASP0, SPI1
 *         ��Щ����ʱ��Դ������ PLL0_SYSCLK2(Ĭ��)�� PLL1_SYSCLK2 ��ѡ��
 *
 *         �޸� System Configuration (SYSCFG) Module �Ĵ��� Chip Configuration 3 Register (CFGCHIP3)
 *         ASYNC3_CLKSRC ��
 *         ��Ĭ��ֵ�� 0 ʹ�� PLL0_SYSCLK2
 *                 1 ʹ�� PLL1_SYSCLK2
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

#include "interrupt.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �궨��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ʱ��
#define SYSCLK_1_FREQ     (456000000)
#define SYSCLK_2_FREQ     (SYSCLK_1_FREQ / 2)

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ȫ�ֱ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
char txData[] = "\r\nCoreKernel UART2 Example...\r\n";

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
//      �жϷ�����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// StarterWare system_config.lib �������ڲ���ʹ�� interrupt �ؼ������� �˴���Ϊ�ص�����
void UARTIsr()
{
    static unsigned int length = sizeof(txData);
    static unsigned int count = 0;
    unsigned char rxData = 0;
    unsigned int int_id = 0;

    // ȷ���ж�Դ
    int_id = UARTIntStatus(SOC_UART_2_REGS);

    // ��� UART2 ϵͳ�ж�
    IntEventClear(SYS_INT_UART2_INT);

    // �����ж�
    if(UART_INTID_TX_EMPTY == int_id)
    {
        if(0 < length)
        {
            // дһ���ֽڵ� THR
            UARTCharPutNonBlocking(SOC_UART_2_REGS, txData[count]);
            length--;
            count++;
        }
        if(0 == length)
        {
            // ���÷����ж�
            UARTIntDisable(SOC_UART_2_REGS, UART_INT_TX_EMPTY);
        }
     }

    // �����ж�
    if(UART_INTID_RX_DATA == int_id)
    {
        // �����ַ�
        rxData = UARTCharGetNonBlocking(SOC_UART_2_REGS);
        UARTCharPutNonBlocking(SOC_UART_2_REGS, rxData);
    }

    // ���մ���
    if(UART_INTID_RX_LINE_STAT == int_id)
    {
        while(UARTRxErrorGet(SOC_UART_2_REGS))
        {
            // �� RBR ��һ���ֽ�
            UARTCharGetNonBlocking(SOC_UART_2_REGS);
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
    IntRegister(C674X_MASK_INT4, UARTIsr);

    // ӳ���жϵ� DSP �������ж�
    IntEventMap(C674X_MASK_INT4, SYS_INT_UART2_INT);

    // ʹ�� DSP �������ж�
    IntEnable(C674X_MASK_INT4);
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

    // ʹ�ܴ����ж�
    UARTIntEnable(SOC_UART_2_REGS, UART_INT_LINE_STAT | UART_INT_TX_EMPTY | UART_INT_RXDATA_CTI);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void main()
{
    // ʹ������
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_UART2, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // �ܽŸ�������
    GPIOBankPinMuxSet();

    // DSP �жϳ�ʼ��
    InterruptInit();

    // ���ڳ�ʼ��(��Ҫ�ѳ�ʼ�� DSP �жϲ�����������)
    UARTInit();

    for(;;)
    {

    }
}
