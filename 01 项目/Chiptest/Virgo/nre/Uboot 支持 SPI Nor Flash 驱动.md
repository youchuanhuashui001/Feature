
# 系统启动流程



`

# Uboot

- `ROM --> SPL --> Uboot Proper (完整 Uboot) --> Linux Kernel

## SPL (Secondary Program Loader)

- 作用：初始化必要的硬件，加载完整的 Uboot 到内存。
- 编译生成：`spl/u-boot-spl.bin`

## Proper (完整的 Uboot)

- 作用：提供完整的 Uboot 功能，例如设备驱动、文件系统支持、网络功能等，最终引导 Linux Kernel 启动。
- 编译生成：`u-boot.bin`

**说明：**
-   SPL 是一个精简版的 Uboot，主要负责初始化系统，为加载完整的 Uboot 准备环境。
-   完整的 Uboot (Uboot Proper) 则提供更多的功能，用于启动 Linux Kernel。

### env
- 默认的板级环境是通过使用一个简单的文本格式的 .env 环境文件创建的。此文件的基准名称由 CONFIG_ENV_SOURCE_FILE 定义，如果该值为空，则由 CONFIG_SYS_BOARD 定义。
-  `board/nationalchip/virgo/virgo.env`
- 由键值对组成：`var=value`，也可通过 `var+=value` 来添加




# Q&A
## Q1：启动流程是怎样的？会用到 Uboot 的 SPL 吗？

## Q2：分区表信息的 cmdline 是从 Uboot 传给 Linux 的？

