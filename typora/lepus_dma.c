#if 0
//spi_device_context->cr0_solid = SPI_MST | ((spi->mode << SPI_MODE_OFFSET) \
//			& SPI_MODE_MASK) ;

//spi_device_context->clk_div = CONFIG_SF_DEFAULT_CLKDIV;
//spi_device_context->sample_delay = ((CONFIG_SF_DEFAULT_SAMPLE_DELAY >> 1) & 0xFFFF) | \
//				   ((CONFIG_SF_DEFAULT_SAMPLE_DELAY & 0x01) << 16);
//transfer->bits_per_word = 8
//dws->n_bytes = 1

//spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip

//dw_writel(dws, SPIM_SAMPLE_DLY, context->sample_delay);
*(volatile unsigned int *)0x910000f0 = 0x2;// sample_delay
//spi_set_clk(dws, context->clk_div);
*(volatile unsigned int *)0x91000014 = 0x8;// clk_div
//dw_writel(dws, SPIM_CTRLR1, 0);
*(volatile unsigned int *)0x91000004 = 0x0;// CTRLR1
//dw_writel(dws, SPIM_DMACR,  0);
*(volatile unsigned int *)0x9100004c = 0x0;// DMACR


//spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip
//dw_writel(dws, SPIM_SER, 1);
*(volatile unsigned int *)0x91000010 = 0x1;// enable slave
//dw_writel(dws, SPIM_DMACR, 0);
*(volatile unsigned int *)0x9100004c = 0x0;// DMACR


//cr0 = context->cr0_solid;
//cr0 |= transfer->bits_per_word - 1;
//cr0 |= SPI_TMOD_TO << SPI_TMOD_OFFSET;
//dw_writel(dws, SPIM_CTRLR0, cr0);
/* 1 << 31| ((0x500) << 0x8)& (0x3 << 0x8) | 0x7 | 0x1 << 10*/
*(volatile unsigned int *)0x91000000 = 0x80000407;// master | spi_mode(0) | bits_per_word(8) | tmode_to
//dw_writel(dws, SPIM_SPI_CTRLR0, 0x0000202);
*(volatile unsigned int *)0x910000f4 = 0x202;// spi_ctrlr0
//spi_enable_chip(dws, 1);
*(volatile unsigned int *)0x91000008 = 0x1;// enable chip

//items = dw_readl(dws, SPIM_TXFLR) & rx_len_mask;
//items = dws->tx_fifo_len - items;
//size = min(items, steps);
//for (i = 0; i < size; i++) {
//	if (inc == 1)
//		dw_writel(dws, SPIM_TXDR, data8[i]);
*(volatile unsigned int *)0x91000060 = 0x3b;// opcode
*(volatile unsigned int *)0x91000060 = 0x30;// address
*(volatile unsigned int *)0x91000060 = 0x00;// address
*(volatile unsigned int *)0x91000060 = 0x00;// address
*(volatile unsigned int *)0x91000060 = 0x00;// dummy

///* 等待fifo空 */
//wait_till_tf_empty(dws);
///* 等待发送完成 */
//wait_till_not_busy(dws);


//		transfer->bits_per_word = 8;
//		dws->n_bytes = 1;
//	spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip

//	dw_writel(dws, SPIM_SAMPLE_DLY, context->sample_delay);
*(volatile unsigned int *)0x910000f0 = 0x2;// sample_delay
//	spi_set_clk(dws, context->clk_div);
*(volatile unsigned int *)0x91000014 = 0x8;// clk_div
//	dw_writel(dws, SPIM_CTRLR1, 0);
*(volatile unsigned int *)0x91000004 = 0x0;// CTRLR1
//	dw_writel(dws, SPIM_DMACR,  0);
*(volatile unsigned int *)0x9100004c = 0x0;// DMACR
//	spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip
//	dw_writel(dws, SPIM_SER, 1);
*(volatile unsigned int *)0x91000010 = 0x1;// enable slave
//	dw_writel(dws, SPIM_DMACR, 0);
*(volatile unsigned int *)0x9100004c = 0x0;// DMACR
//	dw_writel(dws, SPIM_DMARDLR, 7);
*(volatile unsigned int *)0x91000054 = 0x7;// DMARDLR
//	dw_writel(dws, SPIM_DMATDLR, 8);
*(volatile unsigned int *)0x91000050 = 0x8;// DMATRLD

//	cr0 = context->cr0_solid;
//			cr0 |= transfer->bits_per_word - 1;
//			cr0 |= SPI_TMOD_RO << SPI_TMOD_OFFSET;

//			if (transfer->rx_nbits == SPI_NBITS_DUAL) {
//				cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
//			} else if (transfer->rx_nbits == SPI_NBITS_QUAD) {
//				cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
//			}

//			dw_writel(dws, SPIM_CTRLR0, cr0);
/* 1 << 31| ((0x500) << 0x8)& (0x3 << 0x8) | 0x1 << 22 | 0x7 | 0x2 << 10*/
*(volatile unsigned int *)0x91000000 = 0x80400407;// master | dual | spi_mode(0) | bits_per_word(8) | tmode_ro
//			dw_writel(dws, SPIM_DMACR, 0x01);
*(volatile unsigned int *)0x9100004c = 0x1;// DMACR
//			wait_till_not_busy(dws);
//			spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip
//			block_len = min((int)dma_len, DMA_BLOCK_SIZE);

//			dw_writel(dws, SPIM_CTRLR1, block_len - 1);
*(volatile unsigned int *)0x91000004 = 0x2000;// CTRLR1
//			dma_xfer((void *)buffer, (void *)(dws->regs + SPIM_RXDR), block_len, DMA_DEV_TO_MEM, 0);
//			spi_enable_chip(dws, 1);
*(volatile unsigned int *)0x91000008 = 0x1;// enable chip
//			dw_writel(dws, SPIM_TXDR, 0x00);
*(volatile unsigned int *)0x91000060 = 0x00;// dummy -> tx_buff

//			while(__raw_readl(0x930002c0) == 0);

//			/* wait spi rx fifo empty */
//			while(dw_readl(dws, SPIM_RXFLR) != 0);

//			/* wait spi not busy */
//			while(dw_readl(dws, SPIM_SR) & 0x01);


//	spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip


        *(volatile unsigned int *)0x93000000 = lli.sar = dma_src; // channel 0 SAR
        *(volatile unsigned int *)0x93000008 = lli.dar = dma_dest; // channel 0 DAR

        /* LLP_SRC_EN | LLP_DST_EN | SMS(x) | DMS(x) | SRC_MSIZE(8) | DEST_MSIZE(32) | SRC_TR_WIDTH(8) | DST_TR_WIDTH(8) | INT_EN */
        *(volatile unsigned int *)0x93000018 = lli.ctl[0] = 0x18A0A401 ; // channel 0 CTL low 32 bit
        *(volatile unsigned int *)0x93000040 = 0x000;      // channel 0 CFG low 32 bit
        *(volatile unsigned int *)0x93000044 = 0x402; // channel 0 CFG high 32 bit

        lli.ctl[0]= lli.ctl[0] & (~(0x03 << 27));
        lli.ctl[1]= 0xfff;
        lli.dstat = 0;
        lli.llp = 0;

        flush_dcache_all();

        *(volatile unsigned int *)0x93000010 = (((unsigned int)&lli) & 0x0fffffff) | 1; // channel 0 LLP low 32 bit

        *(volatile unsigned int *)0x93000398 = 0x1; // DmaCfgReg
        *(volatile unsigned int *)0x930003a0 = 0x101; // ChEnReg

//        while (((*(volatile unsigned int *) 0x930002c0) & 0x01) == 0);
//
//        *(volatile unsigned int *)0x93000398 = 0x0; // DmaCfgReg
#endif















//spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip

//dw_writel(dws, SPIM_SAMPLE_DLY, context->sample_delay);
*(volatile unsigned int *)0x910000f0 = 0x2;// sample_delay
//spi_set_clk(dws, context->clk_div);
*(volatile unsigned int *)0x91000014 = 0x8;// clk_div
//dw_writel(dws, SPIM_CTRLR1, 0);
*(volatile unsigned int *)0x91000004 = 0x0;// CTRLR1
//dw_writel(dws, SPIM_DMACR,  0);
*(volatile unsigned int *)0x9100004c = 0x0;// DMACR


//spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip
//dw_writel(dws, SPIM_SER, 1);
*(volatile unsigned int *)0x91000010 = 0x1;// enable slave
//dw_writel(dws, SPIM_DMACR, 0);
*(volatile unsigned int *)0x9100004c = 0x0;// DMACR


//dw_writel(dws, SPIM_CTRLR0, cr0);
/* 1 << 31| ((0x500) << 0x8)& (0x3 << 0x8) | 0x7 | 0x1 << 10*/
*(volatile unsigned int *)0x91000000 = 0x80000407;// master | spi_mode(0) | bits_per_word(8) | tmode_to
//dw_writel(dws, SPIM_SPI_CTRLR0, 0x0000202);
*(volatile unsigned int *)0x910000f4 = 0x202;// spi_ctrlr0
//spi_enable_chip(dws, 1);
*(volatile unsigned int *)0x91000008 = 0x1;// enable chip

//items = dw_readl(dws, SPIM_TXFLR) & rx_len_mask;
//items = dws->tx_fifo_len - items;
//size = min(items, steps);
//for (i = 0; i < size; i++) {
//	if (inc == 1)
//		dw_writel(dws, SPIM_TXDR, data8[i]);
*(volatile unsigned int *)0x91000060 = 0x3b;// opcode
*(volatile unsigned int *)0x91000060 = 0x30;// address
*(volatile unsigned int *)0x91000060 = 0x00;// address
*(volatile unsigned int *)0x91000060 = 0x00;// address
*(volatile unsigned int *)0x91000060 = 0x00;// dummy

///* 等待fifo空 */
//wait_till_tf_empty(dws);
///* 等待发送完成 */
//wait_till_not_busy(dws);


//	spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip

//	dw_writel(dws, SPIM_SAMPLE_DLY, context->sample_delay);
*(volatile unsigned int *)0x910000f0 = 0x2;// sample_delay
//	spi_set_clk(dws, context->clk_div);
*(volatile unsigned int *)0x91000014 = 0x8;// clk_div
//	dw_writel(dws, SPIM_CTRLR1, 0);
*(volatile unsigned int *)0x91000004 = 0x0;// CTRLR1
//	dw_writel(dws, SPIM_DMACR,  0);
*(volatile unsigned int *)0x9100004c = 0x0;// DMACR
//	spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip
//	dw_writel(dws, SPIM_SER, 1);
*(volatile unsigned int *)0x91000010 = 0x1;// enable slave
//	dw_writel(dws, SPIM_DMACR, 0);
*(volatile unsigned int *)0x9100004c = 0x0;// DMACR
//	dw_writel(dws, SPIM_DMARDLR, 7);
*(volatile unsigned int *)0x91000054 = 0x7;// DMARDLR
//	dw_writel(dws, SPIM_DMATDLR, 8);
*(volatile unsigned int *)0x91000050 = 0x8;// DMATRLD

//	cr0 = context->cr0_solid;
//			cr0 |= transfer->bits_per_word - 1;
//			cr0 |= SPI_TMOD_RO << SPI_TMOD_OFFSET;

//			if (transfer->rx_nbits == SPI_NBITS_DUAL) {
//				cr0 |= SPI_FRM_DUAL << SPI_FRM_OFFSET;
//			} else if (transfer->rx_nbits == SPI_NBITS_QUAD) {
//				cr0 |= SPI_FRM_QUAD << SPI_FRM_OFFSET;
//			}

//			dw_writel(dws, SPIM_CTRLR0, cr0);
/* 1 << 31| ((0x500) << 0x8)& (0x3 << 0x8) | 0x1 << 22 | 0x7 | 0x2 << 10*/
*(volatile unsigned int *)0x91000000 = 0x80400407;// master | dual | spi_mode(0) | bits_per_word(8) | tmode_ro
//			dw_writel(dws, SPIM_DMACR, 0x01);
*(volatile unsigned int *)0x9100004c = 0x1;// DMACR
//			wait_till_not_busy(dws);
//			spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip
//			block_len = min((int)dma_len, DMA_BLOCK_SIZE);

//			dw_writel(dws, SPIM_CTRLR1, block_len - 1);
*(volatile unsigned int *)0x91000004 = 0x2000;// CTRLR1

//			dma_xfer((void *)buffer, (void *)(dws->regs + SPIM_RXDR), block_len, DMA_DEV_TO_MEM, 0);
        *(volatile unsigned int *)0x93000000 = lli.sar = dma_src; // channel 0 SAR
        *(volatile unsigned int *)0x93000008 = lli.dar = dma_dest; // channel 0 DAR

        /* LLP_SRC_EN | LLP_DST_EN | SMS(x) | DMS(x) | SRC_MSIZE(8) | DEST_MSIZE(32) | SRC_TR_WIDTH(8) | DST_TR_WIDTH(8) | INT_EN */
        *(volatile unsigned int *)0x93000018 = lli.ctl[0] = 0x18A0A401 ; // channel 0 CTL low 32 bit
        *(volatile unsigned int *)0x93000040 = 0x000;      // channel 0 CFG low 32 bit
        *(volatile unsigned int *)0x93000044 = 0x402; // channel 0 CFG high 32 bit

        lli.ctl[0]= lli.ctl[0] & (~(0x03 << 27));
        lli.ctl[1]= 0xfff;
        lli.dstat = 0;
        lli.llp = 0;

        flush_dcache_all();

        *(volatile unsigned int *)0x93000010 = (((unsigned int)&lli) & 0x0fffffff) | 1; // channel 0 LLP low 32 bit

        *(volatile unsigned int *)0x93000398 = 0x1; // DmaCfgReg
        *(volatile unsigned int *)0x930003a0 = 0x101; // ChEnReg


//			spi_enable_chip(dws, 1);
*(volatile unsigned int *)0x91000008 = 0x1;// enable chip
//			dw_writel(dws, SPIM_TXDR, 0x00);
*(volatile unsigned int *)0x91000060 = 0x00;// dummy -> tx_buff

        while (((*(volatile unsigned int *) 0x930002c0) & 0x01) == 0);

//			/* wait spi rx fifo empty */
//			while(dw_readl(dws, SPIM_RXFLR) != 0);
        while ((*(volatile unsigned int *) 0x91000024) != 0);

//			/* wait spi not busy */
//			while(dw_readl(dws, SPIM_SR) & 0x01);
        while (((*(volatile unsigned int *) 0x91000028) & 0x01) == 0);


//	spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip




//        while (((*(volatile unsigned int *) 0x930002c0) & 0x01) == 0);
//
//        *(volatile unsigned int *)0x93000398 = 0x0; // DmaCfgReg
