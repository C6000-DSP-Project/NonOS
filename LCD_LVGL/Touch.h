/****************************************************************************/
/*                                                                          */
/*              LCD                                                         */
/*                                                                          */
/*              2014年07月29日                                              */
/*                                                                          */
/****************************************************************************/
#ifndef _TOUCH_H_
#define _TOUCH_H_

#include "soc_C6748.h"

/****************************************************************************/
/*                                                                          */
/*              全局变量                                                      */
/*                                                                          */
/****************************************************************************/
// 触摸屏坐标
typedef struct
{
    unsigned char Flag;         // 触摸标志
    unsigned char Num;          // 触摸点数
    unsigned short X[10];       // 横坐标
    unsigned short Y[10];       // 纵坐标
} stTouchInfo;

extern stTouchInfo TouchInfo;

/****************************************************************************/
/*                                                                          */
/*              函数声明                                                    */
/*                                                                          */
/****************************************************************************/
void TouchInit();

void Delay(volatile unsigned int delay);

void I2CInit(unsigned int baseAddr, unsigned int slaveAddr);
void I2CHWRegWrite(unsigned int baseAddr, unsigned short regAddr, unsigned char regData);
unsigned char I2CHWRegRead(unsigned int baseAddr, unsigned short regAddr);

#endif
