# 概述
实时时钟模块主要用于提供时间信息，可在设备主电源掉电后，采用备用电池进行供电。用于维持时间计时、定时、用户信息保存。

## 功能
### 计时功能：
- 特性：
	- 寄存器访问：
		- 正确访问 ip 内部寄存器
	- 计时配置：
		- 增长的计数器；长期、精确的秒表：当以 1Hz 时钟进行计数时，可以跟踪未来长达 136 年的时间
	- 预分频配置：
		- 配置 `RTC_CCR.RTC_PRESCLR_EN = 1` 时，开启预分频计数功能
		- 预分频计数流程：
			- 配置 `RTC_CCR.RTC_PRESCLR_EN = 1` 时，开启预分频计数功能
			- 预分频计数器会一直计数，当计数到 `RTC_CPSR-1` 的时候，会生成 `rtc_psclr_cnt_en` 脉冲。生成的脉冲被 rtc 计数器的时钟使能，也就是这里的脉冲作为 rtc 的计数。可以让 rtc 工作在实际工作频率的 `1/n`

### 闹钟功能：
- 特性：
	- 中断使能配置：
		- `RTC_CMR` 可以被编程，并与内部计数器进行比较
		- 当寄存器的值和内部计数器的值相等，如果启动了中断 `RTC_CCR[0]`，则会产生中断；如果 `RTC_CCR[0]` 设定为 1 时，可以屏蔽此中断


# Register

## RTC_CCVR
- **Name**: RTC Current Counter Value Register
- **Description**: 当前计数器值寄存器
- **Size**: 32 bits
- **Offset**: 0x0
- **Exists**: Always

| 位域  | 名称                    | 访问  | 描述                         |
| --- | --------------------- | --- | -------------------------- |
| x:0 | Current_Counter_Value | R   | 当前 RTC 计数器值。只读，反映实时的计数器状态。 |

**注意**:
- 该寄存器为只读，读取时返回当前计数器的值。
- 计数器宽度由参数 `RTC_CNT_WIDTH` 决定（8 至 32 位），未使用的位读取为 0。

---

## RTC_CMR
- **Name**: RTC Counter Match Register
- **Description**: 计数器匹配寄存器
- **Size**: 32 bits
- **Offset**: 0x4
- **Exists**: Always

| 位域  | 名称            | 访问  | 描述                                                             |
| --- | ------------- | --- | -------------------------------------------------------------- |
| x:0 | Counter_Match | R/W | 中断匹配寄存器。<br>当内部计数器与此寄存器的值相等时，生成中断（若启用）。宽度由 `RTC_CNT_WIDTH` 决定。 |

**注意**:
- 该寄存器可读写，写入时设置匹配值。
- 若 `SLAVE_INTERFACE_TYPE > 1` 且 `SLVERR_RESP_EN = 1`，则写入时需满足保护级别 (`PPROT`)，否则触发 `PSLVERR`。

#### Q：CMR 寄存器的值受预分频的影响吗？
- 当 RTC 内部的计数器与 RTC_CMR 寄存器的值相同，则会产生中断
- 预分频开着的时候：
	- 设定时间不需要 `* dev->pclk`，因为预分频配的值就是 `dev->pclk`，这会导致实际以 1Hz 的频率运行
	- 设定时间直接写到 `CLR` 寄存器
- 预分频没开的时候：
	- 设定时间需要 `* dev->pclk`，因为时间单位是 S，而 32K 的时钟一个周期只有 32us 左右，这里 `*dev->pclk` 就是将单位变为 `S`
	- 将放大后的值写到 `CLR` 寄存器
- 而 RTC 的中断是 `RTC_CMR` 寄存器和 `CCVR` 的值比较，无论是否开启预分频，`CLR` 中写的都是实际值，会更新到 `CCVR` 寄存器；而配置 `RTC_CMR` 的时候也会 `*dev->pclk`，所以这里比较的就是真实值，和预分频无关


---

## RTC_CLR
- **Name**: RTC Counter Load Register
- **Description**: 计数器加载寄存器
- **Size**: 32 bits
- **Offset**: 0x8
- **Exists**: Always

| 位域  | 名称           | 访问  | 描述                                                    |
| --- | ------------ | --- | ----------------------------------------------------- |
| x:0 | Counter_Load | R/W | 作为加载值加载到计数器。写入时，计数器将加载此值并开始计数。宽度由 `RTC_CNT_WIDTH` 决定。 |

**注意**:
- 该寄存器可读写，写入时设置计数器的初始值。
- 若 `SLAVE_INTERFACE_TYPE > 1` 且 `SLVERR_RESP_EN = 1`，则写入时需满足保护级别 (`PPROT`)，否则触发 `PSLVERR`。

---

## RTC_CCR
- **Name**: RTC Control Register
- **Description**: 控制寄存器
- **Size**: 32 bits
- **Offset**: 0xC
- **Exists**: Always

| 位域   | 名称             | 访问      | 描述                                                                                            |
| ---- | -------------- | ------- | --------------------------------------------------------------------------------------------- |
| 31:6 | RSVD_RTC_CCR   | R       | 保留位，读取为 0。                                                                                    |
| 7:5  | rtc_port_level | *varies | 该字段保存的是 dw_apb_rtc 的保护级别值                                                                     |
| 4    | rtc_psclr_en   | *varies | 可选。允许用户控制 rtc 预分频功能的使用。<br>0: 禁用预分频器<br>1: 启用预分频器<br>默认值: 0。                                  |
| 3    | rtc_wen        | *varies | 可选。允许用户在匹配时强制计数器换行，而不是等达到最大计数。<br>0: 计数器在达到最大值时回绕。禁用 wrap<br>1: 计数器在匹配值时回绕。启用 wrap<br>默认值: 0。 |
| 2    | rtc_en         | *varies | 可选。允许用户控制计数器中的计数。<br>0: 禁用计数器。<br>1: 使能计数器。<br>默认值: 0。                                        |
| 1    | rtc_mask       | *varies | 允许用户屏蔽中断的产生。<br>0: 不屏蔽中断。<br>1: 屏蔽中断。<br>默认值: 1（屏蔽）。                                          |
| 0    | rtc_ien        | *varies | 允许用户禁用中断生成。<br>0: 禁用中断生成。<br>1: 使能中断生成（当计数器值与 RTC_CMR 匹配时）。<br>默认值: 0。                        |


**注意**:
- 复位后，所有位为 0。
- 若 `RTC_EN_MODE = 0`，则 RTC_EN 位不存在，计数器始终使能。
- 若 `RTC_WRAP_MODE = 0`，则 RTC_WEN 位不存在。
- 若 `RTC_PRESCLR_EN = 0`，则 RTC_PRESCLR_EN 位不存在。

---

## RTC_STAT
- **Name**: RTC Interrupt Status Register
- **Description**: 中断状态寄存器
- **Size**: 32 bits
- **Offset**: 0x10
- **Exists**: Always

| 位域   | 名称            | 访问  | 描述                                  |
| ---- | ------------- | --- | ----------------------------------- |
| 31:1 | RSVD_RTC_STAT | R   | 保留位，读取为 0。                          |
| 0    | rtc_stat      | R   | 被屏蔽的原始中断状态。<br>0: 无中断。<br>1: 中断已触发。 |

**注意**:
- 该寄存器为只读，反映当前中断状态。
- 读取后不会自动清除中断，需读取 `RTC_EOI` 清除。

---

## RTC_RSTAT
- **Name**: RTC Raw Interrupt Status Register
- **Description**: 原始中断状态寄存器
- **Size**: 32 bits
- **Offset**: 0x14
- **Exists**: Always

| 位域   | 名称             | 访问  | 描述                                   |
| ---- | -------------- | --- | ------------------------------------ |
| 31:1 | RSVD_RTC_RSTAT | R   | 保留位，读取为 0。                           |
| 0    | src_rstat      | R   | 原始中断状态。<br>0: 无中断。<br>1: 中断已触发（未屏蔽）。 |

**注意**:
- 该寄存器为只读，反映未屏蔽的中断状态。
- 即使中断被屏蔽 (`RTC_CCR.RTC_MASK = 1`)，此位仍会置位。

---

## RTC_EOI
- **Name**: RTC End of Interrupt Register
- **Description**: 中断结束寄存器
- **Size**: 32 bits
- **Offset**: 0x18
- **Exists**: Always

| 位域  | 名称      | 访问  | 描述                       |
| --- | ------- | --- | ------------------------ |
| 0   | rtc_eoi | R   | 读取此寄存器以清除中断。<br>读取时返回 0。 |

**注意**:
- 该寄存器为只读，读取操作会清除中断。
- 写入无效。

---

## RTC_COMP_VERSION
- **Name**: RTC Component Version Register
- **Description**: 组件版本寄存器
- **Size**: 32 bits
- **Offset**: 0x1C
- **Exists**: Always

| 位域   | 名称               | 访问 | 描述                   |
|------|------------------|----|----------------------|
| 31:0 | RTC_COMP_VERSION | R  | 组件版本号。只读，固定值。 |

**注意**:
- 该寄存器为只读，反映 RTC IP 的版本信息。
- 具体版本号由 `RTC_VERSION_ID` 参数决定。

---

## RTC_CPSR
- **Name**: RTC Counter Prescaler Register
- **Description**: 计数器预分频寄存器
- **Size**: 32 bits
- **Offset**: 0x20
- **Exists**: When `RTC_PRESCLR_EN = 1`

| 位域  | 名称                      | 访问  | 描述                                                                |
| --- | ----------------------- | --- | ----------------------------------------------------------------- |
| x:0 | Counter_Prescaler_Value | R/W | 预分频值。宽度由 `RTC_PRESCLR_WIDTH` 决定（2 至 32 位）。默认值: `RTC_PRESCLR_VAL`。 |

**注意**:
- 该寄存器仅在 `RTC_PRESCLR_EN = 1` 时存在。
- 写入时设置预分频值，影响计数器更新速率。
- 建议在启用预分频器前编程，避免运行中更改导致意外行为。

---

## RTC_CPCVR
- **Name**: RTC Current Prescaler Counter Value Register
- **Description**: 当前预分频计数器值寄存器
- **Size**: 32 bits
- **Offset**: 0x24
- **Exists**: When `RTC_PRESCLR_EN = 1`

| 位域  | 名称                              | 访问  | 描述                   |
| --- | ------------------------------- | --- | -------------------- |
| x:0 | Current_Prescaler_Counter_Value | R   | 当前预分频计数器值。只读，反映实时状态。 |

**注意**:
- 该寄存器为只读，读取时返回当前预分频计数器的值。
- 仅在 `RTC_PRESCLR_EN = 1` 时存在。

### Q:预分频值和实际工作频率的关系?
![[Pasted image 20250311101219.png]]
- 预分频寄存器位宽为 16 位，rtc 的时钟用的是 32k
- 预分频的功能就是，预分频会从 0 开始计数，计到 N 后 rtc 实际计数一次
- 目前预分频配置为 32768，rtc 时钟为 32k，那么在 32k 的工作频率下计数 32768 次，rtc 实际计数一次，也就实现了工作频率 1Hz
![[Pasted image 20250311101551.png]]






# Program Guid
以下是一个基本流程示例：

1. **复位后初始化**：
    - 写入 RTC_CLR 设置初始值。
    - 写入 RTC_CMR 设置匹配值。
    - 配置 RTC_CCR：设置 RTC_EN = 1（若适用）、RTC_IEN = 1、RTC_MASK = 0。
2. **启动计数器**：
    - 若 RTC_EN_MODE = 1，确保 RTC_CCR.RTC_EN = 1。
3. **等待中断**：
    - 中断触发时，读取 RTC_STAT 确认状态。
    - 读取 RTC_EOI 清除中断。
4. **读取当前时间**：
    - 定期读取 RTC_CCVR 获取计数器值。




# 测试用例
- 产生 1Hz 计数
	- `rtc init`
		- 开启预分频功能，1S RTC 计数一次
	- `rtc write`
		- 设置 RTC 的 tick
	- `rtc read`
		- 获取 RTC 的 tick
- 分频配置
	- 和上面一样
- 开启闹钟功能
	- `rtc set_alarm`
	- 需要确认一下和预分频是否有关



# 测试 case
- 没有头文件 `dw_rtc.c` 的函数直接在 `bootmenu.c` 中调用了
- `rtc init`：`gx_rtc_init()` 
- `rtc write`：`gx_rtc_set_tick(&time)`
- `rtc read`：`gx_rtc_get_tick(&time)`
- `rtc set_alarm`：`gx_rtc_set_alarm_time_s(&time)`




# Q&&A

## Q1：对于预分频开启与关闭对于计数的影响
- 假设预分频开启，驱动中将预分频的值配置为 32768，则 rtc 内部以 32k 的频率计数 32768 次，`RTC_CCVR` 寄存器增加 1 次，由于 32k 频率下一个周期时间为 31.25us，计数 32768 次则刚好达到 1s
- 如果预分频值配置为 n，则 rtc 内部以 32k 的频率计数 n 次，`RTC_CCVR` 寄存器增加 1 次

## Q2：RTC 外部时钟频率对计数的影响
- 假设外部时钟频率为 32K，则 RTC 跳一下是 32K 频率的一个周期时间，跳 32K 次就是 1S
- 假设外部时钟频率为 24M，则 RTC 跳一下是 24M 频率的一个周期时间，跳 24M 次就是 1S
