#ifndef __PS2_H
#define __PS2_H

// ��ֵ
#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2          9
#define PSB_R2          10
#define PSB_L1          11
#define PSB_R1          12
#define PSB_GREEN       13
#define PSB_RED         14
#define PSB_BLUE        15
#define PSB_PINK        16
#define PSB_TRIANGLE    13
#define PSB_CIRCLE      14
#define PSB_CROSS       15
#define PSB_SQUARE      26

#define PSS_RX          5   // ��ҡ��
#define PSS_RY          6
#define PSS_LX          7   // ��ҡ��
#define PSS_LY          8

void PS2Init();
unsigned char PS2RedLight();
unsigned char PS2KEYData();
unsigned char PS2AnologData(unsigned char button);
void PS2Vibration(unsigned char motor1, unsigned char motor2);

#endif
