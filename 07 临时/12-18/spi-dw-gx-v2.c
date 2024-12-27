/*
 *  driver for DW SPI
 *
 * Copyright (c) 2021, Nationalchip.com.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/scatterlist.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/completion.h>

#include <linux/of_platform.h>
#include <linux/property.h>
#include <linux/dma-mapping.h>
#include <linux/highmem.h>
#include <linux/dmaengine.h>

#include "dw_spi_v2.h"

#define DRIVER_NAME "dw_spi_v2"
//#define CONFIG_ENABLE_DW_SPI_IRQ

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

#define CONFIG_SPI_SLAVE_NUM_MAX  2
#define CONFIG_GENERAL_SPI_BUS_SN 0

#define CYGNUS_DW_SPI_CS_BASE     0x0030a904

#define DW_SPI_ONECE_READ_MAX_DATA  0x10000

#ifndef roundup
#define roundup(x, y)     ((((x) + ((y) - 1)) / (y)) * (y))
#endif

int dw_spi_debugfs_init(struct dw_spi *dws);
void dw_spi_remove_host(struct dw_spi *dws);
extern unsigned int gx_chip_id_probe(void);

volatile unsigned int set_dma = 0;

/* SPI device running context */
struct device_context {
	struct spi_device *spi;
	uint32_t cr0_solid;
	uint32_t clk_div;        /* baud rate divider */
	uint32_t  sample_delay;
};

static struct device_context spi_device_context_tab[CONFIG_SPI_SLAVE_NUM_MAX];

static void wait_till_tf_empty(struct dw_spi *dws)
{
	while ((dw_readl(dws, SPIM_SR) & SR_TF_EMPT) != SR_TF_EMPT);
}

static void wait_till_not_busy(struct dw_spi *dws)
{
	while ((dw_readl(dws, SPIM_SR) & SR_BUSY) == SR_BUSY);
}

static int spi_cs_init(struct dw_spi *dws)
{
	char __iomem *base = NULL;
	unsigned int gx_chipid = gx_chip_id_probe();

	pr_info("spi_cs_init is call gx_chipid (0x%x)\n",gx_chipid);

	if (dws->cs_regs)
		return 0;

	switch(gx_chipid){
		case 0x6705: /* cygnus */
			base = ioremap(0x0030a900, 0x04);
			if (!base) {
				pr_err("error: [%s %d]\n", __func__, __LINE__);
				return -ENOMEM;
			}
			/* 切换到DW SPI */
			writel(0x01, base);
			iounmap(base);

			dws->cs_regs = ioremap(CYGNUS_DW_SPI_CS_BASE, 4);

			break;
		case 0x6631:  //canopus
		case 0x6633:  //vega
			if (dws->regs != 0)
				dws->cs_regs = (void __iomem *)((uint32_t)dws->regs + 0x404);
			break;
		default:
			break;
	}

	return 0;
}


static int spi_cs_deinit(struct dw_spi *dws)
{
	unsigned int gx_chipid = gx_chip_id_probe();

	pr_info("spi_cs_init is call gx_chipid (0x%x)\n",gx_chipid);

	if (dws->cs_regs) {
		iounmap(dws->cs_regs);
		return 0;
	}

	switch(gx_chipid){
		case 0x6705: /* cygnus */
			iounmap(dws->cs_regs);
			break;
		case 0x6631:  //canopus
		case 0x6633:  //vega
			break;
		default:
			break;
	}

	return 0;
}

static void spi_cs_control(struct dw_spi *dws, uint32_t cs, uint8_t flag)
{
	unsigned int gx_chipid = gx_chip_id_probe();

	if (cs >= CONFIG_SPI_SLAVE_NUM_MAX)
		return;

	switch(flag) {

	case MRST_SPI_DISABLE:
		if (gx_chipid == 0x3502)
			__raw_writel(0xf, dws->cs_regs);
		else
			__raw_writel(3, dws->cs_regs);
		break;
	case MRST_SPI_ENABLE:
		if (gx_chipid == 0x3502)
			__raw_writel(0xe, dws->cs_regs);
		else
			__raw_writel(2, dws->cs_regs);
		break;
	default:
		return;
	}
}

static void dw_spi_set_cs(struct spi_device *spi, bool enable)
{
	struct dw_spi *dws = spi_master_get_devdata(spi->master);
	unsigned int gx_chipid = gx_chip_id_probe();

	if (enable) {
		if (gx_chipid == 0x3502)
			__raw_writel(0xf, dws->cs_regs);
		else
			__raw_writel(3, dws->cs_regs);
	} else {
		if (gx_chipid == 0x3502)
			__raw_writel(0xe, dws->cs_regs);
		else
			__raw_writel(2, dws->cs_regs);
	}
}

static int dw_half_duplex_poll_transfer(struct dw_spi *dws)
{
	int inc         = dws->n_bytes;
	int len         = dws->len;
	int steps       = len / inc;
	uint8_t *buffer = (uint8_t*)dws->buffer;
	int size;
#ifndef CONFIG_ENABLE_DW_SPI_IRQ
	int i, left, items;
	uint8_t  *data8 = (uint8_t*)buffer;
	uint16_t  *data16 = (uint16_t*)buffer;
	uint32_t  *data32 = (uint32_t*)buffer;
	uint32_t rx_len_mask = dws->rx_fifo_len * 2 -1;
	uint32_t align_steps;
	uint32_t data;
#endif
	struct spi_transfer *transfer = dws->cur_transfer;
	struct device_context *context = spi_get_ctldata(dws->cur_msg->spi);
	uint32_t cr0;

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

	/*
	* Adjust transfer mode if necessary. Requires platform dependent
	* chipselect mechanism.
	*/
	cr0 = context->cr0_solid;

	if (transfer->tx_buf) {
		cr0 |= transfer->bits_per_word - 1;
		cr0 |= SPI_TMOD_TO << SPI_TMOD_OFFSET;

		if (transfer->tx_nbits == SPI_NBITS_DUAL) {
			cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, 0x0000202);
		} else if (transfer->tx_nbits == SPI_NBITS_QUAD) {
			cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, 0x0000202);
		} else if (transfer->tx_nbits == SPI_NBITS_OCTAL) {
			cr0 |= SPI_FRM_OCTAL << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, 0x0000202);
		}

		dw_writel(dws, SPIM_CTRLR0, cr0);
		spi_enable_chip(dws, 1);

#ifdef CONFIG_ENABLE_DW_SPI_IRQ
		dw_writel(dws, SPIM_TXFTLR, dws->tx_fifo_len / 2);
		dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW | SPI_INT_TXEI);

		while (wait_for_completion_interruptible(&dws->irq_completion));
#else /* CONFIG_ENABLE_DW_SPI_IRQ */
		while (steps) {
			items = dw_readl(dws, SPIM_TXFLR) & rx_len_mask;
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

			steps -= size;
		}
#endif /* CONFIG_ENABLE_DW_SPI_IRQ */
		/* 等待fifo空 */
		wait_till_tf_empty(dws);
		/* 等待发送完成 */
		wait_till_not_busy(dws);
	} else { /* receive */
		cr0 |= SPI_TMOD_RO << SPI_TMOD_OFFSET;

		if (transfer->rx_nbits == SPI_NBITS_DUAL) {
			cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, (1 << 30) | 0);
		} else if (transfer->rx_nbits == SPI_NBITS_QUAD) {
			cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, (1<<30)|0);
		} else if (transfer->rx_nbits == SPI_NBITS_OCTAL) {
			cr0 |= SPI_FRM_OCTAL << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, (1 << 30) | 0);
		}

#ifdef CONFIG_ENABLE_DW_SPI_IRQ
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
			size = min((uint32_t)(DW_SPI_ONECE_READ_MAX_DATA/4), (uint32_t)align_steps);
			dw_writel(dws, SPIM_RXFTLR, dws->rx_fifo_len-1);
			dw_writel(dws, SPIM_CTRLR1, size-1);
			spi_enable_chip(dws, 1);
			dw_writel(dws, SPIM_TXDR, 0x00);
			left = size;

			while (left) {
				items = dw_readl(dws, SPIM_RXFLR) & rx_len_mask;

				/* 机顶盒产品线大小端由驱动转换 */
				if (likely(!dws->config_endian)) {
					for (i = 0; i < items; i++) {
						data = dw_readl(dws, SPIM_TXDR);

#if 1
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
				/* AI 产品线大小端由硬件转换 */
				}else {
					for (i = 0; i < items; i++) {
						data = dw_readl(dws, SPIM_TXDR);

						data8[3] = data >> 24;
						data8[2] = data >> 16;
						data8[1] = data >> 8;
						data8[0] = data >> 0;
						data8 += 4;
					}
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
#ifdef CONFIG_ENABLE_DW_SPI_IRQ
			dws->complete_minor = 0;
#endif
			spi_enable_chip(dws, 0);
			size = min(dws->rx_fifo_len, (uint32_t)steps);
			dw_writel(dws, SPIM_CTRLR1, size-1);
			dw_writel(dws, SPIM_RXFTLR, size-1);
			spi_enable_chip(dws, 1);
			dw_writel(dws, SPIM_TXDR, 0x00);
#ifdef CONFIG_ENABLE_DW_SPI_IRQ
			while (wait_for_completion_interruptible(&dws->irq_completion));
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

	spi_enable_chip(dws, 0);
#ifdef CONFIG_ENABLE_DW_SPI_IRQ
	dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW);
#endif

	return 0;
}

#if 1

static unsigned int my_rx_done = 0;
static void dw_spi_dma_rx_done(void *arg)
{
//	struct dw_spi *dws = arg;
	my_rx_done = 1;
	pr_info("dma rx done\n");
}

static bool dw_spi_dma_filter(struct dma_chan *chan, void *param)
{
    struct dw_spi *dws = param;
    // 根据实际硬件指定具体的 DMA 通道
    return (chan->chan_id == 0 && chan->device->dev_id == 0); 
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
//	uint32_t align_steps;
//	uint32_t data;
	uint32_t cpu_len, cpu_transfer_len, offset, dma_len;
	uint32_t block_len, poll_len;

	dma_cap_mask_t mask;
	struct dma_chan *dma_channel;
	struct dma_slave_config rxconf;
	struct dma_async_tx_descriptor *rxdesc;
//	dma_addr_t dma_handle;
	dma_cookie_t cookie;
	enum dma_status		status;
	struct scatterlist rx_sg;
	int ret;

	bool vmalloced_buf;
	struct page *vm_page;
	bool kmap_buf = false;

	dma_addr_t dma_handle;
	size_t  page_left, page_ofs;




//	DW_DMAC_CH_CONFIG dma_config;
//	int dma_channel;

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
	dw_writel(dws, SPIM_DMARDLR, 3);
	dw_writel(dws, SPIM_DMATDLR, 8);

	/*
	* Adjust transfer mode if necessary. Requires platform dependent
	* chipselect mechanism.
	*/
	cr0 = context->cr0_solid;

	if (transfer->tx_buf) {
		return dw_half_duplex_poll_transfer(dws);
#if 0
		if (steps < max(dws->tx_fifo_len, (uint32_t)ARCH_DMA_MINALIGN)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		offset = (uint32_t)buffer % ARCH_DMA_MINALIGN;
		if (offset) {
			cpu_len = CACHE_LINE_SIZE - offset;
			cpu_transfer_len = min(cpu_len, len);
			dws->len = cpu_transfer_len;
			dw_half_duplex_poll_transfer(dws);
			dws->len = len - cpu_transfer_len;
			dws->buffer = (void*)((uint32_t)buffer + cpu_transfer_len);
		}
		dma_len = dws->len & ~(ARCH_DMA_MINALIGN - 1);

		cr0 |= transfer->bits_per_word - 1;
		cr0 |= SPI_TMOD_TO << SPI_TMOD_OFFSET;

		if (transfer->tx_nbits == SPI_NBITS_DUAL) {
			cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, 0x0000202);
		} else if (transfer->tx_nbits == SPI_NBITS_QUAD) {
			cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
			dw_writel(dws, SPIM_SPI_CTRLR0, 0x0000202);
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
		dma_config.mblk_type = DWAXIDMAC_MBLK_TYPE_CONTIGUOUS;

		dw_dma_channel_config(dma_channel, &dma_config);

		if (dma_len != 0) {
			dcache_flush();

			dw_writel(dws, SPIM_CTRLR1, dma_len -1);
			dw_writel(dws, SPIM_DMACR, 0x02);
			spi_enable_chip(dws, 1);

			dw_dma_xfer((void *)(dws->regs + SPIM_TXDR), (void *)dws->buffer, \
					dma_len, dma_channel);
			dw_dma_wait_complete(dma_channel);

			dws->cur_msg->actual_length += dma_len;
		}
		/* 等待fifo空 */
		wait_till_tf_empty(dws);
		/* 等待发送完成 */
		wait_till_not_busy(dws);

		if (dma_len != dws->len) {
			dws->len -= dma_len;
			dws->buffer = (void*)((uint32_t)dws->buffer + dma_len);
			dw_half_duplex_poll_transfer(dws);
		}
#endif
	} else {  /* receive */


		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE, mask);

		if (steps < max(dws->rx_fifo_len, (uint32_t)ARCH_DMA_MINALIGN)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		offset = (uint32_t)buffer % (uint32_t)ARCH_DMA_MINALIGN;
		if (offset) {
			cpu_len = (uint32_t)ARCH_DMA_MINALIGN - offset;
			cpu_transfer_len = min_t(uint32_t, cpu_len, len);
			dws->len = cpu_transfer_len;
			dw_half_duplex_poll_transfer(dws);
			dws->len = len - cpu_transfer_len;
			dws->buffer = (void*)((uint32_t)buffer + cpu_transfer_len);
		}

		dma_len = dws->len & ~(ARCH_DMA_MINALIGN - 1);
		poll_len = dws->len % ARCH_DMA_MINALIGN;

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

			//dcache_flush();

			dw_writel(dws, SPIM_CTRLR0, cr0);
			dw_writel(dws, SPIM_DMACR, 0x01);
		}

		dma_channel = dma_request_channel(mask, dw_spi_dma_filter, dws);
		if (!dma_channel) {
			pr_err("dma_request_channel failed\n");
			return -EINVAL;
		}


		memset(&rxconf, 0, sizeof(rxconf));

/*		dma_handle = dma_map_single(&dws->master->dev, dws->buffer, \
						dma_len, DMA_TO_DEVICE);
		if (dma_mapping_error(&dws->master->dev, dma_handle))
			return -ENOMEM;
*/

		rxconf.direction = DMA_DEV_TO_MEM;
//		rxconf.src_addr = (uint32_t)(dws->regs + SPIM_RXDR);
		rxconf.src_addr = (dma_addr_t)(dws->paddr + SPIM_RXDR);
		rxconf.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		rxconf.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		rxconf.device_fc = false;
		ret = dmaengine_slave_config(dma_channel, &rxconf);
		if (ret) {
			dev_err(&dws->master->dev, "dmaengine_slave_config failed\n");
			return ret;
		}

//		sg_init_table(&rx_sg, 1);
//		sg_dma_address(&rx_sg) = dma_handle;
//		sg_dma_len(&rx_sg) = dma_len;

//		if (dma_mapping_error(&dws->master->dev, dma_handle))
//			return -ENOMEM;




// 使用 dma_handle 配置 DMA 传输
//rxconf.src_addr = (uint32_t)(dws->regs + SPIM_RXDR);
//rxconf.dst_addr = dma_handle;  // 使用映射后的地址



		while (dma_len != 0) {
			wait_till_not_busy(dws);
			spi_enable_chip(dws, 0);
			block_len = min_t(uint32_t, dma_len, 0x10000);

			dw_writel(dws, SPIM_CTRLR1, block_len - 1);

//			sg_init_one(&rx_sg, dws->buffer, block_len);
//			ret = dma_map_sg(&dws->master->dev, &rx_sg, 1, DMA_DEV_TO_MEM);
//			if (ret) {
//				dev_err(&dws->master->dev, "dma_map_sg failed\n");
//				return ret;
//			}

// 检查缓冲区类型
vmalloced_buf = is_vmalloc_addr(dws->buffer);
#ifdef CONFIG_HIGHMEM
kmap_buf = ((unsigned long)dws->buffer >= PKMAP_BASE &&
        (unsigned long)dws->buffer < (PKMAP_BASE + (LAST_PKMAP * PAGE_SIZE)));
#endif

// 根据缓冲区类型选择映射方式
if (vmalloced_buf || kmap_buf) {
    // 处理 vmalloc 或 kmap 的内存
    vm_page = vmalloc_to_page(dws->buffer);
    if (!vm_page)
        return -ENOMEM;
    page_ofs = offset_in_page(dws->buffer);
    page_left = PAGE_SIZE - page_ofs;
    block_len = min_t(uint32_t, block_len, page_left);
    dma_handle = dma_map_page(&dws->master->dev, vm_page,
                page_ofs, block_len, DMA_DEV_TO_MEM);
	pr_info("vmlloced_buf or kmap_buf\n");
} else {
    // 处理普通内存
    dma_handle = dma_map_single(&dws->master->dev, dws->buffer,
                block_len, DMA_DEV_TO_MEM);
	pr_info("normal\n");
}

	pr_info("dws->buffer:0x%x\n", dws->buffer);
	if (dma_mapping_error(&dws->master->dev, dma_handle))
	    return -ENOMEM;

	// 初始化 scatter-gather 列表
	sg_init_table(&rx_sg, 1);
	sg_dma_address(&rx_sg) = dma_handle;
	sg_dma_len(&rx_sg) = block_len;

	rxdesc = dmaengine_prep_slave_sg(dma_channel, &rx_sg, 1,
                     DMA_DEV_TO_MEM,
                     DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!rxdesc) {
	    dev_err(&dws->master->dev, "dmaengine_prep_slave_sg failed\n");
	    dma_unmap_single(&dws->master->dev, dma_handle, block_len, DMA_DEV_TO_MEM);
	    return -EINVAL;
	}

			rxdesc->callback = dw_spi_dma_rx_done;
			rxdesc->callback_param = dws;

			cookie = dmaengine_submit(rxdesc);

	if (dma_submit_error(cookie)) {
	    dev_err(&dws->master->dev, "dma_submit_error\n");
	    dma_unmap_single(&dws->master->dev, dma_handle, block_len, DMA_DEV_TO_MEM);
	    return -EBUSY;
	}

//			dma_async_issue_pending(dma_channel);	
/*			dw_dma_xfer((void *)dws->buffer, (void *)(dws->regs + SPIM_RXDR), \
					block_len, dma_channel);*/
			spi_enable_chip(dws, 1);
			dw_writel(dws, SPIM_TXDR, 0x00);

			dma_async_issue_pending(dma_channel);	
			while (!my_rx_done);

//				status = dma_async_is_tx_complete(dma_channel, cookie, NULL, NULL);
//				pr_info("dma_async_is_tx_complete: %s\n", (status == 0) ? "done" : "busy");

			/* wait spi rx fifo empty */
			while(dw_readl(dws, SPIM_RXFLR) != 0);

			/* wait spi not busy */
			while(dw_readl(dws, SPIM_SR) & 0x01);

			dws->buffer = (void*)((uint32_t)dws->buffer + block_len);
			dma_len -= block_len;
			dws->cur_msg->actual_length += block_len;
		}
		dma_release_channel(dma_channel);

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
static int dw_half_duplex_dma(struct dw_spi *dws)
{
	int inc         = dws->n_bytes;
	uint32_t len    = dws->len;
	int steps       = len / inc;
	uint8_t *buffer = (uint8_t*)dws->buffer;
	struct spi_transfer *transfer = dws->cur_transfer;
	struct device_context *context = spi_get_ctldata(dws->cur_msg->spi);
	uint32_t cr0;
	uint32_t cpu_len, cpu_transfer_len, offset, dma_len;
	uint32_t block_len, poll_len;
	bool vmalloced_buf;
	struct page *vm_page;
	bool kmap_buf = false;

	dma_addr_t dma_handle;
	size_t  page_left, page_ofs;

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
		if (steps < max(dws->tx_fifo_len, (uint32_t)ARCH_DMA_MINALIGN)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		if ((transfer->tx_nbits != SPI_NBITS_DUAL) && (transfer->tx_nbits != SPI_NBITS_QUAD) \
				&& (transfer->tx_nbits != SPI_NBITS_OCTAL)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		offset = (uint32_t)buffer % ARCH_DMA_MINALIGN;
		if (offset) {
			cpu_len = ARCH_DMA_MINALIGN - offset;
			cpu_transfer_len = min(cpu_len, len);
			dws->len = cpu_transfer_len;
			dw_half_duplex_poll_transfer(dws);
			dws->len = len - cpu_transfer_len;
			dws->buffer = (void*)((uint32_t)buffer + cpu_transfer_len);
		}

		dma_len = dws->len & ~(ARCH_DMA_MINALIGN - 1);
		poll_len = dws->len % ARCH_DMA_MINALIGN;

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
			dw_writel(dws, SPIM_SPI_CTRLR0,  (1<<30) | 0x01);
			dw_writel(dws, SPIM_DMACR,  (1 << 6) | (2 << 3) | (1 << 2));
			dw_writel(dws, SPIM_AXIARLEN,  (dws->tx_fifo_len / 2 -1) << 8);
			vmalloced_buf  = is_vmalloc_addr(dws->buffer);
#ifdef CONFIG_HIGHMEM
			kmap_buf = ((unsigned long)buf >= PKMAP_BASE &&
					(unsigned long)buf < (PKMAP_BASE +
						(LAST_PKMAP * PAGE_SIZE)));
#endif
		}

#define DW_SPI_BLOCK_TS   0x10000

		while (dma_len != 0) {
			block_len = min_t(uint32_t, dma_len, DW_SPI_BLOCK_TS);
			if (vmalloced_buf)
				vm_page = vmalloc_to_page(dws->buffer);
#ifdef CONFIG_HIGHMEM
			else if (kmap_buf)
				vm_page = kmap_to_page(dws->buffer);
#endif

			if (vmalloced_buf || kmap_buf) {
				if (!vm_page)
					return -ENOMEM;
				page_ofs  = offset_in_page(dws->buffer);
				page_left = PAGE_SIZE - page_ofs;
				block_len = min_t(uint32_t, block_len, page_left);
				dma_handle = dma_map_page(&dws->master->dev, vm_page, \
						page_ofs, block_len, DMA_TO_DEVICE);
			} else {
				dma_handle = dma_map_single(&dws->master->dev, dws->buffer, \
						block_len, DMA_TO_DEVICE);
			}

			if (dma_mapping_error(&dws->master->dev, dma_handle))
				return -ENOMEM;

			dw_writel(dws, SPIM_CTRLR1, block_len - 1);
			dw_writel(dws, SPIM_AXIAR0,  (uint32_t)dma_handle);

			spi_enable_chip(dws, 1);
#ifdef CONFIG_ENABLE_DW_SPI_IRQ
			dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW | SPI_INT_DONEM);
			while (wait_for_completion_interruptible(&dws->irq_completion));
			dws->complete_minor = 0;
#else
			while (block_len != (dw_readl(dws, SPIM_SR) >> 15));
#endif
			if (vmalloced_buf || kmap_buf)
				dma_unmap_page(&dws->master->dev, dma_handle, \
						block_len, DMA_TO_DEVICE);
			else
				dma_unmap_single(&dws->master->dev, dma_handle, \
						block_len, DMA_TO_DEVICE);

			dws->buffer = (void*)((uint32_t)dws->buffer + block_len);
			dma_len -= block_len;
		}

		if (poll_len) {
			dws->len = poll_len;
			dws->buffer = (void*)((uint32_t)dws->buffer);
			dw_half_duplex_poll_transfer(dws);
		}
	} else {  /* receive */
		if (steps < max(dws->rx_fifo_len, (uint32_t)ARCH_DMA_MINALIGN)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		if ((transfer->rx_nbits != SPI_NBITS_DUAL) && (transfer->rx_nbits != SPI_NBITS_QUAD) \
				&& (transfer->rx_nbits != SPI_NBITS_OCTAL)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		offset = (uint32_t)buffer % ARCH_DMA_MINALIGN;
		if (offset) {
			cpu_len = ARCH_DMA_MINALIGN - offset;
			cpu_transfer_len = min(cpu_len, len);
			dws->len = cpu_transfer_len;
			dw_half_duplex_poll_transfer(dws);
			dws->len = len - cpu_transfer_len;
			dws->buffer = (void*)((uint32_t)buffer + cpu_transfer_len);
		}
		dma_len = dws->len & ~(ARCH_DMA_MINALIGN - 1);
		poll_len = dws->len % ARCH_DMA_MINALIGN;

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

			dw_writel(dws, SPIM_CTRLR0, cr0);
			dw_writel(dws, SPIM_SPI_CTRLR0,  (1<<30) | 0x01);
			dw_writel(dws, SPIM_DMACR,  (1 << 6) | (2 << 3) | (1 << 2));
			dw_writel(dws, SPIM_AXIAWLEN,  0xf << 8);
			dw_writel(dws, SPIM_RXFTLR, dws->rx_fifo_len - 1);
			vmalloced_buf  = is_vmalloc_addr(dws->buffer);
#ifdef CONFIG_HIGHMEM
			kmap_buf = ((unsigned long)buf >= PKMAP_BASE &&
					(unsigned long)buf < (PKMAP_BASE +
						(LAST_PKMAP * PAGE_SIZE)));
#endif
		}

		while (dma_len != 0) {
			block_len = min_t(uint32_t, dma_len, DW_SPI_BLOCK_TS);
			if (vmalloced_buf)
				vm_page = vmalloc_to_page(dws->buffer);
#ifdef CONFIG_HIGHMEM
			else if (kmap_buf)
				vm_page = kmap_to_page(dws->buffer);
#endif

			if (vmalloced_buf || kmap_buf) {
				if (!vm_page)
					return -ENOMEM;
				page_ofs  = offset_in_page(dws->buffer);
				page_left = PAGE_SIZE - page_ofs;
				block_len = min_t(uint32_t, block_len, page_left);
				dma_handle = dma_map_page(&dws->master->dev, vm_page, \
						page_ofs, block_len, DMA_FROM_DEVICE);
			} else {
				dma_handle = dma_map_single(&dws->master->dev, dws->buffer, \
						block_len, DMA_FROM_DEVICE);
			}

			if (dma_mapping_error(&dws->master->dev, dma_handle))
				return -ENOMEM;


			dw_writel(dws, SPIM_CTRLR1, block_len - 1);
			dw_writel(dws, SPIM_AXIAR0,  (uint32_t)dma_handle);
			spi_enable_chip(dws, 1);
#ifdef CONFIG_ENABLE_DW_SPI_IRQ
			dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW | SPI_INT_DONEM);
			while (wait_for_completion_interruptible(&dws->irq_completion));
			dws->complete_minor = 0;
#else
			while (block_len != (dw_readl(dws, SPIM_SR) >> 15));
#endif
			if (vmalloced_buf || kmap_buf)
				dma_unmap_page(&dws->master->dev, dma_handle, \
						block_len, DMA_FROM_DEVICE);
			else
				dma_unmap_single(&dws->master->dev, dma_handle, \
						block_len, DMA_FROM_DEVICE);

			dws->buffer = (void*)((uint32_t)dws->buffer + block_len);
			dma_len -= block_len;
		}

		if (poll_len) {
			dws->len = poll_len;
			dws->buffer = (void*)((uint32_t)dws->buffer);
			dw_half_duplex_poll_transfer(dws);
		}
	}

	spi_enable_chip(dws, 0);
#ifdef CONFIG_ENABLE_DW_SPI_IRQ
	dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW);
#endif

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
	message  = dws->master->cur_msg;
	transfer = dws->cur_transfer;
	spi      = message->spi;
	context  = spi_get_ctldata(spi);

	dws->buffer = (void*)(transfer->tx_buf ? transfer->tx_buf : transfer->rx_buf);
	dws->len    = transfer->len;

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


	/* 由分频和采样延时由loader配置，这里不修改 */
	//dw_writel(dws, SPIM_SAMPLE_DLY, context->sample_delay);
	//spi_set_clk(dws, context->clk_div);
	dw_writel(dws, SPIM_CTRLR1, 0);
	dw_writel(dws, SPIM_DMACR,  0);

//	if (dws->hw_version >= 0x3130332a)
	if (set_dma == 1)
		return dw_half_duplex_dma_transfer(dws);
	else
		return dw_half_duplex_poll_transfer(dws);
}

static int dw_spi_transfer_one(struct spi_master *master,
		struct spi_device *spi, struct spi_transfer *transfer)
{
	struct dw_spi *dws = spi_master_get_devdata(spi->master);
	int ret = 0;

	dws->cur_msg = master->cur_msg;
	dws->cur_transfer = transfer;
	ret = dw_pump_transfers(dws);

	return ret;
}

/* This may be called twice for each spi dev */
static int dw_spi_setup(struct spi_device *spi)
{
	int i, div;
	struct device_context *spi_device_context;
	unsigned int frequence = 0;
	uint16_t mode = 0;
	struct dw_spi *dws = (struct dw_spi *)spi_master_get_devdata(spi->master);

	if (dws->mode_reg) {
		mode = (uint16_t) __raw_readl(dws->mode_reg); // get speed mode to regs, for kernel use
		spi->mode = (mode & 0xff) << 8;
	}
	else
		spi->mode = SPI_RX_DUAL | SPI_RX_QUAD | SPI_TX_DUAL | SPI_TX_QUAD;

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
		pr_err("spi error: spi_device context null\n");
		return -ENOMEM;
	}

	if (spi->max_speed_hz == 0)
		spi->max_speed_hz = 10000000;


	/* SPI分频为偶数, 适配device最大时钟频率 */
	div = frequence / spi->max_speed_hz;
	div &= ~0x01;

	if ((div * spi->max_speed_hz) != frequence)
		div += 2;

	spi_device_context->spi = spi;
	spi_device_context->cr0_solid = SPI_MST | ((spi->mode << SPI_MODE_OFFSET) \
			& SPI_MODE_MASK) ;
	spi_device_context->clk_div = div;

	/* 由分频和采样延时由loader配置，这里不修改 */
	spi_device_context->sample_delay = 0;
	spi_set_ctldata(spi, spi_device_context);

	return 0;
}

static void dw_spi_cleanup(struct spi_device *spi)
{
}


/* Restart the controller, disable all interrupts, clean rx fifo */
static void spi_hw_init(struct dw_spi *dws)
{
	spi_enable_chip(dws, 0);
	dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW);
	dw_writel(dws, SPIM_SPI_CTRLR0, 0);

	dws->hw_version = dw_readl(dws, SPIM_VERSION_ID);

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
		dw_writel(dws, SPIM_TXFTLR, 0);
	}

	spi_cs_init(dws);
	spi_cs_control(dws, 0, MRST_SPI_DISABLE);
}

static irqreturn_t spi_master_irq_handler(int interrupt, void *pdata)
{
	struct spi_master *master = pdata;
	struct dw_spi *dws = spi_master_get_devdata(master);

#ifdef CONFIG_ENABLE_DW_SPI_IRQ
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

	if (!int_status)
		return IRQ_NONE;

	if (int_status & SPI_INT_RXFI) {
		pr_devel("Receive FIFO full\n");
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
			complete(&dws->irq_completion);
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
				if (size == items) {
					dw_writel(dws, SPIM_IMR, SPI_INT_OVERFLOW);
					complete(&dws->irq_completion);
				}

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
		complete(&dws->irq_completion);
	}
#else
	uint32_t int_status = dw_readl(dws, SPIM_ISR);
	if (int_status & SPI_INT_RXFI) {
		pr_devel("Receive FIFO full\n");
	}

	if (int_status & SPI_INT_TXEI) {
		pr_devel("Transmit FIFO empty\n");
	}
#endif

	if (int_status & SPI_INT_TXOI) {
		pr_err("Transmit FIFO overflow\n");
		dw_readl(dws, DW_SPI_TXOICR);
	}
	if (int_status & SPI_INT_RXUI) {
		pr_err("Receive FIFO underflow\n");
	}
	if (int_status & SPI_INT_RXOI) {
		pr_err("Receive FIFO overflow\n");
		dw_readl(dws, DW_SPI_RXOICR);
	}
	if (int_status & SPI_INT_MSTI) {
		pr_err("multi-master contention\n");
	}
	if (int_status & SPI_INT_XRXOI) {
		pr_err("xip receive fifo overflow\n");
	}
	if (int_status & SPI_INT_XRXOI) {
		pr_err("xip receive fifo overflow\n");
	}
	if (int_status & SPI_INT_TXUIM) {
		pr_err("transmit fifo underflow\n");
	}
	if (int_status & SPI_INT_AXIEM) {
		pr_err("AXI error\n");
	}
	if (int_status & SPI_INT_SPITES) {
		pr_err("spi transmit error\n");
	}

	return IRQ_HANDLED;
}

static void dw_spi_handle_err(struct spi_master *master,
		struct spi_message *msg)
{
	struct dw_spi *dws = spi_master_get_devdata(master);

	spi_enable_chip(dws, 0);
}



static int dw_spi_v2_probe(struct platform_device *pdev)
{
	struct dw_spi *dws;
	struct resource *mem;
	struct spi_master *master;
	int ret;
	uint32_t cs_reg = 0;
	uint32_t spi_mode_reg = 0;

	dws = devm_kzalloc(&pdev->dev, sizeof(struct dw_spi),
			GFP_KERNEL);
	if (!dws)
		return -ENOMEM;

	pr_info("register dw_spi master v2\n");
	/* Get basic io resource and map it */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dws->paddr = mem->start;
	dws->regs = devm_ioremap_resource(&pdev->dev, mem);
	pr_info("dws->regs:0x%x\n", dws->regs);

	if (IS_ERR(dws->regs)) {
		dev_err(&pdev->dev, "SPI region map failed\n");
		return PTR_ERR(dws->regs);
	}

	dws->irq = platform_get_irq(pdev, 0);
	if (dws->irq < 0) {
		dev_err(&pdev->dev, "no irq resource?\n");
		return dws->irq; /* -ENXIO */
	}

	device_property_read_u32(&pdev->dev, "bus-num", &dws->bus_num);
	device_property_read_u32(&pdev->dev, "reg-io-width", &dws->reg_io_width);
	device_property_read_u32(&pdev->dev, "num-cs", &dws->num_cs);
	device_property_read_u32(&pdev->dev, "cs-reg", &cs_reg);
	device_property_read_u32(&pdev->dev, "spi-mode-reg", &spi_mode_reg);
	device_property_read_u32(&pdev->dev, "config-endian", &dws->config_endian);

	if (cs_reg)
		dws->cs_regs = ioremap(cs_reg, 4);
	else
		dws->cs_regs = NULL;

	if (spi_mode_reg)
		dws->mode_reg = ioremap(spi_mode_reg, 4);
	else
		dws->mode_reg = NULL;

	master = spi_alloc_master(&pdev->dev, 0);
	if (!master) {
		devm_kfree(&pdev->dev, dws);
		return -ENOMEM;
	}

	dws->master = master;
	dws->type = SSI_MOTO_SPI;
#ifdef CONFIG_ENABLE_DW_SPI_IRQ
	init_completion(&dws->irq_completion);
#endif
	snprintf(dws->name, sizeof(dws->name), "dw_spi%d", dws->bus_num);

	ret = request_irq(dws->irq, spi_master_irq_handler, IRQF_SHARED, dws->name, master);
	if (ret < 0) {
		dev_err(&pdev->dev, "can not get IRQ\n");
		goto err_free_master;
	}

	master->mode_bits = SPI_CS_HIGH | SPI_MODE_0;
	master->bits_per_word_mask = SPI_BPW_MASK(8) | SPI_BPW_MASK(16) | SPI_BPW_MASK(32);
	master->bus_num = dws->bus_num;
	master->num_chipselect = dws->num_cs;
	master->setup = dw_spi_setup;
	master->cleanup = dw_spi_cleanup;
	master->set_cs = dw_spi_set_cs;
	master->transfer_one = dw_spi_transfer_one;
	master->handle_err = dw_spi_handle_err;
	master->max_speed_hz = dws->max_freq;
	master->dev.of_node = pdev->dev.of_node;

	/* Basic HW init */
	spi_hw_init(dws);

	spi_master_set_devdata(master, dws);
	ret = devm_spi_register_master(&pdev->dev, master);
	if (ret) {
		dev_err(&master->dev, "problem registering spi master\n");
		goto err_free_master;
	}

	dw_spi_debugfs_init(dws);
	platform_set_drvdata(pdev, dws);

	return 0;

err_free_master:
	spi_enable_chip(dws, 0);
	free_irq(dws->irq, master);
	spi_master_put(master);
	kfree(master);
	spi_cs_deinit(dws);
	dw_spi_remove_host(dws);
	devm_kfree(&pdev->dev, dws);

	return ret;
}

static int dw_spi_v2_remove(struct platform_device *pdev)
{
	struct dw_spi *dws= platform_get_drvdata(pdev);

	dw_spi_remove_host(dws);
	spi_cs_deinit(dws);
	kfree(dws->master);
	devm_kfree(&pdev->dev, dws);

	return 0;
}

static const struct of_device_id dw_spi_v2_of_match[] = {
	{ .compatible = "nationalchip-dw-spi,v2", },
	{ /* end of table */}
};
MODULE_DEVICE_TABLE(of, dw_spi_v2_of_match);

static struct platform_driver dw_spi_gx_driver = {
	.probe		= dw_spi_v2_probe,
	.remove		= dw_spi_v2_remove,
	.driver		= {
		.name	= DRIVER_NAME,
		.of_match_table = dw_spi_v2_of_match,
	},
};
module_platform_driver(dw_spi_gx_driver);

MODULE_AUTHOR("gx");
MODULE_DESCRIPTION("gx");
MODULE_LICENSE("GPL v2");
