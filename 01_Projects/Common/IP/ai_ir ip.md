
# 芯片特性
- Apus：
	- FPGA 阶段使用数字，芯片阶段包括模拟接收和模拟发射
- Fornax：
	- FPGA 阶段就有模拟，芯片阶段包括模拟接收和模拟发射
- Sagitta：
	- FPGA 阶段使用外围电路接收和发射，芯片阶段使用外围电路发射，使用内部模拟接收
		- 客户觉得芯片内部的模拟发射驱动能力太弱，因此使用外围电路辅助发射

# IR 控制器

- 相关问题号：

  - https://git.nationalchip.com/redmine/issues/307610       Apus MPW FPGA IR 验证

  - https://git.nationalchip.com/redmine/issues/325348       Apus SDK IR子系统

  - https://git.nationalchip.com/redmine/issues/328613       Apus MPW IR 验证



- 可调波频率以及载波占空比
- 支持数字调制/解调和模拟调制/解调
- 固定模式：
  - 支持 9012/NEC(8bit)/RC5/RC6 四种传输协议的接收、发射
  - 六个中断源：`std_tx_done、std_rx_done、repeat_det、rx_cc_err、rx_kdc_err、prot_det_err`
  - 每个中断源独立的使能控制
  - 接收模块硬件自识别传输协议
- 灵活模式：
  - 支持数字调制/解调和模拟调制/解调
  - 中断源：`ud_tx_done、ud_rx_done、txfifo_empty、txfifio_full、txfifio_thr、rxfifo_empty、rxfifo_full、rxfifio_thr`
  - 每个中断源独立的使能控制
  - 接收/发射独立的 FIFO

- 调制：调制成某个频率的波形
	- 发送时通过寄存器 div_num 和 duty_num 控制计数器产生调制载波，载波频率为 `irc_clk/(div_num + 1)`，占空比为 `(duty_num + 1) / (div_num + 1)`
	  - **载波频率为 38K，占空比为 30%**
- 解调：将接收到的波
	- 接收可调制的最低频率为 `irc_clk/16/(16 + 2*democe_length)`，当 `demod_length` 为 `0` 时，最低可解调频率为 `16.6KHz`，模块最高可解调频率为 `100KHz`。
	- **模块频率为 32MHz，分频到 8MHz 进行采样，每隔 32 个时钟周期对输入的接收信号 (带载波) 进行采样，并送入移位寄存器 rx_buf 中。只要移位寄存器中任一 bit 为 1，则解调输出为 1. 如果所有 bit 都为 0，则解调为 0.**
	- 移位寄存器的位宽为 30 bit，可通过 demod_length 配置 8 档解调窗口。




## 标准协议描述

### 9012 协议：

- 9012 协议采用不同间隔的脉冲信号来对信号进行编码。脉冲信号脉宽为 1Tm(560us = 0.56ms)，逻辑 ‘1’ 需要 4Tm，逻辑 '0' 只需 2Tm。载波频率为 38KHz，建议载波占空比为 1/4 或 1/3。
- ![image-20240226141153118](image/image-20240226141153118.png)

- Tm = 256/Fosc = 0.56ms(Fosc = 455KHz)
- 重发间隔 = 192Tm = 108ms
- 载波频率 = Fosc/12 = 455/12 ≈ 38KHz
- 数据段包括 8 位 customer code 以及 8 位 command code length
- 为提高可靠性，customer code 和 command code 会连发两次
- 最后是 1Tm 的逻辑 ‘1’ 作为 Stopbit
  - ![image-20240226141353312](image/image-20240226141353312.png)

- 如果按键长按，每隔 192Tm 重发间隔就会发射一个重复命令。重复命令为 8Tm 的逻辑'1'，跟着 8Tm 的逻辑 '0' 再加 1Tm 的脉冲，最后再跟 1Tm 的stopbit。
  - ![image-20240226142435771](image/image-20240226142435771.png)





### NEC 协议(8bit):

- NEC 协议采用不同间隔的脉冲信号来对信号进行编码。脉冲信号脉宽为 1Tm(560us)，逻辑 '1' 需要 4Tm，逻辑 '0' 需要 2Tm。载波频率为 38KHz，建议载波占空比为 1/4 或 1/3。
  - ![image-20240226153220598](image/image-20240226153220598.png)
- 发送：在数据段之前，会先发 16Tm 的逻辑 '1'，然后跟 8Tm 的逻辑 '0'，然后才发 customer code 和 command code，customer code 和 command code 均为 LSB 先发。为提高可靠性，customer code 和 command code 会连发两次，第二次的 customer code 为第一次 customer code 的取反，第二次的 command code 为第一次的 command code 的取反，用于验证接收数据的正确性。第二次 command code 之后，跟 1Tm 的逻辑 '1' 作为 stopbit，标志传输结束。
  - ![image-20240226142141691](image/image-20240226142141691.png)

- 如果按键长按，每隔 192Tm 重发间隔就会发射一个重复命令。重复命令为 16Tm 的逻辑 '1'，跟着 4Tm 的逻辑 '0' 再加 1Tm 的脉冲，最后跟着 1Tm 的stopbit。





### RC5 协议：

- RC5 协议采用曼彻斯特编码，每一 bit 的长度都是 1.68ms(2Tm)。
  - 逻辑 '0' 为 1Tm 的高电平(带载波)在前，跟着 1Tm 的空间隔(无载波)。
  - 逻辑 '1' 为 1Tm 的空间隔(无载波) 在前，跟着 1Tm 的高电平(带载波)。
  - RC5 协议的载波频率为 38KHz，建议载波占空比为 1/4 或 1/3。
    - ![image-20240226143108859](image/image-20240226143108859.png)
  - 数据段包括 5 位 customer code 以及 6 位 command code

- 帧结构中，前 2bit 为 start bit(逻辑 '1')，第三位是翻转位，之后的 5bit 是 custom code，RC5 中定义为红外设备地址，以 MSB 的方式传输。后 6bit 是 command code，同样以 MSB 方式传输。
- 一帧数据总共 14bit，总时长 28Tm。
  - 翻转位：每当按键按下并松开后翻转位将取反，接收机通过判断前后两次接收的翻转位就能知道按键是否一直按下在重复发同样的数值。
  - 按了松开了，就翻转一下。再按了再松开，再翻转一下。两次的 翻转位不同，说明按下的按键。两次的翻转位相同，说明按键一直在被按着。
- ![image-20240226143752167](image/image-20240226143752167.png)

- 当按键长按，同样的信息将以 128Tm(108ms) 为间隔重发。重发的包中翻转位保持不变。
  - ![image-20240226143852067](image/image-20240226143852067.png)





### RC6 协议：

- RC6 是 RC5 的继承版本。本设计中支持 RC6 的模式0。
- RC6 采用 36KHz 的红外载波调制，占空比在 25% ~ 50% 之间。传输数据采用曼彻斯特编码，每一 bit 的长度为 888us(2T)。
- 逻辑 '0' 为 1Tm 的空间隔(无载波) 在前，跟着 1Tm 的高电平(带载波)。
- 逻辑 '1' 为 1Tm 的高电平(带载波)在前，跟着 1Tm 的空间隔(无载波)。
  - ![image-20240226144817549](image/image-20240226144817549.png)

- 1T = 1*16/36K = 444us
- 1Bit = 2T = 888us
- 传输总时间 = 22bit = 23.1ms(message) +2.7ms(no signal)
- 重发间隔 = 240T = 106.7ms
- 帧结构：Head 段、Control 段、Information 段和 Signal free 段
  - ![image-20240226145153855](image/image-20240226145153855.png)
- Head 段：包括 Leader位(第一 startbit 和 第二 startbit)、模式位和翻转位
  - Leader 位包括 Leader Synbol(LS) 和 Start Bit(SB)。
    - LS：由 6T (2.666ms) 的调制信号以及 2T(0.889ms) 的空间隔组成，用于接收机完成增益调整(本设计的应用场景中，接收仅在遥控器与新设备进行学习时，超短距离使用一次，后面不再使用，因此不考虑增益变化)。
    - SB：由逻辑 '1' 组成
  - 模式位 mb2，mb0 用于配置 RC6 的模式，当前设计仅支持模式 0，因此这三位皆为逻辑 '0'
  - 翻转位由 2T 调制信号和 2T 空间隔组成。每当按键释放时翻转，用于区分新发射包和重复发射包。
    - ![image-20240226145627212](image/image-20240226145627212.png)

- Control 段：由 8bit 组成，用于设备地址，意味着 RC6 的模式0 可以支持 256 个设备，Control 段是 MSB 传输。
- Information 段：由 8bit 组成，用于命令的传输，意味着可以支持 256 种不同的命令。Information 段是 MSB 传输。
- Signal Free Time 段：指没有任何数据传输的一段时间，用于保证传输之间有足够的间隔，避免接收错误。RC6 中，Signal Free 段的长度为 6T(2.666ms)。





## 模块设计

- 红外模块分为固定模式和灵活模式两种工作模式。
  - 固定模式：
    - 用于收发协议为标准的 9012/NEC/RC5/RC6 的应用场景下，由硬件完成解码，输出解码数据。
    - 接收/发射可通过寄存器 `std_rx_en/std_tx_en` 使能和关闭
  - 灵活模式：
    - 用于非标准协议的应用场景下，由于无法识别协议，硬件模块只能对接受波形进行采样并存于 FIFIO 中，解码必须由软件完成。
    - 接收/发射可通过寄存器 `ud_rx_en/ud_tx_en` 使能和关闭
  - 固定模式和灵活模式的发射不可同时使能，只能选一个进行发射。固定模式和灵活模式的接收可以同时使能。





## 编程指南

![image-20240226160137973](image/image-20240226160137973.png)

![image-20240226160155133](image/image-20240226160155133.png)











## 寄存器

### REG_IRC_VERSION：0x00

- 默认值：0x00010000

| Name        | Bits | Description  |
| ----------- | ---- | ------------ |
| irc_version | 31:0 | IRC 版本信息 |


### REG_IRC_CARR：0x04

- 默认值：0x004500d2

| Name        | Bits | Description  |
| ----------- | ---- | ------------ |
| duty_num    | 27:16| 载波占空比配置，例如：1/3占空比，duty_num=(div_num + 1)/3  - 1|
| div_num     | 11:0 | 生成载波频率的分配比，div_num=(irc_clk/载波频率) - 1|



### REG_IRC_RX_CTRL：0x10

- 默认值：0x000f0000

| Name        | Bits | Description  |
| ----------- | ---- | ------------ |
| deb_length  | 20:16| 消抖滤波器配置 |
| demod_length| 9:7  | 解调选档，可解调最低载波频率=irc_clk/32/8/(16+2*demod_length)|
| inv_en      | 6    | 接收信号反相使能，1：使能；0：禁止|
| kdc_cmp_en  | 5    | 固定模式 command code 比较使能，1：使能；0：禁止|
| cc_cmp_en   | 4    | 固定模式 customer code 比较使能，1：使能；0：禁止|
| demod_en    | 3    | 接收解调使能，1：使能；0：旁路|
| deb_en      | 2    | 消抖滤波器使能，1：使能；0：禁止|
| std_rx_en   | 1    | 固定模式接收使能，1：使能；0：禁止|
| ud_rx_en    | 0    | 灵活模式接收使能，1：使能；0：禁止|









### REG_IRC_INT_STA：0x24

- 默认值：0x00000000
- 写 1 清零

| Name        | Bits | Description  |
| ----------- | ---- | ------------ |
| txfifo_empty| 21   | TXFIFO Empty|
| txfifo_full | 20   | TXFIFO FULL|
| txfifo_thr_flag| 19   | TXFIFO 到达门限标志|
| rxfifo_empty| 18   | RXFIFO 空|
| rxfifo_full | 17   | RXFIFO 满|
| rxfifo_thr_flag| 16   | RXFIFO 到达门限标志|
| std_rxtov_ovf| 8    | 固定模式接收 timeout 中断标志|
| ud_rxdone  | 7 | 灵活模式接收结束|
| std_rxdone| 6| 固定模式接收结束|
| ud_txdone   | 5    | 灵活模式发送结束|
| std_txdone  | 4   | 固定模式发射结束|
| rx_cc_err   | 3    | 固定模式9012/NEC协议中，custom code比较错误|
| rx_kdc_err  | 2    | 固定模式9012/NEC协议的 command code 比较错误|
| repeat_det  | 1    | 固定模式检测到重复码中断|
| prot_det_err| 0    | 固定模式协议识别失败中断|








### REG_IRC_UDRX_TOV：0x2c

- 默认值：0x000009c4

| Name        | Bits | Description  |
| ----------- | ---- | ------------ |
| rx_tov      | 15:0 | 接收信号超时配置，当接收信号当前 bit 周期的市场大于等于 rx_tov * clk_div 个 irc_clk 时钟周期，则认为灵活模式接收结束 |


### REG_IRC_UD_CTRL：0x34

- 默认值：0x031f000c

| Name        | Bits | Description  |
| ----------- | ---- | ------------ |
| clk_div     | 25:16| 灵活模式下计数时钟分频比，计数时钟频率为模块时钟 irc_clk/(clk_div+1) |
| txfifo_thr_sel | 3:2  | TXFIFO 门限配置，0x0：TXFIFO 空；0x1:TXFIFO 剩余2个；0x2:TXFIFO剩余1/4；0x3:TXFIFO剩余1/2|
| txfifo_thr_sel | 1:0  | RXFIFO 门限配置，0x0:RXFIFO 有一个数据；0x1:RXFIFO已满1/4;0x2:已满1/2；0x3:差2个 Full|



### REG_IRC_FIFO_PARA：0x38

- 默认值：0x00000000

| Name        | Bits | Description  |
| ----------- | ---- | ------------ |
| txfifo_depth| 31:24| 读取 TXFIFO 深度 |
| txfifo_width| 23:16| 读取 TXFIFO 位宽 |
| rxfifo_depth| 15:8 | 读取 RXFIFO 深度 |
| txfifo_width| 7:0  | 读取 RXFIFO 位宽 |




### REG_IRC_ANA：0x3c

- 默认值：0x00000028

| Name        | Bits | Description  |
| ----------- | ---- | ------------ |
| tx_tmd_en   | 13   | 红外发射测试模式，1：进入测试模式，发射管常开；0：测试模式关闭 |
| tx_vstset   | 12   | 红外发射电路软启动控制 |
| tx_sel_msb  | 11:10| 红外模拟发射驱动能力控制|
| ana_tx_en   | 9    | 红外模拟发射使能，1：使能；0：禁止|
| ana_rx_en   | 8    | 红外模拟接收时能，1：使能；0：禁止|
| rx_test_en  | 7    | 红外模拟接收测试使能，1：使能；0：禁止|
| rx_test_sel | 6    | 红外模拟接收测试选通不同电压输出到 TEST ANA，0：vpcas；1：vcom|
| rx_isel     | 5    | 红外模拟接收参考电流控制，1：1uA；0：0.5uA|
| rx_vcmsel   | 4:2  | 红外模拟接收参考电压档位控制|
| tx_sel_lsb  | 1:0  | 红外模拟发射驱动能力控制|


### REG_IRC_TEST：0x40

- 默认值：0x00000003

| Name        | Bits | Description  |
| ----------- | ---- | ------------ |
| irc_txd_sel | 1    | 发射通道选择，1：正常模式，内部数字接内部模拟；0：外部测试信号送给模拟发射电路，旁路数字发射 |
| irc_rxd_sel | 0    | 接收通道选择，1：正常模式，内部数字接内部模拟；0：外部测试信号送给数字接收电路，旁路模拟接收|


### REG_IRC_STDRX_TOV：0x48

- 默认值：0x0000f000

| Name        | Bits | Description  |
| ----------- | ---- | ------------ |
| std_rx_tov  | 23:0 | 固定模式接收信号超时配置，时长 >= std_rx_tov * irc_clk 时钟周期，认为接收超时结束 |



## 驱动：

### drivers/hal/src/ir/gx_hal_ir.c

#### gx_hal_ir_init(GX_HAL_IR *ir_dev, GX_HAL_IR_CFG_T *cfg)

- src_clk：模块时钟源频率
- tx_carrier_freq：发送载波频率
- tx_carrier_duty：发送载波占空比
    - div_num = clk/freq
    - duty_num = (div_num + 1) * tx_carrier_duty / 100
    - div_num 和 duty_num 写入到寄存器 `GX_HAL_IRC_CARR`
- internal_ana_en：芯片内部模拟部分使能
    - internal_ana_en 写入到寄存器 `GX_HAL_IRC_TEST`
- 模拟配置寄存器：
    - tx_vstset：模拟发射电路软启动控制，开启后可控制过冲
        - 默认开启
    - tx_tmod_en：模拟发射电路测试模式，1：测试模式，发射常开；0：关闭
        - 默认不开
    - tx_drv_capability：发送驱动能力(分为高位和低位)
        - 默认为 3
    - vcmsel：模拟接收参考电压档位
        - 默认为 1
    - isel：模拟接收参考电流档位，0：1uA；1：0.5uA
        - 默认为 0
    - testsel：模拟接收测试通不通电压到 test ana，0：vpcas；1：vcom
        - 默认为 0
    - test_ana_pin_en：模拟接收测试引脚测试使能
        - 配置为 不使能
    - 把上面配置好了之后一次写入到寄存器 `GX_HAL_IRC_ANA`
    - 使用联合体 union：
        - 联合体允许在通一片内存位置存储不同类型的数据。
        - 联合体的所有成员共享同一片内存空间，因此联合体的大小等于其最大成员的大小。
        - 联合体的所有成员共享同一片内存空间，修改一个成员可能会影响其他成员的值
    - 读 `GX_HAL_IRC_FIFO_PARA` 寄存器获取灵活模式下的 `tx_fifo_depth/width` 等信息
        - tx_fifo_depth
        - tx_fifo_width
        - rx_fifo_depth
        - rx_fifo_width

#### tx：



- 相关结构体：

  ```c
  /**
   * @brief IR 发送配置参数
   */
  typedef struct {
  	unsigned char modulation_en;  /*!< 使能调制, 1:使能，0:关闭 */
  	unsigned char type;           /*!< 标准模式的协议类型，0：9012， 1：NEC， 2：RC5， 3：RC6 */
  	unsigned char tx_mode;        /*!< 0: UD, 1:std */
  	unsigned char ud_txfifo_thr;  /*!< tx FIFO 门限配置，0：空， 1：剩余 2 个， 2：剩余 1/4, 3：剩余 1/2 */
  	unsigned short ud_clk_div;    /*!< 计数时钟分频比(10bit有效长度)，计数时钟评率为模块时钟频率 irc_clk/(ud_clk_div+1) */
  	unsigned int data_len;        /*!< 发送数据长度 */
  	unsigned int *data;           /*!< 待发送的数据 */
  }GX_HAL_IR_TX_CFG_T;
  ```



#### rx:

- 相关结构体

  ```c
  /**
   * @brief IR 接收配置参数
   */
  typedef struct {
  	unsigned short rx_thr_start;  /*!< 标准模式下接收 startbit 的判决门限，误差容限，+/-n，以模块 clock(irc_clk) 计 */
  	unsigned short rx_thr_data;   /*!< 标准模式下接收数据的判决门限，仅对NEC和9012协议有效，误差容限，+/-n，以模块 clock(irc_clk) 计 */
  	unsigned char deb_legth;      /*!< 消抖滤波器配置，5bit 有效长度 */
  	unsigned char demod_legth;    /*!< 解调选档，可解调最低载波频率 = irc_clk/32/8/(8+demod_legth) */
  	unsigned char invert_en;      /*!< 接收信号反相使能，标准模式和自由模式下都有效。1：使能， 0：禁用 */
  	unsigned char kdc_cmp_en;     /*!< 标准模式 command code 校验使能，1：使能， 0：禁用 */
  	unsigned char cc_cmp_en;      /*!< 标准模式 custom code 校验使能，1：使能， 0：禁用 */
  	unsigned char demod_en;       /*!< 接收解调使能，1：使能， 0：禁用 */
  	unsigned char deb_en;         /*!< 消抖滤波器使能，1：使能， 0：禁用 */
  	unsigned char ud_rxfifo_thr;  /*!< 自定义模式 rx FIFO 门限配置，0：FIFO中有一个数据， 1：已有1/4个， 2：有1/2, 3:差2个满 */
  	unsigned int std_rx_tov;      /*!< 标准模式接收低电平超时配置，数据上升沿清零计数器开始计时，若接收低电平大于等于 std_rx_tov 个 irc_clk 时钟周期，则认为接收结束 */
  	unsigned short ud_clk_div;    /*!< 自定义模式计数时钟分频比( 10bit 有效长度)，计数时钟频率为模块时钟频率irc_clk/(ud_clk_div+1) */
  	unsigned short ud_rx_tov;     /*!< 自定义模式接收低电平超时配置，高电平+电平总时长，若接收低电平大于等于 ud_rx_tov*ud_clk_div 个 irc_clk 时钟周期，则认为接收结束 */
  }GX_HAL_IR_RX_CFG_T;
  ```

#### `gx_hal_ir_recv_cfg`

- 读寄存器 `GX_HAL_IRC_ANA` 的配置
- 如果开启了内部模拟，并且红外模拟接收没使能
    - 使能红外模拟接收
    - 不使能红外模拟发送
    - 配置写到寄存器 `GX_HAL_IRC_ANA`
- 如果模式是 UD 灵活模式：
    - 如果开启：
        - 将 `GX_HAL_IRC_UD_CTRL` 中的配置读出来到 val
        - 把配置的 `ud_rxfifo_thr、val & (0x3 << 2)、ud_clk_div` 组合起来写入到寄存器 `GX_HAL_IRC_UD_CTRL`
        - 将 `ud_rx_tov` 写入到寄存器 `GX_HAL_IRC_UDRX_TOV`
        - 将 0x70080 写入到寄存器 `GX_HAL_IRC_INT_STA`，清掉 UD 模式的中断状态
        - 配置 `std_rx_en` 使能
    - 如果关闭：
        - 配置 `std_rx_en` 失能
- 如果模式是 STD 标准模式：
    - 如果开启：
        - 把配置的 `rx_thr_data、rx_thr_start` 组合起来写入到寄存器 `GX_HAL_IRC_RX_THR`
        - 把 0x14f 写入到寄存器 `GX_HAL_IRC_INT_STA`，清掉 STD 模式的中断状态
        - 把 std_rx_tov 写入到寄存器 `GX_HAL_IRC_STD_RXTOV` 配置超时时间
        - 配置 `std_rx_en` 使能
    - 如果关闭：
        - 配置 `std_rx_en` 失能
- 配置 `ud_rx_en、std_rx_en、deb_en、demod_en、cc_cmp_en、kdc_cmp_en、invert_en、demod_length、deb_length`
- 将上述配置写入到寄存器 `GX_HAL_IRC_RX_CTRL`


#### `gx_hal_ir_recv_data_std`

- 读寄存器 `GX_HAL_IRC_INTEN`
- 关闭除固定模式协议识别失败中断外的所有中断
- 写入到寄存器 `GX_HAL_IRC_INTEN`
- while (1)
    - 读寄存器 `GX_HAL_IRC_INT_STA` 寄存器获取当前的中断状态到 int_stat
    - RX_DONE 中断：
        - 从寄存器 `GX_HAL_IRC_RX_BUF` 寄存器读数据到 *data，用于返回给用户
        - 从寄存器 `GX_HAL_IRC_STA` 寄存器读出当前的控制器状态到 val
            - 协议识别成功：
                - 写 0x04 到寄存器 `GX_HAL_IRC_STA` 寄存器清除状态
                - 如果中断状态是固定模式读到 `repeat` 码
                    - ret = 0x10
                    - 写 0x02 到寄存器 `GX_HAL_IRC_INT_STA` 寄存器清除 repeat
                    - *type = 0x80 | (val & 0x3(识别到的协议类型))
                - 读到的不是 `repeat` 码
                    - ret = 0
                    - *type = 识别到的协议类型
        - 写 0x40 到寄存器 `GX_HAL_IRC_INT_STA` 寄存器清除 `RX_DONE` 中断
    - Custom_err 中断：
        - ret |= GX_HAL_IR_CUSTM_CODE_ERR
        - 写 0x08 到寄存器 `GX_HAL_IRC_INT_STA` 寄存器清除 `Custom err` 中断
    - Command err 中断：
        - ret |= GX_HAL_IR_COMAND_CODE_ERR
        - 写 0x04 到寄存器 `GX_HAL_IRC_INT_STA` 寄存器清除 `Command err` 中断
    - 固定模式协议识别失败中断：
        - ret |= GX_HAL_IR_DET_ERR
        - 写 0x01 到寄存器 `GX_HAL_IRC_INT_STA` 寄存器清除 STD 模式识别错误中断
    - 如果 ret 被修改了，则退出当前循环读取中断状态模式
    - 如果有 timeout，则循环 timeout 次后退出
- 返回 ret

#### `gx_hal_ir_recv_std_it_start`

- 调用 `gx_hal_ir_recv_reg(ir_dev, rx, GX_HAL_IR_RECV_STD_MODE, GX_HAL_IR_ON)`
    - 配 RX THR 阈值
    - 清中断
    - 配超时时间
    - 把 `std_rx_en、deb_en、demod_en、cc_cmp_en、kdc_cmp_en...` 等等配到寄存器`GX_HAL_IRC_RX_CTRL`
- 读寄存器 `GX_HAL_IRC_INTEN` 获取中断使能状态
- 开启 STD RECV 相关的中断，配置到寄存器 `GX_HAL_IRC_INTEN`

- 应该就到了中断服务函数：`gx_hal_ir_int_handler`
    - 从 `GX_HAL_IRC_INT_STA` 寄存器获取中断状态到 `int_stat`
    - 从 `GX_HAL_IRC_INTEN` 寄存器获取当前的中断使能状态到 `int_en`
    - 将 `int_stat` 写入到 `GX_HAL_IRC_INT_STA` 寄存器清掉所有的中断
    - 如果是 `Custom code err` 中断：
        - err |= GX_HAL_IR_CUSTROM_CODE_ERR
    - 如果是 `Command code err` 中断：
        - err |= GX_HAL_IR_COMMAND_CODE_ERR
    - 如果是 固定模式协议识别失败 中断：
        - err |= GX_HAL_IR_PORT_DET_ERR
    - 如果是 固定模式接收 timeout 中断：
        - err |= GX_HAL_IR_STD_RX_TIMEOUT
    - 如果是 固定模式接收完成 中断：
        - 从 `GX_HAL_IRC_STA` 寄存器获取当前控制器状态到 `val`
            - 如果 val & 0x04 表示协议识别成功
                - 写 0x04 到 `GX_HAL_IRC_STA` 寄存器清掉状态
                - 如果 中断状态 & 0x02 表示此次是 repeat code
                    type = 0x80 | (val&0x3)
                - 否则
                    type = val & 0x3
                将 `GX_HAL_IRC_RX_BUF` 中存储的数据保存到 `val`
            - 否则是协议识别失败
                - val = 0
                - err |= GX_HAL_IR_PROT_DET_ERR
            - 如果注册了 `std_rx_done_cb`
                - 执行 `std_rx_done_cb(ir_dev, val, type, err)`
                    - 解析获取到的 type 和 data，打印
                    - 如果是 err，打印产生了何种 err
    - 否则 前面出错了 err 有效
        - 如果注册了 `std_rx_err_cb` 
            - 执行 `std_rx_err_cb(ir_dev, err)`
                - 打印当前产生了何种 err
    - 如果 `int_stat` 中产生了 `GX_HAL_IRC_INT_UD_RXFIFO_THR_FLAG_MASK` 灵活模式 RXFIFO 到达门限中断
        - 如果注册了 `ud_rx_thr_cb`
            - 执行 `ud_rx_thr_cb(ir_dev)`：将 RXFIFO 中的数据拷贝到 ringbuffer[recv_ud_w] 处
                - **调用 hal 提供的接口 `gx_hal_ir_get_udrx_fifo_count` 从 `GX_HAL_IRC_FIFO_CAP` 寄存器获取 RXFIFO 中当前包含的数据个数到 `cnt`**
                - while (cnt--) 循环从 RXFIFO 取数据
                    - **调用 hal 提供的接口 `gx_hal_ir_get_udrx_fifo_data` 从 `GX_HAL_IRC_FIFO` 寄存器获取 RXFIFO 中当前的数据到 `val`**
                    - **调用 `gx_ir_test_write_ud_recv_data(ir_dev, &val, 1)**
                        - 定义一个指针 ` GX_IR_TEST_RING_BUFFER_T *ring_buf` 用于指向 `ir_dev->user_data`
                        - 调用 `gx_ir_test_write_ud_rx_buf(ring_buf->recv_ud_buff, &ring_buf->recv_ud_w, &ring_buf->recv_ud_r, data, len)`
                            - 如果 len 超过 ringbuffer 的长度，则最大长度是 ringbuffer
                            - 循环 len 次
                                - memcpy(ring_buf->recv_ud_buff, data, 4) 把 data 拷贝到 ringbuffer 固定的位置
                                - 移动 w_buffer 的位置
                                - 如果 w_buffer 的位置和 r_buffer 的位置一样则 r_buffer 向前移位
    - 如果 `int_stat` 中产生了 `GX_HAL_IRC_INT_UD_RXFIFO_FULL_FLAG_MASK` 灵活模式 RXFIFO 满中断
        - 如果注册了 `ud_rx_full_cb`
            - 执行 `ud_rx_full_cb(ir_dev)`
                - 打印 rx fifo full
    - 如果 `int_stat` 中产生了 `GX_HAL_IRC_INT_UR_RXFIFO_EMPTY_FLAG_MASK` 灵活模式 RXFIFO Empty 中断
        - 如果注册了 `ud_rx_empty_cb`
            - 执行 `ud_rx_empty_cb(ir_dev)`
                - return 0
    - 如果 `int_stat` 中产生了 `GX_HAL_IRC_INT_UD_TXFIFO_THR_FLAG_MASK` 灵活模式 TXFIFO 到达门限中断
        - 如果注册了 `ud_tx_thr_cb`
            - 执行 `ud_tx_thr_cb`
                - **调用 `gx_hal_ir_get_udtx_fifo_count` 从 `GX_HAL_IRC_FIFO_CAP` 寄存器获取当前 TXFIFO 包含的数据个数，用 `txfifo depth`-count 计算当前 txfifo 有多少空闲**
                - **调用 `gx_hal_ir_udtx_fifo_put(ir_dev, g_data[g_count]` 将 g_data 中的数据都发送到 TXFIFO**
                - 循环将要发送的数据发送完




### cmd/gx_ir_test.c

#### init:
- 配置 IRC 模块时钟源，分频，开启 IRC 模块时钟
- 配置占空比为 30
- 配置发送载波频率为 38Khz
- 使能内部模拟模拟部分:internal_ana_en
- 配置模拟接收测试引脚不使能:test_ana_pin_en
- 配置模拟接受测试通不通电压到 test ana:testtsel(填参)
- 配置模拟接收测试参考电流:isel(填参)
- 配置模拟接收参考电压档位:vcmsel(填参)
- 配置发送驱动能力：drv_cap(填参)
- 配置模拟发射电路软启动控制:vstset(填参)
- 配置模拟发射电路测试模式:testmod_en(填参)
- 获取当前模块时钟：src_clk
- 配置当前寄存器基址
- 配置标准模式 rx done 回调
- 配置标准模式 rx err 回调
- 配置灵活模式 rx thr 回调
- 配置灵活模式 rx done 回调
- 配置灵活模式 tx done 回调
- 配置标准模式 tx done 回调
- 配置灵活模式 rx full 回调
- 配置灵活模式 rx empty 回调
- 配置灵活模式 rx thr 回调
- 配置灵活模式 tx full 回调
- 配置灵活模式 tx empty 回调
- 配置用户用于测试时的 buffer：`gx_ir_test_ring_buff` 一片固定的内存你地址，大小为 256
- 申请中断，包括中断号、中断服务程序、参数(GX_HAL_IR g_test_ir_dev)
- **通过 hal 提供的接口 `gx_hal_ir_init(&g_test_ir_dev, &hal_cfg)` 将上述配置写入到寄存器**


#### recv_std

- `gx_ir_test_recv_std`：标准模式接收，带解调
    - 配置 rx 模式下接收 startbit 的裁决门限 rx_thr_start:0x7ff
    - 配置 rx 模式下接收 data 的裁决门限 rx_thr_data:0x7ff
    - 配置消抖滤波器 deb_length：0x0f
    - 配置解调选档 demod_length：0x07
    - 配置接收解调使能 demod_en: 1
    - 配置是否翻转电平 invert_en
    - 配置消抖滤波器使能 deb_en：1
    - 配置标准模式接收低电平超时配置 std_rx_tov: 120*1000
        - **通过 hal 提供的接口 `gx_hal_ir_recv_cfg(ir_dev, &rx, GX_HAL_RECV_STD_MODE, GX_HAL_IR_ON)` 将上述配置写入到寄存器**

    - 配置到寄存器后， while(1) 循环：
        - **通过 hal 提供的接口 `gx_hal_ir_recv_data_std(ir_dev, &rx, (void*)&data, &type, 0)` 获取数据到 data，获取按键的协议到 type**
        - 判断是否为 `repeat`，以及按键的类型和键值


#### recv_std_it

- 'gx_ir_test_recv_std_it`：标准模式中断接收
    - 配置 rx 模式下 startbit 的裁决门限 rx_thr_start:0x7ff
    - 配置 rx 模式下 data 的裁决门限 rx_thr_data:0x7ff
    - 配置消抖滤波器 deb_length:0x0f
    - 配置接收解调使能 demod_en:1
    - 配置是否翻转电平 invert_en:0
    - 配置消抖滤波器使能 deb_en:1
    - 配置标准模式 command 校验使能 kdc_cmp_en:1
    - 配置标准模式 custom 校验使能 cc_cmp_en:1
    - 配置标准模式接收低电平超时配置 std_rx_tov:120*1000
    - **调用 hal 提供的接口 `gx_hal_ir_recv_std_it_start(ir_dev, &rx)` **






# FAQ

- `gx_hal_ir_init` 函数中配置 `internal_ana_en` 时，如果开启了就 发射和接收都配，如果没开启就只配发送？为什么没开启还要配发送？
- `gx_hal_ir_recv_data_std` 函数中把所有的 std 模式中断都关掉了，下面的 `GX_HAL_IRC_INT_STA` 还能检测到中断标志吗？


## 标准模式
### TX
- 如何发数据？
	- 先把数据填到 tx_buf，再使能 tx_done，再触发发送
- 如何发 Repeat？发 Repeat 时会附带数据吗？还是单独的 Repeat ？
	- 先把数据填到 tx_buf，再使能 tx_done，再触发发送，满足 repeat 就迅速的往 ctrl 寄存器配置使能 repeat，再触发发送
	- 发 Repeat 的时候就是单独的 repeat





### RX
- 如何收数据？
	- 使能相关的中断，使能 rx 开始接收，从 rx_buf 中取走数据
- 如何收 Repeat？
	- 使能相关的中断，使能 rx 开始接收，如果 repeat_det 有效，则接收到的数据不变，不需要读 rx_buf，认为是 repeat 码


## 载波问题
- 发射加入载波，和接收去载波，都是数字逻辑完成的，红外头只是充当传输载体的功能
- 原理上，载波的0101，这个由数字实现，体现在发射红外头就是红外线的亮灭亮灭
- 对于接收来说，接收头就是有红外信号--无红外信号--有红外信号--无红外信号，载波的37k不会发生变化的
- 对于使用的发射管来发射和接收，接收的时候不会去载波
- 如果用的是带屏蔽头的接收管，接收的时候会去载波


## 灵活模式
### TX
将待发射数据填到 TX_FIFO，高 16bit 为两次上升沿之间的计数值，低 16bit 为高电平的时长计数值。
配置 tx_trig = 1 触发发射后，用 irc_clk 分频出 100KHz 时钟。

**要发的数据数量是由往 tx fifo 写的数据来决定，如果一直写，就一直发。不想发了，就往 tx fifo 填一个 0xffffffff，就停止发送。**

假如要发送的数据很多，超过了 fifo 深度，可以使能 tx_thr 中断，在触发了之后继续往 fifo 填数据，数据填完之后最后要补上 0xffffffff。最后等待 tx_done 中断。
假如要发的数据不多，没超过 fifo 深度，使能 tx_done 中断，往 fifo 填数据，最后补上 0xffffffff。最后等待 tx_done 中断。

### RX
用 irc_clk 分频出 100KHz 时钟，然后从每个接收信号上升沿计数器清零，开始计数，下降沿记录高电平脉宽，下一次上升沿记录完整周期的计数值，把记录的计数值存入 RX FIFO，然后计数器清零。
![[Pasted image 20250610171024.png]]

若低电平持续时间长于设定的阈值 rx_tov，则认为已经完成接收，产生 rx_done 中断。
![[Pasted image 20250610171033.png]]


若低电平持续时间长于设定的阈值 rx_tov，则认为已经接收完成，产生 rx_done 中断。
也就是所，如果外面一直发，那么我这边也可以一直收，存在两个中断：
- 阈值中断：配置 fifo 的一半，rx fifo 数据到了一半之后，就取走
- 满中断：如果 fifo 满了，如果还没处理呢？会不会溢出？
- rx_done 中断：低电平持续时长长于设定的阈值 rx_tov，认为接收完成了，把 rx fifo 中的数据都读走






### 灵活模式使用中断发送的方式，发现发射操作一直不完成

#### **核心问题**

使用灵活模式中断的方式发数据，有一大批数据，采用阈值中断，当阈值中断产生之后就往 tx fifo 填数据。

在上述模式下，如果因为处理延迟，未能在**阈值中断** (`threshold interrupt`) 发生时及时向 TX FIFO 填充数据，会导致红外发送流程**卡死**。

#### **具体表现**

1. **发送中止**：即使后续向 TX FIFO 写入新数据，红外模块也不会再继续发送。
2. **中断不触发**：此时写入结束标记 `0xFFFFFFFF`，也无法正常触发 **发送完成中断** (`tx done interrupt`)。
3. **流程挂起**：整个发送流程将一直处于挂起状态，无法正常结束。

#### **处理规则与注意事项**

1. **必须在 `tx empty` 中断中处理**：为了避免上述卡死情况，必须在 **发送空中断** (`tx empty interrupt`) 产生时，及时处理并正确退出当前的发送流程。
2. **结束标记的写入时机至关重要**：
    - 结束标记 `0xFFFFFFFF` **必须在发送最后一个有效数据之前** 就填入 TX FIFO。
    - **严禁** 在所有数据都已从 FIFO 发送完毕后，再尝试写入结束标记。这种操作将无法触发 `tx done` 中断，导致流程异常。




### 软件获取接收数据时的载波频率 (Sagitta 硬件已实现)

#### **核心原理**

接收时使用的是红外发射管的弱接收功能，由于接收到的波形都是带载波的，红外发射管并不会解调，原来是通过 ip 硬件来解调，现在不解调波形，灵活模式接收的时候进来的数据就是带载波的，可以拿到载波的一个周期时长是多少，就能算出载波频率。

通过设置特定的“灵活模式”，关闭内置的解调和去抖动模块，让原始的、带有载波的信号直接进入接收逻辑。同时，配置一个高速的采样时钟，对该原始载波信号进行测量，从而获得其周期和频率信息。

#### **操作步骤与配置**

1. **进入载波测量模式**:
    
    - 在 `RX_CTRL` 寄存器中，将 `demod_en` (解调使能) 和 `deb_en` (去抖动使能) 位都配置为 **`0`**。
    - **目的**: 这会绕过（Bypass）内部的解调模块，确保原始的载波信号能够被后续模块直接处理。
2. **配置采样时钟**: (时钟越高，采样到的时间越准)
    
    - 在 `UD_CTRL` 寄存器中，配置 `clk_div` (时钟分频) 位。
    - **配置原则**: 需要配置一个比常规模式下**更高频率**的采样时钟，以确保能精确捕捉到载波的高低电平时间。
    - **示例**:
        - 假设 IP 的主时钟 `irc_clk` 为 8MHz。
        - 为了测量载波，可以将 `clk_div` 配置为 **`7`**。这表示进行 8 分频 (2(7+1) 的情况，或者根据具体设计，是 `7+1`=8分频)，从而得到一个 **1MHz** 的采样时钟 (8MHz/8=1MHz)。
        - （_注意：具体分频系数 `N` 与 `clk_div` 值的关系需参考您的 IP 设计文档，此处 `clk_div` 设为 `7` 是一个示例。_）
3. **读取 FIFO 获取数据**:
    
    - 完成上述配置后，像常规的灵活模式一样从 `RX FIFO` 中读取数据。
    - 此时 FIFO 中存储的不再是解调后的逻辑 '0' 和 '1' 的脉宽，而是**载波信号本身**的参数。
4. **计算频率**:
    
    - 从 FIFO 中可以读出两个关键值：
        - **高电平宽度**: 载波脉冲的持续时间。
        - **完整周期宽度**: 载波单个周期的总时间。
    - 通过 “完整周期宽度” 这个值，就可以直接计算出载波的频率 (f=T1​，其中 T 为周期宽度)。


## NEC协议，我们一帧的结束是在stopbit发完，而客户提到需要以固定的108ms为一帧，如果在108ms内发新数据，可能会造成对端设备解码错误 (Sagitta 硬件已实现)
- apus 发送时可能存在一帧数据不是完整的 108ms，对方设备可能解码错误
- sagitta 增加了一个帧间间隔功能，打开之后可以配置每帧之间的空闲时长，确保每帧在 108ms 及以上；不打开和 apus 相同，可以直接发一帧数据。



## 重复调用 send_std_it + repeat_en + send_stop，只会发送 repeat_code
- send_std_id 先发数据，然后立马置位 repeat_en 此时可能间隔比较近，导致直接发了 repeat，没有发数据
- 在 repeat_en 之前加 1us 延时，就正常了


## 自由模式下如果 txfifo num 为 0，将数据填到 txfifo 后可能无法发出
- 先使能 tx，再往 txfifo 填数据，可能导致从 txfifo 取数据时略过 startbit 导致无法发出数据
- 软件规避：使能 tx 之前，先往 txfifo 中填充至少一个数据。



## 外围电路解析
![[6ec55400f680b8ab5b0b1b9ceb6cc7e9 1.jpg]]
NPN 导通条件： Vb＞Ve+0.7V
PNP 导通条件： Vb＜Ve-0.7V
### 发射
![[Pasted image 20250808133341.png]]

### 接收

![[Pasted image 20250808133353.png]]


- 测量 R65 电阻上的波形，也就是 IR_REC1 引脚上的波形
- 也可以测量 C 极上的波形，一样的。
	- 注意：需要查看三极管的封装图，有的管子单独那个脚是 C 极，有的单独的脚是 B 极。
![[a66d00568f499a1e2c18aa50bef04474.jpg]]


## 测试接口
![[Pasted image 20250809111032.png]]
正常发射：芯片的发射脚通过 o_irc_txd 到模拟部分的 tx 脚发出去
正常接收：从模拟部分的 rx 脚收到数据，经过i_irc_rxd，再到 irc_rxd 就能收到数据
旁路数字发射：数字PAD 过 irc_txd_pad，再过 o_irc_txd 给到模拟部分的 tx 脚发出去。   这里旁路数字发射指的是，把旁路的数字信号发射出去吧，看看要发的数据对不对
旁路模拟发射：irc_txd 过 irc_txd_pad，再过 io matrix，通过数字 PAD 发出。                  这里旁路模拟发射指的是，用旁路的数字脚来模拟发射数据吧，看看发出的数据对不对
旁路数字接收：模拟部分的 rx 脚收到的数据直接给到 ANA TEST。                                     这里旁路数字接收指的是，用旁路的数字脚来接收数据吧，看看收到的数据对不对
旁路模拟接收：ANA TEST 脚的数据给到 i_irc_rxd ，再到 irc_rcd。                                   这里旁路模拟接收指的是，把旁路的数字信号接收进来吧，看看要收的数据对不对



### 正常发射：
- REG_IRC_TEST(0x40) 配置 irc_txd_sel = 1
- REG_IRC_ANA(0x3C) 配置 ana_tx_en = 1，使能内部的发射模拟电路
- 内部的数字信号 --> 调制器 --> 内部的模拟部分 --> 外部的引脚 --> 驱动 发射管

### 正常接收：
- REG_IRC_TEST(0x40) 配置 irc_rxd_sel = 1
- REG_IRC_ANA(0x3C) 配置 ana_rx_en = 1，使能接收模拟电路
- 可选配置：rx_isel、rx_vcmsel
- 红外管 --> 外部的引脚 --> 内部的模拟电路 --> 解调 --> 去抖 --> 内部的数字部分 --> FIFO

![[Pasted image 20250812142634.png]]


### 旁路数字发射：irc_txd_sel = 0；ANA_TEST 配置为红外 TX 输入
- 独立验证 内部的 TX 部分和外部驱动电路。
- 可以直接在外部的 PAD 上灌带有载波的红外波形，查看是否能够正常驱动红外发射管。
- **其实就是把内部的数字换成旁路的数字 PAD，然后用内部的模拟发射部分发出去，看看内部的模拟发射部分或者外部的驱动电路是否正常。**
- REG_IRC_TEST(0x40) 配置 irc_txd_sel = 0
- REG_IRC_ANA(0x3C) 配置 ana_tx_en = 1，使能内部的发射模拟电路
- 外部引脚 --> IO Matrix --> irc_txd_pad --> irc_txd_sel = 0 --> o_irc_txd --> 内部的 TX 模拟部分 --> 外部的引脚 --> 驱动发射管。




### 旁路模拟发射：
- 独立验证 数字逻辑 的波形生成功能。
- 通过把数字核心产生的信号直接输出到一个通用的 IO 口，完全绕过内部的 TX 模拟发射模块和外部的红外管。
- REG_IRC_TEST(0x40) 配置应该没关系
- REG_IRC_ANA(0x3C) 配置 ana_tx_en = 0，确保内部的模拟发射电路关闭。
- 数字逻辑产生的波形 --> 调制 --> irc_tx_sel = 1 --> o_irc_txd --> 外部的测试引脚。
- **其实就是把内部产生的数字逻辑放到一个PAD上发出去，看看内部的数字发射部分是否正常。**

![[Pasted image 20250812142652.png]]

- 代码配置：
	- 0x40：发射配成 1，发射通道选择正常模式，内部数字接内部模拟
	- 0x3C：ana_tx_en 配成0，表示不启用内部的模拟发射
	- 引脚复用随便将一个引脚配置成 IRC_TX_TEST(O)
		- 因为是发射的时候，这个引脚是输出模式
```diff
diff --git a/board/nationalchip/apus_nre_1v/pinmux.c b/board/nationalchip/apus_nre_1v/pinmux.c
index ab2fc100..c496a717 100644
--- a/board/nationalchip/apus_nre_1v/pinmux.c
+++ b/board/nationalchip/apus_nre_1v/pinmux.c
@@ -57,7 +57,7 @@ static const GX_PIN_CONFIG pin_table[] = {
     /* {13,  2  }, // P1_5 */
     /* {14,  33 }, // P1_6 */
     {15,  0  }, // P1_7
-    {16,  29 }, // P2_0
+    {16,  126 }, // P2_0
     {17,  10 }, // P2_1
     {18,  97 }, // P2_2
     {19,  98 }, // P2_3
diff --git a/cmd/gx_ir_test.c b/cmd/gx_ir_test.c
index c8977251..d406fc77 100644
--- a/cmd/gx_ir_test.c
+++ b/cmd/gx_ir_test.c
@@ -816,10 +816,10 @@ int gx_ir_test_send_std(GX_HAL_IR *ir_dev, uint8_t type)
 	uint32_t data[4] = {0x23456789,0xeeff1122,0x1b2b3b4b,0x90441c02};
 	memset(&tx, 0, sizeof(tx));
 	tx.type = type;
-	tx.modulation_en = 1;
+	tx.modulation_en = 0;
 	tx.data = (void*)data;
 	tx.data_len = 4;
-	gx_hal_ir_send_data_std(ir_dev, &tx, gx_mdelay, 0);
+	gx_hal_ir_send_data_std(ir_dev, &tx, gx_mdelay, 20);
 
 	return 0;
 }
diff --git a/drivers/hal/src/ir/gx_hal_ir.c b/drivers/hal/src/ir/gx_hal_ir.c
index aa0f81e2..169994a9 100644
--- a/drivers/hal/src/ir/gx_hal_ir.c
+++ b/drivers/hal/src/ir/gx_hal_ir.c
@@ -338,12 +338,12 @@ int gx_hal_ir_send_data_std(GX_HAL_IR *ir_dev, const GX_HAL_IR_TX_CFG_T *tx,
 
 	gx_hal_ir_recv_close(ir_dev);
 	gx_hal_ir_get_ana_reg(ir_dev, &ana_reg);
-	if(ir_dev->internal_ana_en && !ana_reg.field.tx_ana_en)
-	{
+//	if(ir_dev->internal_ana_en && !ana_reg.field.tx_ana_en)
+//	{
 		ana_reg.field.rx_ana_en = 0;
-		ana_reg.field.tx_ana_en = 1;
+		ana_reg.field.tx_ana_en = 0;
 		gx_hal_ir_set_ana_reg(ir_dev, &ana_reg);
-	}
+//	}
```
- 默认发的波形是带载波的，现在把载波去掉了，逻辑分析仪抓到的波形是这样的
![[Pasted image 20250812150117.png]]






### 旁路数字接收：
- 独立验证 数字逻辑核心的解码功能
- REG_IRC_TEST(0x40) 配置 irc_rxd_sel = 0
- REG_IRC_ANA(0x3C) 配置 ana_rx_en = 0
- 外部引脚灌入带载波的红外波形 --> 解调 --> 去抖 --> 内部的数字部分 --> FIFO
- **外部用一个引脚灌数字红外波形，数据不过内部模拟，直接给到内部数字部分，查看内部的数字部分能否接受正常**




- 上面的说法感觉是错误的，在接收的时候，经由内部的模拟接收部分放大、滤波后的数据，通过 ANA TEST 引脚输出出来，查看内部的模拟接收部分是否正常。
	- 此时 ANA TEST 是输出模式

### 旁路模拟接收：
- 独立验证 模拟部分的放大功能。
- REG_IRC_TEST(0x40) 配置 irc_rxd_sel = 0
- REG_IRC_ANA(0x3C) 配置 ana_rx_en = 1
- REG_IRC_ANA(0x3C) 配置 rx_test_en = 1
- 外部的红外光 --> 模拟脚 --> 内部的模拟接收部分 --> 测试引脚
- **外部用一个引脚接收红外管的数据，数据不过内部数字部分，直接把模拟接收到的数据用一个引脚发出来，查看内部的模拟部分是否接受正常**





- 上面的说法感觉是错误的，应该是外部有一个模拟接收的电路，遥控器发送红外波形，经过红外发射管之后的数据被外围电路放大、滤波，然后给到 ANA TEST 引脚 --> i_irc_rxd --> irc_rxd 给到内部数字部分
	- 没有使用内部的模拟接收部分，采用外部的模拟接收电路，以及内部的数字接收部分。
	- 此时 ANA TEST 是输入模式



![[Pasted image 20250812142704.png]]




# 什么时候需要配置反相？
- 内部数字要求无信号时为低电平，如果模拟给数字的信号，IDLE时是高电平，那就需要反相



# 发射时可以使用手机相机查看红外管是否有变红
- 正常情况下，红外发射管导通的时候，使用手机相机查看可以看到红外发射管变红


# timeout irq
- 标准模式接收超时：
	- 标准模式接收低电平超时配置，数据上升沿清零计数器开始计时，若接收低电平大于等于 std_rx_tov 个 irc_clk 时钟周期，则认为接收超时
	- 目前配置是 `120*1000`，对于 24MHz 时钟，就是 大约 5ms；而标准协议中没有低电平超过 5ms 的情况


# IR 标准模式关闭发射时，会复位 RC5、RC6 的翻转位
```c
int gx_hal_ir_send_data_std_it_stop(GX_HAL_IR *ir_dev)
{
	unsigned int val;

	val = IR_READ_REG(ir_dev, GX_HAL_IRC_INTEN);
	val &= ~GX_HAL_IRC_STD_SEND_INT_CLEAR_BITS;
	IR_WRITE_REG(ir_dev, val, GX_HAL_IRC_INTEN); ///< disable int
	IR_WRITE_REG(ir_dev, 0, GX_HAL_IRC_TX_CTRL); ///< close tx, will reset RC5 and RC6 toggle.
	IR_WRITE_REG(ir_dev, 0x10, GX_HAL_IRC_INT_STA); ///< clear int status

	return 0;
}
```
- 由于 rc5、rc6 是依靠翻转位来判断是否是重复码，因此标准模式发射完成后，不能关闭 TX
	- 但是这样的话，如果先用 it 模式发送，再用 poll 模式发送，就会导致 poll 发不出
	- gx_ir_test.c 也有这个问题！
	- 因此驱动中在 sned_data_std 中发送前添加了判断，如果使能了 std tx done 中断，就关闭掉这个中断
- 否则对于 RC5、RC6 来说每次都是重复码
