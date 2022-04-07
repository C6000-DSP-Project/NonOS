/****************************************************************************/
/*                                                                          */
/*              ��ʱ�� / ����������                                         */
/*                                                                          */
/*              2014��06��01��                                              */
/*                                                                          */
/****************************************************************************/
// ע�⣺DSP ports, Shared RAM, UART0, EDMA, SPI0, MMC/SDs,
//       VPIF, LCDC, SATA, uPP, DDR2/mDDR (bus ports), USB2.0, HPI, PRU
//       ��Щ����ʹ�õ�ʱ����ԴΪ PLL0_SYSCLK2 Ĭ��Ƶ��Ϊ CPU Ƶ�ʵĶ���֮һ
//       ���ǣ�ECAPs, UART1/2, Timer64P2/3, eHRPWMs,McBSPs, McASP0, SPI1
//       ��Щ�����ʱ����Դ������ PLL0_SYSCLK2 �� PLL1_SYSCLK2 ��ѡ��
//       ͨ���޸� System Configuration (SYSCFG) Module
//       �Ĵ��� Chip Configuration 3 Register (CFGCHIP3) ����λ ASYNC3_CLKSRC
//       ����ʱ����Դ
//       ��Ĭ��ֵ�� 0 ��Դ�� PLL0_SYSCLK2
//                  1 ��Դ�� PLL1_SYSCLK2
//       �������Ϊ�˽��͹��ģ��������޸����ֵ������Ӱ��������������ʱ��Ƶ��

#include <stdio.h>

#include "hw_types.h"
#include "hw_syscfg0_C6748.h"
#include "soc_C6748.h"

#include "timer.h"

#include "interrupt.h"

/****************************************************************************/
/*                                                                          */
/*              �궨��                                                      */
/*                                                                          */
/****************************************************************************/
// 64λ ��ʱ�� / ����������
// ��ʱʱ��  1ms
#define TMR_PERIOD_LSB32  (228 * 1000)  // ��32λ
#define TMR_PERIOD_MSB32  (0)           // ��32λ 0

/****************************************************************************/
/*                                                                          */
/*              �жϷ�����                                                */
/*                                                                          */
/****************************************************************************/
void TimerIsr(void)
{
    // ���ö�ʱ�� / �������ж�
    TimerIntDisable(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);

    // ����жϱ�־
    IntEventClear(SYS_INT_T64P2_TINTALL);
    TimerIntStatusClear(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);

    lv_tick_inc(1);

    // ʹ�� ��ʱ�� / ������ �ж�
    TimerIntEnable(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);
}

/****************************************************************************/
/*                                                                          */
/*              �жϳ�ʼ��                                   */
/*                                                                          */
/****************************************************************************/
static void InterruptInit()
{
    // ע���жϷ�����
    IntRegister(C674X_MASK_INT6, TimerIsr);

    // ӳ���жϵ� DSP �������ж�
    IntEventMap(C674X_MASK_INT6, SYS_INT_T64P2_TINTALL);

    // ʹ�� DSP �������ж�
    IntEnable(C674X_MASK_INT6);

    // ʹ�� ��ʱ�� / ������ �ж�
    TimerIntEnable(SOC_TMR_2_REGS, TMR_INT_TMR12_NON_CAPT_MODE);
}

/****************************************************************************/
/*                                                                          */
/*              ��ʱ����ʼ��                                                  */
/*                                                                          */
/****************************************************************************/
void TimerInit()
{
    // ��ʱ�� / ��������ʼ��
    // ���� ��ʱ�� / ������ 2 Ϊ 64 λģʽ
    TimerConfigure(SOC_TMR_2_REGS, TMR_CFG_64BIT_CLK_INT);

    // ��������
    TimerPeriodSet(SOC_TMR_2_REGS, TMR_TIMER12, TMR_PERIOD_LSB32);
    TimerPeriodSet(SOC_TMR_2_REGS, TMR_TIMER34, TMR_PERIOD_MSB32);

    // ʹ�� ��ʱ�� / ������ 2
    TimerEnable(SOC_TMR_2_REGS, TMR_TIMER12, TMR_ENABLE_CONT);

    // DSP �жϳ�ʼ��
    InterruptInit();
}
