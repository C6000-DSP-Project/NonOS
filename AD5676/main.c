// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      AD5676R DAC
//
//      2022��04��25��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    ģ���ź����
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

#include "interrupt.h"

#include "uartStdio.h"

#include <stdio.h>
#include <math.h>

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �궨��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// SPI �ܽ�����
#define SPI_CS                  (1 << 2)

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void SPIInit(unsigned char cs);
void SPIWrite(unsigned char cs, unsigned char *data, unsigned int len);

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��Դʱ��ʹ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void PSCInit()
{
    // ʹ�� GPIO ģ��
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽŸ�������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinMuxSet()
{
    // ������Ӧ�� GPIO �ڹ���Ϊ��ͨ���������
    // RESET
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) &
                                                       (~(SYSCFG_PINMUX7_PINMUX7_7_4))) |
                                                        ((SYSCFG_PINMUX7_PINMUX7_7_4_GPIO3_14 << SYSCFG_PINMUX7_PINMUX7_7_4_SHIFT));

    // LDAC
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) &
                                                       (~(SYSCFG_PINMUX0_PINMUX0_3_0))) |
                                                        ((SYSCFG_PINMUX0_PINMUX0_3_0_GPIO0_15 << SYSCFG_PINMUX0_PINMUX0_3_0_SHIFT));

    // RANGE �����ѹ��Χ
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) &
                                                       (~(SYSCFG_PINMUX7_PINMUX7_11_8))) |
                                                        ((SYSCFG_PINMUX7_PINMUX7_11_8_GPIO3_13 << SYSCFG_PINMUX7_PINMUX7_11_8_SHIFT));
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽų�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinInit()
{
    // ���� GPIO3[14] / RESET Ϊ���ģʽ
    GPIODirModeSet(SOC_GPIO_0_REGS, 63, GPIO_DIR_OUTPUT);

    // ���� LDAC Ϊ���ģʽ
    GPIODirModeSet(SOC_GPIO_0_REGS, 16, GPIO_DIR_OUTPUT);
    GPIOPinWrite(SOC_GPIO_0_REGS, 16, GPIO_PIN_HIGH);

    // ���� GPIO3[13] / RANGE Ϊ����ģʽ
    GPIODirModeSet(SOC_GPIO_0_REGS, 62, GPIO_DIR_INPUT);
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
//      DAC ��λ
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void DACReset()
{
    GPIOPinWrite(SOC_GPIO_0_REGS, 63, GPIO_PIN_LOW);
    Delay(0x1FFF);
    GPIOPinWrite(SOC_GPIO_0_REGS, 63, GPIO_PIN_HIGH);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      DAC ��ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
#define AD5676CMD_WRITE  (1 << 20)             // 0001B
#define AD5676CMD_WRUP   (3 << 20)             // 0011B
#define AD5676CMD_PWDN   (4 << 20)             // 0100B
#define AD5676CMD_RESET  (6 << 20)             // 0110B

#define AD5676CH0        (0)                   // ͨ�� 0
#define AD5676CH1        (1 << 16)             // ͨ�� 1
#define AD5676CH2        (2 << 16)             // ͨ�� 2
#define AD5676CH3        (3 << 16)             // ͨ�� 3
#define AD5676CH4        (4 << 16)             // ͨ�� 4
#define AD5676CH5        (5 << 16)             // ͨ�� 5
#define AD5676CH6        (6 << 16)             // ͨ�� 6
#define AD5676CH7        (7 << 16)             // ͨ�� 7

void AD5676RInit()
{
    unsigned int data;

    // �����λ
    data = AD5676CMD_RESET;
    SPIWrite(SPI_CS, (unsigned char *)&data, 3);

    // ������ͨ��
    data = AD5676CMD_PWDN;
    SPIWrite(SPI_CS, (unsigned char *)&data, 3);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��ȡ�����ѹ��Χ
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static unsigned char DACRangeGet()
{
    // 1 5V
    // 0 2.5V
    return GPIOPinRead(SOC_GPIO_0_REGS, 62);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
#define pi  3.1415926535897932384626433832795  // ��
#define Fs  1000.0                             // ����Ƶ��

void main()
{
    UARTStdioInit();
    UARTprintf("\r\nCoreKernel AD5676 Example...\r\n");

    // ʹ������
    PSCInit();

    // �ܽŸ�������
    GPIOBankPinMuxSet();

    // GPIO �ܽų�ʼ��
    GPIOBankPinInit();

    // SPI ��ʼ��
    SPIInit(SPI_CS);

    // ��λ DAC
    DACReset();

    // DAC ��ʼ��
    AD5676RInit();

    // ��������ѹ��Χ����
    char DACRange = DACRangeGet();
    char *DACRangStr[2] = {"2.5V", "5V"};
    UARTprintf("DAC Output Range %s\r\n", DACRangStr[DACRange]);

    // ��������
    // ����(50 ��ÿ���� 20 ����)
    unsigned short SquareWave[1000];
    unsigned int i, j;

    for(i = 0; i < 20; i++)
    {
        for(j = 0; j < 25; j++)
        {
            SquareWave[i * 50 + j] = 65535;
            SquareWave[i * 50 + j + 25] = 0;
        }
    }

    unsigned int data;

    for(;;)
    {
        // ���� LDAC �ܽ�
//      GPIOPinWrite(SOC_GPIO_0_REGS, 16, GPIO_PIN_LOW);

        for(i = 0; i < 1000; i++)
        {
            data = AD5676CMD_WRITE | AD5676CH0 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH1 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH2 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH3 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH4 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH5 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH6 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);

            data = AD5676CMD_WRITE | AD5676CH7 | SquareWave[i];
            SPIWrite(SPI_CS, (unsigned char *)&data, 3);
        }
    }
}
