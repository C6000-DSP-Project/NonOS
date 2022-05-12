#include "hw_types.h"
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "gpio.h"

#include "SoftSPI.h"
#include "LCD.h"

#include "delay.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
#define LCD_RES_CLR()  GPIOPinWrite(SOC_GPIO_0_REGS, 2, GPIO_PIN_LOW)   // GPIO0[1]  RESET
#define LCD_RES_SET()  GPIOPinWrite(SOC_GPIO_0_REGS, 2, GPIO_PIN_HIGH)

#define LCD_DC_CLR()   GPIOPinWrite(SOC_GPIO_0_REGS, 3, GPIO_PIN_LOW)   // GPIO0[2]  RS/DC 命令/数据选择
#define LCD_DC_SET()   GPIOPinWrite(SOC_GPIO_0_REGS, 3, GPIO_PIN_HIGH)

#define LCD_BLK_CLR()                                                   // 背光
#define LCD_BLK_SET()

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚复用配置
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinMuxSet()
{
    // RESET
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) & (~(SYSCFG_PINMUX1_PINMUX1_27_24))) |
                                                   (SYSCFG_PINMUX1_PINMUX1_27_24_GPIO0_1 << SYSCFG_PINMUX1_PINMUX1_27_24_SHIFT);

    // RS/DC(命令数据选择)
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) & (~(SYSCFG_PINMUX1_PINMUX1_23_20))) |
                                                   (SYSCFG_PINMUX1_PINMUX1_23_20_GPIO0_2 << SYSCFG_PINMUX1_PINMUX1_23_20_SHIFT);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      GPIO 管脚初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void GPIOBankPinInit()
{
    GPIODirModeSet(SOC_GPIO_0_REGS, 2, GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_0_REGS, 3, GPIO_DIR_OUTPUT);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      延时（非精确）
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void delay_ms(volatile unsigned int ms)
{
    Sysdelay(ms);
}

/******************************************************************************
      函数说明：LCD  写入数据
      入口数据：data 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(unsigned char data)
{
    SoftSPIWrite(data);
}

/******************************************************************************
      函数说明：LCD  写入数据
      入口数据：data 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(unsigned short data)
{
    SoftSPIWrite(data >> 8);
    SoftSPIWrite(data);
}

/******************************************************************************
      函数说明：LCD  写入命令
      入口数据：data 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(unsigned char data)
{
    LCD_DC_CLR();  // 写命令
    SoftSPIWrite(data);
    LCD_DC_SET();  // 写数据
}

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1, x2 设置列的起始和结束地址
                y1, y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCDAddressSet(u16 x1, u16 y1, u16 x2, u16 y2)
{
    if(USE_HORIZONTAL == 0)
    {
        LCD_WR_REG(0x2a);  // 列地址设置
        LCD_WR_DATA(x1 + 26);
        LCD_WR_DATA(x2 + 26);
        LCD_WR_REG(0x2b);  // 行地址设置
        LCD_WR_DATA(y1 + 1);
        LCD_WR_DATA(y2 + 1);
        LCD_WR_REG(0x2c);  // 储存器写
    }
    else if(USE_HORIZONTAL == 1)
    {
        LCD_WR_REG(0x2a);  // 列地址设置
        LCD_WR_DATA(x1 + 26);
        LCD_WR_DATA(x2 + 26);
        LCD_WR_REG(0x2b);  // 行地址设置
        LCD_WR_DATA(y1 + 1);
        LCD_WR_DATA(y2 + 1);
        LCD_WR_REG(0x2c);  // 储存器写
    }
    else if(USE_HORIZONTAL == 2)
    {
        LCD_WR_REG(0x2a);  // 列地址设置
        LCD_WR_DATA(x1 + 1);
        LCD_WR_DATA(x2 + 1);
        LCD_WR_REG(0x2b);  // 行地址设置
        LCD_WR_DATA(y1 + 26);
        LCD_WR_DATA(y2 + 26);
        LCD_WR_REG(0x2c);  // 储存器写
    }
    else
    {
        LCD_WR_REG(0x2a);  // 列地址设置
        LCD_WR_DATA(x1 + 1);
        LCD_WR_DATA(x2 + 1);
        LCD_WR_REG(0x2b);  // 行地址设置
        LCD_WR_DATA(y1 + 26);
        LCD_WR_DATA(y2 + 26);
        LCD_WR_REG(0x2c);  // 储存器写
    }
}

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta, ysta   起始坐标
                xend, yend   终止坐标
				color        要填充的颜色
      返回值：  无
******************************************************************************/
void LCDFill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color)
{          
	u16 i, j;
	LCDAddressSet(xsta, ysta, xend - 1, yend - 1);  // 设置显示范围

	for(i = ysta; i < yend; i++)
	{													   	 	
		for(j = xsta; j < xend; j++)
		{
			LCD_WR_DATA(color);
		}
	} 					  	    
}

/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x, y  画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void LCDPointDraw(u16 x, u16 y, u16 color)
{
	LCDAddressSet(x, y, x, y);  // 设置光标位置
	LCD_WR_DATA(color);
} 

/******************************************************************************
      函数说明：画线
      入口数据：x1, y1   起始坐标
                x2, y2   终止坐标
                color    线的颜色
      返回值：  无
******************************************************************************/
void LCDLineDraw(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
	u16 t; 
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;

	delta_x = x2 - x1;       // 计算坐标增量
	delta_y = y2 - y1;
	uRow = x1;               // 画线起点坐标
	uCol = y1;

	if(delta_x>0)
    {
	    incx=1;              // 设置单步方向
    }
	else if(delta_x==0)
    {
	    incx=0;              // 垂直线
    }
	else
	{
	    incx = -1;
	    delta_x = -delta_x;
	}

	if(delta_y > 0)
    {
	    incy = 1;
    }
	else if(delta_y == 0)
    {
	    incy = 0;            // 水平线
    }
	else
	{
	    incy = -1;
	    delta_y = -delta_y;
	}

	if(delta_x > delta_y)
    {
	    distance = delta_x;  // 选取基本增量坐标轴
    }
	else
	{
	    distance=delta_y;
	}

	for(t = 0; t < distance + 1; t++)
	{
		LCDPointDraw(uRow, uCol, color); // 画点
		xerr += delta_x;
		yerr += delta_y;

		if(xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if(yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

/******************************************************************************
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   矩形的颜色
      返回值：  无
******************************************************************************/
void LCDRectangleDraw(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCDLineDraw(x1, y1, x2, y1, color);
	LCDLineDraw(x1, y1, x1, y2, color);
	LCDLineDraw(x1, y2, x2, y2, color);
	LCDLineDraw(x2, y1, x2, y2, color);
}

/******************************************************************************
      函数说明：画圆
      入口数据：x0, y0  圆心坐标
                r       半径
                color   圆的颜色
      返回值：  无
******************************************************************************/
void LCDCircleDraw(u16 x0, u16 y0, u8 r, u16 color)
{
	int a, b;

	a = 0;
	b = r;

	while(a <= b)
	{
		LCDPointDraw(x0 - b, y0 - a, color);             // 3
		LCDPointDraw(x0 + b, y0 - a, color);             // 0
		LCDPointDraw(x0 - a, y0 + b, color);             // 1
		LCDPointDraw(x0 - a, y0 - b, color);             // 2
		LCDPointDraw(x0 + b, y0 + a, color);             // 4
		LCDPointDraw(x0 + a, y0 - b, color);             // 5
		LCDPointDraw(x0 + a, y0 + b, color);             // 6
		LCDPointDraw(x0 - b, y0 + a, color);             // 7
		a++;

		if((a * a + b * b) > (r * r))                    // 判断要画的点是否过远
		{
			b--;
		}
	}
}

/******************************************************************************
      函数说明：显示汉字
      入口数据：x, y  显示坐标
                *s    要显示的汉字串
                fc    字的颜色
                bc    字的背景色
                num   字数
                sizey 字号 可选 16 24 32
                mode:  0 非叠加模式  1 叠加模式
      返回值：  无
******************************************************************************/
extern unsigned char FontIndex[][2];

void LCDChineseDraw(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 *font, u16 num, u8 sizey, u8 mode)
{
    // 显示汉字
    u8 i, j, m = 0;
    u16 k;
    u16 TypefaceNum;                                // 一个字符所占字节大小
    u16 x0 = x;

	while(*s != 0)
	{

	    TypefaceNum = (sizey / 8 + ((sizey % 8) ? 1 : 0)) * sizey;

	    for(k = 0; k < num; k++)
	    {
	        if((FontIndex[k][0] == *(s)) && (FontIndex[k][1] == *(s + 1)))
	        {
	            LCDAddressSet(x, y, x + sizey - 1, y + sizey - 1);

	            for(i = 0; i < TypefaceNum; i++)
	            {
	                for(j = 0; j < 8; j++)
	                {
	                    if(!mode)                       // 非叠加方式
	                    {
	                        if(font[k * TypefaceNum + i] & (0x01 << j))
	                        {
	                            LCD_WR_DATA(fc);
	                        }
	                        else
	                        {
	                            LCD_WR_DATA(bc);
	                        }

	                        m++;

	                        if(m % sizey == 0)
	                        {
	                            m = 0;
	                            break;
	                        }
	                    }
	                    else                            // 叠加方式
	                    {
	                        if(font[k * TypefaceNum + i] & (0x01 << j))
	                        {
	                            LCDPointDraw(x, y, fc); // 画一个点
	                        }
	                        x++;

	                        if((x - x0) == sizey)
	                        {
	                            x = x0;
	                            y++;
	                            break;
	                        }
	                    }
	                }
	            }
	        }
	        continue;                                   // 查找到对应点阵字库立即退出 防止多个汉字重复取模带来影响
	    }

		s += 2;
		x += sizey;
	}
}

/******************************************************************************
      函数说明：显示单个字符
      入口数据：x, y  显示坐标
                num   要显示的字符
                fc    字的颜色
                bc    字的背景色
                sizey 字号
                mode:  0 非叠加模式  1 叠加模式
      返回值：  无
******************************************************************************/
void LCDCharDraw(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 sizey, u8 mode)
{
	u8 temp, sizex, t, m = 0;
	u16 i, TypefaceNum;                                   // 一个字符所占字节大小
	u16 x0 = x;
	sizex = sizey / 2;
	TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
	num = num - ' ';                                     // 得到偏移后的值
	LCDAddressSet(x, y, x + sizex - 1, y + sizey - 1);   // 设置光标位置

	for(i = 0; i < TypefaceNum; i++)
	{ 
        if(sizey == 12)
        {
            temp = ASCII12[num][i];                      // 12 字体
        }
        else if(sizey == 16)
        {
            temp = ASCII16[num][i];                      // 16 字体
        }
        else if(sizey == 24)
        {
            temp = ASCII24[num][i];                      // 24 字体
        }
        else if(sizey == 32)
        {
            temp = ASCII32[num][i];                      // 32 字体
        }
        else
        {
            return;
        }

		for(t = 0; t < 8; t++)
		{
			if(!mode)                                    // 非叠加模式
			{
				if(temp & (0x01 << t))
                {
				    LCD_WR_DATA(fc);
                }
				else
				{
				    LCD_WR_DATA(bc);
				}
				m++;
				if(m % sizex == 0)
				{
					m = 0;
					break;
				}
			}
			else                                         // 叠加模式
			{
				if(temp & (0x01 << t))
                {
				    LCDPointDraw(x, y, fc);              // 画一个点
                }
				x++;
				if((x - x0) == sizex)
				{
					x = x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}

/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0 非叠加模式  1 叠加模式
      返回值：  无
******************************************************************************/
void LCDStringDraw(u16 x, u16 y, char *p, u16 fc, u16 bc, u8 sizey, u8 mode)
{         
	while(*p != '\0')
	{       
		LCDCharDraw(x, y, *p, fc, bc, sizey, mode);
		x += sizey / 2;
		p++;
	}  
}

/******************************************************************************
      函数说明：显示图片
      入口数据：x, y   起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组    
      返回值：  无
******************************************************************************/
void LCDPictureDraw(u16 x, u16 y, u16 length, u16 width, const u8 pic[])
{
	u16 i, j;
	u32 k = 0;

	LCDAddressSet(x, y, x + length - 1, y + width - 1);

	for(i = 0; i < length; i++)
	{
		for(j = 0; j < width; j++)
		{
			LCD_WR_DATA8(pic[k * 2]);
			LCD_WR_DATA8(pic[k * 2 + 1]);
			k++;
		}
	}			
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      LCD 初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void LCDInit()
{
    // 软件 SPI 初始化
    SoftSPIInit();

    // 管脚复用配置
    GPIOBankPinMuxSet();

    // GPIO 管脚初始化
    GPIOBankPinInit();

    // 复位 LCD
    LCD_RES_CLR();
    delay_ms(100);

    LCD_RES_SET();
    delay_ms(100);

    // 打开背光
    LCD_BLK_SET();
    delay_ms(100);

    // LCD 寄存器配置
    LCD_WR_REG(0x11);     // Sleep out
    delay_ms(120);        // 延时

    LCD_WR_REG(0xB1);     // Normal mode
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);

    LCD_WR_REG(0xB2);     // Idle mode
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);

    LCD_WR_REG(0xB3);     // Partial mode
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);

    LCD_WR_REG(0xB4);     // Dot inversion
    LCD_WR_DATA8(0x03);

    LCD_WR_REG(0xC0);     // AVDD GVDD
    LCD_WR_DATA8(0xAB);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x04);

    LCD_WR_REG(0xC1);     // VGH VGL
    LCD_WR_DATA8(0xC5);   // C0

    LCD_WR_REG(0xC2);     // Normal Mode
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x00);

    LCD_WR_REG(0xC3);     // Idle
    LCD_WR_DATA8(0x8D);
    LCD_WR_DATA8(0x6A);

    LCD_WR_REG(0xC4);     // Partial + Full
    LCD_WR_DATA8(0x8D);
    LCD_WR_DATA8(0xEE);

    LCD_WR_REG(0xC5);     // VCOM
    LCD_WR_DATA8(0x0F);

    LCD_WR_REG(0xE0);     // positive gamma
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x10);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x02);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x0F);
    LCD_WR_DATA8(0x25);
    LCD_WR_DATA8(0x36);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x10);

    LCD_WR_REG(0xE1);     // negative gamma
    LCD_WR_DATA8(0x0A);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x0F);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x02);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x0F);
    LCD_WR_DATA8(0x25);
    LCD_WR_DATA8(0x35);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x10);

    LCD_WR_REG(0xFC);
    LCD_WR_DATA8(0x80);

    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);
    LCD_WR_REG(0x36);

    // 显示方向
    if(USE_HORIZONTAL == 0)
    {
        LCD_WR_DATA8(0x08);
    }
    else if(USE_HORIZONTAL == 1)
    {
        LCD_WR_DATA8(0xC8);
    }
    else if(USE_HORIZONTAL == 2)
    {
        LCD_WR_DATA8(0x78);
    }
    else
    {
        LCD_WR_DATA8(0xA8);
    }

    LCD_WR_REG(0x21);     // Display inversion
    LCD_WR_REG(0x29);     // Display on

    LCD_WR_REG(0x2A);     // Set Column Address
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x1A);   // 26
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x69);   // 105

    LCD_WR_REG(0x2B);     // Set Page Address
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x01);   // 1
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0xA0);   // 160

    LCD_WR_REG(0x2C);
}
