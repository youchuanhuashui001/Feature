
# 编译脚本

- 编译脚本需要添加 eCos 的 spi nand 驱动文件，然后编译完成后拷贝过去
- 包括 xxx_defconfig 文件
- 编译哪些文件，在 config.mk 文件中添加

# 添加驱动
### gxmisc
[http://git.nationalchip.com/gerrit/#/c/79875/2](http://git.nationalchip.com/gerrit/#/c/79875/2)

- 添加 `drivers/spinand/ecos/spinand.c`根据 eCos 的框架，上面对接 eCos，下面对应 spinand_base.c 相当于做一个封装层
- spinand_base.c、spinand_bbt.c、spinand_ids.c 等文件中需要确认是否有跟 os 相关的东西需要修改
- 头文件中，goxceed 会跨 os 所以这边要做 os 的适配，用统一的接口

#### 详解 中间的封装层

- 头文件定义：`struct cyg_flash_dev_funs`
```c
struct cyg_flash_dev_funs {
	int     (*flash_init) (struct cyg_flash_dev *dev);
	size_t  (*flash_query) (struct cyg_flash_dev *dev, void * data, size_t len);
	int     (*flash_erase) (struct cyg_flash_dev *dev, cyg_flashaddr_t base, size_t len);
	int     (*flash_program) (struct cyg_flash_dev *dev, cyg_flashaddr_t base, const void* data, size_t len);
	int     (*flash_read) (struct cyg_flash_dev *dev, const cyg_flashaddr_t base, void* data, size_t len);
	int     (*flash_info) (struct cyg_flash_dev *dev, char* info);
#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING    
	int     (*flash_block_lock) (struct cyg_flash_dev *dev, const cyg_flashaddr_t block_base);
	int     (*flash_block_unlock) (struct cyg_flash_dev *dev, const cyg_flashaddr_t block_base);
	int     (*flash_set_lock_sr) (struct cyg_flash_dev *dev);
	int     (*flash_get_lock_sr) (struct cyg_flash_dev *dev, int *lock);
#endif
	int     (*flash_otp_enter)  (struct cyg_flash_dev *dev, unsigned char on);
	int     (*flash_otp_lock)   (struct cyg_flash_dev *dev);
	int     (*flash_otp_status) (struct cyg_flash_dev *dev, unsigned char *buf);
	int     (*flash_otp_erase)  (struct cyg_flash_dev *dev);
	int     (*flash_otp_read)   (struct cyg_flash_dev *dev, const cyg_flashaddr_t base,
									void *data, size_t len);
	int     (*flash_otp_write)  (struct cyg_flash_dev *dev, cyg_flashaddr_t base,
									const void *data, size_t len);
	int     (*flash_otp_get_region) (struct cyg_flash_dev *dev, unsigned int *region);
	int     (*flash_otp_set_region) (struct cyg_flash_dev *dev, unsigned int region);
	int     (*flash_otp_get_current_region) (struct cyg_flash_dev *dev, unsigned int *region);
	int     (*flash_otp_get_region_size) (struct cyg_flash_dev *dev, unsigned int *size);
	int     (*flash_xip_enter)  (struct cyg_flash_dev *dev);
	int     (*flash_xip_exit)   (struct cyg_flash_dev *dev);

	int     (*flash_sync)   (struct cyg_flash_dev *dev);
	int     (*flash_scan_ident)   (struct cyg_flash_dev *dev);
	int     (*flash_read_oob)  (struct cyg_flash_dev *dev, const cyg_flashaddr_t base, struct mtd_oob_ops *ops);
	int     (*flash_write_oob)  (struct cyg_flash_dev *dev, cyg_flashaddr_t base, struct mtd_oob_ops *ops);
	int     (*flash_block_isbad)  (struct cyg_flash_dev *dev, loff_t ofs);
	int     (*flash_block_markbad) (struct cyg_flash_dev *dev, loff_t ofs);
};
```

- `drivers/spinand/ecos/spinand.c`
   - 宏 `CYG_FLASH_DRIVER`：定义一个结构 `struct cyg_flash_dev`类型的变量，并且将其通过 attribute 到某一个段
```c
#define CYG_FLASH_DRIVER(_name_, _funs_, _flags_, _start_, _end_, _num_block_infos_,\
			_block_info_, _priv_, _jedec_id_)  \
struct cyg_flash_dev _name_ CYG_HAL_TABLE_ENTRY(cyg_flashdev) = \
{                                                               \
    .funs               = _funs_,                               \
    .flags              = _flags_,                              \
    .start              = _start_,                              \
    .end                = _end_,                                \
    .num_block_infos    = _num_block_infos_,                    \
    .block_info         = _block_info_,                         \
    .priv               = _priv_,                               \
    .jedec_id           = _jedec_id_,                           \
}


#define CYG_HAL_TABLE_ENTRY( _name ) \
        CYGBLD_ATTRIB_SECTION(".ecos.table." __xstring(_name) ".data") \
        CYGBLD_ATTRIB_USED
```

   - 宏 `CYG_FLASH_FUNS`：定义一个结构 `struct cyg_flash_dev_funs`类型的变量，其中的成员都是各种函数
```c
#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
# define CYG_FLASH_FUNS(_funs_, _init_, _query_ , _erase_, _prog_ , _read_, _info_,\
		_lock_, _unlock_, _otp_enter_, _otp_lock_, _otp_status_, _otp_erase_, _otp_read_, _otp_write_,\
		_otp_get_region_, _otp_set_region_,\
		_xip_enter_, _xip_exit_, _sync_, \
		_set_lock_sr_, _get_lock_sr_, _get_current_region, _get_region_size, \
		_scan_ident_, _read_oob_, _write_oob_, _block_isbad_, _block_markbad_)\
struct cyg_flash_dev_funs _funs_ =      \
{					    \
	.flash_init             = _init_,        \
	.flash_query            = _query_,       \
	.flash_erase            = _erase_,       \
	.flash_program          = _prog_,        \
	.flash_read             = _read_,        \
	.flash_info             = _info_,        \
	.flash_block_lock       = _lock_,        \
	.flash_block_unlock     = _unlock_,      \
	.flash_otp_enter        = _otp_enter_,   \
	.flash_otp_lock         = _otp_lock_,    \
	.flash_otp_status       = _otp_status_,  \
	.flash_otp_erase        = _otp_erase_,   \
	.flash_otp_read         = _otp_read_,    \
	.flash_otp_write        = _otp_write_,   \
	.flash_otp_get_region   = _otp_get_region_,  \
	.flash_otp_set_region   = _otp_set_region_,  \
	.flash_otp_get_current_region = _get_current_region, \
	.flash_otp_get_region_size = _get_region_size,       \
	.flash_xip_enter        = _xip_enter_,    \
	.flash_xip_exit         = _xip_exit_,     \
	.flash_sync             = _sync_,         \
	.flash_set_lock_sr      = _set_lock_sr_,  \
	.flash_get_lock_sr      = _get_lock_sr_,  \
	.flash_scan_ident       = _scan_ident_,  \
	.flash_read_oob         = _read_oob_,  \
	.flash_write_oob        = _write_oob_,  \
	.flash_block_isbad      = _block_isbad_,  \
	.flash_block_markbad    = _block_markbad_  \
}
```

- `struct cyg_flash_dev` 结构用于在 eCos 中描述一个 flash 设备，其定义如下：
```c
struct cyg_flash_dev {
	const struct cyg_flash_dev_funs *funs;            // Function pointers
	cyg_uint32                      flags;            // Device characteristics
	cyg_flashaddr_t                 start;            // First address
	cyg_flashaddr_t                 end;              // Last address
	cyg_uint32                      num_block_infos;  // Number of entries
	cyg_flash_block_info_t          *block_info;      // Info about one block size

	const void                      *priv;            // Devices private data
	cyg_uint32                      jedec_id;         // It has a high byte of zero plus three data bytes:
                                                      // the manufacturer id, then a two byte device id.
	struct cyg_flash_otp_dev        otp;              // Otp info
	

	// The following are only written to by the FLASH IO layer.
	cyg_flash_printf                *pf;              // Pointer to diagnostic printf
	cyg_bool                        init;             // Device has been initialised
#ifdef CYGPKG_KERNEL
	cyg_mutex_t                     mutex;            // Mutex for thread safeness
#endif
#if (CYGHWR_IO_FLASH_DEVICE > 1)    
	struct cyg_flash_dev            *next;            // Pointer to next device
#endif
	cyg_uint32                      status;		  //flash status
	cyg_uint32                      protect_addr;	  //flash protect addr
	cyg_uint8                       mode;		  //flash mode: mode_top or mode_bottom 
} CYG_HAL_TABLE_TYPE;

```

- gxmisc 中最先被调到的函数 init：
   - 把 flash 设备跟 spi 绑定起来，struct cyg_spinand spinand_dev 根据这个变量就可以拿到对应的 spi 了
   - 组建好一个结构体传给 eCos，eCos 根据这个结构体来拿到 gxmisc 中的各个接口，这个结构体就是 gx_spi_nand_device，作为 flash 设备的私有数据
```c
static struct spi_device spi_dev = {
	.master = NULL,
	.max_speed_hz = 30000000,
	.chip_select = 0,
	.mode = 0,
};

struct cyg_spinand {
	cyg_drv_mutex_t lock;
	struct spinand_chip flash_chip;
};

struct cyg_spinand spinand_dev = {
	.flash_chip = {
		.info = NULL,
		.spi = &spi_dev,
	},
};


struct cyg_flash_dev gx_spinand_device =
{
	.funs               = &cyg_devs_gxspinand_funs,
	.flags              = 0,
	.start              = 0,
	.end                = 0,
	.num_block_infos    = 1,
	.block_info         = &gx_spinand_device_block_info,
	.priv               = &spinand_dev,
	.jedec_id           = 0
};

__externC void gx_devs_spinand_init_v2(struct spi_master *master)
{
	spinand_dev.flash_chip.spi->master = master;
	flash_dev_register(&gx_spinand_device);
}

```

- 再看一下 flash_dev_register 是在干什么，这个函数的实现在 `packages/io/dev_manage/v3_0/src/dev_regist.cxx`中
- 刚刚定义了一个结构体变量，函数里面定义一个指针，找所有的成员，如果 有一个成员的.funs 不是空的，那么这个位置就是空的，然后调用 memcpy 把初始化好的结构体变量拷贝过去
- 所有的成员看下面是怎么做的：通过一个 cyg_flashdevtab[0] 和 cyg_flashdevtab_end 两个变量来限定的
   - 相当于是用汇编定义了一个 section ，这个section的头和尾就分别是 cyg_flashdevtab[0] 和 cyg_flashdevtab_end
```c
__externC struct cyg_flash_dev cyg_flashdevtab[];
CYG_HAL_TABLE_BEGIN(cyg_flashdevtab, cyg_flashdev);

// end of the flashdev table
__externC struct cyg_flash_dev cyg_flashdevtab_end;
CYG_HAL_TABLE_END(cyg_flashdevtab_end, cyg_flashdev);


#ifndef CYG_HAL_TABLE_BEGIN
#define CYG_HAL_TABLE_BEGIN( _label, _name )                                 \
__asm__(".section \".ecos.table." __xstring(_name) ".begin\",\"aw\"\n"       \
    ".globl " __xstring(CYG_LABEL_DEFN(_label)) "\n"                         \
    ".type    " __xstring(CYG_LABEL_DEFN(_label)) ",object\n"                \
    ".p2align " __xstring(CYGARC_P2ALIGNMENT) "\n"                           \
__xstring(CYG_LABEL_DEFN(_label)) ":\n"                                      \
    ".previous\n"                                                            \
       )
#endif

#ifndef CYG_HAL_TABLE_END
#define CYG_HAL_TABLE_END( _label, _name )                                   \
__asm__(".section \".ecos.table." __xstring(_name) ".finish\",\"aw\"\n"      \
    ".globl " __xstring(CYG_LABEL_DEFN(_label)) "\n"                         \
    ".type    " __xstring(CYG_LABEL_DEFN(_label)) ",object\n"                \
    ".p2align " __xstring(CYGARC_P2ALIGNMENT) "\n"                           \
__xstring(CYG_LABEL_DEFN(_label)) ":\n"                                      \
    ".previous\n"                                                            \
       )
#endif

```

- 那现在可以看 flash_dev_register 函数的实现了，就是在一个 section 中遍历，如果里面没有成员，那么就把刚初始化好的结构体成员拷贝过去放在那个 section 中
```c
Cyg_ErrNo flash_dev_register(struct cyg_flash_dev *tab)
{
	struct cyg_flash_dev *t;

	if (tab == NULL)
		return EINVAL;

	for (t = &cyg_flashdevtab[0]; t != &cyg_flashdevtab_end; t++) {
		if(t->funs ==NULL) {
			//diag_printf("find one empty entry\n");
			break;
		}
	}

	if(t == &cyg_flashdevtab_end)
	{
		D("there is no empty cyg_flashdevtab\n");
		return ENOMEM;
	}

	memcpy(t, tab, sizeof(struct cyg_flash_dev));
	return ENOERR;
}
```

- 然后剩下的都是各种接口了，现在已经 flash 和 spi 链接上了，而且和 eCos 链接上了，eCos 可以通过某种方式调用到 gxmisc 中的各个接口
- 上面执行的是在 eCos_shell 中定义了 dw_spi 的时候会执行的 flash 和 spi 的初始化，还要定义 flashio，执行 flashio 的初始化
- flashio 的初始化就会调用 flashio_init 函数，这个函数会调用 block_dev_register，这里涉及到 eCos 的驱动相关知识
   - eCos 驱动需要用到两个宏 `BLOCK_DEVTAB_ENTRY`、`BLOCK_DEVIO_TABLE`，这两个宏相当于定义了两个结构体变量，分别是 `cyg_devio_table_t  xxx` 和 `cyg_devtab_entry_t xxx`
```c
#define BLOCK_DEVIO_TABLE(_l,_bwrite,_bread,_select,_get_config,_set_config)    \
cyg_devio_table_t _l = {                                        \
    cyg_devio_cwrite,                                           \
    cyg_devio_cread,                                            \
    _bwrite,                                                    \
    _bread,                                                     \
    _select,                                                    \
    _get_config,                                                \
    _set_config												\
};


#define BLOCK_DEVTAB_ENTRY(_l,_name,_dep_name,_handlers,_init,_lookup,_priv)  \
cyg_devtab_entry_t _l CYG_HAL_TABLE_ENTRY(devtab) = {                   \
   _name,                                                               \
   _dep_name,                                                           \
   _handlers,                                                           \
   _init,                                                               \
   _lookup,                                                             \
   _priv,                                                               \
   CYG_DEVTAB_STATUS_BLOCK                                              \
};
```

   - 但驱动中使用的是 `block_dev_register`
```c
cyg_devio_table_t cyg_io_flashdev_ops={
	cyg_devio_cwrite,
	cyg_devio_cread,
	&flashiodev_bwrite,
	&flashiodev_bread,
	NULL, // no select
	&flashiodev_get_config,
	&flashiodev_set_config,
	&flashiodev_ioctl,
	flashiodev_close
};

static cyg_devtab_entry_t cyg_io_flashdev={
	"/dev/flash/",
	NULL,
	&cyg_io_flashdev_ops,
	&flashiodev_init,
	&flashiodev_lookup,
	NULL 
};

EMPTY_DEVTAB_ENTRY(cyg_io_flashdev_void);
__externC void flashio_init(void)
{
	block_dev_register(&cyg_io_flashdev, "/dev/flash/", NULL);
}
```

- 这里也是定义了那两个结构体变量 `cyg_devio_table_t  xxx` 和 `cyg_devtab_entry_t xxx`
- 然后看 block_dev_register 做了什么事情，定义一个指针 `cyg_devtab_entry_t *tab`指向刚刚初始化好的结构体变量
```c
Cyg_ErrNo block_dev_register(cyg_devtab_entry_t *ops, char *name, void *priv_dev)
{
	cyg_devtab_entry_t *tab;

	if (ops == NULL)
		return EINVAL;

	tab = ops;
	tab->name = name;
	tab->priv = priv_dev;

	tab->status = CYG_DEVTAB_STATUS_BLOCK | CYG_DEVTAB_STATUS_AVAIL;

	return gx_devtab_copy(tab);
}
```

- 基本就是将可用的信息赋值过去，然后配置一些 status，再调用 gx_devtab_copy(tab)
```c
static Cyg_ErrNo
gx_devtab_copy(struct cyg_devtab_entry *tab)
{
	struct cyg_devtab_entry *t;
	bool ret_value;

	if (tab == NULL)
		return EINVAL;

    for (t = &__DEVTAB__[0]; t != &__DEVTAB_END__; t++) {
		if(t->status == CYG_DEVTAB_STATUS_EMPTY) {
			break;
		}
	}

    if(t == &__DEVTAB_END__)
	{
		diag_printf("there is no empty entry\n");
		return ENOMEM;
    }

	t->dep_name = tab->dep_name;
	t->handlers = tab->handlers;
	t->init = tab->init;
	t->lookup = tab->lookup;
	t->name = tab->name;
	t->priv = tab->priv;
	t->status = tab->status;
	if(t->init !=NULL)
	{
		ret_value=t->init(t);
		if(ret_value==true)
		{
			//diag_printf("dynamic device \'%s\' init finished \n",t->name);
		}else{
			diag_printf("dynamic device \'%s\' init error \n",t->name);
			return EINITERR;
		}
	}
	return ENOERR;
}

```

- 在 __DEVTAB__[0] 到 __DEVTAB_END_结构中找，如果 t->status 是空的话，就相当于找到了可用的空间，需要看这个开头和结尾代表的是什么，相当于还是在 section 中找的，找到了就把值都拷贝过去，然后如果 t->init 存在的话就执行一下 t->init 函数，相当于执行的是 `flashiodev_init`函数
```c
CYG_HAL_TABLE_BEGIN( __DEVTAB__, devtab );
CYG_HAL_TABLE_END( __DEVTAB_END__, devtab );

```

- 看一下 `flashiodev_init`函数怎么实现的
```c
	static cyg_bool
flashiodev_init( struct cyg_devtab_entry *tab )
{
	int stat = cyg_flash_init(NULL);
	cyg_ucount32 i;

	if (stat == CYG_FLASH_ERR_OK)
	{
		for (i = 0; i < CYGNUM_IO_FLASH_BLOCK_DEVICES; i++)
		{
			CYG_ASSERT( tab->handlers == &cyg_io_flashdev_ops, "Unexpected handlers - not flashdev_ops" );
			flashiodev_table[i].handle.handlers = &cyg_io_flashdev_ops;
			flashiodev_table[i].handle.status   = CYG_DEVTAB_STATUS_BLOCK;
			flashiodev_table[i].handle.priv     = &flashiodev_table[i]; // link back
		} // for
		cyg_drv_mutex_init( &flashiodev_table_lock );
	} // if

	return (stat == CYG_FLASH_ERR_OK);
} // flashiodev_init()
```

- 首先会调用 cyg_flash_init，这个函数的实现在 `packages/io/flash/v3_0/src/flashiodev.c`中，函数主要目的是在所有的 flashdevtab 表中执行每个 dev 的dev->funs->flash_init ，这里调到了 gxmisc 中的函数 gx_spinand_init 函数，这个函数里初始化了互斥锁，并且调用了 spi->setup 函数
```c
static int gx_spinand_init(struct cyg_flash_dev *dev)
{
	struct cyg_spinand *cyg_spinand = (struct cyg_spinand *)dev->priv;

	cyg_drv_mutex_init(&cyg_spinand->lock);

	spi_setup(cyg_spinand->flash_chip.spi);

	return FLASH_ERR_OK;
}
```

- flashuidev_init 函数中调用 gxmisc 的flash初始化成功后，又涉及到一个 `struct flashiodev_priv_t `
```c
struct flashiodev_priv_t{
	cyg_devtab_entry_t handle;
	cyg_flashaddr_t start;
	cyg_flashaddr_t end;
	cyg_bool        valid;
	cyg_uint32 devno;
};
static struct flashiodev_priv_t flashiodev_table[CYGNUM_IO_FLASH_BLOCK_DEVICES];

			CYG_ASSERT( tab->handlers == &cyg_io_flashdev_ops, "Unexpected handlers - not flashdev_ops" );
			flashiodev_table[i].handle.handlers = &cyg_io_flashdev_ops;
			flashiodev_table[i].handle.status   = CYG_DEVTAB_STATUS_BLOCK;
			flashiodev_table[i].handle.priv     = &flashiodev_table[i]; // link back
```

- 把上面的结构和这个新的结构链起来，再初始化一下驱动的互斥锁，至此初始化过程就结束了，即在 ecos_shell 中定义的几个类所执行的函数完成。
- 还要关心使用 gxmisc 接口时的函数调用过程，事先关心 gxmisc 提供给 eCos 有哪些接口：
- 擦除接口：
   - 计算总共要擦几个 block，最小是 1 个block
   - 调用 chip->erase_block 擦除 block
   - 如果擦除过程出现错误，就标记当前 block 为坏块，更新 flash 中的坏块表，并且将page_main_size 这个地址写 0，表示是坏块
- 写接口：
   - 先计算是写 oob 区域还是 main 区域，然后组建一个 buf，buf 的大小是 main+oob 这么大，把要写的数据拷贝到 main 区域，要写到 oob 区域的拷贝到 oob 区域，然后调用 chip->program_page 接口，往 flash 写数据的时候支持 0~main_size+oob_size - 1
- 读接口：
   - 先计算是读 oob 区域还是 main 区域，还是一个 buf，buf 的大小是 main+oob 那么大，调用 chip->read_page 接口读数据，读到 buf对应的区域，然后拷贝到 ops->datbuf 对应的区域，其实就是掉 chip->read_page 接口把flash 中的数据读到 buf，然后buf拷贝到 ops
- 其余的接口基本都是加个锁，然后调用 spinand_base.c 的接口
- 坏块表的创建，就是原来放在 nos 初始化的那些函数，放在了 scan_ident 接口中了，只是换了个位置而已

### eCos
[http://git.nationalchip.com/gerrit/#/c/79872](http://git.nationalchip.com/gerrit/#/c/79872)

- 修改编译脚本，在 clean 的时候清除掉 spinand.a
- arm.ecc、arm_smp.ecc、csky.ecc 、csky_debug.ecc 中添加 SPINAND 相关的 package
- `packages/devs/flash/spinand/v3_0/cdl/gx_spinand_flash.cdl` 添加 cdl 文件，指定编译路径
- 添加 flash 的类的构造函数，包括执行 spi 和 flash 的初始化函数
```c
class cyg_gx_spinand_flash_init_class {
public:
	cyg_gx_spinand_flash_init_class(void) {
		gx_devs_spi_init();
		gx_devs_spinand_init_v2(&gx_spi_master);
	}
};
```

- 添加 ecos.db ，将刚刚的 cdl 文件添加到组件库中
- 添加编译阶段的链接库
- 添加 测试用的 类定义
```c
cyg_spinand_init_class spinandflash0 CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_DEV_FLASH+30)
```

- 添加 封装层，下面对接 gxmisc 中 def->funs 函数，上面给到 flashio 层用，在 get_config、set_config 里面调用封装层
- 看一下封装层：`pacages/io/flash/v3_0/src/flash.c`
   - 基本都是套路一样的，就是在调 gxmisc 的 dev->funs->函数
- 在哪里做的跳坏块？
   - flash.c cyg_flash_erase --> dev->funs->flash_erase --> spinand_erase_opts 这个里面会处理跳坏块
- 看一下 flash io层：`packages/io/flash/v3_0/src/flashiodev.c`
   - 这里面主要是定义 DEVTAB_ENTRY 、 DEV_IO_TABLE 这种东西的，然后实现了 read、write、set_config、get_config、lookup、close
   - `flashiodev_lookup`：可以有两种方式定义 flash，目前用的是 /dev/flash/0/0x0，扫描第1 个0，要在0~9之间，然后调用 cyf_flash_scan_ident 去读 id，然后识别 ids，下面就是一些获取参数的
```c
	static Cyg_ErrNo
flashiodev_lookup(struct cyg_devtab_entry **tab,
		struct cyg_devtab_entry  *sub_tab,
		const char *name) 
{
	// We enter with **tab being the entry for /dev/flash only
	// We will leave with **tab instead pointing to a newly configured flash device instance
	// We could allocate the space for recording these dynamically, but that's overkill,
	// so we instead choose the eCos ethos of "let the user decide at configure time".
	// After all there's a good chance there'll be only one or two needed at one time.

	// There are two styles of path:
	//    /dev/flash/fis/<fispartitionname>
	// and
	//    /dev/flash/<deviceno>/<offset>[,<length>]

	cyg_flashaddr_t start=0;
	cyg_flashaddr_t end=0;
	cyg_bool valid = false;
	cyg_uint32 flashdevno  = 0;

#ifdef CYGFUN_IO_FLASH_BLOCK_FROM_FIS
	// First, support FIS
	if ((name[0] == 'f') 
			&& (name[1] == 'i') 
			&& (name[2] == 's') 
			&& (name[3] == '/'))
	{
		CYG_ADDRESS	flash_base;
		unsigned long	size;

		if(!CYGACC_CALL_IF_FLASH_FIS_OP(CYGNUM_CALL_IF_FLASH_FIS_GET_FLASH_BASE, 
					(char *)&name[4],
					&flash_base))
			return -ENOENT; 

		if(!CYGACC_CALL_IF_FLASH_FIS_OP(CYGNUM_CALL_IF_FLASH_FIS_GET_SIZE, 
					(char *)&name[4],
					&size))
			return -ENODEV; // If the previous call worked, then this failing would be very wrong.
		start = flash_base;
		end = flash_base + size - 1;
		valid = true;
	} else
#endif // ifdef CYGFUN_IO_FLASH_BLOCK_FROM_FIS
#ifdef CYGFUN_IO_FLASH_BLOCK_FROM_DEVOFFSET
		// Next, support device numbers with offsets encoded in path name
		// Note that for now we assume < 10 flash devices. I think this is reasonable
		// to avoid unnecessary code. It can be changed later if needed.
		if ( (name[0] >= '0') && (name[0] <= '9') )
		{
			int res;
			cyg_flash_info_t info;
			cyg_uint32 offset;
			const char *tempstr;

			flashdevno = name[0] - '0';
			if (name[1] != '/')
				return -ENOTSUP;

			cyg_flash_scan_ident(flashdevno);
			res = cyg_flash_get_info( flashdevno, &info );
			if (CYG_FLASH_ERR_OK != res)
				return -ENOENT;

			// Now modify with offset and optional length
			tempstr = &name[2];
			offset = parsenum( &tempstr );

			start = info.start + offset;
			end = info.end;

			if (*tempstr == ',') // optional length
			{
				cyg_uint32 length;

				tempstr++;
				length = parsenum( &tempstr );

				// Check we don't exceed end of device.
				// Note the flash end address is the last byte of addressible flash *not* the base+size
				if ( (start + length - 1) > end )
					return -EINVAL;
				end = start + length - 1;
			}
			valid = true;
		} // else if
#endif

	if (valid)
	{
		// Find a slot and use it
		cyg_ucount32 i;

		cyg_drv_mutex_lock( &flashiodev_table_lock );
		for (i=0; i<CYGNUM_IO_FLASH_BLOCK_DEVICES; i++)
		{
			if ( !flashiodev_table[i].valid ) {
				flashiodev_table[i].start = start;
				flashiodev_table[i].end = end;
				flashiodev_table[i].valid = true;
				flashiodev_table[i].devno = flashdevno;
				cyg_drv_mutex_unlock( &flashiodev_table_lock );

				*tab = &flashiodev_table[i].handle;
				// Theoretically we could set up the devtab entry more fully, e.g.
				// the name, but I don't see the need!
				return ENOERR;
			} // if
		} // for
		cyg_drv_mutex_unlock( &flashiodev_table_lock );

		// If we got here we must have reached the end of the table without finding a space
		return -ENOMEM;
	}
	// Wasn't valid, so....
	return -ENOENT;
} // flashiodev_lookup()

```

- `flashiodev_bread`主要就是判断一些参数，然后就把要读的 长度和 buf 调用 cyg_flash_read 也就是 flash.c 中封装层，然后调到 gxmisc dev->funs->xxx
- `flashiodev_bwrite`：主要就是判断一些参数，然后传递要写的 长度和 buf 给 cyg_flash_program，也就是 flash.c 中的封装层，调到了 gxmisc dev->funs->xxx
- `flashiodev_get_config`：一个大的 switch case，定义各种 key，在 key 里面按要求调用 flash.c 中封装的接口
- `flashiodev_set_config`：一个大的 switch case，定义各种 key，在 key 里面按要求调用 flash.c 中封装的接口
- `flashiodev_ioctl`：也是一个大的 switch case，定义各种 key，在 key 里面调用 get_config 或 set_config
- `flashiodev_close`：把 dev->valid 置成 false

### eCos_Shell

- eCos 中的 flashio 层提供了 read、write 、lookup 等接口，需要在应用中调用
- 首先按照名称 查找 flash 设备，调用 `cyg_io_lookup(devname, &flash_dev)`找到的设备就存在 flash_dev 中，接着就是调用 flash io 层提供的各个接口来完成测试用例的实现
