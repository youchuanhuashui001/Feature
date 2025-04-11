

# gxloader
- 非中断
	- init: 设置波特率、81N、FIFO Enable (Mode1)、
	- uart_getc:
		- 循环查询 `LSR. DR = 1` 表示数据准备好了
		- 从 `RBR` 寄存器读一个字符
	- uart_putc:
		- 循环查询 `LSR.TEMT` 等待 `tx fifo empty`
		- 把字符往 `THR` 寄存器写
	- 设置波特率：让波特率尽可能准确
		- 设置 LCR 寄存器的 DLAB 位
		- 循环查询 `USR.BUSY`
		- 手册描述时钟分频计算方法：`clkdiv = uart_clock / (16*baudrate)`
		- 驱动中的时钟分频计算方法：`clkdiv = (uart_clock + (8*baudrate)) / (16*baudrate)
		- 小数波特率计算方法：`主频/（16*baudrate） = 整数 + 小数`，`小数*2^DLF_SIZE = dlf`，将这个值写到 `DLF` 寄存器
			- 驱动计算方法：`先算出小数部分，再 * 2^DLF_SIZE，然后将算出来的值除以波特率，进行四舍五入，得到 dlf，写到 DLF 寄存器`  
		- 把计算出来的 `clkdiv` 高 `8` 位写到 `DLH`，低 `8` 位写到 `DLL`，并且小数波特率写到 `DLF` 寄存器
	- 获取 fifo 深度
		- 从 `CPR` 寄存器的第 16 位获取配置的 fifo 深度
	- 获取触发 `tx fifo empty` 或 ` rx fifo 数据可用 ` 中断
		- rx：从影子寄存器 `SRT` 中读到配置的 rx 触发等级
		- tx：从影子寄存器 `STET` 中读到配置的 tx 触发等级
	- uart_tstc
		- 读 `LSR` 寄存器，如果有数据可读返回 1，无数据可读返回 0
	- uart_halt_tx
		- 配置 `HTX` 寄存器为 1 则停止发送；配置为 0 则继续发送
	- uart_get_tx_fifo_size
		- 读 `TFL` 寄存器获取当前 `tx fifo` 中有多少个数据
	- uart_get_rx_fifo_size
		- 读 `RFL` 寄存器获取当前 `rx fifo` 中有多少个数据
	- uart_enable_auto_flow
		- 读 `MCR` 寄存器
		- 修改 `MCR` 寄存器中的 `AFCE、RTS` 位来启用 `tx、rx` 的自动流控
		- 写回寄存器
		- 配置 `FCR` 寄存器为 `0xC7`
		- 备注：
			- `RTS` 信号用于发送请求，为输出信号，表示准备好接收数据了，低电平有效
			- `CTS` 信号用于接收请求，为输入信号，表示可以发送数据了，低电平有效
			- 使能 `RTS` 后告诉对方设备可以发数据了
			- 对方设备开始发数据
			- 发数据过程中一直检测自己的 fifo 情况，如果不能接收数据了就会拉高 `RTS` 引脚，对方的 `CTS` 为输入信号，检测到拉高之后就会暂停传输数据。
			- 等自己的 fifo 有空闲了，自动拉低 `RTS` 引脚继续传输
	- uart_start_break
		- 读 `LCR` 寄存器
		- 修改，配置 `BC` 位，用于停止 `transmit`
		- 写回 `LCR` 寄存器
	- uart_end_break
		- 读 `LCR` 寄存器
		- 修改，清掉 `BC` 位，用于恢复 `transmit`
		- 写回 `LCR` 寄存器
- 中断
	- init：
		- alloc 一个 kfifo
		- 写 `IER` 寄存器为 1 使能接收中断
		- 注册中断服务函数
	- irq：
		- 读 `LSR` 寄存器，查询是否有数据
		- 有数据则从 `RBR` 寄存器读一个字节数据，然后把数据放到 kfifo
	- uart_flush_in
		- 关中断
		- 清空 kfifo
		- 开中断
	- getc
		- 判断 kfifo 是否有数据
		- 有数据从 kfifo 取一个字节；没有数据返回 0
- dma
	- prepare
		- **配置 `DMASA` 寄存器为 1 ???????? 干什么用的？先清一下？**
	- get_dma_burst_size
		- 获取触发等级
		- 根据触发等级配置对应的 burst_size
	- uart_get_per
		- 根据传入的 port 获取握手信号
	- uart_raw_putc
		- 配置 dma
		- 使用同步 dma
		- 启动 dma，传输长度是 1
	- uart_dma_getc
		- 从 RBR 寄存器读数据到 sram
		- 配置 dma 为异步 dma
		- 启动 dma，传输长度是 1
- brc
	- 上位机连续发送 0xef，小端先发，加上 Start、Stop 位的波形如下： ![[Pasted image 20250212110702.png]]
	- 用于测量串口传输的波特率
	- 等待 brc 检测完成
	- 检测完成计算 uart 传输 5bit 数据的所耗费的周期时间
	- `brc_val1`：第二个下降沿到第三个下降沿的时间，一直发 0xef 就刚好是 5bit 数据的传输时间
	- `brc_val2` ：第二个下降沿到第四个下降沿的时间，一直发 0xef 就刚好是 10bit 的传输时间
	- width = 5bit 传输的周期数
	- 如果 `width <79` 则认为不可用，如果 ` width==79`，就认为耗时 80，否则的话就是正常的值
	- 波特率计算方法：
		- 一个时钟周期：`1/F`
		- width 个周期耗时：`width * 1/F`
		- 但是 width 的周期是 5bit 才占这么久，所以 `width/5*F` 表示 1bit 耗时的时间
		- `baud  = 1/周期`
			- 因为 baud 表示的是每秒传输多少 b 的数据，就是 `f --> = 1/周期`
		- 最后计算 uart 的工作频率为：`Freq = (baud * width)/5` 
		- 因此可以通过上位机一直发 0xef 来得到 uart 的工作频率

# ecos


# linux


## 概念

### 终端、tty 设备、控制台
![[Pasted image 20250213095155.png]]

### tty 框架
对上，向应用程序提供使用终端的统一接口；对下，提供编写终端驱动程序（如 serial driver）的统一框架。



## 驱动

- 如何支持多个串口
	- probe 的时候每次匹配到设备树信息只有就注册一个串口，用 malloc 的方式创建串口相关的数据接口，然后注册到系统中
	- 每来一个就 malloc 一个，这样可以同时支持多个串口
- probe
	- 从设备树获取寄存器信息 regs
	- 从设备数获取中断号 irq


https://zhuanlan.zhihu.com/p/666660181
https://www.cnblogs.com/fuzidage/p/18203864
https://developer.aliyun.com/article/1421624


### 8250


#### 8250_core . c


- serial8250_init
	- 


#### 8250_port. c




#### 8250_dma. c




#### 8250_dw. c






