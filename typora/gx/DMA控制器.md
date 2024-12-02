- 硬件握手接口：使用硬件信号控制在ip和源/目标外设之间传递单个或突发事务传输
- 软件握手接口：使用软件寄存器控制ip和源/目标外设之间的单个或突发事务传输
- 共有8个channel
- 最多四个同时 dma 传输

# 寄存器：

- CHx_SAR：
  - 起始源地址在 DMA 通道启用之前由软件编程，或者在 DMA 传输开始之前通过 LLI 更新进行编程。当 DMA 传输正在进行时，此寄存器会更新以反映当前 AXI 传输的源地址

| Bits  | Name | Description         |
| ----- | ---- | ------------------- |
| 63：0 | SAR  | DMA传输的当前源地址 |

- CHx_DAR：

| Bits  | Name | Description           |
| ----- | ---- | --------------------- |
| 63：0 | DAR  | DMA传输的当前目标地址 |

- CHx_BLOCK_TS
  - 当 DW_axi_dmac 为流控制器时，DMAC 在通道启用块大小之前使用此寄存器

| Bits  | Name     | Description                                                  |
| ----- | -------- | ------------------------------------------------------------ |
| 21：0 | BLOCK_TS | 块传输大小 = BLOCK_TS + 1。<br>写入 BLOCK_TS 字段的数字表示在 DMA 块传输中要传输的宽度为 CHx_CTL.SRC_TR_WIDTH 的数据总数。<br>传输开始时，读回值是已经从源外设读取的数据项总数。 |

- CHx_CTL

| Bits   | Name                   | Description                                                  |
| ------ | ---------------------- | ------------------------------------------------------------ |
| 63     | SHADOWREG_OR_LLI_VALID | 指示从内存中取出的影子寄存器的内容/链表项是否有效。<br>0：影子寄存器的内容或链表项无效<br>1：影子寄存器的内容或链表项有效 |
| 62     | SHADOWNREG_OR_LLI_LAST | 0：不是最后一个影子寄存器/链表项<br>1：是最后一个影子寄存器/链表项 |
| 58     | IOC_BlkTfr             | 是否使能完成块传输中断<br>1：使能CHx_InStatuReg.中DONE 字段<br>0：不使能 |
| 57     | DST_STAT_EN            | 1：启动目标的状态获取并存储到CH1_DSTAT寄存器                 |
| 56     | SRC_STAT_EN            | 1：使能源的状态获取并存储到CH1_SSTAT寄存器                   |
| 55：48 | AWLEN                  | 目标数据burst传输长度                                        |
| 47     | AWLEN_EN               | 1：使能目标数据burst传输长度功能                             |
| 46：39 | ARLEN                  | 源数据burst传输长度                                          |
| 38     | ARLEN_EN               | 1：使能源数据burst传输长度功能                               |
| 30     | NonPosted_LastWrite_En | 1：最后一次传输必须不通过块传输<br>0：传输可能通过块传输     |
| 21：18 | DST_MSIZE              | 每次从相应的硬件或软件握手接口发出目标突发事务请求时要写入目标的数据数量。<br>0x0：在突发传输时从DST 读取1 个数据项 <br>0x1: 4 Data<br>0x2: 8 Data<br>0x3: 16 Data<br>0x4: 32 Data<br>0x5: 64 Data<br>0x6: 128 Data<br>0x7:256 Data<br>0x8: 512 Data<br>0x9: 1024 Data |
| 17:14  | SRC_MSIZE              | 0x0：在突发传输时从SRC 读取1 个数据项 <br>0x1: 4 Data<br>0x2: 8 Data<br>0x3: 16 Data<br>0x4: 32 Data<br>0x5: 64 Data<br>0x6: 128 Data<br>0x7:256 Data<br>0x8: 512 Data<br>0x9: 1024 Data |
| 13:11  | DST_TR_WIDTH           | DST传输宽度<br>0x0 : 8 bit<br>0x1: 16 bits<br>0x2: 32 bits<br>0x3: 64 bits<br>0x4: 128 bits<br>0x5: 256 bits<br>0x6: 512 bits |
| 10:8   | SRC_TR_WIDTH           | SRC传输宽度<br>0x0 : 8 bit<br>0x1: 16 bits<br>0x2: 32 bits<br>0x3: 64 bits<br>0x4: 128 bits<br>0x5: 256 bits<br>0x6: 512 bits |
| 6      | DINC                   | 目标地址递增<br>0：递增<br>1：不改变                         |
| 4      | SINC                   | 源地址递增<br>0：递增<br>1：不改变                           |
| 2      | DMS                    | 目标主机层选择。<br>0：master1 接口层上的目标设备<br>1：master2 接口层上的目标设备 |
| 0      | SMS                    | 源主机层选择。<br>0：master1 接口层上的源设备<br>1：master2 接口层上的源设备 |

- CHx_CFG

| Bits  | Name             | Description                                                  |
| ----- | ---------------- | ------------------------------------------------------------ |
| 62:59 | DST_OSR_LMT      | DST未完成请求限制，最大是16，限制 = DST_OSR_LMT + 1          |
| 58:55 | SRC_OSR_LMT      | SRC未完成请求限制，最大是16，限制 = SRC_OSR_LMT + 1          |
| 54:53 | LOCK_CH_L        | 通道锁等级。<br>00：完成DMA传输<br>01：完成DMA 块传输<br>1x：保留 |
| 52    | LOCK_CH          | 通道锁<br>0：传输期间不锁<br>1：通道锁，且授予对主总线接口的独占访问权限 |
| 51:49 | CH_PRIOR         | 通道优先级，范围是0~DMAX_NUM_CHANNELS - 1                    |
| x:44  | DST_PER          |                                                              |
| x:39  | SRC_PER          |                                                              |
| 38    | DST_HWHS_POL     | DST硬件握手接口优先级。<br>0：高优先级<br>1：低优先级        |
| 37    | SRC_HWHS_POL     | SRC硬件握手接口优先级。<br>0：高优先级<br>1：低优先级        |
| 36    | HS_SEL_DST       | DST软件或硬件握手选择，<br>0：硬件握手<br>1：软件握手        |
| 35    | HS_SEL_SRC       | SRC软件或硬件握手选择，<br>0：硬件握手<br>1：软件握手        |
| 34:32 | TT_FC            | 传输类型和流控。<br>0x0：m2m 和 DW_axi_dmac<br>0x1：m2p 和DW_axi_dmac<br>0x2：p2m 和 DW_axi_dmac<br>0x3：p2p和DW_axi_dmac<br>0x4：p2m和SRC 外设<br>0x5：p2p和SRC 外设<br>0x6：m2p和DST 外设<br>0x7：p2p和DST 外设 |
| 3:2   | DST_MULTBLK_TYPE | DST多个block传输类型。<br>00：连续<br>01：重装<br>10：影子寄存器<br>11：链接列表 |
| 1:0   | SRC_MULTBLK_TYPE | SRC多个block传输类型。<br>00：连续<br>01：重装<br>10：影子寄存器<br>11：链接列表 |

- CHx_LLP

| Bits | Name | Description                                          |
| ---- | ---- | ---------------------------------------------------- |
| 63:6 | LOC  | 存储LLI块的起始地址。                                |
| 0    | LMS  | LLI 主机选择。<br>0：AXI Master 1<br>1：AXI Master 2 |

- CHx_STATUSREG

| Bits  | Name                | Description      |
| ----- | ------------------- | ---------------- |
| 46:32 | DATA_LEFT_IN_FIFO   | FIFO剩余数据     |
| 21:0  | CMPLTD_BLK_TFT_SIZE | 完成块传输的大小 |

- CHx_SWHSSRCREG
  - 通道x的软件握手源寄存器

| Bits | Name               | Description                                                  |
| ---- | ------------------ | ------------------------------------------------------------ |
| 5    | SWHS_LST_SRC_WE    | 使能最后一次写请求来自源通道的软件握手                       |
| 4    | SWHS_LST_SRC       | 源通道最后一次请求软件握手。<br>0x1：源外设到dmac，表示当前传输是最后一次传输<br>0x0：源外设表示当前传输不是最后一次传输 |
| 3    | SWHS_SGLREQ_SRC_WE | 使能来自源通道软件握手的单次请求。<br>1：使能SWHS_SGLREQ_SRC位<br>0：失能 |
| 2    | SWHS_SGLREQ_SRC    | 源通道软件握手的单次请求。<br>0x1:源外设请求单次dma传输<br>0x0:源外设不是请求单次dma传输 |
| 1    | SWHS_REQ_SRC_WE    |                                                              |
| 0    |                    |                                                              |

- CHx_BLK_TFR_RESUMEREQREG
  - Channelx块传输恢复请求寄存器
  - 基于链表的多块传输、LLI.CHx_CTL中的ShadowReg_或_LLL_Valid位，指示从内存中获取的链接列表项是否有效
  - 基于阴影寄存器的多块传输，在chx_ctl中使用ShadowReg_或_LLL_Valid位 ，寄存器指示阴影寄存器内容是否有效

| Bits | Name              | Description                              |
| ---- | ----------------- | ---------------------------------------- |
| 0    | BLK_TFR_RESUMEREQ | 0：无请求恢复块传输<br>1：请求恢复块传输 |

- CHx_AXI_IDREG

| Bits | Name                | Description |
| ---- | ------------------- | ----------- |
| x:16 | AXI_WRITE_ID_SUFFIX |             |
| x:0  | AXI_READ_ID_SUFFIX  |             |

- CHx_SSTAT

| Bits | Name  | Description                                                  |
| ---- | ----- | ------------------------------------------------------------ |
| 31:0 | SSTAT | SRC状态信息来自硬件从 CHx_SSTATAR寄存器内容指向的地址检索的源状态信息<br><br> |

- CHx_DSTAT

| Bits | Name  | Description                                                  |
| ---- | ----- | ------------------------------------------------------------ |
| 31:0 | DSTAT | DST状态信息来自硬件从 CHx_DSTATAR寄存器内容指向的地址检索的源状态信息<br><br> |

- CHx_INTSTATUS_ENABLEREG

| Bits | Name                              | Description                                                  |
| ---- | --------------------------------- | ------------------------------------------------------------ |
| 31   | Enable_CH_ABORTED_IntStat         | 通道中止状态启用。<br>0：失能CH1_INTSTATUSREG寄存器产生通道中止中断<br>1：使能CH1_INSTATUSREG寄存器产生通道中止中断 |
| 30   | Enable_CH_DISABLED_IntStat        | 通道失能状态使能。<br>1：使能CH1_INTSTATUSREG寄存器产生通道失能中断 |
| 29   | Enable_CH_SUSPENDED_InSta         | 通道暂停状态使能                                             |
| 28   | Enable_CH_SRC_SUSPENDED_IntStat   | 通道源暂停状态使能                                           |
| 27   | Enable_CH_LOCK_CLEARED_IntStat    | 通道锁清除状态使能                                           |
| 21   | Enable_SLVIF_WRONHOLD_ERR_IntStat | 影子寄存器写入有效错误状态使能                               |
|      |                                   |                                                              |
|      |                                   |                                                              |
|      |                                   |                                                              |
|      |                                   |                                                              |
|      |                                   |                                                              |
|      |                                   |                                                              |
|      |                                   |                                                              |
|      |                                   |                                                              |
|      |                                   |                                                              |
|      |                                   |                                                              |
|      |                                   |                                                              |

- CHx_INTSTATUS

| Bits | Name                                   | Description             |
| ---- | -------------------------------------- | ----------------------- |
| 31   | CH_ABORTED_IntStat                     | 通道中止                |
| 30   | CH_DISABLED_IntStat                    | 通道失能                |
| 29   | CH_SUSPENDED_InSta                     | 通道暂停                |
| 28   | CH_SRC_SUSPENDED_IntStat               | 通道源暂停              |
| 27   | CH_LOCK_CLEARED_IntStat                | 通道锁清除状态          |
| 21   | SLVIF_WRONHOLD_ERR_IntStat             | 从接口写入保持错误      |
| 20   | SLVIF_SHADOWREG_WRON_VALID_ERR_IntStat | 影子寄存器写入有效错误  |
| 19   |                                        | 从接口写入通道启用错误  |
| 18   |                                        | 从接口读到只写错误      |
| 17   |                                        | 从接口写入只读错误      |
| 16   | SLVIF_DEC_ERR_IntStat                  | 从机解码错误            |
| 14   |                                        | 从机多个block类型错误   |
| 13   |                                        | 影子寄存器或LLI无效错误 |
| 12   |                                        | LLI写从机错误           |
| 11   |                                        | LLI读从机错误           |
| 10   |                                        | LLI写解码错误           |
| 9    |                                        | LLI读解码错误           |
| 8    |                                        | 目标从机错误            |
| 7    |                                        | 源从机错误              |
| 6    |                                        | 目标解码错误            |
| 5    |                                        | 源解码错误              |
| 4    |                                        | 目标传输完成            |
| 3    |                                        | 源传输完成              |
| 1    |                                        | DMA传输完成             |
| 0    |                                        | 块传输完成              |

- 基于链表的多块传输的编程流程
  - 软件读取DMAC通道启动寄存器(DMAC_ChEnReg)以选择可用的(未使用的)通道
  - 软件编程CHx_CFG寄存器，带有DMA传输配置的数值，SRC_MLTBLK_TYPE和DST_MLTBLK_TYPE必须设为`2'b11`
  - 软件编程第一个链接列表项的基地址和主接口，其中链接列表项可以在CHx_LLP寄存器中使用
  - 软件在内存中创建一个或多个链接列表项。软件可以创建整个链接列表项或者使用CHx_CTL.ShadowReg 或 LLI_Valid和CHx_CTL.LLI_Last是否有效
  - 软件通过写1到DMAC_ChEnReg寄存器来使能相应通道
  - 控制器根据块传输的设置启动DMA块传输操作。块传输可能立即启动或者在硬件或软件握手请求之后，取决于CHx_CFG寄存器的TT_FC字段。
  - 在链表列表获取阶段：
    - 如果CHx_CTL.ShadownReg 或 LLI_Last位是1，控制器认为当前块是传输的最后一块，并完成DMA
    - 如果CHx_CTL.ShadownReg 或 LLI_Last位是0，控制器认为有一个或多个块要传输，进入上一步骤，启动DMA块传输操作
    - 如果CHx_CTL.ShadownReg 或 LLI_Valid位是0，控制器可能产生ShadiwnReg 或 LLI_Invalid_ERR 中断。在尝试另一个LLI读取操作之前，控制器等待软件将(任何值)写入CHx_BLK_TFT_ResumeReqReg 以表示LLI可用

硬件握手接口-使用硬件信号控制在DW_ ahb_ dmac和源或目标外设之间传输单个或突发事务。有关此接口的更多信息，参见第4 0页的“硬件握手-外设不是流量控制器”和第4 9页的“软件握手-外置流量控制器”。软件握手接口-使用软件寄存器控制dmac和源或目标外设之间的单个或突发事务传输。1/O上不需要特殊的DW_ ahb_ dmac握手信号该模式用于将现有外设连接到DW_ ahb_ dmac而无需修改。







# CTLx

- SRC_TR_WIDTH:
- DST_TR_WIDTH:
  - DMA 传输的位宽是多少
- SRC_MSIZE：
- DEST_MSIZE：
  - 每次 DMA 传输要传输多少，MSIZE 寄存器的配置数量 * WIDTH