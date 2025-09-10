
# 增加单线模式，回归验证
- ok





# 灯效

https://github.com/kitesurfer1404/WS2812FX
https://colordesigner.io/convert/hsvtorgb

## 如何表示 bit1、bit0
- 现表示的方式：
	- 2byte 表示一个 ws2812 的 bit，2byte 为另一个输出脚，不输出；时钟频率是 32MHz，一个周期是 31.25ns；配置的延时是 2，那么实际的周期数就是 `31.25*3 = 93.75ns`
	- bit1：`0x00005555` 就是 8 个 1，8 个 0，高电平时长为 `93.75*8 = 750ns`，低电平 `750ns`
	- bit0：`0x00000555` 就是 6 个 1，10 个 0，高电平时长为 `93.75*6 = 562.5ns`，低电平 `937.5ns`
- 换成用 6bit 表示：
- 换成用 4bit 表示：

8MHZ：一个周期是 125ns
- 4bit 表示：
	- bit1：0011
	- bit0：0111
- 原本的波形：
	- bit0：
		- 高电平时长：360ns
		- 低电平时长：620ns
	- bit1：
		- 高电平时长：500ns
		- 低电平时长：500ns
解决方案：
- iodma 模块频率分到 16MHz，一个周期 62.5us，tx_delay 配到 4，则一个周期持续时间为 312.5us
- 用 iodma 的 4 个 bit 表示 ws2812 的 1 个 Bit，再加上另外一个引脚的空闲输出，就是一个 byte 表示 ws2812 的 1 个 Bit
- ws2812：
    - 0 码高电平：1 个周期高电平 (312.5us)，3 个周期低电平 (937.5us)
    - 1 码高电平：2 个周期高电平 (625us)，2 个周期低电平 (625us)


## 【已解决】Q1：tx_delay 配到很大的时候电压会往下掉
- tx_delay 配成 2/3
![[aa809ea92b027e33db97c0fd05e5a80a.jpg]]
- tx_delay 配成 15000
![[740426926a050163030b19909d7d11ca.jpg]]
- tx_delay 配成 1000
![[36aa0f9079934b76b2c5efedf4dceb10.jpg]]
- tx_delay 配成 5000
![[5a934ae853dc8751e3f4884fb43d72f7.jpg]]
- tx_delay 配成 3000
![[ff5f2cc633a7849a0c5b043e971c138f.jpg]]

- tx_delay 配成 2000
![[8905802beedefecbae40570a0a4fe386.jpg]]


- **A：gen_spi 分配比配的很大的时候也是一样的**


## QMK 如何表示各种不同的灯效
- 网站： https://wokwi.com/projects/404843885655728129
	- 在线模拟 esp32 和 ws2812 
- hsv：
	- h：Hue 色相
		- 色相通过一个角度值来表示，范围 `0~360°`
		- 0 表示红色、120 表示绿色、240 表示蓝色
		- 通过调整色相，可以改变颜色的类别。
		- 例如：从 0~120，颜色从红色逐渐过渡到绿色
	- s：Saturation 饱和度
		- 饱和度表示颜色的纯度或强度。饱和度越高，颜色越鲜艳；饱和度越低，颜色越接近灰色；范围 `0~100%`
		- `0%` 表示灰色 (无色彩)
		- `100%` 表示最纯净、最鲜艳的颜色。
		- 例如：一个高饱和度的红色会非常鲜艳，而低饱和度的红色则偏向灰红色。
	- v：Value 明度
		- 明度表示颜色的亮度。亮度越高，颜色越亮；亮度越低，颜色越暗；范围 `0~100%`
		- `0%` 表示完全黑色
		- `100%` 表示最亮的颜色
		- 例如：一个高明度的黄色会显得明亮耀眼，而低明度的黄色则显得暗淡。
- 不同的灯效做成回调，定时器中根据当前的模式跳到对应的回调中处理灯光效果
- rainbow：
	- 有一个定时器，定时器会定时更新灯光数据
	- 更新灯光数据，则根据参数来修改 `hsv.h` 也就是色相值即可完成 rainbow
- breath：
	- 有一个定时器，定时器会定时更新灯光数据
	- 根据配置的参数，计算得到新的 `hsv.v` 也就是明亮度，即可完成 breath

## 事业部键盘灯效：
```


driver_test ws2812 breath 240 100 100 4000 1

driver_test ws2812 flow 240 100 100

driver_test ws2812 key_press 0 0 100 3000

```




# 带死区 pwm

首先考虑使用 iodma 输出 pwm 的方式，再进一步考虑如何输出带死区的 pwm，以及 spwm，互补 pwm、互补 spwm 等等。

## pwm
实现方法 1：
- 由于每次发送的数据都是 word，共 32bit，因此最小颗粒度为 `1/32 = 3.125%`
	- 占空比 0：32 个 0
	- 占空比 50：16 个 0，16 个 1
	- 占空比 100：32 个 1
实现方法 2：
- 发送 100 个 word，每个 word 表示 `单个 1/0`，因此 tx_delay 的配置需要为 1/32，对于频率的要求较高
	- 占空比 0：连续发 100 个 0
	- 占空比 50：连续发 50 个 0，连续发 50 个 1
	- 占空比 100：连续发 100 个 1

对于上面的两种实现方式，考虑先使用方法 1 来实现，颗粒度足够，并且对于频率要求不高。

实际测试方法 1，IODMA 模块频率为 48MHz，能够正常表示 100KHz，但无法表示 1MHz。
- 使用一个 word 来表示数据，相当于 32bit 表示一个周期。
- 因此能输出的最高频率为：48MHz/32 = 1.5MHz
	- 当要输出 1MHz 的时候，需要的延时是 31ns，32 个就是差不多 1000ns，但是 iodma 频率为 48MHz，一个周期是 20.83ns，分不出来 31ns，所以其实 1MHz 也达不到。
- 考虑是否可以 10bit 表示一个周期：最高能表示 4MHz 左右
	- 假设要输出 1MHz，一个周期时长为 1000ns，分到 10 个周期就是 100ns；
	- 48MHz 的 iodma 频率，分 5 个差不多 104.15ns 。可以达到这个频率
- 如果需要更高的频率，那么可以分出来的占空比就更少，跨度很大
	- 假设用 4bit 表示一个周期：最高能表示 10MHz 左右，但能表示的占空比很少
		- 假设要输出 1MHz，一个周期时长为 1000ns，分到 4 分就是 250ns；
		- 40MHz 的 iodma 频率，分 10 多个分得到，可以达到该频率
		- 假设要输出 10MHz，一个周期时长为 100ns，分到 4 份就是 25ns；
		- 40MHz 的 iodma 频率，分到 1 或者 2，也可以达到该频率
- **最终决定使用 16bit 表示 pwm 的一个周期：能够表示的最高频率为 3MHz**
	- 根据周期和占空比计算得到数据和 tx_delay 的值
	- 由于是 16bit 表示一个周期，而 iodma 最小发送单位为 word，因此连发两个相同的数据



## 互补 pwm
假设是 50% 的占空比，此时主通道输出 50% 时间，互补通道输出 50% 时间：
```

32 bit:

16 bit 表示一个周期：8bit 高电平，8bit 低电平


32bit 表示一个周期，16bit 高电平，16bit 低电平
0000 0000 1111 1111
1111 1111 0000 0000
```

### 【已解决】问题 ：使用双线输出时，如果 tx_delay = 0，输出的数据高电平时间正常，低电平会多保持 2 个 delay
- iodma 频率为 48MHz，tx_delay 配为 0，数据为 2 个 word，依次为 `0xaaaa5555、0xaaaa5555`，则 io0 发的数据：8 个 1，8 个 0，8 个 1，8 个 0，实际看到的是 8 个 1，10 个 0，8 个 1，10 个 0
	- tx_delay 配成 1 或其他值的时候没有这个问题，不会多发
	- iodma 的时钟配成晶振频率 24MHz，没问题
	- iodma 从 pll 的 48MHz 做 2 分频到 24MHz，也没问题
```c
//#define CONFIG_SAGITTA_IODMA_TXDELAY_TEST

  

#ifdef CONFIG_SAGITTA_IODMA_TXDELAY_TEST

#include <gx_clock.h>

#include <gx_dcache.h>

uint32_t test_data[1000] = {0};

#endif

  

int main(int argc, char **argv)

{

loader_init();

  

#ifdef CONFIG_SAGITTA_IODMA_TXDELAY_TEST

// 准备测试数据（假设在0x200000）

test_data[0] = 0xaaaa5555;

test_data[1] = 0xaaaa5555;

  

gx_dcache_clean();

gx_clk_mod_set_src(GX_CLK_MOD_IODMA, GX_CLK_MOD_SRC_PLL);

// gx_clk_mod_set_div(GX_CLK_MOD_IODMA, 2);

gx_clk_mod_set_gate(GX_CLK_MOD_IODMA, 1);

  

// 1. 配置引脚复用

padmux_set(0, 46); // P0_0 UART_CTS0_L IODMA[0]

padmux_set(1, 47); // P0_1 UART_RTS0_L IODMA[1]

  

// 2. 配置 IODMA、dma lite

*(volatile unsigned int *)(0xB0D00000 + 0x4) = 0; // IODMA_EN = 0

*(volatile unsigned int *)(0xB0D00000 + 0x1000+ 0x4) = 0; // DMA_LITE_EN = 0

*(volatile unsigned int *)(0xB0D00000 + 0xC) = 0; // IODMA_IER = 0

*(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0xC) = 0; // DMA_LITE_IER = 0

  
  

// 3. 配置 iodma

*(volatile unsigned int *)(0xB0D00000 + 0x8) = 0; // IODMA_CTRL = 0，输出数据bit[1:0]有效，固定长度模式

  

*(volatile unsigned int *)(0xB0D00000 + 0x14) = 2; // IODMA_TX_NUM = 2 48字节

  

*(volatile unsigned int *)(0xB0D00000 + 0x18) = 0; // IODMA_TX_DELAY = 0 延时 0

  

// 4. 配置 DMA_LITE

*(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x38) = 0; // DMA_LITE_ADDR_LEN_USED_NUM = 0 使用第一组

*(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x18) = virt_to_phys(&test_data); // DMA_LITE_ADDR0 = 0x200000

*(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x1C) = 2; // DMA_LITE_LEN0 = 2

  

*(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x0C) = 0x3f; // DMA_LITE_IER 开启所有中断

  

*(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x3C) = 0x100; // DMA_LITE_TO_VALUE = 0x100

  

*(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x8) = 0; // DMA_LITE_CTRL = 0，读模式

  

*(volatile unsigned int *)(0xB0D00000 + 0xc) = 1; // IODMA_IER = 1，使能 iodma 的 tx done 中断

*(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x4) = 1; // DMA_LITE_EN = 1，使能 dma lite

*(volatile unsigned int *)(0xB0D00000 + 0x4) = 1; // IODMA_EN = 1，使能 iodma

  

#endif

app_start();

  

while(1)

continue;

}
```
