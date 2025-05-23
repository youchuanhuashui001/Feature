
## 1. 基本信息

- **Redmine 问题号**: `#408040`
- **芯片型号/项目名称**: `Virgo_NRE`
- **被测模块**: `I2C`
- **固件/FPGA 镜像版本**: `#404698 [vu9p_virgo_nre_sub_a7.bit] test3 a7核 vu9p板子`
- **测试日期**: `2025-05-12`
- **相关文档**: [[01 项目/Common/IP/dw_i2c ip]]
- **Redmine 问题号**: `#408040` 

## 2. 测试目标

简要描述本次测试的主要目的和范围。
- 兼容 32 位、64 位驱动、测试代码
- I2C 模块验证


## 3. 测试环境

### 3.1 硬件环境

- **FPGA 型号**: `VU9P-05`

### 3.2 软件环境
```
commit c8c3f8dcc3b0587775e99383adc177c5804cf808 (HEAD -> virgonre, refs/published/virgonre)
Author: tanxzh <tanxzh@nationalchip.com>
Date:   Wed Apr 16 20:20:38 2025 +0800

    406477: Virgo NRE Flash_SPI 模块验证
            1. 新增 ddr 模式
            2. 测试程序支持 64 位
    
    Change-Id: Iff4f3a0f3de42b5cfced0c780313511093660454

commit bbb3a47e7060674f40d776b6e05979adccd18671
Merge: bdc426ea 8444e4ca
Author: 刘非 <liufei@nationalchip.com>
Date:   Sun Apr 27 15:10:15 2025 +0800

    Merge "406515: gxloader 支持 nos lib 的编译" into develop2

commit 8444e4cabfddb2a9ce696f5f0b539cb29e8e745a
Author: yemb <yemb@nationalchip.com>
Date:   Wed Apr 16 16:13:28 2025 +0800

    406515: gxloader 支持 nos lib 的编译
    
    Change-Id: I2c9a76e8f50a46c8e4c67bfa18de5495d9c9f1e9

commit bdc426ea3806369fb99bc6fa4e4c190a7f1fc2bc
Author: yangmch <yangmch@nationalchip.com>
Date:   Fri Mar 14 16:26:33 2025 +0800

    398922: 添加gxloader 支持 eMMC 和 SD支持
    
    Change-Id: Ie240b97650bfbb1572bcaa7e3f0a3029207fb54a
```

## 4. 测试用例与结果

### 测试用例 1: I2C Master 快速模式 3.4MHz 读写测试


- **测试步骤**:
    1. `通过I2C总线向从设备特定地址写入一系列数据 (例如：0xAA, 0x55, 0x12, 0x34)`
    2. `从同一地址读回写入的数据`
    3. `使用逻辑分析仪捕获I2C总线波形 (SCL, SDA)`
- **预期结果**:
    - `写入操作成功完成，无NACK`
    - `读回的数据与写入的数据完全一致`
    - `逻辑分析仪显示SCL时钟频率接近3.4MHz，数据传输符合I2C快速模式时序规范`
- **实际结果**:
    - `记录实际观察到的现象和数据`
    - `[[附上逻辑分析仪截图或log文件链接 (如果适用)]]`
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:
    - `覆盖范围: Feature - 支持模块通信速率配置`
    - `注意实际速率可能受限于FPGA时钟和I/O设计`

---

### 测试用例 2: I2C DMA 传输功能测试


- **测试步骤**:
	- 打开 `.config` 中的 `ENABLE_I2C = y `
	- 输入命令：`i2c_async_test -y 0 1`
	- 能够触发 dma tx, rx 传输完成回调，并读写正常。
- **实际结果**:
    - <font color="#ff0000">nre a7 测试失败</font>
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:


---

### 测试用例 3: I2C RX_FULL 中断测试

- **测试步骤**:
	- 打开 `.config` 中的 `ENABLE_IRQ = y， ENABLE_I2C = y`
	- `dw_driver_i2c.c` 开头添加 `#undef CONFIG_ENABLE_AXI_DMA` 来选择使用中断模式；
	- `gx_hal_dw_i2c.c` 中断服务函数中在添加 rx_full、tx_empty 中断打印
	- 输入命令：`i2c_async_test -y 0 1`
	- 能够触发中断服务函数中的打印，并测试正常。
- **实际结果**:
    - nre a7 测试通过
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:
    - `覆盖范围: Feature - 支持中断配置`

---

### 测试用例 4: I2C TX_EMPTY 中断测试


- **测试步骤**:
	- 打开 `.config` 中的 `ENABLE_IRQ = y， ENABLE_I2C = y`
	- `dw_driver_i2c.c` 开头添加 `#undef CONFIG_ENABLE_AXI_DMA` 来选择使用中断模式；
	- `gx_hal_dw_i2c.c` 中断服务函数中在添加 rx_full、tx_empty 中断打印
	- 输入命令：`i2c_async_test -y 0 1`
	- 能够触发中断服务函数中的打印，并测试正常。
- **实际结果**:
    - nre a7 测试通过
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:
    - `覆盖范围: Feature - 支持中断配置`

---

### 测试用例 5: I2C Master 写操作测试

- **测试步骤：**
	- 打开 `.config` 中的 `ENABLE_I2C = y`
	- 输入命令： `i2cset -y 0 0x0c 0x0b 0x33`，向 eeprom 的 0x0b 地址处写入数据 0x23，可通过后续的读操作校验数据
![[Pasted image 20250514160204.png]]
- **实际结果**:
    - nre a7 测试通过
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:


---

### 测试用例 6: I2C Master 快速模式 400KHz 读写测试

- **测试步骤**:
	- 切换 i2c-core. c 中 i2c 总线速度为 400k： `i2c-core.c` 中的 `i2c_bus_freq、high_time、low_time` 依次修改为 `400000UL, 1250, 1250`
	- i2c 设备子板上没有 GX1133，autotest 中去掉对 gx1133 的测试程序
	- 配置 i2c 设备子板上 grus 为 i2c_slave：`i2c_slave 0 0x18`
	- 输入命令 `i2c_auto_test -y 0 100`
	- 逻辑分析仪查看 scl 引脚频率是否为 400kHz
![[Pasted image 20250514160356.png]]
```
[2025-05-14 15:51:37] boot> i2c_auto_test -y 0 100
[2025-05-14 15:54:40] test cnt : 1
[2025-05-14 15:54:40] start test RDA5815M
[2025-05-14 15:54:40] test RDA5815M successfully
[2025-05-14 15:54:40] start test MXL608
[2025-05-14 15:54:40] test MXL608 successfully
[2025-05-14 15:54:40] start test GRUS
[2025-05-14 15:54:40] test GRUS successfully
[2025-05-14 15:54:40] start test EEPROM M24C32
[2025-05-14 15:54:40] test EEPROM M24C32 successfully
```
- **实际结果**:
    - nre a7 测试通过
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:
```diff
+//             /* GX1133 */
+//             printf("start test GX1133\n");
+//             printf("test GX1133 successfully\n");


--- a/drivers/i2c/i2c-core.c
+++ b/drivers/i2c/i2c-core.c
@@ -15,7 +15,7 @@ struct gx_i2c_device  g_gx_i2c_device[MAX_I2C_DEVICES];
 
 #if defined(CONFIG_ARCH_ARM_VIRGO) || defined(CONFIG_ARCH_ARM64_VIRGO2)
 struct gx31x0_i2c g_gx31x0_i2c[] = {
-       {(REG_BASE_I2C1), 24000000UL, 100000UL, 5000, 5000},
+       {(REG_BASE_I2C1), 24000000UL, 400000UL, 1250, 1250},
```

---

### 测试用例 7: I2C Master 标准模式 100KHz 读写测试
- **测试步骤**:
	- i2c-core. c 中将 i2c 总线切回 100khz
	- 输入命令：i2c_auto_test -y 0 100，进行 100 次测试，测量 scl 引脚频率
![[Pasted image 20250514160503.png]]
```
[2025-05-14 15:51:37] boot> i2c_auto_test -y 0 100
[2025-05-14 15:54:40] test cnt : 1
[2025-05-14 15:54:40] start test RDA5815M
[2025-05-14 15:54:40] test RDA5815M successfully
[2025-05-14 15:54:40] start test MXL608
[2025-05-14 15:54:40] test MXL608 successfully
[2025-05-14 15:54:40] start test GRUS
[2025-05-14 15:54:40] test GRUS successfully
[2025-05-14 15:54:40] start test EEPROM M24C32
[2025-05-14 15:54:40] test EEPROM M24C32 successfully
```
- **实际结果**:
    - nre a7 测试通过
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:
    - `覆盖范围: Feature - 支持模块通信速率配置`

---

### 测试用例 8: I2C Master 读操作测试

- **测试步骤**:
	-  输入命令：i2cget -y 0 0x0c 0x0b 1，读取 eeprom 的 0x0b 处的数据读出，与写入一致，测试通过
![[Pasted image 20250514165017.png]]
- **实际结果**:
    - nre a7 测试通过
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:
    - `覆盖范围: Feature - 支持Master模式配置`

---

### 测试用例 9: I2C 时钟延展 (Clock Stretching) 测试
- 时钟延展功能是在编 ip 时配置是否支持，软件无法通过寄存器进行配置，grus 支持时钟延展并且数据处理很慢，400k 模式下与 grus 通信，查看是否存在时钟延展的情况
- autotest 中去掉与其它设备通信的测试代码，循环与 grus 进行通信，逻辑分析仪抓到 scl 脚存在时钟延展的情况，并且数据传输正确，测试通过
- 切换 i2c 速度为 400k
- i2c_auto_test -y 0 100
![[Pasted image 20250514164706.png]]
- **实际结果**:
	- nre a7 测试通过
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:
    - `覆盖范围: Feature - Master模式时钟延展配置`
    - `需要从设备配合进行测试`

---

### 测试用例 10: I2C 时序参数配置测试 (SDA 保持时间等)

- **测试步骤**:
	- 通过配置寄存器 0x7c 来改变 scl 由高变低后的 sda 维持时间，时间单位是 ic_clk 一个周期，默认为1，查看 scl 由高拉低后，sda 在 200ns 后开始修改；
		- 配置为 0x100010，多了 15 个周期，差不多是 15 * 40 =  600ns，实际测试 sda 的维持时间变为 800ns 了，与配置基本一致，测试通过。
		- `set *0xfc80007c=0x100010`
		- 此寄存器需要在 i2c_enable 为 0 时才可配置，默认未传输期间 I2c_enable 一直为 0，所以可以直接配
![[Pasted image 20250514172441.png]]
![[Pasted image 20250514172449.png]]
- **实际结果**:
    - nre a7 测试通过
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:
    - `覆盖范围: Feature - Master模式SDA保持时间配置`
    - `此测试可能需要高精度测量设备`

---

### 测试用例 11: I2C Master 读写多个 Slave 测试

auto_test 时已测到，测试通过

---




## 7. 问题与缺陷记录

### 问题 1：对接 I2C 设备小板经常写入出错
```
[2025-05-14 13:49:27] Error during I2C_RDWR ioctl with error code: -11
[2025-05-14 13:49:27] write i2c chip_addr 0x0c reg 0x0b failed
[2025-05-14 13:49:27] boot> i2cset -y 0 0x0c 0x0b 0x33
[2025-05-14 13:49:28] Error during I2C_RDWR ioctl with error code: -11
[2025-05-14 13:49:28] write i2c chip_addr 0x0c reg 0x0b failed
[2025-05-14 13:49:28] boot> i2cset -y 0 0x0c 0x0b 0x33
[2025-05-14 13:49:29] Error during I2C_RDWR ioctl with error code: -11
[2025-05-14 13:49:29] write i2c chip_addr 0x0c reg 0x0b failed
[2025-05-14 13:49:29] boot> i2cset -y 0 0x0c 0x0b 0x33
```
有时能够成功写入，有时会写入失败；将 FPGA 与 I2C 设备小板对接的线换短后正常。
若 slave 未应答，则会返回出错。错误码为 -110 或-121
- -110 表示 TIMEOUT
	- 此时由于用错了 i2c，编的是 I2c0，用的是 i2c1，导致检测不到 i2c1 的 fifo 中断状态，等计时时间到后，产生超时。
- -121 表示 IO 设备错误
	- 发第一个 message，包括 slave address + r/w，再发第二个 msg，包括要操作的 slave 寄存器地址，处理完两个 msg 后再处理报错
	- 查看中断状态(DW_IC_INTR_STAT)，发生了 tx_abort 中断
		- `dev->abort_source` 保存中断源 (0x80)，读寄存器 DW_IC_CLR_TX_ABRT 清掉中断
	- 注意这里不会循环不会退出，直到循环了 N 次再退出，所以会检查到错误中断状态
	- 检查到 tx_abrt 后将 cmd_err 标记为 err_tx_abrt，并且 mask 所有的中断，不上报任何中断
	- 从 `dev->abort_source` 中查看中断源：
		- noack：
			- 包括 7b addr、10addr、txdata 等 noack 的情况
			- 查看具体是什么  noack，打印出来
			- 返回 -121
		- arb_lost：
			- 返回 -11 (AGAIN)
		- gcall_read：
			- 返回 -22，错误的 msg data


## 8. 测试总结与结论

- **测试覆盖率评估 (简述)**: `例如：I2C主要功能 (标准/快速模式读写, 中断, DMA) 已覆盖，边界条件 (如最大速率，多从设备) 部分覆盖。`
- **模块稳定性评估**: `例如：在常规操作下稳定，但在高速率和长时DMA传输下需进一步验证。`
- **遗留问题**: `列出尚未解决的关键问题。`
- **后续步骤/建议**:
    - `例如：对BUG-I2C-001进行根本原因分析，检查FPGA时序约束或驱动逻辑。`
    - `例如：增加针对I2C从机模式的完整测试用例。`
    - `例如：进行I2C总线错误注入测试 (如SDA/SCL短路到地/电源)。`
- **总体测试结果**: `通过 / 条件通过 / 失败`

## 9. 附件

- `[[链接到Log文件]]`
- `[[链接到配置文件]]`
- `[[链接到I2C协议分析仪捕获文件.sal]]`
- `[[链接到其他相关截图或数据]]`


