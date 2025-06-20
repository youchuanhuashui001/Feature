
## 1. 基本信息

- **Redmine 问题号**: `#408040`
- **芯片型号/项目名称**: `Virgo_NRE`
- **被测模块**: `I2C`
- **固件/FPGA 镜像版本**: 
	- a7：#404698 vu9p_fpga_top_sub_a7_20250524.bit test7 a7核 flash中48M
	- a55: #404698 vu9p_fpga_top_sub_a55_20250524.bit test8 a55核 flash中48M
- **相关文档**: [[01 项目/Common/IP/dw_i2c ip]]


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
    - 测试通过
- **状态**: `通过 / 失败 /阻塞 / 未执行`
- **备注/观察**:
	- 对于 a55 中断基础环境还没准备好，dma read 时，需要先调用 gx_hal_i2c_master_receive_dma_start 把命令发出去，然后 poll 启动 dma
```diff
--- a/bootmenu.c
+++ b/bootmenu.c
@@ -2827,22 +2827,22 @@ void i2c_async_test(int argc, const char **argv)
                data_len = 2;
                finish = false;
                gx_i2c_async_rx(bus_id, chip_addr, reg_addr, reg_addr_width, rx_data, data_len, async_callback, NULL);
-               while(finish == false);
+//             while(finish == false);
 
-               if (rx_data[0] != 0x58 || rx_data[1] != 0xf8) {
-                       printf("read rda5815m ID error\n");
-                       goto i2c_auto_test_exit;
-               }
+//             if (rx_data[0] != 0x58 || rx_data[1] != 0xf8) {
+//                     printf("read rda5815m ID error\n");
+//                     goto i2c_auto_test_exit;
+//             }
                reg_addr = 0x0b;
                data_len = 1;
                tx_data[0] = 0x33;
 
                finish = false;
                gx_i2c_async_tx(bus_id, chip_addr, reg_addr, reg_addr_width, tx_data, data_len, async_callback, NULL);
-               while(finish == false);
-               finish = false;
+//             while(finish == false);
+//             finish = false;
                gx_i2c_async_rx(bus_id, chip_addr, reg_addr, reg_addr_width, rx_data, data_len, async_callback, NULL);
-               while(finish == false);
+//             while(finish == false);


diff --git a/drivers/i2c/dw_driver_i2c.c b/drivers/i2c/dw_driver_i2c.c
index 4e62f2e5c..a814839ef 100644
--- a/drivers/i2c/dw_driver_i2c.c
+++ b/drivers/i2c/dw_driver_i2c.c
@@ -618,8 +618,13 @@ int gx_i2c_async_write(unsigned char bus_id, unsigned char slv_addr,
        gx_hal_i2c_master_transmit_dma_start(i2c, slv_addr);
 
        dw_dma_channel_config(handler->dma_channel, &handler->tx_dma_config);
-       dw_dma_register_complete_callback(handler->dma_channel, gx_i2c_dma_tx_callback, handler);
-       dw_dma_xfer_int((void *)((uintptr_t)i2c->regs + DW_IC_DATA_CMD), (void *)buf, len - 1, handler->dma_channel);
+//     dw_dma_register_complete_callback(handler->dma_channel, gx_i2c_dma_tx_callback, handler);
+       dw_dma_xfer_poll((void *)((uintptr_t)i2c->regs + DW_IC_DATA_CMD), (void *)buf, len - 1, handler->dma_channel);
+       printf("async write finish.\n");
+       gx_hal_i2c_master_transmit_dma_finish(i2c);
+       dw_dma_release_channel(handler->dma_channel);
+       handler->dma_channel = -1;
+       printf("%s\n", __func__);
 
        return 0;
 
@@ -696,12 +701,19 @@ static int i2c_dw_async_read_start(struct gx_i2c_handler *handler, unsigned char
        dcache_flush();
 #endif
 
-       dw_dma_register_complete_callback(handler->dma_channel, gx_i2c_dma_rx_callback, handler);
+//     dw_dma_register_complete_callback(handler->dma_channel, gx_i2c_dma_rx_callback, handler);
        len = len > i2c->rx_fifo_depth ? i2c->rx_fifo_depth : len;
        dw_dma_channel_config(handler->dma_channel, &handler->rx_dma_config);
-       dw_dma_xfer_int(buf, (void *)((uintptr_t)i2c->regs + DW_IC_DATA_CMD), len, handler->dma_channel);
 
        gx_hal_i2c_master_receive_dma_start(i2c, slv_addr);
+       dw_dma_xfer_poll(buf, (void *)((uintptr_t)i2c->regs + DW_IC_DATA_CMD), len, handler->dma_channel);
+       printf("async read finish.\n");
+
+       gx_hal_i2c_master_receive_dma_finish(i2c);
+       printf("%s len:%u\n", __func__, len);
+               dw_dma_release_channel(handler->dma_channel);
+               handler->dma_channel = -1;
+
        return 0;
 }
```

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

### 【已解决】问题 1：对接 I2C 设备小板经常写入出错
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


### 【已解决】问题 2：缺少 FAST PLUS 模式 1MHz 频率读写测试
- 使用 `scpu hal` 那套驱动来读写，对接 eeprom m24c32 正常
```diff
--- a/bootmenu.c
+++ b/bootmenu.c
@@ -2974,7 +2974,9 @@ void i2c_auto_test(int argc, const char **argv)
        }
 
        bus_id = atoi(argv[2]);
-       i2c_dev = gx_i2c_open(bus_id);
+       gx_i2c_set_speed(bus_id, 1000);
+
+       i2c_dev = gx_i2c_open1(bus_id);

                /* EEPROM M24C32 */
                printf("start test EEPROM M24C32\n");
                chip_addr = 0x50;
                reg_addr_width = 2;
-               if (gx_i2c_set(i2c_dev, bus_id, chip_addr << 1, reg_addr_width, 1, 0) != 0) {
-                       printf("set i2c %d failed\n", bus_id);
-                       goto i2c_auto_test_exit;
-               }
+//             if (gx_i2c_set(i2c_dev, bus_id, chip_addr << 1, reg_addr_width, 1, 0) != 0) {
+//                     printf("set i2c %d failed\n", bus_id);
+//                     goto i2c_auto_test_exit;
+//             }
                reg_addr = 0x00;
                tx_data[0] = 0x33;
-               data_len = 1;
-               if (gx_i2c_tx(i2c_dev, reg_addr, tx_data, data_len) != data_len) {
+               tx_data[1] = 0x44;
+               tx_data[2] = 0x55;
+               data_len = 3;
+               if (gx_i2c_tx1(bus_id, chip_addr, reg_addr, reg_addr_width, &tx_data[0], data_len)) {
                        printf("write i2c chip_addr 0x%02x reg 0x%02x failed\n", chip_addr, reg_addr);
                        goto i2c_auto_test_exit;
                }
                mdelay(10);
-               if (gx_i2c_rx(i2c_dev, reg_addr, rx_data, data_len) != data_len) {
+               if (gx_i2c_rx1(bus_id, chip_addr, reg_addr, reg_addr_width, &rx_data[0], data_len)) {
                        printf("read i2c chip_addr 0x%02x reg 0x%02x failed\n", chip_addr, reg_addr);
                        goto i2c_auto_test_exit;
                }
@@ -3150,7 +3154,7 @@ void i2c_auto_test(int argc, const char **argv)
        printf("finish i2c test\n");
 
 i2c_auto_test_exit:
-       gx_i2c_close(i2c_dev);
+       gx_i2c_close1(i2c_dev);
        return ;
```





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


