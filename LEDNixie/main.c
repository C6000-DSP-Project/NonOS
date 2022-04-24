// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      LED 数码管(芯片驱动)
//
//      2022年04月24日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *        A
 *      ┏━━━┓
 *    F ┃ G ┃B
 *      ┣━━━┫
 *    E ┃   ┃C
 *      ┗━━━┛. DP
 *        D
 *
 *    数码管显示
 *
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      函数声明
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void SoftI2CInit();
void SoftI2CStart();
void SoftI2CStop();
void SoftI2CWrite(unsigned char ch);
unsigned char SoftI2CRead();

void Delay(volatile unsigned int delay);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      CH452 寄存器读写
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CH451 和 CH452 命令码
#define CH452_NOP           0x0000                  // 空操作
#define CH452_RESET         0x0201                  // 复位
#define CH452_LEVEL         0x0100                  // 加载光柱值 需另加 7 位数据
#define CH452_CLR_BIT       0x0180                  // 段位清 0 需另加 6 位数据
#define CH452_SET_BIT       0x01C0                  // 段位置 1 需另加 6 位数据
#define CH452_SLEEP         0x0202                  // 进入睡眠状态
#define CH452_LEFTMOV       0x0300                  // 设置移动方式-左移
#define CH452_LEFTCYC       0x0301                  // 设置移动方式-左循环
#define CH452_RIGHTMOV      0x0302                  // 设置移动方式-右移
#define CH452_RIGHTCYC      0x0303                  // 设置移动方式-右循环
#define CH452_SELF_BCD      0x0380                  // 自定义 BCD 码 需另加7位数据
#define CH452_SYSOFF        0x0400                  // 关闭显示 关闭键盘
#define CH452_SYSON1        0x0401                  // 开启显示
#define CH452_SYSON2        0x0403                  // 开启显示 键盘
#define CH452_SYSON2W       0x0423                  // 开启显示 键盘
#define CH452_NO_BCD        0x0500                  // 设置默认显示方式 可另加 3 位扫描极限
#define CH452_BCD           0x0580                  // 设置 BCD 译码方式 可另加 3 位扫描极限
#define CH452_TWINKLE       0x0600                  // 设置闪烁控制 需另加 8 位数据
#define CH452_GET_KEY       0x0700                  // 获取按键 返回按键代码
#define CH452_DIG0          0x0800                  // 数码管位 0 显示 需另加8位数据
#define CH452_DIG1          0x0900                  // 数码管位 1 显示 需另加8位数据
#define CH452_DIG2          0x0a00                  // 数码管位 2 显示 需另加8位数据
#define CH452_DIG3          0x0b00                  // 数码管位 3 显示 需另加8位数据
#define CH452_DIG4          0x0c00                  // 数码管位 4 显示 需另加8位数据
#define CH452_DIG5          0x0d00                  // 数码管位 5 显示 需另加8位数据
#define CH452_DIG6          0x0e00                  // 数码管位 6 显示 需另加8位数据
#define CH452_DIG7          0x0f00                  // 数码管位 7 显示 需另加8位数据

// BCD 译码方式下的特殊字符
#define CH452_BCD_SPACE     0x10
#define CH452_BCD_PLUS      0x11
#define CH452_BCD_MINUS     0x12
#define CH452_BCD_EQU       0x13
#define CH452_BCD_LEFT      0x14
#define CH452_BCD_RIGHT     0x15
#define CH452_BCD_UNDER     0x16
#define CH452_BCD_CH_H      0x17
#define CH452_BCD_CH_L      0x18
#define CH452_BCD_CH_P      0x19
#define CH452_BCD_DOT       0x1A
#define CH452_BCD_SELF      0x1E
#define CH452_BCD_TEST      0x88
#define CH452_BCD_DOT_X     0x80

// 有效按键代码
#define CH452_KEY_MIN       0x40
#define CH452_KEY_MAX       0x7F

// 2 线接口 CH452 定义
#define CH452_I2C_ADDR0     0x40            // CH452 的 ADDR = 0 时的地址
#define CH452_I2C_ADDR1     0x60            // CH452 的 ADDR = 1 时的地址
#define CH452_I2C_MASK      0x3E            // CH452 的 2 线接口高字节命令掩码

void CH452Write(unsigned short cmd)  // 写命令
{
    // 开始
    SoftI2CStart();

    // 写地址
    SoftI2CWrite((unsigned char)(cmd >> 7) & CH452_I2C_MASK | CH452_I2C_ADDR0);

    // 写数据
    SoftI2CWrite((unsigned char)cmd);

    // 结束
    SoftI2CStop();
}

unsigned char CH452Read(void)        // 读取按键
{
    // 开始
    SoftI2CStart();

    // 写地址
    SoftI2CWrite((unsigned char)(CH452_GET_KEY >> 7) & CH452_I2C_MASK | 0x01 | CH452_I2C_ADDR0);

    // 读数据
    unsigned char keycode;
    keycode = SoftI2CRead();

    // 结束
    SoftI2CStop();

    return(keycode);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void main()
{
    // I2C 总线初始化
    SoftI2CInit();

    // 配置 CH452
    CH452Write(CH452_SYSON2);    // 两线制方式
    CH452Write(CH452_BCD);       // BCD 译码 8 位数码管

    for(;;)
    {
        CH452Write(CH452_DIG7 | 8);
        CH452Write(CH452_DIG6 | 7);
        CH452Write(CH452_DIG5 | 6);
        CH452Write(CH452_DIG4 | 5);
        CH452Write(CH452_DIG3 | 4);
        CH452Write(CH452_DIG2 | 3 | (1 << 7));  // 秒针
        CH452Write(CH452_DIG1 | 2);
        CH452Write(CH452_DIG0 | 1);

        Delay(100);
    }
}
