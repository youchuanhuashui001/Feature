
	gx_hal_dw_writel(dws, GX_HAL_SPI_ENR, 0);
	gx_hal_dw_writel(dws, GX_HAL_SPI_IMR, (1<<1) | (1<<2) | (1<<3) | (1<<6));
	gx_hal_dw_writel(dws, GX_HAL_SPI_TXFTLR, 0);
	gx_hal_dw_writel(dws, GX_HAL_SPI_RXFTLR, 0);
	gx_hal_dw_writel(dws, GX_HAL_SPI_BAUDR, 0xf0);
	gx_hal_dw_writel(dws, GX_HAL_SPI_SAMPLE_DLY, (1<<16));
//	gx_hal_spi_cs_init(dws, config->cs_num);
	gx_hal_dw_writel(dws, GX_HAL_SPI_ENR, 1);
	while ((gx_hal_dw_readl(dws, GX_HAL_SPI_SR) & GX_HAL_SR_BUSY) == GX_HAL_SR_BUSY);


//TODO: init axm-dma

	dma_write_reg(DMA_RESETREG, 0x01);
	while(1) {
		temp = dma_read_reg(DMA_RESETREG);
		if(0 == temp)
			break;
	}

	for (i = 0; i < 6; i++)
		dma_write_reg(DMA_CHXINTSIGNALEN(dma_channel),0);


/* init finish */

//		gx_hal_spi_set_cs(&dws->hal_dws_spi, spi->chip_select);  // 多cs时，传输前设置spi device的cs片选
//		gx_hal_spi_cs_control(&dws->hal_dws_spi, spi->chip_select, GX_HAL_SPI_ENABLE);


	WRITE_REG(dws->cs_regs, (2 << 0));





//ret = gx_hal_qspi_transfer_prepare(&dws->hal_dws_spi, &trans);

	gx_hal_dw_writel(dws, GX_HAL_SPI_ENR, 0);
	gx_hal_dw_writel(dws, GX_HAL_SPI_DMACR,  0);
	gx_hal_dw_writel(dws, GX_HAL_SPI_SER, 0);
	cr0 = (transfer->tx_buf ?  (GX_HAL_SPI_TMOD_TO << GX_HAL_SPI_TMOD_OFFSET) : \
			(GX_HAL_SPI_TMOD_RO << GX_HAL_SPI_TMOD_OFFSET)) | \
			(transfer->bus_mode << GX_HAL_SPI_FRF2_OFFSET);
	cr0 |= transfer->bits_per_word - 1;
	cr0 |= dws->device_context.cr0_solid;
	// master rx_only  quad 8bit
	gx_hal_dw_writel(dws, GX_HAL_SPI_CTRLR0, (1<<31) | (0x2 << 22) | (0x2 << 10) | (0x7));

	gx_hal_dw_writel(dws, GX_HAL_SPI_SAMPLE_DLY, (1<<16));
	gx_hal_dw_writel(dws, GX_HAL_SPI_BAUDR, 0xf0);

	// clk_stretch inst_addr_enhanced
	gx_hal_dw_writel(dws,GX_HAL_SPI_SPI_CTRLR0, (1 << 30) | (0x2));

	gx_hal_dw_writel(dws, GX_HAL_SPI_TXFTLR, 0 << 16);
	// TODO: 
	gx_hal_dw_writel(dws, GX_HAL_SPI_RXFTLR, dws->rx_fifo_len - 1);
	gx_hal_dw_writel(dws, GX_HAL_SPI_CTRLR1, 8-1);

//		trans.dma_cfg = GX_HAL_SPI_DMA_RX_EN;       1
//		trans.dma_msize = GX_HAL_DW_SPI_DMA_BURST_MSIZE_4;    4
//	gx_hal_spi_set_dma(dws, transfer->dma_cfg, transfer->dma_msize);

	gx_hal_dw_writel(dws, GX_HAL_SPI_DMARDLR, 4-1);
	gx_hal_dw_writel(dws, GX_HAL_SPI_DMACR, 1);
	gx_hal_dw_writel(dws, GX_HAL_SPI_SER, 1);
	gx_hal_dw_writel(dws, GX_HAL_SPI_ENR, 1);


//		ret |= dw_spi_recvmsg_dma_sync(&dws->hal_dws_spi, &trans);
//	dw_dma_channel_config(channel, &dma_config);

	// dst_burst_32 src_burst_8 dst_width_8 src_width_8 dst_inc src_no_inc
	dma_write_reg(DMA_CHXCTL(dma_channel),4 << 18 | 1 << 14 | 0 << 11 | 0 << 8 | 0 << 6 | 1 << 4 | 0 << 2 | 1 << 0);

	// dst_hardware src_hardware per_to_mem_dmac dst_per src_per dst_multblk src_muliblk
	dma_write_reg(DMA_CHXCFG(dma_channel),(0 << 36) | (0 << 35) | (2 << 32) | (0 << 11) | (18 << 4) | (0 << 2) | (0 << 0));


//	dw_dma_xfer((void *)((unsigned int)buf & 0x0FFFFFFF), (void *)(hal_dws->regs + GX_HAL_SPI_RXDR),
//			size, channel);

	// int_en en
	dma_write_reg(DMA_CFGREG, 0x3);
	dma_write_reg(DMA_CHXINTCLEAR(dma_channel),0xffffffff);
	dma_write_reg(DMA_CHXINTSTATUSEN(dma_channel),0xffffffff);
	dma_write_reg(DMA_CHXSAR(dma_channel),0xfc000060);
	dma_write_reg(DMA_CHXDAR(dma_channel),0x0788a7c0);
	dma_write_reg(DMA_CHXBLKTS(dma_channel),8-1);

	dma_write_reg(DMA_CHENREG, (1<<8) | (1<<0));







	while(1) {
		reg_value = dma_read_reg(DMA_CHXINTSTATUS(dma_channel));
		reg_value &= (DWAXIDMAC_IRQ_DMA_TRF);
		if (DWAXIDMAC_IRQ_DMA_TRF == reg_value) {
			break;
		}
	}


	gx_hal_spi_cs_control(&dws->hal_dws_spi, spi->chip_select, GX_HAL_SPI_DISABLE);
