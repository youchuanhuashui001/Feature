

## Common

### 擦除 block 后可以不按顺序写入某个 page 吗？
>  需求：ota 升级时想先升级 stage2，再升级 stage1，这样在升级的时候掉电了也可以起来；

- 对于 `Nor Flash` 可以在擦除单个 block 后，先写这个 block 后面的 page，再写前面的 page
- 对于 `Nand Flash` 也可以擦除单个 block 后，先写这个 block 后面的 page，再写前面的 page
	- 对于并行 Nand Flash，**一个 block 内不支持随机页写, Block内的页需要按照顺序写，参考手册”8.13 Addressing for program operation“， gd9fx2gxf2a_v1.0_20190801.pdf**
	- 对于串行 Nand Flash，暂时没有在手册上看到有描述这种特性的
- **不能页内随机去操作**，意思是必须以 page 为单位
- **单个 page只能写一次**
	- 有些 nor flash 单个 page 内先写后面再写前面会出错 
	- nand flash 必须按 page 操作，因为会算 ecc


## Nor Flash

###  操作 flash 的过程中被其它中断打断，会影响 flash 写操作吗？
- flash 写操作的流程是：发指令、发地址、写数据
- 如果被打断后影响了 spi 的波形，那么是不行的
- 如果其他中断不会操作到 spi 的波形，是可以的
	- 也就是说 flash 只会根据波形来进行操作，如果期间没有 clk 了，但是 cs 一直拉低选中的情况下也是可以正常操作的，后续继续发波形，没关系
	- 例如：开了时钟延展功能的时候，可以看到 cs 拉低之后，clk 脚可以很长一段时间没有波形 `(这里类似被其它中断打断的过程)` 


- 对于中断的时间没有要求，可以很久
- 写操作可能会写错地址
- 读操作可能会读错位置
- 擦除操作可能会擦不完整 

### 什么是 spi 的四倍速？
- 首先从基础 SPI 协议说起，标准的 SPI 是全双工通信，使用 4 根线：
	- SCK：时钟信号
	- MOSI：主设备输出，从设备输入
	- MISO：主设备输入，从设备输出
	- CS：片选信号
- 四倍速 SPI 是在标准 SPI 的基础上，将 MOSI、MISO 切换为 IO0、IO1， 另外增加两根线	IO2、IO3：	
	- MOSI --> IO0 : 数据线 0
	- MISO --> IO1 : 数据线 1
	- IO2：数据线 2
	- IO3：数据线 3
- 从物理层面看，新增加的两个 GPIO 不是原本标准 SPI 协议中规定的一部分，而是一些特殊的 QSPI 控制器才能用于管理额外的 2 个 IO
- 目前通常将 QSPI 控制器集成在主控芯片内部，用于连接 flash 以支持四倍速
- 另外 `八倍速` 也是类似的，只是增加了 6 根线(IO2、IO3、IO4、IO5、IO6、IO7)


### 什么是 XIP ？预取？连续？

- XIP 是 `Execute In Place` 的缩写，表示在闪存中执行代码。允许 CPU 直接从 flash 中读取代码并执行，而无需将代码加载到 RAM 中。Flash 被映射为内存地址空间的一部分。
- 工作原理：
  ```
  传统方式：
  flash --> RAM --> CPU 执行

  XIP 方式：
  flash --> CPU 执行
  ```

- 预取的本质：
	- 预测并提前读取可能需要的指令/数据
	- 将数据放入预取缓冲区 (prefetch buffer)
	- 减少 cpu 等待时间
- 工作原理：
  ```
  无预取时：
  CPU 请求 --> Flash 读取 --> CPU 等待 --> 获得数据

  有预取时：
  CPU 执行当前执行 --> 同时预取下一段代码 --> CPU 直接从预取缓冲区读取数据
  ```
- 预取的数据大小由控制器来决定，可以是一个指令，也可以是多个指令

- 连续：
```
标准XIP读取：
每次读取都需要发送完整的读取命令序列
CMD -> ADDR -> DUMMY -> DATA

连续模式：
首次：CMD -> ADDR -> DUMMY -> DATA
后续：直接读取DATA（无需重发命令和地址）
```
- 工作原理：
	- 首次访问：
    	- 发送完整的读取命令
    	- 设置起始地址
    	- 进入连续读取模式

	- 后续访问：
		- flash 自动递增地址
		- 直接输出数据
		- 无需重新发命令



### block erase 擦除包含写保护区域时，不会做擦除操作
咨询一下，目前测试 P25Q80SH/P25Q40SH 写保护功能时遇到一个问题，希望帮忙分析下：
1. flash 配置写保护区域为从 0 地址开始的 16KB/32KB
2. 用 64KB 擦除的指令 (0xD8) 擦除整片 flash，发现第一个 block 中非保护的区域依然有数据，未被擦掉；
3. 用 4KB 擦除的指令 (0x20) 不会有上面的问题

Flash 厂商答复：
block erase 擦除的地址范围包含写保护的地址就不会执行了
![[Pasted image 20250421134828.png]]




### 开 XIP 时想要动态调频 SPI，有何注意事项
- XIP 是硬件操作，不清楚何时访问，这个要注意
- 调频涉及到 Sample_delay 的同步修改，在配置 Sample_delay 时需要先关控制器，配值，再开控制器



## SPINand Flash

### ROM、Stage1 设计
- rom 读 stage1 时不需要考虑坏块，stage1 读 stage2 的时候需要考虑坏块
	- stage1 的地址是固定在0地址的，rom 只会从这个地址读数据；而 flash 在出厂的时候会确保 block0 是好块，所以不需要考虑坏块。
	- stage2 的地址不是固定的，可能并不在第一个 block，可能在其它的 block，而从0地址到其它的 block 不一定是好块，所以需要考虑坏块。
- 多备份问题：
	- spinand 会产生跳变，即使 block0 是好块，如果纠错能力是4 bit，那么可能会出现跳变5bit、6bit的情况
	- 这样的话 rom 读 stage1 可能会出现起不来的情况，stage1 读 stage2 也可能会出现起不来的情况，甚至后面 stage2 读 kernel 也会出现起不来的情况
	- 因此考虑 stage1 多备份其实是没有什么意义的，因为 rom 读 stage1 读对了，但是 stage1 读 stage2 的时候有跳变，那么也起不来，要备份就 stage1、stage2 都备份
	- 遇到启动不了的情况，也就是说跳变的 bit 太多了，那么就重新烧一遍
- 对于跳坏块的功能，是另一件事情，指的是在擦写的过程中会出现失败的情况，这种时候就标记为坏块。下一次读取的时候就会根据坏块标记跳到下一个 block 读。



## Nand Flash

### 什么是 Nand Flash 的 Cache read?

Nand Flash 的读取方式主要有两种:

1. 普通读取 (Normal Read)
- 每次读取都需要先将数据从存储阵列读到页寄存器
- 然后再从页寄存器读出数据
- 读取过程需要等待 tR (读取时间)
- 适合随机读取

2. Cache Read 模式
- 利用了双缓冲页寄存器
	- 数据输出缓存：用于存放当前页数据输出
	- 感应放大器缓存：用于下一页数据预读取
- 当读取第一页时, 数据会同时存入两个页寄存器
- 读取下一页时, 一个寄存器输出数据, 另一个寄存器同时预读下一页
- 大大减少了等待时间, 提高了连续读取速度
- 适合连续读取大量数据

Cache Read 使用方法:
```
第1页读取：
1. 发送00h (读取命令)
2. 发送地址
3. 发送31h (Cache read命令)
4. 等待R/B就绪
5. 读取第1页数据
   同时，内部开始预读第2页到感应放大器缓存

第2页及后续页读取：
1. 发送31h命令
2. 等待R/B就绪
3. 读取当前页数据
   同时，预读下一页
```

```c
// 示例代码
void NAND_CacheRead(uint32_t startPage, uint32_t numPages) {
    // 第一页读取
    NAND_SendCommand(0x00);    // 读取命令
    NAND_SendAddress(startPage); // 发送地址
    NAND_SendCommand(0x31);    // Cache read命令
    NAND_WaitReady();          // 等待就绪
    NAND_ReadData();           // 读取数据

    // 后续页读取
    for(uint32_t i = 1; i < numPages; i++) {
        NAND_SendCommand(0x31); // 只需发送Cache read命令
        NAND_WaitReady();
        NAND_ReadData();
    }

    // 最后一页需要结束Cache read模式
    NAND_SendCommand(0x3F);    // 结束Cache read
}
```


### nandcomtest 测试时有1bit或2bit 出错
- 使用 nandcomtest 是发现在raw、auto_oob模式时会出现1bit或2bit错误
- nfcv1
	- auto_oob 模式写的时候先写 oob 区域，然后再写 main 区域，相当于 oob 区域的数据没有带 ecc，不会被纠正。所以 oob 区域可能出现跳变，导致数据错误。
	- raw 模式写的时候先写 oob 区域，然后关掉 ecc 再写 main 区域，相当于 oob 区域和 main 区域都没有保护，不会被纠正。所以 main、oob 区域都可能出现跳变，导致数据错误
- 修正方法：
	- 在 nandcomtest 的比较逻辑中，如果只有1bit或2bit错误，则认为测试通过
```c
	if(mode != 0) {
		p_src = p1;
		p_dst = p3;
		for(i = 0; i < oob_len; i++)
		{
			if((*p_src++) != (*p_dst++))
			{

				// 获取刚刚比较的不相同的两个字节的值
				uint8_t read_val = *(p_src - 1);
				uint8_t orig_val = *(p_dst - 1);

				// 通过异或操作找出所有不同的 bit
				uint8_t diff = read_val ^ orig_val;

				// 计算不同的 bit 的数量 (汉明距离)
				int bit_errors = 0;
				while (diff > 0) {
					diff &= (diff - 1); // Brian Kernighan's algorithm
					bit_errors++;
				}

				// 如果 bit 错误数小于等于2 (即1-bit或2-bit跳变)，则认为是可纠正的错误
				if (bit_errors <= 2) {
					// 打印一条提示信息并继续，这不被视为致命错误
					printf("INFO: Correctable OOB error at i=%d. Read=0x%02x, Orig=0x%02x, Bit Diffs=%d\n", i, read_val, orig_val, bit_errors);
					continue;
				}
				while(1);
				free(p1);
				free(p3);
				return -1;
			}
		}
	}
```
