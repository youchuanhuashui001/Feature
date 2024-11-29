
#doing 

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


## 目前对外设不通



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





> [!info]  1.  CONFIG_DMA_OF 没使能 