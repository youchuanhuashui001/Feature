- key_scan 新模块：https://git.nationalchip.com/gerrit/#/c/86514/42
# 背景知识

## IODMA

- 概述：iodma 用于将存储在 sram 中的信号取出，并驱动到 io 上
	- **io 指的是什么？ 外设的 fifo ？**
	- **指的是引脚，将数据通过波形的方式发出去，为了模拟不同的协议**
- 模块工作时钟和  dam_lite 为同一时钟
- 通过**内嵌 dma_lite 模块**，将存储在 sram 中的信号取出。因此此模块具备 dam_lite 的全部功能
- 模块发送时可编程：
	- 发送数据的有效位宽，2bit、4bit、8bit、16bit，每次都是从 sram 取 32bit。如果是 2bit 位宽，就会取 16 次，如果是 4bit，就会取 8 次
	- **这里配置的有效位宽就是外部用几个引脚来输出，最大支持 16 个引脚。可以配置几个引脚输出，比如 2bit 就是用 2 个引脚来输出波形。**
	- 数据发到 IO 上之后，下次发数据需要等多长时间可以配 
- 模块发送时有两种模式：
	- 固定长度：寄存器配置从 sram 中取数据的次数，取完了之后，停止发。数据发完后保持 IO 状态。
		- **从 sram 中取数据的操作是由 iodma 内部的 dma lite 来完成的，dma lite 完成之后数据就已经到 io 上了** 
	- 无尽模式：模块使能后，fifo 不满的时候就会向 dma_lite 发请求。请求不到数据时，保持 IO 状态
- fifo 深度 4，宽度 32
- 中断源：tx_done、tx_fifo_empty、tx_fifo_full、dma_lite_int (具体参见 dma_lite 模块)。每个中断源都有独立的使能控制 

### 功能特性：
#### 数据请求方式：
- 固定长度模式：设置一个传输长度，DMA 每次从 SRAM 中读取固定大小的数据块，填充到 FIFO 中。FIFO 满时停止请求，直到 FIFO 清空后继续请求数据。
	- dma 怎么知道 fifo 满没满? 
		- **这里的 fifo 满指的是 iodma 自己满了，可以使能 dma 满的中断状态 **
	- fifo 的深度作为特性描述了，**深度是 4，宽度是 32**
- 无尽模式：不设定传输长度，DMA 会持续请求数据，只要 FIFO 满了就暂停，FIFO 清空后继续请求，这种模式多用于连续数据流 (例如音频或视频)。
	- fifo 清空后才继续请求吗？不是有空闲就可以吗 
		- **iodma 的 fifo 就 4 个字节 **

#### 输出数据位宽选择
- 支持 4 种位宽选择：2bits、4bits、8bits、16bits。每次从 sram 中取 32bits 数据放到 tx_fifo 中。
- 2bits：在该位宽下，32bits 数据从低到高每 2bit 一组，总共 16 组依次输出。在数据发送完成后，如果 tx_fifo 不为空，就可以向 tx_fifo 申请下一笔要发送的数据。
- 4bits：在该位宽下，32bit 数据从低到高每 4bit 一组，总共 8 组依次输出
	- 按 bit 来？传输的目标是哪里啊？发送到 io 上？发送的 io 是固定的 io 吗？
		- 是的，按 bit 来，传输的目标是 io，发送的 io 是有 iomatrix 和 pinmux 配置的，共有 16 个引脚可以配置 



### 寄存器
- IODMA_VERSION
	- 版本号：`V100 0x00010000`
- IODMA_EN
	- 模块发送使能/禁止
- IODMA_CTRL
	- `[2:1]：00 时输出数据 bit[1:0]有效，01 时输出数据 bit[3:0]有效，10 时输出数据 bit[7:0]有效，11 时输出数据 bit[15:0]有效 ` `
	- bit0: 0 固定长度；1 无尽模式 
- IODMA_IER
	- bit2 tx_fifo_full_en：txfifo 满中断使能，0 关中断，1 开中断
	- bit1 tx_fifo_empty_en：txfifo 空中断使能，0 关中断，1 开中断
	- bit0 tx_done_en：固定模式下，数据发送完成中断使能信号，0 关中断，1 开中断
- IODMA_ISR
	- bit2 tx_fifo_full：txfifo 满，硬件自己清除
	- bit1 tx_fifo_empty：txfifo 空，硬件自己清除
	- bit0 tx_done：固定长度模式下，数据发送完成。写 1 清零
- IODMA_TX_NUM
	- `[23:0]：固定长度模式下，发送数据时，从 sram 请求数据的次数，单位：word`
- IODMA_TX_DELAY
	- `19:0：在数据发送到 IO 上之后，下次发送数据需要等待的时间`


- 没有 sram 的地址，io 的地址，都是固定的位置吗？
	- **sram 的地址是通过 dma_lite 指定的，io 是有 iomatrix 和 pinmux 选择使用哪几个 io 的**



## DMA_LITE

- dma_lite 模块用于将一次读写请求转换成标准的 ahb 总线访问
- 可以用参数配置有效数据位宽，8bit、16bit、32bit
- 总共设置 4 组地址长度寄存器对。可以配置要使用的地址长度寄存器，跑到最后一组之后会重新到第一组
	- **所以这里是不是每次用完一组都要清空掉?**
		- **应该不用吧，看看配置地址长度对个数**
- 每发起一次请求，进行一次 ahb 访问并返回 ack
- 内部设计计时器，每发起一次请求时计数器清零，重新开始计时。如果达到设定的阈值之后产生超时中断。用于检测一定时间内是否有新的请求发起
- 中断源
- 非法访问：
	- **不能访问超过 `16MB` 的地址**
	- **访问地址需要根据 addr[1:0] 对齐**
- 支持方向：
	- 外设到内存：外设接收数据并将收到的数据发送到 sram
	- 内存到外设：外设从 sram 读取数据并将读到的数据发送出去 
- 读写请求：
	- 可配置位宽 32bits、16bit、8bit，对应的 ahb 总线访问是 word、half、byte
	- 写：
		- data_width = 32，接口数据 32bit 有效，ahb 总线每次发送 32bit。一次 ahb 传输结束后返回 ack，并且当前地址+32 作为下次 ahb 总线要访问的地址
		- data_width = 16，接口数据低 16bit 有效，ahb 总线每次发 16bit。一次 ahb 传输结束后返回 ack，并且**当前地址+32** 作为下次 ahb 总线要访问的地址 
		- 16 的时候也加 32？
			- **一直用 32 位宽来操作，因为 iodma 是 32bit 位宽**
	- 读：
		- data_width = 32，接口数据 32bit 有效，ahb 总线每次接收 32bit。一次 ahb 传输结束后返回 ack，并且当前地址 +32 作为下次 ahb 总线要访问的地址
		- data_width  = 16，接口数据低 16bit 有效，ahb 总线每次接收 16bit。一次 ahb 传输结束后返回 ack，并且**当前地址 + 32** 作为一下次 ahb 总线要访问的地址 
		- 16 的时候也加 32？




### 功能特性






### 寄存器 + 0x1000
- DMA_LITE_VERSION
	- `0x00010000 V100`
- DMA_LITE_EN
	- 1 使能；1 禁止
- DMA_LITE_CTRL
	- bit1：timeout counter 清零。**写 1 清零？**
	- bit0：0 读请求，从 sram 读数据到模块；1 写请求，将模块接收到的数据写到 sram
- DMA_LITE_IER
	- bit5：0 关闭 ahb 响应错误中断；1 开启 ahb 响应错误中断
	- bit4：0 关 timeout 中断；1 开 timeout 中断
	- bit3：第四组地址长度对的**传输完成中断使能**信号。0 关中断，1 开中断
	- bit2：第三组地址长度对的**传输完成中断使能**信号。0 关中断，1 开中断  
	- bit1：第二组地址长度对的**传输完成中断使能**信号。0 关中断，1 开中断  
	- bit0：第一组地址长度对的**传输完成中断使能**信号。0 关中断，1 开中断  
- DMA_LITE_ISR
	- bit5：写 1 清零,  ahb 相应中断状态。
	- bit4：写 1 清零，timeout 中断状态 
	- bit4：写 1 清零 , 第四组地址长度对的传输完成中断状态 
	- bit3：写 1 清零 , 第四组地址长度对的传输完成中断状态 
	- bit2：写 1 清零 , 第四组地址长度对的传输完成中断状态 
	- bit1：写 1 清零 , 第四组地址长度对的传输完成中断状态 
	- bit0：写 1 清零 , 第四组地址长度对的传输完成中断状态 
- DMA_LITE_IDX_AND_CNT
	- `[31:16]：req_cnt 当前地址和长度请求总数值`
	- `[1:0]：当前地址和长度索引，0 第一组，1 第二组，2 第三组，3 第四组`
- DMA_LITE_ADDR0
	- `[31:24] addr_base0：第一组地址高 8 位，基地址`
	- `[23:0] addr_offset0：第一组地址低 24 位，偏移地址` 
- DMA_LITE_LEN0
	- `[15:0] length0：第一组请求长度` 
- DMA_LITE_ADDR1
	- `[31:24] addr_base0：第二组地址高 8 位，基地址`
	- `[23:0] addr_offset0：第二组地址低 24 位，偏移地址` 
- DMA_LITE_LEN1
	- `[15:0] length0：第二组请求长度` 
- DMA_LITE_ADDR2
	- `[31:24] addr_base0：第三组地址高 8 位，基地址`
	- `[23:0] addr_offset0：第三组地址低 24 位，偏移地址` 
- DMA_LITE_LEN2
	- `[15:0] length0：第三组请求长度` 
- DMA_LITE_ADDR3
	- `[31:24] addr_base0：第四组地址高 8 位，基地址`
	- `[23:0] addr_offset0：第四组地址低 24 位，偏移地址` 
- DMA_LITE_LEN3
	- `[15:0] length0：第四组请求长度` 
- DMA_LITE_ADDR_LEN_USED_NUM
	- `[1:0] 使用的地址长度对个数。0 使用第一组；1 使用前两组；2 使用前三组；3 四组全用`
- DMA_LITE_TO_VALUE
	- `[15:0] to_value` 两次请求之间的超时值 


- 搞这么四组，有啥用？
	- 我不是用两组就可以了吗 
	- **不用就不配**

- 配置了地址和长度、读或者写，这样可以实现对 sram 到外设，外设到 sram，sram 到 sram


### Q&&A

#### Q1：sram 中的数据假如是 0xaa，这个字节驱动到 io 上使用几个 io 由谁来决定？用 1 个 io？2 个 io？3 个 io？
- ~~io 指的应该是外设的 fifo~~
- **使用几个 io 由 iodma 的寄存器来配置，使用哪几个 io 由 iomatrix 和 pinmux 映射**


#### Q2：如果这些 IO 复用成了其它的功能，有没有关系？
- **有关系，这些 io 必须复用成 iodma 功能**

#### Q3：放到 io 上是指产生波形吗？
#### W3：是的，用来产生波形的，就是想做一个自定义的波形产生模块 

- ~~放到 io 上指的应该是放到 fifo 中，由外设来产生波形，因为没法自己去产生对应协议的波形~~ 

#### Q4：那这样看起来和 dma 没什么区别，只是从配 dma 换成了配 iodma，并且 iodma 是单向的 
- **是的，一个小的 dma lite**

#### Q5：iodma 有通道的概念吗？只有一个通道？
- **没有通道的概念，只有一个通道**

#### Q6：iodma 配合 dmalite 使用？有很多重复的参数，比如 iodma 写好了长度，但是 dma_lite 中也有长度的信息，在传输完成之后，iodma 有传输完成中断，dma_lite 也有传输完成中断 
- iodma 中断号是 47，没有 dma_lite 的中断号 
- dma_lite 是用于将数据从 sram 搬到 iodma 的 fifo？但是 iodam 的 fifo 寄存器在哪呢？
	- 假设默认只往 iodma 的 fifo 搬，那么搬过去了之后，就按照 iodma 的配置来往 io 上发数据 
	- **dma lite 搬完之后数据就已经到了 io 上，产生了波形，iodma 用来配置 dma lite 总共搬多少数据**
- 两个共用一个中断号吗？使用 iodma 的中断号，dma_lite 的中断可以在 iodma 的中断状态寄存器中查到? ，并且开不开也是在 iodma 里面配的 ?
	- 中断源 dma_lite_int 是属于 iodma 的中断源的，文档这里怎么写的 
	- **dma lite 是包在 iodma 里面的，dma lite 的中断要看 dma lite 自己的中断状态寄存器，使能也是 dma lite 自己使能**

#### Q7：模块架构中描述还有握手信号相关的信息
- 模块自己知道吗？寄存器也没有看到可以配的地方？
- **没有握手信号一说，不用握手，因为 dma lite 如果别的模块要用，别的模块里面还会塞一个 dma lite**

#### Q8：dma_lite 模块要当做一个单独的小模块吗？还是只为 iodma 用？
- dma_lite 可以外设到内存，内存到外设，iodma 只是单方向，内存到外设 
- dma lite 是一个通用的模块，可以给其它外设用 
	- 给其它外设用的时候地址就是目标外设的 fifo? 
	- **给其他外设用的时候，会在其它外设里面塞一个 dma lite**


#### Q9：IODMA 和 dmalite 如何交互？

- 我现在要完成一个从 sram 取一个字节放到 io 脚上的功能

![[Pasted image 20250102194051.png]]





- IODMA_CTRL 配置，假如是固定模式
- IODMA_TX_NUM 配置要传多少数据
	- 配置 DMA_LITE_ADDR_LEN_USED_NUM 一组地址长度
	- 配地址是 sram 的地址，和长度
	- 使能中断
	- 配置读还是写
	- 使能模块
- IODMA_IER 使能中断
- IODMA_EN 使能模块



#### Q10：其它外设怎么和 dma lite 交互？
- 和 dma lite 交互实际是和 iodma 交互，数据只能搬到 io 上？
- **其它外设要和 dma lite 交互，会在外设里面放一个 dma lite**



#### Q11：iomatrix 和 pinmux
- iomatrix 是 pinmux 的一个子功能
- 首先在 pinmux 中配置，所有引脚的功能，如果 pinmux 中配不了，就选择功能是 `from_io_matrix`，然后在 `iomatrix` 中来配置某些引脚配成某个功能 (iomatrix 中所有功能可映射到任一 Pad)



### 核心功能
IODMA 是一个波形产生模块，用于将存储在 sram 中的信号取出，并驱动到 IO 上，模拟不同的协议。
- 通过 dma lite 从 sram 读取数据，按照数据位宽转换成对应的波形信号，通过 io 引脚输出波形


### 数据流向和处理过程
- 数据源：存放在 sram
- 数据处理过程：dma lite 以 32bit 为单位从 sram 中读数据搬运到 io 上
- io 输出：iodma 总共有 16 个引脚，可以通过寄存器配置选择 2/8/16 个引脚来输出，通过 iomatrix 和 pinmux 来配置对应的引脚为 iodma 功能

### 其它
- dma lite 是 iodma 内部的一个通用模块，如果其它外设也想使用 dma lite 的话，会在其它模块里面也放一个 dma lite





# 驱动代码

- 配置 0xb0910000 + 0x8、0xc 也就是 P0_2、P0_3 配置成 from_io_matrix[2]、from_io_matrix[3]
	- 首先在 pinmux 中配置要用的引脚是 from_io_matrix
	- 然后配置 `0xb0900000 + xxx` 对应的引脚功能配成 iodma 的功能 
	- 直接在 pinmux. c 中配成 pinmux 或者是 iomatrix 的功能，pinmux 内部应该会自己处理 
```c
hal_status gx_hal_padmux_set(GX_HAL_PADMUX *dev, int pad_id, int function)
  {
  	if (!dev || pad_id < 0 || function < 0 || pad_id > PIN_NUM_MAX)
  		return HAL_ERROR;
  
  	if (function < IOMATRIX_FUNC_OFFSET) {
  		WRITE_REG(get_pinmux_baseaddr(dev, pad_id) + get_pinmux_real_id(dev, pad_id) * 4, function);
  	} else {
  		WRITE_REG(get_pinmux_baseaddr(dev, pad_id) + get_pinmux_real_id(dev, pad_id) * 4, IOMATRIX_FUNC);
  		WRITE_REG((uint32_t)dev->iomatrix_regs + (pad_id * 4), function - IOMATRIX_FUNC_OFFSET);
  	}
  
  	return HAL_OK;
  }
```
- 配置 P02、P03 为功能 38、39
- 配置 sram 数据，`uint32_t *data = (uint32_t *)0x200000;`
- disable iodma、dmalite、disable 中断
	- IODMA_EN、IODMA_IER
	- DMA_LITE_EN、DMA_LITE_IER
- 配置 iodma，用 2 个引脚，固定长度搬 16 个字节
	- IODMA_CTRL = 0; // 2 个引脚有效，固定长度模式
	- IODMA_TX_NUM = 4; // 固定长度模式下，从 sram 请求 4 次，共 16 字节
	- IODMA_TX_DELAY = 100；// 数据发送到 IO 上之后，下次等 100 ms? 
- 配置 dma lite，从 sram 搬 16 个字节到 io 上
	- DMA_LITE_CTRL = 0; // 读请求，从 sram 读数据给模块
	- DMA_LITE_ADDR0 = 0x200000; // 基地址和偏移地址？
	-  !!!!! 这里的地址怎么配？ !!!
	- **偏移地址用 24bit 表示，已经是 16MB，基地址填 0 就可以**
	- DMA_LITE_LEN0 = 0x10;
	- DMA_LITE_ADDR_LEN_USED_NUM = 0x0; // 使用第一组
	- DMA_LITE_TO_VALUE = 1000; // 两次请求之间的超时时间 
- 使能 dma lite 中断和使能 dma lite
	- DMA_LITE_IER = (1 << 5) | (1 << 4) | (1 << 0);//ahb resp err、timeout、第一组的完成中断 
	- DMA_LITE_EN = 1; // 使能 dma lite
- 使能 iodma 中断和使能 iodma
	- IODMA_IER = 0x7; // txfifo 满中断，txfifo 空中断，tx_done
	- IODMA_EN = 1; // 模块发送使能 
- 判断 iodma 中断状态 
	- `IODMA_ISR & 0x1` 





# 测试用例

## 功能测试范围

### IODMA 功能测试范围

1. 输出位宽测试
   - 2bit 模式：验证每 32bit 数据分 16 组输出
   - 4bit 模式：验证每 32bit 数据分 8 组输出
   - 8bit 模式：验证每 32bit 数据分 4 组输出
   - 16bit 模式：验证每 32bit 数据分 2 组输出

2. 传输模式测试
   - 固定长度模式：
     * 配置不同的传输长度
     * 验证传输完成后是否正确停止
     * 验证 IO 状态是否保持
   - 无尽模式：
     * 验证 FIFO 不满时持续请求数据
     * 验证 FIFO 满时暂停请求
     * 验证 FIFO 有空间时继续请求

3. 中断测试
   - tx_done 中断：固定长度模式下传输完成中断
   - tx_fifo_empty 中断：FIFO 空中断
   - tx_fifo_full 中断：FIFO 满中断

4. IO 输出测试
   - 引脚功能复用配置测试
   - 波形输出验证
   - 数据发送延时验证

### DMA_LITE 功能测试范围

1. 地址配置测试
   - 基地址（高8位）和偏移地址（低24位）组合测试
   - 地址对齐测试
   - 访问边界测试（不超过 16MB）

2. 多组地址长度对测试
   - 单组地址长度对使用
   - 多组地址长度对切换
   - 地址长度对循环使用

3. 传输控制测试
   - 读模式验证
   - 传输长度配置
   - 传输完成验证

4. 超时机制测试
   - 配置不同的超时阈值
   - 验证两次请求间隔超时检测
   - 超时中断处理

5. 中断测试
   - 传输完成中断
   - 超时中断
   - AHB 响应错误中断

### 组合测试场景

1. 基本功能组合
   - 2bit 模式 + 固定长度 + 单组地址
   - 4bit 模式 + 无尽模式 + 多组地址
   - 8bit/16bit 模式下的各种组合

2. 异常处理测试
   - FIFO 满/空处理
   - 地址越界处理
   - 中断处理响应

3. 性能测试
   - 不同位宽下的传输速率
   - 不同延时配置下的波形输出
   - FIFO 缓存效率

4. 稳定性测试
   - 长时间运行测试
   - 频繁启停测试
   - 多种配置切换测试

## 测试步骤示例

1. 引脚配置
   - 配置 0xb0910000 + 0x8、0xc 也就是 P0_2、P0_3 配置成 from_io_matrix[2]、from_io_matrix[3]
   - 首先在 pinmux 中配置要用的引脚是 from_io_matrix
   - 然后配置 0xb0900000 + xxx 对应的引脚功能配成 iodma 的功能
   - 配置 P02、P03 为功能 38、39

2. 数据准备
   - 配置 sram 数据，uint32_t *data = (uint32_t *)0x200000

3. 初始化配置
   - disable iodma、dmalite、disable 中断
     * IODMA_EN、IODMA_IER
     * DMA_LITE_EN、DMA_LITE_IER

4. IODMA 配置
   - 配置 iodma，用 2 个引脚，固定长度搬 16 个字节
     * IODMA_CTRL = 0; // 2 个引脚有效，固定长度模式
     * IODMA_TX_NUM = 4; // 固定长度模式下，从 sram 请求 4 次，共 16 字节
     * IODMA_TX_DELAY = 100; // 数据发送到 IO 上之后，下次等 100 ms

5. DMA_LITE 配置
   - 配置 dma lite，从 sram 搬 16 个字节到 io 上
     * DMA_LITE_CTRL = 0; // 读请求，从 sram 读数据给模块
     * DMA_LITE_ADDR0 = 0x200000; // 偏移地址用 24bit 表示，基地址填 0
     * DMA_LITE_LEN0 = 0x10;
     * DMA_LITE_ADDR_LEN_USED_NUM = 0x0; // 使用第一组
     * DMA_LITE_TO_VALUE = 1000; // 两次请求之间的超时时间

6. 使能配置
   - 使能 dma lite 中断和使能 dma lite
     * DMA_LITE_IER = (1 << 5) | (1 << 4) | (1 << 0); //ahb resp err、timeout、第一组的完成中断
     * DMA_LITE_EN = 1; // 使能 dma lite
   - 使能 iodma 中断和使能 iodma
     * IODMA_IER = 0x7; // txfifo 满中断，txfifo 空中断，tx_done
     * IODMA_EN = 1; // 模块发送使能

7. 结果验证
   - 判断 iodma 中断状态
     * IODMA_ISR & 0x1
   - 观察波形输出
   - 验证数据正确性 