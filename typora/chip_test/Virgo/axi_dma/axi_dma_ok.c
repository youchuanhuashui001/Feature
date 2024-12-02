static int dw_half_duplex_axidma_transfer(struct dw_spi *dws)
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
	dw_writel(dws, SPIM_DMARDLR, 3);
	dw_writel(dws, SPIM_DMATDLR, 8);

	/*
	* Adjust transfer mode if necessary. Requires platform dependent
	* chipselect mechanism.
	*/
	cr0 = context->cr0_solid;

	if (transfer->tx_buf) {
		if (steps < max(dws->tx_fifo_len, CACHE_LINE_SIZE)) {
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

			dw_writel(dws, SPIM_SPI_CTRLR0,  (1<<30) | 0x01);
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
		if (steps < max(dws->rx_fifo_len, CACHE_LINE_SIZE)) {
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
		dma_config.src_msize = DWAXIDMAC_BURST_TRANS_LEN_4;
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
//			block_len = min((int)dma_len, 0x3fffff);
			block_len = min((int)dma_len, 0x10000);

			dw_writel(dws, SPIM_CTRLR1, block_len - 1);
			dw_dma_xfer((void *)dws->buffer, (void *)(dws->regs + SPIM_RXDR), \
					block_len, dma_channel);
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

