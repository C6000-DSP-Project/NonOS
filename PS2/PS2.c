
#include "hw_types.h"

#include "SoftSPI.h"
#include "PS2.h"

unsigned char CMD[2] = {0x01, 0x42};	                                          // ����
unsigned char Data[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // ����

// ��ֵ����
unsigned short PS2KEYMASK[] =
{
    PSB_SELECT,
    PSB_L3,
    PSB_R3 ,
    PSB_START,
    PSB_PAD_UP,
    PSB_PAD_RIGHT,
    PSB_PAD_DOWN,
    PSB_PAD_LEFT,
    PSB_L2,
    PSB_R2,
    PSB_L1,
    PSB_R1 ,
    PSB_GREEN,
    PSB_RED,
    PSB_BLUE,
    PSB_PINK
};

// �ж��Ƿ�Ϊ���ģʽ
// ����ֵ 0     ���ģʽ
//		  �� 0  ����ģʽ
unsigned char PS2RedLight()
{
	SoftSPIReadWrite(CMD[0], NULL);      // ��ʼ����
	SoftSPIReadWrite(CMD[1], &Data[0]);  // ��������

	return (Data[0] == 0x73) ? 0 : 1;
}

// ������յ��İ�������
// ����Ϊ 0 δ����Ϊ 1
// ҡ�˵�ģ������Χ 0 - 255
unsigned char PS2KEYData()
{
    unsigned char i;

    // ������ݻ�����
    for(i = 0; i < 9; i++)
    {
        Data[i] = 0;
    }

    // ��ȡ�ֱ�����
    SoftSPI_CS_CLR();

    SoftSPIReadWrite(CMD[0], &Data[0]);  // ��ʼ����
    SoftSPIReadWrite(CMD[1], &Data[1]);  // ��������

    for(i = 2; i < 9; i++)
    {
        SoftSPIReadWrite(0, &Data[i]);
    }

    SoftSPI_CS_SET();

    // �����ֱ�����
    unsigned short key;
    key = (Data[4] << 8) | Data[3];

    for(i = 0; i < 16; i++)
    {
        if((key & (1 << (PS2KEYMASK[i] - 1))) == 0)
        return i + 1;
    }

    return 0;
}

unsigned char PS2AnologData(unsigned char button)
{
    return Data[button];
}

// ��ѯ
void PS2ShortPoll()
{
    SoftSPI_CS_CLR();

    SoftSPIReadWrite(0x01, NULL);
    SoftSPIReadWrite(0x42, NULL);
    SoftSPIReadWrite(0X00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);

    SoftSPI_CS_SET();
}

// ��������ģʽ
void PS2EnterConfing()
{
    SoftSPI_CS_CLR();

    SoftSPIReadWrite(0x01, NULL);
    SoftSPIReadWrite(0x43, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x01, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);

    SoftSPI_CS_SET();
}

// ��������
void PS2ExitConfing()
{
    SoftSPI_CS_CLR();

    SoftSPIReadWrite(0x01, NULL);
    SoftSPIReadWrite(0x43, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x5A, NULL);
    SoftSPIReadWrite(0x5A, NULL);
    SoftSPIReadWrite(0x5A, NULL);
    SoftSPIReadWrite(0x5A, NULL);
    SoftSPIReadWrite(0x5A, NULL);

    SoftSPI_CS_SET();
}

// ʹ����
void PS2VibrationModeEnable()
{
    SoftSPI_CS_CLR();

    SoftSPIReadWrite(0x01, NULL);
    SoftSPIReadWrite(0x4D, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x01, NULL);

    SoftSPI_CS_SET();
}

// ��
void PS2Vibration(unsigned char motor1, unsigned char motor2)
{
    SoftSPI_CS_CLR();

    SoftSPIReadWrite(CMD[0], NULL);  // ��ʼ����
    SoftSPIReadWrite(CMD[1], NULL);  // ��������
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(motor1, NULL);
    SoftSPIReadWrite(motor2, NULL);

    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);

    SoftSPI_CS_SET();
}

// ����ģʽ(����Ӳ������)
void PS2TurnOnAnalogMode()
{
    SoftSPI_CS_CLR();

    SoftSPIReadWrite(0x01, NULL);
    SoftSPIReadWrite(0x44, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x01, NULL);  // analog = 0x01; digital = 0x00  �������ģʽ
    SoftSPIReadWrite(0xEE, NULL);  // 0x03 �������� ������ͨ������ "MODE" ����ģʽ
                                   // 0xEE ������������� ��ͨ������ "MODE" ����ģʽ
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);

    SoftSPI_CS_SET();
}

void PS2Init()
{
    // ��� SPI ��ʼ��
    SoftSPIInit();

    // ��������ģʽ
    PS2ShortPoll();
    PS2ShortPoll();
    PS2ShortPoll();

    PS2EnterConfing();

    // ʹ��ģʽ���
    PS2TurnOnAnalogMode();

    // ʹ����
    PS2VibrationModeEnable();

    // �˳�����ģʽ
    PS2ExitConfing();
}
