



# 测试项
## 测试多种波特率
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


# 问题
## 问题 1：uart1 发不出数据，也收不到数据
- nre a7、a55 都不通
- 看得到 uart1 寄存器，但看不到输出的波形
- 用老的 bit 看下，硬件电路是不是好的
	- final 测试时的 bit 是好的
- uart 发数据的流程：
	- 读 LSR 寄存器，循环等待 tx fifo empty
	- 将数据写到 THR 寄存器
- 第一次读 LSR 有 tx fifo empty，数据写到 THR 寄存器后，第二次再去读 LSR 寄存器，读不到 tx fifo empty
