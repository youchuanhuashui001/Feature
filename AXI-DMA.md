

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
2. cookie 的值是怎么来的？completed_cookie 会在什么时候用？
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

^28af35






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


>[!tips]
> `chan->cookie、chan->completed_cookie` 由 axi dma 的中断服务程序更新



##### cookie update
[[#^28af35]]
