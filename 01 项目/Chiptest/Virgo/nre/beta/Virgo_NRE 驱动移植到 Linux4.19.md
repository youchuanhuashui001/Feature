现有的代码在 Linux4.9. y 版本，由于 Android 需要移植到 Linux4.19 版本，后续可能还会移植到更高版本。
记录各外设模块移植思路以及方式方法。

# 整体梳理
- flash_spi 模块涉及 spi、flash 驱动
	- spi 驱动依然采用 4.9. y 版本的 spi 驱动，对于版本升级带来的影响，使用宏来进行区分
	- flash 驱动依然采用 gxmisc 生成库的方式来进行提供
		- 目前 android 仓库中的 platfomr 不包含 gxmisc 仓库，先直接使用 develop2 分支编译出来的 lib 临时使用
- gen_spi 模块
	- master，共用 flash_spi 模块驱动，这里需要考虑需求，gen_spi 需要对接 axi-dma
	- slave，共用 flash_spi 模块驱动，在设备树定义 slave 时则定义为 slave
- axi-dma 模块
	- 之前一直过一版 6. x 版本的 axi-dma 驱动已经通了，先对比看看差异，再考虑是直接用 4.19 的还是高版本的
	- 需要考虑对接其它的外设，uart、i2c、gen_spi、saradc
- wdt 模块
	- wdt 之前好像没有 linux 驱动，可以直接用 linux 中自带的 wdt 驱动，调通即可
- uart 模块
	- 移植 4.9. y 版本的 uart 驱动
	- 考虑对接 axi-dma
- i2c 模块
	- 好像已经把 4.9. y 版本的 i2c 驱动移到 4.19 了，确认下
	- 考虑对接 axi-dma
- time 模块
	- 自研的 timer 模块，支持定时 timer 以及输出 pwm 功能
	- 需要考虑设计驱动
- rtc 模块
	- rtc 模块好像 4.19 也没有驱动，从 github 上找找
	- https://github.com/alifsemi/zephyr_alif/commit/d072c8982df9ca21e5e00cedcf7f39fffddb2649#diff-a1c4a22eb27c07448c51adaa3216820d0735a9bd7e6be81778d7f85e637b1aee
- gpio 模块
	- 移植 4.9. y 版本的 gpio 驱动
- ir 模块
	- 移植 4.9. y 版本的 ir 驱动




# 【已完成】Flash_SPI 模块
- SPI 驱动：
	- 目前包括 dw_spi_v2. c，dw_spi. c，gx_spi. c，三套驱动，canopus、vega、virgo 用的都是 dw_spi_v2. c
- flash 驱动：
	- spinor：
		- devices 目录下有一套，用的是 gxmisc 中的库
	- spinand：
		- spi-nand 目录下放的有一套
		- spinand 目录下放的有一套，这套是用的 gxmisc 的中的库




## spi 驱动
- configs
	- CONFIG_SPI=y
	- CONFIG_SPI_DESIGNWARE=y
- 设备树
```diff
+       spi0: spi@f8400000 {
+               compatible = "nationalchip-dw-spi,v2";
+               #address-cells = <1>;
+               #size-cells = <0>;
+               reg = <0x00 0xf8400000 0x00 0x1000>;
+               cs-reg = <0xfa490080>;
+               spi-mode-reg = <0xfa490084>;
+               interrupts = <GIC_SPI 61 IRQ_TYPE_LEVEL_HIGH>;
+               num-cs = <4>;
+               bus-num = <0>;
+               reg-io-width = <2>;
+               interrupt-parent = <&gic>;
+        clocks = <&sysclk>;
+        clock-names = "ssi_clk";
+               spi_nand@0 {
+                       compatible        = "spinand";
+                       spi-max-frequency = <10000000>;
+                       reg               = <0>;
+                       status            = "okay";
```
- Makefile
```diff
--- a/drivers/spi/Makefile
+++ b/drivers/spi/Makefile
@@ -9,6 +9,7 @@ ccflags-$(CONFIG_SPI_DEBUG) := -DDEBUG
 # config declarations into driver model code
 obj-$(CONFIG_SPI_MASTER)               += spi.o
 obj-$(CONFIG_SPI_MEM)                  += spi-mem.o
+obj-$(CONFIG_SPI_DESIGNWARE)           += spi-dw-gx-v2.o
 obj-$(CONFIG_SPI_SPIDEV)               += spidev.o
 obj-$(CONFIG_SPI_LOOPBACK_TEST)                += spi-loopback-test.o
```
- 源码
	- spi-dw-gx-v2. c
	- dw-spi-v2. h


## flash 驱动
- spinor:
	- gxmisc 编译 `./build arm64 linux` 生成 `libarm64.fw`
	- configs
		```
		CONFIG_MTD=y
		CONFIG_MTD_CMDLINE_PARTS=y
		CONFIG_MTD_M25P80=y
		CONFIG_MTD_SPI_NOR=y
		# CONFIG_MTD_SPI_NOR_USE_4K_SECTORS is not set
		```
	- dts：不需要
	- Makefile
		- 依赖 CONFIG_MTD_M25P80，在编译 m25p80 时，依赖 gxspinor. o
- spinand:
	- 目前 rom 不支持 spinand 启动，等后续支持之后才可以验证


## 进度：Nor 验证完成，rom 不支持 SPINand 启动，还没开始验证
- mtd_debug 能够正常读写 nor flash：
	- 四线 cpu
	- 四线 dma
	- 双线 cpu
	- 双线 dma




## 问题

### 【已解决】使用 W25Q128 在 Uboot 阶段识别到 flash 之后，就卡死了，换成 XM25Q128B 就好了，但是网络不通
- 使用最新 gxloader 编译出来的. boot 可以使用网络烧录，说明网络是通的。但是 uboot 还是不通，使用林文杰提供的 uboot 可以启动了，需要切换到 100M Phy。


### 【已解决】设备树 address-cells、size-cells 发生变化，设备树更新
- 新的设备树采用 2 字节的 address 和 2 字节的 size，因此地址和大小都是 64 位
- 则 reg 属性编写为 `<0x00 0xf8400000 0x00 0x1000>` 这样就是用 64 位表示了

### 【已解决】cmdline 中添加了 mtd 信息，但是 linux 启动之后并没有解析 mtd 分区
- 对比 4.9. y 和 4.19 版本的 .config 发现没有定义 `CONFIG_MTD_CMDLINE_PARTS`
	- 加了之后发现无法启动，是不是和 `CONFIG_MTD_OF_PARTS` 只能 2 选一？
	- 只开 CMDLINE_PARTS 也是卡死
- 发现没有 mtd 设备，看驱动 parse_mtd_partitions 是如何解析的？可能没解析到分区
	- 修改 uboot 启动时的 bootargs 来显示 pr_debug 的打印
		- 修改成 pr_info 也看不到，在 gxmisc 里面修改代码也不会改变
- 感觉是 uImage 没成功的更新到内存，重新编了一遍 uImage，并且 fpga 断电了之后可以正常读到 MTD_PARTITION
- 感觉 fpga 就不更新 uImage 啊
	- 复位 fpga 可行吗，也不行
	- 重新启动 fpga，也不行说明生成的 uImage 可能有问题
		- 修改 gxmisc 然后更新 lib 后发现 uImage 只有头部的几个字节发生了变化
		- 是的，看起来好像确实是 uImage 没有更新
		- **看编译 log，都没有重新打包 uImage，因此需要手动删除一下 build-in. a**
```
make -C /home/tanxzh/goxceed/android/build/../linux/common LOCALVERSION= CROSS_COMPILE=" /home/tanxzh/goxceed/android/build/../toolchains/aarch64/bin/aarch64-linux-gnu-" ARCH=arm64 Image
make[1]: 进入目录“/home/tanxzh/goxceed/android/linux/common”
  CALL    scripts/checksyscalls.sh
  CHK     include/generated/compile.h
cp ./drivers/mtd/devices/gxspinor_arm64.fw drivers/mtd/devices/gxspinor.o
make[1]: 离开目录“/home/tanxzh/goxceed/android/linux/common”




make -C /home/tanxzh/goxceed/android/build/../linux/common LOCALVERSION= CROSS_COMPILE=" /home/tanxzh/goxceed/android/build/../toolchains/aarch64/bin/aarch64-linux-gnu-" ARCH=arm64 Image
make[1]: 进入目录“/home/tanxzh/goxceed/android/linux/common”
  CALL    scripts/checksyscalls.sh
  CHK     include/generated/compile.h
  CC      drivers/mtd/mtdpart.o
cp ./drivers/mtd/devices/gxspinor_arm64.fw drivers/mtd/devices/gxspinor.o
  AR      drivers/mtd/built-in.a
  AR      drivers/built-in.a
  GEN     .version
  CHK     include/generated/compile.h
  UPD     include/generated/compile.h
  CC      init/version.o
  AR      init/built-in.a
  AR      built-in.a
  MODPOST vmlinux.o
WARNING: modpost: Found 1 section mismatch(es).
To see full details build your kernel with:
'make CONFIG_DEBUG_SECTION_MISMATCH=y'
  KSYM    .tmp_kallsyms1.o
  KSYM    .tmp_kallsyms2.o
  LD      vmlinux
  SORTEX  vmlinux
  SYSMAP  System.map
  OBJCOPY arch/arm64/boot/Image
make[1]: 离开目录“/home/tanxzh/goxceed/android/linux/common”
```


### 【已解决】执行 mtd_debug comtest 时出现 Receive FIFO overflow
Linux 中的 spi 起来之后回去 spi-mode-reg 读模式，但是由于往 spi-mode-reg 写模式是在 stage2 中做的，现在没跑 stage2，跑的是 rom->stage1->uboot 的流程，所以寄存器没有值。
手动配置模式为 DUAL|QUAD 之后发现读写返回 -12，是 spi 驱动中返回的
- 使用 cpu 模式传很少的数据没问题，使用 dma 有问题，返回 -12 
	- -12 在错误中为 ENOMEM，是 vmalloc_to_page 接口返回出错
	- cpu 读写 ok
- dma 返回 -12 是 NOMEM，使用接口 dma_map_single 分配回来的 dma_handle 为 0
	- dma-mapping. c 中有三套 struct dma_map_ops，分别在 map_page 添加打印查看
	- 发现是错误的进到了 dummy_dma_ops，由于传递给 dma_map_single 函数的参数错误，用的是 spi_master->dev，实际应该使用 platform_device->dev
	- [[dma_map_single 返回物理地址为 0]]





### U-boot 识别到 flash 之后卡死了
```
U-Boot 2023.07.02-00005-gaf58e70986 (Apr 11 2025 - 09:52:17 +0800)

DRAM:  1.8 GiB
Core:  25 devices, 14 uclasses, devicetree: board
Loading Environment from SPIFlash... jedec_spi_nor flash@0: unrecognized JEDEC id bytes: 20, 60, 18
*** Warning - spi_flash_probe_bus_cs() failed, using default environment

In:    serial@fc880000
Out:   serial@fc880000
Err:   serial@fc880000
Net:
Warning: eth0@fd000000 (eth0) using random MAC address - f2:c7:9d:a0:6a:84
eth0: eth0@fd000000
Hit any key to stop autoboot:  0
Failed to load 'uImage'
```

```
U-Boot 2023.07.02-00004-g5348f4bad7-dirty (Jun 12 2025 - 19:19:47 +0800)

DRAM:  1.8 GiB
Core:  25 devices, 14 uclasses, devicetree: board
Loading Environment from SPIFlash... SF: Detected XM25QH128B with page size 256 Bytes, erase size 64 KiB, total 16 MiB
```

- 已完成 https://git.nationalchip.com/gerrit/#/c/126533/



# Watchdog 模块
- configs
	- CONFIG_WATCHDOG=y
	- CONFIG_WATCHDOG_CORE=y
	- CONFIG_DW_WATCHDOG=y
- makefile
	- 自带的驱动，不需要更改
- code
	- watchdog_core. o
	- watchdog_dev. o
	- dw_wdt. c
- test_code
	- tools/testing/selftests/watchdog/watchdog-test. c




# 【已完成】I2C 模块
- configs
	- CONFIG_I2C_DESIGNWARE_CORE=y
	- CONFIG_I2C_DESIGNWARE_PLATFORM=y
- makefile
	- 自带的驱动？
- code
	- i2c-designware-core. c
	- i2c-designware-common. c
	- i2c-designware-master. c
	- i2c-designware-platdrv. c
- test_code
	- goxceed/develop2/platform/gxtools/test_cases/i2c-tools
- 这套驱动支持 100k、400k、1MHz、3.4MHz，但是没有 gx 的套接层 i2c-gxlayer. c

- 已完成 https://git.nationalchip.com/gerrit/#/c/127464/


# 【已完成】IR 模块

- configs
	- CONFIG_GX_IRR=y 默认是 y
- makefile
	- 已经有了
- code
	- gx_irr. c
	- gx_irr_pt2264. c


存在两套 dtb：
- zynq 一套 dtb
	- 之前用 zynq 跑的
- arch/arm64/boot/dts/nationalchip/ 
	- 缺少 keymap，不放也可以，这个只是根据键值和实际按键的映射
	- 编译方法，在编 linux Image 的时候加上 dtbs 选项就会编出来 `arch/arm64/boot/dts/nationalchip/virgo-mpw-evb.dtb`，使用这个 dtb 烧进去就可以了

测试代码：
- platform/gxtools/test_cases/irrtest/irr_test. c
- 编译：`arm-linux-uclibcgnueabihf-gcc -static -o irr.elf irr_test.c`


- 已完成 https://git.nationalchip.com/gerrit/#/c/127044/



# 【已完成】AXI-DMA
- 已完成 https://git.nationalchip.com/gerrit/#/c/127464/

