static int dw_half_duplex_dma_transfer(struct dw_spi *dws)
{
	int inc         = dws->n_bytes;
	int len         = dws->len;
	int steps       = len / inc;
	uint8_t *buffer = (uint8_t*)dws->buffer;
	struct spi_transfer *transfer = dws->cur_transfer;
	struct device_context *context = spi_get_ctldata(dws->cur_msg->spi);
	uint32_t cr0;
	uint32_t cpu_len, offset, dma_len;
	uint32_t block_len, poll_len;

	GX_DMA_AHB_CH_CONFIG dma_config;
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
		if (steps <= dws->tx_fifo_len) {
			return dw_half_duplex_poll_transfer(dws);
		}

		offset = (uint32_t)buffer % 4;
		if (offset) {
			cpu_len = 4 - offset;
			dws->len = cpu_len;
			dw_half_duplex_poll_transfer(dws);
			dws->len = len - cpu_len;
			dws->buffer = (void*)((uint32_t)buffer + cpu_len);
		}
		dma_len = dws->len & ~(0x03);

		cr0 |= transfer->bits_per_word - 1;
		cr0 |= SPI_TMOD_TO << SPI_TMOD_OFFSET;

		if (transfer->tx_nbits == SPI_NBITS_DUAL) {
			cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
		} else if (transfer->tx_nbits == SPI_NBITS_QUAD) {
			cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
		}

		dw_writel(dws, SPIM_CTRLR0, cr0);

		dma_channel = dw_request_chan();

		dma_config.trans_width = GX_DMA_AHB_TRANS_WIDTH_8;

		/* dma source config */
		dma_config.src_addr_update = GX_DMA_AHB_CH_CTL_L_INC;
		dma_config.src_hs_select = GX_DMA_AHB_HS_SEL_HW;
		dma_config.src_master_select = GX_DMA_AHB_MASTER_2;
		dma_config.src_msize = GX_DMA_AHB_BURST_TRANS_LEN_32;
		dma_config.src_hs_per = 0;

		/* dma destination config */
		dma_config.dst_addr_update = GX_DMA_AHB_CH_CTL_L_NOINC;
		dma_config.dst_hs_select = GX_DMA_AHB_HS_SEL_HW;
		dma_config.dst_master_select = GX_DMA_AHB_MASTER_1;
		dma_config.dst_msize = GX_DMA_AHB_BURST_TRANS_LEN_8;
		dma_config.flow_ctrl = GX_DMA_AHB_TT_FC_MEM_TO_PER_DMAC;
		dma_config.dst_hs_per = DMA_HS_PRE_SPI0_TX;

		if (dma_len != 0) {
			flush_dcache_all();

			dw_writel(dws, SPIM_CTRLR1, dma_len -1);
			dw_writel(dws, SPIM_DMACR, 0x02);
			spi_enable_chip(dws, 1);

			dma_xfer((void *)(dws->regs + SPIM_TXDR), (void *)dws->buffer, \
				dma_len, 0, dma_channel, &dma_config);

			wait_dma_complete(dma_channel);

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
		if (steps <= dws->rx_fifo_len) {
			return dw_half_duplex_poll_transfer(dws);
		}

		if ((transfer->rx_nbits != SPI_NBITS_DUAL) && (transfer->rx_nbits != SPI_NBITS_QUAD)) {
			return dw_half_duplex_poll_transfer(dws);
		}

		offset = (uint32_t)buffer % ARCH_DMA_MINALIGN;
		if (offset) {
			cpu_len = ARCH_DMA_MINALIGN - offset;
			dws->len = cpu_len;
			dw_half_duplex_poll_transfer(dws);
			dws->len = len - cpu_len;
			dws->buffer = (void*)((uint32_t)buffer + cpu_len);
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

			invalidate_dcache_range((unsigned long)dws->buffer, (unsigned long)(dws->buffer + dma_len));

			dw_writel(dws, SPIM_CTRLR0, cr0);
			dw_writel(dws, SPIM_DMACR, 0x01);
		}

		dma_channel = dw_request_chan();

		dma_config.trans_width = GX_DMA_AHB_TRANS_WIDTH_8;

		/* dma source config */
		dma_config.src_addr_update = GX_DMA_AHB_CH_CTL_L_NOINC;
		dma_config.src_hs_select = GX_DMA_AHB_HS_SEL_HW;
		dma_config.src_master_select = GX_DMA_AHB_MASTER_1;
		dma_config.src_msize = GX_DMA_AHB_BURST_TRANS_LEN_8;
		dma_config.src_hs_per = DMA_HS_PRE_SPI0_RX;

		/* dma destination config */
		dma_config.dst_addr_update = GX_DMA_AHB_CH_CTL_L_INC;
		dma_config.dst_hs_select = GX_DMA_AHB_HS_SEL_HW;
		dma_config.dst_master_select = GX_DMA_AHB_MASTER_2;
		dma_config.dst_msize = GX_DMA_AHB_BURST_TRANS_LEN_32;
		dma_config.dst_hs_per = 0;
		dma_config.flow_ctrl = GX_DMA_AHB_TT_FC_PER_TO_MEM_DMAC;

		while (dma_len != 0) {
			wait_till_not_busy(dws);
			spi_enable_chip(dws, 0);
			block_len = min((int)dma_len, 0x10000);

			dw_writel(dws, SPIM_CTRLR1, block_len - 1);
			dma_xfer((void *)dws->buffer, (void *)(dws->regs + SPIM_RXDR), block_len, \
				0, dma_channel, &dma_config);
			spi_enable_chip(dws, 1);
			dw_writel(dws, SPIM_TXDR, 0x00);

			wait_dma_complete(dma_channel);

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
