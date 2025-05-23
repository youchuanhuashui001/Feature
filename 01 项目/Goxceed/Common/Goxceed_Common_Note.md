

## Common

### nand flash 工厂如何烧录镜像
- 需要用到 `2` 个工具：
	- `gxtools/bin_utils`：
		- 输入分区表 `flash.conf` 以及各个独立的文件
		- 输出：按照 `flash.conf` 以及独立的文件合并后的整 `bin` + 烧录器要用的 `.def` 分区表
	- `gxtools/nand_ecc_tool`：
		- 输入整 `bin`
		- 输出带 `ecc` 的整 `bin`
		- 由于并行 `nand` 依赖我们控制器来生成，在烧录 `bin` 时会往 `oob` 区域写上对应的 `ecc`，而工厂的烧录器用的不是我们的控制器，自然也就不会为我们的 `oob` 区域写上对应的 `ecc`，所以需要我们将整 `bin` 中的数据读出来，计算出 `ecc` 填充到 `oob` 区域，这样工厂烧录时就可以直接将带 `ecc` 的 `bin` 烧到 `flash` 
- 工厂按照 `.def` 文件来烧录，需要按照分区 `(.def)` 烧
- 还有一个问题：烧录器怎么跳坏块，和我们相同的吗？统一的跳坏块方法都是这样的吗？
	- 都是一个 block 一个 block 来操作的，统一的跳到下一个 block 的首地址
	- 跳坏块的方法都是一样的 
	- 因为 `nand flash` 在擦除或写入单个 `block` 时可能会产生坏块，所以只能按照 `block` 为单位操作 `nand flash` 
https://git.nationalchip.com/redmine/issues/352665#note-3


## VU440 使用方法
- 替换 `USRDATA` 分区内的 `Fpga1.bin`
- 有三个按键，最左边的按键是复位按键。最右边的 (靠近 fpga 风扇) 的按键不能按，按了之后串口就挂了


### A55 如何开启 GDBServer
- `JLinkGDBServer -endian little -select USB -device Cortex-A55 -if JTAG -speed 10000`
- 直接执行即可，因为 `JLinkGDBServer` 安装到了 `usr/bin` 目录下



## Gxloader

### Gxloader 如何关 Icache、Dcache、MMU？
- 参考 `boot.c` 中的接口 `boot_close_module()` 接口，其中包含了这些接口

> [!tips]  `virgo`  芯片验证时测试可以关掉 `Icache、Dcache`  运行，但是不能关闭 `mmu`  运行 


### Gxloader 不同大小的 DDR 可以通用驱动吗？
>  背景：事业部给了一个 `6607H2 xxx .boot ` 在 ` loader ` 中找不到对应的板级，只有相似的一个板级 `6607H1 xxx`，可以用 `H1 .boot` 在客户的板子上。

- 一般后缀名 `2/1` 代表的是 `ddr` 的容量是 `2G/1G bit`
- 不同大小的 `ddr` 相位一般是可以通用的，普通测试没什么问题，高低温测试或拷机会有问题
-  **对于内置和外置的不可通用，因为内置的走线短，外置的走线长，相位不一致**


### Gxloader 中的 `.boot` 如何执行？
> 背景：想要直接跑 `flash` 的测试代码，不跑从 `flash` 启动的那套流程

- 直接跑 flash 的测试程序：**这里的程序不是从 flash 加载的，而是用 boot 工具拿到的** 
	- 先将 `.boot 的 stage1` 灌到 `sram`，`.boot 的 stage1` 会初始化 `ddr、时钟树` 等等  
	- 然后将 `.boot 的 stage2` 灌到 `ddr`，`.boot 的 stage2` 起来再执行命令 
		![[Pasted image 20241204142355.png]] 

### Gxloader 不打开 `DEBUG` 选项编译出来的 `.elf` 为什么不能直接跑起来？
> 背景：想要直接跑 `flash` 的测试代码，不跑从 `flash` 启动的那套流程，想直接把 `.elf` 用 `gdb` 加载到内存跑起来 

- `.elf` 中有所有的信息，包含 `Stage1、Stage2、符号表等等`
- `.bin` 就是根据 `.elf` 再进行处理，`objcopy` 成二进制，去掉 `bss 段、mmu_table 段` 等等得到的 
- 为什么不打开 `DEBUG` 宏，直接加载 `.elf` 无法启动？
	- 因为 `Stage1` 里面会处理，如果没开 `DEBUG` 宏就会从 `flash` 拷贝 ` Stage2 `
	- 所以直接 `load` 的话，还是会从 `flash` 启动，而不是从 `.elf` 拿 `Stage2`




### Gxloader 中 Stage2 的代码会访问 SRAM 吗？
> 背景：`Virgo SPINand`  开中断后直接灌 `.boot`  到内存，跑到 `Stage2`  的 `flash`  测试代码发现访问了 `SRAM` 

- 查看链接脚本：
	```linkscript
	MEMORY
	 {
	  stage1 : ORIGIN = 0xf0400000, LENGTH = 0x4000 - 32
	  mmutable : ORIGIN = 0xf0408000, LENGTH = 0x8000
	  stage2 : ORIGIN = 0x07b40000, LENGTH = 0xc0000
	 }
	 ENTRY(ResetEntry)
	 SECTIONS {
	  . = ALIGN(4);
	  .text :
	  {
	   _stage1_start_ = .;
	   cpu/arm/virgo/virgo_start.o(.text*)
	   cpu/arm/virgo/virgo_pll_mini.o(.text*)
	   cpu/arm/virgo/virgo_pll_full.o(.text*)
	   cpu/arm/virgo/virgo_sdram.o(.text*)
	   cpu/arm/cpu/armv7/cache-cp15.o(.text*)
	   cpu/copy.o(.text*)
	   common/secure/common.o(.text*)
	   cpu/arm/virgo/virgo_start.o(.rodata*)
	   cpu/arm/virgo/virgo_pll_mini.o(.rodata*)
	   cpu/arm/virgo/virgo_sdram.o(.rodata*)
	   cpu/arm/cpu/armv7/cache-cp15.o(.rodata*)
	   cpu/copy.o(.rodata*)
	   common/secure/common.o(.rodata*)
	   cpu/arm/virgo/virgo_start.o(.data*)
	   cpu/arm/virgo/virgo_pll_mini.o(.data*)
	   cpu/arm/virgo/virgo_pll_full.o(.data*)
	   cpu/arm/virgo/virgo_sdram.o(.data*)
	   cpu/arm/cpu/armv7/cache-cp15.o(.data*)
	   cpu/copy.o(.data*)
	   common/secure/common.o(.data*)
	   *(.cache_section)
	   *(.reset_patch)
	   *(.sram_text)
	   *(.sram)
	   _stage1_end_ = .;
	  } > stage1
	  .mmu_tables :{. = ALIGN(0x4000); *(.mmu_tables) } > mmutable
	```
- 发现  `Stage1` 的代码都是跑在 `SRAM` 上的， `Stage2` 的代码都是跑在 `DDR` 上的
- 现在在跑 `flash` 的测试程序，那么应该不会访问 ` SRAM ` 才对
	- `Stage2` 中的 ` cache ` 相关代码放在 ` sram `
	- 中断向量表相关代码放在 `Start.S` 中，所以也是在 `sram`
	- 现在没有用到 `dma `，所以也不会处理 ` cache ` 相关操作，但是用了 ` irq ` 所以会用到中断向量表，所以 ` Stage2 ` 代码会访问 ` sram ` 


> [!tips]
> - <font color="#245bdb">具体什么代码跑在哪里，依据链接脚本 </font> 



### Canopus 3215B nand flash 掉电测试脚本找不到字符串 Hit any key to stop autoboot
- 默认 `Canopus 3215B` 的配置文件中配置 `bootdelay = 0`，所以不会显示这个字符串
- 手动修改 `conf/canopus/3215B/debug.conf` 中的 `CONFIG_BOOTDELAY = 3` 即可 


### GoXceed 3.2.1、3.2.2 都是特殊的分支，loader 会烧不进去
- 如果要测试此分支的 `ecos、linux` 可以用其它分支的 `loader` 烧进去
- 如果要在此分支上加 `flash`，可以用 `.boot` 来测试 


### 为什么 loader 需要打开 UBIFS 的宏，否则 Linux kernel 在启动的时候不会去挂载 ubi 卷？
- loader 在启动 Linux 的时候会根据这个宏来传递给 linux 的 cmdline
```c
	/* if no root=, add automatically */
	if (strstr(params->u.cmdline.cmdline, "root=") == NULL) {
		int len = 42;
		char root[len];
		memset(root, 0, len);
		struct partition_info *p = all_partition_get("rootfs");
		if (p == NULL)
			p = all_partition_get("root");
		if (p) {
			flash_type = flash_get_type();
			rootfs_mtdid = get_rootfs_mtd_id(p, flash_type);
#ifdef CONFIG_ENABLE_ROOTFS_UBIFS
			sprintf(root, "ubi.mtd=%d", rootfs_mtdid);
			cmdline_add(root);
			sprintf(root, "root=ubi0_0");
			cmdline_add(root);
			sprintf(root, "rootfstype=%s", p->file_system_type ? get_fstype(p->file_system_type) : "ubifs");
			cmdline_add(root);
#elif defined(CONFIG_ENABLE_ROOTFS_INITRD)
			sprintf(root, "initrd=0x%x,0x%0x", INITRD_DRAM_START_ADDR, INITRD_DRAM_SIZE);
			cmdline_add(root);
			sprintf(root, "root=/dev/ram rw");
			cmdline_add(root);
#else
			sprintf(root, "root=/dev/mtdblock%d", rootfs_mtdid);
			cmdline_add(root);
#endif
		}
	}
```



### GoXceed v1.9.6.4-lts 分支 gdbserver 连不上？
- 手动打开 gdb 的引脚复用后可以了
```diff
--- a/board/gx3211/board-6622-dvbs2/board-init.c
+++ b/board/gx3211/board-6622-dvbs2/board-init.c
@@ -92,11 +92,11 @@ struct mulpin_config_s mulpin_table[] = {
         {88,        75,     107,    MP_INV_V,  1},           // NC          | SC2RST_PORT43
         {90,        76,     108,    MP_INV_V,  1},           // NC          | SC2DATC8_PORT44
         {92,        77,     109,    MP_INV_V,  1},           // NC          | SC2DATC7_PORT45
-        {104,       0,      32,     MP_INV_V,  1},           // 29          | DBGTDI_PORT0
-        {105,       1,      33,     MP_INV_V,  1},           // 30          | DBGTDO_PORT1
-        {106,       2,      34,     MP_INV_V,  1},           // 31          | DBGTMS_PORT2
-        {107,       3,      35,     MP_INV_V,  1},           // 32          | DBGTCK_PORT3
-        {108,       4,      36,     MP_INV_V,  1},           // 33          | DBGTRST_PORT4
+        {104,       0,      32,     MP_INV_V,  0},           // 29          | DBGTDI_PORT0
+        {105,       1,      33,     MP_INV_V,  0},           // 30          | DBGTDO_PORT1
+        {106,       2,      34,     MP_INV_V,  0},           // 31          | DBGTMS_PORT2
+        {107,       3,      35,     MP_INV_V,  0},           // 32          | DBGTCK_PORT3
+        {108,       4,      36,     MP_INV_V,  0},           // 33          | DBGTRST_PORT4
         {110,       128,    132,    MP_INV_V,  0},           // 34          | SPISCK_NFRDY0_NFRDY0_SPISCKDW
         {111,       129,    133,    MP_INV_V,  0},           // 35          | SPIMOSI_NFOE_NFWE_SPIMOSIDW
         {112,       130,    134,    MP_INV_V,  0},           // 36          | SPICSn_NFCLE_NFALE_SPICSnDW
```


### vega 修改 gpio 驱动能力
https://git.nationalchip.com/redmine/issues/385267


### Gxloader 中 arm v8 架构中的 flush cache 接口包括 Clean && Invalid
- `include/cpu_func.h：flush_dcache_all`：包括 clean && invalid




## Linux

### Linux 如何以 O0 方式编译某个 .c 文件
- 移植高版本 `axi-dma` 驱动到 `Linux4.9` 时发现使用 `GDB` 调试某些变量看不到，Linux 默认是以比较高的优化等级编译的，可以单独以 `O0` 来编译某个 `.c` 文件来方便调试
- 通过修改 `Makefile` 实现：
	```Makefile
	CFLAGS_dw-axi-dmac-platform.o = -O0 -g
	obj-$(CONFIG_DW_AXI_DMAC) += dw-axi-dmac-platform.o
	```


### Linux2.6.27.55 编译失败

参考 https://git.nationalchip.com/redmine/issues/398032 进行编译。
- 拷贝 config 作为 .config，然后执行编译命令即可

- 需要使用老的那套编译工具链，在编译 `linux4.9` 或其它版本时用的是一套新的编译工具链
- 修改完 `zshrc` 之后看一下 `gcc` 的版本就可以确认是哪套工具链 
```bash
# ck610 用于 Linux2.6/3.0 内核及应用程序编译
export PATH=${PATH}:/opt/gxtools/csky-linux-tools-i386-uclibc-20170724_cross_compiler/csky-linux-tools-i386-uclibc-20170724/bin
export PATH=${PATH}:/opt/gxtools/csky-linux-tools-i386-uclibc-20180905_cross_compiler/bin
export PATH=${PATH}:/opt/gxtools/csky-linux-tools-i386-uclibc-20180905_cross_compiler/csky-linux/bin

# ck610 用于 Linux4.9 内核及应用程序编译
#export PATH=${PATH}:/opt/gxtools/csky-linux-gnu-tools-i386-glibc-linux-4.9.56-20190605_cross_compiler/csky-linux-gnu-tools-i386-glibc-linux-4.9.56-20190605/bin
#export PATH=${PATH}:/opt/gxtools/csky-linux-gnu-tools-i386-glibc-linux-4.9.56-20190605_cross_compiler/csky-linux-gnu-tools-i386-glibc-linux-4.9.56-20190605/csky-linux-gnu/bin
#export PATH=${PATH}:/opt/gxtools/csky-linux-uclibc-tools-i386-uclibc-linux-4.9.56-20190605_cross_compiler/csky-linux-uclibc-tools-i386-uclibc-linux-4.9.56-20190605/bin
#export PATH=${PATH}:/opt/gxtools/csky-linux-uclibc-tools-i386-uclibc-linux-4.9.56-20190605_cross_compiler/csky-linux-uclibc-tools-i386-uclibc-linux-4.9.56-20190605/csky-linux-uclibc/bin

```

- 编译 video 失败，则注释掉相关的编译，再接着编译
```diff
diff --git a/drivers/media/video/Makefile b/drivers/media/video/Makefile
index 6a2e163e..a75e7171 100644
--- a/drivers/media/video/Makefile
+++ b/drivers/media/video/Makefile
@@ -106,7 +106,7 @@ obj-$(CONFIG_VIDEO_CAFE_CCIC) += cafe_ccic.o
 obj-$(CONFIG_VIDEO_OV7670)     += ov7670.o
 
 obj-$(CONFIG_VIDEO_TCM825X) += tcm825x.o
-obj-$(CONFIG_NATIONALCHIP_DECODER_CORE) += nationalchip/
+#obj-$(CONFIG_NATIONALCHIP_DECODER_CORE) += nationalchip/
 
 obj-$(CONFIG_USB_DABUSB)        += dabusb.o
 obj-$(CONFIG_USB_OV511)         += ov511.o
```




## eCos




# 设备树

### `#address-cells` 和 `#size-cells`
- ​**作用**：定义父节点中子节点 `reg` 属性的地址和大小部分的单元格数量（每个单元格为 32 位）。
- ​**使用场景**：通常出现在总线或父节点中，用于规范子节点的 `reg` 属性格式。
- ​**示例**：
```dts
parent-node {
    #address-cells = <1>;  // 地址部分占 1 个单元格（32 位）
    #size-cells = <1>;     // 大小部分占 1 个单元格（32 位）
    child-node {
        reg = <0x1000 0x200>; // 地址=0x1000，大小=0x200
    };
};
```
- ​**常见值**：
    - `#address-cells = <1>`：地址用 1 个 32 位数表示。
    - `#size-cells = <0>`：无大小部分（如固定大小的寄存器）。
    - `#address-cells = <2>`：地址用 2 个 32 位数（64 位地址）。



# 调试

## canopus-spinand-gdb-linux
`CANOPUS-6631SH1D SPINAND` 板级使用 gdb 加载 linux 调试流程：
- 烧录一个可以正常启动的整 bin
- gxloader：
	- `./build canopus 6631SH1D debug` 编译后生成的 bin 文件烧录到 flash
```diff

```