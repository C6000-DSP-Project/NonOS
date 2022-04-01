C6748 DSP 无操作系统例程

CCSv7 IDE
- 7.4 不依赖 CCS IDE 版本 
      但是工程基于 CCSv7 创建 低于该版本 CCS 需要重新创建工程
    
编译器
- TI C6000 CGT 8.3.12

依赖组件
- 依赖库位于 Include/Library 目录

测试方法

核心板


EVM-CKL138PKT 底板

WM8960 音频模块硬件连接

| 模块 | 底板 | 引脚说明 |
| :---- | :---- | :---- |
|I2C_SDA | CON16.27 | GPIO1[04]/SPI1_SCS6/TIMER64P3_OUT12/I2C0_SDA |
|I2C_SCL | CON16.28 | GPIO1[05]/SPI1_SCS7/TIMER64P2_OUT12/I2C0_SCL |
|I2S_CLK | CON16.13 | GPIO0[14]/McASP_ACLKX |
|I2S_LRCLK | CON16.25 | GPIO0[12]/McASP_AFSX |
|I2S_DAC | CON16.16 | GPIO0[03]/McASP_AXR11/McBSP1_FSX1 |

已知问题 需要断开 I2C CLK/FS 信号等待 WM8960 I2C 初始化完成后再连接

- WM8960_LINE_OUT      运行程序会循环播放一段音乐
- WM8960_LINE_OUT_WAV  需要使用 CCSv5 加载运行
                       [1] 加载程序
			           [2] 使用 CCS Load Memory 方式加载 WAV 格式音频文件到内存 wav_sound 中(0xC0001000)
			           [3] 运行程序会循环播放 WAV 文件
					   WAV 文件采样率需要为 44.1K 与音频芯片配置匹配