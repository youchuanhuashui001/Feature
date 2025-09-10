---
tags:
  - Virgo_MPW模块验证
---
# 验证进度：
- 总验证点:: 15
- 已完成:: 15
- 备注:: 确认下 linux 如何测试


# 修改点
- 时钟：24MHz，没有用pll
- 引脚：


# 验证项

## 测试DMA传输功能
- UART0
	- tx、rx ok
- UART1
	- tx ok
	- rx ok
- UART2
	- tx ok
	- rx ok


## 测试偶检验
- UART0 ok
- UART1 ok
- UART2 ok


## 测试发送中断
- UART0:bit5、bit6
```
(gdb) x/x 0xfc880014
0xfc880014:     0x00000061
```
- UART1：bit5、bit6
```
state:0x60
```
- UART2：bit5、bit6
```
state:0x60
```



## 测试发送硬件 FIFO 大小
- UART0：128
- UART1：128
- UART2：128


## 测试多种波特率
- UART0:
	- 9600
	- 57600
	- 115200
	- 230400
	- 460800
	- 500000
	- 576000
	- 921600
		- 乱码
	- 1000000
		- 乱码
- UART1
	- 9600
	- 57600
	- 115200
	- 230400
	- 460800
		- 乱码
	- 500000
	- 576000
		- 乱码
	- 921600
		- 乱码
	- 1000000
		- 乱码
- UART2
	- 9600
	- 57600
	- 115200
	- 230400
	- 500000
	- 576000
	- 921600
		- 乱码
	- 1000000


## 测试奇校验
- UART0 测试ok
- UART1 测试ok
- UART2 测试 ok
```
(gdb) x/x 0xfc8a0014
0xfc8a0014:     0x000000e5
```


## 测试小数波特率
- UART0
	- 576000
		- 无乱码
	- 921600
		- 无乱码
- UART1
	- 576000
		- 无乱码
	- 921600
		- 无乱码
- UART2
	- 576000
		- 无乱码
	- 921600
		- 无乱码

## 测试接收中断
- UART0:ok
- UART1:ok
- UART2:ok
```
state:0x61
```

## 测试接收硬件 FIFO 大小
- UART0：128
- UART1：128
- UART2：128


## 测试接收硬件 FIFO 溢出检测
- UART0
```
(gdb) x/x 0xfc880014
0xfc880014:     0x00000061
(gdb) x/x 0xfc880084
0xfc880084:     0x0000006c
(gdb) x/x 0xfc880084
0xfc880084:     0x00000078
(gdb) x/x 0xfc880084
0xfc880084:     0x00000080
(gdb) x/x 0xfc880014
0xfc880014:     0x00000063
```
- UART1
```
(gdb) x/x 0xfc890014
0xfc890014:     0x00000063
```
- UART2
```
(gdb) x/x 0xfc8a0014
0xfc8a0014:     0x00000063
```


## 测试硬件流控 CTS
- UART0 没有
- UART1 ok
- UART2 ok


## 测试硬件流控 RTS
- UART0 没有
- UART1 ok
- UART2 ok

## 测试波特率检测功能
- 测试ok



## 测试串口长时间收发
- UART0 测试ok
- UART1 测试ok
- UART2 测试ok




## 测试波特率和实际波特率偏差
- UART0 开启小数波特率
	- 逻辑分析仪抓到 115200 时每位数据为 8.66 微秒，计算值为 8.66 微秒，测试一致
- UART1 开启小数波特率
	- 逻辑分析仪抓到 115200 时每位数据为 8.66 微秒，计算值为 8.66 微秒，测试一致
- UART2 开启小数波特率
	- 逻辑分析仪抓到 115200 时每位数据为 8.66 微秒，计算值为 8.66 微秒，测试一致



# 问题

## 【已解决】UART1 不通
- RX 测试时缺少电阻：R144
- TX 看电路图应该也缺少 R159
- RTS 缺少电阻：R102
- CTS 缺少电阻：R121 

- RX  对应 SPI1_MST_SCLK_IN
- TX  对应 SS_0
- RTSn 对应 SS_1
- CTSn 对应 SS_2

- GPIO_E02
- GPIO_E03 需要拉出来



- 注意：默认 UART1 RX、TX 已经引出来了，不需要额外再拉出来



## 【已解决】UART1 HTX 寄存器无法写入
```
(gdb) x/x 0xfc8900a4
0xfc8900a4:     0x00000000
(gdb) set *0xfc8900a4=0x1
(gdb) x/x 0xfc8900a4
0xfc8900a4:     0x00000000
(gdb) x/x 0xfc8800a4
0xfc8800a4:     0x00000000
(gdb) set *0xfc8800a4=0x1
(gdb) x/x 0xfc8800a4
0xfc8800a4:     0x00000001
(gdb) x/x 0xfc8900a4
0xfc8900a4:     0x00000000
(gdb) set *0xfc8900a4=0x1
(gdb) x/x 0xfc8900a4
0xfc8900a4:     0x00000000
```

- 问题原因：没有初始化 uart1，初始化之后可以写入