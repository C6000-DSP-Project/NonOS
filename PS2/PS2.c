
#include "hw_types.h"

#include "SoftSPI.h"
#include "PS2.h"

unsigned char CMD[2] = {0x01, 0x42};	                                          // 命令
unsigned char Data[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // 数据

// 键值掩码
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

// 判断是否为红灯模式
// 返回值 0     红灯模式
//		  非 0  其他模式
unsigned char PS2RedLight()
{
	SoftSPIReadWrite(CMD[0], NULL);      // 开始命令
	SoftSPIReadWrite(CMD[1], &Data[0]);  // 请求数据

	return (Data[0] == 0x73) ? 0 : 1;
}

// 处理接收到的按键数据
// 按下为 0 未按下为 1
// 摇杆的模拟量范围 0 - 255
unsigned char PS2KEYData()
{
    unsigned char i;

    // 清除数据缓冲区
    for(i = 0; i < 9; i++)
    {
        Data[i] = 0;
    }

    // 读取手柄数据
    SoftSPI_CS_CLR();

    SoftSPIReadWrite(CMD[0], &Data[0]);  // 开始命令
    SoftSPIReadWrite(CMD[1], &Data[1]);  // 请求数据

    for(i = 2; i < 9; i++)
    {
        SoftSPIReadWrite(0, &Data[i]);
    }

    SoftSPI_CS_SET();

    // 处理手柄数据
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

// 查询
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

// 进入配置模式
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

// 保存配置
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

// 使能震动
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

// 震动
void PS2Vibration(unsigned char motor1, unsigned char motor2)
{
    SoftSPI_CS_CLR();

    SoftSPIReadWrite(CMD[0], NULL);  // 开始命令
    SoftSPIReadWrite(CMD[1], NULL);  // 请求数据
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(motor1, NULL);
    SoftSPIReadWrite(motor2, NULL);

    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);

    SoftSPI_CS_SET();
}

// 配置模式(覆盖硬件配置)
void PS2TurnOnAnalogMode()
{
    SoftSPI_CS_CLR();

    SoftSPIReadWrite(0x01, NULL);
    SoftSPIReadWrite(0x44, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x01, NULL);  // analog = 0x01; digital = 0x00  软件设置模式
    SoftSPIReadWrite(0xEE, NULL);  // 0x03 锁存设置 即不可通过按键 "MODE" 设置模式
                                   // 0xEE 不锁存软件设置 可通过按键 "MODE" 设置模式
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);
    SoftSPIReadWrite(0x00, NULL);

    SoftSPI_CS_SET();
}

void PS2Init()
{
    // 软件 SPI 初始化
    SoftSPIInit();

    // 进入配置模式
    PS2ShortPoll();
    PS2ShortPoll();
    PS2ShortPoll();

    PS2EnterConfing();

    // 使能模式输出
    PS2TurnOnAnalogMode();

    // 使能震动
    PS2VibrationModeEnable();

    // 退出配置模式
    PS2ExitConfing();
}
