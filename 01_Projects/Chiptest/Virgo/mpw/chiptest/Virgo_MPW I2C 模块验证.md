---
tags:
  - Virgo_MPW模块验证
---
# 验证进度：
- 总验证点:: 11
- 已完成:: 10
- 备注:: 高速模式 3.4MHz 未提供硬件，无法测试




# 测试修改项
- 引脚复用：
	- I2C：GPIO_D10 -- I2C0_SCL   GPIO_D11  -- I2C0_SDA
- 模块频率：99MHz
	- 内部不需要进行分频





# 测试项

## 测试 master 快速模式 3.4MHz 读写



## 测试 DMA 传输功能
- 使用 hal i2c 那套驱动，打开 IRQ
- 能够触发 i2c dma tx rx 回调
- 所有 i2c 测试 ok


## 测试 RX_FULL 中断
- 使用 hal i2c 那套驱动，打开 `.config` 中的 `IRQ`，开头添加 `#undef CONFIG_ENABLE_AXI_DMA` 
- 能够触发中断
- 所有 i2c 测试 ok


## 测试 TX_EMPTY 中断
- 使用 hal i2c 那套驱动，打开 `.config` 中的 `IRQ`，开头添加 `#undef CONFIG_ENABLE_AXI_DMA` 
- 能够触发中断
-  所有 i2c 测试 ok


## 测试 master 写
- 对接 i2c 小板，测试通过


## 测试 master 快速模式 400kHz 读写
- 对接 i2c 小板，测试通过
- 所有 i2c 测试 ok



## 测试 master 标准模式 100kHz 读写
- 对接 i2c 小板，测试通过
- 所有 i2c 测试 ok



## 测试 master 读
- 对接 i2c 小板，测试通过


## 测试时钟延展
- 对接 i2c 小板，测试通过


## 测试配置 i2c 时序
- 配置为 400kHz，修改 high 为 1550，low 为 950
	- 测得 high 约为 1550 ns，low 约为 950ns


## 测试 master 读写多个 slave
- 对接 i2c 小板，测试通过




# 问题
## 【已解决】I2C 不通
### 修改了引脚复用，模块频率，使用命令：`i2cget -y 0 0x23 0x34 2 -r 1` 进行测试，逻辑分析仪抓不到波形
- 查看原理图，有些电阻没有焊接
	- 硬件补焊后，I2C1 通了，但是 I2C2、I2C3 还是不通
	- I2C3 的硬件电路上很干净，但是仅上电后 SCL 脚可以到 3.3v，但 SDA 脚只能到 2v 左右，发出的波形 SCL 脚也可以从 3.3 拉到 0，但是 SDA 只能从 2 拉到 1.7 左右
	- 默认驱动能力是最弱的，将驱动能力调到最强之后，SCL 正常了，但是 SDA 还是只能从 2v 到 0.6v 左右
	- 一个引脚会复用多种功能，去掉 i2c 另外一边的电阻后，测试正常了，也不需要修改驱动强度
- I2C0 量下来电压有 4v，原理图中是 5v 供电，但 I2C 没有用 5v 供电的要求
	- 测试 i2c 的时候先把供电电压调成 3.3
	- 到时候测试 ir 的时候再把供电电压调成 5
		- 不需要调成 5v，3.3v 也可以工作只是可能接收范围变小了
		- 当时设计的时候是因为原来 i2c 和 ir 都是 5v 的，i2c 是耐 5v 引脚

结论：
- 需要补焊必须的电阻
- 对于 SDA 拉不上去的板子，需要去掉复用功能的电阻


测试流程：
- 一直死循环发 i2c 的波形，不管有没有应答，i2c slave 的地址总会发出去的
- 查看波形是否正常，看看电压，看看波形对不对，把外面复用的其它东西都去掉


## 【已解决】找芯片确认下 I2C 确实只有 4 路吧？
- 是的，只有 4 路


## 【已解决】gxloader 中使用 HAL i2c 那套接口 irq 传输不通
- 需要修改 hal i2c 的模块频率为 99MHz
- 需要修改回调函数中的变量 finish 为 volatile 修饰



## 【已解决】使用 hal i2c dma 传输不通
- i2c3 通了
- i2c1、i2c0 都是 tx 通了，rx 没通

问题原因：
- 驱动需要添加 i2c3
- 接线需要对好
- 提示：如果不通的话先用普通模式测试看看


## 高速 i2c 器件需要问下硬件


## 【已解决】Linux 扫描不到器件
- 此时设备树配置的时钟是 24MHz，切换成 99Mhz 之后测试看看
- loader 中配了引脚复用，并且 loader 可以正常操作设备

问题原因：i2cdetect 工具中，只会扫描 0x30~0x37,0x50~0x5f 的设备
```c
//				if ((i+j >= 0x30 && i+j <= 0x37)
//				 || (i+j >= 0x50 && i+j <= 0x5F))
				 	cmd = MODE_READ;
//				else
//					cmd = MODE_QUICK;
//				break;
			}
```

```
[root@gx6602 gx]# ./i2cdetect -y 3
Warning: Can't use SMBus Quick Write command, will skip some addresses
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- 0c -- -- --
10: -- -- -- -- -- -- -- -- 18 -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 2f
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: 50 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: 60 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --

```


Linux i2c 通不通，怎么确认？i2cdetect 可以扫到设备就行了？
- 使用 i2cset 、i2cget 工具读写看看
```
[root@gx6602 gx]# i2cset -y 1 0x0c 0x0b 0x65
[root@gx6602 gx]# i2cget -y 1 0x0c 0x0b
0x65
[root@gx6602 gx]# i2cget -y 2 0x0c 0x0b
0x65
[root@gx6602 gx]# i2cset -y 2 0x0c 0x0b 0x33
[root@gx6602 gx]# i2cget -y 2 0x0c 0x0b
0x33
[root@gx6602 gx]# i2cset -y 2 0x0c 0x0b 0x12
[root@gx6602 gx]# i2cget -y 2 0x0c 0x0b
```


## 【事业部测试】ecos  测试

- 需要将频率直接改成 99MHz，不需要考虑老的 27MHz
- 需要在 `ecos3.0/packages/io/i2c/v3_0/src/gx_i2c.c` 中修改 `max_busid = 4`
- ecos 做了一层 gx 套接层，事业部会使用套接层来操作 ecos i2c

[^1]: 
