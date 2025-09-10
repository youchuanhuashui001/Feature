


## 基本信息
- ID：`0xB36015  `
- UID：128 bit  
- OTP：`3*1024 Byte  `
- 支持双线、四线读操作，并且最高工作频率为 85MHz
- 四倍速状态寄存器操作方式：支持 0
	 - `sflash_enable_quad_0`：  
		```
		0x35: read_sr2()  
		0x31: write_sr2()  
		```
	 - `sflash_enable_quad_1  `
		```
		0x05: read_sr1()  
		0x35: read_sr2()  
		0x01: write_sr1_sr2  
		```

## 驱动修改

- scpu：  
```diff
--- a/drivers/legacy/mtd/spinor/spi_nor_ids.c  
+++ b/drivers/legacy/mtd/spinor/spi_nor_ids.c  
@@ -214,6 +214,7 @@ NORFLASH_INFO_T spinor_list[] = {  
 
        /*--- ucun ---*/  
        {"uc25wq80",   0x00b36014,   0x100000,   DUAL_READ | QUAD_READ, NULL, NULL},  
+       {"uc25wq16",   0x00b36015,   0x200000,   DUAL_READ | QUAD_READ, NULL, NULL},  
        {"uc25hq32",   0x00b36016,   0x400000,   DUAL_READ | QUAD_READ, NULL, NULL},  
 
        /*--- Tsingteng ---*/  
(END)  

```
- vsp：  
```c
	{"uc25wq16",   0x00b36015,   0x200000,   DUAL_READ | QUAD_READ },  
```


## 测试用例

- 使能测试程序`flash_test`
	```
	make menuconfig                                                                                                                                                
	[*] Memory Technology Device (MTD) support  --->    
		[*]   MTD tests suppor    

	Application  --->    
		[*] flash test    
	```
- 修改工作模式
	```
	make menuconfig    

	[*]   Designware FLASH_SPI nor    
		FLASH_SPI frame format (Dual speed)  --->    
	[ ]     Flash spi 32 bits width    
	[ ]     Support flash spi xip    
	
	```
- XIP 测试：
	- 关掉 Stage1 的 XIP 初始化
	- 关掉 Stage2 的 XIP 初始化
	    - flash 初始化完之后查看 `0x30000000` 地址
		- 已进行 xip 初始化，查看 `0x30000000` 地址正常，显示 flash 中的数据
		- 未进行 xip 初始化，查看 `0x30000000` 地址卡死

- 掉电测试
	- 掉完电之后擦不掉 flash
	- 读状态寄存器一次就读到了，并且数据都是 0x0
- [x] 用 `W25Q128` 测试一把，这是新加的测试程序，理论来说不会有影响
	- 掉电测试后正常
- [x] 再拿一颗 `uc25wq16` 测试一把，是不是跑完`掉电测试程序`也变成这样
	- 还是正常的
- [x] 发现是被软件写保护了
	- 读出来的状态寄存器:`SR1=0x0,SR2=0x62`
	- 对应的 CMP = 1，其余的 BP = 0，导致保护了整个 flash
	- 解除了写保护之后 ok
- [x] 说明是在之前的某一次使能四倍速的时候读错了值，导致把 CMP 位置成 1 之后写进去了
	- 确认 Stage1 的时钟和相位
		- 50MHz，配置的相位为 2(1 ~ 4 可用)
	- 确认 Stage2 的时钟和相位
		- 50MHz，配置的相位为2(1 ~ 4 可用)
	- 确认这个 flash 支持的时钟和相位
		```
		CLK_SRC                       200000000
		CLK_DIV -- Sample_Delay       4	  5 error Stage1 起 Stage2 失败;Stage2 rw error
		CLK_DIV -- Sample_Delay       4	  4
		CLK_DIV -- Sample_Delay       4	  3
		CLK_DIV -- Sample_Delay       4	  2
		CLK_DIV -- Sample_Delay       4	  1
		CLK_DIV -- Sample_Delay       4	  0 error Stage1 起 Stage2 失败;Stage2 rw error
		```


| 测试项     | 测试结果 |
| ------- | ---- |
| 双线读写擦测试 | ok   |
| 四线读写擦测试 | ok   |
| XIP 测试  | ok   |
| 掉电测试    | ok   |



### 后续记录
- [x] datasheet 添加到云盘
- [x] 支持的问题号添加到 Robots WIKI


