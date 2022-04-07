/****************************************************************************/
/*                                                                          */
/*    新核科技(广州)有限公司                                                */
/*                                                                          */
/*    Copyright (C) 2022 CoreKernel Technology (Guangzhou) Co., Ltd         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*    LCD                                                                   */
/*                                                                          */
/*    2014年10月14日                                                        */
/*                                                                          */
/****************************************************************************/
/*
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include "hw_types.h"
#include "hw_syscfg0_C6748.h"
#include "hw_psc_C6748.h"

#include "soc_C6748.h"
#include "psc.h"
#include "gpio.h"
#include "raster.h"

#include "interrupt.h"

#include "LCD.h"

/****************************************************************************/
/*                                                                          */
/*              全局变量                                                    */
/*                                                                          */
/****************************************************************************/
#pragma DATA_ALIGN(g_pucBuffer, 4);
#pragma DATA_SECTION(g_pucBuffer, ".DDR2");
unsigned char g_pucBuffer[2][4 + (16 * 2) + (LCD_WIDTH * LCD_HEIGHT * 2)];

/****************************************************************************/
/*                                                                          */
/*              GPIO 管脚复用配置                                           */
/*                                                                          */
/****************************************************************************/
static void GPIOBankPinMuxSet()
{
    // LCD 信号
    #define PINMUX16_LCD_ENABLE     (SYSCFG_PINMUX16_PINMUX16_31_28_LCD_D2 << SYSCFG_PINMUX16_PINMUX16_31_28_SHIFT) | \
                                     (SYSCFG_PINMUX16_PINMUX16_27_24_LCD_D3 << SYSCFG_PINMUX16_PINMUX16_27_24_SHIFT) | \
                                     (SYSCFG_PINMUX16_PINMUX16_23_20_LCD_D4 << SYSCFG_PINMUX16_PINMUX16_23_20_SHIFT) | \
                                     (SYSCFG_PINMUX16_PINMUX16_19_16_LCD_D5 << SYSCFG_PINMUX16_PINMUX16_19_16_SHIFT) | \
                                     (SYSCFG_PINMUX16_PINMUX16_15_12_LCD_D6 << SYSCFG_PINMUX16_PINMUX16_15_12_SHIFT) | \
                                     (SYSCFG_PINMUX16_PINMUX16_11_8_LCD_D7 << SYSCFG_PINMUX16_PINMUX16_11_8_SHIFT)

    #define PINMUX17_LCD_ENABLE     (SYSCFG_PINMUX17_PINMUX17_31_28_LCD_D10 << SYSCFG_PINMUX17_PINMUX17_31_28_SHIFT) | \
                                     (SYSCFG_PINMUX17_PINMUX17_27_24_LCD_D11 << SYSCFG_PINMUX17_PINMUX17_27_24_SHIFT) | \
                                     (SYSCFG_PINMUX17_PINMUX17_23_20_LCD_D12 << SYSCFG_PINMUX17_PINMUX17_23_20_SHIFT) | \
                                     (SYSCFG_PINMUX17_PINMUX17_19_16_LCD_D13 << SYSCFG_PINMUX17_PINMUX17_19_16_SHIFT) | \
                                     (SYSCFG_PINMUX17_PINMUX17_15_12_LCD_D14 << SYSCFG_PINMUX17_PINMUX17_15_12_SHIFT) | \
                                     (SYSCFG_PINMUX17_PINMUX17_11_8_LCD_D15 << SYSCFG_PINMUX17_PINMUX17_11_8_SHIFT) | \
                                     (SYSCFG_PINMUX17_PINMUX17_7_4_LCD_D0 << SYSCFG_PINMUX17_PINMUX17_7_4_SHIFT) | \
                                     (SYSCFG_PINMUX17_PINMUX17_3_0_LCD_D1 << SYSCFG_PINMUX17_PINMUX17_3_0_SHIFT)

    #define PINMUX18_LCD_ENABLE     (SYSCFG_PINMUX18_PINMUX18_31_28_LCD_MCLK << SYSCFG_PINMUX18_PINMUX18_31_28_SHIFT) | \
                                     (SYSCFG_PINMUX18_PINMUX18_27_24_LCD_PCLK << SYSCFG_PINMUX18_PINMUX18_27_24_SHIFT) | \
                                     (SYSCFG_PINMUX18_PINMUX18_7_4_LCD_D8 << SYSCFG_PINMUX18_PINMUX18_7_4_SHIFT) | \
                                     (SYSCFG_PINMUX18_PINMUX18_3_0_LCD_D9 << SYSCFG_PINMUX18_PINMUX18_3_0_SHIFT)

    #define PINMUX19_LCD_ENABLE     (SYSCFG_PINMUX19_PINMUX19_27_24_NLCD_AC_ENB_CS << SYSCFG_PINMUX19_PINMUX19_27_24_SHIFT) | \
                                     (SYSCFG_PINMUX19_PINMUX19_7_4_LCD_VSYNC << SYSCFG_PINMUX19_PINMUX19_7_4_SHIFT) | \
                                     (SYSCFG_PINMUX19_PINMUX19_3_0_LCD_HSYNC << SYSCFG_PINMUX19_PINMUX19_3_0_SHIFT)

    unsigned int savePinMux = 0;

    savePinMux = HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(16)) & \
                       ~(SYSCFG_PINMUX16_PINMUX16_31_28 | \
                         SYSCFG_PINMUX16_PINMUX16_27_24 | \
                         SYSCFG_PINMUX16_PINMUX16_23_20 | \
                         SYSCFG_PINMUX16_PINMUX16_19_16 | \
                         SYSCFG_PINMUX16_PINMUX16_15_12 | \
                         SYSCFG_PINMUX16_PINMUX16_11_8);

    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(16)) = (PINMUX16_LCD_ENABLE | savePinMux);

    savePinMux = HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(17)) & \
                       ~(SYSCFG_PINMUX17_PINMUX17_31_28 | \
                         SYSCFG_PINMUX17_PINMUX17_27_24 | \
                         SYSCFG_PINMUX17_PINMUX17_23_20 | \
                         SYSCFG_PINMUX17_PINMUX17_19_16 | \
                         SYSCFG_PINMUX17_PINMUX17_15_12 | \
                         SYSCFG_PINMUX17_PINMUX17_11_8 | \
                         SYSCFG_PINMUX17_PINMUX17_7_4 | \
                         SYSCFG_PINMUX17_PINMUX17_3_0);

    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(17)) = (PINMUX17_LCD_ENABLE | savePinMux);

    savePinMux = HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(18)) & \
                       ~(SYSCFG_PINMUX18_PINMUX18_31_28 | \
                         SYSCFG_PINMUX18_PINMUX18_27_24 | \
                         SYSCFG_PINMUX18_PINMUX18_7_4 |  \
                         SYSCFG_PINMUX18_PINMUX18_3_0);

    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(18)) = (PINMUX18_LCD_ENABLE | savePinMux);

    savePinMux = HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(19)) & \
                       ~(SYSCFG_PINMUX19_PINMUX19_27_24 | \
                         SYSCFG_PINMUX19_PINMUX19_7_4 | \
                         SYSCFG_PINMUX19_PINMUX19_3_0);

    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(19)) = (PINMUX19_LCD_ENABLE | savePinMux);

    // LCD 背光
    savePinMux = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) & ~(SYSCFG_PINMUX1_PINMUX1_3_0));
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) = ((SYSCFG_PINMUX1_PINMUX1_3_0_GPIO0_7 << SYSCFG_PINMUX1_PINMUX1_3_0_SHIFT) | savePinMux);
}

/****************************************************************************/
/*                                                                          */
/*              LCD 背光                                                    */
/*                                                                          */
/****************************************************************************/
void LCDBacklightEnable()
{
    // 使能背光 GPIO0[7]（也可以使用 ECAP APWM2 调光）
    GPIODirModeSet(SOC_GPIO_0_REGS, 8, GPIO_DIR_OUTPUT);
    GPIOPinWrite(SOC_GPIO_0_REGS, 8, 1);
}

void LCDBacklightDisable()
{
    // 禁用背光 GPIO0[7]
    GPIODirModeSet(SOC_GPIO_0_REGS, 8, GPIO_DIR_OUTPUT);
    GPIOPinWrite(SOC_GPIO_0_REGS, 8, 0);
}

/****************************************************************************/
/*                                                                          */
/*              硬件中断线程                                                */
/*                                                                          */
/****************************************************************************/
#include "LVGL/porting/lv_port_disp_c674x.h"
extern lv_disp_drv_t disp_drv;
void LCDISR()
{
    unsigned int  status;

    status = RasterIntStatus(SOC_LCDC_0_REGS,  RASTER_FIFO_UNDERFLOW_INT_STAT |
                                               RASTER_END_OF_FRAME0_INT_STAT  |
                                               RASTER_END_OF_FRAME1_INT_STAT );

    if(status & RASTER_FIFO_UNDERFLOW_INT_STAT)
    {
        RasterDisable(SOC_LCDC_0_REGS);
        RasterEnable(SOC_LCDC_0_REGS);
    }

    status = RasterClearGetIntStatus(SOC_LCDC_0_REGS, status);

    /* 重要!!!
     * 通知图形库显示更新就绪 */
//    lv_disp_flush_ready(&disp_drv);
}

/****************************************************************************/
/*                                                                          */
/*              中断初始化                                   */
/*                                                                          */
/****************************************************************************/
static void InterruptInit()
{
    IntRegister(C674X_MASK_INT5, LCDISR);
    IntEventMap(C674X_MASK_INT5, SYS_INT_LCDC_INT);
    IntEnable(C674X_MASK_INT5);
}

/****************************************************************************/
/*                                                                          */
/*              版本识别                                                    */
/*                                                                          */
/****************************************************************************/
unsigned int LCDVersionGet()
{
    return 1;
}

/****************************************************************************/
/*                                                                          */
/*              LCD 初始化                                                  */
/*                                                                          */
/****************************************************************************/
void LCDInit()
{
	// 使能 LCD 模块
	PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_LCDC, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

	// 管脚复用配置
	GPIOBankPinMuxSet();

    // 中断初始化
	InterruptInit();

	// LCD 初始化
    // 禁用光栅
    RasterDisable(SOC_LCDC_0_REGS);
    
    // 时钟配置
    RasterClkConfig(SOC_LCDC_0_REGS, 30000000, LCD_CLK);

    // 配置 LCD DMA 控制器
    RasterDMAConfig(SOC_LCDC_0_REGS, RASTER_DOUBLE_FRAME_BUFFER, RASTER_BURST_SIZE_16, RASTER_FIFO_THRESHOLD_8, RASTER_BIG_ENDIAN_DISABLE);

    // 模式配置(例如:TFT 或者 STN,彩色或者黑白 等等)
    RasterModeConfig(SOC_LCDC_0_REGS, RASTER_DISPLAY_MODE_TFT, RASTER_PALETTE_DATA, RASTER_COLOR, RASTER_RIGHT_ALIGNED);

    // 帧缓存数据以 LSB 方式排列
    RasterLSBDataOrderSelect(SOC_LCDC_0_REGS);
    
    // 禁用 Nibble 模式
    RasterNibbleModeDisable(SOC_LCDC_0_REGS);
   
    // 配置光栅控制器极性
    RasterTiming2Configure(SOC_LCDC_0_REGS, RASTER_FRAME_CLOCK_LOW | RASTER_LINE_CLOCK_LOW | RASTER_PIXEL_CLOCK_HIGH | RASTER_SYNC_EDGE_RISING | RASTER_SYNC_CTRL_ACTIVE | RASTER_AC_BIAS_HIGH, 0, 255);

    // 配置水平 / 垂直参数
    RasterHparamConfig(SOC_LCDC_0_REGS, 800, 30, 210, 45);
    RasterVparamConfig(SOC_LCDC_0_REGS, 480, 10, 21, 22);

    // 配置 FIFO DMA 延时
    RasterFIFODMADelayConfig(SOC_LCDC_0_REGS, 2);

    // 配置显存
    RasterDMAFBConfig(SOC_LCDC_0_REGS, (unsigned int)(g_pucBuffer[0] + PALETTE_OFFSET), (unsigned int)(g_pucBuffer[0] + PALETTE_OFFSET) + sizeof(g_pucBuffer[0]) - 2 - PALETTE_OFFSET, 0);
    RasterDMAFBConfig(SOC_LCDC_0_REGS, (unsigned int)(g_pucBuffer[1] + PALETTE_OFFSET), (unsigned int)(g_pucBuffer[1] + PALETTE_OFFSET) + sizeof(g_pucBuffer[1]) - 2 - PALETTE_OFFSET, 1);

    // 16Bit 无需调色板
    *((unsigned short *)&g_pucBuffer[0][4]) = 0x4000;
    *((unsigned short *)&g_pucBuffer[1][4]) = 0x4000;

    // 使能 LCD 帧结束中断
    RasterEndOfFrameIntEnable(SOC_LCDC_0_REGS);
    RasterIntEnable(SOC_LCDC_0_REGS, RASTER_END_OF_FRAME0_INT | RASTER_END_OF_FRAME1_INT);

    // 使能光栅
    RasterEnable(SOC_LCDC_0_REGS);
}
