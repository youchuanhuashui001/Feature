

# 概述



## NFC_V1：

- 特点：
  1. 控制器自带 ECC，提供给 Flash 用于纠错，修改了之前的擦除标记 bug。
  2. 读写擦等各种操作，有两种方式：
     - 自动的方式：通过配置控制器，由控制器来自动发指令、数据等操作
     - 手动的方式：通过配置控制器，手动往控制器的命令产生模块写入数据来发指令、数据等操作(类似 NFC_V0)
  
- 操作：
  - 读 Main 区域：
    - 地址对齐：直接读
    - 地址不对齐：要读的地址不是 页对齐，先  malloc 一块 page_size 大小的空间，然后判断当前 block 是否为坏块，如果为坏块则：`addr += block_size`，判断当前要读的总长度是否小于一个 page，然后调用 `nand_read_skip_bad` 函数来读数据，地址是 addr 所在的 page 首地址，长度是一个 page，把数据读到 临时 buffer，然后用 memcpy 把临时buffer 中后面的部分，也就是不对齐的那部分拷贝到要读到的内存地址去
    - 读操作：
      - 检测当前长度是否为坏块，是否需要跳坏块
      - 计算当前地址所在的 整个 flash 中的 page 偏移，如果是 build_in 模式或者 RAW 模式的话就关掉控制器的 ECC，否则配置开启控制器的 ECC
      - 判断当前要读的地址所在的列地址是否是 0，如果不是说明不是 页对齐的，那么找一个临时 buffer，读一个 page 的数据，然后把临时 buffer 后面的数据拷贝到真正的内存地址去
      - 计算要读到的内存地址地址是否是 cache_line_size、4 字节对齐，如果不是先读到一个 对齐的临时 buffer，然后 memcpy 过去，调用 `nandflash_readpage` 来读一个 page 的数据
        - 配置控制器、配ECC、把要读的地址、长度等信息配置给控制器，控制器自动发命令来完成读操作
  
  - 写 Main 区域：
    - 地址页对齐：直接写
    - 地址非页对齐：先判断当前地址是否是坏块，若是坏块则：`addr += block_size`，malloc 一个 page_size 大小的 buffer，然后把这个 buffer memset 成 0xff，把要写的最前面不对齐的那部分，拷贝到 buffer 的后面，相当于有一个 buffer，前面是 0xff，后面是要写的数据，然后调用接口来写一个 page，把这个 buffer 写进去
    - 写操作：
      - 写接口是按照 block 的方式来写的，如果不满一个 block，则要写的大小是所有的长度；如果是多个 block，则要循环多次写
      - 没有 oob 区域，所以只会写 main 区域
        - 自带 ecc 或者使用 RAW 模式：关掉控制器的 ECC
        - 不自带 ECC：配置控制器的 ECC 寄存器，调用函数 `gxnfc_nand_write_data   gx_nfc_v1.c` 来写数据
        - 把要写的地址转换成 行地址`(row = to / page_size)`、列地址`(col = to % page_size)` 的形式
        - 计算要写的地址不是页对齐，也就是列地址不为0，先把 buffer 置为 0xff，然后把要写的数据拷贝到 buffer+col 的地址去，相当于是 buffer 里面前面是 0xff，后面是要写的数据，然后这个 page 的数据就不是 data，而是这个 buffer，把这个 buffer 写到 flash 中
        - 如果是写的地址是页对齐的，计算要写的长度有几个 page，先把整 page 的数据写完，然后计算要写的数据所在的地址是否是 CACHE_LINE_SIZE 对齐或者其它的方式对齐的
          - 如果是 cache_line_size 对齐的话，就调用 `nandflash_programpage` 直接写，配置控制器、配置 ECC、把要写的数据的地址和长度等配给控制器，控制器自己发命令来自动完成，这样完成写一个 page
          - 如果不对齐的话，则把数据拷贝到一个对齐的地址，然后写一个 page
        - 上面把 整个 page 的数据写完了，还需要写末尾不是 page 对齐的数据，找一个对齐的 buffer，然后把要写的数据拷贝到这个 buffer，这个buffer里面后面是 0xff，前面是要写的数据，然后用 `nandflash_programpage` 来让控制器自动完成写入操作
    - 如果写入失败的话，就标记一下这个块是坏块
      - 标记坏块会把当前 block 的 oob 区域第一个字节标记为 0x00，然后把内存中的坏块表更新为 01，然后更新 flash 中的坏块表
  
  - 擦除操作：
    - 把要擦除的地址和长度重新进行计算，让其是从 block 起始，并且 block 结尾，即擦除操作是整个 block 的，但是也没有用计算出来的起始地址和结束地址
    - 重新计算要擦除的 地址和长度，让地址是block起始，长度是 block 对齐
    - 调用 `nand_erase_opts` 来进行擦除操作
    - 判断当前要擦除的地址是否是坏块，调用 `nand_erase` 进行擦除操作
      - 根据要擦除的 block 转换成 page，调用 `chip->erase_cmd` 来进行擦除操作，使用手动发命令的方式按照 flash 的要求来完成擦除操作
  
  - 读 OOB 区域：
  
    - 读的 oob 区域是当前地址所在 page 的 oob 区域，每个 page 都有 oob 区域
    - 将要读的地址转换成真实的 page，然后调用 `chip->ecc.read_oob` 
    - 配控制器的 type 寄存器，让数据的地址放置在 oob 区域，要读的 page 号配置好、长度是 oobsize，然后启动控制器，等控制器就绪，然后从 data 寄存器读数据
    - 刚刚控制器读到的数据是读到了chip->oob_poi 这个里面，现在用 memcpy 把数据拷贝到 oobbuf 
  
  - 写 OOB 区域：
  
    - 写的 oob 区域是当前地址所在 page 的 oob 区域
    - 由于没有 datbuf，所以调用 `nand_do_write_oob`
    - 将要写的地址转换成真实的 page，然后调用 `nand_fill_oob`，将要写的 `ops->oobbuf` 拷贝到 `chip->oob_poi` 这个地方去
    - 调用 `chip->ecc.write_oob` 接口
    - `const uint8_t *buf = chip->oob_poi` 这里要写的 buf 就是刚刚 memcpy 的
    - 发命令 0x80，列地址 0x800，行地址 0x0
    - 写数据
    - 发命令 0x10
  
  - 同时读 Main 区域 + OOB 区域：
    - 自带 ECC：
    - 不带 ECC：
      - 赋值 datbuf、oobbuf，调用`nand_do_read_ops`
      - 根据地址，计算出实际的 page、行地址
      - <font color=red>先读 main 区域</font>
      - 如果模式是 RAW 或者是 build_in 的话，就关掉控制器的 ECC
      - 计算列地址是否是页对齐的，如果不是页对齐就先把数据读到临时 buf，然后再把后面不对齐的部分拷贝到真实的 buf
      - 如果是页对齐的，调用 `nandflash_readpage` 读取数据，配置控制器的 自动模式，然后读 main 数据
      - <font color=red>再读 oob 区域</font>
      - 调用 `gxnfc_read_oob` 来读 oob 区域的数据，配置控制器的 自动模式，然后读 oob 数据到 chip->oob_poi，使用 `nand_transfer_oob` 接口来将数据从 `chip->oob_poi` 拷贝到 `oobbuf`
  
  - 同时写 Main 区域 + OOB 区域：
  
    - 自带 ECC：
    - 不带 ECC：
      - 赋值 datbuf、oobbuf，调用 `nand->write_oob`
      - <font color=red>先写 oob 区域</font>
      - 把数据从 `oobbuf` 拷贝到 `chip->oob_poi`
      - 调用 `chip->ecc.write_oob`
      - 发命令 0x80，行地址是 当前 page，列地址是 oob 区域
      - 写数据
      - 发命令 0x10
      - <font color=red>再写 main 区域</font>
      - 如果模式是 RAW 或者 build_in，就关掉控制器的ECC
      - 调用 `chip->write_data`
      - 把要写的地址、page、长度之类的配置给控制器，控制器自动发 main 区域的数据
  
  
  - 软件方式计算 ecc
    - 写操作的时候，canopus 按照 sector 的方式每次写入 512 个字节的数据，然后计算出 ecc
    - 把数据写到 ecc_data 寄存器，然后读 ecc_code 寄存器的 ecc 码
  - 软件方式 ecc 纠错
    - 纠错 ecc，先把数据写到 ecc_data 寄存器，然后把 ecc_code 写到 校验 寄存器，然后读状态寄存器，如果出错把错误的位置找出来
    - canopus 先发命令，地址，然后按照 sector 的方式把 main 数据读出来，根据 main 数据算 ecc，然后读 oob 区域的数据，把 oob 区域的 ecc code 单独拎出来，按照每个 sector 的方式来纠错，使用软件的方式来纠错(控制器根据传入的 data、ecc 检验出哪一个 bit 或哪几个 bit 有跳变，然后手动把跳变的 bit 纠正回来)
    - sirius 先把 oob 区域读出来，memcpy 到 chip->oob_poi 里面，发一个 0x05 0 0xE0，读一个 page，按照 sector 的方式来纠错，使用 main 数据，sector number，ecc_code 用软件的方式纠错
      - 没看到写寄存器的操作啊



## NFC_V0：

- 特点：
  
  1. 控制器带 ECC，提供给 Flash 用于纠错，但当 flash 的数据为全 0xFF 时，ECC 数据不为全 0xFF，无法纠错，采用软件的方法进行规避：在 OOB 区域内 ecc 的前面放置长度为 4 个字节的擦除标记，每个字节代表 Flash 的某个 Sector(大小为 512)，若 Sector 数据为全 0xFF 的时候，则对应的字节在擦除时被写为 0xFF。
  
  2. 读写擦等各种操作，都是通过发指令的方式来进行的。
  
- 操作：
  - 读 Main 区域：
  
    - 通过 `chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page);` 来做的读操作，至于有的行地址是两个字节，有的行地址是三个字节，是根据 flash size 来处理的，如果是 2G 的，那么会发三个字节，如果是 1g 的只发 两个字节。
  - 写 Main 区域：
  - 读 OOB 区域：
  - 写 OOB 区域：
  - 一次读写 Main 区域 + OOB 区域：
    - 自带 ECC：
    - 不带 ECC：
  - 分次读写 Main 区域、OOB 区域：
    - 自带 ECC：
    - 不带 ECC：
  
  - 软件方式计算 ecc
  
    - 写操作的时候，sirius 一次写入整个page 的数据，然后计算出 ecc
  
      
  
  - 软件方式纠错 ecc
  
    - 先要把 ecc 数据从 oob 区域读出来，写到控制器的寄存器，然后开始读 page，控制器会告诉你在哪里有错误，然后用软件进行纠错













## nand.c

- 这个文件用于向 gxloader、linux 提供应用接口，主要是做一些封装工作，下层封装的是 base.c 中的接口
- gxmisc 驱动会独立维护一个 `struct gx_nand_dev`结构，编译生成库之后其它程序调用的时候，不需要传递任何的设备信息，没有任何关联，只需要传递要操作的地址、长度、buffer 等信息

- 事先查看一些结构体：
   - `struct gx_nand_dev`：这个结构体用于表示一个 nand flash 设备
   - `struct gx_nand_chip`：这个结构体用于表示一个 nand flash 控制器
```c
struct gx_nand_dev {
	void *                 regs;
	struct gx_nand_chip    chip;
	struct gx_mtd_info     mtd;
};

// 这是控制器相关的一些信息，并且抽象出控制器支持的一些功能
struct gx_nand_chip {
	void  __iomem   *IO_ADDR_R;
	void  __iomem   *IO_ADDR_W;
	struct nand_request *request;
	uint8_t         (*read_byte)(struct gx_mtd_info *mtd);
	u16             (*read_word)(struct gx_mtd_info *mtd);
	void            (*write_buf)(struct gx_mtd_info *mtd, const uint8_t *buf, int len);
	void            (*read_buf)(struct gx_mtd_info *mtd, uint8_t *buf, int len);
	int             (*verify_buf)(struct gx_mtd_info *mtd, const uint8_t *buf, int len);
	void            (*select_chip)(struct gx_mtd_info *mtd, int chip);
	int             (*block_bad)(struct gx_mtd_info *mtd, loff_t ofs, int getchip);
	int             (*block_markbad)(struct gx_mtd_info *mtd, loff_t ofs);
	void            (*cmd_ctrl)(struct gx_mtd_info *mtd, int dat, unsigned int ctrl);
	int             (*dev_ready)(struct gx_mtd_info *mtd);
	void            (*cmdfunc)(struct gx_mtd_info *mtd, unsigned command, int column, int page_addr);
	int             (*waitfunc)(struct gx_mtd_info *mtd, struct gx_nand_chip *this);
	void            (*erase_cmd)(struct gx_mtd_info *mtd, int page);
	int             (*scan_bbt)(struct gx_mtd_info *mtd);
	int             (*errstat)(struct gx_mtd_info *mtd, struct gx_nand_chip *this, int state, int status, int page);
	int             (*write_page)(struct gx_mtd_info *mtd, struct gx_nand_chip *chip, const uint8_t *buf, int page, int cached, int raw);
	int             (*read_data)(struct gx_mtd_info *mtd, unsigned int from, unsigned char *data, unsigned int len, unsigned int* readlen);
	int             (*write_data)(struct gx_mtd_info *mtd, unsigned int to,  unsigned char *data, unsigned int len, unsigned int* writelen);

	int             chip_delay;
	unsigned int    options;
	unsigned int    bbt_options;

	int             page_shift;
	int             phys_erase_shift;
	int             bbt_erase_shift;
	int             chip_shift;
	int             numchips;
	uint64_t        chipsize;
	int             pagemask;
	int             pagebuf;
	int             subpagesize;
	uint8_t         cellinfo;
	int             badblockpos;
	int             onfi_version;
#ifdef CONFIG_SYS_NAND_ONFI_DETECTION
	struct nand_onfi_params onfi_params;
#endif

	int                    state;
	uint8_t                *oob_poi;
	uint8_t                *page_erase_mark;
	int                    ecc_buildin;
	struct nand_ecclayout  *ecclayout;
	struct nand_ecc_ctrl   ecc;
	struct nand_buffers    *buffers;
	void                   *malloc_buffers;
	struct gx_mtd_oob_ops  ops;

	uint8_t                *bbt;
	struct nand_bbt_descr  *bbt_td;
	struct nand_bbt_descr  *bbt_md;
	struct nand_bbt_descr  *badblock_pattern;
	void                   *priv;
	uint8_t                cache_read_flag;
};

// 这个可以看做是 mtd 设备的一些信息，和接口，这些接口是提供给 mtd 层使用的
struct gx_mtd_info {
	u_char type;
	u_int32_t flags;
	uint64_t size;    /* Total size of the MTD */

	/* "Major" erase size for the device. Na?ve users may take this
	 * to be the only erase size available, or may use the more detailed
	 * information below if they desire
	 */
	u_int32_t erasesize;
	/* Minimal writable flash unit size. In case of NOR flash it is 1 (even
	 * though individual bits can be cleared), in case of NAND flash it is
	 * one NAND page (or half, or one-fourths of it), in case of ECC-ed NOR
	 * it is of ECC block size, etc. It is illegal to have writesize = 0.
	 * Any driver registering a struct gx_mtd_info must ensure a writesize of
	 * 1 or larger.
	 */
	u_int32_t writesize;
	u_int32_t oobsize;   /* Amount of OOB data per block (e.g. 16) */
	u_int32_t oobavail;  /* Available OOB bytes per block */

	/* Kernel-only stuff starts here. */
	const char *name;
	uint8_t manufactor_id;
	uint8_t device_id;

	/* ecc layout structure pointer - read only ! */
	struct nand_ecclayout *ecclayout;

	/* max number of correctible bit errors per ecc step */
	unsigned int ecc_strength;

	/*
	 * Erase is an asynchronous operation.  Device drivers are supposed
	 * to call instr->callback() whenever the operation completes, even
	 * if it completes with a failure.
	 * Callers are supposed to pass a callback function and wait for it
	 * to be called before writing to the block.
	 */
	int (*erase) (struct gx_mtd_info *mtd, struct gx_erase_info *instr);
	int (*read) (struct gx_mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
	int (*write) (struct gx_mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
	int (*read_oob) (struct gx_mtd_info *mtd, loff_t from, struct gx_mtd_oob_ops *ops);
	int (*write_oob) (struct gx_mtd_info *mtd, loff_t to, struct gx_mtd_oob_ops *ops);
	void (*sync) (struct gx_mtd_info *mtd);

	/* Chip-supported device locking */
	int (*lock) (struct gx_mtd_info *mtd, loff_t ofs, uint64_t len);
	int (*unlock) (struct gx_mtd_info *mtd, loff_t ofs, uint64_t len);

	/* Bad block management functions */
	int (*block_isbad) (struct gx_mtd_info *mtd, loff_t ofs);
	int (*block_markbad) (struct gx_mtd_info *mtd, loff_t ofs);

	/* ECC status information */
	struct gx_mtd_ecc_stats ecc_stats;
	/* Subpage shift (NAND) */
	int subpage_sft;

	void *priv;
	void *mtd_dev;
	unsigned int max_dma_len;
};

```
### init

- init 函数中，首先会调用 gxnfc_init_chip_vx ，这是在初始化结构 `struct gx_nand_chip`，这个结构会定义在 `struct gx_nand_dev`中，通过一个指针的形式建立起链接
```c
int gxnfc_init_chip_v1(struct gx_nand_dev *info)
{
	struct gx_nand_chip *chip  = &info->chip;

	chip->select_chip   = NULL;

	chip->write_buf     = nandflash_write_buf;
	chip->read_buf      = nandflash_read_buf;
	chip->chip_delay    = 0;
	chip->priv          = info;
	chip->options       = 0;
	chip->IO_ADDR_W     = NULL;
	chip->IO_ADDR_R     = NULL;
	chip->cmd_ctrl      = gxnfc_nand_hwcontrol;
	chip->dev_ready     = gxnfc_nand_dev_ready;
	chip->cmdfunc       = gxnfc_nand_command;
	chip->waitfunc      = gxnfc_nand_wait_ready; /* NOTE: only used for write operations */
	chip->read_byte     = gxnfc_nand_read_byte;
	chip->read_data     = gxnfc_nand_read_data;
	chip->write_data    = gxnfc_nand_write_data;

	info->mtd.priv      = info;
	chip->ecc.calculate = gxnfc_nand_calculate_ecc;
	chip->ecc.correct   = NULL;
	chip->ecc.mode      = NAND_ECC_HW;
	chip->ecc.size      = GX_NAND_512_PAGESIZE;
	chip->ecc.hwctl     = gxnfc_nand_enable_hwecc;
	chip->ecc.read_oob  = gxnfc_nand_read_oob;
	chip->ecc.config    = gxnfc_nand_ecc_config_v1;

	gxnfc_nand_inithw(info);
	return 0;
}

```

- 现在先不关心这些接口是怎么实现的，直接看上层是怎么用的
- 调用 nand_scan(&dev->mtd, 1)，调到了 nand_base.c 中，后面再看具体的实现
- init 接口结束

### nand_erase_remap

- 由于 nand flash 的擦除都是以 block 为单位，此函数用作起始地址和结束地址保持 block 对齐
```c
static void nand_erase_remap(loff_t *offset, loff_t *length)
{
	int addr = *offset;
	int len = *length;
	int end_addr = addr + len;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	uint32_t block_size = nand->erasesize;

	if (addr % block_size)
		addr = addr / block_size * block_size;
	if (end_addr % block_size)
		end_addr = (end_addr + block_size - 1) / block_size * block_size;

	*offset = addr;
	*length = end_addr - addr;
}
```

### erase

- chiperase接口：用来擦除整个flash
```c
void gx_nand_chiperase(void)
{
	int ret = 0;
	int quiet = 1;
	struct nand_erase_options opts;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	memset(&opts, 0, sizeof(opts));
	opts.offset = 0;
	opts.length = nand->size;
	opts.quiet = quiet;
	opts.scrub = 0;

	ret = nand_erase_opts(nand, &opts);
	gxlog_i("%s\n", ret ? "ERROR" : "OK");
}

```

- 先看 `struct nand_erase_options`结构体，抽象出一个擦除操作，包括长度、在 flash 中的偏移，是否擦除 bbt
```c
struct nand_erase_options {
	loff_t length;          /* number of bytes to erase */
	loff_t offset;          /* first address in NAND to erase */
	int quiet;              /* don't display progress messages */
	/* if true, really clean NAND by erasing bad blocks (UNSAFE) */
	int scrub;
	/* Don't include skipped bad blocks in size to be erased */
	int spread;
};
```

- 调用 nand_erase_opts(nand, &opts) 的时候就调到了 nand_utils.c 中，目的是在调用 控制器的接口的时候要先做一些处理
```c
int nand_erase_opts(struct gx_mtd_info *mtd, const struct nand_erase_options *opts)
{
	struct gx_erase_info erase;
	unsigned long erase_length, erased_length; /* in blocks */
	int bbtest = 1;
	int result;
	int percent_complete = -1;
	const char *mtd_device = mtd->name;
	struct gx_mtd_oob_ops oob_opts;
	struct gx_nand_chip *chip = mtd_to_chip(mtd);

	if ((opts->offset & (mtd->writesize - 1)) != 0) {
		gxlog_e("Attempt to erase non page aligned data\n");
		return -1;
	}

	memset(&erase, 0, sizeof(erase));
	memset(&oob_opts, 0, sizeof(oob_opts));

	erase.len  = mtd->erasesize;
	erase.addr = opts->offset;
	erase_length = lldiv(opts->length + mtd->erasesize - 1, mtd->erasesize);

	/* scrub option allows to erase badblock. To prevent internal
	 * check from erase() method, set block check method to dummy
	 * and disable bad block table while erasing.
	 */
	if (opts->scrub) {
		erase.scrub = opts->scrub;
		/*
		 * We don't need the bad block table anymore...
		 * after scrub, there are no bad blocks left!
		 */
		if (chip->bbt) {
			kfree(chip->bbt);
		}
		chip->bbt = NULL;
	}
	for (erased_length = 0; erased_length < erase_length; erase.addr += mtd->erasesize) {
		if (erase.addr >= mtd->size) {
			gxlog_e("skip erase outside flash area\n");
			break;
		}

		if (!opts->scrub && bbtest) {
			int ret = mtd->block_isbad(mtd, erase.addr);
			if (ret > 0) {
				if (!opts->quiet)
					gxlog_i("\rSkipping bad block at 0x%08x\n", erase.addr);
				if (!opts->spread)
					erased_length++;
				continue;
			} else if (ret < 0) {
				gxlog_e("\n%s: MTD get bad block failed: %d\n", mtd_device, ret);
				return -1;
			}
		}
		result = nand_erase(mtd, &erase);
		if (result == -EIO) {
			int ret = mtd->block_markbad(mtd, erase.addr);
			if (ret == 0) {
				if (!opts->quiet)
					gxlog_e("\nMTD erase failed,make bad block at 0x%08x\n", erase.addr);

				if (!opts->spread)
					erased_length++;

				continue;
			}
			else {
				gxlog_e("\nMTD make bad block failed at 0x%08x: %d\n",erase.addr, ret);
				return -1;
			}
		}
		else if (result != 0) {
			gxlog_e("%s: MTD Erase failure at 0x%08x: %d\n", mtd_device,erase.addr,result);
			return -1;
		}
		erased_length++;
		if (!opts->quiet) {
			unsigned long long n = erased_length * 100ULL;
			int percent;

			do_div(n, erase_length);
			percent = (int)n;

			/* output progress message only at whole percent
			 * steps to reduce the number of messages printed
			 * on (slow) serial consoles
			 */
			if (percent != percent_complete) {
				percent_complete = percent;
			}
		}
	}
	//if (!opts->quiet)
	//gxlog_e("\n");

	if (opts->scrub)
		chip->scan_bbt(mtd);

	return 0;
}

```

- 看一下 `struct gx_erase_info`结构，
```c
struct gx_erase_info {
	uint32_t addr;
	uint32_t len;
	uint32_t fail_addr;
	u_char state;
	int scrub;
};
```

- 看一下 `struct gx_mtd_oob_ops`结构：会包括一些 oob 的信息，但是也包括了 main 区域的信息
```c
struct gx_mtd_oob_ops {
	unsigned int mode;
	size_t       len;
	size_t       retlen;
	size_t       ooblen;
	size_t       oobretlen;
	uint32_t     ooboffs;
	uint8_t      *datbuf;
	uint8_t      *oobbuf;
};
```

- 擦除过程，先判断一下当前要擦除的地址是不是坏块，如果是坏块，就增加一个 block_size；然后调用 nand_erase(mtd, &erase) 接口来擦除一个 block
- 再看 nand_erase 接口：--> nand_erase_nand(mtd, instr, 0) 擦除一个或多个 block
- nand_erase_nand：先限定长度、地址都 block 对齐，限定长度不能超过 flash 的大小，检查是否写保护，根据地址找到对应的 page，如果 scrub 为0，也就是不擦坏块表且这个page存在坏块的话，这个 block 不擦并标识此 block 已经被擦除了，调用 chip->erase_cmd(mtd, page&chip->pagemask) 接口
   - chip->erase_cmd --> single_erase_cmd
```c
static void single_erase_cmd(struct gx_mtd_info *mtd, int page)
{
	struct gx_nand_chip *chip = mtd_to_chip(mtd);
	/* Send commands to erase a block */
	chip->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page);
	chip->cmdfunc(mtd, NAND_CMD_ERASE2, -1, -1);
}

0x60
write_cmd(dev, NAND_CMD_ERASE1)

write_addr(dev, (__u8) (page_addr & 0xff));
write_addr(dev, (__u8) ((page_addr >> 8) & 0xff));

0xd0
write_cmd(dev, NAND_CMD_ERASE2)
```

- 然后等待 ready，如果要擦除多个 block，就继续上面的循环步骤
- 再回退到之前的函数，执行完 nand_erase 函数就结束了，如果擦除失败的话，就调用 mtd->block_markbad 标记这是个坏块，否则这一个 block 就擦除成功了，循环调用来擦掉整个 chip
   - mtd->block_markbad：
```c
static int nand_block_markbad(struct gx_mtd_info *mtd, loff_t ofs)
{
	int ret;

	ret = nand_block_isbad(mtd, ofs);
	if (ret) {
		/* If it was bad already, return success and do nothing */
		if (ret > 0)
			return 0;
		return ret;
	}

	return nand_block_markbad_lowlevel(mtd, ofs);
}

static inline uint8_t bbt_get_entry(struct gx_nand_chip *chip, int block)
{
	uint8_t entry = chip->bbt[block >> BBT_ENTRY_SHIFT];
	entry >>= (block & BBT_ENTRY_MASK) * 2;
	return entry & BBT_ENTRY_MASK;
}


int nand_isbad_bbt(struct gx_mtd_info *mtd, loff_t offs, int allowbbt)
{
	struct gx_nand_chip *this = mtd_to_chip(mtd);
	int block, res;

	block = (int)(offs >> this->bbt_erase_shift);
	res = bbt_get_entry(this, block);
#if 0
	gxlog_d("nand_isbad_bbt(): bbt info for offs 0x%08x: (block %d) 0x%02x\n",
			(unsigned int)offs, block, res);
#endif
	switch (res) {
	case BBT_BLOCK_GOOD:
		return 0;
	case BBT_BLOCK_WORN:
		return 1;
	case BBT_BLOCK_RESERVED:
		return allowbbt ? 0 : 1;
	}
	return 1;
}



```

- 先计算这个地址在哪一个 block，然后看这个 block 是否是坏块， allowbbt 代表是否允许进入坏块表，即坏块表所在的block在应用中被看做坏块
- 检查完是否是坏块之后，对坏块进行标记更新
   - 先擦掉这个 block，  这个地方有待跟踪，需要继续看
   - 标记坏块表，nand_markbad_bbt(mtd, ofs)，先更新 内存中的坏块表，如果 坏块表存在 flash 上，再更新 flash 上的坏块表
```c
int nand_markbad_bbt(struct gx_mtd_info *mtd, loff_t offs)
{
	struct gx_nand_chip *this = mtd_to_chip(mtd);
	int block, ret = 0;

	block = (int)(offs >> this->bbt_erase_shift);

	/* Mark bad block in memory */
	bbt_mark_entry(this, block, BBT_BLOCK_WORN);

	/* Update flash-based bad block table */
	if (this->bbt_options & NAND_BBT_USE_FLASH)
		ret = nand_update_bbt(mtd, offs);

	return ret;
}
```

- 看如何更新坏块表：
   - 计算长度，更新坏块表的版本号(主表+镜像表)，写主表
```c
static int nand_update_bbt(struct gx_mtd_info *mtd, loff_t offs)
{
	struct gx_nand_chip *this = mtd_to_chip(mtd);
	int len, res = 0;
	int chip, chipsel;
	uint8_t *buf;
	struct nand_bbt_descr *td = this->bbt_td;
	struct nand_bbt_descr *md = this->bbt_md;

	if (!this->bbt || !td)
		return -EINVAL;

	/* Allocate a temporary buffer for one eraseblock incl. oob */
	len = (1 << this->bbt_erase_shift);
	len += (len >> this->page_shift) * mtd->oobsize;
	buf = kmalloc(len, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	/* Do we have a bbt per chip? */
	if (td->options & NAND_BBT_PERCHIP) {
		chip = (int)(offs >> this->chip_shift);
		chipsel = chip;
	} else {
		chip = 0;
		chipsel = -1;
	}

	td->version[chip]++;
	if (md)
		md->version[chip]++;

	/* Write the bad block table to the device? */
	if (td->options & NAND_BBT_WRITE) {
		res = write_bbt(mtd, buf, td, md, chipsel);
		if (res < 0)
			goto out;
	}
	/* Write the mirror bad block table to the device? */
	if (md && (md->options & NAND_BBT_WRITE)) {
		res = write_bbt(mtd, buf, md, td, chipsel);
	}

out:
	kfree(buf);
	return res;
}

```

- 写坏块表，write_bbt，获取坏块表所在的 block，根据 block 转换成 page，组建好坏块表，memcpy 到 buf 中去，然后擦掉这个 block，把 buf 写到那个 page 去，长度是 页对齐的
```c
static int write_bbt(struct gx_mtd_info *mtd, uint8_t *buf,
		struct nand_bbt_descr *td, struct nand_bbt_descr *md,
		int chipsel)
{
	struct gx_nand_chip *this = mtd_to_chip(mtd);
	struct gx_erase_info einfo;
	int i, res, chip = 0;
	int bits, page, offs, numblocks, sft, sftmsk;
	int nrchips, pageoffs, ooboffs;
	uint8_t msk[4];
	uint8_t rcode = td->reserved_block_code;
	size_t retlen, len = 0;
	loff_t to;
	struct gx_mtd_oob_ops ops;
	uint32_t crc;

	ops.ooblen = mtd->oobsize;
	ops.ooboffs = 0;
	ops.datbuf = NULL;
	ops.mode = GX_MTD_OPS_PLACE_OOB;

	if (!rcode)
		rcode = 0xff;
	/* Write bad block table per chip rather than per device? */
	if (td->options & NAND_BBT_PERCHIP) {
		numblocks = (int)(this->chipsize >> this->bbt_erase_shift);
		/* Full device write or specific chip? */
		if (chipsel == -1) {
			nrchips = this->numchips;
		} else {
			nrchips = chipsel + 1;
			chip = chipsel;
		}
	} else {
		numblocks = (int)(mtd->size >> this->bbt_erase_shift);
		nrchips = 1;
	}

	/* Loop through the chips */
	while (chip < nrchips) {
		int block;

		block = get_bbt_block(this, td, md, chip);
		if (block < 0) {
			gxlog_e("No space left to write bad block table\n");
			res = block;
			goto outerr;
		}

		/*
		 * get_bbt_block() returns a block number, shift the value to
		 * get a page number.
		 */
		page = block << (this->bbt_erase_shift - this->page_shift);

		/* Set up shift count and masks for the flash table */
		bits = td->options & NAND_BBT_NRBITS_MSK;
		msk[2] = ~rcode;
		switch (bits) {
		case 1: sft = 3; sftmsk = 0x07; msk[0] = 0x00; msk[1] = 0x01;
			msk[3] = 0x01;
			break;
		case 2: sft = 2; sftmsk = 0x06; msk[0] = 0x00; msk[1] = 0x01;
			msk[3] = 0x03;
			break;
		case 4: sft = 1; sftmsk = 0x04; msk[0] = 0x00; msk[1] = 0x0C;
			msk[3] = 0x0f;
			break;
		case 8: sft = 0; sftmsk = 0x00; msk[0] = 0x00; msk[1] = 0x0F;
			msk[3] = 0xff;
			break;
		default: return -EINVAL;
		}

		to = ((loff_t)page) << this->page_shift;

		/* Must we save the block contents? */
		if (td->options & NAND_BBT_SAVECONTENT) {
			/* Make it block aligned */
			to &= ~(((loff_t)1 << this->bbt_erase_shift) - 1);
			len = 1 << this->bbt_erase_shift;
			res = mtd->read(mtd, to, len, &retlen, buf);
			if (res < 0) {
				if (retlen != len) {
					gxlog_e("nand_bbt: error reading block for writing the bad block table\n");
					return res;
				}
				gxlog_e("nand_bbt: ECC error while reading block for writing bad block table\n");
			}
			/* Read oob data */
			ops.ooblen = (len >> this->page_shift) * mtd->oobsize;
			ops.oobbuf = &buf[len];
			res = mtd->read_oob(mtd, to + mtd->writesize, &ops);
			if (res < 0 || ops.oobretlen != ops.ooblen)
				goto outerr;

			/* Calc the byte offset in the buffer */
			pageoffs = page - (int)(to >> this->page_shift);
			offs = pageoffs << this->page_shift;
			/* Preset the bbt area with 0xff */
			memset(&buf[offs], 0xff, (size_t)(numblocks >> sft));
			ooboffs = len + (pageoffs * mtd->oobsize);

		} else if (td->options & NAND_BBT_NO_OOB) {
			ooboffs = 0;
			offs = td->len;
			/* The version byte */
			if (td->options & NAND_BBT_VERSION)
				offs++;
			offs += BBT_MARK_LENGTH_LEN + BBT_MARK_RESERVED_LEN;
			/* Calc length */
			len = (size_t)(numblocks >> sft);
			len += offs;
			/* Make it page aligned! */
			len = gx_roundup(len, mtd->writesize);
			/* Preset the buffer with 0xff */
			memset(buf, 0xff, len);
			/* Pattern is located at the begin of first page */
			memcpy(buf, td->pattern, td->len);
		} else {
			/* Calc length */
			len = (size_t)(numblocks >> sft);
			/* Make it page aligned! */
			len = gx_roundup(len, mtd->writesize);
			/* Preset the buffer with 0xff */
			memset(buf, 0xff, len +
					(len >> this->page_shift)* mtd->oobsize);
			offs = 0;
			ooboffs = len;
			/* Pattern is located in oob area of first page */
			memcpy(&buf[ooboffs + td->offs], td->pattern, td->len);
		}

		if (td->options & NAND_BBT_VERSION)
			buf[ooboffs + td->veroffs] = td->version[chip];

		buf[offs-2] = (uint8_t)(numblocks >> sft);
		buf[offs-1] = (uint8_t)((numblocks >> sft) >> 8);

		/* Walk through the memory table */
		for (i = 0; i < numblocks; i++) {
			uint8_t dat;
			int sftcnt = (i << (3 - sft)) & sftmsk;
			dat = bbt_get_entry(this, chip * numblocks + i);
			/* Do not store the reserved bbt blocks! */
			buf[offs + (i >> sft)] &= ~(msk[dat] << sftcnt);
		}

		if (td->options & NAND_BBT_NO_OOB) {
			crc = bbt_crc32(buf, offs + (numblocks >> sft));
		}

		memcpy(buf + offs + (numblocks >> sft), &crc, 4);

		memset(&einfo, 0, sizeof(einfo));
		einfo.addr = to;
		einfo.len = 1 << this->bbt_erase_shift;
		res = nand_erase_nand(mtd, &einfo, 1);
		if (res < 0) {
			gxlog_e("nand_bbt: error while erasing BBT block %d\n",
					res);
			mark_bbt_block_bad(this, td, chip, block);
			continue;
		}

		res = scan_write_bbt(mtd, to, len, buf,
				td->options & NAND_BBT_NO_OOB ? NULL :
				&buf[len]);
		if (res < 0) {
			gxlog_e("nand_bbt: error while writing BBT block %d\n",
					res);
			mark_bbt_block_bad(this, td, chip, block);
			continue;
		}

		gxlog_i("Bad block table written to 0x%08x, version 0x%02X\n", (unsigned int)to, td->version[chip]);

		/* Mark it as used */
		td->pages[chip++] = page;
	}
	return 0;

outerr:
	if (res != -ENOSPC)
		gxlog_e("nand_bbt: error while writing bad block table %d\n", res);
	return res;
}

```

- 现在已经有了 内存中临时的坏块表 buf，要写的page地址，要写的长度(page对齐)，调用 scan_write_bbt 写：oob 是空的，datbuf 是组建好的坏块表，len是长度，offs 是地址，模式用 place_oob，再调用 nand_do_write_ops(mtd, to, ops)
```c
static int scan_write_bbt(struct gx_mtd_info *mtd, loff_t offs, size_t len,
		uint8_t *buf, uint8_t *oob)
{
	struct gx_mtd_oob_ops ops;

	ops.mode = GX_MTD_OPS_PLACE_OOB;
	ops.ooboffs = 0;
	ops.ooblen = mtd->oobsize;
	ops.datbuf = buf;
	ops.oobbuf = oob;
	ops.len = len;

	return mtd->write_oob(mtd, offs, &ops);
}

static int nand_do_write_ops(struct gx_mtd_info *mtd, loff_t to, struct gx_mtd_oob_ops *ops)
{
	int chipnr, realpage, page, blockmask, column;
	struct gx_nand_chip *chip = mtd_to_chip(mtd);
	uint32_t writelen = ops->len;
	uint8_t *oob = ops->oobbuf;
	uint8_t *buf = ops->datbuf;
	int ret = 0, subpage, i = 0;

	ops->retlen = 0;
	if (!writelen)
		return 0;

	/*use for nfc v1*/
	if (chip->write_data != NULL) {
		if(unlikely(oob)) {
			page = (int)(to >> chip->page_shift);
			memset(chip->oob_poi, 0xff, mtd->oobsize);
			oob = nand_fill_oob(chip, oob, ops);

			/* if write data and oob */
			if(buf && ops->mode == GX_MTD_OPS_PLACE_OOB) {
				uint32_t *eccpos = chip->ecc.layout->eccpos;
				for (i = 0; i < chip->ecc.total; i++)
					chip->oob_poi[eccpos[i]] = 0xff;
			}
			ret = chip->ecc.write_oob(mtd, chip, page & chip->pagemask);
			ops->oobretlen = ops->ooblen;
			if(ret < 0)
				return ret;
		}

		if(buf) {
			if ((ops->mode == GX_MTD_OPS_RAW) || (chip->ecc_buildin == true))
				chip->ecc.hwctl(mtd, NAND_ECC_DISABLE);
			else
				chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
			ret = chip->write_data(mtd, to, buf, ops->len, &writelen);
			ops->retlen = writelen;
		}
		return ret;
	}

	column = to & (mtd->writesize - 1);
	subpage = column || (writelen & (mtd->writesize - 1));

	if (subpage && oob)
		return -EINVAL;

	chipnr = (int)(to >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);

	/* Check, if it is write protected */
	if (nand_check_wp(mtd)) {
		gxlog_e (KERN_NOTICE "nand_do_write_ops: Device is write protected\n");
		return -1;
	}

	realpage = (int)(to >> chip->page_shift);
	page = realpage & chip->pagemask;
	blockmask = (1 << (chip->phys_erase_shift - chip->page_shift)) - 1;

	/* Invalidate the page cache, when we write to the cached page */
	if (to <= (chip->pagebuf << chip->page_shift) && (chip->pagebuf << chip->page_shift) < (to + ops->len))
		chip->pagebuf = -1;

	/* If we're not given explicit OOB data, let it be 0xFF */
	if (likely(!oob))
		memset(chip->oob_poi, 0xff, mtd->oobsize);

	while(1) {
		//WATCHDOG_RESET();

		int bytes = mtd->writesize;
		int cached = writelen > bytes && page != blockmask;
		uint8_t *wbuf = buf;

		/* Partial page write ? */
		if (unlikely(column || writelen < (mtd->writesize - 1))) {
			cached = 0;
			bytes = min_t(int, bytes - column, (int) writelen);
			chip->pagebuf = -1;
			memset(chip->buffers->databuf, 0xff, mtd->writesize);
			memcpy(&chip->buffers->databuf[column], buf, bytes);
			wbuf = chip->buffers->databuf;
		}

		if (unlikely(oob))
			oob = nand_fill_oob(chip, oob, ops);

		ret = chip->write_page(mtd, chip, wbuf, page, cached, (ops->mode == GX_MTD_OPS_RAW));
		if (ret)
			break;

		writelen -= bytes;
		if (!writelen)
			break;

		column = 0;
		buf += bytes;
		realpage++;

		page = realpage & chip->pagemask;
		/* Check, if we cross a chip boundary */
		if (!page) {
			chipnr++;
			chip->select_chip(mtd, -1);
			chip->select_chip(mtd, chipnr);
		}
	}

	ops->retlen = ops->len - writelen;
	if (unlikely(oob))
		ops->oobretlen = ops->ooblen;
	return ret;
}

```

- 如果是调用 nfc_v1 的话，直接调用 chip->write_data(mtd, to, buf, ops->len. &writelen)，按照 page 写入flash
- 如果是调用 nfc_v0 的话，计算出 page，没有 oobbuf的情况就把 chip->oob_poi 全部置为 0xff，调用 chip->write_page
   - 发 0x01, 行地址，列地址是0，然后写 buf到 flash，这个过程会根据是否4字节对齐来做32bit或8bit传输，然后把控制器内存储的 ecc 数据读到 chip->buffers->ecccalc，然后把 chip->buffer->ecccalc 放到 chip->oob_poi 中，再把 ecc 数据写到 oob 区域
```c
static int nand_write_page(struct gx_mtd_info *mtd, struct gx_nand_chip *chip,
		const uint8_t *buf, int page, int cached, int raw)
{
	int status;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);

	if (unlikely(raw))
		chip->ecc.write_page_raw(mtd, chip, buf);
	else
		chip->ecc.write_page(mtd, chip, buf);

	/*
	 * Cached progamming disabled for now, Not sure if its worth the
	 * trouble. The speed gain is not very impressive. (2.3->2.6Mib/s)
	 */
	cached = 0;

	if (!cached || !(chip->options & NAND_CACHEPRG)) {

		chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);
		/*
		 * See if operation failed and additional status checks are
		 * available
		 */
		if ((status & NAND_STATUS_FAIL) && (chip->errstat))
			status = chip->errstat(mtd, chip, FL_WRITING, status, page);

		if (status & NAND_STATUS_FAIL)
			return -EIO;
	} else {
		chip->cmdfunc(mtd, NAND_CMD_CACHEDPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);
	}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/* Send command to read back the data */
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);

	if (chip->verify_buf(mtd, buf, mtd->writesize))
		return -EIO;
#endif
	return 0;
}


static void nand_write_page_hwecc(struct gx_mtd_info *mtd, struct gx_nand_chip *chip, const uint8_t *buf)
{
	int i = 0, eccsize = chip->ecc.size;
#ifndef CONFIG_GXNFC_NAND_HWECC
	int eccbytes = chip->ecc.bytes;
#endif
	int eccsteps = chip->ecc.steps;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	const uint8_t *p = buf;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

#ifndef CONFIG_GXNFC_NAND_HWECC
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
		chip->write_buf(mtd, p, eccsize);
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);
	}
#else
	chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
	chip->write_buf(mtd, p, (eccsize*eccsteps));
	chip->ecc.calculate(mtd, p, &ecc_calc[i]);
#endif

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	nand_fill_page_erase_mark(chip, p);
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
}

```

### program
#### gx_nand_program --> nand_write --> 只写 数据 区域，不写 oob 区域

- 要写的地址不是 page 对齐的单独调用 `nand_write_skip_bad`，先 malloc 一个 buf，然后把 buf 全部置为 0xff，把 buf 拷贝到 buf+offset 这个位置，保证写入的地址是 page 开始的位置，前面的空余是全 0xff .相当于如果我要写数据的其实地址不是 page 对齐，那么就把这些数据直接挪到 page 对齐的地方开始写。如果所在的 block 是坏块，就需要跳坏块： addr += block_size
```c
int gx_nand_pageprogram(uint32_t addr, uint8_t *data, uint32_t len)
{
	int ret = 0;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	unsigned int page_size = nand->writesize;
	int block_size = nand->erasesize;

	unsigned int page_offset = addr & (page_size - 1);
	if (page_offset) {
		unsigned int tmp_len;
		unsigned char *buf = malloc(page_size);
		while (gx_nand_block_isbad(addr)) {
			addr +=  block_size;
		}
		tmp_len = min(page_size - page_offset, len);
		memset(buf, 0xff, page_size);
		memcpy(buf + page_offset, data, tmp_len);
		ret = nand_write_skip_bad(nand, addr - page_offset, &page_size, buf, 0);
		addr += tmp_len;
		data += tmp_len;
		len -= tmp_len;
		free(buf);
	}
	if (len)
		ret = nand_write_skip_bad(nand, addr, &len, (uint8_t *)data, 0);

	return ret;
}

```

-  page 对齐的部分循环调用 `nand_write_skip_bad`，函数里面就是在做循环调用 `nand_write`，每次写 一个block
```c
int nand_write_skip_bad(struct gx_mtd_info *nand, loff_t offset, size_t *length,
		u_char *buffer, int flags)
{
	int rval = 0, blocksize;
	size_t left_to_write = *length;
	u_char *p_buffer = buffer;
	int need_skip;

	if ((offset % nand->writesize)!= 0) {
		gxlog_e("error: not page aligned addr 0x%08x!\n",offset);
		return -EINVAL;
	}

#ifdef CONFIG_CMD_NAND_YAFFS
	if (flags & WITH_YAFFS_OOB) {
		if (flags & ~WITH_YAFFS_OOB)
			return -EINVAL;

		int pages;
		pages = nand->erasesize / nand->writesize;
		blocksize = (pages * nand->oobsize) + nand->erasesize;
		if (*length % (nand->writesize + nand->oobsize)) {
			gxlog_e("Attempt to write incomplete page in yaffs mode\n");
			return -EINVAL;
		}
	} else
#endif
	{
		blocksize = nand->erasesize;
	}

	/*
	 * nand_write() handles unaligned, partial page writes.
	 *
	 * We allow length to be unaligned, for convenience in
	 * using the $filesize variable.
	 *
	 * However, starting at an unaligned offset makes the
	 * semantics of bad block skipping ambiguous (really,
	 * you should only start a block skipping access at a
	 * partition boundary).  So don't try to handle that.
	 */
	if ((offset & (nand->writesize - 1)) != 0) {
		gxlog_e("Attempt to write non page aligned data\n");
		*length = 0;
		return -EINVAL;
	}

	need_skip = check_skip_len(nand, offset, *length);
	if (need_skip < 0) {
		gxlog_e("Attempt to write outside the flash area\n");
		*length = 0;
		return -EINVAL;
	}
#if 0
	if (!need_skip && !(flags & WITH_DROP_FFS)) {
		rval = nand_write (nand, offset, length, buffer);
		if (rval == 0)
			return 0;

		*length = 0;
		gxlog_e("NAND write to offset %llx failed %d\n", offset, rval);
		return rval;
	}
#endif
	while (left_to_write > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t write_size, truncated_write_size;

		if (nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			gxlog_i("Skip bad block 0x%08x\n", offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_write < (blocksize - block_offset))
			write_size = left_to_write;
		else
			write_size = blocksize - block_offset;

#ifdef CONFIG_CMD_NAND_YAFFS
		if (flags & WITH_YAFFS_OOB) {
			int page, pages;
			size_t pagesize = nand->writesize;
			size_t pagesize_oob = pagesize + nand->oobsize;
			struct gx_mtd_oob_ops ops;

			ops.len = pagesize;
			ops.ooblen = nand->oobsize;
			ops.mode = GX_MTD_OPS_AUTO_OOB;
			ops.ooboffs = 0;

			pages = write_size / pagesize_oob;
			for (page = 0; page < pages; page++) {

				ops.datbuf = p_buffer;
				ops.oobbuf = ops.datbuf + pagesize;

				rval = nand->write_oob(nand, offset, &ops);
				if (rval == -EIO) {
					int ret = nand->block_markbad(nand, offset/nand->erasesize*nand->erasesize);
					if (ret == 0)
						gxlog_e("MTD erase write,make bad block at 0x%08x\n",
								offset/nand->erasesize*nand->erasesize);
					else
						gxlog_e("%s: MTD make bad block failed: %d\n", nand->name, ret);
					return -1;
				}
				else if (rval) {
					gxlog_e("MTD write failed !\n");
					return -1;
				}
				offset += pagesize;
				p_buffer += pagesize_oob;
			}
		}
		else
#endif
		{
			truncated_write_size = write_size;
#ifdef CONFIG_CMD_NAND_TRIMFFS
			if (flags & WITH_DROP_FFS)
				truncated_write_size = drop_ffs(nand, p_buffer,
						&write_size);
#endif
			rval = nand_write(nand, offset, &truncated_write_size, p_buffer);
			if (rval == -EIO) {
				int ret = nand->block_markbad(nand, offset/nand->erasesize*nand->erasesize);
				if (ret == 0)
					gxlog_e("MTD write failed,make bad block at 0x%08x\n", offset/nand->erasesize*nand->erasesize);
				else
					gxlog_e("%s: MTD make bad block failed: %d\n", nand->name, ret);
				return -1;
			}
			else if (rval) {
				gxlog_e("MTD write failed !\n");
				return -1;
			}
			offset += write_size;
			p_buffer += write_size;
		}

		if (rval != 0) {
			gxlog_e("NAND write to offset %llx failed %d\n", offset, rval);
			*length -= left_to_write;
			return rval;
		}
		left_to_write -= write_size;
	}
	return 0;
}

```

- `nand_write`函数：这个接口目前只写数据区，不写 oob 区域
   - `nand_do_write_ops`函数中支持同时写 数据区和 oob 区
   - nfc_v1.c 
      - 如果写 oob 区域，先调用 `nand_fill_oob`，再调用 `chip->ecc.write_oob`写 oob 区域
      - 如果写 数据 区域，先配置 ECC，再调用 `chip->write_data`写 数据 区域
   - nfc_v0.c 
      - 调用 `chip->write_page`接口，这个接口里面会写 数据 区域和 oob 区域

#### gx_nand_writeoob：用来写 oob 区域，不写 数据 区域

- `gx_nand_writeoob`：
```c
int gx_nand_writeoob(uint32_t addr, uint8_t *data, uint32_t len)
{
	int ret;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	struct gx_mtd_oob_ops ops = {
		.oobbuf = (u8 *)data,
		.ooblen = len,
		.mode = GX_MTD_OPS_RAW
	};

	ret = nand->write_oob(nand, addr, &ops);

	return ret;
}
```

#### gx_nand_writedata_noecc：用来写 数据 区域，但是不开 ecc
```c
int gx_nand_writedata_noecc(unsigned int addr, unsigned char *data, unsigned int len)
{
	int ret;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	struct gx_mtd_oob_ops ops = {
		.len    = len,
		.datbuf = (uint8_t *)data,
		.mode   = GX_MTD_OPS_RAW
	};

	ret = nand->write_oob(nand, addr, &ops);

	return ret;
}
```

### read
#### gx_nand_readdata --> nand_read_skip_bad --> nand_read，读 数据 区域，不读 数据 区域

- 如果要读的 地址 不是页对齐的，就把地址挪到这一个page的起始地址，然后读数据到 buf+page_offset 中，然后把 buf+page_offset 拷贝到真实读取的 data
```c
int gx_nand_readdata(uint32_t addr, uint8_t *data, uint32_t len)
{
	int ret = 0;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	unsigned int page_size = nand->writesize;
	int block_size = nand->erasesize;

	unsigned int page_offset = addr & (page_size - 1); // 0x3
	if (page_offset) {
		unsigned int tmp_len;
		unsigned char *buf = malloc(page_size);
		while (gx_nand_block_isbad(addr)) {
			addr +=  block_size;
		}
		tmp_len = min(page_size - page_offset, len);// tmp_len = 0x7ff - 0x3
		ret = nand_read_skip_bad(nand, addr - page_offset, &page_size, buf);// addr - page_offset = 0
		memcpy(data, buf + page_offset, tmp_len);
		addr += tmp_len;
		data += tmp_len;
		len -= tmp_len;
		free(buf);
	}
	if (len)
		ret = nand_read_skip_bad(nand, addr, &len, data);
	return ret;
}
```

- 调用 `nand_read_skip_bad`：首先检查当前所在的地址和要操作的长度内是否有坏块，如果有坏块就跳坏块，否则就调用 `nand_read` 读数据，只读 data 区域，不读 oob 区域，再调用 `nand_do_read_ops`
```c
static int nand_read(struct gx_mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uint8_t *buf)
{
	struct gx_nand_chip *chip = mtd_to_chip(mtd);
	int ret;

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	chip->ops.len = len;
	chip->ops.datbuf = buf;
	chip->ops.oobbuf = NULL;

	ret = nand_do_read_ops(mtd, from, &chip->ops);
	*retlen = chip->ops.retlen;

	return ret;
}
```

- `nand_do_read_ops`：首先根据地址计算真实的 page，计算出列地址，根据 chip->read_data 函数是否为空选用 v1 和 v0
   - v0.c：首先发一个 读命令 0x00 `chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page)`
      - 先拉低 CLE，发命令 0x00，如果命令是 READ_OOB，列地址换成 `mtd->writesize`就是page_size 最后，拉高 CLE，拉低 ALE，发行地址、发列地址，拉高 ALE，发 0x30；
      - 接下来 flash 中出来的就是数据，调用 chip->ecc.read_page，调到 `nand_read_page_hwecc`
         - 先读 erase_mark，如果全部都被 擦除了的话，就不用读 page 内的数据了，因为全是 0xff，否则读数据，
         - 发一个 0x05, 行地址是0，列地址是-1就不发列地址，发 0xE0；
         - 然后配置 ecc 为读模式，就是配 NFC
         - 调用 `chip->read_buf`真正的开始读数据，调用 nfc 的驱动，把数据从 regs->rddata_fifo 读到 buffer
         - 再 配一下 ecc 的模式
         - 调用 `chip->ecc.correct`把 ecc 的数据从 nfc 的寄存器读到 chip->oob_poi
      - 前面是读了 data 区域数据和 oob 区域数据，如果需要读 oob 区域数据，就调用 `nand_transfer_oob`把 oob 数据从 chip->oob_poi 这个位置调用 memcpy 拷贝到 oob，也就是 chip->oobbuf
      - 递增当前的 page，更新当前实际读到的 长度
   - v1.c：如果 buf 区域存在的话，根据 mode 是否为 GX_MTD_OPS_RAW 选择是否开启 ecc，再调用 chip->read_data；如果 oob 区域存在的话，调用 chip->ecc.read_oob 读到 chip->oob_poi，如果 mode 是 GX_MTD_OPS_RAW 则只从 chip->oob_poi 拷贝 oobavail 大小，否则拷贝 mtd->oobsize；
      - `chip->read_data`：计算行地址和列地址(page首地址，所以这里为0)，计算要读几个page，如果是 DMA模式，需要 cache_line 对齐，如果是 cpu 模式，需要 4 字节对齐，调用 nandflash_readpage
         - 首先配置 NFC 寄存器，根据 当前的 oobsize 配置 NFC 的 TYPE 等寄存器，写行地址到 START_ROW_ADDR 寄存器，配置 ctrl 到 CTRL_CONFIG 寄存器，然后 enable NFC 控制器，将 数据 buffer 的地址写到 DMA_START_ADDR 寄存器，将 总共要读的长度写到 XFER_TOTAL_SIZE 寄存器，将读的最后一个  page 配到 C3F_ROW_ADDR 寄存器，然后配置 1 到 XFER_START 寄存器来开始传输，使用了内部 DMA 只需要等 `idle_all、pgrn_en、bsy_timeout`或者查询中断转态；如果是 寄存器模式，还需要等 `buffer_not_full`根据 `fifo_amount_remain`的值，从 `data_register`读走或向 `data_register` 写入数据
      - `chip->ecc.read_oob`：`nand_read_oob_std`，首先发一个 NAND_CMD_READOOB，实际发送的是 0x00，发地址 也就是行地址，列地址是0，然后调用 chip->read_buf 读数据到 chip->oob_poi，这里没看到发 0x30
```c
void nandflash_read_buf(struct gx_mtd_info *mtd, uint8_t *buf, int len)
{
	struct gx_nand_dev *dev = mtd_to_dev(mtd);
	uint32_t     regs = (uint32_t)dev->regs;
	unsigned int reg_val, i;

	NAND_WRITE_REG(NAND_SEQ_RB << 4, SW_CMD_TYPE);
	NAND_WRITE_REG(0x00, SW_CMD_ADDR_LO);

	for (i = 0; i < len; i++) {
		NAND_WRITE_REG(1, SW_CMD_START);

		do {
			reg_val = NAND_READ_REG(SW_CMD_START);
		}while(reg_val & 0x01);

		buf[i] = NAND_READ_REG(STATUS_RDATA_SW);
	}
}
```

         - 常规操作流程：发指令、地址、读数据、发指令
         - `nand_transfer_oob`：刚刚从 flash 读到的数据通过 memcpy 拷贝到 ops->oobbuf
```c
static uint8_t *nand_transfer_oob(struct gx_nand_chip *chip, uint8_t *oob, struct gx_mtd_oob_ops *ops, size_t len)
{
	switch(ops->mode) {

	case GX_MTD_OPS_PLACE_OOB:
	case GX_MTD_OPS_RAW:
		memcpy(oob, chip->oob_poi + ops->ooboffs, len);
		return oob + len;

	case GX_MTD_OPS_AUTO_OOB: {
		struct gx_nand_oobfree *free = chip->ecc.layout->oobfree;
		uint32_t boffs = 0, roffs = ops->ooboffs;
		size_t bytes = 0;

		for(; free->length && len; free++, len -= bytes) {
			/* Read request not from offset 0 ? */
			if (unlikely(roffs)) {
				if (roffs >= free->length) {
					roffs -= free->length;
					continue;
				}
				boffs = free->offset + roffs;
				bytes = min_t(size_t, len, (free->length - roffs));
				roffs = 0;
			} else {
				bytes = min_t(size_t, len, free->length);
				boffs = free->offset;
			}
			memcpy(oob, chip->oob_poi + boffs, bytes);
			oob += bytes;
		}
		return oob;
	}
	default:
		GXM_BUG();
	}
	return NULL;
}
```
####  gx_nand_readoob：用来读 oob 区域，不读 数据 区域

- `gx_nand_readoob`：
```c
int gx_nand_readoob(uint32_t addr, uint8_t *data, uint32_t len)
{
	int ret;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	/* out-of-band data */
	struct gx_mtd_oob_ops ops =
	{
		.oobbuf = (u8 *)data,
		.ooblen = len,
		.mode = GX_MTD_OPS_RAW
	};

	ret = nand->read_oob(nand, addr, &ops);

	return ret;
}
```

#### gx_nand_readdata_noskip：用来读 数据 区域，不读 oob 区域，不跳坏块

- 跟上面 gx_nand_readdata 一致的，就是前面的跳坏块功能去掉了

### gx_nand_calcblockrange

- 此函数用于计算 block 的范围：首先将 起始地址换换成 block 的起始地址，然后根据 block 的起始地址转换成 具体的地址，然后将 结束地址转换成 block 的起始地址，然后根据 block 的起始地址转换成具体的地址，这样就拿到了 当前地址的长度，这个范围落在哪两个block之间，并且返回起始block和末尾block的地址
```c
void gx_nand_calcblockrange(uint32_t addr, uint32_t len, uint32_t *pstart, uint32_t *pend)
{
	uint32_t block, start_addr = addr;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);

	block = start_addr / nand->erasesize;
	*pstart = block * nand->erasesize;

	start_addr += len;
	block = start_addr / nand->erasesize;
	if (start_addr % nand->erasesize == 0)
	{
		*pend = block * nand->erasesize;
	}
	else
	{
		*pend = (block + 1) * nand->erasesize;
	}
}
```


### gx_nand_badinfo

- 此函数用于重新构建 坏块表，然后根据坏块表查看整个 flash 的每个 block 是否为 坏块
```c
int gx_nand_badinfo(void)
{
	unsigned long off;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);

	gxlog_i("\nfind bad blocks start \n");

	//rebuild bbt
	nand_default_bbt(nand);

	for (off = 0; off < nand->size; off += nand->erasesize) {
		if (gx_nand_block_isbad(off))
			gxlog_i("bad blocks:  %08lx\n", off);
	}
	gxlog_i("\nfind bad blocks end\n");

	return 0;
}
```





## chip/gx_nfc_vx.c

- 目前有有两个驱动，一个是 nfc_v0 一个是 nfc_v1，sirius 用的是 v0，canopus、vega 用的是 v1
- 看 v1 和 v0 在写数据的时候，什么时候传输的地址，只看到往 buffer 里面赋值了，没看到写地址的操作
  - 在 `nos/nand.c` 中会调用 `nand_write_skip_bad` 接口去写数据，
    - 首先会跳坏块操作，然后调用 `nand_write` 函数
      - 这边就配 `chip->ops` 配好了调用 `nand_do_write_ops` 
        - 根据 v1 或 v0 的驱动，有 oob 就写 oob，有 数据 就写 数据
          - v1：`chip->write_data` ：`gxnfc_nand_write_data` 根据是否对齐调用 `nandflash_programpage`
            - `nandflash_programpage` ：这个函数用了内部 DMA，要写哪个 page 配到 `START_ROW_ADDR` 寄存器，把要写的数据的地址配到 `DMA_START_ADDR` 寄存器，把长度赔给 `XFER_TOTAL_SIZE` 寄存器
            - 应该是这个控制器内部会自己发指令，只要按照要求把要操作的地址、长度、数据配置到寄存器就可以了
          - v0：`chip->write_page` ：`nand_write_page`  --> `chip->ecc.write_page`  --> `nand_write_page_hwecc` --> `chip->write_buf` 
            - 这个也是控制器内部自己发指令，根据传入地址确定 page 页地址，然后向 flash 发送 执行和地址

- v1 和 v0 在读 oob 的时候，控制器接口都已经实现了 `chip->ecc.read_oob` 接口
  - v1：使用批量读取的方式
  - v0：使用发命令的方式


### gx_nfc_v1.c

#### gxnfc_nand_inithw(info)

- 此函数用于做初始化，配一些 flash 相关的时间
```c
static int gxnfc_nand_inithw(struct gx_nand_dev *dev)
{
	uint32_t     regs = (uint32_t)dev->regs;
	unsigned int reg_val;

	reg_val = NAND_READ_REG(ENABLE_STATUS);
	NAND_WRITE_REG(reg_val & (~0x01), ENABLE_STATUS);

	NAND_WRITE_REG(0x3f << 24, ECC_ENABLE_STATUS);

	NAND_WRITE_REG((3<<16) | 2, TWP_TWH);            // twp>12ns  twh>10ns
	NAND_WRITE_REG((10<<16) | 6, TWB_TRR);           // twb<100ns  trr>20ns
	NAND_WRITE_REG((5<<16) | 4, TRP_TREH);
	NAND_WRITE_REG((3<<16) | 2, TRP_TREH_FAST);      // trp>12ns  treh>10ns
	NAND_WRITE_REG((0<<24) | 2, SMP_DLY_NEG_FAST);
	NAND_WRITE_REG(20, TRWAD);                       // trwad>100ns
	NAND_WRITE_REG(0, INT_ENABLE);
	/* enable NAND Controller */
	NAND_WRITE_REG(reg_val | 0x01, ENABLE_STATUS);
	/* reset nand flash */
	write_cmd(dev, 0xff);
	gxnfc_nand_dev_ready(&dev->mtd);

	return 0;
}
```

#### chip->write_buf = nandflash_write_buf

- 配置 nfc 控制器 写一个数据
- 把数据发送到 寄存器中，此寄存器功能：软件发送的 Flash 命令的地址、数据和命令字
- 写 1 表示发送一个 Flash 命令，读 1 表示上一个命令还未完成
```c
void nandflash_write_buf(struct gx_mtd_info *mtd, const uint8_t *buf, int len)
{
	struct gx_nand_dev *dev = mtd_to_dev(mtd);
	uint32_t     regs = (uint32_t)dev->regs;
	unsigned int reg_val, i;

	NAND_WRITE_REG(NAND_SEQ_WB << 4, SW_CMD_TYPE);

	for (i = 0; i < len; i++) {
		NAND_WRITE_REG(buf[i], SW_CMD_ADDR_LO);
		NAND_WRITE_REG(1, SW_CMD_START);

		do {
			reg_val = NAND_READ_REG(SW_CMD_START);
		}while(reg_val & 0x01);
	}
}
```

#### chip->read_buf = nandflash_read_buf

- 配置 nfc 控制器 读一个数据
- 发个 0x00 到寄存器：SW_CMD_ADDR_LO
- 写 1 表示发送一个 Flash 命令，读 1 表示上一个命令还未完成
- 从 寄存器中读到 buf
```c
void nandflash_read_buf(struct gx_mtd_info *mtd, uint8_t *buf, int len)
{
	struct gx_nand_dev *dev = mtd_to_dev(mtd);
	uint32_t     regs = (uint32_t)dev->regs;
	unsigned int reg_val, i;

	NAND_WRITE_REG(NAND_SEQ_RB << 4, SW_CMD_TYPE);
	NAND_WRITE_REG(0x00, SW_CMD_ADDR_LO);

	for (i = 0; i < len; i++) {
		NAND_WRITE_REG(1, SW_CMD_START);

		do {
			reg_val = NAND_READ_REG(SW_CMD_START);
		}while(reg_val & 0x01);

		buf[i] = NAND_READ_REG(STATUS_RDATA_SW);
	}
}

```

#### chip->cmd_ctrl = gxnfc_nand_hwcontrol

- 空的

#### chip->dev_ready = gxnfc_nand_dev_ready
```c
static int gxnfc_nand_dev_ready(struct gx_mtd_info *mtd)
{
	struct gx_nand_dev  *dev  = mtd_to_dev(mtd);
	uint32_t             regs = (uint32_t)dev->regs;
	unsigned int reg_val;

	NAND_WRITE_REG(NAND_SEQ_WRD<< 4, SW_CMD_TYPE);
	NAND_WRITE_REG(0x00, SW_CMD_ADDR_LO);
	NAND_WRITE_REG(1, SW_CMD_START);

	do {
		reg_val = NAND_READ_REG(SW_CMD_START);
	}while(reg_val & 0x01);

	return 1;
}
```


#### chip->cmdfunc = gxnfc_nand_command

- 此函数主要用于发命令0x00、行地址、列地址、发命令 0x30
   - 按照 flash 的要求发送好命令和地址，后面 Flash 出来的都是 buffer，用上面的
- 此函数里面主要用于 发读命令
- 写数据用的是 `chip->write_data`

#### chip->waitfunc = gxnfc_nand_wait_ready

- 用于向 flash 发命令，0x70 或 0x71 来读状态寄存器，然后调用 gxnfc_nand_read_bye 读一个字节的数据，让 flash 不在 busy 状态
```c
static int gxnfc_nand_wait_ready(struct gx_mtd_info *mtd, struct gx_nand_chip *chip)
{
	int state = chip->state;

	gxnfc_nand_dev_ready(mtd);
	if ((state == FL_ERASING) && (chip->options & NAND_IS_AND))
		chip->cmdfunc(mtd, NAND_CMD_STATUS_MULTI, -1, -1); /* 0x71 */
	else
		chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1); /* 0x70 */

	if (gxnfc_nand_read_byte(mtd) & 0x1) {
		return NAND_STATUS_FAIL;
	}
	return 0;
}
```

#### chip->read_byte = gxnfc_nand_read_byte

- 用于读取一个字节，发指令和地址给flash 的操作在别的函数中做好了，这里读的就是 Flash 出来的 buffer
```c
void nandflash_read_buf(struct gx_mtd_info *mtd, uint8_t *buf, int len)
{
	struct gx_nand_dev *dev = mtd_to_dev(mtd);
	uint32_t     regs = (uint32_t)dev->regs;
	unsigned int reg_val, i;

	NAND_WRITE_REG(NAND_SEQ_RB << 4, SW_CMD_TYPE);
	NAND_WRITE_REG(0x00, SW_CMD_ADDR_LO);

	for (i = 0; i < len; i++) {
		NAND_WRITE_REG(1, SW_CMD_START);

		do {
			reg_val = NAND_READ_REG(SW_CMD_START);
		}while(reg_val & 0x01);

		buf[i] = NAND_READ_REG(STATUS_RDATA_SW);
	}
}
```


#### chip->read_data = gxnfc_nand_read_data

- 计算当前的 addr 是否页对齐，若不对齐，则调用 nandflash_readpage 读一整个 page 的数据到 chip->buffers->databuf，然后用 memcpy 将 readbuf + col 开始的数据拷贝到真实要读的 data 中
   - 若对齐，计算要读多少个 page，以及这么多个page 对应的长度
   - 计算当前要读到的数据地址是否 4字节对齐或 cache_line 对齐
      - 如果不对齐，则在 for 循环中循环调用 nandflash_readpage ，每次读一个 page
      - 如果对齐，则直接调用 nandflash_readpage 读所有的长度
   - 如果最后还有没读完的，在调用  nandflash_readpage 读一个 page，在 memcpy 到真实的 buffer
```c
static int gxnfc_nand_read_data(struct gx_mtd_info *mtd, unsigned int from, unsigned char *data, unsigned int len, unsigned int* readlen)
{
	struct gx_nand_dev *dev = mtd_to_dev(mtd);
	uint8_t *readbuf = NULL;
	int page_size, i, ret = 0;
	unsigned int row, col, remain;
	int leading, major, major_rows, trailing;
	struct gx_nand_chip *chip = &dev->chip;

	*readlen = 0;
	page_size = mtd->writesize;
	// from = 0x03 len = 0x3000
	row = from / page_size; // 0
	col = from % page_size; // 0x03
	remain = page_size - col; // 0x7fd
	leading = min(remain, len); // 0x7fd
	if (leading != page_size) {
		readbuf = chip->buffers->databuf;
		ret = nandflash_readpage(mtd, row, readbuf, page_size);
		// 0 + 0x03 --> data
		memcpy(data, readbuf + col, leading);
		from += leading; // 0x800
		data += leading; // update point
		len  -= leading; // len = 0x3000 - 0x7fd
		*readlen += leading;
		row++;          // next page
		if(ret < 0)
			goto final;
	}

	// major_rows = 0x2803 / 0x8000 = number of page
	// major = number of page * page_size  --> len
	major_rows = len / page_size;
	major = major_rows * page_size;
	if (major) {
#if NAND_DMA_ENABLE
#ifdef CONFIG_GX_LINUX
		/* 控制器要求4字节对齐, Linux系统cache line对齐 */
		if (IS_ALIGNED((unsigned int)data, ARCH_DMA_MINALIGN)) {
#else
		/* dma模式要求4字节对齐，gxloader要求cache line对齐 */
		if (((uint32_t)data % CACHE_LINE_SIZE) == 0x00) {
#endif
#else
		/* cpu模式要求4字节对齐 */
		if (((uint32_t)data & 0x3) == 0x00) {
#endif
			ret = nandflash_readpage(mtd, row, data, major);
			data += major;
			row  += major_rows;
			from += major;
			len  -= major;
			*readlen += major;
			if(ret < 0)
				goto final;
		} else {
			readbuf = chip->buffers->databuf;
			for (i = 0; i < major_rows; i++) {
				ret = nandflash_readpage(mtd, row, readbuf, page_size);
				memcpy(data, readbuf, page_size);
				data += page_size;
				*readlen += page_size;
				row++;
				if(ret < 0)
					goto final;
			}
			from += major;
			len  -= major;
		}
	}

	trailing = len;
	if (trailing) {
		readbuf = chip->buffers->databuf;
		ret = nandflash_readpage(mtd, row, readbuf, page_size);
		memcpy(data, readbuf, trailing);
		from += trailing;
		len  -= trailing;
		*readlen += trailing;
		if(ret < 0)
			goto final;
	}

final:
	return ret;
}
```

#### chip->write_data = gxnfc_write_data

- 计算当前要读的地址的行地址和列地址
- 如果 page 不对齐，则先拷贝到临时 buffer，writebuf = chip->buffers->databuf，先 memset 这片数据 page_size，然后调用 memcpy 把真实的数据拷贝到 writebuf + col 也就是真实对应到 page 的地方，不对齐的部分全部是 0xff，调用 nandflash_programpage 将临时buffer 写到flash，写一个page，相当于是前面部分是 0xff，写到 flash 没关系，后面是真实的数据
- 如果 page 对齐，则再 计算要写的地址是否 字节对齐、cache_line 对齐
   - 如果对齐，则直接调用 nandflash_programpage 将所有要写的数据都写过去
   - 如果不对齐，则先将要写的数据拷贝到 临时buffer，然后调用 nandflash_programpage 写临时buffer到 flash，每次写一个 page，循环写每个 page
- 如果后面还有地址不对齐的，还是找临时buffer，memset 为 0xff，然后将 数据拷贝到临时buffer，调用 nandflash_programpage 将数据写到 flash，写一个 page
```c
static int gxnfc_nand_write_data(struct gx_mtd_info *mtd, unsigned int to, unsigned char *data, unsigned int len, unsigned int* writelen)
{
	struct gx_nand_dev *dev = mtd_to_dev(mtd);
	uint8_t *writebuf = NULL;
	int page_size, ret = 0, i;
	unsigned int row, col, remain;
	int leading, major, major_rows, trailing;
	struct gx_nand_chip *chip = &dev->chip;

	*writelen = 0;
	page_size = mtd->writesize;
	row = to / page_size;
	col = to % page_size;
	remain = page_size - col;
	leading = min(remain, len);
	if (leading != page_size) {
		writebuf = chip->buffers->databuf;
		memset(writebuf, 0xff, page_size);
		memcpy(writebuf + col, data, leading);
		ret = nandflash_programpage(mtd, row, writebuf, page_size);
		to += leading;
		data += leading;
		len  -= leading;
		*writelen += leading;
		row++;
		if(ret < 0)
			goto final;
	}

	major_rows = len / page_size;
	major = major_rows * page_size;
	if (major) {
#if NAND_DMA_ENABLE
#ifdef CONFIG_GX_LINUX
		/* 控制器要求4字节对齐, Linux系统cache line对齐 */
		if (IS_ALIGNED((unsigned int)data, ARCH_DMA_MINALIGN)) {
#else
		/* dma模式要求4字节对齐，gxloader要求cache line对齐 */
		if (((uint32_t)data % CACHE_LINE_SIZE) == 0x00) {
#endif
#else
		/* cpu模式要求4字节对齐 */
		if (((uint32_t)data & 0x3) == 0x00) {
#endif
		ret = nandflash_programpage(mtd, row, data, major);
			data += major;
			row  += major_rows;
			to   += major;
			len  -= major;
			*writelen += major;
			if(ret < 0)
				goto final;
		} else {
			writebuf = chip->buffers->databuf;
			for (i = 0; i < major_rows; i++) {
				memcpy(writebuf, data, page_size);
				ret = nandflash_programpage(mtd, row, writebuf, page_size);
				data += page_size;
				*writelen += page_size;
				row++;
				if(ret < 0)
					goto final;
			}
			to   += major;
			len  -= major;
		}
	}

	trailing = len;
	if (trailing) {
		writebuf = chip->buffers->databuf;
		memset(writebuf, 0xff, page_size);
		memcpy(writebuf, data, trailing);
		ret = nandflash_programpage(mtd, row, writebuf, page_size);
		to += trailing;
		data += trailing;
		len -= trailing;
		*writelen += trailing;
		if(ret < 0)
			goto final;
	}

final:
	return ret;
}
```

#### chip->ecc.calculate = gxnfc_nand_calculate_ecc

- 空


#### chip->ecc.correct = NULL


#### chip->ecc.mode = NAND_ECC_HW


#### chip->ecc.size = GX_NAND_512_PAGESIZE



#### chip->ecc.hwctl = gxnfc_nand_enable_hwecc

- 此函数用于配置 ecc 的能力到 nfc 寄存器
```c
static void gxnfc_nand_enable_hwecc(struct gx_mtd_info *mtd, int mode)
{
	unsigned int reg_val;
	struct gx_nand_dev *dev = mtd_to_dev(mtd);
	uint32_t     regs = (uint32_t)dev->regs;

	/* disable NAND Controller */
	reg_val = NAND_READ_REG(ENABLE_STATUS);
	NAND_WRITE_REG(reg_val & (~0x01), ENABLE_STATUS);

	reg_val = NAND_READ_REG(CTRL_CONFIG);
	if (mode == NAND_ECC_WRITE) {
		ecc_enable_flag = 1;
		reg_val |= (1 << NAND_CFG_MODULE_OFFSET_ECC);
		reg_val |= (mtd->ecc_strength << NAND_CFG_MODULE_OFFSET_ECC_TYPE);
	} else if (mode == NAND_ECC_READ) {
		ecc_enable_flag = 1;
		reg_val |= (1 << NAND_CFG_MODULE_OFFSET_ECC);
		reg_val |= (mtd->ecc_strength << NAND_CFG_MODULE_OFFSET_ECC_TYPE);
	} else if (mode == NAND_ECC_DISABLE) {
		ecc_enable_flag = 0;
		reg_val &= ~(1 << NAND_CFG_MODULE_OFFSET_ECC);
	} else if (mode == NAND_ECC_WAIT_READY) {
	}
	NAND_WRITE_REG(reg_val, CTRL_CONFIG);

	/* enable NAND Controller */
	reg_val = NAND_READ_REG(ENABLE_STATUS);
	NAND_WRITE_REG(reg_val | 0x01, ENABLE_STATUS);

	return;
}
```


#### chip->ecc.read_oob = gxnfc_nand_read_oob

- 此函数用于读取 oob 区域，往 寄存器写 0x3f 复位 ecc 最大错误数，写列地址和后续page的列地址到 TYPE0_START_ADDR 寄存器，配置数据类型 0 的每一块长度为 oobsize，后面的数据类型都没有，配置 结束列地址为 page_size + mtd->oobsize - 1 就是要读 oobsize 大小，配起始 行地址，配 CTRL_CONFIG 寄存器，使能控制器，配 mtd->oobsize 到 XFER_TOTAL_SIZE 一批传输的总长度，等控制器 notbusy，写 1 到 XFER_START 寄存器开始传输，循环读 数据寄存器
```c
static int gxnfc_nand_read_oob(struct gx_mtd_info *mtd, struct gx_nand_chip *chip, int page, int sndcmd)
{
	unsigned int reg_val;
	struct gx_nand_dev *dev = mtd_to_dev(mtd);
	int fifo_amount_remain, i = 0, minor;
	int page_size = mtd->writesize;
	uint32_t     regs = (uint32_t)dev->regs;
	int len = mtd->oobsize;

	/* disable NAND Controller */
	reg_val = NAND_READ_REG(ENABLE_STATUS);
	NAND_WRITE_REG(reg_val & (~0x01), ENABLE_STATUS);
	NAND_WRITE_REG(0x00, INT_ENABLE);
	NAND_WRITE_REG(0x3f << 24, ECC_ENABLE_STATUS);

	NAND_WRITE_REG((page_size<<16) | (page_size), TYPE0_START_ADDR);
	NAND_WRITE_REG(mtd->oobsize, TYPE0_LENGTH);
	NAND_WRITE_REG(0, TYPE0_GAP);
	NAND_WRITE_REG(0, TYPE1_LENGTH);
	NAND_WRITE_REG(0, TYPE2_LENGTH);
	NAND_WRITE_REG(0xffff, END_TYPE012_ADDR);
	NAND_WRITE_REG(0xffff, TYPE3_RELOAD_COL_ADDR);
	NAND_WRITE_REG(page_size + mtd->oobsize - 1, END_COL_ADDR);

	NAND_WRITE_REG(page, START_ROW_ADDR);

	NAND_WRITE_REG((0<< 28) | (4<<16) | (8<<8) | (0<<5) | (0<<4) | (0<<1), CTRL_CONFIG);

	NAND_WRITE_REG(0, BAD_BLOCK_MARK_0);
	NAND_WRITE_REG(0, BAD_BLOCK_MARK_1);
	/* enable NAND Controller */
	NAND_WRITE_REG(reg_val | 0x01, ENABLE_STATUS);
	NAND_WRITE_REG(mtd->oobsize, XFER_TOTAL_SIZE);

	do {
		reg_val = NAND_READ_REG(INT_STATUS);
	} while(!(reg_val & 0x0100));

	NAND_WRITE_REG(1, XFER_START);

	while (len) {
		reg_val = NAND_READ_REG(ENABLE_STATUS);
		if (reg_val & (1<<4)) {
			fifo_amount_remain = (NAND_READ_REG(ENABLE_STATUS) >> 16) & 0x3ff;
			minor = min(fifo_amount_remain, len);

			while (minor--)
			{
				((uint8_t*)chip->oob_poi)[i] = NAND_READ_REG(DATA_REGISTER);
				i++;
				len--;
			}
		}
	}

	do {
		reg_val = NAND_READ_REG(INT_STATUS);
	} while(!(reg_val & 0x01));

	return 0;
}
```


#### chip->ecc.config = gxnfc_nand_ecc_config_v1

- 此函数用于根据 ecc/layout->oobfree 区域的排布，来计算 对应的  eccpos 
```c
static int gxnfc_nand_ecc_config_v1(struct gx_mtd_info *mtd)
{
	struct gx_nand_chip *chip = mtd_to_chip(mtd);
	mtd->max_dma_len = 0x10000000;

	if (!chip->ecc.layout) {
		switch (mtd->oobsize) {
		case 64:
			chip->ecc.layout = &nand_oob_64;
			chip->ecc.bytes = NAND_4BIT_ECC_SIZE_PER_SECTOR;
			mtd->ecc_strength = 4;
			break;
		case 112:
			chip->ecc.layout = &nand_oob_112;
			chip->ecc.bytes = NAND_8BIT_ECC_SIZE_PER_SECTOR;
			mtd->ecc_strength = 8;
			break;
		case 128:
			chip->ecc.layout = &nand_oob_128;
			chip->ecc.bytes = NAND_8BIT_ECC_SIZE_PER_SECTOR;
			mtd->ecc_strength = 8;
			break;
		case 256:
			chip->ecc.layout = &nand_oob_256;
			chip->ecc.bytes = NAND_16BIT_ECC_SIZE_PER_SECTOR;
			mtd->ecc_strength = 16;
			break;
		default:
			gxlog_e(KERN_WARNING "No oob scheme defined for oobsize %d\n", mtd->oobsize);
		}
		gxnfc_nand_fill_eccpos(mtd);
	}

	return 0;
}


static int gxnfc_nand_fill_eccpos(struct gx_mtd_info *mtd)
{
	int i = 0, j = 0, flag = 0;
	struct gx_nand_chip *chip = mtd_to_chip(mtd);
	struct nand_ecclayout *layout = chip->ecc.layout;
	if(layout == NULL)
		return 0;

	uint32_t* eccpos = layout->eccpos;

	for(i = 2; i < mtd->oobsize; i++) {
		flag = 0;
		struct gx_nand_oobfree *free = chip->ecc.layout->oobfree;
		for(j = 0; free->length > 0 && j < MTD_MAX_OOBFREE_ENTRIES; j++, free++) {
			if(i >= free->offset && i < (free->offset + free->length)) {
				flag = 1;
				break;
			}
		}
		if(flag == 0)
			*eccpos++ = i;
	}
	return 0;
}
```


### gx_nfc_v0.c
#### gxnfc_nand_inithw(info)

- 此函数用于配置控制器进行初始化，进行一些复位操作和关闭控制器
```c
static int gxnfc_nand_inithw(struct gx_nand_dev *dev)
{
	struct gxnfc_nand_regs __iomem *regs = dev->regs;

	regs->cfg_module = 0x00010000;//enable nand module
	regs->cfg_module =  0x00010f1f;

	regs->timing_sequence = TIME_PARAMS_8;
	regs->cfg_dma_waittime = 5400000;

	/* It should be reset when flash is started
	* at first time.
	*/

	regs->sel |= 1 << 0; //select chip
	write_cmd(dev,NAND_CMD_RESET);
	wait_nand_ready(regs);
	NAND_DIS(regs);
	return 0;
}
```

#### chip->write_buf = gxnfc_nand_write_buf

- 此函数的实质是封装 write_data_bulk 的一层，实际的写数据不在此函数，调用完 write_data_bulk 之后等控制器的发送完成标志位，然后清掉这个标志位
- 先清掉 FIFO，往 cfg_dma_ctrl 寄存器写 1<<28
- 配置时间序列为固定的参数
- 将要写的数据长度写入到 控制器的 regs->status_reg2 = (len << 16)
- 需要判断地址和长度是否4字节对齐，如果地址对齐的话，就用 4 字节的方式去读写数据；如果地址不对齐的话，就用 1 字节的方式去读写数据
```c
static inline void write_data_bulk(struct gx_nand_dev *dev, const u8 *dp, size_t n)
{
	struct gxnfc_nand_regs __iomem *regs = dev->regs;
	u32 tmp;
	u8 *p;
	u32 *ip;
	u32 i;
	u32 res = 0;
	u32 k = 0;
	u8 *res_buf = NULL;

	FIFO_CLEAR(regs);
	regs->timing_sequence = TIME_PARAMS;
	regs->status_reg2 = (n<<16);

	// n = 0x802  res = 2
	res = n % 4;// 末尾长度不对齐部分
	// k = 0x802 - 2
	k = n - res;// 末尾长度对齐部分

	// 地址对齐
	if (likely(((u32)dp&3)==0)) {
		ip = (u32 *)dp;
		// 对齐部分，直接把数据 4字节 往 regs->wrdata_fifo 寄存器灌
		// 这里写的是头部数据，前面所有长度对齐的部分
		for(i = 0; i < k; i+=4) {
			while(regs->status_reg1 & (1<<16));       //wait for fifo not full (0x18.16=0)
			regs->wrdata_fifo = *(ip++);
		}

		// 尾部存在不对齐的部分，构建一个 4字节 数据，然后一次写进去，空的字节为0
		if (res) {
			res_buf = (u8 *)ip;
			while(regs->status_reg1 & (1<<16));       //wait for fifo not full (0x18.16=0)
			tmp = 0;
			for(i = 0; i < res; i++) {
				tmp |= (*(res_buf + i)) << (i * 8);
			}
			regs->wrdata_fifo = tmp;
		}
	// 地址不对齐，cpu 就不能 4 字节的方式去内存里拿数据
	} else {
		p= (u8 *)dp;
		// 长度对齐部分
		for(i = 0; i < k; i+=4) {
			while(regs->status_reg1 & (1<<16));       //wait for fifo not full (0x18.16=0)
			tmp = 0;
			tmp |= *(p++)<<0;
			tmp |= *(p++)<<8;
			tmp |= *(p++)<<16;
			tmp |= *(p++)<<24;
			regs->wrdata_fifo = tmp;
		}

		// 长度不对齐部分
		if (res) {
			res_buf = (u8 *)p;
			while(regs->status_reg1 & (1<<16));       //wait for fifo not full (0x18.16=0)
			tmp = 0;
			for(i = 0; i < res; i++) {
				tmp |= (*(res_buf + i)) << (i * 8);
			}
			regs->wrdata_fifo = tmp;
		}
	}
}
```

#### chip->read_buf = gxnfc_nand_read_buf

- 此函数用于从控制器的 fifo 读取要求长度的数据到 buf
- 具体的读操作在 read_data_bulk 函数中，此函数只是一个封装层，没有做任何操作
- 配置好时间序列
- 如果要读的长度不是 4 字节对齐的，直接退出
- 如果要读的地址是四字节对齐，就用 4 字节的方式从 fifo 读数据到内存
- 如果要读的地址不是四字节对齐，就用 1 字节的方式从 fifo 读数据到内存，注意 fifo 是32位的，每次都是从fifo读 4 字节，然后依次用 1 字节的方式写到内存的
- 从 fifo 读之前会先等待 fifo 满，这个等待是会有 timeout 的
- 读完之后会等 ready，也是会有 timeout
```c
static inline void read_data_bulk(struct gx_nand_dev *dev, u8 *dp, size_t n)
{
	struct gxnfc_nand_regs __iomem *regs = dev->regs;
	u32 tmp;
	u32 *ip;
	u8 *p;
	u32 i;

	regs->timing_sequence = TIME_PARAMS;
	/* Most of the time, we expect to be dealing with word-aligned
	* multiples of 4 bytes, so optimise for that case. */
	if (unlikely(n%4)) {
		gxlog_e("%s error: data num %d is not word-aligned\n",__FUNCTION__,n);
		return;
	}
	// 地址四字节对齐
	if (likely(((u32)dp&3)==0)) {
		ip = (u32 *)dp;
		regs->ctl_receive = (n << 16)|1;
		for(i = 0; i < n; i+=4) {
			wait_fifo_full(dev);
			*(ip++) = regs->rddata_fifo;
		}
		wait_receive_polled(regs);
	}
	else {
		p = (u8 *)dp;
		regs->ctl_receive = (n << 16)|1;
		for(i = 0; i < n; i+=4) {
			wait_fifo_full(dev);
			tmp = regs->rddata_fifo;
			*(p++) = (tmp>>0)&0xff;
			*(p++) = (tmp>>8)&0xff;
			*(p++) = (tmp>>16)&0xff;
			*(p++) = (tmp>>24)&0xff;
		}
		wait_receive_polled(regs);
	}
}
```

#### chip->select_chip = gxnfc_nand_select_chip

- 此函数用于使能或失能控制器
```c
static void gxnfc_nand_select_chip(struct gx_mtd_info *mtd, int chip)
{
	struct gx_nand_dev *dev = mtd_to_dev(mtd);
	struct gxnfc_nand_regs __iomem *regs = dev->regs;

	if (chip == -1){
		NAND_DIS(regs); //De-select chip
	}
	else{
		NAND_EN(regs); //select chip
	}
}
```

#### chip->IO_ADDR_W = (void __iomem *) &dev_to_regs(info)->wrdata_fifo

- 这里的 __iomem 为空

#### chip->IO_ADDR_R = (void __iomem*)&dev_to_regs(info)->rddata_fifo


#### chip->cmd_ctrl = gxnfc_nand_hwcontrol

- 空

#### chip->dev_ready = gxnfc_nand_dev_ready

- 此函数用于等待控制器 ready
```c
static int gxnfc_nand_dev_ready(struct gx_mtd_info *mtd)
{
	uint32_t status;
	struct gx_nand_dev *dev = mtd_to_dev(mtd);
	struct gxnfc_nand_regs __iomem *regs = dev->regs;

	status = (regs->status_reg1 >> 8) & 0x01;

	return status;
}
```


#### chip->cmdfunc = gxnfc_nand_command

- 此函数用于发一些命令，主要是读
- 先使能控制器
- 拉低 CLE，表示发命令
- 根据 pagesize 改变列地址，如果是 2k page 直接发 0x0，如果要发的命令是 读OOB，则 column = mtd->writesize
- 拉高 CLE
- 如果要发行地址和列地址
- 先拉低 ALE
- 写列地址
- 写行地址
- 拉高ALE


#### chip->waitfunc = gxnfc_nand_wait_ready

- 此函数只用作写操作
- 先等控制器 ready
- 发 0x70 给 flash，读状态寄存器
- 等 flash ready


#### chip->read_byte = gxnfc_nand_ready_byte

- 此函数用于读一个字节
- 先配特殊的时间序列
- 配接收控制寄存器单独的值
- 等 fifo 满
- 直接返回 regs->rddata_fifo


#### chip->ecc.calculate = gxnfc_nand_calculate_ecc

- 此函数用于读控制器的 ecc 值
- 不支持 小 pagesize(256, 512)
- 等 ready
- 根据当前 flash oob 区域中多少字节数用于存放 ecc，来从控制器中读 ecc 码然后存起来
- 然后根据数据和读到的ecc码，做一个操作：当数据为全0xff时，让ecc码也为全0xff


#### chip->ecc.correct = gxnfc_nand_correct_data

- 此函数用于根据控制器的状态和 ecc 码纠正 data
- 首先根据参数确定是哪一个 sector，这个sector对应的ecc状态从regs->ecc_status 中读出来
- 如果没有 ecc 错误，直接退出，如果错误太多无法纠错，也直接退出
- 然后计算错误数量
- 从寄存器中获取当前的错误位置
- 然后根据错误的位置和错误数量纠错数据


#### chip->ecc.mode = NAND_ECC_HW


#### chip->ecc.size = GX_NAND_512_PAGE_SIZE


#### chip->ecc.hwctl = gxnfc_nand_enable_hwecc

- 此函数用于配置regs->cfg_module  regs->cfg_ecc 控制器


#### chip->ecc.read_oob = gxnfc_nand_read_oob

- 此函数用于读 oob 区域
- 先清掉状态寄存器
- 如果 sndcmd 为1，先执行 chip->cmdfunc(func, NAND_CMD_READOOB, 0, page)
- 清掉 FIFO
- 配 ecc
- 配接收寄存器
- 等fifo 满
- 4字节的方式从 fifo 读数据到 buffer
- 等 ready
- 将 buffer 中的数据 memcpy 到 chip->oob_poi，拷贝 mtd->oobsize

#### chip->ecc.config = gxnfc_nand_ecc_config_v0

- 此函数用于根据 oobsize 配置一些 ecc 相关的变量的值
- 配 oob 区域的排布、每个 sector 几bit ecc，ecc能力


## nand_base.c

​	首先执行的就是 `nand_scan()`函数，看一下实现：

```c
int nand_scan(struct gx_mtd_info *mtd, int maxchips)
{
	int ret;

	ret = nand_scan_ident(mtd, maxchips, NULL);
	if (!ret)
		ret = nand_scan_tail(mtd);
	return ret;
}
```

- nand_scan：这个函数会在初始化的时候调用到

  - 配 bus_width 也就是有几根数据线
  - 配一些默认的函数 `nand_set_defaults(chip, busw)` 
    - 如果 控制器中没有实现这些接口，就在这里配置
  - 读取 flash 的id
    - 调用 `chip->cmdfunc` 来发读 ID 命令给 flash
    - 调用 `chip->read_byte` 两次，来读两个字节的 ID
    - 上面的操作会做两次，比较两次读到的 ID 是否相同，若相位没调好，可能出现两次读到的 ID 不同
    - 调用 `nand_flash_detect_onfi` 函数通过 onfi 获取 flash 的信息，如果 flash 支持 onfi 标准的话，就用 onfi 获取 flash 的信息；如果不支持的话，就用发 指令的方式去拿
    - 配一些 chip-> 信息到这个结构中

- nand_scan_tail：

  - `chip->oob_poi = chip->buffers->databuf + mtd->writesize` 
  - 调用 控制器的 ecc 配置函数
  - 如果控制器没提供写 page 接口，这里提供一个写 page 接口：`nand_write_page`
  - 根据 chip->ecc.mode 配一些函数指针，默认使用的模式是 `NAND_ECC_HW` 也就是控制器内部 ecc
  - 配置 `chip->ecc.steps = mtd->writesize / chip->ecc.size` 这里一般都是 4
  - 配置 page 擦除标记放在 ecc 数据的前面：`chip->oob_poi + mtd->oob_size - chip->ecc.total - chip->ecc.steps` 
  - 填充 mtd 信息
  - 建立坏块表：`chip->scan_bbt` 

  初始化过程结束，后续用户通过调用 nand.c 中的接口来调用到 nand_base.c 中的文件，后面查看各个接口的功能。

- nand_base.c 中由 Linux 提供了一套接口，用户只需要配置好 chip 结构体的信息就可以了，并且提供了用户自定义 chip 函数的方法，如果用户没自定义自己的控制器功能函数，那么就使用 Linux 提供的函数
- nand_base.c 中有很多时候函数跟 控制器驱动里面的接口类似



#### nand_read_page_hwecc：nfc_v0 使用

- 循环检测每个 eccsteps，如果 检测 chip->page_erase_mask[sector] 里面的数据是不是 0xff，即使其中存在 5 个 1，也认为擦除标记为 true，那么代表 sector 擦除标记为 true，否则擦除标记为 false，需要读数据
  - 擦除标记是在写 page 的时候操作的，如果 ecc 的数据是全 0xff，那么标记 `chip->page_erase_mask[sector] = 0xff`
- 如果 sector 擦除标记全为 true，不用读页数据
- 循环 eccsteps ，如果 擦除标记为 true，那么将数据置为 0xff，然后通过 ecc 数据和 main 数据进行校验，看是否有 位翻转



#### nand_transfer_oob

​	此函数用于将 `chip->oob_poi` 位置的 oob 数据拷贝到 `ops->oob_buf`，存在三种模式：

- GX_MTD_OPS_PLACE_OOB：oob 数据放置在给定的偏移量处
- GX_MTD_OPS_AUTO_OOB：oob 数据根据 ecclayout 定义的空闲区域自动放置 oob 数据
- GX_MTD_OPS_RAW：此模式读取一大块 main+oob，oob 数据被插入到数据中，这就是 flash 中的原始数据
  - 此模式一般都是用于关闭掉 ecc，再读 oob 区域



#### nand_do_read_ops

​	此函数会覆盖 nfc_v1 和 nfc_v0 两种控制器的读接口，根据 `ops->oobbuf、ops->datbuf` 若有定义，则读取对应的数据到这些 buf 中去

- v1：命令、地址 都是控制器来发的，不需要手动发
  
  > 要操作的数据是否页对齐，在 控制器的驱动里做处理
  >
  > 要操作的数据是否 4 字节对齐，也在操作器的驱动里做处理
  >
  > 都是在 `gxnfc_nand_read_data` 接口处理的
  
  - ops->datbuf
    - 如果模式是 GX_MTD_OPS_RAW，就手动关掉 ecc
    - 调用 `chip->read_data` --> `gxnfc_nand_read_data` ，根据要读的地址是否 page 对齐选择在调用 `nandflash_readpage` 之前是否先读到临时 buffer
      - `nandflash_readpage` 配控制器，配 ecc 相关的寄存器、把要读的 page 配到 `START_ROW_ADDR` 寄存器、把要读到的地址配到 `DMA_START_ADDR` 寄存器、把 要读的 len 配到 `XFER_TOTAL_SIZE` 寄存器，配 1 到 `XFER_START` 寄存器启动传输，等 dma 传输完，并且看状态寄存器是否有 位翻转
  - ops->oobbuf
    - 调用 `chip->ecc.read_oob` 接口可以直接读 oob 数据，控制器实现的接口是用批量读写操作的方式来读取 oob 数据，读到  chip->oob_poi 里面，然后调用 `nand_transfer_oob` 根据 ops->mode 将 chip->oob_poi 里面的数据用 memcpy 到 ops->oobbuf
  
- v0：命令、地址都是手动发的
  
  > 根据当前要写的地址是否4字节对齐、要操作的长度是否是页对齐来判断是否对齐。
  >
  > 发0x30、地址、0x00
  >
  > 使用 `chip->ecc.read_page` 把 `数据+oob` 都读出来
  >
  > 如果 `data` 区域不对齐，就把刚刚读到的数据重新 `memcpy` 一遍
  >
  > 数据部分如果对齐，就直接读到 buf，如果不对齐，先读到 tmp_buf，再 memcpy 到 buf
  >
  > oob 部分采用 `nand_transfer_oob` 接口，把刚刚读到的 oob 数据 memcpy 到 oob
  >
  > `数据和oob` 分开读
  
  - ops->datbuf
    - 调用 `chip->ecc.read_page`  --> `nand_read_page_hwecc` 
      - 首先检测 sector 的擦除标记，如果擦除标记为 true，就不读直接全部赋值为 0xff
      - 调用 `chip->cmdfunc(mtd, NAND_CMD_RNDOUT, 0, -1)` ，控制器发命令，由于函数里面没有 `NAND_CMD_RNDOUT` 这个宏，所以默认会发 0x00，然后发地址，0x30
      - 然后调用 `chip->read_buf` 从控制器的 fifo 中读数据到 buffer，然后读控制器的状态寄存器是否有 位翻转
  - ops->oobbuf：发指令的方式读
    - 调用 chip->ecc.read_oob --> `nand_read_oob_std` 
      - 先发一个读数据的指令，然后把列地址配置成 oob 的位置
      - 然后调用 `chip->read_buf` 读 oob 区域的数据
      - 然后调用 `nand_transfer_oob` 来将数据从 `chip->oob_poi` memcpy 到 `ops->oobbuf` 



#### nand_do_write_ops

- v1：命令和地址由 控制器发送
  
  > 先写数据，再写ecc；
  >
  > 由于控制器在写 数据 的时候就会自动把 ecc 数据也写进去，所以要先把 ecc 数据写进去，然后再写数据，写数据的时候控制器会同时把 ecc 码也写进去
  
  - ops->datbuf
    - 调用 `chip->write_data` --> `gxnfc_nand_write_data` 
      - 根据要写的数据是否地址对齐，选择是否先将 数据拷贝到临时buffer再写，调用 `nandflash_programpage` 
        - 配控制器
        - 配 ecc 相关寄存器
        - 把 要写的 page 配到`START_RAW_ADDR` 寄存器
        - 把 要写的数据所在地址配到 `DMA_START_ADDR` 寄存器
        - 把 要写的长度 配到 `XFER_TOTAL_SIZE` 寄存器
        - 写 1 到 `XFER_START` 寄存器开始传输
        - 读状态寄存器，看是否出现bit 翻转的情况
  - ops->oobbuf：用发命令+地址的方式来写的 oob 区域
    - 调用 `chip->ecc.write_oob`  --> `nand_write_oob_std` 
    - 调用 `chip->cmdfunc(mtd, NAND_CMD_SEQIN,mtd->writesize, page)` 
      - 发命令 0x80、行地址、列地址
    - 调用 `chip->write_buf` 把数据写过去
    - 调用 `chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1)` 发命令 0x10
    - 发 0x70 等 flash ready
  
- v0：命令和地址都是手动发的

  > 如果要传输的地址不是页对齐，或者要写的长度小于一整页，或者要写的地址不是 4 字节对齐，就需要把数据先拷贝到一个临时buf，然后和0xff组成一整个page，由于0xff写进去还是0xff，不会影响数据，所以用这种方法组建好 不对齐的部分，然后使用 `chip->write_page` 接口来写数据
  >
  > `oob+data` 在调用 `chip->write_page` 接口的时候一次写进去的
  >
  > ​	先写 data，因为控制器不是自动写 ecc 数据到 oob 区域的。然后再写 ecc 码到 oob 区域。
  
  - ops->datbuf
    - `chip->write_page` --> `nand_write_page` 
      - 拉低 CLE、发命令 0x80、拉高 CLE、拉低 ALE、发地址、拉高ALE
    - `chip->ecc.write_page`  --> `nand_write_page_hwecc` 
      - 先配 ecc 
      - 调用 `chip->write_buf`  --> `gxnfc_nand_write_buf` 
        - 写数据到 控制器的 fifo
        - 读 控制器根据数据生成的 ecc 码到 buffer
        - 对数据区域进行 擦除标记，如果数据全0xff 就标记，否则不标记
        - 调用 `chip->write_buf` --> `gxnfc_nand_write_buf` 把 oob 数据 写到 oob 区域，跟上面写数据不同的就是写的地址不同来区分是 oob 区域还是 main 区域
  - ops->oobbuf：oob buf 在上面的接口已经写进去了





#### nand_erase_opts (nand_utils.c)

- 如果要擦坏块表，直接 free 掉 chip->bbt
- 每次擦除是擦 一个 block
  - 每次都会判断当前的 block 是否是坏块
  - 调用 `nand_erase`  --> `nand_erase_nand` 
  - 根据当前的地址要擦除的地址，找到所在的 page
  - 调用 `chip->erase_cmd` --> `single_erase_cmd` 
  - 调用 `chip->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page)` 
    - v0：拉低 CLE、发命令 0x60、拉高 CLE、拉低ALE、发地址、行地址是哪一个 page、列地址是 -1、拉高ALE
    - v1：发命令 0x60、发地址
  - 调用 `chip->cmdfunc(mtd, NAND_CMD_ERASE2, -1. -1)` 
    - v0：拉低 CLE、发命令 0xd0、拉高 CLE、不发地址、等 flash ready
    - v1：发命令 0xd0
  - 等 flash ready
  - 增加 一个block 这个多个 page，相当于到下一个 block 的第一个page 再继续循环上面的操作



## nand_bbt.c

#### 坏块表信息结构：

主表：

| magic | version | reserved |  length(小端)  |  bbt[]   |  crc   |
| :---: | :-----: | :------: | :------------: | :------: | :----: |
| Bbt0  |  0x01   |   0xff   | block 数量长度 | 坏块信息 | 4 byte |

镜像表：只是 magic 换成 `1tbB`

- `magic`：代表标记头信息
- `version`：代表坏块表的版本号
- `reserved`：保留，未使用

- `bbt[]` ：

| 2bit 代表一个 block | block 信息     |
| ------------------- | -------------- |
| 11b                 | 好块           |
| 00b                 | 工厂坏块       |
| 01b、10b            | 使用过程的坏块 |

- `crc` ：由前面所有的字段数据计算得到



​	坏块表存储在内存中，主表和镜像表各占一个 block，NAND 分配末尾 4 个 block 用于存放坏块表，存放在其中的 第一个 page；在应用中，末尾的 4 个预留块被标记为坏块，驱动会默认跳过这些块。

​	末尾 4 个 block 可能状态：

| 好块个数 | 主表、镜像表存储状态                                         |
| :------- | :----------------------------------------------------------- |
| 0        | 主表和镜像表无空间存储，使用开机读取每个 block 坏块标记在内存中创建的 坏块表 |
| 1        | Flash 中存储主表，不存储镜像表，使用主表的坏块信息           |
| 2、3、4  | Flash 中存储主表和镜像表，使用主表的坏块信息                 |





#### nand_default_bbt

- 主表和镜像表结构：

  ```c
  static struct nand_bbt_descr bbt_mirror_descr = {
  	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
  		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
  	.offs = 8,
  	.len = 4,
  	.veroffs = 12,
  	.maxblocks = NAND_BBT_SCAN_MAXBLOCKS,
  	.pattern = mirror_pattern
  };
  
  static struct nand_bbt_descr bbt_main_no_oob_descr = {
  	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
  		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP
  		| NAND_BBT_NO_OOB,
  	.len = 4,
  	.veroffs = 4,
  	.maxblocks = NAND_BBT_SCAN_MAXBLOCKS,
  	.pattern = bbt_pattern
  };
  
  static struct nand_bbt_descr bbt_mirror_no_oob_descr = {
  	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
  		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP
  		| NAND_BBT_NO_OOB,
  	.len = 4,
  	.veroffs = 4,
  	.maxblocks = NAND_BBT_SCAN_MAXBLOCKS,
  	.pattern = mirror_pattern
  };
  ```

  

- 调到 `nand_scan_bbt(mtd, this->badblock_pattern)` 
- 先申请 `this->bbt` 这个内存空间，用于在内存中保存坏块信息
- 验证 主表和镜像表的一些参数
- 申请一个 buf，大小是 block，包含 oob
- 先调用 `search_read_bbts` 








## nand_ecc.c

