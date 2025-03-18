# GX Timer 芯片测试文档

## 1. 测试环境
- FPGA 版本：`394932 peri_final3_gpio_FPGA_2024-12-03.tar`
- VU9P

## 2. 测试用例

### 2.1 时钟持续有效测试
- 测试步骤：
	- 配置时钟持续有效
	- 观察定时器工作状态
- 预期结果：
	- 定时器正常工作
	- 时钟持续有效

### 2.2 自动开关时钟测试
- 测试步骤：
	- 配置时钟自动开关
	- 观察定时器工作状态
- 预期结果：
	- 定时器正常工作
	- 时钟可以自动开关

### 2.3 预分频测试
- 测试步骤：
	- 配置不同的预分频值
	- 观察定时器计数频率
- 预期结果：
	- 预分频配置正确
	- 计数频率符合预期

### 2.4 计数模式测试
- 测试步骤：
	- 配置不同的计数模式
	- 观察定时器计数行为
- 预期结果：
	- 计数模式配置正确
	- 计数行为符合预期

### 2.5 使能计数器测试
- 测试步骤：
	- 使能计数器
	- 观察计数行为
- 预期结果：
	- 计数器正常计数

### 2.6 禁用计数器测试
- 测试步骤：
	- 禁用计数器
	- 观察计数行为
- 预期结果：
	- 计数器停止计数

### 2.7 清零定时器测试
- 测试步骤：
	- 清零定时器
	- 观察计数值
- 预期结果：
	- 计数值清零

### 2.8 溢出中断测试
- 测试步骤：
	- 配置溢出中断
	- 等待定时器溢出
- 预期结果：
	- 溢出时触发中断

### 2.9 定时器阈值测试
- 测试步骤：
	- 配置定时器阈值
	- 观察阈值触发行为
- 预期结果：
	- 达到阈值时正确触发

### 2.10 读取定时器值测试
- 测试步骤：
	- 读取定时器当前值
- 预期结果：
	- 读取值准确

### 2.11 定时器溢出次数读取测试
- 测试步骤：
	- 读取溢出次数
- 预期结果：
	- 溢出次数准确

### 2.12 PWM 模式配置测试
- 测试步骤：
	- 配置 PWM 模式
- 预期结果：
	- PWM 模式配置正确

### 2.13 使能 PWM 测试
- 测试步骤：
	- 使能 PWM
- 预期结果：
	- PWM 输出正常

### 2.14 失能 PWM 测试
- 测试步骤：
	- 失能 PWM
- 预期结果：
	- PWM 输出停止

### 2.15 上升沿起始位置配置测试
- 测试步骤：
	- 配置上升沿起始位置
- 预期结果：
	- 上升沿位置正确

### 2.16 下降沿起始位置配置测试
- 测试步骤：
	- 配置下降沿起始位置
- 预期结果：
	- 下降沿位置正确

### 2.17 使能定时器测试
- 测试步骤：
	- 使能定时器
- 预期结果：
	- 定时器正常工作

### 2.18 清零定时器测试
- 测试步骤：
	- 清零定时器
- 预期结果：
	- 计数值清零

### 2.19 定时器阈值测试
- 测试步骤：
	- 配置定时器阈值
- 预期结果：
	- 达到阈值时正确触发

### 2.20 读取定时器值测试
- 测试步骤：
	- 读取定时器当前值
- 预期结果：
	- 读取值准确

### 2.21 读取版本号测试
- 测试步骤：
	- 读取定时器版本号
- 预期结果：
	- 版本号正确

### 2.22 占空比 0% 测试
- 测试步骤：
	- 配置 PWM 占空比为 0%
- 预期结果：
	- PWM 输出为持续低电平

### 2.23 占空比 50% 测试
- 测试步骤：
	- 配置 PWM 占空比为 50%
- 预期结果：
	- PWM 输出为 50% 高电平，50% 低电平

### 2.24 占空比 75% 测试
- 测试步骤：
	- 配置 PWM 占空比为 75%
- 预期结果：
	- PWM 输出为 75% 高电平，25% 低电平

### 2.25 占空比 100% 测试
- 测试步骤：
	- 配置 PWM 占空比为 100%
- 预期结果：
	- PWM 输出为持续高电平

### 2.26 微秒级计数测试
- 测试步骤：
	- 配置微秒级计数
- 预期结果：
	- 计数精度为微秒级

### 2.27 毫秒级计数测试
- 测试步骤：
	- 配置毫秒级计数
- 预期结果：
	- 计数精度为毫秒级

### 2.28 秒级计数测试
- 测试步骤：
	- 配置秒级计数
- 预期结果：
	- 计数精度为秒级

### 2.29 PWM 功能通道 0 测试
- 测试步骤：
	- 测试 PWM 通道 0 功能
- 预期结果：
	- 通道 0 PWM 输出正常

### 2.30 PWM 功能通道 7 测试
- 测试步骤：
	- 测试 PWM 通道 7 功能
- 预期结果：
	- 通道 7 PWM 输出正常

## 3. 测试结果
- [ ] 2.1 时钟持续有效测试
- [ ] 2.2 自动开关时钟测试
- [ ] 2.3 预分频测试
- [ ] 2.4 计数模式测试
- [ ] 2.5 使能计数器测试
- [ ] 2.6 禁用计数器测试
- [ ] 2.7 清零定时器测试
- [ ] 2.8 溢出中断测试
- [ ] 2.9 定时器阈值测试
- [ ] 2.10 读取定时器值测试
- [ ] 2.11 定时器溢出次数读取测试
- [ ] 2.12 PWM 模式配置测试
- [ ] 2.13 使能 PWM 测试
- [ ] 2.14 失能 PWM 测试
- [ ] 2.15 上升沿起始位置配置测试
- [ ] 2.16 下降沿起始位置配置测试
- [ ] 2.17 使能定时器测试
- [ ] 2.18 清零定时器测试
- [ ] 2.19 定时器阈值测试
- [ ] 2.20 读取定时器值测试
- [ ] 2.21 读取版本号测试
- [ ] 2.22 占空比 0% 测试
- [ ] 2.23 占空比 50% 测试
- [ ] 2.24 占空比 75% 测试
- [ ] 2.25 占空比 100% 测试
- [ ] 2.26 微秒级计数测试
- [ ] 2.27 毫秒级计数测试
- [ ] 2.28 秒级计数测试
- [ ] 2.29 PWM 功能通道 0 测试
- [ ] 2.30 PWM 功能通道 7 测试

## 4. 测试记录
（测试过程中记录具体结果和问题）

## 5. 已知问题

## 1. 编译 gxtest 有问题
- 先同步所有的代码
- 再用 `make.sh` 编译一遍所有的代码
- 再去编译 `gxtest` 报没有定义 `gx_chiptest_set_flag`
- 因为 `chip_test.c` 没有编译，需要手动在 `.config` 中添加 `CONFIG_GX_CHIP_TEST = y`

- 编译出来之后发现卡死了，去掉 av 相关的代码
```diff
--- a/stb/goxceed/main.c
+++ b/stb/goxceed/main.c
@@ -109,8 +109,8 @@ void regitster_chip()
 #endif
 
 #ifdef VIRGO
-       REGISTER_GXSE_CHIP(virgo);
-       REGISTER_AV(virgo);
+//     REGISTER_GXSE_CHIP(virgo);
+//     REGISTER_AV(virgo);
 #endif
 }
 #endif
```
- 不用烧到 flash，直接加载 `.elf` 就行：`arm-none-eabi-gdb ~/goxceed/develop2/gxtest/stb/chiptest/out.elf`
- 起来之后敲命令 `time`

## 2. GPIO 复用
- 首先有一个管脚复用的功能
	- 可以配置管脚的上下拉，输出电流，复用之类的，这里要复用成 gpio
- 然后有一个 gpio 模块
	- `AFIO_EN` 来选择是当成普通 gpio 来用，还是复用成其它模式
	- `AFIO_SEL` 选择对应 GPIO 输出到哪一个复用端口
- 为什么不都放到管脚复用里面去做？
	- timer pwm 算是 gpio 的一个特性
	- 而且 `AFIO_SEL` 只有在芯片集成的时候才知道有哪几种功能，现在 fpga 还不知道，只支持 timer
- 先看下 gpio 能否正常拉高拉低
	- `394932 peri_final3_gpio_FPGA_2024-12-03. tar` 不能拉高拉低
	- `382541 9p_peri_newcode_FPGA_2024-09-03.tar` 可以拉高拉低
	- 子板都是好的
- 再看下 timer 能否正常计数
	- 需要添加管脚复用，把要用的 gpio 复用成 gpio，并且需要将 gxloader 编译成库，gxtest 会用到
	- 并且 gxtest 里面还要添加调用 `gpio_init`，用来初始化 gpio，注意不是 `gx_gpio_init`
```diff
diff --git a/stb/goxceed/nos/init.c b/stb/goxceed/nos/init.c
index b338f2ba2..375b9aa30 100644
--- a/stb/goxceed/nos/init.c
+++ b/stb/goxceed/nos/init.c
@@ -35,6 +35,7 @@ int main(int argc, char **argv)
        io_framework_init();
        stdio_init(); // init stdin, stdout, stderr, so can use fprintf(stdout, "nationalchip")
        irr_init(); // 注释后会出现 OTA 不能引导 APP 问题
+       gpio_init();
        gx_gpio_init();
        gxflash_init();
 
```
```diff
diff --git a/board/virgo/board-fpga/board-init.c b/board/virgo/board-fpga/board-init.c
index 9a70216b..9b527354 100644
--- a/board/virgo/board-fpga/board-init.c
+++ b/board/virgo/board-fpga/board-init.c

@@ -187,6 +187,8 @@ struct mulpin_config_s mulpin_table[] = {
 };
 
 struct gpio_entry_bootloader g_gpio_table[] = {
+       {0 ,  0, GX_GPIO_CONFIG_VALID, GX_GPIO_OUTPUT, GX_GPIO_HIGH},
+       {1 ,  1, GX_GPIO_CONFIG_VALID, GX_GPIO_OUTPUT, GX_GPIO_HIGH},
 /*     {9 ,  1, GX_GPIO_CONFIG_VALID, GX_GPIO_OUTPUT, GX_GPIO_HIGH},
        {10,  0, GX_GPIO_CONFIG_VALID, GX_GPIO_OUTPUT, GX_GPIO_HIGH},
        {11,  3, GX_GPIO_CONFIG_VALID, GX_GPIO_OUTPUT, GX_GPIO_LOW },
```
- 再看 pwm 是否正常
	- 正常了

