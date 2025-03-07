# AXI_DMA

DW_axi_dmac 是一个高度可配置、高度可编程、高性能、多主多通道的DMA控制器，以 AXI 作为数据传输的总线接口。

# 1. 特性

- 独立的内核，从机接口和主机接口时钟
- 关闭从机时钟
  - 当 `slvif_busy` 输出是 `de-asserted`时，主机可以关闭从机时钟
  - 尝试再次访问从机接口时，主机必须重启时钟
- 当没有外设处于活动状态时，独立关闭主机接口时钟
- 高达 8 个通道，每个通道都有一对源和目标，可以在`coreConsultant` 配置
- 数据传输仅在单个方向
- 高达两个 AXI 主机接口，`coreConsultant` 中可配置
  - 两个 master 接口 支持 multilayer
  - 支持不同 ACLK 在不同 AMBA 层
- M2M、M2P、P2M、P2P DMA 传输
- AXI 主机数据总线位宽高达 512bits(每个 AXI 主机接口)，`coreConsultant` 中可配置
- AXI 主机接口小端模式可以选择静态或动态，`coreConsultant` 中可配置
- 用于动态选择字节序方案的输入引脚
- 可选的辨识寄存器，`coreConsultant` 中可配置
- 支持通道锁定
- DMAC 指示输出状态
  - `Idle/busy`
- DMA 保持 功能

- 多层次的传输层次接口。
  - dma 传输分为传输block 和 完成 DMA 传输层次

- 支持 AXI 不对齐传输
- 支持上下文敏感的低功耗选项



- 通道 buffer
  - 每个通道单个 FIFO
  - FIFO 深度，`coreConsultant` 中可配置
  - 自动对数据进行打包/解包，以适应 FIFO 宽度



- 通道控制
  - 每个通道可编程的传输类型(M2M、M2P、P2M、P2P)
  - 单个或多个 DMA 传输
  - 每个通道可配置的多个传输大小
  - 每个通道可配置的最大 AMBA burst 传输大小
  - 关闭通道的情况下不会丢失数据
  - 通道 suspend and resume
  - 可编程的通道优先级
  - 在不同的传输层次上对主总线接口的内部通道仲裁进行锁定
  - 编程多个block传输使用 `linked list`，自动重载，连续地址，和影子寄存器方式
  - 链表的动态扩展
  - 独立配置 `SRC/DST` 多个 block 传输类型
  - 数据和 LLI 访问的分离状态机
  - 控制信号，例如 cache 和 保护、编程每个 DMA block
  - 可编程的传输长度(block length)
  - 错误状态寄存器，以便在错误事件期间进行调试



- 流控
  - 在dma 传输等级可编程的流控
    - 如果在DMA 初始化的时候已知要传输的长度，那么 DMA 控制器在传输等级中作为流控
    - 如果在 DMA 初始化的时候不知道要传输的长度，未定义的长度传输中，源或目标可以当做流控



- 握手接口
  - 非内存外设中可编程的软件和硬件握手接口
  - 高达 16 个硬件握手接口/外设，`coreConsultant` 中可配置
  - 使能、使能 独立的握手接口
  - 可编程的映射外设和通道，多对一映射，每次只有一个外设处于激活状态
  - 内存映射寄存器控制DMA传输在软件握手模式



- 中断输出
  - 混合的和独立的中断输出
  - 中断产生：
    - DMA 传输完成
    - block 传输完成
    - 单个或多个传输完成
    - 错误产生
    - 通道 suspend / disable
  - 中断使能和屏蔽



- 总线接口
  - master 接口数据总线位宽高达 512bits，`coreConsultant` 中可配置
  - master 接口上尚未传输完成的数据
  - master 接口设置每个通道尚未传输完成的数据限制
  - 配置 AXI 传输位宽
  - 在同一主控接口上支持不同通道的乱序事务；特定通道的事务始终按顺序初始化
  - master 接口自增和固定地址传输
  - 源和目标数据传输地址，必须按照相应的传输宽度对齐
  - 从机数据总线位宽 32/64 bits，`coreConsultant` 中可配置
  - 传输大小用于从机接口，必须和数据总线位宽相同



- 名词

  - 源外设 – AXI 层上的设备，DW_axi_dmac 从中读取数据。然后 DW_axi_dmac 将数据存储在通道 FIFO 中。源外设与目标外设一起形成通道。
  - 目标外设 – DW_axi_dmac 将 FIFO 中存储的数据写入到的设备（数据之前从源外设读取）。
  - 内存 – 始终准备好进行 DMA 传输且不需要握手接口即可与 DW_axi_dmac 交互的源或目标。
  - 通道 – 一个已配置 AXI 层上的源外设与同一或不同 AXI 层上的目标外设之间的读/写数据路径，通过通道 FIFO 发生。如果源外设不是存储器，则将源握手接口分配给该通道。如果目标外设不是存储器，则将目标握手接口分配给该通道。可以通过对通道寄存器进行编程来动态分配源和目标握手接口。
  - 握手接口 – 一组符合协议的信号或软件寄存器，用于在 DW_axi_dmac 与源或目标外设之间执行握手。握手有助于控制 DW_axi_dmac 和外设之间的单个或突发事务的传输。该接口用于请求、确认和控制 DW_axi_dmac 事务。通道可以通过两种类型的握手接口之一接收请求：硬件或软件。
  
    - 硬件握手接口 – 使用硬件信号控制 DW_axi_dmac 与源或目标外设之间的单个或突发事务传输。硬件握手接口的简单用途是当来自外设的中断线连接到硬件握手接口的“dma_req”输入时；其他接口信号将被忽略。
    - 软件握手接口 – 使用软件寄存器控制 DW_axi_dmac 与源或目标外设之间的单个或突发事务传输。在此模式下，外设的 I/O 上不需要特殊的 DW_axi_dmac 握手信号。此模式对于将现有外设连接到 DW_axi_dmac 而无需对其进行修改非常有用。
  - 流控制器 – 确定 DMA 块传输长度并终止它的设备（DW_axi_dmac，或者源或目标外设）。如果在启用通道之前已知块的长度，则应将 DW_axi_dmac 编程为流量控制器。如果在启用通道之前不知道块的长度，则源或目标外设应终止块传输。在此模式下，外设（源/目标）是流量控制器。
  
  - 
    传输层次结构 – 传输最多分为四个级别：DMA 传输级别、块传输级别、传输级别`(single/burst)`和 AXI 传输`(single/burst)`级别。这样做是为了最大限度地减少通道被授予一组特定外设，但外设没有足够的数据来连续传输的情况的影响。在这种情况下，通道不能提供给任何其他外设，这会降低性能。
  
    图 1-3 说明了非内存外设的 DMA 传输、块传输、事务（单次或突发）和 AMBA AXI 传输（单次或突发）之间的层次结构。
  
  ![选区_482](image/选区_482.png)
  
  
  
  
  
  - Transaction：DW_axi_dmac 传输的基本单位，由硬件或软件握手接口确定。如果外设是非内存设备，则传输仅与 DW_axi_dmac 和源或目标外设之间的传输相关。有两种类型的传输方式：
    - Single transaction：single transaction 长度始终为 1，并且转换为 burst 长度为 1 的 INCR AXI 传输
    - Burst transaction：burst transaction 的长度被变成到 DW_axi_dmac 中。一个 Burst transaction 转换为 AXI burst transfer。Burst transaction 的长度由编程控制，通常与 DW_axi_dmac、Source、Destination 外设中的 FIFO 大小有关。
  - Block：DW_axi_dmac 数据块，其数量为块长度，由流控确定。对于 DW_axi_dmac 和内存之间的传输，块被直接分解为 burst 传输序列。对于 DW_axi_dmac 和非内存外设之间的传输，一个 block 被分成一系列 DW_axi_dmac 传输。这些又被分解成一系列 AXI 传输。
  
  - DMA 传输：软件控制有多少个 block 要传输。一旦 DMA 传输完成，DW_axi_dmac 内的硬件就会禁用通道，并可以生成中断以指示 DMA 传输完成，然后可以对通道重新编程以进行新的 DMA 传输。
    - single-block DMA 传输：单个 block 组成
    - Multi-Block DMA 传输：DMA 传输由多个 DMA 块组成。通过链表指针、通道寄存器、影子寄存器和连续块的自动重新状态来支持多块 DMA 传输，源和目的地可以独立选择使用哪种方法。
      - Linked lists：链表指针指向系统中下一个链表项(LLI)的内存地址。LLI 包含一组描述下一个块的寄存器和一个 LLP 寄存器。当启用块链接时，DW_axi_dmac 在每个块的开头获取 LLI 结构。LLI 访问时钟使用与数据总线宽度相同的 burst 大小(arsize/awsize)，并且不能更改或编程为除此之外的任何类容。burst 长度(awlen/arlen)根据数据总线宽度进行选择，以便访问不会跨越 64 字节的一个完整 LLI 结构。DW_axi_dmac 在一次 AXI burst 中获取整个 LLI(40 字节)，受限其他设置。
      - Auto-reloading：DW_axi_dmac 自动将每个块末尾的通道寄存器重新加载为首次启用通道时设置的值
      - contiguous register：连续块之间的地址被选择为前一个块末尾的延续。
      - 影子寄存器：DW_axi_dmac 自动从每个块末尾设置的影子寄存器的内容加载通道寄存器。当前块传输正在进行时，软件可以使用与下一个块传输时对应的值对影子寄存器进行编程
    - 通道锁定：软件可以对通道进行编程，通过在 DMA 传输、block 传输或  transaction(single/burst) 期间锁定主总线接口的仲裁来保持 AXI 主接口。



# 2. 功能



## 2.1 数据流

![选区_299](image/选区_299.png)



## 2.2 single transaction region

在某些情况下，仅使用突发事务无法完成 DMA 块传输。通常，当块大小不是突发事务长度的倍数时，就会发生这种情况。在这些情况下，块传输使用突发事务，直到完成块的剩余数据量小于突发事务中的数据量。此时，DW_axi_dmac 对 `dma_single` 状态标志进行采样，并使用单个事务完成块传输。外设断言单个状态标志，以向 DW_axi_dmac 指示有足够的数据或空间来完成来自或到源或目标外设的单个事务。

- 源 single transaction size in bytes：
  - `src_single_size_bytes = CHx_CTL.SRC_TR_WIDTH / 8`
- 源 burst transaction size in bytes：
  - `src_burst_size_bytes = CHx_CTL.SRC_MSIZE * src_single_size_bytes`

- 目标 single transaction size in bytes:

  - `dst_single_size_bytes = CHx_CTL.DST_TR_WIDTH / 8`

- 目标 burst transaction size in bytes:

  - `dst_burst_size_bytes = CHx_CTL.DST_MSIZE * dst_single_size_bytes`

- Block size in bytes:

  - 如果 `DW_axi_dmac` 是流控，处理器会将要传输的数据项数据(块大小)设置到DMA 控制器中。
    - `CHx_CTL.SRC_TR_WIDTH`：指定了源传输的宽度，即每个数据项的字节数
    - `CHx_BLOCK_TS.BLOCK_TS`：指定了要传输的数据项数目
    - 因此，一个块中要传输的总字节数计算公式为：
      - `block_size_bytes_dma = CHx_BLOCK_TS.BLOCK_TS * src_single_size_bytes`

  - 如果外设是流控
    - `block_size_bytes_src = burst 次数 * src_burst_size_bytes + single 次数 * src_single_size_bytes`
  - 如果目标外设是 block 流控
    - `blk_size_bytes_dst = burst 次数 * dst_burst_size_bytest + single times * dst_single_size_bytes`



- 何时进入 single 传输模式：(仅适用于不是流控的外围设备。进入该区域的具体定于取决于谁是流控)

  - 如果 `DW_axi_dmac` 是流控：
    - 当 src block 传输的待完成字节数小于 `src_burst_size_bytes` 时，源外设进入  `single transaction region`
      - 如果 `blk_size_bytes_dma/src_burst_size_bytes` 是整数，则源外设永远不会进入此区域，并且源 block 传输仅使用 `burst transaction`
    - 当 dst block 传输的待完成字节数小于 `dst_burst_size_bytes` 时，目标外设进入 `single transaction regino`
      - 如果 `blk_size_bytes_dma/dst_burst_size_bytes` 是整数，则目标外设永远不会进入此区域，并且目标 block 传输仅使用 `burst transaction`

  - 如果源外设或目标外设是流控：<font color=red>(用不到，都是 dw_dmac 做流控)</font>
    - `single transaction region` 不适用于外设做流控，也就不会进入 `single transaction region`
    - 如果源外设是流控，...





## 2.3 握手接口

握手接口用于`transaction level`来控制`single/burst transaction`。握手接口的操作取决于外设还是 `DW_axi_dmac` 是流控。外设使用握手接口向 `DW_axi_dmac` 指示它已准备好通过 AXI 总线传输或接收数据。

- 硬件握手：
  - dma_req ：来自外设的 `burst transaction` 请求输入。该信号的功能取决于外设是否是流控。
    - 如果外设不是流控：`DW_axi_dmac` 始终将 `dma_req` 信号解释为 `burst transaction` 请求，无论 `dma_single` 的级别如何。一旦 `dma_req` 被置位，它必须保持置位状态，直到 dma_ack 被置位。当驱动 dma_req 的外设确定 dma_ack 已置位时，它必须取消 dma_req 置位。如果在单个事务区域中检测到 dma_req 上的活动级别，则使用提前终止的突发事务来完成块传输。
    - 如果外设是流控：dma_req 上的活动级别启动事务请求。事务类型（无论是单次还是突发）由 dma_single 输入限定。一旦 dma_req 被置位，它必须保持置位状态，直到 dma_ack 被置位。当驱动 dma_req 的外设确定“dma_ack”已置位时，它必须取消置位 dma_req。
  - dma_single：来自外设的 `single transaction` 请求输入。该信号的功能取决于外设是否是流控。
    - 如果外设不是流控：`dma_single` 信号是一种状态信号，当源或目标外设可以传输或接收至少一个源或目标数据项时，该信号由源或目标外设断言(置位)；否则被清除。该信号
    - 如果外设是流控：不会用到。
  - dma_last：来自外设的块请求中的最后一个事务。该信号的功能取决于外设是否是流控。
    - 如果外设不是流控：`dma last` 信号不相关并且被忽略
    - 如果外设是流控：不会用到。
  - dma_ack：`DW_axi_dmac` 确认输出到外设的信号。该信号的功能取决于外设是否是流控。
    - 如果外设不是流控：`dma_ack` 信号在当前事务`(single/burst)` 中最后一次 `AXI` 数据传输到外设完成后置位。对于 `single` ，`dma_ack` 保持有效状态，直到外设取消 `dma_single` 有效。(即：dma_ack 在 dma_single 无效后的一个 dmac_core_clock 周期被取消有效)。对于 `burst`，`dma_ack` 保持有效状态，直到外设取消 `dma_req` 有效。(即：dma_ack 在 dma_req 无效后的一个 dmac_core_clock 周期被取消)
    - 如果外设是流控：不会用到。
  - dma_finish:这是输出到外设的`DW_axi_dmac` 块传输完成指示信号。 dma_finish 信号被置位以指示块完成。这使用与 dma_ack 相同的时序，并与 dma_req 和/或 dma_single 形成握手循环。



![选区_608](image/选区_608.png)



## 2.4 传输控制

- 传输控制逻辑促进从通道的源外设到目的地外设的数据传输。它与多个其他模块交互，如通道源和目的状态机、链表控制逻辑、通道寄存器、通道FIFO控制逻辑等。
- 来自源外设的数据在发送到目标外设之前暂时存储在通道 FIFO 中。如果源外设和目标外设使用不同的传输大小（arsize、awsize）进行数据传输，则 DW_axi_dmac 实现打包和解包数据所需的逻辑以适应 FIFO 配置。



### 2.4.1 single 传输

如果 DMA 传输由单个块组成，则软件将 CHx_CFG 寄存器中源和目标外设的多块类型位设置为 2’b00。在这种情况下，一旦完成与 CHx_BLOCK_TS 寄存器中编程的块长度相对应的块传输，DW_axi_dmac 就会禁用该通道。

CHx_IntStatusReg 寄存器使用与已完成的块传输相对应的状态进行更新。 CHx_IntStatusReg 寄存器根据 CHx_IntMaskReg 寄存器的设置进行更新，并根据 CHx_IntMaskReg、DMAC_IntMaskReg 和 DMAC_CfgReg 寄存器的设置生成中断。

### 2.4.2 多块传输

如果 DW_axi_dmac 编程用于多块传输，则在多块传输的连续块上使用以下方法之一对 CHx_SAR 和 CHx_DAR 寄存器进行重新编程：

- 连续地址
- 自动装载
- 影子寄存器
- 链表

使用以下任一方法对多块传输的连续块对 CHx_CTL 和 CHx_BLOCK_TS 寄存器进行重新编程：

- 自动装载
- 影子寄存器
- 链表

CHx_IntStatusReg 寄存器使用与已完成的块传输相对应的状态进行更新。 CHx_IntStatusReg 寄存器根据 CHx_IntMaskReg 的设置进行更新，并根据 CHx_IntMaskReg、DMAC_IntMaskReg 和 DMAC_CfgReg 寄存器的设置生成中断。





### 2.4.3 连续地址：多个 block 地址连起来

在这种情况下，连续块之间的地址被选择为前一个块末尾的延续。使源或目标地址能够在块之间连续是 CHx_CTL.SRC_MLTBLK_TYPE 和 CHx_CTL.DST_MLTBLK_TYPE 寄存器字段的功能。

> 如果启用基于连续地址的多块传输，则完整的 DMA 传输中必须至少有两个块。否则，将会导致不可预测的行为。



### 2.4.4 影子寄存器：两套寄存器，用一套，更新时从另一套读

在这种情况下，通道传输控制寄存器在每个块完成时从其相应的影子寄存器加载，并且这些值用于新块。部分或全部 CHx_SAR、CHx_DAR、CHx_BLOCK_TS 和 CHx_CTL 通道寄存器在新块传输开始时从其相应的影子寄存器加载，具体取决于为源和目标外设选择的多块传输类型。

没有为影子寄存器访问定义单独的存储器映射。无论使用何种多块传输类型，软件始终写入 CHx_SAR、CHx_DAR、CHx_BLOCK_TS 和 CHx_CTL 寄存器。如果基于影子寄存器的多块传输用于源或目标传输，则 DW_axi_dmac 在内部将数据路由到相应的影子寄存器。在开始新的块传输之前，DW_axi_dmac 将影子寄存器内容复制到 CHx_SAR、CHx_DAR、CHx_BLOCK_TS 和 CHx_CTL 寄存器。

对 CHx_SAR、CHx_DAR、CHx_BLOCK_TS 和 CHx_CTL 寄存器的读操作始终返回与当前块传输相对应的数据，而不是影子寄存器内容（与下一个块相对应）。

对于基于影子寄存器的多块传输，CHx_CTL 寄存器中的 ShadowReg_Or_LLI_Valid 位指示影子寄存器内容是否有效。 0 表示影子寄存器内容无效，1 表示影子寄存器内容有效。如果在影子寄存器获取阶段该位被读取为零，则 DW_axi_dmac 将丢弃影子寄存器内容并生成 ShadowReg_Or_LLI_Invalid_ERR 中断。 DW_axi_dmac 等待软件向 CHx_BLK_TFR_ResumeReqReg 寄存器写入任何值以指示有效的影子寄存器可用性，然后再尝试另一个影子寄存器获取操作以继续下一个块传输。



### 2.4.5 链表：在内存中用链表结构表示每次 block 传输，传完了从链表取下一个节点

在这种情况下，DW_axi_dmac 在每个块开始之前通过从系统内存中获取该块的块描述符来重新编程通道传输控制寄存器。这称为 LLI 更新。 DW_axi_dmac 块链接使用链表指针寄存器 (CHx_LLP) 来存储下一个链表项在内存中的地址。每个 LLI 包含以下块描述符：

- `CHx_SAR、CHx_DAR、CHx_BLOCK_TS、CHx_CTL、CHx_LLP`

要设置块链接，应在内存中对链表序列进行编程。 DW_axi_dmac允许动态扩展链表，这样就不需要提前在系统内存中创建整个链表。 LLI 的 CHx_CTL.ShadowReg_Or_LLI_Valid 和 CHx_CTL.LLI_Last 字段用于实现此功能。

对于基于链表的多块传输，LLI.CHx_CTL 寄存器的 ShadowReg_Or_LLI_Valid 位指示从内存中获取的链表项是否有效。如果该位设置为 0，则表示 LLI 无效，而设置为 1 则表示 LLI 有效。如果LLI无效，则DW_axi_dmac丢弃LLI并生成LLI错误中断（如果相应的通道错误中断屏蔽位设置为0）。此错误情况会导致 DW_axi_dmac 正常停止相应的通道。 DW_axi_dmac 会等待软件将任何值写入 CHx_BLK_TFR_ResumeReqReg 寄存器以指示有效 LLI 的可用性，然后再尝试另一个 LLI 读取操作。

在 LLI 预取的情况下，即使预取 LLI 的 ShadowReg_Or_LLI_Valid 位设置为 0，也不会生成 ShadowReg_Or_LLI_Invalid_ERR 中断。在这种情况下，DW_axi_dmac 在完成当前块传输后重新尝试 LLI 获取操作。仅当 ShadowReg_Or_LLI_Valid 位仍设置为 0 时，DW_axi_dmac 才会生成 ShadowReg_Or_LLI_Invalid_ERR 中断。


LLI 访问始终使用与数据总线宽度相同的突发大小（arsize 或 awsize），并且不能更改或编程为除此之外的任何值。突发长度（awlen 或 arlen）根据数据总线宽度进行选择，以便访问不会跨越 64 字节的一个完整 LLI 结构。如果突发长度不受其他设置限制，DW_axi_dmac 在一次 AXI 突发中获取整个 LLI（40 字节）。

如果启用状态写回选项，DW_axi_dmac 会将 CHx_CTL、CHx_LLP_STATUS、CHx_SSTAT 和 CHx_DSTAT 信息写回到为此字段定义的位置，即从地址 [CHx_LLP] + 0x20 到 [CHx_LLP] + 0x34。 CHx_SSTAT 和 CHx_DSTAT 写回可以在 coreConsultant 中独立启用或禁用

![选区_483](image/选区_483.png)



### 2.4.6 多块传输结束

如果源或目标外设使用影子寄存器或基于链表的多块传输，则 CHx_CTL_ShadowReg / LLI.CHx_CTL 寄存器中相应的最后一个块指示位 ShadowReg_Or_LLI_Last 指示当前块是传输中的最后一个块还是不是。如果当前块的该位设置为 1，则 DW_axi_dmac 认为当前块是传输中的最后一个块，并在当前块传输结束时完成 DMA 传输操作。

对于连续地址和基于自动重载的多块传输（如果源外设和目标外设均不使用影子寄存器或基于链接表的多块传输），如果相应的多块类型选择位即 CHx_CFG.SRC_MLTBLK_TYPE 和/或 CHx_CFG.DST_MLTBLK_TYPE位在块传输结束时为 2'b00，DW_axi_dmac 知道前一个块是传输中的最后一个块，并完成 DMA 传输操作。





# 相关名词

#### 2.1 DMA channels

一个DMA controller可以“同时”进行的DMA传输的个数是有限的，这称作DMA channels。当然，这里的channel，只是一个逻辑概念，因为：

> 鉴于总线访问的冲突，以及内存一致性的考量，从物理的角度看，不大可能会同时进行两个（及以上）的DMA传输。因而DMA channel不太可能是物理上独立的通道；
>
> 很多时候，DMA channels是DMA controller为了方便，抽象出来的概念，让consumer以为独占了一个channel，实际上所有channel的DMA传输请求都会在DMA controller中进行仲裁，进而串行传输；
>
> 因此，软件也可以基于controller提供的channel（我们称为“物理”channel），自行抽象更多的“逻辑”channel，软件会管理这些逻辑channel上的传输请求。实际上很多平台都这样做了，在DMA Engine framework中，不会区分这两种channel（本质上没区别）。

#### 2.2 DMA request lines

由图片1的介绍可知，DMA传输是由CPU发起的：CPU会告诉DMA控制器，帮忙将xxx地方的数据搬到xxx地方。CPU发完指令之后，就当甩手掌柜了。而DMA控制器，除了负责怎么搬之外，还要决定一件非常重要的事情（特别是有外部设备参与的数据传输）：

> 何时可以开始数据搬运？

因为，CPU发起DMA传输的时候，并不知道当前是否具备传输条件，例如source设备是否有数据、dest设备的FIFO是否空闲等等。那谁知道是否可以传输呢？设备！因此，需要DMA传输的设备和DMA控制器之间，会有几条物理的连接线（称作DMA request，DRQ），用于通知DMA控制器可以开始传输了。

这就是DMA request lines的由来，通常来说，每一个数据收发的节点（称作endpoint），和DMA controller之间，就有一条DMA request line（memory设备除外）。

最后总结：

> DMA channel是Provider（提供传输服务），DMA request line是Consumer（消费传输服务）。在一个系统中DMA request line的数量通常比DMA channel的数量多，因为并不是每个request line在每一时刻都需要传输。

#### 2.3 传输参数

在最简单的DMA传输中，只需为DMA controller提供一个参数----transfer size，它就可以欢快的工作了：

> 在每一个时钟周期，DMA controller将1byte的数据从一个buffer搬到另一个buffer，直到搬完“transfer size”个bytes即可停止。

不过这在现实世界中往往不能满足需求，因为有些设备可能需要在一个时钟周期中，传输指定bit的数据，例如：

> memory之间传输数据的时候，希望能以总线的最大宽度为单位（32-bit、64-bit等），以提升数据传输的效率；
> 而在音频设备中，需要每次写入精确的16-bit或者24-bit的数据；
> 等等。

因此，为了满足这些多样的需求，我们需要为DMA controller提供一个额外的参数----transfer width。

另外，当传输的源或者目的地是memory的时候，为了提高效率，DMA controller不愿意每一次传输都访问memory，而是在内部开一个buffer，将数据缓存在自己buffer中：

> memory是源的时候，一次从memory读出一批数据，保存在自己的buffer中，然后再一点点（以时钟为节拍），传输到目的地；
> memory是目的地的时候，先将源的数据传输到自己的buffer中，当累计一定量的数据之后，再一次性的写入memory。

这种场景下，DMA控制器内部可缓存的数据量的大小，称作burst size----另一个参数。

#### 2.4 scatter-gather

> 一般情况下，DMA传输一般只能处理在物理上连续的buffer。但在有些场景下，我们需要将一些非连续的buffer拷贝到一个连续buffer中（这样的操作称作scatter gather，挺形象的）。
>
> 对于这种非连续的传输，大多时候都是通过软件，将传输分成多个连续的小块（chunk）。但为了提高传输效率（特别是在图像、视频等场景中），有些DMA controller从硬件上支持了这种操作。
> 注2：具体怎么支持，和硬件实现有关，这里不再多说（只需要知道有这个事情即可，编写DMA controller驱动的时候，自然会知道怎么做）。





#### 2.5 名词：

- 源外设：位于 AXI 层上的设备，`DW_axi_dmac` 从中读取数据，然后 `DW_axi_dmac` 将数据存储在通道 FIFO 中。源和目标组成形成通道。

- 目标外设：DW_axi_dmac 将存储的数据从 FIFO 写入到设备(数据是从源获取的)。
- 内存：总是为 DMA 传输做好准备的源或外设，不需要握手接口来与 DW_axi_dmac 交互。

- trans_width：传输数据的位宽，比如 8 bit，32bit，64bit

- MSIZE：burst size，每次传输的单位，比如 1、4、8

- AWLEN、ARLEN：每次 burst 的节拍

  ```
  例如：
  	AWADDR：0x800000
  	AWLEN:0x3(表示 4 个数据节拍)
  	MSIZE:0x2(表示每个传输单元为 4 个字节)
  	trans_width:0x8(表示每次传输的位宽为 8bit)
  	则会进行 4 次 burst 传输，每次 burst 传输的长度为：4 * 1 = 4 个字节，共传输 16 个字节数据
  ```

  







# 寄存器

### 通用寄存器

#### DMA_IDREG:0x00

| Bits  | Name    | 描述       |
| ----- | ------- | ---------- |
| 63：0 | DMAC_ID | DMAC ID 号 |



#### DMAC_COMPVERREG:0x08

| Bits  | Name         | 描述            |
| ----- | ------------ | --------------- |
| 31：0 | DMAC_COMPVER | DMAC 组件版本号 |





#### DMAC_CFGREG:0x10

| Bits | Name    | 描述                                                         |
| ---- | ------- | ------------------------------------------------------------ |
| 1    | INT_EN  | 此位用于全局启用中断生成。<br />0x0：中断禁用<br />0x1：中断启用 |
| 0    | DMAC_EN | 此位用于启用 DW_axi_dmac。(如果在任何通道处于活动状态时清除此位，则此位仍返回1)<br />0x0：禁用 DW_axi_dmac<br />0x1：使能 DW_axi_dmac |





#### DMAC_CHENREG:0x18

这是DW_axi_dmac通道启用寄存器。如果软件想要设置新通道，它可以读取此寄存器以找出当前处于非活动状态的通道，然后以所需的优先级启用非活动通道。

当全局使能位 （DMAC_CfgReg.DMAC_EN） 为 0 时DW_axi_dmac，此寄存器的所有位都清除为 0。当 DMAC_CfgReg.DMAC_EN 为 0 时，将忽略对DMAC_ChEnReg寄存器的写入，并且读取始终读回 0。

仅当在同一从接口写入传输上置位 DMAC_ChEnReg.CH_EN_WE 时，才会写入通道使能位 DMAC_ChEnReg.CH_EN。例如，写十六进制XXXX01X1将 1 写入 DMAC_ChEnReg [0]，而 DMAC_ChEnReg [7：1] 保持不变。写十六进制XXXX00XX会使 DMAC_ChEnReg [7：0] 保持不变。

仅当在同一从接口写入传输上置位 DMAC_ChEnReg.CH_SUSP_WE 时，才会写入通道挂起位 DMAC_ChEnReg.CH_SUSP。例如，写入十六进制 01X1XXXX 会将 1 写入 DMAC_ChEnReg [16]，而 DMAC_ChEnReg [23：17] 保持不变。写入十六进制 00XXXXXX 使 DMAC_ChEnReg [23：16] 保持不变。仅当在同一从接口写入传输上置位 DMAC_ChEnReg.CH_ABORT_WE 时，才会写入通道中止位 DMAC_ChEnReg.CH_ABORT。

| Bits | Name         | 描述                                                         |
| ---- | ------------ | ------------------------------------------------------------ |
| 47   | CH8_ABORT_WE | 此位用于写入启用通道8中止位。此寄存器位的回读值始终为0.<br />0x1：启用写入 CH8_ABORT 位<br />0x0：禁止写入 CH8_ABORT 位 |
| 39   | CH8_ABORT    | channel-8 中止请求。<br />软件将此位设置为 1 以请求通道中止。如果此位设置为1，则DW_axi_dmac 立即禁用通道。终止通道可能会导致 AXI 协议冲突，因为 DW_axi_dmac 无法确保在主接口上启动的所有 AXI 传输都已完成。不建议终止通道，并且应仅在特定通道由于相应的 AXI 从接口没有响应而挂起且软件希望在不重置整个 DW_axi_dmac 的情况下禁用通道的情况下使用。建议先尝试禁用 channel，然后再选择 channel abort。<br />0：没有请求通道 8 中止<br />1：请求通道 8 中止 |
| 31   | CH8_SUSP_WE  | 此位用作 channel 8 挂起位的写入使能。此寄存器的回读值始终为 0.<br />0x0：禁用写入 CH8_SUSP 位<br />0x1：启用写入 CH8_SUSP 位 |
| 23   | CH8_SUSP     | channel 8 暂停请求。<br />软件将此位设置为1以请求通道挂起。如果此位设置为1，则DW_axi_dmac 正常挂起来自源的所有 DMA 数据传输，直到清除此位。不能保证当前的 dma 传输完成。此位还可以与 CH8_Status.CHSUSPENDED 结合使用，以干净利落地禁用通道而不会丢失任何数据。在这种情况下，软件首先将 CH8_SUSP 位设置为 1，然后轮询 CH8_Status.CH_SUSPENDED 直到将其设置为 1.然后，软件可以将 CH8_EN 位清除为0以禁用通道。<br />0x0：无通道暂停请求<br />0x1：请求通道暂停 |
| 15   | CH8_EN_WE    | DW_axi_dmac channel 8 使能写入使能位。此寄存器位的回读值始终为 0.<br />0x0：禁用写入 CH8_EN位<br />0x1：启用写入 CH8_EN 位 |
| 7    | CH8_EN       | 此位用于启用 DW_axi_dmac channel 8.<br />0x0：DW_axi_dmac channel 8 禁用<br />0x1：DW_axi_dmac channel 8 启用<br />在DMA传输到目标的最后一次AMBA传输完成后，硬件会自动清除位 `DMAC_CHEnReg.CH8_EN` 以禁用通道。因此，软件可以轮询此位，已确定此通道何时可用于新的 DMA 传输。 |





#### DMAC_INTSTATUSREG:0x30

| Bits | Name              | 描述                                                         |
| ---- | ----------------- | ------------------------------------------------------------ |
| 16   | CommonReg_IntStat | 公共寄存器中断状态位。<br />0x1：公共寄存器中断处于活动状态<br />0x0：公共寄存器中断处于非活动状态 |
| 7    | CH8_IntStat       | 通道 8 中断状态位。<br />0x1：通道 8 中断处于活动状态<br />0x0：通道 8 中断处于非活动状态 |



#### DMAC_COMMONREG_INTCLEARREG:0x38

将 1 写入特定字段可清除 DMAC 公共寄存器中断状态寄存器`(DMAC_CommonReg_IntStatusReg)`中的相应字段

| Bits | Name                                       | 描述                                                         |
| ---- | ------------------------------------------ | ------------------------------------------------------------ |
| 8    | Clear_SLVIF_UndefinedReg_DEC_ERR_IntStat   | 从接口为定义寄存器解码错误中断清除位。该位用于清除 `DMAC_CommonReg_IntStatusReg` 中对应的通道中断状态 bit(SLVIF_UndefinedReg_DEC_ERR_IntStat)。<br />0x1：清除 DMAC_CommonReg_IntStatusReg 中的 SLVIF_UndefinedReg_DEC_ERR 中断<br />0x0：未采取任何行动 |
| 3    | Clear_SLVIF_CommonReg_WrOn Hold_ERR_InStat | 0x1：清除 DMAC_CommonReg_IntStatusReg 中的 SLVIF_CommonReg_WrOnHold_ERR 中断<br />0x0：未采取任何行动 |
| 2    | Clear_SLVIF_CommonReg_RD2 WO_ERR_IntStat   | 0x1：清除 DMAC_CommonReg_IntStatusReg 中的 SLVIF_CommonReg_RD2WO_ERR 中断<br />0x0：未采取任何行动 |
| 1    | Clear_SLVIF_CommonReg_WR2 RO_ERR_IntStat   | 0x1：清除 DMAC_CommonReg_IntStatusReg 中的 SLVIF_CommonReg_WR2RO_ERR 中断<br />0x0：未采取任何行动 |
| 0    | Clear_SLVIF_CommonReg_DEC _ERR_IntStat     | 0x1：清除 DMAC_CommonReg_IntStatusReg 中的 SLVIF_CommonReg_DEC_ERR 中断<br />0x0：未采取任何行动 |



#### DMAC_COMMONREG_INTSTATUS_ENABLEREG:0x40

将 1 写入特定字段可在 DMAC 公共寄存器中断状态寄存器 （DMAC_CommonReg_IntStatusReg） 中生成相应的中断状态。

| Bits | Name                                            | 描述                                                         |
| ---- | ----------------------------------------------- | ------------------------------------------------------------ |
| 8    | Enable_SLVIF_UndefinedReg<br />_DEC_ERR_IntStat | 从接口为定义寄存器解码错误中断清除位。该位用于使能 `DMAC_CommonReg_IntStatusReg` 中对应的通道中断状态 bit(SLVIF_UndefinedReg_DEC_ERR_IntStat)。<br />0x1：使能 DMAC_CommonReg_IntStatusReg 中的 SLVIF_UndefinedReg_DEC_ERR 中断<br />0x0：禁止 |
| 3    | Enable_SLVIF_CommonReg_WrOn Hold_ERR_InStat     | 0x1：使能 DMAC_CommonReg_IntStatusReg 中的 SLVIF_CommonReg_WrOnHold_ERR 中断<br />0x0：禁止 |
| 2    | Enable_SLVIF_CommonReg_RD2 WO_ERR_IntStat       | 0x1：使能 DMAC_CommonReg_IntStatusReg 中的 SLVIF_CommonReg_RD2WO_ERR 中断<br />0x0：禁止 |
| 1    | Enable_SLVIF_CommonReg_WR2 RO_ERR_IntStat       | 0x1：使能 DMAC_CommonReg_IntStatusReg 中的 SLVIF_CommonReg_WR2RO_ERR 中断<br />0x0：禁止 |
| 0    | Enable_SLVIF_CommonReg_DEC _ERR_IntStat         | 0x1：使能 DMAC_CommonReg_IntStatusReg 中的 SLVIF_CommonReg_DEC_ERR 中断<br />0x0：禁止 |





#### DMAC_COMMONREG_INTSTATUSREG:0x50

此寄存器捕获从接口访问错误。包括：解码错误、写了RO寄存器、读了WO寄存器、写入暂停、未定义的地址

| Bits | Name                                | 描述 |
| ---- | ----------------------------------- | ---- |
| 8    | SLVIF_UndefinedReg_DEC_ERR_IntStat  |      |
| 3    | SLVIF_CommonReg_WrOnHold_ERR_InStat |      |
| 2    | SLVIF_CommonReg_RD2 WO_ERR_IntStat  |      |
| 1    | SLVIF_CommonReg_WR2 RO_ERR_IntStat  |      |
| 0    | SLVIF_CommonReg_DEC _ERR_IntStat    |      |





#### DMAC_RESETREG:0x58

此寄存器用于软件重置 DW_axi_dmac

| Bits | Name     | 描述                                                         |
| ---- | -------- | ------------------------------------------------------------ |
| 0    | DMAC_RST | DMAC 重置请求位，软件将1写入此位以重置 DW_axi_dmac 并轮询此位将其视为 0.DW_axi_dmac 复位除总线接口模块之外的所有模块，并将此位清除为 0. |






### 通道寄存器

#### CHx_SAR:(0x100 + (x-1) * 0x100)

起始源地址在启用 DMA 通道之前由软件编程，或在 DMA 传输开始之前由  LLI 更新编程。在进行DMA传输时，此寄存器将更新以反映当前 AXI 传输的源地址。

| Bits  | Name | 描述                                                         |
| ----- | ---- | ------------------------------------------------------------ |
| 63：0 | SAR  | DMA 传输的当前源地址。<br />每次源传输后更新。CHx_CTL寄存器中的 SINC 字段确定在整个块传输中，每个源传输的地址是递增还是保持不变。 |



#### CHx_DST:(0x108 + (x-1) * 0x100)

起始目标地址在启用 DMA 通道之前由软件编程，或在 DMA 传输开始之前由  LLI 更新编程。在进行DMA传输时，此寄存器将更新以反映当前 AXI 传输的目标地址。

| Bits  | Name | 描述                                                         |
| ----- | ---- | ------------------------------------------------------------ |
| 63：0 | DST  | DMA 传输的当前目标地址。<br />每次目标传输后更新。CHx_CTL寄存器中的 DINC 字段确定在整个块传输中，每个目标传输的地址是递增还是保持不变。 |



#### CHx_BLOCK_TS:(0x110 + (x-1) * 0x100)

当 DW_axi_dmac 是流控时，DMAC 在通道启用块大小之前使用此寄存器。

| Bits  | Name     | 描述                                                         |
| ----- | -------- | ------------------------------------------------------------ |
| 21：0 | BLOCK_TS | 块传输大小。<br />编程到 BLOCK_TS 字段中的数字表示宽度 CHx_CTL 的数据总数。<br />SRC_TR_WIDTH 在 DMA 块传输中传输。块传输大小 = BLOCK_TS + 1 传输开始时，回读值是已从源外设读取的数据项总数，无论谁是流控。当将源或目标外设指定为流控时，传输开始前的值在 DMAX_CHx_MAX_BLK_SIZE 饱和，但实际块大小可能更大。 |



#### CHx_CTL:(0x118 + (x-1) * 0x100)

此寄存器包含控制 DMA 传输的字段。在启用通道之前，应对该寄存器进行编程，但基于 LLI 的多块传输除外。启用基于 LLI 的多块传输时，CHx_CTL 寄存器将从 LLI 的相应位置加载，并且可以在 DMA 传输中逐块更改。软件不允许通过 DW_axi_dmac 从接口直接更新此寄存器。在基于 LLI 的多块传输器件，对此寄存器的任何写入都将被忽略。

| Bits   | Name                   | 描述                                                         |
| ------ | ---------------------- | ------------------------------------------------------------ |
| 63     | SHADOWREG_OR_LLI_VALID | 影子寄存器内容/链表项有效。指示影子寄存器的内容或从内存中获取的链表项是否有效。<br />0：影子寄存器内容/LLI 无效<br />1：影子寄存器内容/LLI 有效<br />从 LLI 加载。因此，软件不允许通过DW_axi_dmac从接口直接更新此寄存器。此字段可用于通过软件动态扩展 LLI。在注意到此位为 0 时，如果相应的通道错误中断掩码位设置为 0，则 DW_axi_dmac 丢弃 LLI 并生成ShadowReg_Or_LII_Invalid_ERR中断。在 LLI 预取的情况下，即使预取的 LLI 的 ShadowReg_Or_LLI_Valid 位为 0，也不会生成ShadowReg_Or_LLI_Invalid_ERR中断。在这种情况下，DW_axi_dmac 在完成当前块传输后再次尝试 LLI 获取操作，并且仅当ShadowReg_Or_LII_Valid位仍为 0 时才生成ShadowReg_Or_LII_Invalid_ERR中断。<br />此错误情况会导致DW_axi_dmac正常停止相应的通道。DW_axi_dmac 等到软件写入（任何值）到 CHx_BLK_TFR_ResumeReqReg 以指示有效的 LLI 可用性，然后再尝试另一个 LLI 读取操作。启用 LLI 写回选项时，此位被清除为 0，并在块传输完成后写回相应的 LLI 位置。因此，对于基于 LLI 的多块传输，如果启用了 LLI 回写选项，软件可能会操作/重新定义 ShadowReg_Or_LII_Valid 位设置为 0 的任何描述符。<br /><br /><br />基于 Shadow Reg 的多块传输：注意到这一点<br />在影子寄存器获取阶段，位为 0，DW_axi_dmac丢弃影子寄存器内容并生成ShadowReg_Or_LLI_Invlid_ERR中断。在这种情况下，软件必须在更新卷影寄存器并将ShadowReg_Or_LLI_Valid位设置为 1 后写入（任何值）CHx_BLK_TFR_ResumeReqReg，以向DW_axi_dmac指示卷影寄存器内容有效，并且可以恢复下一次块传输。DW_axi_dmac复制卷影寄存器内容后将此位清除为 0。仅当ShadowReg_Or_LLI_Valid位为 0 时，软件才能对影子寄存器进行重新编程。软件需要在块完成中断服务例程中读取此寄存器（如果启用了中断）/连续轮询此寄存器（如果未启用中断），以确保在更新影子寄存器之前此位为 0。如果启用了基于影子寄存器的多块传输，并且软件在位为 1 时尝试写入ShadowReg_Or_LLI_Valid影子寄存器，则DW_axi_dmac生成SLVIF_ShadowReg_WrOnValid_ERR中断。 |
| 62     | SHADOWREG_OR_LLI_LAST  | 最后一个影子寄存器/链表项。<br />指示影子寄存器内容或从内存中获取的链表项是否是最后一个。<br />0x0：不是最后一个影子寄存器/LLI<br />0x1：是最后一个影子寄存器/LLI<br /><br /><br />基于 LLI 的多块传输：DW_axi_dmac使用此位以确定当前 DMA 传输中是否需要另一个 LLI 提取。<br />如果此位为 0，则DW_axi_dmac从当前 LLI 中 LLP 字段指出的地址中获取下一个 LLI。<br />如果此位为 1，则DW_axi_dmac理解当前块是 dma 传输中的最后一个块，并在与当前块对应的 AMBA 传输完成后结束 dma 传输。<br /><br /><br />基于 Shadow Reg 的多块传输：DW_axi_dmac 使用此位来决定在当前 DMA 传输中是否需要另一个影子寄存器提取。<br />如果此位为 0，则DW_axi_dmac理解当前块中有一个或多个块要传输，因此一个或多个影子寄存器集内容将有效并需要获取。<br />如果此位为 1，则DW_axi_dmac理解当前块是 dma 传输中的最后一个块，并在与当前块对应的 AMBA 传输完成后结束 dma 传输。 |
| 58     | IOC_BlkTfr             | 块传输完成时中断 该位用于控制块传输完成中断生成，以块为块，用于基于影子寄存器或链表的多块传输。如果在寄存器中启用了此中断生成，则将 1 写入此寄存器字段将启用 CHx_IntStatusReg.BLOCK_TFR_DONE_IntStat 字段。CHx_IntStatus_EnableReg并且如果在寄存器中启用了此中断生成，则断言外部中断输出CHx_IntSignal_EnableReg。注意：如果源和目标未同时使用基于链表或影子寄存器的多块传输（例如，如果源和目标使用连续地址或基于自动重新加载的多块传输），则无法修改每个块的此字段的值。此外，值在通道启用之前编程用于 DMA 传输中的所有模块。<br />0x1：启用 CHx_IntStatusReg.BLOCK_TFR_DONE_INTStat 字段<br />0x0：禁用 |
| 57     | DST_STAT_EN            | 目标状态启用.启用从CHx_DSTATAR寄存器内容指向的通道 x 的目标外设获取状态的逻辑，并将其存储在寄存器CHx_DSTAT。如果 DMAX_CHx_LLI_WB_EN 设置为 1，并且源或目标外围设备使用基于链表的多块传输，则此值将在每次块传输结束时写回链表的CHx_DSTAT位置。<br />0x1：启用目标状态提取并将值存储在寄存器 CH1_DSTAT<br />0x0：目标设备没有状态提取 |
| 56     | SRC_STAT_EN            | 源状态启用。启用逻辑，以从CHx_SSTATAR寄存器的内容指向的通道 x 的源外设获取状态，并将其存储CHx_SSTAT寄存器中。如果 DMAX_CHx_LLI_WB_EN 设置为 1，并且源或目标外围设备使用基于链表的多块传输，则此值将写回每个块传输结束时链表的CHx_SSTAT位置。<br />0x1：启用源的状态提取并将值存储在寄存器 CH1_SSTAT<br />0x0：源设备没有状态提取 |
| 55：48 | AWLEN                  | 目标突发长度。AXI 用于目标数据传输的突发长度。指定的突发长度尽可能用于目标数据传输;其余传输使用小于或等于 DMAX_CHx_MAX_AMBA_BURST_LENGTH 的最大可能值。AWLEN的最大值受DMAX_CHx_MAX_AMBA_BURST_LENGTH限制。注意：AWLEN 设置可能不适用于端到块传输、交易结束（仅适用于非内存外周）和 4K 边界交叉期间。 |
| 47     | AWLEN_EN               | 目标突发长度启用。如果此位设置为 1，则DW_axi_dmac 使用 CHx_CTL 值。AWLEN 作为 AXI 尽可能用于目标数据传输的突发长度;其余传输使用最大可能的突发长度。如果此位设置为 0，则DW_axi_dmac使用任何小于或等于 DMAX_CHx_MAX_AMBA_BURST_LENGTH 的可能值作为目标数据传输的 AXI 突发长度。<br />0x1：AXI burst 长度为 CH1_CTL.AWLEN(尽可能)用于目标数据传输<br />0x0：AXI burst 长度是目标数据传输的任何可能值 <= DMAX_CH1_MAX_AMBA_BURST_LENGTH |
| 46:39  | ARLEN                  | 源突发长度。AXI 用于源数据传输的突发长度。指定的突发长度用于源数据传输，尽可能;其余传输使用小于或等于 DMAX_CHx_MAX_AMBA_BURST_LENGTH 的最大可能值。ARLEN的最大值受DMAX_CHx_MAX_AMBA_BURST_LENGTH限制 |
| 38     | ARLEN_EN               | 源突发长度启用。如果此位设置为 1，则DW_axi_dmac 使用 CHx_CTL 值。ARLEN 作为 AXI 尽可能传输源数据的突发长度;其余传输使用最大可能的突发长度。如果此位设置为 0，则 DW_axi_dmac 使用任何小于或等于 DMAX_CHx_MAX_AMBA_BURST_LENGTH 的可能值作为源数据传输的 AXI 突发长度。<br />0x1：AXI burst 长度为 CH1_CTL.ARLEN(尽可能)用于源数据传输<br />0x0：AXI burst 长度是源数据传输的任何可能值 <= DMAX_CH1_MAX_AMBA_BURST_LENGTH |
| 37:35  | AW_PORT                | AXI `aw_prot` 信号                                           |
| 34:32  | AR_PORT                | AXI `ar_prot` 信号                                           |
| 30     | NonPosted_LastWrite_EN | Non Posted Last Write Enable 此位决定是否可以在整个块传输过程中使用已发布的写入。<br />0x1：块中的最后一次写入必须是未发布的<br />0x0：可以在整个块传输过程中使用已发布的写入 |
| 39：26 | AW_CACHE               | AXI `aw_cache` 信号                                          |
| 25：22 | AR_CACHE               | AXI `ar_cache` 信号                                          |
| 21：18 | DST_MSIZE              | 目标突发事务长度。数据项数，每个数据项的宽度CHx_CTL.DST_TR_WIDTH，每次从相应的硬件或软件握手接口发出目标突发事务请求时，都会写入目标。注意：此值与 AXI awlen 信号无关。<br />0x0：burst 从目标读 1 个数据项<br />0x1：burst 从目标读 4 个数据项<br />8、16、32、64、128、256、512、1024 |
| 17：14 | SRC_MSIZE              | 源突发事务长度。数据项数，每个数据项的宽度CHx_CTL。SRC_TR_WIDTH，每次从相应的硬件或软件握手接口发出源突发事务请求时，从源读取。DST_MSIZE 的最大值受DMAX_CHx_MAX_MSIZE限制。<br />0x0：burst 从源读取 1 个数据项<br />4、8、16、32、64、128、256、512、1024 |
| 13：11 | DST_TR_WIDTH           | 目标传输宽度。映射到 AXI 总线时 ，此值必须小于或等于 DMAX_M_DATA_WIDTH.<br />0x0：目标传输宽度为 8位<br />16、32、64、128、256、512 |
| 10：8  | SRC_TR_WIDTH           | 源传输宽度。映射到 AXI 总线大小时，此值必须小于或等于 DMAX_M_DATA_WIDTH<br />0x0：源传输宽度为 8位<br />16、32、64、128、256、512 |
| 6      | DINC                   | 目标地址增量。指示是否在每次目标传输时递增目标地址。如果设备从具有固定地址的源外设 FIFO 写入数据，则将此字段设置为“无更改”。<br />0：每次源传输时目标地址递增<br />0x1：目标地址固定 |
| 4      | SINC                   | 源地址增量。指示是否在每次源传输时递增源地址。如果设备使用固定地址从源外设FIFO获取数据，则将此字段设置为“无更改”。<br />0：每次源传输时源地址递增<br />0x1：源地址固定 |
| 2      | DMS                    | 目标 master 选择。<br />0:AXI Master 1<br />1:AXI Master 2   |
| 0      | SMS                    | 源 master 选择。<br />0:AXI Master 1<br />1:AXI Master 2     |





#### CHx_CFG:(0x120 + (x-1) * 0x100) (CH <= 8)

此寄存器包含配置 DMA 传输的字段。应在启用通道之前对该寄存器进行编程。

通道配置寄存器的位 [63：32] 对于多块传输的所有块保持固定，并且仅在通道被禁用时才能进行编程。

即使使能通道，也可以对通道配置寄存器的位 [3：0] 进行编程。

软件清除这些位以结束多块传输。对于基于连续地址和自动重新加载的多块传输（如果源和目标外围设备均不使用基于 ShadowRegister 或基于链接列表的多块传输），如果相应的多块类型选择位即 CHx_CFG。SRC_MLTBLK_TYPE和/或CHx_CFG。DST_MLTBLK_TYPE位在块传输结束时显示为 2'b00，则DW_axi_dmac了解前一个块是传输中的最后一个块并完成 DMA 传输操作。

| Bits       | Name             | 描述                                                         |
| ---------- | ---------------- | ------------------------------------------------------------ |
| 62:59      | DST_OSR_LMT      | 目标未完成请求限制。<br />支持的最大未完成请求为 16.<br />源未完成请求限制 = DST_OSR_LMT + 1 |
| 58:55      | SRC_OSR_LMT      | 源未完成请求限制。<br />支持的最大未完成请求为 16.<br />源未完成请求限制 = SRC_OSR_LMT + 1 |
| 54:53      | LOCK_CH_L        | 通道锁定级别。此位表示 CHx_CFG 的持续时间。LOCK_CH 位适用。<br />00：已完成 DMA 传输<br />01：通过 DMA block transfer<br />1x：保留<br />如果配置参数 DMAX_CHx_LOCK_EN 设置为 False，则此字段不存在。这种情况下，回读值始终为 0.<br />0x0：通道锁定的持续时间适用于整个 DMA 传输<br />0x1：通道锁定的持续时间适用于当前 block 传输 |
| 52         | LOCK_CH          | 通道锁定位。当通道被授予主总线接口的控制权时，如果 CHx_CFG.LOCK_CH位被置位，则在CHx_CFG中指定的持续时间内，不会授予其他通道对主总线接口的控制权。LOCK_CH_L. 向主总线接口仲裁器指示此通道希望在CHx_CFG中指定的持续时间内对主总线接口进行独占访问。LOCK_CH_L。如果配置参数 DMAX_CHx_LOCK_EN 设置为 False，则此字段不存在;在这种情况下，回读值始终为 0。锁定通道会锁定相应主接口上的 AXI 读地址、写地址和写数据通道。注意：通道锁定功能仅支持块传输和DMA传输级别的内存到内存传输。硬件不会检查通道锁定设置的有效性，因此软件必须注意仅在块传输或 DMA 传输级别的内存到内存传输中启用通道锁定。通道锁定的非法编程可能会导致不可预知的行为。<br />0x0：传输过程中通道未锁定<br />0x1：通道被锁定并被授予对主总线接口的独占访问权限 |
| 51：49     | CH_PRIOR         | 通道优先级。DMAX_NUM_CHANNELS - 1 的优先级为最高优先级。0为最低优先级。 |
| （47）x:44 | DST_PER          | 目标硬件握手接口(0 ~ DMAX_NUM_HS_IF - 1)。如果CHx_CFG.HS_SEL_DST字段为 0,则到 Channelx 的目的地。否则，将忽略此字段。然后，通道可以通过分配的硬件握手接口与连接到该接口的目标外设进行通信。注意：为了正确的DW_axi_dmac操作，应仅将一个外设（源或目标）分配给同一握手接口。如果配置参数 DMAX_NUM_HS_IF 设置为 0，则此字段不存在。如果 DMAC_NUM_HS_IF 为 1，则 x = 44 x = ceil（log2（DMAC_NUM_HS_IF）） + 43（如果 DMAC_NUM_HS_IF 大于 1 位 47：（x+1） 不存在，读取时返回 0。 |
| （42）x:39 | SRC_PER          | 源硬件握手接口(0 ~ DMAX_NUM_HS_IF - 1)。                     |
| 38         | DST_HWHS_POL     | 目标硬件握手接口极性。<br />0：高电平有效<br />1：低电平有效 |
| 37         | SRC_HWHS_POL     | 源硬件握手接口极性。<br />0：高电平有效<br />1：低电平有效   |
| 36         | HS_SEL_DST       | 目标软件或硬件握手选择。<br />0：硬件握手。软件发起的 transaction 请求将被忽略<br />1：软件握手。硬件发起的 transaction 请求将被忽略 |
| 35         | HS_SEL_SRC       | 源软件或硬件握手选择。<br />0：硬件握手。软件发起的 transaction 请求将被忽略<br />1：软件握手。硬件发起的 transaction 请求将被忽略 |
| 34：32     | TT_FC            | 传输类型和流控。<br />0x0：内存到内存，流控为 DW_axi_dmac<br />0x1：内存到外设，流控为 DW_axi_dmac<br />0x2：外设到内存，流控为 DW_axi_dmac<br />0x3：外设到外设，流控为 DW_axi_dmac<br />0x4：外设到内存，流控为 源外设<br />0x5：外设到外设，流控为 源外设<br />0x6：内存到外设，流控为 目标外设<br />0x7：外设到外设，流控为 目标外设 |
| 3：2       | DST_MULTBLK_TYPE | 目标多块传输类型。<br />00：连续<br />01：重新装载<br />10：影子寄存器<br />11：链表<br />如果选择的类型是连续的，则CHx_DAR寄存器将在每个块的末尾加载前一个块的最终源地址 + 1 的值，以进行多块传输。然后启动新的区块传输。如果选择的类型为“重新加载”，则CHx_DAR寄存器将从每个块末尾的 DAR 初始值重新加载，以进行多块传输。然后启动新的区块传输。如果选择的类型为“卷影寄存器”（Shadow Register），则CHx_DAR寄存器将从其卷影寄存器的内容（如果CHx_CTL）加载。对于多块传输，每个块末尾的 ShadowReg_Or_LLI_Valid 位设置为 1。然后启动新的区块传输。如果选择的类型是链表，则 CTL 时将从链表加载CHx_DAR寄存器。对于多块传输，每个块末尾的 ShadowReg_Or_LLI_Valid 位设置为 1。然后启动新的区块传输。CHx_CTL 和 CHx_BLOCK_TS 寄存器从其初始值或其影子寄存器的内容（如果CHx_CTL）加载。ShadowReg_Or_LLI_Valid位设置为 1）或来自链表（如果为 CTL.ShadowReg_Or_LLI_Valid位设置为 1），用于基于为源和目标外设编程的多块传输类型进行多块传输的多块传输。源外设和目标外设上的连续传输不是有效的多块传输配置。 |
| 1：0       | SRC_MULTBLK_TYPE | 源多块传输类型。<br />00：连续<br />01：重新装载<br />10：影子寄存器<br />11：链表 |







#### CHx_CFG2:(0x120 + (x-1) * 0x100) (CH > 8)

此寄存器包含配置 DMA 传输的字段。应在启用通道之前对该寄存器进行编程。

通道配置寄存器的位 [63：32] 对于多块传输的所有块保持固定，并且仅在通道被禁用时才能进行编程。

即使使能通道，也可以对通道配置寄存器的位 [3：0] 进行编程。

软件清除这些位以结束多块传输。对于基于连续地址和自动重新加载的多块传输（如果源和目标外围设备均不使用基于 ShadowRegister 或基于链接列表的多块传输），如果相应的多块类型选择位即 CHx_CFG。SRC_MLTBLK_TYPE和/或CHx_CFG。DST_MLTBLK_TYPE位在块传输结束时显示为 2'b00，则DW_axi_dmac了解前一个块是传输中的最后一个块并完成 DMA 传输操作。

| Bits     | Name                                | 描述                                                         |
| -------- | ----------------------------------- | ------------------------------------------------------------ |
| 62:59    | DST_OSR_LMT                         | 目标未完成请求限制。<br />支持的最大未完成请求为 16.<br />源未完成请求限制 = DST_OSR_LMT + 1 |
| 58:55    | SRC_OSR_LMT                         | 源未完成请求限制。<br />支持的最大未完成请求为 16.<br />源未完成请求限制 = SRC_OSR_LMT + 1 |
| 54:53    | LOCK_CH_L                           | 通道锁定级别。此位表示 CHx_CFG 的持续时间。LOCK_CH 位适用。<br />00：已完成 DMA 传输<br />01：通过 DMA block transfer<br />1x：保留<br />如果配置参数 DMAX_CHx_LOCK_EN 设置为 False，则此字段不存在。这种情况下，回读值始终为 0.<br />0x0：通道锁定的持续时间适用于整个 DMA 传输<br />0x1：通道锁定的持续时间适用于当前 block 传输 |
| 52       | LOCK_CH                             | 通道锁定位。当通道被授予主总线接口的控制权时，如果 CHx_CFG.LOCK_CH位被置位，则在CHx_CFG中指定的持续时间内，不会授予其他通道对主总线接口的控制权。LOCK_CH_L. 向主总线接口仲裁器指示此通道希望在CHx_CFG中指定的持续时间内对主总线接口进行独占访问。LOCK_CH_L。如果配置参数 DMAX_CHx_LOCK_EN 设置为 False，则此字段不存在;在这种情况下，回读值始终为 0。锁定通道会锁定相应主接口上的 AXI 读地址、写地址和写数据通道。注意：通道锁定功能仅支持块传输和DMA传输级别的内存到内存传输。硬件不会检查通道锁定设置的有效性，因此软件必须注意仅在块传输或 DMA 传输级别的内存到内存传输中启用通道锁定。通道锁定的非法编程可能会导致不可预知的行为。<br />0x0：传输过程中通道未锁定<br />0x1：通道被锁定并被授予对主总线接口的独占访问权限 |
| 51：47   | <font color=red>**CH_PRIOR**</font> | 通道优先级。DMAX_NUM_CHANNELS - 1 的优先级为最高优先级。0为最低优先级。 |
| 38       | DST_HWHS_POL                        | 目标硬件握手接口极性。<br />0：高电平有效<br />1：低电平有效 |
| 37       | SRC_HWHS_POL                        | 源硬件握手接口极性。<br />0：高电平有效<br />1：低电平有效   |
| 36       | HS_SEL_DST                          | 目标软件或硬件握手选择。<br />0：硬件握手。软件发起的 transaction 请求将被忽略<br />1：软件握手。硬件发起的 transaction 请求将被忽略 |
| 35       | HS_SEL_SRC                          | 源软件或硬件握手选择。<br />0：硬件握手。软件发起的 transaction 请求将被忽略<br />1：软件握手。硬件发起的 transaction 请求将被忽略 |
| 34：32   | TT_FC                               | 传输类型和流控。<br />0x0：内存到内存，流控为 DW_axi_dmac<br />0x1：内存到外设，流控为 DW_axi_dmac<br />0x2：外设到内存，流控为 DW_axi_dmac<br />0x3：外设到外设，流控为 DW_axi_dmac<br />0x4：外设到内存，流控为 源外设<br />0x5：外设到外设，流控为 源外设<br />0x6：内存到外设，流控为 目标外设<br />0x7：外设到外设，流控为 目标外设 |
| (16)x:11 | <font color=red>**DST_PER**</font>  | 目标硬件握手接口(0 ~ DMAX_NUM_HS_IF - 1)。如果CHx_CFG.HS_SEL_DST字段为 0,则到 Channelx 的目的地。否则，将忽略此字段。然后，通道可以通过分配的硬件握手接口与连接到该接口的目标外设进行通信。注意：为了正确的DW_axi_dmac操作，应仅将一个外设（源或目标）分配给同一握手接口。如果配置参数 DMAX_NUM_HS_IF 设置为 0，则此字段不存在。如果 DMAC_NUM_HS_IF 为 1，则 x = 44 x = ceil（log2（DMAC_NUM_HS_IF）） + 43（如果 DMAC_NUM_HS_IF 大于 1 位 47：（x+1） 不存在，读取时返回 0。 |
| (9)x:4   | <font color=red>**SRC_PER**</font>  | 源硬件握手接口(0 ~ DMAX_NUM_HS_IF - 1)。                     |
| 3：2     | DST_MULTBLK_TYPE                    | 目标多块传输类型。<br />00：连续<br />01：重新装载<br />10：影子寄存器<br />11：链表<br />如果选择的类型是连续的，则CHx_DAR寄存器将在每个块的末尾加载前一个块的最终源地址 + 1 的值，以进行多块传输。然后启动新的区块传输。如果选择的类型为“重新加载”，则CHx_DAR寄存器将从每个块末尾的 DAR 初始值重新加载，以进行多块传输。然后启动新的区块传输。如果选择的类型为“卷影寄存器”（Shadow Register），则CHx_DAR寄存器将从其卷影寄存器的内容（如果CHx_CTL）加载。对于多块传输，每个块末尾的 ShadowReg_Or_LLI_Valid 位设置为 1。然后启动新的区块传输。如果选择的类型是链表，则 CTL 时将从链表加载CHx_DAR寄存器。对于多块传输，每个块末尾的 ShadowReg_Or_LLI_Valid 位设置为 1。然后启动新的区块传输。CHx_CTL 和 CHx_BLOCK_TS 寄存器从其初始值或其影子寄存器的内容（如果CHx_CTL）加载。ShadowReg_Or_LLI_Valid位设置为 1）或来自链表（如果为 CTL.ShadowReg_Or_LLI_Valid位设置为 1），用于基于为源和目标外设编程的多块传输类型进行多块传输的多块传输。源外设和目标外设上的连续传输不是有效的多块传输配置。 |
| 1：0     | SRC_MULTBLK_TYPE                    | 源多块传输类型。<br />00：连续<br />01：重新装载<br />10：影子寄存器<br />11：链表 |













#### CHx_LLP:(0x128 + (x-1) * 0x100)

这是链表指针寄存器。如果启用了基于链表的块链，则必须将此寄存器编程为指向内存中的第一个链表项 （LLI），然后才能启用通道。在 dma 传输的 LLI 更新阶段，此寄存器将使用链表指针的新值进行更新。

| Bits  | Name | 描述                                                         |
| ----- | ---- | ------------------------------------------------------------ |
| 63：6 | LOC  | LLI 块的起始地址内存.如果启用了`Link List`，则下一个 LLI 的起始地址。不存储起始地址的 6 个 LSB，因为假定该地址与 64 字节边界对齐。LLI 访问始终使用与数据总线宽度相同的突发大小 （arsize/awsize），并且不能更改或编程为除此以外的任何值。突发长度（awlen/arlen）是根据数据总线宽度选择的，因此访问不会跨越一个完整的 64 字节 LLI 结构。如果突发长度不受其他设置的限制，DW_axi_dmac 将在一次 AXI 突发中获取整个 LLI（40 字节）。 |
| 0     | LMS  | LLI Master 选择。此位标识存储下一个 LLI 所在的存储设备在需要用哪个 Master 交互。<br />0:AXI Master 1<br />1:AXI Master 2 |









#### CHx_STATUSREG:(0x130 + (x-1) * 0x100)

说明：Channelx 状态寄存器包含指示 Channelx 的 DMA 传输状态的字段。

| Bits  | Name                | 描述                                                         |
| ----- | ------------------- | ------------------------------------------------------------ |
| 46:32 | DATA_LEFT_IN_FIFO   | FIFO 中留下的数据。<br />该位表示完成当前块传输后DW_axi_dmac通道FIFO中剩余的数据总数。通道FIFO中的数据宽度等于`CHx_CTL.SRC_TR_WIDTH`。对于正常的区块传输完成而没有错误，`Data_Left_In_FIFO = 0`。如果在dma传输过程中发生任何错误，则块传输可能会提前终止，在这种情况下，`Data_Left_In_FIFO`指示通道FIFO中剩余的数据无法传输到目标外设。 |
| 21:0  | CMPLTD_BLK_TFR_SIZE | 已完成的块传输大小。<br />该位表示位宽为 `CHx_CTL.SRC_TR_WIDTH `的上一个block transfer 的数据总数.对于正常的 block transfer 完成且没有任何错误，此值等于 CHx_BLOCK_TS.BLOCK_TS 字段。 |



#### CHx_SWHSSRCREG:(0x138 + (x-1) * 0x100)

描述：Channelx Software 握手源寄存器。

| Bits | Name               | 描述                                                         |
| ---- | ------------------ | ------------------------------------------------------------ |
| 5    | SWHS_LST_SRC_WE    | 源通道的最后一次软件握手请求写使能位。<br />0x1：允许写入<br />0x0：禁止写入 |
| 4    | SWHS_LST_SRC       | 源通道的最后一次软件握手请求。<br />0x1：源外设指示 dma 表示当前传输是最后一次传输<br />0x0：不是最后一次传输 |
| 3    | SWHS_SGLREG_SRC_WE | 源通道的单次请求软件握手写使能。<br />0x1：允许写入 SWHS_SGLREG_SRC 位<br />0x0：禁止写入 |
| 2    | SWHS_SGLREQ_SRC    | 源通道的单次软件握手请求。如果相应通道的源选择了软件握手，并且此位为1，用于请求 SINGLE dma 传输(AXI burst 长度 = 1)。<br />0x1：源外设请求 single dma 传输<br />0x0：源外设未请求 single dma 传输 |
| 1    | SWHS_REQ_SRC_WE    | 源通道的软件握手请求写使能。<br />0x1：允许写入<br />0x0：禁止写入 |
| 0    | SWHS_REQ_SRC       | 源通道的软件握手请求。如果相应通道的源选择了软件握手，并且此位为1，用于请求 dma 传输。<br />0x1：源外设请求 dma 传输<br />0x0：源外设未请求 dma 传输 |





#### CHx_SWHSDSTREG:(0x140 + (x-1) * 0x100)

描述：Channelx Software 握手目标寄存器。

| Bits | Name               | 描述                                                         |
| ---- | ------------------ | ------------------------------------------------------------ |
| 5    | SWHS_LST_DST_WE    | 目标通道的最后一次软件握手请求写使能位。<br />0x1：允许写入<br />0x0：禁止写入 |
| 4    | SWHS_LST_DST       | 目标通道的最后一次软件握手请求。<br />0x1：目标外设指示 dma 表示当前传输是最后一次传输<br />0x0：不是最后一次传输 |
| 3    | SWHS_SGLREG_DST_WE | 目标通道的单次请求软件握手写使能。<br />0x1：允许写入 SWHS_SGLREG_SRC 位<br />0x0：禁止写入 |
| 2    | SWHS_SGLREQ_DST    | 目标通道的单次软件握手请求。如果相应通道的目标选择了软件握手，并且此位为1，用于请求 SINGLE dma 传输(AXI burst 长度 = 1)。<br />0x1：目标外设请求 single dma 传输<br />0x0：目标外设未请求 single dma 传输 |
| 1    | SWHS_REQ_DST_WE    | 目标通道的软件握手请求写使能。<br />0x1：允许写入<br />0x0：禁止写入 |
| 0    | SWHS_REQ_DST       | 目标通道的软件握手请求。如果相应通道的目标选择了软件握手，并且此位为1，用于请求 dma 传输。<br />0x1：目标外设请求 dma 传输<br />0x0：目标外设未请求 dma 传输 |





#### CHx_BLK_TFR_RESUMEREQREG:(0x148 + (x-1) * 0x100)

描述： Channelx Block Transfer Resume Request Register。此寄存器在基于链表或影子寄存器的多块传输期间使用。

对于基于链表的多块传输，LLI.CHx_CTL 中的ShadowReg_Or_LLI_Valid位表示从内存中获取的链表项是否有效（0：LLI 无效，1：LLI 有效）。当注意到此位为 0 时，如果相应的通道错误中断掩码位设置为 0，则 DW_axi_dmac 丢弃 LLI 并生成ShadowReg_Or_LLI_Invalid_ERR Interrupt。此错误情况会导致DW_axi_dmac正常停止相应的通道。DW_axi_dmac 等待软件写入（任何值）以CHx_BLK_TFR_ResumeReqReg指示有效的 LLI 可用性，然后再尝试另一个 LLI 读取操作。

对于基于卷影寄存器的多块传输，CHx_CTL寄存器中的ShadowReg_Or_LLI_Valid位指示卷影寄存器内容是否有效（0：卷影寄存器内容无效，1：卷影寄存器内容有效）。在影子寄存器获取阶段注意到此位为 0 时，DW_axi_dmac丢弃影子寄存器内容并生成ShadowReg_Or_LLI_Invalid_ERR中断。DW_axi_dmac 等待软件写入（任何值）以CHx_BLK_TFR_ResumeReqReg指示有效的卷影寄存器可用性，然后再尝试另一个卷影寄存器提取操作并继续下一个块传输。

| Bits | Name              | 描述                                                         |
| ---- | ----------------- | ------------------------------------------------------------ |
| 0    | BLK_TFR_RESUMEREQ | 在基于链表/影子寄存器的多块传输器件，块传输恢复请求。<br />0x0：没有恢复 block 传输请求<br />0x1：有恢复 block 传输请求 |







#### CHx_AXI_IDREG:(0x150 + (x-1) * 0x100)

描述： Channelx AXI ID 寄存器。仅当通道被禁用时，才允许更新此寄存器，这意味着它在整个 DMA 传输中保持固定。

| Bits | Name | 描述 |
| ---- | ---- | ---- |
| 0    |      |      |





#### CHx_SSTAT:(0x160 + (x-1) * 0x100)

描述：Channelx 源状态寄存器。每次块传输完成后，硬件可以从CHx_SSTATAR寄存器内容所指向的地址检索源状态信息。然后，该状态信息存储在CHx_SSTAT寄存器中，并在下一个块开始之前写入 LLI 的CHx_SSTAT寄存器位置。

仅当 DMAX_CHx_LLI_WB_EN = 1 并且为通道的源或目标外设启用了基于链表的多块传输时，才会执行对 LLI CHx_SSTAT寄存器位置的源状态写回。

如果 DMAC_CHx_SRC_STAT_EN 设置为 False，则此寄存器不存在;在这种情况下，回读值始终为 0。

| Bits  | Name  | 描述                                                         |
| ----- | ----- | ------------------------------------------------------------ |
| 31：0 | SSTAT | 源状态。<br />硬件从 CHx_SSTATAT 寄存器内容所指向的地址检索的源状态信息。源外设应该在 CHx_SSTATAR 指向的位置更新原状态信息。此状态与DW_axi_dmac的任何状态都无关。 |



#### CHx_SSTAT:(0x168 + (x-1) * 0x100)

描述：Channelx 目标状态寄存器。每次块传输完成后，硬件可以从CHx_DSTATAR寄存器内容所指向的地址检索源状态信息。然后，该状态信息存储在CHx_DSTAT寄存器中，并在下一个块开始之前写入 LLI 的CHx_DSTAT寄存器位置。

仅当 DMAX_CHx_LLI_WB_EN = 1 并且为通道的源或目标外设启用了基于链表的多块传输时，才会执行对 LLI CHx_DSTAT寄存器位置的源状态写回。

如果 DMAC_CHx_DST_STAT_EN 设置为 False，则此寄存器不存在;在这种情况下，回读值始终为 0。

| Bits  | Name  | 描述                                                         |
| ----- | ----- | ------------------------------------------------------------ |
| 31：0 | DSTAT | 目标状态。<br />硬件从 CHx_DSTATAT 寄存器内容所指向的地址检索的目标状态信息。目标外设应该在 CHx_DSTATAR 指向的位置更新目标状态信息。此状态与DW_axi_dmac的任何状态都无关 |



#### CHx_SSTATAR:(0x170 + (x-1) * 0x100)

描述： Channelx 源状态提取寄存器。在完成每个块传输后，硬件可以从用户定义的地址检索源状态信息，CHx_SSTATAR寄存器的内容指向该地址。您可以选择系统内存中提供 64 位值的任何位置来指示源传输的状态。

如果 DMAC_CHx_SRC_STAT_EN 设置为 False，则此寄存器不存在;在这种情况下，回读值始终为 0。

| Bits  | Name    | 描述                                                         |
| ----- | ------- | ------------------------------------------------------------ |
| 63：0 | SSTATAR | 源状态获取地址指针 硬件可以从中获取源状态信息，如果 DMAX_CHx_LLI_WB_EN = 1，则在下一个块开始之前将源状态信息注册到CHx_SSTAT寄存器中，并写出到 LLI 的CHx_SSTAT寄存器位置，并且为通道的源或目标外围设备启用了基于链表的多块传输。<br />源外围设备应在CHx_SSTATAR指向的位置更新源状态信息（如果有），以使用此功能。此状态与DW_axi_dmac的任何内部状态无关。 |





#### CHx_DSTATAR:(0x178 + (x-1) * 0x100)

描述：Channelx 目标状态提取寄存器。完成每个块传输后，硬件可以从用户定义的地址检索目标状态信息，CHx_DSTATAR的内容将指向该地址。您可以在系统内存中选择任何位置，该位置将提供 64 位值来指示目标传输的状态。

如果 DMAC_CHx_SRC_STAT_EN 设置为 False，则此寄存器不存在;在这种情况下，回读值始终为 0。

| Bits  | Name    | 描述                                                         |
| ----- | ------- | ------------------------------------------------------------ |
| 63：0 | SSTATAR | 目标状态获取地址指针，硬件可以从中获取目标状态信息，如果DMAX_CHx_LLI_WB_EN = 1，则在下一个块开始之前，该信息在CHx_DSTAT寄存器中注册并写出到LLI的CHx_DSTAT寄存器位置，并且为通道的源或目标外围设备启用了基于链表的多块传输。目标外围设备应在CHx_DSTATAR指向的位置更新目标状态信息（如果有），以使用此功能。此状态与DW_axi_dmac的任何内部状态无关。 |





#### CHx_INTSTATUS_ENABLEREG:(0x180 + (x-1) * 0x100)：使能中断

说明：将 1 写入特定字段可在 Channelx 中断状态寄存器 （CH1_IntStatusReg） 中生成相应的中断状态。

| Bits | Name                                          | 描述                                                         |
| ---- | --------------------------------------------- | ------------------------------------------------------------ |
| 31   | Enable_CH_ABORTED_IntStat                     | 通道 abort 状态启用。<br />0x1：在 CH1_INTSTATUSREG 中启用通道 abort 中断的生成<br />0x0：禁用 |
| 30   | Enable_CH_DISABLED_IntStat                    | 通道禁用状态启用。<br />0x1：启用通道禁用中断的生成<br />0x0：禁用通道禁用中断的生成 |
| 29   | Enable_CH_SUSPENDED_IntStat                   | 通道 suspend 状态启用。<br />0x1：启用通道 suspend 中断的生成<br />0x0：禁用通道 suspend 中断的生成 |
| 28   | Enable_CH_SRC_SUSPENDED_IntStat               | 通道源 suspend 状态启用。                                    |
| 27   | Enable_CH_LOCK_CLEARED_IntStat                | 通道 lock 已清除状态启用。                                   |
| 21   | Enable_SLVIF_WRONHOLD_ERR_IntStat             | 从接口保持写入状态启用                                       |
| 20   | Enable_SLVIF_SHADOWREG_WRON_VALID_ERR_IntStat | 影子寄存器写入有效错误状态启用。                             |
| 19   | Enable_SLVIF_WRONCHEN_ERR_IntStat             | 从接口在通道上写入已启用错误状态启用。                       |
| 18   | Enable_SLVIF_RD2RWO_ERR_IntStat               | 从接口只读写错误中断启用。                                   |
| 17   | Enable_SLVIF_WR2RO_ERR_IntStat                | 从接口写入只读错误状态启用。                                 |
| 16   | Enable_SLVIF_DEC_ERR_IntStat                  | 从接口解码错误状态启用                                       |
| 14   | Enable_SLVIF_MULTIBLKTYPE_ERR_IntStat         | 从接口多块类型错误状态启用                                   |
| 13   | Enable_SHADOWREG_OR_LLI_INVALID_ERR_IntStat   | 影子寄存器/LLI 无效错误状态启用                              |
| 12   | Enable_LLI_WR_SLV_ERR_IntStat                 | LLI Write Slave Error 状态启用                               |
| 11   | Enable_LLI_RD_SLV_ERR_IntStat                 | LLI Read Slave Error Status Enable                           |
| 10   | Enable_LLI_WR_DEC_ERR_IntStat                 | LLI Write Decode 错误状态启用                                |
| 9    | Enable_LLI_RD_DEC_ERR_IntStat                 | LLI 读取解码错误状态启用                                     |
| 8    | Enable_DST_SLV_ERR_IntStat                    | 目标 slave 错误状态启用                                      |
| 7    | Enable_SRC_SLV_ERR_IntStat                    | 源 slave 错误状态启用                                        |
| 6    | Enable_DST_DEC_ERR_IntStat                    | 目标解码错误状态启用                                         |
| 5    | Enable_SRC_DEC_ERR_IntStat                    | 源解码错误状态启用                                           |
| 4    | Enable_DST_TRANSCOMP_IntStat                  | 目标传输完成状态启用                                         |
| 3    | Enable_SRC_TRANSCOMP_IntStat                  | 源传输完成状态启用                                           |
| 1    | Enable_DMA_TFR_DONE_IntStat                   | DMA 传输完成中断状态启用                                     |
| 0    | Enable_BLOCK_TFR_DONE_IntStat                 | block 传输完成中断状态启用                                   |







#### CHx_INTSTATUS:(0x188 + (x-1) * 0x100)：中断状态寄存器

说明： Channelx 中断状态寄存器捕获 Channelx 特定的中断

| Bits | Name                                   | 描述                                                         |
| ---- | -------------------------------------- | ------------------------------------------------------------ |
| 31   | CH_ABORTED_IntStat                     | channel abort 中断。如果启用了 CHx_INTSTATUS_ENABLEReg 中的相应位，则生成错误中断。在寄存器中将 1 写入相应的通道中断清除位时，该位被清除。CHx_IntClearReg 为 0. |
| 30   | CH_DISABLE_IntStat                     | channel 已禁用中断。这向软件表示 DW_axi_dmac 中的相应通道已禁用。 |
| 29   | CH_SUSPEND_IntStat                     | channel 挂起中断。                                           |
| 28   | CH_SRC_SUSPENDED_IntStat               | src channel 挂起中断。                                       |
| 27   | CH_LOCK_CLEARED_IntStat                | 通道锁定已清除                                               |
| 21   | SLVIF_WRONHOLD_ERR_IntStat             |                                                              |
| 20   | SLVIF_SHADOWRED_WRON_VALID_ERR_IntStat |                                                              |
| 19   | SLVIF_WRONCHEN_ERR_IntStat             |                                                              |
| 18   | SLVIF_RD2RWO_ERR_IntStat               |                                                              |
| 17   | SLVIF_WR2RO_ERR_IntStat                |                                                              |
| 16   | SLVIF_DEC_ERR_IntStat                  |                                                              |
| 14   | SLVIF_MULTIBLKTYPE_ERR_IntStat         |                                                              |
| 13   | SHADOWREG_OR_LLI_INVALID_ERR_IntStat   |                                                              |
| 12   | LLI_WR_SLV_ERR_IntStat                 |                                                              |
| 11   | LLI_RD_SLV_ERR_IntStat                 |                                                              |
| 10   | LLI_WR_DEC_ERR_IntStat                 |                                                              |
| 9    | LLI_RD_DEC_ERR_IntStat                 |                                                              |
| 8    | DST_SLV_ERR_IntStat                    |                                                              |
| 7    | SRC_SLV_ERR_IntStat                    |                                                              |
| 6    | DST_DEC_ERR_IntStat                    |                                                              |
| 5    | SRC_DEC_ERR_IntStat                    |                                                              |
| 4    | DST_TRANSCOMP_IntStat                  | 目标传输已完成。                                             |
| 3    | SRC_TRANSCOMP_IntStat                  | 源传输已完成                                                 |
| 1    | DMA_TFR_DONE_IntStat                   | DMA 传输完成                                                 |
| 0    | BLOCK_TFR_DONE_IntStat                 | block 传输完成                                               |





#### CHx_INTSIGNAL_ENABLEREG:(0x190 + (x-1) * 0x100)：使能产生端口级别的中断

- <font color=red>**外设级别的中断，此寄存器的默认值是 `0xffffffff` ，相当于系统复位后，默认开启所有的中断**</font>
- 产生中断后也还是看 0x188 寄存器中的中断状态



#### CHx_INTCLEARREG:(0x198 +(x-1) * 0x100)：写 1 清除中断



# 编程流程

## 基于影子寄存器的多块传输的编程流程



## 基于链表的多 block 传输的编程流程

1. 软件读取 DMAC 通道使能寄存器(DMAC_ChEnReg) 以选择可用通道。
2. 软件编程 `CHx_CFG`  寄存器具有适当地 DMA 传输值。`SRC_MLTBLK_TYPE、DST_MLTBLK_TYPE` 必须设置为 `2'b 11`
3. 软件编程 第一个链表项的基地址和 `CHx_LLP.lms` 。
4. 软件在系统内存中创建一个或多个链表项。软件可以提前创建整个链表项，也可以使用`CHx_CTL.ShadowReg_Or_LLI_Valid and CHx_CTL.LLI_Last` 字段动态扩展链表项。
5. 软件通过将 1 写入`DMAC_ChEnReg`寄存器中的适当位置来启用通道。

> 可以交换步骤4和步骤5的顺序。但是，如果步骤5在步骤4之前执行，或者如果在多block传输器件的任何时候系统内存中都不提供下一个block传输的链表项，如`CHx_CTL.ShadowReg_Or_LLI_Valid` 位将提取 LLI 设置为 0，`DW_axi_dmac` 可能会生成 `ShadowReg_Or_LLI_Invalid_ERR` 中断。

6. DW_axi_dmac 根据块传输的设置启动 dma 块传输操作。块传输可能立即开始，也可能在硬件或软件握手请求之后开始，具体取决于 `CHx_CFG.TT_FC`字段的设置。`DW_axi_dmac` 将链表内容复制到用于执行 DMA 块传输的寄存器(即 `CHx_SAR、CHx_DAR、CHx_BLOCK_TS、CHx_CTL` 寄存器)并启动 DMA 块传输。
7. 在链表获取阶段：
   1. 如果 DW_axi_dmac 看到 `CHx_CTL.ShadowReg_Or_LLI_Last` 获取到  LLI 为 1，它理解当前块是传输中的最后一个块，并在当前 block 传输结束时完成 DMA 传输操作。
   2. 如果 DW_axi_dmac 看到 `CHx_CTL.ShadowReg_Or_LLI_Last` 获取到 LLI 为 0，它了解有一个或多个块要传输，并转到步骤 6.
   3. 如果 DW_axi_dmac 看到 `CHx_CTL.ShadowReg_Or_LLI_Valid` 获取到 LLI 为 0，DW_axi_dmac 可能会生成 `ShadowReg_Or_LLI_Invalid_ERR` 中断。`DW_axi_dmac` 等待软件写入(任何值)到`CHx_BLK_TFR_ResumeReqReg` 表示LLI 独立的有效性，然后在尝试另一个 LLI 读操作。



> 1. LLI 必须 64 字节对齐
> 2. `CHx_LLP_STATUS[63] 、CHx_LLP_STATUS[62]` 表示 `DMA_TFR_DONE、BLOCK_TFR_DONE` 状态响应。





## 单块传输的编程流程

1. 软件读取 DMAC 通道使能寄存器`DMAC_ChEnReg` 已选择空闲通道
2. 软件编程 `CHx_CFG` 寄存器的源和目标外设的多块类型值都为 `2'b 00`
3. 软件编程 `CHx_SAR、CHx_DAR、CHx_BLOCK_TS、CHx_CTL` 寄存器合适的值。
4. 软件将 `1` 写入 `DMAC_ChEnReg` 寄存器适当位置来启用通道
5. 源和目标请求 `single / burst` dma 传输来传输数据块(假设为非内存外设)。DW_axi_dmac 会在block的每个传输(single/burst)完成时确认，并执行数据块传输
6. 软件等待块传输完成中断/轮询`CHx_IntStatusReg` 寄存器中的块传输完成指示位`BLOCK_TFR_DONE` ，直到该位为 1.







# Linux AXI DMA



## dmaengine.h

- `struct dma_chan` ：这是套接层的 dma channel，用于在外面给用户相同的套阶层。axi dma 驱动里会把这个转成 `struct axi_dma_chan`。



## dmaengine.c

```c
static inline int dmaengine_slave_config(struct dma_chan *chan,
					  struct dma_slave_config *config)
{
	if (chan->device->device_config)
		return chan->device->device_config(chan, config);

	return -ENOSYS;
}



static inline struct dma_async_tx_descriptor *dmaengine_prep_slave_single(
	struct dma_chan *chan, dma_addr_t buf, size_t len,
	enum dma_transfer_direction dir, unsigned long flags)
{
	struct scatterlist sg;
	sg_init_table(&sg, 1);
	sg_dma_address(&sg) = buf;
	sg_dma_len(&sg) = len;

	if (!chan || !chan->device || !chan->device->device_prep_slave_sg)
		return NULL;

	return chan->device->device_prep_slave_sg(chan, &sg, 1,
						  dir, flags, NULL);
}



static inline dma_cookie_t dmaengine_submit(struct dma_async_tx_descriptor *desc)
{
	return desc->tx_submit(desc);
}

```





## virt-dma

```c
static inline struct dma_async_tx_descriptor *vchan_tx_prep(struct virt_dma_chan *vc,
	struct virt_dma_desc *vd, unsigned long tx_flags)
{
	unsigned long flags;

	dma_async_tx_descriptor_init(&vd->tx, &vc->chan);
	vd->tx.flags = tx_flags;
	vd->tx.tx_submit = vchan_tx_submit;
	vd->tx.desc_free = vchan_tx_desc_free;

	vd->tx_result.result = DMA_TRANS_NOERROR;
	vd->tx_result.residue = 0;

	spin_lock_irqsave(&vc->lock, flags);
	list_add_tail(&vd->node, &vc->desc_allocated);
	spin_unlock_irqrestore(&vc->lock, flags);

	return &vd->tx;
}

```







## dw-axi-dmac-platform.c

- probe

  ```c
  
  	dw->dma.device_prep_dma_memcpy = dma_chan_prep_dma_memcpy;
  	dw->dma.device_synchronize = dw_axi_dma_synchronize;
  	dw->dma.device_config = dw_axi_dma_chan_slave_config;
  	dw->dma.device_prep_slave_sg = dw_axi_dma_chan_prep_slave_sg;
  	dw->dma.device_prep_dma_cyclic = dw_axi_dma_chan_prep_cyclic;
  
  
  ```

  

- link list

  ```c
  struct scatterlist {
  	unsigned long	page_link;
  	unsigned int	offset;
  	unsigned int	length;
  	dma_addr_t	dma_address;
  #ifdef CONFIG_NEED_SG_DMA_LENGTH
  	unsigned int	dma_length;
  #endif
  };
  
  /*
   * These macros should be used after a dma_map_sg call has been done
   * to get bus addresses of each of the SG entries and their lengths.
   * You should only work with the number of sg entries dma_map_sg
   * returns, or alternatively stop on the first sg_dma_len(sg) which
   * is 0.
   */
  #define sg_dma_address(sg)	((sg)->dma_address)
  
  #ifdef CONFIG_NEED_SG_DMA_LENGTH
  #define sg_dma_len(sg)		((sg)->dma_length)
  #else
  #define sg_dma_len(sg)		((sg)->length)
  #endif
  
  struct sg_table {
  	struct scatterlist *sgl;	/* the list */
  	unsigned int nents;		/* number of mapped entries */
  	unsigned int orig_nents;	/* original size of list */
  };
  
  struct sg_append_table {
  	struct sg_table sgt;		/* The scatter list table */
  	struct scatterlist *prv;	/* last populated sge in the table */
  	unsigned int total_nents;	/* Total entries in the table */
  };
  
  
  
  struct scatterlist *sg;
  
  mem = sg_dma_address(sgl);
  len = sg_dma_len(sgl);
  
  axi_block_len = calculate_block_len(chan, mem, len, direction);
  
  for_each_sg(sgl, sg, sg_len, i)
      num_sgs += DIV_ROUND_UP(sg_dma_len(sg), axi_block_len);
  
  for_each_sg(sgl, sg, sg_len, i) {
      mem = sg_dma_address(sg);
      len = sg_dma_len(sg);
      num_segments = DIV_ROUND_UP(sg_dma_len(sg), axi_block_len);
      segment_len = DIV_ROUND_UP(sg_dma_len(sg), num_segments);
      
      do {
          hw_desc = &dwsc->hw_desc[loop++];
          status = dw_axi_dma_set_hw_desc(chan, hw_desc, mem, sugment_len);
          if (status < 0)
              goto err_desc_get;
          dwsc->length += hw_desc->len;
          len -= segment_len;
          mem += segment_len;
      } while (len >= segment_len);
  }
  
  set_desc_last(&desc->hw_desc[num_sgs - 1]);
  
  do {
      hw_desc = &dwsc->hw_desc[--num_sgs];
      write_desc_llp(hw_desc, llp | lms);
      llp = hw_desc->llp;
  } while (num_sgs);
  
  dw_axi_dma_set_hw_channel(chan, true);
  
  return vchan_tx_prep(&chan->vc, &desc->vd, flags);
  
  
  
  
  
  // 从内存申请一片内存，这篇内存的起始地址由 dw_desc->lli 保存，内存的物理地址由 hw_desc->llp 保存
  dw_desc->lli = axi_desc_get(chan, &hw_desc->llp);
  
  hw_desc->lli->ctl_hi = ctlhi;
  hw_desc->lli->sar = src_addr;
  hw_desc->lli->dst = dst_addr;
  hw_desc->lli->block_ts_lo = block_ts - 1;
  hw_desc->lli->ctl_lo = ctllo;
  hw_desc->lli->ctl_lo sms = ;
  hw_desc->len = len;
  
  
  
  
  static struct dma_async_tx_descriptor *
  dw_axi_dma_chan_prep_slave_sg(struct dma_chan *dchan, struct scatterlist *sgl,
  			      unsigned int sg_len,
  			      enum dma_transfer_direction direction,
  			      unsigned long flags, void *context)
  {
  	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);
  	struct axi_dma_hw_desc *hw_desc = NULL;
  	struct axi_dma_desc *desc = NULL;
  	u32 num_segments, segment_len;
  	unsigned int loop = 0;
  	struct scatterlist *sg;
  	size_t axi_block_len;
  	u32 len, num_sgs = 0;
  	unsigned int i;
  	dma_addr_t mem;
  	int status;
  	u64 llp = 0;
  	u8 lms = 0; /* Select AXI0 master for LLI fetching */
  
  	if (unlikely(!is_slave_direction(direction) || !sg_len))
  		return NULL;
  
  	mem = sg_dma_address(sgl);
  	len = sg_dma_len(sgl);
  
  	axi_block_len = calculate_block_len(chan, mem, len, direction);
  	if (axi_block_len == 0)
  		return NULL;
  
  	for_each_sg(sgl, sg, sg_len, i)
  		num_sgs += DIV_ROUND_UP(sg_dma_len(sg), axi_block_len);
  
  	desc = axi_desc_alloc(num_sgs);
  	if (unlikely(!desc))
  		goto err_desc_get;
  
  	desc->chan = chan;
  	desc->length = 0;
  	chan->direction = direction;
  
  	for_each_sg(sgl, sg, sg_len, i) {
  		mem = sg_dma_address(sg);
  		len = sg_dma_len(sg);
  		num_segments = DIV_ROUND_UP(sg_dma_len(sg), axi_block_len);
  		segment_len = DIV_ROUND_UP(sg_dma_len(sg), num_segments);
  
  		do {
  			hw_desc = &desc->hw_desc[loop++];
  			status = dw_axi_dma_set_hw_desc(chan, hw_desc, mem, segment_len);
  			if (status < 0)
  				goto err_desc_get;
  
  			desc->length += hw_desc->len;
  			len -= segment_len;
  			mem += segment_len;
  		} while (len >= segment_len);
  	}
  
  	/* Set end-of-link to the last link descriptor of list */
  	set_desc_last(&desc->hw_desc[num_sgs - 1]);
  
  	/* Managed transfer list */
  	do {
  		hw_desc = &desc->hw_desc[--num_sgs];
  		write_desc_llp(hw_desc, llp | lms);
  		llp = hw_desc->llp;
  	} while (num_sgs);
  
  	dw_axi_dma_set_hw_channel(chan, true);
  
  	return vchan_tx_prep(&chan->vc, &desc->vd, flags);
  
  err_desc_get:
  	if (desc)
  		axi_desc_put(desc);
  
  	return NULL;
  }
  
  
  
  static void dma_chan_issue_pending(struct dma_chan *dchan)
  {
  	struct axi_dma_chan *chan = dchan_to_axi_dma_chan(dchan);
  	unsigned long flags;
  
  	spin_lock_irqsave(&chan->vc.lock, flags);
  	if (vchan_issue_pending(&chan->vc))
  		axi_chan_start_first_queued(chan);
  	spin_unlock_irqrestore(&chan->vc.lock, flags);
  }
  
  ```





## drivers/tty/serial/8250/8250_dma.c

- 串口使用 dmaengine 调用到 axi-dma

  ```c
  dma_request_slave_channel_compat               // 申请一个通道
  
  dma_get_slave_caps(dma->rxchan, &caps);
  
  dma_engine_slave_config(dma->rxchan, &dma->rxconf);  // 把配置信息传给 dw-axi-dma
  
  dma->rx_buf = dma_alloc_coherent(dma->rxchan->device->dev, dma->rx_size, &dma->rx_addr, GFP_KERNEL);
  
  dma->tx_addr = dma_map_single(dma->txchan->device->dev, p->port.state->xmit.buf, UADT_XMIT_SIZE, DMA_TO_DEVICE);
  ```

- tx

  ```c
  dmaengine_prep_slave_single(dma->txchan, dma->tx_addr + axmit->tail, dma->tx_size, DMA_MEM_TO_DEV, INTERRUPT);
  
  dma->tx_cookie = dmaengine_submit(desc);
  
  dma_sync_single_for_device(dma->txchan->device->dev, dma->txa_addr, UADT_XMIT_SIZE, DMA_TO_DEVICE);
  
  dma_async_issue_pending(dma->txchan);
  ```

  





## 流程

1. 分配 DMA 从通道：`dma_request_chan`
2. 设置从站和控制器特定参数：`dmaengine_slave_config`
3. get a descrpitor for transaction：`dmaengine_prep_slave_sg`
4. Submit a transaction：`dma_cookie_t dmaengine_submit(struct dma_async_tx_descriptor *desc)`
5. Issue pengding DMA requests and wait for callback notification:`void dma_async_issue_pending(struct dma_chan *chan)`

6. use `dma_async_is_tx_complete` get transfer done?





```c
// 1. dma_reqeust_chan
	dw->dma.device_alloc_chan_resources = dma_chan_alloc_chan_resources; // 分配一个用于存储 dma desc 的内存池 
																		 // struct axi_dma_chan *chan->desc_pool
// 2. dmaengine_slave_config
	dw->dma.device_config = dw_axi_dma_chan_slave_config;	// 用于将用户层配置的 trans_width、之类的传递给 dw-axi-dma

// 3. dma_engine_prep_slave_sg
	dw->dma.device_prep_slave_sg = dw_axi_dma_chan_prep_slave_sg;
	static struct dma_async_tx_descriptor *dw_axi_dma_chan_prep_slave_sg(
    	struct dma_chan *dchan, struct scatterlist *sgl, unsigned int sg_len,
    	emum dma_transfer_direction direction, unsigned long flags, void *context)
        
        // 准备好 addr、len、direction、flags
        struct scatterlist sg;
		sg->addr = buf;
		sg->len = len;

		return chan->device->device_prep_slave_sg(chan, &sg, 1, dir, flags, NULL)
            
            
            
    // 从 sgl 中取出 mem、len，这代表要传输的首地址和总长度
	mem = sg_dma_address(sgl);
	len = sg_dma_len(sgl);

	// 算 block len?

	for_each_sg(sgl, sg, sg_len, i) {
        mem = sg_dma_address(sg);
        len = sg_dma_len(sg);
        num_segments = DIV_ROUND_UP(sg_dma_len(sg), axi_block_len);
        segment_len = DIV_ROUND_UP(sg_dma_len(sg), num_segments);
        
        do {
            hw_desc = &desc->hw_desc[loop++];
            status = dw_axi_dma_set_hw_desc(chan, hw_desc, mem, segment_len);
            if (status < 0)
                goto err_desc_get;
            desc->length += hw_desc->len;
            len -= segment_len;
            mem += segment_len;
        } while (len >= segment_len);
    }




	// 从 dma 内存池申请了一块空间，空间的指针返回给 lli，空间的物理地址返回给 llp
	hw_desc->lli = axi_desc_get(chan, &hw_desc->llp);
	hw_desc->lli->ctl_hi = ctlhi;
	hw_desc->lli->src = src;
	hw_desc->lli->dst = dst;
	hw_desc->lli->block_ts = block_ts; // 根据 len 计算得来的
	hw_desc->lli->ctl = ctl | master;
	

	hw_desc->len = len;





```







# Channel Suspend，Disable, and Abort

正常操作下，软件通过写1到 `channel enable register(DMAC_ChEnReg.CH_EN)` 使能一个通道，而硬件传输时完成通过清零 `DMAC_ChReg.CH_EN`寄存器来禁用通道。

软件可以在传输完成之前暂停、禁用或中止通道。以下各节介绍了 suspend、disable 和 abort 过程。

## Channel Suspend

要在 DMA 传输期间 suspend channel：

1. 软件写 1 到 channel suspend 在通道使能寄存器`DMAC_ChEnReg.CH_SUSP`

2. 在完成源外设上所有已初始化的 AXI 传输后，DW_axi_dmac 优雅的停止来自源外设的所有传输

3. DW_axi_dmac 将 `CHx_IntStatusReg.CH_SRC_SUSPENDED` bit 设置为 1 ，以指示源数据传输 suspend，如果未屏蔽，则生成中断。

   > 如果 channel FIFO 已满，并且目标外设未请求数据传输，DW_axi_dmac 无法在相应的主接口上接收更多数据，这可能会导致死锁。
   >
   > - 由源/目标/LLI 状态机发起的、存在于主接口读取地址通道和写入地址通道 FIFO 中的请求，要在 AXI 主接口上发送，即使启动了通道 suspend 请求，也将在 AXI 主接口上发送。根据主接口读取地址和写入地址通道 FIFO 深度配置，最多可以发起 8 个读/写请求，并且 DW_axi_dmac 在suspend 通道之前也会等待这些请求的 data 响应。

4. DW_axi_dmac 将在通道 FIFO 中的所有数据传输到目标外设

   当 `CHx_CTL.SRC_TR_WIDTH < CHx_CTL.DST_TR_WIDTH` 并且 `DMAC_ChEnReg.CH_SUSP` bit 是高电平，则通道 FIFO 中可能仍有数据，但不足以形成 `CHx_CTL.DST_TR_WIDTH` 的单次传输。如果稍后恢复通道，则通道 FIFO 中剩余的数据将传输给目标，这会导致在通道 FIFO 中填充更多数据

5. DW_axi_dmac 清除通道锁定并重置 `CHx_CFG` 寄存器中的通道锁定设置
6. DW_axi_dmac 将 `CHx_IntStatusReg.ChLock_Cleared` 位设置为 1，以指示通道锁定已清除
7. DW_axi_dmac 将 `CHx_IntStatusReg.CH_SUSPENDED` 位设置为 1，以指示通道已暂停
8. DW_axi_dmac 会生成 `CH_SUSPENDED 中断`（如果未屏蔽）。

通道 suspend 后，软件可以在一段时间后恢复通道，也可以禁用通道。



## Channel Suspend and Resume

要暂停和恢复通道：

1. 按照 Channel Suspend 中的步骤 1 到 4 进行

2. 软件将 0 写入 `channel enable register(DMAC_ChEnReg.CH_SUSP)`

3. DW_axi_dmac 将从 DMA 暂停的点恢复 DMA 传输

   > 一旦软件通过将 1 写入通道挂起位 `DMAC_ChEnReg.CH_SUSP` 来启动通道挂起程序，`CHx_IntStatusReg.CH_SUSPENDED`位断言之前不允许将 0 写入 `DMAC_ChEnReg.CH_SUSP` 位恢复通道





## Channel Suspend and Disable Prior to Transfer Completion

要暂定和禁用通道：

1. 按照 Channel Suspend 中的步骤 1 到 4 进行操作

2. 使用软件来禁用 suspend channel，DW_axi_dmac 断言到 `CHx_IntStatusReg.CH_SUSPENDED` 位为 1 之后写 0 到 `DMAC_ChEnReg.CH_EN` 位以指示通道已暂停

   当 `CHx_CTL.SRC_TR_WIDTH < CHx_CTL.DST_TR_WIDTH` 并且 `DMAC_ChEnReg.CH_SUSP` 位是高电平，则通道 fifo 中可能仍有数据，但不足以形成 `CHx_CTL.DST_TR_WIDTH` 的单次传输。在这种情况下，一旦通道被禁用，通道 FIFO 中的剩余数据不会传输到目标外设，并且会丢失

3. DW_axi_dmac 将 `CHx_IntStatusReg.CH_DISABLED` 位设置为 1 以指示通道已禁用
4. DW_axi_dmac 会生成 `CH_DISABLED` 中断(如果未屏蔽)
5. DW_axi_dmac 将 `DMAC_ChEnReg.CH_EN`位清零





## Channel Disable Prior to Transfer Completion without Suspend

要禁用通道而不暂停：

1. 软件将 0 写入通道使能寄存器 `DMAC_ChEnReg.CH_EN`位

2. DW_axi_dmac 在完成源外设上启动的所有 AXI 传输后，优雅的停止来自源外设的所有传输

   > - 如果通道 FIFO 已满，并且目标外设未请求数据传输，则 DW_axi_dmac 无法在相应的主接口上接收更多数据，这可能导致死锁。
   > - 由源/目标/LLI状态机发起的、存在于主接口读取地址通道和写入地址通道 FIFO 中的请求，要在 AXI 主接口上发送，即使启动了通道挂起请求，也将在AXI 主接口上发送。根据主接口读取地址和写入地址通道 FIFO 深度配置，最多可以发起 8 个读/写请求，DW_axi_dmac 在暂停通道之前也等待这些请求的数据/响应。

3. DW_axi_dmac 将通道 FIFO 中的所有数据传输到目标外设

   当 `CHx_CTL.SRC_TR_WIDTH < CHx_CTL.DST_TR_WIDTH` 并且 `DMAC_ChEnReg.CH_SUSP` 位是高电平，则通道 fifo 中可能仍有数据，但不足以形成 `CHx_CTL.DST_TR_WIDTH` 的单次传输。在这种情况下，一旦通道被禁用，通道 FIFO 中的剩余数据不会传输到目标外设，并且会丢失

4. DW_axi_dmac 清除通道锁定并重置 `CHx_CFG` 寄存器中的通道锁定设置
5. DW_axi_dmac 将 `CHx_IntStatusReg.ChLock_Cleared` 位设置为 1，以指示通道锁定已清除
6. DW_axi_dmac 将 `CHx_IntStatusReg.CH_DISABLED` 位设置为 1，以指示通道已暂停
7. DW_axi_dmac 会生成 `CH_DISABLED 中断`（如果未屏蔽）
8. DW_axi_dmac 将 DMAC_ChEnReg.CH_EN 位清零





## Abnormal Channel Abort

中止通道不是推荐的过程。仅当软件想要在不重置整个 DW_axi_dmac 的情况下禁用通道时，才应使用此过程，例如，如果特定通道由于未收到来自相应握手界面的相应而挂起。在中止通道之前，建议先尝试禁用通道。

要中止通道：

1. 软件将 1 写入`DMAC_ChEnReg.CH_ABORT` 位
2. DW_axi_dmac 在完成源/目标外设上启动的所有 AXI 传输后，正常停止来自源/目标外设的所有传输。
3. Channel FIFO 中的数据被刷新，基本上是丢失的。
4. DW_axi_dmac 清除通道锁定并重置 CHx_CFG 寄存器中的通道锁定设置。
5. DW_axi_dmac 将 CHx_IntStatusReg.ChLock_Cleared 位设置为 1，以指示通道锁定已清除。
6. DW_axi_dmac 将 CHx_IntStatusReg.CH_ABORTED 位设置为 1 以指示通道已中止。
7. 如果未屏蔽CH_ABORTED则 DW_axi_dmac 会生成中断。
8. DW_axi_dmac 将 DMAC_ChEnReg.CH_EN 位清除为 0。




# Linux Module Param

记录 Linux dmatest.c 中的 Param 处理方式:
```c

struct kernel_param {
	const char *name;
	struct module *mod;
	const struct kernel_param_ops *ops;
	const u15 perm;
	s7 level;
	u7 flags;
	union {
		void *arg;
		const struct kparam_string *str;
		const struct kparam_array *arr;
	};
};

static int dmatest_run_set(const char *val, const struct kernel_param *kp);
static int dmatest_run_get(char *val, const struct kernel_param *kp);
static const struct kernel_param_ops run_ops = {
	.set = dmatest_run_set,
	.get = dmatest_run_get,
};
static bool dmatest_run;
module_param_cb(run, &run_ops, &dmatest_run, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(run, "Run the test (default: false)");
```

- `MODULE_PARM_DESC` 提供了的描述信息，这会显示在模块文档中
- `module_param_cb` 实质上就是初始化了 `struct kernel_param` 这样的一个结构体，`dmatest_run` 作为 `arg`
- 调用 `dmatest_run_set` 接口时，会将用户传进来的 val 赋值给 `arg` 也就是 `dmatest_run`
- 调用 `dmatest_run_get` 接口时，会根据 `arg` 也就是 `dmatest_run` 的值，返回给用户
- `kernel_param_ops` 的 `set` 和 `get` 函数指针分别指向 `dmatest_run_set` 和 `dmatest_run_get` 函数

- 因此，当模块加载后，可以通过 `echo 0 > /sys/module/dmatest/parameters/run` 来设置 `dmatest_run` 的值，通过 `cat /sys/module/dmatest/parameters/run` 来获取 `dmatest_run` 的值

# Linux dmatest.c

## init

- 初始化流程:
  1. 检查 `dmatest_run` 变量状态，若有效则执行 `run_thread_test`
  2. `run_thread_test` 主要做两件事:
     - 参数初始化: 
       - 通过 `module_param_cb` 获取用户配置的参数，存入全局变量
       - 使用全局变量给 `struct dmatest_info *info->params` 赋值
       - 后续操作都基于 info 结构体进行
     - 通道请求: 调用 `request_channels` 依次请求 3 种类型的 DMA 通道:
       - `DMA_MEMCPY`: 内存拷贝
       - `DMA_XOR`: 异或运算
       - `DMA_SG`: 分散/聚集传输
       - `DMA_PQ`: P+Q 校验
     - 通道请求流程:
       1. 使用 `dma_cap_zero()` 清空 caps
       2. 使用 `dma_cap_set()` 设置对应通道类型的 caps
       3. 调用 `dma_request_channel()` 申请通道
       4. 若申请成功,调用 `dmatest_add_channel()` 将申请成功的通道创建测试线程
          1. 每个通道都根据 `caps` 创建对应的测试线程
            1. 用 `char *op` 来保存每个通道的 `caps` 
            2. 创建线程：`dmatest_func`，将通道特有的参数传进去
               1. 组织好 buffer、addr、len，都是通过 `dma_map_page` 或者 `sg` 相关接口将 `addr、len` 转换成 `sg` 相关的
               2. addr 和 len 都映射到 DMA 缓冲区 `dmaengine_get_unmap_data`
               3. 调用 `dev->device_prep_dma_xxx`
               4. 注册 `tx->callback`
               5. 执行 `tx->submit` 来提交描述符并返回一个 `cookie`
               6. 执行 `dma_async_issue_pending(chan)` 来触发一个 pending 
               7. 调用 `status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);` 来等待传输完成
               8. 调用 `dmaengine_unmap_put` 将映射出来的 DMA 缓冲区都释放掉
               9. 对比数据
          2. 将通道添加到测试列表中
  3. 这里的测试参数基本都是默认的，因为可能 `init` 的时候还没有执行 `set` 给参数赋值

## exit
  1. 遍历链表中所有的 channel
     1. 删除当前的节点
     2. 调用 `dmatest_cleanup_channel` 清除通道
        1. 遍历当前通道下所有的线程，停止线程
        2. 删除链表节点
        3. 删除 task 
        4. 调用 `dmaengine_terminate_all` 来停止所有的 dma 传输
     3. 调用 `dma_release_channel` 释放通道
  2. `info->nr_channels = 0`

## run
当用户通过 sysfs 将 dmatest_run 参数设置为 1 时:

1. 执行 dmatest_run_set 函数:
   - 将 dmatest_run 参数设置为 1
   - 调用 is_thread_test_run 检查所有通道和线程状态
     - 如果有线程未完成,返回 BUSY
     - 如果所有线程都完成,执行下一步

2. 执行 restart_thread_test 函数:
   - 检查是否已执行过 init,若未执行则退出
   - 调用 run_thread_test 开始测试




# Linux AXI DMA 设备树

官方提供的设备树文档如下：
```yaml
# SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)
%YAML 1.2
---
$id: http://devicetree.org/schemas/dma/snps,dw-axi-dmac.yaml#
$schema: http://devicetree.org/meta-schemas/core.yaml#

title: Synopsys DesignWare AXI DMA Controller

maintainers:
  - Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>

description:
  Synopsys DesignWare AXI DMA Controller DT Binding

allOf:
  - $ref: dma-controller.yaml#

properties:
  compatible:
    enum:
      - snps,axi-dma-1.01a
      - intel,kmb-axi-dma
      - starfive,jh7110-axi-dma

  reg:
    minItems: 1
    items:
      - description: Address range of the DMAC registers
      - description: Address range of the DMAC APB registers

  reg-names:
    items:
      - const: axidma_ctrl_regs
      - const: axidma_apb_regs

  interrupts:
    description:
      If the IP-core synthesis parameter DMAX_INTR_IO_TYPE is set to 1, this
      will be per-channel interrupts. Otherwise, this is a single combined IRQ
      for all channels.
    minItems: 1
    maxItems: 8

  clocks:
    items:
      - description: Bus Clock
      - description: Module Clock

  clock-names:
    items:
      - const: core-clk
      - const: cfgr-clk

  '#dma-cells':
    const: 1

  dma-channels:
    minimum: 1
    maximum: 8

  resets:
    minItems: 1
    maxItems: 2

  snps,dma-masters:
    description: |
      Number of AXI masters supported by the hardware.
    $ref: /schemas/types.yaml#/definitions/uint32
    enum: [1, 2]

  snps,data-width:
    description: |
      AXI data width supported by hardware.
      (0 - 8bits, 1 - 16bits, 2 - 32bits, ..., 6 - 512bits)
    $ref: /schemas/types.yaml#/definitions/uint32
    enum: [0, 1, 2, 3, 4, 5, 6]

  snps,priority:
    description: |
      Channel priority specifier associated with the DMA channels.
    $ref: /schemas/types.yaml#/definitions/uint32-array
    minItems: 1
    maxItems: 8

  snps,block-size:
    description: |
      Channel block size specifier associated with the DMA channels.
    $ref: /schemas/types.yaml#/definitions/uint32-array
    minItems: 1
    maxItems: 8

  snps,axi-max-burst-len:
    description: |
      Restrict master AXI burst length by value specified in this property.
      If this property is missing the maximum AXI burst length supported by
      DMAC is used.
    $ref: /schemas/types.yaml#/definitions/uint32
    minimum: 1
    maximum: 256

required:
  - compatible
  - reg
  - clocks
  - clock-names
  - interrupts
  - '#dma-cells'
  - dma-channels
  - snps,dma-masters
  - snps,data-width
  - snps,priority
  - snps,block-size

if:
  properties:
    compatible:
      contains:
        enum:
          - starfive,jh7110-axi-dma
then:
  properties:
    resets:
      minItems: 2
      items:
        - description: AXI reset line
        - description: AHB reset line
        - description: module reset
else:
  properties:
    resets:
      maxItems: 1

additionalProperties: false

examples:
  - |
    #include <dt-bindings/interrupt-controller/arm-gic.h>
    #include <dt-bindings/interrupt-controller/irq.h>
    /* example with snps,dw-axi-dmac */
    dma-controller@80000 {
        compatible = "snps,axi-dma-1.01a";
        reg = <0x80000 0x400>;
        clocks = <&core_clk>, <&cfgr_clk>;
        clock-names = "core-clk", "cfgr-clk";
        interrupt-parent = <&intc>;
        interrupts = <27>;
        #dma-cells = <1>;
        dma-channels = <4>;
        snps,dma-masters = <2>;
        snps,data-width = <3>;
        snps,block-size = <4096 4096 4096 4096>;
        snps,priority = <0 1 2 3>;
        snps,axi-max-burst-len = <16>;
    };

```

## virgo 设备树

```dts

		/* example with snps,dw-axi-dmac */
		axi_dma: dma-controller@fcc00000 {
		    compatible = "snps,axi-dma-1.01a";
		    reg = <0xfcc00000 0x1000>;
		    //clocks = <&core_clk>, <&cfgr_clk>;
		    //clock-names = "core-clk", "cfgr-clk";
		    interrupt-parent = <&intc>;
		    interrupts = <GIC_SPI 91 IRQ_TYPE_LEVEL_HIGH>;
		    #dma-cells = <1>;
		    dma-channels = <6>;
		    snps,dma-masters = <2>;
		    snps,data-width = <5>;
		    snps,block-size = <0x200000 0x200000 0x200000
				       0x200000 0x200000 0x200000>;
		    snps,priority = <0 1 2 3 4 5>;
		    snps,axi-max-burst-len = <16>;
		};
```
probe 函数中: 
  - malloc chip、dw、hdata、chan
  - irq 号: `platform_get_irq`
  - 寄存器地址: `platform_get_resource` 
  - 寄存器映射: `devm_ioremap_resource` 
  - 时钟: `devm_clk_get`
  - 解析设备树: `parse_device_properties`
    - 获取 `dma-channels`
    - 获取 `snps,dma-masters`
    - 获取 `snps,data-width`
    - 获取 `snps,block-size`
    - 获取 `snps,priority`
    - 获取 `snps,axi-max-burst-len` 
  - 注册中断: `devm_request_irq`
  - 初始化 `dw->dma.channels` 为链表初始节点
  - 遍历 `dw->chan`
    - 初始化 `dw->chan[i].chip = chip`
    - 初始化 `dw->chan[i].id = i`
    - 初始化 `dw->chan[i].chan_regs = chip->regs + COMMON_REG_LEN + i * CHAN_REG_LEN`
    - 初始化 `dw->chan[i].descs_allocated = 0`
    - 初始化 `dw->chan[i].vc` 使用 `vchan_init`
  - 设置 `dw->dma.cap_mask`
    - 设置 `DMA_MEMCPY`
    - 设置 `DMA_SLAVE`
    - 设置 `DMA_CYCLIC`
  - 设置 `dw->dma.chancnt = hdata->nr_channels`
  - 设置 `dw->dma.max_burst = hdata->axi_rw_burst_len`
  - 设置 `dw->dma.src_addr_widths`
  - 设置 `dw->dma.dst_addr_widths`
  - 设置 `dw->dma.directions` 
  - 设置 `dw->dma.residue_granularity`
  - 设置 `dw->dma.dev`
  - 设置 `dw->dma.device_tx_status`
  - 设置 `dw->dma.device_issue_pending`
  - 设置 `dw->dma.device_terminate_all`   
  - 设置 `dw->dma.device_alloc_chan_resources`
  - 设置 `dw->dma.device_free_chan_resources`
  - 设置 `dw->dma.device_prep_dma_memcpy`
  - 设置 `dw->dma.device_synchronize`
  - 设置 `dw->dma.device_config`
  - 设置 `dw->dma.device_prep_slave_sg`
  - 设置 `dw->dma.device_prep_dma_cyclic`






# todo:

1. gxloader 中 dma 初始化时关闭所有中断是关的 `INT_STATUS_SIG_ENA` 寄存器，但是 `INT_STATUS` 寄存器没有关闭；Linux 中是关的 `INT_STATUS_ENA` 寄存器;