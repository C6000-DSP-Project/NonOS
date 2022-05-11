// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �߾��� PWM ���
//
//      2022��05��11��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    �߾��� PWM ������� LED �����
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
#include "ehrpwm.h"

#include "interrupt.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �궨��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ʱ�ӷ�Ƶ
#define CLOCK_DIV_VAL     228

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽŸ�������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinMuxSet()
{
    // EPWM1A
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) & (~(SYSCFG_PINMUX5_PINMUX5_3_0))) |
                                                   ((SYSCFG_PINMUX5_PINMUX5_3_0_EPWM1A << SYSCFG_PINMUX5_PINMUX5_3_0_SHIFT));

    // EPWM1B
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) & (~(SYSCFG_PINMUX5_PINMUX5_7_4))) |
                                                   ((SYSCFG_PINMUX5_PINMUX5_7_4_EPWM1B << SYSCFG_PINMUX5_PINMUX5_7_4_SHIFT));

    // EPWM1TZ[0]
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(2)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(2)) & (~(SYSCFG_PINMUX2_PINMUX2_3_0))) |
                                                   ((SYSCFG_PINMUX2_PINMUX2_3_0_EPWM1TZ0 << SYSCFG_PINMUX2_PINMUX2_3_0_SHIFT));

    // ʹ�� PWM ʱ��
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_CFGCHIP1) |= SYSCFG_CFGCHIP1_TBCLKSYNC;
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �жϷ�����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void PWMEventIsr()
{
    IntEventClear(SYS_INT_EHRPWM1);

    EHRPWMETIntClear(SOC_EHRPWM_1_REGS);
}

void PWMTZIsr()
{
    IntEventClear(SYS_INT_EHRPWM1TZ);

    EHRPWMTZFlagClear(SOC_EHRPWM_1_REGS, EHRPWM_TZ_CYCLEBYCYCLE_CLEAR);
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
    IntRegister(C674X_MASK_INT4, PWMEventIsr);
    IntRegister(C674X_MASK_INT5, PWMTZIsr);

    // ӳ���жϵ� DSP �������ж�
    IntEventMap(C674X_MASK_INT4, SYS_INT_EHRPWM1);
    IntEventMap(C674X_MASK_INT5, SYS_INT_EHRPWM1TZ);

    // ʹ�� DSP �������ж�
    IntEnable(C674X_MASK_INT4);
    IntEnable(C674X_MASK_INT5);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      PWM ��ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void EHRPWMInit(unsigned int pwm_clk, unsigned short duty_ratio)
{
    // ʱ���׼����
    // ʱ������
    EHRPWMTimebaseClkConfig(SOC_EHRPWM_1_REGS, SOC_EHRPWM_1_MODULE_FREQ / CLOCK_DIV_VAL, SOC_EHRPWM_1_MODULE_FREQ);

    // ��������
    EHRPWMPWMOpFreqSet(SOC_EHRPWM_1_REGS, SOC_EHRPWM_1_MODULE_FREQ / CLOCK_DIV_VAL, pwm_clk, EHRPWM_COUNT_UP, EHRPWM_SHADOW_WRITE_DISABLE);

    // ��������ͬ���ź�
    EHRPWMTimebaseSyncDisable(SOC_EHRPWM_1_REGS);

    // �������ͬ���ź�
    EHRPWMSyncOutModeSet(SOC_EHRPWM_1_REGS, EHRPWM_SYNCOUT_DISABLE);

    // ����ģʽ��Ϊ����
    EHRPWMTBEmulationModeSet(SOC_EHRPWM_1_REGS, EHRPWM_STOP_AFTER_NEXT_TB_INCREMENT);

    // ���ü����Ƚ�����ģ��
    // ���رȽ��� A ֵ
    EHRPWMLoadCMPA(SOC_EHRPWM_1_REGS, (SOC_EHRPWM_1_MODULE_FREQ/CLOCK_DIV_VAL / pwm_clk) * duty_ratio / 100, EHRPWM_SHADOW_WRITE_DISABLE, EHRPWM_COMPA_NO_LOAD, EHRPWM_CMPCTL_OVERWR_SH_FL);

    // ���رȽ��� B ֵ
    EHRPWMLoadCMPB(SOC_EHRPWM_1_REGS, 0, EHRPWM_SHADOW_WRITE_DISABLE, EHRPWM_COMPB_NO_LOAD, EHRPWM_CMPCTL_OVERWR_SH_FL);

    // �����޶����ã�������Ŵ�����ʽ�趨��
    // ʱ���׼����������Ч�����ȽϼĴ��� A/B ֵʱ EPWM1_A ��ת ������ EPWM1_A ���
    EHRPWMConfigureAQActionOnA(SOC_EHRPWM_1_REGS, EHRPWM_AQCTLA_ZRO_DONOTHING, EHRPWM_AQCTLA_PRD_DONOTHING,
                                 EHRPWM_AQCTLA_CAU_EPWMXATOGGLE, EHRPWM_AQCTLA_CAD_DONOTHING, EHRPWM_AQCTLA_CBU_EPWMXATOGGLE,
                                 EHRPWM_AQCTLA_CBD_DONOTHING, EHRPWM_AQSFRC_ACTSFA_DONOTHING);

    // ��������ģ�飨��·��ģ�� �ź�ֱ�������ն����ģ�飩
    EHRPWMDBOutput(SOC_EHRPWM_1_REGS, EHRPWM_DBCTL_OUT_MODE_BYPASS);

    // ����ն����ģ��
    EHRPWMChopperDisable(SOC_EHRPWM_1_REGS);

    // ���ô�������¼�
    EHRPWMTZTripEventDisable(SOC_EHRPWM_1_REGS, EHRPWM_TZ_ONESHOT);
    EHRPWMTZTripEventDisable(SOC_EHRPWM_1_REGS, EHRPWM_TZ_CYCLEBYCYCLE);

    // �¼���������
    // ÿ�����¼����������ж�
    EHRPWMETIntPrescale(SOC_EHRPWM_1_REGS, EHRPWM_ETPS_INTPRD_THIRDEVENT);

    // ʱ���׼����������Ч�����ȽϼĴ��� B ֵ �����¼�
    EHRPWMETIntSourceSelect(SOC_EHRPWM_1_REGS, EHRPWM_ETSEL_INTSEL_TBCTREQUCMPBINC);

    // ʹ���ж�
    EHRPWMETIntEnable(SOC_EHRPWM_1_REGS);

    // ���ø߾�����ģ��
    EHRPWMHRDisable(SOC_EHRPWM_1_REGS);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void main()
{
    // ʹ������
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_EHRPWM, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // �ܽŸ�������
    GPIOBankPinMuxSet();

    // DSP �жϳ�ʼ��
    InterruptInit();

    // EHRPWM ��ʼ��
    EHRPWMInit(10000, 80);

    for(;;)
    {

    }
}
