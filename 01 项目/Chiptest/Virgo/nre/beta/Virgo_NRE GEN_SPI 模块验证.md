
# 驱动熟悉
## Master
- spi->mode 用于控制单线、双线、四线，可以通过在初始化时配置，也可以通过 format 进行配置

## Slave
- Slave 模式时可以配置 sample_delay，时钟源自 master
- Apus GEN_SPI 模块频率为 32MHz，则在仅做接收时，Master 的最高工作频率只能在 5MHz 左右；接收+发送时，Master 的最高工作频率只能在 4MHz。


# 测试过程
## Master
- Virgo 做 Master，Apus 做 Slave
- Apus 中 gen_spi 默认的引脚复用：(master/slave 时用的是同一组脚)
```c
padmux_set(0, 76); //cs    p0_0
padmux_set(1, 77); //clk   p0_1
padmux_set(2, 78); //si    p0_2
padmux_set(3, 79); //so    p0_3
padmux_set(16, 80); //wp   p2_0
padmux_set(17, 81); //hold p2_1
```

### Master 发，Slave 收：
由于 GEN_SPI 对应的引脚是单向电平转换，通过拨码开关用于控制输出/输入。当 Master 需要发送时，需要手动将拨码开关拨到 `OUTPUT`。
Slave 先开始读数据，由于时钟都是源自 Master，此时 Master 没有启动，所以 Slave 会一直卡住；等待 Master 开始发时钟过来，Slave 就能够收到 Master 发出来的数据。
```
boot> spis_test read 1
00000000: aa                                                 .


boot> spim_test write aa 1
```

### Master 收，Slave 发：
由于 GEN_SPI 对应的引脚是单向电平转换，通过拨码开关用于控制输出/输入。当 Master 需要接收时，并且由于 *CS、CLK* 引脚都是由 Master 发出的，需要仍然保持 `OUTPUT`，*MOSI、MISO、WP、HOLD* 都作为数据脚，需要手动将拨码开关拨到 ` INPUT `。
Slave 先开始发数据，由于时钟都是源自 Master，此时 Master 没有启动，所以 Slave 会一直卡住；等待 Master 开始发时钟过来，Slave 就能够将数据发给 Master。
```
boot> spis_test write aabbccdd 4 1 8 0
00000000: aa bb cc dd                                        ....


boot> spim_test read 4
0xaa 0xbb 0xcc 0xdd
```



## Slave
### Master 发，Slave 收：
由于 Virgo 作为 Slave，并且此时是接收模式，所以需要将拨码开关全部拨成 INPUT。
Slave 先收，由 Master 发出 clk 来启动传输。
```
boot> spis_test read 2
0xaa 0x55

boot> spim_test write aa55 2 1 8 0 
```

### Master 收，Slave 发：
由于 Virgo 作为 Slave，并且此时是发送模式，因此需要将 *CS、CLK*拨码为*OUTPUT*，*MOSI、MISO、WP、HOLD*拨码为 *INPUT*。
```
boot> spis_test write aa55 2
0xaa 0x55

boot> spim_test read 2 1 8 0
00000000: aa 55                                              .U
```

## 测试用例

- master
	- 非 dma 单双四线
		- a55 ok
	- 同步 dma 单双四线
		- a55 ok
	- 异步 dma 单双四线
		- a55 不可测
- slave
	- 非 dma 单双四线
		- a55 ok
	- 同步 dma 单双四线
		- a55 ok


### 标准模式读写
- [x] 非 DMA 模式
- [ ] 同步 DMA 模式
- [ ] 异步 DMA 模式


### Dual 模式读写



### Quad 模式读写



### 片选 CS


### 中断
- `.config` 中打开 `ENABLE_GEN_SPI_TEST=y、ENABLE_IRQ=y`
- `gx_hal_spi.c` 中不要从 fifo 取数据，执行测试命令：`spim_test read 100`， 能够触发 rx fifo overflow 中断
```diff
diff --git a/drivers/spi/gen_spi/gx_hal_spi.c b/drivers/spi/gen_spi/gx_hal_spi.c
index 2c399a030..afa4f0416 100644
--- a/drivers/spi/gen_spi/gx_hal_spi.c
+++ b/drivers/spi/gen_spi/gx_hal_spi.c
@@ -367,11 +367,11 @@ static int gx_hal_spi_poll_recv_msg(struct gx_hal_dw_spi *dws, struct gx_hal_spi
                while(!(gx_hal_dw_readl(dws, GX_HAL_SPI_SR) & GX_HAL_SR_RF_NOT_EMPT));
 
                if (dws->n_bytes == 1) {
-                       ((uint8_t*)transfer->rx_buf)[i] = gx_hal_dw_readl(dws, GX_HAL_SPI_RXDR);
+                       //((uint8_t*)transfer->rx_buf)[i] = gx_hal_dw_readl(dws, GX_HAL_SPI_RXDR);
                } else if (dws->n_bytes == 2) {
-                       ((uint16_t*)transfer->rx_buf)[i] = gx_hal_dw_readl(dws, GX_HAL_SPI_RXDR);
+                       //((uint16_t*)transfer->rx_buf)[i] = gx_hal_dw_readl(dws, GX_HAL_SPI_RXDR);
                } else if (dws->n_bytes == 4) {
-                       ((uint32_t*)transfer->rx_buf)[i] = gx_hal_dw_readl(dws, GX_HAL_SPI_RXDR);
+                       //((uint32_t*)transfer->rx_buf)[i] = gx_hal_dw_readl(dws, GX_HAL_SPI_RXDR);
                }
        }


--- a/drivers/spi/gen_spi/gx_dw_spi.c
+++ b/drivers/spi/gen_spi/gx_dw_spi.c
@@ -1040,6 +1040,7 @@ static enum interrupt_return dw_spi_irq_handler(uint32_t interrupt, void *pdata)
 {
        struct spi_data *dws = pdata;
        struct gx_hal_spi_tran trans = {0};
+       printf("irq status:0x%x.\n", *(volatile unsigned int *)0xfc000030);
 #ifdef CONFIG_ENABLE_SPI_INT
        struct spi_transfer *transfer = NULL;


```


### DMA



# 问题

## 【已解决】问题 1：SPI 测试程序无法驱动 CS 引脚
- 经查阅发现是之前编的是 GEN_SPI1，这次编的是 GEN_SPI0
- 将初始化中的基地址和 CS 换成 GEN_SPI0 后可以了


## 【已解决】问题 2：测到四线的时候，数据不对
- clk 来自 master，所以 master 需要跑的很低的频率，并且也要考虑下 sample_delay
- 做 master、slave 的时候都需要关心 sample_delay

- Apus 发送的是 `0 1 2 3 4 5 6 7 8 9`
- Virgo 接收到的是 `0 1 2 3 8 9 a b 4 5`
- WP、HOLD 引脚接反了，接正确后测试通过


## 【已解决】问题 3：单线测试通过，到了双线之后测试失败
- 双线、四线模式的时候不会发 1 个字节的情况，但是 slave 发了一个字节，就很奇怪
- 追下去发现是命令写错了，应该是 `spis_test case [times] [if_debug]`
- 实际测试的时候敲成了 `spis_test case [bits_per_word] [times]`


## 问题 4：同步 dma 不通
- spi_read 操作：
	- 源完成中断有了，目标完成中断没有
	- 源是 fifo，说明握手信号是对的，目标是内存，这里可能有问题
```
DMA_CFGREG(fcc00010) is 3      使能 dma、使能中断
DMA_CHENREG(fcc00018) is 0
INTSTATUSREG(fcc00030) is 10000  common irq enable
INTSTATEREG(fcc00050) is 4       write on hold err
RESETREG(fcc00058) is 0
LOWPOWER_CFGREG(fcc00060) is 0
DMA_CHXSAR(fcc00100) is fc000060  src:spi0 dr
DMA_CHXDAR(fcc00108) is 3b1c0     dst:没有更新
DMA_CHXBLKTS(fcc00110) is 7       8 个数据
DMA_CHXCTL(fcc00118) is 104011    src：master 2； dst：master1
                                  noinc          inc
                                  
DMA_CHXCFG(fcc00120) is 2800200000120  src:18    dst:0
                                       per -> mem dmac
                                       hardware handshake
DMA_CHXLLP(fcc00128) is 0
DMA_CHXSTATUS(fcc00130) is 0
DMA_CHXSWHSSRC(fcc00138) is 0
DMA_CHXSWHSDST(fcc00140) is 0
DMA_CHXINTSTATUSEN(fcc00180) is ffffffffffffffff
DMA_CHXINTSTATUS(fcc00188) is 40040088   src complete

DMA_CHXINTSIGNALEN(fcc00190) is ffffffff07c08004
```
- 先用 a7 的 bit 试一下，也无法完成
- 用之前 final 测试时的 bit 可以完成  #394932  peri_final3_gpio_FPGA_2024-12-03.tar
	- 之前的 bit gen_spi 可以，uart 也可以
- 用现在的 bit gen_spi、uart 都不行

> [!note]
> - 测试 nre 时的代码修改已经 git stash，目前的代码修改是可用于 mpw、nre a7 的代码修改


### 需要提供寄存器级别的仿真代码
```
(gdb) x/20x 0xfcc00000
0xfcc00000:     0x00000000      0x00000000      0x3130322a      0x00000000
0xfcc00010:     0x00000003      0x00000000      0x00000000      0x00000000
0xfcc00020:     0x00000000      0x00000000      0x00000000      0x00000000
0xfcc00030:     0x00000000      0x00000000      Cannot access memory at address 0xfcc00038
(gdb) x/20x 0xfcc00000+0x180
0xfcc00180:     0xffffffff      0xffffffff      0x40000088      0x00000000
0xfcc00190:     0x07c08004      0xffffffff      Cannot access memory at address 0xfcc00198
(gdb) x/20x 0xfcc00100
0xfcc00100:     0xfc000060      0x00000000      0x0788d440      0x00000000
0xfcc00110:     0x00000007      0x00000000      0x00104011      0x00000000
0xfcc00120:     0x00000120      0x00028002      0x00000000      0x00000000
0xfcc00130:     0x00000000      0x00000000      0x00000000      0x00000000
0xfcc00140:     0x00000000      0x00000000      Cannot access memory at address 0xfcc00148
(gdb) x/20x 0xfcc00000
0xfcc00000:     0x00000000      0x00000000      0x3130322a      0x00000000
0xfcc00010:     0x00000003      0x00000000      0x00000000      0x00000000
0xfcc00020:     0x00000000      0x00000000      0x00000000      0x00000000
0xfcc00030:     0x00010000      0x00000000      Cannot access memory at address 0xfcc00038
(gdb) x/20x 0xfc000000
0xfc000000:     0x80800807      0x00000007      0x00000001      0x00000000
0xfc000010:     0x00000001      0x000000f0      0x00000000      0x00000007
0xfc000020:     0x00000000      0x00000008      0x0000001e      0x0000000e
0xfc000030:     0x00000000      0x00000011      0x00000000      0x00000000
0xfc000040:     0x00000000      0x00000000      0x00000000      0x00000001
(gdb) x/20x 0xfcc00000+0x180
0xfcc00180:     0xffffffff      0xffffffff      0x40040088      0x00000000
0xfcc00190:     0x07c08004      0xffffffff      Cannot access memory at address 0xfcc00198
(gdb)
```


- CH_EN 不同，之前的已经关了，现在还开着
- CFG2 不同，通道极性
- gen_spi 0x1c 一个是 7，一个是 6
- gen_spi fifo 没数据

- 若是 RX_ONLY 模式，并且无地址、无指令，需要先发 0x00 来启动传输



genspi 对接 axi-dma 水线相关配置：
- rx
	- DMARDLR 配成 **fifo 一半 - 1**，在目前为止 3
	- dma
		- 源、目标传输宽度：8
		- 源 msize：4，刚好是和 DMARDLR +1 相同，相当于每次 Burst 是 4 Byte
		- 目标 msize：32，对内存没关系，相当于每次是 32 Byte
	- 传输情况：
		- 假设传输 3 个字节，则会有 3 次 single
		- 假设传输 4 个字节，则会有 1 次 burst
- tx



## 问题 5：【已解决】NRE 使用 SPI0 时代码修改
```diff
diff --git a/drivers/spi/dw_spi/gx_dw_spi.c b/drivers/spi/dw_spi/gx_dw_spi.c
index 776e9d88..21dace9e 100644
--- a/drivers/spi/dw_spi/gx_dw_spi.c
+++ b/drivers/spi/dw_spi/gx_dw_spi.c
@@ -10,7 +10,7 @@
 #include <gx_hal_spi_ctrl.h>
 #include "interrupt.h"
 #if defined(CONFIG_ENABLE_AXI_DMA)
-#include <driver/gx_dma_axi.h>
+#include "axi_dma.h"
 #endif
 
 #define CONFIG_SPI_SLAVE_NUM_MAX    3
@@ -23,7 +23,7 @@
 #define DW_SPI_DMA_SYNC_MODE         1
 #define DW_SPI_DMA_ASYNC_MODE        2
 
-#define DW_SPI_TRANSFER_MODE DW_SPI_DMA_NOTUSE_MODE
+#define DW_SPI_TRANSFER_MODE DW_SPI_DMA_SYNC_MODE
 
 #define DW_SPI_TRANSFER_MAX_SIZE 0x10000
 
@@ -90,7 +90,7 @@ static int dw_spi_dma_tx_sync(struct gx_hal_dw_spi *hal_dws, int channel,
 	dma_config.dst_hs_select     = DWAXIDMAC_HS_SEL_HW;
 	dma_config.dst_master_select = AXI_MASTER_2;
 	dma_config.dst_msize         = DWAXIDMAC_BURST_TRANS_LEN_8;
-	dma_config.dst_per           = DMA_HS_PER_SPI2_DMA_TX;
+	dma_config.dst_per           = DMA_HS_PER_SPI1_DMA_TX;
 
 	dcache_flush();
 	dw_dma_channel_config(channel, &dma_config);
@@ -148,7 +148,7 @@ static int dw_spi_dma_rx_sync(struct gx_hal_dw_spi *hal_dws, int channel,
 	dma_config.src_hs_select     = DWAXIDMAC_HS_SEL_HW;
 	dma_config.src_master_select = AXI_MASTER_2;
 	dma_config.src_msize         = DWAXIDMAC_BURST_TRANS_LEN_4;
-	dma_config.src_per           = DMA_HS_PER_SPI2_DMA_RX;
+	dma_config.src_per           = DMA_HS_PER_SPI1_DMA_RX;
 
 	dma_config.dst_addr_update   = DWAXIDMAC_CH_CTL_L_INC;
 	dma_config.dst_hs_select     = DWAXIDMAC_HS_SEL_HW;
@@ -243,7 +243,7 @@ static int dw_spi_setup(struct spi_device *spi)
 	config.freq_src      = CONFIG_GEN_SPI1_CLK_SRC;
 	config.max_speed_hz  = spi->max_speed_hz;
 	config.is_master     = 1;
-	config.sample_delay  = 0;
+	config.sample_delay  = 1;
 	config.clk_mode      = GX_HAL_SPI_CLK_MODE0;
 	config.cs_init_func = gx_hal_spi_ctrl_cs_init;
 	config.cs_ctrl_func = gx_hal_spi_ctrl_cs_control;
@@ -1056,8 +1056,8 @@ void dw_spi_probe(void)
 	dw_spi_master_set_dws(master, &dws_spi_data);
 
 	dws = dw_spi_master_get_dws(master);
-	dws->base_regs = REG_BASE_GEN_SPI1;
-	dws->cs_reg = REG_BASE_GEN_SPI1_CS;
+	dws->base_regs = REG_BASE_GEN_SPI0;
+	dws->cs_reg = REG_BASE_GEN_SPI0_CS;
 	dws->master = master;
 
 	master->bus_num    = CONFIG_GENERAL_SPI_BUS_SN;
```

### 【已解决】问题 6：Virgo 做 Slave 时，对接 Apus 作为 Master，QUAD 模式数据比较不通过
- 单独测试 quad 可以
- 重新测试 ok，应该是拨码开关没有拨正确