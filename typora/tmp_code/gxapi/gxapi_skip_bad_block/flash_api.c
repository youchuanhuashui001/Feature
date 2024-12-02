#include <stdio.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>

#include <cyg/io/io.h>
#include <cyg/io/disk.h>
#include <cyg/io/flash.h>

#include "common/gxlog.h"
#include <gxflash_api.h>

#define GX_FLASH_ID_LEN  3
#define GX_FLASH_MID_LEN 1
#define GX_FLASH_DID_LEN 2

#define DEVNAME "/dev/flash/0/0"
#define MIN(a, b)	(((a) < (b)) ? (a) : (b))

typedef long		__kernel_loff_t;
typedef __kernel_loff_t		loff_t;

static int fd = 0;

static int  gxflash_range_check(int fd, \
			unsigned int start_addr, unsigned int length)
{
	int ret = 0;
	cyg_io_flash_getconfig_devsize_t flash_dev;

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_DEVSIZE, &flash_dev);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}

	if (start_addr + length > flash_dev.dev_size)
		ret = -1;

	return ret;
}

int GxFlash_Open(void)
{
	int ret = 0;

	if (fd > 0)
		goto exit;

	fd = open(DEVNAME, O_RDWR);

	if (fd < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__);
		ret = -1;
	}

exit:
	return ret;
}

int GxFlash_ReadID(unsigned char *id, size_t len)
{
	int ret = 0;
	int i;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}
	if (!info.jedec_id) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return 0;
	}
	if (len > GX_FLASH_ID_LEN)
		len = GX_FLASH_ID_LEN;
	for (i = len-1; i >= 0; i--) {
		id[i] = info.jedec_id>>(i*8) & 0xff;
	}
	return len;
exit:
	return ret;
}

int GxFlash_ReadDeviceID(unsigned char *id, size_t len)
{
	int ret = 0;
	int i;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}
	if (!info.jedec_id) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return 0;
	}
	if (len > GX_FLASH_DID_LEN)
		len = GX_FLASH_DID_LEN;
	for (i = len-1; i >= 0; i--) {
		id[i] = info.jedec_id>>(i*8) & 0xff;
	}
	return len;
exit:
	return ret;
}

int GxFlash_ReadManufactoryID(unsigned char *id, size_t len)
{
	int ret = 0;
	int i;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}
	if (!info.jedec_id) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return 0;
	}
	if (len > GX_FLASH_MID_LEN)
		len = GX_FLASH_MID_LEN;
	for (i = 0; i < len; i++) {
		id[i] = info.jedec_id>>((i+GX_FLASH_DID_LEN)*8) & 0xff;
	}
	return len;
exit:
	return ret;
}

int GxFlash_ReadUniqueID(unsigned char *id, size_t len)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}
	if (!info.uniqueid.len) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return -1;
	}
	if (len > info.uniqueid.len)
		len = info.uniqueid.len;
	memcpy(id, info.uniqueid.id, len);
	return len;
exit:
	return ret;
}

void GxFlash_Close(void)
{
	if (fd > 0) {
		close(fd);
		fd = 0;
	}
}

int GxFlash_BlockSize(void)
{
	int ret = 0;
	cyg_io_flash_getconfig_blocksize_t flash_block;

	flash_block.offset = 0x10000;
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_BLOCKSIZE, &flash_block);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}
	ret = flash_block.block_size;

exit:
	return ret;
}

int GxFlash_SectorSize(void)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}
	return info.sector_size;
}

int GxFlash_OOBSize(void)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}

	if (info.chip_type == FLASH_TYPE_NOR) {
		gxlogi("error: [%s : %d], Nor Flash Unsupport OOB.\n", __func__, __LINE__ );
		return -1;
	}

	return info.oob_size;
}

int GxFlash_PageSize(void)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}
	return info.page_size;
}

int GxFlash_EraseSize(void)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}
	return info.erase_size;
}

int GxFlash_MatchMode(enum gxflash_match_mode *mode)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}
	*mode = info.match_mode;
	return 0;
}

int GxFlash_ProtectMode(enum gxflash_protect_mode *mode)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}
	*mode = info.protect_mode;
	return 0;
}

int GxFlash_GetProtectLen(void)
{
	int ret = 0;
	cyg_io_flash_getconfig_unlock_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_STATUS, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}
	return info.protect_addr;
}

int GxFlash_Type(enum gxflash_chip_type *type)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}
	*type = info.chip_type;
	return 0;
}

int GxFlash_DevSize(void)
{
	int ret = 0;
	cyg_io_flash_getconfig_devsize_t flash_dev;

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_DEVSIZE, &flash_dev);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}

	ret = flash_dev.dev_size;

exit:
	return ret;
}

char *GxFlash_Name(void)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return NULL;
	}
	return (char *)info.chip_name;
}

static int check_skip_len(loff_t offset, size_t length)
{
	size_t len_excl_bad = 0;
	int ret = 0;
	int rval = 0;
	cyg_io_flash_nand_block_is_bad_t block_is_bad = {0};
	cyg_io_flash_getconfig_info_t info = {0};

	rval = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (rval < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return rval;
	}

	while (len_excl_bad < length) {
		size_t block_len, block_off;
		loff_t block_start;

		if (offset >= info.chip_size)
			return -1;

		block_start = offset & ~(loff_t)(info.block_size - 1);
		block_off = offset & (info.block_size - 1);
		block_len = info.block_size - block_off;

		block_is_bad.ofs = block_start;
		rval = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_NAND_BLOCK_IS_BAD, &block_is_bad);
		if (rval < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
			return rval;
		}
		if (!block_is_bad.is_bad)
			len_excl_bad += block_len;
		else
			ret = 1;

		offset += block_len;
	}

	return ret;
}

static int spinand_write_skip_bad( loff_t offset, size_t *length, u_char *buffer, int flags)
{
	int ret = 0, blocksize;
	size_t left_to_write = *length;
	u_char *p_buffer = buffer;
	int need_skip;
	cyg_io_flash_getconfig_info_t info = {0};
	cyg_io_flash_nand_block_is_bad_t block_is_bad = {0};

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}

	unsigned int spinand_erasesize = info.block_size;
	unsigned int spinand_writesize = info.page_size;

	blocksize = spinand_erasesize;

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
	if ((offset & (spinand_writesize - 1)) != 0) {
		gxlogi("error: Attempt to write non page aligned data!\n");
		*length = 0;
		return -EINVAL;
	}

	need_skip = check_skip_len(offset, *length);
	if (need_skip < 0) {
		gxlogi("error: Attempt to write outside the flash area!\n");
		*length = 0;
		return -EINVAL;
	}

	while (left_to_write > 0) {
		size_t block_offset = offset & (spinand_erasesize - 1);
		size_t write_size, truncated_write_size;

		block_is_bad.ofs = offset;
		ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_NAND_BLOCK_IS_BAD, &block_is_bad);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
			return ret;
		}

		if (block_is_bad.is_bad) {
			gxlogi("Skipping bad block 0x%08x\n", (offset & ~(spinand_erasesize - 1)));
			offset += spinand_erasesize - block_offset;
			continue;
		}

		if (left_to_write < (blocksize - block_offset))
			write_size = left_to_write;
		else
			write_size = blocksize - block_offset;

		truncated_write_size = write_size;

		ret = lseek(fd, offset, SEEK_SET);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__);
			return ret;
		}

		ret = write(fd, p_buffer, truncated_write_size);
		if (ret < 0) {
			cyg_io_flash_nand_block_markbad_t markbad = {0};

			markbad.ofs = offset/info.block_size*info.block_size;

			ret = ioctl(fd, CYG_IO_SET_CONFIG_FLASH_NAND_BLOCK_MARKBAD, &markbad,
					sizeof(cyg_io_flash_nand_block_markbad_t));

			if (ret == 0)
				gxlogi("write failed,mark bad block at 0x%08x\n", offset/info.block_size*info.block_size);
			else
				gxlogi("%s: mark bad block failed: %d\n", info.chip_name, ret);
			return -1;


		}

		offset += write_size;
		p_buffer += write_size;

		left_to_write -= write_size;
	}

	return 0;
}

static int spinand_read_skip_bad(loff_t offset, size_t *length,
		u_char *buffer)
{
	int need_skip;
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	cyg_io_flash_nand_block_is_bad_t block_is_bad = {0};

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}

	unsigned int spinand_erasesize = info.block_size;
	unsigned int spinand_writesize = info.page_size;

	u_char *p_buffer = buffer;
	size_t left_to_read = *length;

	if ((offset & (spinand_writesize- 1)) != 0) {
		*length = 0;
		gxlogi("error: Attempt to read non page aligned data!\n");
		return -EINVAL;
	}

	if ((offset + *length) > info.chip_size) {
		gxlogi("error: read offset+length > spi nand size.");
		return -1;
	}

	if (!*length)
		return 0;

	need_skip = check_skip_len(offset, *length);
	if (need_skip < 0) {
		gxlogi("error: Attempt to read outside the flash area!\n");
		*length = 0;
		return -EINVAL;
	}
	if (!need_skip) {
		ret = lseek(fd, offset, SEEK_SET);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__);
			return ret;
		}

		ret = read(fd, buffer, *length);
		if (ret >= 0)
			return 0;

		*length = 0;
		gxlogi("SPI NAND read1 from offset %08x failed %d\n", offset, ret);
		return ret;
	}

	while (left_to_read > 0) {

		size_t block_offset = offset & (spinand_erasesize - 1);
		size_t read_length;

		block_is_bad.ofs = offset;
		ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_NAND_BLOCK_IS_BAD, &block_is_bad);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
			return ret;
		}

		if (block_is_bad.is_bad) {
			gxlogi("Skipping bad block 0x%08x\n", (offset & ~(spinand_erasesize - 1)));
			offset += spinand_erasesize - block_offset;
			continue;
		}

		if (left_to_read < (spinand_erasesize - block_offset))
			read_length = left_to_read;
		else
			read_length = spinand_erasesize - block_offset;

		ret = lseek(fd, offset, SEEK_SET);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__);
			return ret;
		}

		ret = read(fd, p_buffer, read_length);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__);
			return ret;
		}

		left_to_read -= read_length;
		offset       += read_length;
		p_buffer     += read_length;
	}

	return 0;
}

int GxFlash_Read(unsigned int start_addr, unsigned int length, unsigned char *buffer)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};

	ret = gxflash_range_check(fd, start_addr, length);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__);
		goto exit;
	}

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}

	if (info.chip_type == FLASH_TYPE_NOR) {
		ret = lseek(fd, start_addr, SEEK_SET);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__);
			goto exit;
		}

		ret = read(fd, buffer, length);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__);
			goto exit;
		}
	} else if (info.chip_type == FLASH_TYPE_SPINAND) {
		unsigned int spinand_erasesize = info.block_size;
		unsigned int spinand_writesize = info.page_size;
		unsigned int page_offset = start_addr & (spinand_writesize - 1);

		if (page_offset) {
			unsigned int tmp_len;
			unsigned char *buf = malloc(spinand_writesize);
			cyg_io_flash_nand_block_is_bad_t block_is_bad = {0};

			while (1) {
				block_is_bad.ofs = start_addr;
				ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_NAND_BLOCK_IS_BAD, &block_is_bad);
				if (ret < 0) {
					gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
					goto exit;
				}

				if (block_is_bad.is_bad) {
					gxlogi("warning: skip bad block at 0x%x\n", start_addr & ~(spinand_erasesize -1));
					start_addr += spinand_erasesize;
				} else
					break;
			}
			tmp_len = MIN(spinand_writesize - page_offset, length);
			ret = spinand_read_skip_bad(start_addr - page_offset, &spinand_writesize, buf);
			memcpy(buffer, buf + page_offset, tmp_len);
			start_addr += tmp_len;
			buffer += tmp_len;
			length -= tmp_len;
			free(buf);
		}
		if (length)
			ret = spinand_read_skip_bad(start_addr, &length, buffer);
	}

exit:
	return ret;
}

int GxFlash_Write(unsigned int start_addr, unsigned int length, unsigned char *buffer)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};

	ret = gxflash_range_check(fd, start_addr, length);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__);
		goto exit;
	}

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}

	if (info.chip_type == FLASH_TYPE_NOR) {
		ret = lseek(fd, start_addr, SEEK_SET);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__);
			goto exit;
		}

		ret = write(fd, buffer, length);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__);
			goto exit;
		}
	} else if (info.chip_type == FLASH_TYPE_SPINAND) {
		unsigned int spinand_erasesize = info.block_size;
		unsigned int spinand_writesize = info.page_size;
		unsigned int page_offset = start_addr & (spinand_writesize - 1);

		if (page_offset) {
			unsigned int tmp_len;
			unsigned char *buf = malloc(spinand_writesize);
			cyg_io_flash_nand_block_is_bad_t block_is_bad = {0};

			while (1) {
				block_is_bad.ofs = start_addr;
				ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_NAND_BLOCK_IS_BAD, &block_is_bad);
				if (ret < 0) {
					gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
					goto exit;
				}

				if (block_is_bad.is_bad) {
					gxlogi("warning: skip bad block at 0x%x\n", start_addr & ~(spinand_erasesize -1));
					start_addr += spinand_erasesize;
				} else
					break;
			}
			tmp_len = MIN(spinand_writesize - page_offset, length);
			memset(buf, 0xff, spinand_writesize);
			memcpy(buf + page_offset, buffer, tmp_len);
			ret = spinand_write_skip_bad(start_addr - page_offset, &spinand_writesize, buf, 0);
			start_addr += tmp_len;
			buffer += tmp_len;
			length -= tmp_len;
			free(buf);
		}
		if (length)
			ret = spinand_write_skip_bad(start_addr, &length, buffer, 0);
	}

exit:
	return ret;
}


static void nand_erase_remap(uint32_t *offset, uint32_t *length)
{
	int ret;
	cyg_io_flash_getconfig_info_t info = {0};

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ;
	}

	uint32_t block_size = info.block_size;
	int addr = *offset;
	int len = *length;
	int end_addr = addr + len;

	if (addr % block_size)
		addr = addr / block_size * block_size;
	if (end_addr % block_size)
		end_addr = (end_addr + block_size - 1) / block_size * block_size;

	*offset = addr;
	*length = end_addr - addr;
}

static int spinand_erase(loff_t offset, size_t length)
{
	unsigned long erase_length, erased_length; /* in blocks */
	loff_t erase_addr;      /* the address in NAND to erase */
	erase_addr = offset;
	int ret;
	int percent_complete = -1;
	cyg_io_flash_getconfig_info_t info = {0};
	cyg_io_flash_nand_block_is_bad_t block_is_bad = {0};
	cyg_io_flash_getconfig_erase_t flash_erase;

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}

	if ((erase_addr & (info.block_size - 1)) != 0) {
		gxlogi("error: Attempt to erase non block aligned data!\n");
		return -1;
	}

	erase_length = (length + info.block_size- 1) / info.block_size;
	for (erased_length = 0; erased_length < erase_length; erase_addr += info.block_size) {

		if (erase_addr >= info.chip_size) {
			gxlogi("error: skip erase outside flash area!\n");
			break;
		}

		block_is_bad.ofs = erase_addr;
		ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_NAND_BLOCK_IS_BAD, &block_is_bad);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
			return ret;
		}

		if (block_is_bad.is_bad) {
			gxlogi("warning: skipping bad block at 0x%08x\n", erase_addr);
//			offset += spinand_erasesize - block_offset;
			continue;
		}


//		ret = spinand_block_isbad(erase_addr);
//		if (!opts->scrub) {
//			if (ret > 0) {
//				if (!opts->quiet)
//					gxlog_w("warning: skipping bad block at 0x%08x\n", erase_addr);
//				if (!opts->spread)
//					erased_length++;
//				continue;
//			} else if (ret < 0) {
//				gxlog_e("get bad block failed: %d\n",ret);
//				return -1;
//			}
//		} else {
//			if (ret > 0) {
//				if (!opts->quiet)
//					gxlog_w("warning: erase bad block at 0x%08x\n", erase_addr);
//			}
//		}

		flash_erase.offset = erase_addr;
		flash_erase.len = info.block_size;
		ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_ERASE, &flash_erase,
				sizeof(cyg_io_flash_getconfig_erase_t));
		if (ret != 0) {
			cyg_io_flash_nand_block_markbad_t markbad = {0};

			markbad.ofs = erase_addr;

			ret = ioctl(fd, CYG_IO_SET_CONFIG_FLASH_NAND_BLOCK_MARKBAD, &markbad,
					sizeof(cyg_io_flash_nand_block_markbad_t));
			if (ret == 0) {
				gxlogi("\nerase failed,mark bad block at 0x%08x\n", erase_addr);
				continue;
			} else {
				gxlogi("\nmark bad block failed at 0x%08x: %d\n",erase_addr, ret);
				return -1;
			}
		}

		erased_length++;
//		result = chip->erase_block(chip, erase_addr/chip->info->block_main_size);
//		if (result != 0) {
//			gxlog_e("erase block failure at %08x\n",erase_addr);
//
//			if ((!opts->scrub) && (result == -EIO)) {
//				gxlog_i("try to mark bad block at %08x\n",erase_addr);
//				spinand_markbad_bbt(chip, erase_addr);
//				spinand_block_markbad(erase_addr);
//			}
//			continue;
//		}

//		if (!opts->quiet) {
		if (0) {
			int percent;
			percent =  (int)(((float)erased_length / erase_length) * 100);
			/* output progress message only at whole percent
			 * steps to reduce the number of messages printed
			 * on (slow) serial consoles
			 */
			if (percent != percent_complete) {
				percent_complete = percent;
				gxlogi("\rErasing at 0x%08x -- %3d%% complete.", erase_addr, percent);
			}
		}
	}
#if 0
	if (1)
		gxlogi("\n");
#endif

//	if (opts->scrub) {
//		if (chip->bbt) {
//			gxm_free(chip->bbt);
//			chip->bbt = NULL;
//		}
//
//		spinand_default_bbt(chip);
//	}

	return 0;
}

//static int spinand_erase(loff_t from, size_t len, int scrub, int spread)
//{
//	struct spinand_erase_ops opts;
//
//	memset(&opts, 0, sizeof(opts));
//	opts.offset = from;
//	opts.length = len;
//	opts.scrub = scrub;
//	opts.spread = spread;
//
//	return spinand_erase_opts(&g_spinand_chip, &opts);
//}

int GxFlash_Erase(unsigned int start_addr, unsigned int length)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	cyg_io_flash_getconfig_erase_t flash_erase;

	ret = gxflash_range_check(fd, start_addr, length);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__);
		goto exit;
	}

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}

	if ((info.chip_type == FLASH_TYPE_NOR) || (info.chip_type == FLASH_TYPE_SPINAND)) {
		flash_erase.offset = start_addr;
		flash_erase.len = length;
		ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_ERASE, &flash_erase,
				sizeof(cyg_io_flash_getconfig_erase_t));
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
			goto exit;
		}
	} else if (info.chip_type == FLASH_TYPE_NAND) {
		nand_erase_remap((uint32_t *)&start_addr,(uint32_t *) &length);
		ret = spinand_erase(start_addr, length);
		if (ret < 0) {
			gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
			goto exit;
		}
	}

exit:
	return ret;
}

int GxFlash_WriteProtect(unsigned int length)
{
	int ret = 0;
	cyg_io_flash_getconfig_lock_t flash_lock;

	flash_lock.offset = 0;
	flash_lock.len = length;
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_LOCK, &flash_lock);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}

exit:
	return ret;
}

int GxFlash_SetLockSR(void)
{
	int ret = 0;

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_SET_LOCKSR, NULL);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}

exit:
	return ret;
}

int GxFlash_GetLockSR(int *lock)
{
	int ret = 0;

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_GET_LOCKSR, lock);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}

exit:
	return ret;
}

int GxFlash_WriteUnprotect(void)
{
	int ret = 0;
	cyg_io_flash_getconfig_lock_t flash_lock;

	flash_lock.offset = 0;
	flash_lock.len = 0;
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_UNLOCK, &flash_lock);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		goto exit;
	}

exit:
	return ret;
}

int GxFlash_BlockIsBad(unsigned int addr)
{
	int ret = 0;
	cyg_io_flash_getconfig_info_t info = {0};
	cyg_io_flash_nand_block_is_bad_t block_is_bad = {0};

	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}
	if (info.chip_type != FLASH_TYPE_SPINAND) {
		gxlogi("error: [%s : %d] Only support SPI Nand.\n", __func__, __LINE__);
		return -1;
	}

	block_is_bad.ofs = addr;
	ret = ioctl(fd, CYG_IO_GET_CONFIG_FLASH_NAND_BLOCK_IS_BAD, &block_is_bad);
	if (ret < 0) {
		gxlogi("error: [%s : %d]\n", __func__, __LINE__ );
		return ret;
	}

	return block_is_bad.is_bad;
}

int GxFlash_Get_Ppb(unsigned char *data)
{
	return 0;
}

int GxFlash_Set_Ppb(void)
{
	return 0;
}
