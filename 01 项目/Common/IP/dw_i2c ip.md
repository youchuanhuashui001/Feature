
# 概述
I2C 总线是双线串行接口，由串行数据线 (SDA) 和串行时钟 (SCL) 组成。这些电线在连接到总线的设备之间传输信息。每个设备都有唯一的地址识别，并且可以作为发射器或接收器运行，具体取决于设备的功能。在执行数据传输时，设备也可以被视为主设备或从设备。**主机是一种在总线上启动数据传输并生成时钟信号以允许该传输的设备。** 此时，任何被寻址的设备都被视为从属设备。


> [!note]
> `DW_apb_i2c` 只能被编程为在主模式或从模式下运行。不支持同时作为主机和从机运行。


`DW_apb_i2c` 可以工作在：
- 标准模式 (数据速率 0~100kb/s)
- 快速模式 (数据速率小于或等于 400kb/s)
- 快速模式 (数据速率小于或等于 1000kb/s)
- 高速模式 (速度速率小于或等于 3.4Mb/s)
- 超快速模式 (速度速率小于或等于 5Mb/s)。

i2c 设备也存在模式的概念，设备会支持多种模式。
- i2c 设备可以向下兼容，i2c 设备支持快速，但用标准的 i2c 协议跑，没问题
- i2c 协议不能向上兼容，i2c 设备支持快速，但用高速的 i2c 协议跑，会有问题
- 超快速模式设备不向下兼容，i2c 设备支持超快速，不能用标准、高速模式跑

多主设备：多个主设备能够同时在总线上共存而不发生冲突或数据丢失
仲裁：预定义的程序，每次只授权一个主机控制总线。
同步：同步两个或多个主机提供的时钟信号的预定义程序。

## I2C 行为
主机负责产生时钟并控制数据的传输。
从机负责向主机发送数据或从主机接收数据。
<font color="#ff0000">ACK 由接收数据的设备发送，该设备可以是 master 或者 slave。</font>
每个 slave 有一个由系统设计者确定的唯一地址。当 master 想要和 slave 通信，master 发送一个 `START/RESTART` 条件，然后发送 `slave address` 和一个控制位 `(R/W)`，以确认 master 想要从 slave 传输数据或接收数据。然后 slave 在地址之后发送一个 `ACK` 脉冲。
如果 master 向 slave 写入数据，则 receiver 获得一个字节的数据。传输持续进行，直到 master 传输 `STOP` 条件来终止传输。如果 master 在从 slave 接收数据，slave 发送一个字节数据给 master，并且 master 使用 `ACK` 脉冲来确认传输。该传输持续进行，直到 master 在最后一个字节发送后不确认 `NACK` ，则停止传输。
接收到后，主机发出 `STOP` 条件，或者发出 `RESTART` 信号后寻址另一个主机。
![[Pasted image 20250312161023.png]]
> [!note]
> 在超快速度模式下，主机只能向从机发出写入传输，而从机时钟不会确认 `NACK`。在此模式下不允许读取传输。


### START and STOP 产生
当 I2C 作为主机运行时，将数据放到 `tx fifo` 会导致 `DW_apb_i2c` 在 `I2C` 总线上产生 ` START ` 状态。
允许 `tx fifo` 为空导致 `DW_apb_i2c` 在 `I2C` 总线上产生一个 ` STOP ` 状态。
当 I2c 作为从机运行时，`DW_apb_i2c` 不会根据协议生成 `START、STOP` 状态。然而，如果向 `DW_apb_i2c` 发出读取请求，将会让 `SCL` 保持为低位，直到向其提供数据。



### Combined Formats
`DW_apb_i2c` 支持 7bit 和 10bit 寻址模式下的混合读写组合格式传输。
不支持 7bit 和 10bit 混着用。


## I2C 协议

### 启动和停止条件
当总线空闲时，SCL 和 SDA 信号均通过总线上的外部上拉电阻被拉高。当主机想要在总线上启动传输时，主机会发出 START 条件。这定义为 SCL 为 1 时 SDA 信号从高到低的转换。当主机想要终止传输时，主机会发出 STOP 条件。这定义为 SCL 为 1 时 SDA 线从低到高的转换。图 2-4 显示了 START 和 STOP 条件的时序。
当总线上正在传输数据时，当 SCL 为 1 时，SDA 线必须稳定。
![[Pasted image 20250312162104.png]]

### Slave 地址协议

#### 7bit：
在 7 位地址格式中，第一个字节的前 7 位 (bits 7:1) 设置 slave 地址，LSB bit (bit0) 是 `R/W` 位。
`R = 1； W = 0`
![[Pasted image 20250312162229.png]]

#### 10bit：
在 10 位地址格式中，用两个字节来设备 10 位地址。第一个字节的传输，前 5 位固定为 11110 表示是 10bit 传输。接下来的 2 位是 slave 地址的 bit9、bit8，位 0 是 `R/W`。第二个字节是 slave 地址的 `bit7~0`。
![[Pasted image 20250312162626.png]]

### 发送和接收协议
<font color="#ff0000">所有数据均以字节格式传输，每次数据传输所传输的字节数没有限制。</font>
主机发送地址和 R/W 位或主机传输一个字节的数据给从机后，接收器必须以确认信号 `ACK` 进行响应。当接收器未发 `ACK`，master 将发出 `stop` 来中止传输。

#### master 发数据，由 slave 回 ack

如果 master 正在发数据，slave 在接收到每个数据字节后都会用 `ack` 脉冲响应 master。
![[Pasted image 20250312162935.png]]

#### master 收数据，由 master 回 ack
如果 master 正在收数据，master 在接收到每个数据字节后都会用 `ack` 脉冲响应 slave。
![[Pasted image 20250312163112.png]]

### tx fifo 管理和 start、stop、restart 产生
当作为 master 运行时，`DW_apb_i2c` 支持两种 `tx fifo` 管理模式：
- `IC_EMPTYFIFO_HOLD_MASTER_EN == 0`
	- 只要 Tx FIFO 变空，该组件就会在总线上生成 STOP。如果启用了 RESTART 生成功能，则当 Tx FIFO 命令中的传输方向从读取变为写入或反之亦然时，组件将生成 RESTART；如果未启用 RESTART，则在这种情况下会产生 STOP，然后是 START。
- `IC_EMPTYFIFO_HOLD_MASTER_EN == 1`
	- 当 IC_EMPTYFIFO_HOLD_MASTER_EN 的值为 1 时，如果 Tx FIFO 变空，则组件不会产生 STOP；在这种情况下，组件将保持 SCL 线处于低位，从而暂停总线直到 Tx FIFO 中有新的条目可用。仅当您通过设置写入 IC_DATA_CMD 寄存器的命令的第 9 位（停止位）来明确请求时，才会生成 STOP 条件。


### 多 master 仲裁

DW_apb_i2c 总线协议允许多个主机驻留在同一个总线上。如果同一条 I²C 总线上有两个主机，且它们都试图通过同时生成 START 条件来同时控制总线，则需要进行仲裁程序。一旦主机（例如，微控制器）控制了总线，其他主机就无法控制总线，除非第一个主机发送 STOP 条件并将总线置于空闲状态。

当 SCL 线为 1 时，仲裁在 SDA 线上进行。当一个主机传输 1 而另一个主机传输 0 时，该主机将失去仲裁并关闭其数据输出阶段。失去仲裁的主机可以继续生成时钟，直到字节传输结束。如果两个主设备都寻址同一个从设备，则仲裁可以进入数据阶段。

当检测到它已失去对另一个主机的仲裁时，DW_apb_i2c 停止生成 SCL（ic_clk_oe）。


### IC_CLK 频率配置
当 `DW_apb_i2c` 配置为 `Stand、Fast、Fast-Mode Plus、High Speed` master，这些 `*CNT` 寄存器必须在任何 `i2c` 总线传输前按顺序配置好，以确保适合的 `I/O` 时序。这些寄存器如下：
```
IC_SS_SCL_HCNT
IC_SS_SCL_LCNT
IC_FS_SCL_HCNT
IC_FS_SCL_LCNT
IC_HS_SCL_HCNT
IC_HS_SCL_LCNT
```

当 `DW_apb_i2c` 配置为 `Ultra-Fast Mode` master，也是一样必须要先配置好。
```
IC_UFM_SCL_HCNT
IC_UFM_SCL_LCNT
```

> [!note]
> -  `START、STOP` 和 `RESTART` 的 ` tBUF ` 时序和建立/保持时间使用相应速度模式的 `*HCNT 或 *HLCNT` 寄存器的配置。
> - 如果 `DW_apb_i2c` 作为 slave 运行，则无需对任何 `*CNT` 寄存器编程，因为这些寄存器只在 i2c 作为 master 时的时序要求。







### SDA 保持时间
I2C 协议规范要求在**标准模式和快速模式下 SDA 信号 (tHD; DAT) 的保持时间为 300ns**，在高速模式和快速模式下，保持时间足够长以弥合 SCL 下降沿的逻辑 1 和逻辑 0 之间的未定义部分。
SCL 和 SDA 信号的电路板延迟可能意味着保持时间要求在 I2C 主器件上得到满足，但在 I2C 从器件上却未得到满足（反之亦然）。由于每个应用程序都会遇到不同的电路板延迟，DW_apb_i2c 包含一个软件可编程寄存器 (IC_SDA_HOLD)，以实现对 SDA 保持时间的动态调整。
`位[15:0]` 用于控制在从机模式和主机模式下传输期间 SDA 的保持时间（SCL 从高电平变为低电平之后）。
当接收器中的 SCL 为高电平时（无论是主模式还是从模式），`位[23:16]` 用于延长 SDA 转换（如果有）。

![[Pasted image 20250313093909.png]]

#### `IC_CLK_FREQ_OPTIMIZATION = 0` 时标准 (SS)、快速 (FS)、FM+ 和高速模式的最小高低计数
当 `DW_apb_i2c` 作为 master 运行时，在发送和接收传输中：
- `IC_SS_SCL_L_CNT、IC_FS_SCL_LCNT` 寄存器的值必须大于 `IC_FS_SPKLEN + 7`
- `IC_SS_SCL_HCNT、IC_FS_SCL_HCNT` 寄存器的值必须大于 `IC_FS_SPKLEN + 5`
- 如果对组件编程支持高速，则 `IC_HS_SCL_LCNT` 寄存器的值必须大于 `IC_HS_SPKLEN + 7`
- 如果对组件编程支持高速，则 `IC_HS_SCL_HCNT` 寄存器的值必须大于 `IC_HS_SPKLEN + 5`


#### `IC_CLK_FREQ_OPTIMIZATION = 1` 时 SS、FS、FM+和高速模式的最小高低计数
- 总 SCL 低电平周期由 `DW_apb_i2c` 为 `IC_*LCNT` 寄存器值驱动。硬件不支持将小于 6 的值写入到 `IC_*LCNT` 寄存器。此外，`DW_apb_i2c` 支持的最小 SCL 低电平时间为 `6 ic_clk` 周期。
- 有 `DW_apb_i2c` 驱动的总 `SCL` 高电平周期为 `IC_*HCNT 寄存器值+SPKLEN + 3 `。此外，`DW_apb_i2c` 支持的最小 `SCL` 低电平时间为 `5 ic_clk 周期 [1+1+1+3]`。

`DW_apb_i2c` 产生的 SCL 的总高时和低时也受到 `SCL` 线上的上身时间和下降时间的影响。`SCL` 上升和下降时间参数因外部因素而异，例如：
- IO 驱动程序的特点
- 上拉电阻值
- SCL 线上的总电容

#### 最小 `IC_CLK` 频率
在 slave 模式，需要对 `IC_SDA_HOLD(Thd;dat)` 和 `IC_SDA_SETUP(Tsu:dat)` 进行编程，以满足 I2C 协议的时序要求。
以下示例适用于 `IC_FS_SPKLEN` 和 `IC_HS_SPKLEN` 编程为 2 的情况：
##### SM、FM、FM+和高速模式，`IC_CLK_PREQ_OPTIMIZATION = 1`
###### master
本节介绍 DW_apb_i2c 为每种速度模式支持的最小 ic_clk 频率以及相关的高计数值和低计数值。以下示例适用于 IC_FS_SPKLEN = 1、IC_HS_SPKLEN = 1 和 IC_CLK_FREQ_OPTIMIZATION = 1 的情况。

以下是快速模式下最小 DW_apb_i2c ic_clk 值的条件和计算：
- 快速模式的数据速率为 `400kb/s`，意味着 `SCL` 周期为 `1/400KHz = 2.5us`
- 最小 hcnt 值为 5 作为种子值; `IC_HCNT_FS = 5`
- 协议最小 `SCL` 高电平和低电平时间：
	- MIN_SCL_LOWtime_FS = 1300ns
	- MIN_SCL_HIGHtime_FS = 600ns
![[Pasted image 20250313100514.png]]
![[Pasted image 20250313100537.png]]



## 功能








# Register
## COMP_PARAM_1
bit8~
bit
### DW_IC_INTR_MASK
### DW_IC_COMP_TYPE



Below is a complete and organized list of all registers for the DesignWare DW_apb_i2c IP, formatted consistently with the provided examples (WDT_CCVR and WDT_CRR). Each register includes its name, description, size, offset, existence condition, bit field details, and notes.

---

## IC_CON
- **Name**: I2C Control Register
- **Description**: I2C 控制寄存器
- **Size**: 32 bits
- **Offset**: 0x0
- **Exists**: Always

| 位域   | 名称               | 访问 | 描述                                                                 |
|-------|-------------------|-----|--------------------------------------------------------------------|
| 31:10 | RSVD_IC_CON       | R   | 保留位，读取为 0                                                    |
| 9     | IC_SLAVE_DISABLE  | R/W | 禁用从机模式。<br>0: 启用从机模式<br>1: 禁用从机模式<br>默认值: 1     |
| 8     | IC_RESTART_EN     | R/W | 启用重启条件。<br>0: 禁用<br>1: 启用<br>默认值: 1                    |
| 7     | IC_10BITADDR_MASTER | R/W | 主机模式下使用 10 位地址。<br>0: 7 位地址<br>1: 10 位地址<br>默认值: 0 |
| 6     | IC_10BITADDR_SLAVE | R/W | 从机模式下使用 10 位地址。<br>0: 7 位地址<br>1: 10 位地址<br>默认值: 0 |
| 5:4   | SPEED             | R/W | 速度模式。<br>00: 保留<br>01: 标准模式 (100 Kb/s)<br>10: 快速模式 (400 Kb/s)<br>11: 高速模式 (3.4 Mb/s)<br>默认值: 2 |
| 3     | MASTER_MODE       | R/W | 主机模式启用。<br>0: 禁用<br>1: 启用<br>默认值: 1                    |
| 2:1   | RSVD_IC_CON       | R   | 保留位，读取为 0                                                    |
| 0     | IC_SLAVE_DISABLE  | R/W | 禁用从机模式。<br>0: 启用从机模式<br>1: 禁用从机模式<br>默认值: 1     |

**注意**:
- 该寄存器控制 I2C 模块的主要操作模式和特性。
- 部分位域可能受配置参数影响，例如 IC_MAX_SPEED_MODE。

### IC_CON 注意事项
- `DW_IC_CON_RESTART_EN` ：
	- 用于控制 I2C 控制器是否支持重复起始条件 (Repeated START)，也就是说允许主设备在不释放总线的情况下发送新的起始信号
	- 对于某些需要先写后读的操作非常重要
	- 实际应用场景：
		- 先发送寄存器地址 (写操作)，然后不释放总线直接发起读操作
		- 多字节读取：在读取多个字节时保持总线控制权
		- 总线仲裁：避免在关键操作中是去总线控制权
	- 不启用时的影响：
		- 每次操作之间必须有 STOP 条件
		- 无法执行原子性的写后读操作
		- 可能导致某些设备无法正常工作

---

## IC_TAR
- **Name**: I2C Target Address Register
- **Description**: I2C 目标地址寄存器
- **Size**: 32 bits
- **Offset**: 0x4
- **Exists**: Always

| 位域   | 名称            | 访问 | 描述                                                                 |
|-------|----------------|-----|--------------------------------------------------------------------|
| 31:12 | RSVD_IC_TAR    | R   | 保留位，读取为 0                                                    |
| 11    | SPECIAL        | R/W | 特殊功能启用。<br>0: 禁用<br>1: 启用<br>默认值: 0                    |
| 10    | GC_OR_START    | R/W | 通用呼叫或 START 字节。<br>0: 通用呼叫<br>1: START 字节<br>默认值: 0 |
| 9:0   | TAR            | R/W | 目标地址。<br>7 位地址: [6:0]<br>10 位地址: [9:0]<br>默认值: 0x55    |

**注意**:
- 该寄存器设置主机模式下要通信的从机地址。
- SPECIAL 和 GC_OR_START 位用于特殊传输模式。

---

## IC_SAR
- **Name**: I2C Slave Address Register
- **Description**: I2C 从机地址寄存器
- **Size**: 32 bits
- **Offset**: 0x8
- **Exists**: Always

| 位域   | 名称            | 访问 | 描述                                                                 |
|-------|----------------|-----|--------------------------------------------------------------------|
| 31:10 | RSVD_IC_SAR    | R   | 保留位，读取为 0                                                    |
| 9:0   | SAR            | R/W | 从机地址。<br>7 位地址: [6:0]<br>10 位地址: [9:0]<br>默认值: 0x55    |

**注意**:
- 该寄存器设置从机模式下的地址。

---

## IC_HS_MADDR
- **Name**: I2C High Speed Master Mode Code Address Register
- **Description**: I2C 高速主机模式代码地址寄存器
- **Size**: 32 bits
- **Offset**: 0xc
- **Exists**: When IC_MAX_SPEED_MODE = 3

| 位域   | 名称               | 访问 | 描述                                       |
|-------|-------------------|-----|-------------------------------------------|
| 31:3  | RSVD_IC_HS_MADDR  | R   | 保留位，读取为 0                           |
| 2:0   | HS_MADDR          | R/W | 高速模式主机代码地址。<br>默认值: 1          |

**注意**:
- 仅在高速模式下使用，用于设置主机代码。

---

## IC_DATA_CMD
- **Name**: I2C Data Buffer and Command Register
- **Description**: I2C 数据缓冲和命令寄存器
- **Size**: 32 bits
- **Offset**: 0x10
- **Exists**: Always

| 位域   | 名称               | 访问 | 描述                                       |
|-------|-------------------|-----|-------------------------------------------|
| 31:11 | RSVD_IC_DATA_CMD  | R   | 保留位，读取为 0                           |
| 10    | RESTART           | R/W | 重启条件。<br>0: 不发送<br>1: 发送<br>默认值: 0 |
| 9     | STOP              | R/W | 停止条件。<br>0: 不发送<br>1: 发送<br>默认值: 0 |
| 8     | CMD               | R/W | 命令。<br>0: 写<br>1: 读<br>默认值: 0         |
| 7:0   | DAT               | R/W | 数据。<br>默认值: 0                         |

**注意**:
- 该寄存器用于发送和接收数据，以及控制传输的命令。

---

## IC_SS_SCL_HCNT
- **Name**: Standard Speed I2C Clock SCL High Count Register
- **Description**: 标准速度 I2C 时钟 SCL 高电平计数寄存器
- **Size**: 32 bits
- **Offset**: 0x14
- **Exists**: Always

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:16 | RSVD_IC_SS_SCL_HCNT | R   | 保留位，读取为 0                           |
| 15:0  | HCNT                | R/W | SCL 高电平计数。<br>默认值: 4000            |

**注意**:
- 用于设置标准模式下 SCL 高电平的持续时间。

### IC_SS_SCL_HCNT 注意事项：
- 此寄存器必须在没有任何 i2c 总线传输之前配置好，确保 io 的时间。
- 第一次编程必须先写低字节，第二次再写高字节。

---

## IC_SS_SCL_LCNT
- **Name**: Standard Speed I2C Clock SCL Low Count Register
- **Description**: 标准速度 I2C 时钟 SCL 低电平计数寄存器
- **Size**: 32 bits
- **Offset**: 0x18
- **Exists**: Always

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:16 | RSVD_IC_SS_SCL_LCNT | R   | 保留位，读取为 0                           |
| 15:0  | LCNT                | R/W | SCL 低电平计数。<br>默认值: 4700            |

**注意**:
- 用于设置标准模式下 SCL 低电平的持续时间。

---

## IC_FS_SCL_HCNT
- **Name**: Fast Speed I2C Clock SCL High Count Register
- **Description**: 快速速度 I2C 时钟 SCL 高电平计数寄存器
- **Size**: 32 bits
- **Offset**: 0x1c
- **Exists**: Always

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:16 | RSVD_IC_FS_SCL_HCNT | R   | 保留位，读取为 0                           |
| 15:0  | HCNT                | R/W | SCL 高电平计数。<br>默认值: 600             |

**注意**:
- 用于设置快速模式下 SCL 高电平的持续时间。

---

## IC_FS_SCL_LCNT
- **Name**: Fast Speed I2C Clock SCL Low Count Register
- **Description**: 快速速度 I2C 时钟 SCL 低电平计数寄存器
- **Size**: 32 bits
- **Offset**: 0x20
- **Exists**: Always

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:16 | RSVD_IC_FS_SCL_LCNT | R   | 保留位，读取为 0                           |
| 15:0  | LCNT                | R/W | SCL 低电平计数。<br>默认值: 1300            |

**注意**:
- 用于设置快速模式下 SCL 低电平的持续时间。

---

## IC_HS_SCL_HCNT
- **Name**: High Speed I2C Clock SCL High Count Register
- **Description**: 高速 I2C 时钟 SCL 高电平计数寄存器
- **Size**: 32 bits
- **Offset**: 0x24
- **Exists**: When IC_MAX_SPEED_MODE = 3

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:16 | RSVD_IC_HS_SCL_HCNT | R   | 保留位，读取为 0                           |
| 15:0  | HCNT                | R/W | SCL 高电平计数。<br>默认值: 60              |

**注意**:
- 用于设置高速模式下 SCL 高电平的持续时间。

---

## IC_HS_SCL_LCNT
- **Name**: High Speed I2C Clock SCL Low Count Register
- **Description**: 高速 I2C 时钟 SCL 低电平计数寄存器
- **Size**: 32 bits
- **Offset**: 0x28
- **Exists**: When IC_MAX_SPEED_MODE = 3

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:16 | RSVD_IC_HS_SCL_LCNT | R   | 保留位，读取为 0                           |
| 15:0  | LCNT                | R/W | SCL 低电平计数。<br>默认值: 160             |

**注意**:
- 用于设置高速模式下 SCL 低电平的持续时间。

---

## IC_INTR_STAT
- **Name**: I2C Interrupt Status Register
- **Description**: I2C 中断状态寄存器
- **Size**: 32 bits
- **Offset**: 0x2c
- **Exists**: Always

| 位域   | 名称               | 访问 | 描述                                       |
|-------|-------------------|-----|-------------------------------------------|
| 31:12 | RSVD_IC_INTR_STAT | R   | 保留位，读取为 0                           |
| 11    | GEN_CALL          | R   | 通用呼叫中断。<br>0: 无<br>1: 有            |
| 10    | START_DET         | R   | 启动条件检测中断。<br>0: 无<br>1: 有        |
| 9     | STOP_DET          | R   | 停止条件检测中断。<br>0: 无<br>1: 有        |
| 8     | ACTIVITY          | R   | 活动中断。<br>0: 无<br>1: 有                |
| 7     | RX_DONE           | R   | 接收完成中断。<br>0: 无<br>1: 有            |
| 6     | TX_ABRT           | R   | 发送中止中断。<br>0: 无<br>1: 有            |
| 5     | RD_REQ            | R   | 读取请求中断。<br>0: 无<br>1: 有            |
| 4     | TX_EMPTY          | R   | 发送 FIFO 空中断。<br>0: 无<br>1: 有        |
| 3     | TX_OVER           | R   | 发送 FIFO 溢出中断。<br>0: 无<br>1: 有      |
| 2     | RX_FULL           | R   | 接收 FIFO 满中断。<br>0: 无<br>1: 有        |
| 1     | RX_OVER           | R   | 接收 FIFO 溢出中断。<br>0: 无<br>1: 有      |
| 0     | RX_UNDER          | R   | 接收 FIFO 下溢中断。<br>0: 无<br>1: 有      |

**注意**:
- 该寄存器显示当前的中断状态。
- 每个中断位可通过 IC_INTR_MASK 屏蔽。

---

## IC_INTR_MASK (Used)
- **Name**: I2C Interrupt Mask Register
- **Description**: I2C 中断屏蔽寄存器
- **Size**: 32 bits
- **Offset**: 0x30
- **Exists**: Always

| 位域   | 名称               | 访问 | 描述                                           |
|-------|-------------------|-----|-----------------------------------------------|
| 31:12 | RSVD_IC_INTR_MASK | R   | 保留位，读取为 0                               |
| 11    | M_GEN_CALL        | R/W | 通用呼叫中断屏蔽。<br>0: 屏蔽<br>1: 启用        |
| 10    | M_START_DET       | R/W | 启动条件检测中断屏蔽。<br>0: 屏蔽<br>1: 启用    |
| 9     | M_STOP_DET        | R/W | 停止条件检测中断屏蔽。<br>0: 屏蔽<br>1: 启用    |
| 8     | M_ACTIVITY        | R/W | 活动中断屏蔽。<br>0: 屏蔽<br>1: 启用            |
| 7     | M_RX_DONE         | R/W | 接收完成中断屏蔽。<br>0: 屏蔽<br>1: 启用        |
| 6     | M_TX_ABRT         | R/W | 发送中止中断屏蔽。<br>0: 屏蔽<br>1: 启用        |
| 5     | M_RD_REQ          | R/W | 读取请求中断屏蔽。<br>0: 屏蔽<br>1: 启用        |
| 4     | M_TX_EMPTY        | R/W | 发送 FIFO 空中断屏蔽。<br>0: 屏蔽<br>1: 启用    |
| 3     | M_TX_OVER         | R/W | 发送 FIFO 溢出中断屏蔽。<br>0: 屏蔽<br>1: 启用  |
| 2     | M_RX_FULL         | R/W | 接收 FIFO 满中断屏蔽。<br>0: 屏蔽<br>1: 启用    |
| 1     | M_RX_OVER         | R/W | 接收 FIFO 溢出中断屏蔽。<br>0: 屏蔽<br>1: 启用  |
| 0     | M_RX_UNDER        | R/W | 接收 FIFO 下溢中断屏蔽。<br>0: 屏蔽<br>1: 启用  |

**注意**:
- 该寄存器控制哪些中断可以触发 ic_intr 信号。
- 默认情况下，所有中断都屏蔽（0）。

---

## IC_RAW_INTR_STAT
- **Name**: I2C Raw Interrupt Status Register
- **Description**: I2C 原始中断状态寄存器
- **Size**: 32 bits
- **Offset**: 0x34
- **Exists**: Always

| 位域   | 名称                  | 访问 | 描述                                       |
|-------|----------------------|-----|-------------------------------------------|
| 31:12 | RSVD_IC_RAW_INTR_STAT | R   | 保留位，读取为 0                           |
| 11    | GEN_CALL             | R   | 通用呼叫原始中断。<br>0: 无<br>1: 有        |
| 10    | START_DET            | R   | 启动条件检测原始中断。<br>0: 无<br>1: 有    |
| 9     | STOP_DET             | R   | 停止条件检测原始中断。<br>0: 无<br>1: 有    |
| 8     | ACTIVITY             | R   | 活动原始中断。<br>0: 无<br>1: 有            |
| 7     | RX_DONE              | R   | 接收完成原始中断。<br>0: 无<br>1: 有        |
| 6     | TX_ABRT              | R   | 发送中止原始中断。<br>0: 无<br>1: 有        |
| 5     | RD_REQ               | R   | 读取请求原始中断。<br>0: 无<br>1: 有        |
| 4     | TX_EMPTY             | R   | 发送 FIFO 空原始中断。<br>0: 无<br>1: 有    |
| 3     | TX_OVER              | R   | 发送 FIFO 溢出原始中断。<br>0: 无<br>1: 有  |
| 2     | RX_FULL              | R   | 接收 FIFO 满原始中断。<br>0: 无<br>1: 有    |
| 1     | RX_OVER              | R   | 接收 FIFO 溢出原始中断。<br>0: 无<br>1: 有  |
| 0     | RX_UNDER             | R   | 接收 FIFO 下溢原始中断。<br>0: 无<br>1: 有  |

**注意**:
- 该寄存器显示未经屏蔽的中断状态。
- 可用于轮询模式操作。

---

## IC_RX_TL (Used)
- **Name**: I2C Receive FIFO Threshold Register
- **Description**: I2C 接收 FIFO 阈值寄存器
- **Size**: 32 bits
- **Offset**: 0x38
- **Exists**: Always

| 位域   | 名称            | 访问 | 描述                                       |
|-------|----------------|-----|-------------------------------------------|
| 31:8  | RSVD_IC_RX_TL  | R   | 保留位，读取为 0                           |
| 7:0   | RX_TL          | R/W | 接收 FIFO 阈值。<br>默认值: 0               |

**注意**:
- 设置接收 FIFO 的中断触发水平。
- 当接收 FIFO 中的数据量大于或等于此值时，触发 RX_FULL 中断。

---

## IC_TX_TL (Used)
- **Name**: I2C Transmit FIFO Threshold Register
- **Description**: I2C 发送 FIFO 阈值寄存器
- **Size**: 32 bits
- **Offset**: 0x3c
- **Exists**: Always

| 位域   | 名称            | 访问 | 描述                                       |
|-------|----------------|-----|-------------------------------------------|
| 31:8  | RSVD_IC_TX_TL  | R   | 保留位，读取为 0                           |
| 7:0   | TX_TL          | R/W | 发送 FIFO 阈值。<br>默认值: 0               |

**注意**:
- 设置发送 FIFO 的中断触发水平。
- 当发送 FIFO 中的数据量小于或等于此值时，触发 TX_EMPTY 中断。

---

## IC_CLR_INTR
- **Name**: Clear Combined and Individual Interrupt Register
- **Description**: 清除组合和个体中断寄存器
- **Size**: 32 bits
- **Offset**: 0x40
- **Exists**: Always

| 位域   | 名称             | 访问 | 描述                                       |
|-------|-----------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_INTR | R   | 保留位，读取为 0                           |
| 0     | CLR_INTR         | R   | 读取此位清除所有中断。<br>始终返回 0         |

**注意**:
- 读取此寄存器会清除所有中断状态。

---

## IC_CLR_RX_UNDER
- **Name**: Clear RX_UNDER Interrupt Register
- **Description**: 清除 RX_UNDER 中断寄存器
- **Size**: 32 bits
- **Offset**: 0x44
- **Exists**: Always

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_RX_UNDER | R   | 保留位，读取为 0                           |
| 0     | CLR_RX_UNDER         | R   | 读取此位清除 RX_UNDER 中断。<br>始终返回 0  |

**注意**:
- 专用于清除 RX_UNDER 中断。

---

## IC_CLR_RX_OVER
- **Name**: Clear RX_OVER Interrupt Register
- **Description**: 清除 RX_OVER 中断寄存器
- **Size**: 32 bits
- **Offset**: 0x48
- **Exists**: Always

| 位域   | 名称                | 访问 | 描述                                       |
|-------|--------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_RX_OVER | R   | 保留位，读取为 0                           |
| 0     | CLR_RX_OVER         | R   | 读取此位清除 RX_OVER 中断。<br>始终返回 0   |

**注意**:
- 专用于清除 RX_OVER 中断。

---

## IC_CLR_TX_OVER
- **Name**: Clear TX_OVER Interrupt Register
- **Description**: 清除 TX_OVER 中断寄存器
- **Size**: 32 bits
- **Offset**: 0x4c
- **Exists**: Always

| 位域   | 名称                | 访问 | 描述                                       |
|-------|--------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_TX_OVER | R   | 保留位，读取为 0                           |
| 0     | CLR_TX_OVER         | R   | 读取此位清除 TX_OVER 中断。<br>始终返回 0   |

**注意**:
- 专用于清除 TX_OVER 中断。

---

## IC_CLR_RD_REQ
- **Name**: Clear RD_REQ Interrupt Register
- **Description**: 清除 RD_REQ 中断寄存器
- **Size**: 32 bits
- **Offset**: 0x50
- **Exists**: Always

| 位域   | 名称               | 访问 | 描述                                       |
|-------|-------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_RD_REQ | R   | 保留位，读取为 0                           |
| 0     | CLR_RD_REQ         | R   | 读取此位清除 RD_REQ 中断。<br>始终返回 0    |

**注意**:
- 专用于清除 RD_REQ 中断。

---

## IC_CLR_TX_ABRT
- **Name**: Clear TX_ABRT Interrupt Register
- **Description**: 清除 TX_ABRT 中断寄存器
- **Size**: 32 bits
- **Offset**: 0x54
- **Exists**: Always

| 位域   | 名称                | 访问 | 描述                                       |
|-------|--------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_TX_ABRT | R   | 保留位，读取为 0                           |
| 0     | CLR_TX_ABRT         | R   | 读取此位清除 TX_ABRT 中断。<br>始终返回 0   |

**注意**:
- 专用于清除 TX_ABRT 中断。

---

## IC_CLR_RX_DONE
- **Name**: Clear RX_DONE Interrupt Register
- **Description**: 清除 RX_DONE 中断寄存器
- **Size**: 32 bits
- **Offset**: 0x58
- **Exists**: Always

| 位域   | 名称                | 访问 | 描述                                       |
|-------|--------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_RX_DONE | R   | 保留位，读取为 0                           |
| 0     | CLR_RX_DONE         | R   | 读取此位清除 RX_DONE 中断。<br>始终返回 0   |

**注意**:
- 专用于清除 RX_DONE 中断。

---

## IC_CLR_ACTIVITY
- **Name**: Clear ACTIVITY Interrupt Register
- **Description**: 清除 ACTIVITY 中断寄存器
- **Size**: 32 bits
- **Offset**: 0x5c
- **Exists**: Always

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_ACTIVITY | R   | 保留位，读取为 0                           |
| 0     | CLR_ACTIVITY         | R   | 读取此位清除 ACTIVITY 中断。<br>始终返回 0  |

**注意**:
- 专用于清除 ACTIVITY 中断。

---

## IC_CLR_STOP_DET
- **Name**: Clear STOP_DET Interrupt Register
- **Description**: 清除 STOP_DET 中断寄存器
- **Size**: 32 bits
- **Offset**: 0x60
- **Exists**: Always

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_STOP_DET | R   | 保留位，读取为 0                           |
| 0     | CLR_STOP_DET         | R   | 读取此位清除 STOP_DET 中断。<br>始终返回 0  |

**注意**:
- 专用于清除 STOP_DET 中断。

---

## IC_CLR_START_DET
- **Name**: Clear START_DET Interrupt Register
- **Description**: 清除 START_DET 中断寄存器
- **Size**: 32 bits
- **Offset**: 0x64
- **Exists**: Always

| 位域   | 名称                  | 访问 | 描述                                       |
|-------|----------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_START_DET | R   | 保留位，读取为 0                           |
| 0     | CLR_START_DET         | R   | 读取此位清除 START_DET 中断。<br>始终返回 0 |

**注意**:
- 专用于清除 START_DET 中断。

---

## IC_CLR_GEN_CALL
- **Name**: Clear GEN_CALL Interrupt Register
- **Description**: 清除 GEN_CALL 中断寄存器
- **Size**: 32 bits
- **Offset**: 0x68
- **Exists**: Always

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_GEN_CALL | R   | 保留位，读取为 0                           |
| 0     | CLR_GEN_CALL         | R   | 读取此位清除 GEN_CALL 中断。<br>始终返回 0  |

**注意**:
- 专用于清除 GEN_CALL 中断。

---

## IC_ENABLE
- **Name**: I2C Enable Register
- **Description**: I2C 启用寄存器
- **Size**: 32 bits
- **Offset**: 0x6c
- **Exists**: Always

| 位域   | 名称             | 访问 | 描述                                       |
|-------|-----------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_ENABLE  | R   | 保留位，读取为 0                           |
| 0     | ENABLE           | R/W | 启用 I2C 模块。<br>0: 禁用<br>1: 启用<br>默认值: 0 |

**注意**:
- 启用或禁用 I2C 模块。
- 在禁用时，某些寄存器不可写。

---

## IC_STATUS
- **Name**: I2C Status Register
- **Description**: I2C 状态寄存器
- **Size**: 32 bits
- **Offset**: 0x70
- **Exists**: Always

| 位域   | 名称             | 访问 | 描述                                       |
|-------|-----------------|-----|-------------------------------------------|
| 31:7  | RSVD_IC_STATUS  | R   | 保留位，读取为 0                           |
| 6     | SLV_ACTIVITY    | R   | 从机活动状态。<br>0: 不活动<br>1: 活动      |
| 5     | MST_ACTIVITY    | R   | 主机活动状态。<br>0: 不活动<br>1: 活动      |
| 4     | RFF             | R   | 接收 FIFO 满。<br>0: 未满<br>1: 满          |
| 3     | RFNE            | R   | 接收 FIFO 不空。<br>0: 空<br>1: 不空        |
| 2     | TFE             | R   | 发送 FIFO 空。<br>0: 不空<br>1: 空          |
| 1     | TFNF            | R   | 发送 FIFO 未满。<br>0: 满<br>1: 未满        |
| 0     | ACTIVITY        | R   | I2C 活动状态。<br>0: 不活动<br>1: 活动      |

**注意**:
- 提供 I2C 模块的当前状态信息。

---

## IC_TXFLR
- **Name**: I2C Transmit FIFO Level Register
- **Description**: I2C 发送 FIFO 水平寄存器
- **Size**: 32 bits
- **Offset**: 0x74
- **Exists**: Always

| 位域   | 名称            | 访问 | 描述                                       |
|-------|----------------|-----|-------------------------------------------|
| 31:8  | RSVD_IC_TXFLR  | R   | 保留位，读取为 0                           |
| 7:0   | TXFLR          | R   | 发送 FIFO 中的数据量                        |

**注意**:
- 显示当前发送 FIFO 中的数据条目数。

---

## IC_RXFLR
- **Name**: I2C Receive FIFO Level Register
- **Description**: I2C 接收 FIFO 水平寄存器
- **Size**: 32 bits
- **Offset**: 0x78
- **Exists**: Always

| 位域   | 名称            | 访问 | 描述                                       |
|-------|----------------|-----|-------------------------------------------|
| 31:8  | RSVD_IC_RXFLR  | R   | 保留位，读取为 0                           |
| 7:0   | RXFLR          | R   | 接收 FIFO 中的数据量                        |

**注意**:
- 显示当前接收 FIFO 中的数据条目数。

---

## IC_SDA_HOLD
- **Name**: I2C SDA Hold Time Register
- **Description**: I2C SDA 保持时间寄存器
- **Size**: 32 bits
- **Offset**: 0x7c
- **Exists**: Always

| 位域   | 名称              | 访问 | 描述                                           |
|-------|------------------|-----|-----------------------------------------------|
| 31:16 | RSVD_IC_SDA_HOLD | R   | 保留位，读取为 0                               |
| 15:0  | SDA_HOLD         | R/W | SDA 保持时间（以 IC_CLK 周期为单位）。<br>默认值: 1 |

**注意**:
- 设置 SDA 信号的保持时间，以确保满足 I2C 协议的时序要求。

---

## IC_TX_ABRT_SOURCE
- **Name**: I2C Transmit Abort Source Register
- **Description**: I2C 发送中止源寄存器
- **Size**: 32 bits
- **Offset**: 0x80
- **Exists**: Always

| 位域   | 名称                   | 访问 | 描述                                       |
|-------|-----------------------|-----|-------------------------------------------|
| 31:16 | RSVD_IC_TX_ABRT_SOURCE | R   | 保留位，读取为 0                           |
| 15:0  | ABRT_SOURCE           | R   | 发送中止的原因代码                          |

**注意**:
- 提供发送中止的具体原因，便于调试。

---

## IC_SLV_DATA_NACK_ONLY
- **Name**: I2C Slave Data NACK Only Register
- **Description**: I2C 从机数据仅 NACK 寄存器
- **Size**: 32 bits
- **Offset**: 0x84
- **Exists**: When IC_SLV_DATA_NACK_ONLY = 1

| 位域   | 名称                      | 访问 | 描述                                       |
|-------|--------------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_SLV_DATA_NACK_ONLY | R   | 保留位，读取为 0                           |
| 0     | NACK                     | R/W | 从机数据仅 NACK。<br>0: 正常 ACK<br>1: 仅 NACK |

**注意**:
- 控制从机在接收数据时是否仅发送 NACK。

---

## IC_DMA_CR
- **Name**: I2C DMA Control Register
- **Description**: I2C DMA 控制寄存器
- **Size**: 32 bits
- **Offset**: 0x88
- **Exists**: When IC_HAS_DMA = 1

| 位域   | 名称            | 访问 | 描述                                       |
|-------|----------------|-----|-------------------------------------------|
| 31:2  | RSVD_IC_DMA_CR | R   | 保留位，读取为 0                           |
| 1     | TDMAE          | R/W | 发送 DMA 启用。<br>0: 禁用<br>1: 启用        |
| 0     | RDMAE          | R/W | 接收 DMA 启用。<br>0: 禁用<br>1: 启用        |

**注意**:
- 控制 DMA 功能的使用。

---

## IC_DMA_TDLR
- **Name**: I2C DMA Transmit Data Level Register
- **Description**: I2C DMA 发送数据水平寄存器
- **Size**: 32 bits
- **Offset**: 0x8c
- **Exists**: When IC_HAS_DMA = 1

| 位域   | 名称              | 访问 | 描述                                       |
|-------|------------------|-----|-------------------------------------------|
| 31:8  | RSVD_IC_DMA_TDLR | R   | 保留位，读取为 0                           |
| 7:0   | TDLR             | R/W | 发送 DMA 数据水平。<br>默认值: 0             |

**注意**:
- 设置发送 DMA 的水印级别。

---

## IC_DMA_RDLR
- **Name**: I2C DMA Receive Data Level Register
- **Description**: I2C DMA 接收数据水平寄存器
- **Size**: 32 bits
- **Offset**: 0x90
- **Exists**: When IC_HAS_DMA = 1

| 位域   | 名称              | 访问 | 描述                                       |
|-------|------------------|-----|-------------------------------------------|
| 31:8  | RSVD_IC_DMA_RDLR | R   | 保留位，读取为 0                           |
| 7:0   | RDLR             | R/W | 接收 DMA 数据水平。<br>默认值: 0             |

**注意**:
- 设置接收 DMA 的水印级别。

---

## IC_SDA_SETUP
- **Name**: I2C SDA Setup Register
- **Description**: I2C SDA 建立时间寄存器
- **Size**: 32 bits
- **Offset**: 0x94
- **Exists**: Always

| 位域   | 名称              | 访问 | 描述                                           |
|-------|------------------|-----|-----------------------------------------------|
| 31:8  | RSVD_IC_SDA_SETUP | R   | 保留位，读取为 0                               |
| 7:0   | SDA_SETUP         | R/W | SDA 建立时间（以 IC_CLK 周期为单位）。<br>默认值: 100 |

**注意**:
- 设置 SDA 信号的建立时间，以确保满足 I2C 协议的时序要求。

---

## IC_ACK_GENERAL_CALL
- **Name**: I2C ACK General Call Register
- **Description**: I2C ACK 通用呼叫寄存器
- **Size**: 32 bits
- **Offset**: 0x98
- **Exists**: Always

| 位域   | 名称                    | 访问 | 描述                                       |
|-------|------------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_ACK_GENERAL_CALL | R   | 保留位，读取为 0                           |
| 0     | ACK_GEN_CALL           | R/W | ACK 通用呼叫。<br>0: NACK<br>1: ACK<br>默认值: 1 |

**注意**:
- 控制是否对通用呼叫地址进行 ACK。

---

## IC_ENABLE_STATUS
- **Name**: I2C Enable Status Register
- **Description**: I2C 启用状态寄存器
- **Size**: 32 bits
- **Offset**: 0x9c
- **Exists**: Always

| 位域   | 名称                    | 访问 | 描述                                       |
|-------|------------------------|-----|-------------------------------------------|
| 31:3  | RSVD_IC_ENABLE_STATUS  | R   | 保留位，读取为 0                           |
| 2     | SLV_RX_DATA_LOST       | R   | 从机接收数据丢失。<br>0: 无<br>1: 有        |
| 1     | SLV_DISABLED_WHILE_BUSY | R   | 从机在忙时被禁用。<br>0: 无<br>1: 有        |
| 0     | IC_EN                  | R   | I2C 启用状态。<br>0: 禁用<br>1: 启用        |

**注意**:
- 提供 I2C 模块的启用状态和错误条件。

---

## IC_FS_SPKLEN
- **Name**: I2C Fast Speed Spike Length Register
- **Description**: I2C 快速速度尖峰长度寄存器
- **Size**: 32 bits
- **Offset**: 0xa0
- **Exists**: Always

| 位域   | 名称              | 访问 | 描述                                       |
|-------|------------------|-----|-------------------------------------------|
| 31:8  | RSVD_IC_FS_SPKLEN | R   | 保留位，读取为 0                           |
| 7:0   | FS_SPKLEN         | R/W | 快速模式尖峰抑制长度。<br>默认值: 5          |

**注意**:
- 设置快速模式下尖峰抑制的时钟周期数。

---

## IC_HS_SPKLEN
- **Name**: I2C High Speed Spike Length Register
- **Description**: I2C 高速尖峰长度寄存器
- **Size**: 32 bits
- **Offset**: 0xa4
- **Exists**: When IC_MAX_SPEED_MODE = 3

| 位域   | 名称              | 访问 | 描述                                       |
|-------|------------------|-----|-------------------------------------------|
| 31:8  | RSVD_IC_HS_SPKLEN | R   | 保留位，读取为 0                           |
| 7:0   | HS_SPKLEN         | R/W | 高速模式尖峰抑制长度。<br>默认值: 5          |

**注意**:
- 设置高速模式下尖峰抑制的时钟周期数。

---

## IC_CLR_RESTART_DET
- **Name**: Clear RESTART_DET Interrupt Register
- **Description**: 清除 RESTART_DET 中断寄存器
- **Size**: 32 bits
- **Offset**: 0xa8
- **Exists**: When IC_SLV_RESTART_DET_EN = 1

| 位域   | 名称                   | 访问 | 描述                                       |
|-------|-----------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_RESTART_DET | R   | 保留位，读取为 0                           |
| 0     | CLR_RESTART_DET        | R   | 读取此位清除 RESTART_DET 中断。<br>始终返回 0 |

**注意**:
- 专用于清除 RESTART_DET 中断（从机模式下检测到重启条件）。

---

## IC_SCL_STUCK_AT_LOW_TIMEOUT
- **Name**: I2C SCL Stuck at Low Timeout Register
- **Description**: I2C SCL 卡在低电平超时寄存器
- **Size**: 32 bits
- **Offset**: 0xb0
- **Exists**: When IC_BUS_CLEAR_FEATURE = 1

| 位域   | 名称                       | 访问 | 描述                                           |
|-------|---------------------------|-----|-----------------------------------------------|
| 31:0  | SCL_STUCK_AT_LOW_TIMEOUT  | R/W | SCL 卡在低电平的超时周期数。<br>默认值: 0       |

**注意**:
- 设置 SCL 卡在低电平的超时时间，用于总线恢复功能。

---

## IC_SDA_STUCK_AT_LOW_TIMEOUT
- **Name**: I2C SDA Stuck at Low Timeout Register
- **Description**: I2C SDA 卡在低电平超时寄存器
- **Size**: 32 bits
- **Offset**: 0xb4
- **Exists**: When IC_BUS_CLEAR_FEATURE = 1

| 位域   | 名称                       | 访问 | 描述                                           |
|-------|---------------------------|-----|-----------------------------------------------|
| 31:0  | SDA_STUCK_AT_LOW_TIMEOUT  | R/W | SDA 卡在低电平的超时周期数。<br>默认值: 0       |

**注意**:
- 设置 SDA 卡在低电平的超时时间，用于总线恢复功能。

---

## IC_DEVICE_ID
- **Name**: I2C Device ID Register
- **Description**: I2C 设备 ID 寄存器
- **Size**: 32 bits
- **Offset**: 0xb8
- **Exists**: When IC_DEVICE_ID = 1

| 位域   | 名称         | 访问 | 描述                                       |
|-------|-------------|-----|-------------------------------------------|
| 31:0  | DEVICE_ID   | R   | 设备 ID。<br>默认值: 0                      |

**注意**:
- 提供设备的唯一标识符。

---

## IC_SMBUS_CLOCK_LOW_SEXT
- **Name**: SMBus Clock Low SEXT Register
- **Description**: SMBus 时钟低电平 SEXT 寄存器
- **Size**: 32 bits
- **Offset**: 0xbc
- **Exists**: When IC_SMBUS = 1

| 位域   | 名称                   | 访问 | 描述                                           |
|-------|-----------------------|-----|-----------------------------------------------|
| 31:0  | SMBUS_CLOCK_LOW_SEXT  | R/W | SMBus 时钟低电平扩展周期数。<br>默认值: 0       |

**注意**:
- 用于 SMBus 协议的时钟扩展。

---

## IC_SMBUS_CLOCK_LOW_MEXT
- **Name**: SMBus Clock Low MEXT Register
- **Description**: SMBus 时钟低电平 MEXT 寄存器
- **Size**: 32 bits
- **Offset**: 0xc0
- **Exists**: When IC_SMBUS = 1

| 位域   | 名称                   | 访问 | 描述                                           |
|-------|-----------------------|-----|-----------------------------------------------|
| 31:0  | SMBUS_CLOCK_LOW_MEXT  | R/W | SMBus 时钟低电平主扩展周期数。<br>默认值: 0     |

**注意**:
- 用于 SMBus 协议的时钟扩展。

---

## IC_SMBUS_THIGH_MAX_BUS_IDLE_COUNT
- **Name**: SMBus THIGH Max Bus Idle Count Register
- **Description**: SMBus THIGH 最大总线空闲计数寄存器
- **Size**: 32 bits
- **Offset**: 0xc4
- **Exists**: When IC_SMBUS = 1

| 位域   | 名称                           | 访问 | 描述                                           |
|-------|-------------------------------|-----|-----------------------------------------------|
| 31:0  | SMBUS_THIGH_MAX_BUS_IDLE_COUNT | R/W | SMBus THIGH 最大总线空闲周期数。<br>默认值: 0   |

**注意**:
- 设置 SMBus 协议中 THIGH 的最大总线空闲时间。

---

## IC_SMBUS_INTR_STAT
- **Name**: SMBus Interrupt Status Register
- **Description**: SMBus 中断状态寄存器
- **Size**: 32 bits
- **Offset**: 0xc8
- **Exists**: When IC_SMBUS = 1

| 位域   | 名称                   | 访问 | 描述                                       |
|-------|-----------------------|-----|-------------------------------------------|
| 31:4  | RSVD_IC_SMBUS_INTR_STAT | R   | 保留位，读取为 0                           |
| 3     | SMBUS_ALERT_DET       | R   | SMBus 警报检测。<br>0: 无<br>1: 有          |
| 2     | SMBUS_SUSPEND_DET     | R   | SMBus 挂起检测。<br>0: 无<br>1: 有          |
| 1     | SMBUS_CLOCK_LOW_TIMEOUT | R   | SMBus 时钟低电平超时。<br>0: 无<br>1: 有    |
| 0     | SMBUS_DATA_LOW_TIMEOUT | R   | SMBus 数据低电平超时。<br>0: 无<br>1: 有    |

**注意**:
- 显示 SMBus 相关的中断状态。

---

## IC_SMBUS_INTR_MASK
- **Name**: SMBus Interrupt Mask Register
- **Description**: SMBus 中断屏蔽寄存器
- **Size**: 32 bits
- **Offset**: 0xcc
- **Exists**: When IC_SMBUS = 1

| 位域   | 名称                       | 访问 | 描述                                           |
|-------|---------------------------|-----|-----------------------------------------------|
| 31:4  | RSVD_IC_SMBUS_INTR_MASK   | R   | 保留位，读取为 0                               |
| 3     | M_SMBUS_ALERT_DET         | R/W | SMBus 警报检测中断屏蔽。<br>0: 屏蔽<br>1: 启用  |
| 2     | M_SMBUS_SUSPEND_DET       | R/W | SMBus 挂起检测中断屏蔽。<br>0: 屏蔽<br>1: 启用  |
| 1     | M_SMBUS_CLOCK_LOW_TIMEOUT | R/W | SMBus 时钟低电平超时中断屏蔽。<br>0: 屏蔽<br>1: 启用 |
| 0     | M_SMBUS_DATA_LOW_TIMEOUT  | R/W | SMBus 数据低电平超时中断屏蔽。<br>0: 屏蔽<br>1: 启用 |

**注意**:
- 控制 SMBus 相关的中断是否触发 ic_intr 信号。

---

## IC_SMBUS_INTR_RAW_STATUS
- **Name**: SMBus Interrupt Raw Status Register
- **Description**: SMBus 中断原始状态寄存器
- **Size**: 32 bits
- **Offset**: 0xd0
- **Exists**: When IC_SMBUS = 1

| 位域   | 名称                         | 访问 | 描述                                       |
|-------|-----------------------------|-----|-------------------------------------------|
| 31:4  | RSVD_IC_SMBUS_INTR_RAW_STATUS | R   | 保留位，读取为 0                           |
| 3     | SMBUS_ALERT_DET_RAW         | R   | SMBus 警报检测原始状态。<br>0: 无<br>1: 有  |
| 2     | SMBUS_SUSPEND_DET_RAW       | R   | SMBus 挂起检测原始状态。<br>0: 无<br>1: 有  |
| 1     | SMBUS_CLOCK_LOW_TIMEOUT_RAW | R   | SMBus 时钟低电平超时原始状态。<br>0: 无<br>1: 有 |
| 0     | SMBUS_DATA_LOW_TIMEOUT_RAW  | R   | SMBus 数据低电平超时原始状态。<br>0: 无<br>1: 有 |

**注意**:
- 显示未经屏蔽的 SMBus 中断状态。

---

## IC_CLR_SMBUS_INTR
- **Name**: Clear SMBus Interrupt Register
- **Description**: 清除 SMBus 中断寄存器
- **Size**: 32 bits
- **Offset**: 0xd4
- **Exists**: When IC_SMBUS = 1

| 位域   | 名称                  | 访问 | 描述                                       |
|-------|----------------------|-----|-------------------------------------------|
| 31:1  | RSVD_IC_CLR_SMBUS_INTR | R   | 保留位，读取为 0                           |
| 0     | CLR_SMBUS_INTR        | R   | 读取此位清除所有 SMBus 中断。<br>始终返回 0 |

**注意**:
- 读取此寄存器会清除所有 SMBus 中断状态。

---

## IC_OPTIONAL_SAR
- **Name**: I2C Optional Slave Address Register
- **Description**: I2C 可选从机地址寄存器
- **Size**: 32 bits
- **Offset**: 0xd8
- **Exists**: When IC_SMBUS = 1

| 位域   | 名称                | 访问 | 描述                                       |
|-------|--------------------|-----|-------------------------------------------|
| 31:10 | RSVD_IC_OPTIONAL_SAR | R   | 保留位，读取为 0                           |
| 9:0   | OPTIONAL_SAR       | R/W | 可选从机地址。<br>默认值: 0                 |

**注意**:
- 用于 SMBus 协议中的可选从机地址。

---

## IC_SMBUS_UDID_LSB
- **Name**: SMBus UDID Least Significant Bits Register
- **Description**: SMBus UDID 最低有效位寄存器
- **Size**: 32 bits
- **Offset**: 0xdc
- **Exists**: When IC_SMBUS = 1

| 位域   | 名称         | 访问 | 描述                                       |
|-------|-------------|-----|-------------------------------------------|
| 31:0  | UDID_LSB    | R/W | UDID 的最低 32 位。<br>默认值: 0            |

**注意**:
- 用于 SMBus 设备标识的 UDID。

---

## IC_SMBUS_UDID_MSB
- **Name**: SMBus UDID Most Significant Bits Register
- **Description**: SMBus UDID 最高有效位寄存器
- **Size**: 32 bits
- **Offset**: 0xe0
- **Exists**: When IC_SMBUS = 1

| 位域   | 名称         | 访问 | 描述                                       |
|-------|-------------|-----|-------------------------------------------|
| 31:0  | UDID_MSB    | R/W | UDID 的最高 32 位。<br>默认值: 0            |

**注意**:
- 用于 SMBus 设备标识的 UDID。

---

## IC_DEBUG_STATUS
- **Name**: I2C Debug Status Register
- **Description**: I2C 调试状态寄存器
- **Size**: 32 bits
- **Offset**: 0xe4
- **Exists**: Always

| 位域   | 名称           | 访问 | 描述                                       |
|-------|---------------|-----|-------------------------------------------|
| 31:0  | DEBUG_STATUS  | R   | 调试状态信息。<br>具体位定义依赖于实现       |

**注意**:
- 提供内部状态信息，用于调试目的。

---

## IC_VERSION_ID
- **Name**: I2C Version ID Register
- **Description**: I2C 版本 ID 寄存器
- **Size**: 32 bits
- **Offset**: 0xe8
- **Exists**: Always

| 位域   | 名称         | 访问 | 描述                                       |
|-------|-------------|-----|-------------------------------------------|
| 31:0  | VERSION_ID  | R   | IP 核的版本标识。<br>默认值: 特定于版本      |

**注意**:
- 标识 IP 核的版本号。

---

## IC_COMP_PARAM_1 (Used)
- **Name**: Component Parameter Register 1
- **Description**: 组件参数寄存器 1
- **Size**: 32 bits
- **Offset**: 0xf4
- **Exists**: Always

| 位域   | 名称                 | 访问 | 描述                                       |
|-------|---------------------|-----|-------------------------------------------|
| 31:24 | RSVD_IC_COMP_PARAM_1 | R   | 保留位，读取为 0                           |
| 23:16 | TX_BUFFER_DEPTH     | R   | 发送 FIFO 深度                              |
| 15:8  | RX_BUFFER_DEPTH     | R   | 接收 FIFO 深度                              |
| 7     | ADD_ENCODED_PARAMS  | R   | 编码参数启用                                |
| 6     | HAS_DMA             | R   | DMA 支持                                    |
| 5     | INTR_IO             | R   | 中断 IO                                     |
| 4     | HC_COUNT_VALUES     | R   | 硬编码计数器值                              |
| 3:2   | MAX_SPEED_MODE      | R   | 最大速度模式                                |
| 1:0   | APB_DATA_WIDTH      | R   | APB 数据宽度                                |

**注意**:
- 提供 IP 核的配置参数信息。

---

## IC_COMP_VERSION
- **Name**: Component Version Register
- **Description**: 组件版本寄存器
- **Size**: 32 bits
- **Offset**: 0xf8
- **Exists**: Always

| 位域   | 名称           | 访问 | 描述                                       |
|-------|---------------|-----|-------------------------------------------|
| 31:0  | COMP_VERSION  | R   | 组件版本号。<br>默认值: 特定于版本           |

**注意**:
- 标识组件的版本。

---

## IC_COMP_TYPE (Used)
- **Name**: Component Type Register
- **Description**: 组件类型寄存器
- **Size**: 32 bits
- **Offset**: 0xfc
- **Exists**: Always

| 位域   | 名称        | 访问  | 描述                                      |
| ---- | --------- | --- | --------------------------------------- |
| 31:0 | COMP_TYPE | R   | 组件类型标识。<br>默认值: 0x44570140 (DW_apb_i2c) |

**注意**:
- 标识组件的类型。

---

### 总结
以上是 DesignWare DW_apb_i2c IP 核的所有寄存器整理，涵盖了控制、状态、中断、FIFO 管理、DMA、SMBus 支持等多个方面。每个寄存器的位域、访问类型和功能都已详细列出，方便用户在开发和调试时参考。

**特别注意**:
- 某些寄存器的存在与否依赖于配置参数，例如 IC_MAX_SPEED_MODE、IC_SMBUS 等。
- 寄存器的默认值和具体位宽可能因配置而异，实际使用时需参考具体配置的文档。
- 中断相关的寄存器较多，需仔细管理以确保正确处理各种事件。










# Program Guid





# 测试用例





# 测试 case





# Q&&A
## I2C 作为 Master Receiver 时，当接收完最后一个字节数据后，会发送 NACK 给 Slave，则 Slave 停止传输。
- I2C 怎么知道应该发 ACK 还是 NACK？
	- 驱动会往 CMD_DATA 寄存器写，来开始传输，那是不是这里就有一个时间？多久之后没有继续往 CMD_DATA 里面写，就会发 NACK？
	- 这样如果在多线程的环境下，数据没处理完，这个线程就一直不被调度，不就有问题？
- 感觉应该是根据 stop 信号来的，因为每次传输都会发一个 Read，最后一次传输的时候会发一个 STOP，这样控制器知道了要停止传输，就会在发完数据之后，发 STOP 之前，发出一个 NACK
- 实际测试，读 3 个字节，在倒数第 2 个字节的时候，本来只发一个 Read，修改成 Read | Stop，逻辑分析仪抓到倒数第二个字节的时候  Master 主动发了一个 NACK 出去，并且发了一个 STOP。

