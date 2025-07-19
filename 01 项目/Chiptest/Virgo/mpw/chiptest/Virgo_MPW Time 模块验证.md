---
tags:
  - Virgo_MPW模块验证
---
# 验证进度：
- 总验证点:: 33
- 已完成:: 33
- 备注:: linux 需要动态调压


- 编译：
	- 先同步所有代码
	- 编译 gxloader release 失败，
- linux dvfs 动态调压：不需要 adc 回采
	- https://www.digikey.tw/zh/blog/the-i-mx-8m-plus-system-on-module-dvfs-support
	- https://blog.csdn.net/u012294613/article/details/133275405?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_baidulandingword~default-0-133275405-blog-136207792.235^v43^pc_blog_bottom_relevance_base1&spm=1001.2101.3001.4242.1&utm_relevant_index=2



# gxloader Timer 输出 PWM 驱动 DC-DC 电路动态调整 CPU、GPU 电压
涉及到 padmux，gpioafio，pwm，需要提供几段固定的电压。
- 用法：
	- CONFIG_CPU_GPU_DY_POWER=y
	- `configure_cpu_gpu_voltage(int cpu_voltage, int gpu_voltage);`
- padmux：
	- 不同的板级配置不同的 board-init. c
	- 默认的时候配置成 0.9v
- pwm 需要预留两个
	- timer_malloc 的时候预留 Channel1、2



# 验证项



## 时钟一直有效
- 测试完成

## 自动开关时钟
- 测试完成

## 预分频
- 测试完成

## 计数模式
- 测试完成

## 使能计数器
- 测试完成

## 禁用计数器
- 测试完成

## 清零定时器
- 测试完成

## 溢出中断
- 测试完成

## 定时器阀值
- 测试完成

## 读取定时器值
- 测试完成

## 定时器溢出次数读取
- 测试完成

## 时钟一直有效
- 测试完成

## 自动开关时钟
- 测试完成

## 预分频
- 测试完成

## PWM模式
- 测试完成

## 使能PWM
- 测试完成

## 失能PWM
- 测试完成

## 配置上升沿起始位置
- 测试完成

## 配置下降沿起始位置
- 测试完成

## 使能定时器
- 测试完成

## 清零定时器
- 测试完成

## 定时器阀值
- 测试完成

## 读取定时器值
- 测试完成

## 读取版本号
- 测试完成

## 占空比0
- 测试完成

## 占空比50
- 测试完成

## 占空比75
- 测试完成

## 占空比100
- 测试完成

## 微妙级计数
- 测试完成

## 毫秒级计数
- 测试完成

## 秒级计数
- 测试完成

## PWM功能通道0
- 测试完成

## PWM功能通道7
- 测试完成




# 问题
## 1. 编译 gxtest 有问题


- 测试代码路径
    - gxtest: stb/chiptest/cases/TIME_V100/
- 编译方法：
	- 到 `gxtest/stb/chiptest/` 目录下
    - 编译./build config -m TIME_V100 
    - ./build arm nos virgo 3502 dvbs bin debug
    - 依赖 gxloader 的 nos lib，需要先编译 gxloader
- 编译后生成的整 bin 在 `/home/tanxzh/goxceed/develop2/gxtest/stb/chiptest/output/image`
- 也可以直接 gdb 加载 `chiptest` 目录下的 `out.elf`

- minicom 不显示汉字：
	- `minicom -R utf8`




- 先同步所有的代码
- 再用 `make.sh` 编译一遍所有的代码
- 再去编译 `gxtest` 报没有定义 `gx_chiptest_set_flag`
- 因为 `chip_test.c` 没有编译，需要手动在对应的 `config` 中添加 `CONFIG_GX_CHIP_TEST = y`

- 编译 av 报错，去掉 av 相关的代码
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



- 报错 undefine reference to debug
	- 最后不加上 debug，就可以了





## 2. GPIO 复用
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
--- a/board/virgo/board-3502/board-init.c
+++ b/board/virgo/board-3502/board-init.c

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


## 3. timer、pwm、gpio 的对应关系
- Virgo MPW 中放有 2 个 Timer 模块，每个 Timer 模块有  8 个 PWM 通道，可以用来输出 PWM
- 每个 PWM 可以单独输出到某个引脚，需要先将该引脚在 Pinmux 中配置成 GPIO 功能，然后再将 gpio 复用成 timer 功能
	- 要用哪个 pwm，就在 `gx_timer_enable_pwm(x, period, duty)` 的参数中传入几，其中 0 用于延时 `gx_mdelay` 不可使用，用其他的 pwm 即可
	- 要用哪个 gpio 输出，先将物理 gpio 和虚拟 gpio 进行隐射，映射完成后通过 `gx_gpio_afio_select(virt_gpio, pwmx)` 将 virt_gpio 复用成哪一路 pwm
	- 然后启用 `gx_gpio_afio_enable(virt_gpio)` 启动复用
![[Pasted image 20250622181344.png]]



## 4. gxtest 跑起来之后没有 pwm 输出
- 映射到 gpio0 输出，发现没有 pwm

- 解决方法：
	- 将整 bin 烧到 flash，然后从 flash 启动，可以正常输出了
	- 可能是gdb加载的时候不会跑 gpio 初始化了