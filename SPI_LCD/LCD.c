#include "hw_types.h"
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "gpio.h"

#include "SoftSPI.h"
#include "LCD.h"

#include "delay.h"

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �궨��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
#define LCD_RES_CLR()  GPIOPinWrite(SOC_GPIO_0_REGS, 2, GPIO_PIN_LOW)   // GPIO0[1]  RESET
#define LCD_RES_SET()  GPIOPinWrite(SOC_GPIO_0_REGS, 2, GPIO_PIN_HIGH)

#define LCD_DC_CLR()   GPIOPinWrite(SOC_GPIO_0_REGS, 3, GPIO_PIN_LOW)   // GPIO0[2]  RS/DC ����/����ѡ��
#define LCD_DC_SET()   GPIOPinWrite(SOC_GPIO_0_REGS, 3, GPIO_PIN_HIGH)

#define LCD_BLK_CLR()                                                   // ����
#define LCD_BLK_SET()

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽŸ�������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinMuxSet()
{
    // RESET
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) & (~(SYSCFG_PINMUX1_PINMUX1_27_24))) |
                                                   (SYSCFG_PINMUX1_PINMUX1_27_24_GPIO0_1 << SYSCFG_PINMUX1_PINMUX1_27_24_SHIFT);

    // RS/DC(��������ѡ��)
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) & (~(SYSCFG_PINMUX1_PINMUX1_23_20))) |
                                                   (SYSCFG_PINMUX1_PINMUX1_23_20_GPIO0_2 << SYSCFG_PINMUX1_PINMUX1_23_20_SHIFT);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      GPIO �ܽų�ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
static void GPIOBankPinInit()
{
    GPIODirModeSet(SOC_GPIO_0_REGS, 2, GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_0_REGS, 3, GPIO_DIR_OUTPUT);
}

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��ʱ���Ǿ�ȷ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void delay_ms(volatile unsigned int ms)
{
    Sysdelay(ms);
}

/******************************************************************************
      ����˵����LCD  д������
      ������ݣ�data д�������
      ����ֵ��  ��
******************************************************************************/
void LCD_WR_DATA8(unsigned char data)
{
    SoftSPIWrite(data);
}

/******************************************************************************
      ����˵����LCD  д������
      ������ݣ�data д�������
      ����ֵ��  ��
******************************************************************************/
void LCD_WR_DATA(unsigned short data)
{
    SoftSPIWrite(data >> 8);
    SoftSPIWrite(data);
}

/******************************************************************************
      ����˵����LCD  д������
      ������ݣ�data д�������
      ����ֵ��  ��
******************************************************************************/
void LCD_WR_REG(unsigned char data)
{
    LCD_DC_CLR();  // д����
    SoftSPIWrite(data);
    LCD_DC_SET();  // д����
}

/******************************************************************************
      ����˵����������ʼ�ͽ�����ַ
      ������ݣ�x1, x2 �����е���ʼ�ͽ�����ַ
                y1, y2 �����е���ʼ�ͽ�����ַ
      ����ֵ��  ��
******************************************************************************/
void LCDAddressSet(u16 x1, u16 y1, u16 x2, u16 y2)
{
    if(USE_HORIZONTAL == 0)
    {
        LCD_WR_REG(0x2a);  // �е�ַ����
        LCD_WR_DATA(x1 + 26);
        LCD_WR_DATA(x2 + 26);
        LCD_WR_REG(0x2b);  // �е�ַ����
        LCD_WR_DATA(y1 + 1);
        LCD_WR_DATA(y2 + 1);
        LCD_WR_REG(0x2c);  // ������д
    }
    else if(USE_HORIZONTAL == 1)
    {
        LCD_WR_REG(0x2a);  // �е�ַ����
        LCD_WR_DATA(x1 + 26);
        LCD_WR_DATA(x2 + 26);
        LCD_WR_REG(0x2b);  // �е�ַ����
        LCD_WR_DATA(y1 + 1);
        LCD_WR_DATA(y2 + 1);
        LCD_WR_REG(0x2c);  // ������д
    }
    else if(USE_HORIZONTAL == 2)
    {
        LCD_WR_REG(0x2a);  // �е�ַ����
        LCD_WR_DATA(x1 + 1);
        LCD_WR_DATA(x2 + 1);
        LCD_WR_REG(0x2b);  // �е�ַ����
        LCD_WR_DATA(y1 + 26);
        LCD_WR_DATA(y2 + 26);
        LCD_WR_REG(0x2c);  // ������д
    }
    else
    {
        LCD_WR_REG(0x2a);  // �е�ַ����
        LCD_WR_DATA(x1 + 1);
        LCD_WR_DATA(x2 + 1);
        LCD_WR_REG(0x2b);  // �е�ַ����
        LCD_WR_DATA(y1 + 26);
        LCD_WR_DATA(y2 + 26);
        LCD_WR_REG(0x2c);  // ������д
    }
}

/******************************************************************************
      ����˵������ָ�����������ɫ
      ������ݣ�xsta, ysta   ��ʼ����
                xend, yend   ��ֹ����
				color        Ҫ������ɫ
      ����ֵ��  ��
******************************************************************************/
void LCDFill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color)
{          
	u16 i, j;
	LCDAddressSet(xsta, ysta, xend - 1, yend - 1);  // ������ʾ��Χ

	for(i = ysta; i < yend; i++)
	{													   	 	
		for(j = xsta; j < xend; j++)
		{
			LCD_WR_DATA(color);
		}
	} 					  	    
}

/******************************************************************************
      ����˵������ָ��λ�û���
      ������ݣ�x, y  ��������
                color �����ɫ
      ����ֵ��  ��
******************************************************************************/
void LCDPointDraw(u16 x, u16 y, u16 color)
{
	LCDAddressSet(x, y, x, y);  // ���ù��λ��
	LCD_WR_DATA(color);
} 

/******************************************************************************
      ����˵��������
      ������ݣ�x1, y1   ��ʼ����
                x2, y2   ��ֹ����
                color    �ߵ���ɫ
      ����ֵ��  ��
******************************************************************************/
void LCDLineDraw(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
	u16 t; 
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;

	delta_x = x2 - x1;       // ������������
	delta_y = y2 - y1;
	uRow = x1;               // �����������
	uCol = y1;

	if(delta_x>0)
    {
	    incx=1;              // ���õ�������
    }
	else if(delta_x==0)
    {
	    incx=0;              // ��ֱ��
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
	    incy = 0;            // ˮƽ��
    }
	else
	{
	    incy = -1;
	    delta_y = -delta_y;
	}

	if(delta_x > delta_y)
    {
	    distance = delta_x;  // ѡȡ��������������
    }
	else
	{
	    distance=delta_y;
	}

	for(t = 0; t < distance + 1; t++)
	{
		LCDPointDraw(uRow, uCol, color); // ����
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
      ����˵����������
      ������ݣ�x1,y1   ��ʼ����
                x2,y2   ��ֹ����
                color   ���ε���ɫ
      ����ֵ��  ��
******************************************************************************/
void LCDRectangleDraw(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCDLineDraw(x1, y1, x2, y1, color);
	LCDLineDraw(x1, y1, x1, y2, color);
	LCDLineDraw(x1, y2, x2, y2, color);
	LCDLineDraw(x2, y1, x2, y2, color);
}

/******************************************************************************
      ����˵������Բ
      ������ݣ�x0, y0  Բ������
                r       �뾶
                color   Բ����ɫ
      ����ֵ��  ��
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

		if((a * a + b * b) > (r * r))                    // �ж�Ҫ���ĵ��Ƿ��Զ
		{
			b--;
		}
	}
}

/******************************************************************************
      ����˵������ʾ����
      ������ݣ�x, y  ��ʾ����
                *s    Ҫ��ʾ�ĺ��ִ�
                fc    �ֵ���ɫ
                bc    �ֵı���ɫ
                num   ����
                sizey �ֺ� ��ѡ 16 24 32
                mode:  0 �ǵ���ģʽ  1 ����ģʽ
      ����ֵ��  ��
******************************************************************************/
extern unsigned char FontIndex[][2];

void LCDChineseDraw(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 *font, u16 num, u8 sizey, u8 mode)
{
    // ��ʾ����
    u8 i, j, m = 0;
    u16 k;
    u16 TypefaceNum;                                // һ���ַ���ռ�ֽڴ�С
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
	                    if(!mode)                       // �ǵ��ӷ�ʽ
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
	                    else                            // ���ӷ�ʽ
	                    {
	                        if(font[k * TypefaceNum + i] & (0x01 << j))
	                        {
	                            LCDPointDraw(x, y, fc); // ��һ����
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
	        continue;                                   // ���ҵ���Ӧ�����ֿ������˳� ��ֹ��������ظ�ȡģ����Ӱ��
	    }

		s += 2;
		x += sizey;
	}
}

/******************************************************************************
      ����˵������ʾ�����ַ�
      ������ݣ�x, y  ��ʾ����
                num   Ҫ��ʾ���ַ�
                fc    �ֵ���ɫ
                bc    �ֵı���ɫ
                sizey �ֺ�
                mode:  0 �ǵ���ģʽ  1 ����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCDCharDraw(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 sizey, u8 mode)
{
	u8 temp, sizex, t, m = 0;
	u16 i, TypefaceNum;                                   // һ���ַ���ռ�ֽڴ�С
	u16 x0 = x;
	sizex = sizey / 2;
	TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
	num = num - ' ';                                     // �õ�ƫ�ƺ��ֵ
	LCDAddressSet(x, y, x + sizex - 1, y + sizey - 1);   // ���ù��λ��

	for(i = 0; i < TypefaceNum; i++)
	{ 
        if(sizey == 12)
        {
            temp = ASCII12[num][i];                      // 12 ����
        }
        else if(sizey == 16)
        {
            temp = ASCII16[num][i];                      // 16 ����
        }
        else if(sizey == 24)
        {
            temp = ASCII24[num][i];                      // 24 ����
        }
        else if(sizey == 32)
        {
            temp = ASCII32[num][i];                      // 32 ����
        }
        else
        {
            return;
        }

		for(t = 0; t < 8; t++)
		{
			if(!mode)                                    // �ǵ���ģʽ
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
			else                                         // ����ģʽ
			{
				if(temp & (0x01 << t))
                {
				    LCDPointDraw(x, y, fc);              // ��һ����
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
      ����˵������ʾ�ַ���
      ������ݣ�x,y��ʾ����
                *p Ҫ��ʾ���ַ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0 �ǵ���ģʽ  1 ����ģʽ
      ����ֵ��  ��
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
      ����˵������ʾͼƬ
      ������ݣ�x, y   �������
                length ͼƬ����
                width  ͼƬ���
                pic[]  ͼƬ����    
      ����ֵ��  ��
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

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      LCD ��ʼ��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
void LCDInit()
{
    // ��� SPI ��ʼ��
    SoftSPIInit();

    // �ܽŸ�������
    GPIOBankPinMuxSet();

    // GPIO �ܽų�ʼ��
    GPIOBankPinInit();

    // ��λ LCD
    LCD_RES_CLR();
    delay_ms(100);

    LCD_RES_SET();
    delay_ms(100);

    // �򿪱���
    LCD_BLK_SET();
    delay_ms(100);

    // LCD �Ĵ�������
    LCD_WR_REG(0x11);     // Sleep out
    delay_ms(120);        // ��ʱ

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

    // ��ʾ����
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
