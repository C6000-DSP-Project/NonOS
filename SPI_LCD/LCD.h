#ifndef LCD_H
#define LCD_H

#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long

#define USE_HORIZONTAL 2  // 设置横屏或者竖屏显示 0 或 1 为竖屏 2 或 3 为横屏

#if USE_HORIZONTAL == 0 || USE_HORIZONTAL == 1
#define LCD_WIDTH   80
#define LCD_HEIGHT  160
#else
#define LCD_WIDTH   160
#define LCD_HEIGHT  80
#endif

// 画笔颜色
#define WHITE          0xFFFF
#define BLACK          0x0000
#define BLUE           0x001F
#define BRED           0xF81F
#define GRED           0xFFE0
#define GBLUE          0x07FF
#define RED            0xF800
#define MAGENTA        0xF81F
#define GREEN          0x07E0
#define CYAN           0x7FFF
#define YELLOW         0xFFE0
#define BROWN          0xBC40  // 棕色
#define BRRED          0xFC07  // 棕红色
#define GRAY           0x8430  // 灰色
#define DARKBLUE       0x01CF  // 深蓝色
#define LIGHTBLUE      0x7D7C  // 浅蓝色
#define GRAYBLUE       0x5458  // 灰蓝色
#define LIGHTGREEN     0x841F  // 浅绿色
#define LGRAY          0xC618  // 浅灰色(PANNEL) 窗体背景色
#define LGRAYBLUE      0xA651  // 浅灰蓝色(中间层颜色)
#define LBBLUE         0x2B12  // 浅棕蓝色(选择条目的反色)

// 字体
extern const unsigned char ASCII12[][12];
extern const unsigned char ASCII16[][16];
extern const unsigned char ASCII24[][48];
extern const unsigned char ASCII32[][64];

extern unsigned char FONT12[];
extern unsigned char FONT16[];
extern unsigned char FONT24[];
extern unsigned char FONT32[];

void LCDFill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color);

void LCDPointDraw(u16 x, u16 y, u16 color);
void LCDLineDraw(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCDRectangleDraw(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCDCircleDraw(u16 x0, u16 y0, u8 r, u16 color);

void LCDChineseDraw(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 *font, u16 num, u8 sizey, u8 mode);
void LCDCharDraw(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 sizey, u8 mode);
void LCDStringDraw(u16 x, u16 y, char *p, u16 fc, u16 bc, u8 sizey, u8 mode);

void LCDPictureDraw(u16 x, u16 y, u16 length, u16 width, const u8 pic[]);

void LCDInit();

#endif
