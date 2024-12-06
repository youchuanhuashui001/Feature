#ifndef __SPINOR_BASE_H__
#define __SPINOR_BASE_H__

#include "autoconf.h"

#ifndef NULL
#define NULL 0
#endif

// series serial flash
#define GX_CMD_WREN           0x06      // write enable
#define GX_CMD_WRDI           0x04      // write disable
#define GX_CMD_RDSR1          0x05      // read status1 register
#define GX_CMD_RDSR2          0x35      // read status2/configure register
#define GX_CMD_RDSR3          0x15      // read status3/configure register
#define GX_CMD_WRSR1          0x01      // write status/configure register
#define GX_CMD_WRSR2          0X31      // write status/configure register
#define GX_CMD_WRSR3          0X11      // write status/configure register
#define GX_CMD_READ           0x03      // read data bytes
#define GX_CMD_FAST_READ      0x0b      // read data bytes at higher speed
#define GX_CMD_DREAD          0x3B      // read data dual output
#define GX_CMD_QREAD          0x6B      // read data quad output
#define GX_CMD_PP             0x02      // page program
#define GX_CMD_QUAD_PP        0x32      // page program
#define GX_CMD_SE             0x20      // sector erase
#define GX_CMD_BE             0xd8      // block erase
#define GX_CMD_CE             0xc7      // chip erase
#define GX_CMD_DP             0xb9      // deep power down
#define GX_CMD_RES            0xab      // release from deep power down
#define GX_CMD_READID         0x9F      // read ID
#define GX_CMD_READID_BAK     0x90      // read ID for WinBond
#define GX_CMD_WREAR          0xC5      //Write Extended Address Register

#define GX_NOR_SR2_QE         0x02      // SPI NOR Flash QE bit
#define GX_NOR_SR1_QE         0x40      // SPI NOR FLASH QE bit

// dummy clock
#define FAST_READ_DUMMY_CLOCK 8         //fast read dummy clock size

/* Used for Macronix/Winbond/XMC flashes. */
#define GX_CMD_EN4B     0xb7    /* Enter 4-byte mode */
#define GX_CMD_EX4B     0xe9    /* Exit 4-byte mode */
/* Used for Spansion flashes only. */
#define GX_CMD_BRWR     0x17    /* Bank register write */
/* Usec for ISSI flashes only. */
#define GX_CMD_RDFR     0x48    /* Read Function Register */
#define GX_CMD_WRFR     0x42    /* Write Function Register */

// GX25Lxx series status regsiter
#define GX_STAT_WIP           0x01      // write in progress bit
#define GX_STAT_WEL           0x02      // write enable latch
#define GX_STAT_BP0           0x04      // block protect bit 0
#define GX_STAT_BP1           0x08      // block protect bit 1
#define GX_STAT_BP2           0x10      // block protect bit 2
#define GX_STAT_SRWD          0x80      // status register write protect
#define GX_STAT_SRL           0x01      // lock status register

// define flash size
#define SPI_FLASH_BLOCK_SIZE  0x10000
#define SPI_FLASH_SECTOR_SIZE 0x1000
#define SPI_FLASH_PAGE_SIZE   0x100
#ifdef CONFIG_GX_MTD_SPI_NOR_USE_4K_SECTORS
#define SPI_FLASH_ERASE_SIZE SPI_FLASH_SECTOR_SIZE
#else
#define SPI_FLASH_ERASE_SIZE SPI_FLASH_BLOCK_SIZE
#endif

/*
 * spi read/write/erase  mode Macro
 */
#define SPI_NOR_DUAL_READ     0x20    /* Flash supports Dual Read */
#define SPI_NOR_QUAD_READ     0x40    /* Flash supports Quad Read */
#define SPI_NOR_QUAD_WRITE    0x80    /* Flash supports Quad Write */

#define WP_BOTTOM             0x1
#define WP_TOP                0x2

#define WRITE_COUNT(x) sizeof(x)/sizeof(struct spi_nor_flash_protect_range)

typedef enum {
	UNKNOW_MARKID,

	MX25L3206E,
	MX25L3233F,
	MX25L32,

	GD25Q128C,
	GD25Q128B,
	GD25Q127C,
	GD25Q128,

	MX25L6433F,
	MX25L6406E,
	MX25L6408E,
	KH25L6436F,
	MX25L64,

	XT25F32B_S,
	XT25F32F,

	XT25F64B_S,
	XT25F64F,

	XM25QH32B_XM25QH32C_XM25QE32C,
	XM25QH32D,

	XM25QH64C,
	XM25QH64D,

	XM25QH128C,
	XM25QH128D,

	ZB25VQ32_B,
	ZB25VQ32D,

	ZB25VQ64_A,
	ZB25VQ64B_C_D,

	ZB25VQ128A,
	ZB25VQ128D,
} spi_nor_flash_markid;

struct spi_nor_flash_protect_range {
	unsigned char status1;
	unsigned char mask1;
	unsigned char status2;
	unsigned char mask2;
	unsigned int  wp_len;   /* len */
};

struct spi_nor_flash_protect {
	struct spi_nor_flash_protect_range *sf_range;
	int count;    //sizeof(sf_range)/sizeof(struct spi_nor_flash_range)
};

typedef struct spi_nor_otp{
	unsigned int          start_addr;
	unsigned int          skip_addr;
	unsigned int          region_size;
	unsigned int          region_num;
	unsigned int          flags;
} spi_nor_otp;

typedef struct spi_nor_info{
	char                  *name;
	int                   jedec_id;
	unsigned int          size; // size of chip.
	unsigned int          flag;
	int (*get_flash_markid)(struct spi_device *spi, unsigned int jedec_id, spi_nor_flash_markid mark_id);
	spi_nor_flash_markid  markid;
	struct spi_nor_flash_protect *write_protect;
	struct spi_nor_otp    *otp;
} spi_nor_info;

struct spi_nor_flash
{
	unsigned char addr_width;
	unsigned int protect_mode;
	unsigned int match_mode;
	unsigned int  size;
	spi_nor_info  *info;
	struct spi_device *spi;

#define CMD_SIZE 6
	unsigned char command[CMD_SIZE];
};


int spi_nor_readid(struct spi_device *spi, int *jedec);
unsigned int spi_nor_get_info(struct spi_nor_flash *flash, enum gx_flash_info flash_info);
int spi_nor_read_uniqueid(struct spi_nor_flash *flash, unsigned char *buf, unsigned int buf_len, unsigned int *id_len);
void spi_nor_set_4byte(struct spi_nor_flash *flash);
void spi_nor_exit_4byte(struct spi_nor_flash *flash);
int spi_nor_detect(struct spi_nor_flash *flash);
int spi_nor_chiperase(struct spi_device *spi);
int spi_nor_write(struct spi_nor_flash *flash, unsigned int to, unsigned char *buf, unsigned int len);
int spi_nor_normal_read(struct spi_nor_flash *flash, unsigned int from, unsigned char *buf,unsigned int len);
int spi_nor_fast_read(struct spi_nor_flash *flash, unsigned int from, unsigned char *buf,unsigned int len);
int spi_nor_dual_read(struct spi_nor_flash *flash, unsigned int from, unsigned char *buf,unsigned int len);
int spi_nor_read(struct spi_nor_flash *flash, unsigned int addr,unsigned char *to, unsigned int len);
void spi_nor_blockerase(struct spi_nor_flash *flash, unsigned int addr);
void spi_nor_sectorerase(struct spi_nor_flash *flash, unsigned int addr);
int spi_nor_erase(struct spi_nor_flash *flash, unsigned int addr, unsigned int len);
int spi_nor_write_protect_status(struct spi_nor_flash *flash, unsigned int *len);
int spi_nor_write_protect_mode(struct spi_nor_flash *flash);
int spi_nor_write_protect_lock(struct spi_nor_flash *flash, unsigned int protect_len, unsigned int *real_protect_len);
int spi_nor_write_protect_generic_unlock(struct spi_nor_flash *flash);
int spi_nor_set_lock_reg(struct spi_nor_flash *flash);
int spi_nor_get_lock_reg(struct spi_nor_flash *flash, int *lock);
void spi_nor_calcblockrange(struct spi_nor_flash *flash, unsigned int addr, unsigned int len, unsigned int *pstart, unsigned int *pend);
void spi_nor_sync(struct spi_device *spi);

int spi_nor_otp_entry(struct spi_nor_flash *flash, unsigned int on);
int spi_nor_otp_lock(struct spi_nor_flash *flash);
int spi_nor_otp_status(struct spi_nor_flash *flash, unsigned char *buf);
int spi_nor_otp_erase(struct spi_nor_flash *flash);
int spi_nor_otp_write(struct spi_nor_flash *flash, unsigned int addr, unsigned char *buf, unsigned int len);
int spi_nor_otp_read(struct spi_nor_flash *flash, unsigned int addr, unsigned char *buf, unsigned int len);
int spi_nor_otp_get_regions(struct spi_nor_flash *flash, unsigned int *regions);
int spi_nor_otp_set_region(struct spi_nor_flash *flash, unsigned int region);
int spi_nor_otp_get_current_region(struct spi_nor_flash *flash, unsigned int *region);
int spi_nor_otp_get_region_size(struct spi_nor_flash *flash, int *size);

#endif
