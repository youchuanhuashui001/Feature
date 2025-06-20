# 概述
- drawio 流程图路径：
- 代码路径：



# 系统集成

## .config
```d
CONFIG_DW_AXI_DMAC=y
CONFIG_DMATEST=y
```

## Kconfig
```d
config DW_AXI_DMAC
	tristate "Synopsys DesignWare AXI DMA support"
	depends on OF
	depends on HAS_IOMEM
	select DMA_ENGINE
	select DMA_VIRTUAL_CHANNELS
	help
	  Enable support for Synopsys DesignWare AXI DMA controller.
	  NOTE: This driver wasn't tested on 64 bit platform because
	  of lack 64 bit platform with Synopsys DW AXI DMAC.
```


## Makefile
```d
obj-$(CONFIG_DW_AXI_DMAC) += dw-axi-dmac-platform.o
```

## dts
```d
dmac: dma-controller@80000 {
	compatible = "snps,axi-dma-1.01a";
	reg = <0x80000 0x400>;
	clocks = <&core_clk>, <&cfgr_clk>;
	clock-names = "core-clk", "cfgr-clk";
	interrupt-parent = <&intc>;
	interrupts = <27>;

	dma-channels = <4>;
	snps,dma-masters = <2>;
	snps,data-width = <3>;
	snps,block-size = <4096 4096 4096 4096>;
	snps,priority = <0 1 2 3>;
	snps,axi-max-burst-len = <16>;
};

axi_dma: dma-controller@fcc00000 {
    compatible = "snps,axi-dma-1.01a";
    reg = <0xfcc00000 0x1000>;
    // core-clk 用于：
    // cfgr-clk 用于：
    //clocks = <&core_clk>, <&cfgr_clk>;
    //clock-names = "core-clk", "cfgr-clk";
    interrupt-parent = <&intc>;
    interrupts = <GIC_SPI 91 IRQ_TYPE_LEVEL_HIGH>;
    #dma-cells = <1>;
    dma-channels = <6>;
    // 修改到 2 个
    snps,dma-masters = <1>;
    snps,data-width = <4>;
    snps,block-size = <0x200000 0x200000 0x200000
		       0x200000 0x200000 0x200000>;
    snps,priority = <0 1 2 3 4 5>;
    // 可选的
    //snps,axi-max-burst-len = <16>;
    status = "okay";
};
```


# 驱动分析

## 数据结构

```c
// 硬件能力描述，全部来自设备树，由 parse_device_properties() 解析
struct dw_axi_dma_hcfg {
	u32	nr_channels;
	u32	nr_masters;
	u32	m_data_width;
	u32	block_size[DMAC_MAX_CHANNELS];
	u32	priority[DMAC_MAX_CHANNELS];
	/* maximum supported axi burst length */
	u32	axi_rw_burst_len;
	/* Register map for DMAX_NUM_CHANNELS <= 8 */
	bool	reg_map_8_channels;
	bool	restrict_axi_burst_len;
};

// 单个 dma 通道的软件上下文
// 继承 virt_dma_chan，从而复用 virt-dma 的基础设施(cookie、tasklet、链表等)
struct axi_dma_chan {
	struct axi_dma_chip		*chip;
	void __iomem			*chan_regs;
	u8				id;
	u8				hw_handshake_num;
	atomic_t			descs_allocated;

	// 分配 64Byte 对齐的 axi_dma_lli
	struct dma_pool			*desc_pool;
	struct virt_dma_chan		vc;

	// 运行中的当前描述符
	struct axi_dma_desc		*desc;
	// slave_config 备份
	struct dma_slave_config		config;
	enum dma_transfer_direction	direction;
	bool				cyclic;
	/* these other elements are all protected by vc.lock */
	bool				is_paused;
};

// 代表 DMA 控制器实例，内含 Linux 通用 dma_device，
// 以及硬件能力描述 dw_axi_dma_hcfg
// 维护多个通道数组 axi_dma_chan
struct dw_axi_dma {
	struct dma_device	dma;
	struct dw_axi_dma_hcfg	*hdata;
	struct device_dma_parameters	dma_parms;

	// 动态分配的 axi_dma_chan 数组
	/* channels */
	struct axi_dma_chan	*chan;
};

// 代表平台设备的根节点，负责保存寄存器基地址、IRQ、时钟、以及指向下一层 dw_axi_dma 的指针
struct axi_dma_chip {
	struct device		*dev;
	int			irq;
	void __iomem		*regs;
	// Intel KeemBay 可选
	void __iomem		*apb_regs;
	struct clk		*core_clk;
	struct clk		*cfgr_clk;
	struct dw_axi_dma	*dw;
};

// 描述链表项
/* LLI == Linked List Item */
struct __packed axi_dma_lli {
	__le64		sar;
	__le64		dar;
	__le32		block_ts_lo;
	__le32		block_ts_hi;
	__le64		llp;
	__le32		ctl_lo;
	__le32		ctl_hi;
	__le32		sstat;
	__le32		dstat;
	__le32		status_lo;
	__le32		status_hi;
	__le32		reserved_lo;
	__le32		reserved_hi;
};

// 在驱动中对应单个链表项的包装
struct axi_dma_hw_desc {
	struct axi_dma_lli	*lli;
	dma_addr_t		llp;
	u32			len;
};

// 一次完整的 dma 请求，可能有多个  lli，继承 virt_dma_desc，可挂入 virtDMA 队列
struct axi_dma_desc {
	struct axi_dma_hw_desc	*hw_desc;

	struct virt_dma_desc		vd;
	struct axi_dma_chan		*chan;
	u32				completed_blocks;
	u32				length;
	u32				period_len;
};

// 传输信息，应该是用作中间转接层
struct axi_dma_chan_config {
	u8 dst_multblk_type;
	u8 src_multblk_type;
	u8 dst_per;
	u8 src_per;
	u8 tt_fc;
	u8 prior;
	u8 hs_sel_dst;
	u8 hs_sel_src;
};
```


## probe

// 平台设备
- malloc struct axi_dma_chip *chip;
// dw_axi_dma ip
- malloc struct dw_axi_dma *dw;
// 设备树描述信息
- malloc struct dw_axi_dma_hcfg *hdata;

- chip->dw = dw;
- chip->dev = &pdev->dev;
- chip->dw->hdata = hdata;

- 从设备树拿到 irq 号
- 从设备树中拿到基地址，然后 ioremap

- malloc struct axi_dma_chan(hdata->nr_channales * dw->chan)

- 获取设备树的 core-clk、cfgr-clk ？？？？？

- 解析设备树
	- 获取 dma-channels
	- 获取 snps,dma-masters
	- 获取 snps,data-width
	- 获取 snps,block-size
	- 获取 snps,priority
	- 获取 snps,axi-max-burst-len 可选！！！？？？？
- 注册中断服务函数：dw_axi_dma_interrupt
- 初始化 dw->dma.channels 为一个初始链表，后续会将所有的 channel 链上去
	- struct dw_axi_dma dw;
	- struct dma_device dma;
	- struct list_head channels;
- 循环配置所有的 dma_chan：
	- chan->chip = chip；
	- chan->id = i;
	- chan->chan_regs = chip->regs + COMMON_REG_LEN + i * CHAN_REG_LEN;
	// 说明没有描述符被申请
	- atomic_set(&chan->descs_allocated, 0);
	- vchan_init(&chan->vc, &dw->dma);
- 配置 dw->dma.cap_mask(该 dma device 的能力)
	- memcpy
	- slave
	- cyclic
- 配置 dw->dma(struct dma_device)

- 将 struct axi_dma_chip *chip 作为 platform_device->dev->driver_data

- axi_dma_resume
	- 使能 cfgr_clk、core_clk
	- 使能 dma、irq

- axi_dma_hw_init
	- 关闭所有通道的 irq(配置的是 0x80 INTSTATUS_ENA 寄存器)、disable 所有通道

- dma_async_device_register(&dw->dma);
	- 向 dmaengine 注册 struct dma_device

- of_dma_controller_register
	- 向 dts 提供 dma = <&axi_dma X> 解析


## 对上层的接口




# 应用程序




# 关联知识




# Q&&A

## core-clk、cfgr-clk 作用是什么？
- core-clk 描述的是：
- cfg-clk 描述的是：

## Linux 初始化时关闭的是 0x80 INT_STATUS_ENA 寄存器，而 gxloader 中关的都是 0x90 INT_STAUTS_SIGNAL_ENA
- 如果开着 INT_STATUS_ENA，不开 INT_STATUS_SIGNAL_ENA，会触发中断状态吗？会触发中断线吗？
	- 会触发中断状态，不会触发中断线
- 如果不开 INT_STATUS_ENA，会触发中断状态吗？会触发中断线吗？
	- 不会触发中断状态，也不会触发中断线
- Linux 中用的都是中断模式，所以初始化时关闭 INT_STATUS_ENA 比较好，gxloader 中的初始化其实也应该关闭 INT_STATUS_ENA

## snps,axi-max-burst-len 是干什么的？设备树中默认可以不用写这个属性

- 从代码中看是对应 awlen、arlen，默认是不需要的
- awlen、arlen 是用来

```c

	if (chan->chip->dw->hdata->restrict_axi_burst_len) {
		burst_len = chan->chip->dw->hdata->axi_rw_burst_len;
		ctlhi |= CH_CTL_H_ARLEN_EN | CH_CTL_H_AWLEN_EN |
			 burst_len << CH_CTL_H_ARLEN_POS |
			 burst_len << CH_CTL_H_AWLEN_POS;
	}

```


## 如何与 virt-dma 打交道？如何与 dmaengine 打交道？

- dw-axi-dmac-platform.c：具体硬件驱动，包括寄存器编程、描述符构造、IRQ 处理
- dmaengine.c：框架核心层，包括统一的 chan 分配、引用计数、sysfs、能力匹配
- virt-dma.c：通用软件通道库，包括 cookie、队列、tasklet等逻辑，供各个 DMA 驱动复用

### 与 dmaengine 的交互：
- 注册阶段：`dma_async_device_register(&dw->dma);`
	- 把 dw->dma 挂入全局 dma_device_list
	- 为每条 dma_chan 创建 sysfs 节点与引用计数
	- 对 public 通道预申请资源
- 上层通道申请
	- 外设驱动调用 dma_request_chan()
	- dmaengine.c 在 dma_device_list 遍历，找到满足条件的 dw->chan[i]
	- 如果首次使用，dmaengine.c 调用驱动中的 device_alloc_chan_resources 完成 dma_poll_create() 等初始化
- 事务准备 & 提交
	- 外设调用 dmaengine_prep_* 直接调用驱动中的回调
	- 之后 tx_submit()/issue_pending() 借助 virt-dma
- 状态查询、终止
	- 上层调用 dma_async_is_tx_complete() 等

## 对接外设流程：

```c

struct dma_chan *dma_request_chan(struct device *dev, const char *name);

int dmaengine_slave_config(struct dma_chan *chan,
                  struct dma_slave_config *config);

struct dma_async_tx_descriptor *dmaengine_prep_slave_sg(
           struct dma_chan *chan, struct scatterlist *sgl,
           unsigned int sg_len, enum dma_data_direction direction,
           unsigned long flags);

struct dma_async_tx_descriptor *dmaengine_prep_peripheral_dma_vec(
           struct dma_chan *chan, const struct dma_vec *vecs,
           size_t nents, enum dma_data_direction direction,
           unsigned long flags);

struct dma_async_tx_descriptor *dmaengine_prep_dma_cyclic(
           struct dma_chan *chan, dma_addr_t buf_addr, size_t buf_len,
           size_t period_len, enum dma_data_direction direction);

struct dma_async_tx_descriptor *dmaengine_prep_interleaved_dma(
           struct dma_chan *chan, struct dma_interleaved_template *xt,
           unsigned long flags);

dma_cookie_t dmaengine_submit(struct dma_async_tx_descriptor *desc);


void dma_async_issue_pending(struct dma_chan *chan);


enum dma_status dma_async_is_tx_complete(struct dma_chan *chan,
          dma_cookie_t cookie, dma_cookie_t *last, dma_cookie_t *used)

```



### dma_request_slave_channel_compat

存在两种方式，标准的做法是使用设备树
- 优先从设备树中获取 dma channel(struct dma_chan)
- 使用 dma_filter_fn 获取 dma channel(struct dma_chan)

`struct dma_chan *chan = dma_request_slave_channel(dev, "tx")`
这里是兼容层，下面也是调到 `dma_request_chan(dev, name);`

- `dma_request_chan(dev, name)`
- 优先从设备树获取 channel
	- 从设备树查找 "dmas" 属性，没找到就返回报错
	- 从设备树查找 "dma-names" 属性，获取存在该属性的个数保存到 count
	- 循环 count 次
		- 判断设备树中的这些属性是不是有效的 `dma-names`
		- 根据设备树中找到的 dma 属性，找到 dma 控制器
		- 调用 dma->of_dma_xlate
			- dw_axi_dma_of_xlate
			- 获取任一个可用的 dma channel
			- 将握手信号保存起来
			- 返回 struct dma_chan *dchan
- 通过 acpi 获取 channel
- 如果获取到的 channel 有效，就返回 channel
- 通过过滤的方式获取 channel


### dma_get_slave_caps(dma->rxchan, &caps)

- 从返回的 struct dma_chan 找到对应的 struct dma_device
- dw-axi-dmac-platform.c 中会配置 struct dma_device
- 直接从 struct dma_device 拿到 caps，保存到 caps

### dmaengine_slave_config(dma->rxchan, &dma->rxconf)

```c
struct dma_slave_config {
	enum dma_transfer_direction direction;
	phys_addr_t src_addr;
	phys_addr_t dst_addr;
	enum dma_slave_buswidth src_addr_width;
	enum dma_slave_buswidth dst_addr_width;
	u32 src_maxburst;
	u32 dst_maxburst;
	u32 src_port_window_size;
	u32 dst_port_window_size;
	bool device_fc;
	unsigned int slave_id;
};
```


- 配置 tx/rxconf，包括方向，地址，位宽
- `dmaengine_slave_config(dma->rxchan, &dma->rxconf)`
- `dw_axi_dma_chan_slave_config` 会把所有的配置拷贝到 `struct axi_dma_chan *chan->config`


### dma_alloc_coherent

- 一致性 DMA 映射，硬件保证设备和 CPU 可以并行访问数据，并且能够看到对方所做的更新，无需显式的软件刷新。
- 可以将其理解为同步的。
- 返回物理地址

### dma_map_single

- 流式 DMA 映射，用于单个传输。
- 会调用 ops->map_page 申请 page。
- 返回物理地址

### dmaengine_prep_slave_single

- 调到 dw_axi_dma_chan_prep_slave_sg
	- 将 struct dma_chan *dchan 转换成 struct axi_dma_chan *chan;
	- 从 sg 拿到 dma_address、len，计算需要的 block 数量
	- 申请 n 个 描述符
	- 依次配置每个描述符
		- 使用刚刚申请出来的描述符，然后进行配置
		- 配置 block_ts、lli
	- 配置最后一个描述符的 ctl_hi 寄存器，说明是 LAST_LLI
	- 将描述符写到内存中去
- vchan_tx_prep


### dmaengine_submit

- 调用 desc->tx_submit
- 到了 vchan_tx_submit，分配一个 cookie


### dma_sync_single_for_device


### dma_async_issue_pending

- 调用 dma_chan_issue_pending-->axi_chan_block_xfer_start
- 使能 axi_dma
- 配置 cfg 寄存器:方向和流控、握手信号
- 配置 llp 寄存器
- 使能 DMA_TRF、ALL_ERR 中断
- 使能一些 IRQ_STATUS
- 使能 dma 通道

### dmaengine_tx_status






# AI 交互历史

## 请深入分析 @dw-axi-dmac-platform.c 
下面针对 `dw-axi-dmac-platform.c`（约 1.6 k 行）逐段做“源代码—功能—注意点”的深度剖析，力求让你在阅读或二次开发时能快速定位到关键逻辑、潜在坑点以及与 Linux DMA-engine 框架的交互位置。全文以“控制器视角”贯穿：初始化→通道生命周期→数据传输→中断→电源管理。

---

## 1. 文件整体布局

```
1-130     寄存器访问内联函数、通用宏
131-410   通道/控制器启停、IRQ 屏蔽/使能、描述符分配/回收
411-820   传输准备：memcpy、slave SG、cyclic
821-1020  传输控制：issue_pending、同步、terminate、pause/resume
1021-1180 中断服务、错误处理、完成回调
1181-1370 设备树解析、platform_probe/remove
1371-1618 PM 回调、OF-DMA xlate、platform_driver 注册
```

---

## 2. 基础封装：寄存器 & IRQ

1. **寄存器读写**  
   * `axi_dma_iowrite32()/ioread32()` 针对 **公共寄存器**  
   * `axi_chan_iowrite32()/ioread32()` 针对 **通道寄存器**  
   * `axi_chan_iowrite64()` 适配部分 SoC 不支持 64-bit AXI 访问的特性，“拆 64 为两次 32”  
   → 统一入口便于未来加 memory barrier 或 big-endian 适配。

2. **IRQ 层级**  
   * 全局 `DMAC_CFG.INT_EN_MASK`（控制器级）  
   * 通道 `CH_INTSTATUS_ENA / INTSIGNAL_ENA`（通道级）  
   → `axi_dma_irq_disable/enable` 先总闸，`axi_chan_irq_*` 精细控制具体位。  

3. **启停通道**  
   * `axi_chan_enable/disable` 需携带两套 *WE* 位（≤8 通道与 >8 通道寄存器映射不同）。  
   * `axi_chan_is_hw_enable()` 只读 `DMAC_CHEN` 里对应位，用于 ASSERT/状态复位。

---

## 3. 描述符管理

1. **三层抽象**（在 `dw-axi-dmac.h` 已解读）  
2. **分配策略**  
   * LLI 使用 `dma_pool_create()` 保证 **64 字节** 对齐（硬件要求）  
   * 通过 `atomic_t descs_allocated` 跟踪外借对象，便于调试泄漏  
3. **填充流程**  
   * `dw_axi_dma_set_hw_desc()` 根据方向计算 `mem_width / reg_width / block_ts`  
   * `set_desc_for_master()` 动态决定 `SRC_MAST/DST_MAST` 以支持单/双 AXI master  
   * 可选 `restrict_axi_burst_len`：若设备树配置 `snps,axi-max-burst-len`，在 CTL_H 写 ARLEN/AWLEN

---

## 4. 传输准备（prep 阶段）

1. `dma_chan_prep_dma_memcpy()`  
   * 简单 memcpy，但同样遵守硬件 `BLOCK_TS<=block_size[i]` 约束，因此可能拆分多段。  
2. `dw_axi_dma_chan_prep_slave_sg()`  
   * 针对外设 Scatter/Gather。对每个 SG entry 再细分成 **segments** 以符合 `axi_block_len`。  
3. `dw_axi_dma_chan_prep_cyclic()`  
   * 为音频等持续流设计  
   * 关键点：  
     - 计算一个 `period` 需要多少 segment  
     - 每个 segment 都打 `CH_CTL_H_LLI_LAST`，借助硬件**周期回环**触发中断  
     - `vchan_cyclic_callback()` 在 ISR 中周期性调用  
4. `dw_axi_dma_set_hw_channel()`  
   * Intel KeemBay 专用 APB 寄存器：把 **HW Handshake** 号写入 `DMAC_APB_HW_HS_SEL_0`  
   * 解除映射用 `UNUSED_CHANNEL (0x3F)`，否则某些外设再请求会失败

---

## 5. issue_pending & 队列调度

1. 使用 `virt-dma` 框架：  
   * `vchan_next_desc()` 查链表头  
   * `chan->vc.lock` 软中断环境保护  
2. `axi_chan_block_xfer_start()`  
   * 首次启动：  
     - 写 `CH_CFG_L/H` 或 `CH_CFG2_L/H`（8 通道 / 16 通道差异）  
     - 把首个 `LLP` 地址写 `CH_LLP`  
     - 设定 irq mask = `DMA_TRF | error | SUSPENDED`  
     - 最后置位 `DMAC_CHEN`  
   * 若通道已 busy → 报错并返回

---

## 6. 中断处理

```
dw_axi_dma_interrupt()
 ├─ 关闭全局 IRQ
 ├─ for each channel
 │    ├─ 读 CH_INTSTATUS / 清除
 │    ├─ error? → axi_chan_handle_err()
 │    └─ DMA_TRF? → axi_chan_block_xfer_complete()
 └─ 重新打开全局 IRQ
```

### 6.1 错误分支 `axi_chan_handle_err()`
* 从 `vc` 队列摘掉当前坏描述符 → `vchan_cookie_complete()` 上报  
* 打印 LLI dump 辅助 debug  
* 再次尝试 `axi_chan_start_first_queued()` 保持服务不中断  

### 6.2 正常完成 `axi_chan_block_xfer_complete()`
* cyclic:  
  - 根据当前硬件 LLP 定位 **周期索引**，更新 `completed_blocks`  
  - 当完成块数 × len 达到 `period_len` 时做 callback  
  - 重新 `axi_chan_enable()` 继续跑  
* 非 cyclic:  
  - 将 vd 从链表摘掉并 complete  
  - 自动启动下一个队列任务（若有）  

---

## 7. 通道控制 API

| API | 功能要点 |
| --- | -------- |
| `dma_chan_tx_status` | 计算剩余字节。cyclic 中结合 `completed_blocks * len` 反推。 |
| `dma_chan_terminate_all` | 读-改-写 `DMAC_CHEN` + poll idle；回收所有 vdesc；同时取消握手映射。 |
| `dma_chan_pause/resume` | 通过 `DMAC_CHSUSPREG` 或 `DMAC_CHEN.SUSP` 位实现硬暂停；resume 再清位。 |

---

## 8. Platform 驱动流程

1. **probe** (`dw_probe`)  
   * 解析 DT 属性 → `dw_axi_dma_hcfg`  
   * `devm_ioremap_resource()` 建立寄存器映射  
   * `devm_request_irq()` 共享型 IRQ  
   * 为每个通道 `vchan_init()`  
   * `dma_async_device_register()` 注册到 dmaengine Core  
   * `of_dma_controller_register()` 提供 dts `dma = <&axi_dma X>` 解析  
   * 打印 `DesignWare AXI DMA Controller, n channels`  
2. **remove**  
   * 停时钟 → disable IRQ → 关通道 → 释放 vchan → 注销 dmaengine  
3. **PM**  
   * `SET_RUNTIME_PM_OPS` 只在 `axi_dma_runtime_suspend/resume` 中把 **IRQ/DMA_EN** 开关  
   * probe 时 `pm_runtime_enable` + 手动 `pm_runtime_get_noresume` 确保启动阶段硬件已供电

---

## 9. 设备树属性一览（见 parse_device_properties）

```
dma-channels               = <n>;          // 必填 ≤16
snps,dma-masters           = <1|2>;
snps,data-width            = <0-7>;        // 2^n bytes
snps,block-size            = <blk0 blk1 ...>; // 通道级
snps,priority              = <p0 p1 ...>;     // 通道级
snps,axi-max-burst-len     = <1-257>;     // 可选
```

> 若通道 ≤ 8，驱动自动置 `reg_map_8_channels=true`，内部选用 `CH_CFG_L/H`。否则走 `CH_CFG2_*`。

---

## 10. 可能踩坑/优化点

1. **block_size 与 period_len/sg_len 在用户层不匹配** → 驱动会拆分并打印 `invalid alignment` 警告。  
2. `restrict_axi_burst_len` 仅影响 **memcpy** 路径；slave SG/ cyclic 也许需要同样限制，可按需补丁。  
3. `axi_chan_is_hw_enable()` 多处 assert，如遇异步 reset 有可能误判，可在 SoC 电源域调试时留意。  
4. 支持 64 bit DMA mask，若硬件只连 32 bit axi，应在设备树禁用，否则会出现 “DMA of address xxxxx out of range”。  

---

## 结语

`dw-axi-dmac-platform.c` 把 Synopsys DW AXI DMAC 的所有 **寄存器编程时序**、**链表构造逻辑** 以及 **Linux DMA-engine 适配** 封装在一个文件内，逻辑清晰分层：

* **内联函数** —— 简化寄存器操作  
* **描述符** —— 把多种上层 API 转为硬件 LLI  
* **调度 & 中断** —— 完成 virt-DMA 与硬件的 glue  
* **平台 glue code** —— DT 解析 + PM + OF-DMA

理解这些模块，你可以轻松：

1. 扩展对新外设 handshake 的支持（修改 `dw_axi_dma_set_hw_channel`）。  
2. 优化大块 memcpy 吞吐（调整 `block_size` / `axi-max-burst-len`）。  
3. 排查外设 DMA 异常（用 `axi_chan_dump_lli()` 打印 LLI 状态）。  

如需进一步挖掘某一段代码或特定寄存器含义，随时提问！

## 1. 请详细说明设备树中 core-clk、cfgr-clk 
2. 请说明 @dw-axi-dmac-platform.c 如何与 @dmaengine.c 交互， @dmaengine.c 如何与 @virt-dma.c  交互，三者如何交互？
3. 用户程序如何调用 dma？请详细说明完整的交互流程

以下内容针对您提出的 3 个问题展开说明，全部基于 Linux DMA-engine 框架 + Synopsys DW-AXI-DMAC 驱动实际代码流程。

============================================================
一、设备树中的 core-clk 与 cfgr-clk 含义  
============================================================

1. clock-names  
   • `"core-clk"`：驱动 DMA 核心（AXI master 端）的 **工作时钟**。  
   • `"cfgr-clk"`：驱动 **配置接口**（通常是 APB/AXI-lite slave 端）的时钟。  

2. 典型 DTS 片段  
```dts
axi_dma0: dma@e0000000 {
        compatible = "snps,axi-dma-1.01a";
        reg        = <0x0 e0000000 0x0 0x1000>;
        interrupts = <0 42 IRQ_TYPE_LEVEL_HIGH>;

        clocks      = <&clk_axi>, <&clk_apb>;
        clock-names = "core-clk", "cfgr-clk";

        dma-channels      = <8>;
        snps,dma-masters  = <2>;
        snps,data-width   = <4>;          /* 2^4 = 16-byte bus */
        snps,block-size   = <1024 1024 1024 1024 1024 1024 1024 1024>;
        snps,priority     = <0 1 2 3 4 5 6 7>;
};
```

3. 在驱动中的使用  
dw-axi-dmac-platform.c（上游版本）在 probe 时执行  

```c
chip->core_clk = devm_clk_get(dev, "core-clk");
chip->cfgr_clk = devm_clk_get(dev, "cfgr-clk");
clk_prepare_enable(core_clk);
clk_prepare_enable(cfgr_clk);
```

• **resume() / suspend() / runtime-pm** 阶段再统一开关这两个时钟；  
• 如果省略 `cfgr-clk`，部分 SoC 会默认从 `core-clk` 分频；否则配置寄存器访问将超时（现象：probe 卡死或读回 0xdeadbeef）。

============================================================
二、dw-axi-dmac-platform.c、dmaengine.c、virt-dma.c 的三角关系  
============================================================

1. 角色定位  
   • **dw-axi-dmac-platform.c**：硬件专用 **DMA 设备驱动**；向框架声明 “我能做哪些事务”。  
   • **dmaengine.c**：框架 **核心层**，维护所有 `dma_device`，实现通道分配、sysfs、引用计数等通用逻辑。  
   • **virt-dma.c**：为 DMA 设备驱动提供 **软件通道抽象**（`virt_dma_chan` / `virt_dma_desc`），把通用的队列、tasklet、cookie 等重复代码封装成库。

2. 交互链路  
   a) **驱动 → virt-dma**  
      - 在 `dw_probe()` 中：  
        ```c
        vchan_init(&chan->vc, &dw->dma);   // 每条通道挂进 virt-DMA 基础设施
        ```  
      - `dw-axi-dmac-platform.c` 创建/回收描述符时调用  
        `vchan_tx_prep()`、`vchan_tx_submit()` 等库函数。  

   b) **驱动 → dmaengine 核心**  
      - probe 结束前调用  
        ```c
        dma_async_device_register(&dw->dma);
        ```  
        把 `dw->dma`（类型 `struct dma_device`）加入 **全局链表** `dma_device_list`。  
      - 驱动在 `dw->dma` 中填充回调指针：`device_prep_*() / device_issue_pending / device_tx_status ...`，供核心层及客户端调用。  

   c) **dmaengine 核心 ↔ virt-dma**  
      - 核心层不直接调用 virt-dma；但是客户端拿到 `struct dma_chan` 后，调用驱动提供的回调，而驱动内部再使用 virt-dma 操作队列。  
      - 反向地，virt-dma 调度完成时会触发 `tasklet`，最终通过 `dmaengine_desc_callback_invoke()` 执行客户端注册的回调。  

3. 典型时序（MEMCPY 为例）  
   ```
   应用        驱动/内核其他模块        dmaengine.c         dw-axi-dmac        virt-dma
     |                 |                    |                    |                  |
     | memcpy ioctl    |                    |                    |                  |
     |---------------> | dma_request_chan() |                    |                  |
     |                 |----锁+查表-------> |                    |                  |
     |                 | <------返回 chan---|                    |                  |
     |                 | prep_dma_memcpy()  |                    |                  |
     |                 |---------------------------------------> | vchan_tx_prep() |
     |                 | tx_submit()        |                    | vchan_tx_submit |
     |                 | issue_pending()    |                    | ==>启动HW       |
     |                 |                    |                    |                  |
     |                 |  中断              |  ISR              |  vchan_complete  |
     |                 |<--------------------------------------- | (tasklet)       |
     |  wake-up/callback|                    |                    |                  |
   ```

============================================================
三、上层调用 DMA 的完整链路  
============================================================

下面分 “内核子系统驱动” 与 “用户程序” 两层说明。

------------------------------------------------------------
1. 内核子系统 / 外设驱动层调用  
------------------------------------------------------------

以 I2S 声卡播放（cyclic 模式）为例：  

1) **声卡驱动**  
   ```c
   dai->dma_chan = dma_request_chan(dev, "tx");   // 找到 DMA 通道
   snd_dmaengine_pcm_prepare_slave_config(substream, &conf);  // 填 slave_config
   ```
2) **prepare**  
   ```c
   dmaengine_prep_dma_cyclic(chan, buf, buf_len,
                             period_len, DMA_MEM_TO_DEV, flags);
   → dw_axi_dma_chan_prep_cyclic()  // 驱动生成 LLI 链表
   ```
3) **提交 + 使能**  
   ```c
   cookie = tx->tx_submit(tx);          // vchan_tx_submit
   dma_async_issue_pending(chan);       // 驱动启动硬件
   ```
4) **硬件循环传输**  
   • 每到一个 `period` 完成触发 IRQ，  
   • ISR → `axi_chan_block_xfer_complete()` → `vchan_cyclic_callback()` → 声卡 PCM 回调，更新 ALSA ring buffer。  

------------------------------------------------------------
2. 用户态到 DMA 的路径  
------------------------------------------------------------

用户进程 → 系统调用 → 外设驱动 → DMA 框架 → DW-AXI-DMAC → 硬件  
（用户空间并 **不直接** 操作 DMA，所有接口均由内核外设驱动封装）

举 3 个常见形式：

1) **ALSA/PCM 音频**  
   `aplay xxx.wav` → `write()` PCM → SoC 声卡驱动如上分配 cyclic DMA。

2) **SPI 大批量收发**  
   `spidev` ioctl(`SPI_IOC_MESSAGE`) → SPI 控制器驱动若检测传输长度满足阈值，则：  
   * `dma_request_chan()` 获取 RX/TX 通道  
   * `dmaengine_prep_slave_sg()` 把 sg 列表提交给 DW-AXI-DMAC  
   * 中断回调后 SPI 驱动唤醒等待的用户进程。  

3) **memcpy off-load（dmatest 模块）**  
   * root 下 `modprobe dmatest run=1`  
   * dmatest.c 通过 `dma_request_chan_by_mask()` 找 MEMCPY 通道，调用 `dmaengine_prep_dma_memcpy()`，验证 HW 复制正确性。

------------------------------------------------------------
3. 系统级重要函数链  
------------------------------------------------------------

1) **通道获取**  
   `dma_request_chan()` → dmaengine.c 在 `dma_device_list` 遍历 → 返回 `dw->chan[i]`  
2) **描述符准备**  
   `dmaengine_prep_*` → 驱动的 `device_prep_*` → virt-dma 生成 `axi_dma_desc`/cookie  
3) **提交**  
   `tx_submit()` → virt-dma 把 vd 丢进 `desc_submitted` 队列  
4) **issue_pending**  
   驱动把“队首”描述符赋给硬件寄存器，置位 `DMAC_CHEN`  
5) **硬件完成中断**  
   ISR → vchan_complete tasklet → 回调上层 → 若非 cyclic，释放 LLI → dmaengine core 更新统计。  

这样一条链路把“应用数据”安全、高效地搬运到外设或内存，无须用户态关心任何底层细节。

============================================================
总结  
============================================================

• `core-clk` 驱动 DMAC 主通路，`cfgr-clk` 驱动其配置接口，两者缺一不可。  
• dw-axi-dmac-platform.c 负责**硬件适配**，借助 virt-dma 管理通道队列，并通过 dmaengine.c 向系统出口发布 **标准 DMA 能力**。  
• 应用最终调用的是外设驱动或测试模块，这些驱动只需遵守 dmaengine API，无须了解底层寄存器细节，实现了硬件无关 + SoC 可移植性。
