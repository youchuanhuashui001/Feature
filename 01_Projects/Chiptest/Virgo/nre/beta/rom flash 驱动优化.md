- [ ] 优化 spinor flash 驱动，使用双倍速
	- [ ] 四字节地址的问题
	- [ ] 需要考虑四倍速的情况，stage1 bbt 会用到四倍速
	- [ ] 四线的时候要根据芯片判断启不启动
	- [ ] stage1 用的是控制器自动拉高拉低 cs，因为有一些读寄存器的操作，如果手动的去拉低拉高 cs 代码就很麻烦
- [ ] 添加 spinand flash 驱动
	- [ ] 4k page，需要确认 develop 分支的代码，目前用的是 develop2 的代码可能会有部分代码不是最新的
	- [ ] rom 需求：不需要读坏块表，要求能够兼容所有的 flash，包括 2kpage，4kpage
		- [ ] 确认是不是所有的 spi 操作都是使用 spinand_cmd 接口来完成的，因为只有这个接口的前后会处理 cs，其余的地方不会处理 cs
	- [ ] stage1 需求：需要读坏块表，要求能够兼容尽可能多的 flash，包括 2kpage，4kpage



# stage1 驱动规划
```
boot --> spinor; spinand; ppinand
```


## spinor flash 驱动

### stage1
直接用 scpu 中 apus 的 stage1 驱动，需要添加 4 字节地址即可
- [ ] 三字节地址模式、四字节地址模式
- [ ] BBT/SLT 板级时支持四倍速


## spinand 驱动

> [!note]
> - rom 考虑是不是不要备份
> - stage1 不能读坏块表，需要读坏块标记，这个代码在 vega 中已经在用了
> - stage1 需要参数页，并且参数页中 spinand 需要去掉一些参数 (之前用的是 nfc 来读的)
> - stage1 还需要考虑一个问题：block2、3 坏，读 block2 和 3 都得到 4，此时需要计算上前面的坏块数量
> - ![[Pasted image 20250326162054.png]]



rom--> 拷贝 stage1 --> stage1 拷贝 bl31、dtb、u-boot 等等
rom 不支持 spinand；rom 直接用 gdb 加载；stage1 跑起来就可以了




### rom
- 需要一份精简的 rom 驱动，目前用的是 stage1 的驱动，还会去读坏块表。
- rom 用 nfc 的代码不会读坏块表
- 之前 rom 都是用 gx_spi 的吗？否则用上 dw_spi 之后好像都是用的 nfc 来读的

### stage1
- 现有的 stage1 驱动读坏块表：会从 block2047、2046、1023、1022、2045、2044、1021、1020 这几个块读，读一个 page，看前面的 4 个字节数据是不是 `Bbt0 or 1tbB`
	- 读到了就算下 crc：从 bbt_size + 8 的偏移位置，多少个数据算一下，算出个 crc
		- 计算从 nand_bbt 开始到 bbt_size + 8 的位置的 crc
	- 如果算出来的 crc 和 `nand_bbt[crc_offs/4]` 的 crc 一致，就认为 bbt 取好了
	- 后面有 bbt_ready 的时候才会读 bbt，否则不会读
- 后面的驱动就是跳坏块读数据

	- bbt 存放方式：
```
magic   version   reserved   length(小端)        bbt[]     crc
Bbt0    0x01      0xff       bbt 字节长度(2byte)  bbt 数据  4byte
4byte   1byte     1byte
bbt 数据长度 0x100:2bit 表示一个 block 好坏；0x100 = 256 个字节 * 4 = 1024个block 坏块信息

bbt_size = (nand_bbt[1] >> 16) & 0xffff
         = (0x100ff01 >> 16) & 0xffff
         = 0x100
         
bbt_size + 8 = 就刚好是从 magic ~ bbt 数据所有信息
从开头到 bbt_size + 8 开始算 crc，crc 生成4个字节，然后和 bbt 中末尾的 4 个字节crc(nand_bbt[(bbt_size + 8)/4]) 进行比较       
= (256 + 8)/4
= 66
nand_bbt[66] = nand_bbt + 4 * 66 = crc 所在的位置
```





# Q&&A
## 写地址的时候直接用 `writel` 接口把地址一次写掉，会有问题吗？（再确认一下!!!）
- `writel` 接口一次最多可以写四个字节，当 spi 位宽配成 8bit 的时候，写四个字节的操作会被分成 4 次写入操作
- 但有一个问题：
	- 3 字节地址模式时，flash 需要收到 `0x3b + 3 byte address + dummy`：这样地址配了 3 个字节，最后一个字节会不会当成 dummy 发出去？
	- 4 字节地址模式的时候，就没有 dummy 了

### 读 magic 的时候用一个 `uint32_t` 变量来保存存在大小端问题
- 原来读 flash 的操作，是读一个字节，移位，再读一个字节，再移位
- 但是用 `uint32_t` 直接保存的时候，不会移位，所以需要手动调整大小端


### 发地址的时候，Virgo 和 Canopus/Cygnus 不一样
- Virgo 可以发 0x64
- Canopus 不能发 0x64，
- Cygnus 不能发 0x64，换成发 0x60 起来了 (develop 分支)


gdb 看 buf 数据
关 dmacr



### Stage1 中 spinand bbt 放在 ddr 中的什么位置？
- 因为 stage1 的代码可能会用到比较大的 buffer，所以只能放到 ddr
- bbt 放在：`TEMPORARY_BUF_BBT = BUG_START_2 + BUFFER_LENGTH`
```
TEMPORARY_BUF_BBT = BUF_START_2 + BUFFER_LENGTH
                  = BUF_START_1 + BUFFER_LENGTH + BUFFER_LENGTH
                  = SDRAM_START_ADDR + BUFFER_LENGTH
                  = DRAM_BASE + BUFFER_LENGTH*3
                  = DRAM_BASE + 3*(1024*1024)
```


### 如何调试 stage1？
- 使用的是 a55 的 cpu，使用 uboot 启动的，前面 32k 是 stage1 代码
- 可以直接 load 用 gdb 调试
- 也可以烧到 flash 调试，把前面 32k 合并到 flash u-boot. bin

- 先拷贝 dte_boot_code 从 flash 的 0x8000 拷贝到内存 0x2000000
![[Pasted image 20250324141023.png]] 
- 再从 flash 的 0x100000 地址拷贝 fdt 到 `0x8000000`
![[Pasted image 20250324141121.png]]


## 四字节无法启动
- 看起来是 bl31，也就是 teeloader 有问题
- stage1 驱动会将 teeloader 从 flash 的 0x120000，拷贝到 0xa0000，大小是 0x20000
- gdb 看内存数据和 flash 波形一致
![[Pasted image 20250324165310.png]]


- 加载了一下 rom 的 .elf 之后可以起来
- 如果不加载就会卡死在 345
	- 因为不加载的时候用的是 2 0 4，也就是 boot_device 是 2，3 字节地址的
	- 加载了之后用的是 3 0 4，也就是 boot_device 是 3，4 字节地址的


## 如果 stage1 的代码没跑起来，就连 gdb 打不到 rom 的断点
- stage1 起来了之后，gdb 可以打到 `bl1_main`
- 不用修改，`rom` 在 fpga 阶段的时候用的是一片 sram，可以直接 load，只要 fpga 不断电，load 进去的程序就可以一直跑


## rom、stage1 都读不到 4k spinand flash
现象：
- rom 只读 2k，不读到
- stage1 读 4k，也读不到
- bin 做的是 4k 的 bin，前面 2k 数据，后面 2k 是 0xff

测试：
- 换成烧 2k 的 bin，是否可以读到？也不行
- 注意：只有 gd 的那款 4k nand 命令不同，其余的都是相同的
- 注意：**4k 的 flash 擦除大小是 256k**

结果：
- ok 了，问题原因是由于 gd 的需要不同的 cmd 序列，而 xtx 的不需要
- rom 中读 gd 4k 的时候不需要额外的 cmd 序列，因为必定是从 page 的 0 地址开始读数据


## 有几款 flash 需要不同的方式使能 ecc，确认下是不是默认就开了？
![[Pasted image 20250327204205.png]]
- FS35ND01G-S1F1：需要手动 enable ecc，明天看下手册



# 测试结果确认：

spinor：
- [ ] 3 字节启动
- [ ] 4 字节启动
spinand：
- [ ] rom 不需要读坏块
- [ ] rom 需要支持 2k page，4k page 启动
- [ ] rom 从非 0 地址读 stage1
- [ ] rom 是否不需要备份？备份的原因是因为有坏块，但是 spinand 都自带 ecc，并且确认了 block0 是好块；那么即使有 block0 是坏块的情况，直接把 flash 片子丢掉好了，不要使用。
- [ ] rom 读操作之后没有看 ecc 状态
- [ ] 之前 cygnus、sirius 好像就支持 4k page 启动啊，怎么做的？命令序列不一样啊
	- [ ] 默认从 0 地址读，发 4 个字节，都是 0，就无论 dummy 的前后了
	- [ ] virgo 怎么做？只支持 0 地址开始读？还是无论什么地址？
	- [ ] 没关系，这里是 read_from_cache 不一样。不是 read_to_cache。
	- [ ] 这里确保每次都从 cache 0 地址开始读就行了
- [ ] stage1 需要读坏块标记，而不是坏块表
- [ ] stage1 需要跳过前面的坏块，例如：block2、3 为坏块，烧的时候将 block3 烧到了 block5，但是读的时候读 block2、block3 都会读到 block4，需要计算前面的坏块
- [ ] stage1 许多信息都要从参数配置中取
```c
typedef struct {
	unsigned int clk_div;        /* spinor 分频值 */
	unsigned int sample_delay;   /* spinor 读采样延时 */
	unsigned int addr_cyc;       /* spinor 地址长度 */
	unsigned int enable_quad;    /* spinor 使能四倍速模式 */
	unsigned int quad_type;      /* spinor 使能四倍速模式时操作寄存器的类型 */
}nor_flash_info_t;

typedef struct {
	unsigned int clk_div;                /* spinand 分频值 */
	unsigned int smp_dly_neg_spi;        /* spinand 采样延时 */
	unsigned int page_size;              /* spinand 页大小 */
	unsigned int ecc_enable;             /* spinand 使能ecc的方法 */
	unsigned int buf_enable;             /* spinand 需要使能buf功能 */
	unsigned int page_num_per_block;     /* spinand block含的页数 */
	unsigned int plane_sel;              /* spinand plane奇偶选择 */
	unsigned int tsc_tcshs;              /* spinand cs高电平最小宽度 */
	unsigned int dummy_before_addr;      /* spinand addr前是否有dummy字节 */
	unsigned int dummy_after_addr;       /* spinand addr后面是否有dummy字节 */
	unsigned int addr_cyc;               /* spinand 地址长度 */
}spinand_flash_info_t;

typedef struct {
	unsigned int page_size;              /* nand 页大小 */
	unsigned int page_num_per_block;     /* nand block含的页数 */
	unsigned int twp_twh;                /* nand we#高低电平宽度 */
	unsigned int trp_treh;               /* nand re#高低电平宽度 */
	unsigned int trp_treh_fast;          /* nand 批量读re#高低电平宽度 */
	unsigned int smp_dly_neg_fast;       /* nand 读采样延时 */
	unsigned int twb_trr;                /* nand rb#采样延时 */
	unsigned int trwad;                  /* nand 读写切换的时间 */
	unsigned int busy_timeout;           /* nand busy 超时时间 */
	unsigned int addr_cyc;               /* nand 地址长度 */
}paranand_flash_info_t;

```


## spinand ：
- rom：
	- [ ] 2k/4k page；
	- [ ] 0 地址启动，128k 地址启动
- spinand：
	- [ ] 2k/4k page；
	- [ ] 0 地址启动，128k 地址启动

2k page, 0 地址启动：ok
4k page, 0 地址启动：
- xtx ok
	- stage1 不要用 4k read 的接口，用普通的接口
	- stage1 读 dtebootcode 需要从 64k 的位置读
	- 烧 bin 的时候需要按照 256k 为一个 block 来烧，bl31、dtb 需要隔开一个 block，也就是一个 256k
- gd ok
	- **stage1 需要用 4k read 的接口**


2k page，256k 地址启动：ok
- rom 不需要改
- aux_cude. bin 需要烧到 256k 的位置，其余的继续烧到 2M 开始的位置
- stage1 读 dtebootcode 从 256+32k 的位置读

4k page，256k 地址启动：ok
- rom 不需要改
- aux_code. bin 需要烧到 256k 的位置，其余的继续烧到 2M 开始的位置
- stage1 需要用 4k read 的接口
- stage1 读 dtebootcode 从 256+64k 的位置读



## spinor
- rom：支持 3、4 字节地址启动，使用双线 0x3b 来读数据
- stage1：
	- 支持 3、4 字节地址启动，默认用双线 0x3b 读数据
	- 3、4 字节的参数 `addr_cyc` 没有用到，因为上面会 3 字节、4 字节分别读一次，rom 中就已经确认了模式
	- 开了宏 `CONFIG_ENABLE_BBT_SLT_TEST` 宏之后使用四线 0x6b 读数据
	- 使能四线的模式由变量 `quad_type` 决定，不需要 `enable_quad` 变量，只有 bbt/slt 才会用到四线模式
	- 使用硬件 cs
- 测试项：
	- 3 字节、4 字节地址启动
	- 0 地址、其它地址启动
	- stage1 双线、四线启动
- 实际测试：
	- 3 字节地址，0 地址启动：ok
		- rom：双线
		- stage1：四线
	- 3 字节地址，0 地址启动：ok
		- rom：双线
		- stage1：双线
	- 3 字节地址，256k 地址启动：ok
		- rom 不变
		- stage1：dtebootcode 从 256+32k 的位置开始读，后面的 bin 都从 2M 的位置开始烧
	- 4 字节地址，256k 地址启动：
		- rom 不变
		- stage1：也不需要修改
		- stage1 烧到 0x40000，后面的 bin 烧到 2M 开始的位置
		- 一开始 3 字节模式是否可以启动：ok
		- 进入 4 字节模式是否可以启动：ok


# 说明
spinand：
- rom：
	- [ ] 读 stage1 ，无论是 4k/2k page 每个 page 都只读 2k 数据
	- [ ] 支持 stage1 放在 flash 的任何位置
- stage1:
	- [ ] 读 stage2 时会跳过坏块，会计算前面从 0 开始到要读的地址的所有坏块数量
	- [ ] 2k flash 每个 page 读 2k，4k flash 每个 page 读 4k
- stage1 参数如下：
```c

```