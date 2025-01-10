


# UART

* 驱动路径：
	* gxloader： drivers/serial/dw_uart.c
		* virgo 只有 dw
	* linux： 
	* ecos： 
* 测试 Case：
	* gxloader:bootmenu.c
		![[Pasted image 20250106171037.png]]
	* linux:
	* ecos:

# I2C
- 驱动路径：
	- gxloader:drivers/i2c/dw_i2c. c
		- virgo 也只有 i2c
	- linux:
	- ecos:
- 测试 Case：
	- gxloader:bootmenu. c
		  ![[Pasted image 20250106171244.png]]
		- 中断、dma 补丁没合：https://git.nationalchip.com/gerrit/#/c/115970/
		- 测试需要 i2c 小板 
	- ecos:
	- linux:

# RTC
- 驱动路径：
	- gxloader: drivers/rtc/dw_rtc.c
	- linux:
	- ecos:
- 测试 Case：
	- gxloader: bootmenu. c
		![[Pasted image 20250106171421.png]]
	- linux:
	- ecos:

# WDT
- 驱动路径：
	- gxloader: drivers/watchdog/dw_wdt.c
	- linux:
	- ecos:
- 测试 Case：
	- gxloader: bootmenu. c
	  ![[Pasted image 20250106171508.png]]
	- linux:
	- ecos:



# GPIO
- 驱动路径：
	- gxloader:drivers/gpio/gx_gpio.c
		* gpio 用的是 GPIO_V3
	- linux:
	- ecos:
- 测试 Case：
	- gxloader: bootmenu. c
		![[Pasted image 20250106171605.png]]
	- linux:
	- ecos:



# RTC
- 驱动路径:
	- gxloader:
	- linux:
	- ecos:
- 测试 Case：
	- https://git.nationalchip.com/gerrit/#/c/115855/2
