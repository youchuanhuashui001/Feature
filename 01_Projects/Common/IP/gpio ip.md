# 概述
GPIO 模块，支持多路 GPIO，且每一路都可独立配置中断。支持多种中断类型、快速翻转、IO 功能复用、输入降采样功能。
模块最大支持 8 组 GPIO，每组 GPIO 含 32 个 GPIO 口。具体的 GPIO 需见管脚复用章节。


## 功能
### 读取模块信息功能：
- 特性：
	- 版本获取配置
	- param 参数获取


### 输出功能：
- 特性：
	- 输入输出模式配置
	- 端口输出电平选择配置
	- 低功耗时钟使能配置
	- IO 复用配置



### 输入功能：
- 特性：
	- 输入输出模式配置
	- 中断使能配置
	- 中断使能配置
	- 中断模式选择配置
	- 输入降采样配置
	- 输入电平获取配置
	- 低功耗时钟使能配置




# Register





# Program Guid





# 测试用例
- 版本号读取
- 参数获取
- 输入模式
- 输出模式
- 输出高电平
- 输出低电平
- 普通 IO 模式
- 端口复用输出模式
- 时钟一直有效
- 自动开关时钟
- 输入模式
- 输出模式
- 使能中断
- 失能中断
- 高电平触发
- 低电平触发
- 上升沿触发
- 下降沿触发
- 使能降采样
- 失能降采样




# 测试 case
- 




# Q&&A
## Q：什么是线程中断？还有双线程功能？

## Q：GPIO 降采样指的是降低 gpio 输入功能时的采样功能
- 为什么需要这个功能？

## Q：pdm 是干嘛的？只有 GX_V1 的接口有

## Q：GPIO 的中断可以同时选择多种模式？
- 驱动中看起来可以把高电平、低电平、上升沿、下降沿都配上


## Q：PWM 功能只有 CONFIG_GPIO_GX_V2，没有 V3 的，是不是只需要复用成  pwm 功能，然后使用 timer 来输出 pwm？
- 看起来是这样的，gpio 只需要配置复用为 pwm 功能，由 timer 来控制输出 pwm
```c
static int test_14_test(const char *cstr, int autoflag)
{
	unsigned int time_id;
	//00 Function-PWM功能[02.pwm]- Feature-定时器预分频配置[02.Frequency_div] = 预分频
	time_id = gx_timer_enable_pwm(-1, 1000000, 500000);
	gx_gpio_afio_select(0, time_id);
	gx_gpio_afio_enable(0);
	printf("定时器预分频周期为1us, GPIO0 PWM周期为1000us 上升沿为500us。输入任意键结束\n");
	getc();
	gx_timer_disable_pwm(time_id);
	gx_gpio_afio_disable(0);

	return 0;
}

```

- gpio 怎么配成 pwm 功能？


## Q：GPIO 专门的还有 pad_mux 功能，用作初始化引脚复用
- 放在 `driver/padmux/virgo_pad_mux.c`


## Q：降采样怎么测试？没有降采样的驱动，测试代码也没有
- 配置为降采样后，IO 检测输入从 0~1 或者从 1~0 有一定延迟


## Q：Virgo MPW PIN 如何配置成开漏或推挽输出？默认是什么状态？
- pin 是推挽还是开漏取决于当前的功能
- 例如：
	- 当前 pin 配置为 gpio 功能，默认是推挽输出
	- 当前 pin 配置为 iic 功能，默认是开漏输出
- 具体什么功能，可以通过文档查看，带有 od 标识的就是开漏输出，否则就是推挽输出。输入是直通的，不区分开漏、推挽
![[Pasted image 20250826105809.png]]

