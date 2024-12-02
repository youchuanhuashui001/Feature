#include "autoconf.h"
#include "kernelcalls.h"
#ifdef CONFIG_GX_NOS
#include "driver/spi.h"
#elif defined(CONFIG_GX_LINUX)
#include <linux/version.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#endif
#include <gx_flash_common.h>
#include "spinand_ids.h"
#include "spinand_base.h"

#define ID_TABLE_FILL(_id, _name, _info, _ecc, _get_ecc_status, _ecc_strength) {\
	.id             = _id,  \
	.name           = _name,\
	.info_index     = _info,\
	.ecc_index      = _ecc, \
	.get_ecc_status = _get_ecc_status,\
	.ecc_strength = _ecc_strength,    \
}

enum {
	NAND_1G_PAGE2K_OOB64 = 0,
	NAND_2G_PAGE2K_OOB64,
	NAND_1G_PAGE2K_OOB120,
	NAND_1G_PAGE2K_OOB128,
	NAND_2G_PAGE2K_OOB128,
	NAND_4G_PAGE2K_OOB128,
	NAND_4G_PAGE4K_OOB256,
};

enum {
	ECC_LAYOUT_OOB256_TYPE1 = 0,
	ECC_LAYOUT_OOB256_TYPE2,
	ECC_LAYOUT_OOB128_TYPE1,
	ECC_LAYOUT_OOB128_TYPE2,
	ECC_LAYOUT_OOB128_TYPE3,
	ECC_LAYOUT_OOB128_TYPE4,
	ECC_LAYOUT_OOB128_TYPE5,
	ECC_LAYOUT_OOB120_TYPE1,
	ECC_LAYOUT_OOB64_TPYE1,
	ECC_LAYOUT_OOB64_TPYE2,
	ECC_LAYOUT_OOB64_TPYE3,
	ECC_LAYOUT_OOB64_TPYE4,
	ECC_LAYOUT_OOB64_TPYE5,
	ECC_LAYOUT_OOB64_TPYE6,
	ECC_LAYOUT_OOB64_TPYE7,
	ECC_LAYOUT_OOB64_TPYE8,
	ECC_LAYOUT_OOB64_TPYE9,
	ECC_LAYOUT_OOB64_TPYE10,
	ECC_LAYOUT_OOB64_TPYE11,
};

struct spinand_ecclayout spi_nand_ecclayout[] = {
	[ECC_LAYOUT_OOB256_TYPE1] = {
		.oobavail = 124,
		.oobfree = {
			{.offset = 4,  .length = 124},
		}
	},
	[ECC_LAYOUT_OOB256_TYPE2] = {
		.oobavail = 32,
		.oobfree = {
			{.offset = 4,  .length = 4},
			{.offset = 36, .length = 4},
			{.offset = 68, .length = 4},
			{.offset = 100,.length = 4},
			{.offset = 132,.length = 4},
			{.offset = 164,.length = 4},
			{.offset = 196,.length = 4},
			{.offset = 228,.length = 4},
		}
	},
	[ECC_LAYOUT_OOB128_TYPE1] = {
		.oobavail = 48,
		.oobfree = {
			{.offset = 4,  .length = 12},
			{.offset = 20, .length = 12},
			{.offset = 36, .length = 12},
			{.offset = 52, .length = 12},
		}
	},
	[ECC_LAYOUT_OOB128_TYPE2] = {
		.oobavail = 16,
		.oobfree = {
			{.offset = 4,  .length = 4},
			{.offset = 36, .length = 4},
			{.offset = 68, .length = 4},
			{.offset = 100,.length = 4},
		}
	},
	[ECC_LAYOUT_OOB128_TYPE3] = {
		.oobavail = 32,
		.oobfree = {
			{.offset = 32,  .length = 32},
		}
	},
	[ECC_LAYOUT_OOB128_TYPE4] = {
		.oobavail = 62,
		.oobfree = {
			{.offset = 2,  .length = 62},
		}
	},
	[ECC_LAYOUT_OOB128_TYPE5] = {
		.oobavail = 64,
		.oobfree = {
			{.offset = 2,  .length = 62},
		}
	},
	[ECC_LAYOUT_OOB120_TYPE1] = {
		.oobavail = 48,
		.oobfree = {
			{.offset = 4,  .length = 4},
			{.offset = 34, .length = 12},
			{.offset = 64, .length = 12},
			{.offset = 94, .length = 12},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE1] = {
		.oobavail = 48,
		.oobfree = {
			{.offset = 4,  .length = 12},
			{.offset = 20, .length = 12},
			{.offset = 36, .length = 12},
			{.offset = 52, .length = 12},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE2] = {
		.oobavail = 32,
		.oobfree = {
			{.offset = 8,  .length = 8},
			{.offset = 24, .length = 8},
			{.offset = 40, .length = 8},
			{.offset = 56, .length = 8},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE3] = {
		.oobavail = 32,
		.oobfree = {
			{.offset = 4,  .length = 8},
			{.offset = 20, .length = 8},
			{.offset = 36, .length = 8},
			{.offset = 52, .length = 8},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE4] = {
		.oobavail = 30,
		.oobfree = {
			{.offset = 2,  .length = 6},
			{.offset = 16, .length = 8},
			{.offset = 32, .length = 8},
			{.offset = 48, .length = 8},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE5] = {
		.oobavail = 16,
		.oobfree = {
			{.offset = 4,  .length = 4},
			{.offset = 20, .length = 4},
			{.offset = 36, .length = 4},
			{.offset = 52, .length = 4},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE6] = {
		.oobavail = 16,
		.oobfree = {
			{.offset = 4,  .length = 4},
			{.offset = 12, .length = 4},
			{.offset = 20, .length = 4},
			{.offset = 28, .length = 4},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE7] = {
		.oobavail = 62,
		.oobfree = {
			{.offset = 2,  .length = 62},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE8] = {
		.oobavail = 62,
		.oobfree = {
			{.offset = 2,  .length = 14},
			{.offset = 16, .length = 16},
			{.offset = 32, .length = 16},
			{.offset = 48, .length = 16},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE9] = {
		.oobavail = 60,
		.oobfree = {
			{.offset = 4, .length = 60},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE10] = {
		//oob区域前八个字节不受ecc保护
		.oobavail = 40,
		.oobfree = {
			{.offset = 8,  .length = 40},
		}
	},
	[ECC_LAYOUT_OOB64_TPYE11] = {
		.oobavail = 8,
		.oobfree = {
			{.offset = 4,  .length = 2},
			{.offset = 19, .length = 2},
			{.offset = 34, .length = 2},
			{.offset = 49, .length = 2},
		}
	},
};
int g_spi_nand_ecclayout_count = (sizeof(spi_nand_ecclayout) / sizeof((spi_nand_ecclayout)[0]));

/* page info */
#define PAGE_MAIN_SIZE_2K          2048
#define PAGE_MAIN_SIZE_4K          4096
#define PAGE_SPARE_SIZE_64B        64
#define PAGE_SPARE_SIZE_120B       120
#define PAGE_SPARE_SIZE_128B       128
#define PAGE_SPARE_SIZE_256B       256
#define PAGE_SIZE_2K_64B           (PAGE_MAIN_SIZE_2K + PAGE_SPARE_SIZE_64B)
#define PAGE_SIZE_2K_120B          (PAGE_MAIN_SIZE_2K + PAGE_SPARE_SIZE_120B)
#define PAGE_SIZE_2K_128B          (PAGE_MAIN_SIZE_2K + PAGE_SPARE_SIZE_128B)
#define PAGE_SIZE_4K_256B          (PAGE_MAIN_SIZE_4K + PAGE_SPARE_SIZE_256B)
#define PAGE_NUM_PER_BLOCK_64      64
/* block info */
#define BLOCK_MAIN_SIZE_128K       (PAGE_MAIN_SIZE_2K * PAGE_NUM_PER_BLOCK_64)
#define BLOCK_MAIN_SIZE_256K       (PAGE_MAIN_SIZE_4K * PAGE_NUM_PER_BLOCK_64)
#define BLOCK_SIZE_128K_4K         (PAGE_SIZE_2K_64B * PAGE_NUM_PER_BLOCK_64)
#define BLOCK_SIZE_128K_8K         (PAGE_SIZE_2K_128B * PAGE_NUM_PER_BLOCK_64)
#define BLOCK_SIZE_256K_16K        (PAGE_SIZE_4K_256B * PAGE_NUM_PER_BLOCK_64)
#define BLOCK_NUM_PER_CHIP_1024    1024
#define BLOCK_NUM_PER_CHIP_2048    2048
/* nand info */
#define NAND_MAIN_SIZE_128M        (BLOCK_MAIN_SIZE_128K * BLOCK_NUM_PER_CHIP_1024)
#define NAND_MAIN_SIZE_256M        (BLOCK_MAIN_SIZE_128K * BLOCK_NUM_PER_CHIP_2048)
#define NAND_MAIN_SIZE_512M        (BLOCK_MAIN_SIZE_256K * BLOCK_NUM_PER_CHIP_2048)
#define NAND_SIZE_128M_4M          (BLOCK_SIZE_128K_4K * BLOCK_NUM_PER_CHIP_1024)
#define NAND_SIZE_128M_8M          (BLOCK_SIZE_128K_8K * BLOCK_NUM_PER_CHIP_1024)
#define NAND_SIZE_256M_8M          (BLOCK_SIZE_128K_4K * BLOCK_NUM_PER_CHIP_2048)
#define NAND_SIZE_256M_16M         (BLOCK_SIZE_128K_8K * BLOCK_NUM_PER_CHIP_2048)
#define NAND_SIZE_512M_32M         (BLOCK_SIZE_256K_16K * BLOCK_NUM_PER_CHIP_2048)
/* other info */
#define BLOCK_SHIFT_128K           (17)
#define BLOCK_SHIFT_256K           (18)
#define BLOCK_MASK_128K            (BLOCK_MAIN_SIZE_128K - 1)
#define BLOCK_MASK_256K            (BLOCK_MAIN_SIZE_256K - 1)
#define PAGE_SHIFT_2K              (11)
#define PAGE_SHIFT_4K              (12)
#define PAGE_MASK_2K               (PAGE_MAIN_SIZE_2K - 1)
#define PAGE_MASK_4K               (PAGE_MAIN_SIZE_4K - 1)

struct spinand_info spi_nand_info[] = {
	[NAND_1G_PAGE2K_OOB64] = {
		.nand_size           = NAND_SIZE_128M_4M,
		.usable_size         = NAND_MAIN_SIZE_128M,
		.block_size          = BLOCK_SIZE_128K_4K,
		.block_main_size     = BLOCK_MAIN_SIZE_128K,
		.block_num_per_chip  = BLOCK_NUM_PER_CHIP_1024,
		.page_size           = PAGE_SIZE_2K_64B,
		.page_main_size      = PAGE_MAIN_SIZE_2K,
		.page_spare_size     = PAGE_SPARE_SIZE_64B,
		.page_num_per_block  = PAGE_NUM_PER_BLOCK_64,
		.block_shift         = BLOCK_SHIFT_128K,
		.block_mask          = BLOCK_MASK_128K,
		.page_shift          = PAGE_SHIFT_2K,
		.page_mask           = PAGE_MASK_2K,
	},
	[NAND_1G_PAGE2K_OOB120] = {
		.nand_size           = NAND_SIZE_128M_4M,
		.usable_size         = NAND_MAIN_SIZE_128M,
		.block_size          = BLOCK_SIZE_128K_4K,
		.block_main_size     = BLOCK_MAIN_SIZE_128K,
		.block_num_per_chip  = BLOCK_NUM_PER_CHIP_1024,
		.page_size           = PAGE_SIZE_2K_120B,
		.page_main_size      = PAGE_MAIN_SIZE_2K,
		.page_spare_size     = PAGE_SPARE_SIZE_120B,
		.page_num_per_block  = PAGE_NUM_PER_BLOCK_64,
		.block_shift         = BLOCK_SHIFT_128K,
		.block_mask          = BLOCK_MASK_128K,
		.page_shift          = PAGE_SHIFT_2K,
		.page_mask           = PAGE_MASK_2K,
	},

	[NAND_2G_PAGE2K_OOB64] = {
		.nand_size           = NAND_SIZE_256M_8M,
		.usable_size         = NAND_MAIN_SIZE_256M,
		.block_size          = BLOCK_SIZE_128K_4K,
		.block_main_size     = BLOCK_MAIN_SIZE_128K,
		.block_num_per_chip  = BLOCK_NUM_PER_CHIP_2048,
		.page_size           = PAGE_SIZE_2K_64B,
		.page_main_size      = PAGE_MAIN_SIZE_2K,
		.page_spare_size     = PAGE_SPARE_SIZE_64B,
		.page_num_per_block  = PAGE_NUM_PER_BLOCK_64,
		.block_shift         = BLOCK_SHIFT_128K,
		.block_mask          = BLOCK_MASK_128K,
		.page_shift          = PAGE_SHIFT_2K,
		.page_mask           = PAGE_MASK_2K,
	},
	[NAND_1G_PAGE2K_OOB128] = {
		.nand_size           = NAND_SIZE_128M_8M,
		.usable_size         = NAND_MAIN_SIZE_128M,
		.block_size          = BLOCK_SIZE_128K_8K,
		.block_main_size     = BLOCK_MAIN_SIZE_128K,
		.block_num_per_chip  = BLOCK_NUM_PER_CHIP_1024,
		.page_size           = PAGE_SIZE_2K_128B,
		.page_main_size      = PAGE_MAIN_SIZE_2K,
		.page_spare_size     = PAGE_SPARE_SIZE_128B,
		.page_num_per_block  = PAGE_NUM_PER_BLOCK_64,
		.block_shift         = BLOCK_SHIFT_128K,
		.block_mask          = BLOCK_MASK_128K,
		.page_shift          = PAGE_SHIFT_2K,
		.page_mask           = PAGE_MASK_2K,
	},
	[NAND_2G_PAGE2K_OOB128] = {
		.nand_size           = NAND_SIZE_256M_16M,
		.usable_size         = NAND_MAIN_SIZE_256M,
		.block_size          = BLOCK_SIZE_128K_8K,
		.block_main_size     = BLOCK_MAIN_SIZE_128K,
		.block_num_per_chip  = BLOCK_NUM_PER_CHIP_2048,
		.page_size           = PAGE_SIZE_2K_128B,
		.page_main_size      = PAGE_MAIN_SIZE_2K,
		.page_spare_size     = PAGE_SPARE_SIZE_128B,
		.page_num_per_block  = PAGE_NUM_PER_BLOCK_64,
		.block_shift         = BLOCK_SHIFT_128K,
		.block_mask          = BLOCK_MASK_128K,
		.page_shift          = PAGE_SHIFT_2K,
		.page_mask           = PAGE_MASK_2K,
	},
	[NAND_4G_PAGE2K_OOB128] = {
		.nand_size           = NAND_SIZE_512M_32M,
		.usable_size         = NAND_MAIN_SIZE_512M,
		.block_size          = BLOCK_SIZE_128K_8K,
		.block_main_size     = BLOCK_MAIN_SIZE_128K,
		.block_num_per_chip  = BLOCK_NUM_PER_CHIP_2048,
		.page_size           = PAGE_SIZE_2K_128B,
		.page_main_size      = PAGE_MAIN_SIZE_2K,
		.page_spare_size     = PAGE_SPARE_SIZE_128B,
		.page_num_per_block  = PAGE_NUM_PER_BLOCK_64,
		.block_shift         = BLOCK_SHIFT_128K,
		.block_mask          = BLOCK_MASK_128K,
		.page_shift          = PAGE_SHIFT_2K,
		.page_mask           = PAGE_MASK_2K,
	},
	[NAND_4G_PAGE4K_OOB256] = {
		.nand_size           = NAND_SIZE_512M_32M,
		.usable_size         = NAND_MAIN_SIZE_512M,
		.block_size          = BLOCK_SIZE_256K_16K,
		.block_main_size     = BLOCK_MAIN_SIZE_256K,
		.block_num_per_chip  = BLOCK_NUM_PER_CHIP_2048,
		.page_size           = PAGE_SIZE_4K_256B,
		.page_main_size      = PAGE_MAIN_SIZE_4K,
		.page_spare_size     = PAGE_SPARE_SIZE_256B,
		.page_num_per_block  = PAGE_NUM_PER_BLOCK_64,
		.block_shift         = BLOCK_SHIFT_256K,
		.block_mask          = BLOCK_MASK_256K,
		.page_shift          = PAGE_SHIFT_4K,
		.page_mask           = PAGE_MASK_4K,
	},
};
int g_spi_nand_info_count = (sizeof(spi_nand_info) / sizeof((spi_nand_info)[0]));

static int generic_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = GENERIC_ECC_BITS_MAX - 1;
		break;
	case 0x30:
		*bitflips = GENERIC_ECC_BITS_MAX;
		break;
	case 0x20:
		*bitflips = GENERIC_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:;
	}
	return ret;
}

static int gd_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = GD_ECC_BITS_MAX - 1;
		break;
	case 0x30:
		*bitflips = GD_ECC_BITS_MAX;
		break;
	case 0x20:
		*bitflips = GD_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:;
	}
	return ret;
}

static int gd_ecc4_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
	case 0x30:
		*bitflips = GD_ECC_4BITS_MAX;
		break;
	case 0x20:
		*bitflips = GD_ECC_4BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:;
	}
	return ret;
}

static int gd_ecc8_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GD_ECC8_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = 3;
		break;
	case 0x20:
		*bitflips = 4;
		break;
	case 0x30:
		*bitflips = 5;
		break;
	case 0x40:
		*bitflips = 6;
		break;
	case 0x50:
		*bitflips = 7;
		break;
	case 0x60:
		*bitflips = GD_ECC_BITS_MAX;
		break;
	case 0x70:
		*bitflips = GD_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:;
	}
	return ret;
}

static int toshiba_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = TOSHIBA_ECC_BITS_MAX - 1;
		break;
	case 0x30:
		*bitflips = TOSHIBA_ECC_BITS_MAX;
		break;
	case 0x20:
		*bitflips = TOSHIBA_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:;
	}
	return ret;
}

static int micron_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_MICRON_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = 3;
		break;
	case 0x30:
		*bitflips = 6;
		break;
	case 0x50:
		*bitflips = MICRON_ECC_BITS_MAX;
		break;
	case 0x20:
		*bitflips = MICRON_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:;
	}
	return ret;
}

/* XT26G01A */
static int xtx_ecc8_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = (status & STATUS_XTX_ECC8_MASK) >> 2;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x08:
		*bitflips = XTX_ECC_8BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	case 0x0c:
		*bitflips = 8;
		break;
	default:
		*bitflips = ecc_status;
	}
	return ret;
}

/* XT26G01C */
static int xtx_ecc8_verC_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = (status & STATUS_XTX_ECCC8_MASK) >> 4;

	switch(ecc_status) {
	case 0x0f:
		*bitflips = XTX_ECC_8BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:
		*bitflips = ecc_status;
	}
	return ret;
}

static int xtx_ecc4_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = (status & STATUS_XTX_ECC4_MASK) >> 4;

	switch(ecc_status) {
	case 0x07:
		*bitflips = XTX_ECC_4BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:
		*bitflips = ecc_status;
	}
	return ret;
}


static int zetta_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = ZETTA_ECC_BITS_MAX;
		break;
	case 0x20:
	default:
		*bitflips = ZETTA_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int esmt_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = ESMT_ECC_BITS_MAX;
		break;
	case 0x20:
	default:
		*bitflips = ESMT_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int esmt_ecc8_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_ESMT_ECC8_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = 3;
		break;
	case 0x30:
		*bitflips = 6;
		break;
	case 0x50:
		*bitflips = 8;
		break;
	case 0x20:
	default:
		*bitflips = ESMT_ECC_8BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int winbond_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = WINBOND_ECC_BITS_MAX;
		break;
	case 0x30:
	case 0x20:
	default:
		*bitflips = WINBOND_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int winbond_ecc8_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
	case 0x30:
		*bitflips = WINBOND_ECC_8BITS_MAX;
		break;
	case 0x20:
	default:
		*bitflips = WINBOND_ECC_8BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int mxic_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = MXIC_ECC_BITS_MAX;
		break;
	case 0x30:
	case 0x20:
	default:
		*bitflips = MXIC_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int foresse_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = (status & STATUS_FORESEE_ECC_MASK) >> 4;

	if (likely(ecc_status != 0x07)) {
		*bitflips = ecc_status ;
	} else {
		*bitflips = FORESSE_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
	}

	return ret;
}

static int foresse_ecc_status2(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_FORESEE_ECC_MASK2;
	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = FORESSE_ECC_BITS_MAX;
		break;
	case 0x30:
	case 0x20:
	default:
		*bitflips = FORESSE_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int foresse_ecc_status3(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_FORESEE_ECC_MASK2;
	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = GENERIC_ECC_1BIT_MAX;
		break;
	case 0x30:
	case 0x20:
	default:
		*bitflips = GENERIC_ECC_1BIT_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int etron_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = ETRON_ECC_BITS_MAX - 1;
		break;
	case 0x30:
		*bitflips = ETRON_ECC_BITS_MAX;
		break;
	case 0x20:
		*bitflips = ETRON_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:;
	}
	return ret;
}

static int etron_ecc_status2(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = ETRON_ECC_BITS_MAX - 1;
		break;
	case 0x20:
		*bitflips = ETRON_ECC_BITS_MAX;
		break;
	case 0x30:
		*bitflips = ETRON_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:;
	}
	return ret;
}

static int etron_ecc8_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = ETRON_ECC_8BITS_MAX - 1;
		break;
	case 0x30:
		*bitflips = ETRON_ECC_8BITS_MAX;
		break;
	case 0x20:
		*bitflips = ETRON_ECC_8BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	default:;
	}
	return ret;
}

static int dosilicon_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = DOSILICON_ECC_BITS_MAX;
		break;
	case 0x30:
	case 0x20:
	default:
		*bitflips = DOSILICON_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int dosilicon_ecc8_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_DOSILI_ECC8_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = 3;
		break;
	case 0x30:
		*bitflips = 6;
		break;
	case 0x50:
		*bitflips = DOSILICON_ECC_8BITS_MAX;
		break;
	case 0x20:
	default:
		*bitflips = DOSILICON_ECC_8BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int heyang_ecc4_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = HEYANG_ECC_4BITS_MAX - 1;
		break;
	case 0x30:
		*bitflips = HEYANG_ECC_4BITS_MAX;
		break;
	case 0x20:
	default:
		*bitflips = HEYANG_ECC_4BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int heyang_ecc14_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = HEYANG_ECC_14BITS_MAX - 1;
		break;
	case 0x30:
		*bitflips = HEYANG_ECC_14BITS_MAX;
		break;
	case 0x20:
	default:
		*bitflips = HEYANG_ECC_14BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int fudanwei_ecc1_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GENERIC_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = FUDANWEI_ECC_1BITS_MAX;
		break;
	case 0x30:
	case 0x20:
	default:
		*bitflips = FUDANWEI_ECC_1BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int fudanwei_ecc4_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_FUDANWEI_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 0;
		break;
	case 0x10:
		*bitflips = 1;
		break;
	case 0x20:
		*bitflips = 2;
		break;
	case 0x30:
		*bitflips = 3;
		break;
	case 0x40:
		*bitflips = 4;
		break;
	case 0x70:
	default:
		*bitflips = FUDANWEI_ECC_4BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

static int xmc_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = (status & STATUS_XMC_ECC_MASK) >> 4;

	if (likely(ecc_status == 0x00)) {
		*bitflips = 0;
	} else if (likely(ecc_status == 0x07)) {
		*bitflips = XMC_ECC_BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
	} else {
		*bitflips = ecc_status + 2;
	}

	return ret;
}

static int gsto_ecc_status(unsigned char status, unsigned int *bitflips)
{
	int ret = 0;
	unsigned int ecc_status = status & STATUS_GSTO_ECC_MASK;

	switch(ecc_status) {
	case 0x00:
		*bitflips = 3;
		break;
	case 0x10:
		*bitflips = GSTO_ECC_4BITS_MAX;
		break;
	case 0x20:
	case 0x30:
	default:
		*bitflips = GSTO_ECC_4BITS_MAX + 1;
		ret = ECC_NOT_CORRECT;
		break;
	}
	return ret;
}

struct spinand_index spinand_id_table[] = {
	/* GD spi nand flash */
	ID_TABLE_FILL(0xF1C8, "GD5F1GQ4UAYIG",   NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE3,  gd_ecc_status,        GD_ECC_BITS_MAX),
	ID_TABLE_FILL(0xF2C8, "GD5F2GQ4RAYIG",   NAND_2G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE3,  gd_ecc_status,        GD_ECC_BITS_MAX),
	ID_TABLE_FILL(0x51C8, "GD5F1GQ5UEYIG",   NAND_1G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE1, gd_ecc4_status,       GD_ECC_4BITS_MAX),
	ID_TABLE_FILL(0x52C8, "GD5F2GQ5UEYIG",   NAND_2G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE1, gd_ecc4_status,       GD_ECC_4BITS_MAX),
	ID_TABLE_FILL(0xD1C8, "GD5F1GQ4U",       NAND_1G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE1, gd_ecc_status,        GD_ECC_BITS_MAX),
	ID_TABLE_FILL(0xD2C8, "GD5F2GQ4U",       NAND_2G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE1, gd_ecc_status,        GD_ECC_BITS_MAX),
	ID_TABLE_FILL(0xC1C8, "GD5F1GQ4R",       NAND_1G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE1, gd_ecc_status,        GD_ECC_BITS_MAX),
	ID_TABLE_FILL(0xC2C8, "GD5F2GQ4R",       NAND_2G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE1, gd_ecc_status,        GD_ECC_BITS_MAX),
	ID_TABLE_FILL(0x91C8, "GD5F1GM7UEYIG",   NAND_1G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE5, gd_ecc_status,        GD_ECC_BITS_MAX),
	ID_TABLE_FILL(0x92C8, "GD5F2GM7UEYIG",   NAND_2G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE5, gd_ecc_status,        GD_ECC_BITS_MAX),
	ID_TABLE_FILL(0x95C8, "GD5F4GM8UEYIG",   NAND_4G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE5, gd_ecc_status,        GD_ECC_BITS_MAX),
	ID_TABLE_FILL(0x68B4, "GD5F4GM5UFYIG",   NAND_4G_PAGE4K_OOB256, ECC_LAYOUT_OOB256_TYPE1, gd_ecc8_status,       GD_ECC_BITS_MAX),
	/* TOSHIBA spi nand flash */
	ID_TABLE_FILL(0xC298, "TC58CVG053HRA1G", NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE9,  toshiba_ecc_status,   TOSHIBA_ECC_BITS_MAX),
	/* Micron spi nand flash */
	ID_TABLE_FILL(0x122C, "MT29F1G01ZAC",    NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  micron_ecc_status,    MICRON_ECC_BITS_MAX),
	ID_TABLE_FILL(0x112C, "MT29F1G01ZAC",    NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  micron_ecc_status,    MICRON_ECC_BITS_MAX),
	ID_TABLE_FILL(0x132C, "MT29F1G01ZAC",    NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  micron_ecc_status,    MICRON_ECC_BITS_MAX),
	ID_TABLE_FILL(0x142C, "MT29F1G01ABAFD",  NAND_1G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE3, micron_ecc_status,    MICRON_ECC_BITS_MAX),
	ID_TABLE_FILL(0x242c, "MT29F2G01ABAGD/F50L2G41XA/XT26G02EBGIGA/GTS2G3M1S", NAND_2G_PAGE2K_OOB128,ECC_LAYOUT_OOB128_TYPE3,esmt_ecc8_status, ESMT_ECC_8BITS_MAX),
	/* 芯天下 spi nand flash */
	ID_TABLE_FILL(0xE1A1, "PN26G01AWS1UG",   NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE11, generic_ecc_status,   GENERIC_ECC_BITS_MAX),
	ID_TABLE_FILL(0xE10B, "XT26G01AWS1UG",   NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE10, xtx_ecc8_status,      XTX_ECC_8BITS_MAX),
	ID_TABLE_FILL(0x110B, "XT26G01CWSIG",    NAND_1G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE4, xtx_ecc8_verC_status, XTX_ECC_8BITS_MAX),
	ID_TABLE_FILL(0xE20B, "XT26G02AWSEGA",   NAND_2G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE10, xtx_ecc8_status,      XTX_ECC_8BITS_MAX),
	ID_TABLE_FILL(0xF20B, "XT26G02BWSUGA",   NAND_2G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE7,  xtx_ecc4_status,      XTX_ECC_4BITS_MAX),
	ID_TABLE_FILL(0x120B, "XT26G02CWSIGA",   NAND_2G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE4, xtx_ecc8_verC_status, XTX_ECC_8BITS_MAX),
	ID_TABLE_FILL(0x130B, "XT26G04CWSIGA",   NAND_4G_PAGE4K_OOB256, ECC_LAYOUT_OOB256_TYPE1, xtx_ecc8_verC_status, XTX_ECC_8BITS_MAX),
	/* Zetta Confidentia spi nand flash */
	ID_TABLE_FILL(0x71ba, "ZD35X1GA",        NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  zetta_ecc_status,     ZETTA_ECC_BITS_MAX),
	ID_TABLE_FILL(0x72ba, "ZD35Q2GA",        NAND_2G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  zetta_ecc_status,     ZETTA_ECC_BITS_MAX),
	ID_TABLE_FILL(0x21ba, "ZD35X1GA",        NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  zetta_ecc_status,     ZETTA_ECC_BITS_MAX),
	/* ESMT spi nand flash */
	ID_TABLE_FILL(0x21C8, "F50L1G41A",       NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE2, esmt_ecc_status,      ESMT_ECC_BITS_MAX),
	ID_TABLE_FILL(0x01C8, "F50L1G41LB",      NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5, esmt_ecc_status,      ESMT_ECC_BITS_MAX),
	/* winbond spi nand flash */
	ID_TABLE_FILL(0x21AAEF, "W25N01GVZEIG",  NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  winbond_ecc_status,   WINBOND_ECC_BITS_MAX),
	ID_TABLE_FILL(0x22AAEF, "W25N02KVZEIR",  NAND_2G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE1, winbond_ecc8_status,  WINBOND_ECC_8BITS_MAX),
	ID_TABLE_FILL(0x23AAEF, "W25N04KVZEIR",  NAND_4G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE1, winbond_ecc8_status,  WINBOND_ECC_8BITS_MAX),
	/* Mxic spi nand flash */
	ID_TABLE_FILL(0x12C2, "MX35LF1GE4AB",    NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE1, mxic_ecc_status,      MXIC_ECC_BITS_MAX),
	/* foresee spi nand flash */
	ID_TABLE_FILL(0xa1cd, "FS35ND01G-D1F1",  NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE4, foresse_ecc_status,   FORESSE_ECC_BITS_MAX),
	ID_TABLE_FILL(0xb1cd, "FS35ND01G-S1F1",  NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE8, foresse_ecc_status,   FORESSE_ECC_BITS_MAX),
	ID_TABLE_FILL(0xeacd, "FS35ND01G-S1Y2",  NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE8, foresse_ecc_status2,  FORESSE_ECC_BITS_MAX),
	ID_TABLE_FILL(0x71cd, "FS35SQA001G",     NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE8, foresse_ecc_status3,  GENERIC_ECC_1BIT_MAX),
	/* EtronTech spi nand flash */
	ID_TABLE_FILL(0x1cd5, "EM73C044VCD-H",   NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  etron_ecc_status,     ETRON_ECC_BITS_MAX),
	ID_TABLE_FILL(0x25d5, "EM73C044VCF-H",   NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE6,  etron_ecc_status,     ETRON_ECC_BITS_MAX),
	ID_TABLE_FILL(0x11d5, "EM73C044SNB-G",   NAND_1G_PAGE2K_OOB120, ECC_LAYOUT_OOB120_TYPE1, etron_ecc8_status,    ETRON_ECC_8BITS_MAX),
	ID_TABLE_FILL(0x1fd5, "EM73D044VCG-H",   NAND_2G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  etron_ecc_status,     ETRON_ECC_BITS_MAX),
	ID_TABLE_FILL(0x20d5, "EM73D044BDE-H",   NAND_2G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  etron_ecc_status,     ETRON_ECC_BITS_MAX),
	ID_TABLE_FILL(0x2501, "EM73C044VCS-H",   NAND_2G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE2, etron_ecc_status,     ETRON_ECC_BITS_MAX),
	/* dosilicon spi nand flash */
	ID_TABLE_FILL(0x71e5, "DS35Q1GA",        NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  dosilicon_ecc_status, DOSILICON_ECC_BITS_MAX),
	ID_TABLE_FILL(0xf1e5, "DS35Q1GB",        NAND_1G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE5, dosilicon_ecc8_status, DOSILICON_ECC_8BITS_MAX),
	ID_TABLE_FILL(0x72e5, "DS35Q2GA",        NAND_2G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  dosilicon_ecc_status, DOSILICON_ECC_BITS_MAX),
	ID_TABLE_FILL(0xf2e5, "DS35Q2GB",        NAND_2G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE5, dosilicon_ecc8_status, DOSILICON_ECC_8BITS_MAX),
	/* HeYangTek spi nand flash */
	ID_TABLE_FILL(0x21c9, "HYF1GQ4UDACAE",   NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE5,  heyang_ecc4_status,   HEYANG_ECC_4BITS_MAX),
	ID_TABLE_FILL(0x5ac9, "HYF2GQ4UHCCAE",   NAND_2G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE2, heyang_ecc14_status,  HEYANG_ECC_14BITS_MAX),
	ID_TABLE_FILL(0x52c9, "HYF2GQ4UAACAE",   NAND_2G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE2, heyang_ecc14_status,  HEYANG_ECC_14BITS_MAX),
	ID_TABLE_FILL(0xd4c9, "HYF4GQ4UAACBE",   NAND_4G_PAGE4K_OOB256, ECC_LAYOUT_OOB256_TYPE2, heyang_ecc14_status,  HEYANG_ECC_14BITS_MAX),
	/* Fudan Microelectronics */
	ID_TABLE_FILL(0xa1a1, "FM25S01",         NAND_1G_PAGE2K_OOB128, ECC_LAYOUT_OOB128_TYPE4, fudanwei_ecc1_status, FUDANWEI_ECC_1BITS_MAX),
	ID_TABLE_FILL(0xe4a1, "FM25S01A",        NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE7,  fudanwei_ecc1_status, FUDANWEI_ECC_1BITS_MAX),
	ID_TABLE_FILL(0xf2a1, "FM25S02",         NAND_2G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE4,  fudanwei_ecc4_status, FUDANWEI_ECC_4BITS_MAX),
	ID_TABLE_FILL(0xe5a1, "FM25S02A",        NAND_2G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE7,  fudanwei_ecc1_status, FUDANWEI_ECC_1BITS_MAX),
	/* Unim Semiconductor Co, Ltd */
	ID_TABLE_FILL(0xf1a1, "TX25G01",         NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE4,  fudanwei_ecc4_status, FUDANWEI_ECC_4BITS_MAX),
	/* Wuhan Xinxin Semiconductor */
	ID_TABLE_FILL(0x2120, "XM25NH02B",       NAND_2G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE7,  xmc_ecc_status,       XMC_ECC_BITS_MAX),
	/* GSTO spi nand flash */
	ID_TABLE_FILL(0x13BA52, "GSS01GSAK1",    NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE8,  gsto_ecc_status,       GSTO_ECC_4BITS_MAX),
	/* item end */
	ID_TABLE_FILL(0x0001, "General flash",   NAND_1G_PAGE2K_OOB64,  ECC_LAYOUT_OOB64_TPYE2,  generic_ecc_status,   GENERIC_ECC_BITS_MAX),
};

int g_spinand_id_table_count = (sizeof(spinand_id_table) / sizeof((spinand_id_table)[0]));
