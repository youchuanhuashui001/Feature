# ahb_dma 控制器

## 概念

- DMA 之所以能够把一个地址的数据搬运到另一个地址，是由于这些地址内部由AHB和APB总线管理，所以DMA必须挂在AHB和APB上。

- Source Peripheral：AHB层上DW_ahb_dmac读取数据的源外设设备，然后将读取到的数据存储在通道FIFO中。源外设和目标外设组成一个通道。源外设设备是一个AHB或APB从机。如果源是APB从机，则通过AHB-APB桥来访问。

- Destination Peripheral：DW_ahb_dmac从FIFO写入到存储数据的设备。目标外设是一个AHB或APB从机。如果目标外设是APB从机，则通过AHB-APB桥来访问。

- Memory：总是为DMA传输`ready`的源或目标，不需要握手信号与DW_ahb_dmac交互。

- Channel：在一个已配置的AHB层上的源外设和同一或不同层的AHB层上的目标外设之间，通过通道FIFO进行读写数据。如果源外设不是内存，那么源握手接口被分配给通道。如果目的外设不是内存，那么目标握手接口被分配给通道。源握手和目的接收可以通过对通道寄存器进行编程来动态分配。

- Master Interface：主接口，从源端读取数据，并通过AHB总线将数据写入目标端。

- Slave Interface：从机接口

- Handshaking Interface：一组信号或软件寄存器。通道可以通过三种握手信号中的一种来接收请求：硬件、软件、外设。
  - 硬件握手信号：使用硬件信号控制在DW_ahb_dmac和源或目标外设之间传输single or burst

  - 软件握手信号：使用软件寄存器来控制在DW_ahb_dmac和源或目标外设之间传输single or burst

  - 外设握手信号：简单实用硬件握手接口，来自外设的中断线被绑定到硬件握手接口的dma_req输入，其它接口信号被忽略

- Flow Controller：设备(DW_ahb_dmac或Source/Peripheral)，它决定DMA块传输的长度并终止它。
  - **谁决定传输的长度并停止传输，谁就是流控端**
  - 如果在启用通道之前知道一个块的长度，那么应该将DW_ahb_dmac编程为流控
  - 如果在启用通道之前不知道块的长度，需要由源或目标来终止块传输。这种模式下，外设就是流控。

- 流控模式(CFGx.FCMODE)：仅当目的外设为流量控制器时才使用的特殊模式。它控制从源外设预取数据

- 传输层次：描述DW_ahb_dmac传输、块传输、事务(single/burst)和非内存外设的AHB传输(single/burst)之间的层次结构

  - 非内存外设的传输层次结构

  ![image-20230228093756302](image/image-20230228093756302.png)

  - 内存的传输层次结构

    ![image-20230228093955250](image/image-20230228093955250.png)

  - Block：DW_ahb_dmac数据的块，其数量是块长度，由流控决定。**对于DW_ahb_dmac和内存之间的传输，一个块被直接分解成一个burst/single。对于DW_ahb_dmac和外设之间的传输，一个块被分解成一个事务(single/burst)。这些依次被分解成一系列的AHB转移。**

  - 传输事务：DW_ahb_dmac 传输的基本单位，由硬件或软件握手接口决定。有两种类型的事务：

    - single：single的长度总是1，并转换为单个AHB传输
    - burst：burst的长度被编程到DW_ahb_dmac，通常与DW_ahb_dmac、Source、Destination中的FIFO大小有关。Burst被转换成一连串的burst和single

  - DMA Transfer：软件控制要传输的块数量，一旦DMA传输完成，DW_ahb_dmac内的硬件禁用通道并且可以产生一个中断来表示DMA传输完成。

    - 单块DMA传输：由一个单一块组成
    - 多块DMA传输：DMA传输可能由多个DW_ahb_dmac块组成，通过链表、自动重加载寄存器和连续块，支持多块DMA传输。
      - 链表：链表指针指向系统内存中下一个链表项存在的位置。
        - LLI 访问通常是 32bit 访问(Hsize=2). 对齐到 32bit以及不能修改或编程到任何其它超过 32bit，甚至如果 LLI 的 AHB master 接口支持超过 32bit 数据位宽.
      - Auto-reloading
      - Contiguous blocks

- Scatter：与块内的目标传输有关。当到达一个分散边界时，目的地址会按设定的数量递增或递减。

- Gather：与块内的源传输有关。当到达一个聚集边界时，源地址按设定的数量递增或递减。

- FIFO模式：提高带宽的特殊模式。当启用时，通道等待，直到FIFO小于满的一半，从源外设获取数据，并等待，直到FIFO大于或等于一半，以将数据发送到目标外设。

- 伪飞跃操作：通常，它需要两个AHB总线周期来完成传输--一个用来读取源，一个用于写入目标，然而，当DMA传输的源外设和目标外设位于不同的AHB层时，DW_ahb_dmac在从channel FIFO提取数据并写入目标外设的同时，也可以从源外设取数据并将其存储在channel fifo中。这种操作被称为伪飞跃操作。

- Transfer Width:`CTLx.SRC_TR_WIDTH/CTLx.DST_TR_WIDTH` 
  - DMA 读写数据的位宽，映射到AHB总线的hsize。
  - 如果使用外设的话，则是外设 DR 的宽度
  - 如果使用内存的话则无区别(8/16/32 bits)

- Burst Length：`CTLx.DEST_MSIZE/CTLx.SRC_MSIZE`
  - DMA 一次传输多少数据(单位：byte)，由于DMAC是挂在AHB总线上的，如果需要使用AHB总线，必须要AHB总线仲裁给予权限，每获取一次权限的时候传输多个单位，DMAC先把数据存到FIFO中，如果当前DMAC获取到了AHB总线的授权，且当前FIFO的数据大于传输数据阈值，就会传输Burst Length。
  - 目的地突发事务长度。数据项的个数，每个宽度为CTLx.DST_TR_WIDTH，每次从相应的硬件或软件握手接口发出目的地突发事务请求时，都要写入目的地。

- 流控：流控主要是指谁来控制DMA传输停止，是DMA还是外设。
  - 外设 做流控：要传输的数据项的数目未知并由源或目标外设控制，这些外设通过硬件发出传输结束的信号
  - DMA 做流控：要传输的数据项的数目由软件编程，传输完成后由DMA来发送传输结束的信号 

- DMA 的 MSIZE：DMA 在内存和 CPU 之间进行数据传输时，数据的宽度也称为内存块的大小。它的大小应该适配内存和外设之间数据传输的地址和数据总线宽度。例如，系统总线宽度为32bit，则`MSIZE`应该为32bits
- SPI总线的水线：是指在进行串行数据传输时，为了保证外设和主控的同步，控制传输的速率并防止数据传输错误，SPI总线所需要的数据保持时间。外设准备好足够的数据，达到了水线就会开始传输。
- 在进行DMA传输的时候，需要考虑合适的msize和水线。如果水线太小，则会导致发送数据频率

- CFG.FIFO_EMPTY：表示DMA的通道FIFO为空，DMA无法进行搬运，需要等待数据进入通道FIFO，直到非空时才开始DMA传输



**水线：**

- 当 tx fifo 中的待发数据`向下减少`到水线值时，ssi向dma发起`dma_tx_req`，此时dmac应该往 tx fifo 中注入数据，使得待发数据个数重新达到水线之上。
- 当 rx fifo 中的待收数据`向上增长`到水线值时，ssi向dma发起`dma_rx_req`，此时dmac应该从 rx fifo 中读取数据，使得待收数据个数降到水线之下。



### 地址产生

- 可编程的源地址和目的地址(在AHB总线上)
- 地址递增、递减或不变化
- 通过以下方式实现多块传输：
  - 链表
  - 自动重新加载通道寄存器
  - 块之间的连续地址
- 多块传输类型的独立源、目标选择
- 分散/聚集

### 通道FIFO

- 单个FIFO每个通道的Source 、Destination
- 可配置FIFO depth
- 自动数据包装或拆封适合FIFO宽度

### 通道控制

- 每个通道源或目标可编程
- 每个通道编程传输类型(m2m/m2p/p2m/p2p)
- 每个通道编程burst transaction size
- 编程使能或使能DMA通道
- 支持禁用通道没有数据丢失
- 支持暂停DMA操作
- 可编程的每个通道最大burst size

### 传输初始化

- Source/Destination 的握手接口(up to 16)
  - 硬件握手
  - 软件握手
  - 外设中断握手
- 握手接口支持single/burst
- 硬件握手接口的极性控制
- 启用和禁用单个DMA握手接口

### 流控

- 块传输级别的可编程流控(Source/Destination/DW_ahb_dmac)
- 目的地为流控时源数据预取的软件控制



### 中断

- 混合和单个的中断请求
- 中断产生：
  - DMA传输(多块)完成
  - 块传输完成
  - single/burst完成
  - error
- 支持中断启用和屏蔽





## 功能描述



### Block Flow Controller and Transfer Type

控制块长度的设备称为流量控制器，必须指定`DW_ahb_dmac、Source、Destination`作为流控。

- 如果在通道启用之前块大小是已知的，那么应该将DW_ahb_dmac作为流控。块大小应该被编程到CTLx.BLOCK_TS字段
- 如果在启用DW_ahb_dmac通道时大小未知，则Source/Destination必须是流控

CTLx.TT_FC字段表示该通道的传输类型和流控：

![image-20230228103441794](image/image-20230228103441794.png)

- eg：
  - 当DMA块必须从外设传输到内存是，DW_ahb_dmac可以被编程为流控。在块传输中，软件编写SPI的CTTLR1.NDF字段为要传输的数据项-1，然后配置DMA控制器，对CTLx进行编程，BLOCK_TS使用相同的值注册并将DW_ahb_dmac作为流控
  - 当一个DMA块必须从以太网控制器转移到内存是，外设是一个流控，这种情况下，在启用dma之前，可能不知道以太网数据包的大小。因此，以太网控制器需要能够在块传输完成时发送停止请求到DMA控制器



### 握手接口

握手接口用于传输，用于控制single/burst的流量。握手接口的操作不同，取决于外设还是DMA是流控。

外设使用握手接口向DMA传输已经准备好通过AHB总线传输或接收数据。

非内存外设可以使用两种握手接口之一通过DW_ahb_dmac请求DMA传输：

- 硬件：使用专用的握手接口完成
- 软件：通过内存映射寄存器完成



### 基本接口定义：

- Source single transaction size in bytes
  - src_single_size_bytes = CTLx.SRC_TR_WIDTH/8
- Source burst transaction size in bytes
  - src_burst_size_bytes = CTLx.SRC_MSIZE * src_single_size_bytes
- Destination single transaction size in bytes
  - dst_single_size_bytes = CTLx.DST_TR_WIDTH/8
- Destination burst ransaction size in bytes
  - dst_burst_size_bytes = CTLx.DEST_MSIZE * dst_single_size_bytes
- Block size in bytes:
  - DW_ahb_dmac 是流控，处理器用`CTLx.SRC_TR_WIDTH`来表示传输的数据项数量(块大小)，用`CTLx.BLOCK_TS`来表示要传输的总字节数：
    - block_size_bytes_dma = CTLx.BLOCK_TS * src_single_size_bytes
  - Source 是流控
    - block_size_bytes_src = burst数量 * src_burst_size_bytes + single数量 * src_single_size_bytes
  - Destination 是流控
    - block_size_bytes_dst = burst数量 * dst_burst_size_bytest + single数量 * dst_single_size_bytes



### 软件握手

当外设需要DW_ahb_dmac执行DMA事务时，它通过向CPU或中断控制器发送中断通信请求。中断服务函数中使用在`Software Handshaking Registers`中描述详细的软件寄存器来发起和控制DMA事务。这组寄存器用于实现软件握手接口。

​	`CFGx寄存器中的HS_SEL_SRC/HS_SEL_DST`必须设置为启用软件握手

- 中断后通过`Channel Registers`编程并使能通过
- 通过`Software Handshaking Registers`发起和控制外设和DW_ahb_dmac之间的DMA事务

![image-20230228105716892](image/image-20230228105716892.png)



- 软件握手寄存器：

  - ReqSrcReg：源软件传输请求
  - ReqDstReg：目标软件传输请求
  - SglReqSrcReg：single 源传输请求
  - SglReqDstReg：single 目标传输请求

  - LstSrcReg：最后一个源事务请求
  - LstDstReg：最后一个目的事务请求



### 硬件握手 - 外设不是流控

当外设不是流控时，外设(无论是Src/Dest)和DW_ahb_dmac之间的硬件握手接口：

![image-20230228110254607](image/image-20230228110254607.png)

- dma-acks：输出信号，DW_ahb_dmac接收到外设传输完成信号(single:一直发此信号，burst：周期发)。
- dma_finish：输出，传输完成
- dma_last：输入：由于外设不是流控，此信号会被忽略
- dma_req：输入：来自外设的事务请求。
- dma_single：输入：single transfer状态。外设可以接受至少一个数据项时产生。



## Example

### Example1：block transfer when the DW_ahb_dmac is the flow controller



| Parameter                     | Description                             |
| ----------------------------- | --------------------------------------- |
| CTLx.TT_FC = 3'b 011          | DW_ahb_dmac 作为流控，p2p 传输          |
| CTLx.BLOCK_TS = 12            | 要传输的数量                            |
| CTLx.SRC_TR_WIDTH = 3'b 010   | 32bit                                   |
| CTLx.DST_TR_WIDTH = 3'b 010   | 32bit                                   |
| CTLx.SRC_MSIZE = 3'b 001      | Source burst transaction length = 4     |
| CTLx.DEST_MSIZE = 3'b 001     | Destination burst transacton length = 4 |
| CTLx.MAX_ABRST = 1'b 0        | No limit on maximum AMBA burst length   |
| DMAH_CHx_FIFO_DEPTH = 16 byte |                                         |





## 外设 Burst Transaction Requests

- 对于 Source FIFO，当 Source FIFO 超过水线时，在`dma_req`上触发一次有效边沿。

- 对于 Destination FIFO，当 Destination FIFO 低于水线时，在`dma_req`上触发一次有效边沿。

​	本节研究水线在Source 和 Destination 上的最佳设置，以及与 Burst Size 的搭配：

- Source Transfer Length：CTLx.SRC_MSIZE
- Destination Transfer Length:CTLx.DEST_MSIZE

接收端 apb_ssi SPI作为源，发送端 apb_ssi SPI作为目标。



### 传输水线和传输FIFO下溢

在SPI串行传输器件，当SPI传输FIFO数量小于或等于传输`SSI.DMATDLR`值时，就像DW_ahb_dmac发送传输FIFO请求。DW_ahb_dmac通过写入SPI的TXFIFO中写入burst数据来响应，长度是`DMA.CTLx.DEST_MSIZE`

数据应该经常从DW_ahb_dmac中获取，以便SPI的TXFIFO来连续执行串行传输。也就是说，当SPI的TXFIFO开始清空时，应该会触发另一个burst事务请求。否则，SPI传输FIFO下溢。



### 选择发送水线

假设：`DMA.CTLx.DEST_MSIZE = SSI_TX_FIFO_DEPTH - SSI.DMATDLR`

> SSI_TX_FIFO_DEPTH 为 SPI TX FIFO的深度。
> DMATDLR控制SPI 传输逻辑发送 DW_ahb_dmac 目标burst transfer的级别，它等于水线。
> 即当DW_ahb_dmac的TXFIFO有效数据等于或小于这个值时，产生Destination Burst 请求。





## 中断寄存器

- IntBlock：Block 传输完成中断
  - DMA块传输完成到目标外设时产生中断
- IntDstTran：目标传输完成中断
  - 目标端的握手接口(硬件/软件)完成请求的single/burst的最后一次AHB传输后产生
- IntErr：错误中断
  - 当一个错误相应在DMA转移器件从HRESP总线上的一个AHB Slave被接收时，这个中断产生。
- IntSrcTran：源传输完成中断
  - 这个中断是在源端捂手接口(硬件/软件)完成请求的single/burst的最后一次AHB传输后产生
- IntTfr：DMA传输完成中断
  - DMA传输完成到目标外设上产生
- 与中断相关的寄存器：
  - RawBlock、RawDstTran、RawErr、RawSrcTran、RawTfr
  - StatusBlock、。。。。。。

当通道已经使能并产生中断时，以下情况为True：

- 中断事件存储在Raw状态寄存器
- Raw状态寄存器被Mask寄存器中的内容Mask
- masked中断存储在Status寄存器



### DW_ahb_dmac Transfer Type

​	dma传输包含单个或多个block传输。SARx/DARx寄存器可以通过下面的方式重新写入：

- 使用链表的方式

- 自动加载

- 连续地址

  在多块传输的的连续块上，`DW_ahb_dmac`中的`CTLx`寄存器使用一下方法之一重新编程：

- 使用链表的块链

- 自动装载

  当使用块链时，在连续的块上，`DW_ahb_dmac`中的`LLPx`寄存器使用带链表的块链重新编程。

  块描述符由六个寄存器组成：`SARx、DARx、LLPx、CTLx、SSTATX、DSTATx`,`DW_ahb_dmac`使用前四个寄存器和`CFGx`寄存器来设置和描述块传输。



### Multi-Block Transfer

通过将`DMAH_CHX_MULTI_BLK_EN`配置为`True`, 可以启用多块传输。



### 链表

要使用链表传输Multi-Block，必须配置参数`DMAH_CHx_MULTI_BLK_EN`为`True`，将`DMAH_CHX_HC_LLP`参数设置为`False`.

此情况下，控制器通过从系统内存中获取该块的快描述符，在每个块开始之前重新编程通道寄存器。这就是所谓的LLI更新。

`DW_ahb_dmac`块链使用一个链表指针寄存器(LLPx)，它将地址存储在下一个链表项的内存中。每个LLI包含相应的块描述符：

1. SARx
2. DARx
3. LLPx
4. CTLx
5. SSTATx
6. DSTATx

要建立块链，需要在内存中编写链表序列。

LLI访问始终是与32位字节对齐(Hsize=2)，并且不能更改或编程为32位意外的任何东西，即使LLI的AHB主接口支持超过32位的访问数据宽度。

#### 使用链表

#### 使用链表

#### 使用链表

SARx、DARx、LLPx和CTLx寄存器在LLI更新时从系统内存中提取。如果配置参数`DMAH_CHx_CTL_WB_EN=True`，则在块完成时将`CTLx、SSTATx、DSTATx`寄存器的更新内容写会内存，下图展示如何使用块链在内存中使用链表来定义多块传输：

![image-20230224094829834](image/image-20230224094829834.png)

当配置参数`DMAH_CHx_STAT_SRC`设置为false时，假设没有为源状态分配内存，如果该参数为False，则链表项的顺序如下：

1. SARx
2. DARx
3. LLPx
4. CTLx
5. DSTATx

![image-20230224094844730](image/image-20230224094844730.png)





### 配置要点

1. SOC 中包含几个 DMA IP，每个 IP 有几个 通道，每个 DMA 可以访问什么(外设、内存)
2. DMA 数据配置，包括数据从哪里来，要到哪里去，传输的数据的单位，要传输多少数据，普通还是链表？
   - 方向：M2P、P2M、M2M 等
   - 单位：源和目标存储的数据宽度需要保持一致，例如某个外设的 DR 是 8bit，则传输的数据宽度也要是 8bit
   - 数量：数据什么时候传输完成，可以通过查询标志位或者通过中断的方式来鉴别
   - 普通/链表：









### 块流控和传输类型

`CTLx.TT_FC`字段表示传输类型和流控对应方式：

| CTLx.TT_FC | Transfer Type |    Flow Controller     |
| :--------: | :-----------: | :--------------------: |
|    000     |      M2M      |      DW_ahb_dmac       |
|    001     |      M2P      |      DW_ahb_dmac       |
|    010     |      P2M      |      DW_ahb_dmac       |
|    011     |      P2P      |      DW_ahb_dmac       |
|    100     |      P2M      |       Peripheral       |
|    101     |      P2P      |   Source Peripheral    |
|    110     |      M2P      |       Peripheral       |
|    111     |      P2P      | Destination Peripheral |

- 外设作为 Source or Destination ，使用 Peripheral.DR 作为dma 的地址







## 寄存器

### DMAC/Channel_x_Registers

- SARx:通道x的源地址寄存器

  - 地址偏移：0x0 + x*0x58

  | 31:0 | SAR  | R/W  | DMA 传输的当前源地址 |
  | ---- | ---- | ---- | -------------------- |

  

- DARx：通道x的目标地址寄存器
  - 地址偏移：0x8 + x*0x58

| 31:0 | DAR  | R/W  | DMA 传输的当前目标地址 |
| ---- | ---- | ---- | ---------------------- |



- LLPx：通道x的链表寄存器
  - 地址偏移:0x10 + x*0x58

| 31:2 | LOC  | R/W  | 如果Block使能，保存内存中的下一个LLI的起始地址。此寄存器的最低两位不会被存储在当前字段。 |
| ---- | ---- | ---- | ------------------------------------------------------------ |
| 1：0 | LMS  | R/W  | List Master 选择。<br />0x0：内存设备存储下一个链表节点在AHB Master 1<br />0x1：AHB Master 2<br />0x2：AHB Master 3<br />0x3：AHB Master 4 |



- CTLx：通道x的控制寄存器
  - 地址偏移：0x18 + x*0x58

| 44     | DONE           | R/W  | 完成位。软件可以轮询LLI.CTLx.CONE位，查看块传输何时完成LLI.CTLx在启用通道之前，当在内存中建立链表时，DONE位应该被清除<br />0x0：块传输完成时DONE位被取消断言<br />0x1：块传输完成时设置DONE位 |
| ------ | -------------- | ---- | ------------------------------------------------------------ |
| x:32   | BLOCK_TS       | R/W  | 块传输大小,当dmac为流控时，用户需要在通道使能前写入该字段以指示块大小。编程到此字段的数字表示每个块传输要执行的单个事务的总数。宽度有SRC_TR_WIDTH决定。一旦传输开始，回读值就是已从源外设读取的数据项总数。|
| 28     | LLP_SRC_EN     | R/W  | 0x0:Source 端块不启用链表传输<br />0x1：Source 端启用链表传输 |
| 27     | LLP_DST_EN     | R/W  | 0x0：Destination 端不启用链表传输<br />0x1：Destination 端启用链表传输 |
| 26：25 | SMS            | R/W  | Source Master 选择。<br />0x0：Master 1<br />0x1：Master 2<br />0x2：Master 3<br />0x3：Master 4 |
| 24：23 | DMS            | R/W  | Destination Master 选择。<br />0x0：Master 1<br />0x1：Master 2<br />0x2：Master 3<br />0x3：Master 4 |
| 22:20  | TT_FC          | R/W  | 传输类型和流控。<br />0x0：M2M，DW_ahb_dmac<br />0x1：M2P，DW_ahb_dmac<br />0x2：P2M，DW_ahb_dmac<br />0x3：P2P，DW_ahb_dmac<br />0x4：P2M，Peripheral<br />0x5：P2P，Source Peripheral<br />0x6：M2P，Peripheral<br />0x7：P2P，Source Peripheral |
| 18     | DST_SCATTER_EN | R/W  | 目标分散使能。只有当CTLx.DINC位是递增或递减地址控制。<br />0x0：目标分散失能<br />0x1：目标分散使能 |
| 17     | SRC_GATHER_EN  | R/W  | 源聚集使能。只有当CTLx.DINC位是递增或递减地址控制。<br />0x0：目标聚集失能<br />0x1：目标聚集使能 |
| 16：14 | SRC_MSIZE      | R/W  | 源 Burst 传输长度。每次传输的宽度是CTLx.SRC_TR_WIDTH。<br />0x0：传输数据量为1<br />0x1：传输数据量为4<br />0x2：传输数据量为8<br />0x3：传输数据量为16<br />0x4：传输数据量为32<br />0x5：传输数据量为64<br />0x6：传输数据量为128<br />0x7：传输数据量为256 |
| 13：11 | DEST_MSIZE     | R/W  | 目标 Burst 传输长度。每次传输的宽度是CTLx.DST_TR_WIDTH。<br />0x0：传输数据量为1<br />0x1：传输数据量为4<br />0x2：传输数据量为8<br />0x3：传输数据量为16<br />0x4：传输数据量为32<br />0x5：传输数据量为64<br />0x6：传输数据量为128<br />0x7：传输数据量为256 |
| 10：9  | SINC           | R/W  | 源地址递增。<br />0x0：源地址递增<br />0x1：源地址递减<br />0x2：源地址不改变<br />0x3：源地址不改变 |
| 8：7   | DINC           | R/W  | 目的地址递增。<br />0x0：目的地址递增<br />0x1：目的地址递减<br />0x2：目的地址不改变<br />0x3：目的地址不改变 |
| 6：4   | SRC_TR_WIDTH   | R/W  | 源传输宽度。映射到AHB 总线的 HSIZE。对于非内存的外设，典型值是外设的FIFO宽度。<br />0x0：源传输宽度是8bits<br />0x1：源传输宽度是16bits<br />0x2：源传输宽度是32bits<br />0x3：源传输宽度是64bits<br />0x4：源传输宽度是128bits<br />0x5：源传输宽度是256bits<br />0x6：源传输宽度是256bits<br />0x7：源传输宽度是256bits |
| 3:1    | DST_TR_WIDTH   | R/W  | 目标传输宽度。映射到AHB总线的HSIZE。对于非内存的外设，典型值是外设的FIFO宽度。<br />0x0：目标传输宽度是8bits<br />0x1：目标传输宽度是16bits<br />0x2：目标传输宽度是32bits<br />0x3：目标传输宽度是64bits<br />0x4：目标传输宽度是128bits<br />0x5：目标传输宽度是256bits<br />0x6：目标传输宽度是256bits<br />0x7：目标传输宽度是256bits |
| 0      | INT_EN         | R/W  | 中断使能位。如果置一，则所有的中断产生源被使能。0x0：中断失能<br />0x1：中断使能 |



- SSTATx:通道x的源状态寄存器

| 31:0 | SSTAT | R/W  | 由硬件从STATRx寄存器内容所指向的地址检索的源状态信息 |
| ---- | ----- | ---- | ---------------------------------------------------- |



- DSTATx：通道x的目标状态寄存器

| 31:0 | DSTAT | R/W  | 由硬件从DTATRx寄存器内容所指向的地址检索的源状态信息 |
| ---- | ----- | ---- | ---------------------------------------------------- |



- CFGx：通道x的配置寄存器

| x：43  | DEST_PER   | R/W  | 目标硬件握手。如果CFGx.HS_SEL_DST为0，将一个硬件握手接口(0:DMAH_NUM_HS_INT - 1)分配给通道x的目标。否则被忽略。然后通道可以通过指定的硬件握手接口与连接到该接口的目标外设通信。<br />note1:对于正确的DW_ahb_dmac操作，应该只将一个外设(Src/Dst)分配给同一个握手接口<br />note2：如果配置参数DMAH_NUM_HS_INT = 0，该字段不存在 |
| ------ | ---------- | ---- | ------------------------------------------------------------ |
| x:39   | SRC_PER    | R/W  | 源硬件握手。如果CFGx.HS_SEL_SRC为0，将一个硬件握手接口(0:DMAH_NUM_HS_INT - 1)分配给通道x的源。否则被忽略。然后通道可以通过指定的硬件握手接口与连接到该接口的源外设通信。<br />note1:对于正确的DW_ahb_dmac操作，应该只将一个外设(Src/Dst)分配给同一个握手接口<br />note2：如果配置参数DMAH_NUM_HS_INT = 0，该字段不存在 |
| 38     | SS_UPD_EN  | R/W  | 源状态更新使能                                               |
| 37     | DS_UPD_EN  | R/W  | 目标状态更新使能                                             |
| 36：34 | PROTCTL    | R/W  | 保护控制位用于驱动AHB HPROT[3:1]总线。                       |
| 33     | FIFO_MODE  | R/W  | FIFO Mode Select.决定burst传输开始之前FIFO中需要有多少空间或数据有效<br />0x0：<br />0x1：FIFO中有效的数据大于或等于一半 |
| 32     | FCMODE     | R/W  | 流控模式                                                     |
| 31     | RELOAD_DST | R/W  | 自动目标重载。0x0：失能 0x1：使能                            |
| 30     | RELOAD_SRC | R/W  | 自动源重载。 0x0：失能 0x1：使能                             |
| 29:20  | MAX_ABRST  | R/W  | 最大AMBA Burst长度。                                         |
| 19     | SRC_HS_POL | R/W  | 源握手接口极性。0x0：high  0x1：low                          |
| 18     | DST_HS_POL | R/W  | 目标握手接口极性。0x0: high 0x1:low                          |
| 17     | LOCK_B     | R/W  | 总线锁。0x0：Bus 锁失能。 0x1：Bus 锁使能                    |
| 16     | LOCK_CH    | R/W  | 通道锁。0x0：Channel 锁失能。 0x1：Channel锁使能             |
| 15：14 | LOCK_B_L   | R/W  | 总线锁等级                                                   |
| 13：12 | LOCK_CH_L  | R/W  | 通道锁等级                                                   |
| 11     | HS_SEL_SRC | R/W  | 源软件或硬件握手信号选择。0x0：硬件  0x1:软件                |
| 10     | HS_SEL_DST | R/W  | 目标软件或硬件握手信号选择。0x0：硬件 0x1：软件              |
| 9      | FIFO_EMPTY | R    | 通道FIFO状态。0x0：通道非空  0x1：通道空                     |
| 8      | CH_SUSP    | R/W  | 通道 Suspend                                                 |
| 7:5    | CH_PRIOR   | R/W  | 通道优先级。优先级7最高，0最低。0x0：0  0x7:7                |



- SGRx：通道x的源聚集寄存器
- DSRx：通道x的目标聚集寄存器



### DMAC/Interrupt Registers

- Raw Tfr:原始状态的传输中断

| x:0  | RAW  | R/W  | 原始状态的传输中断。0x0：未激活 0x1：激活 |
| ---- | ---- | ---- | ----------------------------------------- |





### DMAC/Miscellaneous Registers

- DmaCfgReg：DW_ahb_dmac 配置寄存器

| 0    | DMA_EN | R/W  | DW_ahb_dmac 使能位。0x0：失能 0x1：使能 |
| ---- | ------ | ---- | --------------------------------------- |



- ChEnReg：DW_ahb_dmac 通道使能寄存器

| x:8  | CH_EN_WE | W    | 通道使能寄存器                 |
| ---- | -------- | ---- | ------------------------------ |
| x:0  | CH_EN    | R/W  | 通道使能。0x0：失能  0x1：使能 |



- DmaIdReg：DW_ahb_dmac ID 寄存器

| 31:0 | DMA_ID | R    | DW_ahb_dmac 外设ID 的硬件码 |
| ---- | ------ | ---- | --------------------------- |



- DMA_COMP_PARAMS_7:通道7的参数状态寄存器，read-only

![image-20230228154238756](image/image-20230228154238756.png)




h1. dma 的使用和工作原理

h2. 寄存器配置

* dma 有几个关键寄存器，ctrl,cfg,LLP
* TR_WIDTH: 代表dma读写数据以多少位宽来操作
* MSIZE：代表dma一次传输(burst)多少个位宽单元
* BLOCK_TS: 代表总共需要传输多少个位宽单元
* INC: 配置地址递增、递减、不变
* TT_FC: 配置流控，可选MEM_TO_MEM_DMA, MEM_TO_PER_DMA, PER_TO_MEM_DMA, PER_TO_PER_DMA, PER_TO_MEM_SRC, PER_TO_PER_SRC, MEM_TO_PER_DST, PER_TO_PER_DST
* MS: 配置使用的dma master
* LLP_EN: 配置是否使用链表
* PER: 硬件握手信号值
* HS_SEL: 配置握手信号，有硬件握手和软件握手，当配置成软件握手的时候，硬件握手信号的请求无效

h2. 链表配置

* 链表节点结构:
<pre>
struct dw_lli {
	/* values that are not changed by hardware */
	uint32_t  sar;
	uint32_t  dar;
	uint32_t  llp;      // chain to next lli
	uint32_t  ctl[2];
	uint32_t  dstat;
};
</pre>
* 链表一个节点最多传送4095个单位的外设fifo的位宽
* 使用链表的时候需要将链表的首节点起始地址赋值给dma的LLP寄存器，并在dma ctrl寄存器里使能接收和发送链表
* 最后一个节点的llp成员赋值NULL
* 配置好以后dma会去读取链表里的原地址和目的地址以及dma 的ctl配置

h2. dma和外设握手信号

* dma 和 外设之间有三个信号线比较重要: dma_single dma_req dma_ack
* dma 在配置了传输数量的情况下，dma可以当作流控
* 外设的dma对接时,外设有几个跟dma相关的配置比较重要：TX_DMA_WATER_LEVEL, RX_DMA_WATER_LEVEL, RX_DMA_EN, TX_DMA_EN

h2. dma和外设工作逻辑

* 配置好外设的TX_DMA_WATER_LEVEL, RX_DMA_WATER_LEVEL, RX_DMA_EN, TX_DMA_EN
* 配置流程，配置外设，配置dma，使能dma，使能外设，不能先使能外设再使能dma，会有隐患使能外设后，外设开始工作，dma还没工作导致外设fifo满溢出或者空
* 外设发送模式下，当外设的发送fifo不满时会发送dma_single信号，当外设的发送fifo数量小于等于TX_DMA_WATER_LEVEL的fifo位宽的时候会发送dma_req信号，dma 作为流控需要发送的数据大于MSIZE的外设fifo位宽时，响应dma_req信号，传送一次burst，并发送应答信号dma_ack, 反之响应dma_single信号，传送一个单位外设fifo位宽.
* 外设接收模式下，当外设的接收fifo不空时会发送dma_single信号，当外设的接收fifo数量大于等于RX_DMA_WATER_LEVEL的fifo位宽的时候会发送dma_req信号，dma 作为流控需要接收的数据大于MSIZE的外设fifo位宽时，响应dma_req信号，传送一次burst，并发送应答信号dma_ack, 反之响应dma_single信号，传送一个单位外设fifo位宽.

h2. 使用场景

h3. 内存拷贝

* TR_WIDTH: 配置成8bit, 16bit, 32bit都可以
* MSIZE：配置成1,2,4,8,16,32都可以
* BLOCK_TS: 使用LLP的话可不配置
* INC: 递增
* TT_FC: MEM_TO_MEM_DMA
* MS: 配置成能访问内存的dma master
* LLP_EN: 使用链表
* PER: 无效
* HS_SEL: 软件握手

h3. 内存到设备

* TR_WIDTH: 配置成设备发送寄存器的位宽
* MSIZE：配置成设备fifo大小的一半
* BLOCK_TS: 使用LLP的话可不配置
* INC: 源地址递增、目的地址不变
* TT_FC: MEM_TO_PER_DMA
* MS: SMS 配置成访问内存的dma master, DMS配置成访问设备的dma master
* LLP_EN: 使用链表
* PER: 硬件握手信号值
* HS_SEL: 硬件握手

h3. 设备到内存

* TR_WIDTH: 配置成设备接收寄存器的位宽
* MSIZE：配置成设备fifo大小的一半
* BLOCK_TS: 使用LLP的话可不配置
* INC: 源地址不变、目的地址递增
* TT_FC: PER_TO_MEM_DMA
* MS: SMS配置成访问设备的dma master, DMS配置成访问内存的dma master
* LLP_EN: 使用链表
* PER: 硬件握手信号值
* HS_SEL: 硬件握手


# Program Example

## Linked List Multi-Block Transfer

- 4 个 blocks 从目标到外设连续传输

1. 配置传输类型：m2m
	- ctlx[22:20] = 000
2. 配置传输特征
	- src 传输位宽:STC_TR_WIDTH，dst 传输位宽:DST_TR_WIDTH
		- ctlx[6:4] = 001; ctlx[3:1] = 001
	- src master 选择:SMS，dst master 选择:DMS
		- ctlx[26:25] = 00; ctlx[24:23] = 00
	- src 地址递增:SINC，dst 地址递增:DINC
		- ctlx[10:9] = 00; ctlx[8:7] = 00



reached：到达，达到
sit:坐，栖息
perform：执行
consider：考虑
near：附近
demonstrates:证明，演示
presented:提出，出席，介绍
so far：目前为止
exceeds：超过、超出、领先
drops:下降、滴
during:在...期间

源外设，不知道 block 的长度，只能够去请求 burst 传输。坐着等待一个新的 burt 传输请求之前 fifo 级别到达水线级别    
这个区域，源 block 要传输的数据总数小于 src_burst_size_bytes , 作为 single transaction region
在single 传输区域，dmac从源外设执行 single 传输直到源 block 传输完成。在这个例子，dma 用四次single传输从源完成源block传输

新考虑 caseb.源外设可以动态调整水线，在block 结尾附近触发一次burst 传输请求.
稍后第一个源 burst 传输完成，源外设 recognizes that 它只有四个数据要传输在 block 和调整fifo水线触发burst请求为4.
触发一次burst请求，使用一次burst 传输完成 block。
然而，MSIZE=8，他们是只有四个数据项将要传输在源 block。dmac 终端 提前终止burst

目标位设进去 single 传输区域当 dmac 作为流控。这个例子说明如何 dmac 送到 通道 fifo flushed 在block传输到目标结束.
考虑这个 case 参见参数设置的值列表
要传 44 字节，源 burst 4，目标burst 64

block传输到目标结尾附近,要传输的数据总量小于 burst 并且目标进入single
展示一种方法 当block传输到目标可以发生。

in all the examples presented so far.
