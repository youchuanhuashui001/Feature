
# Uboot 简介

## 启动流程

- `ROM --> SPL --> Uboot Proper (完整 Uboot) --> Linux Kernel

### SPL (Secondary Program Loader)

- 作用：初始化必要的硬件，加载完整的 Uboot 到内存。
- 编译生成：`spl/u-boot-spl.bin`

### Proper (完整的 Uboot)

- 作用：提供完整的 Uboot 功能，例如设备驱动、文件系统支持、网络功能等，最终引导 Linux Kernel 启动。
- 编译生成：`u-boot.bin`

**说明：**
-   SPL 是一个精简版的 Uboot，主要负责初始化系统，为加载完整的 Uboot 准备环境。
-   完整的 Uboot (Uboot Proper) 则提供更多的功能，用于启动 Linux Kernel。

### env
- 默认的板级环境是通过使用一个简单的文本格式的 .env 环境文件创建的。此文件的基准名称由 CONFIG_ENV_SOURCE_FILE 定义，如果该值为空，则由 CONFIG_SYS_BOARD 定义。
-  `board/nationalchip/virgo/virgo.env`
- 由键值对组成：`var=value`，也可通过 `var+=value` 来添加



# 修改：

## 需要增加的默认配置

- 需要打开的宏：
```shell
CONFIG_SPI=y
CONFIG_DM_SPI=y
CONFIG_DESIGNWARE_SPI=y

CONFIG_CMD_SF=y
CONFIG_CMD_SF_TEST=y
CONFIG_CMD_SPI=y
# CONFIG_DEFAULT_SPI_BUS=0  xxx
# CONFIG_DEFAULT_SPI_MODE=0 xxx 

# CONFIG_SF_DEFAULT_SPEED=1000000  xxx
# CONFIG_SF_DEFAULT_MODE=0x0   xxx

CONFIG_DM_SPI_FLASH=y
CONFIG_SPI_FLASH=y
# CONFIG_SF_DEFAULT_BUS=0  xxx
# CONFIG_SF_DEFAULT_CS=0   xxx
# 使能 spi flash 作为 boot 设备；允许从 spi flash 读一个脚本用于启动 os
# CONFIG_BOOTDEV_SPI_FLASH is not set
# CONFIG_SPI_FLASH_SFDP_SUPPORT is not set
# 智能的 flags，例如 SPI_RX_DUAL、SPI_TX_QUAD 等
# CONFIG_SPI_FLASH_SMART_HWCAPS=y  xxx
# CONFIG_SPI_NOR_BOOT_SOFT_RESET_EXT_INVERT is not set
# CONFIG_SPI_FLASH_SOFT_RESET is not set
# 用于支持在 3 字节地址模式用额外的寄存器访问 >16MB 的flash
# CONFIG_SPI_FLASH_BAR is not set
# 有些 flash 倾向于上电的时候默认打开软件写保护，需要先解锁
# CONFIG_SPI_FLASH_UNLOCK_ALL=y xxx
# CONFIG_SPI_FLASH_ATMEL is not set
# CONFIG_SPI_FLASH_EON is not set
CONFIG_SPI_FLASH_GIGADEVICE=y
# CONFIG_SPI_FLASH_ISSI is not set
# CONFIG_SPI_FLASH_MACRONIX is not set
# CONFIG_SPI_FLASH_SPANSION is not set
# CONFIG_SPI_FLASH_STMICRO is not set
# CONFIG_SPI_FLASH_SST is not set
CONFIG_SPI_FLASH_WINBOND=y
CONFIG_SPI_FLASH_XMC=y
# CONFIG_SPI_FLASH_XTX is not set
# 需要手动关掉，默认是开着的，默认需要用64k擦除，这样配置才是关掉，如果配置成 =n 并没有关掉
# CONFIG_SPI_FLASH_USE_4K_SECTORS is not set
# 使能访问 spi-flash 为基础的 AT45xxx dataflash 片子。许多都是由两个 buffer 组成。
# CONFIG_SPI_FLASH_DATAFLASH is not set
# 启用对 spi flash 层的 mtd 支持；将 mtd_read、mtd_write 转换成 spi read、write 命令
# CONFIG_SPI_FLASH_MTD is not set
```

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
setenv fdt_addr 0x8000000 && setenv kernel_addr_r 0x12200000 && setenv ramdisk_addr_r 0x15000000 && setenv bootargs console=ttyS0,115200 earlycon=uart8250,mmio32,0xFC880000 root=/dev/nfs rw nfsroot=192.168.108.149:/opt/nfs/virgo_nre/,v3 ip=192.168.108.198:::255.255.255.0  mtdparts=m25p80:128k@0m(BOOT),64k@128k(TABLE),2880k@192k(KERNEL),1792k@3m(ROOTFS),2368k@4864k(USER),960k@7232k(DATA) mtdparts_end
 && bootm ${kernel_addr_r} - ${fdt_addr}
```


## SPI 和 Flash 工作情况：
- SPI 模块频率？工作频率？
	- 模块频率：24Mhz，由设备树指定 clk 源自哪里
	- 工作频率：1000000 
		- 设备树通过参数 `spi-max-frequency` 指定 spi 最大工作频率
	- 在哪里设置的 flash 工作频率？
		- 设备树通过参数 `spi-max-frequency` 指定 flash 最大工作频率
		- `dm_spi_claim_bus`：`struct udevice` 认领 spi 总线
			- 从 dev 拿到 `struct spi_slave` (代表 flash 设备) 结构的频率和模式，和 spi 总线的最大时钟频率
			- 设备树没配置 spi 的最大时钟频率，所以这里 flash 就会被配成 10000
			- 调用 ops->set_speed 来设置分配比 `badu` 寄存器
		- 如何将 spi 的最大时钟频率设置为 24MHz？或者是从设备树取？
			- dw_spi_of_to_plat 函数中会解析设备树中的 `spi-max-frequency` 参数来配置频率，默认是 `500000` 
			- spi 的最高频率配成了 24MHz，flash 的频率配成了 2Mhz
		- simple_delay 怎么处理的？
			- 没有 sample_delay，所以频率要足够低
- Flash 指令？模式？
	- spi 工作模式选择：spi_slae_of_to_plat
		- 通过设备树中描述的字符串，来选择 tx/rx 模式
	- readdata：
		- flash ids 中存在 flag 描述 flash 支持的工作模式；设备树中描述 flash 的工作模式，tx/rx 支持什么模式；两者匹配之后会用不同的指令
		- `spi_slave_of_to_plat  spi_nor_scan`
	- readreg:
		- 单线发一个 opcode
- cs 用的 gpio 怎么处理？
	- 设备树描述一个 `cs->reg`


## Flash 读写性能
```shell
=>sf test 0x800000 0×100000
SPI Flash test
0 erase：6582 sticks, 155 KiB/s 1.240 Mbps
1 check: 4510 tickets, 277 KiB/s 1.816 Mbps
2 write: 6547 tickets, 156 KiB/s 1.248 Mbps
3 read: 4196 ticks, 244 KiB/s 1.952 Mbps
Test passed
0 erase：6582 ticksels,155 KiB/s 1.240 Mbps
1 check: 4510 ticks,c 27 KiB/s 1.816 Mbps
2 write: 6547 ticks,c 156 KiB/s 1.248 Mbps
3 read: 4196 ticks,244 KiB/s 1.952 Mbps
=>sf test 0x800000 0×200000
SPI flash test:
0 erase：693 ticks,295 KiB/s 2.360 Mbps
1 check: 9021 ticksels,272 KiB/s 1.816 Mbps
2 write: 12928 ticks,158 KiB/s 1.264 Mbps
3 read: 8391 ticks,244 IB/s 1.952 Mbps
Test passed
0 erase：6939 ticks,295 KiB/s 2.360 Mbps
1 check: 9021 ticks,158 Ki/s 1.816 Mbps
size: 12928 ticks,158 Ki/s 1.264 Mbps
3 read: 8391 ticks,144 KiB/s 1.952 Mbps
```
# Env
- 如何保存 Env？
	- 使用 spi flash 来保存，存在地址 0x400000，大小是 0x40000

# 验证：
- sf test
- 起 kernel



# Q&A
## Q1：启动流程是怎样的？会用到 Uboot 的 SPL 吗？
- 启动 Android 使用 Uboot，`rom --> gxloader stage1 --> uboot --> Android`
- 启动 Linux 使用 gxloader，`rom --> gxloader stage1 --> gxloader stage2 --> Android`

## Q2：分区表信息的 cmdline 是从 Uboot 传给 Linux 的？
- 启动 Android 时的 cmdline 由 uboot 传给 Linux，cmdline 固定写死在 env
- 启动 Linux 时的 cmdline 由 gxloader 传给 Linux，cmdline 固定写死在 gxloader

## Q3：Uboot 没有看到对应的设备树？
- uboot 会用设备树，Linux 也会用设备树，uboot 里面没加设备树，linux 里面也没加设备树，
- 老司机单独把 dtb 做了一份，同时提供给 uboot 和 linux 用
- `/home/tanxzh/goxceed/android/build/virgo/dtb`

## Q4：VU440 复位后一直没打印
- 重启了之后 ok



## Q5：断点打不到对应的位置，看不到 spi 的初始化函数
- 不知道用的是不是我编出来的这个代码
- 开了 spi 之后，执行命令 `dm uclass` 可以看到 `spi@f8400000`
- 不开之后，执行命令 `dm uclass` 看不到了，说明是正常的
- uboot 会重定位，所以打断点打不到对应的位置，需要修改一下偏移
	- https://wiki.stmicroelectronics.cn/stm32mpu/wiki/U-Boot_-_How_to_debug
```
symbol-file u-boot                            --> only for "gd_t" definition
set $offset = ((gd_t *)$x18)->relocaddr       --> get relocation offset, gd in x18 register
symbol-file                                   --> clear previous symbol 
add-symbol-file u-boot $offset
```


## Q6：`ENV_IS_IN_FLASH` 和 `ENV_IS_IN_SPI_FLASH`
- `ENV_IS_IN_SPI_FLASH`：
	- 依赖 `SPI`
	- 必选参数：
		- `CONFIG_ENV_OFFSET`: 定义 env 区域在 spi flash 的偏移；必须与擦除的大小对齐
		- `CONFIG_ENV_SIZE`: 定义 env 区域在 spi flash 占用的大小
	- 可选参数：
		- `CONFIG_ENV_SECT_SIZE`：定义 spi flash 的扇区大小
		- `CONFIG_ENV_OFFSET_REDUND`：可选的，备用区域。当 `saveenv` 时出现断电，此区域用于备份
	- 其它参数：
	- `CONFIG_ENV_ADDR`：是指定 flash 中包含 env 的另一种方式
- 对于 `ENV_IS_IN_SPI_FLASH` 来说这两种方式的区别：`CONFIG_ENV_OFFSET、CONFIG_ENV_ADDR`
	- `CONFIG_ENV_ADDR`：在初始化时会到此地址，对此地址所存在的 ENV 进行 CRC 计算，如果失败则认为不可用；如果计算 CRC 正确，则认为可用
	- `CONFIG_ENV_OFFSET`：在 `saveenv、load env` 时都是直接从 `CONFIG_ENV_OFFSET` 地址操作的

## Q7：SPI 的 CS reg 如何处理？
- `designware_spi. c` 中对于 `cs_manage` 是定义的一个弱函数，所以可以直接直接在别的地方放一个 `cs_manage` 函数来替换
- 放到 board 目录下做成一个 .c 专门处理


