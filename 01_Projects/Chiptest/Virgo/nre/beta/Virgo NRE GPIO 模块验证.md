
## 1. 基本信息

- **Redmine 问题号**: `#408040`
- **芯片型号/项目名称**: `Virgo_NRE`
- **被测模块**: `GPIO`
- **固件/FPGA 镜像版本**: 
	- a7： #404698 vu9p_fpga_top_sub_a7_20250524. bit test7 a7 核 flash 中 48M
	- a55: #404698 vu9p_fpga_top_sub_a55_20250524. bit test8 a55 核 flash 中 48M
- **相关文档**: [[gpio ip]]


## 2. 测试目标
简要描述本次测试的主要目的和范围。
- 兼容 32 位、64 位驱动、测试代码
- GPIO 模块验证


## 3. 测试项

### 版本号读取       
- 查看寄存器 0xfa400000，正确查看到版本号 0x00010001，测试通过
```
(gdb) x/x 0xfa400000
0xfa400000:     0x00010001
(gdb) x/x 0xfa401000
0xfa401000:     0x00010001
(gdb) x/x 0xfa402000
0xfa402000:     0x00010001
(gdb) x/x 0xfa403000
0xfa403000:     0x00010001
(gdb) x/x 0xfa404000
0xfa404000:     0x00010001
(gdb) x/x 0xfa405000
0xfa405000:     0x00010001
(gdb) x/x 0xfa408000
0xfa408000:     0x00000000
(gdb) x/x 0xfa407000
0xfa407000:     0x00000000
(gdb) x/x 0xfa406000
0xfa406000:     0x00000000
```


### 参数获取        
- 此功能支持获取当前 gpio 模块的 gpio 总数
- 查看寄存器 0xfa400000+0xe0，能够正确获取当前 gpio 模块的 gpio 总数为 32，测试通过
```
(gdb) x/x 0xfa400000+0xe0
0xfa4000e0:     0x00000020
```




### 输入模式        
- `board/virgo/board-fpga/board-init.c` 中的 `g_gpio_table` 中需要配置要使用的 gpio
```diff
--- a/board/virgo/board-fpga/board-init.c
+++ b/board/virgo/board-fpga/board-init.c
@@ -187,6 +187,8 @@ struct mulpin_config_s mulpin_table[] = {
 };
 
 struct gpio_entry_bootloader g_gpio_table[] = {
+       {0,  0, GX_GPIO_CONFIG_VALID, GX_GPIO_OUTPUT, GX_GPIO_HIGH},
+       {1,  1, GX_GPIO_CONFIG_VALID, GX_GPIO_OUTPUT, GX_GPIO_HIGH},
```
- fmc 子板拨到 INPUT，通过杜邦线连接 gpio0 到 gnd 或高电平的引脚，能够正确获取到当前引脚上的电平，测试通过。
```
boot> gpio set_io 0 0
worning: set v_gpio '0' mode 'input'
boot> gpio get_io 0
v_gpio '0' mode 'input'
boot> gpio get_level 0
v_gpio: 0, value: 1
boot> gpio get_level 0
v_gpio: 0, value: 0

```



### 输出模式        
- 前提条件与输入模式一致
- fmc 子板拨到 OUTPUT，逻辑分析仪量 gpio0 脚的电平，能够正常输出高低电平，测试通过。
```
boot> gpio set_io 0 1
worning: set v_gpio '0' mode 'output'
boot> gpio get_io 0
v_gpio '0' mode 'output'

boot> gpio set_level 0 1
operate sucess !
boot> gpio set_level 0 0
operate sucess !
boot> gpio set_level 0 1
operate sucess !
```



### 输出高电平       
- 测试输出模式时已覆盖。



### 输出低电平       
- 测试输出模式时已覆盖。



### 普通IO模式      
- 测试输出模式时已覆盖。



### 端口复用输出模式    
- 测试 Timer 模块时已覆盖，复用为输出 PWM 模式。



### 时钟一直有效      




### 自动开关时钟      




### 输入模式        
- 已测过



### 输出模式        
- 已测过



### 使能中断        
- 打开 `.config` 中的 `ENABLE_IRQ = y`
- 其余前置条件与之前测试一致。
- fmc 子板切换到 input 模式，使用杜邦线连接 gpio 来制造触发条件。
```
gpio request_irq 0 1
gpio disable 0

gpio request_irq 0 2
gpio disable 0

gpio request_irq 0 3
gpio disable 0

gpio request_irq 0 4
gpio disable 0

gpio request_irq 0 8
gpio disable 0
```



### 失能中断        
- 使能中断测试时已覆盖。



### 高电平触发       
- 使能中断测试时已覆盖。



### 低电平触发       
- 使能中断测试时已覆盖。



### 上升沿触发       
- 使能中断测试时已覆盖。



### 下降沿触发       
- 使能中断测试时已覆盖。



### 使能降采样       




### 失能降采样       




### 时钟一直有效      




### 自动开关时钟      




### 输入电平获取      
- 之前已测试过。



### 测试所有封装的PWM功能
- 不可测



### 测试所有的输入端口   
- 不可测



### 测试所有封装的输入输出功
- 不可测



### 测试所有封装的中断功能 
- 不可测







