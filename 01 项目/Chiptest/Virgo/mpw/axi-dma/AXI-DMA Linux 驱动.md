



## 参考资料
- `jh-7110` 文档：
	- https://doc-en.rvspace.org/VisionFive2/Developer_Guide/SDK_DG_DMA.pdf




介绍描述符、Cookie、虚拟通道三者的功能以及联系：
- 描述符：是 DMA 传输的基本单元，包含了一次传输所需的所有信息，对应 `struct virt_dma_desc`
	```c
	struct virt_dma_desc {
		struct dma_async_tx_descriptor tx; // 基本传输描述符
		struct list_head_node;             // 用于将描述符链接成链表
	};

	struct dma_async_tx_descriptor {
		dma_cookie_t cookie;   // 传输标识符
		struct dma_chan *chan; // 关联的 DMA 通道
		...
	};
	```
- 虚拟通道：对物理 DMA 通道的软件抽象，管理描述符队列：
	```c
	struct virt_dma_chan {
		struct dma_chan chan;              // 基础的 dma 通道
		struct list_head desc_submitted;   // 已提交的描述符
		struct list_head desc_issued;      // 已发出的描述符
		struct list_head desc_completed;   // 已完成的描述符
		struct list_head desc_allocated;   // 已分配的描述符
	}
	```
- cookie：用于跟踪 dma 传输的唯一标识符
	- 定义在 `struct dma_chan` 中，其中 `completed_cookie` 用于记录已完成的 `cookie`
- 一般的工作流程如下：
	```
	应用程序
	   ↓
	创建描述符
	   ↓
	分配Cookie
	   ↓
	提交到虚拟通道
	   ↓
	虚拟通道管理描述符队列
	   ↓
	硬件执行DMA传输
	   ↓
	使用Cookie查询完成状态	
	```
- 状态流程如下：
	```
	描述符: 已分配 -> 已提交 -> 已发出 -> 已完成
	Cookie: 分配 -> 使用中 -> 完成
	虚拟通道: 管理整个过程
	```
基于上面的介绍，提出几个相关问题：
1. 描述符和 cookie 是一一对应的吗？
	- 描述符通常是一个结构体，包含了执行 DMA 传输所需的所有信息。每个描述符代表一个单独的传输任务
	- cookie 是一个唯一的标识符，submit 之后会分配，用于跟踪和管理 dma 传输的状态
	- 大多数情况下是一一对应的，但是也可能一个描述符对应多个 cookie(多个 submit)
2. cookie 的值是怎么来的？completed_cookie 会在什么时候用？
	- cookie 的值是在执行了 `tx->submit` 之后根据 `chan->cookie + 1`分配的
		- `chan->cookie` 是在 `dw_probe` 的时候执行 `vchan_init` 的时候分配的 `chan->cookie = DMA_MIN_COOKIE`
	- completed_cookie 是在 `dma interrupt` 中根据判断 `0x188` 寄存器的状态更新的
		- 如果 dma 传输完成/产生 error，`tx->chan->completed_cookie = tx->cookie`
3. 虚拟描述符和实际的描述符是怎么对应的？



## 数据结构


### axi-dma 相关

- axi dma 通道：包含已分配的硬件描述符计数器、对应的虚拟 dma 通道、对应的 axi_dma_desc 描述符
```c
struct axi_dma_chan {
// 对应的 axi dma ip
struct axi_dma_chip *chip;
// 通道对应的通道:例如 chan0 是 0xfcc00100、chan1 是 fcc00200
void __iomem *chan_regs;
// 通道 ID
u8 id;
// 握手信号
u8 hw_handshake_num;
// 已分配的硬件描述符计数器
atomic_t descs_allocated;

  
// 描述符对应的 dma 内存池
struct dma_pool *desc_pool;
// 虚拟 dma 通道
struct virt_dma_chan vc;

  
// 对应的 dma 描述符
struct axi_dma_desc *desc;
// 对应的 dma 配置
struct dma_slave_config config;
// dma 传输方向
enum dma_transfer_direction direction;
// 标志位：是否为循环传输模式
bool cyclic;

/* these other elements are all protected by vc.lock */
// 是否为暂停状态(由 vc.lock 保护)
bool is_paused;

};
```

- axi dma 描述符：用于传输的时候和 `virt_dma_hw_desc、axi_dma_chan` 产生联系，但是实际传输时不是使用这个结构
```c
struct axi_dma_desc {
	struct axi_dma_hw_desc *hw_desc;
	struct virt_dma_desc vd;
	struct axi_dma_chan *chan;
	u32 completed_blocks;
	u32 length;
	u32 period_len;
}
```


- axi dma 实际的描述符
```c
struct axi_dma_hw_desc {
	// 指向的链表项指针
	struct axi_dma_lli *lli;
	// 链表项所在的物理地址
	dma_addr_t  llp;
	// 传输长度
	u32 len;
};
```



- dma 异步发送描述符
```c
struct dma_async_tx_descriptor {
	// cookie 用于跟踪传输状态，如果 cookie 是 -EBUSY 表示该描述符正在依赖列表中等待
	dma_cookie_t cookie;
	// flags：来自用户程序
	enum dma_ctrl_flags flags; /* not a 'long' to pack with cookie */
	// 描述符的物理地址，  ??? 哪里映射的	
	dma_addr_t phys;
	// 该操作的目标 DMA 通道	
	struct dma_chan *chan;
	// 提交描述符的函数指针；分配 cookie 并将描述符标记为 pending，在 issue_pending() 调用的时候处理
	dma_cookie_t (*tx_submit)(struct dma_async_tx_descriptor *tx);
	// 释放描述符的函数指针	
	int (*desc_free)(struct dma_async_tx_descriptor *tx);
	// dma 传输完成的回调函数，这里由用户注册
	dma_async_tx_callback callback;
	// 带结果的回调
	dma_async_tx_callback_result callback_result;
	// 回调的参数
	void *callback_param;
	// dma 映射的数据结构指针
	struct dmaengine_unmap_data *unmap;

#ifdef CONFIG_ASYNC_TX_ENABLE_CHANNEL_SWITCH

	struct dma_async_tx_descriptor *next;
	
	struct dma_async_tx_descriptor *parent;
	
	spinlock_t lock;

#endif

};
```



## 驱动流程


## AXI-DMA

### dw_probe
```c
struct axi_dma_chip *chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
struct dw_axi_dma *dw = devm_kzalloc(&pdev->dev, sizeof(*dw), GFP_KERNEL);
struct dw_aix_dma_hcfg *hdata = devm_kzalloc(&pdev->dev, sizeof(*hdata), GFP_KERNEL);


chip->dw = dw;
chip->dev = &pdev->dev;
chip->dw->hdata = hdata;

chip->regs = dev_ioremap_resource(&pdev->dev, mem);

ret = parse_device_properties(chip);

// 申请 hdata->nr_channels 个 sizeof(*dw->chan) 这么大的空间，用 dw->chan 保存
dw->chan = devm_kcalloc(chip->dev, hdata->nr_channels, sizeof(*dw->chan), GFP_KERNEL);

// 初始化每个 chan
for (i = 0; i < hdata->nr_channels; i++) {
	struct axi-dma_chan *chan = &dw->chan[i];

	chan->chip = chip;
	chan->id = i;
	chan->chan_regs = chip->regs + COMMON_REG_LEN + i * CHAN_REG_LEN;
	chan->descs_allocated = 0;
	chan->vc.desc_free = vchan_desc_put;
	vchan_init(&chan->vc, &dw->dma);

	// vc->chan->cookie = DMA_MIN_COOKIE;
	// vc->chan->completed_cookie = DMA_MIN_COOKIE;
	// INIT_LIST_HEAD(&vc->desc_allocated);
	// INIT_LIST_HEAD(&vc->desc_submitted);
	// INIT_LIST_HEAD(&vc->desc_issued);
	// INIT_LIST_HEAD(&vc->desc_completed);

	// tasklet_init(&vc->task, vchan_complete, (unsigned long)vc);
	// vc->chan.device = dmadev;
	// list_add_tail(&vd->chan.device_node, &dmadev->channels);
}
// 配置 struct dma_device dma;
dma_cap_set(DMA_EMECPY | DMA_SLAVE | DMA_CYCLIC, dw->dma.cap_mask);

dw->dma.xxxxx  = xxxx;

// enable dma、irq
ret = axi_dma_resume(chip);

// disable all channel INTSTATUS、disable all channel(DMAC_CHEN)
axi_dma_hw_init(chip);

// 将 dma device 注册到系统中去
ret = dma_async_device_register(&dw->dma);
```


- `device_alloc_chan_resources`：从 dma 内存池申请 64byte 对齐的区域，使用 `chan->desc_pool` 保存
- `device_prep_dma_memcpy`：根据`src_adr、dst_adr、len` 初始化好链表，并将这些链表项都链接起来
- `device_issur_pending`：将 `tx_submit` 链表中的节点都加入到 `issue_pending` 链表中，并将 `tx_submit` 链表都初始化成 0；根据描述符和 `chan` 配置好 ctl、cfg 寄存器，使能 dma 开始传输


### dw_axi_dma_interrupt(int irq, void \*dev_id)
```c
static irqreturn_t dw_axi_dma_interrupt(int irq, void *dev_id)
{
	struct axi_dma_chip *chip = dev_id;
	struct dw_axi_dma *dw = chip->dw;
	struct axi_dma_chan *chan;

	u32 status, i;

	/* Disable DMAC interrupts. We'll enable them after processing channels */
	axi_dma_irq_disable(chip);

	/* Poll, clear and process every channel interrupt status */
	for (i = 0; i < dw->hdata->nr_channels; i++) {
		chan = &dw->chan[i];
		status = axi_chan_irq_read(chan);
		axi_chan_irq_clear(chan, status);

		dev_vdbg(chip->dev, "%s %u IRQ status: 0x%08x\n",
			axi_chan_name(chan), i, status);

		if (status & DWAXIDMAC_IRQ_ALL_ERR)
			axi_chan_handle_err(chan, status);
		else if (status & DWAXIDMAC_IRQ_DMA_TRF)
			axi_chan_block_xfer_complete(chan);
	}

	/* Re-enable interrupts */
	axi_dma_irq_enable(chip);

	return IRQ_HANDLED;
}
```


###### axi_chan_handle_err()
```c
axi_chan_disable(chan);

// 从 vc->desc_issued 链表中找到第一个节点
vd = vchan_next_desc(&chan->vc);
// 从 vc->desc_issued 链表中删除这个节点
list_del(&vd->node);
// 打印所有 chan->desc_allocated 的链表寄存器
axi_chan_list_dump_lli(chan, vd_to_axi_desc(vd));
// 配置 chan->completed_cookie = tx->cookie
// 配置 vd->tx.cookie = 0
// 将 node 链接到 desc_completed 链表上
// 调度执行 vchan_complete
vchan_cookie_complete(vd);
// 如果还有要传输的会继续传
axi_chan_start_first_queued(chan);
```

###### axi_chan_block_xfer_complete
```c
if (axi_chan_is_hw_enable(chan)) {
	// bug：DMA_TRF is caught，but channel not idle
	axi-chan_disable(chan);
}

// 从 vc->desc_issued 链表中找到第一个节点
vd = vchan_next_desc(&chan->vc);

// 从 desc_issued 链表中删除这个节点
list_del(&vd->node);
// 配置 chan->completed_cookie = tx->cookie
// 配置 vd->tx.cookie = 0
// 将 node 链接到 desc_completed 链表上
// 调度执行 vchan_complete
vchan_cookie_complete(vd);
	// cookie = vd->tx.cookie;
	// vd->tx->chan->completed_cookie = vd->tx.cookie
	// vd->tx.cookie = 0;
	// list_add_tail(&vd->node, &vc->desc_completed);
	// tasklet_schdule(&vc->task);
// 如果还有要传输的会继续传
axi_chan_start_first_queued(chan);
```








### dmatest

#### dma_request_channel
```c
// 根据 mask、filter、params 找到对应的第一个 chan
struct dma_chan *chan = dma_request_channel(mask, filter, params);
if (chan){
	// 找到对应的 chan，dmatest 添加测试线程
	if (dmatest_add_channel(info, chan)){
		dma_release_channel(chan);
		break;
	}
}
```

#### dmatest_func

##### `struct dma_async_tx_descriptor`
```c
// 组织好数据和长度
...

// device_prep_dma_memcpy 回调会返回一个 tx 描述符
struct dma_async_tx_descriptor *tx = dev->device_prep_dma_memcpy(chan, dsts[0] + dst_off, srcs[0], len, flags);
```

![[Pasted image 20241121153856.png]]


##### cookie

![[Pasted image 20241121170003.png]]


```c
// 这里注册的 callback 会配置 done 为 1
// dw_probe 的时候为每个通道 vchan_init 时会创建一个 tasklet
	// task_init(&vc_task, vchan_complete, (unsigned long)vc);
// vchan_complete 中会根据 tx 拿到 tx->callback，然后执行 dmatest_callback
// vchan_complete 会在 axidma 的中断中调用 vchan_cookie_complete 在里面执行 tasklet_schedule(&vc->task); 从而执行到 vchan_complete
tx->callback = dmatest_callback;
tx->callback_param = &done;
// 这里根据 struct dma_asyn_tx_descriptor *tx 拿到一个 cookie
cookie = tx->tx_submit(tx);
```

![[Pasted image 20241121155109.png]]


##### dma_async_issue_pending(chan)

![[Pasted image 20241121160620.png]]



##### dma_async_is_tx_complete(chan, cookie, NULL, NULL)

```c
static inline enum dma_status dma_async_is_tx_complete(struct dma_chan *chan, 
			dma_cookie_t cookie, dma_cookie_t *last, dma_cookie_t *used) 
{
	status = chan->device->device_tx_status(chan, cookie, &state);
	return status;
}
```

![[Pasted image 20241121162714.png]]


>[!info]
> `chan->cookie、chan->completed_cookie` 由 axi dma 的中断服务程序更新








- 上层用户都是操作 `dma_chan`，然后根据 `dma_chan` 可以进行 `dmaengine_prep_xxx` 来拿到对应的 `struct dma_async_tx_descriptor`，这样就可以提交 `submit`拿到 `cookie`，进而用 `dma_chan` 来产生一个 `issue_pending`
- 中间层是 `virt_dma`
- 底层是 `axi_dma`


# 全志 Linux DMAC 开发指南

## DMA Engine 框架图
![[Pasted image 20241126142721.png]]

## 源码结构
![[Pasted image 20241126142923.png]]



## API 接口
```c
// 申请一个可用通道，返回 dma 通道操作句柄(在 linux-5.4 分支上需要使用 dma_request_chan)
// 参数：mask：所有申请的传输类型的掩码
// 参数 fn：DMA驱动私有的过滤函数，假如有多个 dma 设备，用于选择对应的 dma 设备，可以为 NULL
// 参数 fn_param：过滤函数的参数
// 返回：成功返回 dma 通道操作句柄；失败返回 NULL
struct dma_chan *dma_request_channel(const dma_cap_mask_t *mask, dma_filter_fn fn, void * fn_param)


// 参数 name：通道名字，与设备数的 dma-names 对应
struct dma_chan *dma_request_chan(struct device *dev, const char *name)


// 释放指定的 dma 通道
// 参数 chan：指向要释放的 dma 通道句柄
void dma_release_channel(struct dma_chan *chan)




// 配置 dma 通道的 slave 信息
// 参数 chan:指向要操作的 dma 通道句柄
// 参数 config: dma 通道 slave 的参数
// 成功返回 0；失败返回 错误码
int dmaengine_slave_config(struct dma_chan *chan, struct dma_slave_config *config)



// 准备一次单包传输
// 参数 chan：指向要操作的 dma 通道句柄
// 参数 sgl：散列表地址。此散列表传输之前需要建立
// 参数 sg_len：散列表内 buffer 的个数
// 参数 dma_transfer_direction dir：传输方向，此处为 DMA_MEM_TO_DEV，DMA_DEV_TO_MEM
// flags：传输标志
// 成功返回一个传输描述符指针；失败返回 NULL
struct dma_async_tx_descriptor *dmaengine_prep_slave_sg(struct dma_chan *chan, struct scatterlist *sgl, unsigned int sg_len, enum dma_transfer_direction dir, unsigned long flags, void *context)



// 准备一次环形 buffer 传输
// 参数 chan：指向要操作的 dma 通道句柄
// 参数 buf_addr：目标地址
// 参数 buf_len：环形 buffer 的长度
// 参数 period_len：每一小片 buffer 的长度
// 参数 dma_transfer_direction dir:传输方向，此处为 DMA_MEM_TO_DEV，DMA_DEV_TO_MEM
// 参数 flags：传输标志
// 成功返回一个传输描述符指针；失败返回 NULL
struct dma_async_tx_descriptor *dmaengine_pre_dma_cylic(strcut dma_chan *chan, dma_addr_t buf_addr, size_t buf_len, size_t period_len, enum dma_transfer_direction  dir, unsigned long flags)




// 提交已经做好准备的传输
// 参数 desc：指向要提交的传输描述符
// 成功返回一个大于 0 的 cookie；失败返回错误码
dma_cookie_t dmaengine_submit(struct dma_async_tx_descriptor *desc)


// 启动通道传输
// 参数 chan：指向要使用的通道
void dma_async_issue_pending(struct dma_chan *chan)


// 停止通道上的所有传输(会丢弃未开始的传输)
// 参数 chan：指向要终止的通道
// 成功返回 0；失败返回错误码
int dmaengine_terminate_all(struct dma_chan *chan)


// 暂停某通道的传输
// 参数 chan：指向要暂停的通道
// 成功返回 0；失败返回错误码
int dmaengine_pause(struct dma_chan *chan)


// 恢复某通道的传输
// 参数 chan：指向要恢复的通道
// 成功返回 0；失败返回错误码
int dmaengine_resume(struct dma_chan *chan)

// 查询某次提交的状态
// 参数 chan：指向要查询传输状态的通道
// 参数 cookie：dmaengine_submit 接口返回的 cookie
// 参数 state：用于获取状态的变量地址
// 返回 DMA_SUCCESS 表示传输成功完成
// 返回 DMA_IN_PROGRESS 表示提交尚未处理或处理中
// 返回 DMA_PAUSE 表示传输已经暂停
// 返回 DMA_ERROR 表示传输失败
enum dma_status dmaengine_tx_status(struct dma_chan *chan, dma_cookie_t cookie, struct dma_tx_state *state)
```

>[!info]
>回调函数里不允许休眠，以及调度
>回调函数时间不宜过长
>pending 并不是立即传输而是等待软中断的到来，cyclic 模式除外



## Example
```c
struct dma_chan *chan;
dma_cap_mask_t mask;
dma_cookie_t cookie;
struct dma_slave_config config;
struct dma_tx_state state;
struct dma_async_tx_descriptor *tx = NULL;
void *src_buf;
dma_addr_t src_dma;

dma_cap_zero(mask);
dma_cap_set(DMA_SLAVE, mask);
dma_cap_set(DMA_CYCLIC, mask);

/* 申请一个可用通道 */
chan = dma_request_channel(dt->mask, NULL, NULL);
if (!chan){
	return -EINVAL;
}

src_buf = kmalloc(1024*4, GFP_KERNEL);
if (!src_buf) {
	dma_release_channel(chan);
	return -EINVAL;
}

/* 映射地址用DMA访问 */
src_dma = dma_map_single(NULL, src_buf, 1024*4, DMA_TO_DEVICE);
　　
config.direction = DMA_MEM_TO_DEV;
config.src_addr = src_dma;
config.dst_addr = 0x01c;
config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
config.src_maxburst = 1;
config.dst_maxburst = 1;
config.slave_id = sunxi_slave_id(DRQDST_AUDIO_CODEC, DRQSRC_SDRAM);

dmaengine_slave_config(chan, &config);

　　tx = dmaengine_pre_dma_cyclic(chan, scr_dma, 1024*4, 1024, DMA_MEM_TO_DEV,
　　 DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
　　
　　/* 设置回调函数 */
　　tx->callback = dma_callback;
　　tx->callback = NULL;
　　
　　/* 提交及启动传输 */
　　cookie = dmaengine_submit(tx);
　　dma_async_issue_pending(chan);

// 后面还可以查一下状态
```


## 设备树
```dts
/* DMA控制器设备树节点 */
dma: dma-controller@01c02000 {
    compatible = "allwinner,sun8i-v3s-dma";
    reg = <0x01c02000 0x1000>;
    interrupts = <GIC_SPI 50 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&ccu CLK_BUS_DMA>;
    resets = <&ccu RST_BUS_DMA>;
    #dma-cells = <1>;
};
/* DMA客户端设备树节点，以SPI为例 */
spi2: spi@01c6a000 {
        compatible = "allwinner,sun6i-a31-spi";
        reg = <0x01c6a000 0x1000>;
        interrupts = <0 67 4>;
        clocks = <&ahb1_gates 22>, <&spi2_clk>;
        clock-names = "ahb", "mod";
        dmas = <&dma 25>, <&dma 25>;
        dma-names = "rx", "tx";
        resets = <&ahb1_rst 22>;
};
```
- `dmas`：该属性一共有两个，第一个 DMA 控制器的节点名，第二个是驱动的端口
- `dma-names`：该属性用于 `dma_request_chan` 接口，传入该接口中的 name 参数需要与设备树中的 `dma-names` 一直，这样才能申请到 DMA 通道


## dma debug 宏
```dts
Device Drivers  --->
	[*] DMA Engine support  --->
		[*]   DMA Engine debugging
		[*]     DMA Engine verbose debugging
```







# ahb dma 对接 dw spi
https://git.nationalchip.com/gerrit/#/c/13870/3




### 梳理 dw_spi --  dma 驱动





- spi-dw-gx.c 设备树匹配会执行这里的  probe
	- `dw_spi_dma_init`
	```c
	// 1. Init tx channel
	dws->txchan = dma_request_channel(mask, mid_spi_dma_chan_filter, txs);
	dws->master->dma_tx = dws->txchan;
	// 2. Init rx channel
	dws->master->dma_rx = dws->rxchan;

	dws->dma_initedj = 1;
	```
	- `dw_spi_can_dma`
	```c
	if (!dws->dma_inited)
		return false;
	return xfer->len > dws->tx_fifo_len;
	```
	- `dw_spi_dma_setup`
	```c
	dw_writel(dws, DMARDLR, 7);
	dw_writel(dws, DMATDLR, 0x10);
	dws->transfer_handler = dma_transfer;
	```
	- `dw_spi_dma_transfer`
	```c
	struct dma_async_tx_descriptor *tx_desc, *rxdesc;

	txdesc = dw_spi_dma_prepare_tx(dws, xfer);

		struct dma_slave_config tx_conf;
		struct dma_async_tx_descriptor *txdesc;

		txconf.direction = DMA_MEM_TO_DEV;
		txconf.dst_addr = dws->dma_addr;
		txconf.dst_addr_width = convert_dma_width(dws->dma_width);
		txconf.device_fc = false;

		dmaengine_slave_config(dws->tx_chan, &txconf);

		txdesc = dmaengine_prep_slave_sg(dws->txchan,
					xfer->tx_sg.sgl,
					xfer->tx_sg.nents,
					DMA_MEM_TO_DEV,
					DMA_PERP_INTERRUPT | DMA_CTRL_ACK);
		txdesc->callback = dw_spi_dma_tx_done;
		txdesc->callback_param = dws;
		return txdesc;


	rxdesc = dw_spi_dma_prepare_rx(dws, xfer);

	if (rxdesc) {
		dmaengine_submit(rxdesc);
		dma_async_issue_pending(dws->rxchan);
	}

	if (txdesc) {
		dmaengine_submit(txdesc);
		dma_async_issue_pending(dws->txchan);
	}
	```
- spi-dw.c：probe 成功后会调用 dw_spi_add_host 到了这个文件
	- `dw_spi_add_host`
	```c
	dws->dma_inited = 0;
	dws->dma_addr = (dma_addr_t)(dws->paddr + DW_SPI_DR);

	...

	if (dws->dma_ops && dws->dma_ops->dma_init) {
		ret = dws->dma_ops->dma_init(dws);
		master->can_dma = dws->dma_ops->can_dma;
	}
	```
	- `transfer_one`
		```c
		if (dws->dma_mapped) {
			ret = dws->dma_ops->dma_setup(dws, transfer);
		}
		if (dws->dma_mapped) {
			ret = dws->dma_ops->dma_transfer(dws, transfer);
		}
		```



# SPI DMA 相关问题解答

## Q1: slave_config 和 device_prep_dma_sg 这两个里面都会配置 src_addr
-  slave_config 是一次性的基础配置，用于配置 DMA 通道的基本参数；
 - device_prep_dma_sg 是用于准备具体的一次 DMA 传输，每次传输都需要重新设置

## Q2: device_prep_dma_sg 和 device_prep_slave_sg 有什么差异？
- device_prep_dma_sg 是用于内存到内存之间的传输，源和目的都是内存地址
- device_prep_slave_sg 是用于外设与内存之间的传输，一端固定位外设地址(已用 dma_slave_config 配置)
- 差异：
	- slave_sg 只需提供内存端地址，外设地址已通过 dma_slave_config 配置；dma_sg 需要同时提供源和目的地址；
	- slave_sg 用于外设数据传输；dma_sg 用于大块内存拷贝   

## Q3: sg 是怎么构建的？在 spi-dw-mid.c 的参数中就有 sg 相关的信息
- 由上层应用构建的 sg，比如 flash 驱动在使用 spi 的时候会构建这样的一个 sg，然后放在 `struct spi_transfer *transfer` 中传下来
- 纠正：
	- 上层应用并不会构建 sg，而是构建 spi_message，由 spi.c 来实现将 spi_message 中的信息转换成 sg'
	- 可以用 sg_init_one、dma_map_sg 自己构建 sg，只需要地址、长度、散列表的个数
	- `sg_init_one(&rx_sg, dws->buffer, dma_len)` 函数只是一次性的初始化配置，它会将长度信息保存在 `scatterlist` 结构体中。<span style="background:rgba(160, 204, 246, 0.55)">一旦初始化完成，即使是变量发生变化，`scatterlist` 中保存的长度值也不会改变</span>

## Q4: dma_map_single、dma_map_sg 的差异？
- `dma_map_single` 会根据虚拟地址映射出一个 `dma_addr_t` 类型的地址返回回来，后续用户用这个地址
- `dma_map_sg` 也是根据虚拟地址映射出一个 `dma_addr_t` 类型的地址返回回来，但是直接更新到了 `sg->dma_address`，用户不需要换地址
- `dma_map_single` 用于单个连续内存块的映射; `dma_map_sg` 用于多个可能不连续的内存块
- `dma_map_single` 完成后需要使用 `dma_map_error` 函数来判断返回的地址是否可用；`dma_map_sg` 直接判断返回值是否为 0 即可


## 如何理解 axi_dma_hw_desc、axi_dma_desc、virt_dma_desc、cookie、axi_dma_chan、virt_dma_chan 这些东西


- `axi_dma_hw_desc`：
	- 实际的硬件描述符，直接与 DMA硬件操作相关联。
	- 包含实际的 dma 传输信息
- `axi_dma_desc`：
	- 软件描述符，封装了一个或多个 `axi_dma_hw_desc`。
	- 用于管理 DMA 传输的高级细节，不直接与硬件交互。
	- 并且<span style="background:#40a9ff">从 `struct axi_dma_chan` 中可以找到 `axi_dma_desc` 而找不到 `axi_dma_hw_desc`</span>
- `virt_dma_desc`：
	- 虚拟 DMA 描述符。
	- 是 DMAEngine 的组成部分，用于抽象和管理 DMA描述符
	- `struct axi_dma_desc` 结构中会包含一个 `virt_dma_desc`
- `cookie`：
	- 用于跟踪和管理特定的 dma 传输。
	- 当一个 dma 传输 submit 后，会被分配一个 cookie
- axi_dma_chan：
	- 特定的 dma 通道，包含通道的配置、状态等
	- `axi_dma_chan` 有对应的 `struct axi_dma_desc`，包含一个 `virt_dma_chan`
- virt_dma_chan:
	- 虚拟 dma 通道，是 DMAEngine 的一个通用组件。
	- 用于抽象和管理具体的 DMA通道，例如 (axi_dma_chan)


# 进度

## 目前对外设不通 
<font color=red>(看寄存器都没啥问题，但是用的是 Flash SPI 和 AXI-DMA 对接的，需要换成 GEN SPI)</font>




提供几个思路：
1. 用逻辑分析仪抓一下波形，看看是不是有波形产生；在进行 dma 操作之前肯定要保证 flash 有数据出来，并且到了 spi 的 fifo
2. axi-dma 对内存是通的，那么就需要分析对比下对内存和对外设之间的差异
  - 握手信号
  - Master 选择
  - 对内存用的 `prep_device_memcpy`
    - 这个接口不能用来对外设吗？好像确实不能
  - 对外设用的 `prep_device_slave_sg`
    - 两个接口有什么差异？
  - 现在是都没看到 axi-dma 的中断产生的
3. 有没有什么办法可以看到 axi-dma 的寄存器值？
  - 用一个全局变量来控制，先启动的时候不用 dma，然后起来了之后再用 dma
  - axi-dma-platform.c 中有没有 debug 的寄存器打印
  - 或者其他的方法
  - 要关心的也没几个寄存器，所以慢慢来，别着急
    - 第一部分：slave_config 的时候
    - 第二部分：链表配置的时候
4. 用设备树的方式配置的 dma 通道以及握手信号
  ```dts
  // UART驱动中
  struct dma_chan *txchan, *rxchan;
  
  // 请求TX通道
  txchan = of_dma_request_slave_channel(dev->of_node, "tx");
  // --> 内部会调用 dw_axi_dma_of_xlate(&dma_spec = {args[0] = 0})
  
  // 请求RX通道
  rxchan = of_dma_request_slave_channel(dev->of_node, "rx");
  // --> 内部会调用 dw_axi_dma_of_xlate(&dma_spec = {args[0] = 1})
  
  // SPI驱动中
  struct dma_chan *txchan, *rxchan;
  
  // 请求TX通道
  txchan = of_dma_request_slave_channel(dev->of_node, "tx");
  // --> 内部会调用 dw_axi_dma_of_xlate(&dma_spec = {args[0] = 2})
  
  // 请求RX通道
  rxchan = of_dma_request_slave_channel(dev->of_node, "rx");
  // --> 内部会调用 dw_axi_dma_of_xlate(&dma_spec = {args[0] = 3})
```



https://github.com/torvalds/linux/blob/b86545e02e8c22fb89218f29d381fa8e8b91d815/arch/arm64/boot/dts/intel/socfpga_agilex5.dtsi#L267


### dw-axi-dma-platform.c 修改

1. 使用 `Prep_slave_sg` 的时候，源固定用的是 Master0，看看能否根据方向来
	- 新提供了一个接口 `set_desc_for_master` 用于根据 `direction` 来配置 `SRC、DST` 使用哪个 `Master`
2. 默认 axi-dma 驱动用的是 `CH_CFG` 寄存器，而实际应该用 `CH_CFG2` 寄存器
	- 驱动中直接注释掉用 0 的
3. 握手信号是从设备树拿的，现在没做设备树，直接配死握手信号是 `20`
4. 从头看一遍代码：打印把配置的所有寄存器打出来
```c
dw_probe


alloc_chan_resources

dw_axi_dma_chan_slave_config

dw_axi_dma_chan_prep_slave_sg
	dw_axi_dma_set_hw_desc //根据参数把 hw_desc 都配好了
	write_desc_llp // 把 llp 配给寄存器		

issue_pending
	axi_chan_block_xfer_start // 至此 axi_chan 已经使能

```
5. 目前用 `dma_async_is_tx_complete` 接口看还是没有完成的 
6. 去掉设备树中的 `axi-burst`
	- 也还是一样没有 dma 的中断产生
	- 看 spi、dma 的寄存器 
		- 数据还在 spi 的寄存器里面，说明 dma 还没去拿，并且 spi 已经把数据准备好了 




### 根文件系统是 ro，没有网络，没有 usb ，要怎么操作？
- 设备树中加一个 `app` 分区试试看，但是如何指定 app 分区的类型呢？

- 直接烧好了，起来之后用 gdb 连上去 
	- axi-dma 的基地址：`0xc48ae000` 




### 确认 AXI-DMA、SPI 的寄存器

#### AXI-DMA (0xc48ae000)
```
(gdb) x/20x 0xc48ae000
0xc48ae000:	0x00000000	0x00000000	0x3130322a	0x00000000
0xc48ae010:	0x00000003	0x00000000	0x00000000	0x00000000
0xc48ae020:	0x00000000	0x00000000	0x00000000	0x00000000
0xc48ae030:	0x00010000	0x00000000	Cannot access memory at address 0xc48ae038
(gdb) x/20x 0xc48ae000+0x100
0xc48ae100:	0x00000000	0x00000000	0x00000000	0x00000000
0xc48ae110:	0x00000000	0x00000000	0x00001200	0x00000000
0xc48ae120:	0x0000014f	0x00000002	0x038c0000	0x00000000
0xc48ae130:	0x00000000	0x00000000	0x00000000	0x00000000
0xc48ae140:	0x00000000	0x00000000	Cannot access memory at address 0xc48ae148
(gdb) x/20x 0xc48ae000+0x180
0xc48ae180:	0x27ffffe6	0xffffffff	0x00040000	0x00000000
0xc48ae190:	0x07ffffe6	0xffffffff	Cannot access memory at address 0xc48ae198

```


#### DW-SPI (0xc4d4b000)


```
(gdb) x/20x 0xc48ae000
0xc48ae000:	0x00000000	0x00000000	0x3130322a	0x00000000
0xc48ae010:	0x00000003	0x00000000	0x00000000	0x00000000
0xc48ae020:	0x00000000	0x00000000	0x00000000	0x00000000
0xc48ae030:	0x00000000	0x00000000	Cannot access memory at address 0xc48ae038
(gdb) x/20x 0xc48ae000+0x100
0xc48ae100:	0x00000000	0x00000000	0x00000000	0x00000000
0xc48ae110:	0x00000000	0x00000000	0x00001200	0x00000000
0xc48ae120:	0x0000014f	0x00000002	0x038c0000	0x00000000
0xc48ae130:	0x00000000	0x00000000	0x00000000	0x00000000
0xc48ae140:	0x00000000	0x00000000	Cannot access memory at address 0xc48ae148
(gdb) x/20x 0xc48ae000+0x180
0xc48ae180:	0x27ffffe6	0xffffffff	0x00040000	0x00000000
0xc48ae190:	0x07ffffe6	0xffffffff	Cannot access memory at address 0xc48ae198
(gdb) x/20x 0xc4d4b000
0xc4d4b000:	0x00400807	0x000000ff	0x00000001	0x00000000
0xc4d4b010:	0x00000001	0x00000004	0x00000000	0x00000000
0xc4d4b020:	0x00000000	0x00000040	0x0000001e	0x0000018e
0xc4d4b030:	0x00000008	0x00000019	0x00000000	0x00000001
0xc4d4b040:	0x00000000	0x00000000	0x00000000	0x00000000
```

`hw_desc->lli = axi_desc_get(chan, &hw_desc->llp);`
虚拟地址存在 `hw_desc->lli`，物理地址存在 `&hw_desc->llp`


![[Pasted image 20241217150245.png]]



### 用 V13P RC 的 bit，换成使用 GEN SPI
- V13P RC 的 bit 默认编的是 8 线的 flash_spi，在 flash 这个位置对应的是 gen_spi1
	- 驱动中修改 dw_spi_v2 默认使用 gen_spi1，确保 loader 可以起来
		- ok
	- 然后修改 linux 的设备树，确保使用 gen_spi1 可以让 kernel 跑起来 
		- 读到错误的 ID，正确应该是 0xef4018，读成了 0x401800
			- 是由于 gen_spi 在单线模式下不能配置 0xf4 为 0x202，需要配成 0x0
- 目前用的是 `dmac2` 的 `bit`，好像这个 `bit` 网络也不能用，换更早一点的 `bit` 试试看 
	- 早的 `rc1 bit` 无法启动，`rc2` 有问题，不打 `x`，`rc3` 网络就不能用了了
	- 更早的 `beta` 也无法启动
	- 还是得用 `rc` 版本的 `dmac2 bit` 
- 我想继续起 `kernel`，应该烧到 `W25Q128`？
	- 后面的驱动应该都是用这个交互的，所以需要烧到 `W25Q128` 
	- 发现无法挂载根文件系统，可能是 `kernel` 中 `gen spi` 的驱动有问题吗？
		- 还原 `gen spi` 的驱动，还是用 `dw_spi`，看看可不可以起来 
		- 不行啊，因为 dw_spi 用的是 `8` 线 flash
	- 换一个根文件系统试试看 

- 用 rc1 的 bit 测试，为啥起不来啊 ?
	- 直接 gdb 都连不上，别搞这个 bit 了

- 用 `dmac2` 的 `bit`，kernel 中把 `flash_spi --> gen_spi` 先完成这一步再说吧
	- 一样的现象，无法挂载根文件系统 
	- 用 gdb 的方式来做，loader 里面不是到了 stage2 吗，我直接连上 gdb，然后加载 kernel，rootfs 这个时候应该已经在 flash 里面了，这样好像也还是一样的，无非就是从 flash 启动变成了 gdb 加载，并且 cmdline 是从 bootchosen 中取的 
		- 是的，也还是一样的现象 
- 可以到 kernel 了，看之前 `gxloader` 支持 `gen_spi` 时添加补丁需要修改大小端 
	- 感觉还是可以查一下 `linux` 启动挂文件系统的那一块，逻辑分析仪也有，`gdb` 也有环境可以查 
	- ![[Pasted image 20241218144214.png]]
	- 可以直接打开 `squashfs`  相关的所有打印，一般来说可以看出来有啥问题 
		- 报了下面这个错误，显示 `sb->s_magic` 和 `SQUASHFS_MAGIC` 大小端反了
```c
	/* Check it is a SQUASHFS superblock */
	sb->s_magic = le32_to_cpu(sblk->s_magic);
	if (sb->s_magic != SQUASHFS_MAGIC) {
//		if (!silent)
			ERROR("Can't find a SQUASHFS superblock on %pg\n",
						sb->s_bdev);
		goto failed_mount;
	}

```
- 使用 `gen spi` 可以正常跑 `linux`

### 继续测试 `gen spi` 对接 `axi dma`
- 测试的时候会每个通道都跑一遍，是因为在 `dma_request_channel(mask, NULL, NULL)` 的时候没有设置回调函数和过滤条件 
	- 修改了过滤条件之后只有一个通道会操作了
	- 发现 `dma` 有 `tfr` 中断，但是 `spi fifo`  溢出了，并且最后 `spi fifo` 里面还有数据没被取走，应该就是 `dma` 没有从 `fifo` 中取数据，最后读到的文件里面也全是 `0x00`
	- 猜测 1：源是 SPI 的 FIFO，也需要转成物理地址？
		- 看了最新的 linux 代码，用的确实就是物理地址
		- 改成物理地址试一把，<font color=red>看下这里物理地址和虚拟地址有啥区别？</font>
			- 无法触发完成中断了，触发的是 R2WO 中断
				- 这里的 `R2WO` 应该是我用 `GDB` 读了 `0x188` 的很多寄存器导致的 
			- <font color=red>虚拟地址的时候 `genspi` 会出现 Receive FIFO Overflow，触发了 axi-dma 的完成中断，但是 spi 的 fifo 中还有数据，所以程序会一直卡住</font>
			- <font color=red>物理地址的时候 `genspi` 不会出现 Receive FIFO Overflow，触发了 axi-dma 的完成中断，spi 的 fifo 中也没有数据了 </font>
			- `dma:0xc48ae000     spi:0xc4d4b000`
		- 链表有效，即使只有一个链表的时候，是不是也应该是配成 1 的？
			- 是的，但是这一点看驱动也是已经配上过的，重新再配一次 
				- 再看还是最高 1bit 没了
		- 看 spi 的波形不对啊，但是发指令都是用 cpu 发的，先用 cpu 读一下看看波形，是不是逻辑分析仪接线有问题，然后再用 dma 的接口 
			- 用 cpu 的波形是对的，前面还会写一些东西，后面才是读操作
			- 用 dma 的时候前面写的都发了，后面读操作就把地址、指令都发出去了，但是没有数据回来，是不是 spi 这边没搞对？flash 的数据还没来，对比下 spi 的驱动 
				- 我去！先开 dma 啊，再开 spi 啊 
				![[Pasted image 20241218203219.png]]
		- 有一种可能，先开 spi，再开 dma，这样 spi 的数据直接就让 spi fifo 溢出了，因为没人取数据，应该先开 dma，再开 spi
		- 先把根文件系统里面的一个初始化啥的都去掉，不然跑一次太慢了 
		- <font color=green>改成先开 dma，后开 spi</font>
		- 抓波形，读的操作都是在很前面，这里应该是实际的读，后面应该是文件系统在操作 
		- 总结现象：先开 dma，后开 spi
			- 能够触发 dma 的完成中断，说明握手是对的
			- 并且链表配的应该没啥问题
			- 现在两个问题：
				1. 链表会进行多次，这个好像是正常的，因为看 cpu 读也是会读多次
				2. 取到的数据都是 0，没有出现 spi fifo 溢出了 
	- 猜测 2：使用 dma 映射的地址，后面还要拷贝回虚拟地址？
		- 使用 `dma` 映射的地址，在取消映射的时候会自己做一致性操作，目前没有做 `unmap`，加上试试
		- <font color=green>添加 unmap</font>
		- `U 盘` 变成只读的了，使用可视化界面，格式化掉 
		- 依旧是可以触发中断，但是没有搬数据，并且 mtd_debug read 的时候会返回出错：`flash_to_file: read, size`，看了下是 mtd_debug. c 调用 read 接口的时候的返回值 < 0
		- <font color = green>将 unmap 放到移动 dws->buffer 之前 </font>
		- 看起来好像是因为读失败了，会重复读多次 
	- 猜测 3：loader 里面 gen_spi 对接 dma 测一把，并且不使能 channel 的时候看下寄存器，并且传输完成了之后看下寄存器 
![[Pasted image 20241219153012.png]]
![[Pasted image 20241219153425.png]]


- Receive FIFO  Overflow
	- loader 中没有用 gen_spi 的驱动，所以起来的模式是单线？好像也不应该是单线 
	- loader 要用 `axidma` 目录下的 `.boot` ，而不能用 ` virgo_final ` 目录下的 `.boot ` 

#### Squashfs 坏掉了 
- 重新烧 `rootfs.bin`

#### mtd_read 失败 
目前现象：
- `read, size 0x1000, n 0x1000`
- dma 操作会跑多次，但是每次 dma 0 chan0 都有 DMA 传输完成中断，上层注册的回调也执行了 
- 内存 buffer 没有数据，全是 0x0，生成的文件里面也没数据
- 波形也有，flash 在出数据，并且出的是对的，说明 spi 已经把数据准备好了，并且 dma 也已经搬走了，但是搬的是什么数据？要看看

- mtd_read 会到 `mtd_read` 
- mtdchar. c 211 行看到的 kbuf 是对的，buf = 0xa92978
	- buf 是用户态的，kbuf 是 kernel 态的
![[Pasted image 20241219163103.png]]

- 那么现在看下 kbuf 到 buf 是不是对的
	- 用我们自己的 linux 测试程序
		- 和 mtd_debug 一样 
	- 换一个新的 mtd_debug
		- mtd_debug 里面的 read 只跑了一次 
	- 在文件系统中加上用户态的 debug_server
		- DebugServer 需要网络 
	- 换了新的 rootfs 之后文件系统会挂失败 
		- 不会失败 
	- 从 mtd_debug 调用 read 之后的调用栈：
		- 是的，会进 `SYSCALL_DEFINE3` 
![[Pasted image 20241219194028.png]]

- count = 0x100, retlen = 0x4
- `mtdchar_read` 里面跑了 11 次，第一次的数据是对的，最后第 11 次的时候 copy_to_user 失败了，返回-14，刚好就是 `Bad address` 
	- 感觉数据已经是读对了，那个 ret_len 错掉了，导致一直去读 
![[Pasted image 20241219201735.png]]

#### m.actual_length 返回值错误导致的

![[Pasted image 20241219203715.png]]


- 手动把这个值改成 `0x105` 就正常了，读到的数据也是对的 
- 原因是：`spi-gx-v2.c` 里面把 `dws->cur_msg->actual_length += block_len`，但是 `spi.c` 里面也会加，所以导致有问题 

- ok! 







# 相关接口 
## unmap_dma 地址里面干了什么？
- `dma_map_single`  将一片虚拟内存映射成物理地址 。内部会以 `cache line`  为最小粒度处理内存一致性，因此映射的区域必须 `cache line`  对齐。
- `dma_unmap_single` 取消先前映射的区域 

## dma_cap_set 有什么用？dma_request_channel 是如何申请到一个通道的? 
- 一般会先配置 `DMA` 的 `dma_cap_mask_t mask`，`mask` 会在 `dma_request_channel` 的时候用到，后续都不能用 
- `dma_cap_mask_t mask` 表示通道必须满足的能力 
- 在 `__dma_request_channel` 接口中会遍历所有的 `dma_device_list` ，根据 `chan = find_candidate(device, mask, fn, fn_param)` 来找到对应 `channel` 
	- `struct dma_chan *chan = private_candidate(mask, device, fn, fn_param)` 是实际的匹配过程 
		- `if (mask && !__dma_device_satisfies_mask(dev, mask))` 如果当前的 dma_device 不满足 `mask`，则直接返回 
			- 根据 `mask` 和 `dma_device` 所拥有的能力来进行匹配，先将 mask 和 `device->cap_mask.bits` 进行与操作，放到 `has.bits`
			- 然后比较 `mask.bits` 和 `has.bits` 是否相等，相等则认为满足
		- 如果 `dma device  -> cap_mask` 没有 `DMA_PRIVATE` 属性，则认为大家共用的 
		- 根据 `fn、fn_param` 来找到对应的通道 
	- 找到 `chan`，将 `device->cap_mask` 设置成 `DMA_PRIVATE` 避免后续有人申请到这个通道 
	- 调用 `dma_chan_get` 会回调 `chan->device->device_alloc_chan_resources(chan)` 来申请资源，到了 `axi-dma` 的回调 
	- 返回 `chan` 


## dmaengine_slave_config 是如何配置的？
- 采用 `struct dma_slave_config` 结构来配置
	```c
	struct dma_slave_config {
		// 传输方向
	 	enum dma_transfer_direction direction;
		// 源地址的物理地址
	 	phys_addr_t src_addr;
	 	// 目的地址的物理地址
	 	phys_addr_t dst_addr;
		// 源的传输位宽
	 	enum dma_slave_buswidth src_addr_width;
		// 目的的传输位宽
	 	enum dma_slave_buswidth dst_addr_width;
		// 一次 burst 中可以发送到设备的最大字数(字数 以 src_addr_width为单位，而不是字节; 通常是 I/O 外设上 fifo 深度的一般，这样就不会溢出, 对于内存可能有用也可能没用)
	 	u32 src_maxburst;
	 	// 与 src_maxburst 相同，但用于目标外设
	 	u32 dst_maxburst;
		// 如果外设是流控，则这里配成 true 
	 	bool device_fc;
		// 最新的 linux 代码已经废弃了，axi-dma 驱动中没有处理这个
	 	unsigned int slave_id;
	};
	```
- `axi-dma` 驱动的结构体中会嵌套这个结构，外设驱动在 `config` 的时候到了 `axi-dma`  直接 ` memcpy ` 配置到 ` axi-dma` 的结构中，就可以用了 



## Highmem、vmalloc、normal_mem
1. linux 中什么是 vmalloc 的 buffer？什么是 highmem？  
	- `vmalloc`：用于分配虚拟地址连续但物理地址不连续的内存，主要用于大块内存分配，在内核虚拟地址空间的 `VMALLOC` 区域 
		- 用 `vmalloc` 申请出来的 `buffer` 
	- highmem: 当物理内存的大小接近或超过虚拟内存的最大大小时，就会使用高内存。
		- 此时，内核不可能在任何时候都保持所有可用的物理内存的映射，这意味着内核需要开始使用它想用访问的物理内存的临时映射。
		- 没有被永久映射覆盖的那部分物理内存就是我们所说的 `highmem`
		- 物理内存布局：
		```
		0 ~ 896MB：ZONE_NORMAL (直接映射)
		> 896MB：ZONE_HIGHMEM (动态映射) 
		```
2. 为什么 vmalloc 的 buffer 和 highmem 需要用 vmalloc_to_page、offset_in_page、dma_map_page 的方式来映射？  
	- `vmalloc buffer` 是物理上不连续的一片地址，所以需要先获取实际的物理页，再对每个物理页单独做 `DMA` 映射 
	- `highmem` 不在直接映射区域，需要先建立临时内核映射，再获取实际的物理页，再进行 `DMA` 映射 
3. 为什么不是 vmalloc 的 buffer 或者不是 highmem ，直接用 dma_map_single 来映射就可以？
	- 普通内存的虚拟地址可以直接转换为物理地址，并且物理地址连续，可以一次性映射，不需要考虑分页问题 



## sg_init_table、sg_dma_address、sg_dma_len
```c
struct scatterlist {
#ifdef CONFIG_DEBUG_SG
	unsigned long	sg_magic;
#endif
	unsigned long	page_link;
	unsigned int	offset;
	unsigned int	length;
	dma_addr_t	dma_address;
#ifdef CONFIG_NEED_SG_DMA_LENGTH
	unsigned int	dma_length;
#endif
};

static inline void sg_mark_end(struct scatterlist *sg)
{
#ifdef CONFIG_DEBUG_SG
	BUG_ON(sg->sg_magic != SG_MAGIC);
#endif
	/*
	 * Set termination bit, clear potential chain bit
	 */
	sg->page_link |= 0x02;
	sg->page_link &= ~0x01;
}

void sg_init_table(struct scatterlist *sgl, unsigned int nents)
{
	memset(sgl, 0, sizeof(*sgl) * nents);
#ifdef CONFIG_DEBUG_SG
	{
		unsigned int i;
		for (i = 0; i < nents; i++)
			sgl[i].sg_magic = SG_MAGIC;
	}
#endif
	sg_mark_end(&sgl[nents - 1]);
}

#define sg_dma_address(sg)	((sg)->dma_address)

#define sg_dma_len(sg)		((sg)->length)
```


## dmaengine_prep_slave_sg
```c
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
		// 如果 sgl.len > axi_block_len，segment_len 会处理
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
```




## 一致性 dma 和流式 dma
- 任何合适的 dma 传输都需要何时的内存映射。`dma` 映射包括分配 `DMA buffer` 并为其生成总线地址。设备实际上使用总线地址，总线地址使用 `dma_addr_t` 来表示。
- 目前存在两种类型的映射：一致性 `dma` 映射和流式 `dma` 映射。
	- 可以在多个传输中使用前者，自动解决缓存一致性问题。但是一致性 `dma` 映射资源很宝贵。
	- 流映射有很多约束，并且不能自动解决一致性问题。
	- 一致性映射通常存在于驱动程序的生命周期中，而**流式映射通常在 `dma` 传输完成后取消映射**
- 我们应该尽可能地使用流式映射，在必要情况下再使用一致性 `dma` 映射 


### dma_map_single
- 如何处理 cache 一致性？
	- 如果是 `DMA_FROM_DEVICE` 表示是设备到 CPU 方向，此时会 `invalid`
		- 从设备读到内存，内存里面的都是新的，cache 里面的是老的，所以要 invalid
	- 如果是 `DMA_TO_DEVICE` 表示是 CPU 到设备方向，此时会 `clean` 
		- 从内存到设备，cache 里面的是新的，内存里面的是老的，所以要把 clean
	- `outer_inv_range/outer_clean_range` 都会在 `arch/arm/mm` 对应的程序中去操作 cache 的寄存器 
	![[Pasted image 20241226103536.png]]
	![[Pasted image 20241226103607.png]]
- 如何映射出一片连续的物理内存？
	- page_to_phys 中已经拿到了 paddr

### dma_map_page
- 如果要映射的大于单个 page 还会继续把所有区域都映射出来吗？
	- 会把所有区域都映射 




## 设备树支持 CONFIG_DMA_OF
- 默认 `.config` 中会打开
	```dts
	config DMA_OF
 	def_bool y
 	depends on OF
 	select DMA_ENGINE
	```

### of_dma_controller_register 
- `kzalloc` 一个 `struct of_dma *ofdma`
- 然后给 `ofdma` 赋值，都是通过参数传过来的，在 `axi-dma` 驱动里面会配好
- 将这个 `ofdma` 链到全局的一个链表上 `of_dma_list`

### dw_axi_dma_of_xlate
- 什么时候调用？
- 从设备树中取出 `args` 当做硬件握手信号，后面驱动就用的这个当做握手信号 


### dma_request_chan
- 如果 `dev->of_node` 存在，那么就调用 `of_dma_request_slave_channel` 
	- 是否有 `dmas` 字符串，没有就 `return error`
	- 获取 `dma-names` 的的个数
	- 遍历这几个 `count`
		- 根据 `name、np(dts_node)` 拿到 `dma_spec`
		- 根据 `dma_spec` 找到 `ofdma`
		- 根据 `ofdma` 来调用 `ofdma->of_dma_xlate(&dma_spec, ofdma)` 返回 `chan` 

### 其它设备如何描述握手信号
- 不要将 `dma-cells` 配的太多
	- 其它外设只需要知道握手信号就行了，`tx/rx` 的握手信号，不需要考虑使用哪个 `master` 
- `dma-requests` ：握手信号数量 
- 将 `dma-cells` 配成 `1` 就可以了，用于描述握手信号 
- 在 `request_channel` 的时候会解析握手信号


## tx 支持  dma

### CTRLR1 寄存器在写的时候需要配置吗？
- [[dw_spi ip#CTRLR1 寄存器]]
	- 如果开了时钟延展，需要配置这个寄存器以表示要传输的数据总量 
	- 如果没开时钟延展没关系 



# Tx 流程

```c
//dma_cap_mask_t mask; // mask 现在用 dma_request_chan 之后用不上了？
					 // 在 dma_request_chan 拿到 channel 之后会做这几步
					 // dma_request_channel 会用到
struct dma_chan *dma_channel; // 申请到的 dma channel
struct dma_slave_config txconf; // 用于配置 slave(源？目标？)
							// 源和目标都配置，但是只配设备的物理地址，不配内存的物理地址
struct dma_async_tx_descriptor *txdesc; // 干了什么之后会拿到？有啥用？
										// prep_sg 后返回，用于配置回调以及回调参数
										// tx_submit 的时候也是用这个
dma_cookie_t cookie;  // submit 之后得到一个 cookie，后续根据这个 cookie 来判断 dma 传输是否完成
struct scatterlist tx_sg; // 由于要用到 prep_sg，所以需要把 addr、len 弄成 tx_sg 格式

bool vmalloced_buf; // 判断是否是 vmalloc 的 buffer
struct page *vm_page; // buffer 对应的 page
bool kmap_buf = false; // 判断 buffer 是否是 kmap

dma_addr_t dma_handle;  // 虚拟地址映射出来的物理地址
size_t page_left, page_ofs; // map page 的时候要用到


//dma_cap_zero(mask);
//dma_cap_set(DMA_SLAVE, mask);

dma_channel = dma_request_chan(&dws->master->dev, "tx");
if (!dma_channel) {
	pr_err("dma_request_chan failed!\n");
	return -EINVAL;
}

memset(&txconf, 0, sizeof(txconf));
txconf.direction = DMA_MEM_TO_DEV;
//  为什么不用配 src_addr ?
// 因为对于 tx，从内存到外设，这里的目标是外设的 fifo
// 源地址会通过 prep_sg 的时候传进去
// axi-dma 的驱动也是这么解析的，对于MEM_TO_DEV，设备地址来自config，mem_addr 来自 prep_sg 的参数
// 也就是说拿的全是外设的物理地址，内存的物理地址通过参数传过去的
txconf.dst_addr  = (dma_addr_t)(dws->paddr + SPIM_RXDR);
txconf.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
txconf.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
txconf.device_fc = false;
// slave_id 不用配置，新版本里面已经去掉了这个成员
ret = dma_engine_slave_config(dma_channel, &txconf);
if (ret) {
	return ret;
}

vmalloc_buf = is_vmalloc_addr(dws->buffer);
#ifdef CONFIG_HIFHMEM
kmap_buf = ....;

if (dma_len != 0) {
	if (vmalloced_buf)
		vm_page = vmalloc_to_page(dws->buffer);
	else if (kmap_buf)
		vm_page = kmap_to_page(dws->buffer);

	if (vmalloced_buf || kmap_buf) {
		if (!vm_page)
			return -ENOMEM;
		page_ofs = offset_in_page(dws->buffer);
		page_left = PAGE_SIZE - page_ofs;
		dma_hanler = dma_map_page(&dws->master->dev, vm_page, \
									page_ofs, dma_len, DMA_TO_DEVICE);
	} else {
		dma_handler = dma_map_single(&dws->master->dev, dws->buffer, \
										dma_len, DMA_TO_DEVICE);
	}

	if (dma_mapping_error(&dws->master->dev, dma_handle))
		return -ENOMEM;


	// sg_init_one() 不能替换下面的三行
	// sg_init_one 会去将虚拟地址映射成物理地址，但是只是简单的映射
	// 并不保证映射后的物理地址是连续的
	// 不会处理 cache 一致性
	// 不会进行 dma 相关的内存对齐
	// 等等
	sg_init_table(&tx_sg, 1);
	sg_dma_address(&tx_sg) = dma_handle;
	sg_dma_len(&tx_sg) = dma_len;
	
	txdesc = dmaengine_prp_slave_sg();
	if (!txdesc)
		return -EINVAL;

	txdesc->callback = dw_spi_dma_tx_done;
	txdesc->callback_param = dws;

	cookie = dmaengine_submit(txdesc);
	if (dma_submit_err(cookie)) {
		return -EBUSY;	
	}

	reinit_completion(&dws->irq_completion);

	...... spi 相关操作
	
	dma_async_issue_pending(dma_channel);
	wait_for_completion(&dws->irq_completion);

	if (vmalloced_buf || kmap_buf)
		dma_unmap_page();
	else
		dma_unmap_single();

}

dma_release_channel(dma_channel);

```

![[Pasted image 20241225164748.png]]
# Rx 流程 
```c
struct dma_chan *dma_channel;
struct dma_slave_config rxconf;
struct dma_async_tx_descriptor *rxdesc;
dma_cookie_t cookie;
struct scatterlist rx_sg;
int ret;

dma_channel = dma_request_chan(&dws->master->dev, "rx");
if (!dma_channel)
	return -EINVAL;
memset(&rxconf, 0, sizeof(rxconf));
rxconf.direction = DMA_DEV_TO_MEM;
rxconf.src_addr = (dma_addr_t)(dws->paddr + SPIM_RXDR);
rxconf.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
rxconf.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
rxconf.device_fc = false;
ret = dmaengine_slave_config(dma_channel, &rxconf);


vmalloced_buf = is_vmalloc_addr(dws->buffer);

sg_init_tbale();
sg_dma_address();
sg_dma_len();

rxdesc = dmaengine_prep_slave_sg();

rxdesc->callback = dw_spi_dma_rx_done;
rxdesc->callback_param = dws;

cookie = dmaengine_submit(rxdesc);

reinit_completion();

dma_async_issue_pending(dma_channel);


wait_for_completion();

dma_unmap_single();

dma_release_channel();
```



# Gen SPI 作为通用 SPI 

## spidev. c
- 这是类似 flash 驱动的一个 `spi device` 通用驱动框架 
- 需要在 `defconfig` 中打开 `CONFIG_SPI_SPIDEV=y`
- 需要在 `dtsi` 中增加 `spidev`
```dts
spidev@1 {
	compatible = "rohm,dh2228fv";
	spi-max-frequency = <1000000>;
	reg               = <1>;
	status            = "okay";
};
```
- 起来之后可以在 `/dev/spidev0.1` 看到这个设备 

## spidev_fdx . c
- 这是类似 flash_test. c 的一个 `spi device` 通用测试程序 
- 操作 `/dev/spidev0.1` 设备
- 直接编译 `arm-none-linux-gnueabihf-gcc spidev_test.c -o spidev_test` 后无法执行：
```shell
[root@gx6602 gx]# ./spidev_test 
-/bin/sh: ./spidev_test: not found
[root@gx6602 gx]# ./spidev_test -h
./spidev_test: line 1: syntax error: unexpected "("
[root@gx6602 gx]# ./spidev_test 
./spidev_test: line 1: syntax error: unexpected "("
```

- 好像是需要用到某个动态库，现在换一种编译方式：
```shell
arm-none-linux-gnueabihf-gcc -static \
-I../include \
-I../arch/arm/include \
-I../arch/arm/include/generated \
spidev_test.c -o spidev_test

# 直接执行之后好了
[root@gx6602 home]# ./spidev_fdx -h
usage: ./spidev_fdx [-h] [-m N] [-r N] /dev/spidevB.D
```
![[Pasted image 20241227103558.png]]









# TODO
> [!info]  
> -  CONFIG_DMA_OF 没使能，后续所有的握手信号都要来自设备树 
> 	-  已解决 
>  - 要关注下 gen spi 多个 cs 分别接 spi 设备的情况


