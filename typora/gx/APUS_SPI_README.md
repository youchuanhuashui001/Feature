# SPI

## 简介

`SPI，Serial Peripheral Interface Bus`，是一种同步串行数据协议，由 Motorola 在 1970 年代开发。该协议旨在取代并行总线并提供短距离的高速数据传输。

它是一个全双工协议，需要四个信号：时钟、主输出/从输入、主输入/从输出和从选择。数据同时发送和接收。借助 SPI，单个主设备可以控制多个从设备，每个从设备都有自己的从设备选择线。典型应用于外部存储 FLASH，支持可配置的芯片内执行 (XIP) 功能，使应用程序可以直接在 FLASH 闪存内运行。

## 主要特性

* 全双工通信
* 支持主 / 从模式操作
* 支持四线模式 SPI 接口
* 可编程的时钟极性和相位
* 32 位深度FIFO
* 完整的协议灵活性，能够任意选择传输消息数据长度、内容及目标
* AHB 总线接口，数据总线位宽32位，支持配置 8 / 32 bit 数据位宽传输
* 支持 DMA 控制器接口，使用握手接口实现传输请求，通过总线与 DMA 控制器连接
* 主模式下，接收到的串行数据位 (RXD) 的采样时间延时可编程，从而实现更高的串行数据比特率
* 支持串行数据位 (RXD) 的可编程采样：上升沿或下降沿采样
* 支持多种串行接口模式(Motorola、Texas Instruments、National Semiconductor Microwire)

## 功能描述

​		SPI 控制器方框图如下：

![](./image/dw_spi.png)

​	通常SPI通过4个引脚与外部器件相连：

- MISO：主设备输入/从设备输出引脚。该引脚在从模式下发送数据，在主模式下接收数据。
- MOSI：主设备输出/从设备输入引脚。该引脚在主模式下发送数据，在从模式下接收数据。
- SCK：串口时钟，作为主设备的输出，从设备的输入
- NSS：从设备选择。这是一个可选的引脚，用来选择主/从设备。它的功能是用来作为“片选引脚”，让主设备可以单独地与特定从设备通讯，避免数据线上的冲突。



**时钟信号的相位和极性**

![](./image/cpol_cpha.png)

主机可以配置与数据有关的时钟极性和相位。Motorola 将这两个选项分别命名为 CPOL 和 CPHA (用于时钟极性和相位)，时序图如上所示：

- CPOL确定时钟的极性：
  - CPOL = 0 时，SCK 引脚在空闲状态保持低电平
  - CPOL = 1时，SCK 引脚在空闲状态保持高电平
- CPHA确定时钟的采样边沿：
  - CPHA = 0时，SCK 时钟的第一个边沿进行数据位采样，数据在第一个时钟边沿被锁存
  - CPHA = 1时，SCK 时钟的第二个边沿进行数据位采样，数据在第二个时钟边沿被锁存



**SPI多线传输模式**

**Dual SPI(双线SPI)**

![](./image/dual_spi.png)

Dual SPI 具有双 I/O 接口，与标准串行闪存设备相比，传输速率可以提高一倍。MISO 和 MOSI 数据引脚在半双工模式下运行，每个时钟周期发送两位。MOSI 线变为 IO0，MISO 线变为 IO1。数据吞吐率可比标准配置下提高一倍。

**Quad SPI(四线SPI)**

![](./image/quad_spi.png)

Quad SPI 类似于 Dual，但将吞吐量提高了四倍。添加了两条额外的数据线，MOSI 线和 MISO 线仍为 IO0、IO1，WP 线变为 IO2，HOLD 线变为 IO3，每个时钟周期传输 4 位。数据传输为：IO0、IO1、IO2 和 IO3。

