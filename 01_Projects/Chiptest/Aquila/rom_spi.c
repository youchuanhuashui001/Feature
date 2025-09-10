/*****************************************
  Copyright (c) 2003-2019
  Nationalchip Science & Technology Co., Ltd. All Rights Reserved
  Proprietary and Confidential
 *****************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <util.h>
#include <stdarg.h>
#include <driver/gx_flash.h>
#include <misc.h>
#include <soc.h>
#include <spl/spl.h>
#include <driver/gx_clock.h>
#include "spl_spinor.h"
#include "autoconf.h"

#define SPI_CS_HIGH()  do {writel(0x03, FLASH_SPI_CS_REG);} while(0);
#define SPI_CS_LOW()   do {writel(0x02, FLASH_SPI_CS_REG);} while(0);
#define SPI_CS_HW()    do {writel(0x01, FLASH_SPI_CS_REG);} while(0);

static inline int wait_bus_ready(void)
{
	uint32_t val;

	do{
		val = readl(SPIM_SR);
	} while (val & SR_BUSY);

	return 0;
}

static inline int wait_rx_done(void)
{
	uint32_t val;

	do{
		val = readl(SPIM_RXFLR);
	} while (val != 0);

	wait_bus_ready();

	return 0;
}

static int sflash_read_reg(uint8_t cmd, void* reg_val, uint32_t len)
{
	uint32_t i, val;
	uint8_t *p;

	p = (uint8_t*)reg_val;
	wait_bus_ready();
	writel(0x00, SPIM_ENR);
	writel(0x00, SPIM_DMACR);
	writel(EEPROM_8BITS_MODE, SPIM_CTRLR0);
	writel(len - 1, SPIM_CTRLR1);
	writel(0x01, SPIM_SER);
	writel(0 << 16, SPIM_TXFTLR);
	writel(0x00, SPIM_SPI_CTRL0);
	writel(0x01, SPIM_ENR);

	writel(cmd, SPIM_TXDR_LE);

	for (i = 0; i < len; i++) {
		do{
			val = readl(SPIM_SR);
		} while (!(val & SR_RF_NOT_EMPT));
		p[i] = readl(SPIM_RXDR_LE);
	}

	wait_rx_done();

	return 0;
}

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static uint8_t read_sr1(void)
{
	uint8_t status;

	sflash_read_reg(GX_CMD_RDSR1, &status, sizeof(status));

	return status;
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns **** if error.
 */
static int wait_till_ready(void)
{
	uint8_t sr;

	do{
		sr = read_sr1();
	}while (sr & GX_STAT_WIP);

	return 1;
}

static int sflash_init(void)
{
	writel(0x0, SPIM_ENR);
	SPI_CS_HW();
	writel(0, SPIM_IMR);
	writel(((CONFIG_SF_DEFAULT_SAMPLE_DELAY%2)<<16)|(CONFIG_SF_DEFAULT_SAMPLE_DELAY/2), SPIM_SAMPLE_DLY);
	writel(CONFIG_SF_DEFAULT_CLKDIV, SPIM_BAUDR);
	writel(0x7, SPIM_RXFTLR);
	writel(0x1, SPIM_ENR);

	return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int sflash_fread(unsigned int from, void *buf,unsigned int len)
{
	/* sanity checks */
	if (!len)
		return 0;

	wait_bus_ready();
	writel(0x00, SPIM_ENR);
	writel(0x00, SPIM_DMACR);
	writel(0x00, SPIM_SER);

	writel(QUAD_RO_8BITS_MODE, SPIM_CTRLR0);
	writel(len-1, SPIM_CTRLR1);
	writel(0 << 16, SPIM_TXFTLR);
	writel(7, SPIM_DMARDLR);
	writel(0x01, SPIM_DMACR);
	writel(STRETCH_WAIT8_INST8_ADDR24, SPIM_SPI_CTRL0);
	writel(0x01, SPIM_ENR);
	writel(GX_CMD_QREAD, SPIM_TXDR_BE);
	writel(from, SPIM_TXDR_BE);

	writel(0x01, SPIM_SER);
	uint32_t i;
	for (i = 0; i < len; i++) {
		while(!(readl(SPIM_SR) & SR_RF_NOT_EMPT));
		((uint8_t*)buf)[i] = 0xff & readl(SPIM_RXDR_LE);
	}
	wait_rx_done();

	return 0;
}

int sflash_readdata(unsigned int offset, void *to, unsigned int len)
{
	unsigned int size;
	unsigned char* p = to;

	sflash_init();
	wait_till_ready();
	while(len != 0) {
		size = min(0x10000, len);
		sflash_fread(offset, p, size);
		len -= size;
		offset += size;
		p += size;
	}

	return 0;
}
