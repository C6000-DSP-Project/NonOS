// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      RGB LED
//
//      2022��04��24��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    TLC5955 RGB LED
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

#include "PLL.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ȫ�ֱ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
unsigned short GSData[] =
{
    0x0000, 0x0000, 0x0000,   // LED0  BGR
    0x0000, 0x0000, 0x0000,   // LED1  BGR
    0x0000, 0x0000, 0x0000,   // LED2  BGR
    0x0000, 0x0000, 0x0000,   // LED3  BGR
    0x0000, 0x0000, 0x0000,   // LED4  BGR
    0x0000, 0x0000, 0x0000,   // LED5  BGR
    0x0000, 0x0000, 0x0000,   // LED6  BGR
    0x0000, 0x0000, 0x0000,   // LED7  BGR
    0x0000, 0x0000, 0x0000,   // LED8  BGR
    0x0000, 0x0000, 0x0000,   // LED9  BGR
    0x0000, 0x0000, 0x0000,   // LED10 BGR
    0x0000, 0x0000, 0x0000,   // LED11 BGR
    0x0000, 0x0000, 0x0000,   // LED12 BGR
    0x0000, 0x0000, 0x0000,   // LED13 BGR
    0x0000, 0x0000, 0x0000,   // LED14 BGR
    0x0000, 0x0000, 0x0000,   // LED15 BGR
};

int Grey[] =
{
    0, 0, 0, 0, 1, 1, 2, 3, 4, 6, 8, 10, 13, 16, 19, 24, 28, 33, 39, 46, 53, 60, 69, 78, 88, 98, 110, 122, 135, 149, 164, 179, 196, 214, 232, 252, 273,
    295, 317, 341, 366, 393, 420, 449, 478, 510, 542, 575, 610, 647, 684, 723, 764, 806, 849, 894, 940, 988, 1037, 1088, 1140, 1194,
    1250, 1307, 1366, 1427, 1489, 1553, 1619, 1686, 1756, 1827, 1900, 1975, 2051, 2130, 2210, 2293, 2377, 2463, 2552, 2642, 2734,
    2829, 2925, 3024, 3124, 3227, 3332, 3439, 3548, 3660, 3774, 3890, 4008, 4128, 4251, 4376, 4504, 4634, 4766, 4901, 5038, 5177,5319,
    5464, 5611, 5760, 5912, 6067, 6224, 6384, 6546, 6711, 6879, 7049, 7222, 7397, 7576, 7757, 7941, 8128, 8317, 8509, 8704, 8902, 9103, 9307,
    9514, 9723, 9936, 10151, 10370, 10591, 10816, 11043, 11274, 11507, 11744, 11984, 12227, 12473, 12722, 12975, 13230,
    13489, 13751, 14017, 14285, 14557, 14833, 15111, 15393, 15678, 15967, 16259, 16554, 16853, 17155, 17461, 17770, 18083, 18399,
    18719, 19042, 19369, 19700, 20034, 20372, 20713, 21058, 21407, 21759, 22115, 22475, 22838, 23206, 23577, 23952, 24330, 24713,
    25099, 25489, 25884, 26282, 26683, 27089, 27499, 27913, 28330, 28752, 29178, 29608, 30041, 30479, 30921, 31367, 31818, 32272,
    32730, 33193, 33660, 34131, 34606, 35085, 35569, 36057, 36549, 37046, 37547, 38052, 38561, 39075, 39593, 40116, 40643, 41175,
    41711, 42251, 42796, 43346, 43899, 44458, 45021, 45588, 46161, 46737, 47319, 47905, 48495, 49091, 49691, 50295, 50905, 51519,
    52138, 52761, 53390, 54023, 54661, 55303, 55951, 56604, 57261, 57923, 58590, 59262, 59939, 60621, 61308, 62000, 62697, 63399, 64106, 64818, 65535
};

// �㶨������У������
char DCData[] =
{
    0x7f, 0x7f, 0x7f,         // GSR0  GSG0  GSB0
    0x7f, 0x7f, 0x7f,         // GSR1  GSG1  GSB1
    0x7f, 0x7f, 0x7f,         // GSR2  GSG2  GSB2
    0x7f, 0x7f, 0x7f,         // GSR3  GSG3  GSB3
    0x7f, 0x7f, 0x7f,         // GSR4  GSG4  GSB4
    0x7f, 0x7f, 0x7f,         // GSR5  GSG5  GSB5
    0x7f, 0x7f, 0x7f,         // GSR6  GSG6  GSB6
    0x7f, 0x7f, 0x7f,         // GSR7  GSG7  GSB7
    0x7f, 0x7f, 0x7f,         // GSR8  GSG8  GSB8
    0x7f, 0x7f, 0x7f,         // GSR9  GSG9  GSB9
    0x7f, 0x7f, 0x7f,         // GSR10 GSG10 GSB10
    0x7f, 0x7f, 0x7f,         // GSR11 GSG11 GSB11
    0x7f, 0x7f, 0x7f,         // GSR12 GSG12 GSB12
    0x7f, 0x7f, 0x7f,         // GSR13 GSG13 GSB13
    0x7f, 0x7f, 0x7f,         // GSR14 GSG14 GSB14
    0x7f, 0x7f, 0x7f          // GSR15 GSG15 GSB15
};

// ����������
char MCData[] = {0x05, 0x05, 0x05}; // MCR[2:0], MCG[2:0], MCB[2:0]

// ȫ����������
char BCData[] = {0x01, 0x01, 0x01}; // BCR[6:0], BCG[6:0], BCB[6:0]

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽŸ�������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinMuxSet()
{
    // ������Ӧ�� GPIO �ڹ���Ϊ��ͨ���������
    // GPIO6[11] SCLK
    // GPIO6[09] SIN
    // GPIO6[10] LAT
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) &
                                                  (~(SYSCFG_PINMUX13_PINMUX13_27_24    |
                                                     SYSCFG_PINMUX13_PINMUX13_23_20    |
                                                     SYSCFG_PINMUX13_PINMUX13_19_16))) |
                                                   ((SYSCFG_PINMUX13_PINMUX13_27_24_GPIO6_9  << SYSCFG_PINMUX13_PINMUX13_27_24_SHIFT) |
                                                    (SYSCFG_PINMUX13_PINMUX13_23_20_GPIO6_10 << SYSCFG_PINMUX13_PINMUX13_23_20_SHIFT) |
                                                    (SYSCFG_PINMUX13_PINMUX13_19_16_GPIO6_11 << SYSCFG_PINMUX13_PINMUX13_19_16_SHIFT));
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽų�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinInit()
{
    GPIODirModeSet(SOC_GPIO_0_REGS, 106, GPIO_DIR_OUTPUT);  // GPIO6[09] SIN
    GPIODirModeSet(SOC_GPIO_0_REGS, 107, GPIO_DIR_OUTPUT);  // GPIO6[10] LAT
    GPIODirModeSet(SOC_GPIO_0_REGS, 108, GPIO_DIR_OUTPUT);  // GPIO6[11] SCLK
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
//      TLC5955 ����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ���Ŷ���
#define SCLK_SET      GPIOPinWrite(SOC_GPIO_0_REGS, 108, GPIO_PIN_HIGH)    // GPIO6[11]
#define SCLK_CLR      GPIOPinWrite(SOC_GPIO_0_REGS, 108, GPIO_PIN_LOW)     // GPIO6[11]
#define SCLK_TOGGLE   {                                                        \
                            GPIOPinWrite(SOC_GPIO_0_REGS, 108, GPIO_PIN_LOW);  \
                            Delay(1);                                          \
                            GPIOPinWrite(SOC_GPIO_0_REGS, 108, GPIO_PIN_HIGH); \
                       }                                                    // GPIO6[11]

#define SIN_SET       GPIOPinWrite(SOC_GPIO_0_REGS, 106, GPIO_PIN_HIGH)    // GPIO6[09]
#define SIN_CLR       GPIOPinWrite(SOC_GPIO_0_REGS, 106, GPIO_PIN_LOW)     // GPIO6[09]

#define LAT_SET       GPIOPinWrite(SOC_GPIO_0_REGS, 107, GPIO_PIN_HIGH)    // GPIO6[10]
#define LAT_CLR       GPIOPinWrite(SOC_GPIO_0_REGS, 107, GPIO_PIN_LOW)     // GPIO6[10]

void SendGSData()
{
    // GS ���� 768Bits

    // MSB = 0 ���� GS(Grayscale) ����
    SIN_CLR;
    SCLK_CLR;
    Delay(1);

    SCLK_SET;

    // ���� 0-767Bits ����(MSB) ÿ 16Bit ���ݱ�ʾһ��ͨ�� GSPWM ����
    int i;
    for(i = 767; i >= 0; i--)
    {
        GSData[i / 16] & (1 << (i % 16)) ? SIN_SET : SIN_CLR;
        SCLK_TOGGLE;
    }

    SCLK_CLR;

    // ��������
    LAT_SET;
    Delay(1);
    LAT_CLR;
}

void SendControlData(char LSDVLT, char ESPWM, char RFRESH, char TMGRST, char DSPRPT, char *BCData, char *MCData, char *DCData)
{
    // �������� 371Bits
    int k, k1, k2, k3;
    char temp1, temp2, temp3, temp4, temp5, temp6;
    char t = 0x96;

    // MSB = 1 ��767-760Bits Ϊ 96h (10010110B) ���Ϳ�������
    SIN_SET;
    SCLK_TOGGLE;

    // ���� 0x96
    for(k = 767; k >= 760; k--)
    {
        t & (1 << (k - 760)) ? SIN_SET : SIN_CLR;
        SCLK_TOGGLE;
    }

    // �ǿ�������(���)
    for(k = 759; k >= 371; k--)
    {
        SIN_SET;
        SCLK_TOGGLE;
    }

    // LED ��·��ѹ���ѡ��
    // 0 70% * VCC
    // 1 90% * VCC
    LSDVLT ? SIN_SET : SIN_CLR;
    SCLK_TOGGLE;

    // ESPWM
    ESPWM ? SIN_SET : SIN_CLR;
    SCLK_TOGGLE;

    // �Զ�����
    RFRESH ? SIN_SET : SIN_CLR;
    SCLK_TOGGLE;

    // ��ʾʱ��λʹ��
    TMGRST ? SIN_SET : SIN_CLR;
    SCLK_TOGGLE;

    // �Զ���ת��ʾ
    DSPRPT ? SIN_SET : SIN_CLR;
    SCLK_TOGGLE;

    // ���� BC ���� BCR[7:0] BCG[7:0] BCB[7:0]
    for(k1 = 20; k1 >= 0; k1--)
    {
        temp1 = 1 << (k1 % 7);
        temp2 = BCData[k1 / 7] & temp1;

        if(temp2 == temp1)
        {
            SIN_SET;
        }
        else
        {
            SIN_CLR;
        }

        SCLK_TOGGLE;
    }

    // ���� MC ���� MCR[2:0] MCG[2:0] MCB[2:0]
    for(k2 = 8; k2 >= 0; k2--)
    {
        temp3 = 1 << (k2 % 3);
        temp4 = MCData[k2 / 3] & temp3;

        if(temp4 == temp3)
        {
            SIN_SET;
        }
        else
        {
            SIN_CLR;
        }

        SCLK_TOGGLE;
    }

    // �� DC ���� DCRn[7:0] DCGn[2:0] DCBn[2:0]
    for(k3 = 335; k3 >=0; k3--)
    {
        temp5 = 1 << (k3 % 7);
        temp6 = DCData[k3 / 7] & temp5;

        if(temp6 == temp5)
        {
            SIN_SET;
        }
        else
        {
            SIN_CLR;
        }

        SCLK_TOGGLE;
    }

    SCLK_CLR;

    LAT_SET;
    Delay(1);
    LAT_CLR;
}

void LEDSet(int led, unsigned short R, unsigned short G, unsigned short B)
{
    GSData[led * 3] = R;
    GSData[led * 3 + 1] = G;
    GSData[led * 3 + 2] = B;

    SendGSData();
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

    // ���� GSCLK ʱ�� 6MHz
    PLL0ClockOut(OSCIN, 4);

    // ���� TLC5955
    SendControlData(1, 0, 1, 1, 1, BCData, MCData, DCData);

    int i, j;

    for(;;)
    {
        for(i = 0; i < 256; i++)
        {
            for(j = 0; j < 16; j++)
            {
                LEDSet(j, 0, Grey[i], Grey[200]);
            }

        }

        for(i = 255; i >= 0; i--)
        {
            for(j = 0; j < 16; j++)
            {
                LEDSet(j, 0, Grey[i], Grey[200]);
            }

        }
        Delay(100);

        for(i = 0; i < 256; i++)
        {
            for(j = 0; j < 16; j++)
            {
                LEDSet(j, Grey[i], 0, Grey[200]);
            }

        }

        for(i = 255; i >= 0; i--)
        {
            for(j = 0; j < 16; j++)
            {
                LEDSet(j, Grey[i], 0, Grey[200]);
            }

        }
        Delay(100);
    }
}
