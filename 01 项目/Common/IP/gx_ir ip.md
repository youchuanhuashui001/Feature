# 红外通信



## 红外光的基本原理

红外线是波长介于微波和可见光之间的电磁波，波长在 760 纳米到 1 毫米之间，是波形比虹广场的非可见光。自然界中的一切物体，只要它的温度高于绝对零度(-273)就存在分子和原子的无规则运动，其表面就会不停地辐射红外线。但是不同的物体辐射的强度不一样。

红外发射管很常用，在我们的遥控器上都可以看到，类似发光二极管，但是它发射出来的是红外光，是我们肉眼所看不到的。发光二极管的亮度会随着电流的增大而增大，红外发射管发射红外线的强度也会随着电流的增大而增强。

红外接收管内部是一个具有红外光敏感特征的 PN 节，属于光敏二极管，但是它只对红外光有反应。无红外光时，光敏管不导通，有红外光时，光敏管导通形成光电流，并且在一定范围内电流随着红外光强度的增大而增大。



![选区_435](image/选区_435.png)

发射部分：当发射控制输出高电平时，三极管 Q1 不导通，红外发射管 L1 不会发射红外信号；当发射控制输出低电平的时候，通过三极管 Q1 导通让 L1 发出红外光。

接收部分：R4 是一个电位器，我们通过调整电位器给 LM393 的 2 脚提供一个阈值电压，这个电压值的大小可以根据实际情况来调试确定。而红外光敏二极管 L2 收到红外光的时候，会产生电流，并且随着红外光的从弱变强，电流会从小变大。当没有红外光或者说红外光很弱的时候，3 脚的电压就会接近 VCC，如果 3 脚比 2 脚的电压高的话，通过 LM393 比较器后，接收检测引脚输出一个高电平。当随着光强变大，电流变大，3 脚的电压值等于 VCC-I*R3，电压就会越来越小，当小到一定程度，比 2 脚的电压还小的时候，接收检测引脚就会变为低电平。





## 红外遥控通信原理

在实际的通信领域，发出来的信号一般有较宽的频谱，而且都是在比较低的频率段分布大量的能量，所以称之为基带信号，这种信号不适合直接在信道中传输。为便于传输，提高抗干扰能力和有效的利用带宽，通常将信号调制到适合信道和噪声特性的频率范围内进行传输，这就叫做信号调制。在通信系统的接收端要对接收到的信号进行解调，恢复出原来的基带信号。

> 1. 基带信号：指发出的没有经过调制(进行频谱搬移和变换)的原始电信号，其特点是频率较低，信号频谱从零频附近开始，具有低通形式。
> 2. 调制：调制本身是一个电信号变换的过程，是按 A 信号的特征然后去改变 B 信号的某些特征值(如振幅、频率、相位等)，导致 B 信号的这个特征值发生有规律的变化，从而得到适于信道传输的数字调制信号
> 3. 解调：在接收端将收到的数字频带信号还原成数字基带信号

平时用到的红外遥控器里的红外通信，通常是使用 38K 左右的载波进行调制的。

调制：就是用带传送信号去控制某个高频信号的幅度、相位、频率等参量变化的过程。即用一个信号去装载另一个信号。比如我们的红外遥控信号要发送的时候，先经过 38K 调制，如下图所示：

![选区_436](image/选区_436.png)

- 原始信号就是我们要发送的数据 0 或数据 1
- 38K 调制就是频率为 38K 的方波信号，调制后信号就是最终我们发射出去的波形



正常通信来讲，接收端要首先对信号进行监测、放大、滤波、解调等等一系列电路处理，然后输出基带信号。但是红外通信的一体化接收头 HS0038B，已经把这些电路全部集成到一起了，我们只需要把这个电路接上去，就可以直接输出我们需要的基带信号了。





## NEC 协议红外遥控器

NEC 协议的数据格式包括了引导码、用户码、用户码反码、按键键码、按键键码反码以及停止位。停止位主要起隔离作用，一般不进行判断。其中数据编码总共是 4 个字节。如下图所示，第一个字节是用户码，第二个字节可能也是用户码/用户反码，第三个字节是当前按键的键数据码，第四个字节是键数据码反码，可用于对数据进行纠错。

![选区_437](image/选区_437.png)

- `引导码`：9ms 的载波 + 4.5ms 的空闲
- `比特值：0` 560us 的载波 + 560us 的空闲，共 1.125ms
- `比特值：1` 560us 的载波 + 1.68ms 的空闲，共 2.25ms

最前面黑乎乎的一段，是引导码的 9ms 载波，紧接着是引导码的 4.5ms 的空闲，而后边的数据码，是众多载波和空闲交叉，它们的长短就由其要
传递的具体数据来决定。HS0038B 这个红外一体化接收头，当收到有载波的信号的时候，会输出一个低电平，空闲的时候会输出高电平，我们用逻辑分析仪抓出来一个红外按键通过HS0038B 解码后的图形来了解一下，如下图所示：

![选区_438](image/选区_438.png)

从图上可以看出，先是 9ms 载波加 4.5ms 空闲的起始码，数据码是低位在前，高位在后，数据码第一个字节是 8 组 560us 的载波加 560us 的空闲，也就是 0x00，第二个字节是 8 组 560us的载波加 1.68ms 的空闲，可以看出来是 0xFF，这两个字节就是用户码和用户码的反码。按键的键码二进制是 0x0C，反码就是 0xF3，最后跟了一个 560us 载波停止位。对于我们的遥控器来说，不同的按键，就是键码和键码反码的区分，用户码是一样的。这样我们就可以通过单片机的程序，把当前的按键的键码给解析出来。

遥控器的单按按键和连续按住按键发出来的信号是不同的。单次按键的结果与上述一致，持续按键，首先会发出一个和单次按键一样的波形出来，经过 40ms 后，会产生一个 9ms 载波 + 2.25ms 空闲，再跟一个停止位的波形，这个叫做重复码。而后，只要还按住按键，那么每隔约 108ms 就会产生一个重复码。如下所示：

![选区_439](image/选区_439.png)

![选区_440](image/选区_440.png)









# Register

## v0

- `IRR_CNTL`

  | Name     | Bits  | Description                                                  |
  | -------- | :---- | :----------------------------------------------------------- |
  | `Tov`    | 27:12 | 消息中合法脉冲的最大长度。当连续 IRR 基本单位的计数器达到该值时，FIFOTS 被置位。 |
  | `fifoil` | 11：6 | NUM 的门限值用于确认是否设置 FIFOLS                          |
  | `Pol`    | 5     | 1：电平反向；<br />0：电平不反向                             |
  | `Fifois` | 4     | 1：当 NUM >= Fifoil，Fifols 被置位<br />0：当 NUM == Fifoil，Fifols 被置位 |
  | `Enir`   | 3     | IRR 模块使能                                                 |
  | `Fifoom` | 2     | FIFO 溢出中断使能                                            |
  | `Fifotm` | 1     | FIFO 超时中断使能                                            |
  | `Fitolm` | 0     | FIFO 门限中断使能                                            |

- `IRR_INT`

  | Name     | Bits | Description                                                  |
  | -------- | ---- | ------------------------------------------------------------ |
  | `Fifoos` | 9    | 当 FIFO 中有有效数据时，FIFO.Fifos 被置位                    |
  | `NUM`    | 8：3 | FIFO 中有效数据项的数量。NUM 在读取数据寄存器时会自动减小    |
  | `Fifoos` | 2    | FIFO 溢出中断标志。当发生 fifo 溢出中断时，fifoos 被置位。写 1 清除 |
  | `Fifots` | 1    | FIFO 超时中断标志。当发生 fifo 超时中断时，fifots 被置位。写 1 清除 |
  | `fifols` | 0    | FIFO 门限中断标志。当发生 fifo 门限中断时，fifols 被置位。写 1 清除 |

- `IRR_FIFO`

  | Name   | Bits   | Description                |
  | ------ | ------ | -------------------------- |
  | `Sig`  | 31：16 | 记录单个脉冲的宽度         |
  | `Mark` | 15：0  | 记录单个脉冲中高电平的宽度 |

- `IRR_DIV`

  | Name         | Bits   | Description                      |
  | ------------ | ------ | -------------------------------- |
  | `High_Level` | 31：20 | anti 接口过程中高电平的宽度      |
  | `Low_Level`  | 19：8  | anti 接口过程中低电平的宽度      |
  | `Div`        | 7：0   | 分频。clock = sys_clk / (DIV +1) |

- 门限中断：接收的数据个数超过或等于门限值之后，会产生中断，通知读取数据。

  - 查看 `IRR_INT` 寄存器中的 bit0 位用于查看是否发生门限中断，写 1 清除门限中断
  -  `IRR_CNTL.fifoil` 用于配置是否产生 `FIFOLS` 中断，以及 `IRR_CNTL.bit4` 配置
    - 如果 FIFO 中的数据量`>= / ==` `IRR_CNTL.fifoil` 配置的值将产生门限中断

- 超时中断：一段脉冲假如持续很长时间，我在接收了多长时间之后还没有接收完，就产生一个超时中断?

  - 查看 `IRR_CNT` 寄存器中的 bit1 位用于查看是否发生超时中断，写 1 清除超时中断
  -  `IRR_CNTL.Tov` 用于配置产生 `FIFOts` 中断的时间，达到配置的值就产生 `fifots` 中断

- 溢出中断：如果数据没有及时读取，接收的数据量到达 N 时，之后接收下来的数据不会被写入 FIFO 中，同时会产生 FIFO 溢出中断。
  - 查看 `IRR_CNT` 寄存器中的 bit2 位用于查看是否发生溢出中断，写 1 清除溢出中断





## v1







# 驱动



## linux 驱动



### Probe：

```c
/*
 * 1. malloc sturct gx_irr_info
 * 2. gx_irr_chip_probe(info):从设备树获取寄存器等相关信息，并进行 ioremap
 * 3. mutex_init(&info->lock):初始化互斥锁
 * 4. init_completion(&info->comp):初始化完成信号量
 */

struct gx_irr_info *info = NULL;
info = kmalloc(sizeof(*info), GFP_KERNEL);
err = gx_irr_chip_probe(info);

if (request_irq(info->irr_irvec, irr_isr, IRQF_SHARED, gx_irr_name, info))

mutext_init(&info->lock);
/*
 * 线程同步的机制，通常用于一个线程等待另一个线程完成某项任务。
 * init_completion(&info-comp);
 * complete(&info->comp);
 */
init_completion(&info->comp);

/*
 * 定义一个定时器，jiffies 为当前的系统时钟节拍，msecs_to_jiffies 将毫秒数转换成 jiffies
 * 相当于定时 g_irr_clear_code_interval 毫秒后触发 irr_clear_code 函数
 */
init_timer(&info->timer);
info->timer.expires = jiffies + msecs_to_jiffies(g_irr_clear_code_interval);
info->timer.data = (unsigned long)info;
info->timer.function = &irr_clear_code;
add_timer(&info->timer);

/* 申请一块 kfifo，后续 key 的值都会通过 get/put 的方式放到 kfifo */
err = kfifo_alloc(&info->queue, IRR_KFIFO_SIZE, GFP_KERNEL);

init_completion(&info->raw_data_comp);

spin_lock_init(&info->raw->queue_lock);

err = kfifo_alloc(&info->raw_queue, IRR_RAW_KFIFO_SIZE, GFP_KERNEL);

input_dev = input_allocate_device();
info->input = input_dev;
set_bit(EV_KEY, input_dev->evbit);
input_dev->keybit[i] = 0xffffffff;
input_dev->name = GX_IRR_DEVNAME:
input_dev->id.bustype = BUS_PCI;
input_dev->id.version = 2;
input_dev->id.product = 3;
input_dev->id.vendor = 4;
ret = input_register_device(input_dev);

/* 
 * init_waitqueue_head(wait_queue_head_t *)
 * wait_event()/wait_event_interruptible()
 * wake_up()/wake_up_interruptible()
 */
init_waitqueue_head(&info->irr_read_queue);

ret = misc_register(&irr_miscdev);
```





### misc_ops:

```c
struct file_operations irr_fops = {
	.owner = THIS_MODULE,
	.open = irr_open,
	.read = irr_read,
	.poll = irr_poll,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
	.unlocked_ioctl = irr_ioctl,
#else
	.ioctl = irr_ioctl,
#endif
	.release = irr_release,
};
```



#### open:

```c
static int irr_open(struct inode *inode, struct file *filp)
{
	filp->private_data = g_info;

	if (g_info == NULL)
		return -1;

	return 0;
}
```



#### read:

```c
/*
 * 互斥锁上锁
 * 如果 kfifo 中没有数据：
 *	如果模式是非阻塞模式：解锁互斥锁，返回 error
 * 	如果模式是阻塞模式:等待完成信号量，释放 cpu
 * 如果 kfifo 中有数据：从 kfifo 取数据并拷贝给应用程序
 * 互斥锁解锁
 */

ssize_t irr_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	struct gx_irr_info *info = filp->private_data;
	size_t len = 0;
	struct irr_code_info_t irr_data;

	if(!info)
		return 0;

	mutex_lock(&info->lock);
retry:
	if (kfifo_len(INFO_QUEUE) == 0) {
		if((info->irr_mode & O_NONBLOCK) == O_NONBLOCK) {
			mutex_unlock(&info->lock);
			return -EAGAIN;
		}
		wait_for_completion(&info->comp);
		goto retry;
	}
	else {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
		unsigned int ret;
		ret = kfifo_out(INFO_QUEUE, (unsigned char *)&irr_data, sizeof(struct irr_code_info_t));
#else
		kfifo_get(INFO_QUEUE, (unsigned char *)&irr_data, sizeof(struct irr_code_info_t));
#endif
	//	if(data == 0) {
	//		mutex_unlock(&info->lock);
	//		return -EAGAIN;
	//	} //delete it.The data of one of the phillips remote control key 0 is 0.
		len = count > sizeof(struct irr_code_info_t) ? sizeof(struct irr_code_info_t):count;
		if (copy_to_user(buf, &irr_data, len))
			return -EIO;
	}

	mutex_unlock(&info->lock);
	return count;
}
```



#### poll：

```c
/*  
 * 互斥锁上锁
 * 如果 kfifo 为空，则等待 info->irr_read_queue，相当于将当前线程添加到等待队列中
 * 互斥锁解锁
 */

unsigned int irr_poll(struct file *filp, struct poll_table_struct *table)
{
	struct gx_irr_info *info = filp->private_data;
	unsigned int mask = 0;

	mutex_lock(&info->lock);
	if (kfifo_len(INFO_QUEUE) == 0)
		poll_wait(filp, &info->irr_read_queue, table);//add to monitor queue
	else
		mask |= POLLIN;		//dev readable
	mutex_unlock(&info->lock);
	return mask;
}
```



#### ioctl:

```c
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
static long irr_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int irr_ioctl(struct inode *inode, struct file *filp,
		unsigned int cmd, unsigned long arg)
#endif
{
	struct gx_irr_info *info = filp->private_data;
	int retval = 0;
	int tmp_value = 0;
	unsigned long flags;

	if (!info)
		return 0;

	switch(cmd) {
		case IRR_SET_MODE: /* Set: arg points to the value */
			retval = __get_user(info->irr_mode, (int __user *)arg);
			break;
		case IRR_SIMPLE_IGNORE_CONFIG:
			retval = __get_user(tmp_value, (int __user *)arg);
			if (tmp_value >= (g_irr_clear_code_interval / 100)) /* g_irr_clear_code_interval > g_irr_simple_ignore_num */
				return -EPERM;
			g_irr_simple_ignore_num = tmp_value;
			break;
		case IRR_DISABLE:
			IRR_MODULE_DIS(info->regs);
			break;
		case IRR_ENABLE:
			IRR_MODULE_EN(info->regs);
			break;
		case IRR_CLEAR_CODE_INTERVAL_CONFIG:
			retval = __get_user(tmp_value, (int __user *)arg);
			if ((tmp_value / 100) <= g_irr_simple_ignore_num) /* g_irr_clear_code_interval > g_irr_simple_ignore_num */
				return -EPERM;
			g_irr_clear_code_interval = tmp_value;
			break;
        /* 获取原始数据：info->pulse */
		case IRR_GET_RAW_DATA:
			return irr_get_raw_data(info, (struct irr_raw_data_t __user*)arg);
		case IRR_ENABLE_RECORD_RAW_DATA:
			retval = __get_user(info->record_raw_data, (int __user *)arg);
			if (info->record_raw_data == 0) {
				spin_lock_irqsave(&info->raw_queue_lock, flags);
				kfifo_reset(&info->raw_queue);
				spin_unlock_irqrestore(&info->raw_queue_lock, flags);
				complete(&info->raw_data_comp);
			}
		default:  /* redundant, as cmd was checked against MAXNR */
			return -ENOTTY;
	}

	return retval;
}
```





### isr:

```c


static irqreturn_t irr_isr(int irq, void *dev_id)
{

    
    
    往 raw_queue 中放 当前 fifo 数据（info->pulse_num）、0
    complete(&info->raw_data_comp)
        
        
    清除 timeout 中断，disable irr
    识别协议：philips、bescon、panasonic、nec
}


irr_parse_nec
{
    根据 fifo 中的值解析出 key_code
}


将 key_code 放到 kfifo
置位完成量
根据 key_code 返回在 key_map 中的 index
修改定时器，继续再定时 1000ms
wake_up(&info->irr_read_queue);


/* 目前已打开 timeout irq、irr_enable */
static irqreturn_t irr_isr(int irq, void *dev_id)
{
	struct gx_irr_info *info = dev_id;
	volatile struct irr_regs __iomem *regs = info->regs;
	unsigned int index, i;
	struct irr_code_info_t irr_data;

    /* 
     * info->pulse_num:当前的 fifo 有多少有效数据;
     * info->pluse_val:当前的 fifo 数据;
     * info->pluse_val 放到 info->raw_queue kfifo
     * 0 放到 info->raw_queue kfifo
     * 置位完成量：info->raw_data_comp
     */
	if (irr_get_fifo_data(info) < 0) {
		regs->irr_int |= 1<<1;
		IRR_MODULE_DIS(regs);
		goto exit;
	}

	/* Timeout clear. */
	regs->irr_int |= 1<<1;
	IRR_MODULE_DIS(regs);

    /* 根据 info->pulse_num 匹配标准 */
	if (irr_protocol_detect(info) < 0)
		goto exit;

    /* 
     * info->pulse_val and info->pulse_num  get key_code
     * info->irr_proto = 当前的协议
     */
	if (info->algo->functionality(info) < 0)
		goto exit;

	irr_debug("key_code:0x%x\n", info->key_code);
	irr_data.code = info->key_code;
	if (info->irr_proto != IRR_PROTO_XMP)
		irr_data.code_extended = 0;
	else
		irr_data.code_extended = info->key_code_extended;
	irr_data.is_simcode = info->is_simcode;
	irr_data.irr_proto = info->irr_proto;
	info->is_simcode = 0;
    /* kfifo 可存放数据,将 irr_data 结构放到 INFO_QUEUE */
	if (kfifo_size(INFO_QUEUE) - kfifo_len(INFO_QUEUE) >= sizeof(struct irr_code_info_t)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
		kfifo_in(INFO_QUEUE, (unsigned char *)&irr_data, sizeof(struct irr_code_info_t));
#else
		kfifo_put(INFO_QUEUE, (unsigned char *)&irr_data, sizeof(struct irr_code_info_t));
#endif
	}
    /* 非阻塞方式,置位完成量:info->comp */
	if((info->irr_mode & O_NONBLOCK) != O_NONBLOCK)
		complete(&info->comp);

    /* 根据 info->key_code 得到 keymap index，报告给 input 子系统 */
	index = irr_get_key(info);
	if (index <= KEY_MAX) {
		for (i = 0; i < keymap_data[index].map_key_num; i++)
			input_report_key(info->input, keymap_data[index].map_key[i], 1); /* PUSHDOWN */
		for (i = 0; i < keymap_data[index].map_key_num; i++)
			input_report_key(info->input, keymap_data[index].map_key[i], 0); /* PUSH  UP */
		input_sync(info->input);
	}

	//mod timer expires
    /* 修改定时器，继续定时 1000ms */
	mod_timer(&info->timer, jiffies + msecs_to_jiffies(g_irr_clear_code_interval));

	//add by huangjb:select wakeup
    /* 唤醒 waitqueue:info->irr_read_queue*/
	wake_up(&info->irr_read_queue);

exit:
	IRR_MODULE_EN(regs);
	return IRQ_HANDLED;
}

```







# 芯片测试

## Gxloader

### v0

gxloader 中打开 `.config` 中的 `ENABLE_IRR=y、ENABLE_IRQ=y`

- FIFO 门限中断：
  - 修改中断门限值 CNTL_FIFOIL 为 1，将使能超时中断换成使能 level 中断，能够触发门限中断
- 超时中断：
  - 能够检测到按键值就触发了

- FIFO 溢出中断：
  - 将使能超时中断换成使能溢出中断，注释掉 irr_get_fifo_data 接口从 fifo 读数据的操作，能够触发溢出中断

- 翻转红外输入信号：
  - 默认开启，关掉初始化中的 POL
  - 进到 fifo 的数据会是反的

### v1

gxloader 中额外在 `ENABLE_IRR=y` 的下面加上 ``ENABLE_IRR_V2=y`

- FIFO 门限中断：
  - 打开驱动中的 FIFO_TEST 宏，能够触发中断
- 超时中断：
  - 默认能读到键值就有超时中断
- FIFO 溢出中断：
  - 不打开 FIFO_TEST 宏，将使能超时中断替换成使能溢出中断，irr_get_fifo_data 接口中注释掉从 fifo 读数据操作，能够触发溢出中断
- 翻转红外输入信号：
  - 关掉 `regs->irr_cntl` 寄存器的 bit8，默认开启





## Linux

- 使用 `gxtest/stb/goxceed/cases/service/periphery/irr/cmd/` 目录下的测试 case

- 在 `gxtest/stb/goxceed` 目录下先打开 `confs/def_config` 文件中的 `USE_SHELL_CLI_SHELL=y` 选项

  - `./build config -m irr`

  - `./build arm linux virgo fpga dvbs bin`

- 生成一个整 bin，将整 bin 烧到 flash，启动之后会启动测试 shell
  - 如果没有启动测试 shell，去到 `dvb/` 目录下执行 `./out.elf`
  - 输入命令 `irr read [timeout]`
    - 如果想一次查看所有键值，可以在测试 case 中加上 while(1) 来一直读

- 能够正常读到键值

**注1：需要使用 cli shell，而不是 gxshell**

**注2：编译 gxtest 之前需要先将基础库都编译通过，建议使用 `./make.sh gxtest-bus arm virgo fpga linux dvbs bin`**





## eCos

- 与 linux 相同的测试方法，只是编译是更换成 ecos 即可

- 能够正常读到键值

## 

