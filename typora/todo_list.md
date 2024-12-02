# Todo_List

- [ ] lepus
  - [ ] fmc
    - [ ] xip
    - [ ] spi nand
  - [ ] spi
    - [ ] dma



- [x] slt慢的补丁需要同步修改 `#if define cygnus` 加上 canopus、vega，vega需要同步修改vega_copy.c
  - [x] 不需要加 canopus、vega，这两款没有用上文件，都是直接赋值



- [ ] MX25L6406E添加otp信息到develop分支的gxmisc



Lepus Uboot代码整理

- [ ] `drivers/mtd/spinand/spinand.c`中为了编译通过放置的几个函数，后续IP验证完后需要处理，添加自己的SPINAND驱动
- [ ] 研究SPL阶段使用的nor和nand驱动，考虑精简处理



打开 flash 测试命令：

- `configs/lepus_fpga_defconfig`中打开CONFIG_CMD_GXFLASH宏

编译motor驱动：

- make defconfig 打开`Device Drivers → SPI Motor Support -> GX_SPI_MOTOR`
- `configs/lepus_fpga_defconfig`中打开`CONFIG_MISC、CONFIG_GX_SPI_MOTOR` 宏

打开motor测试命令：

- `configs/lepus_fpga_defconfig`中打开`CONFIG_CMD_GXMOTOR`宏



- [x] 测试的时候要先测motor，再测flash

  - 由于在注册的时候会先执行drv->parent的probe再执行dev的probe，但是dev的probe还没执行，drv->probe就执行了，就会在spi驱动中卡在找设备

  - uboot的执行顺序如下：先执行设备的父节点的probe，再执行设备的probe。
    - 当先执行motor的测试命令时，执行顺序如下，spi->probe、flash->probe、spi->probe、motor->probe。
    - 当先执行flash的测试命令时，执行顺序如下，spi->probe、flash->probe、flash->probe、spi->probe、motor->probe。
      - 这种情况下会卡在第二次的spi->probe，相当于此时有两个sflash设备，两个spi，就会在链表中匹配bus_num，但是由于sflash设备的bus_num都是同一个，所以一直匹配不上，就会循环卡在里面
  - [x] 在flash的驱动中只调用一次gxmisc




- [x] 需要确认 `linux` 的头文件修改的原因，尽量保持不动
  - [x] 需要修改使用方法，在使用min函数的时候，参数是变量的强转成int类型




- [x] 精简git 提交信息
- [x] 将motor的类修改成一个SPI通用的类

- [x] 默认不开misc和motor的驱动，并且在kconfig中做一个依赖，motor依赖misc



- [ ] dma 水线描述：`https://www.cxymm.net/article/yangguoyu8023/123093508`
  - [ ] `https://blog.csdn.net/yangguoyu8023/article/details/123093508`

- [ ] `include/configs/lepus_fpga.h`文件中描述寄存器基址与 DMA-MASTER 的对照关系，dma 驱动中根据描述信息来在驱动中做区分



dma的水线要和spi的水线设成一样

dma的通道Burst Size，最好配成通道fifo的一半

spi驱动里面要先配好spi，再用dma传输

读的时候：先开dma，再开spi

- 如果先开spi的话，可能spi读了很多数据，但是dma还没来得及搬，导致数据丢失

写的时候：先开spi，再开dma

- 开启spi读取数据之后，然后开启dma来搬运数据

- 确认spi的fifo中是否有数据





# 修改 stage1 flash 驱动，保留 spl_load_image.c

- [x] 保留文件，伪造一个结构出来



# 确认 Rom flash 驱动

- [x] 已确认，无问题，可以修改一下sample_dly

# dma 验证添加一个 写 02 的操作，看数据是否正确

- [x] 不是写02，是要把SPI_COTRLR0寄存器清0，否则把dummy 当成 指令发出去了

# SPI Nand 测试

- [x] 记录三种 test case的操作流程，并打印数据区的数据和oob区域的ecc





# Lepus DMA 驱动

- [x] 去掉 lepus_fpga_defconfig 中 DMA 相关的默认选项
- [x] 去掉 dts 中配置的 SPI0
- [x] DMA 的 通道配置从  `include/dw_dmac.h` 移到 `board 相关配置中`

- [x] dma_xfer 参数需要添加描述



# 联合存储的 Flash 需要在 Cygnus 上测一下四倍速

- [x] 之前在sirius板子上测的，没有测到四倍速
- [x] 6705 板子不支持四倍速



# Canopus T 结尾的SPI NAND 不支持 ROM 启动

- [x] 要确认一下T结尾和非T结尾的区别



# Gxmisc 去掉 IT 结尾的 SPI NAND

- [ ] 由于此款默认 BUF = 0，rom 无法正常读取
- [ ] 0x13 发送 page_id ，来读取一个page 的数据到cache
- [ ] 0x03 发送列地址(page内地址)，来读取page内的数据
- [ ] stage1和stage2去掉BUF=1，看波形是否和ROM读取的一致，读到的数据是否一致(后面会错掉的现象)
  - [ ] Stage2去掉BUF=1，读取到的数据正确，ROM读取到的数据第二个page的最后56个字节错误



# 分析从 mtd_unitls 工具如何调用到下层的 Flash 驱动

- [ ] 





# Canopus 兼容性测试

- [ ] 测试 4k Page 的 SPI NAND
- [ ] 验证 华邦 T 结尾的 SPI NAND



# worn_test 测试程序

- [ ] 传入参数需要保证是 block 的首地址



# nand 工具整理

- [x] 去掉 libmtd_legacy.c
  - [x] 不能去掉这个文件，最新的 mtd_utils 工具目录下也有这个文件
- [x] 删掉原来的目录
- [x] 跟何奇的补丁同步成一个



# nandecc 测试

- 测试1：写ecc测试
  - 生成随机的main数据和全0xff 的oob数据
  - 找到一个随机的block和随机的page
  - 擦除 mtd 设备
  - 把随机的数据和oob数据写入
  - 读出oob数据
  - 将main数据做bit翻转，将翻转后的main数据和全0xff的oob数据写入
  - 读oob数据
  - 比较翻转后的oob数据和翻转前的oob数据是否相同
  - 读取main数据和oob数据 --- 是否在此操作的时候造成的 read ecc error，page id
  - 比较是否将翻转纠正回，此处未纠正回
- 测试2：擦除ecc测试
  - 擦除mtd设备
  - 生成全0xff的main数据和oob数据
  - 把main数据和oob数据写入
  - 读出oob数据
  - 将main数据做翻转，将翻转后的main数据和全0xff的oob数据写入
  - 读oob数据
  - 比较翻转后的oob数据和翻转前的oob数据是否相同
  - 读取main数据和oob数据
  - 比较是否将翻转位纠回



- nand 驱动写操作有两种：
  - 一种是发命令，main数据和ecc数据分开写
  - 一种是一体的，是否开启ecc



# nand 驱动



- `struct gx_mtd_info *mtd`：nand_base.c(nand_scan_tail)

  ```c
  	/* Fill in remaining MTD driver data */
  	mtd->type = MTD_NANDFLASH;
  	mtd->flags = MTD_CAP_NANDFLASH;
  	mtd->erase = nand_erase;
  	mtd->read = nand_read;
  	mtd->write = nand_write;
  	mtd->read_oob = nand_read_oob;
  	mtd->write_oob = nand_write_oob;
  	mtd->sync = nand_sync;
  	mtd->lock = NULL;
  	mtd->unlock = NULL;
  	mtd->block_isbad = nand_block_isbad;
  	mtd->block_markbad = nand_block_markbad;
  
  	/* propagate ecc.layout to mtd_info */
  	mtd->ecclayout = chip->ecc.layout;
  ```

- `struct gx_nand_chip *chip`：gx_nfc_v1.c(gxnfc_init_chip_v1)

  ```c
  int gxnfc_init_chip_v1(struct gx_nand_dev *info)
  {
  	struct gx_nand_chip *chip  = &info->chip;
  
  	chip->select_chip   = NULL;
  
  	chip->write_buf     = nandflash_write_buf;
  	chip->read_buf      = nandflash_read_buf;
  	chip->chip_delay    = 0;
  	chip->priv          = info;
  	chip->options       = 0;
  	chip->IO_ADDR_W     = NULL;
  	chip->IO_ADDR_R     = NULL;
  	chip->cmd_ctrl      = gxnfc_nand_hwcontrol;
  	chip->dev_ready     = gxnfc_nand_dev_ready;
  	chip->cmdfunc       = gxnfc_nand_command;
  	chip->waitfunc      = gxnfc_nand_wait_ready; /* NOTE: only used for write operations */
  	chip->read_byte     = gxnfc_nand_read_byte;
  	chip->read_data     = gxnfc_nand_read_data;
  	chip->write_data    = gxnfc_nand_write_data;
  
  	info->mtd.priv      = info;
  	chip->ecc.calculate = gxnfc_nand_calculate_ecc;
  	chip->ecc.correct   = NULL;
  	chip->ecc.mode      = NAND_ECC_HW;
  	chip->ecc.size      = GX_NAND_512_PAGESIZE;
  	chip->ecc.hwctl     = gxnfc_nand_enable_hwecc;
  	chip->ecc.read_oob  = gxnfc_nand_read_oob;
  	chip->ecc.config    = gxnfc_nand_ecc_config_v1;
  
  	gxnfc_nand_inithw(info);
  	return 0;
  }
  ```

  

- 读接口：默认打开ecc的话，读接口只会读取数据不会读取ecc回来，但是还是调用了`nandflash_ecc8`这个函数，使用来干啥的？
  - 用来配置地址布局的，用于将计算好的ecc数据和main数据按照地址布局写入到flash

- 读 oob 接口：通过发波形的方式来读的，chip->read_buf
- 写接口：写的时候打开了ecc，ecc的数据是由硬件计算并且写入到oob
- 读的时候，会先读512字节，然后读校验码，这个操作是由硬件做的

- flash 如何知道写的是数据还是ecc
  - 配置好ecc模块的地址分布，控制器会自己算出ecc码，并按照ecc的排布将main数据和ecc数据写入



- 驱动操作有两种方式：
  - 手动发命令、地址：chip->read_buf
  - 批量读取或写入，按照要求配置好，填入地址和要读写的长度，控制器自动完成读写：chip->read_data





# 梳理 gxmisc 和 支持列表不匹配的 Flash

- [x] 其中 NM25Q64EVN 需要验一下，驱动中不支持
  - [x] 实际测试不支持此 flash，由于 NM25Q64EVN 掉电测试不通过，gxmisc 去掉支持



# lepus dma 新增释放通道

- [x] 初始化时指定一个chan_used数组，并全部初始化为0
- [x] request channel的时候按顺序从chan_used中分配一个
- [x] release_channel 的时候把 chan_used 配为0
- [x] wait_complete 的时候不用关闭 dma，直接调用 release_channel

- [x] 是不是 program 的时候 norelease_channel 搞的鬼



补丁前：

- [ ] 测试项1：SPI0 使用 DMA 执行读写擦正常
- [ ] 测试项2：SPI0 开启中断执行读写擦正常
- [ ] 测试项3：SPI0 使用 DMA，且开启 DMA 中断，执行读写擦正常
  - [ ] dw_dmac_init(void)函数中配置 dw_dmac->MASK = 0xffff，否则会卡在 flash singletest 0x0 4096
- [ ] 测试项4： SPI0 开启中断，DMA 开启中断，执行读写擦正常





补丁后：

- [x] 测试项1：SPI0 使用 DMA 执行读写擦正常
- [x] 测试项2：SPI0 开启中断执行读写擦正常
- [x] 测试项3：SPI0 使用 DMA，且开启 DMA 中断，执行读写擦正常
  - [x] wait_dma_complete(int ch) 函数中把 dw_release_chan(ch) 放在宏定义外，否则会一直拿着通道不释放
  - [x] dw_dmac_init(void)函数中配置 dw_dmac->MASK = 0x0101，否则会卡在 flash singletest 0x0 4096
  - [x] read 使用 dma 的时候长度需要配成 0x1000 -4 ，默认配的是 0x10000
- [x] 测试项4： SPI0 开启中断，DMA 开启中断，执行读写擦正常



- [ ] 确认 MASK 寄存器是怎么影响的
- [ ] 确认是不是链表组织的有问题
  - [ ] SPI 的驱动中，读取数据的时候长度是 0x100000 ，应该配成 0x1000 - 4




- [ ] `include/configs/lepus_fpga.h` 中 `DW_DMAC_MAX_CH`的通道要修改为 4，这个位置修改成4之后会一直 receive fifo overflow，由于结构体定义的时候是按照偏移来的，如果把结构体中的数组修改成4的话，后面的成员偏移就错了，就不是指向正确的寄存器地址了



ai:默认不加otp，只支持1、2、4M

机顶盒只加4M以上的











# canopus 双核 ecos

- 整理 ecos_shell 容易复现的代码
- [x] 整理 ecos_shell 提交：可以拉下来跟本地的比较
- [x] ecos_shell 、solution 都要测
- [x] csky 也要测一颗芯片
- 整理 flash 手册中的电平时间，贴到存储技术 wiki





- [x] ecos3.0 双核问题提交的代码加一下注释



# nand driver

- pageprogram 接口：先把不满 page 的数据写，然后再写每个 page



- [ ] 测试 ecos_shell 已出错,400多次 read_data:0x 7, write_data：0x 4

  - [ ] 复现看看是否还能出错，

  - >>>>>sflash_test:addr=0x33fc00, len = 661568
    >>>>>sflash_gx_readl finish.
    >>>>>sflash erase finish.
    >>>>>read addr=0x33fc00, len=661568
    >>>>>sflash_gx_readl finish.
    >>>>>erase error, i = 1024

  - [ ] 

    - [ ] 回退 cache 补丁
    - [ ] 修改接口 `gx_dcache_clean_range` 换成 `HAL_DCACHE_FLUSH`，`gx_dcache_inv_range` 换成 `HAL_DCACHE_INVALIDATE`
    - [ ] 继续复现

- [ ] loader 默认不开 DMA
- [ ] 刷 cache 用：`HAL_DCACHE_STORE`
- [ ] 无效 cache 用：`HAL_DCACHE_INVALIDATE`， arm 和 csky 会有区别，用宏来区分开
- [ ] dma cache 操作：读接口，先 clean，dma 搬完之后再 invalidate；写接口，先 clean 再 dma
  - 读的时候：
    - 在前面 invalidate，可能会在 dma 搬运之前，cpu 访问这块地址，而cache中没数据，将数据预取到 cache 中，dma 完成搬运后，cpu 认为 cache 中的数据是新的，然后将内存中的数据读到 cache，导致那部分预取的数据把真实的数据覆盖了
    - 在后面 invalidate，另一个核可能在访问buffer周边的数据，发生投机读取，将数据读取到了cache，后面进行invalidate的时候就把那些数据无效了
    - 在前面 clean，在后面 invalidate，先把数据从 cache 刷到内存，然后进行 dma 操作，dma操作完后 invalidate
  - 写的时候：
    - 在前面 clean，把数据从 cache 刷到内存，然后 dma 搬

- [ ] develop 分支 ecos 中要加中断，用信号量做同步
  - [ ] 等待 dma 的状态换成等信号量，可以释放 cpu
  - [ ] 打开 dma 完成中断，然后在中断里置位信号量







- 另一个核访问 dma 操作的周边数据时，会投机访问 dma 操作的数据





















# eCos 添加 SPI DMA 的功能时会遇到文件系统解压失败死机问题

复现步骤：rc1 版本

1. gxbus 打上补丁 103514
2. gxtest/gxbus `./build config -n 2785`
3. `./make.sh gxtest-bus arm canopus 6631SH1D ecos dvbs bin` ，输出文件在 gxtest/gxbus/output 的 downloader_canopus.bin
4. 插上 u 盘，什么都不操作





# 添加 GRUS OTP 操作

- [ ] 单倍速读取会存在 `receive fifo overflow` ，目前使用的是 `dma` 读数据
  - 采用分段读，每次读 fifo 大小
- [ ] 其余类型的 `flash` 去掉支持 `otp`
- [ ] 每次读 fifo 大小，这个 fifo 大小要么使用探测的方式，要么用宏来区分开 `leo_mini`  和 `grus`







# 专利编写



# 白盒测试用例完善

在机顶盒芯片的白盒测试阶段，对Flash驱动进行以下测试是常见的做法：

1. 功能测试：验证Flash驱动的基本功能是否正常工作，包括读取、写入、擦除等操作。测试用例应涵盖各种常见的操作场景和边界情况，以确保驱动在各种条件下的稳定性和正确性。
2. 性能测试：测试Flash驱动的性能指标，如读取速度、写入速度和擦除速度。通过在不同数据量、数据类型和访问模式下进行测试，评估驱动的性能表现，确保其能够满足产品需求和性能要求。
3. 兼容性测试：验证Flash驱动在与不同型号、不同规格的Flash芯片之间的兼容性。测试应覆盖不同供应商和不同容量的Flash芯片，并验证驱动在各种组合下的正常工作。
4. 异常情况测试：模拟各种异常情况，如掉电、电压波动、信号干扰等，测试Flash驱动的容错能力和稳定性。确保驱动在异常情况下能够正确处理并恢复，以避免数据丢失或系统崩溃。
5. 安全性测试：针对Flash驱动的安全性进行测试，验证其对非授权访问和恶意操作的防护能力。测试包括数据加密、写保护、访问控制等安全功能的正确性和可靠性。
6. 异步操作测试：验证Flash驱动在多任务或多线程环境下的并发访问能力。通过模拟并发读写操作，测试驱动的并发性和资源管理能力，确保不会出现数据冲突或竞争条件。
7. 可靠性测试：进行长时间运行测试，模拟连续读写操作和擦除循环，以验证Flash驱动的可靠性和耐久性。测试应覆盖足够长的时间和大量的操作，以确保驱动在长期使用中不会出现问题。

以上测试旨在验证Flash驱动的功能性、性能、兼容性、安全性和可靠性，以确保其在实际应用中的稳定性和正确性。测试过程应该综合考虑硬件平台、驱动软件和操作系统的交互，并根据具体产品需求进行定制化的测试方案和测试用例设计。



咨询赵方正，spi nor、spi nand 主要主要会用到哪些文件系统，是否需要在文件系统上进行读写操作

跑方案的时候，哪些场景会覆盖到 flash 相关的操作，对于 nor 和 nand 在使用上面有什么差异？

大负载测试，一边进行大内存占用情况，一边进行 flash 读写擦操作

开关机测试，频繁开关机进行上面的负载测试，查看是否还能正常启动，正常操作

文件系统：在文件系统的情况下进行上面的负载测试

- nor：jffs2
- spi nand：yaffs2

http://git.nationalchip.com/redmine/issues/294242







# otp 操作返回的长度有问题

- [x] 修改返回的长度，之前使用的是 message 的方法，长度会去掉 cmd_len 和 dummy，现在自己做了一个接口，所以读写的长度直接就是配给接口的长度





# nand 驱动的问题

- [ ] `sirius` 使用测试命令 `flash nandcomtest` 的时候出现写入失败，由于在 `nand_do_write_ops` 接口同时写 `oob+data` 的时候会计算 `subpage`以及根据 `subpage&&oob` 的判断条件来退出写入操作

  ```c
  	column = to & (mtd->writesize - 1);	/* 存在页不对齐的数据 */
  	printf("to:0x%x, colunm:0x%x, mtd->writesize:0x%x\n", to, column, mtd->writesize);
  	subpage = column || (writelen & (mtd->writesize - 1)); /* 地址页不对齐、要写的长度页不对齐的情况 */
  
  	/* 页不对齐且有 oob 数据 */
  	if (subpage && oob)
  		return -EINVAL;
  ```

  - [ ] 这个 `subpage` 是用来干什么的？

- [ ] `sirius` 使用同时读 `data+oob` 的接口时，存在两个问题

  - [ ] 第一个问题：使用`nand_transfer_oob`的时候把数据从  `chip->oob_poi + ops->ooboffs` 的这个位置拷贝到 `buf` 中去，而不是拷贝到 `oob` 区域，并且拷贝的 `size` 是 `oob->size`；

  - [ ] 第二个问题：在 `flash_nand_comtest`的接口中 `malloc` 的 `oob 区域` 大小设定的是 `oobsize` 的一半，而在拷贝的时候是拷贝的一整个 `mtd->oobsize`，导致出现内存越界，下一次做 `malloc` 的时候就直接挂掉了

    ```diff
    	oob_size  = gxflash_get_info(GX_FLASH_OOB_SIZE);
    	oob_size = oob_size / 2;
    
        oob_addr_tmp = oob_size * ((float)rand_test()/(0x7fff+1) );
        oob_len_tmp = oob_size * ((float)rand_test()/(0x7fff+1));
    
        if(flash_nand_test_single(addr_tmp, len_tmp, oob_addr_tmp, oob_len_tmp, mode))
    
    
    
    -                               } else
    -                                       buf = nand_transfer_oob(chip,
    -                                                       buf, ops, mtd->oobsize);
    +                               } else {
    +
    +                                       int toread = min(oobreadlen, mtd->oobsize);
    +                                       oob = nand_transfer_oob(chip, oob, ops, toread);
    +                                       oobreadlen -= toread;
    +                               }
    ```

    



# SPI NAND 同时读写 data and oob

- [x] 在添加这个接口的时候，只更新了 data 的 buf、len，没有更新 oob 的 buf、len

- [x] 读写  data and oob 的时候，如果我要传输的数据长度是多个 page，要读的 oob 长度是0x10，这个时候读到的 oob 数据是哪里的 oob 数据？

  - [ ] page 也不能跨 page 来读取，因为读数据的时候是把 oob 和 data 同时读上来的，读 oob 和 data 都是通过 addr 来确定 pageid 的，oob 的地址是一个相对偏移，在做 transfer_oob 的时候拷贝多少出来。如果数据或者 oob 要跨 page 的话，没办法根据 确定 pageid
- [x] 如果我要读的数据很短，0x10，要读的 oob 长度是 0x200，这个时候读到的 oob 数据又是哪里的数据？

  - [ ] oob 数据不能够跨 page 来读取

- SPI NAND 读写 oob 接口实现的逻辑：

  ```
  |   page0   |   page1   |   page2   |   page3   |
  | data  oob | data  oob | data  oob | data  oob |
  
  写 oob：需要地址、oobbuffer、len
  	根据地址计算出所在的 page，根据 len 计算要写几个 oob 区域，oobbuffer 用来写入
  	首先计算出来 page，把 oobbuffer 写入到这个 page 的 oob 区域，根据模式确定要写多长
  	更新到下一个 page，更新 oobbuffer 写入到下一个 page 的 oob 区域，根据模式确定要写多长
  	
  读 oob：需要地址、membuffer、len
  	根据地址计算出所在的 page，根据 len 计算要读几个 oob 区域，membuffer 用于读取
  	首先计算出来 page，把这个 page 的 oob 区域数据读到 临时buffer，根据模式来 把临时buffer中的数据拷贝到 membuffer
  ```










# sirius 写数据的时候，怎么写 ecc 码到 oob 区域的？

- sirius 写数据的时候会把数据写到 oob 区域对应的 ecc码区域。sirius 会留出 4 bit 的空间用来存放擦除标记，原因是 sirius 的 NFC 在擦除操作之后，即数据区和 ecc code 全为 0xff 的时候，如果数据区发生跳变不会做纠正操作。所以软件上通过用 4 bit 的空间存放擦除标记，如果擦除完之后出现跳变，相当于是直接用全 0xff 的数据，就不用控制器来纠错。而canopus 的 oob 区域不会预留 4 bit 空间来存放擦除标记，因为擦除完之后即使出现跳变，nfc 也能纠正回来。

- 纠正的时机：由控制器在读的时候，每次先读512，再读ecc码，进行比对，如果有bit翻转，就进行纠错。

- 上层应用读写的时候操作的 oob 大小不能超过 oobavalib大小。
- 写的时候做 `fill_oob` 会判断要写的数据，如果是 auto 模式，首先会计算偏移，偏移是从len =  2+ offset 的，然后能够写的长度就是 free->length - len，但是实际要写的长度是比这个 len 要大的，所以要写的数据会被丢掉。
- 读的时候也是一样的，要读的长度和实际读的长度小，丢掉了部分数据，所以会读写出错。







3.0.0

3.0.1

3.1.0

3.1.1

以上四个分支做 cygnus 降频 SPI 补丁







# gxloader 中测试 firewall testcase 完成后需要 free 操作

- [ ] firewall testcase 中只做了 malloc，没有做 free



# ZB25VQ32 进行 USB 升级时出现烧录数据不一致



- 给的是一个 4M 的 flash 型号，分区表描述是 8M 的，bin 也是 8M 的





1. 测试擦除 4M 数据，然后迅速断电，查看dump 出来的数据【测试擦除的时候断电，只会擦除完整个 sector(0x1000)】

   ```shell
   boot> flash singletest 0x0 0x400000
   sflash_gx_readl finish.
   sflash erase finish.
   read addr=0x0, len=0x400000
   sflash_gx_readl finish.
   sflash page program finish.
   sflash_gx_readl finish.
   cmp finish. src equal dst.
   boot> flash singletest 0x0 0x400000
   sflash_gx_readl finish.
   ��������
   ```

   ![选区_149](image/选区_149.png)



2. 如果擦除 block 擦除完了，然后在写的时候断电会是什么情况？

- 做 flash comtest 的时候，擦除完成、读完成之后，写的时候断电，会不会只写不是页对齐的数据进去？【会把整个 page 的数据写进去】

```shell
>>>>>sflash_test:addr=0x0, len = 0x100000
sflash_gx_readl finish.
sflash erase finish.
read addr=0x0, len=0x100000
sflash_gx_readl finish.
sflash page program finish.
sflash_gx_readl finish.
cmp finish. src equal dst.
>>>>>sflash_test_success.


>>>>>sflash_test:addr=0x0, len = 0x200000
sflash_gx_readl finish.
sflash erase finish.
read addr=0x0, len=0x200000
sflash_gx_readl finish.
���������
```

![选区_146](image/选区_146.png)



3. 做掉电测试













```
============================================================================================================
ID NAME       FS           CRC32  UPDATE VERION   START TOTAL_SIZE  MAIN_SIZE  USED_SIZE   USE%  RES_SIZE OK
============================================================================================================
0  BOOT    RO RAW       FED605E9       0      0 00000000 128.00 KB  128.00 KB  68.23 KB  53.3%       0  B  *
1  SCPU    RW RAW       A6165659       0      0 00020000  64.00 KB   64.00 KB  10.88 KB  17.0%       0  B  *
2  OTA     RO RAW       0C656A28       0      0 00030000 512.00 KB  512.00 KB 415.00 KB  81.1%       0  B  *
3  TABLE   RO RAW                      0      0 000b0000  64.00 KB   64.00 KB    512  B   0.8%       0  B  *
4  LOGO    RO RAW       4CEDB02C       0      0 000c0000  64.00 KB   64.00 KB  29.10 KB  45.5%       0  B  *
5  KERNEL  RO RAW       F94FFA72       0      0 000d0000   4.50 MB    4.50 MB   1.97 MB  43.7%       0  B  *
6  ROOT    RO CRAMFS    2777F80A       0      0 00550000   1.56 MB    1.56 MB 840.00 KB  52.5%       0  B  *
7  OEM     RW RAW       0D915C7D       0      0 006e0000  64.00 KB   64.00 KB    557  B   0.8%       0  B  *
8  BACKOEM RW RAW                      0      0 006f0000  64.00 KB   64.00 KB      0  B   0.0%       0  B  *
9  DATA    RW MINIFS    FF000000       0      0 00700000 768.00 KB  768.00 KB      1  B   0.0%       0  B  *
10 PRIVATE RW RAW                      0      0 007c0000 256.00 KB  256.00 KB      0  B   0.0%       0  B  *
------------------------------------------------------------------------------------------------------------

// bin1
分区     分区起始地址   数据不同地址  备注
SCPU     00020000    00020000 

ROOT     00550000    00590000   00550000~00590000和要升级的数据相同，00590000~006e0000和未升级的数据相同
OEM      006e0000    006e0000   整个不同，数据和未升级前的数据相同
BACKOEM  006f0000    006f0000   整个不同，数据和未升级前的数据相同
DATA     00700000    00710000   00700000~00710000和要升级的数据相同(0xff)，00710000和升级前后的数据都不同
PRIVATE  007c0000    007c0000   007c0000~0x7fffff和升级前后的数据都不同



OTA      0x30000                         升级成功
LOGO     0xc0000                         升级成功
KERNEL   0xd0000                         升级成功
ROOT     0x550000                        
		 0x590000 开始所有的数据都是升级前的数据


// bin2
分区     分区起始地址   数据不同地址   备注
SCPU     00020000    00020000 
KERNEL   000d0000    000f004c     000d0000~000f004c为要升级的数据，000f004c~000fd000为0xff，000fd000~00700000都为升级前的数据
ROOT                              没升级
OEM                               没升级
BACKOEM                           没升级
DATA     00700000    00710000    00700000~00710000和要升级的数据相同(0xff)，00710000到结尾的数据和升级前后的数据都不同


OTA      0x30000                         升级成功
LOGO     0xc0000                         升级成功
KERNEL   0xd0000                         升级了一部分，先擦出了一个BLOCK到地址00FD000，写数据到 000F004c结束，00F004C到 00FD000没写，所以都是0xFF，从 00FD000开始都是升级前的数据，一直到 ROOT 分区都是升级前的数据
ROOT     0x550000                        



// bin3
分区     分区起始地址   数据不同地址   备注
SCPU     00020000    00020000 
ROOT     00550000    00610002     00550000~00610002为要升级的数据，00610002~610100为0xff，610100~610102为要升级的数据，610103~61E000 为 0xff， 61E000~700000为升级前的数据，700000开始之后的数据既不是升级前的数据也不是升级后的数据



OTA      0x30000                         升级成功
LOGO     0xc0000                         升级成功
KERNEL   0xd0000                         升级成功
ROOT     0x550000                        升级了一部分，擦除到地址：0061E000,写数据到00610002就没写了，所以后面全部都是0xFF，0061E000到后面所有的 ROOT 分区都没写了。


```

../.boot/zb25vq32/zb25vq64_dump_erase_serial_04.bin 开始是正常的测试流程：只影响 sector

05：只影响 sector

06：只影响 sector



做写入测试：

01：整个 page

02：整个 page

03：整个 page









# linux 库是 build in 的，是不是 gx_core.a 就是 gxapi 编译出来的

ecos 的驱动编译到了哪个  .a



# bsp.cpp

- bsp.cpp 中用宏 MOD_SPINORFLASH(CHIP) 来注册 串行 Nor，用宏 MOD_SPINANDFLASH(CHIP) 注册 串行 Nand；ecos 不支持并行 Nand

  

  

- canopus 芯片 6631SH1D、6631SH2D 只有 SPI 接口，只能用串行 Nor、串行 Nand

- canopus 芯片 3215B 默认是 并行 Nand

- canopus 芯片 6631SHNG 默认引脚复用是 SPI，如果需要用并行 Nand，需要修改引脚复用: NFC: 17~29 配 3









XT25F64F-S 2308TH0B

XT25F64F 2152TH0B

1. 现在手头上有两款 flash，查看丝印分别为 XT25F64F-S 2308TH0B 和  XT25F64F 2152TH0B，其中 -S 的是针对掉电问题新改版后的正式版 flash，不带 -S 是当时临时修改后发过来的临时版 flash，临时版和正式版的 otp 区域大小不同，分别是 3 * 1024 和 2 * 1024，这个麻烦解释一下？

2. 驱动要支持正式版的 flash，那么只要把临时版的 otp 大小减小一个区域，也变成 2 * 1024 是不是就可以和正式版的兼容？会不会有什么影响？









# ecos 编译流程

- 创建一个 work_arch 目录
- 执行 `scripts/ecosconfig-tree csky/arm`
- 执行 `scripts/ecosconfig --config=work_csky/.ecc --prefix=../../library/csky-ecos/  tree`
- 到 `work_csky/makefile` 目录下执行 make







# pnand 驱动问题

- nand_base.c 中的 `nand_scan_tail` 函数， 判断 if 的时候不能这么判断，还需要加上 device_id = 0xDA 的
- 如何定义一个block为坏块？
  - 做擦除操作，发擦除命令
  - 然后读状态寄存器，控制器 busy
  - 就标记该块为坏块
- 如何标记一个 block 为坏块？
  - 根据当前操作的地址，从坏块表中找，看当前这个地址所在的 block 是否为坏块
  - 如果当前块不在坏块表中，更新内存中的坏块标记
  - 更新坏块表到 flash 上

- 现有情况：从 0x100000 地址开始测试 0x100000 长度的数据，先读取这片区域的数据，然后按 block 对齐擦除这片区域，然后读取这片区域的数据，判断是否为 0xff，然后做一些随机数据，将数据写入这片区域，然后读取出来看数据是否与写入的一致；
  - 最后读取出来的结果发生在其中某个 block 的起始位置，数据为 0xff
  - 第二次仍然执行上面同样的操作，出错的 block 换成后面的 block，前面发生错误的 block ，这次测试通过了
  - 继续执行，出错的 block 继续后移，前面发生错误的  block 测试通过了
  - 即 出错的 block 只会出错 1 次，出错之后查看数据全为 0xff
- 坏块太多，并且全部都是 奇数 blcok





- 重启 dump
- 直接下载完就 dump
- 使用最新的 .boot 烧写
  - 不启动，直接 dump 出来的数据正确
  - 启动，再 dump的数据会变
- 如果出现 ecc 状态无法纠正的时候，那么可能要读的数据没读完或者没写完





- [x] scrub 擦除 all ，会先把 `chip->bbt` free 掉，然后擦除所有的 block，擦除完成之后调用 `chip->scan_bbt`
- [x] 使用过程中出现坏块，是如何标记的



# ONFI 手册

- CE_n 引脚 Reduction 机制

  - Reduction：缩减、减少等的意思
  - 在 Nand Flash 领域，CE_n 引脚是用于激活或禁止芯片的引脚。在读写操作过程中，通过操作 CE_n 引脚，可以对 Nand Flash 芯片进行选中或取消选中
  - CE_n 引脚 Reduction 的核心思想：在连续的读取或写入操作中，不需要在每个操作之间都切换 CE_n 引脚的状态。仅在一系列连续操作的开始和结束时切换 CE_n 引脚的状态。这样可以减少引脚状态切换的次数，从而提高操作效率，并降低功耗。 --- from chartGPT

  - 在高容量的设计中可能会有很多数量的 CE_n 引脚，这种设计会包含许多 NAND 封装，每个封装有 2 到 8 个 CE_n 引脚。CE_n 引脚 reduction 机制可以让 host 的单个 CE_n 引脚被多个 NAND 对象复用，因此可以使 Host 需要的 CE_n 引脚数目大大减少。
  - CE_n 引脚 reduction 机制在初始化过程中，为每个 NAND 对象指定一个 Volume 地址。初始化完成后，host 可以通过 Volume 选择命令来寻址特定的 Volume。

- 存储结构
  - 一个 device 包含一个或多个对象，一个对象由一个 CE_n 信号控制。一个对象位于一个或多个逻辑单元内(LUN)
  - 一个逻辑单元是可独立执行命令并报告状态的最小单元。特别是，独立的 LUN 可以并行运行任意的命令序列。
  - 一个 block 是 逻辑单元的闪存阵列中可擦除的最小数据单元
  - 一个 page 是执行读和编程操作的最小可寻址单元。一个 page 由若干个字节或字组成。每个 page 中用户数据字节的数量，不包含 spare data area，应该是 2 的次幂。每个 block 的 page 数量应该是 32 的整数倍
  - 每个逻辑单元应该至少有一个 page 寄存器。Page 及存在在数据被转移到闪存阵列的一个 page 之前，或数据被从闪存阵列的一个 page 转移出来之后，用来临时存放数据。
  - Page 寄存器中的字节或字的位置被称为列。
- 寻址
  - 有两种地址类型：列地址和行地址。
    - 列地址用来访问一个 page 中的字节或字。
    - 行地址用于寻址 page、block 或者 逻辑单元

- 工厂缺陷映射
  - 通常假设闪存阵列不是完好的，会引入若干个缺陷，这些缺陷可能会造成某些 block 不可用。块粒度被用来映射工厂缺陷，因为这些缺陷可能会破坏块擦除功能。
  - 如果一个 block 是有缺陷的，并且使用 8-bit 数据访问方式，则制造商应该通过设置有缺陷的块的第一个或最后一个 page 中，缺陷区域的第一个 byte 为 0x00，来标记有缺陷的块。
  - 如果一个 block 是有缺陷的，并且使用 16-bit 数据访问方式，则通过设置缺陷块的第一个或最后一个 page 中，缺陷区域的第一个 word 为 0x0000，来标记有缺陷的块
  - 主机不能读或者编程标记为缺陷的块，任何这样的尝试会产生不确定的结果。
  - 主机在擦除或变成钱，检查每个块中第一个和最后一个 page 的缺陷区域中的第一个 byte/word 的值，来确认块是好的

- 单数据总线 Discovery
  - 要测试的 CE_n 是第一个被主机拉低的，以使能对象。在此迁建，所有其他的 CE_n 信号都被拉高。随后主机应该向对象发送 Reset(0xFF) 命令。Reset 之后，主机应该向对象发送一个读 ID 命令。如果 ONFI 签名被地址为 0x20 的读 ID 命令返回，则相应的对象已被连接。如果 ONFI 签名没有被返回，或者过程中任何步骤发生错误或 timeout，则 CE_n 没有被连接，使用该 CE_N 信号的其他操作不会被完成。
- 双/四数据总线 DIscovery
  - 反复循环上面单数据总线的方式，每次切一个 CE_n
- 对象初始化
  - 为了初始化一个已被 discovered 的对象，以下步骤应该被执行。每个被连接的 CE_n 对象都应该执行初始化过程，包括每一个对象的读参数页命令(0xEC)。
  - 主机应该发送 0xEC 命令，该命令返回代表 device 容量、特征以及操作参数的信息，当该信息被从 device 读取后，主机应该在对信息数据执行操作之前，检查其 CRC，以确保接受的数据是正确的的，没有错误的。
  - 在成功获得参数页之后，主机具备了与device 成功通信所需的所有信息。

- 数据接口和时序
  - ONFI 支持 4 种不同的数据接口类型：SDR、NV-DDR、NV-DDR2、NV-DDR3、NV-LPDDR4
    - SDR：传统的 NAND 接口，使用 RE_n 锁存读数据，WE_n 锁存写数据，没有时钟
    - NV-DDR 是双数据率(Double Data Rate-DDR) 接口，含有用来锁存命令和地址的时钟，和一个用来锁存数据的数据选通信号







- GD 
  - tCOH、tRHOH
- DS
  - tCOH、tRHOH

- XTX
  - tRHOH、tRLOH





# SPINAND OTP 无法锁

- 驱动里面会操作两次，第一个锁，第二次解锁，由于有些 状态寄存器不是 one time 的，所以操作两次相当于解锁了，查到的状态就一直是 unlock
- otptest otptest 
  - 前两个 page 无法通过测试，非0xff
- otplock otptest







# 并行 nand 采用软件操作的方式来纠错

- 首先是要拿到 ecc 码，这是
- 然后根据 ecc 码，来纠错
  - 纠错的过程就是把信息位和校验位依次写入 `ECC_DATA_REGISTER`
- 编码过程：
  - 按照 sector 的方式来编码的，写 512，算 ecc，写 512，算 ecc，把算出来的 ecc 都保存好，然后写到 oob 区域
  - sirius ： 一次性写一整个 page，然后算 ecc
- 解码过程：
  - 是不是也是按 sector 的方式来解码的



# SPI Nand readpage 添加判断接口

- 需要模拟测试用例

  | 源                       | 结果       |
  | ------------------------ | ---------- |
  | ECC 关闭、不 disable ecc | 未改变逻辑 |
  | ECC 关闭，disable ecc    | 未改变逻辑 |
  | ECC 开启、不 disable ecc | 未改变逻辑 |
  | ECC 开启、disable ecc    | 未改变逻辑 |









# scpu uid 功能

- 目前在 `cmd/flash/flash.c` 中已经有 uid 的测试程序
- 但是之前加 otp 相关操作的时候是在 `cmd/flash/flash_test.c` 中加的 otp 测试程序，现在 uid 的测试程序是用 `flash.c` 中带的，还是搬一份到 `flash_test.c` 中

- [x] 验证一下 otp 功能，包括 leo_mini 、grus、leo
  - [x] leo_mini、grus 使用的是 flash_spi.c
    - [x] 由于 otp 操作只支持 puya 的几款，所以 leo_mini 没法测
  - [x] leo 使用的是 spinor.c
- [x] 验证一下 uid 功能，包括 leo_mini、grus、leo，然后需要验证 puya、zb 的 flash
  - [x] leo 不需要验
  - [x] leo_mini 通过，grus 通过



vsp 

1. 添加驱动
2. 这边没有测试程序一说，我要找一个地方把测试程序跑一下
   - `vsp/vsp_main_standalone.c` --> `vsp/common/vsp_vpa.c`









# SPI Nand 至讯

- 所有的坏块表

  ![image-20230912153507971](image/image-20230912153507971.png)

- 抓 ecc error 的数据打印或者波形

- 看数据打印，里面至少有两个 page 的数据是全 0，那么 ecc 状态报错只报了一次，应该报至少 2 次，抓一下出错的波形

  - 抓 log，从出错了，后面就没有发命令了，就是没读完，然后里面的数据是全 0x00，那么我把数据 memset 成别的，是不是就是别的数据了
  - 目前看是没发波形了



# leo gdb

- 修改编译等级，打开 jtag
- 开 gdbserver、gdb

- otp 擦除
- otp 读取
  - 发了命令，没有读，xfer.xfer_len 没传过去
  - 读接口需要把  xfer.tx 置为 NULL
- otp 写入
  - 写接口需要把 xfer.rx 置为 NULL







# leo_mini 相位测试

- 时钟频率：200M

  - 分频：4分频，50M

    - 相位：2

      ```
      ********** FLASH SPEED TEST *********
      >>>>>read       8 kb from flash speed: 3259 KB/s
      >>>>>erase      8 kb from flash speed: 81 KB/s
      >>>>>write      8 kb from flash speed: 531 KB/s
      flash comtest finished.
      ```

    - 相位：0

      ```
      ********** FLASH SPEED TEST *********
      >>>>>read       8 kb from flash speed: 3259 KB/s
      >>>>>erase      8 kb from flash speed: 81 KB/s
      >>>>>write      8 kb from flash speed: 531 KB/s
      flash comtest finished.
      ```

    - 相位：3

      ```
      ********** FLASH SPEED TEST *********
      >>>>>read       8 kb from flash speed: 3185 KB/s
      >>>>>erase      8 kb from flash speed: 78 KB/s
      >>>>>write      8 kb from flash speed: 532 KB/s
      flash comtest finished.
      ```

    - 相位：4

      - 启动失败

  - 分频：2分频，100M

    - 相位：2

      ```
      ********** FLASH SPEED TEST *********
      >>>>>read       8 kb from flash speed: 3261 KB/s
      >>>>>erase      8 kb from flash speed: 77 KB/s
      >>>>>write      8 kb from flash speed: 529 KB/s
      flash comtest finished.
      ```

    - 相位：0

      - 启动失败

    - 相位：1

      ```
      ********** FLASH SPEED TEST *********
      >>>>>read       8 kb from flash speed: 3263 KB/s
      >>>>>erase      8 kb from flash speed: 76 KB/s
      >>>>>write      8 kb from flash speed: 531 KB/s
      flash comtest finished.
      ```

    - 相位：3

      - 启动失败

- 时钟频率：150M

  - 分频：2分频，75M

    - 相位：0

      - 启动失败

    - 相位：1

      ```
      ********** FLASH SPEED TEST *********
      >>>>>read       8 kb from flash speed: 3265 KB/s
      >>>>>erase      8 kb from flash speed: 75 KB/s
      >>>>>write      8 kb from flash speed: 530 KB/s
      flash comtest finished.
      ```

    - 相位：4

      - 启动失败

    - 相位：3

      - 启动失败

    - 相位：2

      ```
      ********** FLASH SPEED TEST *********
      >>>>>read       8 kb from flash speed: 3259 KB/s
      >>>>>erase      8 kb from flash speed: 76 KB/s
      >>>>>write      8 kb from flash speed: 528 KB/s
      flash comtest finished.
      ```

      



当将DWC_ssi配置为master时，可以在设计中加入额外的逻辑，以延迟rxd信号的默认采样时间。这种额外的逻辑可以帮助提高串行总线上的最大可实现频率。

来自主服务器的sclk_out信号和来自从服务器的rxd信号的往返路由延迟可能意味着rxd信号的时间——正如主服务器所看到的——已经偏离了正常的采样时间，图2-45说明了这种情况。



经过芯片内部线路延迟、CLK 发出到 Flash 的延迟、Flash 接收到 CLK 并发出数据的延迟、PCB 线路延迟等



从机使用来自主机的sclk_out信号作为频亮，以便将rxd信号数据驱动到串行总线上。从设备对sclk_out信号的路由和采样延迟可能意味着在主设备对rxd信号进行采样之前，rxd位还没有稳定到正确的值。图2-45给出了一个例子，说明了rxd信号上的路由延迟如何导致主设备对端口进行采样时默认时间的rxd值不正确。

没有RXD采样延迟逻辑，你必须增加波特率的传输，以确保在RXD信号上的设置时间在范围内;这就会导致串行接口的频率降低。

当包含RXD采样延迟逻辑时，您可以动态编程一个延迟值，以便将RXD信号的采样时间从默认值移动到等于若干ssi_clk周期。RXD采样延迟逻辑可以配置为使用ssi_clk的正负边来采样RXD信号。这可以增加sclk_out周期内采样点的数量，并有助于满足更高频率的定时。

DWC_ssi使用RX_SAMPLE_DLY寄存器来改变RXD信号的采样点

如果RX_SAMPLE_DLY。将SE设置为0，则DWC_ssi通过编程的ssi_clk周期数延迟采样点。

如果RX_SAMPLE_DLY。SE设为1，则DWC_ssi通过编程的ssi_clk周期数+ 0.5 * (ssi_clk周期)延迟采样点。因此，在ssi_clk的负边进行采样，如图2-46所示。这在单个sclk_out时钟周期内为用户提供了更多的采样点。

RX_SAMPLE_DLY。只有当ssi_has_rx_sample_delay配置参数设置为“使用ssi_clk的正负边进行RXD采样”时，SE位才可用。否则，DWC_ssi只使用ssi_clk的正边来采样RXD信号。

根据编程，只有RXD采样逻辑工作在ssi_clk的负边，其余的逻辑工作在ssi_clk的正边。DWC_ssi_clk_mux_2x1。V模块实现了一个时钟复用器，用于复用ssi_clk和ssi_clk_n。建议将此模块替换为技术库中的均衡多路复用器。	



- 50M 模块频率

  - 2 分频

  - 相位范围：

    - 2 无法启动
    - 1 
      - 双倍速
        - 开 dma
        - 不开 dma：测试
      - 四倍速
        - 开 dma：测试
        - 不开 dma：测试

    - 0
      - 双倍速
        - 不开 dma：测试
      - 四倍速
        - 不开 dma：

- 100M 模块频率
  - 2 分频：50M
  - 相位范围：
    - 0：不开 dma 
      - 双倍速测试通过
      - 四倍速
    - 1：不开 dma 
      - 双倍速测试通过
      - 四倍速
    - 2：
      - 双倍速无法启动
      - 四倍速无法启动







# 【已完成】XTX Flash 驱动能力补丁

- [x] 驱动能力补丁，读一下寄存器，判断一下值，如果是 0 就不改，如果是 非 0 就改成 0
- [x] 以具体 ID 区分，有部分型号的 Flash 不支持配置驱动能力



# Leo_mini 相位测试

- [x] 200M 4 分频，要测双倍速、四倍速的相位
- [x] 150M 2 分频，要测双倍速、四倍速的相位
- [ ] 找陈树确认 75M 频率下的两个相位选择哪一个
  - [ ] 黄色是 CLK，蓝色是 MOSI，模块频率150MHz，2 分频，Flash 工作频率75MHz，相位 1 和 2 都可以，这是 1 的相位




# 【已完成】Canopus 的 SPI MODE 使用的是哪种？

- [x] 上升沿采样还是下降沿采样
  - [x] 下降沿采样
- [x] 由 CTRL0寄存器的 SCPH、SCPOL 位 8、9位决定



# 【已完成】XM25QH32D 支持

- 参考 XM25QH64D



# Block/Sector WP

- ecos_shell
  - `prj/spi_cramfs_jffs2/source/spi_commands.c`：应用程序，调用`cyg_io_get_config` 
- gxapi
  - ：需要ecos 里面提供 ioctl



- ecos
  - `ecos3.0/packages/io/flash/v3_0/src/flashiodev.c`：ecos 的驱动程序，提供 ioctl 给应用程序，下面对接 gxmisc
  - /home/tanxzh/goxceed/v1.9.8-8.3-jio/ecos3.0/ecos3.0/packages/io/flash/v3_0/src/flash.c
    - 需要仔细验证接口
    - [ ] 这里的 find_dev 是用来干什么的？



- gxmisc
  - `drivers/spinor/ecos/spinor.c`：提供给 ecos ，这个是 ecos 的驱动
  - `drivers/spinor/spinor_base.c`：裸机驱动，ecos 的驱动就是调这个文件内的接口的



- [ ] flash_dev 是干什么的
  - 用于在 `cyg_flashdevtab` 数组中创建一个数组成员，并执行初始化函数，然后返回 `cyg_flashdevtab` 所在的数组成员
- [ ] ecos_shell 中添加完整的测试程序
- [ ] ioctl 会操作返回值，是判断返回值为 0 就是 unlock，非 0 就是 lock，还是修改成使用参数的方式带回来
  - [ ] ioctl 传一个 buf 下去，buf 在 gxmisc/ecos/spinor.c 处理 status，返回值还是 ret





# 【已完成】确认 CS 维持时间的最小值

- GD：20ns
- 华邦：50ns
- [x] 其他型号的 Flash 如果最小值是很大，而我们代码运行的时间比较短，也有可能会导致代码运行完的 CS 维持时间还没有到 Flash 要求的最小值时间



- spinor

  - 华邦：50ns


  - 兆易创新 GD：20ns


  - 宜杨 EON：40ns


  - 优存 UC：30ns


  - 聚辰 GT：30ns

  - 旺宏 MXIC：30ns

  - 普冉 PUYA：30ns

  - 三峡 SANXIA:50ns

  - 紫光青藤 TH：20ns

  - 武汉新芯 XMC：30ns

  - 芯天下 XTX：20ns




- spinand
  - stage1 50MHz，相位 01
  - 复旦微：Tcs = 20ns   Tchsl = 5ns
  - 武汉新芯：Tcs = 30ns Tchsl = 3.375ns
  - ESNT：Tcs = 80ns Tchsl = 5ns
  - 东芯：Tcs = 100ns Tchsl = 5ns
  - MXIC：Tcs = 100ns Tchsl = 4ns
  - XTX：Tcs = 30ns Tchsl = 5ns
  - 镁光：Tcs = 30ns Tchsl = 3.375ns
  - 江波龙：Tcs = 20ns Tchsl = 5ns
  - 仲联：Tcs = 20ns Tchsl = 5ns
  - YHY：Tchsl = 2ns
  - 华邦：Tcs = 50ns Tchsl = 5ns
  - Zetta：Tcs = 100ns Tchsl = 5ns
  - GSTO：Tcs = 30ns Tchsl = 3.375ns
  - GD：Tcs = 20ns Tchsl = 5ns



# 【已完成】WPS = 1

- [x]  WPS = 1，上电后所有的 block 都会被保护，那不就之前保护的 block 覆盖了吗
- [x] 是的，所以单独保护一个 block/sector 好之后，在下一次启动的时候需要按照需要来解锁不需要保护的 block



- 咨询一下：W25Q128 这款 Flash 的 Individual block protect 操作，手册上描述的使用方法是配置寄存器的 WPS 位为 1，然后发指令和地址来锁定、解锁、获取状态。

- 1. 手册描述是支持 block/sector 的方式进行写保护，如果我发地址 0x0 的时候，flash 会按照什么方式来保护？

- 2. 发送的地址必须是什么对齐的吗？比如我发地址 0x3000 的时候，flash 会怎么锁定？怎么解锁？怎么获取状态？

  

- block 0/255 使用 sector 进行锁定和解锁、获取状态

- block 1~254 使用 block 进行锁定和解锁、获取状态

- 驱动需要判断 block0/255 的时候循环发多次指令+地址来保护一整个 block，不提供给用户 sector 保护

  - block0/255 的时候 sector 操作，block1~254 的时候 block 操作
  - protect_len 向上 block_size 对齐
  - protect_addr 向下 block_size 对齐

- [x] 烧录程序的时候，必须是 WPS = 0，否则启动之后所有的 block 都锁了

- [x] ecos_shell 调不到 gxapi 的接口

  - [x] 添加 -l gxcorelib

- [x] open、使用 open() 的方式、close 什么的参考同级目录下的是实现方式

- [x] gxmisc 中区分 id 的做一个子函数来实现

- [x] enable_wps 的接口中，采用异或的方式，即 wps 状态和 enable 不同的时候才操作，否则不操作
- [x] 使用 switch 的方式来区分厂家号，具体的 id 放在厂家号下面做区分
- [x] 把 cmd 换成使用 flash->command 





- SPB:固态保护位，用于启用或禁用对扇区和块的写保护  非易失
  - block0、block255  sector 保护
  - block1 ~ block254 block 保护
  - 默认出厂状态 SPB 位为 0
  - 通过 WRSPB 单独设置为 1，而无法单独清 0，通过 ESSPB 将所有的 SPB 清 0
  - 发命令 WRSPB/ERSPB 命令之前需要通过 WREN 来设置 WEL 位
  - RDSPB 命令用于读取 sector/block 的锁定状态。返回 0x00 表示 unlock，返回 0xff 表示 lock
  - SPB LOCK 寄存器
    - 是一个 16 bit 的 one time program 寄存器
    - bit6 是 SPB 锁定位 SPBLKDN：分配用于控制所有 SPB bit 状态
    - 当 SPBLKDN 为 1 时，SPB 可以被修改；当 SPBLKDN 为 0 时，所有 SPB 都不能在更改，并且 SPBLKDN 位本身也无法被修改(one time)
    - 在使用命令 WRLR 命令对锁定寄存器编程之前，必须先试用 WREN 命令设置 WEL 位
- DPB:动态保护位，用于启用或禁用对扇区和块的写保护 易失性
  - block0、block 255  sector 保护
  - block1 ~ block 254 block 保护
  - 可以单独启用写保护，而不管SPB 的状态。但是 DPB 位只能取消对 SPB 位为 0 的sector/block 的保护
  - 当 DPB 为 1 的时候，相应的 sector / block 将写保护
  - 通电或复位后，所有 DPB 默认为  1 。
  - 当 DPB 为 0 的时候，如果对应的 SPB 为为 0，则相关的 sector/block 将不被保护
  - 通过 WRDPB 命令将 DPB 为单独设置为 1/0 。
  - 通过 GBULK 命令解锁所有 DPB ，使用 GBLK 命令锁定所有 DPB
  - 必须在执行 WREN 命令在发送 WRDPB、GBULK、GBLK 命令之前设置 WEL 位
    - 在发命令 WRDPB、GBULK、GBLK 命令之前必须使用 WREN 命令设置 WEL 位
  - RDDPB 命令读取sector/block 的 DPB 状态。RDDPB 返回 00 表示 unlock，返回 FF 表示 lock





- 配置 WPSEL = 1：WRSCUR 配置 bit7，需要先发 WREN 命令、RDSCUR (WPSEL 是否 为1)，为 1 就已经使能，为 0 就使用 WRSCUR 配置 bit7，读状态寄存器等 WIP，RDSCUR 读 WPSEL 是否为1 表示配置成功或失败
- 单独写 block：WRDPB 命令将 DPB 单独配置为 1/0
- 单独解锁 block：WRDPB 命令将 DPB 单独配置为 0
  - 0xE1、32bit 地址，前 8 bit 是dummy，一字节数据：写 1 是lock，写 0 是unlock
- 写所有 block：GBLK 命令锁定所有 DPB
  - 0x7E、
- 解锁 所有 block：GBULK 命令解锁所有 DPB
  - 0x98
- 获取状态：RDDPB 命令返回 00 表示 unlock，返回FF表示lock
  - 0xE0、32bit 地址，前 8 bit 是 dummy，返回一字节数据





# 多核、总线仲裁





# Vega 编译流程

## 文件介绍：

- `board/vega/board-generic/param/board-param.c` 保存 stage1 的各种参数信息，包括 ddr、flash、secure 信息
- `board/vega/param/*.c` ：保存 clk、dram 参数、相位等参数信息
- `tools/generate_param_tool/generate_param.c`：根据上面几个 .c 编译输出 param.bin(包括 spl_boot_param.bin、spl_clock_param.bin、spl_dram_param.bin、spl_flash_param.bin)
- `cpu/arm/vega/stage1/*.bin`：固化的  stage1 代码



## 编译流程

- stage1 代码固化成 .bin，在编译阶段使用 openssl 工具解密将其生成为 loader-flash.bin
- 编译 param.c 生成参数信息的 .o，然后使用 参数工具将 .o 制作成 param.bin
- 将 param.bin 接在 loader-flash.bin 后面，此时 loader-flash.bin 包含 stage1 代码和 参数代码
- 编译stage2 的代码，生成 loader-stage2.bin
- 将 loader-stage2.bin 接在 loader-flash.bin 后面，此时 loader-flash.bin 包含 stage1 代码、参数代码和 stage2 代码





## Q&&A

- stage1 固化，则每次读取参数的地址都是固定的，就需要 param.bin 放在 bin 中固定的位置，在哪里做的这步操作？
- 在 cat 的时候是直接把 param.bin 放在 stage1.bin、auxcode、key_and_flag ... 的后面，前面这些大小都是固定的，所以 param.bin 也是在固定的位置





# 优化

- copy.c

![选区_227](image/选区_227.png)

- stage1 的 SPI 分频要跑低一点，现在跑的是 50MHz，使用的指令是 0x03，这个指令在手册上很多要求最高上限频率是 50MHz
- stage1 的 SPI 时钟跑 30MHz，分频和相位都写死掉<font color=red>(vega 和 canopus 都一样)</font>

- EON/ESMT：50MHz
- MICRON：54MHz
- Winbond：50MHz
- MXIC：50MHz 
  - MX25L6406E：33MHz  2014 年的手册
  - MX25L6408E：33MHz  2013 年的手册
  - MX25L3206E：33MHz  2013 年的手册
- DOsilicon：50MHz
- Zetta：50MHz
- SX：55MHz
- Boya：55MHz
- Zbit：50MHz
- GD：60MHz
- XTX：54MHz
- Tsingteng：80MHz
- Ucun：50MHz
- GT：120MHz
- PUYA：55MHz
  - P25Q40SL：33MHz  1.65 ~ 2.0v 供电电压
  - P25Q80SL：33MHz  1.65 ~ 2.0v 供电电压
- Fudan：50MHz
- XMC：50MHz





# vega 信号量卡住

- 不用信号量，只用中断看看会怎么样
  - 中断和信号量是一起加的，不能单独测试
- 跑 minifs 的测试程序
  - 
- 用 canopus 跑这个应用，看会不会卡住等信号量
  - 不会卡住

- 中断的优先级改成 4 试一下
  - 一样的情况



# Vega 补丁

- gxloader：

  ```diff
  --- a/cpu/arm/vega/gxloader.lds
  +++ b/cpu/arm/vega/gxloader.lds
  @@ -10,6 +10,7 @@ SECTIONS {
          . = ALIGN(2048);
          .text :
          {
  +               _stage2_start_ = .;
                  _stage2_text_start_ = .;
                  cpu/arm/vega/vega_start.o(.text*)
                  cpu/arm/vega/vega_start.o(.rodata*)
                  
                  
                  
                  
  --- a/conf/vega/6631SH1D/debug.config
  +++ b/conf/vega/6631SH1D/debug.config
  @@ -96,7 +96,7 @@ EEPROM_TYPE = "24c128"
   ENABLE_IO_FRAMEWORK = n
   
   # if you want to use flash write/erase, you must enable it
  -ENABLE_FLASH_FULLFUNCTION = n
  +ENABLE_FLASH_FULLFUNCTION = y
   # if you want to use otp write, you must enable it
   ENABLE_OTP_FULLFUNCTION = y
   
  @@ -140,6 +140,9 @@ CONFIG_UART_TYPE = DW
   
   CONFIG_MULPIN_VERIFY = y
   
  +# SPI_CONFIG
  +ENABLE_SPI_QUAD = y
  +
  ```

  



- 降低频率到 7/8 MHz 左右
  - 不会出现空中断
- 使用晶振时钟才会出现空中断
- 看看会忽略几次 空中断
  - 每次忽略比 Receive FIFO full 少 2 次

- 降低cpu频率，spi 使用晶振频率
  - 如果不复现问题就正常
- 不降低cpu频率，spi使用pll倍出来的频率降低
  - 如果复现问题就正常



- cpu 频率：pll 倍频，spi 频率：pll 倍频为 50MHz
  - 会出现空中断
- cpu 频率：pll 倍频，spi 频率：晶振频率 27MHz
  - 会出现空中断
- cpu 频率：晶振频率 27MHz，spi 频率：晶振频率 27MHz
  - 不会出现空中断
- cpu 频率：pll 倍频，spi 频率：pll 倍频为 297MHz
  - 不会出现空中断

- 做了上述尝试，发现出现空中断的情况都是 cpu 频率和 spi 模块频率相差很大







# 【已解决】Nor - 4Byte

你好，咨询一下 W25Q256JV 这款 Nor Flash 的 4 byte 模式，SR3寄存器中有 ADP 和 ADS 两位分别用于表示 上电或复位之后工作在 4byte 模式 或 3 byte 模式以及当前工作在 4 byte 或 3 byte 模式。

通过发命令 0xB7 进入 4 byte 模式，此时读到的 ADS 位为 1，所以发命令 0xB7 只会影响 ADS 位的数值。ADP 是通过写状态寄存器的方式来配置的(默认是 0，并且是掉电保存的)，如果不配置的话，那么每次启动或复位 flash 都会运行在 3 byte 模式，无论是否发命令 0xB7。

这样理解是正确的吗？





- ADP 代表上电或复位之后 flash 在 4 byte 模式还是 3 byte 模式，当 ADP = 1 的时候就在 4 byte 模式，ADP = 0 的时候就在 3 byte 模式
- 通过发命令 7B 进入 4 字节模式的时候，并没有改变 ADP 位，从 ADS 中读取出来的是当前的字节模式，所以 ADS 读出来的是 1
- 所以下一次启动还是在 3 byte 模式，只要没用写状态寄存器的方式写 ADP 位，默认启动就一直是 3 byte 模式





# 【已解决】SCPU - WPTEST

- 擦除整个 flash
- program_single_test(0, 0)
  - lock 0 长度
  - 每个 block 的开头写 0xab
  - 读取每个 block 的开头第 mark_offs 个字节，是否是 0xff，如果是就是被保护了，否则可能没被保护
  - 从可能没被保护的地址开始读后面的每个block 的开头第一个字节，看数据是否是 0xab，如果是的话那就没被保护
- 循环测试多个长度
- erase_single_test(flash_size, mark_offs++)
  - 配置成写保护 0
  - 擦除整个 flash
  - 在每个 block 的开头第一个字节，写上个 0xcd
  - 保护特定长度
  - 擦除整个 flash
  - 读每个 block 的第一个字节，看数据是不是 0xff





# 【已解决】DMA 死机

- 开关 dma 看是否会出现死机情况
- 目前是双核，编译单核试试看
- dma 操作的时候改成同步的，不让cpu 进入休眠
- cpu 读的时候会不会访问栈的边界
- 上面做一个 max
  - 比较fifo和cache_line
- 下面做一个 min
  - 比较实际要传的长度和cpu 想要传的长度
- 测试一下其它的内存地址和长度会不会有问题
  - 主要是内存地址不对齐，长度不对齐的情况



```
flash read 0x0 0x70000001 0x22
** 将 flash  0 地址的数据读到内存 0x70000001，长度为 0x22

offset = (uint32_t)buffer % HAL_DCACHE_LINE_SIZE;
        if (offset) {
            cpu_len = HAL_DCACHE_LINE_SIZE - offset;   // cpu_len = 64 - 1 = 63
            dws->len = cpu_len;                        // dws->len = 63
            dw_half_duplex_poll_transfer(dws);         // cpu 传 63 个字节
            dws->len = len - cpu_len;                  // dws->len = 40 - 63 --> 出来一个很大的值会冲内存
            dws->buffer = (void*)((uint32_t)buffer + cpu_len);
        }

** spi 采用内部 dma 的方式读数据，由于地址 0x70000001 不是 cache_line 对齐，则 cpu 需要传输 63 个数据来让 buffer 保持 cache_line 对齐，但实际只需要读 0x22 个数据，导致计算时 dws->len = len - cpu_len 时出来负值循环成一个很大的值
** 由于是 dma 操作，所以这里会把 全部搬到内存地址 0x70000001 起，长度是 0xfffffffx，从而冲内存

操作之前：全为 0xff
操作之后：连接上 gdb 查看内存，发现数据全部都被更改了
(gdb) x/20x 0x70000000
0x70000000:	0xaa55aaff	0x00000055	0x00000000	0x00000000
0x70000010:	0x00000000	0x00000000	0x00000000	0x00000000
0x70000020:	0x00002000	0x9ff014ea	0x9ff014e5	0x9ff014e5
0x70000030:	0x9ff014e5	0x9ff014e5	0x9ff014e5	0x9ff014e5
0x70000040:	0x400198e5	0x40019880	0x40019880	0x40019880


打上补丁之后：内存中其它位置的数据仍保持正常
aa 55 aa 55 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
20 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff 
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 
ff ff ff ff ff ff ff ff
```



* todo
  * 写一个测试程序：自动测 spi 相位，测完自己选中间值
  * 目前有 spi 的 set_clk 接口，再做一个 set_sample_dly 接口
  * 只要控制器没有开启的时候，就可以去配置 clk 和 sample_dly
  * 配一个参数，然后轮询一轮测试，通过就ok，不通过就退掉







- canopus spinand 速度

  - 100MHz

    - 双倍速

      ```
      boot> flash speedtest
      erase flash size 8MiB
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      program 8MiB elapse 2472ms
      read 8MiB elapse 716ms
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      erase 8MiB elapse 331ms
      ```

    - 四倍速

      ```
      boot> flash speedtest
      erase flash size 8MiB
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      program 8MiB elapse 2080ms
      read 8MiB elapse 650ms
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      erase 8MiB elapse 328ms
      ```

  - 66.67MHz

    - 双倍速

      ```
      boot> flash speedtest
      erase flash size 8MiB
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      program 8MiB elapse 2817ms
      read 8MiB elapse 884ms
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      erase 8MiB elapse 333ms
      ```

    - 四倍速

      ```
      boot> flash speedtest
      erase flash size 8MiB
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      program 8MiB elapse 2164ms
      read 8MiB elapse 653ms
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      erase 8MiB elapse 333ms
      ```

      



- vega spinand速度

  - 50MHz

    - 双倍速

      ```
      boot> flash speedtest
      erase flash size 8MiB
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      program 8MiB elapse 3069ms, 2669 KB/S
      read 8MiB elapse 1214ms, 6747 KB/S
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      erase 8MiB elapse 364ms, 22505 KB/S
      ```

    - 四倍速

      ```
      boot> flash speedtest
      erase flash size 8MiB
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      program 8MiB elapse 2141ms, 3826 KB/S
      read 8MiB elapse 874ms, 9372 KB/S
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      erase 8MiB elapse 364ms, 22505 KB/S
      ```

  - 74MHz

    - 双倍速

      ```
      boot> flash speedtest
      erase flash size 8MiB
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      program 8MiB elapse 2588ms, 3165 KB/S
      read 8MiB elapse 989ms, 8283 KB/S
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      erase 8MiB elapse 364ms, 22505 KB/S
      ```

    - 四倍速

      ```
      boot> flash speedtest
      erase flash size 8MiB
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      program 8MiB elapse 2019ms, 4057 KB/S
      read 8MiB elapse 849ms, 9648 KB/S
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      erase 8MiB elapse 364ms, 22505 KB/S
      ```

  - 37MHz

    - 双倍速

      ```
      boot> flash speedtest
      erase flash size 8MiB
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      program 8MiB elapse 3553ms, 2305 KB/S
      read 8MiB elapse 1443ms, 5677 KB/S
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      erase 8MiB elapse 364ms, 22505 KB/S
      ```

    - 四倍速

      ```
      boot> flash speedtest
      erase flash size 8MiB
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      program 8MiB elapse 2263ms, 3619 KB/S
      read 8MiB elapse 991ms, 8266 KB/S
      Erasing at 0x007e0000 -- 100% complete.
      erase OK
      erase 8MiB elapse 364ms, 22505 KB/S
      ```

      



- Vega SPINor 性能测试

  - DMA读、位宽8bit

    - 75MHz

      - 四倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 27672ms, 592 KB/S
        read 16MiB elapse 654ms, 25051 KB/S
        erase 16MiB elapse 54635ms, 299 KB/S
        ```

      - 双倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 28333ms, 578 KB/S
        read 16MiB elapse 913ms, 17945 KB/S
        erase 16MiB elapse 54676ms, 299 KB/S
        ```

    - 50MHz

      - 四倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 27977ms, 585 KB/S
        read 16MiB elapse 688ms, 23813 KB/S
        erase 16MiB elapse 54458ms, 300 KB/S
        ```

      - 双倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 28358ms, 577 KB/S
        read 16MiB elapse 1365ms, 12002 KB/S
        erase 16MiB elapse 54505ms, 300 KB/S
        ```

        

  - DMA读、位宽32bit

    - 75MHz

      - 四倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 27882ms, 587 KB/S
        read 16MiB elapse 855ms, 19162 KB/S
        erase 16MiB elapse 54803ms, 298 KB/S
        ```

      - 双倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 26831ms, 610 KB/S
        read 16MiB elapse 1115ms, 14694 KB/S
        erase 16MiB elapse 54798ms, 298 KB/S
        ```

    - 50MHz

      - 四倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 28182ms, 581 KB/S
        read 16MiB elapse 889ms, 18429 KB/S
        erase 16MiB elapse 55069ms, 297 KB/S
        ```

      - 双倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 27767ms, 590 KB/S
        read 16MiB elapse 1567ms, 10455 KB/S
        erase 16MiB elapse 55061ms, 297 KB/S
        ```

        

  - CPU 读、位宽 8bit

    - 75MHz

      - 四倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 28307ms, 578 KB/S
        read 16MiB elapse 3503ms, 4677 KB/S
        erase 16MiB elapse 54980ms, 297 KB/S
        ```

      - 两倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 28338ms, 578 KB/S
        read 16MiB elapse 3503ms, 4677 KB/S
        erase 16MiB elapse 54955ms, 298 KB/S
        ```

    - 50MHz

      - 四倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 28323ms, 578 KB/S
        read 16MiB elapse 3503ms, 4677 KB/S
        erase 16MiB elapse 55217ms, 296 KB/S
        ```

      - 双倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 28353ms, 577 KB/S
        read 16MiB elapse 3503ms, 4677 KB/S
        erase 16MiB elapse 54918ms, 298 KB/S
        ```

        

  - CPU 读、位宽 32bit

    - 75MHz

      - 四倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 25879ms, 633 KB/S
        read 16MiB elapse 932ms, 17579 KB/S
        erase 16MiB elapse 54958ms, 298 KB/S
        ```

      - 双倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 26840ms, 610 KB/S
        read 16MiB elapse 933ms, 17560 KB/S
        erase 16MiB elapse 55097ms, 297 KB/S
        ```

    - 50MHz

      - 四倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 25892ms, 632 KB/S
        read 16MiB elapse 932ms, 17579 KB/S
        erase 16MiB elapse 55112ms, 297 KB/S
        ```

      - 双倍速

        ```
        boot> flash speedtest
        erase flash size 16MiB
        program 16MiB elapse 27761ms, 590 KB/S
        read 16MiB elapse 1356ms, 12082 KB/S
        erase 16MiB elapse 55118ms, 297 KB/S
        ```

        

- 四倍速两边抓波形看一下
- 不管数据对错，把flash_spi.c中的大小端转换去掉







# 添加 Sample_delay 自动测试程序

- gxloader
  - 添加一个回调`spi_config`，用于配置`Mode、clk_div、sample_delay`
  - 添加测试程序，调用 gxmisc 提供的接口多次修改参数后进行读写擦来测试相位范围
  - [ ] 找出来最好的相位之后，直接配给 SPI，然后后续还可以进行读写擦测试
- gxmisc
  - 添加一个接口，调用 gxloader 提供的 `spi_config` 配置参数
  - [x] mode 不通过结构传进去，作为一个参数，传进去之后在 config 里面再赋值





# GX_SPI 测试用例

- 没有中断
- 没有相位
- 没有四倍速
- [ ] 去掉增强模式



# sirius、canopus 添加 stage1 读 stage2 时判断 从 0 地址到 addr + len 结束的所有区间内跳坏块功能

- sirius、canopus 的 stage1、stage2 都是放在 block0
- vega 的 stage1 放在 block0，block 1 空闲，block 2 放 stage1 备份，block 3 放 stage2
- 目前 stage1 读 stage2 的代码是只判断 stage2 的 addr 到 addr+len 区间内是否有坏块，进行跳坏块；若 block 0 或 block 1、block 2 中存在坏块的时候则会导致读取 stage2 读到了 block3 + 坏块 的数据



- vega 已经有了这个代码
- sirius、canopus 新加的这个代码，在 `script` 目录下找一下 canopus 如何制作的 bin，stage2 的地址手动改到后面的 block，并且 copy.c 中读 stage2 的地址也要变，然后在 loader 中手动制造坏块，看功能能否正常





# 测试 spi dma 关掉刷 cache 的速度

- 关掉之后只快了不到 1M 的速度，说明速度的影响不在刷 cache
- cpu 32bit 位宽 75MHz 工作频率下四倍速的频率是 35MHz，dma 8bit 位宽 75MHz 工作频率下四倍速的频率是 25MHz







# Scorpio

- [ ] 文档中还有 GX_SPI 的中断号，不是说没有 gx_spi 的中断吗？

  - [ ] 需要测一下

- [ ] dw_spi 的中断直接用会卡住，加几个打印就不会卡住，cpu 的频率是多少，spi 的频率是多少？

  - [ ] spi 频率是 27MHz，是由于编译器优化了指令

- [x] 确认一下 dw_spi 的 fifo 是多大
  - [x] tx_fifo:32 rx_fifo_64

- [ ] GX_SPI 速度

  - [ ] 双倍速：6分频

    ```
    boot> flash speedtest
    protect len: 0x0
    erase flash size 16MiB
    program 16MiB elapse 74226ms, 220 KB/S
    dual!
    read 16MiB elapse 32904ms, 497 KB/S
    erase 16MiB elapse 36278ms, 451 KB/S
    ```

  - [ ] 单倍速：6分频

    ```
    boot> flash speedtest
    protect len: 0x0
    erase flash size 16MiB
    program 16MiB elapse 74116ms, 221 KB/S
    standard!
    read 16MiB elapse 48322ms, 339 KB/S
    erase 16MiB elapse 36013ms, 454 KB/S
    ```

  - [ ] 单倍速：4分频

    ```
    boot> flash speedtest
    protect len: 0x0
    erase flash size 16MiB
    program 16MiB elapse 62067ms, 263 KB/S
    standard!
    read 16MiB elapse 36439ms, 449 KB/S
    erase 16MiB elapse 36309ms, 451 KB/S
    ```

    

- gdb 调试失败

  ![选区_281](image/选区_281.png)



# Stage1 时钟降低到 30MHz

- 目前 Stage1 使用的是 03 指令，很多 Nor Flash 的最低要求频率是 50MHz
- ai 那边的 Stage1 使用的是 0b 指令，不限制频率
- 确认机顶盒这边的 Stage1 驱动是使用 0b 还是降低频率





# Virgo

- FMC

  - [x] 目前开启 DMA 的速度和不开启 DMA 的速度对比：
  - [x] DMA 32bit 位宽的情况下，需要手动转换大小端，由于 CPU 传输的时候硬件转换了大小端，DMA 32bit 位宽的时候没有转换，需要软件转

  - [ ] 需要一个寄存器，用于保存 `spi->mode`
  - [x] 现在的 SPI、CPU 频率分别是多少
  - [ ] cs 怎么切换？不是有两个 cs 吗？



- DMA
  - [x] 熟悉 SCPU 中的 AXI 驱动
  - [ ] 熟悉 Linux 中的 DMA 驱动
  - [x] 确认是否支持 `S/G` 的方式
    - [x] 支持，通过CHx_CFG2 寄存器的`[1:0]` 配置
      - `00:contiguous`
      - `01:Reload`
      - `10:Shadow Register`
      - `11:Linked List`



- SPI
  - [ ] 查看 Linux 中如何实现的多个 SPI





# ZB25VQ128D 开关机数据有跳变

- gxloader 已经 commit --amend，需要 `repo upload .`
- gxmisc 还没 commit --amend
- ecos 也没 commit--amend



# 同步 develop 分支里面 gxloader、ecos、linux 的补丁到 release



# Stage1 搬运 Stage2 使用四倍速







# pavo 131 测试方法

```
./pavo131 -i 2 1 2 4 -p 1000
```



- [x] gpio reset pavo131
- [ ] get_long
- [x] 补丁





































- 138M 没跑过
- 180M 没跑过
- 159M 没跑过





100M 24M

100M 27M 

108M 24M NO

108M 27M

120M 24M

120M 27M NO

130M 24M

130M 27M

140M 24M NO

140M 27M NO

60M

160M NO
159M NO
138M NO
150M
157.5M
180M NO
129M





# ZB25VQ128D 掉电擦除测试程序

- 使用 64K 擦除的时候会出现问题，结果是导致有数据跳变 0->1
  - 这里的擦除地址是多少，数据跳变的地址是多少？
  - 原因是什么导致的数据跳变？
- 使用 4K 擦除的时候不会出现问题



咨询一下开关机数据跳变的问题，当时的现象是用户在进行开机之后使用64K擦除的方式，擦除了2个block，然后等擦除操作完成之后掉电，再重启；重复这样的操作，发现有其它 block 的数据发生了跳变(0->1)导致无法启动。

添加了写保护功能之后，开机打开写保护，擦除的时候关掉写保护，擦除完成继续开启写保护，不会导致其它 block 的数据发生跳变

换成 4K 擦除的方式，仍然擦除 2 个block，等擦除操作完成之后掉电，再重启；重复这样的操作，不会导致其它 block 的数据发生跳变。



对此，你们给出了一个 refresh all(单个阱全刷新机制)，使用 64k 擦除的时候没有触发到这个机制，使用 4k 擦除的时候触发到了这个机制。

麻烦解释一下为什么 64k 不会触发，4k 会触发？和擦除的 block 数量、block 地址有关系吗？







# Aquila

- 小板、电源、cklink



- ## 359761

- SDR 指的是什么？手册描述的是 STR，SDR 就是 single
- DDR 是新的 ddr 模式，只有在 dual、quad 的模式下在支持，DDR 就是 Double
- 为什么使用驱动中的汇编读写？
  - 目前 read 的时候会出现 Receive fifo overflow. 这是由于 spi 数据出的过快，cpu 来不及去搬导致的，使用汇编能够加快 cpu 读取速度，就不会出现 Receive fifo overflow.
- 驱动和测试程序要怎么弄？现在的驱动都是放在 drv-apus 目录下的，是要移到 drv-aquila 目录下面去吗？测试程序也是 depends on apus 的
  - 驱动是要移植一份到 drv-aquila 下面
  - 测试程序 depens on apus || depens on aquila



http://git.nationalchip.com/redmine/issues/371242

- 编译方法：

  ```
  make aquila_riscv_fpga_1v_defconfig
  make
  ```

- 使用 gdb load .elf(默认已经开启了 Jtag)

  - .gdbinit 脚本：

    ```
    target remote 127.0.0.1:1025
    
    # 启动 core1
    set *0xa3500014=0xf4000000
    
    # core1 复位地址设置为 0x10000000
    set *0xa3700004=0x10000000
    ```

  - 运行：

    ```
    DebugServerConsole
    riscv64-unknown-elf-gdb 
    ```

    









![选区_333](image/选区_333.png)





# 时间节点：

## Aquila MPW：3 月底

- 片内 Flash SPI0：XIP、DMA
- 片外 Flash SPI1：安全



## 直播星 Nand 兼容性测试：3 月 20 日

- SPI Nand BGA24 封装兼容性测试
- PPI Nand XTX BGA24 封装兼容性测试



## Scorpio Final：3 月 15 日

- 功能测试、性能简单测一下
- ROM 4 字节 Nor Flash 启动，Ecos 性能测试
- 兼容性测试不需要测
- 测试用例换成 Excel







[已完成]# SPINAND 支持 OTP、UID 样片 大厂


# 钰创、Skyhigh SPINand

- 如果 ECC 关不掉，那么在标记坏块的时候是需要关掉 ECC 写的，这个时候 oob 的数据写的进去吗？
- 做 singletest 读写擦 main 区域数据，然后查看 oob 区域的数据是多少
- Skyhigh 写完 main 区域，对应 page 的 oob 区域有数据，不是 0xff
```diff
--- a/bootmenu.c
+++ b/bootmenu.c
@@ -1420,6 +1420,13 @@ void command_flash(int argc, const char **argv)
        else if (strcmp(argv[1], "ecctest") == 0) {
                flash_nand_ecc_test();
        }
+        else if (strcmp(argv[1], "skyhigh") == 0) {
+                                                                unsigned int ram_addr = 0x20000;
+                       gxflash_erasedata(0x0, 0x20000);
+                       gxflash_pageprogram(0x0, (unsigned char *)ram_addr, 0x10);
+                       gxflash_readoob(0x0, (unsigned char *)ram_addr, 0x90);
+       }
+
```

```shell
boot> flash scrub all

boot> flash badinfo


boot> flash singletest 0x300000 0x10  
sflash_gx_readl finish.
Erasing at 0x00300000 -- 100% complete.
erase OK
sflash erase finish.
read addr=0x300000, len=0x10
sflash_gx_readl finish.
sflash page program finish.
sflash_gx_readl finish.
cmp finish. src equal dst.

boot> flash read 0x300000 0x20000 0x20
29 23 be 84 e1 6c d6 ae 52 90 49 f1 f1 bb e9 eb 
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 

boot> flash skyhigh
Erasing at 0x00000000 -- 100% complete.
erase OK

boot> flash read 0x0 0x200000 0x20

boot> flash oobread 0x0 0x200000 0x20

```
目前两种测试 case：都会导致 oob mark bad 区域不是 0xff
1. 0x10 有效数据 + 2k-0x10 0xff + 128 0xff，读 oob 区域 128 字节都有数据
2. 0x10 有效数据 + 1k-0x10 0xff，读 oob 区域 前64字节 有数据

你好！咨询一下关于 S35ML02G300WHI000 SPI Nand 产品相关问题，目前在调试这颗 Flash 时发现以下几个问题，麻烦解答一下。
1. 这款颗粒有 2 个 plane，在使用的时候什么时候用 plane1，什么时候用 plane2？如何选择 plane？手册描述通过 BA[0] 来选择 plane，是在发 0x13/0x10 的时候把 PA[6] 配成 0/1 选择 palne1/2 ?
    [用户不需要区分]
2. 颗粒默认写保护，并且配置写保护的几个 bit 位都是易失性的，去掉写保护的流程如下(是否需要修改?)：如果 0xA0 寄存器的值是 0x78，bit[1] 配1, bit[6:3] 清零，bit[1] 配0
3. UID 获取方法：在发 0x13 的时候 page_id 需要配置成 0x180？长度是 16 字节
    [是的]
4. OTP 区域操作方法：在发 0x13 的时候 page_id 需要配置成 0x182? 还是配成 0 就行?
5. 手册未描述 oob layout
    [128byte]
6. 钰创的颗粒 EM73D044VCS-H ID 等各种信息都和这款颗粒一致，是他们买的你们的 die 吗？
7. ecc 不能关掉，在读坏块标记的时候，如果读 ecc 的状态失败，你们描述是此时数据不可信，那么 oob 区域的坏块标记数据也是不可信的，那么怎么做坏块标记？
    也不能擦除掉，擦除了 oob 区域又都变成了 0xff
8. 出厂坏块，表示 ecc 无法纠错，那么 ecc 状态肯定是 失败，那么此时 oob 区域的数据不可靠

>>>>>sflash_test:addr=0x66b8730, len = 0xdf9a00
sflash_gx_readl finish.
Erasing at 0x074a0000 -- 100% complete.
erase OK
sflash erase finish.
read addr=0x66b8730, len=0xdf9a00
sflash_gx_readl finish.
sflash page program finish.
sflash_gx_readl finish.
cmp error. i = 0. read=0xff, orig=0x85
*****sflash_test_failed:addr=0x66b8730, len = 0xdf9a00, times=37

>>>>>sflash_test:addr=0x487c496, len = 0xf70000
sflash_gx_readl finish.
Erasing at 0x057e0000 -- 100% complete.
erase OK
sflash erase finish.
read addr=0x487c496, len=0xf70000
sflash_gx_readl finish.
sflash page program finish.
sflash_gx_readl finish.
cmp error. i = 0. read=0xff, orig=0xd9
*****sflash_test_failed:addr=0x487c496, len = 0xf70000, times=17


#  Aquila 测试 XIP

- 使用地址 0x14000000 测试 xip
- 先同步最新代码，使用最新 bit 文件，将之前的代码提交拉下来，使用提交的修改测试
- 需要在多个核上分别测试 Flash SPI0、Flash SPI1


bus mode  : QUAD
data width: 32bits
dma       : Disable
ddr       : Disable

********** SINGLE TEST START *********
>>>>>single test:addr=0x0, len = 16
sflash_gx_readl finish 16.
>>>>>>>>>erase addr = 0x0, erase len  = 0x10
sflash erase finish 4096.
read addr=0x0, len=16
sflash_gx_readl finish 16.
sflash page program finish 16.
sflash_gx_readl finish 16.
cmp error. i = 0. read=0x3b, orig=0xb3
cmp error. i = 4. read=0x7, orig=0x87
cmp error. i = 12. read=0xe, orig=0x6
cmp finish. src equal dst.
*****sflash_test_failed:addr=0x0, len = 16

使用 aquila_top_k7_bit0_03-06.bit 测试 Flash SPI(0xAA000000) 32bit 的时候会出现数据读取出错的情况，用之前的 bit 不会，同样的环境用 8bit 也不会
bus mode  : QUAD
data width: 32bits
dma       : Disable
ddr       : Disable

********** SINGLE TEST START *********
>>>>>single test:addr=0x0, len = 16
sflash_gx_readl finish 16.
>>>>>>>>>erase addr = 0x0, erase len  = 0x10
sflash erase finish 4096.
read addr=0x0, len=16
sflash_gx_readl finish 16.
sflash page program finish 16.
sflash_gx_readl finish 16.
cmp error. i = 0. read=0x22, orig=0xa2
cmp error. i = 4. read=0xa, orig=0x2
cmp error. i = 8. read=0x83, orig=0x8b
cmp error. i = 12. read=0xd, orig=0x5
cmp finish. src equal dst.
*****sflash_test_failed:addr=0x0, len = 16

看起来好像是每 4 字节会出错一次，并且用逻辑分析仪抓从 flash 出来的数据都是对的
[新 bit 文件测试通过]

# Skyhigh SPINand 测试

- enable/disable ecc 时不发指令，仅改变状态 `chip->ecc_status` 确认读写擦、坏块标记等操作是否正常

# aquila 测试

## dma

- 目前文档只描述了 dma0，没有描述 dma1、dma2，i2c 需要配置用户寄存器来选择 rx/tx
- 需要添加文档用于描述 i2c
- 驱动需要添加其余 i2c 配置
* 测试进度： **aquila_top_FPGA_2024-03-20bit0.bit**

* 测试进度： **aquila_top_FPGA_2024-03-20bit0.bit**

  ```|DMAx|核|功能|测试结果|备注|
  |/33. DMA0 (0xA4000000)|
  |/8. bt 核|不同内存区域的DMA传输|通过||
  |不同的DMA传输宽度|通过||
  |不同源和目的msize的DMA传输|通过||
  |不同master的DMA传输|通过||
  |不同channel 的DMA传输|通过||
  |DMA中断|失败||
  |Flash使用DMA读写数据|通过||
  |flash spi xip 地址到内存的DMA传输|通过||
  |/8. sensor 核|不同内存区域的DMA传输|通过||
  |不同的DMA传输宽度|通过||
  |不同源和目的msize的DMA传输|通过||
  |不同master的DMA传输|通过||
  |不同channel 的DMA传输|通过||
  |DMA中断|通过||
  |Flash使用DMA读写数据|通过||
  |flash spi xip 地址到内存的DMA传输|通过||
  |/8. ap 核|不同内存区域的DMA传输|通过||
  |不同的DMA传输宽度|通过||
  |不同源和目的msize的DMA传输|通过||
  |不同master的DMA传输|通过||
  |不同channel 的DMA传输|通过||
  |DMA中断|通过||
  |Flash使用DMA读写数据|通过||
  |flash spi xip 地址到内存的DMA传输|通过||
  |/8. secure 核|不同内存区域的DMA传输|通过||
  |不同的DMA传输宽度|通过||
  |不同源和目的msize的DMA传输|通过||
  |不同master的DMA传输|通过||
  |不同channel 的DMA传输|通过||
  |DMA中断|失败||
  |Flash使用DMA读写数据|通过||
  |flash spi xip 地址到内存的DMA传输||需要用 secure 核的 flash spi 进行测试，目前无法测试|
  ```

  



```
|/33. DMA1 (0xC2000000)|
|/8. bt 核|不同内存区域的DMA传输|通过||
|不同的DMA传输宽度|通过||
|不同源和目的msize的DMA传输|通过||
|不同master的DMA传输|失败|dma1 只能用 Master2 访问内存|
|不同channel 的DMA传输|通过||
|DMA中断|失败||
|sspi0 使用DMA读写数据|通过||
|flash spi xip 地址到内存的DMA传输|通过||
|/8. sensor 核|不同内存区域的DMA传输|通过||
|不同的DMA传输宽度|通过||
|不同源和目的msize的DMA传输|通过||
|不同master的DMA传输|失败|dma1 只能用 Master2 访问内存|
|不同channel 的DMA传输|通过||
|DMA中断|通过||
|sspi0 使用DMA读写数据|通过||
|flash spi xip 地址到内存的DMA传输|通过||
|/8. ap 核|不同内存区域的DMA传输|通过||
|不同的DMA传输宽度|通过||
|不同源和目的msize的DMA传输|通过||
|不同master的DMA传输|dma1 只能用 Master2 访问内存||
|不同channel 的DMA传输|通过||
|DMA中断|通过||
|sspi0 使用DMA读写数据|通过||
|flash spi xip 地址到内存的DMA传输|通过||
|/8. secure 核|不同内存区域的DMA传输|通过||
|不同的DMA传输宽度|通过||
|不同源和目的msize的DMA传输|通过||
|不同master的DMA传输|失败|dma1 只能用 Master2 访问内存|
|不同channel 的DMA传输|通过||
|DMA中断|失败||
|sspi0 使用DMA读写数据|通过||
|flash spi xip 地址到内存的DMA传输|失败|需要用 secure 核的 flash spi 进行测试，目前无法测试|
```








## 驱动优化

#### ![选区_418](image/选区_418.png)



![选区_419](image/选区_419.png)



支持方莹调试 slt 板四线无法启动问题，硬件 HOLD 引脚虚焊导致

峰哥，以下是我本周芯片验证进度：

1. ate 测试使用 usb-i2c 工具能够进入测试模式、退出测试模式；使用板子上的 i2c 能够进入测试模式，不能退出测试模式(主机发地址，system i2c 不应答)；单线是通的，但是四线需要退出测试模式所以没法测，需要芯片改 system i2c
2. 2分频 ddr 模式读操作会出现 控制器 busy 卡死的情况，芯片出了新的 bit 但是无法读到 flash id，需要再重新出 bit






















```diff
--- a/drivers/hal/src/padmux/aquila/gx_hal_padmux_aquila.c
+++ b/drivers/hal/src/padmux/aquila/gx_hal_padmux_aquila.c
@@ -54,8 +54,10 @@ hal_status gx_hal_padmux_set(GX_HAL_PADMUX *dev, int pad_id, int function)
        } else {
                if (pad_id >= AO_PINMUX_ID_OFFSET) {
                        WRITE_REG((uint32_t)dev->ao_pinmux_regs + ((pad_id - AO_PINMUX_ID_OFFSET) * 4), function - IOMATRIX_OFFSET);
+               } else if (pad_id >= 32){
+                       WRITE_REG((uint32_t)dev->pinmux_regs + ((pad_id - 32) * 4), IOMATRIX_FUNC);
                } else {
-                       WRITE_REG((uint32_t)dev->pinmux_regs + (pad_id * 4), IOMATRIX_FUNC);
+                       WRITE_REG((uint32_t)0xa3900000 + (pad_id * 4), IOMATRIX_FUNC);
                }
                WRITE_REG((uint32_t)dev->iomatrix_regs + (pad_id * 4), function - IOMATRIX_OFFSET);
        }
        
        

```











# ZB PNand

- 读写擦出错后，无法启动，只打印 xxx
- 直接 dump 数据，发现出现 ecc 无法纠错，查看 dump 出来的数据有跳变



- 用 develop 分支的 boot dump ，很多跳变

- 用 develop 分支的 boot，不开启 ecc，dump数据，依然很多跳变

- 烧 .boot 进出，不开 ecc 测试 block0，

  ```
  sflash_gx_readl finish.
  cmp error. i = 2114. read=0xb8, orig=0xba
  
  flash singletest 0x0 0x10000
  cmp error. i = 18035. read=0x83, orig=0x8b
  
  cmp error. i = 16385. read=0xd2, orig=0xd3
  ```

- 测试其它 flash，不开 ecc 测试 block0是否成功

  - gd ：测试 block0 读写多次，不会出错，直接进行 comtest 测试，明天看结果，测试 117次才会出错

    ```
    cmp error. i = 6134592. read=0x8c, orig=0x8d
    *****sflash_test_failed:addr=0x3546c9c, len = 0xd88200, times=117
    ```

- 再补测 zbit 新的片子，不开 ecc，直接测试 block0，以及 comtest

  - 新片子不开 ecc 测试 block0 读写擦多次正常

- 原因：读写擦测试3000次左右之后，无法启动。是由于 block0 会产生 bit 跳变，而rom 读启动代码时是没有 ecc 的，无法纠错跳变，导致无法启动







# 单线模式，tx fifo 深度修改

```diff
diff --git a/board/nationalchip/aquila_fpga_1v/include/board_config.h b/board/nationalchip/aquila_fpga_1v/include/board_config.h
index 97467797..ca510023 100644
--- a/board/nationalchip/aquila_fpga_1v/include/board_config.h
+++ b/board/nationalchip/aquila_fpga_1v/include/board_config.h
@@ -81,7 +81,7 @@
 
 /* SPI FLASH */
 #define CONFIG_FLASH_SPI_CLK_SRC        48000000
-#define CONFIG_SF_DEFAULT_CLKDIV        4  /* 分频数必须为偶数且非0 */
+#define CONFIG_SF_DEFAULT_CLKDIV        64 /* 分频数必须为偶数且非0 */
 #define CONFIG_SF_DEFAULT_SAMPLE_DELAY  1
 #define CONFIG_SF_DEFAULT_SPEED         (CONFIG_FLASH_SPI_CLK_SRC / CONFIG_SF_DEFAULT_CLKDIV)
 
diff --git a/cmd/flash/spi_nor_flash_test.c b/cmd/flash/spi_nor_flash_test.c
index e2557afe..fb15ca3f 100644
--- a/cmd/flash/spi_nor_flash_test.c
+++ b/cmd/flash/spi_nor_flash_test.c
@@ -1460,7 +1460,7 @@ static int do_nor_test(cmd_tbl_t *cmdtp, int flag, int argc,
        }
        else if (strcmp(cmd, "case") == 0)
        {
-               uint8_t bus[3] = {GX_HAL_SF_SPI_QUAD, GX_HAL_SF_SPI_DUAL, GX_HAL_SF_SPI_STAND};
+               uint8_t bus[3] = {GX_HAL_SF_SPI_QUAD, GX_HAL_SF_SPI_DUAL, GX_HAL_SF_SPI_DUAL};
                uint8_t data_32bit_en[2] = {1, 0};
                uint8_t dma_en[2] = {1, 0};
                uint8_t is_test_ddr = 0;
diff --git a/drivers/drv-aquila/spi_nor_flash.c b/drivers/drv-aquila/spi_nor_flash.c
index 37b1ef3c..66cf6529 100644
--- a/drivers/drv-aquila/spi_nor_flash.c
+++ b/drivers/drv-aquila/spi_nor_flash.c
@@ -689,7 +689,11 @@ static int gx_nor_erase_sector(uint32_t pos)
        while(gx_hal_sf_is_tx_done(&spi_nor_dev) == 0);
        while(gx_hal_sf_is_bus_busy(&spi_nor_dev));
 
+
+       gx_hal_spi_cs_control(&hal_dws_spi, 0, GX_HAL_SPI_ENABLE);
        status = gx_hal_sf_erase_sector(&spi_nor_dev, pos);
+       gx_hal_spi_cs_control(&hal_dws_spi, 0, GX_HAL_SPI_DISABLE);
+       gx_hal_spi_cs_init(&hal_dws_spi, 0);
        /// wait tx done
        while(gx_hal_sf_is_tx_done(&spi_nor_dev) == 0);
        while(gx_hal_sf_is_bus_busy(&spi_nor_dev));
@@ -714,7 +718,10 @@ static int gx_nor_erase_block(uint32_t pos)
        while(gx_hal_sf_is_tx_done(&spi_nor_dev) == 0);
        while(gx_hal_sf_is_bus_busy(&spi_nor_dev));
 
+       gx_hal_spi_cs_control(&hal_dws_spi, 0, GX_HAL_SPI_ENABLE);
        status = gx_hal_sf_erase_block(&spi_nor_dev, pos);
+       gx_hal_spi_cs_control(&hal_dws_spi, 0, GX_HAL_SPI_DISABLE);
+       gx_hal_spi_cs_init(&hal_dws_spi, 0);
        /// wait tx done
        while(gx_hal_sf_is_tx_done(&spi_nor_dev) == 0);
        while(gx_hal_sf_is_bus_busy(&spi_nor_dev));
@@ -906,12 +913,16 @@ uint32_t gx_nor_read(GX_HAL_SF_RW_T *r)
        /// wait bus idle
        while(gx_hal_sf_is_bus_busy(&spi_nor_dev));
 
+       gx_hal_spi_cs_control(&hal_dws_spi, 0, GX_HAL_SPI_ENABLE);
+
        if(gx_hal_sf_read_prepare(&spi_nor_dev, r) != 0)
        {
                status = -1;
                goto __FUNC_RET__;
        }
        status = gx_hal_sf_spi_read(&spi_nor_dev, r);
+       gx_hal_spi_cs_control(&hal_dws_spi, 0, GX_HAL_SPI_DISABLE);
+       gx_hal_spi_cs_init(&hal_dws_spi, 0);
 
        while(gx_hal_sf_is_bus_busy(&spi_nor_dev));
 
diff --git a/drivers/hal/src/spi/dw_spi/gx_hal_spi.c b/drivers/hal/src/spi/dw_spi/gx_hal_spi.c
index ce9389ad..cef7b5dd 100644
--- a/drivers/hal/src/spi/dw_spi/gx_hal_spi.c
+++ b/drivers/hal/src/spi/dw_spi/gx_hal_spi.c
@@ -408,11 +408,14 @@ int gx_hal_spi_transfer_prepare(struct gx_hal_dw_spi *dws, struct gx_hal_spi_tra
                dws->n_bytes = 1;
        }
 
+#if 0
        if(transfer->tx_len && transfer->trans_mode != GX_HAL_SPI_TRANS_TX_AND_RX) {
+//     if(transfer->tx_len && ((unsigned char *)(transfer->rx_buf))[0] == 0xD8) {
                gx_hal_dw_writel(dws, GX_HAL_SPI_TXFTLR,
                                (gx_hal_spi_min(dws->tx_fifo_len, transfer->tx_len/dws->n_bytes) - 1) << 16);
        }
        else
+#endif
                gx_hal_dw_writel(dws, GX_HAL_SPI_TXFTLR, 0);
        gx_hal_dw_writel(dws, GX_HAL_SPI_RXFTLR, 0);
```



- 修改：去掉

  ![选区_444](image/选区_444.png)

- 不修改分频，单线直接写大批量数据，看看会不会卡住

  - 擦除：发一个字节，cs 拉高，发一个字节 cs 拉高，发不出去正确的命令 (因为擦除是用的硬件 cs，改成软件 cs 写 64k 正确)

    ![image-20240428193123256](image/image-20240428193123256.png)

- 降低分频，做 comtest 看会不会卡住

  - 读出错，因为读是用的硬件 cs







# todo

- aquila 收尾

  - quad 的问题等芯片仿真
  - 单线 txfifo 配成 0

- esmt 开机慢

  ```
  ESMT:
  
  boot> flash speedtest
  erase flash size 8MiB
  Erasing at 0x007e0000 -- 100% complete.
  erase OK
  program 8MiB elapse 3070ms, 2668 KB/S     = 2.6054MB/S
  read 8MiB elapse 1040ms, 7876 KB/S        = 7.6914MB/S
  Erasing at 0x007e0000 -- 100% complete.
  erase OK
  erase 8MiB elapse 408ms, 20078 KB/S       = 19.6074MB/S
  
  
  Winbond:
  
  program 8MiB elapse 2826ms, 2898 KB/S      = 2.8300MB/S
  read 8MiB elapse 926ms, 8846 KB/S          = 8.6386MB/S
  Erasing at 0x007e0000 -- 100% complete.
  erase OK
  erase 8MiB elapse 309ms, 26511 KB/S        = 25.8896MB/S
  
  
  Forsses:
  
  boot> flash speedtest
  erase flash size 8MiB
  Erasing at 0x007e0000 -- 100% complete.
  erase OK
  program 8MiB elapse 2905ms, 2819 KB/S      = 2.7529MB/S
  read 8MiB elapse 914ms, 8962 KB/S          = 8.7519MB/S
  Erasing at 0x007e0000 -- 100% complete.
  erase OK
  erase 8MiB elapse 342ms, 23953 KB/S        = 23.3916MB/S
  
  
  1. 上电到 kernel 运行、读应用(出画面) 整 bin 总共有多大？
  2. usb 升级的 bin 有多大？
  ```

  

- virgo rom 调试

- virgo 测试用例

- 事业部出现一批板子，做高温老化测试，测试后有芯片挂掉，挂掉了更换了 主芯片，没有换 flash，寄给厂商出现 flash 数据变掉的情况。一颗是在 boot 区域，一颗是在根文件系统区域

  - 重新烧录正常
  - 需要做 comtest、掉电测试



1. `/usr/bin/T-HEAD_DebugServer` 目录下执行：`sudo ./DebugServerConsole.elf`
2. `work/Aquila` 目录下执行：`riscv64-unknown-elf-gdb -x gdbinit.core1.sensor ddr.elf`
   - `-x` ：指定用哪一个 gdbinit 文件，即指定跑哪个 cpu 核，core1 代表跑 1核
   - `ddr.elf`：应用程序
   - `load`：将应用程序加载到内存
   - `c`：开始跑应用程序





- aquila flash spi 驱动整理：

  - dll_en 参数内部消化掉，只提供 dll_delay

- aquila 多核：一个核跑 xip，一个核写 flash，写 flash 的驱动放在 sram

  - 安全核会要求写 flash 的，其它核都是通过 xip 来访问 flash 的

  ```
  海波，咨询一个 aquila flash spi 的问题，在多核的时候一个核跑 xip，另一个核 写 flash，写 flash 的代码是放在 sram 上的，这种用法会有问题吧？是会读到错的数据？还是xip读会卡住，等待写 flash 操作做完之后继续做 xip 读？
  因为实际的产品会用到这种情况，安全核写 flash，然后其它核都是通过 xip 来访问 flash 的。
  ```

  



- aquila spi  3-22.bit

  - xip memcpy 不是四字节对齐的时候，后面四个字节数据会出错

    - 关掉 xip fetch 就没问题了

    - 给芯片仿真的测试程序：

      ```c
      #include <driver/gx_spi_nor_flash.h>
      int main(int argc, char **argv)
      {
      	loader_init();
      
      	unsigned char aaa[256] = {0};
      	volatile unsigned char *xip = (volatile unsigned char *)CONFIG_FLASH_XIP_BASE;
      
      	gx_nor_init();
      
      	gx_nor_xip_init(GX_HAL_SF_SPI_QUAD, 0);
      
      	gx_nor_erase_data(0x0, 0x1000);
      
      	GX_HAL_SF_RW_T w = {0};
      	unsigned char w_buff[256] = {0x0};
      	int i;
      
      	for (i = 0; i < 256; i++) {
      		w_buff[i] = i;
      	}
      
      	w.addr = 0x0;
      	w.data = w_buff;
      	w.len = 0x10;
      	w.bus_mode = GX_HAL_SF_SPI_DUAL;
      	w.data_width_8or32bit = 0;
      	w.dma_en = 0;
      	w.espi_ddr_en = 0;
      	gx_nor_write(&w);
      
      
      	memcpy(&aaa, (void *)CONFIG_FLASH_XIP_BASE, 7);
      
      
      	printf("\n**************************** XIP **********************\n");
      	printf("xip_data[0]:0x%x, xip_data[1]:0x%x, xip_data[2]:0x%x, xip_data[3]:0x%x\n", xip[0], xip[1], xip[2], xip[3]);
      	printf("xip_data[4]:0x%x, xip_data[5]:0x%x, xip_data[6]:0x%x, xip_data[7]:0x%x\n", xip[4], xip[5], xip[6], xip[7]);
      	printf("xip_data[8]:0x%x, xip_data[9]:0x%x, xip_data[10]:0x%x, xip_data[11]:0x%x\n", xip[8], xip[9], xip[10], xip[11]);
      	printf("xip_data[12]:0x%x, xip_data[13]:0x%x, xip_data[14]:0x%x, xip_data[15]:0x%x\n", xip[12], xip[13], xip[14], xip[15]);
      
      	printf("\n\n");
      	printf("\n**************************** MEMORY **********************\n");
      
      	printf("dst_data[0]:0x%x,  dst_data[1]:0x%x,  dst_data[2]:0x%x,  dst_data[3]:0x%x\n",  aaa[0],  aaa[1],  aaa[2],  aaa[3]);
      	printf("dst_data[4]:0x%x,  dst_data[5]:0x%x,  dst_data[6]:0x%x,  dst_data[7]:0x%x\n",  aaa[4],  aaa[5],  aaa[6],  aaa[7]);
      	printf("dst_data[8]:0x%x,  dst_data[9]:0x%x,  dst_data[10]:0x%x, dst_data[11]:0x%x\n", aaa[8],  aaa[9],  aaa[10], aaa[11]);
      	printf("dst_data[12]:0x%x, dst_data[13]:0x%x, dst_data[14]:0x%x, dst_data[15]:0x%x\n", aaa[12], aaa[13], aaa[14], aaa[15]);
      
      
      
      	app_start();
      
      	while(1)
      		continue;
      }
      ```

      

  - flash 读接口，读 70k 数据会卡住

- 确认 zb 这款 nor flash 是否支持按 sector 保护







# 【已解决】SPI Nand boot 无法识别 markbad 标记的坏块

- .boot 不读坏块表，而是扫描整个 flash，再建立基于内存的坏块表

  ```
  init 时 malloc 了一个 buf，大小是 page_size
  creat_bbt(chip->buf)
  	disable_ecc
  	for(i = 0; i < 1024; i++)
  		读第一个 page 的 oob 区域
  		比较第一个字节是不是 0xff，如果不是则返回 -1，使用 bbt_mark_entry 标记为坏块
  			标记坏块：chip->bbt[block >>BBT_ENTRY_SHIFT] |= 0x3   b11
  		
  ```

- 启动阶段：

  ```
  从最后的 4 个block 中读取坏块表，获取坏块表所在的 page
  读取坏块表所在的 page，从第8个字节开始读
  magic 	version 	reserved 	length(小端) 	bbt[] 	crc
  Bbt0 	0x01 	0xff 	bbt字节长度（2byte） 	存放bbt数据 	4byte 
  就能获取到坏块表，然后每 2 bit 解析坏块表
  ```

  

- 扫描阶段：

  ```
  .boot 扫描不到坏块，
  block33 的 oob 区域数据为：
  0x2209600:      0xff    0x22    0xff    0xff    0xff    0xff    0xff    0xff
  第二个字节为 0x22，第一个字节为 0xff，但是 坏块表只会比较一个字节，所以认为是好块
  
  block34 的 oob 区域数据为：
  (gdb) x/20x 0x2209600
  0x2209600:      0xffffffff      0xffffffff      0xffffffff      0xffffffff
  0x2209610:      0xffffffff      0xffffffff      0xffffffff      0xffffffff
  0x2209620:      0xffffffff      0xffffffff      0xffffffff      0xffffffff
  
  Bad eraseblock 32 at 0x00400000
  Bad eraseblock 64 at 0x00800000
  Bad eraseblock 152 at 0x01300000
  Bad eraseblock 154 at 0x01340000
  Bad eraseblock 164 at 0x01480000
  Bad eraseblock 170 at 0x01540000
  Bad eraseblock 258 at 0x02040000
  Bad eraseblock 262 at 0x020c0000
  Bad eraseblock 282 at 0x02340000
  Bad eraseblock 298 at 0x02540000
  Bad eraseblock 515 at 0x04060000
  Bad eraseblock 1023 at 0x07fe0000
  ```

- 标记坏块：

  ```
  现在内存的坏块表中标记：
  chip->bbt[block >> BBT_ENTRY_SHIFT] |= msk;   0x01 标记
  
  由于是在 .boot 中标记，所以不会更新到 flash 的坏块表
  
  调用接口 chip->programpage 写到 所在的 block 的第一个 page 的 oob 区域，写成 0x00
  
  
  换成不是在 .boot 中标记，
  先在内存的坏块表中标记：
  
  标记完之后，找坏块表所在的 block，找到对应的 page，版本号 +1
  构建坏块表结构，然后从 chip->bbt[] 结构中找坏块表并复制到 buf
  擦除坏块表所在的 block，
  然后把构建好的坏块表，重新写入到 block，继续更新镜像表
  ```


- 原因：部分 flash 擦除完之后，如果写了 main 区域，再去写 oob 区域，可能无法写成功。所以坏块标记逻辑改成标记坏块前，先擦除 block，再标记坏块









# 【已解决】apus 使用 bootx 烧程序时出现无法解除写保护，查了之后发现是板子中的程序 spi 频率和相位不匹配，导致采样数据错误出现的







# Virgo FLASH SPI 模块验证

# BW25Q32ASIG、BW25Q64ASIG 掉电 bug ，厂商寄了样片需要重新测试





# 【已完成】BU1 需要在老的分支上加两款 Flash，一款没有样片，已经寄过来了，需要加补丁并在 develop 进行掉电测试







# 【已完成】至讯创新直播星上支持了一款 SPI Nand WSON8 封装，目前寄了新的 BGA24 封装，完整测试下







# HPD 要支持 SkyHigh S35ML01G3 SPI Nand，这款相同型号的 2G 产品之前验证存在问题，第一个 默认全部写保护，第二个 不能关 ecc，第三个 不关 ecc，四线读写擦出错





# 【已解决】APUS Gen_SPI2 修改输出频率

- 模块频率采用 PLL 96MHz，4分频，给到 SPI 模块的频率为 24MHz
- SPI 内部频率配置：`max_speed_hz = 24*1024*1024`

- 如何确认是否当前配置的分频比：`/scpu/drivers/hal/src/spi/dw_spi/gx_hal_spi.c:685` 打印下 div 的值是多少
- 正确的分配比应该是 4，输出时钟是 6MHz







# 【已解决】Virgo Flash_SPI 、Gen_SPI 修改 CS 寄存器，一个 CS 放一个寄存器

- 改成多个寄存器反而不方便



# 【已解决】AI Flash 支持列表 wiki 页面整理







# 【TODO】APUS Gen_SPI rx 分片

- APUS gen spi 异步 dma 接口：

  - TX：
    - 单线：只有多线模式才有时钟延展功能，不需要配置 CTRLR1 寄存器，所以可以直接一次把要写的数据全部写掉
    - 多线：开了时钟延展，需要配置 CTRLR1 寄存器，而 CTRLR1 每次只能配置 64K，如果要发送的数据很多，就要对发送的长度进行 64K 分片，每次用 DMA 传输 64K ，DMA 传输完成之后再配置下 CTRLR1 寄存器，继续传输 64K，直到传输完成

  - RX：
    - 单线/多线：都需要配置 CTRLR1 寄存器，最大是 64K，这里驱动没有做分片，最大只支持读 64K 数据
    - <font color=red>**todo：添加分片，参考 TX 操作**</font>





# Virgo Gen SPI 测试

- Master
  - 对接 flash：修改 flash spi 的基地址、cs、大小端，进行读写擦等各种测试
  - 对接 slave spi：用 gen spi master 驱动？为什么不用 flash spi 驱动，是因为 flash spi 驱动还要外接 flash，这样相当于 gen spi 的引脚上同时接了 flash 和 slave spi，会有干扰
- Slave
  - 对接 master spi：用 gen spi slave 驱动
  - 出现 master 取数据快，slave 放数据慢，slave 的模块输入频率需要是 slave 的 clk_out  的 6 倍或 8 倍
    - 这里 slave 的 clk_out 是由 master 组成的，所以 master 的输出频率要足够低

TXFLR：TXFIFO 中有多少数据

最多可以有 0x10 个数据

rx_len_mask = 0xf

items = 0x10 & 0xf = 0

items = 0x10 - 0 = 0x10;





# TODO

9.9

- [x] Skyhigh flash 验证

- [x] gxapi 接口，eCos Partition 读写擦跳坏块逻辑

9.10

- [x] gxapi 接口，eCos flash_api 读写擦跳坏块逻辑

9.11

- [ ] gxapi 接口，eCos flash_api 擦确认完，确认写、确认读

- [ ] CPU 配成 160MHz，2分频 80MHz 工作频率只有相位 3，其余的芯片有相位3、4、5
  - [ ] 用 SCPU 跑跑看
  - [ ] DP、DM 和 UART 复用了同一组管脚
  - [ ] P2_7  P2_6
  - [x] 无法识别 usb 设备
  
    ```
    tail -f /var/log/syslog
    cat /dev/kmsg
    ```

9.12

- [x] 客户的板子跑 80MHz，16MHz 都会卡死
  - [x] 80MHz 有 4 个相位，每个相位都会死
  - [x] 加电容，也会死，死在控制器 busy
  - [ ] 换芯片，看看会不会死
- [x] 平台的板子跑 80MHz，16MHz 都不会卡死



9.18 ~ 9.20

- Apus xip 问题
  - [ ] 对寄存器
- Gen spi 驱动整理
- Zbit ppi nand 无法启动问题
  - [ ] 对 block0 进行单独读写擦测试
  - [ ] 统计 log 中对 block0 操作的次数
  - [ ] 读写擦测试次数少一点，或者放到 develop 分支上读写擦测试

- Gxapi 读写擦跳坏块、判断是否是坏块接口
  - [ ] ecos 读写擦逻辑放到 gxapi 还是 gxmisc
  - [ ] linux 读写擦逻辑确认

- 不贴 flash 使用 bootx 烧写
  - rom 流程：是不是会先读下 flash，读不到数据才进入下载流程
  - 或者说按下 boot 键，会进入下载流程















- [ ] v1.9.6-4-lts 分支 ecos 读写擦接口的地址、长度打印
- [ ] 客户板子更换之前测试给的样片后再掉电测测看
- [x] 更新支持列表：添加 v3.2.0-tvos 分支



0x14000 0x1b6de



# 10.6

- [x] Gen_SPI 驱动完整测试，整理好格式

  - 需要测试 dma、flash、gen_spi

  ```
  Gen_SPI -- Flash
  
  Quad - No DMA        OK
  Quad - DMA           OK
  Dual - No DMA        OK
  Dual - DMA           OK
  Standard - No DMA    OK
  Standard - DMA       OK
  IRQ                  OK
  
  
  Gen_SPI -- Slave
  
  CPU                  OK
  DMA                  OK
  ASYNC_DMA            OK
  IRQ                  OK
  
  Gen_SPI -- Master
  
  CPU                  OK
  DMA                  OK
  ```

  - [x] 测试 flash_spi 驱动是否是好的

  ```
  Octal - DMA         OK
  Octal - CPU         OK
  Standard - CPU      OK
  ```

  

- [ ] PY25Q64HA 掉电测试无法复现客户的现场，先停掉，想办法再复现

- [x] 新的 bit 中 DMA 内存到内存无法测试通过，确认原因

  - [x] 芯片定位是外设桥的问题，等后续重新发版本

- [x] AXI DMA 驱动中的 debug 功能需要看一下



# 10.7

- [x] Apus 客户板子相位异常实际是由于国科芯片复位我们导致的，做案例分享
  - [x] 初版完成
- [x] Scpu 编译问题以及驱动 bug

- [x] 普冉烧录器烧录进去只有无法启动，有 bit 跳变，再次烧录ok
  - 全部都是最高 bit 发生 0 --> 1 的跳变，原数据 --> 新数据：`0x0a --> 0x8a, 0x22 --> 0xa2`
  - 有问题的 bin 是不是在同一个烧录器上烧录的？
  - 初步怀疑烧录器和 flash 没有接触好导致的，如果是不同的烧录器都会出现有问题的 bin，那可能和 flash 本身有关系



# 10.8

- [x] PY25Q64HA 开关机测试复现
  - [x] 国庆假期跑了七天之后，现在执行开关机测试会失败，直接启动试试看
    - [x] 直接启动也不行，重新烧录 bin 之后可以，应该 dump 出来看看的，这里忘记了，在继续跑开关机测试了，等复现了之后再 dump 吧
- [ ] 恒烁 PPI Nand 做几项测试
- [x] SPI Nand 增加跳坏块接口
  - [x] eCos：读写在 gxapi 跳坏块，擦在 ecos 跳坏块
  - [ ] Linux：读写擦都在 gxapi 跳坏块
    - [ ] 目录下的 bin 烧了之后无法挂载 rootfs 到 ubi 分区
      - ubifs 目录下先看：将设备树和 uImage 合并成了一个 kernel.img
        - 拆分开来，使用设备树和 uImage：ok 了
        - 不拆开，用 flash.conf 试试看，是 kernel.img 有问题
        - 与 flash.conf 无关，使用 nand_flash.conf 更换 kernel.img 也可以

- [x] 普冉烧录器烧录进去无法启动，有 bit 跳变，再次烧录 ok
  - [x] 让事业部寄样片过来测试看看，然后同步寄样片给 flash 厂商分析情况



# 10.9

- [ ] 提供标准测试方法流程到客户端，在客户板子上测试相位范围

- [x]  普冉烧录器烧录无法启动，有 bit 跳变，再次烧录 ok
  - [x] 单线、双线、四线 读写擦测试
    - [x] 单线、双线、四线测过，写保护测过
    - [ ] 掉电正在测
- [x] Apus 客户板子相位异常案例分享
  - [x] 完善语言以及细节

- [x] SPI Nand 增加跳坏块接口
  - [x] Linux 用 sirius 板子开始调试

- [x] Virgo IR、FMC、GEN_SPI、DMA 自测阶段测试结果评审



# 10.10

- [ ] PY25Q64HA 开关机测试失败了，dump 出来看下数据
  - [ ] dump 下来之后无法在 u 盘看到文件，换个 u 盘试试

- [x] Virgo IR、FMC、GEN_SPI、DMA 测试用例结果准备

- [ ] Linux 增加跳坏块读写擦接口
- [x] Apus 相位异常案例分享



# 10.11

- [ ] Virgo RC 测试
- [ ] Linux 增加跳坏块读写擦接口
  - [x] gxflash_read_mtd 的 ret 值错了？不是要读的长度？
    - [x] ret 配成 0 了
  - [x] 擦地址 0x80000，0x20000 之后读到 0 地址不是 0xff，而是 0xcf
    - [x] 擦的地址一直是 0 地址，用的 `loff_t offset` 导致上面传的参数下面收不到
  - [ ] 写会出错，在写的时候是按照 block 来写的，因为要跳坏块，导致 ret 的值一直是 block，上面需要的是整个长度
    - [ ] 很奇怪，不出了
  - [x] 随机长度会太大，应用程序 malloc 不出来，最大 malloc 16M 好了
  - [x] 跳坏块有问题，测试 `0x1de0000, 0x1020000` 的时候会跳 `0xb00000、0xb40000` 的坏块
    - [x] 这里好像是因为是按照 mtd 来的，所以起始地址会减去分区的起始地址
      - [x] 应用要操作：0x100000 , 0x400000
      - [x] 所在的分区在是第二个分区，第一个分区 0~0x100000
      - [x] 第二个分区 start_addr -= info->start_addr
      - [x] 首先找到地址所在的分区，起始地址 -= info->start_addr，然后做擦除操作
        - [x] 这里调到了 mtd->_isbad 又到了 mtdpart.c 中的 mtd_isbad ，在里面会先加上分区的偏移地址
    - [x] 换成用 fd 的 get_bad 来做，不用 gxmisc 中的杂项设备 ioctl 做试试看
      - [x] 确实应该用 fd 的 get_bad 来做，而不应该在 gxmisc 中加一个杂项设备
    - [x] 用 fd 的 getbadblock 拿到的是 -1?
      - [x] 是由于接口的参数要求是 `loff_t --> long long`，实际传入的是 `unsigned int`
    - [x] 看看 `nandwrite` 接口是怎么标记坏块和跳坏块的
    - [ ] 打印错了，这里要加上偏移地址才对
  - [ ] 其实不是跳坏块擦除有问题，而是read跳坏块有问题
    - [x] check_skip_len 接口用的是 gxmisc 中的跳坏块判断，需要换成 fd 的
    - [ ] read 是按照 block 读的，所以读的长度是 block，那么在跳坏块读成功的时候需要直接返回 *length
    - [ ] 读错数据，因为跳坏块是用的 fd 的方式跳的坏块，换一下
- [ ] PY25Q64HA 开关机测试失败 dump 数据，需要换 u 盘 dump 数据
- [x] ZB 烧录器有 bit 跳变，掉电测试
- [x] Virgo IR、FMC、GEN_SPI、DMA Beta 版本测试评审
- [x] 支持 Unim UM19A9HISW 提供修改驱动能力和获取驱动能力的接口



# 10.12

- [ ] Linux 跳坏块读写擦接口

- [ ] AXI_DMA Suspend 功能测试
  - [ ] 构建一个很大的 buffer 传输，然后没传完就 suspend 看看正不正常
- [ ] Apus bbt 板子烧写
  - [ ] 编译：`scpu/app/bbt/` ./build apus bbt
  - [ ] 烧写：leo_mini usb 烧，烧的时候按住 8008C 的boot 和 reset
  - [ ] 起来之后：先复位 leo_mini，再打开 apus 的电源，看 是否亮灯，再看apus 的串口和 gdb

- [ ] 

```
功能 #372655

功能 #372654: bootx 工具使用 hal 接口操作 Flash
apus 基于 hal 封装 bootx 需要调用的 Flash接口
```







# 10.14

- [x] PY25Q64HA 开关机测试失败 dump 数据，需要换 u 盘 dump 数据
  - [x] 换 u 盘 dump 数据发现和客户的现象不一致，都是 oem 分区数据有问题，并且不是 1 bit 有问题，很多字节数据有问题
  - [ ] 后面有时间再看

- [x] ZB 烧录器有 bit 跳变，寄过来的两颗 flash dump 数据发现都是一片一片的错，错的数据都是一种种的(前面错的数据，后面也会错成一样的数据)
  - [x] 和事业部贴在问题号上的 dump 数据不一样，贴的数据错都是 1bit 1bit 的错









# 10.15

- [x] gxapi spinand 驱动调试





# 10.16

- [x] gxapi 测试，已经调通
  - [ ] 完善驱动

- [x] 钰创不关 ecc 四线读写擦数据出错
- [ ] virgo rc

  - [ ] flash_spi

  - [ ] gen_spi

  - [ ] ir

  - [ ] dma
    - [ ] status 寄存器：产生中断状态

    - [ ] sig 寄存器：产生中断

    - [ ] 使能 status ，但不使能 sig，产生中断状态但不产生中断

- [x] Virgo Linux 启动失败
  - [x] 最新的 kernel，老的 rootfs

  - [x] 先不关心了，用最新的代码直接编都没问题



# 10.17

- [x] gxapi 调试
  - [x] 支持 nand flash 读写擦跳坏块、判断坏块接口

- [x] aijia 板子测试芯片的 clk 脚到 data 脚的延迟



# 10.18

- [x] gxapi 调试争取结束
- [x] aijia 板子测试芯片的 clk 脚到 data 脚的延迟





# 10.19

- [ ] gxapi 测试：

  ```
  eCos:
  	Nor      开 GXAPI 宏   otp test | info | comtest   OK  
  	Nor      关 GXAPI 宏                               OK
  	SPINand  开 GXAPI 宏                               OK
  	SPINand  关 GXAPI 宏   otp test | info | comtest   OK
  Linux:
  	SPINand  comtest                                  OK
  	PPINand  comtest                                  OK
  Nos:
  	SPINand                                           OK
  	PPINand                                           OK
  	
  	
  gxapi --> gxloader --> gxmisc
  如何验证 gxapi 的接口?
  	gxloader enable thirdlib 会链接 gxosal 库
  	gxloader 中调用 gxapi 的接口
  	gxloader 打开 ENABLE_THIRDLIB = y，然后编译
  	发生报错：需要先编译 前置库
  		thirdlib --> gxbus --> gxdownload --> gxapi --> gxfronted --> gxavdev
  		
      注释掉 thirdlib.mk 中链接的库，只保留要用的
      
      gxloader 需要编译 lib
      
      
      
  // todo: ecos_shell/prj/spi_cramfs_jffs2/source/spi_command.c
  	sflash_info_test 中的 id 需要考虑下怎么做  0x%x%x%x id[0],id[1],id[2]
  
  // todo: gxapi/api/os/linux/flash_api.c 中 GxFlash_BlockIsBad 接口的 addr 要不要减去分区的 start_addr
  减去之后，发生 error，ioctl 失败？换成 loff_t 就行了。。。
  ok
  
  Linux PPINand 没测到，补测 info 和 comtest 就行！！
  ok
  0xfc28599 + 0x3109a
  是由于 mtdpart 只有 128M，但是 flash 是 256M 的，导致size 过大
  Linux SPINand 再测下
  ok
  ```

- [ ] virgo rc

- [ ] Zbit PPI Nand



# 10.21

- [ ] Apus 相位问题

我有几个问题要问一下：

1. Apus 芯片可以在 85°C 正常跑吗？测试过吗？上次测试高低温是在 50°C 跑的，高低温试验箱还没跑过 85°C，他们用的时候最高好像就是 50/60°C

   重新给温度范围

2. 板子和 flash 的测试范围有哪些？目前客户的板子全部覆盖吗？目前支持的 flash 全测吗？

   内封的不用测，外挂的全部都要测

3. 测试的板子和芯片由谁来提供？

   先用平台测试板测试



- [ ] ZB PPI Nand 测试
  - [x] 片上 ecc 怎么关？关不了
    1. 测试 ecc 能力需要关掉 ecc
    2. 测试单独读写 oob 区域需要关掉 ecc，写的时候没关系，读的时候会产生 ecc 报错
       - 什么时候计算的 ecc？
       - 写的时候应该是将 main + oob 一起写了之后算了 ecc
       - 然后读的时候会校验 ecc
    3. ecc 纠错能力、掉电测试还没开始测试

# 10.22

- [x] GXAPI

  - [x] ecos 的跳坏块逻辑不要修改，丢到 gxapi 做

    - [x] ecos 不要去跳坏块擦

  - [x] linux 的 gxapi  need_skip_bad_len 丢到下层去做，cur_erase_addr 放到下层去更新

    - [x] 返回实际操作的长度，addr 不好放到下面去

    - [x] 不放到下面，因为中间还有一层，如果把地址放到下面去更新，那么地址是实时变的，

    - [x] 放到上面逻辑比较清晰，放到下面去还要改 nor 的逻辑

    - [x] 要操作的 len ，addr += want to len

    - [x] gx_flash_read_mtd 中要对 ret 做处理，如果 ret < 0 的时候，*length 会是什么值

    - [ ] 0x343e75d 0xbc18a3

    - [x] 调研 uboot 新增坏块处理机制，一种是：新增坏块，标记成坏块，return 出错(因为没擦除要擦除的长度)；一种是：新增坏块，标记坏块，继续擦下一个块，return 成功

      - [x] uboot 擦除逻辑：遇到坏块跳坏块，新增坏块标记坏块，继续后面的操作，不返回出错。第一次提交就是这样的操作流程

      - [x] uboot 擦除不对齐逻辑和写的时候不对齐的逻辑

        - [x] uboot 写操作：要写的地址不是 page 对齐，打印报错，直接返回
        - [x] uboot 读操作：要读的地址不是 page 对齐，打印报错，直接返回

      - [x] 最新的 busybox

        - [x] 擦除操作：首先跳坏块，然后擦除，不管是否成功继续擦

        - [x] 写操作：首先跳坏块，然后写入，不管写入是否成功继续写


    - [x] 为什么一开始提交的是 擦除失败后，不表坏块，直接退出，后来改成擦除失败后，不标坏块接着擦？
      - [ ] uboot 是直接抄的 linux 的 mtd_utils，linux 认为 mtd 层不会处理坏块
      - [ ] 最新的 mtd_utils 工具也是擦除失败后，不标记坏块，继续擦除，直到完成擦除操作


    - [x] Linux 允许跨分区操作，一个 mtd 一个 mtd 的来操作
    - [x] 新增坏块标记坏块，并且退出
    - [x] linux 已经搞定，放在那里跑，然后 ecos 写和擦的逻辑要改一下
      - [ ] ecos 的也搞定了，先看下今天出的 virgo rc，明天验一把

  - [x] erased_length 全部都按照 nos 的方式做，放到下面去，多擦一个 block，也就是保证要擦的长度都擦掉

    > 注：用 gxapi 的分区不要挂文件系统，因为 gxapi 操作这个分区的时候，文件系统不知道，可能造成数据被破坏

- [x] Virgo rc 需要提供新的 bit 再测试

  ```
  |测试项|测试结果|
  |内存到内存测试||
  |中断||
  |低功耗||
  |suspend||
  
  
  功能测试：
  * Flash 型号： <notextile>XM25QH128C、GD55LX02GEBIRY(NOR) GD5F1GM7UEYIG(NAND)</notextile>
  |模式| 位宽 | Nor Flash 测试结果| NAND Flash 测试结果|
  | 发送单倍速，无内部DMA     | 8bit  | OK  |  |
  | 接收单倍速，无内部DMA     | 8bit  | OK  |  |
  | 接收单倍速，无内部DMA     | 32bit | OK  |  |
  | 接收双倍速，开/关内部DMA  | 8bit  |   |  |
  | 接收双倍速，开/关内部DMA  | 32bit |   |  |
  | 发送四倍速，开/关内部DMA  | 8bit  |   |  |
  | 接收四倍速，开/关内部DMA  | 8bit  |   |  |
  | 接收四倍速，开/关内部DMA  | 32bit |   |  |
  | 发送八倍速，开/关内部DMA  | 8bit  | OK  | 无8线flash，不支持 |
  | 接收八倍速，开/关内部DMA  | 8bit  | OK  | 无8线flash，不支持 |
  | 接收八倍速，开/关内部DMA  | 32bit | OK  | 无8线flash，不支持 |
  | 分频配置                | 8bit  | OK |  |
  | 输入时钟延时             | 8bit  | OK |  |
  | 输入时钟上升沿采样        | 8bit  | OK |  |
  | 输入时钟下降沿采样        | 8bit  | OK |  |
  | 中断                   | 8bit  | OK |  |
  | 双 CS                  | 8bit  | OK |  |  
  ```

- [ ] Apus 相位测试
  - [ ] 用平台测试板，测试各种频率，各种电压组合下的相位，还要测试延时时间

- [ ] Zbit PPI Nand
  - [x] 读写擦
  - [x] 掉电
  - [ ] ecc 磨损测试



# 10.23

- [x] 调研 nand 坏块处理逻辑，包括 uboot、busybox、mtd_utils
  - [ ] 按照调研的方式来修改驱动的坏块处理逻辑，ecos、linux 都已经改好了，明天测一把
- [ ] Virgo rc 测试
  - [ ] flash_spi 8线功能都测完了，晚上跑一下 8 线的读写擦
    - [ ] 8 线读写擦 1000 次都没问
- [ ] ZB PPI Nand ecc 磨损



# 10.24

- [ ] Virgo rc 测试
  - [x] flash_spi 8线性能测试
  - [ ] flash_spi 4线功能测试、性能测试
  - [ ] 跑 linux 读写擦和性能测试
- [x] GXAPI 完整测试，昨天已经把驱动整理好了
  - [x] Linux 做测试，擦除失败的情况，写入失败的情况
    - [x] 写入失败的情况测到了，第一次失败，会标记坏块，再次测试ok，需要用 loff_t 传给 ioctl
    - [x] 继续测试 擦除失败的情况
  - [x] eCos 做测试，擦除失败的情况，写入失败的情况
    - [x] 擦除失败的情况，标记坏块，返回失败
    - [x] 写入失败的情况，标记坏块，返回失败
  - [x] 补丁全弄好了，最后测一把
    - [x] eCos：SPINand
    - [x] Linux：SPINand、PPINand
    - [x] 换回 switch case 语句，变量放到上面去定义
      - [x] SPINand
      - [x] PPINand
        - [x] 0xfffbed 0x413 error
    - [x] ecos 中的 erase ioctl return 换掉
    - [x] gxapi 中 ecos 的 need_skip 删掉了

- [x] Zbit PPI Nand
  - [x] 磨损 ecc 测试通过
  - [x] 在 develop 分支上用 3215 板子跑读写擦
  - [x] 在 develop 分支上用 3215B 板子跑读写擦





# 10.30

- [ ] 整理 Virgo 的 rc 测试测试结果
- [ ] gxmisc spi nand 坏块处理逻辑 #384775
- [ ] flash 掉电测试程序完善跳变数据信息 #390465
- [ ] gxmisc 驱动优化 #391928
- [x] 支持 MXIC 8线 Nor flash
- [x] 兼容性测试





# 11.4

- [ ] 整理 Virgo 的 rc 测试结果
- [ ] gxmisc spi nand 坏块处理逻辑
- [ ] flash 掉电测试程序完善跳变数据信息
- [ ] gxmisc 驱动优化
- [ ] 兼容性测试







# gxloader 更新 gxmisc 库从 v1.16 --> v1.18

- git log 确认每个补丁是否独立

  ```shell
  commit 0e075fb5731b5017c5b979b6f7ff47abc138133d
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Tue Sep 24 14:43:23 2024 +0800
  
      372941: gxmisc 版本发布:spinor-version:v1.18.0、spinand-version:v1.10.0、nand-version:1.7.0
      
      Change-Id: Icc7514b6570810d67da3aeede37b122bad675ff0
  
  commit 1877d34b0941098e9e022a1d811f3a905daa8597
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Tue Aug 6 13:47:25 2024 +0800
  
      386928: 支持 Skyhigh SPINAND S35ML01G3
      
      Change-Id: I07389ca97e445e4e6c6e1e8340830246efc3d005
  
  commit 2d9f13033f95f91301ef14b3edff92bded20e031
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Thu Aug 22 09:19:08 2024 +0800
  
      385863: 支持 Nor Flash BW25Q32ASIG
      
      Change-Id: Ic0425fc2a7afe5a290e5fd981defd509ae2ea167
  
  commit 1b8b226e3cae7db071803e6f49498b26987756f3
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Fri Jul 19 09:42:30 2024 +0800
  
      384196: 支持复旦微 SPINAND FM25S02BI3、并行 NAND FM29F02I3
      
      Change-Id: I6a065721ee02f850db884f4ab18b6296625e5899
  
  commit cd1596a68ee366c6b051b0cd78d9970ec77f712f
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Tue Jul 16 14:53:09 2024 +0800
  
      384328: Fix 部分 SPINand oob 排布描述错误
      
      Change-Id: Ief5cef3c4f24fd7a4a11db2475a86241b4950504
  
  commit 930b4b564328ba843b420b6f7a43854e1ec4ad76
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Thu Jul 4 13:34:02 2024 +0800
  
      381454: 修复 spinand_write_ops 在写入oob非0地址时仍然写入到 0 地址的问题
      
      Change-Id: I0dac01ac6a450c3c9b762ee57b04ecff82cd0d43
  
  commit fe97902ddc2ef9b9ba9ece948c0b10c6077952aa
  Author: yemb <yemb@nationalchip.com>
  Date:   Fri Jun 21 16:53:06 2024 +0800
  
      382770: 解决 Linux 打开 spi nor 和 spi nand 时报重定义
      
      Change-Id: Ie6f87be7de358a108ee0810e37d586c7d117f531
  
  commit b2daa8107bac736dce3e47f067246a0846217992
  Merge: df89fe5 855cf77
  Author: 刘怡雄 <liuyx@nationalchip.com>
  Date:   Fri Jun 14 16:41:43 2024 +0800
  
      Merge "381643: 删除 Irdeto image 解密接口 381641: 增加 crypto_default.c 对 keytype 为 EIK 的处理说明 删除 crypto 组件的加密接口"
  
  commit 855cf772e87f83b12e0da0c6cbda2a46f489aa7b
  Author: chenjch <chenjch@nationalchip.com>
  Date:   Wed Jun 5 17:24:02 2024 +0800
  
      381643: 删除 Irdeto image 解密接口
      381641: 增加 crypto_default.c 对 keytype 为 EIK 的处理说明
      删除 crypto 组件的加密接口
      
      Change-Id: Ifcd56cd2b50515a1b0deb4a62704837dd90f295f
  
  commit df89fe5d6b24392123489ad553c76c3656dbd505
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Fri Jun 14 09:30:17 2024 +0800
  
      382050: 完善 nand flash 烧写出错时的打印
      
      Change-Id: I4e2443b86b9556e851acb913f3403f971e7c4e37
  
  commit 900258d58d7c65c240e8294e3a46fdf4a4430802
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Fri Jun 7 17:07:43 2024 +0800
  
      381552: 支持 Nor Flash XM25QH128D
      
      Change-Id: I1f7d17d9330595d4c0a57c2c2d4f49a7ce51e2fd
  
  commit b208e852f9868da11ed8ff9c805cf2fb5dedae62
  Merge: a507c36 cd8997c
  Author: 刘怡雄 <liuyx@nationalchip.com>
  Date:   Thu Jun 6 15:12:32 2024 +0800
  
      Merge "381663: 1. nos、ecos、linux markbad 接口统一     2. markbad 接口添加擦除操作"
  
  commit cd8997c5eabee9478e0bcb4e8477363747cf1f36
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Thu Jun 6 10:07:23 2024 +0800
  
      381663: 1. nos、ecos、linux markbad 接口统一
              2. markbad 接口添加擦除操作
      
      Change-Id: I49cd28067d83427a12bedbffbb172fb2a6337a35
  
  commit a507c36a68573fe0040ebab4a23a7b9458b77bad
  Merge: 4a2222b c0dfe0b
  Author: 刘怡雄 <liuyx@nationalchip.com>
  Date:   Tue Jun 4 13:32:47 2024 +0800
  
      Merge "381343: 1. 解决 SPI Nand 标记坏块时无法将坏块标记写入 oob 问题   2. SPI Nand ID 统一修改为大写字母"
  
  commit c0dfe0b1e09aa6101d8c563482601ae942140a6f
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Mon Jun 3 09:42:44 2024 +0800
  
      381343: 1. 解决 SPI Nand 标记坏块时无法将坏块标记写入 oob 问题
              2. SPI Nand ID 统一修改为大写字母
      
      Change-Id: I7aadd9896c3840b3352d7509f3b58b0ce39ab73d
  
  commit 4a2222b149d94792eb35a7b9b3e21b3adefb6b3e
  Author: chenjch <chenjch@nationalchip.com>
  Date:   Tue Jun 4 09:40:09 2024 +0800
  
      378493: 支持 TDES 算法
      
      Change-Id: I25cd81d10304236904c68fc52f8448e4f53cf821
  
  commit f606012c5789326f104cd14222a4f8cc2f6fb3c1
  Author: liuyx <liuyx@nationalchip.com>
  Date:   Wed May 8 16:44:59 2024 +0800
  
      378226: 支持加密 Flash 分区，在读取 Flash 加密分区时解密数据
      
      Change-Id: Ic6b8e89cb37d2856aec64fb80f89d474838331be
  
  commit c0749039e7e9a475192aa27fe1e909705ce752df
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Mon May 20 14:02:24 2024 +0800
  
      380321: 支持 Zbit Nor Flash ZB25VQ64D
      380411: fix Zbit ZB25VQ128D uid、otp、wp 信息
      
      Change-Id: Iba5c615da6f7464988a4c8911df682c82f0e6fbb
  
  commit 039a81fb3e9172797e2454da4e47083d19015b22
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Wed May 15 11:17:10 2024 +0800
  
      379113: 支持Zbit SPI Nand Flash ZB35Q01BYIG、ZB35Q02BYIG
      
      Change-Id: Icc2200bfee34ee0cebbca6111298fde7d3952f6a
  
  commit 3e75c19c7ef893c294f4b03a18c923c7f66a07c8
  Merge: 0744e26 8dd20da
  Author: 刘怡雄 <liuyx@nationalchip.com>
  Date:   Thu May 9 15:40:00 2024 +0800
  
      Merge "378757: 支持东芯 Nor Flash DS25Q64A-13IA1"
  
  commit 0744e26064b65ae86f1066a663b746ad68bd7ee2
  Author: chenjch <chenjch@nationalchip.com>
  Date:   Sun May 5 18:54:58 2024 +0800
  
      378493: gxmisc 增加 crypto 功能
      
      Change-Id: I9753093253782035f2fded3407b7104b0caa29bd
  
  commit 8dd20da0be38cdfa2ba07aa441e7c9fd0480fd9a
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Mon Apr 29 15:02:21 2024 +0800
  
      378757: 支持东芯 Nor Flash DS25Q64A-13IA1
      
      Change-Id: Iada82e514971f2468ef8a75b5bc3c11cbbeb7bce
  
  commit bbf787e4c11133e73641b393d9e46b9d74d96e6f
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Tue Apr 16 09:53:06 2024 +0800
  
      377586: fix ZB25VQ32 系列 uid、otp 信息
      
      Change-Id: Icc13c2fc6a88bbd9e61a8b4fd280f1e488841ac6
  
  commit 0f97d8049debb80defe8c338ecbd0f3f5679080c
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Wed Apr 3 15:31:18 2024 +0800
  
      376758: fix ZB25VQ64 系列 uid、otp 信息
      
      Change-Id: Ib9bcca3475f88e5634dae6b8a3b1ec33c7239e1d
  
  commit 7c46bc362fa9c2ad1a6d37d2d45589f8d2f7fa6b
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Thu Aug 31 09:58:37 2023 +0800
  
      369771: 1. 支持 SPI Nand UM19A1HISW、GSS02GSAK1、FS35SQA002G、XT26G02DWSIGA、W25N01KVZEIR
              2. 支持 并行Nand ESMT F59L2G81KA
              3. fix 拼写错误
      
      Change-Id: I7d2203b35ec0ee669140dbb09e02584b2cf07726
  
  commit 7da8397f90401bef6100b6a47aaf43715f20139c
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Thu Dec 28 10:42:42 2023 +0800
  
      352926: 1. gxmisc 版本发布:spinor-version:v1.17.0、spinand-version:v1.9.0、nand-version:1.6.0
              2. 删除 S25FL032P、S25FL064P、S25FL128P、S25FL256S、m25p016、m25p128 支持
      
      Change-Id: Ied7e7f7af32b1ca0e87f5583d996878c878585aa
  
  commit 2606e41f2e5429a004c1e672ce10bb928a162899
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Tue Dec 26 11:21:02 2023 +0800
  
      369064: fix spinand 名称错误
      
      Change-Id: Ic4edefc2c2f96f3e3170c5857197383295be90e5
  
  commit fb44645cbb606ce8633e25957a8d8afdb5db7bb7
  Merge: f049d66 e5c4923
  Author: 刘怡雄 <liuyx@nationalchip.com>
  Date:   Mon Dec 25 10:52:06 2023 +0800
  
      Merge "369064: 支持 SPI Nand GD5F1GM7UEYIG、GD5F2GM7UEYIG、GD5F4GM7UEYIG"
  
  commit f049d66032c3bbb229c1b22655296304ffc868e8
  Author: heqi <heqi@nationalchip.com>
  Date:   Tue Dec 12 16:03:48 2023 +0800
  
      368422: MIXC SPINOR 部分型号不支持四倍速bug优化
      
      Change-Id: Ica4272904961449d8268ee966c591497b428f8ee
  
  commit e5c49235813682e90e992320978bd22d8a3fdc41
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Wed Dec 20 15:08:13 2023 +0800
  
      369064: 支持 SPI Nand GD5F1GM7UEYIG、GD5F2GM7UEYIG、GD5F4GM7UEYIG
      
      Change-Id: I778434fcf5f79648aa5f4927d9fb7fa408b0b109
  
  commit c083603887f6e3cedfbce4b16cbb187663cca275
  Author: heqi <heqi@nationalchip.com>
  Date:   Thu Dec 7 10:52:28 2023 +0800
  
      368133: gxmisc 新增设备flash_nand相关修改
      
      Change-Id: Ib5d95e2a15e57091ba891fcef85645321fa6ec7c
  
  commit c3c36c0b76410538208a66af5019b60a053c3d04
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Mon Nov 27 09:30:49 2023 +0800
  
      365701: 支持 Nor Flash DS25Q4AA
      
      Change-Id: I3f003265eea1ebacf8c16fbdf2b0d08a6d76ec9f
  
  commit b5f9be031a503ab0f92f8735b32d0dd9bf80e378
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Fri Nov 24 13:47:18 2023 +0800
  
      366842: build 脚本添加 <ARCH> nos 选项
      
      Change-Id: Ib2af9acccc8a2c3a4bcb7ca4bbacdf403943ea1c
  
  commit eb2d1a9192261780fcff6ca22e6a99febabe2738
  Author: heqi <heqi@nationalchip.com>
  Date:   Thu Nov 2 13:26:31 2023 +0800
  
      361537: vega  nfc控制器使用异步dma方式bug优化
      
      Change-Id: I643848d74e9a132f21a441c0338aa2db5e72a347
  
  commit ae8916271bb3f65092016929af194b2743d8c006
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Wed Oct 18 13:47:16 2023 +0800
  
      362551: 支持 Nor Flash FM25Q32BI3
      
      Change-Id: I3b8fe93a55485db1322620c191cd15b05d9acece
  
  commit af3e6a8927721e1dfe121a729b39527dca4592f3
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Mon Oct 9 14:31:34 2023 +0800
  
      361844: 支持 Nor Flash XM25QH32D
      
      Change-Id: I96ddc795cb522c196a21db3731121aae26668935
  
  commit d039331504af59293f24619521ef514b8f5c1f0a
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Thu Sep 21 10:32:28 2023 +0800
  
      353348: 支持 Nor Flash XM25QH64D
      
      Change-Id: Ic1a535a24dbb3ce9c370811131583f26767263b4
  
  commit 5fc73313ff703e5d24c75fa0f93872bb182f7305
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Thu Aug 31 09:58:37 2023 +0800
  
      359867: SPI Nand readpage 接口添加判断 ecc 状态
      
      Change-Id: I785aee99f07e8742918a8cd269e59fd9ab0f09b9
  
  commit 62cf0cac8bfde5fa532885ecd403f41f302dbc62
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Tue Sep 5 14:03:22 2023 +0800
  
      360107: 完善 Nor Flash BY25Q64AS 写保护范围
      
      Change-Id: I0095d76af33cd830e07d3348656c196a45684d36
  
  commit 13897cf63d9d02f687ec8fb34cc91e21635bb0a3
  Merge: a66873e 419ccdc
  Author: 刘怡雄 <liuyx@nationalchip.com>
  Date:   Fri Sep 1 13:16:38 2023 +0800
  
      Merge "359304: canopus nfc控制器软件ecc方式实现"
  
  commit 419ccdcdd4559e0f0c1f1cc96b5a193adae0ab53
  Author: heqi <heqi@nationalchip.com>
  Date:   Mon Aug 28 10:25:40 2023 +0800
  
      359304: canopus nfc控制器软件ecc方式实现
      
      Change-Id: I24dbd53b7fcd0256a288e1dbb07217550aa73149
  
  commit a66873e8098917dbb5f960585e74ca1c6b34fd85
  Merge: e69738b 3d73ffa
  Author: 刘怡雄 <liuyx@nationalchip.com>
  Date:   Fri Aug 25 13:40:28 2023 +0800
  
      Merge "357681: 支持 Nor Flash BY25Q32ES、BY25Q128ES"
  
  commit 3d73ffa050d707256b9ab1b1594ef3109bf23f10
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Tue Aug 22 09:20:55 2023 +0800
  
      357681: 支持 Nor Flash BY25Q32ES、BY25Q128ES
      
      Change-Id: I35319a20ed8ece03327f5398f2c43b8ff360785d
  
  commit e69738ba2e29deb98e95112085c45d2fe799ddff
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Fri Aug 18 14:08:37 2023 +0800
  
      358131: 支持 Nor Flash FM25Q64AI3
      
      Change-Id: I51fdd5a5f00f4f5fbc448667a9116da304aae773
  
  commit c415ef187a4d0894b4be533aea94f4ecf43f8723
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Tue Aug 15 13:30:42 2023 +0800
  
      357354: 修改 PN27G02BB 的 ecc 能力
      
      Change-Id: I3c24b0640fae656d5128d9d917f338198d4b6b14
  
  commit 1c1bdcda43181c37afbf603cfa98e1000f46dddd
  Merge: ddf3764 ba5e077
  Author: 刘非 <liufei@nationalchip.com>
  Date:   Tue Aug 15 09:52:34 2023 +0800
  
      Merge "356426: 删除对 ntfs 文件系统头文件依赖"
  
  commit ddf37645d9af5599d0a02f5f7191f77377002e4a
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Tue Aug 8 13:24:14 2023 +0800
  
      357087: 修改 XT25F64F-S 的 OTP 信息以及操作方式
      
      Change-Id: I9628e60de6bd2b49f7b55ce18d0ca3685394a804
  
  commit ba5e07771cce112aea80f12f76fae47840847484
  Author: xukai <xukai@nationalchip.com>
  Date:   Mon Aug 7 17:04:11 2023 +0800
  
      356426: 删除对 ntfs 文件系统头文件依赖
      
      Change-Id: I52316b6d1d938e596af83859a0f799c7cae7672f
  
  commit 6efdf5a5d52e3b4302f94532bf14e526df488453
  Author: heqi <heqi@nationalchip.com>
  Date:   Fri Jul 14 14:19:12 2023 +0800
  
      355265: canopus 并行nand NFC控制器 时序兼容性，健壮性优化
      
      Change-Id: I3bf02f7dbecb65f1a223271cfdd69dd59070948d
  
  commit 80ff971403ea29456f8860d14e0fa8c2ea0f46c0
  Author: heqi <heqi@nationalchip.com>
  Date:   Wed Jun 14 17:27:00 2023 +0800
  
      353948: linux系统下nand dma使用中断方式
      
      Change-Id: I92d349b37250de237c7310b8e887967594672496
  
  commit d89691dd28ccf69d086b77b6dc7f3677ecad3104
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Mon Jun 19 13:50:28 2023 +0800
  
      352026: 添加 SPI NAND 同时读写 数据区+OOB区 的接口
      
      Change-Id: Iab9ff403091b04ac8a8db588324a879058490a99
  
  commit 590edab9eaf9371518a9f19bc949e809db9ad3fc
  Author: tanxzh <tanxzh@nationalchip.com>
  Date:   Wed Jun 14 15:21:13 2023 +0800
  
      337318: gxmisc 版本发布:
      spinor-version:v1.16.0
      spinand-version:v1.8.0
      nand-version:1.5.0
      
      Change-Id: I26103594bc2ce31b93bf07eaedf427f8f1e94f7b
  
  
  ```

- 下载两个版本的代码， bcompare 比一下

  ```
  ```

  





# 11.11

- AXI_DMA 内存到内存有问题
  - test14 的时候测试对内存和对 gen_spi 都 ok，用这个bit测测看
  - 新的代码，测试对 gen_spi ok，对内存测试也 ok
    - 地址配置成 no inc 了
- 换成最新的 bit
  - ok

- [x] fix dma测试用例 
  - [x] comtest 修改测试地址
  - [x] comtest 添加数据对比

- [x] 东芯、华邦两款 512MBit SPINand 掉电测试
- [ ] Virgo 外设 RC 评审
- [ ] 研究 Linux AXI-DMA、GEN_SPI 驱动



0x200000 --> data

dst --> 0x0

memcpy  dst--> data

compare



dst-->0x0

cpy_lli  dst--> data

compare



dst-->0x0





- [ ] axi dma channel auto gate

  - 用到的通道打开时钟，没用到的通道不会打开时钟
  - axi dma 模块有 7 个时钟，6 个通道，每个通道有一个对应的时钟，另一个时钟一直开着，关不掉
  - 现在是做了一个 clk gate，当通道使能之后会自动打开时钟，没使能的通道不会打开时钟
    - 测试情况：
      - 配置 0xfa49008c = 0x1，做一次 dma 内存测试，会开 dma channel0，然后关掉 dma channel0 使能
      - 再继续做 dma 内存测试，还是同一个 dma channe，没有 dma 完成状态

  测 dma clk gate 的时候遇到点问题，测试流程是这样的

  1. 配置 0xfa49008c = 0x1
  2. 正常的做一次 dma 内存测试：先配置 dma 寄存器，然后使能 dma channel0，完成 dma 操作后关掉 dma channel0 使能
  3. 再做一次 dma 内存测试，依旧是相同的 dma channel，轮询 0x188 寄存器等不到 dma 完成状态

  ![2024-11-18_10-13](image/2024-11-18_10-13.png)





**dma lowpower 不能动态配置，系统 reset 之后只能配置一次**

















# 整理掉电测试流程，为什么客户可以测出来，我们测不出来，从原理上分析







