
## Leo_mini、Leo、Apus 获取 block_size 实际为 4K
- `apus、leo_mini`  等芯片的驱动在使用接口 ` getinfo(BLOCK_SIZE) ` 获取 `block_size` 时获取到的 `block_size`  为 `4k` ，而不是 `64k` 



## 各芯片 Flash 不同阶段工作频率

### Leo_mini
- ROM：3MHz
- Stage1：50MHz (读 ID 的时候 25MHz)
- Stage2：50MHz
- 备注：
	- Stage1、Stage2 同频，目前可配置为 200MHz、150MHz、100MHz。
	- 目前配置为 200Mhz，4 分频，相位范围：1~4
	- clk 在 stage1 配置，所以 stage1 读 flash 的时候就已经是高频






## Apus 配置 P2_0 ~ P2_5 为通用 SPI 功能
- Apus 的 `Gen SPI` 可以配置成 `Master/Slave`，并且分别是两组引脚
```diff
diff --git a/board/nationalchip/apus_nre_1v/pinmux.c b/board/nationalchip/apus_nre_1v/pinmux.c
index 67a02638..429bd85f 100644
--- a/board/nationalchip/apus_nre_1v/pinmux.c
+++ b/board/nationalchip/apus_nre_1v/pinmux.c
@@ -3,6 +3,8 @@
 #include <gx_clock.h>
 #include <misc.h>
 
+#define CONFIG_VIRGO_TEST_MASTER
+
@@ -57,12 +59,21 @@ static const GX_PIN_CONFIG pin_table[] = {
     /* {13,  2  }, // P1_5 */
     {14,  33 }, // P1_6
     {15,  6  }, // P1_7
-    {16,  29 }, // P2_0
-    {17,  10 }, // P2_1
-    {18,  97 }, // P2_2
-    {19,  98 }, // P2_3
-    {20,  13 }, // P2_4
-    {21,  14 }, // P2_5
+#ifdef CONFIG_VIRGO_TEST_MASTER
+    {16,  62 }, // P2_0
+    {17,  71 }, // P2_1
+    {18,  72 }, // P2_2
+    {19,  73 }, // P2_3
+    {20,  74 }, // P2_4
+    {21,  75 }, // P2_5
+#else
+    {16,  76 }, // P2_0
+    {17,  77 }, // P2_1
+    {18,  78 }, // P2_2
+    {19,  79 }, // P2_3
+    {20,  80 }, // P2_4
+    {21,  81 }, // P2_5
+#endif
     {22,  0  }, // P2_6
     {23,  50 }, // P2_7
     {24,  51 }, // P3_0
```




# Apus、Aquila、Fornax 等芯片 Stage1 会读 ID，Stage1 用的是四倍速读 flash
- `arch/riscv/mach-apus/spl/spl_spinor.c` 中初始化 flash 时会读 id，并使能 quad，以及读接口是用的 quad 模式。xip 模式也是四线模式 
- Aquila 不知道 Stage1 有没有 QUAD 的宏，是不是用的四线
- Fornax Stage1 用的是四倍速 



# bootx 工具烧写 grus 比烧写 apus、fornax 快

> [!Note] 
>  -  grus、apus/fornax 使用相同的 flash p25q80l，发现 grus 比 apus/fornax 烧写速度快
>  - 原因是 grus 在烧写的时候做了策略，uart 收数据的时候采用 dma 异步收数据，边收边写 flash。而 apus/fornax 没有做这个策略，所以会等 uart 数据收完之后再去写 flash，所以会慢。



- 使用 bootx 工具烧写 grus 约 940K 的数据，耗时约 12ms
```
-> % time ./bootx_debug_speed -m grus -t s -c download 0 ../.boot/222 -d /dev/ttyUSB1 -r 1000000 
Version : v1.10.5-debug_2 (20250107)
NationalChip AIoT Download Tools
Copyright (C) 2001-2024 NationalChip Co., Ltd
ALL RIGHTS RESERVED!

wait ROM request... please power on or restart the board...
Using Grus boot
Found serial: /dev/ttyUSB1
downloading [1/2] : 
[==========][100%]
downloading [2/2] : 
[==========][100%]
send boot coust : 4884 ms
Excute cmd : download 0 ../.boot/222
serialdown 0 960660 51200
Flash JEDEC = 0x856014, model = p25q80l.
spi_nor_calc_range get SR1:0x0, SR2:0xa 
protect len: 0x0
downloading [1/19]

[==========][100%]
done
cmd coust : 11189 ms
./bootx_debug_speed -m grus -t s -c download 0 ../.boot/222 -d /dev/ttyUSB1 -  0.20s user 0.30s system 3% cpu 16.080 total
```
- 使用 bootx 工具烧写 apus、fornax 同样大小的 bin，耗时约 20ms
```
-> % time ./bootx_debug_speed -m fornax -t s -c download 0 ../.boot/222 -d /dev/ttyUSB0 -r 1000000 
Version : v1.10.5-debug_2 (20250107)
NationalChip AIoT Download Tools
Copyright (C) 2001-2024 NationalChip Co., Ltd
ALL RIGHTS RESERVED!

Baudrate adaptive interaction completed
prepare cost : 1405 ms
downloading [1/2] : 
[==========][100%]
stage1 cost : 441 ms
downloading [2/2] : 
[==========][100%]
stage1 cost : 687ms
----------------- Excute Command -----------------
[CMD] String : download 0 ../.boot/222
[==========][100%]
[CMD] Result : SUCCESS
stage1 cost : 17563
./bootx_debug_speed -m fornax -t s -c download 0 ../.boot/222 -d /dev/ttyUSB0  0.11s user 0.31s system 2% cpu 20.098 total
```




# Sagitta

## Cache
- 只有 L1 Icache、Dcache
- Flash XIP 会用到 L2 Icache，flash 读写擦不会过 L2 Icache
- L2 Dcache 本来准备用于 Psram，后来 L2 Dcache 有问题，就去掉了，现在没有 L2 Dcache

## IR
- tx/rx 是独立的 buffer，深度均为：16，位宽均为：32








# Apus Flash 跑极限频率 104MHz
- 修改 `board/nationalchip/apus_nre_1v/clk.c` 后 `stage1` 的频率修改了吗？还是只修改了 `stage2` 的时钟频率？
	- 没改，所以 stage1 原本用的是宏，现在都需要手动改成 1



# Flash 支持时需要用到的某些补丁：

- Apus 测试 flash 跑极限频率代码修改
	- https://git.nationalchip.com/gerrit/#/c/112577
- Apus 测试 10ms 休眠唤醒
	- `scpu` 中使用 ` apus_nre_xip_1v_deconfig ` 编译，打开 ` apus pmu test ` 测试选项
	- `main.c` 中将 ` pmu_reboot_for_10ms_test ` 函数 `extern` 过来，并在 `app_start` 之前调用
- leo_mini flash 跑 130Mhz，时钟代码修改
	- https://git.nationalchip.com/gerrit/#/c/123165/
- fornax 使用 psram spi 对接 flash 测试
	- https://git.nationalchip.com/gerrit/#/c/124025/2



# K7 FPGA 对应 FMC 子板原理图
- 老的子板： http://yun.nationalchip.com/v/list/ent/1483267603295113268
- 新的子板： https://git.nationalchip.com/redmine/issues/399831


# Fornax BBT 子板调试
- 环境搭建： https://git.nationalchip.com/redmine/issues/397618
- 包含 3 款芯片，Apus、Fornax、Leo_mini
	- Fornax：待测芯片
	- Leo_mini：用于启动 Fornax，Fornax 的程序是放在 Leo_mini 中的，当 Leo_mini 启动的时候会把 Fornax 的程序发到 Fornax 的内存，然后跑起来
	- Apus：用于与 Fornax 交互，Fornax 跑什么测试程序是由 Apus 决定的；Apus 与 Fornax 的串口是通的
- 可以直接把 Fornax 连接 Leo_mini 的串口跳帽拔掉，插到外接的串口上，这样直接连上 Fornax 的串口和 Jtag，就可以跑 SCPU 的代码

