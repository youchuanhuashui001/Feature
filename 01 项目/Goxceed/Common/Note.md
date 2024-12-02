
## Linux 以 O0 的方式编译某个 .c 文件
- 起因是在移植高版本 axi-dma 驱动到 Linux4.9 时用 GDB 调试发现有些变量看不到，Linux 默认是以 O2 or Os 编译的，那么就需要单独以 O0 的方式编译来查看变量
	```Makefile
	CFLAGS_dw-axi-dmac-platform.o = -O0 -g
	obj-$(CONFIG_DW_AXI_DMAC) += dw-axi-dmac-platform.o
	```
	