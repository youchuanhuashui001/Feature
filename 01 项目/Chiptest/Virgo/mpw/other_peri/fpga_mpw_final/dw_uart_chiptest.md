# DW UART 芯片测试文档

## 1. 测试环境
- FPGA 版本：`382541 9p_peri_newcode_FPGA_2024-09-03.tar`
- VU9P


## 2. 测试用例

### 2.1 DMA 传输功能测试
1. 测试步骤：
	- 打开 `dw_uart.c` 开头的 DMA 测试宏：
		```c
		#define ENABLE_UART_DMA_TEST
		```
	- 使用 `getc` 和 `putc` 进行数据传输
	- 观察数据传输是否正常
2. 预期结果：
	- 数据传输正常，能够正常执行 dma 回调函数

> [!note]
>  - 不能用 `382541 9p_peri_newcode_FPGA_2024-09-03.tar` 
>  - 需要用 `394932 peri_final3_gpio_FPGA_2024-12-03.tar`


### 2.2 偶校验测试 (没测过)
1. 测试步骤：
	- 配置串口为偶校验
	- 配置 minicom 为奇校验
	- 通过串口发送数据
2. 预期结果：
	- 触发校验错误中断
3. 当前问题：
	- 没有驱动和测试代码
	- 需要补充测试代码

### 2.3 发送中断测试
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

### 2.4 发送硬件 FIFO 大小测试 (没测过)
1. 测试命令：
	```bash
	uarttest 1 tx_fifo_size
	```
2. 当前问题：
	- 代码会一直卡在等 tx fifo empty 状态
	- 代码被注释

### 2.5 多种波特率测试
1. 测试步骤：
	```shell
	uarttest 0 baudrate 115200  # 测试本串口
	uarttest 1 baudrate [baud]  # 测试其他串口
	```
	- 初始化串口为指定波特率
	- 发送测试字符串：`i can speek english\r\n`
	- 每次发送后延时 1s
	- 输入 'c' 结束测试
2. 注意事项：
	- 需要用 `382541 9p_peri_newcode_FPGA_2024-09-03.tar`
	- minicom 打开时需要指定波特率

### 2.6 奇校验测试 (没测过)
1. 测试步骤：
	- 配置串口为奇校验
	- 配置 minicom 为偶校验
	- 通过串口发送数据
2. 当前问题：
	- 没有驱动和测试代码
	- 配置后没有出现对应的中断 `LSR[2] 0x14`

### 2.7 小数波特率测试
1. 测试步骤：
	- 随意通信
	- 使用示波器测量数据位周期
2. 预期结果：
	- 115200 波特率：周期约 8.68us
	- 921600 波特率：周期为 1.085us
		- 开启小数波特率：周期约 1.085us
		- 关闭小数波特率：周期约 1.334us
	- 921600 时 `_dlf` 配置为 `0xa`
- 代码修改：
```diff
--- a/drivers/serial/dw_uart.c
+++ b/drivers/serial/dw_uart.c
@@ -132,8 +132,8 @@ struct uart_device uart_dev[] = {
 
 #if defined(CONFIG_ARCH_ARM_CANOPUS) || defined(CONFIG_ARCH_ARM_VEGA) || defined(CONFIG_ARCH_CKMMU_CYGNUS) || \
     defined(CONFIG_ARCH_ARM_VIRGO)
-//#define DW_UART_FRACTIONAL_BAUD_DIV
-//#define UART_DLF_SIZE 4
+#define DW_UART_FRACTIONAL_BAUD_DIV
+#define UART_DLF_SIZE 4
```


### 2.8 接收中断测试
1. 测试步骤：
	- 打开配置 `ENABLE_IRQ、ENABLE_UART_IRQ`
	- 在 `uart_interrupt_isr` 中添加打印
	- 通过 minicom 向串口输入数据
2. 预期结果：
	- 触发接收中断
	- 中断处理函数正常执行

### 2.9 接收硬件 FIFO 大小测试
1. 测试步骤：
	- 关闭中断
	- `uarttest 0 rx_fifo_size`
	- 持续在 minicom 敲下任意按键
	- 观察 FIFO 数据量
2. 注意事项：
	- 不能打开中断，因为中断会一直从 fifo 取数据
	- 手动读 `x/x 0xfc880000`，读一次少一个

### 2.10 接收硬件 FIFO 溢出检测测试 (没测过)
1. 测试步骤：
	- 填满接收 FIFO
	- 继续输入数据
2. 当前问题：
	- 无法触发溢出中断
	- `LSR[1]` 位始终为 0
	- FIFO 中有 128 个数据，但未触发溢出

### 2.11 硬件流控 CTS 测试
- 测试步骤：
	- 敲命令 `uarttest 1 badurate 115200` 初始化待测串口
	- 敲命令 `uarttest 1 flow_cts` 开启流控
	- CTS 接 3.3V：验证无输出
	- CTS 接 GND：验证正常输出，一直打印 `A`
2. 注意事项：
	- 不能直接敲 `uarttest 1 flow_cts`
	- 需要先敲 `uarttest 1 baudrate 115200` 初始化待测串口
	- 如果配置了待测试串口为终端，可以直接敲 `uarttest 1 flow_cts`
	- 串口小板对应的 IO 需要拨到 OUTPUT；`GPIO01` 是 CTS 引脚

- 测试：实际测试需要把 GPIO00、GPIO01 都接到 GND。拨到 output
	- 如果发一会 `A` 不发了，重新拨一下拨码，拨到 input，再拨到 output，应该是 rts 脚被拉高了吧？
	- 是的，确实是 rts 被拉高了，由于 gpio 是单向的，这里接地 cpu 也收不到，所以需要切成 input，来恢复 rts 拉低让 cpu 收到，就可以正常通信了
	- 查状态的时候需要切成 input


### 2.12 硬件流控 RTS 测试
1. 测试命令：
```c
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

### 2.13 波特率检测功能测试
- 测试步骤：
	- `bootmenu.c` 打开 `#define CONFIG_UART_TEST`
	- `bootmenu. c` 和 `dw_uart.c` 都要定义 `#define CONFIG_UART_BRC`
	- 启动 Python 脚本持续发送 0xef：`python3 serial_send.py -b 115200 -p /dev/ttyUSB0`
	- 启动串口 `uart_brc_test 1`
	- 观察检测结果
-  预期结果：
	- 检测到正确的时钟频率（约 24MHz）
- 代码修改：
```diff
--- a/bootmenu.c
+++ b/bootmenu.c
@@ -3110,7 +3110,7 @@ i2c_auto_test_exit:
 }
 #endif
 
-//#define CONFIG_UART_TEST
+#define CONFIG_UART_TEST
 #ifdef CONFIG_UART_TEST
 static int baud[] = {9600, 57600, 115200, 230400, 460800, 500000, 576000, 921600, 1000000, 1152000, 1500000};
 static int uart_do_baudrate_test(int port, int baudrate)
@@ -3279,6 +3279,7 @@ void uarttest(int argc, const char **argv)
        }
 }
 
+#define CONFIG_UART_BRC
 #ifdef CONFIG_UART_BRC

--- a/drivers/serial/dw_uart.c
+++ b/drivers/serial/dw_uart.c
@@ -7,6 +7,7 @@
 #include "uart_core.h"
 #include <driver/gx_dma_axi.h>
 
+#define CONFIG_UART_BRC
 //#define ENABLE_UART_DMA_TEST
```

### 2.14 串口长时间收发测试
- 测试步骤：
	- `bootmenu.c` 打开 `#define CONFIG_UART_TEST`
	- 打补丁：`https://git.nationalchip.com/gerrit/#/c/119227/2/tools/serial_test.py`
	- 先启动串口：`uarttest 1 stability 115200`
	- 再启动测试脚本：`python3 serial_send.py -b 115200 -p /dev/ttyUSB1`
	- 观察测试脚本打印情况
-  注意事项：
	- 测试前关闭对应测试串口
	- 使用提供的 Python 脚本进行测试
	- 脚本地址：`https://git.nationalchip.com/gerrit/#/c/119227/2/tools/serial_test.py`

### 2.15 波特率和实际波特率偏差测试
1. 测试步骤：
	- 使用示波器测量实际波特率
2. 预期结果：
	- 实际波特率与配置波特率基本一致

## 3. 测试结果
- [x] DMA 传输功能
- [ ] 偶校验测试
- [x] 发送中断测试
- [ ] 发送硬件 FIFO 大小测试
- [x] 多种波特率测试
- [ ] 奇校验测试
- [x] 小数波特率测试
- [x] 接收中断测试
- [x] 接收硬件 FIFO 大小测试
- [ ] 接收硬件 FIFO 溢出检测测试
- [x] 硬件流控 CTS 测试
- [x] 硬件流控 RTS 测试
- [x] 波特率检测功能测试
- [x] 串口长时间收发测试
- [x] 波特率和实际波特率偏差测试

## 4 测试记录

### 测试用例和测试代码合并
- brc 测试需要用到 python 脚本，使用波特率 115200 一直发 0xef
- 串口稳定性测试，需要 python 脚本，bootmenu. c 中也需要对应修改

### 测试 brc 用的 python 脚本：一直发 0xef
```python
#!/usr/bin/env python3
import serial
import time
import argparse

def send_byte(port_name, baud_rate, byte_value, interval=0.01):
    """
    向指定串口持续发送特定字节
    
    参数:
    port_name -- 串口名称
    baud_rate -- 波特率
    byte_value -- 要发送的字节值(十六进制)
    interval -- 发送间隔时间(秒)
    """
    try:
        # 打开串口连接
        ser = serial.Serial(
            port=port_name,
            baudrate=baud_rate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=1
        )
        
        print(f"成功打开串口 {port_name}，波特率: {baud_rate}")
        print(f"开始持续发送字节: 0x{byte_value:02x}")
        
        # 创建要发送的单字节数据
        data_byte = bytes([byte_value])
        
        try:
            # 持续发送字节
            while True:
                ser.write(data_byte)
                ser.flush()  # 确保数据被写出
#                time.sleep(interval)  # 控制发送频率
                
        except KeyboardInterrupt:
            # 处理用户中断(Ctrl+C)
            print("\n用户中断，停止发送")
        
        finally:
            # 确保关闭串口
            if ser.is_open:
                ser.close()
                print(f"串口 {port_name} 已关闭")
    
    except serial.SerialException as e:
        print(f"串口错误: {e}")
        
if __name__ == "__main__":
    # 设置命令行参数
    parser = argparse.ArgumentParser(description='向串口持续发送特定字节')
    parser.add_argument('-p', '--port', type=str, default='/dev/ttyUSB0',
                      help='串口设备名 (默认: /dev/ttyUSB0)')
    parser.add_argument('-b', '--baud', type=int, default=9600,
                      help='波特率 (默认: 9600)')
    parser.add_argument('-v', '--value', type=lambda x: int(x, 0), default=0xef,
                      help='要发送的字节值，十六进制 (默认: 0xef)')
    parser.add_argument('-i', '--interval', type=float, default=0.01,
                      help='发送间隔时间，单位秒 (默认: 0.01)')
    
    args = parser.parse_args()
    
    # 调用函数开始发送
    send_byte(args.port, args.baud, args.value, args.interval)
```
