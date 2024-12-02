# MTD 子系统分析



## 目录结构：

```
mtdcore.c
mtdchar.c
mtdblock.c
mtd_blkdevs.c
mtdpart.c
cmdlinepart.c
```



### mtdcore

1. mtd 是一个模块，则这个模块会有 `module_init` 和 `module_exit` 的接口，在 `init`的时候去注册 class、初始化一个 mtd_bdi、创建 proc 下面的mtd 、初始化 `mtdchar` 设备。

   ```c
   static int __init init_mtd(void)
   {
   	int ret;
   
   	ret = class_register(&mtd_class);
   	if (ret)
   		goto err_reg;
   
   	ret = mtd_bdi_init(&mtd_bdi, "mtd");
   	if (ret)
   		goto err_bdi;
   
   	proc_mtd = proc_create("mtd", 0, NULL, &mtd_proc_ops);
   
   	ret = init_mtdchar();
   	if (ret)
   		goto out_procfs;
   
   	return 0;
   
   out_procfs:
   	if (proc_mtd)
   		remove_proc_entry("mtd", NULL);
   err_bdi:
   	class_unregister(&mtd_class);
   err_reg:
   	pr_err("Error registering mtd class or bdi: %d\n", ret);
   	return ret;
   }
   
   static void __exit cleanup_mtd(void)
   {
   	cleanup_mtdchar();
   	if (proc_mtd)
   		remove_proc_entry("mtd", NULL);
   	class_unregister(&mtd_class);
   	bdi_destroy(&mtd_bdi);
   	idr_destroy(&mtd_idr);
   }
   
   module_init(init_mtd);
   module_exit(cleanup_mtd);
   ```



2. 提供 `proc`相关的信息，通过 `proc_create` 的方式创建 proc目录下的设备提供 open、read、llseek、release 的方法。
   - open：先 malloc 一块内存空间，把 `mtd_proc_show` 接口放到 `struct seq_operations` ，show 里面是打印 mtd 的索引、大小和name等信息。
3. 提供`mtd_writev` 接口，基于向量的 mtd 写入方法。如果没有 `mtd->_writev` 接口就调用 `default_mtd_writev-->mtd_write`来写，否则使用 `mtd->_writev` 接口来写。
4. 提供 `mtd_block_markbad` 接口，用于标记坏块，实质上调用到`mtd->_block_markbad`，这里是由 flash 的驱动提供的 `struct mtd_info` 里的函数指针
5. 提供 `mtd->_block_isbad、mtd->_is_locked、mtd->_lock` 主要是根据 flash 的驱动中提供的 信息来封装的接口。

6. 提供一些注册 `mtd 设备` 的方式，和添加 `mtd 分区`  的方法。







### mtdchar

1. 注册 mtd 字符设备的主设备号为 90，次设备号为 0，并提供一些 ops，包括 open、close、read、write 等
   1. open 接口：获取mtd设备，用 `struct mtd_info` 结构来表示这个设备，然后把获取到的设备链到 `strcut mtd_file_info` 的结构上面去，然后把 `struct mtd_file_info` 的结构链到 `struct file` 结构上面去，那么在其它接口里面就可以拿到 `struct mtd_info` 这个结构了
      - 获取 mtd 设备的过程：`struct mtd_info *get_mtd_device(struct mtd_info *mtd, int num)`
        - 首先根据当前打开的哪个设备的次设备号来带当做 num 参数，调到 `__get_mtd_device(struct mtd_info *)`，这个函数里面获取到的 mtd_info 设备，不知道`mtd->_get_device`是在哪里定义的
   2. read 接口：根据用户传递的要读取的模式来选择使用什么接口，可以使用 `mtd_read_oob`  或者 `mtd_read` 接口
      - `kmalloc` 一块对齐的内核态内存，用于存放读到的数据
      - `mtd_read_oob` 接口就调到了 `mtd->_read_oob` 这个是只有 NAND 特有的接口
      - `mtd_read` 接口就调到了`mtd->read` ，这个接口 NOR、NAND 都有
      - 然后检查是否有 bit 跳变或者 ecc 错误
      - 把数据通过 `copy_to_user` 拷贝到用户态
   3. write 接口：根据用户传递的要写的模式来选择使用什么接口，可以使用 `mtd_write_oob` 或者 `mtd_write` 接口
      - `kmalloc` 一块对齐的内核态内存，把用户态的数据通过 `copy_from_user` 拷贝到申请的内核态内存
      - `mtd_write_oob` 接口就调到了 `mtd->write_oob` 这个是只有 NAND 特有的接口
      - `mtd_write` 接口就调到了 `mtd->write` ，这个接口 NOR、NAND 都有
      - 返回写入的长度
   4. llseek 接口：重新配一下 offset
   5. ioctl 接口：调用 `mtdchar_ioctl` 接口，将 `struct file *file, u_int cmd, u_long arg` 传下去
      - 根据 `struct file* file` 拿到 `struct mtd_info` 的信息，然后获取或者返回或者设置某些信息
      - 根据要调用的 命令，来调用 `mtd->func` 来完成功能
   6. release 接口：调用 `put_mtd_device` 把设备弹掉
      - 然后把 `struct file*file->private_data = NULL` 也置成空，把设备信息丢掉，并且`free` 掉 `strcut mtd_file_info *mfi`

![选区_144](image/选区_144.png)



### mtdblock

1. 注册 mtdblock 设备的主设备号为 31，不用次设备号，注册使用接口 `register_mtd_blktrans` 进行注册，注册结构为 `struct mtd_blktrans_ops mtdblock_tr`。
2. open 接口：根据 `struct mtd_blktrans_dev` 接口拿到 `struct  mtdblk_dev` 结构，如果 `mtdblk->count` 未被配置，则说明没有打开，创建`cache info` ，把 `mtdblk->count = 1` ，配置一些信息
3. flush 接口：调用 `write_cached_data` 接口先擦掉 block 设备，然后再把数据写到 block 设备。这里相当于是做了刷新，因为 block 设备的擦除次数有限制，所以很多时候都是把数据先读到 cache 中，在 cache 中对数据做处理，然后在合适的时候把数据写入到 block 设备。
4. release 接口：在写之前先调用 `write_cache_data` 的方式把数据刷新一遍，然后把打开的设备都做一下 `mtd_sync` 之后就 `free` 掉。
5. read 接口：调用 `do_cached_read` 来读数据，先看数据是否在 cache 中，如果在就从 cache 中拿，如果不在，就用 `mtd_read` 接口调用到 flash 下层去读数据，读到buffer
6. write 接口：调用 `do_cached_write` 来写数据到，根据要写的数据大小，来判断将数据写到 cache 还是 flash
7. add_mtd 接口：通过配置 `struct mtd_info` 结构的信息，然后把这个信息通过 `add_mtd_blktrans_dev` 的方式添加。
   - 这个接口的目的是执行 `device_add_disk` ，在里面会调用 `blk_regiser_region、register_disk`



### mtd_blkdevs

此文件会提供一些接口用来给 mtdblock 使用，包括 `register、unregister` 这些操作









## MTD 注册过程

- flash 驱动组织好 `struct mtd_info` 结构，调用 `mtd_device_parse_register` 或 `parse_mtd_partitions + add_mtd_partitions`
- 根据 cmdline 或 ofpart 解析出分区表，并且为每个分区创建一个 mtd 设备
  - 这里的 mtd 设备包括 char、block、blockro，什么时候干的？
  - 在 `add_mtd_partitions` 中会添加 mtd 分区和 mtd 字符设备
  - 在 `mtdblock` 模块注册的时候会去添加 mtd block 设备
    - 首先注册一个 mtdblock 设备
    - 遍历所有的 mtd 设备，为其创建 mtdblock 设备

### mtd_device_parse_register

```
1. 根据 flash 驱动组织好的 struct mtd_info 解析分区表
	1. 默认的分区表方式有 cmdlinepart、ofpart，优先用 cmdlinepart
	2. 遍历 cmdlinepart、ofpart
	
	/* cmdlinepart start */
	3. 寻找 cmdlinepart 的解析器，这个解析器在 drivers/mtd/cmdlinepart.c 中会注册，并且对解析器模块进行 +1 操作，防止在使用过程中被卸载
		1. 使用 __setup("mtdparts=", mtdpart_setup) 捕获 cmdline 中的 mtdparts 开始的字符串，使用 mtdpart_setup 方法解析
		2. 注册 cmdline 解析器，提供 name = cmdlinepart，parse_fn
			1. 将解析器加到 part_parsers 这个链表上
	4. 根据名称在解析器链表中找 cmdlinepart 解析器，找到了之后就执行 parser->parse_fn 回调
		1. cmdline 只解析一次，用一个 static int 的变量来保存
		2. 找到第一个 : 
		3. 创建一个 part，依次创建所有的 part，直到字符串结束
		4. 将所有的 part 放到 struct cmdline_mtd_partition *partitions 返回
		5. 根据 partitions ，继续完善 size、offset 等信息，通过二级指针的方式返回
	/* cmdlinepart end */

	/* ofpart start */
	3. 寻找 ofpart 的解析器，这个解析器在 drivers/mtd/ofpart.c 中会注册，并且对解析器模块进行 +1 操作，防止在使用过程中被卸载
		1. 注册 ofpart 解析器，提供 name = ofpart，parse_fn
			1. 将解析器加到 part_parsers 这个链表上
		2. 注册 ofoldpart 解析器，提供 name = ofoldpart，parse_fn
	4. 根据名称在解析器链表中找 ofpart 解析器，找到了之后就执行 parser->parse_fn 回调
		1. 找到 mtd_node
		2. 找到名称为 partitions 的节点
		...
	5. 对解析器模块进行 -1 操作，可以被卸载
	/* ofpart end */
	
2. 根据 cmdline 解析好的分区，增加 mtd 分区
	1. 依次创建每个 mtd 分区
	2. 向内核申请一块空间用于存放 partition
		1. 根据 struct mtd_info *master 的信息(flash 驱动组织好的)填充 partition
		2. 返回申请出来的 partition
		3. 将 partition 添加到 mtd_partitions 链表
		4. 添加 mtd 设备
			1. 在 mtd class 下面创建 device(device_register、device_create 这套)
		5. 添加 mtd 设备的描述信息
			1. 在 sysfs 中创建描述信息
```

