/****************************************************************************/
/*                                                                          */
/*    新核科技(广州)有限公司                                                */
/*                                                                          */
/*    Copyright (C) 2022 CoreKernel Technology (Guangzhou) Co., Ltd         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*    LVGL DSP C6748 底层接口移植                                           */
/*                                                                          */
/*    2022年04月06日                                                        */
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
#if 1

#include "lv_port_disp_c674x.h"

#include "../../LCD.h"

/****************************************************************************/
/*                                                                          */
/*              全局变量                                                    */
/*                                                                          */
/****************************************************************************/
lv_disp_drv_t disp_drv;            /* 显示驱动描述符 */

/****************************************************************************/
/*                                                                          */
/*              函数声明                                                    */
/*                                                                          */
/****************************************************************************/
static void disp_init(void);
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
static void disp_monitor_cb(lv_disp_drv_t *disp_drv, uint32_t time, uint32_t px);

/****************************************************************************/
/*                                                                          */
/*              LVGL 显示初始化                                             */
/*                                                                          */
/****************************************************************************/
void lv_port_disp_init(void)
{
    /*-------------------------
     * LCD 控制器初始化
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * 创建显示缓冲区
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */

    /* 双显示缓冲区 */
    static lv_disp_draw_buf_t draw_buf_dsc;
    unsigned char *LCDBuf0 = (unsigned char *)g_pucBuffer[0] + 36;
    unsigned char *LCDBuf1 = (unsigned char *)g_pucBuffer[1] + 36;
    lv_disp_draw_buf_init(&draw_buf_dsc, LCDBuf0, LCDBuf1, LCD_WIDTH * LCD_HEIGHT);

    /*-----------------------------------
     * 注册 LVGL 显示设备
     *----------------------------------*/
    lv_disp_drv_init(&disp_drv);

    /* 设置屏幕分辨率 */
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;

    /* 回调函数 */
    disp_drv.flush_cb = disp_flush;
    disp_drv.monitor_cb = disp_monitor_cb;

    /* 配置显示缓冲区 */
    disp_drv.draw_buf = &draw_buf_dsc;

    /* 强制全屏刷新 */
    disp_drv.full_refresh = 1;

    /* 注册驱动 */
    lv_disp_drv_register(&disp_drv);
}

/****************************************************************************/
/*                                                                          */
/*              显示控制器初始化                                            */
/*                                                                          */
/****************************************************************************/
static void disp_init(void)
{
    /* TODO 在外部初始化 LCD 控制器 */

}

/****************************************************************************/
/*                                                                          */
/*              更新显示                                                    */
/*                                                                          */
/****************************************************************************/
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
//    int32_t x;
//    int32_t y;
//
//    unsigned short *LCDBuf = (unsigned short *)g_pucBuffer;
//
//    for(y = area->y1; y <= area->y2; y++)
//    {
//        for(x = area->x1; x <= area->x2; x++)
//        {
//            LCDBuf[((PALETTE_OFFSET + PALETTE_SIZE) / 2) + y * LCD_WIDTH + x] = *((unsigned short *)color_p);
//            color_p++;
//        }
//    }

    /* 维护缓存一致性 */

    /* 重要!!!
     * 通知图形库显示更新就绪 */
    lv_disp_flush_ready(disp_drv);
}

/****************************************************************************/
/*                                                                          */
/*              更新显示                                                    */
/*                                                                          */
/****************************************************************************/
void disp_monitor_cb(lv_disp_drv_t *disp_drv, uint32_t time, uint32_t px)
{

}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
