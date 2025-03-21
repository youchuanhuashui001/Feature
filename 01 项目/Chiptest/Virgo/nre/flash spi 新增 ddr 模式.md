
版本由 1.03a 更新到 2.00a

Dual Data-Rate Support

在标准操作中，SPI模式中的数据传输发生在时钟的正边沿或负边沿。为了提高吞吐量，可以使用双数据率传输来读取或写入存储器。DDR模式支持SPI协议的以下模式：

- 模式0 - 当默认串行时钟相位和默认串行时钟极性未启用时（SCPH = 0 & SCPOL = 0）
- 模式3 - 当默认串行时钟相位和默认串行时钟极性启用时（SCPH = 1 & SCPOL = 1）


DDR命令使数据能够在时钟的两个边沿上传输。以下是DDR命令的不同类型：

- 地址和数据以DDR格式传输（或在数据的情况下接收），而指令以标准格式传输。
- 指令、地址和数据均以DDR格式传输或接收。

`SPI_DDR_EN（SPI_CTRL0[16]）` 位用于确定地址和数据是否必须以DDR模式传输，`INST_DDR_EN（SPI_CTRL0[17]）` 位用于确定指令是否必须以DDR格式传输。这些位仅在 `CTRL0.SPI_FRF ` 位设置为双模式、四模式或八模式时有效。


指令、地址、数据都以 DDR 模式传输：
![[Pasted image 20250319141059.png]]


> [!note]
> - 在 DDR 传输中，地址和指令不能编程为 0 值。在 DDR 读取作中，等待周期数不能为 0。



## 在 DDR 模式下传输数据
在 ddr 模式下，数据在两个边沿上传输，因此很难正确采样数据。`DWC_ssi` 使用内部寄存器来确定应传输数据的边沿。这确保了接收器在采样数据是能够获得稳定的数据。
内部寄存器 `DDR_DRIVER_EDGE` 确定传输数据的边沿。
`DWC_ssi` 根据波特率 clock 发送数据，波特率是内部时钟 `ssi_clk * BAUDR` 的整数倍。
数据需要在半个 `clkck cycle(BAUDR / 2)` 内传输，因此 `DDR_DRIVER_DEGE` 寄存器的最大值等于 `(BAUDR/2) - 1`。
数据在 `(BAUDR/2) - TXD_DRIVE_EDGE` 之前 `ssi_clk` 采样边沿之前驱动。
如果 `DDR_DRIVE_EDGE` 编程值为 0，则数据相对于 `sclk_out(baud clock)` 进行边沿对齐传输。
如果 `DDR_DRIVE_EDGE` 编程值为 1，则数据在 `sclk_out` 的边沿前 2 个 `ssi_clk` 传输。



DDR_DRIVER_EDGE 确定传输数据的边沿。

DWC_ssi 根据波特率 clock 发送数据，波特率是内部时钟 (ssi_clk * BAUDR) 的整数倍。
数据需要在半个 clock cycle (BAUDR/2) 内传输，因此最大的 `DDR_DRIVER_EDGE` 值为：`(BAUDR/2) - 1` 。

数据在采样边沿之前的 `(BAUDR/2) - TX_DRIVE_EDGE`  个 ssi_clk 被驱动？
如果 `DDR_DRIVE_EDGE` 值为 0，则数据相对于 sclk_out (baud clock) 进行边沿对齐传输。
如果设置为 1，数据在 sclk_out 边沿的前 2 个 ssi_clk 传输。


如果是 2 分频，那么 `DDR_DRIVE_EDGE` 的最大值是：
`(BAUD/2) - 1` = 0


**在DDR（双倍数据速率）模式下，数据在时钟的两个边沿（上升沿和下降沿）传输，这使得正确采样数据变得困难。**
**DWC_ssi模块通过一个内部寄存器（`DDR_DRIVE_EDGE`）决定数据应在哪个边沿被驱动，从而确保接收端在采样时能获取稳定的数据。**

**DWC_ssi基于波特率时钟（`sclk_out`，由内部时钟 `ssi_clk` 乘以分频系数 `BAUDR` 生成）发送数据。**

**由于数据需要在半个波特率时钟周期（`BAUDR/2`）内完成传输，因此 `DDR_DRIVE_EDGE` 寄存器的最大可编程值为 `[(BAUDR/2)-1]`。**
- 因为一个周期，包括两个半周期，ddr 会在两个边沿都采样，所以必须半个周期就完成传输
**数据在采样边沿前 `((BAUD/2) - TXD_DRIVE_EDGE)` 个 `ssi_clk` 周期被驱动。**

**若 `DDR_DRIVE_EDGE` 设为0，则数据与 `sclk_out` 的边沿对齐；若设为1，则数据在 `sclk_out` 边沿前2个 `ssi_clk` 周期被驱动。​**




# flash 波形
![[Pasted image 20250319163230.png]]
- 指令：单线
- 地址、模式：ddr 模式
- 数据：ddr 模式

## gxmisc
- 四线读：
	- 指令、地址：一个 transfer，可以同时以单线、多线
	- 数据：可以单线、多线
- 要支持八线读：
	- 一个 msg，三个 transfer
		- 传输每个 msg 的时候，先拉低 cs，再拉高 cs；也就是说所有的操作都在一个 cs 拉低拉高的周期内
	- 指令：一个 transfer，单线传
	- 地址：一个 transfer，多线传
	- 数据：一个 transfer，多线传
