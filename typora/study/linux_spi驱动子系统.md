# Linux SPI 驱动子系统

## 一、概览 Linux SPI 驱动子系统

### 1. 概要

​		Linux 中的 spi 驱动主要是由 spi 子系统来管理的，其核心代码位于 kernel/drivers/spi/spi.c 中。具体的 spi 控制器驱动也在 kernel/drivers/spi/目录下。

​		spi 通常是由 spi 控制器(ip)、spi 总线和连接在 spi 总线上的设备构成：这里的总线只是指物理的 spi 连线，并不是指 Linux 设备模型中逻辑上的总线盖帘。可以把 spi 总线和 spi 控制器看成是一体的，spi 总线就是 spi 控制器加上 spi 设备的连接线。spi 设备包含很多中，它可以是一个 spi 接口的 nor flash，例如 ST 的 M25P80，也可以是一个 spi 网卡，例如 ENC28J60。



### 2. spi 控制器

​		Linux 的 spi 子系统对 spi 控制器的描述使用的是 `struct spi_master` 这个数据结构，所以在内核中，一个 `spi_master` 就代表了一个 spi 控制器，或者说代表一个 spi 主机。

​		它的主要定义如下：

```c
抽象  	-->  数据结构
Soc(SPI) -->  IP 数量、每个IP的CS、IP连接的设备、IP的配置、IP的数据传输、IP的注销

struct spi_master {
     struct device   dev;
     struct list_head list;
     s16         bus_num;        //IP编号，如什么外设挂在SPIx下，总线和设备匹配时用到
     u16         num_chipselect; //片选数量，决定该控制器下面挂接多少个SPI设备
     u16         dma_alignment;
     u16         mode_bits;
     u16         flags;
     spinlock_t      bus_lock_spinlock;
     struct mutex        bus_lock_mutex;
     bool            bus_lock_flag;
     int   (*setup)(struct spi_device *spi);   //自己实现，
                                               //主要配置SPI控制器和工作方式
     int   (*transfer)(struct spi_device *spi,
            struct spi_message *mesg);         //不同的控制器要具体实现，
                                               //传输数据最终都要调用这个函数
     void   (*cleanup)(struct spi_device *spi); //注销时使用
};
```

​		在 Linux-2.6 以后引入设备模型概念，所有的设备和驱动都需要依附于总线。依附于 spi 总线的设备驱动对应的总线类型为 `spi_bus_type`，在内核的`drivers/spi/spi.c`中定义。

```c
struct bus_type spi_bus_type = {
        .name           = "spi",
        .dev_groups     = spi_dev_groups,
        .match          = spi_match_device,
        .uevent         = spi_uevent,
};                                                                                                        
EXPORT_SYMBOL_GPL(spi_bus_type);
```



### 3. spi 设备

​		Linux spi 子系统对 spi 总线上的设备用`struct spi_device`结构来描述，运行的内核中，通常一个`struct spi_device`对象对应一个 spi 设备：<font color=green>(此处描述的设备可以是 flash、网卡等等 spi 通信设备)</font>

​		`struct spi_device`主要用来描述连接在 spi 总线上的一个 spi 设备的一些电气信息。

```c
struct spi_device {
      struct device       dev; 
      struct spi_master   *master;      //挂载在哪个IP主控器下(此处用指针来指向master结构)
      u32         max_speed_hz;         //设备支持的最大速度
      u8          chip_select;          //设备的片选号
      u8          mode;                 //SPI的模式
      u8          bits_per_word;        //每次传输的位数
      int         irq;                  //中断号
      void        *controller_state;
      void        *controller_data;
      char        modalias[SPI_NAME_SIZE]; //设备名
 };
```



​		在 Linux 2.6.xx 版本的内核中可能会经常看到代码中用 `struct spi_board_info`来描述 spi 设备的电气信息，该结构体包含外设的片选号、总线号、模式以及传输速率等信息。

​		`struct spi_board_info`原型如下：

```c
struct spi_board_info {
    /* the device name and module name are coupled, like platform_bus;
     * "modalias" is normally the driver name.
     *
     * platform_data goes to spi_device.dev.platform_data,
     * controller_data goes to spi_device.controller_data,
     * irq is copied too
     */
    char        modalias[SPI_NAME_SIZE];
    const void  *platform_data;
    void        *controller_data;
    int     irq;

    /* slower signaling on noisy or low voltage boards */
    u32     max_speed_hz;

    /* bus_num is board specific and matches the bus_num of some
     * spi_master that will probably be registered later.
     *
     * chip_select reflects how this chip is wired to that master;
     * it's less than num_chipselect.
     */
    u16     bus_num;
    u16     chip_select;
    /* mode becomes spi_device.mode, and is essential for chips
     * where the default of SPI_CS_HIGH = 0 is wrong.
     */
    u8      mode;
    /* ... may need additional spi_device chip config data here.
     * avoid stuff protocol drivers can set; but include stuff
     * needed to behave without being bound to a driver:
     *  - quirks like clock rate mattering when not selected
     */
};
```

​		在板级初始化代码中，2.6.xx版本的内核中通常会用一个 `struct spi_board_info`数组来描述系统中的 spi 设备信息，然后调用 `spi_register_board_info()`将这些设备信息注册到系统中。<font color=red>这些 `struct spi_board_info`最终也会转换为`struct spi_device`结构。</font>



### 4. spi 设备驱动

​		spi 设备的驱动都会有一个 `struct spi_driver`结构体来描述，结构体中定义对应的操作函数指针，用来管理挂载在总线上的相关设备：

```c
struct spi_driver {
    const struct spi_device_id *id_table;
    int   (*probe)(struct spi_device *spi);
    int   (*remove)(struct spi_device *spi);
    void  (*shutdown)(struct spi_device *spi);
    int   (*suspend)(struct spi_device *spi, pm_message_t mesg);
    int   (*resume)(struct spi_device *spi);
    struct device_driver driver;  
}
```

​		对于驱动编写来说，spi 设备的驱动主要就是实现这个结构体中的各个接口，并将其注册到 spi 子系统中去。



### 5. 数据传输相关数据结构

​		`struct spi_transfer`数据结构定义如下：

```c
struct spi_transfer {    
    /* it's ok if tx_buf == rx_buf (right?)
     * for MicroWire, one buffer must be null
     * buffers must work with dma_*map_single() calls, unless
     *   spi_message.is_dma_mapped reports a pre-existing mapping
     */
    const void  *tx_buf;  //發送數據的緩衝區
    void        *rx_buf;  //接收數據的緩衝區
    unsigned    len;      //需要交換數據的長度

    dma_addr_t  tx_dma;  
    dma_addr_t  rx_dma;  

    unsigned    cs_change:1;  //片選信號
    u8      bits_per_word;    
    u16     delay_usecs; 
    u32     speed_hz;    

    struct list_head transfer_list;
};


struct spi_transfer {
        /* it's ok if tx_buf == rx_buf (right?)
        ┆* for MicroWire, one buffer must be null
        ┆* buffers must work with dma_*map_single() calls, unless
        ┆*   spi_message.is_dma_mapped reports a pre-existing mapping
        ┆*/
        const void      *tx_buf;	// 发送数据的缓冲区
        void            *rx_buf;	// 接收数据的缓冲区
        unsigned        len;		// 需要交换数据的长度

        dma_addr_t      tx_dma;
        dma_addr_t      rx_dma;
        struct sg_table tx_sg;
        struct sg_table rx_sg;

        unsigned        cs_change:1;	// 片选信号
        unsigned        tx_nbits:3;
        unsigned        rx_nbits:3;
#define SPI_NBITS_SINGLE        0x01 /* 1bit transfer */
#define SPI_NBITS_DUAL          0x02 /* 2bits transfer */
#define SPI_NBITS_QUAD          0x04 /* 4bits transfer */
        u8              bits_per_word;
        u16             delay_usecs;
        u32             speed_hz;

        struct list_head transfer_list;
};

```

​		一个 `struct spi_transfer`对象代表了一次单段的 spi 数据传输。`struct spi_transfer`结构体中记录了本段传输需要交换的数据和长度，传输的速度，传输时片选信号的变化情况。



​		另一个传输数据相关的结构体为`struct spi_message`，其定义如下：

```c
struct spi_message {
        struct list_head        transfers;

        struct spi_device       *spi;		// 传输数据到这个设备(使用结构指针指向设备)

        unsigned                is_dma_mapped:1;

        /* REVISIT:  we might want a flag affecting the behavior of the
        ┆* last transfer ... allowing things like "read 16 bit length L"
        ┆* immediately followed by "read L bytes".  Basically imposing
        ┆* a specific message scheduling algorithm.
        ┆*
        ┆* Some controller drivers (message-at-a-time queue processing)
        ┆* could provide that as their default scheduling algorithm.  But
        ┆* others (with multi-message pipelines) could need a flag to
        ┆* tell them about such special cases.
        ┆*/

        /* completion is reported through a callback */
        void                    (*complete)(void *context);
        void                    *context;
        unsigned                frame_length;
        unsigned                actual_length;
        int                     status;

        /* for optional use by whatever driver currently owns the
        ┆* spi_message ...  between calls to spi_async and then later
        ┆* complete(), that's the spi_master controller driver.
        ┆*/
        struct list_head        queue;
        void                    *state;

        /* list of spi_res reources when the spi message is processed */
        struct list_head        resources;
};

```

​		一个`struct spi_message`代表对一个设备进行一个多段 spi 数据传输。每一段传输其实就是使用上面提到的`struct spi_transfer`对象完成的。`struct spi_message`主要记录了这次传输针对的设备。上面提到的`struct spi_transfer`对象会被连接到`struct spi_message`对象中。<font color=red>将 `struct spi_transfer`连接成 `struct spi_message`</font>



### 6. 其它数据结构

​		对于3.0内核以前，ARM平台通常的做法是在板级代码中<font color=red>(1):</font>注册`struct platform_device`来描述各种控制器，例如`spi控制器`、`i2c控制器`等等。然后再<font color=red>(2):</font>向系统中注册`struct platform_driver`来管理和驱动对应的平台设备。

​		对于3.0内核以后引入了`Device Tree`来描述ARM平台及板级设备。这样依赖就不需要再板级代码手动的构建并注册描述 spi 的 `platform_device`数据了，一切都由 Device Tree来描述，并且有 Deivce Tree相关代码来解析对应 spi 的相关描述信息(of_xxx)，并自动创建并注册表示spi控制器的platform_device数据。与之对因的`platform_driver`和以前差不多，只不过支持 Device Tree后，platform_driver中可以通过设备树获取 spi 控制器的相关信息。<font color=red>(通过获取设备树信息来创建并注册spi_device和spi_driver)</font>

​	of_device_id 结构定义如下：

```c
struct of_device_id
{
    char name[32];
    char type[32];
    char compatible[128];
    const void *data;
};
```

​		引入设备树后 platform_driver 优先使用 <font color=red>`struct of_device_id`对象中的`compatible成员`</font>和`platform_device`中的`dev.of_node`中的`compatible`成员匹配的。

```c
static struct of_device_if xxx_spi_of_match[] = {
    {.compatible = "vendor, chip-spi", },
    {}
};
MODULE_DEVICE_TABLE(of, xxx_spi_of_match);
```

```c
struct spi_device_id{
    char name[SPI_NAME_SIZE]; 
    kernel_ulong_t driver_data  /* Data private to the driver */
            __attribute__((aligned(sizeof(kernel_ulong_t))));
}
```



## 二、Linux SPI 控制器驱动

### 1. 概览

​		对于ARM平台来说，大多数CPU都是Soc。SPI 控制器被集成在CPU内部，SPI总线上的数据传输过程通常就是这个SPI控制器来控制的。为了使SPI控制器能工作在Linux SPI子系统中，我们就需要针对CPU内部的SPI控制器编写一个驱动。

​		在 Linux SPI 系统中 `struct spi_master`就对应一个 SPI 控制器。编写 SPI 控制器驱动其实就是实现`struct spi_master`中的各种接口，并将该`struct spi_master`结构注册到 spi 子系统中去。



### 2. 向内核声明SPI控制器设备

​		对于ARM平台中各种控制器，主要有两种方法在内核中声明一个SPI控制器设备。

- 一种是在板级代码中向内核中注册一个`struct platform_device`对象。
- 另一种是使用`device tree`来描述 SPI 控制器的各种信息，然后由 `device tree`相关代码解析并向内核声明一个 SPI 控制器相关的设备。

#### 2.1 板级代码中向内核注册 SPI 控制器 

​		<font color=red>组织好platform_device--> 构建 struct spi_master</font>

示例代码：

```c
static struct platform_device *crag6410_devices[] __initdata = {
    ...
    &s3c64xx_device_spi0,
    ...
}

static void __init crag6410_machine_init(void)
{
    ...
    s3c64xx_spi0_set_platdata(NULL, 0, 2);
    platform_add_devices(crag6410_devices, ARRAY_SIZE(crag6410_devices))l
    ...
}

void __init s3c64xx_spi0_set_platdata(int (*cfg_gpio)(void), int src_clk_nr,
                                                int num_cs)
{
        struct s3c64xx_spi_info pd;

        /* Reject invalid configuration */
        if (!num_cs || src_clk_nr < 0) {
                pr_err("%s: Invalid SPI configuration\n", __func__);
                return;
        }

        pd.num_cs = num_cs;
        pd.src_clk_nr = src_clk_nr;
        pd.cfg_gpio = (cfg_gpio) ? cfg_gpio : s3c64xx_spi0_cfg_gpio;

        s3c_set_platdata(&pd, sizeof(pd), &s3c64xx_device_spi0);
}

// 上述代码定义了spi控制器相关的struct platform_device结构：s3c64xx_device_spi0(表示s3c64xx上的spi0)
// 注册crag6410_devices[]中的struct platform_device 对象到内核中
// s3c64xx_spi0_set_platdata:设置一些spi控制器相关信息到s3c64xx_device_spi0中去，包括片选信息，时钟等等

#ifdef CONFIG_S3C64XX_DEV_SPI0
static struct resource s3c64xx_spi0_resource[] = {
        [0] = DEFINE_RES_MEM(S3C_PA_SPI0, SZ_256), //spi寄存器地址信息
        [1] = DEFINE_RES_DMA(DMACH_SPI0_TX),
        [2] = DEFINE_RES_DMA(DMACH_SPI0_RX),
        [3] = DEFINE_RES_IRQ(IRQ_SPI0),//中斷信息
};

struct platform_device s3c64xx_device_spi0 = {
        .name           = "s3c6410-spi",
        .id             = 0,
        .num_resources  = ARRAY_SIZE(s3c64xx_spi0_resource),
        .resource       = s3c64xx_spi0_resource,
        .dev = {
                .dma_mask               = &samsung_device_dma_mask,
                .coherent_dma_mask      = DMA_BIT_MASK(32),
        },
};

//struct resource 对象描述的是与SPI控制器硬件相关信息，包括寄存器起始地址、地址大小、DMA相关信息、终端号等
//若希望一個spi控制器可以正常的工作在linux spi子系統中，構建並註冊一個描述spi控制器的struct platform_device對象是必須的


//使用struct platform_device中的信息来构建和注册struct spi_master对象到Linux SPI子系统中去，所有的spi设备驱动必须调用 struct spi_master对象中的接口来完成发送和接收数据。
```

代码中定义了一个`struct platform_device`指针数组`nexcoder_devices[]`，该数组中的`struct platform_device`指针对应的`struct platform_device`用来表示CPU上各种总线的控制器。这个数组中的各个`struct platform_device`最终都会被注册到内核中。

​		

#### 2.2 Device Tree 描述 SPI 控制器

​		<font color=red>组织好platform_device--> 构建 struct spi_master</font>

​		示例如下：

```c
spi0: spi@48030000 {
    compatible = "ti,omap4-mcspi";  //用来和驱动的的of_device_id项目匹配
    #address-cells = <1>;
    #size-cells = <0>;
    reg = <0x48030000 0x400>;    //spi控制器寄存器地址信息
    interrupt = <65>;            //spi相关中断信息
    ti,spi-num-cs = <2>;         //片选脚个数
    ti,hwmods = "spi0";
    dmas = <&edma 16
        &edma 17
        &edma 18
        &edma 19>;
    dma-names = "tx0", "rx0", "tx1", "rx1";
    status = "disabled";       //控制是否解析该节点
};

spi1: spi@481a0000 {
    compatible = "ti,omap4-mcspi";
    #address-cells = <1>;
    #size-cells = <0>;
    reg = <0x481a0000 0x400>;
    interrupt = <125>;
    ti,spi-num-cs = <2>;
    ti,hwmods = "spi1";
    dmas = <&edma 42
        &edma 43
        &edma 44
        &edma 45>;                           
    dma-names = "tx0", "rx0", "tx1", "rx1";
    status = "disabled";

};
```

​		Device Tree 中描述的SPI控制器信息在Device Tree解析时也会转换成一个`struct platform_device`对象。并注册到内核。

​		每个描述SPI控制器的Device Tree节点会被转化为一个`struct platform_device`对象。Device Tree节点中的`reg`和`interrupt`属性也会被转化为对应的描述寄存器和中断信息的`struct resource`对象，并被该节点转化的`struct platform_device`对象中的`resource`成员引用。



### 3. SPI 控制器平台设备驱动

#### struct bus_type:

```c
struct bus_type spi_bus_type = {
        .name           = "spi",
        .dev_attrs      = spi_dev_attrs,
        .match          = spi_match_device,
        .uevent         = spi_uevent,
        .pm             = &spi_pm,
};
EXPORT_SYMBOL_GPL(spi_bus_type);
```

​		逻辑上代表SPI总线。所有的`struct spi_master`对象和`struct spi_device`对象注册到SPI子系统后都会被连接到这个数据结构中，并且`struct spi_driver`注册到SPI子系统后也会连接到这个数据结构中，其中`match`回调函数实现了SPI设备和SPI设备驱动匹配的策略。



#### struct spi_master:

```c
struct spi_master {
        struct device   dev;

        struct list_head list;

        /* other than negative (== assign one dynamically), bus_num is fully
         * board-specific.  usually that simplifies to being SOC-specific.
         * example:  one SOC has three SPI controllers, numbered 0..2,
         * and one board's schematics might show it using SPI-2.  software
         * would normally use bus_num=2 for that controller.
         */
        s16                     bus_num;	// 需要在驱动中设置，代表该总线的编号，即连接在板子上的哪个SPI上

        /* chipselects will be integral to many controllers; some others
         * might use board-specific GPIOs.
         */
        u16                     num_chipselect; // 片选的个数，与SPI总线上连接的设备有关，即SPI总线支持多少个device

        /* some SPI controllers pose alignment requirements on DMAable
         * buffers; let protocol drivers know about these requirements.
         */
        u16                     dma_alignment;

        /* spi_device.mode flags understood by this controller driver */
        u16                     mode_bits;	// SPI 控制器所支持模式的标志位，包括相位、极性、片选模式等

        /* other constraints relevant to this driver */
        u16                     flags;
#define SPI_MASTER_HALF_DUPLEX  BIT(0)          /* can't do full duplex */
#define SPI_MASTER_NO_RX        BIT(1)          /* can't do buffer read */
#define SPI_MASTER_NO_TX        BIT(2)          /* can't do buffer write */

        /* lock and mutex for SPI bus locking */
        spinlock_t              bus_lock_spinlock;
        struct mutex            bus_lock_mutex;

        /* flag indicating that the SPI bus is locked for exclusive use */
        bool                    bus_lock_flag;

        /* Setup mode and clock, etc (spi driver may call many times).
         *
         * IMPORTANT:  this may be called when transfers to another
         * device are active.  DO NOT UPDATE SHARED REGISTERS in ways
         * which could break those transfers.
         */
        int                     (*setup)(struct spi_device *spi); // 此回调用作在发送数据给一个SPI设备之前，根据SPI设备中记录的SPI总线的电气信息重新配置SPI控制器，使SPI控制器的速率、工作模式等满足SPI设备的要求

        /* bidirectional bulk transfers
         *
         * + The transfer() method may not sleep; its main role is
         *   just to add the message to the queue.
         * + For now there's no remove-from-queue operation, or
         *   any other request management
         * + To a given spi_device, message queueing is pure fifo
         *
         * + The master's main job is to process its message queue,
         *   selecting a chip then transferring data
         * + If there are multiple spi_device children, the i/o queue
         *   arbitration algorithm is unspecified (round robin, fifo,
         *   priority, reservations, preemption, etc)
         *
         * + Chipselect stays active during the entire message
         *   (unless modified by spi_transfer.cs_change != 0).
         * + The message transfers use clock and SPI mode parameters
         *   previously established by setup() for this device
         */
        int                     (*transfer)(struct spi_device *spi,
                                                struct spi_message *mesg);// 在较老的内核版本中，此回调必须要实现。使用SPI总线进行分段数据传输时，需要调用该函数来传输数据。

        /* called on release() to free memory provided by spi_master */
        void                    (*cleanup)(struct spi_device *spi); // 主要用来释放一些资源

        /*
         * These hooks are for drivers that want to use the generic
         * master transfer queueing mechanism. If these are used, the
         * transfer() function above must NOT be specified by the driver.
         * Over time we expect SPI drivers to be phased over to this API.
         */
        bool                            queued;
        struct kthread_worker           kworker;
        struct task_struct              *kworker_task;
        struct kthread_work             pump_messages;
        spinlock_t                      queue_lock;
        struct list_head                queue;
        struct spi_message              *cur_msg;
        bool                            busy;
        bool                            running;
        bool                            rt;

        int (*prepare_transfer_hardware)(struct spi_master *master); // 发送数据之前使SPI控制器做好准备，可能是从低功耗模式转换到正常工作模式等
        int (*transfer_one_message)(struct spi_master *master,
                                    struct spi_message *mesg);	// 若实现 transfer 接口，还要实现工作队列来负责调度和控制SPI接口发送数据。 而 transfer_one_message 接口不需要再实现队列，SPI子系统中已经实现了一个内核线程来负责数据的调度和传输。
        int (*unprepare_transfer_hardware)(struct spi_master *master);
        /* gpio chip select */
        int                     *cs_gpios;
};
```



#### struct spi_transfer:

​		一个`struct spi_transfer`表示一次SPI数据传输

```c
struct spi_transfer {
        /* it's ok if tx_buf == rx_buf (right?)
         * for MicroWire, one buffer must be null
         * buffers must work with dma_*map_single() calls, unless
         *   spi_message.is_dma_mapped reports a pre-existing mapping
         */
        const void      *tx_buf;	// 发送数据缓冲区 
        void            *rx_buf;	// 接收数据缓冲区
        unsigned        len;		// 缓冲区长度

        dma_addr_t      tx_dma;		// 如果使用dma，发送数据缓冲区的dma地址
        dma_addr_t      rx_dma;		// 如果使用dma，接收数据缓冲区的dma地址

        unsigned        cs_change:1;	// 本次数据传输后，是否改变片选信号状态
        u8              bits_per_word;	// 字长信息，每次写入SPI控制器的数据宽度(8bit/32bit)
        u16             delay_usecs;	// 此次传输结束后，需要延时时间，才能开始下一个动作
        u32             speed_hz;		// 传输速率，代表SPI设备支持的最大速度

        struct list_head transfer_list;	// 通过这个链表头，将这个transfer连接到一个spi_message.transfers中
};
```





#### struct spi_message:

​	一个`struct spi_message`代表针对一个设备进行一次分段数据传输

```c
struct spi_message {
        struct list_head        transfers;	// 所有需要传输的struct spi_transfer对象都会连接到这个链表上

        struct spi_device       *spi;		// (结构指针)数据传输至此SPI设备

        unsigned                is_dma_mapped:1;	// 是否是dma传输

        /* REVISIT:  we might want a flag affecting the behavior of the
         * last transfer ... allowing things like "read 16 bit length L"
         * immediately followed by "read L bytes".  Basically imposing
         * a specific message scheduling algorithm.
         *
         * Some controller drivers (message-at-a-time queue processing)
         * could provide that as their default scheduling algorithm.  But
         * others (with multi-message pipelines) could need a flag to
         * tell them about such special cases.
         */

        /* completion is reported through a callback */
        void                    (*complete)(void *context);	// 异步传输时使用
        void                    *context;					// complete()回调的参数，通常为一个 struct completion 对象
        unsigned                actual_length;				// 本次message传输实际传输数据的长度
        int                     status;						// 本次传输的结果

        /* for optional use by whatever driver currently owns the
         * spi_message ...  between calls to spi_async and then later
         * complete(), that's the spi_master controller driver.
         */
        struct list_head        queue;						// 通过此链表头将该message连接到等待SPI控制器传输的message链表中
        void                    *state;						// message驱动会使用这个成员来获取一些状态信息
};
```



#### 3.1 板级代码描述SPI控制器的驱动

​		描述 s3c64xx 平台的 SPI 控制器使用的是`struct platform_device`数据结构，对应的驱动就是`struct platform_driver`数据结构。

- 构建s3c64xx_device_spi0 构建 platform_device

- 构建 s3c64xx_spi_driver  --> platform_driver

- 构建 device 和 driver 匹配成功的 s3c64xx_spi_probe

- 注册 spi_master

```c

static struct platform_driver s3c64xx_spi_driver = {
    .driver = {
        .name = "s3c64xx-spi",
        .owner = THIS_MODULE,
        .pm = &s3c64xx_spi_pm,
        .of_match_table = of_match_ptr(s3c64xx_spi_dt_match),
    }
    .remove = s3c64xx_spi_remove,
    .id_table = s3c64xx_spi_driver_ids,
};
MODULE_ALIAS("platform:s3c64xx-spi");

static int __init s3c64xx_spi_init(void)
{
    // 将 s3c64xx_spi_driver 注册到内核
    // 调用 s3c64xx_spi_probe 去探测设备，与设备进行匹配
    return platform_driver_probe(&s3c64xx_spi_driver, s3c64xx_spi_probe);
}
subsys_initcall(s3c64xx_spi_init);

static void __exit s3c64xx_spi_exit(void)
{
    platfoem_driver_unregister(&s3c64xx_spi_driver);
}

module_exit(s3c64xx_spi_exit);

MODULE_AUTHOR("Jaswinder Singh <jassi.brar@samsung.com>");
MODULE_DESCRIPTION("S3C64XX SPI Controller Driver");
MODULE_LICENSE("GPL");



// 上述代码的简洁写法

// driver --> device 匹配优先级
// struct platform_driver 对象和 struct platform_device 对象的匹配原则如下：
// 1. of_match_table
// 2. id_table
// 3. platform_driver.driver.name 和 platform_device.dev.name
// 注：其实platform_device.dev.name 是由 platform_device.name 和 platform_device.id 连接而成，所以直接使用 device.dev.name 和 driver.driver.name 匹配驱动和设备是不会成功的。
static struct platform_driver s3c64xx_spi_driver = {
    .driver = {
        .name = "s3c64xx-spi",	// 代表驱动的名字，可以在/sys/bus/platform/driver 下看到
        						// 当s3c64xx_spi_driver.driver.of_match_table 和 s3c64xx_spi_driver.id_table都未设置时，会用来和platform_device.name进行匹配

        .owner = THIS_MODULE,
        .pm = &s3c64xx_spi_pm,
        .of_match_table = of_match_ptr(s3c64xx_spi_dt_match),
    }
    .remove = s3c64xx_spi_remove,
    
    .id_table = s3c64xx_spi_driver_ids,
    }
};
module_platform_driver(&s3c64xx_spi_driver);


// platform_device 和 platform_driver 匹配代码如下：
// 即当 s3c64xx_spi_driver 和 s3c64xx_device_spi0 匹配成功后会执行 s3c64xx_spi_probe
static int platform_match(struct device *dev, struct device_driver *drv)
{
        struct platform_device *pdev = to_platform_device(dev);
        struct platform_driver *pdrv = to_platform_driver(drv);

        /* Attempt an OF style match first */
        if (of_driver_match_device(dev, drv))
                return 1;

        /* Then try ACPI style match */
        if (acpi_driver_match_device(dev, drv))
                return 1;

        /* Then try to match against the id table */
        if (pdrv->id_table)
                return platform_match_id(pdrv->id_table, pdev) != NULL;

        /* fall-back to driver name match */
        return (strcmp(pdev->name, drv->name) == 0);
}


// 分析 s3c64xx_device_spi0，根据板级代码创建device
// mem_res ： s3cxx_device_spi0.resource[0]
mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
irq = platform_get_irq(pdev, 0);
// 分配一块连续地址，大小为sizeof(struct spi_master) + siozeof(struct s3c64xx_spi_driver_data)
// 其中sizeof(struct spi_master)给struct spi_master使用
// 剩余的内存地址通过spi_master_set_devdata()调用，设置给了sou_master.dev.driver_data
master = spi_alloc_master(&pdev->dev, sizeof(struct s3c64xx_spi_driver_data));



// 以下代码是 s3c64xx_spi_probe 函数


// 此数据结构描述的是s3c64xx平台SPI控制器相关的信息，包括SPI控制器寄存器地址，SPI控制器时钟源等
// 其中还有 struct spi_master * 指向与该数据描述的SPI控制器对应的 spi_master
struct s3c64xx_spi_driver_data {
        void __iomem                    *regs;
        struct clk                      *clk;
        struct clk                      *src_clk;
        struct platform_device          *pdev;
        struct spi_master               *master;
        struct s3c64xx_spi_info  *cntrlr_info;
        struct spi_device               *tgl_spi;
        spinlock_t                      lock;
        unsigned long                   sfr_start;
        struct completion               xfer_completion;
        unsigned                        state;
        unsigned                        cur_mode, cur_bpw;
        unsigned                        cur_speed;
        struct s3c64xx_spi_dma_data     rx_dma;
        struct s3c64xx_spi_dma_data     tx_dma;
        struct s3c64xx_spi_port_config  *port_conf;
        unsigned int                    port_id;
};

// 配置一些指针的地址，以及SPI控制器相关的数据
        platform_set_drvdata(pdev, master);

        sdd = spi_master_get_devdata(master);
        sdd->port_conf = s3c64xx_spi_get_port_config(pdev);
        sdd->master = master;
        sdd->cntrlr_info = sci;
        sdd->pdev = pdev;
        sdd->sfr_start = mem_res->start;
        if (pdev->dev.of_node) {
                ret = of_alias_get_id(pdev->dev.of_node, "spi");
                if (ret < 0) {
                        dev_err(&pdev->dev, "failed to get alias id, errno %d\n",
                                ret);
                        goto err0;
                }
                sdd->port_id = ret;
        } else {
                sdd->port_id = pdev->id;
        }

        sdd->cur_bpw = 8;

// 构建 struct spi_master
// 设置struct spi_master结构体的信息，包括驱动中要实现的接口
        master->bus_num = sdd->port_id;
        master->setup = s3c64xx_spi_setup;
        master->cleanup = s3c64xx_spi_cleanup;
        master->prepare_transfer_hardware = s3c64xx_spi_prepare_transfer;
        master->transfer_one_message = s3c64xx_spi_transfer_one_message;
        master->unprepare_transfer_hardware = s3c64xx_spi_unprepare_transfer;
        master->num_chipselect = sci->num_cs;
        master->dma_alignment = 8;
        /* the spi->mode bits understood by this driver: */
        master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;


        sdd->regs = devm_request_and_ioremap(&pdev->dev, mem_res);
        if (sdd->regs == NULL) {
                dev_err(&pdev->dev, "Unable to remap IO\n");
                ret = -ENXIO;
                goto err1;
        }

        if (!sci->cfg_gpio && pdev->dev.of_node) {
                if (s3c64xx_spi_parse_dt_gpio(sdd))
                        return -EBUSY;
        } else if (sci->cfg_gpio == NULL || sci->cfg_gpio()) {
                dev_err(&pdev->dev, "Unable to config gpio\n");
                ret = -EBUSY;
                goto err2;
        }

        /* Setup clocks */
        sdd->clk = clk_get(&pdev->dev, "spi");
        if (IS_ERR(sdd->clk)) {
                dev_err(&pdev->dev, "Unable to acquire clock 'spi'\n");
                ret = PTR_ERR(sdd->clk);
                goto err3;
        }

        if (clk_prepare_enable(sdd->clk)) {
                dev_err(&pdev->dev, "Couldn't enable clock 'spi'\n");
                ret = -EBUSY;
                goto err4;
        }

        sprintf(clk_name, "spi_busclk%d", sci->src_clk_nr);
        sdd->src_clk = clk_get(&pdev->dev, clk_name);
        if (IS_ERR(sdd->src_clk)) {
                dev_err(&pdev->dev,
                        "Unable to acquire clock '%s'\n", clk_name);
                ret = PTR_ERR(sdd->src_clk);
                goto err5;
        }

        if (clk_prepare_enable(sdd->src_clk)) {
                dev_err(&pdev->dev, "Couldn't enable clock '%s'\n", clk_name);
                ret = -EBUSY;
                goto err6;
        }

        /* Setup Deufult Mode */
        s3c64xx_spi_hwinit(sdd, sdd->port_id);

        spin_lock_init(&sdd->lock);
        init_completion(&sdd->xfer_completion);
        INIT_LIST_HEAD(&sdd->queue);

        ret = request_irq(irq, s3c64xx_spi_irq, 0, "spi-s3c64xx", sdd);
        if (ret != 0) {
                dev_err(&pdev->dev, "Failed to request IRQ %d: %d\n",
                        irq, ret);
                goto err7;
        }

        writel(S3C64XX_SPI_INT_RX_OVERRUN_EN | S3C64XX_SPI_INT_RX_UNDERRUN_EN |
               S3C64XX_SPI_INT_TX_OVERRUN_EN | S3C64XX_SPI_INT_TX_UNDERRUN_EN,
               sdd->regs + S3C64XX_SPI_INT_EN);

// 注册spi_master到SPI子系统
        if (spi_register_master(master)) {
                dev_err(&pdev->dev, "cannot register SPI master\n");
                ret = -EBUSY;
                goto err8;
        }
```



#### 3.2 使用Device Tree描述SPI控制器的驱动

- 定义设备树的匹配结构
- 构建 platform_driver
- 构建 device 和 driver 匹配成功后的 probe
- probe 中构建好 spi_master
- 注册 spi_master 到 SPI 子系统



### 4. 总结

​		实现一个 SPI 控制器驱动，即 spi_master

- 向内核声明 SPI 控制器设备，Soc中通常使用 `struct platform_device` 或 `Device Tree`
- SPI 控制器相关的 `platform_device` 被注册到 `platform_bus` 上<font color=red>(spi_bus 中进行 device 和 driver 匹配)</font>，需要写一个驱动来绑定这个`platform_device` ，并在 `platform_driver` 的 probe 接口中创建并构件好 `struct spi_master`

- 实现 spi_master 对象中的接口，如 `setup()、cleanup()、transfer()`
- 具体平台相关的代码，例如配置寄存器、设置中断和DMA等
- 注册 `spi_master` 到 SPI 子系统





## 三、Linux SPI 设备驱动

### 1. 概览

​		SPI 子系统中分为 SPI 控制器驱动和 SPI 设备驱动。所谓的 SPI 设备驱动，就是挂在 SPI 总线上形形色色的芯片驱动。它可能是一个 FLash 芯片、一个声音编解码芯片、一个网卡芯片。如果芯片要正常工作，就必须为其编写对应的驱动，这个驱动就是 SPI 设备驱动。



### 2. 向内核注册 SPI 设备

​		如果希望一个 SPI 设备可以再 Linux 系统下很好的工作，除了写驱动，还要向内核声明和注册这个 SPI 设备。

​		目前常用两种方法向内核注册一个 SPI 设备：<font color=red>(都是为了建立一个 struct spi_device对象，并注册到 SPI 子系统)</font>

				1. 旧版本内核(2.6.xx)：通过向内核注册`struct spi_board_info`对象，来声明一个 SPI 设备。
				1. 新版本内核(3.xx)：使用`Device Tree`的方式向内核声明并注册一个 SPI 设备。



#### 2.1 spi_board_info 方式 

​		向系统注册一个名为 “m25p80” 的 SPI 设备。

​		构建 `spi_baord_info`：

```c
struct spi_board_info {
        /* the device name and module name are coupled, like platform_bus;
         * "modalias" is normally the driver name.
         *
         * platform_data goes to spi_device.dev.platform_data,
         * controller_data goes to spi_device.controller_data,
         * irq is copied too
         */
        char            modalias[SPI_NAME_SIZE];	// SPI 设备名称，设备驱动探测时会用到
        const void      *platform_data;
        void            *controller_data;
        int             irq;

        /* slower signaling on noisy or low voltage boards */
        u32             max_speed_hz;	// 设备支持最大速度


        /* bus_num is board specific and matches the bus_num of some
         * spi_master that will probably be registered later.
         *
         * chip_select reflects how this chip is wired to that master;
         * it's less than num_chipselect.
         */
        u16             bus_num;	// 描述该 SPI 设备连接在哪个 SPI 总线上，所在总线编号
        u16             chip_select;	// 该 SPI 设备的片选编号

        /* mode becomes spi_device.mode, and is essential for chips
         * where the default of SPI_CS_HIGH = 0 is wrong.
         */
        u8              mode;		// 该 SPI 设备支持 SPI 总线的工作模式

        /* ... may need additional spi_device chip config data here.
         * avoid stuff protocol drivers can set; but include stuff
         * needed to behave without being bound to a driver:
         *  - quirks like clock rate mattering when not selected
         */
};
```



​		构建好 `struct spi_board_info` 参数后，将其注册到内核中(SPI 子系统)：

```c
// spi_register_board_info：将 struct spi_board_info 对象转换成 struct boardinfo 对象，并将其连接到 board_list 的全局链表中

spi_register_board_info(mx51_3ds_spi_nor_device,
                     	ARRAY_SIZE(mx51_3ds_spi_nor_device));

struct boardinfo{
    struct list_head 		list;
    struct spi_board_info 	board_info;
};

// 将 struct boardinfo 对象添加到 board_list

// spi_master_list记录系统中所有的 SPI 控制器。spi_board_info.bus_num 和 spi_master.bus_num 相等，则说明该 spi_board_info 所对应的 spi 设备连接在该 spi 控制器上。 最终目的是创建并注册 struct spi_device
int spi_register_board_info(struct spi_board_info const *info, unsigned n)
{
        struct boardinfo *bi;
        int i;

        bi = kzalloc(n * sizeof(*bi), GFP_KERNEL);
        if (!bi)
                return -ENOMEM;
    
// 遍历 spi_master_list 上所有的 struct spi_master 对象，并将上面一步 spi_board_info.bus_num 和 spi_master.bus_num 做对比，如果相等则创建 struct spi_device 对象并注册
        for (i = 0; i < n; i++, bi++, info++) {
                struct spi_master *master;

                memcpy(&bi->board_info, info, sizeof(*info));
                mutex_lock(&board_lock);
                list_add_tail(&bi->list, &board_list);
                list_for_each_entry(master, &spi_master_list, list)
                        spi_match_master_to_boardinfo(master, &bi->board_info);
                mutex_unlock(&board_lock);
        }

        return 0;
}
```

​		继续看 spi_master 和 spi_board_info 如何匹配：

```c
static void spi_match_master_to_boardinfo(struct spi_master *master,
                                struct spi_board_info *bi)
{
        struct spi_device *dev;

// 比较spi_master.bus_name 和 spi_board_info.bus_name 匹配则调用 spi_new_device()
        if (master->bus_num != bi->bus_num)
                return;

        dev = spi_new_device(master, bi);
        if (!dev)
                dev_err(master->dev.parent, "can't create new device for %s\n",
                        bi->modalias);
}

// 创建 struct spi_device 对象并注册
struct spi_device *spi_new_device(struct spi_master *master,
                                  struct spi_board_info *chip)
{
        struct spi_device       *proxy;
        int                     status;

        /* NOTE:  caller did any chip->bus_num checks necessary.
         *
         * Also, unless we change the return value convention to use
         * error-or-pointer (not NULL-or-pointer), troubleshootability
         * suggests syslogged diagnostics are best here (ugh).
         */

        proxy = spi_alloc_device(master);
        if (!proxy)
                return NULL;

        WARN_ON(strlen(chip->modalias) >= sizeof(proxy->modalias));

        proxy->chip_select = chip->chip_select;
        proxy->max_speed_hz = chip->max_speed_hz;
        proxy->mode = chip->mode;
        proxy->irq = chip->irq;
        strlcpy(proxy->modalias, chip->modalias, sizeof(proxy->modalias));
        proxy->dev.platform_data = (void *) chip->platform_data;
        proxy->controller_data = chip->controller_data;
        proxy->controller_state = NULL;

        status = spi_add_device(proxy);
        if (status < 0) {
                spi_dev_put(proxy);
                return NULL;
        }

        return proxy;
}
EXPORT_SYMBOL_GPL(spi_new_device);
```



#### 2.2 Device Tree 方式

​		Device Tree 示例：

```c
&spi0 {
    status = "okay";
    spi-flash@0 {
        compatible = "spansion,s25fl064k", "m25p80";
        spi-max-frequency = <24000000>;
        reg = <0>;
    };
};     
```

​		示例 Device Tree 最外面的节点 spi0 描述的就是一个 SPI 控制器。在 SPI 控制器的每一个子节点，都会被内核解析成一个 SPI 设备，最后生成一个 `struct spi_device`，并注册到内核中。

​		当父节点对应的控制器被注册时解析上述代码，也就是说在调用 `spi_register_master()`向内核注册一个 SPI 控制器时，在这个函数中被解析。这个函数的最后调用了 `of_register_spi_devices(master)`，此函数的目的就是遍历`struct spi_master` 对象所对应的 Device Tree 节点的所有子节点，并使用子节点中的属性信息创建对应的 `struct spi_device`，然后注册到内核中。



### 3. SPI 设备驱动

​		上述两种方法注册 SPI 设备，讲的都是 m25p80 这个 SPI Flash 设备。

​		定义一个 `struct spi_driver` 对象，填写 probe、remove、id_table，并注册到`struct spi_driver`对象到内核。

```c
// 定义一个 struct spi_driver
static struct spi_driver m25p80_driver = {
    .driver = {
        .name = "m25p80",
        .owner = THIS_MODULE,
    },
    .id_table = m25p_ids,
    .probe = m25p80_probe,
    .remove = m25p80_remove,
};

// 注册 struct spi_driver 对象至内核
module_spi_driver(m25p80_driver);

// 此结构体记录的就是 SPI 设备的名字和驱动相关的数据，即列举一些驱动所支持的 SPI FLASH 设备
struct spi_device_id {
    char name[SPI_NAME_SIZE];
    kernel_ulong_t driver_data  /* Data private to the driver */
            __attribute__((aligned(sizeof(kernel_ulong_t))));
}
```

​		probe：probe接口的主要目的就是为了设置 mtd 的各个接口，并注册一个 mtd 设备到内核中。SPI 设备驱动的主要工作就是实现 probe 接口，并在该接口中实现所需的设备(字符/块/网络)，并实现该类设备的访问接口。



### 4. 总结

1. 向内核注册一个 `struct spi_device` 对象。可以通过 `spi_board_info`的方式和 `Device Tree`的方式
2. 实现一个 `struct spi_driver` 对象，并实现 probe 和remove 和 id_table 接口
3. 在 `struct spi_driver` 的 probe 接口中实现注册所需类型的设备的代码，并实现该类设备的访问接口







## 四、Linux MTD



### 1. Linux MTD 设备

![image-20221018093822985](image/image-20221018093822985.png)

​		Linux 使用 struct mtd_info 来描述 MTD 原始设备，其中定义了大量关于 MTD 的数据和操作函数：(每个分区也被认为是一个 mtd_info，例如：如果有两个 MTD 原始设备，而每个上有 3 个分区，在系统中就将共有 6 个 mtd_info 结构体，这些  mtd_info 的指针被存放在名为 mtd_table 的数组中)

```c
struct mtd_info {
        u_char type;		// 内存技术的类型
        uint32_t flags;		// 标志位
        uint64_t size;   // MTD 设备的大小

        /* "Major" erase size for the device. Naïve users may take this
        ┆* to be the only erase size available, or may use the more detailed
        ┆* information below if they desire
        ┆*/
        uint32_t erasesize;	// 主要的擦除块大小(同一个 mtd 设备可能有多种不同的 erasesize )
        /* Minimal writable flash unit size. In case of NOR flash it is 1 (even
        ┆* though individual bits can be cleared), in case of NAND flash it is
        ┆* one NAND page (or half, or one-fourths of it), in case of ECC-ed NOR
        ┆* it is of ECC block size, etc. It is illegal to have writesize = 0.
        ┆* Any driver registering a struct mtd_info must ensure a writesize of
        ┆* 1 or larger.
        ┆*/
        uint32_t writesize;	// 写入的大小

        /*
        ┆* Size of the write buffer used by the MTD. MTD devices having a write
        ┆* buffer can write multiple writesize chunks at a time. E.g. while
        ┆* writing 4 * writesize bytes to a device with 2 * writesize bytes
        ┆* buffer the MTD driver can (but doesn't have to) do 2 writesize
        ┆* operations, but not 4. Currently, all NANDs have writebufsize
        ┆* equivalent to writesize (NAND page size). Some NOR flashes do have
        ┆* writebufsize greater than writesize.
        ┆*/
        uint32_t writebufsize;

        uint32_t oobsize;   // Amount of OOB data per block (e.g. 16)
        uint32_t oobavail;  // Available OOB bytes per block

        /*
        ┆* If erasesize is a power of 2 then the shift is stored in
        ┆* erasesize_shift otherwise erasesize_shift is zero. Ditto writesize.
        ┆*/
        uint32_t writebufsize;

        uint32_t oobsize;   // Amount of OOB data per block (e.g. 16)
        uint32_t oobavail;  // Available OOB bytes per block

        /*
        ┆* If erasesize is a power of 2 then the shift is stored in
        ┆* erasesize_shift otherwise erasesize_shift is zero. Ditto writesize.
        ┆*/
        unsigned int erasesize_shift;
        unsigned int writesize_shift;
        /* Masks based on erasesize_shift and writesize_shift */
        unsigned int erasesize_mask;
        unsigned int writesize_mask;

        /*
        ┆* read ops return -EUCLEAN if max number of bitflips corrected on any
        ┆* one region comprising an ecc step equals or exceeds this value.
        ┆* Settable by driver, else defaults to ecc_strength.  User can override
        ┆* in sysfs.  N.B. The meaning of the -EUCLEAN return code has changed;
        ┆* see Documentation/ABI/testing/sysfs-class-mtd for more detail.
        ┆*/
        unsigned int bitflip_threshold;

        // Kernel-only stuff starts here.
        const char *name;
        int index;		// 索引

        /* OOB layout description */
        const struct mtd_ooblayout_ops *ooblayout;

        /* NAND pairing scheme, only provided for MLC/TLC NANDs */
        const struct mtd_pairing_scheme *pairing;

        /* the ecc step size. */
        unsigned int ecc_step_size;

        /* max number of correctible bit errors per ecc step */
        unsigned int ecc_strength;

        /* Data for variable erase regions. If numeraseregions is zero,
        ┆* it means that the whole device has erasesize as given above.
        ┆*/
        int numeraseregions;
        struct mtd_erase_region_info *eraseregions;
        /*
        ┆* Do not call via these pointers, use corresponding mtd_*()
        ┆* wrappers instead.
        ┆*/
        int (*_erase) (struct mtd_info *mtd, struct erase_info *instr);
        int (*_point) (struct mtd_info *mtd, loff_t from, size_t len,
                ┆      size_t *retlen, void **virt, resource_size_t *phys);	// 针对 XIP
        int (*_unpoint) (struct mtd_info *mtd, loff_t from, size_t len);
        unsigned long (*_get_unmapped_area) (struct mtd_info *mtd,
                                        ┆    unsigned long len,
                                        ┆    unsigned long offset,
                                        ┆    unsigned long flags);
        int (*_read) (struct mtd_info *mtd, loff_t from, size_t len,
                ┆     size_t *retlen, u_char *buf);		// 读 flash
        int (*_write) (struct mtd_info *mtd, loff_t to, size_t len,
                ┆      size_t *retlen, const u_char *buf);	// 写 flash
        int (*_panic_write) (struct mtd_info *mtd, loff_t to, size_t len,
                        ┆    size_t *retlen, const u_char *buf);
        int (*_read_oob) (struct mtd_info *mtd, loff_t from,
                        ┆ struct mtd_oob_ops *ops);	// 读 oob
        int (*_write_oob) (struct mtd_info *mtd, loff_t to,
                        ┆  struct mtd_oob_ops *ops);	// 写 oob
        int (*_get_fact_prot_info) (struct mtd_info *mtd, size_t len,
                                ┆   size_t *retlen, struct otp_info *buf);
        int (*_read_fact_prot_reg) (struct mtd_info *mtd, loff_t from,
                                ┆   size_t len, size_t *retlen, u_char *buf);
        int (*_get_user_prot_info) (struct mtd_info *mtd, size_t len,
                                ┆   size_t *retlen, struct otp_info *buf);
        int (*_read_user_prot_reg) (struct mtd_info *mtd, loff_t from,
                                ┆   size_t len, size_t *retlen, u_char *buf);
        int (*_write_user_prot_reg) (struct mtd_info *mtd, loff_t to,
                                ┆    size_t len, size_t *retlen, u_char *buf);
        int (*_lock_user_prot_reg) (struct mtd_info *mtd, loff_t from,
                                ┆   size_t len);
        int (*_writev) (struct mtd_info *mtd, const struct kvec *vecs,
                        unsigned long count, loff_t to, size_t *retlen);
        void (*_sync) (struct mtd_info *mtd);
        int (*_lock) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
        int (*_unlock) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
        int (*_is_locked) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
        int (*_block_isreserved) (struct mtd_info *mtd, loff_t ofs);
        int (*_block_isbad) (struct mtd_info *mtd, loff_t ofs);
        int (*_block_markbad) (struct mtd_info *mtd, loff_t ofs);
        int (*_suspend) (struct mtd_info *mtd);	// 能量管理函数
        void (*_resume) (struct mtd_info *mtd);	// 能量管理函数
        void (*_reboot) (struct mtd_info *mtd);          
            /*
        ┆* If the driver is something smart, like UBI, it may need to maintain
        ┆* its own reference counting. The below functions are only for driver.
        ┆*/
        int (*_get_device) (struct mtd_info *mtd);
        void (*_put_device) (struct mtd_info *mtd);

        /* Backing device capabilities for this device
        ┆* - provides mmap capabilities
        ┆*/
        struct backing_dev_info *backing_dev_info;

        struct notifier_block reboot_notifier;  /* default mode before reboot */

        /* ECC status information */
        struct mtd_ecc_stats ecc_stats;
        /* Subpage shift (NAND) */
        int subpage_sft;

        void *priv;

        struct module *owner;
        struct device dev;
        int usecount;
};
```

​		用户访问 MTD 设备时，不应访问设备驱动，而应通过 `struct mtd_info` 中的 回调函数进行访问。

​		在 NOR 和 NAND 的驱动代码中几乎看不到 mtd_info 的成员函数(即这些成员函数对于 Flash 芯片驱动是透明的)，这是因为 Linux 在 MTD 的下层实现了针对 NOR Hash 和 NAND Hash 的通用的 mtd_info 成员函数。

​		Flash 驱动中使用如下两个函数注册和注销 MTD 设备：

```c
int add_mtd_device(struct mtd_info *mtd);
int del_mtd_device(struct mtd_info *mtd);
```

​		mtd__part 结构体用于描述分区，其 mtd_info 结构体成员用于描述本分区：

```c
struct mtd_part {                                                             
        struct mtd_info mtd;	// 分区的信息
        struct mtd_info *master;	// 该分区的主分区
        uint64_t offset;		// 该分区的偏移地址
        struct list_head list;
};
```

​		mtd_partition 会在 MTD 原始设备层调用 add_mtd_partitions() 时传递分区信息用：

```c
struct mtd_partition {
        const char *name;               /* 标识字符串 */
        uint64_t size;                  /* 分区大小 */
        uint64_t offset;                /* 主 MTD 空间内的偏移 */
        uint32_t mask_flags;            /* 掩码标志 */
};
```

​		Flash 驱动中使用如下两个函数注册和注销分区：

```c
int add_mtd_partitions(struct mtd_info *master, const struct mtd_partition *parts, int nbparts);	// 对每一个新建分区建立一个新的 mtd_part 结构体，将其加入 mtd_partitions，并调用 add_mtd_device() 将此分区作为 MTD 设备加入 mtd_table 
int del_mtd_partitions(struct mtd_info *master);	// 对于 mtd_partitions 上的每一个分区，如果它的分区是master，则将它从 mtd_partitions 和 mtd_table 中删除并释放掉，此函数会调用 del_mtd_device()
```







### 2. Linux MTD 分区

​		结构描述如下：

```c
struct mtd_part_parser {
        struct list_head list;
        struct module *owner;
        const char *name;
        int (*parse_fn)(struct mtd_info *, const struct mtd_partition **, 
                        struct mtd_part_parser_data *); 
        void (*cleanup)(const struct mtd_partition *pparts, int nr_parts);
};


int parse_mtd_partitions(struct mtd_info *master, const char *const *types,
                        ┆struct mtd_partitions *pparts,
                        ┆struct mtd_part_parser_data *data)
{
        struct mtd_part_parser *parser;
        int ret, err = 0;

        if (!types)
                types = default_mtd_part_types;

        for ( ; *types; types++) {
                pr_debug("%s: parsing partitions %s\n", master->name, *types);
            	// 解析 part_parsers 链表中的数据
                parser = mtd_part_parser_get(*types);
                if (!parser && !request_module("%s", *types))
                        parser = mtd_part_parser_get(*types);
                pr_debug("%s: got parser %s\n", master->name,
                        ┆parser ? parser->name : NULL);
                if (!parser)
                        continue;
            	// 调用这个函数 --> parse_cmdline_partitions 根据 cmd_line 信息获取分区信息
                ret = (*parser->parse_fn)(master, &pparts->parts, data);
                pr_debug("%s: parser %s: %i\n",
                        ┆master->name, parser->name, ret);
                if (ret > 0) {
                        printk(KERN_NOTICE "%d %s partitions found on MTD device %s\n",
                        ┆      ret, parser->name, master->name);
                        pparts->nr_parts = ret;
                        pparts->parser = parser;
                        return 0;
                }
                mtd_part_parser_put(parser);
                /*
                ┆* Stash the first error we see; only report it if no parser
                ┆* succeeds
                ┆*/
                if (ret < 0 && !err)
                        err = ret;
        }
        return err;
}

// spi nor 驱动中传递过来的 types 为 cmdlinepart 即通过 uboot 传参，若为 ofpart 则为设备树传参

```

​		调用函数 `parse_mtd_partitions()` 查看 Flash 上是否已有分区信息，并将查看出的分区信息通过 `add_mtd_partitions()`注册。

​		`parse_mtd_partitions()`查看的分区信息由 loader 通过 CMD_LINE 传递过来的，下面调用了函数`parse_cmdline_partitions`，根据 CMD_LINE  传递的分区信息通过pparts返回

```c
// drivers/mtd/cmdlinepart.c
static struct mtd_part_parser cmdline_parser = {
        .parse_fn = parse_cmdline_partitions,
        .name = "cmdlinepart",
};
```





















## 五、Linux Nor Flash 驱动分析



### mudule_init(m25p80_init)

```c
static int m2580_init(void)
{
    
    spi_register_board_info(spi_flash_board_info, ARR_SIZE(spi_flash_board_info));
    spi_register_driver(&m25p80_driver);
    if (extend_m25p80_param) {
        spi_register_board_info(extend_spi_flash_board_info, 
                                ARRAY_SIZE(extend_spi_flash_board_info));
        return spi_register_driver(&extend_m25p80_driver);
    }
}
```

​		设备需要 device 和 driver，设备由 `spi_register_board_info` 创建，驱动由 `spi_register_driver` 注册。先查看设备创建过程：

#### spi_register_board_info()：

```c
struct spi_board_info {
        /* the device name and module name are coupled, like platform_bus;
        ┆* "modalias" is normally the driver name.
        ┆*
        ┆* platform_data goes to spi_device.dev.platform_data,
        ┆* controller_data goes to spi_device.controller_data,
        ┆* irq is copied too
        ┆*/
        char            modalias[SPI_NAME_SIZE];	// 设备名称
        const void      *platform_data;				// 存储的特定数据(特定于驱动程序)。
        void            *controller_data;			// 一些控制器需要有关硬件设置的提示，例如 DMA。
        int             irq;						// 中断号

        /* slower signaling on noisy or low voltage boards */
        u32             max_speed_hz;				// 最大工作频率


        /* bus_num is board specific and matches the bus_num of some
        ┆* spi_master that will probably be registered later.
        ┆*
        ┆* chip_select reflects how this chip is wired to that master;
        ┆* it's less than num_chipselect.
        ┆*/
        u16             bus_num;					// 设备连接的总线号，用来与 spi_mater 结构中的 bus_num 进行匹配
        u16             chip_select;				// 该 SPI 设备的片选编号

        /* mode becomes spi_device.mode, and is essential for chips
         * where the default of SPI_CS_HIGH = 0 is wrong.
         */
        u8              mode;						// 该 SPI 设备支持 SPI 总线的工作模式

        /* ... may need additional spi_device chip config data here.
        ┆* avoid stuff protocol drivers can set; but include stuff
        ┆* needed to behave without being bound to a driver:
        ┆*  - quirks like clock rate mattering when not selected
        ┆*/
};


struct flash_platform_data {
        char            *name;
        struct mtd_partition *parts;
        unsigned int    nr_parts;

        char            *type;

        /* we'll likely add more ... use JEDEC IDs, etc */
};


static struct flash_platform_data platform_data = {                                   
        .name = "m25p80",                                                             
        .parts = NULL,                                                                
        .nr_parts = 0,                                                                
        .type = NULL,                                                                  
}; 

static struct spi_board_info spi_flash_board_info[] = {                                
        {                                     
                .modalias = "m25p80",    
                .platform_data = &platform_data,
            	.mode = SPI_CS_HIGH | SPI_MODE_0,
                .max_speed_hz = 30000000,     
                .bus_num = 0,              
                .chip_select = 0, 
        }                                    
};



int spi_register_board_info(struct spi_board_info const *info, unsigned n)
{
        struct boardinfo *bi;
        int i;

        if (!n)
                return -EINVAL;

        bi = kzalloc(n * sizeof(*bi), GFP_KERNEL);
        if (!bi)
                return -ENOMEM;

        for (i = 0; i < n; i++, bi++, info++) {
                struct spi_master *master;

                memcpy(&bi->board_info, info, sizeof(*info));
                mutex_lock(&board_lock);
                list_add_tail(&bi->list, &board_list);
                list_for_each_entry(master, &spi_master_list, list)
                    
                    // 遍历 spi_master_list 上所有的 struct spi_master 对象，并将上面一步 spi_board_info.bus_num 和 spi_master.bus_num 做对比，如果相等则说明该 spi_board_info 所对应的 spi 设备连接在该 spi 控制器上，创建 struct spi_device 对象并注册
                        spi_match_master_to_boardinfo(master, &bi->board_info);
                mutex_unlock(&board_lock);
        }

        return 0;
}


static void spi_match_master_to_boardinfo(struct spi_master *master,
                                struct spi_board_info *bi)                                                                    
{
        struct spi_device *dev;

        if (master->bus_num != bi->bus_num)
                return;
		
    	// 当 spi_board_info 对应的 spi 设备的 bus_num 等于控制器的 bus_num,则说明这个 spi 设备连接在对应的 控制器上
    	// 在 master 上根据 spi_board_info 创建 device
        dev = spi_new_device(master, bi);
        if (!dev)
                dev_err(master->dev.parent, "can't create new device for %s\n",
                        bi->modalias);
}



struct spi_device *spi_new_device(struct spi_master *master,
                                ┆ struct spi_board_info *chip)
{                                                                                                                                                                                                                               
        struct spi_device       *proxy;
        int                     status;

        /* NOTE:  caller did any chip->bus_num checks necessary.
        ┆*
        ┆* Also, unless we change the return value convention to use
        ┆* error-or-pointer (not NULL-or-pointer), troubleshootability
        ┆* suggests syslogged diagnostics are best here (ugh).
        ┆*/

    	// 分配 spi_device，并初始化 spi->dev 的一些字段
        proxy = spi_alloc_device(master);
        if (!proxy)
                return NULL;

        WARN_ON(strlen(chip->modalias) >= sizeof(proxy->modalias));

    	// spi_board_info 结构中的初始化项
        proxy->chip_select = chip->chip_select;
        proxy->max_speed_hz = chip->max_speed_hz;
        proxy->mode = chip->mode;
        proxy->irq = chip->irq;
        strlcpy(proxy->modalias, chip->modalias, sizeof(proxy->modalias));
        proxy->dev.platform_data = (void *) chip->platform_data;
        proxy->controller_data = chip->controller_data;
        proxy->controller_state = NULL;

    	// 将 spi_device 添加到内核 
        status = spi_add_device(proxy);
        if (status < 0) {
                spi_dev_put(proxy);
                return NULL;
        }

        return proxy;
}


struct spi_device *spi_alloc_device(struct spi_master *master)
{
        struct spi_device       *spi;

        if (!spi_master_get(master))
                return NULL;

        spi = kzalloc(sizeof(*spi), GFP_KERNEL);
        if (!spi) {
                spi_master_put(master);
                return NULL;
        }

        spi->master = master;
        spi->dev.parent = &master->dev;
    	// 设置总线类型是 spi_bus_type，根据spi_bus_type 来匹配 spi_device 和 spi_driver
        spi->dev.bus = &spi_bus_type;
        spi->dev.release = spidev_release;
        spi->cs_gpio = -ENOENT;

        spin_lock_init(&spi->statistics.lock);

        device_initialize(&spi->dev);
        return spi;
}




int spi_add_device(struct spi_device *spi)
{
        static DEFINE_MUTEX(spi_add_lock);
        struct spi_master *master = spi->master;
        struct device *dev = master->dev.parent;
        int status;

        /* Chipselects are numbered 0..max; validate. */
        if (spi->chip_select >= master->num_chipselect) {
                dev_err(dev, "cs%d >= max %d\n",
                        spi->chip_select,
                        master->num_chipselect);
                return -EINVAL;
        }

        /* Set the bus ID string */
    	// 设置 spi_device 在设备驱动模型中的 name，也就是spi0.0,在/dev/下设备节点的名字是proxy->modalias中的名字
        spi_dev_set_name(spi);

        /* We need to make sure there's no other device with this
        ┆* chipselect **BEFORE** we call setup(), else we'll trash
        ┆* its configuration.  Lock against concurrent add() calls.
        ┆*/
        mutex_lock(&spi_add_lock);

        status = bus_for_each_dev(&spi_bus_type, NULL, spi, spi_dev_check);
        if (status) {
                dev_err(dev, "chipselect %d already in use\n",
                                spi->chip_select);
                goto done;
        }

        if (master->cs_gpios)
                spi->cs_gpio = master->cs_gpios[spi->chip_select];

        /* Drivers may modify this initial i/o setup, but will
        ┆* normally rely on the device being setup.  Devices
        ┆* using SPI_CS_HIGH can't coexist well otherwise...
        ┆*/
    	// spi->master->setup(spi); 运行 IP 控制器的 setup 函数
        status = spi_setup(spi);
        if (status < 0) {
                dev_err(dev, "can't setup %s, status %d\n",
                                dev_name(&spi->dev), status);
                goto done;
        }

        /* Device may be bound to an active driver when this returns */
        status = device_add(&spi->dev); 
        if (status < 0)
                dev_err(dev, "can't add %s, status %d\n",
                                dev_name(&spi->dev), status);
        else
                dev_dbg(dev, "registered child %s\n", dev_name(&spi->dev));

done:
        mutex_unlock(&spi_add_lock);
        return status;
}


int device_add(struct device *dev)
{
        struct device *parent = NULL;
        struct kobject *kobj;
        struct class_interface *class_intf;
        int error = -EINVAL;
        struct kobject *glue_dir = NULL;

        dev = get_device(dev);
        if (!dev)
                goto done;

        if (!dev->p) {
                error = device_private_init(dev);
                if (error)
                        goto done;
        }

        /*
        ┆* for statically allocated devices, which should all be converted
        ┆* some day, we need to initialize the name. We prevent reading back
        ┆* the name, and force the use of dev_name()
        ┆*/
        if (dev->init_name) {
                dev_set_name(dev, "%s", dev->init_name);
                dev->init_name = NULL;
        }

        /* subsystems can specify simple device enumeration */
        if (!dev_name(dev) && dev->bus && dev->bus->dev_name)
                dev_set_name(dev, "%s%u", dev->bus->dev_name, dev->id);

        if (!dev_name(dev)) {
                error = -EINVAL;
                goto name_error;
        }
        pr_debug("device: '%s': %s\n", dev_name(dev), __func__);

        parent = get_device(dev->parent);
        kobj = get_device_parent(dev, parent);
        if (kobj)
                dev->kobj.parent = kobj;

        /* use parent numa_node */
        if (parent && (dev_to_node(dev) == NUMA_NO_NODE))
                set_dev_node(dev, dev_to_node(parent));

        /* first, register with generic layer. */
        /* we require the name to be set before, and pass NULL */
        error = kobject_add(&dev->kobj, dev->kobj.parent, NULL);
        if (error) {
                glue_dir = get_glue_dir(dev);
                goto Error;
        }

        /* notify platform of device entry */
        if (platform_notify)
                platform_notify(dev);

        error = device_create_file(dev, &dev_attr_uevent);
        if (error)
                goto attrError;

        error = device_add_class_symlinks(dev);
        if (error)
                goto SymlinkError;
        error = device_add_attrs(dev);
        if (error)
                goto AttrsError;
        error = bus_add_device(dev);
        if (error)     
                            goto DPMError;
        device_pm_add(dev);

        if (MAJOR(dev->devt)) {
                error = device_create_file(dev, &dev_attr_dev);
                if (error)
                        goto DevAttrError;

                error = device_create_sys_dev_entry(dev);
                if (error)
                        goto SysEntryError;

                devtmpfs_create_node(dev);
        }

        /* Notify clients of device addition.  This call must come
        ┆* after dpm_sysfs_add() and before kobject_uevent().
        ┆*/
        if (dev->bus)
                blocking_notifier_call_chain(&dev->bus->p->bus_notifier,
                                        ┆    BUS_NOTIFY_ADD_DEVICE, dev);

        kobject_uevent(&dev->kobj, KOBJ_ADD);
        bus_probe_device(dev);
        if (parent)
                klist_add_tail(&dev->p->knode_parent,
                        ┆      &parent->p->klist_children);

        if (dev->class) {
                mutex_lock(&dev->class->p->mutex);
                /* tie the class to the device */
                klist_add_tail(&dev->knode_class,
                        ┆      &dev->class->p->klist_devices);

                /* notify any interfaces that the device is here */
                list_for_each_entry(class_intf,
                                ┆   &dev->class->p->interfaces, node)
                        if (class_intf->add_dev)
                                class_intf->add_dev(dev, class_intf);
                mutex_unlock(&dev->class->p->mutex);
        }
done:
        put_device(dev);
        return error;
 SysEntryError:
        if (MAJOR(dev->devt))
                device_remove_file(dev, &dev_attr_dev);
 DevAttrError:
        device_pm_remove(dev);
        dpm_sysfs_remove(dev);
 DPMError:
        bus_remove_device(dev);
 BusError:
        device_remove_attrs(dev);
 AttrsError:
        device_remove_class_symlinks(dev);
 SymlinkError:
        device_remove_file(dev, &dev_attr_uevent);
 attrError:
        kobject_uevent(&dev->kobj, KOBJ_REMOVE);
        glue_dir = get_glue_dir(dev);
        kobject_del(&dev->kobj);
 Error:
        cleanup_glue_dir(dev, glue_dir);
        put_device(parent);
name_error:
        kfree(dev->p);
        dev->p = NULL;
        goto done;
}

```



#### spi_register_driver()：

​		`spi_register_driver()`函数用于创建与 spi_flash 设备对应的驱动程序。

```c
static int m25p80_init(void)
{
        /* it must be separated with spi_nand flash driver, or will confict. */
        spi_register_board_info(spi_flash_board_info, ARRAY_SIZE(spi_flash_board_info));                       
        spi_register_driver(&m25p80_driver);
        if (extend_m25p80_param) {
                spi_register_board_info(extend_spi_flash_board_info, ARRAY_SIZE(extend_spi_flash_board_info));
                return spi_register_driver(&extend_m25p80_driver);
        }
        return 0;
}
```

​		先查看 m25p80_driver 结构体的定义，当 driver 与 device 匹配成功之后将会执行 probe 函数：

```c
static struct spi_driver m25p80_driver = {                                                                     
        .driver = {
                .name   = "m25p80",
                .bus    = &spi_bus_type,
                .owner  = THIS_MODULE,
        },
        .probe  = m25p_probe,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
        .remove = m25p_remove,
#else
        .remove = __devexit_p(m25p_remove),
#endif
        .shutdown =m25p_shutdown,
        /* REVISIT: many of these chips have deep power-down modes, which
        ┆* should clearly be entered on suspend() to minimize power use.
        ┆* And also when they're otherwise idle...
        ┆*/
};
```

​		设备和驱动匹配成功后会执行 probe 函数，probe接口的主要目的就是为了设置 mtd 的各个接口，并注册一个 mtd 设备到内核中

```c
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
static int m25p_probe(struct spi_device *spi)
#else
static int __devinit m25p_probe(struct spi_device *spi)
#endif
{
        struct flash_platform_data *data;
        struct m25p                *m25p;
        struct spi_nor_info        *info;
        struct spi_nor_flash       flash;
        unsigned int               i;

#ifdef CONFIG_GX_MTD_SPI_NOR_WRITE_PROTECT
        gx_spi_nor_protect_init();
#endif
        data = spi->dev.platform_data;
        flash.spi = spi;
        if (data && data->type) {
                int ret = spi_nor_detect(&flash);
                if (ret == 0 && strcmp(data->type, flash.info->name) == 0)
                        info = flash.info;
                else {
                        info = NULL;
                }
        }
        else {
                int ret = spi_nor_detect(&flash);
                if (ret == 0)
                        info = flash.info;
                else
                        info = NULL;
        }                                                                                                      

        if (!info)
                return -ENODEV;

        m25p = kzalloc(sizeof *m25p, GFP_KERNEL);
        if (!m25p)
                return -ENOMEM;

        m25p->flash.addr_width = flash.addr_width;
        m25p->flash.protect_mode = flash.protect_mode;
        m25p->flash.match_mode = flash.match_mode;
        m25p->flash.size = flash.size;
        m25p->flash.info = flash.info;
        m25p->flash.spi = flash.spi;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
        sema_init(&m25p->lock, 1);
#else
        init_MUTEX(&m25p->lock);
#endif
        dev_set_drvdata(&spi->dev, m25p);

        if (data->name)
                m25p->mtd.name = data->name;
        else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
                m25p->mtd.name = info->name;
#else
                m25p->mtd.name = spi->dev.bus_id;
#endif

        m25p->mtd.type = MTD_NORFLASH;
        m25p->mtd.writesize = 1;
        m25p->mtd.flags = MTD_CAP_NORFLASH;
        m25p->mtd.size = flash.size;
        m25p->mtd.erasesize = SPI_FLASH_ERASE_SIZE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
        m25p->mtd._erase = m25p80_erase;
        m25p->mtd._read = m25p80_read;
        m25p->mtd._write = m25p80_write;
#ifdef CONFIG_GX_MTD_SPI_NOR_WRITE_PROTECT
        m25p->mtd._lock = m25p80_write_protect_lock;
        m25p->mtd._unlock = m25p80_write_protect_unlock;
#endif
#else
        m25p->mtd.erase = m25p80_erase;
        m25p->mtd.read = m25p80_read;
        m25p->mtd.write = m25p80_write;
#ifdef CONFIG_GX_MTD_SPI_NOR_WRITE_PROTECT
        m25p->mtd.lock = m25p80_write_protect_lock;
        m25p->mtd.unlock = m25p80_write_protect_unlock;
#endif
#endif
        m25p->flash.size = m25p->mtd.size;

        if (!strcmp(data->name,"m25p80")) {
                flash_otp_probe(m25p);
                flash_nor_probe(m25p);
        }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
        dev_info(&spi->dev, "%s (%lld Kbytes)\n", info->name,
                        m25p->mtd.size / 1024);

        DEBUG(MTD_DEBUG_LEVEL2,
                        "mtd .name = %s, .size = 0x%.8llx (%lluM) "
                        ".erasesize = 0x%.8x (%uK) .numeraseregions = %d\n",
                        m25p->mtd.name,
                        m25p->mtd.size, m25p->mtd.size / (1024*1024),
                        m25p->mtd.erasesize, m25p->mtd.erasesize / 1024,
                        m25p->mtd.numeraseregions);

        if (m25p->mtd.numeraseregions)
                for (i = 0; i < m25p->mtd.numeraseregions; i++)
                        DEBUG(MTD_DEBUG_LEVEL2,
                                        "mtd.eraseregions[%d] = { .offset = 0x%.8llx, "
                                        ".erasesize = 0x%.8x (%uK), "
                                        ".numblocks = %d }\n",
                                        i, m25p->mtd.eraseregions[i].offset,
                                        m25p->mtd.eraseregions[i].erasesize,
                                        m25p->mtd.eraseregions[i].erasesize / 1024,
                                        m25p->mtd.eraseregions[i].numblocks);
#else
        dev_info(&spi->dev, "%s (%d Kbytes)\n", info->name,
                        m25p->mtd.size / 1024);

        DEBUG(MTD_DEBUG_LEVEL2,
                        "mtd .name = %s, .size = 0x%.8x (%uM) "
                        ".erasesize = 0x%.8x (%uK) .numeraseregions = %d\n",
                        m25p->mtd.name,
                        m25p->mtd.size, m25p->mtd.size / (1024*1024),
                        m25p->mtd.erasesize, m25p->mtd.erasesize / 1024,
                        m25p->mtd.numeraseregions);

        if (m25p->mtd.numeraseregions)
                for (i = 0; i < m25p->mtd.numeraseregions; i++)
                        DEBUG(MTD_DEBUG_LEVEL2,
                                        "mtd.eraseregions[%d] = { .offset = 0x%.8x, "
                                        ".erasesize = 0x%.8x (%uK), "
                                        ".numblocks = %d }\n",
                                        i, m25p->mtd.eraseregions[i].offset,
                                        m25p->mtd.eraseregions[i].erasesize,
                                        m25p->mtd.eraseregions[i].erasesize / 1024,
                                        m25p->mtd.eraseregions[i].numblocks);
#endif

        /* partitions should match sector boundaries; and it may be good to
        ┆* use readonly partitions for writeprotected sectors (BP2..BP0).
        ┆*/
        if (mtd_has_partitions()) {                                                                            
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
                struct mtd_partitions parsed;
                int    nr_parts = 0;

#ifdef CONFIG_MTD_CMDLINE_PARTS
                static const char *part_probes[] = { "cmdlinepart", NULL, };
                memset(&parsed, 0, sizeof(parsed));

                nr_parts = parse_mtd_partitions(&m25p->mtd, part_probes, &parsed, NULL);
#endif

                if (parsed.nr_parts <= 0 && data && data->parts) {
                        parsed = (struct mtd_partitions){
                                .parts    = data->parts,
                                .nr_parts = data->nr_parts,
                        };
                }

                if (parsed.nr_parts > 0) {
                        for (i = 0; i < parsed.nr_parts; i++) {
                                DEBUG(MTD_DEBUG_LEVEL2, "partitions[%d] = "
                                                "{.name = %s, .offset = 0x%.8llx, "
                                                ".size = 0x%.8llx (%lluK) }\n",
                                                i, parsed.parts[i].name,
                                                parsed.parts[i].offset,
                                                parsed.parts[i].size,
                                                parsed.parts[i].size / 1024);
                                gxlog_i("partitions[%d] = "
                                                "{.name = %s, .offset = 0x%.8llx, "
                                                ".size = 0x%.8llx (%lluK) }\n",
                                                i, parsed.parts[i].name,
                                                parsed.parts[i].offset,
                                                parsed.parts[i].size,
                                                parsed.parts[i].size / 1024);
                        }
                        m25p->partitioned = 1;
                        return add_mtd_partitions(&m25p->mtd, parsed.parts, parsed.nr_parts);
                }
#else
                struct mtd_partition *parts = NULL;
                int    nr_parts = 0;

#ifdef CONFIG_MTD_CMDLINE_PARTS
                static const char *part_probes[] = { "cmdlinepart", NULL, };

                nr_parts = parse_mtd_partitions(&m25p->mtd, part_probes, &parts, 0);
#endif
                if (nr_parts <= 0 && data && data->parts) {
                        parts = data->parts;
                        nr_parts = data->nr_parts;
                }

                if (nr_parts > 0) {
                        for (i = 0; i < data->nr_parts; i++) {
                                DEBUG(MTD_DEBUG_LEVEL2, "partitions[%d] = "
                                                "{.name = %s, .offset = 0x%.8x, "
                                                ".size = 0x%.8x (%uK) }\n",
                                                i, data->parts[i].name,
                                                (uint32_t)data->parts[i].offset,
                                                (uint32_t)data->parts[i].size,
                                                (uint32_t)data->parts[i].size / 1024);
                        }
                        m25p->partitioned = 1;
                        return add_mtd_partitions(&m25p->mtd, parts, nr_parts);
                }
#endif
        } else if (data->nr_parts)
                dev_warn(&spi->dev, "ignoring %d default partitions on %s\n",
                                data->nr_parts, data->name);

        return 0;
}

```

​		上述代码会执行 probe 函数、创建 mtd 设备，使用 `parse_mtd_partitions` 获取来自 loader 的CMD_LINE 信息，使用` ret = (*parser->parse_fn)(master, &pparts->parts, data);` 函数将 CMD_LINE 解析成 partitions 信息，将获取到的 partitions 信息添加到 mtd 设备`(add_mtd_partitions(&m25p->mtd, parsed.parts, parsed.nr_parts))`。

​		Linux 就是根据 mtd 设备的分区表信息来启动 kernel 并挂载根文件系统的，如果要继续挂载其它的文件系统，则可以继续创建 mtd 分区信息，并挂载到根文件系统之上。

​		执行 probe 函数：flash_otp_probe(m25p)、flash_nor_probe(m25p)

```c
struct file_operations flash_otp_fops =                                                                        
{
        .owner   = THIS_MODULE,
        .open    = flash_otp_do_open,
        .llseek  = flash_otp_do_llseek,
        .read    = flash_otp_do_read,
        .write   = flash_otp_do_write,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
        .unlocked_ioctl = flash_otp_do_ioctl,
#else
        .ioctl   = flash_otp_do_ioctl,
#endif
        .release = flash_otp_do_release,
};

static struct miscdevice flash_otp_miscdev =
{
        .minor = MISC_OTP_MINOR,
        .name  = "flash_otp",
        .fops  = &flash_otp_fops,
};

static int flash_otp_probe(struct m25p *m25p)
{
        struct m25p_otp *m25p_otp = NULL;
        int err = 0;

        m25p_otp = kmalloc(sizeof(*m25p_otp), GFP_KERNEL);
        if (!m25p_otp) {
                gxlog_e("flash otp probe: no memory for info\n");
                return -EINVAL;
        }

        memset(m25p_otp, 0, sizeof(*m25p_otp));
                                                                                                               
        m25p_otp->flash.addr_width = m25p->flash.addr_width;
        m25p_otp->flash.size = m25p->flash.size;
        m25p_otp->flash.info = m25p->flash.info;
        m25p_otp->flash.spi = m25p->flash.spi;
        m25p_otp->lock = &m25p->lock;
        g_otp_flash = m25p_otp;

        err = misc_register(&flash_otp_miscdev);
        if (err) {
                gxlog_e("flash otp probe: failed to register misc device\n");
                kfree(m25p_otp);
                err = -EINVAL;
        }

        return err;
}
```

​		创建一个 杂项设备，设备名称为 `flash_otp`，并提供 `open、llseek、read、write、ioctl、release`等方法给用户使用。

```c
struct file_operations flash_nor_fops =
{
        .owner   = THIS_MODULE,
        .open    = flash_nor_do_open,
        .llseek  = NULL,
        .read    = NULL,
        .write   = NULL,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
        .unlocked_ioctl = flash_nor_do_ioctl,
#else
        .ioctl   = flash_nor_do_ioctl,
#endif
        .release = flash_nor_do_release,
};

static struct miscdevice flash_nor_miscdev =
{
        .minor = MISC_NOR_MINOR,
        .name  = "flash_nor",
        .fops  = &flash_nor_fops,
};
                                                                                                               
static int flash_nor_probe(struct m25p *m25p)
{
        struct m25p_flash_nor *m25p_nor = NULL;
        int err = 0;

        m25p_nor = kmalloc(sizeof(*m25p_nor), GFP_KERNEL);
        if (!m25p_nor) {
                gxlog_e("flash nor probe: no memory for info\n");
                return -EINVAL;
        }

        memset(m25p_nor, 0, sizeof(*m25p_nor));

        m25p_nor->flash.addr_width = m25p->flash.addr_width;
        m25p_nor->flash.protect_mode = m25p->flash.protect_mode;
        m25p_nor->flash.match_mode = m25p->flash.match_mode;
        m25p_nor->flash.size = m25p->flash.size;
        m25p_nor->flash.info = m25p->flash.info;
        m25p_nor->flash.spi = m25p->flash.spi;
        m25p_nor->lock = &m25p->lock;
        g_flash_nor = m25p_nor;

        err = misc_register(&flash_nor_miscdev);
        if (err) {
                gxlog_e("flash nor probe: failed to register misc device\n");
                kfree(m25p_nor);
                err = -EINVAL;
        }

        return err;
}


```

​		创建一个 杂项设备，设备名称为 `flash_nor`，并提供 `open、ioctl、release`等方法给用户使用。



### module_exit(m25p80_exit)

​		注销设备时只取消注册 spi 设备驱动：

```c
static void m25p80_exit(void)
{
    spi_unregister_driver(&m25p80_driver);
    if (extend_m25p80_param) {
        spi_unregister_driver(&extend_m25p80_driver);
    }
}
```





## 六、总结

  	SPI 驱动框架分为主机控制器驱动和设备驱动，主机控制器也就是SOC中的SPI IP，设备就是很多SPI设备。

## SPI 主机驱动

​	Linux 内核使用`spi_master`表示SPI主机驱动，驱动主要工作如下：

- 申请`spi_master`
  - `struct spi_master *spi_alloc_master(struct device *dev, unsigned size)`
- 初始化`spi_master`
- 向Linux内核注册`spi_master`
  - `int spi_register_master(struct spi_master *master)`



### 设备驱动

​	Linux 内核使用`spi_driver`结构体表示SPI设备驱动，设备驱动主要工作如下：

- 初始化`struct spi_driver`
- 向Linux内核注册`spi_driver`
  - `int spi_register_driver(struct spi_driver *sdrv)`



### 主机驱动与设备匹配过程

​	SPI设备驱动和主机驱动匹配过程是由SPI总线来完成的，代码如下：

- 通过设备树来进行匹配：`of_driver_match_device(dev, drv)`
- 通过ACPI的方式来匹配:`acpi_driver_match_device(dev, drv)`
- 通过无设备树，比较名称：`spi_match_id(sdrv->id_table, spi)`
- 通过`spi_device`的modalias和`device_driver`的name成员比较：`strcmp(spi->modalias, drv->name)`

```c
static int spi_match_device(struct device *dev, struct device_driver *drv)
{
        const struct spi_device *spi = to_spi_device(dev);
        const struct spi_driver *sdrv = to_spi_driver(drv);

        /* Attempt an OF style match */
        if (of_driver_match_device(dev, drv))
                return 1;

        /* Then try ACPI */
        if (acpi_driver_match_device(dev, drv))
                return 1;

        if (sdrv->id_table)
                return !!spi_match_id(sdrv->id_table, spi);

        return strcmp(spi->modalias, drv->name) == 0;
}

```





-------------

保留疑问：

1. 驱动和设备进行匹配时，是平台设备<font color=green>(platform_device / device tree)</font>和平台设备驱动<font color=green>(platform_driver)</font>进行匹配，还是设备和spi_master进行匹配？
   1. 设备和设备驱动进行匹配，在bus_type.match回调中按照策略匹配
   1. 这里的驱动和设备指的是主机驱动和设备驱动
2. `drivers/spi/spi-dw-gx-v2.c`中获取的 resource 在何处定义？
   1. 是设备树中的寄存器信息，通过在设备树中定义`reg = <>`，可以使用`platform_get_resource`来获取并进行ioremap

3. m25p80 作为 MTD 设备，在 Linux 系统中如何进行测试？
   1. 直接使用测试程序`mtd_unitil`
4. m25p80 在 probe 的时候也会分别注册 nor 和 otp 两个 misc 设备，在 Linux 起来之后如何进行测试？
   1. 使用 `open、read、write、ioctl` 分别操作 `/dev/flash_nor`和`/dev/flash_otp`设备
5. m25p->mtd.read/write 什么时候调用？
   1. 创建了mtd设备，使用mtd_debug来操作mtd设备读写






















烧录：

- 首先烧录BootLoader
- 使用 `load_conf_down`根据 flash.conf 来烧写 分区表table.bin、uImage、rootfs.bin
- 要跑mtd_debug 需要将其编译好后放到 rootfs中





所需文件如下：

- bootloader ->.boot .bin
- dts 和 uImage 组成的 kernel.bin(使用genflash时flash.con中修改uImage为kernel.bin)
- rootfs --> mkcramfs --> rootfs.bin
- 分区表？--> genfalsh --> table.bin + 整bin(包括bootloader.bin + kernel.bin + rootfs.bin + table.bin)
- ubi --> 使用 mkcramfs之后的.bin 再进行制作









1. 在gxloader 中修改 `board-init.c`，打开分区表功能，编译.boot、.bin，bin烧写到flash去

   1. 使用UBI文件系统需要打开conf配置ENABLE_ROOTFS_UBI=y;

2. 编译linux kernel、gxmisc 、linux kernel，生成 dts、uImage，cat组合成kernel.bin

   `cat /opt/goxceed/kernel-arm/arch/arm/boot/dts/sirius_generic.dtb /opt/goxceed/kernel-arm/arch/arm/boot/uImage > kernel.bin`

3. 使用busybox编译根文件系统，在_install 目录生成了根文件系统，手动创建几个文件夹，这个时候需要将文件夹压缩成tar.bz的格式吗？

4. 使用genrootfs 根据 flash.conf <font color=red>(flash.conf从哪里来的?)</font>生成整bin

   1. flash.conf 自带，可以自己修改
   2. 命令用错了`./genflash mkflash flash.con 111.bin`

![image-20221013160113817](image/image-20221013160113817.png)

5. 使用boot工具烧写生成的整bin 还是使用 load_conf_down 命令下载?

- `sudo boot -b xxx.boot -c load_conf_down flash.conf serialdown`

```bash
sudo boot -b sirius-6633H1-sflash.boot -c serialdown 0x0 1.bin
```









