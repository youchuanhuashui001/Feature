## 调试时用到的命令：

```shell
# A55 打开 JLinkServer
JLinkGDBServer -endian little -select USB -device Cortex-A55 -if JTAG -speed 10000

# FPGA 网络有问题需要配置：
mw 0xF0A06088 0x10000

# 切换至 100M Phy
mw 0xfa4900cc 0x4

# U-boot 设定环境变量，并从 tftp 服务器获取 uImage
setenv tftpserver 192.168.108.149 && dhcp 0x12200000 ${tftpserver}:uImage

# U-boot 设定环境变量，并启动 Linux
setenv fdt_addr 0x8000000 && setenv kernel_addr_r 0x12200000 && setenv ramdisk_addr_r 0x15000000 && setenv bootargs console=ttyS0,115200 earlycon=uart8250,mmio32,0xFC880000 root=/dev/nfs rw nfsroot=192.168.108.149:/opt/nfs/virgo_nre/,v3 ip=192.168.108.198:::255.255.255.0 && bootm ${kernel_addr_r} - ${fdt_addr}



# U-boot 设定环境变量，并启动 Linux

setenv fdt_addr 0x8000000 && setenv kernel_addr_r 0x12200000 && setenv ramdisk_addr_r 0x15000000 && setenv bootargs console=ttyS0,115200 earlycon=uart8250,mmio32,0xFC880000 root=/dev/nfs rw nfsroot=192.168.108.149:/opt/nfs/virgo_nre/,v3 ip=192.168.108.198:::255.255.255.0  mtdparts=m25p80:128k@4m(BOOT),64k@4224k(TABLE),3904k@4288k(MTD_TEST) mtdparts_end
 && bootm ${kernel_addr_r} - ${fdt_addr}
 
 
 
 
 setenv fdt_addr 0x8000000 && setenv kernel_addr_r 0x12200000 && setenv ramdisk_addr_r 0x15000000 && 
 
 
 setenv bootargs console=ttyS0,115200 earlycon=uart8250,mmio32,0xFC880000 root=/dev/nfs rw nfsroot=192.168.108.149:/opt/nfs/virgo_nre/,v3 ip=192.168.108.198:::255.255.255.0  mtdparts=m25p80:128k@4m(BOOT),64k@4224k(TABLE),3904k@4288k(MTD_TEST) mtdparts_end
 && bootm ${kernel_addr_r} - ${fdt_addr}
 
 
 
 
# Virgo_MPW uboot linux4.19
setenv tftpserver 192.168.108.149


setenv bootargs 'console=ttyS0,115200 earlycon=uart8250,mmio32,0xFC880000 videomem=240m@0x61000000 vdpmem=16m@0x60000000 cma=1024M root=/dev/ram'

setenv bootargs 'console=ttyS0,1500000 earlycon=uart8250,mmio32,0xFC880000 videomem=240m@0x61000000 vdpmem=16m@0x60000000 cma=1024M root=/dev/ram'



setenv bootcmd "dhcp ${kernel_addr_r} ${tftpserver}:mpw/Image.lzma && tftp ${ramdisk_addr_r} ${tftpserver}:mpw/rootfs.cpio.my && booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr}" 


setenv bootcmd "dhcp ${kernel_addr_r} ${tftpserver}:mpw/Image.lzma && tftp ${ramdisk_addr_r} ${tftpserver}:mpw/rootfs.cpio.uboot && booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr}" 


setenv ipaddr 192.168.108.217 && setenv serverip 192.168.108.216 && tftp 0x02280000 Image.lzma && setenv bootargs mem=291M@0x00000000 vdpmem=4M@0x12300000 hdcpmem=1M@0x12700000 videomem=600M@0x12800000 vworkmem=128M@0x38000000 console=ttyS0,1500000 earlycon=uart8250,mmio32,0xFC880000 nokaslr rodata=off init=/init root=/dev/nfs rw nfsroot=192.168.108.216:/opt/nfs/rootfs,v3 ip=192.168.108.217:::255.255.0.0 && booti ${kernel_addr_r} - ${fdt_addr}



setenv ipaddr 192.168.108.217 && setenv serverip 192.168.108.216 && tftp 0x02280000 Image.lzma && tftp 0x05000000 rootfs-eth.cpio.uboot && setenv bootargs mem=291M@0x00000000 vdpmem=4M@0x12300000 hdcpmem=1M@0x12700000 videomem=600M@0x12800000 vworkmem=128M@0x38000000 console=ttyS0,1500000 earlycon=uart8250,mmio32,0xFC880000 nokaslr rodata=off init=/init root=/dev/ram && booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr}




setenv bootargs 'console=ttyS0,115200 earlycon=uart8250,mmio32,0xFC880000 videomem=240m@0x61000000 vdpmem=16m@0x60000000 cma=1024M root=/dev/ram mtdparts=m25p80:1m@16m(MTD1),15m@17m(MTD2) mtdparts_end'


setenv bootargs 'console=ttyS0,115200 earlycon=uart8250,mmio32,0xFC880000 videomem=240m@0x61000000 vdpmem=16m@0x60000000 cma=1024M root=/dev/ram mtdparts=m25p80:1m@16m(MTD1),15m@17m(MTD2),3m@32m(MTD3) mtdparts_end'

setenv bootcmd "dhcp ${kernel_addr_r} ${tftpserver}:mpw/Image.lzma && tftp ${ramdisk_addr_r} ${tftpserver}:mpw/rootfs.cpio.uboot && booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr}" 

setenv bootcmd "sf probe && sf read 2280000 1000000 a00000 && sf read 5000000 1a00000 3C7758 && booti ${kernel_addr_r} ${ramdisk_addr_r}:0x3c7758 ${fdt_addr}" 


./boot4 -b virgo-3502-D4-sflash-1500000.boot -c serialdown 0x0 ../../goxceed/platform/gxloader/output/loader-sflash.bin -d /dev/ttyUSB0

./boot4 -b virgo-3502-D4-sflash-1500000.boot -c serialdown 0x40000 ../../gxbin/bin/virgo/mpw_bl31.bin -d /dev/ttyUSB0 -t nns

./boot4 -b virgo-3502-D4-sflash-1500000.boot -c serialdown 0x60000 ../../gxbin/bin/virgo/mpw_bl32.bin -d /dev/ttyUSB0 -t nns

./boot4 -b virgo-3502-D4-sflash-1500000.boot -c serialdown 0x1f0000 virgo.dtb -d /dev/ttyUSB0 -t nns

./boot4 -b virgo-3502-D4-sflash-1500000.boot -c serialdown 0x200000 ../../u-boot/u-boot.bin -d /dev/ttyUSB0 -t nns

./boot4 -b virgo-3502-D4-sflash-1500000.boot -c serialdown 0x1000000 my_Image.lzma -d /dev/ttyUSB0 -t nns

./boot4 -b virgo-3502-D4-sflash-1500000.boot -c serialdown 0x1a00000 my_rootfs.cpio.gz -d /dev/ttyUSB0 -t nns
Image.lzma 放到 flash 的 16M 地址，读到 0x02280000;

sf probe
sf read 2280000 1000000 a00000
sf read 5000000 1a00000 3C7758
booti ${kernel_addr_r} ${ramdisk_addr_r}:0x3c7758 ${fdt_addr}



需要把原来的 rootfs 制作成 cpio，然后压缩
压缩完之后，读到内存，读到内存，启动的时候需要 指定 ramdisl_addr_r:size
                                            0x             0x
rootfs.cpio.uboot 放到 flash 的 xxx 地址，读到 0x05000000;

env save

# DDR4
setenv bootargs 'mtdparts=m25p80:1m@16m(MTD1),15m@17m(MTD2) mtdparts_end'


mount -t nfs -o nolock 192.168.108.149:/opt/nfs /tmp








mkimage -A arm64 \
	-T ramdisk \
	-C gzip \
	-a 0x05000000 \
	-e 0x05000000 \
	-n "Root file system" \
	-d rootfs.cpio.gz rootfs.cpio.uboot

 
```

# 编译 uImage
好的，我们来分析一下当您在当前 `build` 工程目录下执行 `make uImage` 命令时所发生的完整流程。

这个过程主要可以分为三个阶段：

1.  **编译 U-Boot**：主要是为了获取 `mkimage` 工具。
2.  **编译 Linux 内核**：生成内核镜像 `Image` 和 `vmlinux` 文件。
3.  **创建 `uImage`**：使用 `mkimage` 工具将内核镜像打包成 U-Boot 可识别的 `uImage` 格式。

下面是详细的步骤分解，这主要基于您提供的 `build/virgo.mk` 文件（该文件与 `build/Makefile` 内容相同）。

---

### 总览

在 `build/virgo.mk` 文件中，`uImage` 目标是一个伪目标（`.PHONY` target），它依赖于 `$(KERNEL_UIMAGE)` 文件。

```makefile:build/virgo.mk
// ... existing code ...
.PHONY: uImage
uImage: $(KERNEL_UIMAGE)
```

`$(KERNEL_UIMAGE)` 变量定义为 `$(BINARIES_PATH)/uImage`，其中 `$(BINARIES_PATH)` 指向 `$(ROOT)/out/bin` 目录。所以最终目标是生成 `$(ROOT)/out/bin/uImage`。

生成 `$(KERNEL_UIMAGE)` 的规则如下：

```makefile:build/virgo.mk
// ... existing code ...
$(KERNEL_UIMAGE): u-boot linux | $(BINARIES_PATH)
	${AARCH64_CROSS_COMPILE}objcopy -O binary \
					-R .note \
					-R .comment \
					-S $(LINUX_PATH)/vmlinux \
					$(BINARIES_PATH)/linux.bin
	$(MKIMAGE_PATH)/mkimage -A arm64 \
				-O linux \
				-T kernel \
				-C none \
				-a $(KERNEL_LOADADDR) \
				-e $(KERNEL_ENTRY) \
				-n "Linux kernel" \
				-d $(BINARIES_PATH)/linux.bin $(KERNEL_UIMAGE)
// ... existing code ...
```

从这个规则可以看出，为了生成 `uImage`，`make` 必须首先完成两个前置依赖任务：`u-boot` 和 `linux`。

---

### 阶段一：编译 U-Boot (`u-boot` 目标)

`uImage` 的生成需要 `mkimage` 工具，这个工具是在编译 U-Boot 的过程中产生的。

1.  **配置 U-Boot**：
    *   `u-boot` 目标依赖 `u-boot-defconfig`，后者又依赖 `$(UBOOT_PATH)/.config` 文件。
    *   系统会检查 `XEN_BOOT` 变量。根据 `virgo.mk`，其默认值为 `n`。
    *   因此，`UBOOT_DEFCONFIG_FILES` 变量会被设置为 `$(UBOOT_PATH)/configs/nationalchip_virgo_defconfig` 和 `$(ROOT)/build/kconfigs/u-boot_virgo.conf`。
    *   `make` 会执行 `scripts/kconfig/merge_config.sh` 脚本，将上述两个配置文件合并，在 `$(UBOOT_PATH)`（即 `$(ROOT)/u-boot`）目录下生成最终的 `.config` 文件。

2.  **编译 U-Boot**：
    *   生成 `.config` 文件后，`make` 会进入 `$(UBOOT_PATH)` 目录，执行 `make` 命令来编译整个 U-Boot 源码。
    *   这个过程会生成 U-Boot 的所有二进制文件，其中最重要的产物是位于 `$(UBOOT_PATH)/tools/` 目录下的 `mkimage` 工具，后续创建 `uImage` 时会用到它。

---

### 阶段二：编译 Linux 内核 (`linux` 目标)

编译内核是生成 `uImage` 的核心步骤。

1.  **配置 Linux 内核**：
    *   `linux` 目标依赖 `linux-common` 目标（此目标很可能定义在 `common.mk` 中，是通用的内核编译目标）。
    *   内核编译的第一步是生成配置文件 `.config`。
    *   `virgo.mk` 中定义了 `LINUX_DEFCONFIG_COMMON_FILES`，它包含了两个文件：
        1.  `$(LINUX_PATH)/arch/arm64/configs/defconfig`：一个基础的 ARM64 架构的内核配置文件（对应您提供的 `configs/defconfig`）。
        2.  `$(CURDIR)/kconfigs/virgo.conf`：针对 `virgo` 平台的特定内核配置（对应您提供的 `build/kconfigs/virgo.conf`），它开启了 `OP-TEE`、`Android` 相关特性、`DWMAC` 网卡驱动等。
    *   这两个 `kconfig` 文件会被合并，在 `$(LINUX_PATH)`（即 `$(ROOT)/linux/common`）目录下生成最终的 `.config` 文件。

2.  **编译 Linux 内核**：
    *   有了 `.config` 文件后，`make` 会在 `$(LINUX_PATH)` 目录下执行编译，目标是 `Image`。
    *   编译成功后，会生成内核文件，主要包括：
        *   `$(LINUX_PATH)/arch/arm64/boot/Image`：压缩后的可执行内核镜像。
        *   `$(LINUX_PATH)/vmlinux`：未压缩、包含符号表的 ELF 格式内核文件。

3.  **链接内核镜像**：
    *   `linux` 目标还会执行 `ln -sf $(LINUX_PATH)/arch/arm64/boot/Image $(BINARIES_PATH)`，在 `$(ROOT)/out/bin` 目录下创建一个指向内核 `Image` 的符号链接。

---

### 阶段三：创建 `uImage`

当 `u-boot` 和 `linux` 两个依赖都成功构建后，`make` 就开始执行创建 `$(KERNEL_UIMAGE)` 的命令。

1.  **转换内核格式** (`objcopy`)：
    *   执行 `objcopy` 命令，将 ELF 格式的 `vmlinux` 文件转换为纯二进制格式。
    *   `objcopy -O binary -R .note -R .comment -S $(LINUX_PATH)/vmlinux $(BINARIES_PATH)/linux.bin`
    *   这个命令会移除调试信息和注释节（section），生成一个更小的纯二进制内核文件 `linux.bin`，并存放在 `$(ROOT)/out/bin` 目录。

2.  **打包成 `uImage`** (`mkimage`)：
    *   使用第一阶段中 `u-boot` 编译出的 `mkimage` 工具，为 `linux.bin` 文件添加一个 U-Boot 使用的头部信息。
    *   `mkimage -A arm64 -O linux -T kernel -C none -a 0x02280000 -e 0x02280000 -n "Linux kernel" -d $(BINARIES_PATH)/linux.bin $(KERNEL_UIMAGE)`
    *   参数解释：
        *   `-A arm64`：指定架构为 arm64。
        *   `-O linux`：指定操作系统为 Linux。
        *   `-T kernel`：指定镜像类型为内核。
        *   `-C none`：不使用压缩。
        *   `-a 0x02280000`：指定内核在内存中的加载地址。
        *   `-e 0x02280000`：指定内核的入口点地址。
        *   `-n "Linux kernel"`：为镜像命名。
        *   `-d linux.bin`：指定输入的纯二进制内核文件。
        *   `$(KERNEL_UIMAGE)`：指定输出文件，即 `$(ROOT)/out/bin/uImage`。

最终，`$(ROOT)/out/bin/` 目录下会生成一个名为 `uImage` 的文件，这就是 `make uImage` 命令的最终产物。这个文件可以直接由 U-Boot 加载和启动。




# 制作 rootfs.cpio.gz 
### 制作 CPIO 归档

`rootfs` 目录已经准备好了。使用 `cpio` 命令将其打包。

1. 进入 `rootfs` 目录。
    
2. 执行打包命令。
    

Bash

```
cd rootfs
find . | cpio -o -H newc > ../rootfs.cpio
cd ..
```

**命令解析**：

- `find .`：找到当前目录下的所有文件和目录，并将其列表通过管道 `|` 传递给 `cpio`。
    
- `cpio -o`：表示创建归档（输出模式）。
    
- `-H newc`：使用 "newc" 格式。这是一种可移植的、被广泛支持的格式，也是内核 `initramfs` 所期望的格式。
    
- `> ../rootfs.cpio`：将输出的归档内容重定向到上级目录的 `rootfs.cpio` 文件中。
    

### 压缩 CPIO 归档

为了减小文件大小，加快传输和加载速度，通常会对 CPIO 归档进行压缩。`gzip` 是最常用的工具。

Bash

```
gzip -9 rootfs.cpio
```

这会生成一个名为 `rootfs.cpio.gz` 的文件。



## 制作 ramdisk
```shell
mkimage -A arm64 \
	-T ramdisk \
	-C gzip \
	-a 0x05000000 \
	-e 0x05000000 \
	-n "Root file system" \
	-d rootfs.cpio.gz rootfs.cpio.uboot
```

## uboot 加载 cpio.gz
```
sf probe
sf read 2280000 1000000 9c135b
sf read 5000000 1a00000 2c71c6
booti ${kernel_addr_r} ${ramdisk_addr_r}:0x2c71c6 ${fdt_addr}
```