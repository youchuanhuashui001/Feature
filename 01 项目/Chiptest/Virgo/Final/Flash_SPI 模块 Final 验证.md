
#doing 

## 问题号：
- 394392：bit 问题号
- 374577：Flash_SPI 模块问题号

## 验证过程：

> [!tips]
> 1. 需要配置引脚复用，见文档服务器 `3.12.1.3`  复用列表中最新的 ` excel `  表格 
> 2. 没有 8 线功能
> 3. 引出了双 CS




- 需要打周龙的管脚复用补丁： https://git.nationalchip.com/gerrit/#/c/120546
- `PE14 ~ PE19` 配成 0 是 `Flash_SPI` 功能 

- [x] 需要测试下引脚复用配错的情况：
	- [x] WP、HOLD 配错 
		- 四线模式有问题 
	- [x] SPI 功能脚配错 
		- 无法正常工作 

- 测试双 cs 时，如果没配另一个 cs 的管脚复用，控制 cs 寄存器不会影响到 cs1


## spinand 打开中断后烧到 flash 无法启动，用 gdb 加载可以正常启动 

> [!info]
>  - 无 dma
>  - 只有一些打印，开了 irq，卡在 flash_init 中了 
>  - gdbserver 打开之后自己就断了





- 测试 1：nor + irq 烧到 flash 可以启动吗? 
	- 可以正常启动 
- 测试 2：换一颗 spi nand
	- 换了江波龙的也还是有问题，并且挂掉的地址不一样了 
- 测试 3：目前是双线模式，换成四线、单线模式看看 
	- 依然不行 
- 测试 4：目前用的是 poll + irq 的模式，poll 会死等变量更新，irq 会更新变量，确认变量是否更新了
	- 加了打印之后可以起来了
- 测试 5：在 `dws->complete_minor、dws->complete`  前面加上 ` volatile `
	- 也还是不行 
- 测试 6：在 while 的时候加上 `gx_mdelay(10)`
	- 也还是不行 
- 测试 7：继续测试 4，用加打印的方式看看程序是不是挂在变量不一致那里 
	- 加了打印就可以起来了 
	- 减少打印依然起来了 
	- 去掉 irq 中的打印依然可以起来
	- 去掉 pool 中的打印起不来了，启动的过程全部都是读，所以不用管写 
	- 用 while (1) 一直打印，如果卡在两个变量不一致的情况下会一直打印的 
		- 现在看起来不是卡在 while 变量判断里面 
- 测试 8：如果还是不行的话，具体分析下卡死在那里 
	- 现在用打印的方式抓不到卡死在哪里，要么看不到，要么就起来了
	- 用 `volatile unsigned int stop_flag = 1; while(stop_flag)` 的方式看看
- 测试 9：相同的代码，编译 gxmisc 用 `O0` 可以起来，编译成 `Os` 也起来了！
	- 编译 gxloader 用的 O0，换成 Os 试试
	- 确认了，gxloader 用 O0 编译就可以，用 Os 编译就不行 
- 测试 10：使用 `static int __attribute__((optimize("O0")))dw_half_duplex_poll_transfer(struct dw_spi *dws)` 修饰函数就可以正常启动了，别的啥都没改 
	- 可以定位到是这个函数被优化了，但是要定位更细，用内存屏障或 volatile 来定位可疑代码
	- 断点打到 `spinand_generic_read_page`，去掉 `loader` 中 `paritition` 相关代码，gdb 单步可以起来
		- 直接 c，起不来 
- 测试 11：拆分开 `tx/rx`  的代码，用 `O0`  编译看是哪部分有问题 
	- 就是 rx 这部分有问题 
	- 单独将 while 变量的丢到一个接口，没有改善 
	- 让更少的代码以 O0 来编译 
	- 定位到 `dws->cur_msg->actual_length += len` 这句需要使用 `O0`  编译 
		- 加上 `volatile` 修饰，没用
		- 加上各种屏障，没用 
		- 反向验证：一句话 Os 编译，另外的都 O0 编译，也可以起来 
			- 所以其实不能证明是那一句话的问题 
		- 在前后加上打印，增加指令依赖性
			- 可以起来，但是这里可能是打印耗时 
- 测试 12：stage2 关掉 cache
	- 还是有问题，需要打开 mmu
	- 依然看不到死在哪里 
	- 去掉读 `partition` 的代码，执行 flash 的测试代码也是一样会卡死 
```
clear_cntvoff() at cpu/arm/virgo/virgo_start.S

partition_init(table_info=table_info@entry=0x0) at partirion.c:786
	if (flash_entry->tables[i].start_addr == AUTO)
```
- 
- 测试 13：dump text 段看看 gdb 加载和 flash 启动是否有差异  
	- gdb 加载的 text 段和 flash 启动的 text 段都是一样的 
	- 这里肯定是一样的，gdb 加载的是 elf 中的 .text 段，flash 拷贝的也是相同的 
		- 除非 flash 拷贝有问题，才会 text 段不一样 
	- 不能关  mmu
	
- 测试 14：参考 apus 调试的时候，感觉和 cache 相关 
- 测试 15：抓波形
	- 看起来没什么问题，波形都是完整的，只有读操作 
- 测试 16：会不会是中断相关问题？
	- 现在 rx 满中断的配置是 `rx_fifo_len`
- 测试 17：换 fpga 试一下，用 rc 版本的 bit，在 19p 上试一下
- 测试 18：芯片仿真，问问能不能仿真一下 


### 问题进度：
- 拉线看到是访问了 `0xf1540240`  这个地址，但是总线没有响应，导致总线卡死，后面其它的总线访问都发不出去了
- `rc`  的 `bit`  直接用 `gdb`  访问 `0xf1540240`  这个地址也是会导致总线卡死，但是跑 `flash`  测试程序不会卡死，说明程序中没有访问过 `0xf1540240` 这个地址






> [!tips]
> - 既然使用 `O0` 编译某个函数就可以，那么就需要缩小是哪些代码有问题
> - `gxloader`  关闭 `iCache、Dcache、MMU`  等：调用 ` boot_close_module ` 可以看到 


