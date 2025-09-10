#include "autoconf.h"
#include "kernelcalls.h"
#ifdef CONFIG_GX_NOS
#include "driver/spi.h"
#include <errno.h>
#elif defined(CONFIG_GX_LINUX)
#include <linux/version.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#elif defined(CONFIG_GX_ECOS)
#include <cyg/io/spi.h>
#endif
#include <gx_flash_common.h>
#include "spinor_ids.h"
#include "spinor_base.h"

#define JEDEC_MFR(_jedec_id)    ((_jedec_id) >> 16)

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static char read_sr1(struct spi_device *spi)
{
	ssize_t retval;
	unsigned char code = GX_CMD_RDSR1;
	unsigned char val;

	retval = spi_write_then_read(spi, &code, 1, &val, 1);

	if (retval < 0) {
		gxlog_e("error %d reading SR\n",(int) retval);
		return retval;
	}

	return val;
}

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static char read_sr2(struct spi_device *spi )
{
	ssize_t retval;
	unsigned char code = GX_CMD_RDSR2;
	unsigned char val;

	retval = spi_write_then_read(spi, &code, 1, &val, 1);

	if (retval < 0) {
		gxlog_e("error %d reading SR\n",(int) retval);
		return retval;
	}

	return val;
}

static char read_sr3(struct spi_device *spi )
{
	ssize_t retval;
	unsigned char code = GX_CMD_RDSR3;
	unsigned char val;

	retval = spi_write_then_read(spi, &code, 1, &val, 1);

	if (retval < 0) {
		gxlog_e("error %d reading SR\n",(int) retval);
		return retval;
	}

	return val;
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int write_enable(struct spi_device *spi)
{
	unsigned char code = GX_CMD_WREN;
	return spi_write(spi, &code, 1);
}

#ifdef CONFIG_GX_NOS
/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int wait_till_ready(struct spi_device *spi)
{
	int sr;
	do{
		sr = read_sr1(spi);
	}while (sr & GX_STAT_WIP);
	return 0;
}
#endif

#ifdef CONFIG_GX_LINUX
/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int wait_till_ready(struct spi_device *spi)
{
	int sr;
	do {
		sr = read_sr1(spi);
		if (!(sr & GX_STAT_WIP))
			return 0;
		cond_resched();
	} while (1);
	return 0;
}
#endif

#ifdef CONFIG_GX_ECOS
static int wait_till_ready(struct spi_device *spi)
{
	int sr;
	do {
		sr = read_sr1(spi);
		if (!(sr & GX_STAT_WIP))
			return 0;
		cyg_thread_yield();
	} while (1);
	return 0;
}
#endif

// Write 8bit or 16bit status register
static int write_sr(struct spi_device *spi, unsigned char *buf, int len, unsigned char reg_order)
{
	unsigned char tx_buf[3] = {0 ,0 ,0};

	if(len > 2)
		return -1;

	switch(reg_order)
	{
		case 1:
			tx_buf[0] = GX_CMD_WRSR1;
			break;
		case 2:
			tx_buf[0] = GX_CMD_WRSR2;
			break;
		case 3:
			tx_buf[0] = GX_CMD_WRSR3;
			break;
		default:
			break;
	}

	tx_buf[1] = *buf++;
	if(len == 2)
		tx_buf[2] = *buf++;

	wait_till_ready(spi);
	write_enable(spi);
	spi_write(spi, tx_buf, len + 1);

	return 0;
}


static int write_function_sr(struct spi_device *spi, unsigned char value)
{
	unsigned char tx_buf[2];

	tx_buf[0] = GX_CMD_WRFR;
	tx_buf[1] = value;
	wait_till_ready(spi);
	write_enable(spi);
	spi_write(spi, tx_buf, sizeof(tx_buf));

	return 0;
}

static int read_function_sr(struct spi_device *spi, unsigned char *value)
{
	int tmp;
	unsigned char cmd = GX_CMD_RDFR;

	wait_till_ready(spi);

	tmp = spi_write_then_read(spi, &cmd, 1, value, 1);
	if (tmp < 0) {
		gxlog_e( "spi bus_num=%d: error %d read function reg\n",
				spi->master->bus_num, tmp);
		return -1;
	}

	return 0;
}

int spi_nor_readid(struct spi_device *spi, int *jedec)
{
	int              tmp;
	unsigned char    code = GX_CMD_READID ;
	unsigned char    id[3];

	tmp = spi_write_then_read(spi, &code, 1, id, 3);
	if (tmp < 0) {
		gxlog_e( "spi bus_num=%d: error %d reading JEDEC ID\n",
				spi->master->bus_num, tmp);
		return -1;
	}
	*jedec = id[0];
	*jedec = *jedec << 8;
	*jedec |= id[1];
	*jedec = *jedec << 8;
	*jedec |= id[2];

	return 0;

}

unsigned int spi_nor_get_info(struct spi_nor_flash *flash, enum gx_flash_info flash_info)
{
	unsigned int len;
	switch(flash_info) {
	case GX_FLASH_CHIP_TYPE:
		return GX_FLASH_CHIP_TYPE_NOR;
	case GX_FLASH_CHIP_NAME:
		return (unsigned int)flash->info->name;
	case GX_FLASH_CHIP_ID:
		return flash->info->jedec_id;
	case GX_FLASH_CHIP_SIZE:
		return flash->size;
	case GX_FLASH_BLOCK_SIZE:
	case GX_FLASH_SECTOR_SIZE:
	case GX_FLASH_ERASE_SIZE:
		return SPI_FLASH_ERASE_SIZE;
	case GX_FLASH_BLOCK_NUM:
	case GX_FLASH_SECTOR_NUM:
	case GX_FLASH_ERASE_NUM:
		return flash->size/SPI_FLASH_ERASE_SIZE;
	case GX_FLASH_PAGE_SIZE:
		return SPI_FLASH_PAGE_SIZE;
	case GX_FLASH_PAGE_NUM:
		return flash->size/SPI_FLASH_PAGE_SIZE;
	case GX_FLASH_PROTECT_MODE:
		return flash->protect_mode;
	case GX_FLASH_PROTECT_LEN:
		if (-1 == spi_nor_write_protect_status(flash, &len))
			return -1;
		else
			return len;
	case GX_FLASH_MATCH_MODE:
		return flash->match_mode;
	default: break;
	}

	return -1;
}

int spi_nor_read_uniqueid(struct spi_nor_flash *flash, unsigned char *buf, unsigned int buf_len, unsigned int *id_len)
{
	struct spi_device *spi = flash->spi;
	unsigned int jedec_id = flash->info->jedec_id;
	unsigned char cmd_dummy[5] = {0x00};
	unsigned int cmd_dummy_len = 0;
	int len = 0;

	switch(JEDEC_MFR(jedec_id)) {
	case 0x5e: /* zb25vq32/t25s32 */
	case 0xef: /* winbond */
	case 0xa1: /* fudan */
	case 0x49: /* foresee */
	case 0x68: /* byt */
	case 0xba: /* zetta */
	case 0xc4: /* gt */
	case 0x94: /* bw */
		// 1byte cmd(0x4B) + 4byte dummy + 8byte uniqueid
		cmd_dummy[0] = 0x4B;
		if (jedec_id == 0xef4019)
			cmd_dummy_len = flash->addr_width + 2;
		else
			cmd_dummy_len = 5;
		if (((jedec_id == 0x5e4017) && (flash->info->markid == ZB25VQ64B_C_D)) ||
				((jedec_id == 0x5e4016) && (flash->info->markid == ZB25VQ32D)) ||
				((jedec_id == 0x5e4018) && (flash->info->markid == ZB25VQ128D)))
			len = 16;
		else
			len = 8;
		break;
	case 0x85: /* p25q */
	case 0xb3: /* ucun */
	case 0xc8: /* gd */
	case 0xcd: /* th25q */
	case 0xe5: /* Doslicon */
		// 1byte cmd(0x4B) + 4byte dummy + 16byte uniqueid
		cmd_dummy[0] = 0x4B;
		cmd_dummy_len = 5;
		len = 16;
		break;
	case 0x1c: /* eno */
		if (jedec_id == 0x1c7018 || jedec_id == 0x1c7017   /* EN25QH128,EN25QH64A */
				|| jedec_id == 0x1c3016 || jedec_id == 0x1c6018   /* en25q32 */
				|| jedec_id == 0x1c7118) { /* en25qx128a */
			// 1byte cmd(0x5A) + 3byte addr + 1byte dummy + 12byte uniqueid
			cmd_dummy[0] = 0x5A;
			cmd_dummy[1] = 0x00;
			cmd_dummy[2] = 0x00;
			cmd_dummy[3] = 0x80;
			cmd_dummy_len = 5;
			len = 12;
		} else if (jedec_id == 0x1c7117) {
			cmd_dummy[0] = 0x5A;
			cmd_dummy[1] = 0x00;
			cmd_dummy[2] = 0x01;
			cmd_dummy[3] = 0xE0;
			cmd_dummy_len = 5;
			len = 12;
		} else if (jedec_id == 0x1c4018) {
			cmd_dummy[0] = 0x5A;
			cmd_dummy[1] = 0x00;
			cmd_dummy[2] = 0x00;
			cmd_dummy[3] = 0xF8;
			cmd_dummy_len = 5;
			len = 12;
		} else if (jedec_id == 0x1c4116) { /* en25qe32a */
			cmd_dummy[0] = 0x4B;
			cmd_dummy_len = 5;
			len = 16;
		}
		break;
	case 0x20: /* xmc */
		if(jedec_id == 0x207018 || jedec_id == 0x207017) { /* x25f128a xmc */
			// 1byte cmd(0x5A) + 3byte addr + 1byte dummy + 16byte uniqueid
			cmd_dummy[0] = 0x5A;
			cmd_dummy[1] = 0x00;
			cmd_dummy[2] = 0x00;
			cmd_dummy[3] = 0x80;
			cmd_dummy_len = 5;
			len = 16;
			break;
		} else {
			cmd_dummy[0] = 0x4B;
			cmd_dummy_len = 5;
			if ((jedec_id == 0x204015)     ||   //xm25qh16c
				((jedec_id == 0x204016) && (flash->info->markid == XM25QH32B_XM25QH32C_XM25QE32C)) ||   //xm25qh32b/xm25qh32c
				((jedec_id == 0x204017) && (flash->info->markid == XM25QH64C)) ||   //xm25qh64c
				((jedec_id == 0x204018) && (flash->info->markid == XM25QH128C)) ||  //xm25qh128c
				(jedec_id == 0x204019))     //xm25qh256c
				// 1byte cmd(0x4B) + 4byte dummy + 8byte uniqueid
				len = 8;
			else if ((jedec_id == 0x206017) || /* xm25qh64b */
				((jedec_id == 0x204017) && (flash->info->markid == XM25QH64D)) ||   //xm25qh64d
				((jedec_id == 0x204016) && (flash->info->markid == XM25QH32D)) ||   //xm25qh32d
				((jedec_id == 0x204018) && (flash->info->markid == XM25QH128D)) ||  //xm25qh128d
				 (jedec_id == 0x206018))   /* xm25qh128b */
				len = 16;
			break;
		}
		// 1byte cmd(0x9F) + 1byte mid + 2byte did + 17byte uniqueid
		cmd_dummy[0] = 0x9F;
		cmd_dummy_len = 4;
		len = 17;
		break;
	case 0x0b: /* xtx */
		if (((jedec_id == 0x0b4017) && (flash->info->markid == XT25F64F)) ||
			jedec_id == 0x0b4018) {
			cmd_dummy[0] = 0x4B;
		} else {
			cmd_dummy[0] = 0x5A;
			cmd_dummy[1] = 0x00;
			cmd_dummy[2] = 0x01;
			cmd_dummy[3] = 0x94;
		}
		cmd_dummy_len = 5;
		len = 16;
		break;
	case 0xc2: /* mxic */
		cmd_dummy[0] = 0x5A;
		cmd_dummy[1] = 0x00;
		cmd_dummy[2] = 0x01;
		cmd_dummy[3] = 0xe0;
		cmd_dummy[4] = 0x00;
		cmd_dummy_len = 5;
		len = 16;
		break;
	case 0x8f: /* sx */
		cmd_dummy[0] = 0x5A;
		cmd_dummy[1] = 0x00;
		cmd_dummy[2] = 0x00;
		cmd_dummy[3] = 0xF8;
		cmd_dummy_len = 5;
		len = 8;
		break;
	default:
		return -1;
	}

	if (buf_len < len)
		return -1;
	*id_len = len;
	spi_write_then_read(spi, cmd_dummy, cmd_dummy_len, buf, len);
	return 0;
}

/*
 * Enable/disable 4-byte addressing mode.
 */
static inline int set_4byte(struct spi_nor_flash *flash, int enable)
{
	struct spi_device *spi = flash->spi;
	unsigned int jedec_id = flash->info->jedec_id;
	switch (JEDEC_MFR(jedec_id)) {
		case 0xC2 /* MXIC */:
		case 0xEF /* winbond */:
		case 0x20 /* XMC */:
			flash->command[0] = enable ? GX_CMD_EN4B : GX_CMD_EX4B;
			if(enable) {
				return spi_write(spi, flash->command, 1);
			}
			else {
				spi_write(spi, flash->command, 1);
				flash->command[0] = GX_CMD_WREN;
				spi_write(spi, flash->command, 1);
				flash->command[0] = GX_CMD_WREAR;
				flash->command[1] = 0x00;
				spi_write(spi, flash->command, 2);
				return 0;
			}
		default:
			/* Spansion style */
			flash->command[0] = GX_CMD_BRWR;
			flash->command[1] = enable << 7;
			return spi_write(spi, flash->command, 2);
	}
}

void spi_nor_set_4byte(struct spi_nor_flash *flash)
{
	set_4byte(flash, 1);
}

//switch from 4-Byte to 3-Byte Address Mode
void spi_nor_exit_4byte(struct spi_nor_flash *flash)
{
	if(flash->size > 0x1000000) {
		set_4byte(flash, 0);
	}
}

static void spi_nor_addr2cmd(struct spi_nor_flash *flash, unsigned int addr, unsigned char *cmd)
{
	/* opcode is in cmd[0] */
	cmd[1] = addr >> (flash->addr_width * 8 -  8);
	cmd[2] = addr >> (flash->addr_width * 8 - 16);
	cmd[3] = addr >> (flash->addr_width * 8 - 24);
	cmd[4] = addr >> (flash->addr_width * 8 - 32);
}

static inline int spi_nor_cmdsz(struct spi_nor_flash *flash)
{
	return 1 + flash->addr_width;
}

int spi_nor_detect(struct spi_nor_flash *flash)
{
	int i = 0;
	int jedec = 0;

	if(spi_nor_readid(flash->spi, &jedec) != 0)
		return -1;

	if (jedec == 0 || jedec == 0xFFFFFF)
		return -1;

	for(i = 0; i< g_spi_nor_data_count; i++){
		if(jedec == g_spi_nor_data[i].jedec_id) {
			int markid_ret = 0;
			if (g_spi_nor_data[i].get_flash_markid != NULL) {
				markid_ret = g_spi_nor_data[i].get_flash_markid(flash->spi, g_spi_nor_data[i].jedec_id, g_spi_nor_data[i].markid);
			}
			if (markid_ret == -1)
				continue;

			flash->size  = g_spi_nor_data[i].size;
			flash->info  = &g_spi_nor_data[i];
			flash->addr_width = (flash->size > 0x1000000) ?\
						(spi_nor_set_4byte(flash), 4) : 3;

			if (flash->info->write_protect == NULL || flash->info->write_protect->sf_range == NULL)
				flash->protect_mode = GX_FLASH_PROTECT_MODE_NONSUPPORT;
			else
				flash->protect_mode = GX_FLASH_PROTECT_MODE_BOTTOM;

			if (markid_ret == 0) {
				flash->match_mode = GX_FLASH_MATCH_MODE_ID;
			}
			else {
				flash->match_mode = GX_FLASH_MATCH_MODE_BLUR;
			}
			break;
		}
	}

	if (i == g_spi_nor_data_count) {
		flash->size = g_spi_nor_data[0].size;
		flash->info = &g_spi_nor_data[0];
		flash->addr_width = 3;
		flash->protect_mode = GX_FLASH_PROTECT_MODE_NONSUPPORT;
		flash->match_mode = GX_FLASH_MATCH_MODE_NONSUPPORT;
		gxlog_w("[Warning]: Unsupported flash type, id: 0x%x. The default configuration will be used\n", jedec);
	}

	gxlog_i("NOR Flash, model: %s, size: %d MB\n", flash->info->name, flash->size>>20);

	return 0;
}

static int spi_nor_enable_quad_0(struct spi_device *spi)
{
	unsigned char status = read_sr2(spi);

	if (status & GX_NOR_SR2_QE)
		return 0;

	status |= GX_NOR_SR2_QE;
	write_sr(spi, &status, 1, 2);
	wait_till_ready(spi);

	return 0;
}

static int spi_nor_enable_quad_1(struct spi_device *spi)
{
	unsigned char status[2];

	status[0] = read_sr1(spi);
	status[1] = read_sr2(spi);

	if (status[1] & GX_NOR_SR2_QE)
		return 0;

	status[1] |= GX_NOR_SR2_QE;
	write_sr(spi, status, 2, 1);
	wait_till_ready(spi);

	return 0;
}

static int spi_nor_enable_quad_2(struct spi_device *spi)
{
	unsigned char status = read_sr1(spi);

	if (status & GX_NOR_SR1_QE)
		return 0;

	status |= GX_NOR_SR1_QE;
	write_sr(spi, &status, 1, 1);
	wait_till_ready(spi);

	return 0;
}

int spi_nor_enable_quad(struct spi_nor_flash *flash)
{
	switch(flash->info->jedec_id) {
		case 0x000b4016:        //XT25F32B-S
		case 0x000b4017:        //XT25F64B-S
		case 0x000b4018:        //XT25F128F
		case 0x005e6015:        //ZB25VQ16A
		case 0x00494016:        //F25SQA032M
		case 0x00ba4017:        //ZD25Q64A
			spi_nor_enable_quad_1(flash->spi);
			break;
		case 0x00c84015:        //GD25Q16
		case 0x00c84016:        //GD25Q32
			spi_nor_enable_quad_0(flash->spi);
			spi_nor_enable_quad_1(flash->spi);
			break;
		case 0x00c22017:        //KH25L6436F
		case 0x00c22018:        //MX25L128/KH25L128
			spi_nor_enable_quad_2(flash->spi);
		default:
			spi_nor_enable_quad_0(flash->spi);
			break;
	}

	return 0;
}

int spi_nor_chiperase(struct spi_device *spi)
{
	unsigned char command[1];

	wait_till_ready(spi);
	write_enable(spi);

	command[0] = GX_CMD_CE;
	spi_write(spi, command, 1);

	wait_till_ready(spi);
	command[0] = GX_CMD_WRDI;
	spi_write(spi, command, 1);

	return 0;
}

void spi_nor_sectorerase(struct spi_nor_flash *flash, unsigned int addr)
{
	wait_till_ready(flash->spi);
	write_enable(flash->spi);

	flash->command[0]=GX_CMD_SE;
	spi_nor_addr2cmd(flash,addr,flash->command);
	spi_write(flash->spi, flash->command, spi_nor_cmdsz(flash));
}

void spi_nor_blockerase(struct spi_nor_flash *flash, unsigned int addr)
{
	wait_till_ready(flash->spi);
	write_enable(flash->spi);

	flash->command[0]=GX_CMD_BE;
	spi_nor_addr2cmd(flash,addr,flash->command);
	spi_write(flash->spi, flash->command, spi_nor_cmdsz(flash));
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * SPI_FLASH_PAGE_SIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
int spi_nor_write(struct spi_nor_flash *flash, unsigned int to, unsigned char *buf, unsigned int len)
{
	unsigned int page_offset, page_size;
	struct spi_transfer t[2];
	struct spi_message m;
	int retlen = 0;

	struct spi_device *spi = flash->spi;

	/* sanity checks */
	if (!len)
		return 0;

	if (to + len > flash->size)
		return -EINVAL;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = spi_nor_cmdsz(flash);
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = buf;
	if ((flash->spi->mode & SPI_TX_QUAD) && (flash->info->flag & SPI_NOR_QUAD_WRITE))
		t[1].tx_nbits = SPI_NBITS_QUAD;
	spi_message_add_tail(&t[1], &m);

	/* Wait until finished previous write command. */
	wait_till_ready(spi);

	write_enable(spi);

	/* Set up the opcode in the write buffer. */
	if ((flash->spi->mode & SPI_TX_QUAD) && (flash->info->flag & SPI_NOR_QUAD_WRITE))
		flash->command[0] = GX_CMD_QUAD_PP;
	else
		flash->command[0] = GX_CMD_PP;

	spi_nor_addr2cmd(flash, to, flash->command);

	/* what page do we start with? */
	page_offset = to % SPI_FLASH_PAGE_SIZE;

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= SPI_FLASH_PAGE_SIZE) {
		t[1].len = len;
		spi_sync(spi, &m);
		retlen = m.actual_length - spi_nor_cmdsz(flash);
	} else {
		unsigned int i;

		/* the size of data remaining on the first page */
		page_size = SPI_FLASH_PAGE_SIZE - page_offset;

		t[1].len = page_size;
		spi_sync(spi, &m);
		retlen = m.actual_length - spi_nor_cmdsz(flash);

		/* write everything in PAGESIZE chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > SPI_FLASH_PAGE_SIZE)
				page_size = SPI_FLASH_PAGE_SIZE;

			/* write the next page to flash */
			spi_nor_addr2cmd(flash, to + i, flash->command);

			t[1].tx_buf = buf + i;
			t[1].len = page_size;

			wait_till_ready(spi);
			write_enable(spi);
			spi_sync(spi, &m);
			if (retlen)
				retlen += m.actual_length - spi_nor_cmdsz(flash);
		}
	}
	return retlen;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
int spi_nor_normal_read(struct spi_nor_flash *flash, unsigned int from, unsigned char *buf,unsigned int len)
{
	struct spi_transfer t[2];
	struct spi_message m;
	int retlen = 0;

	struct spi_device *spi;

	spi = flash->spi;

	/* sanity checks */
	if (!len)
		return 0;

	if (from + len > flash->size)
		return -EINVAL;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = spi_nor_cmdsz(flash);
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = len;
	spi_message_add_tail(&t[1], &m);

	/* Wait till previous write/erase is done. */
	wait_till_ready(spi);

	/* Set up the write data buffer. */
	flash->command[0] = GX_CMD_READ;
	spi_nor_addr2cmd(flash, from, flash->command);

	spi_sync(spi, &m);

	retlen = m.actual_length - spi_nor_cmdsz(flash);
	return retlen;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
int spi_nor_fast_read(struct spi_nor_flash *flash, unsigned int from, unsigned char *buf,unsigned int len)
{
	struct spi_transfer t[2];
	struct spi_message m;
	int retlen = 0;

	struct spi_device *spi;

	spi = flash->spi;

	/* sanity checks */
	if (!len)
		return 0;

	if (from + len > flash->size)
		return -EINVAL;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	/* 1 byte dummy */
	t[0].len = spi_nor_cmdsz(flash) + 1;
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = len;
	spi_message_add_tail(&t[1], &m);

	/* Wait till previous write/erase is done. */
	wait_till_ready(spi);

	/* Set up the write data buffer. */
	flash->command[0] = GX_CMD_FAST_READ;
	spi_nor_addr2cmd(flash, from, flash->command);

	spi_sync(spi, &m);

	retlen = m.actual_length - spi_nor_cmdsz(flash)-1;
	return retlen;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
int spi_nor_dual_read(struct spi_nor_flash *flash, unsigned int from, unsigned char *buf,unsigned int len)
{
	struct spi_transfer t[2];
	struct spi_message m;
	int retlen = 0;

	struct spi_device *spi;

	spi = flash->spi;

	/* sanity checks */
	if (!len)
		return 0;

	if (from + len > flash->size)
		return -EINVAL;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	/* 1 byte dummy */
	t[0].len = spi_nor_cmdsz(flash) + 1;
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = len;
	t[1].rx_nbits = SPI_NBITS_DUAL;
	spi_message_add_tail(&t[1], &m);

	/* Wait till previous write/erase is done. */
	wait_till_ready(spi);

	/* NOTE:  OPCODE_FAST_READ (if available) is faster... */

	/* Set up the write data buffer. */
	memset(flash->command, 0, CMD_SIZE);
	flash->command[0] = GX_CMD_DREAD;
	spi_nor_addr2cmd(flash, from, flash->command);
	spi_sync(spi, &m);

	retlen = m.actual_length - spi_nor_cmdsz(flash)-1;
	return retlen;
}

int spi_nor_quad_read(struct spi_nor_flash *flash, unsigned int from, unsigned char *buf,unsigned int len)
{
       struct spi_transfer t[2];
       struct spi_message m;
       int retlen = 0;

       struct spi_device *spi;

       spi = flash->spi;

       /* sanity checks */
       if (!len)
               return 0;

       if (from + len > flash->size)
               return -EINVAL;

       spi_message_init(&m);
       memset(t, 0, (sizeof t));

       t[0].tx_buf = flash->command;
       /* 1 byte dummy */
       t[0].len = spi_nor_cmdsz(flash) + 1;
       spi_message_add_tail(&t[0], &m);

       t[1].rx_buf = buf;
       t[1].len = len;
       t[1].rx_nbits = SPI_NBITS_QUAD;
       spi_message_add_tail(&t[1], &m);

       /* Wait till previous write/erase is done. */
       wait_till_ready(spi);

       /* NOTE:  OPCODE_FAST_READ (if available) is faster... */

       /* Set up the write data buffer. */
       memset(flash->command, 0, CMD_SIZE);
       flash->command[0] = GX_CMD_QREAD;
       spi_nor_addr2cmd(flash, from, flash->command);
       spi_sync(spi, &m);

       retlen = m.actual_length - spi_nor_cmdsz(flash)-1;
       return retlen;
}


int spi_nor_read(struct spi_nor_flash *flash, unsigned int addr,unsigned char *to, unsigned int len)
{
	int retlen = 0;

	if ((flash->info->flag & SPI_NOR_QUAD_READ) && (flash->spi->mode & SPI_RX_QUAD)) {
		retlen = spi_nor_quad_read(flash, addr, to, len);
	} else if ((flash->info->flag & SPI_NOR_DUAL_READ) && (flash->spi->mode & SPI_RX_DUAL)) {
		retlen = spi_nor_dual_read(flash, addr, to, len);
	} else {
		retlen = spi_nor_fast_read(flash, addr, to, len);
	}
	return retlen;
}

static int spi_nor_calcblock(struct spi_nor_flash *flash, unsigned int addr, unsigned int *pstart, unsigned int *pend)
{
	int i;
	unsigned int start = 0, end = 0;
	unsigned int erase_num = flash->size/SPI_FLASH_ERASE_SIZE;

	*pend = 0;
	*pstart = 0;

	for (i = 0; i < erase_num; ++i) {
		end += SPI_FLASH_ERASE_SIZE;
		if (addr >= start && addr < end) {
			*pstart = start;
			*pend = end;
			return 0;
		}
		start += SPI_FLASH_ERASE_SIZE;
	}

	return -1;
}

int spi_nor_erase(struct spi_nor_flash *flash, unsigned int addr, unsigned int len)
{
	unsigned int erase_start, erase_count, erase_end;

	if(addr >= flash->size)
		return -EINVAL;

	erase_end = addr + len;
	if(erase_end > flash->size)
		erase_end = flash->size;

	erase_start = (addr / SPI_FLASH_ERASE_SIZE) * SPI_FLASH_ERASE_SIZE;
	erase_count = erase_end - erase_start;

	while(erase_count > 0) {
		if (erase_count < SPI_FLASH_ERASE_SIZE)
			erase_count = SPI_FLASH_ERASE_SIZE;

#ifdef CONFIG_GX_MTD_SPI_NOR_USE_4K_SECTORS
		if (((erase_start % SPI_FLASH_BLOCK_SIZE)==0) && (erase_count >= SPI_FLASH_BLOCK_SIZE)) { // block 对齐
			spi_nor_blockerase(flash,erase_start);
			erase_start += SPI_FLASH_BLOCK_SIZE;
			erase_count -= SPI_FLASH_BLOCK_SIZE;
		}
		else {
			spi_nor_sectorerase(flash,erase_start);
			erase_start += SPI_FLASH_SECTOR_SIZE;
			erase_count -= SPI_FLASH_SECTOR_SIZE;
		}
#else
		spi_nor_blockerase(flash,erase_start);
		erase_start += SPI_FLASH_BLOCK_SIZE;
		erase_count -= SPI_FLASH_BLOCK_SIZE;
#endif
	}

	return 0;
}

static int spi_nor_calc_range(struct spi_nor_flash *flash)
{
	unsigned int i, jedec_id, protect_len = -1;
	struct spi_nor_flash_protect_range tmp_range;
	unsigned char tmp_status[2] = {0, 0};
	unsigned char status[3];

	struct spi_device *spi = flash->spi;

	if (flash->info->write_protect == NULL || flash->info->write_protect->sf_range == NULL)
		return -1;

	wait_till_ready(spi);
	jedec_id = flash->info->jedec_id;
	switch (JEDEC_MFR(jedec_id)) {
		case 0x5e:  //zb25vq32/t25s32
		case 0xef:  //winbond
		case 0xe0:  //paragon
		case 0x01:  //spansion
		case 0xf8:  //fm25q64a dosilicon
		case 0xa1:  //fudan
		case 0x49:  //foresee
		case 0x0b:  //xtx
		case 0x68:  //BYT
		case 0x85:  //p25q*
		case 0xba:  //zetta
		case 0xc8:  //Gigadevice
		case 0xb3:  //ucun
		case 0x8f:  //sx
		case 0xc4:  //gt
		case 0xcd:  //th25q
		case 0xe5: /* Doslicon */
		case 0x94: /* bw */
			tmp_status[0] = read_sr1(spi);
			tmp_status[1] = read_sr2(spi);
			break;
		case 0xc2:  //mxic
			tmp_status[0] = read_sr1(spi);
			if ((flash->info->write_protect->count > 0) && (flash->info->write_protect->sf_range[0].mask2 != 0)) {
				tmp_status[1] = read_sr3(spi);
			}
			break;
		case 0x1c:  //eon
		case 0x8c:  //esmt
		case 0x20:  //micron/numonyx/xt/xmc
		case 0x7F:  //PMC
			tmp_status[0] = read_sr1(spi);

			//en25qh32b
			//en25qa64a
			//en25qa128a
			//pn25f64b
			//xm25qh64a
			//xm25qh128a
			if ((jedec_id == 0x1c6017) || (jedec_id == 0x1c6018) ||
				(jedec_id == 0x1c7016) || (jedec_id == 0x1c7017) ||
				(jedec_id == 0x207017) || (jedec_id == 0x207018)) {
				spi_nor_otp_entry(flash, 1);
				tmp_status[1] = read_sr1(spi);
				spi_nor_otp_entry(flash, 0);
			} else if ((jedec_id == 0x204015) || //xm25qh16c
					   (jedec_id == 0x204016) || //xm25qh32b/xm25qh32c
					   (jedec_id == 0x204017) || //xm25qh64c
					   (jedec_id == 0x204018) || //xm25qh128c
					   (jedec_id == 0x204019) || //xm25qh256c
					   (jedec_id == 0x1c4116) || //en25qe32a
					   (jedec_id == 0x1c7117) || //en25qx64a
					   (jedec_id == 0x1c7118) || //en25qx128a
					   (jedec_id == 0x1c4018)) { //jy25vq128a
				tmp_status[1] = read_sr2(spi);
			} else if ((jedec_id == 0x206017) || //xm25qh64b
					   (jedec_id == 0x206018)) { //xm25qh128b
				read_function_sr(spi, &tmp_status[1]);
			}

			break;
		case 0x9d: //ISSI
			tmp_status[0] = read_sr1(spi);
			if ((flash->info->write_protect->count > 0) && (flash->info->write_protect->sf_range[0].mask2 != 0)) {
				status[0] = GX_CMD_RDFR;
				write_enable(spi);
				spi_write_then_read(spi, &status[0], 1, &tmp_status[1], 1);
			}
			break;
		default:
			gxlog_w("gxspi_flash_calc_range switch default, please check.\n");
			break;
	}
	for(i = 0; i < flash->info->write_protect->count; i++){
		tmp_range = flash->info->write_protect->sf_range[i];

		if(((tmp_status[0] & tmp_range.mask1) == tmp_range.status1) &&
			((tmp_status[1] & tmp_range.mask2) == tmp_range.status2)){
			protect_len = tmp_range.wp_len;
			break;
		}
	}

	return protect_len;
}

// get flash write protect status
int spi_nor_write_protect_status(struct spi_nor_flash *flash, unsigned int *len)
{
	*len = 0;
	*len = spi_nor_calc_range(flash);
	if(-1 == *len)
		return -1;

	return 0;
}

int spi_nor_write_protect_mode(struct spi_nor_flash *flash)
{
	if (flash->info->write_protect == NULL || flash->info->write_protect->sf_range == NULL)
		return -1;

	return WP_BOTTOM;
}

// lock the flash start from address of 0 to protect_addr.
int spi_nor_write_protect_lock(struct spi_nor_flash *flash, unsigned int protect_len, unsigned int *real_protect_len)
{
	unsigned int i, flash_size;
	unsigned char tmp_status[2] = {0, 0};
	unsigned char dev_status[2] = {0, 0};
	unsigned char dev_mask[2]   = {0, 0};
	unsigned char status[3] = {0};
	int count;

	struct spi_device *spi = flash->spi;
	struct spi_nor_flash_protect_range *tmp_sf_rangs = NULL;
	unsigned int jedec_id;
	unsigned int real_len;
#define XM25QH128A_BT_OTP 0x08

	*real_protect_len = 0;
	jedec_id = flash->info->jedec_id;
	flash_size = flash->size;

	if(protect_len > flash_size){
		gxlog_e("ERROR: protect_len > flashsize\n");
		return -1;
	}

	if (flash->info->write_protect == NULL || flash->info->write_protect->sf_range == NULL) {
		if (!protect_len)
			return 0;
		return -1;
	}

	wait_till_ready(spi);

	tmp_sf_rangs = flash->info->write_protect->sf_range;

	count = flash->info->write_protect->count;
	for(i = 0; i < count; i++){
		if(protect_len >= tmp_sf_rangs[i].wp_len){
			dev_status[0] = tmp_sf_rangs[i].status1;
			dev_status[1] = tmp_sf_rangs[i].status2;
			dev_mask[0] = tmp_sf_rangs[i].mask1;
			dev_mask[1] = tmp_sf_rangs[i].mask2;
		}else
			break;
	}

	switch (JEDEC_MFR(jedec_id)) {
		case 0x5e: // zb25vq32/t25s32
		case 0xef: // winbond
		case 0xa1: // fudan
		case 0x49: // foresee
		case 0x68: // BYT
		case 0x85: // p25q*
		case 0xb3: // ucun
		case 0x8f: // sx
		case 0xc4: // gt
		case 0xc8: // Gigadevice
		case 0xcd: // th25q
		case 0xe5: /* Doslicon */
		case 0x94: /* bw */
			if(dev_mask[0] != 0){
				tmp_status[0] = read_sr1(spi);
				tmp_status[0] &= ~dev_mask[0];
				tmp_status[0] |= dev_status[0];
				write_sr(spi, &tmp_status[0], 1, 1);
				wait_till_ready(spi);
			}

			if(dev_mask[1] != 0){
				tmp_status[1] = read_sr2(spi);
				tmp_status[1] &= ~dev_mask[1];
				tmp_status[1] |= dev_status[1];
				wait_till_ready(spi);
				write_sr(spi, &tmp_status[1], 1, 2);
			}
			break;
		case 0x01: //spansion
		case 0xe0: //paragon
		case 0xba: //zetta
		case 0x0b: // xtx
		case 0xf8: //fm25q64a dosilicon
			tmp_status[0] = read_sr1(spi);
			tmp_status[0] &= ~dev_mask[0];
			tmp_status[0] |= dev_status[0];

			tmp_status[1] = read_sr2(spi);
			tmp_status[1] &= ~dev_mask[1];
			tmp_status[1] |= dev_status[1];

			write_sr(spi, &tmp_status[0], 2, 1);
			break;
		case 0xc2:  //mxic
			tmp_status[0] = read_sr1(spi);
			tmp_status[0] &= ~dev_mask[0];
			tmp_status[0] |= dev_status[0];

			if(dev_mask[1] != 0){
				tmp_status[1] = read_sr3(spi);
				tmp_status[1] &= ~dev_mask[1];
				tmp_status[1] |= dev_status[1];
				write_sr(spi, &tmp_status[0], 2, 1);
			}else {
				write_sr(spi, &tmp_status[0], 1, 1);
			}
			break;
		case 0x1c:  //eon
		case 0x8c:  //esmt
		case 0x20:  //micron/numonyx/xt/xmc
		case 0x7F:  //PMC
			if (jedec_id == 0x207018) {  // xm25qh128a
				spi_nor_otp_entry(flash, 1);
				tmp_status[1] = read_sr1(spi);
				spi_nor_otp_entry(flash, 0);

				if (tmp_status[1] & XM25QH128A_BT_OTP) {
					gxlog_e("ERROR: xm25qh128a BT(OTP) bit was set\n");
					return -1;
				}
			}

			tmp_status[0] = read_sr1(spi);
			tmp_status[0] &= ~dev_mask[0];
			tmp_status[0] |= dev_status[0];
			write_sr(spi, &tmp_status[0], 1, 1);

			wait_till_ready(spi);

			//en25qh32b
			//en25qa64a
			//en25qa128a
			//pn25f64b
			//xm25qh64a
			//xm25qh128a
			if ((jedec_id == 0x1c6017) || (jedec_id == 0x1c6018) ||
				(jedec_id == 0x1c7016) || (jedec_id == 0x1c7017) ||
				(jedec_id == 0x207017) || (jedec_id == 0x207018)) {
				spi_nor_otp_entry(flash, 1);
				tmp_status[1] = read_sr1(spi);
				tmp_status[1] &= ~dev_mask[1];
				tmp_status[1] |= dev_status[1];
				write_sr(spi, &tmp_status[1], 1, 1);
				spi_nor_otp_entry(flash, 0);
			} else {
				tmp_status[1] = read_sr2(spi);
				tmp_status[1] &= ~dev_mask[1];
				tmp_status[1] |= dev_status[1];

				if ((jedec_id == 0x204015)     ||        //xm25qh16c
					(jedec_id == 0x204016) ||        //xm25qh32b/xm25qh32c
					(jedec_id == 0x204017) ||        //xm25qh64c
					(jedec_id == 0x204018) ||        //xm25qh128c
					(jedec_id == 0x204019) ||        //xm25qh256c
					(jedec_id == 0x1c4116) ||        //en25qe32a
					(jedec_id == 0x1c7117) ||        //en25qx64a
					(jedec_id == 0x1c7118) ||        //en25qx128a
					(jedec_id == 0x1c4018)) {        //jy25vq128a
					write_sr(spi, &tmp_status[1], 1, 2);
				} else if ((jedec_id == 0x206017) || //xm25qh64b
						   (jedec_id == 0x206018)) { //xm25qh128b
					write_function_sr(spi, tmp_status[1]);
				}
			}
			break;
		case 0x9d: //ISSI
			tmp_status[0] = read_sr1(spi);
			tmp_status[0] &= ~dev_mask[0];
			tmp_status[0] |= dev_status[0];
			write_sr(spi, &tmp_status[0], 1, 1);
			wait_till_ready(spi);

			if(dev_mask[1] != 0) {
				status[0] = GX_CMD_RDFR;
				status[2] = GX_CMD_WRFR;
				write_enable(spi);
				spi_write_then_read(spi, &status[0], 1, &status[1], 1);
				status[1] &= ~dev_mask[1];
				status[1] |= dev_status[1];
				spi_write(spi, status, 2);
			}
			break;
		default:
			gxlog_w("sflash_lock switch default, please check.\n");
			break;
	}

	wait_till_ready(spi);

	if (-1 == spi_nor_write_protect_status(flash, &real_len)){
		gxlog_e("get protect status error: [%s : %d] !\n", __func__, __LINE__);
		return -1;
	}

	*real_protect_len = real_len;

	gxlog_i("protect len: 0x%x\n", real_len);

	return 0;
}

int spi_nor_write_protect_generic_unlock(struct spi_nor_flash *flash)
{
	unsigned char dev_status, tb = 0, bp3 = 0, bp2_bp0 = 0x0;
	struct spi_device *spi = flash->spi;

	dev_status  = read_sr1(spi);
	dev_status &=~0x7c;
	dev_status |=((bp2_bp0<<2) | (tb << 5 ) | ( bp3 <<6) | (1<<7));
	write_sr(spi, &dev_status, 1, 1);

	return 0;
}

int spi_nor_set_lock_reg(struct spi_nor_flash *flash)
{
	struct spi_device *spi = flash->spi;
	unsigned char status[2];
	unsigned int jedec_id;

	jedec_id =flash->info->jedec_id;

	wait_till_ready(spi);
	switch (JEDEC_MFR(jedec_id)) {
		case 0xef: //winbond
			status[1] = read_sr2(spi);
			status[1] |= GX_STAT_SRL;
			write_sr(spi, &status[1], 1, 2);
			break;
		default:
			gxlog_w("sflash lock status switch default, please check.\n");
			return -1;
	}

	return 0;
}

int spi_nor_get_lock_reg(struct spi_nor_flash *flash, int *lock)
{
	struct spi_device *spi = flash->spi;
	unsigned char status[2];
	unsigned int jedec_id;

	jedec_id =flash->info->jedec_id;

	wait_till_ready(spi);
	switch (JEDEC_MFR(jedec_id)) {
		case 0xef: //winbond
			status[1] = read_sr2(spi);
			*lock = status[1] & GX_STAT_SRL;
			break;
		default:
			gxlog_w("sflash lock status switch default, please check.\n");
			return -1;
	}

	return 0;
}

void spi_nor_calcblockrange(struct spi_nor_flash *flash, unsigned int addr, unsigned int len, unsigned int *pstart, unsigned int *pend)
{
	unsigned int dummy;

	if (spi_nor_calcblock(flash, addr, pstart, &dummy) == 0)
		if (spi_nor_calcblock(flash, addr + len - 1, &dummy, pend) == 0)
			return ;
}

void spi_nor_sync(struct spi_device *spi)
{
	wait_till_ready(spi);
}

/***************************************** FLASH OTP *********************************************/

#define SET_REGION_FLAGS(flags,num) ((flags)=((flags)&(~0x07))|(num))
#define GET_REGION_FLAGS(flags)     ((flags)&0x07)

int spi_nor_otp_entry(struct spi_nor_flash *flash, unsigned int on)
{
	struct spi_device *spi = flash->spi;
	unsigned char cmd;

	wait_till_ready(spi);
	switch(JEDEC_MFR(flash->info->jedec_id))
	{
		case 0xc2 /* mxic */:
		case 0xef /* winbond */:
		case 0xf8 /* dosilicon fm25q64a */:
		case 0xba /* zd25q64b */:
			cmd = on ? 0xB1 : 0xC1;
			return spi_write(spi, &cmd, 1);
			break;
		case 0x20 /* xt25f128a xmc */:
		case 0x1c /* eon */:
			if(flash->info->jedec_id == 0x1c4116)
				break;
			cmd = on ? 0x3A : 0x04;
			return spi_write(spi, &cmd, 1);
			break;
		default /* gd pm paragon issi */:
			return 0;
	}
	return 0;
}

int spi_nor_otp_lock(struct spi_nor_flash *flash)
{
	struct spi_device *spi = flash->spi;
	unsigned char status[3];
	int region;

	if (flash->info->otp == NULL)
		return -1;

	region = GET_REGION_FLAGS(flash->info->otp->flags);

	wait_till_ready(spi);
	switch(JEDEC_MFR(flash->info->jedec_id))
	{
		case 0x5e /* zb25vq32/t25s32 */:
		case 0xef /* winbond */:
		case 0xe0 /* paragon */:
			status[0] = read_sr1(spi);
			status[1] = read_sr2(spi);
			status[1] |= 1<<(3 + region);
			write_sr(spi, status, 2, 1);
			break;
		case 0xc8 /* gd */:
		case 0x68 /* byt */:
		{
			unsigned char lock_bit = 0;

			if ((flash->info->jedec_id == 0xc84015) ||
					(flash->info->jedec_id == 0xc84215) ||
					((flash->info->jedec_id == 0xc84018) && (flash->info->markid == GD25Q128B)))
				lock_bit = 2;
			else
				lock_bit = (3 + region);

			if (flash->info->jedec_id == 0xc84215) {
				status[0] = read_sr1(spi);
				status[1] = read_sr2(spi);
				status[1] |= 1 << lock_bit;
				write_sr(spi, &status[0], 2, 1);
			} else {
				status[1] = read_sr2(spi);
				status[1] |= 1 << lock_bit;
				write_sr(spi, &status[1], 1, 2);
			}
			break;
		}
		case 0xc2 /* mxic */:
		case 0xf8 /* fm25q64a dosilicon */:
		case 0xba /* zd25q64b */:
			status[0] = 0x2F; //lock otp
			write_enable(spi);
			spi_write(spi, status, 1);
			break;
		case 0x20 /* xt25f128a xmc */:
		case 0x1c /* eon */:
			if ((flash->info->jedec_id == 0x204015) ||        //xm25qh16c
				(flash->info->jedec_id == 0x204016) ||    //xm25qh32b/xm25qh32c
				(flash->info->jedec_id == 0x204017) ||    //xm25qh64c
				(flash->info->jedec_id == 0x204018) ||    //xm25qh128c
				(flash->info->jedec_id == 0x204019) ||    //xm25qh256c
				(flash->info->jedec_id == 0x1c4116) ||    //en25qe32a
				(flash->info->jedec_id == 0x1c7117) ||    //en25qx64a
				(flash->info->jedec_id == 0x1c7118) ||    //en25qx128a
				(flash->info->jedec_id == 0x1c4018)) {    //jy25vq128a
				status[1] = read_sr2(spi);
				status[1] |= 1<<(3 + region);
				write_sr(spi, &status[1], 1, 2);
			} else if ((flash->info->jedec_id == 0x206017) || /* xm25qh64b */
				   (flash->info->jedec_id == 0x206018)) { /* xm25qh128b */
				read_function_sr(spi, &status[1]);
				status[1] |= 1<<(4 + region);
				write_function_sr(spi, status[1]);
			} else {
				spi_nor_otp_entry(flash, 1);
				status[0] = read_sr1(spi);
				status[0] |= 1<<7;
				write_sr(spi, &status[0], 1, 1);
				wait_till_ready(spi);
				spi_nor_otp_entry(flash, 0);
			}
			break;
		case 0x7f /* pmc */:
			status[0] = 0;
			if(flash->info->jedec_id == 0x7f9d46){  /* pm25lq032c */
				flash->info->otp->region_size = 65;
				spi_nor_otp_write(flash, 64, &status[0], 1);
				flash->info->otp->region_size = 64;
			}
			break;
		case 0x9d /* issi */:
			status[0] = GX_CMD_WRFR;
			status[2] = GX_CMD_RDFR;
			write_enable(spi);
			spi_write_then_read(spi, &status[2], 1, &status[1], 1);
			status[1] |= 1 << (4 + region);
			spi_write(spi, status, 2);
			break;
		case 0xa1 /* fudan */:
		case 0x0b /* xtx */:
			status[0] = read_sr1(spi);
			status[1] = read_sr2(spi);
			if ((flash->info->jedec_id == 0x0b4016) || (flash->info->jedec_id == 0x0b4017)
					|| (flash->info->jedec_id == 0xa12816)
					|| (flash->info->jedec_id == 0xa14017)
					|| (flash->info->jedec_id == 0xa14018)) {
				if ((flash->info->markid == XT25F32F) || (flash->info->markid == XT25F64F))
					status[1] |= 1 << (4 + region);
				else
					status[1] |= 1<<2;
			} else if (flash->info->jedec_id == 0x0b4018)
					status[1] |= 1 << (3 + region);
			else
				status[1] |= 1<<(2 + region);
			write_sr(spi, status, 2, 1);
			break;
		case 0x85 /* p25q* */:
		case 0xb3 /* ucun */:
		case 0x8f /* sx */:
		case 0xc4 /* gt */:
		case 0xcd /* th25q */:
		case 0xe5: /* Doslicon */
		case 0x94: /* bw */
			status[1] = read_sr2(spi);
			status[1] |= 1 << (3 + region);
			write_sr(spi, &status[1], 1, 2);
			break;
		default:
			return -1;
	}

	return 0;
}

int spi_nor_otp_status(struct spi_nor_flash *flash, unsigned char *buf)
{
	struct spi_device *spi = flash->spi;
	unsigned char status[3];
	unsigned int region;
	unsigned char otp_lock = 0;

	if (flash->info->otp == NULL)
		return -1;

	region = GET_REGION_FLAGS(flash->info->otp->flags);

	wait_till_ready(spi);
	switch(JEDEC_MFR(flash->info->jedec_id))
	{
		case 0x5e /* zb25vq32/t25s32 */:
		case 0xef /* winbond */:
		case 0xe0 /* paragon */:
		case 0x85 /* p25q* */:
		case 0xb3 /* ucun */:
		case 0x8f /* sx */:
		case 0xc4 /* gt */:
		case 0xcd /* th25q */:
		case 0xe5: /* Doslicon */
		case 0x94: /* bw */
			status[0] = read_sr2(spi);
			otp_lock = (status[0] >> (3 + region)) & 0x01;
			break;
		case 0xc8 /* gd */:
		case 0x68 /* byt */:
		{
			unsigned char lock_bit = 0;
			if ((flash->info->jedec_id == 0xc84015) ||
					(flash->info->jedec_id == 0xc84215) ||
					((flash->info->jedec_id == 0xc84018) && (flash->info->markid == GD25Q128B)))
				lock_bit = 2;
			else
				lock_bit = (3 + region);
			status[0] = read_sr2(spi);
			otp_lock = (status[0] >> lock_bit) & 0x01;
			break;
		}
		case 0xc2 /* mxic */:
		case 0xf8 /* fm25q64a dosilicon */:
		case 0xba /* zd25q64b */:
			status[0] = 0x2B; /* read otp status operation */
			spi_write_then_read(spi, &status[0], 1, &status[1], 1);
			otp_lock = (status[1] >> 1) & 0x01;
			break;
		case 0x20 /* xt25f128a xmc */:
		case 0x1c /* eon */:
			if ((flash->info->jedec_id == 0x204015) ||        //xm25qh16c
				(flash->info->jedec_id == 0x204016) ||    //xm25qh32b/xm25qh32c
				(flash->info->jedec_id == 0x204017) ||    //xm25qh64c
				(flash->info->jedec_id == 0x204018) ||    //xm25qh128c
				(flash->info->jedec_id == 0x204019) ||    //xm25qh256c
				(flash->info->jedec_id == 0x1c4116) ||    //en25qe32a
				(flash->info->jedec_id == 0x1c7117) ||    //en25qx64a
				(flash->info->jedec_id == 0x1c7118) ||    //en25qx128a
				(flash->info->jedec_id == 0x1c4018)) {    //jy25vq128a
				status[0] = read_sr2(spi);
				otp_lock = (status[0] >> (3 + region)) & 0x01;
			} else if ((flash->info->jedec_id == 0x206017) || /* xm25qh64b */
				   (flash->info->jedec_id == 0x206018)) { /* xm25qh128b */
				read_function_sr(spi, &status[0]);
				otp_lock = (status[0] >> (4 + region)) & 0x01;
			} else {
				spi_nor_otp_entry(flash, 1);
				status[0] = read_sr1(spi);
				otp_lock = (status[0] >> 7) & 0x01;
				wait_till_ready(spi);
				spi_nor_otp_entry(flash, 0);
			}
			break;
		case 0x7f /* pmc */:
			if(flash->info->jedec_id == 0x7f9d46){  /* pm25lq032c */
				flash->info->otp->region_size = 65;
				spi_nor_otp_read(flash, 64, &status[0], 1);
				flash->info->otp->region_size = 64;
				otp_lock = (~(status[0] & 0x01)) & 0x1 ;
			}
			break;
		case 0x9d /* issi */:
			status[0] = GX_CMD_RDFR;
			spi_write_then_read(spi, &status[0], 1, &status[1], 1);
			otp_lock = (status[1] >> (4 + region)) & 0x01;
			break;
		case 0xa1 /* fudan */:
		case 0x0b /* xtx */:
			status[0] = read_sr2(spi);
			if ((flash->info->jedec_id == 0x0b4016)
					|| (flash->info->jedec_id == 0x0b4017)
					|| (flash->info->jedec_id == 0xa12816)
					|| (flash->info->jedec_id == 0xa14017)
					|| (flash->info->jedec_id == 0xa14018)) {
				if ((flash->info->markid == XT25F32F) || (flash->info->markid == XT25F64F))
					otp_lock = (status[0] >> (4 + region)) & 0x01;
				else
					otp_lock = (status[0] >> 2) & 0x01;
			} else if (flash->info->jedec_id == 0x0b4018)
				otp_lock = (status[0] >> (3 + region)) & 0x01;
			else
				otp_lock = (status[0] >> (2 + region)) & 0x01;
			break;
		default:
			return -1;
	}
	*buf = otp_lock;

	return 0;
}

int spi_nor_otp_erase(struct spi_nor_flash *flash)
{
	struct spi_device *spi = flash->spi;
	unsigned int addr;
	unsigned int region;

	if (flash->info->otp == NULL)
		return -1;

	region = GET_REGION_FLAGS(flash->info->otp->flags);
	addr = flash->info->otp->start_addr + (region * flash->info->otp->skip_addr);
	switch(JEDEC_MFR(flash->info->jedec_id))
	{
		case 0x5e /* zb25vq32/t25s32 */:
		case 0xef /* winbond */:
		case 0xe0 /* paragon */:
		case 0xc8 /* gd */:
		case 0xa1 /* fudan */:
		case 0x0b /* xtx */:
		case 0x68 /* byt */:
		case 0x85 /* p25q */:
		case 0xb3 /* ucun */:
		case 0x8f /* sx */:
		case 0xc4 /* gt */:
		case 0xcd /* th25q */:
		case 0xe5: /* Doslicon */
		case 0x94: /* bw */
			flash->command[0] = 0x44;
			break;
		case 0x20 /* xt25f128a xmc */:
		case 0x1c /* eon */:
			if ((flash->info->jedec_id == 0x204015) || //xm25qh16c
				(flash->info->jedec_id == 0x204016) || //xm25qh32b/xm25qh32c
				(flash->info->jedec_id == 0x204017) || //xm25qh64c
				(flash->info->jedec_id == 0x204018) || //xm25qh128c
				(flash->info->jedec_id == 0x204019) || //xm25qh256c
				(flash->info->jedec_id == 0x1c4116) || //en25qe32a
				(flash->info->jedec_id == 0x1c7117) || //en25qx64a
				(flash->info->jedec_id == 0x1c7118) || //en25qx128a
				(flash->info->jedec_id == 0x1c4018))   //jy25vq128a
				flash->command[0] = 0x44;
			else if ((flash->info->jedec_id == 0x206017) || /* xm25qh64b */
				     (flash->info->jedec_id == 0x206018)) /* xm25qh128b */
				flash->command[0] = 0x64;
			else
				flash->command[0] = 0x20;
			break;
		case 0x9d /* issi */:
			flash->command[0] = 0x64;
			break;
		case 0xc2 /* mxic */: /* not support erase operation */
		case 0x7f /* pmc  */:
		case 0xf8 /* fm25q64a */:
		case 0xba /* zd25q64b */:
			return -1;
		default:
			return -1;
	}

	wait_till_ready(spi);

	/* set write enable ,the send erase cmd and addr*/
	write_enable(spi);
	spi_nor_otp_entry(flash, 1);
	spi_nor_addr2cmd(flash, addr, flash->command);
	spi_write(spi, flash->command, spi_nor_cmdsz(flash));
	wait_till_ready(spi);
	spi_nor_otp_entry(flash, 0);
	return 0;
}

int spi_nor_otp_write(struct spi_nor_flash *flash, unsigned int addr, unsigned char *buf, unsigned int len)
{
	struct spi_device *spi = flash->spi;
	struct spi_transfer t[2];
	struct spi_message m;
	unsigned int retlen = 0;
	unsigned int page_offset, page_size;
	unsigned int region;
	unsigned char cmd;

	if (flash->info->otp == NULL)
		return -1;

	region = GET_REGION_FLAGS(flash->info->otp->flags);

	if(!len)
		return 0;

	if((addr + len) > flash->info->otp->region_size)
		return -EINVAL;

	addr += flash->info->otp->start_addr + (region * flash->info->otp->skip_addr);

	switch(JEDEC_MFR(flash->info->jedec_id))
	{
		case 0x5e /* zb25vq32/t25s32 */:
		case 0xef /* winbond */:
		case 0xe0 /* paragon */:
		case 0xc8 /* gd */:
		case 0xa1 /* fudan */:
		case 0x0b /* xtx */:
		case 0x68 /* byt */:
		case 0x85 /* p25q */:
		case 0xb3 /* ucun */:
		case 0x8f /* sx */:
		case 0xc4 /* gt */:
		case 0xcd /* th25q */:
		case 0xe5: /* Doslicon */
		case 0x94: /* bw */
			cmd = 0x42;
			break;
		case 0x20 /* xt25f128a xmc */:
		case 0xc2 /* mxic */:
		case 0x1c /* eon */:
		case 0xf8 /* dosilicon fm25q64a */:
		case 0xba /* zd25q64b */:
			if ((flash->info->jedec_id == 0x204015) || //xm25qh16c
				(flash->info->jedec_id == 0x204016) || //xm25qh32b/xm25qh32c
				(flash->info->jedec_id == 0x204017) || //xm25qh64c
				(flash->info->jedec_id == 0x204018) || //xm25qh128c
				(flash->info->jedec_id == 0x204019) || //xm25qh256c
				(flash->info->jedec_id == 0x1c4116) || //en25qe32a
				(flash->info->jedec_id == 0x1c7117) || //en25qx64a
				(flash->info->jedec_id == 0x1c7118) || //en25qx128a
				(flash->info->jedec_id == 0x1c4018))   //jy25vq128a
				cmd = 0x42;
			else if ((flash->info->jedec_id == 0x206017) || /* xm25qh64b */
				   (flash->info->jedec_id == 0x206018)) /* xm25qh128b */
				cmd = 0x62;
			else
				cmd = 0x02;
			break;
		case 0x7f /* pmc */:
			cmd = 0xb1;
			break;
		case 0x9d /* issi */:
			cmd = 0x62;
			break;
		default:
			return -1;
	}

	spi_message_init(&m);
	memset(t, 0, sizeof(t));

	t[0].tx_buf = flash->command;
	t[0].len = spi_nor_cmdsz(flash);
	spi_message_add_tail(&t[0], &m);
	t[1].tx_buf = buf;
	spi_message_add_tail(&t[1], &m);

	/* Wait till previous write/erase is done. */
	wait_till_ready(spi);
	spi_nor_otp_entry(flash, 1);
	write_enable(spi);

	flash->command[0] = cmd;
	spi_nor_addr2cmd(flash, addr, flash->command);

	/*what page do we start write*/
	page_offset = addr % SPI_FLASH_PAGE_SIZE;

	/* all the write data in one page*/
	if(page_offset + len <= SPI_FLASH_PAGE_SIZE){
		t[1].len = len;
		spi_sync(spi, &m);
		retlen = m.actual_length - spi_nor_cmdsz(flash);
	}
	else{
		unsigned int i;
		/*write the remainder data on the first page*/
		page_size = SPI_FLASH_PAGE_SIZE - page_offset;
		t[1].len = page_size;
		spi_sync(spi, &m);
		retlen = m.actual_length - spi_nor_cmdsz(flash);

		/* write everything in pagesize chunks*/
		for(i = page_size; i < len; i += page_size){
			page_size = len - i;
			if(page_size > SPI_FLASH_PAGE_SIZE)
				page_size = SPI_FLASH_PAGE_SIZE;

			/* write next page to flash*/
			spi_nor_addr2cmd(flash, addr + i, flash->command);

			t[1].tx_buf = buf + i;
			t[1].len = page_size;

			wait_till_ready(spi);
			write_enable(spi);
			spi_sync(spi, &m);
			if(retlen)
				retlen += m.actual_length - spi_nor_cmdsz(flash);
		}
	}
	wait_till_ready(spi);
	spi_nor_otp_entry(flash, 0);

	return retlen;
}

int spi_nor_otp_read(struct spi_nor_flash *flash, unsigned int addr, unsigned char *buf, unsigned int len)
{
	struct spi_device *spi = flash->spi;
	struct spi_transfer t[2];
	struct spi_message m;
	unsigned int retlen = 0;
	unsigned int region;
	unsigned char cmd;
	unsigned char dummy;

	if (flash->info->otp == NULL)
		return -1;

	region = GET_REGION_FLAGS(flash->info->otp->flags);

	if(!len)
		return 0;

	if((addr + len) > flash->info->otp->region_size)
		return -EINVAL;

	addr += flash->info->otp->start_addr + (region * flash->info->otp->skip_addr);


	switch(JEDEC_MFR(flash->info->jedec_id))
	{
		case 0x5e /* zb25vq32/t25s32 */:
		case 0xef /* winbond */:
		case 0xe0 /* paragon */:
		case 0xc8 /* gd */:
		case 0xa1 /* fudan */:
		case 0x0b /* xtx */:
		case 0x68 /* byt */:
		case 0x85 /* p25q */:
		case 0xb3 /* ucun */:
		case 0x8f /* sx */:
		case 0xc4 /* gt */:
		case 0xcd /* th25q */:
		case 0xe5: /* Doslicon */
		case 0x94: /* bw */
			cmd = 0x48;
			dummy = 1;
			break;
		case 0x20 /* xt25f128a xmc */:
		case 0xc2 /* mxic */:
		case 0x1c /* eon */:
		case 0xf8 /* fm25q64a dosilicon */:
		case 0xba /* zd25q64b */:
			if ((flash->info->jedec_id == 0x204015) || //xm25qh16c
				(flash->info->jedec_id == 0x204016) || //xm25qh32b/xm25qh32c
				(flash->info->jedec_id == 0x204017) || //xm25qh64c
				(flash->info->jedec_id == 0x204018) || //xm25qh128c
				(flash->info->jedec_id == 0x204019) || //xm25qh256c
				(flash->info->jedec_id == 0x1c4116) || //en25qe32a
				(flash->info->jedec_id == 0x1c7117) || //en25qx64a
				(flash->info->jedec_id == 0x1c7118) || //en25qx128a
				(flash->info->jedec_id == 0x1c4018)) { //jy25vq128a
				cmd = 0x48;
				dummy = 1;
			} else if ((flash->info->jedec_id == 0x206017) || /* xm25qh64b */
				   (flash->info->jedec_id == 0x206018)) { /* xm25qh128b */
				cmd = 0x68;
				dummy = 1;
			} else {
				cmd = 0x03;
				dummy = 0;
			}
			break;
		case 0x7f /* pmc */:
			cmd = 0x4b;
			dummy = 0;
			break;
		case 0x9d /* issi */:
			cmd = 0x68;
			dummy = 1;
			break;
		default :
			return -1;
	}

	spi_message_init(&m);
	memset(t, 0, sizeof(t));

	t[0].tx_buf = flash->command;
	t[0].len = spi_nor_cmdsz(flash) + dummy;
	spi_message_add_tail(&t[0], &m);
	t[1].rx_buf = buf;
	t[1].len = len;
	spi_message_add_tail(&t[1], &m);

	/* Wait till previous write/erase is done. */
	wait_till_ready(spi);
	spi_nor_otp_entry(flash, 1);
	flash->command[0] = cmd;
	spi_nor_addr2cmd(flash, addr, flash->command);
	spi_sync(spi, &m);
	wait_till_ready(spi);
	spi_nor_otp_entry(flash, 0);
	retlen = m.actual_length - spi_nor_cmdsz(flash) - dummy;

	return retlen;
}

int spi_nor_otp_get_regions(struct spi_nor_flash *flash, unsigned int *regions)
{
	if (flash->info->otp == NULL)
		return -1;

	*regions = flash->info->otp->region_num;

	return 0;
}

int spi_nor_otp_set_region(struct spi_nor_flash *flash, unsigned int region)
{
	if (flash->info->otp == NULL)
		return -1;

	if(region >= 0 && region < flash->info->otp->region_num)
		SET_REGION_FLAGS(flash->info->otp->flags, region);
	else
		return -1;

	return 0;
}

int spi_nor_otp_get_current_region(struct spi_nor_flash *flash, unsigned int *region)
{
	if (flash->info->otp == NULL)
		return -1;

	*region = GET_REGION_FLAGS(flash->info->otp->flags);

	return 0;
}

int spi_nor_otp_get_region_size(struct spi_nor_flash *flash, int *size)
{
	if (flash->info->otp == NULL)
		return -1;

	*size = flash->info->otp->region_size;

	return 0;
}
