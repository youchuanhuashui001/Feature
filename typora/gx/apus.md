# apus

1. 命令行无法正常使用
   1. 每次load 前都要按一下重置
2. make menuconfig中使用选择dw _rtc 报错
   1. 驱动程序有问题，要自己修改
3. debug的使用，是只能查看cpu内部的寄存器，不能查看外设的寄存器是吗
   1. 使用 x 命令 b0710008
4. 验证的话是要验什么
   1. 首先根据cmd来验证功能看能否符合要求
5. 想要一套代码来实现apus、grus等等开发板的程序，在scpu中分别实现各个芯片单独的部分，然后整体的部分根据宏来区分开
6. gpio的电平触发中断要由外部来触发，此时gpio的模式为输入模式
7. 管脚上拉、下拉，驱动强度、中断至其他核在芯片手册中未查看相关寄存器，特殊输出功能是指：pwm、clock、out、in等test吗？
   1. 上下拉、驱动强度等bit文件未提供，有的芯片支持就会有相应的寄存器
   2. 中断至其它核，是指一个gpio是否能够中断到多个核中去，且每个核的中断状态寄存器都是相同的，无论是否打开对应的中断源，其状态寄存器仍会实时显示当前的中断状态
      1. 通过注册中断时请求分配不同的中断号，来对应不同的核的中断使能
         ![image-20220304163411000](/home/tanxzh/txz/my/note/image/image-20220304163411000.png)
- rtc 无法 tick

```
rtc_init:

​    gx_clk_mod_set_gate(打开模块时钟) 

​    gx_clk_mod_get_freq(获取时钟频率,跟aquila相同的时钟)    

​        16000000 --> > 0xffff 无法设置rtc --> 手动修改成grus的时钟(0x1f40)

​            writel --> 将时钟写入寄存器

​    请求中断 

​        apus未提供中断号

​    gx_rtc_start_tick()        

​        使能rtc的tick计数 --> 查看寄存器的值为 0x14确实使能了


set_tick:

​    writel --> 0x08寄存器

​        查看寄存器值确实写入了


get_tick:

​    readl --> 0x00寄存器

​        获取到的值为0
```

- gpio set_io 17 0
  - 设置为输入时，寄存器0x10的值未置1
  - 输入输出模式寄存器中的值置0
    - 修改的寄存器不同，要仔细看驱动程序
- wdt init 50000
  - 设置看门狗的值为50000，实际的中断时间才为2s左右
    - 修改时钟
  - wdt ping：喂狗后不产生中断
  - 没有关闭看门狗的api
    - 自己写一个
  - 看门狗关不掉
    - 正常的，因为系统用了这个看门狗来防止有程序卡住

## spi flash

- 驱动位置：drivers/mtd/spinor/flash_spi.c

- 命令行测试程序：cmd/flash/sf.c

- make menuconfig：
  
  - Device Drivers/
    
    - Memory Technology Device(MTD) support/
      
      ![image-20220309110919027](/home/tanxzh/txz/my/note/image/image-20220309110919027.png)
  
  - Command Line Support/
    
    - Memory commands/
      - md, mm, nm, mw, cp, cmp, base, loop

- 使用spi0来当做flash专用

- flash 写之前要先擦除，擦除的最小单位为block，写的时候要有page的概念，读的时候可以任意读
  ![image-20220309124853171](/home/tanxzh/txz/my/note/image/image-20220309124853171.png)

- 主要有用的是：
  
  - sf：读写命令
  - flash_test：自动读写测试
  - 10：是16进制的10

```shell
# 测试命令
sf probe
mw.b 20027000 00 10
md.b 20027000 10
sf erase 0 10
sf read 20027000 0 10
md.b 20027000 10
mw 20027100 12345678 4
sf write 20027100 0 10
sf read 20027000 0 10
md.b 20027000 10
```

- gdb调试时，getinfo函数一定要s进去，否则后面的函数看不到
- 读写fifo的时候不需要地址递增 -- 芯片实现 first in first out
- 读写内存的时候需要地址递增
- xip:cpu通过地址总线直接访问flash上的数据
- spi 通过dma：spi和dma之间的采用硬件流控使用硬件握手信号，使用硬件的方式来决定dma要不要开始动作，dma和内存之间采用软件流控方式，由dma搬完指定数量的数据即可停止动作
- 流控：外设(硬件)流控的时候，在传输数据之前，需要传输数据的多少是未知的，此时，如果数据传输完成，需要外设发出信号，通知DMA数据数据以及发送完成；软件流控的时候，由于内存无法发出信号来通知DMA，只能由DMA自己搬运指定字节的数据，搬运完后停止
- DMA的初始化是通过链表的方式来初始化的，链表指针(LLP)指向系统内存中下一个链表项(LLI)存在的位置。LLI是一组寄存器，描述下一个块(块描述符)和一个LLP寄存器

```c
void gx_dma_init(void)     
{
       priv->dw_dmac     = (dw_dma_regs_t*)GX_REG_BASE_DMA;              /* 
               1. priv->dw_dmac 指向GX_REG_BASE_DMA基地址
               2. priv为一个结构体，所以地址也是连续的，即操作的是在GX_REG_BASE_DMA为基地址连续的一块地址
       */                                                                                                                             
       priv->chan_num    = DW_DMAC_MAX_CH;
       for (int i = 0; i<DW_DMAC_MAX_CH; i++) { 
       ▸       priv->lli[i] = (dw_lli_t *)roundup((unsigned int)priv->llp_buf[i], DCACHE_LINE_SIZE);
       ▸       priv->chan_used[i] = 0;
       }                  
#ifdef CONFIG_ARCH_GRUS    
       gx_clock_set_module_enable(CLOCK_MODULE_DMA, 1);
#endif
       dw_dmac_init();    
#ifdef CONFIG_ARCH_GRUS    
       gx_clock_set_module_enable(CLOCK_MODULE_DMA, 0);
#endif                     
       gx_request_irq(IRQ_NUM_DMA, (irq_handler_t)dw_dma_complete, NULL);
}
```

- XIP只能读，不能写、擦
- 使用xip的时候，把内存中的一片地址映射到存储器(flash、psram)中，像内存一样操作

```
probe:
--> do_gx_spi_flash_probe
  --> gx_spi_flash_probe
     --> flash_spi.init
     --> sflash_init(回调函数)
        --> sflsh_detect  
          -->sflash_readid 
            --> 匹配ID成功 
              --> 回调函数赋值(read_data、program_page) 
              g_flash_info.program_page = sflash_standard_1pageprogram;
              g_flash_info.read_data = sflash_dual_read;
                --> return &flash_spi_dev


把flash 0地址数据擦除，擦除16个字节(实际是最小的一个扇区)                
erase:
--> do_spi_flash_erase
  --> gx_spi_flash_erasedata
    --> gx_spi_flash_getinfo
    --> gx_spi_flash_erasedata
      --> sflash_sector_erase(回调函数)


把flash 0地址的数据读取到内存中20027000的位置，读16个字节
sf read 20027000 0 10:地址-->0x20027000 0-->偏移 10-->字节数(16进制)
--> do_spi_flash_read_write
  --> gx_spi_flash_getinfo
  --> gx_spi_flash_readdata
    --> devp->readdata
    --> sflash_readdata(回调函数)
      --> g_flash_info.read_data()
        --> probe函数时初始化的read函数：sflash_dual_read
         --> for (i = 0; i < len; i++) {
             while(!(readl(SPIM_SR) & SR_RFNOT_EMPT));
             ((uint8_t*)buf)[i] = readl(SPIM_RXDR_LE);
         }

把内存中20027000的数据写入到flash 0地址去，写16个字节
sf write 20027000 0 10:地址-->0x20027000 0-->偏移 10-->字节数(16进制)
--> do_spi_flash_read_write
  --> gx_spi_flash_getinfo
  --> gx_spi_flash_pageprogram
    --> devp->pageprogram
    --> sflash_pageprogram
      --> g_flash_info.program_page
        --> probe初始化时赋值的program_page函数：sflash_standard_1pageprogram
          --> for (i = 0; i < len; i++) {
                  while(!(readl(SPIM_SR) & SR_TF_NOT_FULL));
                  val = ((uint8_t*)buf)[i];
                  writel(val, SPIM_TXDR_LE);
               }
```

- 使用DMA：

drivers/dma/dw_dmac_ahb/dw_dmac_ahb.c:259

boot> sf probe
Flash JEDEC = 0xc86015, model = gd25lq16e.

boot> flash_test comtest
Flash JEDEC = 0xc86015, model = gd25lq16e.
flash type is nor.

sflash_test:addr=0x0, len = 1

erase addr = 0x0, erase len  = 0x1

![image-20220310111619144](/home/tanxzh/txz/my/note/image/image-20220310111619144.png)

![image-20220310111636566](/home/tanxzh/txz/my/note/image/image-20220310111636566.png)

dma des:0x20000004

## Q：

- gx_spi_flash_getinfo --> dev->getinfo 钩子函数
- dev init初始化所有的钩子函数
  - 钩子函数中又调用gx_spi_nor_get_info()函数，在全局中都没有找到#
    - 查看错了文件
- read、write地址是绝对地址，erase地址是相对地址
  - 地址是内存中的地址，偏移是flash中的地址
- 在erase之后要read一下才能生效
  - mw命令是写内存的命令，sf write命令是写flash的命令，只有通过sf read将flash中的数据读取到内存中，才可以通过md命令来查看
- 不支持四倍速
  - 这款flash没有打开四倍速的接口
    - 在sflash_enable_quad中打开即可；
- write 20003000 0 10：失败
  - tspend_handler
- 块传输大小、流控、接收中断

## PSRAM：打补丁

- Q：
  
  - SPI_CS_HIGH：操作的寄存器位置在mcu_config区域？ ---> drivers/mtd/spinor/flash_spi.c
    
    - 没有写在文档中，需要问soc/硬件
  
  - PSRAM需要单独的硬件吗？还是直接就已经在bit文件中做好了 ---> cmd/psram.c
    
    - 需要单独的硬件
  
  - psram xip的不需要probe，读和写都是直接读写吗？根据地址寄存器来进行操作
    
    - 是直接对地址进行读写的，通过xip的方式将内存地址中指定的一块区域映射到psram中去
  
  - psram read 和 md.b read数据不一样
    
    - 没有修改xip的基地址
    - 使用psram的时候对地址进行操作都是从基地址上进行的偏移
    
    ```
    boot> psram_xip read 20027000 10
    01319678: ff ff ff ff ff ff ff ff ff ff                      ..........
    ```
    
    boot> md.b 20027000 10
    20027000: 00 00 00 00 00 00 00 00 04 70 02 20 00 00 00 00    .........p. ....
    
    boot> md.b 18000000 10
    18000000: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
    
    ---> psram_xip 的起始地址可能是0x1800 0000
    
    boot> psram_xip read 20027000 20
    01319678: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
    01319688: ff ff ff ff
    
    boot> md.b 18000000 20          
    18000000: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
    18000010: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
    
    boot> psram_xip write 0 4 12345678    ---> 是相当于在0x1800 0000这个内存地址写入了12345678 这样的数据吗？
    write value:
    00000000: 12 34 56 78                                        .4Vx
    Write done, start at 0, length 4
    
    boot> psram_xip read 0 10
    00000000: 12 34 56 78 78 56 34 12 78 56                      .4VxxV4.xV
    boot> md.b 18000000 10
    18000000: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
    
    ```
    
    ```

- psram 的读写函数是在哪里初始化的
  
  - drivers/spi/dw_spi/dw_spim.c 中使用另一个master变量来初始化的

- todo:
  
  - 修改宏定义的地址，不能直接在原来的地址上修改，要新添加一个地址
  - 使用xip方式的时候，cs引脚的拉高拉低由spi硬件来控制自动完成，在初始化时将cs引脚拉低，一直有效

| 3.3v  | w25q64                                                           |
|:-----:|:----------------------------------------------------------------:|
| 两倍速   | 正常                                                               |
| xip   | 正常                                                               |
| 四倍速   | 正常                                                               |
| 单倍速   | 读写错误                                                             |
| 32bit | 大小端问题                                                            |
| dma   | 卡在等待while((priv->dw_dmac->RAW.XFER[0] & (0x01 << channel)) == 0) |

单倍速问题：

```
boot> sf probe
Flash JEDEC = 0xef4017, model = w25q64.
boot> sf erase 0 10
SF: 16 bytes @ 0x0 Erased: OK
boot> md.b 20027000 10  
20027000: 00 00 00 00 00 00 00 00 04 70 02 20 00 00 00 00    .........p. ....
boot> mw 20027000 12345678 4
boot> sf write 20027000 0 10
SF: 16 bytes @ 0x0 Written: OK
boot> mw 20027000 00 4
boot> md.b 20027000 10
20027000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
boot> sf read 20027000 0 10
SF: 16 bytes @ 0x0 Read: OK
boot> md.b 20027000 10  
20027000: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
```

32bit问题：

```
boot> sf probe                                                    
Flash JEDEC = 0xef4017, model = w25q64. ff ff ff ff ff ff    .....
boot> mw 20027100 12345678 4                                      
boot> sf erase 0 10
SF: 16 bytes @ 0x0 Erased: OK
boot> md.b 20027100 10
20027100: 78 56 34 12 78 56 34 12 78 56 34 12 78 56 34 12    xV4.xV4.xV4.xV4.
boot> sf write 20027100 0 10
SF: 16 bytes @ 0x0 Written: OK
boot> mw 20027000 0 10     
boot> md.b 20027000 10
20027000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
boot> sf read 20027000 0 10
SF: 16 bytes @ 0x0 Read: OK
boot> md.b 20027000
20027000: 12 34 56 78 12 34 56 78 12 34 56 78 12 34 56 78    .4Vx.4Vx.4Vx.4Vx
```

bit：apus_top_FPGA_2022-03-18_0.bit 

flash：w25q64

xip:0x13000000

| DMA | 数据宽度(bit) | spi模式    | 结果    |
| --- | --------- | -------- | ----- |
| 关闭  | 8         | 标准       | 读写问题1 |
| 关闭  | 8         | dual     | 正常1   |
| 关闭  | 8         | quad     | 正常1   |
| 关闭  | 32        | 标准       | 读写问题1 |
| 关闭  | 32        | dual     | 正常1   |
| 关闭  | 32        | quad     | 正常1   |
| 关闭  | 8         | 标准 xip   | 读写问题  |
| 关闭  | 8         | dual xip | 正常1   |
| 关闭  | 8         | quad xip | 正常0   |
| 打开  | 8         | dual     | 正常1   |
| 打开  | 8         | quad     | 正常1   |
| 打开  | 32        | dual     | 正常1   |
| 打开  | 32        | quad     | 正常1   |

```shell
boot> sf probe
Flash JEDEC = 0xef4017, model = w25q64.
boot> mw 20027000 12345678 4
boot> md.b 20027000 10
20027000: 78 56 34 12 78 56 34 12 78 56 34 12 78 56 34 12    xV4.xV4.xV4.xV4.
boot> md.b 20027100 10
20027100: f4 70 02 20 00 00 00 00 00 00 00 00 08 71 02 20    .p. .........q. 
boot> sf erase 0 10
SF: 16 bytes @ 0x0 Erased: OK
boot> sf write 20027000 0 10
SF: 16 bytes @ 0x0 Written: OK
boot> sf read 20027100 0 10
SF: 16 bytes @ 0x0 Read: OK
boot> md.b 20027100 10
20027100: 78 56 34 12 78 56 34 12 78 56 34 12 78 56 34 12    xV4.xV4.xV4.xV4.
```

bug：四倍速 md.b 13000000 10卡住

- 在不开启xip功能时，这个地址是一个不存在的地址，就算存在也是脏数据，不能查看无关紧要

```
boot> sf probe
Flash JEDEC = 0xef4017, model = w25q64.
boot> md.b 13000000 10
13000000:
```

- 常用的寄存器：
  
  ```
  DMACR：DMA Control Register
      bit1：Transmit DMA Enable
      bit0：Received DMA Enable
  
  SER：Slave Enable Register
      x:0：Slave Select Enable Flag
  
  ENR：SSI Enable Register
      0：SSI Enable
      when disable, all serial transfers are halted immediately.
      Transmit and receive FIFO buffers are cleared.
  
  CTRLR0:Control Register 0
      23:22：SPI_FRF:SPI Frame Format
              Standard SPI Format.
              Dual SPI Format.
              Quad SPI Format.
              Octal SPI Format.
      19:16:Control Frame Size.
      11:10:Transfer Mode
              Transfer Only.
              Receive Only.
              Transfer and Receive.
              EEPROM Read Mode.
      9:SCPOL:Serial Clock Polarity.
      8:SCPH:Serial Clock Phase.
      4:0:Data Frame Size.0x7(8bit)
  
  CTRLR1:Control Register 1
      15:0：Number of Data Frames.
      When TMOD = 10 or TMOD = 11,this register field sets the number of data frames to be continuously received by the DWC_ssi.The DWC_ssi continues to receive serial data until the number of data frames received is equal to this register value plus 1.
  
  DMATDLR:DMA Transmit Data Level
      x:0:Transmit Data Level
  
  DMARDLR:DMA Receive Data Level
      x:0:Receive Data Level
  
  DMACR:DMA Control Register
      1:Transmit DMA enable
      0:Receive DMA enable
  
  DR:Data Register.
  ```

spi flash的读模式EEPROM:

`http://blog.chinaunix.net/uid-29913528-id-4970704.html`

eeprom read:standard read 只是一个模式位配置不同

32bit也是一样的，添加一下就好了

```c
1030 ▸       writel(0x00, SPIM_ENR);
1031 ▸       writel(0x00, SPIM_DMACR);                                                                
1032 //▸     writel((SPI_FRF2_STANDARD << SPI_FRF2_OFFSET) | (SPI_TMOD_EPROMREAD << SPI_TMOD_OFFSET) | SPI_DFS_32BIT, SPIM_CTRLR0);
1033 ▸       writel(EEPROM_8BITS_MODE, SPIM_CTRLR0);                                                  
1034 ▸       writel(len - 1, SPIM_CTRLR1);                                                            
1035 ▸       writel(0x01, SPIM_SER);                                                                  
1036 ▸       writel(0 << 16, SPIM_TXFTLR);                                                            
1037 ▸       writel(0x00, SPIM_SPI_CTRL0);                                                            
1038 ▸       writel(0x01, SPIM_ENR);
1039 
1040 ▸       writel(0x0b, SPIM_TXDR_LE);                                                                                                                                                                    
1041 
1042 ▸       writel(from>>16, SPIM_TXDR_LE);                                                          
1043 ▸       writel(from>>8, SPIM_TXDR_LE);                                                           
1044 ▸       writel(from, SPIM_TXDR_LE); 
```

cmd:默认都存在

addr：address length

dummy：长度 --> wait_cycle

`writel(len -1, SPIM_CTRLR1);`：要读的字节数

<mark>psram_xip：测试</mark>

```c
boot> psram_xip read 0 16
00000000: 00 00 40 07 00 00 00 00 00 00 00 00 00 00 00 00    ..@.............
boot> md.b 22000000 10
22000000: 00 00 40 07 00 00 00 00 00 00 00 00 00 00 00 00    ..@.............
boot> psram_xip write 0 4 12345678
write value:
00000000: 12 34 56 78                                        .4Vx
Write done, start at 0, length 4
boot> psram_xip read 0 16
00000000: 12 34 56 78 00 00 00 00 00 00 00 00 00 00 00 00    .4Vx............
boot> md.b 22000000 10
22000000: 12 34 56 78 00 00 00 00 00 00 00 00 00 00 00 00    .4Vx............
```

24.bit

w25q64

| DMA | 数据宽度(bit) | spi模式    | 结果  |
| --- | --------- | -------- | --- |
| 关闭  | 8         | dual     | 正常  |
| 关闭  | 8         | quad     | 正常  |
| 关闭  | 32        | dual     | 正常  |
| 关闭  | 32        | quad     | 正常  |
| 关闭  | 8         | dual xip | 正常  |
| 关闭  | 8         | quad xip | 正常  |
| 打开  | 8         | dual     | 正常  |
| 打开  | 8         | quad     | 正常  |
| 打开  | 32        | dual xip | 正常  |
| 打开  | 32        | quad xip | 正常  |

```shell
boot> sf probe
Flash JEDEC = 0xef4017, model = w25q64.
boot> md.b 0x13000000 10
13000000: 78 56 34 12 78 56 34 12 78 56 34 12 78 56 34 12    xV4.xV4.xV4.xV4.
boot> sf erase 0 10
SF: 16 bytes @ 0x0 Erased: OK
boot> md.b 0x13000000 10
13000000: 78 56 34 12 78 56 34 12 78 56 34 12 78 56 34 12    xV4.xV4.xV4.xV4.
boot> 
```

- 使用xip模式的时候，写擦需要关掉dcache、icache
  
  - 由于cpu读取cache中的数据，然后通过sf修改了flash中的数据，但cache中存储的还是上次的数据，需要刷一下cache

- EEPROM READ模式：
  
  - 在发送的时候，发送命令和地址，此时receive FIFO不接收数据，直到transfer FIFO为空
  
  - 在接收的时候，接受CTRL1寄存器规定的length数据(在此阶段，硬件会自己发送dummy)
  
  - 这种方式不严谨，当transfer FIFO在传输数据时发生了中断或者延时，还有数据要发送，但是transfer FIFO已经为空，可能会出现问题
  
  - 在发送数据的时候可能速度太快了，此时加的dummy只是为了延时，等待数据准备好

- 开启icache,dcache、IBUS
  
  ```shell
  boot> psram_xip read 0 16
  00000000: 00 00 40 07 00 00 00 00 00 00 00 00 00 00 00 00    ..@.............
  boot> md.b 12000000 10
  12000000: 00 00 40 07 00 00 00 00 00 00 00 00 00 00 00 00    ..@.............
  boot> psram_xip write 0 4 11223344
  write value:
  00000000: 11 22 33 44                                        ."3D
  Write done, start at 0, length 4
  boot> psram_xip read 0 16
  00000000: 11 22 33 44 00 00 00 00 00 00 00 00 00 00 00 00    ."3D............
  boot> psram_xip read 0 4 
  00000000: 11 22 33 44                                        ."3D
  boot> psram_xip read 0 16
  00000000: 11 22 33 44 00 00 00 00 00 00 00 00 00 00 00 00    ."3D............
  boot> md.b 12000000 10
  12000000: 11 22 33 44 00 00 00 00 00 00 00 00 00 00 00 00    ."3D............
  ```

- 理解：无论是否初始化，都将psram_xip的区域映射

DMA：

Device Drivers --> DMA support--> DesignWare ahb dma support

Application --> Command Line Support --> Device access commands --> ahb_dma

```shell
Usage:
tdma_config tr_width src_msize dst_msize src_inc dst_inc src_ahb dst_ahb
tr_width 0:8bit  1:16bit  2:32bit  3:64bit
src_msize/dst_msize  0:1 item  1:4 items   2:8 items   3: 16 items .... 7:256 items
src_inc/dst_inc 0:addr inc   1:addr no inc
src_ahb/dst_ahb 0:ahb master 1   1: ahb master 2   2: ahb master 3  3: ahb master 4
eg: tdma_config 0 4 4 0 0 0 0
    tdma_config 3 4 4 1 0 0 0
boot> tdma_config 0 2 2 0 0 2 2 
boot> tdma_poll 
tdma_poll - ahb dma poll command

Usage:
tdma_poll src_addr dest_addr len(hex) channel printdata
eg: tdma_poll 2002e000 2002e800 100 0 1
    tdma_poll 2002e000 2002e800 100 1 1

boot> tdma_poll 0x13000000 0x20027000 100 0 1
src addr : 0x13000000, dest addr : 0x20027000, len : 256 B
```

psram:修改地址-->使用spi1的基地址

- psram xip:CTRLR0-->SPI_FRF 不能写0
1. Set SPI frame format value in -->CTRLR0

2. Set Address length, Wait cyclrs, transaction type --> SPI_CTRLR0
   
   使用xip inst en寄存器的SPI_CTRLR0字段，指令阶段也可以包含在XIP传输中，此时必须设置：
   
   1. SPI_CTRLR0 中设置 指令长度
   
   2. 将opcode写入XIP_INSR_INST 和 XIP_WRAP_INST寄存器
      
      - 通过ahb接口发起一个读操作，使用编程的值传输到spi外设

```
CTRLR0
XIP_CTRL
SPI_CTRLR0
XIP_WRAP_INST
XIP_INCR_INST
XIP_MODE_BITS
XIP_WRITE_INCR_INST
XIP_WRITE_WRAP_INST
```

```shell
10000140: 97 c1 00 10 93 81 81 95 17 25 00 00 13 05 85 0b    .........%......
10000150: 13 65 35 00 73 10 55 30 17 05 00 00 13 05 85 ea    .e5.s.U0........
10000160: 73 10 75 30 37 21 00 20 13 01 c1 ff 73 10 01 34    s.u07!. ....s..4
10000170: ef 00 80 01 ef 10 d0 7c 01 a0 01 00 01 a0 01 00    .......|........
10000180: 01 a0 01 00 82 80 00 00 41 11 06 c6 31 20 a5 20    ........A...1 . 
10000190: 95 20 b2 40 41 01 fd b7 b7 06 93 b0 9c 56 05 46    . .@A........V.F


20027000: 10 93 81 81 95 17 25 00 00 13 05 85 0b 13 65 35    ......%.......e5
20027010: 00 73 10 55 30 17 05 00 00 13 05 85 ea 73 10 75    .s.U0........s.u
20027020: 30 37 21 00 20 13 01 c1 ff 73 10 01 34 ef 00 80    07!. ....s..4...
20027030: 01 ef 10 d0 7c 01 a0 01 00 01 a0 01 00 01 a0 01    ....|...........
20027040: 00 82 80 00 00 41 11 06 c6 31 20 a5 20 95 20 b2    .....A...1 . . .
20027050: 40 41 01 fd b7 b7 06 93 b0 9c 56 05 46 ff ff ff    @A........V.F...
```
