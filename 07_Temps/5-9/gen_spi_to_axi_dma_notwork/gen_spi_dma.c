void gen_spi_dma(void)
{
// init gen_spi
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x08) = 0;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x2c) = (1<<1) | (1<<2) | (1<<3) | (1<<6);
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x18) = 0;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x1c) = 0;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x14) = 0xf0;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0xF0) = (1<<16);
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x08) = 1;

    while (((*(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x28)) & (1 << 0)) == (1 << 0));

// init axm-dma
    *((volatile unsigned long long *)(0xfcc00000 + 0x58)) = 0x01ULL;
    while ((*((volatile unsigned long long *)(0xfcc00000 + 0x58)) & (1 << 0)) == (1 << 0));

    int i;
    for (i = 0; i < 6; i++)
        *((volatile unsigned long long *)(0xfcc00000 + 0x190 + i*0x100)) = 0;

// spi_read
    *(volatile unsigned int *)(uintptr_t)(0xfc001080) = (2 << 0);

//ret = gx_hal_qspi_transfer_prepare(&dws->hal_dws_spi, &trans);

    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x08) = 0;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x4c) = 0;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x10) = 0;

    // master rx_only  quad 8bit
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x00) = (1<<31) | (0x2 << 22) | (0x2 << 10) | (0x7);

    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0xF0) = (1<<16);
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x14) = 0xf0;

    // clk_stretch inst_addr_enhanced
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0xf4) = (1 << 30) | (0x2);

    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x18) = 0 << 16;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x1c) = 8 - 1;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x04) = 8-1;

//    gx_hal_spi_set_dma(dws, transfer->dma_cfg, transfer->dma_msize);

    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x54) = 4-1;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x4c) = 1;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x10) = 1;
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x08) = 1;

    // no ins and no addr, write tx fifo for starting transfer
    *(volatile unsigned int *)(uintptr_t)(0xfc000000 + 0x64) = 0x00;

//        ret |= dw_spi_recvmsg_dma_sync(&dws->hal_dws_spi, &trans);
//    dw_dma_channel_config(channel, &dma_config);

    // dst_burst_32 src_burst_8 dst_width_8 src_width_8 dst_inc src_no_inc
    *((volatile unsigned long long *)(0xfcc00000 + 0x118)) = (4ULL << 18) | (1ULL << 14) | (0ULL << 11) | (0ULL << 8) | (0ULL << 6) | (1ULL << 4) | (0ULL << 2) | (1ULL << 0);

    // dst_hardware src_hardware per_to_mem_dmac dst_per src_per dst_multblk src_muliblk
    *((volatile unsigned long long *)(0xfcc00000 + 0x120)) = (0ULL << 36) | (0ULL << 35) | (2ULL << 32) | (0ULL << 11) | (18ULL << 4) | (0ULL << 2) | (0ULL << 0);

//    dw_dma_xfer((void *)((unsigned int)buf & 0x0FFFFFFF), (void *)(hal_dws->regs + GX_HAL_SPI_RXDR),
//            size, channel);

    // int_en en
    *((volatile unsigned long long *)(0xfcc00000 + 0x10)) = 0x3ULL;
    *((volatile unsigned long long *)(0xfcc00000 + 0x198 )) = 0xffffffffULL;
    *((volatile unsigned long long *)(0xfcc00000 + 0x180 )) = 0xffffffffULL;
    *((volatile unsigned long long *)(0xfcc00000 + 0x100 )) = 0xfc000060ULL;
    *((volatile unsigned long long *)(0xfcc00000 + 0x108 )) = 0x0788a7c0ULL;
    *((volatile unsigned long long *)(0xfcc00000 + 0x110 )) = 7ULL;

    *((volatile unsigned long long *)(0xfcc00000 + 0x18)) = (1ULL<<8) | (1ULL<<0);

    unsigned long long reg_value;
    while(1) {
        reg_value = *((volatile unsigned long long *)(0xfcc00000 + 0x188));
        reg_value &= (1ULL << 1);
        if ((1ULL << 1) == reg_value) {
            break;
        }
    }

    (*(volatile unsigned int *)(uintptr_t)(0xfc001080) = (3 << 0));
}
