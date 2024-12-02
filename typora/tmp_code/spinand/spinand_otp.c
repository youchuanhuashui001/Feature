#include "autoconf.h"
#include "kernelcalls.h"
#ifdef CONFIG_GX_NOS
#include "driver/spi.h"
#include <errno.h>
#elif defined(CONFIG_GX_LINUX)
#include <linux/version.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#endif
#include <gx_flash_common.h>
#include "spinand_ids.h"
#include "spinand_base.h"

int spinand_wait_ready(struct spinand_chip *chip, uint8_t *status);

static int spinand_otp_enable(struct spinand_chip *chip, int enable)
{
	uint8_t otp_status;

	spinand_get_status(chip, REG_OTP, &otp_status);

	if (enable)
		otp_status |= OTP_ENABLE;
	else {
		otp_status &= ~OTP_ENABLE;
		otp_status &= ~OTP_PRT;
	}

	spinand_set_status(chip, REG_OTP, otp_status);

	return 0;
}

int spinand_otp_lock(struct spinand_chip *chip)
{
	uint8_t otp_status, status;

	spinand_get_status(chip, REG_OTP, &otp_status);

	if (otp_status & OTP_PRT)
		return 0;

	status = otp_status | OTP_ENABLE | OTP_PRT;
	spinand_set_status(chip, REG_OTP, status);

	spinand_write_enable(chip);

	spinand_program_execute(chip, 0);

	spinand_wait_ready(chip, &status);

	spinand_set_status(chip, REG_OTP, otp_status);

	return 0;
}

static int spinand_otp_lock_status_0(struct spinand_chip *chip, unsigned char *status)
{
	uint8_t otp_status;

	spinand_get_status(chip, REG_OTP, &otp_status);

	if (otp_status & OTP_PRT)
		*status = 1;
	else
		*status = 0;

	return 0;
}

static int spinand_otp_lock_status_1(struct spinand_chip *chip, unsigned char *status)
{
	uint8_t otp_status;
	uint8_t *buf;
	uint32_t corrected, i, len;

	len = chip->info->page_main_size;

	buf = gxm_malloc(len);
	if (!buf)
		return -ENOMEM;

	spinand_get_status(chip, REG_OTP, &otp_status);

	spinand_set_status(chip, REG_OTP, 0xC0);

	chip->read_page(chip, 0, 0, len, buf, &corrected);

	*status = 1;
	for (i = 0; i < len; i++) {
		if (buf[i] == 0xff) {
			*status = 0;
			break;
		}
	}

	spinand_set_status(chip, REG_OTP, otp_status);

	gxm_free(buf);

	return 0;
}

int spinand_otp_lock_status(struct spinand_chip *chip, unsigned char *status)
{
	uint32_t id;
	int ret;

	id = chip->info->id;

	switch(id & 0xff) {
		case 0x2C: /* Micron */
			ret = spinand_otp_lock_status_1(chip, status);
			break;
		default:
			ret = spinand_otp_lock_status_0(chip, status);
			break;
		}

	return ret;
}

static int generic_otp_read(struct spinand_chip *chip, int page_id, int offset, unsigned char *buf, int buf_len, int *ret_len)
{
	unsigned int corrected;
	int ret;
	int size;

	size = min(buf_len, chip->info->page_main_size - offset);

	spinand_otp_enable(chip, 1);

	ret = chip->read_page(chip, page_id, offset, size, buf, &corrected);

	spinand_otp_enable(chip, 0);

	*ret_len = size;

	if (ret == ECC_NOT_CORRECT) {
		return -EBADMSG;
	}

	return 0;
}

static int generic_otp_program(struct spinand_chip *chip, int page_id, int offset, unsigned char *buf, int buf_len, int *ret_len)
{
	int ret;
	int size;

	if (buf_len > chip->info->page_main_size - offset)
		return -EINVAL;

	spinand_otp_enable(chip, 1);

	size = buf_len;
	ret = chip->program_page(chip, page_id, offset, size, buf);

	spinand_otp_enable(chip, 0);

	*ret_len = size;

	return ret;
}

int spinand_otp_read(struct spinand_chip *chip, int page_id, int offset, unsigned char *buf, int buf_len, int *ret_len)
{
	uint32_t id;
	int ret;

	id = chip->info->id;

	switch(id & 0xff) {
	case 0xC8: /* GigaDevice */
	case 0xEF: /* Winbond */
	case 0xCD: /* FORESEE */
	case 0xE5: /* Dosilicon */
	case 0x0b: /* XTX */
	case 0x2C: /* Micron*/
	case 0xd5: /* EtronTech */
	case 0x01: /* EtronTech(EM73C044VCS-H) */
	case 0xa1: /* Fudan & UNiM */
	case 0xC9: /* HeYangTek */
	case 0x52: /* GSTO */
		ret = generic_otp_read(chip, page_id, offset, buf, buf_len, ret_len);
		break;
	default:
		return -1;
	}

	return ret;
}

int spinand_otp_program(struct spinand_chip *chip, int page_id, int offset, unsigned char *buf, int buf_len, int *ret_len)
{
	uint32_t id;
	int ret;

	id = chip->info->id;

	switch(id & 0xff) {
	case 0xC8: /* GigaDevice */
	case 0xEF: /* Winbond */
	case 0xCD: /* FORESEE */
	case 0xE5: /* Dosilicon */
	case 0x0b: /* XTX */
	case 0x2C: /* Micron*/
	case 0xd5: /* EtronTech */
	case 0x01: /* EtronTech(EM73C044VCS-H) */
	case 0xa1: /* Fudan & UNiM */
	case 0xC9: /* HeYangTek */
	case 0x52: /* GSTO */
		ret = generic_otp_program(chip, page_id, offset, buf, buf_len, ret_len);
		break;
	default:
		return -1;
	}

	return ret;
}

int spinand_otp_set_page(struct spinand_chip *chip, int page_id)
{
	uint32_t id;
	id = chip->info->id;

	/* 先按照型号id区分 */
	switch(id) {
	case 0xEACD: /* FS35ND01G-S1Y2 */
	case 0x91C8: /* GD5F1GM7UEYIG */
	case 0x92C8: /* GD5F2GM7UEYIG */
	case 0x95C8: /* GD5F4GM7UEYIG */
		if (page_id >= 10)
			return -1;
		chip->otp_page = page_id + 2;
		break;
	case 0x71CD: /* FS35SQA001G */
	case 0x2501: /* EM73C044VCS-H */
		if (page_id >= 62)
			return -1;
		chip->otp_page = page_id + 2;
		break;
	case 0xf1a1: /* TX25G01 */
		chip->otp_page = page_id;
		break;
	case 0x25d5: /* EM73C044VCF-H */
		if (page_id >= 63)
			return -1;
		chip->otp_page = page_id + 1;
		break;
	default:
		/* 按照厂家id区分 */
		switch(id & 0xff) {
		case 0xC8: /* GigaDevice */
		case 0x0b: /* XTX */
		case 0xd5: /* EtronTech */
		case 0xC9: /* HeYangTek */
			if (page_id >= 4)
				return -1;
			chip->otp_page = page_id;
			break;
		case 0xEF: /* Winbond */
		case 0x2C: /* Micron*/
		case 0x52: /* GSTO */
			if (page_id >= 10)
				return -1;

			/* otp从page 2开始 eg: W25N01GV */
			chip->otp_page = page_id + 2;
			break;
		case 0xCD: /* FORESEE */
			if (page_id >= 8)
				return -1;

			chip->otp_page = page_id;
			break;
		case 0xE5: /* Dosilicon */
			if (page_id >= 30)
				return -1;

			/* otp从page 2开始 */
			chip->otp_page = page_id + 2;
			break;
		default:
			return -1;
		}
	}

	return 0;
}


int spinand_otp_get_current_page(struct spinand_chip *chip, int *page_id)
{
	uint32_t id;
	id = chip->info->id;

	/* 先按照型号id区分 */
	switch(id) {
	case 0xEACD: /* FS35ND01G-S1Y2 */
	case 0x71CD: /* FS35SQA001G */
	case 0x91C8: /* GD5F1GM7UEYIG */
	case 0x92C8: /* GD5F2GM7UEYIG */
	case 0x95C8: /* GD5F4GM7UEYIG */
	case 0x2501: /* EM73C044VCS-H */
		if (chip->otp_page < 2)
			return -1;
		*page_id = chip->otp_page - 2;
		break;
	case 0xf1a1: /* TX25G01 */
		*page_id = chip->otp_page;
		break;
	case 0x25d5: /* EM73C044VCF-H */
		if (chip->otp_page < 1)
			return -1;
		*page_id = chip->otp_page - 1;
		break;
	default:
		/* 按照厂家id区分 */
		switch(id & 0xff) {
		case 0xC8: /* GigaDevice */
		case 0xCD: /* FORESEE */
		case 0x0b: /* XTX */
		case 0xd5: /* EtronTech */
		case 0xC9: /* HeYangTek */
			if (chip->otp_page < 0)
				return -1;
			*page_id = chip->otp_page;
			break;
		case 0xEF: /* Winbond */
		case 0xE5: /* Dosilicon */
		case 0x2C: /* Micron*/
		case 0x52: /* GSTO */
			if (chip->otp_page < 2)
				return -1;

			/* otp从page 2开始 eg: W25N01GV */
			*page_id = chip->otp_page - 2;
			break;
		default:
			return -1;
		}
	}

	return 0;
}

int spinand_otp_get_pages(struct spinand_chip *chip, int *nr)
{
	uint32_t id;
	id = chip->info->id;

	/* 先按照型号id区分 */
	switch(id) {
	case 0xEACD: /* FS35ND01G-S1Y2 */
	case 0x91C8: /* GD5F1GM7UEYIG */
	case 0x92C8: /* GD5F2GM7UEYIG */
	case 0x95C8: /* GD5F4GM7UEYIG */
		*nr= 10;
		break;
	case 0x71CD: /* FS35SQA001G */
	case 0x2501: /* EM73C044VCS-H */
		*nr= 62;
		break;
	case 0xf1a1: /* TX25G01 */
		*nr= 8;
		break;
	case 0x25d5: /* EM73C044VCF-H */
		*nr= 63;
		break;
	default:
		/* 按照厂家id区分 */

		switch(id & 0xff) {
		case 0xC8: /* GigaDevice */
			*nr= 4;
			break;
		case 0xEF: /* Winbond */
		case 0x2C: /* Micron*/
		case 0x52: /* GSTO */
			*nr= 10;
			break;
		case 0xCD: /* FORESEE */
			*nr= 8;
			break;
		case 0xE5: /* Dosilicon */
			*nr= 30;
			break;
		case 0x0b: /* XTX */
		case 0xd5: /* EtronTech */
		case 0xC9: /* HeYangTek */
			*nr= 4;
			break;
		default:
			return -1;
		}
	}

	return 0;
}

int spinand_otp_get_page_size(struct spinand_chip *chip, int *size)
{
	uint32_t id = chip->info->id;

	switch(id & 0xff) {
	case 0xC8: /* GigaDevice */
	case 0xEF: /* Winbond */
	case 0xCD: /* FORESEE */
	case 0xE5: /* Dosilicon */
	case 0x0B: /* XTX */
	case 0x2C: /* Micron*/
	case 0xD5: /* EtronTech */
	case 0x01: /* EtronTech(EM73C044VCS-H) */
	case 0xA1: /* Fudan & UNiM */
	case 0x52: /* GSTO */
		if (id == 0x130B)
			*size = 4096;
		else
			*size = 2048;
		return 0;
	case 0xC9: /* HeYangTek */
		if(id == 0xD4C9)
			*size = 4096;
		else
			*size = 2048;
		return 0;
	default:
		*size = 0;
		return -1;
	}
}
