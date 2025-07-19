---
tags:
  - Virgo_MPW模块验证
---
# 验证进度：
- 总验证点:: 18
- 已完成:: 18
- 备注:: SPI1 做 slave 时性能不太好，需要硬件隔开板子铜皮与 Apus 共地



# 测试项


## 标准模式读写
- SPI0
	- masster 模式测试通过
	- slave 模式测试通过
- SPI1
	- masster 模式测试通过
	- slave 模式测试通过



## Dual 模式读写
- SPI0
	- masster 模式测试通过
	-  slave 模式测试通过
- SPI1
	- masster 模式测试通过
	- slave 模式测试通过


## Quad 模式读写
- SPI0
	- masster 模式测试通过
	-  slave 模式测试通过
- SPI1
	- masster 模式测试通过
	- slave 模式测试通过



## 标准模式 外接 flash
- 不能从 flash 启动，所以需要用 .boot 进行测试
- 只烧 .boot 的时候好像不会跑 pll_mini 
- SPI0
	- 可以读到 flash id，但是传输数据会出错
		- 换成短线后，单线可以读写，但是会 Receive fifo overflow，双线依然会出错，只是出错概率变小了
		- 增强驱动强度，失败的次数变少了，但还是会卡住
			- 逻辑分析仪抓到，发了16个字节，但是第17个字节有 clk 没有数据，后面就没有 clk
			- 卡在 tx，换成四线也会卡
		- 使用 dma 来传输，好像依然是第十七个字节就卡住了
		- 加了个打印，然后好了？
			- 不会卡住了
			- 关了驱动强度，就数据读写错误
		- 驱动强度目前是 5，然后singletest 0x20 的时候，写只会写一部分数据，就走掉了
		- 确实啊，加了个打印就好了
		- 问题原因是代码问题 [[Virgo_MPW GEN_SPI 模块验证#^d6a5d0]]
	- 增加驱动强度到 5，读写 flash 通过
- SPI1
	- 驱动强度配成5，读写 flash 通过



## Dual 模式外接 flash
- SPI0
	- 驱动强度配成5，读写 flash 通过
- SPI1
	- 驱动强度配成5，读写 flash 通过



## Quad 模式外接 flash
- SPI0
	- 驱动强度配成5，读写 flash 通过
- SPI1
	- 驱动强度配成7，读写 flash 通过
	- 同步进行了两个修改，驱动强度从5改成7，以及长线换成短线，不知道是什么导致数据读写正确的


## 片选 CS
- SPI0 的三个 CS 可以正常拉高拉低，测试通过
- SPI1 的三个 CS 可以正常拉高拉低，测试通过



## 中断
- SPI0
	- tx_empty：
		- 在 `gx_hal_spi_hw_init`的时候使能 tx_empty 中断
		- 打开 CONFIG_SPI_DEBUG 宏
		- 起来之后发几个数据，其实就是初始化完 spi 之后，就可以触发tx empty 中断
	- rx_overflow：
		- 关掉 tx_empty 中断
		- 打开 CONFIG_SPI_DEBUG 宏
		- 注释掉 `gx_hal_spi_poll_recv_msg` 接口中读数据
		- 起来之后读数据，单次超过 fifo 大小就能够触发 Receive overflow 中断
		- fifo 大小是 8
- SPI1
	- 对接 flash 可以正常触发中断
	- tx_empy
		- 可以正常触发
	- rx_overflow：
		- 可以正常触发


## DMA
- SPI0
	- 同步 dma，已完成
	- 异步 dma ，已完成
	- 对接 flash ，已完成
- SPI1
	- 同步 dma，已完成
	- 异步 dma，已完成
	- 对接 flash，已完成



## 性能测试
- SPI0
	- master
		- slave 模块频率为 32MHz，需要同时读写，那么最快 master 也只能跑 4MHz
		- 1MHz，可以和 slave 正常通信
		- 2MHz，可以和 slave 正常通信
		- 3MHz，可以和 slave 正常通信
		- 4MHz，不能和 slave 正常通信，把 apus gen_spi 模块频率切成 pll 96MHz 后，可以正常通信
		- 96MHz 8分频或者 12 分频能到 12MHz 或者 8MHz
		- 6MHz，可以和 slave 正常通信
		- 8MHz，数据会出错，改一下相位试试
			- 相位4的时候，单线可以过
			- 相位 6 的时候，可以测过，10也可以，14也可以，18也可以
		- 10MHz，测试会失败
	- slave
		- master 的模块频率是 32MHz，slave 模块频率是 148.5MHz，理论 master 可以跑到 12MHz 左右
		- 1Mhz，
			- 数据会错误，修改了 master 的 sample_delay 也还是会错
			- 修改 slave 的 sample_delay 试试看
			- 测试 100khz 的时候也会出错，也还是那个位置
			- 测试 1khz 的时候依然出错，也还是那个位置
			- 错误的数据是第8个字节，而 rxfifo 的深度就是8，怀疑是 rxfifo 有数据，但是没有及时取走
			- master 使用 dma 来取数据，单纯 rx tx 可以正常发送和接收数据，但是 case 的时候还是一样的情况
				- 换成 32 bit 也还是不行
			- **需要将线换成短的！！！！**
		- 1MHz 可以和 master 正常通信
		- 4MHz 可以和 master 正常通信
		- 8MHz 的时候不能，换成用 dma 来传输看看
			- 先怀疑 master 取的不够及时，将 master 换成 dma，不行，在 quad 的时候传输 100 个字节，第0个字节就错误了
			- 将 slave 也换成 dma 来放数据，可以了。看起来是 slave 放数据放的不够及时，因为 clk 都是来自 master 的，而 master 会根据 fifo 是否有剩余空间来发 clk，从 slave 取数据的，所以一般 master 不会不及时
			- 只将 slave 换成 dma 放数据，master 端还是 cpu 取数据，可以正常传输。
		- 12MHz 可以和 master 正常通信
		- 16MHz 不可以
		- 18MHz 不行
			- 将 master 的模块频率调到 96MHz，初步可以传输一大批数据了，在 quad 的时候会失败
			- 把 master 调成用 dma 来取数据，依然不行
- SPI1
	- master
		- 4MHz 可以和 slave 正常通信
		- 6MHz 可以和 slave 正常通信
		- 8MHz 修改相位到10，可以和 slave 正常通信
		- 10MHz 不行
	- slave
		- 1MHz 有问题
			- quad 的时候读到第17个字节有问题，修改 sample_delay 试试
				- 看起来没啥用
			- 增加 slave 的驱动强度
				- 到7的时候，dual都无法正常通信
			- slave 端切成使用 dma，master 端频率切到 100K，修改相位到0
				- 因为看起来 master 收到的数据是错的，然后发给 slave，salve 收到的就是那个值
				- 目前怀疑是相位问题，依然不行，同样的位置
			- 怪得很，单独的测试 slave 发送，master 接收正常
			- 怀疑 master 取数据取的慢，换成用 dma
				- 没用啊，slave 也换成用 dma，也没用
				- 怀疑一下线
			- 逻辑分析仪抓一下吧，看看数据到底是什么
			- 难道 slave 的时候 sample_delay 也有效？
				- master 相位配成3，0x7 失败
				- master 相位配成6，0xf 失败
				- master 相位配成8，0xf 失败
				- master 相位配成10，0xf 失败
				- 现在把 master 的环境固定，slave 切成用 spi1
					- **切成 spi1，一开始也不能用，将板子歪一点，就通了！！！**
					- 说明 master 的软件是好的，slave 这边只换一下 spi 接口就行
				- 驱动强度改成1，传的数据变多了
					- 数据从 0 开始变成了，| 0x10
					- 抓 wp、hold 脚的拉高拉低时间，mosi 和 miso 的时间
				- 两块板子接地多借一点，数据好多了
				- **两块板子铜皮隔开焊在一起，1k Hz 可以正常传输了**
					- 1MHz 也可以正常通信
					- 不需要修改驱动强度
					- 不能用 dma
		- 4MHz 可以正常通信
		- 
	




# 问题

## 【已解决】GEN_SPI 不通，逻辑分析仪能够抓到 cs0 的波形，但是看起来不正确
- 可以正常通信，接错 io 口了
- 示波器量到电压是 4v，探头有问题，校准后测试正常

## 【已解决】使用 DMA 不通，即使不解外设，只发4个字节，slave 接收数据也不对
- 查看驱动发现是使用 dma 的时候会清掉高4bit，原因是 scpu 那边的数据都是在 sram，把虚拟地址转换成物理地址，再传给 dma
- gxloader 中物理地址和虚拟地址相同，所以不需要清掉高4bit



## 【已解决】异步 DMA 发现有 32 个字节数据为 0，但是程序没有报错
- 读完之后，打印数据发现其中有 32 个字节为 0，程序会将这些数据和 0、1、2、3、xxx 进行比较的，所以不可能出现这种情况

- cache 问题，gdb 去看也全是 0，手动调一下刷 cahce，数据就对了
	- 原因是async 的时候没有等回调结束就去打印数据了


## 对接 flash 无法正常读写

^d6a5d0

- 在 tx 的时候，加了句打印就好了
- gdb 单步的时候会不会出现，具体无法读写体现在 cpu 直接到了 undefined 异常
- 读写数据出错，修改驱动强度之后好了，修改到 5 
	- 不修改驱动强度，数据会读写出错

- 跑飞了之后 pc、lr都飞了，sp 看看
- 开启中断，看看有没有触发什么中断
	- 没有触发什么奇怪的中断，开了中断之后也好了

- 逻辑分析仪抓波形：
	- 需要写 0x20 个数据，但是写了 17 个数据，然后没有 clk，cs 也是一直拉低的
- 发现是计算错误，fifo 已经满了，但还会继续往 txfifo 填数据



两个测试：
- 一直往 txfifo 写，不管是否有剩余空间
	- 不管是否有剩余空间会导致死掉
- 起来的时候读下 text 段，挂了之后读下 text 段，有没有被修改
	- 没有被修改
- 不要启动 spi，单步往 fifo 写，看看是不是超过了 fifo 大小就会死
	- 不启动 spi，数据放不到 tx fifo 
	- fifo 大小是 16，写17个不会死，写18个不会死
	- watch $lr 看看是谁改变的，因为循环就会死，
```c
	printf("start test\n");
	// C 语言代码片段
	int i;

	// 禁用 SSI
	*(volatile int*)0xfc000008 = 0x0;

	// 配置控制器、波特率等
	*(volatile int*)0xfc000000 = 0x80000C07;
	*(volatile int*)0xfc000014 = 0x4;
	*(volatile int*)0xfc000018 = 0x0;
	*(volatile int*)0xfc0000F4 = 0x0;

	// 使能 SSI 和从设备选择
	*(volatile int*)0xfc000008 = 0x1;
	*(volatile int*)0xfc000010 = 0x1;

	// 循环写入数据到数据寄存器 (16次)
	for (i = 0; i < 17; i++) {
	    *(volatile int*)0xfc000060 = 0x9F;
	}

	printf("end test\n");
  
watch $lr

```


- 在 fpga 上跑跑看，如果可以复现，找芯片仿真看看
	- fpga 上不会复现，用的是 nre a7 的 bit
	- 用 mpw 的bit试试看



- 问题原因：tx fifo 已满，还在往 txfifo 写
	- 代码错误
![[5aab6453f90ffaed301e76a451cd40f0.jpg]]



- 应该会触发 txfifo overflow
	- 中断服务函数中不要去处理数据，看看会不会触发这个中断
	- 确实会触发 Transmit FIFO overflow，但还是 pc 会跑飞




## 【已解决】GEN_SPI1 接线
- TSI2_SYNC
	- 接着 spi0后面的脚
- TSI2_VALID
	- 接着 spi0 后面的脚
	- rm4、rm5 都要去掉
- 硬件已提供
	- 左侧 J65、J16 排针，从左往右，第四根为 MISO、第五根为 WP、第十根为 MOSI、第十一根为CLK
	- 右侧 J68、J18 排针，从左往右，第四根为 HOLD
	- CS 均未引到排针，需要单独接出来
![[d4efbac5e3315d9ca0d9f1b7b5571330.jpg]]


## 【已完成】Gen_SPI1 读写数据出错
- 修改驱动能力到最强，依然会出错；
- 将分频由6切成12

- 单线是好的，那么 MOSI、MISO 都是好的，会不会是有干扰
	- 是因为硬件后面有两个电阻还和 i2c 连着，去掉之后好了

- 四线是通的，但是传输 200 个字节就会出错
	- 换成短线，好了！
- 做 slave 时有问题
	- 需要多共地
	- 割开铜皮进行共地，测试 ok