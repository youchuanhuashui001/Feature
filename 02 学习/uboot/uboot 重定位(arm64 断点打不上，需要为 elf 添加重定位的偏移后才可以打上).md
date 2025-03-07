https://docs.u-boot.org/en/latest/develop/gdb.html
https://wiki.stmicroelectronics.cn/stm32mpu/wiki/U-Boot_-_How_to_debug

# 使用 GDB 调试 U-Boot
使用 JTAG 适配器，可以使用 GDB 调试正在运行的 U-Boot。一种常见的方法是将调试适配器连接到主板的 JTAG 连接器，运行 GDBServer，将 GDB 连接到 GDBServer，然后使用 GDB。

## 编译
建议添加 `-Og` 选项并且不使用链接时间优化来构建 `U-Boot`，以便于调试：
```
CONFIG_CC_OPTIMIZE_FOR_DEBUG=y
CONFIG_LTO=n
```

## 调试
- 开启 GDBServer：
	- arm64: `JLinkGDBServer -endian little -select USB -device Cortex-A55 -if JTAG -speed 10000`
- gdb 链接 GDBServer：
	- arm64：`gdb-multiarch u-boot`
```
target remote 127.0.0.1:2331
```
此时可以在 `U-Boot` 重定位之前正常调试。

**为了在重定位后调试 U-Boot，需要向 GDB 指示重定位地址。可以使用命令 bdinfo 从 U-Boot shell 检索重定位地址：**
```shell
U-Boot> bdinfo
boot_params = 0x20000100
DRAM bank   = 0x00000000
-> start    = 0x20000000
-> size     = 0x08000000
flashstart  = 0x00000000
flashsize   = 0x00000000
flashoffset = 0x00000000
baudrate    = 115200 bps
relocaddr   = 0x27f7a000
reloc off   = 0x0607a000
Build       = 32-bit
current eth = ethernet@f8008000
ethaddr     = 00:50:c2:31:58:d4
IP addr     = <NULL>
fdt_blob    = 0x27b36060
new_fdt     = 0x27b36060
fdt_size    = 0x00003e40
lmb_dump_all:
 memory.cnt = 0x1 / max = 0x10
 memory[0]      [0x20000000-0x27ffffff], 0x08000000 bytes flags: 0
 reserved.cnt = 0x1 / max = 0x10
 reserved[0]    [0x27b31d00-0x27ffffff], 0x004ce300 bytes flags: 0
devicetree  = separate
arch_number = 0x00000000
TLB addr    = 0x27ff0000
irq_sp      = 0x27b36050
sp start    = 0x27b36040
Early malloc usage: cd8 / 2000
```
从上面找到 `relocaddr = 0x27f7a000` 则是需要的地址，对于不同的架构，全局数据指针存储在固定寄存器中：

|Architecture|Register|
|---|---|
|arc|r25|
|arm|r9|
|arm64|x18|
|m68k|d7|
|microblaze|r31|
|mips|k0|
|nios2|gp|
|powerpc|r2|
|riscv|gp|
|sh|r13|
在这些体系结构中，可以通过解引用存储在寄存器中的全局数据指针 (例如 r9) 来确定重定位地址：
```
(gdb) p/x (*(struct global_data*)$r9)->relocaddr
$1 = 0x27f7a000
```
在 gdb shell 中丢弃先前加载的符号表文件并再次添加它，重定位地址如下：
```
(gdb) symbol-file
Discard symbol table from `/home/adahl/build/u-boot/v2024.04.x/u-boot'? (y or n) y
No symbol file now.
(gdb) add-symbol-file u-boot 0x27f7a000
add symbol table from file "u-boot" at
        .text_addr = 0x27f7a000
(y or n) y
Reading symbols from u-boot...
```

或者对于 arm64 调试情况如下：
```
symbol-file u-boot                            --> only for "gd_t" definition
set $offset = ((gd_t *)$x18)->relocaddr       --> get relocation offset, gd in x18 register
symbol-file                                   --> clear previous symbol 
add-symbol-file u-boot $offset
```