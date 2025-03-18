# GX GPIO 芯片测试文档

## 1. 测试环境
- FPGA 版本：`382541 9p_peri_newcode_FPGA_2024-09-03.tar`
- VU9P

## 2. 测试用例

### 2.1 版本号读取
- 测试步骤：
	- 读取 GPIO 控制器版本号
	- 查看寄存器 `0xfa400000/0xfa401000/0xfa402000/0xfa403000`，默认值为 `0x10001`
	- `GPIO_VERSION` 在每组 gpio 中都有相同的寄存器
- 预期结果：
	- 版本号读取正确

### 2.2 参数获取
- 测试步骤：
	- 获取 GPIO 控制器参数
	- 查看寄存器 `0xfa4000e0`，其他组的 gpio 也是相同的方法
	- `GPIO_PARAMETER` 在每组 gpio 中都有相同的寄存器
- 预期结果：
	- 参数获取正确

### 2.3 输入模式
- 测试命令：
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio set_io [gpio] 0` 
- 测试步骤：
	- 配置 GPIO 为输入模式
	- 验证配置是否生效
- 预期结果：
	- GPIO 成功配置为输入模式

### 2.4 输出模式
- 测试命令：
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio set_io [gpio] 1` 
- 测试步骤：
	- 配置 GPIO 为输出模式
	- 验证配置是否生效
- 预期结果：
	- GPIO 成功配置为输出模式

### 2.5 输出高电平
- 测试命令：
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio set_io [gpio] 1` 
	- `gpio set_level [gpio] 1`
- 测试步骤：
	- 配置 GPIO 为输出模式
	- 设置输出高电平
	- 验证输出状态
- 预期结果：
	- GPIO 输出高电平

### 2.6 输出低电平
- 测试命令：
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio set_io [gpio] 1` 
	- `gpio set_level [gpio] 0`
- 测试步骤：
	- 配置 GPIO 为输出模式
	- 设置输出低电平
	- 验证输出状态
- 预期结果：
	- GPIO 输出低电平

### 2.7 普通IO模式
- 测试命令：
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio set_io [gpio] 1` 
	- `gpio set_level [gpio] 0`
- 测试步骤：
	- 配置 GPIO 为普通 IO 模式
	- 验证配置是否生效
- 预期结果：
	- GPIO 成功配置为普通 IO 模式

### 2.8 端口复用输出模式
- 测试命令：
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- 使用 timer 的 pwm 功能可以测到
- 测试步骤：
	- 配置 GPIO 为端口复用输出模式，FPGA 时只能将端口复用为 PWM 功能
	- 验证配置是否生效
- 预期结果：
	- GPIO 成功配置为端口复用输出模式

### 2.9 时钟一直有效
- 测试步骤：
	- 配置 GPIO 时钟一直有效
	- 验证配置是否生效
- 预期结果：
	- GPIO 时钟一直有效

### 2.10 自动开关时钟
- 测试步骤：
	- 配置 GPIO 时钟自动开关
	- 验证配置是否生效
- 预期结果：
	- GPIO 时钟可以自动开关

### 2.11 输入模式
- 测试命令：
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio set_io [port] 0` 
- 测试步骤：
	- 配置 GPIO 为输入模式
	- 验证配置是否生效
- 预期结果：
	- GPIO 成功配置为输入模式

### 2.12 输出模式
- 测试命令：
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio set_io [port] 1` 
- 测试步骤：
	- 配置 GPIO 为输出模式
	- 验证配置是否生效
- 预期结果：
	- GPIO 成功配置为输出模式

### 2.13 使能中断
- 测试命令：
	- 配置 `.config` 使能 `CONFIG_ENABLE_IRQ`
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio request_irq [port] [irq_mode]` 
- 测试步骤：
	- 配置 GPIO 使能中断
	- 验证配置是否生效
- 预期结果：
	- GPIO 中断成功使能，中断回调正常执行

### 2.14 失能中断
- 测试命令：
	- 配置 `.config` 使能 `CONFIG_ENABLE_IRQ`
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio disable_irq [port]` 
- 测试步骤：
	- 配置 GPIO 失能中断
	- 验证配置是否生效
- 预期结果：
	- GPIO 中断成功失能

### 2.15 高电平触发
- 测试命令：
	- 配置 `.config` 使能 `CONFIG_ENABLE_IRQ`
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio request_irq [port] GPIO_TRIGGER_LEVEL_HIGH` 
- 测试步骤：
	- 配置 GPIO 中断为高电平触发
	- 验证触发条件
- 预期结果：
	- 高电平时正确触发中断

### 2.16 低电平触发
- 测试命令：
	- 配置 `.config` 使能 `CONFIG_ENABLE_IRQ`
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio request_irq [port] GPIO_TRIGGER_LEVEL_LOW` 
- 测试步骤：
	- 配置 GPIO 中断为低电平触发
	- 验证触发条件
- 预期结果：
	- 低电平时正确触发中断

### 2.17 上升沿触发
- 测试命令：
	- 配置 `.config` 使能 `CONFIG_ENABLE_IRQ`
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio request_irq [port] GPIO_TRIGGER_LEVEL_RISING` 
- 测试步骤：
	- 配置 GPIO 中断为上升沿触发
	- 验证触发条件
- 预期结果：
	- 上升沿时正确触发中断

### 2.18 下降沿触发
- 测试命令：
	- 配置 `.config` 使能 `CONFIG_ENABLE_IRQ`
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio request_irq [port] GPIO_TRIGGER_LEVEL_FALLING` 
- 测试步骤：
	- 配置 GPIO 中断为下降沿触发
	- 验证触发条件
- 预期结果：
	- 下降沿时正确触发中断

### 2.19 使能降采样
- 测试步骤：
	- 配置 GPIO 使能降采样
	- 验证配置是否生效
- 预期结果：
	- GPIO 降采样成功使能
	- 配置为降采样后，IO 检测输入从 0 到 1 或者 从 1 到 0 有一定延迟

### 2.20 失能降采样
- 测试步骤：
	- 配置 GPIO 失能降采样
	- 验证配置是否生效
- 预期结果：
	- GPIO 降采样成功失能

### 2.21 时钟一直有效
- 测试步骤：
	- 配置 GPIO 时钟一直有效
	- 验证配置是否生效
- 预期结果：
	- GPIO 时钟一直有效

### 2.22 自动开关时钟
- 测试步骤：
	- 配置 GPIO 时钟自动开关
	- 验证配置是否生效
- 预期结果：
	- GPIO 时钟可以自动开关

### 2.23 输入电平获取
- 测试命令：
	- 配置 `.config` 使能 `CONFIG_ENABLE_IRQ`
	- `struct gpio_entry_bootloader g_gpio_table` 中配置要使用的 gpio
	- `gpio getlevel [port]`
- 测试步骤：
	- 配置 GPIO 为输入模式
	- 获取输入电平
- 预期结果：
	- 正确获取输入电平

### 2.24 测试所有封装的PWM功能
- 测试步骤：
	- 配置 GPIO 为 PWM 功能
	- 测试不同占空比
- 预期结果：
	- PWM 功能正常工作

### 2.25 测试所有的输入端口
- 测试步骤：
	- 测试所有可用的输入端口
- 预期结果：
	- 所有输入端口正常工作

### 2.26 测试所有封装的输入输出功能
- 测试步骤：
	- 测试所有封装的输入输出功能
- 预期结果：
	- 所有输入输出功能正常工作

### 2.27 测试所有封装的中断功能
- 测试步骤：
	- 测试所有封装的中断功能
- 预期结果：
	- 所有中断功能正常工作

## 3. 测试结果
- [x] 2.1 版本号读取
- [x] 2.2 参数获取
- [x] 2.3 输入模式
- [x] 2.4 输出模式
- [x] 2.5 输出高电平
- [x] 2.6 输出低电平
- [x] 2.7 普通IO模式
- [x] 2.8 端口复用输出模式
- [x] 2.9 时钟一直有效
- [x] 2.10 自动开关时钟
- [x] 2.11 输入模式
- [x] 2.12 输出模式
- [x] 2.13 使能中断
- [x] 2.14 失能中断
- [x] 2.15 高电平触发
- [x] 2.16 低电平触发
- [x] 2.17 上升沿触发
- [x] 2.18 下降沿触发
- [x] 2.19 使能降采样
- [x] 2.20 失能降采样
- [x] 2.21 时钟一直有效
- [x] 2.22 自动开关时钟
- [x] 2.23 输入电平获取
- [ ] 2.24 测试所有封装的PWM功能
- [ ] 2.25 测试所有的输入端口
- [ ] 2.26 测试所有封装的输入输出功能
- [ ] 2.27 测试所有封装的中断功能

## 4. 测试记录


### 1. GPIO 复用问题
- 首先有一个管脚复用的模块
	- 可以配置管脚的上下拉，输出电流，复用之类的，这里要复用成 gpio
- 然后有一个 gpio 模块
	- `AFIO_EN` 来选择是当成普通 gpio 来用，还是复用成其它模式
	- `AFIO_SEL` 选择对应 GPIO 输出到哪一个复用端口
- 为什么不都放到管脚复用里面去做？
	- timer pwm 算是 gpio 的一个特性
	- 而且 `AFIO_SEL` 只有在芯片集成的时候才知道有哪几种功能，现在 fpga 还不知道，只支持 timer

### 2. GPIO 拉高拉低问题
- `394932 peri_final3_gpio_FPGA_2024-12-03.tar` 不能拉高拉低
- `382541 9p_peri_newcode_FPGA_2024-09-03.tar` 可以拉高拉低
- 子板都是好的

### 3. gpio 设置为输入模式，要怎么验证？
- 在使能中断的时候可以测试到，gpio 配置为输入模式，可以触发中断


### 4. gpio 低功耗、降采样，驱动中找不到
- 手动使能 gate
```diff
--- a/drivers/gpio/gx_gpio.c
+++ b/drivers/gpio/gx_gpio.c
@@ -51,10 +51,14 @@ int gx_gpio_setio(unsigned long gpio, unsigned long io)
                tmp &= ~(1 << offs);
        __raw_writel(tmp, reg_base + GPIO_EPDDR);
 #elif defined(CONFIG_GPIO_GX_V2) || defined(CONFIG_GPIO_GX_V3)
-       if (io)
+       if (io) {
+               __raw_writel(0x3, reg_base + GPIO_CTL);
                __raw_writel(1 << offs, reg_base + GPIO_SET_OUT);
-       else
+       }
+       else {
+               __raw_writel(0x1, reg_base + GPIO_CTL);
                __raw_writel(1 << offs, reg_base + GPIO_SET_IN);
+       }
 #endif

```
- 降采样一直都没用过，而且实际也看不太出来，使能一下看看可以正常采就行了
	- 和 ai 那边用的同样的 ip，ai 也没测过
	- 降低 gpio 的采样频率？原始默认采样频率是多少？
	- 使能 DS_EN、配成 DS_CNT 为 2
	- 因为 DS_EN 是每个 gpio 都可以使能的，而 DS_CNT 只有一个，所以使能后的那组 gpio，用的都是同样的 DS_CNT

### 5. GPIO_PIN_VAL 寄存器做输入的时候不可用？
- 看驱动中描述的是获取输入/输出引脚的 gpio 电平，描述的意思是输入输出都可以用，实际测试也是都可以的
![[Pasted image 20250317101220.png]]
- 描述的应该是 fpga 的 gpio 是否是输入模式，实际的 gpio 应该没这个问题
	- fpga 的 gpio 如果拨码拨到输出，那么是没法采到电平的


### 6. gpio 触发不了中断
- 测试命令：`gpio request_irq 0 0xf`
- 小板切成了输入
- 因为之前测试的时候第一次配置的中断模式是上升沿，后面再配置成所有的中断模式，但是驱动中如果第一次配置了中断模式，后面再配就不会起作用，所以需要确保第一次配置成对应的中断模式，或全开