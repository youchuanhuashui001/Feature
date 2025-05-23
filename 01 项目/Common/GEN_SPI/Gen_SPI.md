# Slave

DWC_ssi 可以配置在串行目标模式下工作，该模式允许与串行控制器外设进行串行通信。
**当 DWC_ssi 配置为串行目标设备 (SSIC_IS_MASTER= 0) 时，所有串行传输都是由串行总线控制器启动和控制的。**

当配置期间选择 DWC_ssi 串行目标时，它将 txd 数据传输到串行总线上。**所有向串行目标的数据传输和来自串行目标的数据传输都在 sclk_in 上调节，由 Master 驱动**。数据从串行时钟线的一个边缘传播，并在相反的边缘上采样。


**如果串行目标设备是 `receive only`，ssi_clk 信号的最低频率是串行控制器设备 (sclk_in) 比特率时钟最大预期频率的 6 倍。** sclk_in 信号进行双同步，以便将其带入 ssi_clk 域，然后检测边缘。
**如果串行目标设备是 `发送和接收`，ssi_clk 信号的最低频率是串行控制器设备 (sclk_in) 比特率时钟最大预期频率的 8 倍。** 这个最低频率是为了确保在串行控制器的 shift 控制逻辑捕获数据之前，串行控制器 rxd 线上的数据稳定。8：1 的比率确保在数据背部或之前，串行目标已经将数据驱动到串行控制器的 rxd 线上两个 ssi_clk 循环。


> [!note]
>   - `ssi_clk` 通常指的是 **SSI 控制器模块的内部核心时钟** (core clock or internal clock)
> - sclk_out：模块线上时钟 (经过分频后的时钟)
> - 当 DesignWare SPI IP **作为主设备 (master)** 时，这个内部时钟 (`ssi_clk`) 用来产生实际输出到外部的串行时钟 `sclk_out` (有时也称为 `spi_clk` 或 `SCK`)，并且也用于驱动控制器内部的逻辑操作，如数据移位、采样等。`sclk_out` 的频率通常是 `ssi_clk` 的一个分频。
> - 当 DesignWare SPI IP **作为从设备 (slave)** 时（在您提供的文本中被称为 "serial target device"），`ssi_clk` 是从设备**内部运行所依赖的时钟**。它用于同步从设备内部的逻辑，包括处理从主设备接收到的 `sclk_in` 信号，以及准备和发送数据。
> - `sclk_in` 指的是当 DesignWare SPI IP **配置为从设备 (slave)** 时，**从外部串行控制器设备（即 SPI 主设备 master）接收到的串行时钟输入信号**。
> - 这个时钟信号 (`sclk_in`) 决定了主从设备之间数据传输的比特率。从设备使用这个 `sclk_in` 信号作为基准，来锁存从主设备接收到的数据 (MOSI 线上的数据)，并在正确的时钟边沿将数据发送给主设备 (MISO 线上的数据)。

做 Slave 时，时钟源自 Master
Slave 在接收数据时，Sample_delay 有效，需要正确配置