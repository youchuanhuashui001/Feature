# NAND FLASH

​		nand flash 使用独立的 nfc 控制器，已经与 spi 无关，nand flash 引脚描述：

- ALE
- CLE





## 主要数据结构：

- `struct gx_nand_dev`：

```c
struct gx_nand_dev {
        void *                 regs;       // 控制器寄存器基址                                                                                                                                                                                     
        struct gx_nand_chip    chip;
        struct gx_mtd_info     mtd;			// mtd 设备层
};
```



* `struct gx_mtd_info`：

```c
struct gx_mtd_info {
        u_char type;
        u_int32_t flags;
        uint64_t size;    /* Total size of the MTD */

        /* "Major" erase size for the device. Na?ve users may take this
        ┆* to be the only erase size available, or may use the more detailed
        ┆* information below if they desire
        ┆*/
        u_int32_t erasesize;
        /* Minimal writable flash unit size. In case of NOR flash it is 1 (even
        ┆* though individual bits can be cleared), in case of NAND flash it is
        ┆* one NAND page (or half, or one-fourths of it), in case of ECC-ed NOR
        ┆* it is of ECC block size, etc. It is illegal to have writesize = 0.
        ┆* Any driver registering a struct gx_mtd_info must ensure a writesize of
        ┆* 1 or larger.
        ┆*/
        u_int32_t writesize;
        u_int32_t oobsize;   /* Amount of OOB data per block (e.g. 16) */
        u_int32_t oobavail;  /* Available OOB bytes per block */

        /* Kernel-only stuff starts here. */
        const char *name;

        /* ecc layout structure pointer - read only ! */
        struct nand_ecclayout *ecclayout;

        /* max number of correctible bit errors per ecc step */
        unsigned int ecc_strength;

        /*
        ┆* Erase is an asynchronous operation.  Device drivers are supposed
        ┆* to call instr->callback() whenever the operation completes, even
        ┆* if it completes with a failure.
        ┆* Callers are supposed to pass a callback function and wait for it
        ┆* to be called before writing to the block.
        ┆*/
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



* `struct gx_nand_chip`：

```c
struct gx_nand_chip {
        void  __iomem   *IO_ADDR_R;         /* 需要读出数据的nandflash地址 */
        void  __iomem   *IO_ADDR_W;         /* 需要写入数据的nandflash地址 */ 
        struct nand_request *request;
    	// 从芯片中读一个字节
        uint8_t         (*read_byte)(struct gx_mtd_info *mtd);
    	// 从芯片中读一个字
        u16             (*read_word)(struct gx_mtd_info *mtd);
    	// 将缓冲区内容写入 nandflash 地址， len：数据长度
        void            (*write_buf)(struct gx_mtd_info *mtd, const uint8_t *buf, int len);
    	// 读 nandflash 地址至缓冲区， len：数据长度
        void            (*read_buf)(struct gx_mtd_info *mtd, uint8_t *buf, int len);
    	// 验证芯片和写入缓冲区中的数据
        int             (*verify_buf)(struct gx_mtd_info *mtd, const uint8_t *buf, int len);
    	// 选中芯片， chip == 0 ：选中； chip == -1 ：取消选中
        void            (*select_chip)(struct gx_mtd_info *mtd, int chip);
    	// 检测是否有坏块
    	int             (*block_bad)(struct gx_mtd_info *mtd, loff_t ofs, int getchip);
    	// 标记坏块
        int             (*block_markbad)(struct gx_mtd_info *mtd, loff_t ofs);
    	// 命令、地址控制函数， dat：要传输的命令/地址
    	// 当 ctl 的 bit[1] == 1:表示要发送的 dat 是命令
    	// 当 ctl 的 bit[2] == 1:表示要发送的 dat 是地址
    	// 当 ctl 的 bit[0] == 1:表示使能 nand； == 0 表示失能 nand
        void            (*cmd_ctrl)(struct gx_mtd_info *mtd, int dat, unsigned int ctrl);
    	// 设备是否就绪，当该函数返回的 RnB 引脚的数据等于 1，表示 nandflash 已就绪
        int             (*dev_ready)(struct gx_mtd_info *mtd);
    	// 实现命令发送，最终调用 gx_nand_chip -> cmd_ctl 来实现
        void            (*cmdfunc)(struct gx_mtd_info *mtd, unsigned command, int column, int page_addr);
    	// 等待函数，通过 gx_nand_chip->dev_readty来等待 nandflash 是否就绪
        int             (*waitfunc)(struct gx_mtd_info *mtd, struct gx_nand_chip *this);
    	// 擦除命令的处理
        void            (*erase_cmd)(struct gx_mtd_info *mtd, int page);
    	// 扫描坏块
        int             (*scan_bbt)(struct gx_mtd_info *mtd);
    	// 错误状态
        int             (*errstat)(struct gx_mtd_info *mtd, struct gx_nand_chip *this, int state, int status, int page);
    	// 写 page
        int             (*write_page)(struct gx_mtd_info *mtd, struct gx_nand_chip *chip, const uint8_t *buf, int page, int cached, int raw);
    	// 读数据
        int             (*read_data)(struct gx_mtd_info *mtd, unsigned int from, unsigned char *data, unsigned int len);
    	// 写数据 
        int             (*write_data)(struct gx_mtd_info *mtd, unsigned int to,  unsigned char *data, unsigned int len);
		// 由板子决定的延迟时间
        int             chip_delay;
    	// 与具体的 NAND 芯片相关的一些选项，默认为 8 位位宽，可以设置为 NAND_BUSWIDTH_16,表示nand的总线宽为16 
        unsigned int    options;
        unsigned int    bbt_options;

    	// 用位表示的 NAND 芯片的 page 大小，如某片 NAND 芯片的一个 page 有 512 字节，那么 page_shift 就是 9 --> 2^9
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
        struct nand_ecclayout  *ecclayout;	// ecc 分布
        //	ecc 校验结构体，若不设置， ecc.mode 默认为 NAND_ECC_NONE
    	// 可以设置为 硬件ECC 和 软件ECC，例如：ecc.mode = NAND_ECC_SOFT
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

```







- mtd < --- > dev

```c
#define mtd_to_dev(_mtd)   ((_mtd)->priv)
#define dev_to_mtd(_dev)   (&(_dev)->mtd)
```







## 主要 API 接口：

- `int gx_nand_init_v1(unsigned int base_addr, unsigned int is_boot)`

```c
int gx_nand_init_v1(unsigned int base_addr, unsigned int is_boot)
{
        int ret;
        struct gx_nand_dev *dev = &gx_nand_dev;

        memset(dev, 0, sizeof(*dev));
        dev->regs = (void *)base_addr;
        dev->mtd.name = "nand";

        /* boot工具不使用NAND存储的坏块表 */
        if (is_boot == 0)
                dev->chip.bbt_options |= NAND_BBT_USE_FLASH | NAND_BBT_NO_OOB;

        gxnfc_init_chip_v1(dev);
        ret = nand_scan(&dev->mtd, 1); 
        return ret;
}

```

 
