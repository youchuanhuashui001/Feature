/* 对接 flash gen_spi、flash_spi 都是通的。gen_spi 要分频到 16，flash_spi 要分频到 64 */
#include <stdio.h>
#include <spi.h>
#include <device.h>
#include <io.h>
#include <linux/errno.h>
#include <gxhwlib_registers.h>
#include "board-common.h"
#include "gpio.h"
#include "dw_spi_v2.h"
#include "interrupt.h"
#include "board_config.h"
#include <driver/gx_dma_axi.h>

#undef CONFIG_ENABLE_IRQ
#undef CONFIG_ENABLE_GEN_SPI

/*#define MODULE_DEBUG 0*/
#define MODULE_DEBUG
//#undef MODULE_DEBUG
#ifdef MODULE_DEBUG
#define md_debug(fmt, arg...) \
	printf(fmt, ##arg)
#else
#define md_debug(fmt, arg...) \
	do {}while(0)
#endif

#define START_STATE     ((void *)0)
#define RUNNING_STATE   ((void *)1)
#define DONE_STATE      ((void *)2)
#define ERROR_STATE     ((void *)-1)

#define QUEUE_RUNNING   0
#define QUEUE_STOPPED   1

#define MRST_SPI_DISABLE   0
#define MRST_SPI_ENABLE    1

#define GENERAL_SPI_VERSION       0x3130322a
#define DW_SPI_VERSION            0x3130322a

#define CONFIG_SPI_SLAVE_NUM_MAX  3
#define CONFIG_GENERAL_SPI_BUS_SN 0

#if defined(CONFIG_ARCH_CKMMU_CYGNUS)
#define DW_SPI_CS_BASE_REG                0xa030a904
#define DW_SPI_SPEED_MODE_REG             0xa030a794      // used for storing dw spi speed mode
#define DW_SPI_IRQ_NUM                    17
#elif defined(CONFIG_ARCH_ARM_CANOPUS) || defined(CONFIG_ARCH_ARM_VEGA)
#define DW_SPI_CS_BASE_REG                (REG_BASE_SPI + 0x404)
#define DW_SPI_SPEED_MODE_REG             0x89400114      // used for storing dw spi speed mode
#define DW_SPI_IRQ_NUM                    45
#elif defined(CONFIG_ARCH_CKMMU_SCORPIO)
#define DW_SPI_CS_BASE_REG                0xa030a708
#define DW_SPI_SPEED_MODE_REG             0xa030a114      // used for storing dw spi speed mode
#define DW_SPI_IRQ_NUM                    17
#elif defined(CONFIG_ARCH_ARM_VIRGO)
#if defined(CONFIG_ENABLE_GEN_SPI)
#define DW_SPI_CS_BASE_REG                0xfc401080
#else
#define DW_SPI_CS_BASE_REG                0xfa490080
#endif
#define DW_SPI_SPEED_MODE_REG             0xfa490084      // used for storing dw spi speed mode
#define DW_SPI_IRQ_NUM                    61
#endif

#define DW_SPI_ONECE_READ_MAX_DATA           0x10000

#define CACHE_LINE_SIZE                      64

#ifndef roundup
#define roundup(x, y)     ((((x) + ((y) - 1)) / (y)) * (y))
#endif


/* SPI device running context */
struct device_context {
	struct spi_device *spi;
	uint32_t cr0_solid;
	uint32_t clk_div;        /* baud rate divider */
	uint32_t  sample_delay;
};

static struct device_context spi_device_context_tab[CONFIG_SPI_SLAVE_NUM_MAX];

static void spi_master_set_dws(struct spi_master *master, void *data)
{
	master->driver_data = data;
}
static struct dw_spi* spi_master_get_dws(struct spi_master *master)
{
	return master->driver_data;
}

/* ctldata is for the bus_master driver's runtime state */
static inline void *spi_get_ctldata(struct spi_device *spi)
{
	return spi->controller_state;
}

static inline void spi_set_ctldata(struct spi_device *spi, void *state)
{
	spi->controller_state = state;
}

static void wait_till_tf_empty(struct dw_spi *dws)
{
	while ((dw_readl(dws, SPIM_SR) & SR_TF_EMPT) != SR_TF_EMPT);
}

static void wait_till_not_busy(struct dw_spi *dws)
{
	while ((dw_readl(dws, SPIM_SR) & SR_BUSY) == SR_BUSY);
}

static void flush(struct dw_spi *dws)
{
	while ((dw_readl(dws, SPIM_SR) & SR_RF_NOT_EMPT))
		dw_readl(dws, SPIM_RXDR);

	wait_till_not_busy(dws);
}

static void spi_cs_init(struct dw_spi *dws)
{
	/* CSn由CPU控制 */
	dws->cs_regs = DW_SPI_CS_BASE_REG;
#if defined(CONFIG_ARCH_ARM_VIRGO)
	__raw_writel(0x0f,   dws->cs_regs);
#else
	__raw_writel(0x03,   dws->cs_regs);
#endif
}

static void spi_cs_control(struct dw_spi *dws, uint32_t cs, uint8_t flag)
{
	if (cs >= CONFIG_SPI_SLAVE_NUM_MAX)
		return;

	switch(flag) {

	case MRST_SPI_DISABLE:
#if defined(CONFIG_ARCH_ARM_VIRGO)
		__raw_writel(0xf, dws->cs_regs);
#else
		__raw_writel(3, dws->cs_regs);
#endif
		break;
	case MRST_SPI_ENABLE:
#if defined(CONFIG_ARCH_ARM_VIRGO)
		__raw_writel(0xf & ~(1 << (2 * cs)), dws->cs_regs);
#else
		__raw_writel(2, dws->cs_regs);
#endif
		break;
	default:
		return;
	}
}

/* Caller already set message->status; dma and pio irqs are blocked */
static void msg_giveback(struct dw_spi *dws)
{
	dws->cur_msg      = NULL;
	dws->cur_transfer = NULL;
}

#if defined(CONFIG_ENABLE_GEN_SPI) && defined(CONFIG_ENABLE_GEN_SPI_TEST)
static int dw_half_duplex_poll_transfer(struct dw_spi *dws)
{
	int inc         = dws->n_bytes;
	int len         = dws->len;
	int steps       = len / inc;
	uint8_t *buffer = (uint8_t*)dws->buffer;
	int size;
#ifndef CONFIG_ENABLE_IRQ
	int i, left, items;
	uint8_t  *data8 = (uint8_t*)buffer;
	uint16_t  *data16 = (uint16_t*)buffer;
	uint32_t  *data32 = (uint32_t*)buffer;
	uint16_t data_tmp_16;
	uint32_t data_tmp_32;
	uint32_t rx_len_mask = dws->rx_fifo_len * 2 -1;
#endif
	struct spi_transfer *transfer = dws->cur_transfer;
	struct device_context *context = spi_get_ctldata(dws->cur_msg->spi);
	uint32_t cr0;
	uint32_t align_steps;
	uint32_t data;

	/* 长度对齐 */
	if (len % inc)
		return -EINVAL;

	/* 地址对齐 */
	if ((uint32_t)buffer % inc)
		return -EINVAL;

	dws->complete = 0;

	spi_enable_chip(dws, 0);
	dw_writel(dws, SPIM_SER, 1);
	dw_writel(dws, SPIM_DMACR, 0);

	/*
	* Adjust transfer mode if necessary. Requires platform dependent
	* chipselect mechanism.
	*/
	cr0 = context->cr0_solid;

	if (transfer->tx_buf) {
		cr0 |= transfer->bits_per_word - 1;
		cr0 |= SPI_TMOD_TO << SPI_TMOD_OFFSET;

		if ((dws->cur_msg->spi->mode & SPI_TX_QUAD) && (dws->cur_msg->spi->mode & SPI_RX_QUAD)) {
			cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;

			if (transfer->bits_per_word == 0x8)
				dw_writel(dws, SPIM_SPI_CTRLR0, (0 << 30) | (0x2 << 8) | 0x2);
			if (transfer->bits_per_word == 0x10)
				dw_writel(dws, SPIM_SPI_CTRLR0, (0 << 30) | (0x2 << 8) | (0x2 << 2) | 0x2);
			if (transfer->bits_per_word == 0x20) {
				dw_writel(dws, SPIM_SPI_CTRLR0, (0 << 30) | (0x2 << 8) | (0x6 << 2) | 0x2);
			}
		} else if ((dws->cur_msg->spi->mode & SPI_TX_DUAL) && (dws->cur_msg->spi->mode & SPI_RX_DUAL)) {
			cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;

			if (transfer->bits_per_word == 0x8)
				dw_writel(dws, SPIM_SPI_CTRLR0, (0 << 30) | (0x2 << 8) | 0x2);
			if (transfer->bits_per_word == 0x10)
				dw_writel(dws, SPIM_SPI_CTRLR0, (0 << 30) | (0x2 << 8) | (0x2 << 2) | 0x2);
			if (transfer->bits_per_word == 0x20) {
				dw_writel(dws, SPIM_SPI_CTRLR0, (0 << 30) | (0x2 << 8) | (0x6 << 2) | 0x2);
			}
		}

		dw_writel(dws, SPIM_CTRLR0, cr0);
		dw_writel(dws, SPIM_CTRLR1, steps-1);
		spi_enable_chip(dws, 1);

		if ((transfer->bits_per_word == 0x10) && ((dws->cur_msg->spi->mode & SPI_TX_DUAL) | (dws->cur_msg->spi->mode & SPI_TX_QUAD))) {

			dw_writel(dws, SPIM_TXDR, data8[1]);
			dw_writel(dws, SPIM_TXDR, data8[0]);
			data16++;
			data8 = (uint8_t *)data16;
			steps -= 1;
		}
		if ((transfer->bits_per_word == 0x20) && ((dws->cur_msg->spi->mode & SPI_TX_DUAL) | (dws->cur_msg->spi->mode & SPI_TX_QUAD))) {

			dw_writel(dws, SPIM_TXDR, data8[0]<<24);
			dw_writel(dws, SPIM_TXDR, ((data8[3] << 24) | (data8[2] << 16) | (data8[1] << 8)));
			data32++;
			data8 = (uint8_t *)data32;
			steps -= 1;
		}

#ifdef CONFIG_ENABLE_IRQ
		dw_writel(dws, SPIM_TXFTLR, dws->tx_fifo_len / 2);
		dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW | SPI_INT_TXEI);

		while (steps != (*(volatile unsigned int *)(&dws->complete)));
#else /* CONFIG_ENABLE_IRQ */
		while (steps) {
			items = dw_readl(dws, SPIM_TXFLR);
			items = dws->tx_fifo_len - items;
			size = min(items, steps);

			for (i = 0; i < size; i++) {
				if (inc == 1)
					dw_writel(dws, SPIM_TXDR, data8[i]);
				else if (inc == 2)
					dw_writel(dws, SPIM_TXDR, data16[i]);
				else if (inc == 4)
					dw_writel(dws, SPIM_TXDR, data32[i]);
			}

			if (inc == 1)
				data8 += size;
			else if (inc == 2)
				data16 += size;
			else if (inc == 4)
				data32 += size;

			steps -= size;
		}
#endif /* CONFIG_ENABLE_IRQ */
		/* 等待fifo空 */
		wait_till_tf_empty(dws);
		/* 等待发送完成 */
		wait_till_not_busy(dws);
	} else { /* receive */
		cr0 |= SPI_TMOD_RO << SPI_TMOD_OFFSET;

		if ((dws->cur_msg->spi->mode & SPI_TX_OCTAL) && (dws->cur_msg->spi->mode & SPI_RX_OCTAL)) {
			cr0 |= SPI_FRM_OCTAL << SPI_FRM_OFFSET;
		} else if ((dws->cur_msg->spi->mode & SPI_TX_QUAD) && (dws->cur_msg->spi->mode & SPI_RX_QUAD)) {
			cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
		} else if ((dws->cur_msg->spi->mode & SPI_TX_DUAL) && (dws->cur_msg->spi->mode & SPI_RX_DUAL)) {
			cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
		}

		dw_writel(dws, SPIM_SPI_CTRLR0, (1<<30)|0);

#ifdef CONFIG_ENABLE_IRQ
		cr0 |= transfer->bits_per_word - 1;
		dw_writel(dws, SPIM_CTRLR0, cr0);
		dw_writel(dws, SPIM_TXFTLR, 0);
		dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW | SPI_INT_RXFI);
#endif

		spi_enable_chip(dws, 0);
		cr0 &= ~0x1f;
		cr0 |= transfer->bits_per_word - 1;
		dw_writel(dws, SPIM_CTRLR0, cr0);

		while (steps != 0) {
#ifdef CONFIG_ENABLE_IRQ
			dws->complete_minor = 0;
#endif
			spi_enable_chip(dws, 0);
			size = min(dws->rx_fifo_len, steps);
			dw_writel(dws, SPIM_CTRLR1, size-1);
			dw_writel(dws, SPIM_RXFTLR, size-1);
			spi_enable_chip(dws, 1);
			dw_writel(dws, SPIM_TXDR, 0x00);
#ifdef CONFIG_ENABLE_IRQ
			while (size != (*(volatile unsigned int *)(&dws->complete_minor)));
			dws->complete += size;
#else
			left = size;

			while (left) {
				items = dw_readl(dws, SPIM_RXFLR) & rx_len_mask;

				for (i = 0; i < items; i++) {
					if (inc == 1) {
						*data8 = dw_readl(dws, SPIM_TXDR);
						data8++;
					} else if (inc == 2) {
						*data16 = dw_readl(dws, SPIM_TXDR);
						data16++;
					} else if (inc == 4) {
						*data32 = dw_readl(dws, SPIM_TXDR);
						data32++;
					}
				}
				left -= items;
			}
#endif
			steps -= size;
		}
	}

#ifdef CONFIG_ENABLE_IRQ
	dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW);
#endif
	spi_enable_chip(dws, 0);

	dws->cur_msg->actual_length += len;

	return 0;
}

static int dw_half_duplex_dma_transfer(struct dw_spi *dws)
{
	int inc         = dws->n_bytes;
	int len         = dws->len;
	int steps       = len / inc;
	uint8_t *buffer = (uint8_t*)dws->buffer;
	struct spi_transfer *transfer = dws->cur_transfer;
	struct device_context *context = spi_get_ctldata(dws->cur_msg->spi);
	uint32_t cr0;
	uint32_t align_steps;
	uint32_t data;
	uint32_t cpu_len, cpu_transfer_len, offset, dma_len;
	uint32_t block_len, poll_len;

	DW_DMAC_CH_CONFIG dma_config;
	int dma_channel;

	/* 长度对齐 */
	if (len % inc)
		return -EINVAL;

	/* 地址对齐 */
	if ((uint32_t)buffer % inc)
		return -EINVAL;

	dws->complete = 0;

	spi_enable_chip(dws, 0);
	dw_writel(dws, SPIM_SER, 1);
	dw_writel(dws, SPIM_DMACR, 0);
	dw_writel(dws, SPIM_DMARDLR, 7);
	dw_writel(dws, SPIM_DMATDLR, 8);

	/*
	* Adjust transfer mode if necessary. Requires platform dependent
	* chipselect mechanism.
	*/
	cr0 = context->cr0_solid;

	if (transfer->tx_buf) {
		if (steps <= max(dws->tx_fifo_len, CACHE_LINE_SIZE)) {
			return dw_half_duplex_poll_transfer(dws);
		}

#if 0
		if ((transfer->tx_nbits != SPI_NBITS_DUAL) && (transfer->tx_nbits != SPI_NBITS_QUAD)) {
			return dw_half_duplex_poll_transfer(dws);
		}
#endif

		offset = (uint32_t)buffer % CACHE_LINE_SIZE;
		if (offset) {
			cpu_len = CACHE_LINE_SIZE - offset;
			cpu_transfer_len = min(cpu_len, len);
			dws->len = cpu_transfer_len;
			dw_half_duplex_poll_transfer(dws);
			dws->len = len - cpu_transfer_len;
			dws->buffer = (void*)((uint32_t)buffer + cpu_transfer_len);
		}
		dma_len = dws->len & ~(CACHE_LINE_SIZE - 1);

		cr0 |= transfer->bits_per_word - 1;
		cr0 |= SPI_TMOD_TO << SPI_TMOD_OFFSET;

		if (transfer->tx_nbits == SPI_NBITS_DUAL) {
			cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
		} else if (transfer->tx_nbits == SPI_NBITS_QUAD) {
			cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
		}

		dw_writel(dws, SPIM_CTRLR0, cr0);

		dma_channel = dw_dma_select_channel();

		/* dma source config */
		dma_config.src_trans_width = DWAXIDMAC_TRANS_WIDTH_8;
		dma_config.src_addr_update = DWAXIDMAC_CH_CTL_L_INC;
		dma_config.src_hs_select = DWAXIDMAC_HS_SEL_HW;
		dma_config.src_master_select = AXI_MASTER_1;
		dma_config.src_msize = DWAXIDMAC_BURST_TRANS_LEN_32;
		dma_config.src_per = 0;

		/* dma destination config */
		dma_config.dst_trans_width = DWAXIDMAC_TRANS_WIDTH_8;
		dma_config.dst_addr_update = DWAXIDMAC_CH_CTL_L_NOINC;
		dma_config.dst_hs_select = DWAXIDMAC_HS_SEL_HW;
		dma_config.dst_master_select = AXI_MASTER_2;
		dma_config.dst_msize = DWAXIDMAC_BURST_TRANS_LEN_8;
		dma_config.flow_ctrl = DWAXIDMAC_TT_FC_MEM_TO_PER_DMAC;
		dma_config.dst_per = DMA_HS_PER_SPI2_DMA_TX;

		dw_dma_channel_config(dma_channel, &dma_config);

		if (dma_len != 0) {
//			flush_dcache_all();
			dcache_flush();

//			dw_writel(dws, SPIM_SPI_CTRLR0,  (1<<30) | 0x01);
			dw_writel(dws, SPIM_CTRLR1, dma_len -1);
			dw_writel(dws, SPIM_DMACR, 0x02);
			spi_enable_chip(dws, 1);

			dw_dma_xfer((void *)(dws->regs + SPIM_TXDR), (void *)dws->buffer, \
					dma_len, dma_channel);
/*			dma_xfer((void *)(dws->regs + SPIM_TXDR), (void *)dws->buffer, \
				dma_len, 0, dma_channel, &dma_config);*/
			dw_dma_wait_complete(dma_channel);

//			wait_dma_complete(dma_channel);

			dws->cur_msg->actual_length += dma_len;
		}

		/* wait spi tx fifo empty */
		while (dw_readl(dws, SPIM_TXFLR) != 0);

		/* wait spi not busy */
		while (dw_readl(dws, SPIM_SR) & 0x01);

		if (dma_len != dws->len) {
			dws->len -= dma_len;
			dws->buffer = (void*)((uint32_t)dws->buffer + dma_len);
			dw_half_duplex_poll_transfer(dws);
		}
	} else {  /* receive */
		if (steps <= max(dws->rx_fifo_len, CACHE_LINE_SIZE)) {
			return dw_half_duplex_poll_transfer(dws);
		}

#if 0
		if ((transfer->rx_nbits != SPI_NBITS_DUAL) && (transfer->rx_nbits != SPI_NBITS_QUAD)) {
			return dw_half_duplex_poll_transfer(dws);
		}
#endif

		offset = (uint32_t)buffer % CACHE_LINE_SIZE;
		if (offset) {
			cpu_len = CACHE_LINE_SIZE - offset;
			cpu_transfer_len = min(cpu_len, len);
			dws->len = cpu_transfer_len;
			dw_half_duplex_poll_transfer(dws);
			dws->len = len - cpu_transfer_len;
			dws->buffer = (void*)((uint32_t)buffer + cpu_transfer_len);
		}

		dma_len = dws->len & ~(CACHE_LINE_SIZE - 1);
		poll_len = dws->len % CACHE_LINE_SIZE;

		if (dma_len != 0) {
			cr0 |= transfer->bits_per_word - 1;
			cr0 |= SPI_TMOD_RO << SPI_TMOD_OFFSET;

			if (transfer->rx_nbits == SPI_NBITS_DUAL) {
				cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
				dw_writel(dws, SPIM_SPI_CTRLR0, 0x0);
			} else if (transfer->rx_nbits == SPI_NBITS_QUAD) {
				cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
				dw_writel(dws, SPIM_SPI_CTRLR0, 0x0);
			}

			dcache_flush();
//			invalidate_dcache_range((unsigned long)dws->buffer, (unsigned long)(dws->buffer + dma_len));

			dw_writel(dws, SPIM_CTRLR0, cr0);
			dw_writel(dws, SPIM_DMACR, 0x01);
		}

		dma_channel = dw_dma_select_channel();

		/* dma source config */
		dma_config.src_trans_width = DWAXIDMAC_TRANS_WIDTH_8;
		dma_config.src_addr_update = DWAXIDMAC_CH_CTL_L_NOINC;
		dma_config.src_hs_select = DWAXIDMAC_HS_SEL_HW;
		dma_config.src_master_select = AXI_MASTER_2;
		dma_config.src_msize = DWAXIDMAC_BURST_TRANS_LEN_8;
		dma_config.src_per = DMA_HS_PER_SPI2_DMA_RX;

		/* dma destination config */
		dma_config.dst_trans_width = DWAXIDMAC_TRANS_WIDTH_8;
		dma_config.dst_addr_update = DWAXIDMAC_CH_CTL_L_INC;
		dma_config.dst_hs_select = DWAXIDMAC_HS_SEL_HW;
		dma_config.dst_master_select = AXI_MASTER_1;
		dma_config.dst_msize = DWAXIDMAC_BURST_TRANS_LEN_32;
		dma_config.dst_per = 0;
		dma_config.flow_ctrl = DWAXIDMAC_TT_FC_PER_TO_MEM_DMAC;

		dw_dma_channel_config(dma_channel, &dma_config);

		while (dma_len != 0) {
			wait_till_not_busy(dws);
			spi_enable_chip(dws, 0);
			block_len = min((int)dma_len, 0x10000);

			dw_writel(dws, SPIM_CTRLR1, block_len - 1);
			dw_dma_xfer((void *)dws->buffer, (void *)(dws->regs + SPIM_RXDR), \
					dma_len, dma_channel);
/*			dma_xfer((void *)dws->buffer, (void *)(dws->regs + SPIM_RXDR), block_len, \
				0, dma_channel, &dma_config);*/
			spi_enable_chip(dws, 1);
			dw_writel(dws, SPIM_TXDR, 0x00);

			dw_dma_wait_complete(dma_channel);
//			wait_dma_complete(dma_channel);

			/* wait spi rx fifo empty */
			while(dw_readl(dws, SPIM_RXFLR) != 0);

			/* wait spi not busy */
			while(dw_readl(dws, SPIM_SR) & 0x01);

			dws->buffer = (void*)((uint32_t)dws->buffer + block_len);
			dma_len -= block_len;
			dws->cur_msg->actual_length += block_len;
		}

		if (poll_len) {
			dws->len = poll_len;
			dws->buffer = (void*)((uint32_t)dws->buffer);
			dw_half_duplex_poll_transfer(dws);
		}
	}

	spi_enable_chip(dws, 0);

	return 0;
}
#else
static int dw_half_duplex_poll_transfer(struct dw_spi *dws)
{
	int inc         = dws->n_bytes;
	int len         = dws->len;
	int steps       = len / inc;
	uint8_t *buffer = (uint8_t*)dws->buffer;
	int size;
#ifndef CONFIG_ENABLE_IRQ
	int i, left, items;
	uint8_t  *data8 = (uint8_t*)buffer;
	uint16_t  *data16 = (uint16_t*)buffer;
	uint32_t  *data32 = (uint32_t*)buffer;
	uint16_t data_tmp_16;
	uint32_t data_tmp_32;
	uint32_t rx_len_mask = dws->rx_fifo_len * 2 -1;
#endif
	struct spi_transfer *transfer = dws->cur_transfer;
	struct device_context *context = spi_get_ctldata(dws->cur_msg->spi);
	uint32_t cr0;
	uint32_t align_steps = 0;
	uint32_t data;

	/* 长度对齐 */
	if (len % inc)
		return -EINVAL;

	/* 地址对齐 */
	if ((uint32_t)buffer % inc)
		return -EINVAL;

	dws->complete = 0;

	spi_enable_chip(dws, 0);
	dw_writel(dws, SPIM_SER, 1);
	dw_writel(dws, SPIM_DMACR, 0);

	/* Config */
//	transfer->tx_nbits = SPI_NBITS_QUAD;
//	transfer->rx_nbits = SPI_NBITS_QUAD;

	/*
	* Adjust transfer mode if necessary. Requires platform dependent
	* chipselect mechanism.
	*/
	cr0 = context->cr0_solid;
	if (transfer->tx_buf) {
		cr0 |= transfer->bits_per_word - 1;
		cr0 |= SPI_TMOD_TO << SPI_TMOD_OFFSET;

		align_steps = steps / 4;
		if (align_steps != 0) {
			cr0 |= transfer->bits_per_word * 4 - 1;
			if (transfer->tx_nbits & ((SPI_NBITS_DUAL | SPI_NBITS_QUAD | SPI_NBITS_OCTAL))) {
				dw_writel(dws, SPIM_SPI_CTRLR0, (0 << 30) | (0x2 << 8) | (0x6 << 2) | 0x2);
//				dw_writel(dws, SPIM_CTRLR1, align_steps-1);
			} else
				dw_writel(dws, SPIM_SPI_CTRLR0, 0x0);
		}

		if (transfer->tx_nbits == SPI_NBITS_DUAL) {
			cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
		} else if (transfer->tx_nbits == SPI_NBITS_QUAD) {
			cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
		} else if (transfer->tx_nbits == SPI_NBITS_OCTAL) {
			cr0 |= SPI_FRM_OCTAL << SPI_FRM_OFFSET;
		}

		dw_writel(dws, SPIM_CTRLR0, cr0);
		spi_enable_chip(dws, 1);

#ifdef CONFIG_ENABLE_IRQ
		dw_writel(dws, SPIM_TXFTLR, dws->tx_fifo_len / 2);
		dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW | SPI_INT_TXEI);

		while (steps != (*(volatile unsigned int *)(&dws->complete)));
#else /* CONFIG_ENABLE_IRQ */

		/* 多线将前四个字节数据放到指令和地址写出去 */
		if ((align_steps != 0) && (transfer->tx_nbits & (SPI_NBITS_DUAL | SPI_NBITS_QUAD | SPI_NBITS_OCTAL))) {

#if defined(CONFIG_ENABLE_GEN_SPI)
			dw_writel(dws, SPIM_TXDR, data8[0]<<24);
			dw_writel(dws, SPIM_TXDR, ((data8[3] << 24) | (data8[2] << 16) | (data8[1] << 8)));
#else
			dw_writel(dws, SPIM_TXDR, data8[0]);
			dw_writel(dws, SPIM_TXDR, ((data8[1] << 16) | (data8[2] << 8) | (data8[3] << 0)));
#endif
			data32++;
			data8 = (uint8_t *)data32;
			align_steps -= 1;
		}

		while (align_steps != 0) {

//			items = dw_readl(dws, SPIM_TXFLR) & rx_len_mask;
			items = dw_readl(dws, SPIM_TXFLR);
			items = dws->tx_fifo_len - items;
			size = min(items, align_steps);

			for (i = 0; i < size; i++) {

#if defined(CONFIG_ENABLE_GEN_SPI)
				dw_writel(dws, SPIM_TXDR, data32[i]);
#else
				data_tmp_32 = (((data32[i]) & 0x000000ff) << 24 ) | (((data32[i]) & 0x0000ff00) << 8) | \
				 (((data32[i]) & 0x00ff0000) >> 8) | (((data32[i]) & 0xff000000 ) >> 24);
				dw_writel(dws, SPIM_TXDR, data_tmp_32);
#endif
			}
			data32 += size;
			data8 = (uint8_t *)data32;
			align_steps -= size;
		}

		/* 等待fifo空 */
		wait_till_tf_empty(dws);
		/* 等待发送完成 */
		wait_till_not_busy(dws);

		spi_enable_chip(dws, 0);
		cr0 &= ~0x1f;
		cr0 |= transfer->bits_per_word - 1;
		dw_writel(dws, SPIM_CTRLR0, cr0);
		dw_writel(dws, SPIM_SPI_CTRLR0, 0x0000202);
		spi_enable_chip(dws, 1);

		steps = dws->len % 4;

		while (steps) {
//			items = dw_readl(dws, SPIM_TXFLR) & rx_len_mask;
			items = dw_readl(dws, SPIM_TXFLR);
			items = dws->tx_fifo_len - items;
			size = min(items, steps);

			for (i = 0; i < size; i++) {
				if (inc == 1)
					dw_writel(dws, SPIM_TXDR, data8[i]);
				else if (inc == 2) {
					data_tmp_16 = (((data16[i]) & 0xff00) >> 8) | (((data16[i]) & 0x00ff ) << 8);

					dw_writel(dws, SPIM_TXDR, data_tmp_16);
				}
				else if (inc == 4) {
					data_tmp_32 = (((data32[i]) & 0x000000ff) << 24 ) | (((data32[i]) & 0x0000ff00) << 8) | \
					 (((data32[i]) & 0x00ff0000) >> 8) | (((data32[i]) & 0xff000000 ) >> 24);

					dw_writel(dws, SPIM_TXDR, data_tmp_32);
				}
			}

			if (inc == 1)
				data8 += size;
			else if (inc == 2)
				data16 += size;
			else if (inc == 4)
				data32 += size;

			steps -= size;
		}
#endif /* CONFIG_ENABLE_IRQ */
		/* 等待fifo空 */
		wait_till_tf_empty(dws);
		/* 等待发送完成 */
		wait_till_not_busy(dws);
	} else { /* receive */
		cr0 |= SPI_TMOD_RO << SPI_TMOD_OFFSET;

		if (transfer->rx_nbits == SPI_NBITS_DUAL) {
			cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, (1<<30)|0);
		} else if (transfer->rx_nbits == SPI_NBITS_QUAD) {
			cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, (1<<30)|0);
		} else if (transfer->rx_nbits == SPI_NBITS_OCTAL) {
			cr0 |= SPI_FRM_OCTAL << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, (1<<30)|0);
		} else
			dw_writel(dws, SPIM_SPI_CTRLR0, 0);


#ifdef CONFIG_ENABLE_IRQ
		cr0 |= transfer->bits_per_word - 1;
		dw_writel(dws, SPIM_CTRLR0, cr0);
		dw_writel(dws, SPIM_TXFTLR, 0);
		dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW | SPI_INT_RXFI);
#else
		align_steps = steps / 4;

		if (align_steps != 0) {
			cr0 |= transfer->bits_per_word * 4 - 1;
			dw_writel(dws, SPIM_CTRLR0, cr0);
		}

		while (align_steps != 0) {
			wait_till_not_busy(dws);
			spi_enable_chip(dws, 0);
			size = min(DW_SPI_ONECE_READ_MAX_DATA/4, align_steps);
			dw_writel(dws, SPIM_RXFTLR, dws->rx_fifo_len-1);
			dw_writel(dws, SPIM_CTRLR1, size-1);
			spi_enable_chip(dws, 1);
			dw_writel(dws, SPIM_TXDR, 0x00);
			left = size;

			while (left) {
				items = dw_readl(dws, SPIM_RXFLR) & rx_len_mask;

				for (i = 0; i < items; i++) {
					data = dw_readl(dws, SPIM_TXDR);
#ifdef CONFIG_ENABLE_GEN_SPI
					data8[3] = data >> 24;
					data8[2] = data >> 16;
					data8[1] = data >> 8;
					data8[0] = data >> 0;
#else
					data8[0] = data >> 24;
					data8[1] = data >> 16;
					data8[2] = data >> 8;
					data8[3] = data >> 0;
#endif
					data8 += 4;
				}
				left -= items;
			}
			align_steps -= size;
		}

		steps = steps % 4;
#endif

		spi_enable_chip(dws, 0);
		cr0 &= ~0x1f;
		cr0 |= transfer->bits_per_word - 1;
		dw_writel(dws, SPIM_CTRLR0, cr0);

		while (steps != 0) {
#ifdef CONFIG_ENABLE_IRQ
			dws->complete_minor = 0;
#endif
			spi_enable_chip(dws, 0);
			size = min(dws->rx_fifo_len, steps);
			dw_writel(dws, SPIM_CTRLR1, size-1);
			dw_writel(dws, SPIM_RXFTLR, size-1);
			spi_enable_chip(dws, 1);
			dw_writel(dws, SPIM_TXDR, 0x00);
#ifdef CONFIG_ENABLE_IRQ
			while (size != (*(volatile unsigned int *)(&dws->complete_minor)));
			dws->complete += size;
#else
			left = size;

			while (left) {
				items = dw_readl(dws, SPIM_RXFLR) & rx_len_mask;

				for (i = 0; i < items; i++) {
					if (inc == 1) {
						*data8 = dw_readl(dws, SPIM_TXDR);
						data8++;
					} else if (inc == 2) {
						*data16 = dw_readl(dws, SPIM_TXDR);
						data16++;
					} else if (inc == 4) {
						*data32 = dw_readl(dws, SPIM_TXDR);
						data32++;
					}
				}
				left -= items;
			}
#endif
			steps -= size;
		}
	}

#ifdef CONFIG_ENABLE_IRQ
	dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW);
#endif
	spi_enable_chip(dws, 0);

	dws->cur_msg->actual_length += len;

	return 0;
}

static int dw_half_duplex_dma_transfer(struct dw_spi *dws)
{
	int inc         = dws->n_bytes;
	uint32_t len    = dws->len;
	int steps       = len / inc;
	uint8_t *buffer = (uint8_t*)dws->buffer;
	int size;
	struct spi_transfer *transfer = dws->cur_transfer;
	struct device_context *context = spi_get_ctldata(dws->cur_msg->spi);
	uint32_t cr0;
	uint32_t align_steps;
	uint32_t data;
	uint32_t cpu_len, cpu_transfer_len, offset, dma_len;
	uint32_t block_len, poll_len;

	/* 长度对齐 */
	if (len % inc)
		return -EINVAL;

	/* 地址对齐 */
	if ((uint32_t)buffer % inc)
		return -EINVAL;

	dws->complete = 0;

	spi_enable_chip(dws, 0);
	dw_writel(dws, SPIM_SER, 1);
	dw_writel(dws, SPIM_DMACR, 0);
	dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW);
	/* clear interrupt DONES flag */
	dw_readl(dws, SPIM_DONECR);

	/*
	* Adjust transfer mode if necessary. Requires platform dependent
	* chipselect mechanism.
	*/
	cr0 = context->cr0_solid;

	if (transfer->tx_buf) {
		if (steps < max(dws->tx_fifo_len, CACHE_LINE_SIZE)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		if ((transfer->tx_nbits != SPI_NBITS_DUAL) && (transfer->tx_nbits != SPI_NBITS_QUAD) \
				&& (transfer->tx_nbits != SPI_NBITS_OCTAL)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		offset = (uint32_t)buffer % CACHE_LINE_SIZE;
		if (offset) {
			cpu_len = CACHE_LINE_SIZE - offset;
			cpu_transfer_len = min(cpu_len, len);
			dws->len = cpu_transfer_len;
			dw_half_duplex_poll_transfer(dws);
			dws->len = len - cpu_transfer_len;
			dws->buffer = (void*)((uint32_t)buffer + cpu_transfer_len);
		}
		dma_len = dws->len & ~(CACHE_LINE_SIZE - 1);

		if (dma_len != 0) {
			cr0 |= transfer->bits_per_word - 1;
			cr0 |= SPI_TMOD_TO << SPI_TMOD_OFFSET;

			if (transfer->tx_nbits == SPI_NBITS_DUAL) {
				cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
			} else if (transfer->tx_nbits == SPI_NBITS_QUAD) {
				cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
			} else if (transfer->tx_nbits == SPI_NBITS_OCTAL) {
				cr0 |= SPI_FRM_OCTAL << SPI_FRM_OFFSET;
			}

			dw_writel(dws, SPIM_CTRLR0, cr0);

	                /* dcache clr & inv*/
			dcache_flush();

			dw_writel(dws, SPIM_SPI_CTRLR0,  (1<<30) | 0x01);
			dw_writel(dws, SPIM_CTRLR1, dma_len -1);
			dw_writel(dws, SPIM_DMACR,  (1 << 6) | (2 << 3) | (1 << 2));
			dw_writel(dws, SPIM_AXIARLEN,  (dws->tx_fifo_len / 2 -1) << 8);        //INIT
			dw_writel(dws, SPIM_AXIAR0,  (uint32_t)dws->buffer - GX_DRAM_VIRTUAL_OFFSET);        //INIT
			spi_enable_chip(dws, 1);
#ifdef CONFIG_ENABLE_IRQ
			dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW | SPI_INT_DONEM);
			while (dma_len != (*(volatile unsigned int *)(&dws->complete_minor)));
			dws->complete_minor = 0;
#else
			while (dma_len != (dw_readl(dws, SPIM_SR) >> 15));
#endif
			dws->cur_msg->actual_length += dma_len;
		}

		if (dma_len != dws->len) {
			dws->len -= dma_len;
			dws->buffer = (void*)((uint32_t)dws->buffer + dma_len);
			dw_half_duplex_poll_transfer(dws);
		}
	} else {  /* receive */
		if (steps < max(dws->rx_fifo_len, CACHE_LINE_SIZE)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		if ((transfer->rx_nbits != SPI_NBITS_DUAL) && (transfer->rx_nbits != SPI_NBITS_QUAD) \
				&& (transfer->rx_nbits != SPI_NBITS_OCTAL)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		offset = (uint32_t)buffer % CACHE_LINE_SIZE;
		if (offset) {
			cpu_len = CACHE_LINE_SIZE - offset;
			cpu_transfer_len = min(cpu_len, len);
			dws->len = cpu_transfer_len;
			dw_half_duplex_poll_transfer(dws);
			dws->len = len - cpu_transfer_len;
			dws->buffer = (void*)((uint32_t)buffer + cpu_transfer_len);
		}
		dma_len = dws->len & ~(CACHE_LINE_SIZE - 1);
		poll_len = dws->len % CACHE_LINE_SIZE;

		if (dma_len != 0) {
			cr0 |= transfer->bits_per_word - 1;
			cr0 |= SPI_TMOD_RO << SPI_TMOD_OFFSET;

			if (transfer->rx_nbits == SPI_NBITS_DUAL) {
				cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
			} else if (transfer->rx_nbits == SPI_NBITS_QUAD) {
				cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
			} else if (transfer->rx_nbits == SPI_NBITS_OCTAL) {
				cr0 |= SPI_FRM_OCTAL << SPI_FRM_OFFSET;
			}

	                /* dcache clr & inv*/
			dcache_flush();

			dw_writel(dws, SPIM_CTRLR0, cr0);
			dw_writel(dws, SPIM_SPI_CTRLR0,  (1<<30) | 0x01);
			dw_writel(dws, SPIM_DMACR,  (1 << 6) | (2 << 3) | (1 << 2));
			dw_writel(dws, SPIM_AXIAWLEN,  (dws->rx_fifo_len / 2 -1) << 8);        //INIT
		}

		while (dma_len != 0) {
			block_len = min(dma_len, 0x10000);

			dw_writel(dws, SPIM_CTRLR1, block_len - 1);
			dw_writel(dws, SPIM_AXIAR0,  (uint32_t)dws->buffer - GX_DRAM_VIRTUAL_OFFSET);        //INIT
			spi_enable_chip(dws, 1);
#ifdef CONFIG_ENABLE_IRQ
			dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW | SPI_INT_DONEM);
			while (block_len != (*(volatile unsigned int *)(&dws->complete_minor)));
			dws->complete_minor = 0;
#else
			while (block_len != (dw_readl(dws, SPIM_SR) >> 15));
#endif

			dcache_flush();

			dws->buffer = (void*)((uint32_t)dws->buffer + block_len);
			dma_len -= block_len;
			dws->cur_msg->actual_length += block_len;
		}

		if (poll_len) {
			dws->len = poll_len;
			dws->buffer = (void*)((uint32_t)dws->buffer);
			dw_half_duplex_poll_transfer(dws);
		}
	}

	spi_enable_chip(dws, 0);

	return 0;
}
#endif

static int dw_pump_transfers(struct dw_spi *dws)
{
	struct spi_message  *message;
	struct spi_transfer *transfer;
	struct spi_device   *spi;
	struct device_context *context;
	uint8_t bits;

	/* Get current state information */
	message  = dws->cur_msg;
	transfer = dws->cur_transfer;
	spi      = message->spi;
	context  = spi_get_ctldata(spi);

	dws->buffer     = (void*)(transfer->tx_buf ? transfer->tx_buf : transfer->rx_buf);
	dws->len        = transfer->len;

#if defined(CONFIG_ENABLE_GEN_SPI) && defined(CONFIG_ENABLE_GEN_SPI_TEST)
	if (spi->bits_per_word)
		transfer->bits_per_word = spi->bits_per_word;
#endif

	if (transfer->bits_per_word){
		bits = transfer->bits_per_word;
		bits = roundup(bits, 8);
		bits = (bits == 24) ? 32 : bits;
		dws->n_bytes = bits / 8;
	} else {
		transfer->bits_per_word = 8;
		dws->n_bytes = 1;
	}

	spi_enable_chip(dws, 0);

	dw_writel(dws, SPIM_SAMPLE_DLY, context->sample_delay);
	spi_set_clk(dws, context->clk_div);
	dw_writel(dws, SPIM_CTRLR1, 0);
	dw_writel(dws, SPIM_DMACR,  0);

//	if (dws->hw_version >= 0x3130332a)
//		return dw_half_duplex_dma_transfer(dws);
//	else
		return dw_half_duplex_poll_transfer(dws);
}

static int dw_pump_messages(struct dw_spi *dws,  struct spi_message *msg)
{
	int ret = 0;
	struct list_head *cursor;
	struct spi_device   *spi;

	/* Make sure we are not already running a message */
	if (dws->cur_msg)
		return 0;

	dws->cur_msg = msg;

	spi = msg->spi;

	spi_cs_control(dws, spi->chip_select, MRST_SPI_ENABLE);

	list_for_each(cursor, &dws->cur_msg->transfers) {
		dws->cur_transfer = list_entry(cursor, \
				struct spi_transfer, transfer_list);
		ret = dw_pump_transfers(dws);
		if (ret < 0)
			break;
	}
	spi_cs_control(dws, spi->chip_select, MRST_SPI_DISABLE);

	return ret;
}

/* spi_device use this to queue in their spi_msg */
static int dw_spi_quick_transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct dw_spi *dws = spi_master_get_dws(spi->master);
	int ret;

	msg->actual_length = 0;
	msg->spi           = spi;

	ret = dw_pump_messages(dws, msg);
	msg_giveback(dws);

	return ret;
}

/* This may be called twice for each spi dev */
static int dw_spi_setup(struct spi_device *spi)
{
	int i, div;
	struct device_context *spi_device_context;

#if !defined(CONFIG_ENABLE_GEN_SPI) || !defined(CONFIG_ENABLE_GEN_SPI_TEST)
#if defined(CONFIG_ENABLE_SPI_OCTAL)
	spi->mode = SPI_RX_OCTAL | SPI_TX_OCTAL;
#elif defined(CONFIG_ENABLE_SPI_QUAD)
	spi->mode = SPI_RX_QUAD | SPI_TX_QUAD | SPI_TX_DUAL | SPI_RX_DUAL;
#else
	spi->mode = SPI_TX_DUAL | SPI_RX_DUAL;
#endif
#endif

	uint8_t reg_data = (uint8_t) ((spi->mode>>8)&0xff);
	__raw_writeb(reg_data, DW_SPI_SPEED_MODE_REG); // set speed mode to regs, for kernel use

	if (spi->bits_per_word == 0)
		spi->bits_per_word = 8;

	/* try to find aviable context */
	spi_device_context = spi_get_ctldata(spi);
	if (!spi_device_context) {
		for (i = 0; i < CONFIG_SPI_SLAVE_NUM_MAX; i++) {
			if (spi_device_context_tab[i].spi == NULL) {
				spi_device_context = &spi_device_context_tab[i];
				break;
			}
		}
	}

	if (spi_device_context == NULL) {
		md_debug("spi error: spi_device context null\n");
		return -ENOMEM;
	}

	if (spi->max_speed_hz == 0)
		spi->max_speed_hz = 10000000;

	unsigned int frequence = CONFIG_FLASH_SPI_CLK_SRC;

	/* SPI分频为偶数, 适配device最大时钟频率 */
	div = frequence / spi->max_speed_hz;
	div &= ~0x01;

	if ((div * spi->max_speed_hz) != frequence)
		div += 2;

	spi_device_context->spi = spi;
	spi_device_context->cr0_solid = SPI_MST;
	spi_device_context->clk_div = CONFIG_SF_DEFAULT_CLKDIV;

	if(CONFIG_SF_DEFAULT_SAMPLE_DELAY == -1)
		spi_device_context->sample_delay = (div >> 1) & 0xFFFF;
	else
		spi_device_context->sample_delay = ((CONFIG_SF_DEFAULT_SAMPLE_DELAY >> 1) & 0xFFFF) | \
						   ((CONFIG_SF_DEFAULT_SAMPLE_DELAY & 0x01) << 16);

	spi_set_ctldata(spi, spi_device_context);

	return 0;
}

int dw_spi_config(unsigned int clk_div, unsigned int sample_delay, unsigned short mode)
{
	int i;
	uint8_t reg_data = (uint8_t) ((mode>>8)&0xff);
	struct device_context *spi_device_context = NULL;

	for (i = 0; i < CONFIG_SPI_SLAVE_NUM_MAX; i++) {
		if (spi_device_context_tab[i].spi != NULL) {
			spi_device_context = &spi_device_context_tab[i];
			break;
		}
	}

	if (spi_device_context == NULL) {
		md_debug("spi error: spi_device context null\n");
		return -1;
	}

	spi_device_context->spi->mode = mode;

	__raw_writeb(reg_data, DW_SPI_SPEED_MODE_REG); // set speed mode to regs, for kernel use

	spi_device_context->cr0_solid = SPI_MST;

	spi_device_context->clk_div = clk_div;

	spi_device_context->sample_delay = ((sample_delay >> 1) & 0xFFFF) | \
						   ((sample_delay & 0x01) << 16);

	return 0;
}

static void dw_spi_cleanup(struct spi_device *spi)
{
}


/* Restart the controller, disable all interrupts, clean rx fifo */
static void spi_hw_init(struct dw_spi *dws)
{
	spi_enable_chip(dws, 0);
#ifdef CONFIG_ENABLE_IRQ
	dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW);
#else
	dw_writel(dws, SPIM_IMR, 0x00);
#endif
	dw_writel(dws, SPIM_SPI_CTRLR0, 0);

	/*
	* Try to detect the FIFO depth if not set by interface driver,
	* the depth could be from 2 to 32 from HW spec
	*/
	if (!dws->tx_fifo_len){
		uint32_t fifo;
		for (fifo = 2; fifo <= 257; fifo++){
			dw_writel(dws, SPIM_TXFTLR, fifo);
			if (fifo != dw_readl(dws, SPIM_TXFTLR))
				break;
		}

		dws->tx_fifo_len = (fifo == 257) ? 0 : fifo;
		dw_writel(dws, SPIM_TXFTLR, 0);
	}

	if (!dws->rx_fifo_len){
		uint32_t fifo;
		for (fifo = 2; fifo <= 257; fifo++){
			dw_writel(dws, SPIM_RXFTLR, fifo);
			if (fifo != dw_readl(dws, SPIM_RXFTLR))
				break;
		}

		dws->rx_fifo_len = (fifo == 257) ? 0 : fifo;
		dw_writel(dws, SPIM_RXFTLR, 0);
	}

	spi_cs_init(dws);
	spi_cs_control(dws, 0, MRST_SPI_DISABLE);
}

static struct dw_spi     dwspi         = {0};
static struct spi_master dw_spi_master = {0};

static enum interrupt_return spi_master_irq_handler(uint32_t interrupt, void *pdata)
{
	struct dw_spi     *dws    = pdata;

#ifdef CONFIG_ENABLE_IRQ
	int inc         = dws->n_bytes;
	int len         = dws->len;
	int steps       = len / inc;

	int i, size, items, fifo_len;
	uint8_t *buffer = (uint8_t*)dws->buffer;
	uint8_t  *data8 = (uint8_t*)buffer + dws->complete;
	uint16_t  *data16 = (uint16_t*)buffer + dws->complete;
	uint32_t  *data32 = (uint32_t*)buffer + dws->complete;
	struct spi_transfer *transfer = dws->cur_transfer;
	uint32_t rx_len_mask = dws->rx_fifo_len * 2 -1;
	uint32_t tx_len_mask = dws->tx_fifo_len * 2 -1;

	uint32_t int_status = dw_readl(dws, SPIM_ISR);

	if (int_status & SPI_INT_RXFI) {
		//md_debug("Receive FIFO full\n");

		if (transfer != NULL && transfer->rx_buf) {
			items = dw_readl(dws, SPIM_RXFLR) & rx_len_mask;

			for (i = 0; i < items; i++) {
				if (inc == 1)
					data8[i] = dw_readl(dws, SPIM_TXDR);
				else if (inc == 2)
					data16[i] = dw_readl(dws, SPIM_TXDR);
				else if (inc == 4)
					data32[i] = dw_readl(dws, SPIM_TXDR);
			}

			dws->complete_minor += items;
		}
	}

	if (int_status & SPI_INT_TXEI) {
		if (transfer != NULL && transfer->tx_buf) {
			items = steps - dws->complete;
			if (items) {
				fifo_len = dw_readl(dws, SPIM_TXFLR) & tx_len_mask;
				fifo_len = dws->tx_fifo_len - fifo_len;
				size = min(fifo_len, items);

				/* 最后一次发送，关闭发送FIFO empty中断 */
				if (size == items)
					dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW);

				for (i = 0; i < size; i++) {
					if (inc == 1)
						dw_writel(dws, SPIM_TXDR, data8[i]);
					else if (inc == 2)
						dw_writel(dws, SPIM_TXDR, data16[i]);
					else if (inc == 4)
						dw_writel(dws, SPIM_TXDR, data32[i]);
				}

				dws->complete += size;
			}
		}
	}

	if (int_status & SPI_INT_DONEM) {
		dws->complete_minor = dw_readl(dws, SPIM_SR) >> 15;
		/* clear interrupt DONES flag */
		dw_readl(dws, SPIM_DONECR);
		dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW);
	}
#else
	uint32_t int_status = dw_readl(dws, SPIM_ISR);
	if (int_status & SPI_INT_RXFI) {
		md_debug("Receive FIFO full\n");
	}

	if (int_status & SPI_INT_TXEI) {
		md_debug("Transmit FIFO empty\n");
	}
#endif

	if (int_status & SPI_INT_TXOI) {
		md_debug("Transmit FIFO overflow\n");
		dw_readl(dws, DW_SPI_TXOICR);
	}
	if (int_status & SPI_INT_RXUI) {
		md_debug("Receive FIFO underflow\n");
	}
	if (int_status & SPI_INT_RXOI) {
		md_debug("Receive FIFO overflow\n");
		dw_readl(dws, DW_SPI_RXOICR);
	}
	if (int_status & SPI_INT_MSTI) {
		md_debug("multi-master contention\n");
	}
	if (int_status & SPI_INT_XRXOI) {
		md_debug("xip receive fifo overflow\n");
	}
	if (int_status & SPI_INT_XRXOI) {
		md_debug("xip receive fifo overflow\n");
	}
	if (int_status & SPI_INT_TXUIM) {
		md_debug("transmit fifo underflow\n");
	}
	if (int_status & SPI_INT_AXIEM) {
		md_debug("AXI error\n");
	}
	if (int_status & SPI_INT_SPITES) {
		md_debug("spi transmit error\n");
	}

	return HANDLED;
}

void gx_spi_probe(void)
{
	struct spi_master *master = &dw_spi_master;
	struct dw_spi     *dws    = NULL;
	int    ret  =     -ENOMEM;

	/* 切换到DW SPI */
#if defined(CONFIG_ARCH_CKMMU_CYGNUS) || defined(CONFIG_ARCH_CKMMU_SCORPIO)
	__raw_writel(1 << 0, GX_REG_BASE_SPI1_SELECT);
#endif

	spi_master_set_dws(master, &dwspi);
	dws                    = spi_master_get_dws(master);
#ifdef CONFIG_ENABLE_GEN_SPI
	dws->regs              = REG_BASE_GEN_SPI1;
#else
	dws->regs              = REG_BASE_SPI;                          // regs addr
#endif
	dws->master            = master;
	dws->hw_version        = dw_readl(dws, SPIM_VERSION_ID);

	master->bus_num        = CONFIG_GENERAL_SPI_BUS_SN;
	master->num_chipselect = CONFIG_SPI_SLAVE_NUM_MAX;
	master->cleanup        = dw_spi_cleanup;
	master->setup          = dw_spi_setup;
	master->transfer       = dw_spi_quick_transfer;                 // quick_transfer

	/* Basic HW init */
	spi_hw_init(dws);
	flush(dws);
	//修改状态, 队列启动
	dws->cur_msg      = NULL;
	dws->cur_transfer = NULL;

	ret = spi_register_master(master);
	if (ret) {
		md_debug("problem registering spi master\n");
	}

#ifdef CONFIG_ENABLE_IRQ
	gx_request_interrupt(DW_SPI_IRQ_NUM, IRQ, spi_master_irq_handler, dws);
#endif
}

