
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