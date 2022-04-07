/****************************************************************************/
/*                                                                          */
/*    �º˿Ƽ�(����)���޹�˾                                                */
/*                                                                          */
/*    Copyright (C) 2022 CoreKernel Technology (Guangzhou) Co., Ltd         */
/*                                                                          */
/****************************************************************************/
/*
 *    - ϣ����Ĭ(bin wang)
 *    - bin@corekernel.net
 *
 *    ���� corekernel.net/.org/.cn
 *    ���� fpga.net.cn
 *
 */
#ifndef Log_H_
#define Log_H_

#include <stdio.h>

/****************************************************************************/
/*                                                                          */
/*              �������                                                    */
/*                                                                          */
/****************************************************************************/
extern void UARTprintf(const char *fmt, ...);

#define DebugWrite(format, ...)		\
     do {							\
                printf("[%16lld | %16s @ %16s, %4d] " format, _itoll(TSCH, TSCL), __func__, __FILE__, __LINE__, ##__VA_ARGS__ ); \
            UARTprintf("[%16lld | %16s @ %16s, %4d] " format, _itoll(TSCH, TSCL), __func__, __FILE__, __LINE__, ##__VA_ARGS__ ); \
        } while (0)

#define ConsoleWrite(format, ...)  \
     do {							\
    	    printf(format, ##__VA_ARGS__ );      \
    	    UARTprintf(format, ##__VA_ARGS__ );  \
        } while (0)

#endif
