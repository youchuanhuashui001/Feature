# DW I2C 芯片测试文档

## 1. 测试环境
- FPGA 版本：`382541 9p_peri_newcode_FPGA_2024-09-03.tar`
- VU9P

## 2. 测试用例

### 2.1 测试 master 高速模式 3.4MHz 读写 (需要高速 i2c 设备)
- 测试步骤：
	- 打补丁：`https://git.nationalchip.com/gerrit/#/c/115970/5`
	- `i2c_highspeed_test -y 1 1`
	- 验证数据传输是否正确
- 预期结果：
	- I2C master 在 3.4MHz 频率下能正常读写数据

### 2.2 测试 DMA 传输功能
- 测试步骤：
	- 打开 `ENABLE_I2C = y；ENABLE_IRQ = y` 选项
	- 打补丁 `https://git.nationalchip.com/gerrit/#/c/115970/5`
	- `i2c_async_test -y 1 1`
	- 查看 DMA 回调是否正常
- 预期结果：
	- I2C 控制器能通过 DMA 正确传输数据

### 2.3 测试 RX_FULL 中断
- 测试步骤：
	- 打开 `ENABLE_I2C = y；ENABLE_IRQ = y` 选项
	- 打补丁 `https://git.nationalchip.com/gerrit/#/c/115970/5`
	- `dw_driver_i2c.c` 的开头加上 `#undef CONFIG_ENABLE_AXI_DMA`
	- `i2c_async_test -y 1 1`
	- 验证中断是否正确触发
	- b 到 `gx_hal_i2c_master_irq_handler`，看寄存器 `0xfc810034`  bit2 是否有对应中断
- 预期结果：
	- 接收缓冲区达到 rx fifo 阈值时正确触发 RX_FULL 中断

### 2.4 测试 TX_EMPTY 中断
- 测试步骤：
	- 打开 `ENABLE_I2C = y；ENABLE_IRQ = y` 选项
	- 打补丁 `https://git.nationalchip.com/gerrit/#/c/115970/5`
	- `dw_driver_i2c.c` 的开头加上 `#undef CONFIG_ENABLE_AXI_DMA`
	- `i2c_async_test -y 1 1`
	- 验证中断是否正确触发
	- b 到 `gx_hal_i2c_master_irq_handler`，看寄存器 `0xfc810034`  bit4 是否有对应中断
- 预期结果：
	- 发送缓冲区空时正确触发 TX_EMPTY 中断

### 2.5 测试 master 写
- 测试步骤：
	- 打开 `.config` 中的 `ENABLE_I2C = y`
	- `i2cset -y 1 0x0c 0x0b 0x33`
	- 验证数据是否正确写入
- 预期结果：
	- I2C master 能正确写入数据到 slave 设备
- 实际波形：
![[Pasted image 20250318112032.png]]

### 2.6 测试 master 快速模式 400 KHz 读写
- 测试步骤：
	- 打开 `.config` 中的 `ENABLE_I2C = y`
	- `i2cset clk 400000`，配置 clk 为 400k
	- `i2cset -y 1 0x0c 0x0b 0x33`
	- 逻辑分析仪量 `scl` 周期为 400k
	- 验证数据传输是否正确
- 预期结果：
	- I2C master 在 400 KHz 频率下能正常读写数据
- 修改代码：
```diff
--- a/bootmenu.c
+++ b/bootmenu.c

@@ -2686,6 +2710,15 @@ void i2cset(int argc, const char **argv)
        int data_len = 0;
        int i = 0;
        unsigned int reg_addr_width = 1;
+       
+       if (strcmp(argv[1], "clk") == 0){
+               unsigned int clk;
+               clk = atoi(argv[2]);
+               i2c_dev = gx_i2c_open(1);
+               gx_i2c_clk_conf(i2c_dev, clk);
+               gx_i2c_close(i2c_dev);
+               return;
+       }

```

### 2.7 测试 master 标准模式 100 KHz 读写
- 测试步骤：
	- 打开 `.config` 中的 `ENABLE_I2C = y`
	- 配置 grus：`i2c_slave 0 0x18`；注释掉 `GX1133` 的测试代码
	- 设置时钟频率为 100 KHz
	- `i2c_auto_test -y 1 1`
	- 执行读写操作
	- 验证数据传输是否正确
- 预期结果：
	- I2C master 在 100 KHz 频率下能正常读写数据

### 2.8 测试 master 读
- 测试步骤：
	- 打开 `.config` 中的 `ENABLE_I2C = y`
	- `i2cget -y 1 0x0c 0x00 2`  `# RDA5815M`
	- 验证数据是否正确读取
- 预期结果：
	- I2C master 能正确从 slave 设备读取数据
- 实际波形：
![[Pasted image 20250318111928.png]]

### 2.9 测试时钟延展
- 测试步骤：
	- 打开 `.config` 中的 `ENABLE_I2C = y`
	- 配置 grus：`i2c_slave 0 0x18`；注释掉 `GX1133` 的测试代码
	- 设置时钟频率为 100 KHz
	- `i2c_auto_test -y 1 1`
	- 验证数据传输是否正确
	- grus 默认开了时钟延展，如果可以和 grus 正常通信，就 ok
- 预期结果：
	- I2C 时钟延展功能正常工作

### 2.10 测试配置 i2c 时序
- 测试命令：
	- 打开 `.config` 中的 `ENABLE_I2C = y`
	- `i2cset clk 400000`，配置 clk 为 400k
	- `clk 400000` 配置为 400k 时，高低电平都是 1.2us，`i2c-spec` 要求高电平只需要 `0.6us`，手动修改 `adap->dev->hight_time = 800`，逻辑分析仪量一下是 `0.8us` 左右即可
	- 验证时序配置是否生效
- 预期结果：
	- I2C 时序配置正确生效

### 2.11 测试 master 读写多个 slave
- 测试步骤：
	- 打开 `.config` 中的 `ENABLE_I2C = y`
	- 配置 grus：`i2c_slave 0 0x18`；注释掉 `GX1133` 的测试代码
	- 设置时钟频率为 100 KHz
	- `i2c_auto_test -y 1 1`
	- 执行读写操作
	- 验证数据传输是否正确
- 预期结果：
	- I2C master 能正确读写多个 slave 设备

## 3. 测试结果
- [ ] 2.1 测试 master 快速模式 3.4MHz 读写
- [x] 2.2 测试 DMA 传输功能
- [x] 2.3 测试 RX_FULL 中断
- [x] 2.4 测试 TX_EMPTY 中断
- [x] 2.5 测试 master 写
- [x] 2.6 测试 master 快速模式 400 KHz 读写
- [x] 2.7 测试 master 标准模式 100 KHz 读写
- [x] 2.8 测试 master 读
- [x] 2.9 测试时钟延展
- [x] 2.10 测试配置 i2c 时序
- [x] 2.11 测试 master 读写多个 slave

## 4. 测试记录

### I2C 子板使用注意：
1. 12V 供电
2. grus 通信时需要先连接 grus 的串口配置为 slave 模式
```
i2c_slave 0 0x18
```
1. 主机通过不同的 slave address 选择不同的 slave 进行通信

### 主机没有波形输出
- 芯片引出来的是 i2c1，而不是 i2c0


### 关注点
- i2c 默认模式，以及如何切模式
	- 默认是 100k，在 `struct gx31x0_i2c g_gx31x0_i2c` 结构中描述了，其中的第三个参数
	- 通过 `gx_i2c_clk_conf` 配置新的 clk 模式，必须要重新调用 `gx_i2c_set` 来将新的 clk 模式配置到寄存器
```c
struct gx31x0_i2c g_gx31x0_i2c[] = {
	{(REG_BASE_I2C1), 24000000UL, 100000UL, 5000, 5000},
	{(REG_BASE_I2C2), 24000000UL, 100000UL, 5000, 5000},
	{(REG_BASE_I2C3), 24000000UL, 100000UL, 5000, 5000},
	{(REG_BASE_I2C4), 24000000UL, 100000UL, 5000, 5000},
	{(REG_BASE_I2C5), 24000000UL, 100000UL, 5000, 5000},
	{(REG_BASE_I2C6), 24000000UL, 100000UL, 5000, 5000},
};
```


- 配置 i2c 时序怎么配？配完之后体现在哪里？I2C 器件对这些时序的要求？
	- 如何配置？
		- 配置 `dev->high_time(tHD;STA = tHIGH)、dev->low_time(tLow)`
		- 默认是 100k，对应的 high、low 都是 5000，按照 designware i2c 的手册要求计算得到可以配置到寄存器的值
		- 对应的配置到 `DW_IC_XX_SCL_HCNT、DW_IC_XX_SCL_LCNT` 寄存器
	- 配完之后体现在哪里？
		- 可以从波形看出来，例如速度 100k，一个周期是 10us，配的都是一半，那么 SCL 线上高电平低电平的时间就是 5us
	- 标准是什么？
		- 标准来自 `i2c-spec`
		- 标准模式要求 `4.0us、4.7us`；快速模式要求 `0.6us、1.3us`
		- 手册描述的都是最小值，驱动中一般配置的都是 50%，也就是 100k 的模式下，周期是 10us，SCL 的高低维持时间都是 5us；400k 模式下，周期是 2.5us，SCL 的高低维持时间都是 1.25us；满足最小的要求
![[Pasted image 20250313093909.png]]
![[Pasted image 20250318102437.png]]


### 默认支持时钟延展？不需要手动打开？
- 看协议上是需要支持时钟延展功能的，所以可能 ip 默认就支持了时钟延展功能
- 是的，默认就支持了时钟延展，grus 默认开了时钟延展，如果可以正常通信就 ok

### 不支持 slave 功能吗？
- ip 支持，但是用不上，所以不测

### 测试高速模式 3.4MHz，实测只有 1.7MHz
- 换 48MHz I2C 时钟的 bit `(382541 virgo_9p_i2c_48M_2_FPGA_2024-09-14.tar)` 测试，依然不行
- **需要用支持高速模式的 i2c 设备来测**

### 中断/DMA 有一个宏可以切
- 需要在 `dw_driver_i2c.c` 的开头 `#undef CONFIG_ENABLE_AXI_DMA` 就默认用中断
- 否则就用 dma


### 配置 slave address 的时候，是不带 R/W 位的
- 例如：EEPROM M25C32 手册描述如下，三个引脚都接地，那么实际的为 0xA0、0xA1；驱动中实际写入 `DW_IC_TAR` 寄存器的是 0x50、0x51
![[Pasted image 20250318170744.png]]
![[Pasted image 20250318170813.png]]
![[Pasted image 20250318170845.png]]
![[Pasted image 20250318170857.png]]
