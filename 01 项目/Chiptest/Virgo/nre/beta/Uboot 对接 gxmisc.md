- uboot
	- spi 驱动
		- 不支持 xfer、mode、speed，都是空函数
		- 测试程序：spi. c 可用吗？
	- flash 驱动
		- 只支持 read、write、erase 接口
		- 测试程序：sf. c 可用
- gxmisc：
	- 拷贝库都是固定的路径，目前仓库结构里面不太一致了，gxmisc 的编译脚本需要考虑如何拷贝库到指定的目录
	- 编译工具链：gxloader 和 uboot 不是同一套工具链，这个其实没啥关系
	- gxmisc 是不是需要在安卓的仓库里面放一份？然后跟着外面的版本走，也就是一份 gxmisc 源码，然后在安卓仓库、goxceed 仓库分别一份备份，两套代码始终用的是最新的 gxmisc 代码
- uboot 和 linux 不能共用设备树，现在 nor linux 是自己会初始化自己的套路，而 uboot 是需要设备树中写 nor flash 设备的套路



# 问题

## 【已解决】问题 1：spi、flash 的 probe 函数每次上层调用的时候都会执行吗？
- 是的，每次执行 `sf probe` 都会执行 `gx_spinor_probe`，所以做了一个 init_flag，只初始化 gxmisc 一次

## 问题 2：驱动移植的相关问题
- gxloader 中的 dw_spi 驱动的宏，例如 DUAL、QUAD 这些，和 uboot 的重复定义
	- 解决方法：注释掉 uboot 中的重复定义的宏，因为 gxmisc 依赖 gxloader 这些宏的值
- spi_write_then_read 函数 gxloader 和 uboot 重复定义
	- gxloader 和 uboot 的定义是重复的，但是实现不同，参数不同
	- 这个函数 gxmisc 会直接调用的，所以不能用 uboot 的定义
	- 解决方法：注释掉 uboot 中函数的定义和实现


## 问题 3：gxmisc 对接 linux4.19 和 Uboot
- gxmisc 的路径是：`goxceed/develop2/platform/gxmisc/`
- linux4.19 的路径是：`goxceed/android/linux/common/`
- uboot 的路径是：`goxceed/android/u-boot/`
- 目前是直接编译出来一个库，对于需要用到的路径都是绝对路径，然后手动拷贝到对应的路径
- 需要考虑怎么对接 linux4.19 和 uboot


## 问题 4：uboot 中的 flash 设备使用设备树来描述，而 gxmisc linux 中的 flash 设备是使用 spi_board_info 来描述的，但是现在 uboot 和 linux 使用同一套设备树
- uboot 中 flash 设备用设备树描述
	- 这里看下不用设备树
- linux 中 flash 设备用 spi_board_info 描述



## 问题 5：uboot 目前只提供 flash 的读写擦接口，其余的都没提供
