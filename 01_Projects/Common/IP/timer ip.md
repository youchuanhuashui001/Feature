# 概述
Timer 模块是一个通过可编程预分配器驱动的 32 位计数器构成，包含 16 位分频计数器、32 位向上计数器以及 32 位中断溢出次数计数器。适用于多种场合，包括普通计时功能和 PWM 生成功能。

一个模块有 N (`N<=8 `，由芯片参数决定，可以通过 `timer_parameter` 寄存器读取) 路定时器，没路定时器可以配置为计数模式或者 PWM 模式。

## 功能
### PWM 功能：
- 特性：
	- PWM 模式使能配置：
		- 每一路对应 1bit，共 8 路
		- 使能、禁用 PWM
		- 最大时钟周期、最小时钟周期
		- PWM 最大占空比、最小占空比
		- 关联寄存器：`TIMER_PWN_CTL.pwm_en[7:0]、TIMER_CNT`
	- 定时器模式配置：
		- 模式 0 为定时器，模式 1 为 PWM
		- 关联寄存器：`TIMER_CTL.mode_sel`
	- 定时器阈值配置：
		- 当 `TIM_CNT` 的值为 N 时，计数周期为 `N+1`
		- 定时器仅支持向上加一计数，计数值达到设定阈值会让 `TIMER_ACC +1` ，产生中断
		- 在 PWM 模式时，这个值就是 PWM 的周期
		- 关联寄存器：`TIMER_CNT`
	- 定时器 CNT 复位配置：
		- 0 无效；1 计数器清零
		- 将预分频计数器、定时器计数器、中断溢出次数计数器的值清零，PWM 模式也是相同
		- 关联寄存器：`TIMER_CTL.cnt_clr`
	- 定时器获取 CNT 配置：
		- 定时器当前值，RO
		- 先读取 `TIMER_VAL`，再读取 `TIMER_ACC`；读取 `TIMER_VAL` 时会对 `ACC_CNT` 计数器进行快照
			- **!!! 这里指的是会把溢出的哪些都算上吗？!!!**
		- 关联寄存器：`TIMER_VAL`
	- 定时器预分频配置：
		- 当 `TIM_PRE` 的值为 `N` 时，分频为 `N+1`
		- `TIMER_PRE` 位宽是 16bit
		- 当输入时钟为 100MHz 时，计数最小颗粒度为 `100MHz/2^16`
		- 关联寄存器：`TIMER_PRE`
	- 计数模式使能配置：
		- 0 禁用定时器；1 使能定时器
		- 关闭计数器使能，计数器保持为当前值，不会清零
		- 关联寄存器：`TIMER_CTL.mode_sel、TIMER_CTL.timer_en`
	- 定时器溢出中断配置：
		- `TIMER_INT_EN=1` 开启中断
		- `TIMER_INT_STATUS=1` 出现中断，写 1 清零
		- 关联寄存器：`TIMER_INT_EN、TIMER_INT_STATUS`
	- 定时器溢出次数读取配置：
		- 记录定时器溢出次数
		- 该寄存器是实际 `ACC` 计数器的快照，当 `TIM_VAL` 寄存器被读时会更新
		- `PWM` 模式下，溢出次数寄存器**不可?**
		- 关联寄存器：`TIMER_ACC`
	- PWM 设置上升沿起始位置配置：
		- 设置 PWM 上升沿起始位置
		- 关联寄存器：`TIMER_PWM_RISE`
	- PWM 设置下降沿起始位置配置：
		- 设置 PWM 下降沿起始位置
		- 关联寄存器：`TIMER_PWM_FAIL`
	- 定时器时钟低功耗使能配置：
		- 0 时钟一直有效；1 模块根据工作状态自动开关时钟
		- 关联寄存器：`TIMER_CTL.global_gate_en`



### 计数功能：
- 特性：
	- 许多都是重叠的，这里不描述了


### 读取版本功能：
- 特性：
	- 读取版本配置：
		- 版本号为 `32'h0001_0000`
		- 关联寄存器：`TIMER_VERSION、TIMER_PARAMTER`



# Register




# Program Guid


## 计数功能

- 设置低功耗使能 `TIMER_CTL.global_gate_en=1`
- 设置定时器预分频 `TIMER_PRE`
- 选择计数模式 `TIMER_CTL.mode_sel=0`
- 设置定时器阈值 `TIMER_CNT`
- 计数模式使能 `TIMER_CTL.timer_en=1`
- 获取计数器溢出次數 `TIMER_ACC`
- 获得定时器 cnt 值 `TIMER_VAL`

## PWM 功能

- 设置低功耗使能 `TIMER_CTL.global_gate_en=1`
- 设置定时器预分频 `TIMER_PRE`
- 选择计数模式 `TIMER_CTL.mode_sel=1`
- 设置定时器阈值 `TIMER_CNT`
- PWM 设置上升沿起始位置 `TIMER_PWM_RISE`
- PWM 设置下降沿计起始位置 `TIMER_PWM_FALL`
- 计数模式使能 `TIMER_CTL.timer_en=1`
- PWM 模块使能 `TIMER_PWM_CTL.pwm_en[7:0]`


# 测试用例
- 




# 测试 case
- 




# Q&&A
## Q：timer 寄存器有 8 个 timer，每个 timer 有 1 路 pwm 吗？还是一个 timer 有 8 路 pwm？
- 每个 timer 只有 1 路 pwm

## Q：测试用例中 gx_timer_register 填参数为 100 的时候定时时间是多长？
- 单位是 ms