#include <stdint.h>
#include <stdbool.h>
#include <common.h>
#include <drivers/gx_spinand.h>
#include <platform_def.h>

//TODO: read from paramater page
#define CONFIG_SF_DEFAULT_CLKDIV 2
#define CONFIG_SF_DEFAULT_SAMPLE_DELAY 0

typedef struct {
	unsigned int clk_div;                /* spinand 分频值 */
	unsigned int sample_delay;           /* spinand 读采样延时 */
	unsigned int page_size;              /* spinand 页大小 */
	unsigned int ecc_enable;             /* spinand 使能ecc的方法 */
	unsigned int buf_enable;             /* spinand 需要使能buf功能 */
	unsigned int page_num_per_block;     /* spinand block含的页数 */
	unsigned int plane_sel;              /* spinand plane奇偶选择 */
}spinand_flash_info_t;

spinand_flash_info_t spinand_flash_info = {
	.clk_div = 8,
	.sample_delay = 0,
	.page_size = 2048,
	.ecc_enable = 0,
	.buf_enable = 0,
	.page_num_per_block = 64,
	.plane_sel = 0
};

/* SPI register offsets */
#define SPIM_CTRLR0             0x00
#define SPIM_CTRLR1             0x04
#define SPIM_ENR                0x08
#define SPIM_SER                0x10
#define SPIM_BAUDR              0x14
#define SPIM_TXFTLR             0x18
#define SPIM_RXFTLR             0x1c
#define SPIM_TXFLR              0x20
#define SPIM_RXFLR              0x24
#define SPIM_SR                 0x28
#define SPIM_IMR                0x2c
#define SPIM_ISR                0x30
#define SPIM_RISR               0x34
#define DW_SPI_TXOICR           0x38
#define DW_SPI_RXOICR           0x3c
#define DW_SPI_RXUICR           0x40
#define SPIM_ICR                0x48
#define SPIM_DMACR              0x4c
#define SPIM_DMATDLR            0x50
#define SPIM_DMARDLR            0x54
#define SPIM_TXDR               0x60
#define SPIM_RXDR               0x60
#define SPIM_RX_SAMPLE_DLY      0XF0


/* Bit fields in SR, 7 bits */
#define SR_MASK                 0x7f            /* cover 7 bits */
#define SR_BUSY                 (1 << 0)
#define SR_TF_NOT_FULL          (1 << 1)
#define SR_TF_EMPT              (1 << 2)
#define SR_RF_NOT_EMPT          (1 << 3)
#define SR_RF_FULL              (1 << 4)
#define SR_TX_ERR               (1 << 5)
#define SR_DCOL                 (1 << 6)

//Bits in CTRL0
#define SPI_DFS_OFFSET          0
#define SPI_DFS_8BIT            0x07

#define SPI_FRF_OFFSET          4    /* Frame Format */
#define SPI_MODE_OFFSET         6     /* SCPH & SCOL */

#define SPI_TMOD_OFFSET         8
#define SPI_TMOD_TR             0x0   /* xmit & recv */
#define SPI_TMOD_TO             0x1   /* xmit only */
#define SPI_TMOD_RO             0x2   /* recv only */

struct mtd_oob_ops {
	uint32_t  mode;
	size_t    len;
	size_t    retlen;
	size_t    ooblen;
	size_t    oobretlen;
	uint32_t  ooboffs;
	uint8_t  *datbuf;
	uint8_t  *oobbuf;
};

struct spi_nand_info{
	uint16_t    nand_id;
	const char *name;
	uint32_t    nand_size;
	uint32_t    usable_size;
	uint32_t    block_size;
	uint32_t    block_main_size;
	uint32_t    block_num_per_chip;
	uint16_t    page_size;
	uint16_t    page_main_size;
	uint16_t    page_spare_size;
	uint16_t    page_num_per_block;
	uint16_t    block_shift;
	uint16_t    page_shift;
	uint32_t    block_mask;
	uint32_t    page_mask;
	struct nand_ecclayout *ecclayout;
};

static struct spi_nand_info nand_info[] = {
	/**
	 *  0x112c 0x122c 0x132c 0xc8f1 0xf1c8 0xc8d1
	 *  0xd1c8 0xaaef 0x21C8 0xc298 0x12c2 0xe1a1
	 */
	[0] = {
		.nand_size          = 1024 * 64 * 2112,
		.usable_size        = 1024 * 64 * 2048,
		.block_size         = 2112*64,
		.block_main_size    = 2048*64,
		.block_num_per_chip = 1024,
		.page_size          = 2112,
		.page_main_size     = 2048,
		.page_spare_size    = 64,
		.page_num_per_block = 64,
		.block_shift        = 17,
		.block_mask         = 0x1ffff,
		.page_shift         = 11,
		.page_mask          = 0x7ff,
		.ecclayout          = NULL,
	},
	[1] = { // 0xc8f2
		.nand_size          = (2048 * 64 * 2112),
		.usable_size        = (2048 * 64 * 2048),
		.block_size         = (2112*64),
		.block_main_size    = (2048*64),
		.block_num_per_chip = 2048,
		.page_size          = 2112,
		.page_main_size     = 2048,
		.page_spare_size    = 64,
		.page_num_per_block = 64,
		.block_shift        = 17,
		.block_mask         = 0x1ffff,
		.page_shift         = 11,
		.page_mask          = 0x7ff,
		.ecclayout          = NULL,
	},
	[2] = { // 0x68b4, 0xd4c9, 0x130b
		.nand_size          = (2048 * 64 * 4352),
		.usable_size        = (2048 * 64 * 4096),
		.block_size         = (4352*64),
		.block_main_size    = (4096*64),
		.block_num_per_chip = 2048,
		.page_size          = 4352,
		.page_main_size     = 4096,
		.page_spare_size    = 256,
		.page_num_per_block = 64,
		.block_shift        = 18,
		.block_mask         = 0x3ffff,
		.page_shift         = 12,
		.page_mask          = 0xfff,
		.ecclayout          = NULL,
	},
};

static struct spi_nand_info* p_snand_info = &nand_info[1];

#define assert_param(_param, _rtn) do{                                         \
	if(!(_param)) return _rtn;                                                 \
}while(0)


struct spinand_cmd {
	uint8_t   cmd;
	uint32_t  n_addr;
	uint8_t   addr[3];
	uint32_t  n_dummy;
	uint32_t  n_tx;
	uint8_t   *tx_buf;
	uint32_t  n_rx;
	uint8_t   *rx_buf;
};
static uint16_t spinand_id = -1;

typedef struct spi_xfer{
	uintptr_t regs;
	volatile uint32_t *cs_regs;
	uint32_t freq;
	uint32_t mode;
	int32_t fifo_len;

	uint32_t len;
	uint8_t *tx;
	uint8_t *tx_end;
	uint8_t *rx;
	uint8_t *rx_end;
}spi_xfer_t;

static spi_xfer_t spi_priv = {
	.regs     = SPI0_BASE,
	.fifo_len = -1,
};

static int spi_cs_set(int enable)
{
	if (enable)
		*(volatile unsigned int *)(SPI0_CS_BASE) = 0x0e;
	else
		*(volatile unsigned int *)(SPI0_CS_BASE) = 0x0f;
	return 0;
}

static void spl_spi_init(int sample, int cs, int speed, int mode)
{
	uint16_t clk_div;
	uint32_t sample_delay = 1;

	spi_priv.freq = speed;
	spi_priv.mode = mode;

	dw_writel(spi_priv.regs, SPIM_ENR, 0);

	uint32_t newmask = dw_readl(spi_priv.regs, SPIM_IMR) & ~0xFF;
	dw_writel(spi_priv.regs, SPIM_IMR,     newmask);

	clk_div = spinand_flash_info.clk_div;
	sample_delay = spinand_flash_info.sample_delay;

	dw_writel(spi_priv.regs, SPIM_BAUDR, clk_div);
	dw_writel(spi_priv.regs, SPIM_RX_SAMPLE_DLY, ((sample_delay >> 1) & 0xFFFF) | ((sample_delay & 0x01) << 16));
	dw_writel(spi_priv.regs, SPIM_CTRLR0,  (SPI_DFS_8BIT << SPI_DFS_OFFSET)   |\
										   (SPI_TMOD_TR  << SPI_TMOD_OFFSET));
	dw_writel(spi_priv.regs, SPIM_CTRLR1,  0);

	if (spi_priv.fifo_len < 0) {
		uint32_t fifo;
		for (fifo = 1; fifo < 256; fifo++) {
			dw_writel(spi_priv.regs, SPIM_TXFTLR, fifo);
			if (fifo != dw_readl(spi_priv.regs, SPIM_TXFTLR))
				break;
		}
		spi_priv.fifo_len = (fifo == 1) ? 0 : fifo;
		dw_writel(spi_priv.regs, SPIM_TXFTLR, 0);
	}

	dw_writel(spi_priv.regs, SPIM_SER,     1);

	dw_writel(spi_priv.regs, SPIM_ENR, 1);

}

static uint32_t tx_max(spi_xfer_t *pf)
{
	uint32_t fifo_left = (pf)->fifo_len - dw_readl((pf)->regs, SPIM_TXFLR);
	uint32_t xfer_left = (pf)->tx_end - (pf)->tx;

	return (fifo_left < xfer_left) ? fifo_left : xfer_left;
}

static void spi_write(spi_xfer_t *priv)
{
	uint32_t max = 0;
	uint16_t txw = 0;

	while (tx_max(priv) == 0);
	while ((dw_readl((priv)->regs, SPIM_SR) & SR_BUSY) == SR_BUSY);
	max = tx_max(priv);
	while(max--){
		if((priv)->tx_end - (priv)->tx)
			txw = *((priv)->tx);
		dw_writel((priv)->regs, SPIM_TXDR, txw);
		++(priv)->tx;
	}
}

static uint32_t rx_max(spi_xfer_t *pf)
{
	uint32_t rx_left   = (pf)->rx_end - (pf)->rx;
	uint32_t fifo_left = dw_readl((pf)->regs, SPIM_RXFLR);

	return (fifo_left < rx_left) ? fifo_left : rx_left;
}

static void spi_read(spi_xfer_t *priv)
{
	uint32_t max = 0;
	uint16_t rxw = 0;

	while(rx_max(priv) == 0);
	while ((dw_readl((priv)->regs, SPIM_SR) & SR_BUSY) == SR_BUSY);
	max = rx_max(priv);
	while(max--) {
		rxw = dw_readl((priv)->regs, SPIM_RXDR);
		if((priv)->rx_end - (priv)->rx)
			*((priv)->rx) = 0xff & rxw;
		++(priv)->rx;
	}
}

static int spl_xfer(uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t flags)
{
	dw_writel(spi_priv.regs, SPIM_ENR,   0);
	dw_writel(spi_priv.regs, SPIM_DMACR, 0);
	dw_writel(spi_priv.regs, SPIM_ENR,   1);

	spi_priv.len    = len;
	spi_priv.tx     = tx;
	spi_priv.tx_end = tx + len;
	spi_priv.rx     = rx;
	spi_priv.rx_end = rx +len;

	do{
		spi_write(&spi_priv);
		spi_read(&spi_priv);
	}while(spi_priv.rx < spi_priv.rx_end);

	return spi_priv.len;
}

static void spinand_cmd_for_4k_page(struct spinand_cmd *cmd)
{
	uint8_t dummy[2] = {0x00,0x00};

	spi_cs_set(1);
	spl_xfer(&cmd->cmd, NULL, 1, SPI_XFER_BEGIN);

	if (cmd->n_dummy)
		spl_xfer(dummy, NULL, cmd->n_dummy, SPI_XFER_BEGIN);

	if(cmd->n_addr)
		spl_xfer(cmd->addr, NULL, cmd->n_addr, SPI_XFER_BEGIN);

	if (cmd->n_tx)
		spl_xfer(cmd->tx_buf, NULL, cmd->n_tx, SPI_XFER_BEGIN);

	if (cmd->n_rx)
		spl_xfer(NULL, cmd->rx_buf, cmd->n_rx, SPI_XFER_BEGIN);

	spi_cs_set(0);
}

static void spinand_cmd(struct spinand_cmd *cmd)
{
	uint8_t dummy[2] = {0x00,0x00};

	spi_cs_set(1);
	spl_xfer(&cmd->cmd, NULL, 1, SPI_XFER_BEGIN);

	if(cmd->n_addr)
		spl_xfer(cmd->addr, NULL, cmd->n_addr, SPI_XFER_BEGIN);

	if (cmd->n_dummy)
		spl_xfer(dummy, NULL, cmd->n_dummy, SPI_XFER_BEGIN);

	if (cmd->n_tx)
		spl_xfer(cmd->tx_buf, NULL, cmd->n_tx, SPI_XFER_BEGIN);

	if (cmd->n_rx)
		spl_xfer(NULL, cmd->rx_buf, cmd->n_rx, SPI_XFER_BEGIN);

	spi_cs_set(0);
}

/**
 * spinand_read_id- Read SPI Nand ID
 *
 * Description:
 *    Read ID: read two ID bytes from the SPI Nand device
 */
static void spinand_read_id(void *id)
{
	struct spinand_cmd cmd;

	cmd.cmd = CMD_READ_ID;
	cmd.n_dummy = 1;
	cmd.n_rx = 2;
	cmd.rx_buf = id;

	cmd.n_addr = 0;
	cmd.n_tx = 0;

	spinand_cmd(&cmd);
}

static void spinand_get_features(uint8_t addr, uint8_t *features)
{
	struct spinand_cmd cmd;

	cmd.cmd = CMD_READ_REG;
	cmd.n_addr = 1;
	cmd.addr[0] = addr;
	cmd.n_rx = 1;
	cmd.rx_buf = features;

	cmd.n_dummy = 0;
	cmd.n_tx = 0;

	spinand_cmd(&cmd);
}

static void spinand_set_features(uint8_t addr, uint8_t *features)
{
	struct spinand_cmd cmd;

	cmd.cmd = CMD_WRITE_REG;
	cmd.n_addr = 1;
	cmd.addr[0] = addr;
	cmd.n_tx = 1;
	cmd.tx_buf = features;

	cmd.n_rx = 0;
	cmd.n_dummy = 0;

	spinand_cmd(&cmd);
}

static int spinand_enable_ecc(void)
{
	uint8_t features = 0, addr = 0;
	// TODO: 从参数页中读取是否需要使能 ecc
//	if (spinand_flash_into_t->ecc_enable) {
	if ((spinand_id == 0xF20B) ||                        /* XT26G02BWSUGA  */  \
			(spinand_id == 0xA1CD) ||            /* FS35ND01G-D1F1 */  \
			(spinand_id == 0xB1CD) ||            /* FS35ND01G-S1F1 */
			(spinand_id == 0xF1A1))              /* TX25G01 */ {
		addr = REG_ECC;
	} else {
		addr = REG_OTP;
	}
	spinand_get_features(addr, &features);

	if ((features & REG_ECC_MASK) == REG_ECC_MASK) {
		return 0;
	}
	else {
		features |= REG_ECC_MASK;
		/* Dosilicon不能写OTP_PRT, 否则不能program */
		features &= ~OTP_PRT;
		spinand_set_features(addr, &features);
		return 1;
	}
}

static void spinand_read_page_to_cache(uint32_t page_id)
{
	struct spinand_cmd cmd;
	uint32_t row;

	row = page_id;

	cmd.cmd = CMD_READ;
	cmd.n_addr = 3;
	cmd.addr[0] = (uint8_t)((row&0xff0000)>>16);
	cmd.addr[1] = (uint8_t)((row&0xff00)>>8);
	cmd.addr[2] = (uint8_t)(row&0x00ff);

	cmd.n_rx = 0;
	cmd.n_dummy = 0;
	cmd.n_tx = 0;

	spinand_cmd(&cmd);
}

static void spinand_read_status(uint8_t *status)
{
	struct spinand_cmd cmd;

	cmd.cmd = CMD_READ_REG;
	cmd.n_addr = 1;
	cmd.addr[0] = REG_STATUS;
	cmd.n_rx = 1;
	cmd.rx_buf = status;

	cmd.n_dummy = 0;
	cmd.n_tx = 0;

	spinand_cmd(&cmd);
}

static void spinand_wait_ready(void)
{
	uint8_t status = 0;

	while (1) {
		spinand_read_status(&status);

		if ((status & STATUS_OIP_MASK) == STATUS_READY)
			break;
	}
}

static void spinand_read_from_cache(uint16_t byte_id, uint16_t len, uint8_t* rbuf)
{
	struct spinand_cmd cmd;
	uint16_t column;

	column = byte_id;

	cmd.cmd = CMD_READ_RDM;
	cmd.n_addr = 2;
	cmd.addr[0] = (uint8_t)((column&0xff00)>>8);
	cmd.addr[1] = (uint8_t)(column&0x00ff);
	cmd.n_dummy = 1;
	cmd.n_rx = len;
	cmd.rx_buf = rbuf;

	cmd.n_tx = 0;

	// TODO: read from paramater
	// if(spinand_flash_info_t->page_size == 4096)
	if(spinand_id == 0x68B4)  // GD5F4GM5UFY 4K pagesize
		spinand_cmd_for_4k_page(&cmd);
	else
		spinand_cmd(&cmd);
}

#define PAGES_PER_BLOCK            64
static int snand_read_page(uint32_t page_id, uint16_t offset, uint16_t len, uint8_t *rbuf)
{
	uint8_t status = 0;

	/* DS35Q2GA, F50L2G41XA, ZD35Q2GA只有一个die,每个die有两个plane.
	   当block number为奇数,选择另外一个plane. */
	// TODO: read from paramater
	// if (spinand_flash_info_t->plane_sel)
	if ((spinand_id == 0x72e5) || (spinand_id == 0xf2e5) || (spinand_id == 0x242c) ||
			(spinand_id == 0x72ba)) {
		if((page_id / PAGES_PER_BLOCK) % 2)
			offset |= (0x1 << 12);
	}

	spinand_read_page_to_cache(page_id);
	while (1) {
		spinand_read_status(&status);

		if ((status & STATUS_OIP_MASK) == STATUS_READY) {
			break;
		}
	}
	spinand_read_from_cache(offset, len, rbuf);
	return 0;
}

int gx_spinand_init(void)
{
	int i;
	uint8_t  s2;

	spl_spi_init(1, 0, 100000000, 0x00);

	spinand_wait_ready();

	for (i = 0; i < 100; i++) {

		spinand_read_id(&spinand_id);

		if ((spinand_id != 0xFFFF) && (spinand_id != 0x00))
			break;
	}

	spinand_enable_ecc();
	spinand_wait_ready();

	/* W25N01GVZEIT开启BUF功能 */
	// TODO: read from paramater
//	if (spinand_into_t->buf_enable)
	if (spinand_id == 0xAAEF) {
		spinand_get_features(REG_OTP, &s2);
		if ((s2 & 0x08) == 0x00) {
			s2 |= 0x08;
			spinand_set_features(REG_OTP, &s2);
		}
	}

	/* GD5F4GM5UFY 4K pagesize */
	// TODO: read from paramater
//	if (spinand_flash_info_t->page_size = 4096)
	if (spinand_id == 0x68B4 || spinand_id == 0xD4C9 || spinand_id == 0x130B) {
		p_snand_info = &nand_info[2];
	}

	return 0;
}

static int snand_read_ops(ssize_t from, struct mtd_oob_ops *ops)
{
	assert_param(ops, -1);
	struct spi_nand_info *info = p_snand_info;
	int page_id, page_offset = 0;
	int count = 0;
	int main_left, main_ok = 0, main_offset = 0;
	int size, retval;

	page_id = from >> info->page_shift;
	page_offset = from & info->page_mask;
	main_left = ops->len;
	main_offset = page_offset;

	while (main_left){
		size = min(main_left, info->page_main_size - main_offset);
		retval = snand_read_page(page_id + count, main_offset, \
					size, ops->datbuf + main_ok);

		if (retval < 0) {
			//printf("snand_read_ops: fail, page=%d!\n", page_id);
			return -1;
		}

		main_ok += size;
		main_left -= size;
		main_offset = 0;
		count++;
	}

	ops->retlen = main_ok;

	return main_ok;
}

static int snand_block_isbad(uint32_t ofs)
{
	struct spi_nand_info *info = p_snand_info;

	uint8_t is_bad = 0x00;
	uint32_t page_id = (ofs >> info->block_shift) * info->page_num_per_block;

	snand_read_page(page_id, info->page_main_size, 1, &is_bad);

	return is_bad == 0XFF ? 0 : 1;
}

static int snand_read_skip_bad(ssize_t offset, void *buf, uint32_t len)
{
	// TODO: 后面是不是要把这个去掉，g_snand_info，改成用参数页中的
	struct spi_nand_info *info = p_snand_info;
	uint32_t blk_sz    = info->block_main_size;
	ssize_t  read_left = len;
	uint32_t page_size = 2048;
	uint32_t page_num_per_block = 64;
	uint32_t physical_addr = 0;
	uint32_t tmp_addr = offset;

	if(len == 0)
		return 0;

	assert_param(offset + len <= info->usable_size, -1);

	uint8_t *pbuf = buf;
	struct   mtd_oob_ops ops;

	// TODO: read from paramater
	//page_size = spinand_info_t->page_size;

	while (physical_addr <= tmp_addr) {
		if (snand_block_isbad(physical_addr)) {
			tmp_addr += page_num_per_block * page_size;
		}
		physical_addr += page_num_per_block * page_size;
	}

	offset = tmp_addr;

	while(read_left > 0) {
		ssize_t blk_ofs = offset & info->block_mask;
		/* skip bad block */
		if(snand_block_isbad(offset & ~info->block_mask)){
			offset += blk_sz - blk_ofs;
			continue;
		}

		ops.datbuf = pbuf;
		ops.len    = min(read_left, blk_sz-blk_ofs);

		if(snand_read_ops(offset, &ops) < 0)
			return -1;

		pbuf      += ops.retlen;
		offset    += ops.retlen;
		read_left -= ops.retlen;
	}

	return pbuf-(uint8_t*)buf;
}

int gx_spinand_read(uint32_t offset, uint32_t len, uint8_t *dest)
{
	int ret;

	ret = snand_read_skip_bad(offset, dest, len);
	if (ret < 0)
		return -1;

	return 0;
}



