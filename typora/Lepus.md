# Bit 文件问题号：#336193

# Canopus 测试问题号：#284584

# Uboot

### debug 信息

- 开启debug信息，在对应的.c文件开头加上：

```C
#define DEBUG
#undef CONFIG_LOGLEVEL
#define CONFIG_LOGLEVEL 8
```

| **Uboot**               | **GoXceed**                                       |
| ----------------------- | ------------------------------------------------- |
| spi/designware_spi.c    | dw_spi/dw_spi_v2.c                                |
| mtd/spi/sf_probe.c      | gxmisc/drivers/spinor/nos/spinor.c                |
| simple_spi/simple_spi.c | gxloader/drivers/spi/dw_spi/dw_spi_v2.c --> setup |
| mtd/spi/spi_flash.c     | gxmisc/drivers/spinor/nos/spinor.c                |

spi 驱动： --> `"snps,dw-apb-ssi"` dts <--> driver

- 执行 

  ```
  drivers/spi/designware_spi.c
  ```

   中的 

  ```
  probe
  ```

   函数：

  - 当 udevice 和 driver 匹配成功时会执行 probe
    - probe 会获取 dts 的 `regs、frequency、cs_reg、 spi_hw_init(priv)`

flash 驱动：`"spi-flash"` dts <--> driver

- 调用 spi 驱动去给 flash 发送指令

- ```
  drivers/mtd/spi/sf_probe.c
  ```

   中会执行 

  ```
  probe
  ```

   函数：

  - `simple_spi_init`：根据 bus、cs、speed、mode 配置 spi 控制器

  - ```
    spi_flash_probe_slave
    ```

    ：

    - `spi_flash_scan`：`spi_flash_cmd` 发送读 ID，给 struct spi_flash 赋值、配置是否开启 四倍速

## spi 驱动：

- 驱动程序：采用 drivers/spi/designware_spi.c 驱动，设备树匹配成功时执行 probe 函数配置 SPI 的寄存器，并向外提供 .xfer、.set_speed、.set_mode 接口
- 测试程序：在 cmd/spi.c 中通过执行 spi xxx 命令，通过 drivers/spi/spi-uclass.c 实现调到驱动程序提供的 .xfer、.set_speed、.set_mode 接口

![img](https://iklhoemxll.feishu.cn/space/api/box/stream/download/asynccode/?code=NjAzMDAyYzYyNWUzMGNiNjQ5OThjY2UxYjg2ODEyYTVfRzhzVkdMMFRRMWFuSEU3MHhtdUpZY052WWFnc0xsaTNfVG9rZW46Ym94Y252VUJocmtycjVIS1lNT3dTeFNJOXFiXzE2Njk2MjI4MzM6MTY2OTYyNjQzM19WNA)

## flash 驱动：

- 驱动程序：drivers/mtd/spi/sf_probe.c 中当设备树和驱动匹配成功时会执行 probe 函数，probe 函数会根据 spi 的 bus、cs、speed、mode 配置 SPI 控制器，并发送 读取 ID 指令和配置 flash，向外提供ops，包括 .read、.write、.erase
- 测试程序：在 cmd/sf.c 中通过 sf xxx 命令调用到 drivers/mtd/spi/sf-uclass.c 实现调到驱动程序提供的 ops， read、write、erase
- flash 支持列表：`Lepus/uboot/drivers/mtd/spi/sf_params.c`

![img](https://iklhoemxll.feishu.cn/space/api/box/stream/download/asynccode/?code=NDkxZGMzNGQ2MjRhYWYxYTJkNjFkMzI4NGU5ZGQwYjJfcXc4bU5zaDN6ZGVDVGhwZURVaElKVlpxMmVmT0JmM0ZfVG9rZW46Ym94Y25WQzlDTVJHMjJMbldjcVM2VWFaWDNkXzE2Njk2MjI4MzM6MTY2OTYyNjQzM19WNA)

## 设备和驱动匹配：

- 设备：通过 设备树动态配置

- 驱动：`driver` 目录下使用 `UBOOT_DRIVER` 编写

- 匹配：通过 

  ```
  lists_bind_fdt
  ```

   查找子设备，并且根据查找到的子设备，进而查找对应的驱动进行绑定，实现了 device 和 driver 的绑定

  - 根据扫描到的`udevice`设备信息，通过`compatible`来匹配`compatible`相同的`driver`，匹配成功后，就会创建对应的`struct udevice`结构体，它会同时指向设备资源和driver，这样设备资源和driver就绑定在一起了。

- probe 函数：

  - 定义了 DM_SPI_FLASH 的情况：确保所有的 parent 执行 probe，然后执行 `drv->probe` 也就是驱动里面的 probe 函数

## 文件：

- spi：
  - 驱动程序:
    - designware_spi.c /home/tanxzh/goxceed/Lepus/uboot/drivers/spi：提供 dts 和 driver 匹配成功的 probe 函数和 ops，提供 dw_spi_xfer 函数使用指针对SPI寄存器进行操作
  - 中间层：spi-uclass.c `/home/tanxzh/goxceed/Lepus/uboot/drivers/spi`：使 cmd 目录下能够调用到 driver 目录下的 ops
  - 测试程序:
    - spi.c `/home/tanxzh/goxceed/Lepus/uboot/cmd`：SPI 的测试程序，包括配置 spi 的mode、speed、cs、和 xfer
- Flash
  - 驱动程序：
    - spi_flash.c `/home/tanxzh/goxceed/Lepus/uboot/drivers/mtd/spi` ：包括扫描 ID 匹配 IDS、填充 `struct spi_flash` 结构体，包括`read、write、erase、lock、unlock`函数和 各种 size
    - sf_params.c `/home/tanxzh/goxceed/Lepus/uboot/drivers/mtd/spi`：包括 IDS
    - sf_probe.c `/home/tanxzh/goxceed/Lepus/uboot/drivers/mtd/spi`：提供 dts 和 driver 匹配成功的 probe 函数和 ops，这个 .c 文件中会调用 `simple_spi_init` 执行 SPI 控制器的设置，以及 注册从设备(`spi_flash_scan` 扫描 ID并匹配)
    - simple_spi.c `/home/tanxzh/goxceed/Lepus/uboot/drivers/simple_spi` ：提供 `simple_spi_init`函数(初始化 SPI 控制器)和 `simple_xfer`函数(提供spi_write 和 spi_read 对寄存器操作)
    - sf.c `/home/tanxzh/goxceed/Lepus/uboot/drivers/mtd/spi`：提供给用户 cmd 调用的最外层，封装好的API
  - 中间层：
    - sf-uclass.c `/home/tanxzh/goxceed/Lepus/uboot/drivers/mtd/spi`：使 cmd 目录下能够调用到 driver 目录下的 ops
  - 测试程序：
    - sf.c `/home/tanxzh/goxceed/Lepus/uboot/cmd`：FLASH 的测试程序，包括读写擦和 probe
- Cmd --> sf probe --> spi_flash_probe --> spi_flash_probe(sf_probe.c) --> spi_setup_slave --> spi_flash_probe_tail --> spi_flash_probe_slave --> spi_flash_scan

## 梳理流程：

1. spi 驱动：uboot 直接使用 gxloader 中的 spi 驱动
2. flash 驱动：uboot 使用 gxmisc 中的 spi_nor.c 中的配置，并且文件名改成 sf_probe.c ，构造 ops，前期构造简单的 ops，用于对接框架，后期构造完整的 ops 用于测试，cmd 通过调用 ops 来执行测试程序
3. gxmisc 中对接 uboot，通过 头文件、defconfig、或者 driver 下面来完成区分

### ops 对接：

- `include/configs/lepus_common.h`：通过调用命令 `sf probe; sf read ${fdt_addr} ${fdt_offs} ${fdt_size}; sf read ${kernel_addr} ${kernel_offs} ${kernel_size}` 来完成读取设备树和内核
- `cmd/sf.c` ：实现了对于命令行的处理函数
- `drivers/mtd/spi/spinor.c`：实现了实际的 read、write、erase 函数
  1. 使用 uboot 自带的驱动 ops 定义，在下面封装一层再调用 gxmisc 中的函数
  2. 直接修改掉 uboot 的驱动 ops 定义
  3. 直接实现函数，然后 cmd/ 下面调用的时候不管 ops，直接调函数指针

可以跑到 probe ，但是不能跑到 read，这个地方跑完 probe 之后赋给了 spi_flash，但是我们并没有使用 spi_flash，所以这个地方传进去 flash→dev 也是找不到东西的一个指针

![img](https://secure2.wostatic.cn/static/5LEz8dkicaAkMmohkxpwiW/image.png?auth_key=1675904151-2T8cacm7EwKCvc1s3vFqoB-0-7cf3aa6a24258ff39fb10835d0c32bdb)

### uboot 运行流程：

```C
board_init_r(gd_t *new_gd, uling dest_addr)
    initcall_run_list(init_sequence_r)// 运行上面的 函数指针列表
        init_sequence_r[]//其中包含很多的initr_xxx common/board_r.c
      ret = (*init_fnc_ptr)();    
      initr_env(void)// common/board_r.c
        env_relocate()
          env_relocate_spec()// common/env_spiflash.c
            spi_nor_env_relocate_spec(void)// common/env_sf.c
              spi_flash_probe()
              spi_flash_read()
              // spi_flash_probe
                spi_flash_probe_bus_cs(bus, cs, max_hz, spi_mode, &dev)// sf-uclass.c
                  spi_get_bus_and_cs(busnum, cs, max_hz, spi_mode, "spi_flash_std", str, &bus, &slave)
                    device_probe(dev)// drivers/core/device.c
                      drv->probe(dev)// driver/mtd/spi/spinor.c
                        U_BOOT_DRIVER(dw_spi_flash_std)->ops->probe
                //spi_flash_read
                spi_flash_read_dm(flash->dev, offset, len, buf)// include/spi_flash.h
                  sf_get_ops(dev)->read(dev, offset, len, buf)//drivers/mtd/spi/sf-uclass.c
                    U_BOOT_DRIVER(dw_spi_flash_std)->ops->read
        run_main_loop(void)// common/board_r.c
          main_loop()// main.c
            cli_loop(void)// common/cli.c
              cli_simple_loop(void)//common/cli_simple.c
                cli_readline(const char *const prompt)
                run_command_repeatable(const char *cmd, int flag)
```

### probe 流程：

```C
struct spi_flash *spi_flash_probe()
  spi_flash_probe_bus_cs(bus, cs, max_hz, spi_mode, &dev)
    // drivers/spi/spi-uclass.c
    spi_get_bus_and_cs(busnum,cs, max_hz, spi_mode, "spi_flash_std", str,&bus, &slave)
      // dev 扫描设备树时会创建好
      (gdb) p *dev
      $10 = {driver = 0x7fe4998, name = 0x5f9ea94 "s25fl128s@0", platdata = 0x0 <v7_arch_cp15_set_acr>, parent_platdata = 0x5fa0218, uclass_platdata = 0x0 <v7_arch_cp15_set_acr>, of_offset = 776, driver_data = 0, 
      parent = 0x5fa0138, priv = 0x0 <v7_arch_cp15_set_acr>, uclass = 0x5fa01a0, uclass_priv = 0x0 <v7_arch_cp15_set_acr>, parent_priv = 0x0 <v7_arch_cp15_set_acr>, uclass_node = {next = 0x5fa01a8, prev = 0x5fa01a8}, 
      child_head = {next = 0x5fa01f8, prev = 0x5fa01f8}, sibling_node = {next = 0x5fa0170, prev = 0x5fa0170}, flags = 72, req_seq = -1, seq = -1}
      device_probe(dev)// drivers/core/device.c
      // dev 对应的 drv 就是使用 U_BOOT_DRIVER 宏定义的数据结构
      (gdb) p *drv 
      $11 = {name = 0x7fdec9e "spi_flash_std", id = UCLASS_SPI_FLASH, of_match = 0x7fd44d4, bind = 0x0 <v7_arch_cp15_set_acr>, probe = 0x7fb57d1 <spi_flash_std_probe>, remove = 0x0 <v7_arch_cp15_set_acr>, 
      unbind = 0x0 <v7_arch_cp15_set_acr>, ofdata_to_platdata = 0x0 <v7_arch_cp15_set_acr>, child_post_bind = 0x0 <v7_arch_cp15_set_acr>, child_pre_probe = 0x0 <v7_arch_cp15_set_acr>, 
      child_post_remove = 0x0 <v7_arch_cp15_set_acr>, priv_auto_alloc_size = 52, platdata_auto_alloc_size = 0, per_child_auto_alloc_size = 0, per_child_platdata_auto_alloc_size = 0, ops = 0x7fd44e4, flags = 0}
     
      // probe spi
      (gdb) p *dev->parent
      $13 = {driver = 0x7fe4800, name = 0x5f9e990 "spi@00303000", platdata = 0x5fa0190, parent_platdata = 0x0 <v7_arch_cp15_set_acr>, uclass_platdata = 0x0 <v7_arch_cp15_set_acr>, of_offset = 516, driver_data = 0, 
      parent = 0x5fa00b0, priv = 0x5fa1448, uclass = 0x5fa0118, uclass_priv = 0x5fa0430, parent_priv = 0x0 <v7_arch_cp15_set_acr>, uclass_node = {next = 0x5fa0120, prev = 0x5fa0120}, child_head = {next = 0x5fa0200, 
      prev = 0x5fa0200}, sibling_node = {next = 0x5fa02a0, prev = 0x5fa00e8}, flags = 67, req_seq = 0, seq = 0}
      ret = device_probe(dev->parent); 
     
     
        drv = dev->driver
        ret = drv->probe(dev)// drivers/core/device.c
      
```

### spi 驱动：

- designware_spi.c 中前半部分的配置，与 ecos 中 dw_spi_v2.h 中的类容基本类似，可以将 .h 中的文件按照 uboot 的方式放置在 .c 的前半部分

![img](https://secure2.wostatic.cn/static/gfFCskJb14iRpec7hJYHFE/image.png?auth_key=1675904151-rUmHbs7ucBZV4kxBdUjLMc-0-d10ac76230577d46c1753f4b8d8cc77c)

- dm_spi_xfer 这个接口不要变，下面调用 spi_write_then_read()，通过 priv 的方式，替代 `struct dw_spi_priv` 换成自己的数据结构

- set_speed 接口可以使用 spi_set_clk() 接口来替换

- set_mode 接口考虑是留空或者直接去掉，dw_spi 不允许随意修改 spi_mode

- ```
  struct spi_master/device
  ```

   等结构体考虑如何嵌入进去

  - 不用嵌进去，直接拷贝过去即可，因为在 spi.c 中并没有涉及到什么具体的操作

- 在 spi.c 中加入 spi_write_then_read() 的实现

# 保留疑问：

## spi

- 结构定义：

```C
struct spi_transfer {
        const void    *tx_buf;
        void          *rx_buf;
        unsigned      len;

        unsigned      cs_change:1;
        unsigned      tx_nbits:3;
        unsigned      rx_nbits:3;
#define SPI_NBITS_SINGLE        0x01 /* 1bit transfer */
#define SPI_NBITS_DUAL          0x02 /* 2bits transfer */
#define SPI_NBITS_QUAD          0x04 /* 4bits transfer */
        unsigned char bits_per_word;
        unsigned int  delay_usecs;
        unsigned int  speed_hz;

        struct list_head transfer_list;
};

struct spi_device ;
struct spi_message {
        struct list_head      transfers;
        struct spi_device     *spi;
        unsigned              is_dma_mapped:1;
        void                  (*complete)(void *context);
        void                  *context;
        unsigned              actual_length;
        int                   status;
        struct list_head      queue;
        void                  *state;
};

struct spi_master {
        int              bus_num;
        unsigned int     num_chipselect;
        int              (*setup)(struct spi_device *spi);
        int              (*transfer)(struct spi_device *spi,struct spi_message *mesg);
        void             (*cleanup)(struct spi_device *spi);
        void             *data;
        void             *driver_data;
        struct list_head list;
};

struct spi_device {
        struct spi_master *master;
        unsigned int      max_speed_hz;
        unsigned char     chip_select;
        unsigned char     bits_per_word;
        unsigned short    mode;
#define SPI_TX_DUAL     0x100   /* transmit with 2 wires */
#define SPI_TX_QUAD     0x200   /* transmit with 4 wires */
#define SPI_RX_DUAL     0x400   /* receive with 2 wires */
#define SPI_RX_QUAD     0x800   /* receive with 4 wires */
        void              *controller_state;
};

struct sflash_master {
        int    bus_num;
        struct spi_device spi;
        struct list_head  list;
};
```

- gxloader 中的 spi 是用的类似 linux 的一套 spi，里面传输数据都是用的 message 组成的，包括 setup、transfer、cleanup
- spi 不提供具体的寄存器操作，所有的寄存器操作通过 spi_device 结构来对应不同厂家的 spi ip

## dw_spi

- 结构定义：

```C
struct dw_spi {
        struct spi_master       *master;
        uint32_t                 regs;
        uint32_t                 cs_regs;
        uint32_t                 tx_fifo_len;       /* depth of the FIFO buffer */
        uint32_t                 rx_fifo_len;       /* depth of the FIFO buffer */
        struct spi_message       *cur_msg;
        struct spi_transfer      *cur_transfer;
        void                     *buffer;
        void                     *cur_buffer;
        uint32_t                 len;
        uint32_t                 complete;          /* current transfer byte number */
        uint32_t                 complete_minor;     /* current transfer byte number */
        uint8_t                  n_bytes;           /* current is a 1/2/4 bytes op */
        uint32_t                 hw_version;
};
```

- dw_spi 是 spi 的一种，所有用户都是用 spi，但是 ip 不一致要进行修改特定的东西，所以有一套专属 ip 的结构：`struct dw_spi`
- 通过 `struct spi_master *master` 来找到 spi 驱动
- 包含专属 ip 的寄存器定义

## flash

- nos

```C
static struct spi_nor_flash g_flash_info = {0, 0, 0, -1, NULL, NULL};

static struct sflash_master g_spi_nor_master = {
  .bus_num = 0,
  .spi = {
     .master = NULL,
     .chip_select = 0,
     },
};
```

- ecos

```C
static struct spi_device spi_dev = {
  .master = NULL,
  .max_speed_hz = 3000000,
  .chip_select = 0,
  .mode = 0,
};

struct m25p {
  cyg_drv_mutex_t lock;
  struct spi_nor_flash flash;
}
```

- nos 和 ecos 中的 m25p 和 sflash_master 结构体等价

![img](https://secure2.wostatic.cn/static/mwUZ318VncnvqXPkrHe2Ly/image.png?auth_key=1675904152-hBJnBnwUjQU6PV5cDT4ykg-0-a1235950414d6243092d102ce1a151b7)

# 移植步骤：

## 1. 通用 SPI 层

- `driver/spi/spi.c` 中添加 `spi_write_then_read`
- `include/spi.h` 中添加一些结构体的定义：`struct device，struct transfer，struct message`

## 2. SPI IP 层

- 构建一个`struct dw_spi_platdata`

```C
struct dw_spi_platdata {
    uint32_t frequency;
    uint32_t regs;
    uint32_t cs_regs;
    /* 后续还可以添加更多设备树中的内容 */
};
```

- 构建一个 priv

```C
struct dw_spi_priv {
    /* 在 probe 的时候将设备连接过来，以后就可以通过 bus 来找到这个结构 */
    struct dw_spi *p_dws;
    /* 还不知道怎么连接这个结构，这个结构用于 setup 和 transfer */
    struct spi_device *p_spi;
};
```

## 3. FLASH 驱动层

## todo

- 编译问题：如何把 base.h 放在 `drivers/mtd/spi` 目录下，然后在 `include/spi_flash.h` 中包含这个头文件？
- sf.c 中使用的是 `struct spi_flash` ，考虑将其替换成 `struct  spi_nor_flash`
- 读取数据为全 F，是不是引脚复用没有配？
  - 因为 flash 里面没有数据
- 大小端问题，使用 uboot 自带的 designware 驱动可以正常读取，使用 dw_spi 移植过来的驱动大小端反的(代码不进行大小端转换了，大小端转换由 IP 来做)
  - 修改代码如下解决大小端问题：

```C
--- a/drivers/spi/dw_spi_v2.c
+++ b/drivers/spi/dw_spi_v2.c
@@ -362,7 +362,7 @@ static int dw_half_duplex_poll_transfer(struct dw_spi *dws)
        struct device_context *context = spi_get_ctldata(dws->cur_msg->spi);
        uint32_t cr0;
        uint32_t align_steps;
-       uint32_t data;
+//     uint32_t data;
 
        /* 长度对齐 */
        if (len % inc)
@@ -478,12 +478,13 @@ static int dw_half_duplex_poll_transfer(struct dw_spi *dws)
                                items = dw_readl(dws, SPIM_RXFLR) & rx_len_mask;
 
                                for (i = 0; i < items; i++) {
-                                       data = dw_readl(dws, SPIM_TXDR);
-                                       data8[0] = data >> 24;
-                                       data8[1] = data >> 16;
-                                       data8[2] = data >> 8;
-                                       data8[3] = data >> 0;
-                                       data8 += 4;
+                                       *data32 = dw_readl(dws, SPIM_TXDR);
+                                       data32++;
+//                                     data8[0] = data >> 24;
+//                                     data8[1] = data >> 16;
+//                                     data8[2] = data >> 8;
+//                                     data8[3] = data >> 0;
+//                                     data8 += 4;
```

- 添加 cmd 的 ops 来实现 test case

- 替换 `struct spi_slave` 结构体和 `struct spi_device` 结构体

  - 不需要替换，gxmisc 中独立维护了数据结构，外部并不需要传递数据结构

- 替换 `struct spi_flash` 结构体换成 `struct spi_nor_flash` 结构体

  - 不需要替换，与上理由相同

- gxmisc 差异

  - spinor.c

    ![img](https://secure2.wostatic.cn/static/vs234GBbiWjrTdPDrGH4jr/image.png?auth_key=1675904152-eBcL3KMpdBDQTe4cx8xm4Q-0-29c9cf82a7dd014b808fd13ece79decb)

    ![img](https://secure2.wostatic.cn/static/caQTidWaZaYqRAiAn2YJLy/image.png?auth_key=1675904152-o6AbE5GMDFyUwkty8hNTqX-0-ed632775be0deb4580a567a2051b490a)

    - ops 调用 gxmisc 中的接口时，需要再封一层
      - 此处不需要再封一层，`struct udevice` 不需要与 `struct spi_nor_flash` 建立联系

  - spinor_base.c

    ![img](https://secure2.wostatic.cn/static/gHwT9pHgScVP7ygz99knfW/image.png?auth_key=1675904153-3CjbV8jigweF4jwFd2yaJs-0-a7307e96f7f7ceb486f384644a59c656)

  - spinor_base.h

    ![img](https://secure2.wostatic.cn/static/9oDi7JhVgTweyWMmeGubHJ/image.png?auth_key=1675904152-9u4Mr8vS5VhuqHi6Fn8nGB-0-9edfcb825967c16721892a7f5647736e)

  - spinor_ids.c

    ![img](https://secure2.wostatic.cn/static/pDYkaUaGU1G7DsJpiWVtew/image.png?auth_key=1675904152-3uvULCeJnL7RJCdHFmcjc-0-55e259d1027d97c7a9a21e7f810ddd48)

  - spinor_ids.h

    - 无差异

- [ ] 确认 `drivers/mtd/spi/Makefile` 中 SPL 部分的编译内容：

```Makefile
obj-$(CONFIG_DM_SPI_FLASH) += sf-uclass.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_SPI_LOAD)      += spi_spl_load.o
obj-$(CONFIG_SPL_SPI_BOOT)  += fsl_espi_spl.o
obj-$(CONFIG_SPL_SPI_SUNXI) += sunxi_spi_spl.o
endif

obj-$(CONFIG_SPI_FLASH) += sf_probe.o spi_flash.o sf_params.o sf.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_DW_SPI_FLASH) += spinor_uboot.o spinor_base.o spinor_ids.o spinor.o
endif
obj-$(CONFIG_SPI_FLASH_DATAFLASH) += sf_dataflash.o
obj-$(CONFIG_SPI_FLASH_MTD) += sf_mtd.o
obj-$(CONFIG_SPI_FLASH_SANDBOX) += sandbox.o

#ifdef CONFIG_SPL_BUILD                                                                                                                                                                                                         
#$(obj)/spinor.o:
#   cp $(srctree)/$(src)/spinor.fw spl/$(src)/spinor.o
#$(obj)/spinor_base.o:
#   cp $(srctree)/$(src)/spinor_base.fw spl/$(src)/spinor_base.o
#$(obj)/spinor_ids.o:
#   cp $(srctree)/$(src)/spinor_ids.fw spl/$(src)/spinor_ids.o
#else
$(obj)/spinor.o:
    cp $(srctree)/$(src)/spinor.fw $(src)/spinor.o
$(obj)/spinor_base.o:
    cp $(srctree)/$(src)/spinor_base.fw $(src)/spinor_base.o
$(obj)/spinor_ids.o:
    cp $(srctree)/$(src)/spinor_ids.fw $(src)/spinor_ids.o
#endif
```

- [x] gxmisc 编译成库
- [x] 完善 ops，添加一个目录，在下面加测试程序
  - [x] flash_test.h 中的测试程序(comtest、wptest) 放在 `include/spi_flash.h`
- [x] uboot 的原有驱动尽量不动，新增的尽量和 gxloader 相同
- [x] `flash_into_test` 中 gxflash_get_info 中获取 name 用 %d 打印 %s
- [x] uboot 编译时要修改 bootdelay：
  - make menuconfig
  - (5) delay in seconds before automatically booting
  - 在 uboot 目录下 make ARCH=arm CROSS_COMPILE=arm-none-eabi- -j12
  - set env也要配置
- [x] 在起 kernel 的时候使用的语法：`sf read offs addr size`
  - 所以在 sf.c 中不能直接删掉 `mtd_arg_off_size`
  - 需要传递一个 flash 的整体大小(由于使用的是 gxmisc 的库，所以uboot 完全没有 flash 的信息)，来判断一下要读取的数据没有超过 flash 的容量
- [x] comtest 的随机数
- [x] wptest 和 wplock、wpstatus 没测
- [x] speedtest 需要添加 时钟 来计时
- [x] 目前 flash uid 会挂掉
  - buf 的地址是 0x7fe4a34
  - malloc 出来的地址是 0x5f924b8，在调到 gxmisc 库的时候 buf 的地址变成了 0x7f94d65，然后在 spi 的驱动中，读取数据到 0x7f84d65 这个地址的时候出错的
  - 测试其它指令：flash read 0x0 0x100000 0x10，将flash 0 地址的数据读取长度 0x10 到 0x100000，传下来的buf地址一直都是 0x100000，到了调用 gxmisc 库的时候buf地址变成了 0x5f8d5c4     0x7feca8d
  - `由于字节对齐导致的问题`
- [x] flash all0 测试太慢，后面再测
- [ ] local_buf 来判断一下n_rx%4，rx_buf = local_buf+n_rx+n_rx+4，保证这个地方的地址是四字节对齐传到 transfer，定义数组的时候编译器默认会分配一个四字节对齐的地址，并且在transfer 中 32bit 的位置判断一下地址对齐，Inc 变成了 4

```Diff
diff --git a/drivers/spi/spi_v2.c b/drivers/spi/spi_v2.c
index db93f9f..4c423b6 100644
--- a/drivers/spi/spi_v2.c
+++ b/drivers/spi/spi_v2.c
@@ -40,12 +40,16 @@ int spi_write_then_read(struct spi_device *spi,
        struct spi_message  message;
        struct spi_transfer x[2];
        unsigned char       *local_buf;
+       unsigned char n_tx_align = 0;
+
+       if (n_tx % 4)
+               n_tx_align = 4 - (n_tx % 4);
 
        /* Use preallocated DMA-safe buffer.  We can't avoid copying here,
         * (as a pure convenience thing), but we can keep heap costs
         * out of the hot path ...
         */
-       if ((n_tx + n_rx) > SPI_BUFSIZ)
+       if ((n_tx + n_tx_align + n_rx) > SPI_BUFSIZ)
                return -EINVAL;
 
        spi_message_init(&message);
@@ -63,7 +67,7 @@ int spi_write_then_read(struct spi_device *spi,
 
        memcpy(local_buf, txbuf, n_tx);
        x[0].tx_buf = local_buf;
-       x[1].rx_buf = local_buf + n_tx;
+       x[1].rx_buf = local_buf + n_tx + n_tx_align;
 
        /* do the i/o */
        status = spi_sync(spi, &message);
- 不使用 dma 传输，读env

![](https://secure2.wostatic.cn/static/9qg1TTVcrZRch2s728bFBn/image.png)

- 使用 dma 传输，读了 几十个字节之后停住了，没有时钟了(读的数据都是对的)
    - 修改 dw_writel(dws, SPIM_RXFTLR, (dws->rx_fifo_len)-1); 没用
    - 为什么传输的地址要减去 虚拟地址
        - DMA 是通过 AHB 总线访问的
        - 怎么在芯片架构图上看出来 fmc 是怎么跟总线接的？
```

# DMA ：

## 传输配置：

![img](https://secure2.wostatic.cn/static/dTKcquoKs3bhX6Ti5Vk7b2/image.png)

- 一次burst长度：3 * *4*(ATW  * ARLEN) = 12 Bytes
- 总共读取长度：12 * 32 / 4 = 52 Bytes

## 水线的概念

### 寄存器：

![img](https://secure2.wostatic.cn/static/4erBWhoo3vwGv5SpkGUYXv/image.png)

- SR(0x28)一直是busy状态，芯片回复：CMPLTD_DF不是实时更新的，会在busy由1到0时更新，但是读寄存器busy为1

## 中断：

![img](https://secure2.wostatic.cn/static/k5RwD9uhTvnPQ9YKaetBcR/image.png)

- dma 的中断触发判断 ISR (0x30) 寄存器中的第 11 bit 位 SPI_INT_DONEM是否为1
- ip 的中断有很多种，soc 的中断号只有一个，怎么建立联系的？
  - 根据判断 ip 的中断状态寄存器标志位来判断是什么中断?

# 进度记录：

- http://git.nationalchip.com/gerrit/#/c/96203/10
  - 能够启动 kernel、读写擦通过(读会存在大小端问题)
- http://git.nationalchip.com/gerrit/#/c/95886/1
  - 添加 ops 步骤
- http://git.nationalchip.com/gerrit/#/c/96203/13
- http://git.nationalchip.com/gerrit/#/c/96596/5
  - gxmisc 编译生成的 .a 进行解压，然后拷贝到 uboot
  - uboot 支持测试命令
- http://git.nationalchip.com/gerrit/#/c/96203/14
- http://git.nationalchip.com/gerrit/#/c/96596/6
  - gxmisc 执行脚本，uboot 修改一下 大小端可以直接启动 linux

# SOC 修改项测试：

- beta8：
  - 0指令，四倍速写、dma模式传输256个字节数据
  - 测试程序如下：

```text
# 1. cs enable                                                                                  
set *(int*)0xd1000014 = 0x3                                                                     
set *(int*)0xd1000014 = 0x2                                                                     
# 2. SSI disable                                                                                
set *(int*)0xd1800008 = 0x0                                                                     
set *(int*)0xd1800050 = 0x0                                                                     
set *(int*)0xd1800054 = 0x0                                                                     
# 3. CRTLR0 = (master | Quad | Transmit Only | 8bit)                                            
set *(int*)0xd1800000 = 0x80000407 | (0x2 << 22)                                                
# 4. TXFTLR = 0                                                                                 
set *(int*)0xd1800018 = 0x0                                                                     
# 5. SPI_CTRLR0 = (CLK_STRETCH_EN | 8bit Instruction | Instruction sent in Standard. address sen
set *(int*)0xd18000F4 = (1<<30) | (0x0 << 8) | 0x01                                            
# 6. CTRLR1 = 255                                                                               
set *(int*)0xd1800004 = 255                                                                     
# 7. BAUDR = 4                                                                                  
set *(int*)0xd1800014 = 0x4                                                                     
# 8. TXFTLR = 0x0                                                                               
set *(int*)0xd1800018 = 0x0                                                                     
# 9. DMACR = (IDMAE | AINC)                                                                     
set *(int*)0xd180004C = 0x04 | (1 << 6) | (2 << 3)                                              
# 10. Source Burset length                                                                      
set *(int*)0xd1800050 = 0xf << 8                                                                
# 11. memory address                                                                            
set *(int*)0xd1800128 = 0x1000                                                               
set *(int*)0xd180012C = 0x00000000                                                              
# 12. Device Adress                                                                             
set *(int*)0xd1800124 = 0x00                                                                    
# 13. Device Instruction code                                                                   
set *(int*)0xd1800120 = 0x32                                                                    
# 14. Slave Select Enable                                                                       
set *(int*)0xd1800010 = 0x1                                                                     
# 15. SSI Enable                                                                                
set *(int*)0xd1800008 = 0x1
- 寄存器如下：
(gdb) x/30x 0xd1800000
0xd1800000:     0x00800407      0x000000ff      0x00000000      0x00000000
0xd1800010:     0x00000001      0x00000004      0x00000000      0x00000007
0xd1800020:     0x00000000      0x00000000      0x00800006      0x00000000
0xd1800030:     0x00000000      0x00000800      0x00000000      0x00000000
0xd1800040:     0x00000000      0x00000000      0x00000000      0x00000054
0xd1800050:     0x00000f00      0x00000000      0xffffffff      0x3130332a
0xd1800060:     0x4615460f      0x4615460f      0x4615460f      0x4615460f
0xd1800070:     0x4615460f      0x4615460f
- 传输的数据源如下：
(gdb) x/100x 0x1000
0x1000 <do_bootm+16>:   0x4615460f      0xf103461c      0xdd2c0a04      0x68582210
0x1010 <do_bootm+32>:   0xf037a905      0x9b05fe35      0xb322781a      0xd0222a3a
0x1020 <do_bootm+48>:   0xd0202a23      0x49182209      0xf0106860      0x1eaaf9c7
0x1030 <do_bootm+64>:   0x0308f104      0x68c1b1a0      0x29014814      0x2107bf08
0x1040 <do_bootm+80>:   0x0f01f011      0xf8d0d107      0x42a140c0      0x4810dc03
0x1050 <do_bootm+96>:   0xfda1f037      0x2400e004      0x94029001      0xe00b9100
0x1060 <do_bootm+112>:  0x30fff04f      0x2301e00c      0x4b089302      0x93014642
0x1070 <do_bootm+128>:  0x730ff240      0x46539300      0x46304639      0xff26f00a
0x1080 <do_bootm+144>:  0xe8bdb007      0xbf0085f0      0x0004c7a8      0x00053d04
0x1090 <do_bootm+160>:  0x000401b3      0x460cb510      0xf0084803      0x4621fef9
0x10a0 <do_bootd+12>:   0x4010e8bd      0xbd34f00f      0x000401dd      0x4601b538
0x10b0 <image_info+4>:  0x48234604      0xfd6ff037      0xf00e4620      0x2801f82d
0x10c0 <image_info+20>: 0xd0024605      0xd0212802      0x481ee032      0xff00f00c
0x10d0 <image_info+36>: 0x4b1d6822      0xd001429a      0xe004481c      0xf00d4620
0x10e0 <image_info+52>: 0xb918fe3d      0xf00c481a      0xe027fef3      0xf00d4620
0x10f0 <image_info+68>: 0x4818ff35      0xfeecf00c      0xf00d4620      0xb908fe44
0x1100 <image_info+84>: 0xe7f04815      0xf00c4815      0xe016fee3      0xf00c4814
- 实际传输的波形如下：

    ![](https://secure2.wostatic.cn/static/9vKqe6fLg9yqMuxcGX8f2S/image.png)
- 总结：采用0指令的模式进行四倍速写操作，SR状态寄存器中CMPLETE 位 传输完成了 256个字节，与要传输的字节数相同，但传输时的数据是4个字节正常，另四个字节全0，循环往复。
- 读取的时候，擦除完成后的数据为全0xFF，但驱动解析到有非0xFF的数据
Lepus #flash singletest 0x0 0x100
sflash_gx_readl finish.
sflash erase finish.
read addr=0x0, len=256
sflash_gx_readl finish.
erase error, i = 4
    ![](https://secure2.wostatic.cn/static/c1ajiT9UG9mzKDDhEDbmjX/image.png)
- beta9
    - j-link 连不上
Connecting to J-Link...
J-Link is connected.
Firmware: J-Link V9 compiled May 17 2019 09:50:41
Hardware: V9.60
S/N: 69664030
Feature(s): RDI, GDB, FlashDL, FlashBP, JFlash
Checking target voltage...
Target voltage: 3.31 V
Listening on TCP/IP port 2331
Connecting to target...ERROR: Cortex-A/R-JTAG (connect): Could not determine address of core debug registers. Incorrect CoreSight ROM table in device?
ERROR: Could not connect to target.
Target connection failed. GDBServer will be closed...Restoring target state and closing J-Link connection...
Shutting down...
Could not connect to target.
Please check power, connection and settings.% 
    - 0指令模式，四倍速写、dma模式传输256个字节数据，
        - 写入的数据：
Lepus #md 0x1000
00001000: 4607b087 4615460e f103461c dd2e0a04    ...F.F.F.F......
00001010: 22106858 f037a905 9b05fc51 b332781a    Xh."..7.Q....x2.
00001020: d0242a3a d0222a23 68602209 f0104918    :*$.#*".."`h.I..
00001030: 1eaaf9af 0308f104 68c1b1b0 29014815    ...........h.H.)
00001040: 2107bf08 0f01f011 f8d0d107 428c40c0    ...!.........@.B
00001050: 4811db03 fbbdf037 9100e006 90012100    ...H7........!..
00001060: 91024638 e00d4631 30fff04f f240e00c    8F..1F..O..0..@.
00001070: 9300730f 46384b07 46424631 23019301    .s...K8F1FBF...#
00001080: 46539302 ff62f00a e8bdb007 bf0085f0    ..SF..b.........
00001090: 0004c360 000538c0 0003fd4b 460cb510    `....8..K......F
000010a0: f0084803 4621ff45 4010e8bd bd2cf00f    .H..E.!F...@..,.
000010b0: 0003fd75 4604b538 48234621 fb89f037    u...8..F!F#H7...
000010c0: f00e4620 2801f833 d0024605 d0212802     F..3..(.F...(!.
000010d0: 481ee032 ff04f00c 4b1d6822 d001429a    2..H...."h.K.B..
000010e0: e004481c f00d4620 b918fe45 f00c481a    .H.. F..E....H..
000010f0: e027fef7 f00d4620 4818ff3d fef0f00c    ..'. F..=..H....
        - 写入的波形：

            ![](https://secure2.wostatic.cn/static/7Vq4GGEzCsSWvNmpAp3Dyr/image.png)
        - beta10
            - 测试读写擦
            - 四倍速写时的寄存器

                ![](https://secure2.wostatic.cn/static/ij3oGAfiCCYZzpBEGkh9wR/image.png)
            - 程序修改

                ![](https://secure2.wostatic.cn/static/x3Kz3j4eM13fMZKZNxtyg4/image.png)

                ![](https://secure2.wostatic.cn/static/4RnuFTVfdN72LyuXNDtKLk/image.png)
- beta14
    - 内部DMA四倍速写，有完成中断，无busy，COMPLE_位未更新
```

# kernel 调试：

- uboot 改动：

![img](https://secure2.wostatic.cn/static/p7nHy5PmCnBvFREsMLdj4t/image.png)

- kernel改动

![img](https://secure2.wostatic.cn/static/9xk6MJqmDMdPABCewBuXJk/image.png)

![img](https://secure2.wostatic.cn/static/2DMSP6anQdFBmJyCdHfmGP/image.png)

- 驱动不匹配
  - menuconfig 中默认打开的是 gx_spi，需要修改成 designware_spi
- kernel 启动阶段无 mtd 分区
  - 修改 uboot 中的 cmdline
- 使用 nfs 挂载根文件系统

```text
setenv bootargs "console=ttyS0,115200 loglvel=6 root=/dev/nfs rw nfsroot=192.168.108.111:/opt/nfs ip=192.168.108.10:192.168.108.111::::eth0:off"
bootz 0x10000 - 0x1000

nfsboot=setenv bootargs  console=ttyS0,115200 ${mtdparts_default} nfsvers=3 loglevel=6 root=/dev/nfs rw nfsroot=${serverip}:${rootfs_dir} ip=${ipaddr}:${serverip}::::eth0:off;}

setenv bootargs "console=ttyS0,115200 ${mtdparts_default} nfsvers=3 loglevel=6 root=/dev/nfs rw nfsroot=${serverip}:${rootfs_dir} ip=${ipaddr}:${serverip}::::eth0:off;}"
```

setenv bootargs "console=ttyS0,115200 ${mtdparts_default} nfsvers=3 loglevel=6 root=/dev/nfs rw nfsroot=192.168.108.111:/opt/nfs ip=192.168.108.10:192.168.108.111::::eth0:off"

bootz 0x10000 - 0x1000

setenv bootargs "console=ttyS0,115200 loglevel=6 root=/dev/nfs rw nfsroot=192.168.108.111:/opt/nfs ip=192.168.108.10:192.168.108.111::::eth0:off"

## nfs 根文件系统

- ubuntu 主机上的 nfs 配置：

```text
/opt/nfs *(rw,sync,no_root_squash, no_subtree_check)
# 去掉 no_subtree_check
```

- bootargs:

```text
run netload
setenv bootargs "console=ttyS0,115200 ${mtdparts_default} loglvel=6 root=/dev/nfs rw nfsroot=192.168.108.111:/opt/nfs ip=192.168.108.10:192.168.108.111::::eth0:off"
bootz 0x10000 - 0x1000
```

- linux kernel 中需要去掉 dma，使用cpu的方式读，去掉setup中的四倍速
- nfs 根文件系统并添加 init 脚本位置
  - setenv bootargs "noinitrd console=ttyS0,115200 ${mtdparts_default}  init=/init loglvel=6 root=/dev/nfs rw nfsroot=192.168.108.111:/opt/nfs ip=192.168.108.10:192.168.108.111::::eth0:off"
- uboot 改动

```Diff
diff --git a/drivers/spi/dw_spi_v2.c b/drivers/spi/dw_spi_v2.c
index 26c6184..2c795ac 100644
--- a/drivers/spi/dw_spi_v2.c
+++ b/drivers/spi/dw_spi_v2.c
@@ -613,10 +613,10 @@ static int dw_half_duplex_dma(struct dw_spi *dws)
       flush_dcache_all();
 
 
-      dw_writel(dws, SPIM_SPI_CTRLR0,  (1<<30) | (0x2 << 8) | 0x02);// NEED TO MODIFY
+      dw_writel(dws, SPIM_SPI_CTRLR0,  (1<<30) | (0x0 << 8) | 0x01);// NEED TO MODIFY
       dw_writel(dws, SPIM_CTRLR1, dma_len -1);
       dw_writel(dws, SPIM_DMACR,  (1 << 6) | (2 << 3) | (1 << 2));
-      dw_writel(dws, SPIM_SPI_SPIDR,  0x37);        //TEST, NEED TO MODIFY
+//      dw_writel(dws, SPIM_SPI_SPIDR,  0x37);        //TEST, NEED TO MODIFY
       dw_writel(dws, SPIM_SPI_SPIAR,  0x0);        //TEST, NEED TO MODIFY
       dw_writel(dws, SPIM_AXIAWLEN,  0xf << 8);        //NEED TO MODIFY
       dw_writel(dws, SPIM_AXIARLEN,  0 << 8);        //NEED TO MODIFY
diff --git a/include/configs/lepus_common.h b/include/configs/lepus_common.h
index 5cea1d9..773bfca 100644
--- a/include/configs/lepus_common.h
+++ b/include/configs/lepus_common.h
@@ -91,7 +91,7 @@
     "bootz ${kernel_addr} - ${fdt_addr}\0"        \
                     \
   "nfsboot=setenv bootargs  " CONFIG_BOOTARGS         \
-    " ${mtdparts_default} loglevel=6 root=/dev/nfs rw "    \
+    " ${mtdparts_default} nfsvers=3 loglevel=6 init=/init root=/dev/nfs rw "    \
     "nfsroot=${serverip}:${rootfs_dir} "        \
     "ip=${ipaddr}:${serverip}::::eth0:off;"        \
     "bootz ${kernel_addr} - ${fdt_addr}\0"        \
diff --git a/include/configs/lepus_fpga.h b/include/configs/lepus_fpga.h
index f359be8..5651cbf 100644
--- a/include/configs/lepus_fpga.h
+++ b/include/configs/lepus_fpga.h
@@ -300,7 +300,7 @@
  *
  */
 
-#define MTDIDS_NOR    "spi32766.1"
+#define MTDIDS_NOR    "m25p80"
 #define MTDIDS_NAND    "spinand_mcu"
 #define MTDPARTS_DEFAULT  "mtdparts=${mtdids_default}:"      \
         "1m(mcu),"          \
```

- kernel 改动

```Diff
diff --git a/arch/arm/boot/dts/lepus.dtsi b/arch/arm/boot/dts/lepus.dtsi
index 32d4f4958..754c479c8 100755
--- a/arch/arm/boot/dts/lepus.dtsi
+++ b/arch/arm/boot/dts/lepus.dtsi
@@ -291,13 +291,14 @@
       interrupt-parent = <&intc>;
       interrupts = <GIC_SPI 22 IRQ_TYPE_LEVEL_HIGH>;
     };
+    */
 
-    dw_spi_gx: dw_spi_gx@82402000 {
-      compatible = "nationalchip-dw-spi";
+    dw_spi_v2: dw_spi_v2@0xd1800000 {
+      compatible = "nationalchip-dw-spi,v2";
       #address-cells = <1>;
       #size-cells = <0>;
-      reg = <0x82402000 0x1000>;
-      interrupts = <GIC_SPI 40 IRQ_TYPE_LEVEL_HIGH>;
+      reg = <0xD1800000 0x1000>;
+      interrupts = <GIC_SPI 95 IRQ_TYPE_LEVEL_HIGH>;
       num-cs = <4>;
       bus-num = <0>;
       reg-io-width = <2>;
@@ -309,7 +310,6 @@
         status            = "okay";
       };
     };
-    */
 
     uart0: serial0@0xa0000000 {
       compatible = "snps,dw-apb-uart";
diff --git a/drivers/spi/spi-dw-gx-v2.c b/drivers/spi/spi-dw-gx-v2.c
index 7fe5a97f7..b5873c130 100644
--- a/drivers/spi/spi-dw-gx-v2.c
+++ b/drivers/spi/spi-dw-gx-v2.c
@@ -47,6 +47,7 @@
 #define CONFIG_GENERAL_SPI_BUS_SN 0
 
 #define CYGNUS_DW_SPI_CS_BASE     0x0030a904
+#define LEPUS_DW_SPI_CS_BASE     0xd1000014
 
 #ifndef roundup
 #define roundup(x, y)     ((((x) + ((y) - 1)) / (y)) * (y))
@@ -100,7 +101,8 @@ static int spi_cs_init(struct dw_spi *dws)
       if (dws->regs != 0)
         dws->cs_regs = (void __iomem *)((uint32_t)dws->regs + 0x404);
       break;
-    default:
+    default:      //lepus
+      dws->cs_regs = ioremap(LEPUS_DW_SPI_CS_BASE, 4);
       break;
   }
 
@@ -120,6 +122,7 @@ static int spi_cs_deinit(struct dw_spi *dws)
     case 0x6631:  //canopus
       break;
     default:
+      iounmap(dws->cs_regs);
       break;
   }
 
@@ -276,10 +279,10 @@ static int dw_half_duplex_poll_transfer(struct dw_spi *dws)
 
         for (i = 0; i < items; i++) {
           data = dw_readl(dws, SPIM_TXDR);
-          data8[0] = data >> 24;
-          data8[1] = data >> 16;
-          data8[2] = data >> 8;
-          data8[3] = data >> 0;
+          data8[3] = data >> 24;
+          data8[2] = data >> 16;
+          data8[1] = data >> 8;
+          data8[0] = data >> 0;
           data8 += 4;
         }
         left -= items;
@@ -621,9 +624,9 @@ static int dw_pump_transfers(struct dw_spi *dws)
   dw_writel(dws, SPIM_CTRLR1, 0);
   dw_writel(dws, SPIM_DMACR,  0);
 
-  if (dws->hw_version >= 0x3130332a)
-    return dw_half_duplex_dma(dws);
-  else
+//  if (dws->hw_version >= 0x3130332a)
+//    return dw_half_duplex_dma(dws);
+//  else
     return dw_half_duplex_poll_transfer(dws);
 }
 
@@ -647,7 +650,7 @@ static int dw_spi_setup(struct spi_device *spi)
   struct device_context *spi_device_context;
   unsigned int frequence = 0;
 
-  spi->mode = SPI_RX_DUAL | SPI_RX_QUAD | SPI_TX_DUAL | SPI_TX_QUAD;
+  spi->mode = SPI_RX_DUAL | SPI_TX_DUAL;
 
   if (spi->bits_per_word == 0)
     spi->bits_per_word = 8;
```

## 四倍速测试

- 当前lepus 工程中 kernel 使用的 gxspinor_arm.fw 是直接拷贝 develop 分支的库
- develop 分支对应的 gitlog 如下：

```text
commit a8b423216e396206986d3a81106d350e85bc1b1c (HEAD, m/develop, develop/develop)
Author: liufei <liufei@nationalchip.com>
Date:   Tue Oct 18 10:20:47 2022 +0800

    update gxosal.fw
    
    Change-Id: If8d1d2f7a5e4fa9b0edf00e609363d8db62a687a

commit b17d558dd447a7dead7156636038a0e026c4a91e
Merge: ddd9d67f4 c59d8426b
Author: 刘非 <liufei@nationalchip.com>
Date:   Mon Oct 10 15:25:34 2022 +0800

    Merge "332587: 解决 philips rcmm 协议按键时间间隔比较长的情况会忽略第一个按键的情况" into develop
```

- 重新编译了 gxmisc 中对应的 库文件，并进行了更新

# SPI 模块验证

```
    之前验证的是FMC模块，连接了flash作为device进行测试，现在要同时添加SPI模块进行验证，连接电机驱动板作为device进行测试。
```

- 设备树：
  - FMC：(SPI0)
    - spi-flash
  - SPI：(SPI1)
    - spi-motor
- 驱动：
  - SPI的驱动
  - flash的驱动
  - motor的驱动
- uboot启动：
  - spi → probe
  - flash → probe
  - motor → probe

### uboot 设备树

```C
#0  device_bind_common (parent=0x5f910b0, drv=0x7fe4260, name=0x5f8f870 "spi@d1800000", platdata=platdata@entry=0x0 <v7_arch_cp15_set_acr>, driver_data=0, 
    of_offset=of_offset@entry=548, devp=devp@entry=0x5f8d4dc, of_platdata_size=0) at drivers/core/device.c:166
#1  0x07fa4ca0 in device_bind_with_driver_data (parent=<optimized out>, drv=<optimized out>, name=<optimized out>, driver_data=<optimized out>, of_offset=548, 
    devp=0x5f8d4dc) at drivers/core/device.c:221
#2  0x07fa5152 in lists_bind_fdt (parent=0x5f910b0, blob=0x5f8f610, offset=548, devp=0x0 <v7_arch_cp15_set_acr>) at drivers/core/lists.c:173
#3  0x07fa5282 in dm_scan_fdt_node (parent=0x5f910b0, blob=0x5f8f610, offset=548, pre_reloc_only=<optimized out>) at drivers/core/root.c:207
#4  0x07fa580a in simple_bus_post_bind (dev=0x5f910b0) at drivers/core/simple-bus.c:43
#5  0x07fa4ba8 in device_bind_common (parent=0x5f91028, drv=0x7fe443c, name=0x5f8f814 "soc", platdata=platdata@entry=0x0 <v7_arch_cp15_set_acr>, driver_data=0, 
    of_offset=of_offset@entry=456, devp=devp@entry=0x5f8d58c, of_platdata_size=0) at drivers/core/device.c:159
#6  0x07fa4ca0 in device_bind_with_driver_data (parent=<optimized out>, drv=<optimized out>, name=<optimized out>, driver_data=<optimized out>, of_offset=456, 
    devp=0x5f8d58c) at drivers/core/device.c:221
#7  0x07fa5152 in lists_bind_fdt (parent=0x5f91028, blob=0x5f8f610, offset=456, devp=0x0 <v7_arch_cp15_set_acr>) at drivers/core/lists.c:173
#8  0x07fa5282 in dm_scan_fdt_node (parent=0x5f91028, blob=0x5f8f610, offset=456, offset@entry=0, pre_reloc_only=<optimized out>) at drivers/core/root.c:207
#9  0x07fa52da in dm_scan_fdt (blob=<optimized out>, pre_reloc_only=pre_reloc_only@entry=false) at drivers/core/root.c:232
#10 0x07fa52fa in dm_init_and_scan (pre_reloc_only=<optimized out>) at drivers/core/root.c:257
#11 0x07fc40ce in initcall_run_list (init_sequence=0x7fdfc84) at lib/initcall.c:31
#12 0x07f9dd94 in board_init_r (new_gd=<optimized out>, dest_addr=<optimized out>) at common/board_r.c:1038



dm_scan_fdt_node 中找到了 spi1 这个节点
device_bind_common
    //(dev->name = name)
    
#0  device_probe (dev=0x5f91138) at drivers/core/device.c:284
#1  0x07fa54dc in uclass_get_device_tail (dev=0x5f91138, ret=<optimized out>, devp=0x5f8d520) at drivers/core/uclass.c:341
#2  0x07fa5516 in uclass_get_device_by_seq (id=UCLASS_SPI, seq=0, devp=0x5f8d520) at drivers/core/uclass.c:385
#3  0x07fb78d2 in spi_get_bus_and_cs (busnum=0, cs=0, speed=1000000, mode=3, drv_name=0x7fdb072 "spi_flash_std", dev_name=0x5f91490 "spi_flash@0:0", busp=0x5f8d55c, devp=0x5f8d558) at drivers/spi/spi-uclass.c:278
#4  0x07faf52a in spi_flash_probe_bus_cs (busnum=0, cs=0, max_hz=1000000, spi_mode=3, devp=devp@entry=0x5f8d5a4) at drivers/mtd/spi/sf-uclass.c:197
#5  0x07faf54a in spi_flash_probe (bus=<optimized out>, cs=<optimized out>, max_hz=<optimized out>, spi_mode=<optimized out>) at drivers/mtd/spi/sf-uclass.c:169
#6  0x07f9f0a0 in spi_nor_env_relocate_spec () at common/env_sf.c:329
#7  0x07f9dcbc in initr_env () at common/board_r.c:506
#8  0x07fc4152 in initcall_run_list (init_sequence=0x7fdfdcc) at lib/initcall.c:31
#9  0x07f9dd94 in board_init_r (new_gd=<optimized out>, dest_addr=<optimized out>) at common/board_r.c:1038

/* s25f128s 节点执行 probe 函数 */    
#0  device_probe (dev=0x5f911c8) at drivers/core/device.c:274
#1  0x07fb7938 in spi_get_bus_and_cs (busnum=0, cs=<optimized out>, speed=1000000, mode=3, drv_name=0x7fdb072 "spi_flash_std", dev_name=0x5f91490 "spi_flash@0:0", busp=0x5f8d55c, devp=0x5f8d558)
    at drivers/spi/spi-uclass.c:310
#2  0x07faf52a in spi_flash_probe_bus_cs (busnum=0, cs=0, max_hz=1000000, spi_mode=3, devp=devp@entry=0x5f8d5a4) at drivers/mtd/spi/sf-uclass.c:197
#3  0x07faf54a in spi_flash_probe (bus=<optimized out>, cs=<optimized out>, max_hz=<optimized out>, spi_mode=<optimized out>) at drivers/mtd/spi/sf-uclass.c:169
#4  0x07f9f0a0 in spi_nor_env_relocate_spec () at common/env_sf.c:329
#5  0x07f9dcbc in initr_env () at common/board_r.c:506
#6  0x07fc4152 in initcall_run_list (init_sequence=0x7fdfdcc) at lib/initcall.c:31
#7  0x07f9dd94 in board_init_r (new_gd=<optimized out>, dest_addr=<optimized out>) at common/board_r.c:1038



设备树匹配可以匹配到 SPI1节点
执行 device_probe 函数无法查看到 device 设备(struct udevice *dev)
```

### SPI驱动能够成功执行两次改动：

```Diff
diff --git a/arch/arm/dts/lepus.dtsi b/arch/arm/dts/lepus.dtsi
index e9de9ab..4fafd6c 100644
--- a/arch/arm/dts/lepus.dtsi
+++ b/arch/arm/dts/lepus.dtsi
@@ -13,7 +13,7 @@
 
        aliases {
                spi0 = &spi0;
-               /*spi1 = &spi1;*/
+               spi1 = &spi1;
        };
 
        cpus {
@@ -87,7 +87,7 @@
                };
 */
 
-               spi0: spi@00303000 {
+               spi0: spi@d1800000 {
                        compatible     = "snps,dw-apb-ssi";
                        #address-cells = <1>;
                        #size-cells    = <0>;
@@ -123,28 +123,29 @@
                        };
                };
 
-/*
-               spi1: spi@02302000 {
+
+               spi1: spi@91000000 {
                        compatible     = "snps,dw-apb-ssi";
                        #address-cells = <1>;
                        #size-cells    = <0>;
-                       reg            = <0x02302000 0x1000>;
+                       reg            = <0x91000000 0x1000>;
                        interrupts     = <0 10 4>;
                        num-cs         = <4>;
                        dma_ctrl       = <0>;
                        bits_per_word  = <8>;
                        bus-num        = <1>;
                        clocks         = <198000000>;
-                       cs-reg         = <0x0230a600>;
+                       cs-reg         = <0xa0000018>;
                        spi-max-frequency = <85000000>;
                        u-boot,dm-pre-reloc;
                        status         = "okay";
 
-                       flash1: s25fl128s@1 {
+/*
+                       motor1: tmi8150b@0 {
                                #address-cells = <1>;
                                #size-cells = <1>;
-                               compatible = "spi-flash";
-                               spi-max-frequency = <85000000>;
+                               compatible = "spi-motor";
+                               spi-max-frequency = <5000000>;
                                reg = <1>;
                        };
 
@@ -156,9 +157,11 @@
                                cs-reg            = <0x230a600>;
                                dma-reg           = <0x02800000>;
                                reg               = <1>;
-                       };
+                       };*/
                };
 
+/*
+
                dma0: dma@2800000{
                        compatible = "snps,dw-ahb-dmac";
                        #address-cells = <1>;
diff --git a/arch/arm/dts/lepus_fpga.dts b/arch/arm/dts/lepus_fpga.dts
index cdb286a..a499e12 100644
--- a/arch/arm/dts/lepus_fpga.dts
+++ b/arch/arm/dts/lepus_fpga.dts
@@ -72,3 +72,17 @@
                reg = <0>;
        };
 };
+
+&spi1 {
+       status = "okay";
+       spi-max-frequency = <25000000>;
+       u-boot,dm-pre-reloc;
+
+       motor0: tmi8150b@0 {
+               #address-cells = <1>;
+               #size-cells = <1>;
+               compatible = "spi-motor";
+               spi-max-frequency = <25000000>;
+               reg = <0>;
+       };
+};
diff --git a/arch/arm/mach-lepus/board.c b/arch/arm/mach-lepus/board.c
index d5fe929..ac328c8 100644
--- a/arch/arm/mach-lepus/board.c
+++ b/arch/arm/mach-lepus/board.c
@@ -56,6 +56,19 @@ int board_init(void)
        v |= ((1 << 0) | (1 << 8));
        writel(v, 0xb0100044);
 
+       // spi0 pinmux config
+       v = readl(0xb010001c);
+       v |= ((5 << 0) | (5 << 8) | (5 << 16) | (5 << 24));
+       writel(v, 0xb010001c);
+
+       v = readl(0xb0100020);
+       v |= ((5 << 0) | (6 << 8) | (6 << 16) | (6 << 24));
+       writel(v, 0xb0100020);
+
+       v = readl(0xb0100024);
+       v |= ((6 << 0) | (6 << 8));
+       writel(v, 0xb0100024);
+
        return 0;
 }
 
diff --git a/cmd/spi.c b/cmd/spi.c
index 0762867..f8c71c9 100644
--- a/cmd/spi.c
+++ b/cmd/spi.c
@@ -53,8 +53,8 @@ static unsigned int readlen = 0;
 static int do_spi_xfer(int bus, int cs)
 {
        int i;
-       g_spi_master.bus_num= bus;
-       g_spi_master.spi.chip_select  = cs;
+       g_spi_master.bus_num= 1;
+       g_spi_master.spi.chip_select  = 0;
        g_spi_master.spi.max_speed_hz = 1000000;
        g_spi_master.spi.mode         = 0;
 
diff --git a/drivers/mtd/spi/Makefile b/drivers/mtd/spi/Makefile
index ecf66f8..e08102a 100644
--- a/drivers/mtd/spi/Makefile
+++ b/drivers/mtd/spi/Makefile
@@ -17,6 +17,7 @@ obj-$(CONFIG_SPI_FLASH) += sf_probe.o spi_flash.o sf_params.o sf.o
 
 ifndef CONFIG_SPL_BUILD
 obj-$(CONFIG_GX_SPI_FLASH) += gx_sf_probe.o libspinor_arm.o
+obj-y += gx_motor_probe.o
 endif
 obj-$(CONFIG_SPI_FLASH_DATAFLASH) += sf_dataflash.o
 obj-$(CONFIG_SPI_FLASH_MTD) += sf_mtd.o
diff --git a/drivers/mtd/spi/gx_sf_probe.c b/drivers/mtd/spi/gx_sf_probe.c
index d7e8217..7c1fcb1 100644
--- a/drivers/mtd/spi/gx_sf_probe.c
+++ b/drivers/mtd/spi/gx_sf_probe.c
@@ -384,6 +384,7 @@ static void spi_flash_std_erase(struct udevice *dev, unsigned int addr, unsigned
 
 static int dw_spi_flash_std_probe(struct udevice *dev)
 {
+       printf("%d %s hello flash!\n", __LINE__, __FUNCTION__);
        struct spi_slave *slave = dev_get_parent_priv(dev);
        struct spi_flash *flash;
 
diff --git a/drivers/mtd/spi/sf-uclass.c b/drivers/mtd/spi/sf-uclass.c
index dfbd841..ed54d99 100644
--- a/drivers/mtd/spi/sf-uclass.c
+++ b/drivers/mtd/spi/sf-uclass.c
@@ -194,8 +194,11 @@ int spi_flash_probe_bus_cs(unsigned int busnum, unsigned int cs,
        snprintf(name, sizeof(name), "spi_flash@%d:%d", busnum, cs);
        str = strdup(name);
 #endif
+       ret = spi_get_bus_and_cs(1, cs, max_hz, spi_mode,
+                                 "spi_motor_std", str, &bus, &slave);
        ret = spi_get_bus_and_cs(busnum, cs, max_hz, spi_mode,
                                  "spi_flash_std", str, &bus, &slave);
+
        if (ret)
                return ret;
 
diff --git a/drivers/spi/dw_spi_v2.c b/drivers/spi/dw_spi_v2.c
index 3542ec4..96719b8 100644
--- a/drivers/spi/dw_spi_v2.c
+++ b/drivers/spi/dw_spi_v2.c
@@ -200,7 +200,7 @@ static inline void spi_umask_intr(struct dw_spi *dws, uint32_t mask)
 
 /* SPI FLASH */
 #define CONFIG_FLASH_SPI_CLK_SRC          200000000    /* 200MHz */
-#define CONFIG_SF_DEFAULT_CLKDIV          4            /* 分频数必须为偶数且非0 */
+#define CONFIG_SF_DEFAULT_CLKDIV          8            /* 分频数必须为偶数且非0 */
 #define CONFIG_SF_DEFAULT_SAMPLE_DELAY    1
 //#define CONFIG_SF_DEFAULT_SPEED           (CONFIG_FLASH_SPI_CLK_SRC / CONFIG_SF_DEFAULT_CLKDIV)
 
@@ -715,9 +715,9 @@ static int dw_pump_transfers(struct dw_spi *dws)
        dw_writel(dws, SPIM_CTRLR1, 0);
        dw_writel(dws, SPIM_DMACR,  0);
 
-       if (dws->hw_version >= 0x3130332a)
-               return dw_half_duplex_dma(dws);
-       else
+//     if (dws->hw_version >= 0x3130332a)
+//             return dw_half_duplex_dma(dws);
+//     else
                return dw_half_duplex_poll_transfer(dws);
 }
 
@@ -773,7 +773,7 @@ static int dw_spi_setup(struct spi_device *spi)
 #ifdef CONFIG_ENABLE_SPI_QUAD
        spi->mode = SPI_RX_QUAD | SPI_TX_QUAD | SPI_TX_DUAL | SPI_RX_DUAL;
 #else
-       spi->mode = SPI_TX_DUAL | SPI_RX_DUAL;
+       spi->mode = 0;
 #endif
        uint8_t reg_data = (uint8_t) ((spi->mode>>8)&0xff);
 
@@ -875,8 +875,8 @@ static void spi_hw_init(struct dw_spi *dws)
        spi_cs_control(dws, 0, MRST_SPI_DISABLE);
 }
 
-static struct dw_spi     dwspi         = {0};
-static struct spi_master dw_spi_master = {0};
+static struct dw_spi     dwspi[2]         = {{0}, {0}};
+static struct spi_master dw_spi_master[2] = {{0}, {0}};
 
 static enum interrupt_return spi_master_irq_handler(uint32_t interrupt, void *pdata)
 {
@@ -1011,21 +1011,25 @@ static int dw_spi_set_mode(struct udevice *bus, uint mode)
 
 static int dw_spi_probe(struct udevice *bus)
 {
+       static int bus_num = 1;
+       static int init_device = 0;
        struct dw_spi_platdata *plat = dev_get_platdata(bus);
-       struct spi_master *master = &dw_spi_master;
+       struct spi_master *master = &dw_spi_master[init_device];
        struct dw_spi     *dws    = NULL;
        int    ret  =     -ENOMEM;
 
-       device_list_init();
+       if (!init_device) {
+               device_list_init();
+       }
 
-       spi_master_set_dws(master, &dwspi);
+       spi_master_set_dws(master, &dwspi[init_device]);
        dws                    = spi_master_get_dws(master);
        dws->regs              = (uint32_t)plat->regs;                          // regs addr
        dws->master            = master;
        dws->hw_version        = dw_readl(dws, SPIM_VERSION_ID);
        dws->mode_reg          = (uint32_t)plat->mode_reg;

-       master->bus_num        = CONFIG_GENERAL_SPI_BUS_SN;
+       master->bus_num        = bus_num;
        master->num_chipselect = CONFIG_SPI_SLAVE_NUM_MAX;
        master->cleanup        = dw_spi_cleanup;
        master->setup          = dw_spi_setup;
@@ -1052,6 +1056,8 @@ static int dw_spi_probe(struct udevice *bus)
 #ifdef CONFIG_ENABLE_IRQ
        gx_request_interrupt(plat->irq, IRQ, spi_master_irq_handler, dws);
 #endif
+       bus_num--;
+       init_device++;
        return 0;
 }
 
@@ -1062,6 +1068,9 @@ static int dw_spi_ofdata_to_platdata(struct udevice *bus)
        int node = bus->of_offset;
 
        plat->regs = (uint32_t)dev_get_addr(bus);
+
+       printf("dw_spi_ofdata_to_platdata:0x%x\n", plat->regs);
+
        plat->cs_regs = (uint32_t)fdtdec_get_uint(gd->fdt_blob, bus->of_offset,"cs-reg", 0xd1000014);
        plat->mode_reg = (uint32_t)fdtdec_get_uint(gd->fdt_blob, bus->of_offset,"spi-mode-reg", 0xd0000008);
        plat->irq = (uint32_t)fdtdec_get_uint(gd->fdt_blob, bus->of_offset,"interrupts", 63);
diff --git a/include/configs/lepus_fpga.h b/include/configs/lepus_fpga.h
index 5651cbf..cd74adc 100644
--- a/include/configs/lepus_fpga.h
+++ b/include/configs/lepus_fpga.h
@@ -202,7 +202,7 @@
  * Designware SPI support
  */
 #ifdef CONFIG_CMD_SPI
-#define CONFIG_DEFAULT_SPI_BUS         0
+#define CONFIG_DEFAULT_SPI_BUS         1
 #define CONFIG_DEFAULT_SPI_CS          0
 #define CONFIG_DEFAULT_SPI_MODE                SPI_MODE_0
```



### Motor

- 封装ops：
  - 工作模式：手动/自动
  - 电机方向&step模式：
  - 速度设置：
  - 目标圈数设定：
  - 目标相位设定：
  - 实时圈数读取：
  - X轴电机驱动
  - X轴电机停止



 



## 问题

1. spi的probe会运行几次，由于设备树中描述了两个SPI控制器，它们有着不同的地址，通过`spi_register_master`注册到链表上去，所以是不是在probe函数里面master→bus_num不能为一个固定值，然后device在初始化的时候跟master→bus_num匹配成功，表示这个device挂载在哪一个SPI上

   1. spi的probe函数会执行两次，而且是当挂载在spi总线下的device和驱动匹配成功之后才会执行spi的probe函数

2. 电机驱动芯片

   spi和jtag的端口映射问题，SPI0.SDO和SPI0.SDI引脚soc编反了

3. CH12_MD0寄存器配置成逆时针转模式时，info读取到的当前圈数确实是一直在递减，但是实际的电机转动方向还是顺时针转动，无法通过程序来控制电机切换正反转吗？

   - 电机接线问题，已解决

4. `dev_get_priv` 中的priv空间是在什么时候malloc的，看接口`driver/core/device.c  device_probe`

   - 由 motor 的驱动中 priv_auto_malloc_size 决定





# DMA 模块验证

- 使用链表测试：Source/Destination
  - 通道`Source Master Select`：AHB Master 3
  - 通道`Destination Master Seltct`：AHB Master 2

## 使用链表模式精简命令测试

1. 配置 LLI.CTLx 寄存器

   1. 配置传输模式为M2M:`ctlx[22:20] = 0x00`
   2. 配置传输特征：
      1. 源传输Width(SRC_TR_WIDTH)：`ctlx[6:4] = 0X01`
      2. 目标传输Width(DST_TR_WIDTH)：`ctlx[3:1] = 0x01`
      3. 源Master选择(SMS)：`ctlx[26:25] = 0x00`
      4. 目标Master选择(DMS)：`ctlx[24:23] = 0x00`
      5. 源地址递增(SINC)：`ctlx[10:9] = 0x00`
      6. 目标地址递增(DINC)：`ctlx[8:7] = 0x00`

2. 将通道配置写入`CFGx`寄存器

   1. 硬件/软件握手信号选择(HS_SEL_SRC/HS_SEL_DST):`cfgx[11] = 0x0 cfgx[10] = 0x0`
      - 由于配置为内存到内存传输模式，故此配置无效
   2. 如果源或目标外设的硬件握手信号有效，对齐源和目标外设的握手接口通过配置`SRC_PER、DEST_PER`：`cfgx[46:43] = 0x0 cfgx[42:39] = 0x0`

3. 按照如下流程配置连续传输：

   1. `LLI.LLPx`寄存器代表所有的LLI进入内存到下一个链表项
   2. `LLI.SARx、LLI.DARx`寄存器表示所有的LLI进入内存去启动源/目标块地址预取

   ```c
   for (i = 0; i <4; i++)
       if (i == 3)
           llpx = 0;	// end of LLI
   	else
           llpx = llp_addr + 20; // start of next LLI
   
   // Program SAR
   // Program DAR
   // Program LLP
   // Program CTL
   // Update Pointers
   ```

4. 如果Gather配置使能(DMAH_CHx_SRC_GAT_EN)和(CTLx_SRC_GATHER_EN)，则编程SGRx寄存器到通道1
5. 如果Scatter配置是能(DMAH_CHx_DST_SCA_EN)和(CTLx_DST_SCATTER_EN)，则编程DSRx寄存器到通道1
6. 清掉所有发生的中断，写入中断清除寄存器
7. 最后，使能通道(ChEnReg.CH_EN)，传输开始执行





# DMA M2M 测试修改项

```c
/* dmac.c */
DW_LIST_SET
    LLP = xxx | 2
dw_dmac_xfer_m2m
    LLP[0] = xxx | 0x2

get_dma_ctrl
    index = 0
    
/* dmac.h */
#define DWC_COM_CTRL0  DWC_CTLL_DST_MSIZE(1) | DWC_CTLL_SRC_MSIZE(1)
    
#define DWC_ME2ME_CTRLR0
    	DWC_CTLL_DMS(1) | DWC_CTLL_SMS(1)
    
/* dts */
ctrl_num = 0

/* cmd/dma.c */
dw_dma_xfer(xxx, xxx, 0)
    
    
    
/* cmd/dma.c */
struct dw_lli {
        /* values that are not changed by hardware */
        volatile unsigned int  sar;
        volatile unsigned int  dar;
        volatile unsigned int  llp;      // chain to next lli
        volatile unsigned int  ctl[2];
        volatile unsigned int  dstat;
};

#define DCACHE_LINE_SIZE  16
static int dma_m2m_lli_sample(unsigned int dma_src, unsigned int dma_dest)
{
        struct dw_lli lli __attribute__ ((aligned(DCACHE_LINE_SIZE)));;

        *(volatile unsigned int *)0x93000338 = 0xffffffff; // clearTfr
        *(volatile unsigned int *)0x93000340 = 0xffffffff; // clearBlock
        *(volatile unsigned int *)0x93000348 = 0xffffffff; // clearSrcTran
        *(volatile unsigned int *)0x93000350 = 0xffffffff; // clearDstTran
        *(volatile unsigned int *)0x93000358 = 0xffffffff; // clearErr

        *(volatile unsigned int *)0x93000310 = 0xff00; // MaskTfr
        *(volatile unsigned int *)0x93000318 = 0xff00; // MaskBlock
        *(volatile unsigned int *)0x93000320 = 0xff00; // MaskSrcTran
        *(volatile unsigned int *)0x93000328 = 0xff00; // MaskDstTran

        *(volatile unsigned int *)0x93000000 = lli.sar = dma_src; // channel 0 SAR
        *(volatile unsigned int *)0x93000008 = lli.dar = dma_dest; // channel 0 DAR

	/* LLP_SRC_EN | LLP_DST_EN | SMS(1) | DMS(1) | SRC_MSIZE(32) | DEST_MSIZE(32) | SRC_TR_WIDTH(32) | DST_TR_WIDTH(32) | INT_EN */
        *(volatile unsigned int *)0x93000018 = lli.ctl[0] = 0x1a812025 ; // channel 0 CTL low 32 bit
        *(volatile unsigned int *)0x93000040 = 0xc00;      // channel 0 CFG low 32 bit
        *(volatile unsigned int *)0x93000044 = 0x00000002; // channel 0 CFG high 32 bit

        lli.ctl[0]= lli.ctl[0] & (~(0x03 << 27));
        lli.ctl[1]= 0x40;
        lli.dstat = 0;
        lli.llp = 0;

	flush_dcache_all();

        *(volatile unsigned int *)0x93000010 = (((unsigned int)&lli) & 0x0fffffff) | 1; // channel 0 LLP low 32 bit

        *(volatile unsigned int *)0x93000398 = 0x1; // DmaCfgReg
        *(volatile unsigned int *)0x930003a0 = 0x101; // ChEnReg

	while (((*(volatile unsigned int *) 0x930002c0) & 0x01) == 0);

        *(volatile unsigned int *)0x93000398 = 0x0; // DmaCfgReg

	return 0;
}


int do_test_dma(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint8_t *dma_src  = NULL;
	uint8_t *dma_dest = NULL;
	uint32_t len      = 8;
	int i;

	printf("argc = %d\n", argc);

	if(argc >= 3){
		printf("\n---------- dma test memory to memory ----------\n");
		dma_src  = (uint8_t*)simple_strtoul(argv[1], NULL, 16);
		dma_dest = (uint8_t*)simple_strtoul(argv[2], NULL, 16);
		len      = simple_strtoul(argv[3], NULL, 16);
	}

	printf("src addr : 0x%x, dest addr: 0x%x\n",(uint32_t)dma_src, (uint32_t)dma_dest);

	dma_xfer(dma_dest, dma_src, len, DMA_MEM_TO_MEM, 0);

	for(i = 0; i < len; ++i){
		if(*dma_src != *dma_dest){
			printf("memcmp error: <0x> src = %d, dest = %d\n", \
				(uint32_t)dma_src, (uint32_t)dma_dest);
			break;
		}
		++dma_src;
		++dma_dest;
	}



#if 0

	unsigned int dma_src = 0, dma_dest = 0;

	dma_src  = simple_strtoul(argv[1], NULL, 16);
	dma_dest = simple_strtoul(argv[2], NULL, 16);
	dma_m2m_lli_sample(dma_src, dma_dest);
#endif

	return 0;
    
    
    
/* defconfig */
CONFIG_DW_DMAC=y
CONFIG_CMD_DMA=y
```





 P2M DMA 操作中仍然遇到了 FIFO_EMPTY 的问题，这可能是由以下原因之一导致的：

1. 目标内存地址不可用：请确保目标内存的地址空间可用，未受到访问限制，目标内存地址正确。
2. DMA 长度设置不正确：确保 DMA 传输长度以及每个 DMA 事务的传输大小已经正确设置。如果 DMA 传输的长度过短，可能会导致 FIFO_EMPTY 位被置高，因为 DMA 引擎无法在短时间内将足够的数据传输到目的地。
3. 目标内存缓冲区已满：请检查目标内存缓冲区是否存在存储数据的瓶颈。如果目标内存区域的缓冲区已满，DMA 引擎将无法写入数据，并触发 FIFO_EMPTY 位的状态。
4. DMA 通道资源冲突：DMA 通道和其他系统资源之间存在冲突，从而导致 DMA 传输失败。请检查 DMA 通道和其他系统资源之间的协议和配置是否正确。

综上所述，您可以进一步排除这些问题的可能性，以确保 DMA P2M 操作能够在正确的条件下进行传输。





# DMA P2M test case

```c
static int dma_p2m_lli_sample(unsigned int dma_src, unsigned int dma_dest)
{

#if 1
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

	/* cs */
	*(volatile unsigned int *)0xa0000018 = 0x3;// CS_INIT
	*(volatile unsigned int *)0xa0000018 = 0x2;// CS_ENABLE

	//dw_writel(dws, SPIM_CTRLR0, cr0);
	/* 1 << 31| ((0x500) << 0x8)& (0x3 << 0x8) | 0x7 | 0x1 << 10*/
	*(volatile unsigned int *)0x91000000 = 0x80000407;// master | spi_mode(0) | bits_per_word(8) | tmode_to
	//dw_writel(dws, SPIM_SPI_CTRLR0, 0x0000202);
	*(volatile unsigned int *)0x910000f4 = 0x202;// spi_ctrlr0
	//spi_enable_chip(dws, 1);
	*(volatile unsigned int *)0x91000008 = 0x1;// enable chip

	//items = dw_readl(dws, SPIM_TXFLR) & rx_len_mask;
//        while ((*(volatile unsigned int *) 0x91000020) == 0x00);
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
        while (((*(volatile unsigned int *) 0x91000028) & 0x04) != 0x04);
	///* 等待发送完成 */
	//wait_till_not_busy(dws);
        while (((*(volatile unsigned int *) 0x91000028) & 0x01) == 0x01);


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

	*(volatile unsigned int *)0x9100001c = 0x1f;// RXFTLR


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
	*(volatile unsigned int *)0x91000000 = 0x80400807;// master | dual | spi_mode(0) | bits_per_word(8) | tmode_ro
//	*(volatile unsigned int *)0x91000000 = 0x80000807;// master | dual | spi_mode(0) | bits_per_word(8) | tmode_ro
	//			dw_writel(dws, SPIM_DMACR, 0x01);
	*(volatile unsigned int *)0x9100004c = 0x1;// DMACR
	//			wait_till_not_busy(dws);
        while (((*(volatile unsigned int *) 0x91000028) & 0x01) == 0x01);
	//			spi_enable_chip(dws, 0);
	*(volatile unsigned int *)0x91000008 = 0x0;// disable chip
	//			block_len = min((int)dma_len, DMA_BLOCK_SIZE);

	//			dw_writel(dws, SPIM_CTRLR1, block_len - 1);
	*(volatile unsigned int *)0x91000004 = 0x40;// CTRLR1

//			dma_xfer((void *)buffer, (void *)(dws->regs + SPIM_RXDR), block_len, DMA_DEV_TO_MEM, 0);
        struct dw_lli lli __attribute__ ((aligned(DCACHE_LINE_SIZE)));;

        *(volatile unsigned int *)0x93000338 = 0xffffffff; // clearTfr
        *(volatile unsigned int *)0x93000340 = 0xffffffff; // clearBlock
        *(volatile unsigned int *)0x93000348 = 0xffffffff; // clearSrcTran
        *(volatile unsigned int *)0x93000350 = 0xffffffff; // clearDstTran
        *(volatile unsigned int *)0x93000358 = 0xffffffff; // clearErr

        *(volatile unsigned int *)0x93000310 = 0xff00; // MaskTfr
        *(volatile unsigned int *)0x93000318 = 0xff00; // MaskBlock
        *(volatile unsigned int *)0x93000320 = 0xff00; // MaskSrcTran
        *(volatile unsigned int *)0x93000328 = 0xff00; // MaskDstTran


        *(volatile unsigned int *)0x93000000 = lli.sar = dma_src; // channel 0 SAR
        *(volatile unsigned int *)0x93000008 = lli.dar = dma_dest; // channel 0 DAR

        /* LLP_SRC_EN | LLP_DST_EN | SMS(x) | DMS(x) | SRC_MSIZE(8) | DEST_MSIZE(32) | SRC_TR_WIDTH(8) | DST_TR_WIDTH(8) | INT_EN */
        *(volatile unsigned int *)0x93000018 = lli.ctl[0] = 0x18A0A401 ; // channel 0 CTL low 32 bit
        *(volatile unsigned int *)0x93000040 = 0x000;      // channel 0 CFG low 32 bit
        *(volatile unsigned int *)0x93000044 = 0x402; // channel 0 CFG high 32 bit

        lli.ctl[0]= lli.ctl[0] & (~(0x03 << 27));
        lli.ctl[1]= 0x40;
        lli.dstat = 0;
        lli.llp = 0;

        flush_dcache_all();

        *(volatile unsigned int *)0x93000010 = (((unsigned int)&lli) & 0x0fffffff) | 0x2; // channel 0 LLP low 32 bit

        *(volatile unsigned int *)0x93000398 = 0x1; // DmaCfgReg
        *(volatile unsigned int *)0x930003a0 = 0x101; // ChEnReg


//			spi_enable_chip(dws, 1);
*(volatile unsigned int *)0x91000008 = 0x1;// enable chip
//			dw_writel(dws, SPIM_TXDR, 0x00);
*(volatile unsigned int *)0x91000060 = 0x00;// dummy -> tx_buff

        while (((*(volatile unsigned int *) 0x930002c0) & 0x01) == 0);

//			/* wait spi rx fifo empty */
//			while(dw_readl(dws, SPIM_RXFLR) != 0);
//        while ((*(volatile unsigned int *) 0x91000024) != 0);

//			/* wait spi not busy */
//			while(dw_readl(dws, SPIM_SR) & 0x01);
//        while (((*(volatile unsigned int *) 0x91000028) & 0x01) == 0);


//	spi_enable_chip(dws, 0);
*(volatile unsigned int *)0x91000008 = 0x0;// disable chip


	*(volatile unsigned int *)0xa0000018 = 0x3;// CS_DISABLE

#endif

	return 0;
}
```



## dma 、cpu 模式都配置成单倍速

- dma读取端：

  - spi 配置成 单倍速、flash 读取使用双倍速：读取数据正确(仅读取MISO线上的数据)

    ![image-20230320161213361](image/image-20230320161213361.png)

![image-20230320161229684.png](image/image-20230320161229684.png)

- cpu 读取端：
  - spi 配置成 单倍速、flash 读取使用双倍速：读取数据正确(仅读取MISO线上的数据)![image-20230320162002237.png](image/image-20230320162002237.png)



## dma、cpu 端都配置成双倍速

- dma 读取端：

  - dma 配置成双倍速、flash 读取双倍速：读取数据()

    ![image-20230320162259082](image/image-20230320162259082.png)

![image-20230320162325497](image/image-20230320162325497.png)



- cpu 读取端：

  - spi 配置成双倍速、flash 读取双倍速

    ![image-20230320162418992](image/image-20230320162418992.png)

![image-20230320162436704](image/image-20230320162436704.png)





# DMA 驱动新增功能

- 对齐 scpu 中 dma 的驱动



## 使用方法：

​	以 uart 为例：

- 选择通道：`gx_dma_select_channel`
- Configure dma 
- 注册完成回调：gx_dma_register_complete_callback(channel, func, param)
- 配置dma_xfer：`gx_dma_xfer_int(dest, src, len, chanel, dma_config)`



以 spi 为例：

- 选择通道：`gx_dma_select_channel`
- Configure dma
- 调用dma：`gx_dma_xfer(dest, src, channel, dma_config)`
- 等待dma完成：`gx_dma_wait_complete(channel)`





# 1. 编译问题

- lepus 在 stage2 阶段的串口下载功能中需要用到 Flash 的几个功能，但是默认的配置文件中是没有开这几个配置选项的，需要看看

- - 有一些 uboot 本身的宏 ARM_FLASH 什么的这种东西，会跟 GXFLASH 这个配置项引起歧义，是否需要将 GXFLASH 这个宏放到 DRACO_FLASH 里面去
  - stage2 阶段的串口下载功能需要用到 flash 的几个功能，然后这个功能在 加 stage1 驱动的时候限定了，就是只能在 stage2 使用，但是stage1驱动里面会用到，就换一个宏来限定好了
  - 如果开了这个宏，那么就能用 flash 的这些功能，如果没开这个宏，那么就不能用 flash 的这些功能
  - stage1 和 stage2 原生 uboot 是共用一套 flash 驱动的，通过在 Makefile 中定义宏来进行区分，如果是 stage1 的话就用 spl_spi.c ，如果是 stage2 的时候就编 gx_spi_probe.c

- - - stage1 和 stage2 的策略是一样的，定义 uboot 的框架是一模一样的，所以只能同时编一个，之所以不用 stage2 的代码来给 stage1 用，是由于 stage2 的代码会用到 gxmisc，而 gxmisc 太大



- - 目前看起来 simple_spi.c 也还是会编，我不需要编
  - 有两套接口，一套是用来给 sf 命令起 kernel 用的，就是用户使用，这个接口只有读写擦
  - 另一套接口是用来做 flash 的测试程序的，供维护驱动人员使用，这一套能够完整测试 flash 
  - 两套接口的读写擦实现都是一样的，只不过上面封的名称不一样



测试情况：

- stage 1

- - 烧到 Flash 中能够正常启动
  - 用 douzhk 脚本 会生成一个 output_nor.bin ,然后和 u-boot.img 文件一起下载到 Flash 中

- stage 2

- - flash 测试

- uart 下载
- dma



- 【原因分析】由于 stage1 和 stage2 都会包含同样的头文件，为了让 flash 的测试程序只在 stage2 使用，使用宏 CONFIG_SPL_BUILD 来对 stage1 所支持的接口进行过滤(仅支持读) 	 	
- 串口下载会用到 擦除和写入、解除写保护的接口，所以需要打开 CONFIG_CMD_GXFLASH 宏来打开所有的 flash 接口，但是由于使用的是 CONFIG_SPL_BUILD 进行过滤的，打开了 CONFIG_CMD_GXFLASH 也没用，CONFIG_SPL_BUILD 是代表 stage1 编译的宏，而串口下载的这个.c stage1 和 stage2 阶段都会编译，所以这个宏一直存在，就会报上述的错误

- 【解决方法】使用 CONFIG_CMD_GXFLASH 来进行过滤 flash 的接口，不关心 stage1 还是 stage2 阶段编译，flash 接口是否完整取决于 CONFIG_CMD_GXFLASH





# 2. Stage1 flash 驱动

## 2.1. 改动点：

- stage1 的 flash 驱动代码需要有一个 probe 函数 	 	

- - probe 函数放在 flash 的 stage2 驱动中，stage1阶段也编译stage2的 flash 代码
  - probe 函数不要



- stage1 的 flash 驱动 read 接口，多一个 flash 的参数 	 	

- - 手动去掉这个flash 的参数
  - 在stage1 的flash 驱动中添加一个参数，但是不用

 	

## 2.2. 结构

stage1 flash 驱动： 	 	

- 使用 arch/arm/mach-lepus/spl_spinor.c 文件来实现 stage1 阶段的 flash 驱动
- 采用 uboot 的框架，跟 drivers/mtd/spi/gx_sf_probe.c 中的实现方式一致
- 在编译阶段用宏来区分，stage1 阶段编译 spl_spinor.c， stage2 阶段编译 gx_sf_probe.c

 	

## 2.3 验证方法

使用脚本：`./douzhk.sh spl/u-boot-spl.bin output_nor.bin` 生成 stage1 代码，然后加上 u-boot.img 组成 bootloader

```bash
#!/bin/bash

if [ $# != 2 ];then
	echo "usage   : $0 input_file output_file"
	echo "example : $0 spl/u-boot-spl.bin output_nor.bin"
	exit
fi

FILENAME=$1
OUTPUT_FILE_NAME=$2

# header
# struct boot_header {
# 	unsigned int medium_magic;
# 	unsigned int load_addr;
# 	unsigned int stage1_size;
# 	unsigned int stage1_crc;
# 	unsigned int reserved;
# 	unsigned int header_crc;
# };

ROUND_UP_SIZE=0
CRC_RESULT=0
ENDIAN_SWITCH_RESULT=0

# $1 : orign size
# $2 : align size
function roundup () {
	var_a=`expr $1 / $2`
	var_b=`expr $1 % $2`
	if [ $var_b -ne 0 ]; then
		var_c=`expr $var_a + 1`
		var_return=`expr $var_c \* $2`
	else
		var_return=`expr $var_a \* $2`
	fi

	ROUND_UP_SIZE=$var_return
	return $var_return
}

# $1 filename
# $2 file_len
# $3 paded_size
function cal_crc() {
	dd if=/dev/zero of=./$1 count=$3 bs=1 seek=$2
	CRC_RESULT=`crc32 $1`
}

# $1 input hex data
function switch_endian() {
	value=$1
	ENDIAN_SWITCH_RESULT=${value:0:2}${value:8:2}${value:6:2}${value:4:2}${value:2:2}
}

HEADER_SIZE=24
MEDIUM_MAGIC_NOR=0x55aa55aa
MEDIUM_MAGIC_NAND=0x55bb55bb
MEDIUM_MAGIC_UART=0x55cc55cc
#MEDIUM_MAGIC=$MEDIUM_MAGIC_UART
MEDIUM_MAGIC=$MEDIUM_MAGIC_NOR
#MEDIUM_MAGIC=$MEDIUM_MAGIC_NAND
LOAD_ADDR=0x80000000
RESERVED=0x00000000

# to be cal
STAGE1_SIZE=0
STAGE1_CRC=0
HEADER_CRC=0

PAGE_SIZE=2048
ALIGN_SIZE=$PAGE_SIZE
FILE_SIZE=`wc -c $FILENAME | cut -d ' ' -f 1`
ORIGN_SIZE=`expr $FILE_SIZE + $HEADER_SIZE`

# ------------------------------------ start calculate ------------------------------------

roundup $ORIGN_SIZE $ALIGN_SIZE
TOTAL_SIZE=$ROUND_UP_SIZE
PADED_SIZE=`expr $TOTAL_SIZE - $ORIGN_SIZE`

TMP_FILE_1="__tmp_file_1"
TMP_FILE_2="__tmp_file_2"
TMP_FILE_3="__tmp_file_3"
cat $1 > $TMP_FILE_1
cal_crc $TMP_FILE_1 $FILE_SIZE $PADED_SIZE

STAGE1_CRC=`printf 0x%08x 0x$CRC_RESULT`
STAEG1_SIZE=`printf 0x%08x $(expr $TOTAL_SIZE - $HEADER_SIZE)`

switch_endian $MEDIUM_MAGIC
MEDIUM_MAGIC=$ENDIAN_SWITCH_RESULT
switch_endian $LOAD_ADDR
LOAD_ADDR=$ENDIAN_SWITCH_RESULT
switch_endian $STAEG1_SIZE
STAEG1_SIZE=$ENDIAN_SWITCH_RESULT
switch_endian $STAGE1_CRC
STAGE1_CRC=$ENDIAN_SWITCH_RESULT

echo -n "$MEDIUM_MAGIC $LOAD_ADDR $STAEG1_SIZE $STAGE1_CRC $RESERVED" > $TMP_FILE_2
xxd -r -p $TMP_FILE_2 > $TMP_FILE_3
VAR_CRC=`crc32 $TMP_FILE_3`
HEADER_CRC=`printf 0x%08x 0x$VAR_CRC`

# switch endian
switch_endian $HEADER_CRC
HEADER_CRC=$ENDIAN_SWITCH_RESULT

echo -n "$MEDIUM_MAGIC $LOAD_ADDR $STAEG1_SIZE $STAGE1_CRC $RESERVED $HEADER_CRC" > $TMP_FILE_2

xxd -r -p $TMP_FILE_2 > $OUTPUT_FILE_NAME
cat $TMP_FILE_1 >> $OUTPUT_FILE_NAME

# ------------------------------------ clean ------------------------------------
rm $TMP_FILE_1 $TMP_FILE_2 $TMP_FILE_3
```







# 3. DMA 驱动

## 3.1. 改动点：

- 大部分是采用 scpu 中的驱动代码
- 还有部分是 uboot 原生代码



## 3.2. 测试方法：

- 由于 FMC 有内部 DMA，测试的是通用 DMA，需要使用 SPI0 来进行测试
- 设备树中屏蔽掉 spi1，然后把 fmc 的信息都修改成 spi0 的
- soc 把 spi0 引到了 jlink 接口上，用杜邦线按线序接好 flash 测试 dma
