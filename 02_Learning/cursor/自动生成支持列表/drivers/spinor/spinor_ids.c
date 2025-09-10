#include "autoconf.h"
#include "kernelcalls.h"
#ifdef CONFIG_GX_NOS
#include "driver/spi.h"
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

#ifdef CONFIG_GX_MTD_SPI_NOR_WRITE_PROTECT
#ifdef CONFIG_GX_MTD_SPI_NOR_32M_SIZE
static struct spi_nor_flash_protect_range protect_range_32m_type1[] = {
	{ 0x00, 0x3c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x44, 0xfc, 0x00, 0x41, 1   * 0x10000 },
	{ 0x48, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x4c, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x50, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x54, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x58, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x5c, 0xfc, 0x00, 0x41, 64  * 0x10000 },
	{ 0x60, 0xfc, 0x00, 0x41, 128 * 0x10000 },
	{ 0x64, 0xfc, 0x00, 0x41, 256 * 0x10000 },
	{ 0x20, 0xfc, 0x40, 0x40, 384 * 0x10000 },
	{ 0x1c, 0xfc, 0x40, 0x40, 448 * 0x10000 },
	{ 0x18, 0xfc, 0x40, 0x40, 480 * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x40, 496 * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x40, 504 * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x40, 508 * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x40, 510 * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x40, 511 * 0x10000 },
	{ 0x38, 0xb8, 0x00, 0x41, 512 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_32m_type2[] = {
	{ 0x00, 0xbc, 0x08, 0x08, 0   * 0x10000 },
	{ 0x04, 0xbc, 0x08, 0x08, 1   * 0x10000 },
	{ 0x08, 0xbc, 0x08, 0x08, 2   * 0x10000 },
	{ 0x0c, 0xbc, 0x08, 0x08, 4   * 0x10000 },
	{ 0x10, 0xbc, 0x08, 0x08, 8   * 0x10000 },
	{ 0x14, 0xbc, 0x08, 0x08, 16  * 0x10000 },
	{ 0x18, 0xbc, 0x08, 0x08, 32  * 0x10000 },
	{ 0x1c, 0xbc, 0x08, 0x08, 64  * 0x10000 },
	{ 0x20, 0xbc, 0x08, 0x08, 128 * 0x10000 },
	{ 0x24, 0xbc, 0x08, 0x08, 256 * 0x10000 },
	{ 0x28, 0xbc, 0x08, 0x08, 512 * 0x10000 },
};
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
static struct spi_nor_flash_protect_range protect_range_16m_type1[] = {
	{ 0x00, 0x1c, 0x00, 0x40, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x40, 4   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x40, 8   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x40, 16  * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x40, 32  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x40, 64  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x40, 128 * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x40, 192 * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x40, 224 * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x40, 240 * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x40, 248 * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x40, 252 * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x40, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type2[] = {
	{ 0x00, 0x9c, 0x00, 0x40, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x40, 4   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x40, 8   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x40, 16  * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x40, 32  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x40, 64  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x40, 128 * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x40, 192 * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x40, 224 * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x40, 240 * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x40, 248 * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x40, 252 * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x40, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type3[] = {
	{ 0x00, 0x1c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 64  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 128 * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x41, 192 * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x41, 224 * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x41, 240 * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x41, 248 * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x41, 252 * 0x10000 },
	{ 0x1c, 0x1c, 0x00, 0x41, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type4[] = {
	{ 0x00, 0x9c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 64  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 128 * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x41, 192 * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x41, 224 * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x41, 240 * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x41, 248 * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x41, 252 * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x41, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type6[] = {
	{ 0x00, 0x9c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 64  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 128 * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x41, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type7[] = {
	{ 0x00, 0x1c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 64  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 128 * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x41, 192 * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x41, 224 * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x41, 240 * 0x10000 },
	{ 0x1c, 0x1c, 0x00, 0x41, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type9[] = {
	{ 0x00, 0xbc, 0x08, 0x08, 0   * 0x10000 },
	{ 0x04, 0xbc, 0x08, 0x08, 1   * 0x10000 },
	{ 0x08, 0xbc, 0x08, 0x08, 2   * 0x10000 },
	{ 0x0c, 0xbc, 0x08, 0x08, 4   * 0x10000 },
	{ 0x10, 0xbc, 0x08, 0x08, 8   * 0x10000 },
	{ 0x14, 0xbc, 0x08, 0x08, 16  * 0x10000 },
	{ 0x18, 0xbc, 0x08, 0x08, 32  * 0x10000 },
	{ 0x1c, 0xbc, 0x08, 0x08, 64  * 0x10000 },
	{ 0x20, 0xbc, 0x08, 0x08, 128 * 0x10000 },
	{ 0x24, 0xbc, 0x08, 0x08, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type10[] = {
	{ 0x20, 0xbc, 0x00, 0x08, 0   * 0x10000 },
	{ 0x24, 0xbc, 0x00, 0x08, 4   * 0x10000 },
	{ 0x28, 0xbc, 0x00, 0x08, 8   * 0x10000 },
	{ 0x2c, 0xbc, 0x00, 0x08, 16  * 0x10000 },
	{ 0x30, 0xbc, 0x00, 0x08, 32  * 0x10000 },
	{ 0x34, 0xbc, 0x00, 0x08, 64  * 0x10000 },
	{ 0x38, 0xbc, 0x00, 0x08, 128 * 0x10000 },
	{ 0x3c, 0xbc, 0x00, 0x08, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type11[] = {
	{ 0x00, 0xbc, 0x02, 0x02, 0   * 0x10000 },
	{ 0x04, 0xbc, 0x02, 0x02, 1   * 0x10000 },
	{ 0x08, 0xbc, 0x02, 0x02, 2   * 0x10000 },
	{ 0x0c, 0xbc, 0x02, 0x02, 4   * 0x10000 },
	{ 0x10, 0xbc, 0x02, 0x02, 8   * 0x10000 },
	{ 0x14, 0xbc, 0x02, 0x02, 16  * 0x10000 },
	{ 0x18, 0xbc, 0x02, 0x02, 32  * 0x10000 },
	{ 0x1c, 0xbc, 0x02, 0x02, 64  * 0x10000 },
	{ 0x20, 0xbc, 0x02, 0x02, 128 * 0x10000 },
	{ 0x24, 0xbc, 0x02, 0x02, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type12[] = {
	{ 0x00, 0xfc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x00, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x00, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x00, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x00, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x00, 16  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x00, 32  * 0x10000 },
	{ 0x3c, 0xfc, 0x00, 0x00, 64  * 0x10000 },
	{ 0x60, 0xfc, 0x00, 0x00, 128 * 0x10000 },
	{ 0x7c, 0xfc, 0x00, 0x00, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type13[] = {
	{ 0x00, 0xbc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x18, 0xbc, 0x00, 0x00, 224 * 0x10000 },
	{ 0x14, 0xbc, 0x00, 0x00, 240 * 0x10000 },
	{ 0x10, 0xbc, 0x00, 0x00, 248 * 0x10000 },
	{ 0x0c, 0xbc, 0x00, 0x00, 252 * 0x10000 },
	{ 0x08, 0xbc, 0x00, 0x00, 254 * 0x10000 },
	{ 0x04, 0xbc, 0x00, 0x00, 255 * 0x10000 },
	{ 0x1c, 0xbc, 0x00, 0x00, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type14[] = {
	{ 0x20, 0xbc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x24, 0xbc, 0x00, 0x00, 4   * 0x10000 },
	{ 0x28, 0xbc, 0x00, 0x00, 8   * 0x10000 },
	{ 0x2c, 0xbc, 0x00, 0x00, 16  * 0x10000 },
	{ 0x30, 0xbc, 0x00, 0x00, 32  * 0x10000 },
	{ 0x34, 0xbc, 0x00, 0x00, 64  * 0x10000 },
	{ 0x38, 0xbc, 0x00, 0x00, 128 * 0x10000 },
	{ 0x3c, 0xbc, 0x00, 0x00, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type15[] = {
	{ 0x00, 0xfc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x00, 4   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x00, 8   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x00, 16  * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x00, 32  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x00, 64  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x00, 128 * 0x10000 },
	{ 0x3c, 0xfc, 0x00, 0x00, 256 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_16m_type16[] = {
	{ 0x00, 0x9c, 0x00, 0x00, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x00, 4   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x00, 8   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x00, 16  * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x00, 32  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x00, 64  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x00, 128 * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x00, 256 * 0x10000 },
};
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
static struct spi_nor_flash_protect_range protect_range_8m_type1[] = {
	{ 0x00, 0x1c, 0x00, 0x40, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x40, 2   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x40, 4   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x40, 8   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x40, 16  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x40, 32  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x40, 64  * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x40, 96  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x40, 112 * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x40, 120 * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x40, 124 * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x40, 126 * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x40, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_8m_type2[] = {
	{ 0x00, 0x9c, 0x00, 0x40, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x40, 2   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x40, 4   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x40, 8   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x40, 16  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x40, 32  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x40, 64  * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x40, 96  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x40, 112 * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x40, 120 * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x40, 124 * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x40, 126 * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x40, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_8m_type3[] = {
	{ 0x00, 0x1c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 64  * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x41, 96  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x41, 112 * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x41, 120 * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x41, 124 * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x41, 126 * 0x10000 },
	{ 0x1c, 0x1c, 0x00, 0x41, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_8m_type4[] = {
	{ 0x00, 0x9c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 64  * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x41, 96  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x41, 112 * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x41, 120 * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x41, 124 * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x41, 126 * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x41, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_8m_type7[] = {
	{ 0x00, 0xbc, 0x08, 0x08, 0   * 0x10000 },
	{ 0x04, 0xbc, 0x08, 0x08, 1   * 0x10000 },
	{ 0x08, 0xbc, 0x08, 0x08, 2   * 0x10000 },
	{ 0x0c, 0xbc, 0x08, 0x08, 4   * 0x10000 },
	{ 0x10, 0xbc, 0x08, 0x08, 8   * 0x10000 },
	{ 0x14, 0xbc, 0x08, 0x08, 16  * 0x10000 },
	{ 0x18, 0xbc, 0x08, 0x08, 32  * 0x10000 },
	{ 0x1c, 0xbc, 0x08, 0x08, 64  * 0x10000 },
	{ 0x20, 0xbc, 0x08, 0x08, 96  * 0x10000 },
	{ 0x24, 0xbc, 0x08, 0x08, 112 * 0x10000 },
	{ 0x28, 0xbc, 0x08, 0x08, 120 * 0x10000 },
	{ 0x2c, 0xbc, 0x08, 0x08, 124 * 0x10000 },
	{ 0x30, 0xbc, 0x08, 0x08, 126 * 0x10000 },
	{ 0x34, 0xbc, 0x08, 0x08, 127 * 0x10000 },
	{ 0x38, 0xbc, 0x08, 0x08, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_8m_type8[] = {
	{ 0x00, 0xbc, 0x02, 0x02, 0   * 0x10000 },
	{ 0x04, 0xbc, 0x02, 0x02, 1   * 0x10000 },
	{ 0x08, 0xbc, 0x02, 0x02, 2   * 0x10000 },
	{ 0x0c, 0xbc, 0x02, 0x02, 4   * 0x10000 },
	{ 0x10, 0xbc, 0x02, 0x02, 8   * 0x10000 },
	{ 0x14, 0xbc, 0x02, 0x02, 16  * 0x10000 },
	{ 0x18, 0xbc, 0x02, 0x02, 32  * 0x10000 },
	{ 0x1c, 0xbc, 0x02, 0x02, 64  * 0x10000 },
	{ 0x20, 0xbc, 0x02, 0x02, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_8m_type9[] = {
	{ 0x00, 0xbc, 0x08, 0x08, 0   * 0x10000 },
	{ 0x04, 0xbc, 0x08, 0x08, 1   * 0x10000 },
	{ 0x08, 0xbc, 0x08, 0x08, 2   * 0x10000 },
	{ 0x0c, 0xbc, 0x08, 0x08, 4   * 0x10000 },
	{ 0x10, 0xbc, 0x08, 0x08, 8   * 0x10000 },
	{ 0x14, 0xbc, 0x08, 0x08, 16  * 0x10000 },
	{ 0x18, 0xbc, 0x08, 0x08, 32  * 0x10000 },
	{ 0x1c, 0xbc, 0x08, 0x08, 64  * 0x10000 },
	{ 0x20, 0xbc, 0x08, 0x08, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_8m_type10[] = {
	{ 0x00, 0xbc, 0x08, 0x08, 0   * 0x10000 },
	{ 0x04, 0xbc, 0x08, 0x08, 2   * 0x10000 },
	{ 0x08, 0xbc, 0x08, 0x08, 4   * 0x10000 },
	{ 0x0c, 0xbc, 0x08, 0x08, 8   * 0x10000 },
	{ 0x10, 0xbc, 0x08, 0x08, 16  * 0x10000 },
	{ 0x14, 0xbc, 0x08, 0x08, 32  * 0x10000 },
	{ 0x18, 0xbc, 0x08, 0x08, 64  * 0x10000 },
	{ 0x1c, 0xbc, 0x08, 0x08, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_8m_type11[] = {
	{ 0x00, 0xbc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x24, 0xbc, 0x00, 0x00, 64  * 0x10000 },
	{ 0x28, 0xbc, 0x00, 0x00, 96  * 0x10000 },
	{ 0x2c, 0xbc, 0x00, 0x00, 112 * 0x10000 },
	{ 0x30, 0xbc, 0x00, 0x00, 120 * 0x10000 },
	{ 0x34, 0xbc, 0x00, 0x00, 124 * 0x10000 },
	{ 0x38, 0xbc, 0x00, 0x00, 126 * 0x10000 },
	{ 0x3c, 0xbc, 0x00, 0x00, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_8m_type12[] = {
	{ 0x00, 0xbc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x18, 0xbc, 0x00, 0x00, 96  * 0x10000 },
	{ 0x14, 0xbc, 0x00, 0x00, 112 * 0x10000 },
	{ 0x10, 0xbc, 0x00, 0x00, 120 * 0x10000 },
	{ 0x0c, 0xbc, 0x00, 0x00, 124 * 0x10000 },
	{ 0x08, 0xbc, 0x00, 0x00, 126 * 0x10000 },
	{ 0x04, 0xbc, 0x00, 0x00, 127 * 0x10000 },
	{ 0x1c, 0xbc, 0x00, 0x00, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_8m_type13[] = {
	{ 0x00, 0xfc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x00, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x00, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x00, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x00, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x00, 16  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x00, 32  * 0x10000 },
	{ 0x3c, 0xfc, 0x00, 0x00, 64  * 0x10000 },
	{ 0x7c, 0xfc, 0x00, 0x00, 128 * 0x10000 },
};
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
static struct spi_nor_flash_protect_range protect_range_4m_type1[] = {
	{ 0x00, 0x1c, 0x00, 0x40, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x40, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x40, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x40, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x40, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x40, 16  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x40, 32  * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x40, 48  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x40, 56  * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x40, 60  * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x40, 62  * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x40, 63  * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x40, 64  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type2[] = {
	{ 0x00, 0x9c, 0x00, 0x40, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x40, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x40, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x40, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x40, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x40, 16  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x40, 32  * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x40, 48  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x40, 56  * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x40, 60  * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x40, 62  * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x40, 63  * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x40, 64  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type3[] = {
	{ 0x00, 0x1c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x41, 48  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x41, 56  * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x41, 60  * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x41, 62  * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x41, 63  * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x41, 64  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type4[] = {
	{ 0x00, 0x1c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x41, 48  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x41, 56  * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x41, 60  * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x41, 62  * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x41, 63  * 0x10000 },
	{ 0x1c, 0x1c, 0x00, 0x41, 64  * 0x10000 },
};


static struct spi_nor_flash_protect_range protect_range_4m_type5[] = {
	{ 0x00, 0x9c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x14, 0xfc, 0x40, 0x41, 48  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x41, 56  * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x41, 60  * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x41, 62  * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x41, 63  * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x41, 64  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type6[] = {
	{ 0x00, 0x9c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x41, 64  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type7[] = {
	{ 0x00, 0x1c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 32  * 0x10000 },
	{ 0x1c, 0x9c, 0x00, 0x41, 64  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type9[] = {
	{ 0x00, 0xbc, 0x08, 0x08, 0   * 0x10000 },
	{ 0x04, 0xbc, 0x08, 0x08, 1   * 0x10000 },
	{ 0x08, 0xbc, 0x08, 0x08, 2   * 0x10000 },
	{ 0x0c, 0xbc, 0x08, 0x08, 4   * 0x10000 },
	{ 0x10, 0xbc, 0x08, 0x08, 8   * 0x10000 },
	{ 0x14, 0xbc, 0x08, 0x08, 16  * 0x10000 },
	{ 0x18, 0xbc, 0x08, 0x08, 32  * 0x10000 },
	{ 0x1c, 0xbc, 0x08, 0x08, 64  * 0x10000 },
	{ 0x20, 0xbc, 0x08, 0x08, 96  * 0x10000 },
	{ 0x24, 0xbc, 0x08, 0x08, 112 * 0x10000 },
	{ 0x28, 0xbc, 0x08, 0x08, 120 * 0x10000 },
	{ 0x2c, 0xbc, 0x08, 0x08, 124 * 0x10000 },
	{ 0x30, 0xbc, 0x08, 0x08, 126 * 0x10000 },
	{ 0x34, 0xbc, 0x08, 0x08, 127 * 0x10000 },
	{ 0x38, 0xbc, 0x08, 0x08, 128 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type10[] = {
	{ 0x00, 0xbc, 0x08, 0x08, 0   * 0x10000 },
	{ 0x04, 0xbc, 0x08, 0x08, 1   * 0x10000 },
	{ 0x08, 0xbc, 0x08, 0x08, 2   * 0x10000 },
	{ 0x0c, 0xbc, 0x08, 0x08, 4   * 0x10000 },
	{ 0x10, 0xbc, 0x08, 0x08, 8   * 0x10000 },
	{ 0x14, 0xbc, 0x08, 0x08, 16  * 0x10000 },
	{ 0x18, 0xbc, 0x08, 0x08, 32  * 0x10000 },
	{ 0x1c, 0xbc, 0x08, 0x08, 48  * 0x10000 },
	{ 0x20, 0xbc, 0x08, 0x08, 56  * 0x10000 },
	{ 0x24, 0xbc, 0x08, 0x08, 60  * 0x10000 },
	{ 0x28, 0xbc, 0x08, 0x08, 62  * 0x10000 },
	{ 0x2c, 0xbc, 0x08, 0x08, 63  * 0x10000 },
	{ 0x30, 0xbc, 0x08, 0x08, 64  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type11[] = {
	{ 0x00, 0xbc, 0x08, 0x08, 0   * 0x10000 },
	{ 0x04, 0xbc, 0x08, 0x08, 1   * 0x10000 },
	{ 0x08, 0xbc, 0x08, 0x08, 2   * 0x10000 },
	{ 0x0c, 0xbc, 0x08, 0x08, 4   * 0x10000 },
	{ 0x10, 0xbc, 0x08, 0x08, 8   * 0x10000 },
	{ 0x14, 0xbc, 0x08, 0x08, 16  * 0x10000 },
	{ 0x18, 0xbc, 0x08, 0x08, 32  * 0x10000 },
	{ 0x1c, 0xbc, 0x08, 0x08, 64  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type12[] = {
	{ 0x00, 0xbc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x18, 0xbc, 0x00, 0x00, 32  * 0x10000 },
	{ 0x14, 0xbc, 0x00, 0x00, 48  * 0x10000 },
	{ 0x10, 0xbc, 0x00, 0x00, 56  * 0x10000 },
	{ 0x0c, 0xbc, 0x00, 0x00, 60  * 0x10000 },
	{ 0x08, 0xbc, 0x00, 0x00, 62  * 0x10000 },
	{ 0x04, 0xbc, 0x00, 0x00, 63  * 0x10000 },
	{ 0x1c, 0xbc, 0x00, 0x00, 64  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type13[] = {
	{ 0x00, 0xbc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x24, 0xbc, 0x00, 0x00, 1   * 0x10000 },
	{ 0x28, 0xbc, 0x00, 0x00, 2   * 0x10000 },
	{ 0x2c, 0xbc, 0x00, 0x00, 4   * 0x10000 },
	{ 0x30, 0xbc, 0x00, 0x00, 8   * 0x10000 },
	{ 0x34, 0xbc, 0x00, 0x00, 16  * 0x10000 },
	{ 0x38, 0xbc, 0x00, 0x00, 32  * 0x10000 },
	{ 0x3c, 0xbc, 0x00, 0x00, 64  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type14[] = {
	{ 0x20, 0xbc, 0x00, 0x00, 0  * 0x10000 },
	{ 0x24, 0xbc, 0x00, 0x00, 1  * 0x10000 },
	{ 0x28, 0xbc, 0x00, 0x00, 2  * 0x10000 },
	{ 0x2c, 0xbc, 0x00, 0x00, 4  * 0x10000 },
	{ 0x30, 0xbc, 0x00, 0x00, 8  * 0x10000 },
	{ 0x34, 0xbc, 0x00, 0x00, 16 * 0x10000 },
	{ 0x38, 0xbc, 0x00, 0x00, 32 * 0x10000 },
	{ 0x3c, 0xbc, 0x00, 0x00, 64 * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_4m_type15[] = {
	{ 0x00, 0xbc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x24, 0xbc, 0x00, 0x00, 32  * 0x10000 },
	{ 0x28, 0xbc, 0x00, 0x00, 48  * 0x10000 },
	{ 0x2c, 0xbc, 0x00, 0x00, 56  * 0x10000 },
	{ 0x30, 0xbc, 0x00, 0x00, 60  * 0x10000 },
	{ 0x34, 0xbc, 0x00, 0x00, 62  * 0x10000 },
	{ 0x38, 0xbc, 0x00, 0x00, 63  * 0x10000 },
	{ 0x3c, 0xbc, 0x00, 0x00, 64  * 0x10000 },
};

#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
static struct spi_nor_flash_protect_range protect_range_2m_type1[] = {
	{ 0x00, 0x9c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x41, 24  * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x41, 28  * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x41, 30  * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x41, 31  * 0x10000 },
	{ 0x18, 0x98, 0x00, 0x41, 32  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_2m_type2[] = {
	{ 0x00, 0x1c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x10, 0xfc, 0x40, 0x41, 24  * 0x10000 },
	{ 0x0c, 0xfc, 0x40, 0x41, 28  * 0x10000 },
	{ 0x08, 0xfc, 0x40, 0x41, 30  * 0x10000 },
	{ 0x04, 0xfc, 0x40, 0x41, 31  * 0x10000 },
	{ 0x38, 0xfc, 0x00, 0x41, 32  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_2m_type3[] = {
	{ 0x00, 0x9c, 0x00, 0x41, 0   * 0x10000 },
	{ 0x24, 0xfc, 0x00, 0x41, 1   * 0x10000 },
	{ 0x28, 0xfc, 0x00, 0x41, 2   * 0x10000 },
	{ 0x2c, 0xfc, 0x00, 0x41, 4   * 0x10000 },
	{ 0x30, 0xfc, 0x00, 0x41, 8   * 0x10000 },
	{ 0x34, 0xfc, 0x00, 0x41, 16  * 0x10000 },
	{ 0x18, 0x98, 0x00, 0x41, 32  * 0x10000 },
};

static struct spi_nor_flash_protect_range protect_range_2m_type4[] = {
	{ 0x00, 0xbc, 0x00, 0x00, 0   * 0x10000 },
	{ 0x28, 0xbc, 0x00, 0x00, 16  * 0x10000 },
	{ 0x2c, 0xbc, 0x00, 0x00, 24  * 0x10000 },
	{ 0x30, 0xbc, 0x00, 0x00, 28  * 0x10000 },
	{ 0x34, 0xbc, 0x00, 0x00, 30  * 0x10000 },
	{ 0x38, 0xbc, 0x00, 0x00, 31  * 0x10000 },
	{ 0x3c, 0xbc, 0x00, 0x00, 32  * 0x10000 },
};
#endif
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
static struct spi_nor_otp otp_type1  = {0x000000, 0x000000, 64,   1, };
static struct spi_nor_otp otp_type2  = {0x000000, 0x000000, 256,  4, };
static struct spi_nor_otp otp_type3  = {0x000000, 0x000000, 512,  1, };
static struct spi_nor_otp otp_type4  = {0x000000, 0x000000, 1024, 1, };
static struct spi_nor_otp otp_type5  = {0x000000, 0x000100, 256,  4, };
static struct spi_nor_otp otp_type6  = {0x000000, 0x000200, 512,  2, };
static struct spi_nor_otp otp_type7  = {0x000000, 0x000200, 1024, 1, };
static struct spi_nor_otp otp_type8  = {0x000000, 0x000400, 1024, 3, };
static struct spi_nor_otp otp_type9  = {0x000000, 0x001000, 256,  4, };
static struct spi_nor_otp otp_type10 = {0x000010, 0x000000, 496,  1, };
static struct spi_nor_otp otp_type11 = {0x000100, 0x000100, 256,  3, };
static struct spi_nor_otp otp_type12 = {0x001000, 0x001000, 256,  3, };
static struct spi_nor_otp otp_type13 = {0x001000, 0x001000, 1024, 3, };
static struct spi_nor_otp otp_type14 = {0x001000, 0x001000, 2048, 3, };
static struct spi_nor_otp otp_type15 = {0x002000, 0x001000, 1024, 2, };
static struct spi_nor_otp otp_type16 = {0x004000, 0x004000, 256,  3, };
static struct spi_nor_otp otp_type17 = {0x3FD000, 0x001000, 512,  3, };
static struct spi_nor_otp otp_type18 = {0x3FD000, 0x001000, 1024, 3, };
static struct spi_nor_otp otp_type19 = {0x3FF000, 0x000000, 512,  1, };
static struct spi_nor_otp otp_type20 = {0x7FD000, 0x001000, 512,  3, };
static struct spi_nor_otp otp_type21 = {0x7FF000, 0x000000, 512,  1, };
static struct spi_nor_otp otp_type22 = {0xFFD000, 0x001000, 512,  3, };
static struct spi_nor_otp otp_type23 = {0xFFF000, 0x000000, 512,  1, };
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_32M_SIZE
static struct spi_nor_flash_protect protect_32m_type1 = {NULL, 0};
static struct spi_nor_flash_protect protect_32m_type2 = {NULL, 0};
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
static struct spi_nor_flash_protect protect_16m_type1  = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type2  = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type3  = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type4  = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type6  = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type7  = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type9  = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type10 = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type11 = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type12 = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type13 = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type14 = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type15 = {NULL, 0};
static struct spi_nor_flash_protect protect_16m_type16 = {NULL, 0};
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
static struct spi_nor_flash_protect protect_8m_type1  = {NULL, 0};
static struct spi_nor_flash_protect protect_8m_type2  = {NULL, 0};
static struct spi_nor_flash_protect protect_8m_type3  = {NULL, 0};
static struct spi_nor_flash_protect protect_8m_type4  = {NULL, 0};
static struct spi_nor_flash_protect protect_8m_type7  = {NULL, 0};
static struct spi_nor_flash_protect protect_8m_type8  = {NULL, 0};
static struct spi_nor_flash_protect protect_8m_type9  = {NULL, 0};
static struct spi_nor_flash_protect protect_8m_type10 = {NULL, 0};
static struct spi_nor_flash_protect protect_8m_type11 = {NULL, 0};
static struct spi_nor_flash_protect protect_8m_type12 = {NULL, 0};
static struct spi_nor_flash_protect protect_8m_type13 = {NULL, 0};
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
static struct spi_nor_flash_protect protect_4m_type1  = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type2  = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type3  = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type4  = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type5  = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type6  = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type7  = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type9  = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type10 = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type11 = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type12 = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type13 = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type14 = {NULL, 0};
static struct spi_nor_flash_protect protect_4m_type15 = {NULL, 0};
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
static struct spi_nor_flash_protect protect_2m_type1  = {NULL, 0};
static struct spi_nor_flash_protect protect_2m_type2  = {NULL, 0};
static struct spi_nor_flash_protect protect_2m_type3  = {NULL, 0};
static struct spi_nor_flash_protect protect_2m_type4  = {NULL, 0};
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_WRITE_PROTECT
void gx_spi_nor_protect_init(void)
{
#ifdef CONFIG_GX_MTD_SPI_NOR_32M_SIZE
	protect_32m_type1.sf_range = protect_range_32m_type1;
	protect_32m_type1.count = WRITE_COUNT(protect_range_32m_type1);

	protect_32m_type2.sf_range = protect_range_32m_type2;
	protect_32m_type2.count = WRITE_COUNT(protect_range_32m_type2);
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	protect_16m_type1.sf_range = protect_range_16m_type1;
	protect_16m_type1.count = WRITE_COUNT(protect_range_16m_type1);

	protect_16m_type2.sf_range = protect_range_16m_type2;
	protect_16m_type2.count = WRITE_COUNT(protect_range_16m_type2);

	protect_16m_type3.sf_range = protect_range_16m_type3;
	protect_16m_type3.count = WRITE_COUNT(protect_range_16m_type3);

	protect_16m_type4.sf_range = protect_range_16m_type4;
	protect_16m_type4.count = WRITE_COUNT(protect_range_16m_type4);

	protect_16m_type6.sf_range = protect_range_16m_type6;
	protect_16m_type6.count = WRITE_COUNT(protect_range_16m_type6);

	protect_16m_type7.sf_range = protect_range_16m_type7;
	protect_16m_type7.count = WRITE_COUNT(protect_range_16m_type7);

	protect_16m_type9.sf_range = protect_range_16m_type9;
	protect_16m_type9.count = WRITE_COUNT(protect_range_16m_type9);

	protect_16m_type10.sf_range = protect_range_16m_type10;
	protect_16m_type10.count = WRITE_COUNT(protect_range_16m_type10);

	protect_16m_type11.sf_range = protect_range_16m_type11;
	protect_16m_type11.count = WRITE_COUNT(protect_range_16m_type11);

	protect_16m_type12.sf_range = protect_range_16m_type12;
	protect_16m_type12.count = WRITE_COUNT(protect_range_16m_type12);

	protect_16m_type13.sf_range = protect_range_16m_type13;
	protect_16m_type13.count = WRITE_COUNT(protect_range_16m_type13);

	protect_16m_type14.sf_range = protect_range_16m_type14;
	protect_16m_type14.count = WRITE_COUNT(protect_range_16m_type14);

	protect_16m_type15.sf_range = protect_range_16m_type15;
	protect_16m_type15.count = WRITE_COUNT(protect_range_16m_type15);

	protect_16m_type16.sf_range = protect_range_16m_type16;
	protect_16m_type16.count = WRITE_COUNT(protect_range_16m_type16);

#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	protect_8m_type1.sf_range = protect_range_8m_type1;
	protect_8m_type1.count = WRITE_COUNT(protect_range_8m_type1);

	protect_8m_type2.sf_range = protect_range_8m_type2;
	protect_8m_type2.count = WRITE_COUNT(protect_range_8m_type2);

	protect_8m_type3.sf_range = protect_range_8m_type3;
	protect_8m_type3.count = WRITE_COUNT(protect_range_8m_type3);

	protect_8m_type4.sf_range = protect_range_8m_type4;
	protect_8m_type4.count = WRITE_COUNT(protect_range_8m_type4);

	protect_8m_type7.sf_range = protect_range_8m_type7;
	protect_8m_type7.count = WRITE_COUNT(protect_range_8m_type7);

	protect_8m_type8.sf_range = protect_range_8m_type8;
	protect_8m_type8.count = WRITE_COUNT(protect_range_8m_type8);

	protect_8m_type9.sf_range = protect_range_8m_type9;
	protect_8m_type9.count = WRITE_COUNT(protect_range_8m_type9);

	protect_8m_type10.sf_range = protect_range_8m_type10;
	protect_8m_type10.count = WRITE_COUNT(protect_range_8m_type10);

	protect_8m_type11.sf_range = protect_range_8m_type11;
	protect_8m_type11.count = WRITE_COUNT(protect_range_8m_type11);

	protect_8m_type12.sf_range = protect_range_8m_type12;
	protect_8m_type12.count = WRITE_COUNT(protect_range_8m_type12);

	protect_8m_type13.sf_range = protect_range_8m_type13;
	protect_8m_type13.count = WRITE_COUNT(protect_range_8m_type13);
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	protect_4m_type1.sf_range = protect_range_4m_type1;
	protect_4m_type1.count = WRITE_COUNT(protect_range_4m_type1);

	protect_4m_type2.sf_range = protect_range_4m_type2;
	protect_4m_type2.count = WRITE_COUNT(protect_range_4m_type2);

	protect_4m_type3.sf_range = protect_range_4m_type3;
	protect_4m_type3.count = WRITE_COUNT(protect_range_4m_type3);

	protect_4m_type4.sf_range = protect_range_4m_type4;
	protect_4m_type4.count = WRITE_COUNT(protect_range_4m_type4);

	protect_4m_type5.sf_range = protect_range_4m_type5;
	protect_4m_type5.count = WRITE_COUNT(protect_range_4m_type5);

	protect_4m_type6.sf_range = protect_range_4m_type6;
	protect_4m_type6.count = WRITE_COUNT(protect_range_4m_type6);

	protect_4m_type7.sf_range = protect_range_4m_type7;
	protect_4m_type7.count = WRITE_COUNT(protect_range_4m_type7);

	protect_4m_type9.sf_range = protect_range_4m_type9;
	protect_4m_type9.count = WRITE_COUNT(protect_range_4m_type9);

	protect_4m_type10.sf_range = protect_range_4m_type10;
	protect_4m_type10.count = WRITE_COUNT(protect_range_4m_type10);

	protect_4m_type11.sf_range = protect_range_4m_type11;
	protect_4m_type11.count = WRITE_COUNT(protect_range_4m_type11);

	protect_4m_type12.sf_range = protect_range_4m_type12;
	protect_4m_type12.count = WRITE_COUNT(protect_range_4m_type12);

	protect_4m_type13.sf_range = protect_range_4m_type13;
	protect_4m_type13.count = WRITE_COUNT(protect_range_4m_type13);

	protect_4m_type14.sf_range = protect_range_4m_type14;
	protect_4m_type14.count = WRITE_COUNT(protect_range_4m_type14);

	protect_4m_type15.sf_range = protect_range_4m_type15;
	protect_4m_type15.count = WRITE_COUNT(protect_range_4m_type15);
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	protect_2m_type1.sf_range = protect_range_2m_type1;
	protect_2m_type1.count = WRITE_COUNT(protect_range_2m_type1);

	protect_2m_type2.sf_range = protect_range_2m_type2;
	protect_2m_type2.count = WRITE_COUNT(protect_range_2m_type2);

	protect_2m_type3.sf_range = protect_range_2m_type3;
	protect_2m_type3.count = WRITE_COUNT(protect_range_2m_type3);

	protect_2m_type4.sf_range = protect_range_2m_type4;
	protect_2m_type4.count = WRITE_COUNT(protect_range_2m_type4);
#endif

}
#endif

static void read_sfdp(struct spi_device *spi, unsigned int addr, unsigned char *rbuf, unsigned int rbuf_len)
{
	unsigned char tx_buf[5];

	tx_buf[0]=0x5A;
	tx_buf[1] = addr >> 16;
	tx_buf[2] = addr >> 8;
	tx_buf[3] = addr >> 0;
	spi_write_then_read(spi, tx_buf, 5, rbuf, rbuf_len);
}

int get_flash_markid(struct spi_device *spi, unsigned int jedec_id, spi_nor_flash_markid mark_id)
{
	spi_nor_flash_markid markid = UNKNOW_MARKID;
	int if_blur = 0;
	unsigned char rbuf;
	switch(jedec_id){
	case 0xc22016: //MX25L32
		read_sfdp(spi, 0x64, &rbuf, 1);
		if (rbuf == 0x9E)
			markid = MX25L3233F;
		else if (rbuf == 0xF6) {
			markid = MX25L3206E;
		}
		else {
			markid = MX25L32;
			if_blur = 1;
			gxlog_w("[Warning]: unsupported flash type, blur matches to MX25L32");
		}
		break;
	case 0xc22017: //MX25L64
		read_sfdp(spi, 0x32, &rbuf, 1);
		if (rbuf == 0x81)
			markid = MX25L6406E;
		else if (rbuf == 0xf1) {
			read_sfdp(spi, 0x68, &rbuf, 1);
			if (rbuf == 0xfe) {
				read_sfdp(spi, 0x1f2, &rbuf, 1);
				if (rbuf == 0x21)
					markid = MX25L6433F;
				else if (rbuf == 0x24)
					markid = KH25L6436F;
			} else if (rbuf == 0x85)
				markid = KH25L6436F;
		} else if (rbuf == 0x00) {
			markid = MX25L6408E;
		}
		else {
			markid = MX25L64;
			if_blur = 1;
			gxlog_w("[Warning]: unsupported flash type, blur matches to MX25L64");
		}
		break;
	case 0xc84018:
		read_sfdp(spi, 0x40, &rbuf, 1);
		if (rbuf == 0xfe)
			markid = GD25Q128C;
		else if (rbuf == 0xee)
			markid = GD25Q127C;
		else if (rbuf == 0xff)
			markid = GD25Q128B;
		else {
			markid = GD25Q128;
			if_blur = 1;
			gxlog_w("[Warning]: unsupported flash type, blur matches to GD25Q128");
		}
		break;
	case 0x00b4016:
		read_sfdp(spi, 0x04, &rbuf, 1);
		if (rbuf == 0x06)
			markid = XT25F32F;
		else
			markid = XT25F32B_S;
		break;
	case 0x00b4017:
		read_sfdp(spi, 0x04, &rbuf, 1);
		if (rbuf == 0x06)
			markid = XT25F64F;
		else
			markid = XT25F64B_S;
		break;
	case 0x0204016:
		read_sfdp(spi, 0x32, &rbuf, 1);
		if ((rbuf & 0x8)) // bit3
			markid = XM25QH32D;
		else
			markid = XM25QH32B_XM25QH32C_XM25QE32C;
		break;
	case 0x0204017:
		read_sfdp(spi, 0x32, &rbuf, 1);
		if ((rbuf & 0x8)) // bit3
			markid = XM25QH64D;
		else
			markid = XM25QH64C;
		break;
	case 0x0204018:
		read_sfdp(spi, 0x32, &rbuf, 1);
		if ((rbuf & 0x8)) // bit3
			markid = XM25QH128D;
		else
			markid = XM25QH128C;
		break;
	case 0x05e4016:
		read_sfdp(spi, 0x4, &rbuf, 1);
		if (rbuf & 0x6)
			markid = ZB25VQ32_B;
		else if(rbuf & 0x8)
			markid = ZB25VQ32D;
		break;
	case 0x05e4017:
		read_sfdp(spi, 0x4, &rbuf, 1);
		if (rbuf & 0x6)
			markid = ZB25VQ64_A;
		else if(rbuf & 0x8)
			markid = ZB25VQ64B_C_D;
		break;
	case 0x05e4018:
		read_sfdp(spi, 0x4, &rbuf, 1);
		if (rbuf & 0x6)
			markid = ZB25VQ128A;
		else if(rbuf & 0x8)
			markid = ZB25VQ128D;
		break;
	default:
		gxlog_e("get_flash_markid error, please check.\n");
		break;
	}

	if (markid == mark_id && if_blur == 0)
		return 0;

	if (markid == mark_id && if_blur == 1) // blur matches
		return 1;

	return -1;
}

spi_nor_info g_spi_nor_data[] = {
	{
		"Unsupported Flash Type",
		0xffffffff,
		0x1000000,
		0,
		NULL,
		0,
		NULL,
		NULL,
	}, // default info

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	//en25qxx spi flash, eon silicon solution
	{
		"EN25F16",
		0x001c3115,
		0x200000,
		0,
		NULL,
		0,
		NULL,
		NULL,
	},   //en25f16-100HIP 16 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	{
		"EN25Q16",
		0x001c3015,
		0x200000,
		0,
		NULL,
		0,
		NULL,
	},   // en25q16 16 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	{
		"EN25QH16",
		0x001c7015,
		0x200000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		NULL,
		NULL,
	},   // en25qh16-104HIP 16 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"EN25F32",
		0x001c3116,
		0x400000,
		0,
		NULL,
		0,
		NULL,
		NULL,
	},   // en25f32 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"EN25Q32",
		0x001c3016,
		0x400000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_4m_type12,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type19,
#endif
	},   // en25q32b-104HIP 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"EN25QE32A",
		0x001c4216,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type2,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type18,
#endif
	},   // en25qe32a-104HIP 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"EN25QH32B",
		0x001c7016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type10,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type17,
#endif
	},   // en25qh32b-104HIP 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"EN25Q64",
		0x001c3017,
		0x800000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_8m_type12,
		NULL,
	},   // en25q64-104HIP 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"EN25QA64A",
		0x001c6017,
		0x800000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_8m_type7,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type21,
#endif
	},   // en25qa64a 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"EN25QX64A",
		0x001c7117,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type20,
#endif
	},   // en25qx64a 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"EN25QX128A",
		0x001c7118,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type22,
#endif
	},   // en25qx128a 128 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"EN25QA128A",
		0x001c6018,
		0x1000000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_16m_type15,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type23,
#endif
	},   // en25qa64a 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"EN25Q128",
		0x001c3018,
		0x1000000,
		0,
		NULL,
		0,
		&protect_16m_type13,
		NULL,
	},   // en25q128_d 128 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"EN25QH128",
		0x001c7018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type14,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type23,
#endif
	},   // en25qh128_d 128 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	//f25lxx spi flash, elite semiconductor
	{
		"F25L016",
		0x008c2115,
		0x200000,
		0,
		NULL,
		0,
		NULL,
		NULL,
	},   // f25l16pa  16 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"F25L032",
		0x008c2116,
		0x400000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_4m_type15,
		NULL,
	},   // f25l032qa 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	//gd25xx spi flash, elite semiconductor
	{
		"GD25Q16",
		0x00c84015,
		0x200000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_2m_type3,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type2,
#endif
	},   // gd25q16 16 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	//gd25xx spi flash, elite semiconductor
	{
		"GD25VE16C",
		0x00c84215,
		0x200000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_2m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type2,
#endif
	},   // gd25q16 16 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"GD25Q32",
		0x00c84016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type2,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},   // gd25q32 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"GD25Q64",
		0x00c84017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type2,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},   // gd25q64 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"GD25Q127C/GD25Q128E",
		0x00c84018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		GD25Q127C,
		&protect_16m_type2,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},   // gd25q128 128 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"GD25Q128B",
		0x00c84018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		GD25Q128B,
		&protect_16m_type16,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type5,
#endif
	},   // gd25q128 128 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"GD25Q128C",
		0x00c84018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		GD25Q128C,
		&protect_16m_type2,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},   // gd25q128 128 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"GD25Q128",
		0x00c84018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		GD25Q128,
		NULL,
		NULL,
	},   // gd25q128xxx 128 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"N25Q032",
		0x0020ba16,
		0x400000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_4m_type13,
		NULL,
	},   // flash n25q032pa 32MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"N25Q064",
		0x0020ba17,
		0x800000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_8m_type13,
		NULL,
	},   // flash n25q064 64MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"N25Q128",
		0x0020ba18,
		0x1000000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_16m_type12,
		NULL,
	},   // flash n25q128 128MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	//mx25lxx spi flash,MXIC
	{
		"MX25L16",
		0x00c22015,
		0x200000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_2m_type4,
		NULL,
	},    // mx25l1606E 16 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"MX25L3206E",
		0x00c22016,
		0x400000,
		SPI_NOR_DUAL_READ,
		get_flash_markid,
		MX25L3206E,
		&protect_4m_type15,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		//mxic flash OTP hardware does not support the erase operation
		&otp_type1,
#endif
	},   // mx25l3206E 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"MX25L3233F",
		0x00c22016,
		0x400000,
		SPI_NOR_DUAL_READ,
		get_flash_markid,
		MX25L3233F,
		&protect_4m_type11,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		//mxic flash OTP hardware does not support the erase operation
		&otp_type3,
#endif
	},   // mx25l3233f 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"MX25L32",
		0x00c22016,
		0x400000,
		0,
		get_flash_markid,
		MX25L32,
		NULL,
		NULL,
	},   // mx25l32xxx 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"MX25L6406E",
		0x00c22017,
		0x800000,
		SPI_NOR_DUAL_READ,
		get_flash_markid,
		MX25L6406E,
		&protect_8m_type11,
	},   // mx25l6406E 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"MX25L6433F",
		0x00c22017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		MX25L6433F,
		&protect_8m_type9,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		//mxic flash OTP hardware does not support the erase operation
		&otp_type7,
#endif
	},   // mx25l6433f 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"KH25L6436F",
		0x00c22017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		KH25L6436F,
		&protect_8m_type10,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		//mxic flash OTP hardware does not support the erase operation
		&otp_type6,
#endif
	},   // mx25l6433f 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"MX25L6408E",
		0x00c22017,
		0x800000,
		SPI_NOR_DUAL_READ,
		get_flash_markid,
		MX25L6408E,
		&protect_8m_type11,
		NULL,
	},   // mx25l6408E 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"MX25L64",
		0x00c22017,
		0x800000,
		SPI_NOR_DUAL_READ,
		get_flash_markid,
		MX25L64,
		NULL,
		NULL,
	},   // mx25l64xxx 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		/* MX25L12833FM2I-10G、MX25L12835F、KH25L12833FM2I-10G */
		"MX25L128/KH25L128",
		0x00c22018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type9,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		//mxic flash OTP hardware does not support the erase operation
		&otp_type3,
#endif
	},   // mx25l12835E 128 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_32M_SIZE
	{
		"MX25L256",
		0x00c22019,
		0x2000000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_32m_type2,
		NULL,
	},   // mx25l25635F 256 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	//w25qxx spi flash, winbond
	{
		"W25Q16",
		0x00ef4015,
		0x200000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		NULL,
		NULL,
	},   // w25q16bv /W25Q16 16 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"W25Q32",
		0x00ef4016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},   // w25q32bv /W25Q32 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"W25Q32DW",
		0x00ef6016,
		0x400000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		NULL,
		NULL,
	},   // w25q32dw 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"W25Q64",
		0x00ef4017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},   // w25q64bv /W25Q64 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"W25Q128",
		0x00ef4018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},   // w25q128bv /W25Q128 128 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_32M_SIZE
	{
		"W25Q256",
		0x00ef4019,
		0x2000000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_32m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},   // w25q256bv /W25Q256 256 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	//pm25lqxx spi flash, pmc
	{
		"PM25LQ032",
		0x007f9d46,
		0x400000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_4m_type14,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		//pmc flash OTP hardware does not support the erase operation
		&otp_type1,
#endif
	},   //pm25lq032  32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	/* paragon */
	{
		"PN25F32",
		0x00e04016,
		0x400000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_4m_type7,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type11,
#endif
	},   //pn25f32  32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"EN25QH64A/PN25F64B",
		0x001c7017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type7,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type21,
#endif
	},   //pn25f64b 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	/* issi */
	{
		"IC25LP064",
		0x009d6017,
		0x800000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_8m_type8,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type9,
#endif
	},   //ic25lp064  64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	/* zb */
	{
		"ZB25VQ16A",
		0x005e6015,
		0x200000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_2m_type2,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	}, //zb25vq32 32 Mbit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	/* zb */
	{
		"ZB25VQ32/ZB25VQ32B/T25S32",
		0x005e4016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		ZB25VQ32_B,
		&protect_4m_type3,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	}, //zb25vq32 32 Mbit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	/* zb */
	{
		"ZB25VQ32D",
		0x005e4016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		ZB25VQ32D,
		&protect_4m_type3,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	}, //zb25vq32 32 Mbit devices.
#endif


#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"ZB25VQ64/ZB25VQ64A",
		0x005e4017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		ZB25VQ64_A,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	}, //zb25vq64 64 Mbit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"ZB25VQ64B/ZB25VQ64C/ZB25VQ64D",
		0x005e4017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		ZB25VQ64B_C_D,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	}, //zb25vq64 64 Mbit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	/* zb */
	{
		"ZB25VQ128A",
		0x005e4018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		ZB25VQ128A,
		&protect_16m_type3,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	}, //zb25vq128 128 Mbit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	/* zb */
	{
		"ZB25VQ128D",
		0x005e4018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		ZB25VQ128D,
		&protect_16m_type3,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	}, //zb25vq128 128 Mbit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	/* byt */
	{
		"BY25Q32BS/BH25Q32BS/BY25Q32ES",
		0x00684016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type5,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},  //by25q32bs 32 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"BY25Q64AS",
		0x00684017,
		0x800000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_8m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},  //by25q64bs 64 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"BY25Q128AS/BY25Q128ES",
		0x00684018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},  //by25q128bs 128 MBit devices.
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_2M_SIZE
	/* xmc */
	{
		"XM25QH16C",
		0x00204015,
		0x200000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_2m_type2,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"XM25QH32B/XM25QH32C/XM25QE32C",
		0x00204016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		XM25QH32B_XM25QH32C_XM25QE32C,
		&protect_4m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"XM25QH32D",
		0x00204016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		XM25QH32D,
		&protect_4m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"XM25QH64A",
		0x00207017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type9,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type21,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"XM25QH64B",
		0x00206017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type8,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type9,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"XM25QH64C",
		0x00204017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		XM25QH64C,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"XM25QH64D",
		0x00204017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		XM25QH64D,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"XM25QH128A",
		0x00207018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type10,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type23,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"XM25QH128B",
		0x00206018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type11,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type9,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"XM25QH128C",
		0x00204018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		XM25QH128C,
		&protect_16m_type3,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"XM25QH128D",
		0x00204018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		XM25QH128D,
		&protect_16m_type3,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_32M_SIZE
	{
		"XM25QH256C",
		0x00204019,
		0x2000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_32m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"FM25Q64A",
		0x00f83217,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type3,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type3,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"DS25Q4AA",
		0x00e53118,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"DS25Q64A",
		0x00e53117,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type3,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"FM25W32",
		0x00a12816,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type6,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type5,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		//brand: 复旦微
		"FM25Q128A",
		0x00a14018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type7,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type5,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		//brand: FORESEE(江波龙)/复旦微
		"FS25Q064/FM25Q64/FM25Q64AI3",
		0x00a14017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type5,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"FS25Q032/FM25Q32BI3",
		0x00a14016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type5,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type4,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"F25SQA032M",
		0x00494016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type5,
		NULL,
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"XT25F32B-S",
		0x000b4016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		XT25F32B_S,
		&protect_4m_type5,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type5,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"XT25F32F",
		0x000b4016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		XT25F32F,
		&protect_4m_type5,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type15,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"XT25F64B-S",
		0x000b4017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		XT25F64B_S,
		&protect_8m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type5,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"XT25F64F",
		0x000b4017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		get_flash_markid,
		XT25F64F,
		&protect_8m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type15,
#endif
	},
#endif
#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"XT25F128F",
		0x000b4018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"PY25Q32HB",
		0x00852016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type5,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"PY25Q64HA",
		0x00852017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type2,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"PY25Q128HA",
		0x00852018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"ZD25Q32",
		0x00ba4016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type1,
		NULL,
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"ZD25Q64A",
		0x00ba4017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type1,
		NULL,
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"ZD25Q64B",
		0x00ba3217,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type10,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
	{
		"JY25VQ128A",
		0x001c4018,
		0x1000000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_16m_type4,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type16,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"UC25HQ32IA",
		0x00b36016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"UC25HQ64IA",
		0x00b36017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"SX25Q64A",
		0x008f4017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type16,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"GT25Q32A",
		0x00c46016,
		0x400000,
		SPI_NOR_DUAL_READ,
		NULL,
		0,
		&protect_4m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type8,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"TH25Q32HA",
		0x00cd6016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type2,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type14,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
	{
		"TH25Q64HA",
		0x00cd6017,
		0x800000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type12,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"BW25Q32A",
		0x00944016,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type2,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type13,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"XM25Q55A",
		0x00332705,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_4m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type10,
#endif
	},
#endif

#ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
	{
		"XM25Q55A",
		0x00332706,
		0x400000,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
		NULL,
		0,
		&protect_8m_type1,
#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
		&otp_type1,
#endif
	},
#endif

};

int g_spi_nor_data_count = (sizeof(g_spi_nor_data) / sizeof((g_spi_nor_data)[0]));
