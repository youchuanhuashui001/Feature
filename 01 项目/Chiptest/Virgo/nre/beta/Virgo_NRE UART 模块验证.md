
## 1. 基本信息

- **Redmine 问题号**: `#408040`
- **芯片型号/项目名称**: `Virgo_NRE`
- **被测模块**: `UART`
- **固件/FPGA 镜像版本**: 
	- a7： #404698 vu9p_fpga_top_sub_a7_20250524. bit test7 a7 核 flash 中 48M
	- a55: #404698 vu9p_fpga_top_sub_a55_20250524. bit test8 a55 核 flash 中 48M
- **测试日期**: `2025-05-12`
- **相关文档**: [[dw_uart ip]]




# 测试项


## 测试 DMA 传输功能
1. 测试步骤：
	- 打开 `dw_uart.c` 开头的 DMA 测试宏：
		```c
		#define ENABLE_UART_DMA_TEST
		```
	- 使用 `getc` 和 `putc` 进行数据传输
		- getc 能够直接使用串口 0 获取数据，使用异步 dma 
		- putc 需要使用串口 1 发送数据，使用同步 dma
	- 观察数据传输是否正常
2. 预期结果：
	- 数据传输正常，能够正常执行 dma 回调函数


## 测试偶校验
1. 测试步骤：
	- 配置串口为偶校验
	- 配置 minicom 为奇校验
	- gdb 将代码加载后停住，在 minicom 中敲入按键
	- 查看寄存器 `LSR[2]  0x14` 是否触发对应错误中断
	- 通过串口发送数据
2. 预期结果：
	- 触发校验错误中断
- 代码修改：
```diff
diff --git a/drivers/serial/dw_uart.c b/drivers/serial/dw_uart.c
index a0def6139..ca74b3616 100644
--- a/drivers/serial/dw_uart.c
+++ b/drivers/serial/dw_uart.c
@@ -191,7 +191,8 @@ static void uart_init_port(int baudrate, int port)
        __raw_writel(0x3, (dw_uartbase + DSW_MCR));
 
        DW_WAIT_IDLE(dw_uartbase, DSW_USR, _usr, DSW_USR_BUSY);
-       __raw_writel(0x03, (dw_uartbase + DSW_LCR));
+//     __raw_writel(0x03, (dw_uartbase + DSW_LCR));
+       __raw_writel(0x03 | (1<<3) | (1<<4), (dw_uartbase + DSW_LCR));
        __raw_writel(0x01, (dw_uartbase + DSW_FCR));
```
```
(gdb) x/x 0xfc880014
0xfc880014:     0x000000e5
```



## 测试发送中断
1. 测试步骤：
	- 打开配置 `ENABLE_IRQ、ENABLE_UART_IRQ`
	- 修改 `uart_init_port` 函数，打开 `tx irq`：
	```c
	__raw_writel(0x03, (dw_uartbase + DSW_IER))
	```
	- 观察系统行为
2. 预期结果：
	- 系统卡死
	- 触发 tx empty 中断（中断寄存器：`0xfc880000+0x8 0xfc880008: 0x000000c2`）

```
(gdb) x/x 0xfc880008
0xfc880008:     0x000000c2
```


## 测试发送硬件 FIFO 大小
- 执行命令 `uarttest 0 tx_fifo_size`
- 注释掉的代码需要打开，并且修改代码：
```diff
-//#define CONFIG_UART_TEST
+#define CONFIG_UART_TEST
 #ifdef CONFIG_UART_TEST
 static int baud[] = {9600, 57600, 115200, 230400, 460800, 500000, 576000, 921600, 1000000, 1152000, 1500000};
 static int uart_do_baudrate_test(int port, int baudrate)
@@ -3272,11 +3272,13 @@ void uarttest(int argc, const char **argv)
                }
                uart_do_baudrate_test(port, baudrate);
        } else if (0 == strcmp("tx_fifo_size", cmd)) {
-#if 0
+#if 1
                int fifo_size = 0;
                volatile int j = 1;
                printf("Start testing tx_fifo_size\n");
                uart_halt_tx(port, 1);
+               extern volatile unsigned int tx_fifo_test;
+               tx_fifo_test = 1;
                while (1) {
                        uart_putc('t', port);
                        if (uart_get_tx_fifo_size(port) > fifo_size)
@@ -3284,8 +3286,9 @@ void uarttest(int argc, const char **argv)
                        else
                                break;
                }
+               tx_fifo_test = 0;
                uart_halt_tx(port, 0);
-               printf("tx_fifo_size : %d\n", fifo_size);
+               printf("\n\ntx_fifo_size : %d\n", fifo_size);

+volatile unsigned int tx_fifo_test = 0;
 
 static inline void uart_raw_putc(int ch, int port)
 {
@@ -517,10 +519,14 @@ static inline void uart_raw_putc(int ch, int port)
 
        p_uart_dev = get_uart_dev(port);
        dw_uartbase = p_uart_dev->reg_base;
+       extern volatile int j;
+
+       if (!tx_fifo_test) {
+               do {
+                       state = __raw_readl(dw_uartbase + DSW_LSR);
+               } while ( !(state & (1 << DSW_LSR_TEMT)) );
+       }
 
-       do {
-               state = __raw_readl(dw_uartbase + DSW_LSR);
-       } while ( !(state & (1 << DSW_LSR_TEMT)) );
```

```
boot> uarttest 0 tx_fifo_size
Start testing tx_fifo_size
ttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt

tx_fifo_size : 128
```

- a7 ok




## 测试多种波特率 
- 在 bootmenu. c 中打开 `CONFIG_UART_TEST` 宏
- 选择对应的串口切换不同的波特率进行测试，minicom 也需要对应的切换配置
```
boot> uarttest 0 baudrate 115200
Test Baudrate : 115200, Press Enter to start
starting testing... Press 'c' to cancel
i can speak english
i can speak english
i can speak english
boot> uarttest 0 baudrate 9600
Test Baudrate : 9600, Press Enter to start
i can speak english Press 'c' to cancel
i can speak english
i can speak english
```

- 串口 1：
	- 9600
	- 57600
	- 115200
	- 230400
	- 460800：字符可以正常，但是空格有时候打不出来，可能波特率偏差有点大，开个小数波特率试试
		- 打开小数波特率后正常
	- 500000
	- 576000：字符显示不正常
		- 打开小数波特率后正常
	- 921600：字符显示不正常
		- 打开小数波特率后正常
	- 1152000：字符显示不正常
		- 打开小数波特率后正常
	- 1500000
- 串口 0：
	- 9600
	- 57600
	- 115200
	- 230400
	- 460800：空格有时打不出来
	- 500000
	- 576000：字符显示不正常
	- 921600：字符显示不正常
	- 1152000：字符显示不正常
	- 1500000

- 测试结果：
	- a7 ok
	- a55 ok



## 测试奇校验
1. 测试步骤：
	- 配置串口为奇校验
	- 配置 minicom 为偶校验
	- gdb 加载代码，然后 gdb 停住，在 minicom 中按下按键
	- 查看 `LSR[2]  0x14` 寄存器是否触发错误中断状态
- 代码修改：
```diff
diff --git a/drivers/serial/dw_uart.c b/drivers/serial/dw_uart.c
index a0def6139..ae4ba27ee 100644
--- a/drivers/serial/dw_uart.c
+++ b/drivers/serial/dw_uart.c
@@ -191,7 +191,8 @@ static void uart_init_port(int baudrate, int port)
        __raw_writel(0x3, (dw_uartbase + DSW_MCR));
 
        DW_WAIT_IDLE(dw_uartbase, DSW_USR, _usr, DSW_USR_BUSY);
-       __raw_writel(0x03, (dw_uartbase + DSW_LCR));
+       __raw_writel(0x03 | (1<<3), (dw_uartbase + DSW_LCR));
        __raw_writel(0x01, (dw_uartbase + DSW_FCR));
```

```
(gdb) x/x 0xfc880014
0xfc880014:     0x000000e5
```




## 测试小数波特率
- 测试多种波特率时，打开了小数波特率后原本发送不正常的字符变得正常了，可以说明问题





## 测试接收中断
1. 测试步骤：
	- 打开配置 `ENABLE_IRQ、ENABLE_UART_IRQ`
	- 在 `uart_interrupt_isr` 中添加打印
	- 通过 minicom 向串口输入数据
2. 预期结果：
	- 触发接收中断，bit2
	- 中断处理函数正常执行
```
(gdb) x/x 0xfc880008
0xfc880008:     0x000000cc
```


## 测试接收硬件 FIFO 大小
- 执行 `uarttest 0 rx_fifo_size`，一直往 minicom 输入数据，实时打印当前 rx fifo 的数据个数
- gdb 查看寄存器：0xfc880000，每查看一次，少一个数据
```
rx_fifo_size : 128
rx_fifo_size : 128
```




## 测试接收硬件 FIFO 溢出检测
- 测试步骤：
	- gdb 加载程序后跑起来，然后停住
	- 在 minicom 中一直输入数据，填满 uart rx fifo 直至溢出
	- 查看寄存器 `LSR.OE bit1 0x14` 是否为 1
```
(gdb) x/x 0xfc880014
0xfc880014:     0x00000063
```

## 测试硬件流控 CTS
- 测试步骤：
	- 敲命令 `uarttest 1 badurate 115200` 初始化待测串口
	- 敲命令 `uarttest 1 flow_cts` 开启流控
	- CTS 接 3.3V：验证无输出
	- CTS 接 GND：验证正常输出，一直打印 `A`
2. 注意事项：
	- 不能直接敲 `uarttest 1 flow_cts`
	- 需要先敲 `uarttest 1 baudrate 115200` 初始化待测串口
	- 如果配置了待测试串口为终端，可以直接敲 `uarttest 1 flow_cts`
	- 串口小板对应的 IO 需要拨到 INPUT；`GPIO10` 是 CTS 引脚




## 测试硬件流控 RTS
1. 测试命令：
```c
uarttest 1 baudrate 115200
uarttest 1 flow_rts
```
2. 测试步骤：
	- 使能流控后使用 GDB 暂停程序，不要用 `uart_getc` 读数据
	- 持续通过 minicom 输入数据到待测串口
	- 观察 RTS 引脚电平变化
		- 如果 rxfifo 接近满的时候 (剩余 2 个字节)，rts 会拉高
		- 将 rxfifo 中的数据取出 (剩余超过 2 个字节)，rts 会恢复低
3. 注意事项：
	- 小板 IO 需配置为输出模式
	- 使用 GDB 时，b 到 while 里面
	- 通过 minicom 持续写数据
	- 观察 RTS 电平变化



## 测试波特率检测功能
- 测试步骤：
	- `bootmenu.c` 打开 `#define CONFIG_UART_TEST`
	- `bootmenu. c` 和 `dw_uart.c` 都要定义 `#define CONFIG_UART_BRC`
	- 启动 Python 脚本持续发送 0xef：`python3 serial_send.py -b 115200 -p /dev/ttyUSB1`
	- 启动串口 `uart_brc_test 1`
	- 观察检测结果
-  预期结果：
	- 检测到正确的时钟频率（约 24MHz）
```shell
-> % python3 serial_send.py -b 115200 -p /dev/ttyUSB1
成功打开串口 /dev/ttyUSB1，波特率: 115200
开始持续发送字节: 0xef

boot> uart_brc_test 1
freq = 24030720
```
```diff
--- a/conf/virgo/fpga/debug.config
+++ b/conf/virgo/fpga/debug.config
 CONFIG_UART_PORT = 0
 CONFIG_UART_BAUDRATE = 115200
 CONFIG_UART_TYPE = DW
+CONFIG_UART_BRC = y
```




## 测试长时间收发
- 测试步骤：
	- `bootmenu.c` 打开 `#define CONFIG_UART_TEST`
	- 打补丁：`https://git.nationalchip.com/gerrit/#/c/119227/2/tools/serial_test.py`
	- 先启动串口：`uarttest 1 stability 115200`
	- 再启动测试脚本：`python3 serial_test.py /dev/ttyUSB1 115200`
	- 观察测试脚本打印情况




## 测试波特率和实际波特率偏差
1. 测试步骤：
	- 使用示波器测量实际波特率
2. 结果：
	- 逻辑分析仪抓到 115200 时每位数据为 8.66 微秒，计算值为 8.68 微秒，基本一致




# 问题
## 【已解决】问题 1：uart1 发不出数据，也收不到数据
- nre a7、a55 都不通
- 看得到 uart1 寄存器，但看不到输出的波形
- 用老的 bit 看下，硬件电路是不是好的
	- final 测试时的 bit 是好的
- uart 发数据的流程：
	- 读 LSR 寄存器，循环等待 tx fifo empty
	- 将数据写到 THR 寄存器
- 第一次读 LSR 有 tx fifo empty，数据写到 THR 寄存器后，第二次再去读 LSR 寄存器，读不到 tx fifo empty


- 芯片没有给 uart1 时钟


## 问题 2：uart 接收中断触发不了
查看代码发现是由于 kfifo 无法 malloc，在初始化 heap 之前就调用了 malloc 函数，自然失败。
问下妙兵为什么把 uart 放到这么前面去初始化？
- 仔细分析 develop 分支的代码如何实现的
- 考虑是否需要添加一个 early_uart_init 接口来专门用作早期的 printf



## 问题 3：BRC 功能如何实现的？
查看文档描述 BRC 功能用于测量 rxd 线上波特率，但驱动实现的是检测线上模块频率
![[Pasted image 20250528134409.png]]
![[Pasted image 20250528134425.png]]
![[Pasted image 20250528134445.png]]
![[Pasted image 20250528134458.png]]
![[Pasted image 20250528134511.png]]
![[Pasted image 20250528134521.png]]
![[Pasted image 20250528134531.png]]

- 保持疑问：芯片设计的是可以测量 rxd 线上波特率
	- 驱动实现的确是根据 rxd 线上固定波特率检测出模块的频率
