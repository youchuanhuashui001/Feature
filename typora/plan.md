# Plan

- [ ] Linux 设备驱动第三版
  - [ ] 2023-07-11 ~ 



- [ ] 迅为电子 Linux 教程
  - [ ] 2023-07-11 ~
  - [ ] 

- [x] MTD 子系统
  - [x] 包括 mtdcore、mtdchar、mtdblock、mtdblk_dev、mtdpart、cmdlinepart

- [x] UBI
  - [x] UBI 卷
  - [x] UBIFS

- [x] NAND 控制器驱动，包括每一个寄存器的作用
  - [x] sirius：nfc_v0
  - [x] canopus：nfc_v1

- [x] onfi 手册概览








- [x] arm 常用的一些功能特性
  - [x] 包括指令执行、指令退休、




- [ ] 韦东山相关的 uboot、re-thread、freertos



- [x] sram 的大小是 4k，启动的时候通过编译和链接脚本让start.S 等文件放在前 4K 的位置，代码从 start.S 到 copy_from_flash 结束的代码不能超过 4K，因为 rom 只会从 flash 读 4K 数据到 sram。代码运行到 copy_from_flash 这个代码必须要在 前 4K 内，假设是在 3K 的位置，那么后面的 1k 代码就不会运行。



- [x] gic-v2
- [x] ARM 体系结构与编程
  - [ ] ARM 体系结构与编程第二部

- [ ] ARM-CortexA7-Program-Guide
  - [ ] cache
  - [ ] mmu
  - [ ] interrupt/exception

- [ ] arm64
- [ ] linux os
- [ ] ecos

- [ ] 移植最新 uboot、linux、根文件系统
  - [ ] https://www.cnblogs.com/zyly/p/17418892.html


- Linux 内核完全注释
  - [ ] block_dev 驱动
    - 目前 blk.h 看完。9.3 hd.c 程序未开始
  - [ ] fs



- [ ] 工具链熟悉，例如 readelf，objdump
- [ ] gdb 调试手段：dump、store 等
- [ ] 工具使用：xxd，hexdump 等
