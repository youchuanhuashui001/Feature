---
tags:
  - Virgo_MPW模块验证
---
# 验证进度：
- 总验证点:: 27
- 已完成:: 21
- 备注:: 降采样未测试，所有封装及所有 GPIO 未测试，需要硬件提供小板


# 修改点
- 时钟：198MHz



# 驱动支持
## Linux
- `arm-linux-uclibcgnueabihf-gcc -static -o gpio_test.elf gpio_test.c`




# 验证项

## 版本号读取
- 查看寄存器 0xfa400000
```
(gdb) x/x 0xfa400000
0xfa400000:     0x00010001
```
- 测试 ok


## 参数获取
- 此功能支持获取当前 gpio 模块的 gpio 总数
- 查看寄存器 0xfa400000+0xe0，能够正确获取当前 gpio 模块的 gpio 总数为 32，测试通过
```
(gdb) x/x 0xfa400000+0xe0
0xfa4000e0:     0x00000020
(gdb) x/x 0xfa401000+0xe0
0xfa4010e0:     0x00000020
(gdb) x/x 0xfa402000+0xe0
0xfa4020e0:     0x00000020
(gdb) x/x 0xfa403000+0xe0
0xfa4030e0:     0x00000020

```




## 输入模式
- GPIO0 测试 ok


## 输出模式
- GPIO0 测试 ok


## 输出高电平
- GPIO0 测试 ok


## 输出低电平
- GPIO0 测试 ok


## 普通IO模式
- GPIO0 测试 ok


## 端口复用输出模式
- GPIO0 复用成 PWM0、PWMx 测试 ok


## 时钟一直有效
- GPIO0 测试 ok


## 自动开关时钟
- 代码修改：
```diff
--- a/drivers/gpio/gx_gpio.c
+++ b/drivers/gpio/gx_gpio.c
@@ -57,6 +57,7 @@ int gx_gpio_setio(unsigned long gpio, unsigned long io)
                __raw_writel(1 << offs, reg_base + GPIO_SET_IN);
 #endif
 
+       __raw_writel(1, reg_base + GPIO_CTL);
        return 0;
 }
```
- 可以正常输出，GPIO0 测试 ok


## 输入模式
- GPIO0 测试 ok


## 输出模式
- GPIO0 测试 ok


## 使能中断
- GPIO0 测试 ok


## 失能中断
- GPIO0 测试 ok


## 高电平触发
- GPIO0 测试 ok


## 低电平触发
- GPIO0 测试 ok


## 上升沿触发
- GPIO0 测试 ok


## 下降沿触发
- GPIO0 测试 ok


## 使能降采样【未测试】



## 失能降采样【未测试】



## 时钟一直有效
- 测试 ok


## 自动开关时钟
- 测试 ok


## 输入电平获取
- GPIO0 测试 ok





## 测试所有封装的PWM功能【未测试】



## 测试所有的输入端口【未测试】



## 测试所有封装的输入输出功能【未测试】



## 测试所有封装的中断功能【未测试】










# 问题


## 模块频率和 linux 驱动
- 模块频率是 198，还在找芯片确认
- linux 驱动gpio和timer 需要分开，pwm放到 timer 中
	- timer 整一个独立的驱动出来
	- 需要提供 内核态和用户态的接口
		- 内核态，用 export 提供出去
		- 用户态，gxapi
