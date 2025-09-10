#include <common.h>
#include <string.h>
#include <stdio.h>
#include <types.h>
#include <misc.h>
#include <base_addr.h>
#include <board_config.h>
#include <autoconf.h>
#include <gx_console.h>

#include <gx_uart.h>
#include <gx_timer.h>
#include <gx_flash.h>
#include <gx_irq.h>
#include <gx_delay.h>
#include <device.h>
#include <driver/spi.h>
#include <driver/board.h>
#ifdef CONFIG_GX_USBSLAVE_SERIAL
#include <usbslave.h>
#endif
#ifdef CONFIG_DW_AXI_DMA
#include <driver/dma_axi.h>
#endif
#if defined(CONFIG_DW_AHB_DMA) || defined(CONFIG_DW_AHB_DMA_HAL)
#include <gx_dma_ahb.h>
#endif

#ifdef CONFIG_BOOT_TOOL
#include <spl/spl.h>
#endif

#include <app.h>

static void boot_init(void)
{
#if (defined CONFIG_ARCH_APUS) && (defined CONFIG_APP_BBT)
	return;
#else
#ifdef CONFIG_ENABLE_MALLOC
	/* initialize heap */
	heap_init((void *)CONFIG_SYS_MALLOC_BASE, CONFIG_SYS_MALLOC_LEN);
#endif
#endif
}

void loader_init(void)
{
	bootinfo_init();
	gx_irq_init();
	pinmux_init();

#ifdef CONFIG_BOOT_TOOL
	if (boot_info_s2.boot_device == BOOT_DEVICE_UART) {
		if (boot_info_s2.info.uart.uart_boot_port == GX_UART_PORT1)
			gx_console_init(GX_UART_PORT1, get_baudrate_in_boot_mode());
		else
			gx_console_init(GX_UART_PORT0, get_baudrate_in_boot_mode());
	} else {
		gx_console_init(CONFIG_SERIAL_PORT, CONFIG_SERIAL_BAUD_RATE);
	}
#else
	gx_console_init(CONFIG_SERIAL_PORT, CONFIG_SERIAL_BAUD_RATE);
#endif

	boot_init();
	gx_timer_init();

#ifdef CONFIG_SPI
	device_list_init();
#endif
#ifdef CONFIG_DESIGNWARE_SPI
	gx_spi_probe();
#endif

#ifdef CONFIG_SPI_MASTER_V3
	spi_master_v3_probe();
#endif

#ifdef CONFIG_GX_USBSLAVE_SERIAL
	gx_usb_gadget_init();
	gs_open();
#endif

#ifdef CONFIG_DW_AXI_DMA
	dw_dmac_init();
#endif

#if defined(CONFIG_DW_AHB_DMA) || defined(CONFIG_DW_AHB_DMA_HAL)
	gx_dma_init();
#endif

#if defined(CONFIG_BOOT_TOOL) && defined(CONFIG_CMD_I2C_DOWNLOAD_FLASH)
	extern int i2c_download_flash(void);
	if ((boot_info.info.i2c.i2c_info >> 2) & 0x1)
		i2c_download_flash();
#endif


#ifdef CONFIG_DL
	extern int dl_init(void);
	dl_init();
#endif
}

#include <gx_clock.h>
#include <gx_dcache.h>
uint32_t test_data[1000] = {0};

int main(int argc, char **argv)
{

	loader_init();

    // 准备测试数据（假设在0x200000）
	for (int i = 0; i < 1000; i++) {
		test_data[i] = i;  // 测试数据模式
	}

	gx_dcache_clean();
        // 时钟 fpga 阶段不用配置
    gx_clk_mod_set_gate(GX_CLK_MOD_IODMA, 1);

        // 1. 配置引脚复用
//        *(unsigned int *)(0xb0910000 + 0x4 * 8) = 0x4;  // P1_0 复用为 iomatrix
//        *(unsigned int *)(0xb0910000 + 0x4 * 9) = 0x4;  // P1_1 复用为 iomatrix
//        *(unsigned int *)(0xb0900000 + 0x4 * 8) = 38;   // P1_0 复用成 iodma[0]
//        *(unsigned int *)(0xb0900000 + 0x4 * 9) = 39;   // P1_1 复用成 iodma[1]

        // 2. 配置 IODMA、dma lite
    *(volatile unsigned int *)(0xB0D00000 + 0x4) = 0;          // IODMA_EN = 0
    *(volatile unsigned int *)(0xB0D00000 + 0x1000+ 0x4) = 0;  // DMA_LITE_EN = 0
    *(volatile unsigned int *)(0xB0D00000 + 0xC) = 0;           // IODMA_IER = 0
    *(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0xC) = 0;  // DMA_LITE_IER = 0

//        gx_request_irq(IRQ_NUM_IODMA, iodma_irq_handler, NULL);

    // 3. 配置 iodma
    *(volatile unsigned int *)(0xB0D00000 + 0x8) = 4;           // IODMA_CTRL = 4，输出数据bit[7:0]有效，固定长度模式

    *(volatile unsigned int *)(0xB0D00000 + 0x14) = 100;           // IODMA_TX_NUM = 1  4 字节

    *(volatile unsigned int *)(0xB0D00000 + 0x18) = 100;           // IODMA_TX_DELAY = 100  延时 100

        // 4. 配置 DMA_LITE
    *(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x38) = 0;           // DMA_LITE_ADDR_LEN_USED_NUM = 0 使用第一组
    *(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x18) = virt_to_phys(&test_data);    // DMA_LITE_ADDR0 = 0x200000
    *(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x1C) = 100;           // DMA_LITE_LEN0 = 4

    *(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x0C) = 0x3f;      // DMA_LITE_IER 开启所有中断

    *(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x3C) = 0x100;        // DMA_LITE_TO_VALUE = 0x100

    *(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x8) = 0;        // DMA_LITE_CTRL = 0，读模式

    *(volatile unsigned int *)(0xB0D00000 + 0xc) = 1;        // IODMA_IER = 1，使能 iodma 的 tx done 中断
    *(volatile unsigned int *)(0xB0D00000 + 0x1000 + 0x4) = 1;        // DMA_LITE_EN = 1，使能 dma lite
    *(volatile unsigned int *)(0xB0D00000 + 0x4) = 1;        // IODMA_EN = 1，使能 iodma

	app_start();

	while(1)
		continue;
}
