# 寄存器分析
[[dw_uart ip]]


# 驱动分析


## 测试 case 和测试用例分析：

## gxloader

### 测试用例：
- 测试 DMA 传输功能：
	- dw_uart. c 文件开头的 `ENABLE_UART_DMA_TEST` 宏打开
	- `getc、putc` 都会使用 dma
- 测试奇/偶校验：
	- #virgo_q 执行测试步骤 4？没地方写
	- 没有测试代码
- 测试发送中断：
	- #virgo_q 代码里面中断相关用的都是 rx 啊
- 测试发送硬件 FIFO 大小：
	- #virgo_q 测试代码为什么注释掉了？
	- `uarttest tx_fifo_size`
- 测试多种波特率：
	- 打开两个串口，一个是交互串口，一个是待测试串口
	- 交互串口用于启动，停止测试
	- 待测试串口用于修改不同的波特率来查看打印是否正确
- 测试小数波特率：
	- `dw_uart.c` 中定义了 `DW_UART_FRACTIONAL_BAUD_DIV`，打开后就会使用小数波特率
	- #virgo_q 怎么判断小数波特率是不是更准？
- 测试接收中断：
	- 打开 `CONFIG_ENABLE_IRQ、CONFIG_ENABLE_UART_IRQ`
- 测试接收硬件 FIFO 大小：
	- `uarttest rx_fifo_size`
- 测试接收硬件 FIFO 溢出检测：
	- #virgo_q 怎么伪造出这个中断？
- 测试硬件流控 CTS：
	- 配置待测试串口为终端
	- CTS 管脚接到 3.3V 上时，确认串口是否能正常输出  
	- CTS 管脚接到 GND 上时，确认串口是否能正常输出
- 测试硬件流控 RTS：
	- 配置待测试串口为终端
	- 通过 GDB 加载程序，程序启动后将程序暂停运行 (CTRL + C)  
	- 确认此时 RTS 管脚的电平  
	- PC 端向控制台持续输入字符  
	- 当 RX FIFO 接近满的时候 (剩余 2 个字节)，确认 RTS 管脚的电平  
	- 将 RX FIFO 中的数据取出 (剩余超过 2 个字节)，确认 RTS 管脚的电平
- 测试波特率检测功能：
	- `uart_brc_test [port]`
- 测试串口长时间收发：
	- 没找到测试代码
- 测试波特率和实际波特率偏差：
	- `uart_brc_test [port]`


### 测试代码：
- baudrate：测试不同的波特率是否可正常工作
	- 测试波特率范围从 9600 ~ 1500000
	- 显示当前要测试的波特率，等待用户按 Enter 开始
	- 将要测试的串口初始化为指定的波特率
	- 循环发送测试字符串：`i can speek english\r\n`
	- 每次发送后延时 1s
	- 当用户在串口输入 `c` 表示结束测试
	- 结束测试前将待测试的串口波特率配置为 115200
- tx_fifo_size: 测试串口 tx fifo 缓冲区的实际大小
	- 停止 tx fifo，不要传输数据
	- 往 tx fifo 持续填充字符
	- 获取 tx fifo 中的数据量，直到 fifo 中的数量达到最大容量
	- 恢复 tx fifo，可以传输数据
	- #virgo_q : 这段代码为什么注释掉了？
- rx_fifo_size: 测试串口 rx fifo 缓冲区的实际大小
	- 实时监测串口 rx fifo 的使用情况
	- #virgo_q ：哪里往 fifo 放数据？
- flow_cts: 测试串口 cts 硬件流控
	- cts 用于告知发送方是否可以发送数据；接收方准备好接收数据时，会将 cts 信号置为有效；否则置为无效；
	- 计数器初始化为 0
	- 启动串口的流控
	- 持续发送字符 `A`，每 16 个字符发一个换行符
	- 我这边作为主机一直发一直发，从机没准备好处理数据，会拉低 cts
	- #virgo_q ：这里是要看下 cts 的波形就行了？从机是独立的小板吗？
- flow_rts:
	- rts 是接收方用来控制发送方的信号；接收方准备好接收数据时，会将 RTS 置为有效；否则无效
	- 启用流控
	- 循环接收 128 个字符
	- 每接收一个字符后延时 2s
	- #virgo_q ：这里也是看下 rts 的波形？应该还有另一块小板？这里是模拟接收方来不及接收数据的情况？让 rts 有效？
- uart_brc_test: 用来测试串口波特率
	- 启动 brc 测试
	- 如果检测到了，计算频率
	- #virgo_q ：另一块板子一直发 0xef






## ecos

## linux







# 测试用例
- [x]  测试DMA传输功能
	- 发送 ok，接收 ok
	- 不能用 `382541 9p_peri_newcode_FPGA_2024-09-03.tar`，这个可能当时 dma 有问题
	- 需要用 `394932 peri_final3_gpio_FPGA_2024-12-03.tar`
- [ ]  测试偶检验
	- 没有驱动、没有测试代码
	- 没测过
- [x]  测试发送中断
	- 驱动里只有接收中断，没有发送中断
	- 使能 tx irq，再看寄存器
	- 打开 `ENABLE_IRQ、ENABLE_UART_IRQ`，代码中 `uart_init_port` 配置 `__raw_writel(0x03, (dw_uartbase + DSW_IER))` 使能 tx irq
	- 系统起来之后就卡死了，看中断寄存器 `x/x 0xfc880000+0x8 0xfc880008:     0x000000c2` 触发了 `tx empty` 中断  
- [ ]  测试发送硬件 FIFO 大小
	- `uarttest 1 tx_fifo_size` 代码会一直卡在等 tx fifo empty 状态
	- 当时没测过
- [x]  测试多种波特率
	- ok，引脚复用的问题
	- 需要用 `382541 9p_peri_newcode_FPGA_2024-09-03.tar`
	- 测试本串口：`uarttest 0 baudrate 115200`
	- 测试其它串口：`uarttest 1 baudrate [baud]`，其它串口打开时需要指定波特率
- [ ]  测试奇校验
	- 没有驱动、没有测试代码
	- 串口配成偶校验或者奇校验，minicom 配成相反的，交互，看看有没有对应的中断
	- `__raw_writel(0x03|(1<<3)|(1<<4), (dw_uartbase + DSW_LCR));` 然后串口一直敲乱数据，并**没有出现对应的中断 LSR[2] 0x14**
	- 没测过
- [x]  测试小数波特率
	- 需要用示波器看一下
	- 例如用 115200 波特率，波特率 `115200 bps` 计算，单个数据位的周期为 `1/115200 大概 8.68us`
	- 921600 的周期大概是 `1.085us`，开小数波特率量很准，不开的话误差很大 `(1.334us)`
	- 921600 的时候 `_dlf` 配的是 `0xa`
- [x]  测试接收中断
	- `.config` 中打开 `ENABLE_IRQ、ENABLE_UART_IRQ`
	- `dw_uart.c` 中在 `uart_interrupt_isr` 函数中添加打印，启动后敲回车就能触发接收中断
- [x]  测试接收硬件 FIFO 大小
	- 不能打开中断，因为中断会一直从 fifo 取数据
	- `uarttest 0 rx_fifo_size`
	- 手动敲按键，向串口发数据，由于没有从 uart fifo 中取数据，所以 fifo 中的数据一直在
	- 手动读 `x/x 0xfc880000`，读一次少一个
- [ ]  测试接收硬件 FIFO 溢出检测
	- `LSR.[1]` 代表 fifo 溢出，怎么测？
	- `rx fifo size` 填满之后，再填，就会触发
	- 没测过
	- **一直填，没有触发 fifo 溢出，看 fifo 中有 128 个数据，但是 `lsr[1]` 一直为 0**
- [x]  测试硬件流控 CTS
	- 配置待测试串口为终端  
	- `382541 9p_peri_newcode_FPGA_2024-09-03.tar` 测试可以，不能直接敲 `uarttest 1 flow_cts`，还需要先敲 `uarttest 1 baudrate 115200` 来初始化待测串口
	- 如果配置了待测试串口为终端，应该可以直接敲 `uarttest 1 flow_cts`
	- CTS 管脚接到 3.3V 上时，确认串口是否能正常输出  
		- 没输出
	- CTS 管脚接到 GND 上时，确认串口是否能正常输出
		- 有输出
- [x]  测试硬件流控 RTS
	- 配置待测试串口为终端
		- **`.config` 中配了之后起不来了**
	- 通过 GDB 加载程序，程序启动后将程序暂停运行(CTRL + C)
	- 确认此时 RTS 管脚的电平
	- PC 端向控制台持续输入字符
		- **这里应该是有个脚本吧？**
	- 当 RX FIFO 接近满的时候(剩余 2 个字节)，确认 RTS 管脚的电平
	- 将 RX FIFO 中的数据取出(剩余超过 2个字节)，确认 RTS 管脚的电平
	- ![[Pasted image 20250314102420.png]]
	- gdb 起来之后 b 到 while 里面，上面 minicom 一直写数据给串口，但是这边不去收，就会有 rts 拉高，然后 gdb 继续跑，从 fifo 收数据，这里 rts 就会变低了
	- **rts 脚没变化，一直都是低电平**
	- 小板上的 io 需要切成 output，不能拨到 input 那边
	- 可以了
- [x]  测试波特率检测功能
	- 这里应该有脚本跑在上位机一直发 0xef 吧？
	- python 写一个一直发 0xef 的波特率，这里检测的是发 0xef 的波特率
	- **我让 gxapt 写了一个，但是测出来不准**
	- brc 测出来的是 `时钟频率`，24M 左右，正常的
- [x]  测试串口长时间收发
	- 怎么测？
	- 有个补丁：https://git.nationalchip.com/gerrit/#/c/119227/2/tools/serial_test.py
	- 串口先启动：`uarttest 1 stability 115200`
	- 脚本再启动：`python3 serial_send.py -b 1500000 -p /dev/ttyUSB1`
	- 不能打开对应的测试串口
- [x]  测试波特率和实际波特率偏差
	- 用示波器看，基本一致





# Q&&A
## Q：27MHz 的晶振和 24MHz 的晶振对于打印有什么影响？