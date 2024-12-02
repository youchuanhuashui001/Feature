# eCos flash 

## 启动流程



#### 驱动注册加载：

- `ecos3.0/packages/io/flash/v3_0/src/flashiodev.c` 中定义了一个  `class cyg_flashio_init_class` 类，构造函数是 `flashio_init()`

  ```c
  #0  flashio_init () at /home/tanxzh/goxceed/develop/platform/ecos3.0/ecos3.0/packages/io/flash/v3_0/src/flashiodev.c:152
  #1  0x0000d220 in cyg_flashio_init_class::cyg_flashio_init_class (this=0x85778 <flashio>) at /home/tanxzh/goxceed/develop/platform/ecos3.0/ecos_shell/../../../library/goxceed/arm-ecos/include/cyg/io/gx_flashio_init.hxx:21
  #2  0x0000d0a0 in __static_initialization_and_destruction_0 (__initialize_p=1, __priority=61000) at ./include/gxcore_ecos_module.hxx:26
  #3  0x0000d178 in _GLOBAL__sub_I.61000_jffs20(void) () at source/spi_threads.cxx:51
  #4  0x0004ced8 in cyg_hal_invoke_constructors () at /home/alan/Goxceed/develop/platform/iecos/ecos3.0/packages/hal/gx/arm/arch/v3_0/src/hal_misc.c:271
  #5  0x0000864c in start () at /home/alan/Goxceed/develop/platform/iecos/ecos3.0/packages/hal/gx/arm/arch/v3_0/src/vectors.S:681
  
  
  ecos_shell 中在头文件中实例化了一个类：
      cyg_gx_spinand_flash_init_class spinandflash0 CYGBLD_ATTRBIT_INIT_PRI(60910);
  	cyg_flashio_init_class flashio CYGBLD_ATTRIB_INIT_PRI(61000)
  
  
  flashio_init()
      	cyg_devio_table_t cyg_io_flashdev_ops = {
      		cyg_devio_cwrite,
      		cyg_devio_cread,
      		&flashiodev_bwrite,
      		&flashiodev_bread,
      		NULL,
      		&flashiodev_get_config,
      		&flashiodev_set_config,
      		&flashiodev_ioctl,
      		flashiodev_close
  		};
      	static cyg_devtab_entry_t cyg_io_flashdev = {
      		"/dev/flash",
      		NULL,
      		&cyg_io_flashdev_ops,
      		&flashiodev_init,
      		&flashiodev_lookup,
      		NULL,
  		};
      block_dev_register(&cyg_io_flashdev, "/dev/flash/", NULL);
  		--> gx_devtab_copy(tab)
  
  (gdb) p __DEVTAB__[0]@10
  $5 = {{name = 0x62bb0 "/dev/ser0", dep_name = 0x0, handlers = 0x7bfa4 <cyg_io_serial_devio>, init = 0x19d5c <dsw_serial_init_void>, lookup = 0x19c58 <dsw_serial_lookup>, priv = 0x7be38 <dsw_serial_channel0>, status = 4097}, {
      name = 0x6878c "/dev/ser1", dep_name = 0x0, handlers = 0x7bfa4 <cyg_io_serial_devio>, init = 0x19d5c <dsw_serial_init_void>, lookup = 0x19c58 <dsw_serial_lookup>, priv = 0x7bc98 <dsw_serial_channel1>, status = 4097}, {
      name = 0x62bb0 "/dev/ser0", dep_name = 0x0, handlers = 0x7bfa4 <cyg_io_serial_devio>, init = 0x19d5c <dsw_serial_init_void>, lookup = 0x19c58 <dsw_serial_lookup>, priv = 0x7be38 <dsw_serial_channel0>, status = 4097}, {
      name = 0x6878c "/dev/ser1", dep_name = 0x0, handlers = 0x7bfa4 <cyg_io_serial_devio>, init = 0x19d5c <dsw_serial_init_void>, lookup = 0x19c58 <dsw_serial_lookup>, priv = 0x7bc98 <dsw_serial_channel1>, status = 4097}, {
      name = 0x62bb0 "/dev/ser0", dep_name = 0x0, handlers = 0x7bfa4 <cyg_io_serial_devio>, init = 0x19d5c <dsw_serial_init_void>, lookup = 0x19c58 <dsw_serial_lookup>, priv = 0x7be38 <dsw_serial_channel0>, status = 4097}, {
      name = 0x6878c "/dev/ser1", dep_name = 0x0, handlers = 0x7bfa4 <cyg_io_serial_devio>, init = 0x19d5c <dsw_serial_init_void>, lookup = 0x19c58 <dsw_serial_lookup>, priv = 0x7bc98 <dsw_serial_channel1>, status = 4097}, {name = 0x0, 
      dep_name = 0x0, handlers = 0x0, init = 0x0, lookup = 0x0, priv = 0x0, status = 16384}, {name = 0x0, dep_name = 0x0, handlers = 0x0, init = 0x0, lookup = 0x0, priv = 0x0, status = 16384}, {name = 0x0, dep_name = 0x0, handlers = 0x0, 
      init = 0x0, lookup = 0x0, priv = 0x0, status = 16384}, {name = 0x0, dep_name = 0x0, handlers = 0x0, init = 0x0, lookup = 0x0, priv = 0x0, status = 16384}}
  
           	在 __DEVTAB__[0] 到 __DEVTAB__END__ 中间找到空的一块，然后把 tab 的信息拷贝过去
                  如果 t->init 存在，则执行 t->init 也就是 flashiodev_init
              
  猜测应该在系统起来的某个时候会调用 flashiodev_init
      将 cyg_io_flashdev_ops 作为 handlers 给新注册的 flashiodev_table(一个全局数组)
                  遍历所有的 flash 设备，如果有 dev->funs->flash_init(dev) 就执行，相当于初始化每个 flash
  $9 = {{
      funs = 0x7c2a4 <cyg_devs_gxspinand_funs>, 
      flags = 0, 
      start = 0, 
      end = 0, 
      num_block_infos = 1, 
      block_info = 0xd4e68 <gx_spinand_device_block_info>, 
      priv = 0x7c228 <spinand_dev>, 
      jedec_id = 0, 
      otp = {
        start_addr = 0, 
        skip_addr = 0, 
        region_size = 0, 
        region_num = 0, 
        flags = 0
      }, 
      pf = 0x0, 
      init = 0, 
      mutex = {
        locked = 0 '\000', 
        owner = 0x0, 
        queue = {
          queue = 0x0
        }, 
        protocol = CYG_MUTEX_PROTOCOL_NONE, 
        ceiling = 0
      }, 
      next = 0x0, 
      status = 0, 
      protect_addr = 0, 
      mode = 0 '\000'
    }, {
  	初始化完 flash 之后，给 flashiodev_table[] 进行赋值，把所有的信息都放到这里，后面就操作这个结构
  ```

  















#### 用户程序

- 由 `cyg_user_start` 开始启动

- 启动之后执行 shell

  ```c
  cyg_io_lookup
      --> 根据名称找到对应的 lookup 回调执行，flashiodev_lookup
      	--> 根据。。。找到对应的 flashiodev 句柄，后面就可以用这个 flashiodev 句柄(用一个全局变量保存下来了)
      --> 如果是直接调用 eCos 的接口
      	--> 通过 cyg_io_get_config(flash_dev, CYG_IO_GET_CONFIG_FLASH_ERASE, &e, &elen) 的方式去调到对应的接口，也就是说所有的接口基本都在 get_config、set_config 里面
      		--> t->handlers->get_config(handle, key, buf, len);
  	--> 如果是调用 GxAPI 接口
          --> 通过 open 拿到一个句柄，read、write 等都通过这个句柄来操作
          	--> open 会调到哪里？？？？
          		--> open 到了 ecos3.0/packages/io/fileio/v3_0/src/file.cxx 中的 open 接口
          			--> 实际还是调到了 cyg_io_lookup(name, &handle)
          				--> 将 name 和 __DEVTAB__[0] ~ __DEVTAB_END__ 中的 name 比较来找到设备
          		--> 返回一个 fd 句柄
          --> 通过 ioctl 会调到哪里？
          	--> 根据 fd 获取 cyg_file *fp
          	--> 根据 fp-f_ops->fo_ioctl 调过去
          		--> cyg_io_ioctl
          			--> flashiodev_ioctl
          
         ----------------
         |     GxAPI    |  
         ----------------
         | flashiodev.c |
         ----------------
         |    flash.c   |
         ----------------
         |    gxmisc    |
         ----------------
  ```

  





## Partition

### Init

```c
GxCore_PartitionFlashInit(void)
    open("/dev/flash/0/0") 拿到句柄放到 table->fd
    ioctl(fd, CYG_IO_GET_CONFIG_FLASH_DEVSIZE) 获取到 flash_size
    GxCore_PartitionGetStartAddr(&offset)
    	获取整个 cmdline，指针指到 mtdparts= 处
    	在 mtdparts= 中找 "TABLE" ，找到了之后将 TABLE 所在的 part 信息保存到一个临时的 buffer
    	根据这个临时 buffer 解析字符串，找到对应的地址，也就获取到了 TABLE 分区的起始地址
    malloc 一块大 buffer，用来保存分区表信息
    ioctl(table->fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info) 获取 info
    	Nor flash：
    		lseek 到分区表所在的地址
    		read magic 到刚刚 malloc 的大 buffer
    		根据 magic 判断分区表的版本，这里以 V1 为例
    		重新 lseek 到分区表起始地址
    		读整个分区表 512字节到刚刚 malloc 的大 buffer
    		将读到的分区表信息填充到 table 中
    	SPINand flash：
    	Nand flash：
    		调用 nandread 读 magic
    			limit = partition->start_addr + partition->total_size 要读的长度不能超过这个总长度
    			计算当前要读的地址是否需要跳坏块
    				如果要读的地址就是好块，则就是这个地址
    				如果要读的地址是坏块，则跳坏块
    			如果要读的地址不是 block 对齐
    				计算 block_off = 要读的地址在一个 block 中的偏移地址
    				计算 block_len = 要读的地址在一个 block 中的偏移长度
    				如果要读的总长度小于 block_len，则 readlen = len - bufoffset
    				否则 len = block_len
    			如果要读的地址是 block 对齐
    				要读的长度小于一个 block，则 readlen = len - bufoffset
    				要读的长度大于等于 block，则 readlen = info->block_size
    			lseek 到偏移地址
    			read readlen到 buf+bufoffset
    			循环读，直到读完所有要读的长度
    		根据 magic 判断分区表的版本，这里以 V1 为例
    		读整个分区表
    		将读到的分区表信息填充到 table 中
    	将填充好的 table 返回给上层，上层可以操作这个分区表
```



### Read

```c
fd = open("dev/flash/0/0")
ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info)
根据 info 判断 flash 类型
    Nor：
    	lseek 到 partiton->start_addr+offset
    	read(fd,buf,size)
    SPINand：
    Nand：
    	nadn_calc_phy_offset,根据逻辑地址计算物理地址：
    		从分区的起始地址到逻辑地址，中间是不是有坏块，如果有坏块就跳坏块计算出物理地址。如果没有坏块，逻辑地址就等于物理地址。
    	nandread
    		先跳坏块
    		然后 lseek，再 read
```



### Write

```c
fd = open("dev/flash/0/0")
ioctl(fd, CYG_IO_GET_CONFIG_FLASH_INFO, &info)
根据 info 判断 flash 类型
    Nor：
    	lseek(fd, partition->start_addr + offset, SEEK_SET)
    	write(fd, buf, size)
    SPINand:
	Nand:
		nand_calc_phy_offset(fd, partition, &info, offset, &phy_offset)
            从分区的起始地址，到要操作的地址，判断这个区间是否有坏块，如果有坏块，则物理地址跳坏块；否则物理地址就等于逻辑地址
        nandwrite(fd, partition, &info, buf, size)
            地址跳坏块
            nand_write_block
            	这里传进去的长度会不满 block
            	如果长度和地址都 block 对齐，则直接 lseek 到 offset，然后 write
            	否则：
            		要写的长度大于 page
            			地址页对齐，则 writelen = len
            			地址不是页对齐，则 writelen = page_len
            		要写的长度小于等于 page
            			writelen = len
            		如果地址页对齐，并且长度页对齐
            			lseek 到 offset，write
            		否则：
            			malloc 一个临时 page_buf
            			lseek 到 page_start
            			读一个 page 出来到 buffer
            			把不对齐的要写的数据拷贝到 page_buf + page_off
            			lseek 到 page 开头
            			write 一个 page
            如果写失败，则标记坏块
```



### Erase

```c
open("dev/flash/0/0")
获取 block_size
获取 flash_info
SPINand、Nand：
    block_start 找到一个好块
    block_end 这个好块的下一个块
如果分区在一个 block 的中间:先读出来备份，擦掉，再写进去
    lseek(fd, block_start)
    read(fd, block_buf, block_size)
    擦除 block_start 所在的一个 block
    lseek(fd, block_start)
    write(fd, block_buf, unchange_size)
    lseek(fd, start_addr + write_size)
    write(fd, block_buf + start_addr - block_addr + write_size, unchange_size)
基本都是一样的操作，读出来备份、擦除、写进去
```







## Flash API

按照 `gxmisc/drivers/spinand/nos/spinand.c` 中的逻辑来处理