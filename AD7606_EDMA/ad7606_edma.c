// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      AD7606 ADC EDMA 模式
//
//      2022年04月25日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    定时器启动 ADC 转换 转换完成后 BUSY 信号触发 EDMA3 读取数据
 *
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include "hw_types.h"
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"

#include "psc.h"
#include "gpio.h"

#include "edma.h"
#include "edma_event.h"

#include "interrupt.h"
#include "dspcache.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
#define MAX_ACOUNT        (2u)          // 16bit
#define MAX_BCOUNT        (8u)          // 采集 8 个DAC通道
#define MAX_CCOUNT        (1024u)       // 每个 ADC 通道采集 1024 组数据

#define MAX_BUFFER_SIZE   (MAX_ACOUNT * MAX_BCOUNT * MAX_CCOUNT)

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// DMA 数据缓冲区
char buf1[MAX_BUFFER_SIZE];
char buf2[MAX_BUFFER_SIZE];

// ADC 数据
extern short ADCDataCH0[MAX_CCOUNT];
extern short ADCDataCH1[MAX_CCOUNT];
extern short ADCDataCH2[MAX_CCOUNT];
extern short ADCDataCH3[MAX_CCOUNT];
extern short ADCDataCH4[MAX_CCOUNT];
extern short ADCDataCH5[MAX_CCOUNT];
extern short ADCDataCH6[MAX_CCOUNT];
extern short ADCDataCH7[MAX_CCOUNT];


// 用户程序参数
unsigned int EVT_QUEUE_NUM = 0; // 使用的事件队列

// DMA 通道配置参数
static struct EDMA3CCPaRAMEntry dmaPar[3] =
{
    {
        (unsigned int)(1 << 2 | 1 << 20 | (EDMA3_CHA_GPIO_BNKINT5 << EDMA3CC_OPT_TCC_SHIFT)),  // Opt
        (unsigned int)SOC_EMIFA_CS2_ADDR,					                                   // 源地址
        (unsigned short)(MAX_ACOUNT),                                                          // aCnt
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt
        (unsigned int)buf1,                                                                    // 目标地址
        (short)(MAX_ACOUNT),                                                                   // 源索引
        (short)(MAX_ACOUNT * MAX_CCOUNT),                                                      // 目标索引
        (unsigned short)(32u * 40u),                                                           // 链接地址
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt 重装值
        (short)(MAX_ACOUNT * MAX_BCOUNT),                                                      // 源 cIdx
        (short)(MAX_ACOUNT),                                                                   // 目标 cIdx
        (unsigned short)MAX_CCOUNT                                                             // cCnt
    },
    {
        (unsigned int)(1 << 2 | 1 << 20 | (EDMA3_CHA_GPIO_BNKINT5 << EDMA3CC_OPT_TCC_SHIFT)),  // Opt
        (unsigned int)SOC_EMIFA_CS2_ADDR,                                                      // 源地址
        (unsigned short)(MAX_ACOUNT),                                                          // aCnt
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt
        (unsigned int)buf2,                                                                    // 目标地址
        (short)(MAX_ACOUNT),                                                                   // 源 bIdx
        (short)(MAX_ACOUNT * MAX_CCOUNT),                                                      // 目标 bIdx
        (unsigned short)(32u * 41u),                                                           // 链接地址
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt 重装值
        (short)(MAX_ACOUNT * MAX_BCOUNT),                                                      // 源 cIdx
        (short)(MAX_ACOUNT),                                                                   // 目标 cIdx
        (unsigned short)MAX_CCOUNT                                                             // cCnt
    },
    {
        (unsigned int)(1 << 2 | 1 << 20 | (EDMA3_CHA_GPIO_BNKINT5 << EDMA3CC_OPT_TCC_SHIFT)),  // Opt
        (unsigned int)SOC_EMIFA_CS2_ADDR,                                                      // 源地址
        (unsigned short)(MAX_ACOUNT),                                                          // aCnt
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt
        (unsigned int)buf1,                                                                    // 目标地址
        (short)(MAX_ACOUNT),                                                                   // 源 bIdx
        (short)(MAX_ACOUNT * MAX_CCOUNT),                                                      // 目标 bIdx
        (unsigned short)(32u * 40u),                                                           // 链接地址
        (unsigned short)(MAX_BCOUNT),                                                          // bCnt 重装值
        (short)(MAX_ACOUNT * MAX_BCOUNT),                                                      // 源 cIdx
        (short)(MAX_ACOUNT),                                                                   // 目标 cIdx
        (unsigned short)MAX_CCOUNT                                                             // cCnt
    }
};

// DMA 传输完成状态
volatile int irqRaised;

unsigned int pingpong = 1;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      函数声明
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void (*cb_Fxn[EDMA3_NUM_TCC])(unsigned int tcc, unsigned int status, void *appData);

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      电源时钟使能
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
static void PSCInit()
{
    // 使能 EDMA3CC_0
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_CC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // 使能 EDMA3TC_0
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_TC0, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      回调函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void EDMA3IntCallback(unsigned int tccNum, unsigned int status, void *appData)
{
    (void)tccNum;
    (void)appData;

    if(EDMA3_XFER_COMPLETE == status)
    {
        if(pingpong)
        {
            CacheInv((unsigned int)buf1, MAX_BUFFER_SIZE);

        	memcpy(ADCDataCH0, &buf1[MAX_ACOUNT * MAX_CCOUNT * 0], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH1, &buf1[MAX_ACOUNT * MAX_CCOUNT * 1], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH2, &buf1[MAX_ACOUNT * MAX_CCOUNT * 2], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH3, &buf1[MAX_ACOUNT * MAX_CCOUNT * 3], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH4, &buf1[MAX_ACOUNT * MAX_CCOUNT * 4], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH5, &buf1[MAX_ACOUNT * MAX_CCOUNT * 5], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH6, &buf1[MAX_ACOUNT * MAX_CCOUNT * 6], MAX_ACOUNT * MAX_CCOUNT);
        	memcpy(ADCDataCH7, &buf1[MAX_ACOUNT * MAX_CCOUNT * 7], MAX_ACOUNT * MAX_CCOUNT);
        }
        else
        {
            CacheInv((unsigned int)buf1, MAX_BUFFER_SIZE);

        	memcpy(ADCDataCH0, &buf2[MAX_ACOUNT * MAX_CCOUNT * 0], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH1, &buf2[MAX_ACOUNT * MAX_CCOUNT * 1], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH2, &buf2[MAX_ACOUNT * MAX_CCOUNT * 2], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH3, &buf2[MAX_ACOUNT * MAX_CCOUNT * 3], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH4, &buf2[MAX_ACOUNT * MAX_CCOUNT * 4], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH5, &buf2[MAX_ACOUNT * MAX_CCOUNT * 5], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH6, &buf2[MAX_ACOUNT * MAX_CCOUNT * 6], MAX_ACOUNT * MAX_CCOUNT);
			memcpy(ADCDataCH7, &buf2[MAX_ACOUNT * MAX_CCOUNT * 7], MAX_ACOUNT * MAX_CCOUNT);
        }
        pingpong = !pingpong;
    }
    else if(EDMA3_CC_DMA_EVT_MISS == status)
    {
		// 传输导致 DMA 事件丢失错误
		irqRaised = -1;
    }

    else if(EDMA3_CC_QDMA_EVT_MISS == status)
    {
    	// 传输导致 QDMA 事件丢失错误
		irqRaised = -2;
    }

    irqRaised = 1;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      中断服务函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void EDMA3CCComplIsr()
{
    volatile unsigned int pendingIrqs;
    volatile unsigned int isIPR = 0;

    unsigned int indexl;
    unsigned int Cnt = 0;
    indexl = 1u;

    IntEventClear(SYS_INT_EDMA3_0_CC0_INT1);

    isIPR = EDMA3GetIntrStatus(SOC_EDMA30CC_0_REGS);
    if(isIPR)
    {
        while((Cnt < EDMA3CC_COMPL_HANDLER_RETRY_COUNT)&& (indexl != 0u))
		{
			indexl = 0u;
			pendingIrqs = EDMA3GetIntrStatus(SOC_EDMA30CC_0_REGS);
			while (pendingIrqs)
			{
				if(TRUE == (pendingIrqs & 1u))
				{
					// 如果没有指定回调函数 就无法清除 IPR 相应位
					// 写 ICR 清除 IPR 相应位
					EDMA3ClrIntr(SOC_EDMA30CC_0_REGS, indexl);
					(*cb_Fxn[indexl])(indexl, EDMA3_XFER_COMPLETE, NULL);
				}

				++indexl;
				pendingIrqs >>= 1u;
			}

			Cnt++;
		}
    }
}

void EDMA3CCErrIsr()
{
    volatile unsigned int pendingIrqs;
    unsigned int Cnt = 0u;
    unsigned int index;
    unsigned int evtqueNum = 0;  // 事件队列数目

    pendingIrqs = 0u;
    index = 1u;

    IntEventClear(SYS_INT_EDMA3_0_CC0_ERRINT);

    if((EDMA3GetErrIntrStatus(SOC_EDMA30CC_0_REGS) != 0 )
        || (EDMA3QdmaGetErrIntrStatus(SOC_EDMA30CC_0_REGS) != 0)
        || (EDMA3GetCCErrStatus(SOC_EDMA30CC_0_REGS) != 0))
    {
        // 循环 EDMA3CC_ERR_HANDLER_RETRY_COUNT 次
    	// 直到没有等待中的中断时终止
        while((Cnt < EDMA3CC_ERR_HANDLER_RETRY_COUNT) && (index != 0u))
        {
            index = 0u;
            pendingIrqs = EDMA3GetErrIntrStatus(SOC_EDMA30CC_0_REGS);

            while(pendingIrqs)
            {
				// 执行所有等待中的中断
				if(TRUE == (pendingIrqs & 1u))
				{
					// 清除 SER
					EDMA3ClrMissEvt(SOC_EDMA30CC_0_REGS, index);
				}
					++index;
					pendingIrqs >>= 1u;
			}

			index = 0u;
			pendingIrqs = EDMA3QdmaGetErrIntrStatus(SOC_EDMA30CC_0_REGS);

			while (pendingIrqs)
			{
				// 执行所有等待中的中断
				if(TRUE == (pendingIrqs & 1u))
				{
					// 清除 SER
					EDMA3QdmaClrMissEvt(SOC_EDMA30CC_0_REGS, index);
				}
				++index;
				pendingIrqs >>= 1u;
			}

			index = 0u;
			pendingIrqs = EDMA3GetCCErrStatus(SOC_EDMA30CC_0_REGS);

			if(pendingIrqs != 0u)
			{
				// 执行所有等待中的 CC 错误中断
				// 事件队列 队列入口错误
				for(evtqueNum = 0u; evtqueNum < SOC_EDMA3_NUM_EVQUE; evtqueNum++)
				{
					if((pendingIrqs & (1u << evtqueNum)) != 0u)
					{
						// 清除错误中断
						EDMA3ClrCCErr(SOC_EDMA30CC_0_REGS, (1u << evtqueNum));
					}
				}

				// 传输完成错误
				if((pendingIrqs & (1 << EDMA3CC_CCERR_TCCERR_SHIFT)) != 0u)
				{
					EDMA3ClrCCErr(SOC_EDMA30CC_0_REGS, (0x01u << EDMA3CC_CCERR_TCCERR_SHIFT));
				}

				++index;
			}

			Cnt++;
        }
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      EDMA3 中断初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void EDMA3InterruptInit()
{
    // 传输完成中断
    IntRegister(C674X_MASK_INT7, EDMA3CCComplIsr);
    IntEventMap(C674X_MASK_INT7, SYS_INT_EDMA3_0_CC0_INT1);
    IntEnable(C674X_MASK_INT7);

    // 传输错误中断
    IntRegister(C674X_MASK_INT8, EDMA3CCErrIsr);
    IntEventMap(C674X_MASK_INT8, SYS_INT_EDMA3_0_CC0_ERRINT);
    IntEnable(C674X_MASK_INT8);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      获取 EDMA3 版本
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned int EDMAVersionGet()
{
    return 1;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      EDMA3 初始化
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void AD7606EDMA3Init()
{
    // 使能外设
    PSCInit();

    // EDMA3 中断初始化
    EDMA3InterruptInit();

    // EDMA3 初始化
    EDMA3Init(SOC_EDMA30CC_0_REGS, EVT_QUEUE_NUM);

    // 申请 DMA 通道
    unsigned int retVal = 0u;
    retVal = EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
                                 EDMA3_CHA_GPIO_BNKINT5, EDMA3_CHA_GPIO_BNKINT5, EVT_QUEUE_NUM);

    // 注册回调函数
    cb_Fxn[EDMA3_CHA_GPIO_BNKINT5] = &EDMA3IntCallback;

    if(retVal == TRUE)
    {
        // 初始化 DMA 参数
        EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, EDMA3_CHA_GPIO_BNKINT5, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[0])));
        EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, 40, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[1])));
        EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, 41, (struct EDMA3CCPaRAMEntry *)(&(dmaPar[2])));

        // 使能传输
        EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_GPIO_BNKINT5, EDMA3_TRIG_MODE_EVENT);
    }
}
