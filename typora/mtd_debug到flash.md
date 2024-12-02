# mtd_debug 到 flash 驱动流程分析



# 概述

- flash 驱动中根据 `mtd` 子系统规则来创建`mtd`设备，组建`mtd->read、mtd->write、mtd->erase`等函数
- 文件系统通过访问 `flash`驱动创建的`mtd`设备，调用`mtd->read、write、erase`接口来访问`flash`
- 用户使用 `mtd_debug`工具作为Linux的应用层，访问mtd设备通过`VFS`虚拟文件系统层，`VFS`来根据当前的文件系统类型找到具体的文件系统，使用具体的文件系统接口来操作`flash`





# 流程分析



## gxloader：

- gxloader 中可以不需要特定文件系统的驱动，即使包含了文件系统的驱动，也会指定它操作的驱动与flash驱动的对应接口
- gxloader 相当于需要提供烧写kernel，rootfs到flash的功能
- gxloader 需要提供从 flash 驱动，传递命令行到kernel



## kernel：

- 提供 flash 的驱动，驱动中需要构建`mtd`设备，提供`mtd->read、mtd->write、mtd->erase`函数等

  ```c
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
  ```

- 提供解析`cmdline`功能

  ```c
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
  ```

- 提供文件系统的驱动，驱动需要能够通过访问文件系统的接口访问到`mtd`设备，从而完成flash 的读写擦操作

  ```c
  int jffs2_flash_read(struct jffs2_sb_info *c, loff_t ofs, size_t len, size_t *retlen, u_char *buf)
  {
  	loff_t	orbf = 0, owbf = 0, lwbf = 0;
  	int	ret;
  
  	if (!jffs2_is_writebuffered(c))
  		return mtd_read(c->mtd, ofs, len, retlen, buf);
  
  	/* Read flash */
  	down_read(&c->wbuf_sem);
  	ret = mtd_read(c->mtd, ofs, len, retlen, buf);
  	.....
  }
  
  
  /* 调到flash中的驱动*/
  int mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen,
  	     u_char *buf)
  {
  	int ret_code;
  	*retlen = 0;
  	if (from < 0 || from >= mtd->size || len > mtd->size - from)
  		return -EINVAL;
  	if (!len)
  		return 0;
  
  	ledtrig_mtd_activity();
  	/*
  	 * In the absence of an error, drivers return a non-negative integer
  	 * representing the maximum number of bitflips that were corrected on
  	 * any one ecc region (if applicable; zero otherwise).
  	 */
  	ret_code = mtd->_read(mtd, from, len, retlen, buf);
  	if (unlikely(ret_code < 0))
  		return ret_code;
  	if (mtd->ecc_strength == 0)
  		return 0;	/* device lacks ecc */
  	return ret_code >= mtd->bitflip_threshold ? -EUCLEAN : 0;
  }
  ```

  



## rootfs：

* 根文件系统：`busybox`编译生成的文件夹，使用`mkfs.jffs2`工具将文件夹

  ```
  mkfs.jffs2 是用于创建 JFFS2（Journalling Flash File System 2）文件系统的命令。它将指定的目录中的所有文件和子目录打包为一个 JFFS2 文件系统映像文件，并且可以添加各种选项来指定文件系统的特性。
  
  具体来说，mkfs.jffs2 命令执行以下步骤：
  
  1. 分析指定目录中的所有文件和子目录，并为每个文件和目录创建相应的节点（inode）和文件名缩写（name abbreviation）。
  
  2. 根据指定的选项设置和目录中的内容，构建 JFFS2 文件系统的元数据结构，包括超级块、节点垃圾回收信息、文件系统大小和块大小等。
  
  3. 将元数据和数据写入 JFFS2 文件系统映像文件中，同时使用 JFFS2 文件系统的特殊算法来压缩数据并减少闪存中的空间占用。
  
  4. 在文件系统映像文件中为每个文件和目录创建相应的元数据和数据节点，并写入到闪存中。
  
  总的来说，mkfs.jffs2 命令将目录中的文件和子目录打包成一个可用于闪存的文件系统映像文件，并在闪存上创建了相应的文件和目录节点，以便于在运行时进行读写操作。它是嵌入式系统中常用的文件系统格式之一，因为它支持闪存的固态特性，可以提供更好的性能和可靠性。
  ```



- linux 数据区使用 `jffs2`

  - linux ：

    ```diff
    --- a/arch/arm/configs/canopus_defconfig
    +++ b/arch/arm/configs/canopus_defconfig
    @@ -154,6 +154,7 @@ CONFIG_FAT_DEFAULT_UTF8=y
     CONFIG_NTFS_FS=y
     CONFIG_TMPFS=y
     CONFIG_CONFIGFS_FS=y
    +CONFIG_JFFS2_FS=y
     CONFIG_CRAMFS=y
    ```

  - flash.conf：

    ```
    BOOT    loader-sflash.bin       false   RAW       ro      0      0        0x000000  auto
    TABLE   NULL                    true    RAW       ro      2      1        0x020000  64k
    LOGO    logo.bin                false   RAW       ro      0      0        0x030000  64k
    KERNEL  uImage                  true    RAW       ro      1      1        0x040000  auto
    ROOTFS  rootfs.img              true    SQUASHFS  ro      1      1        auto      auto
    #ROOTFS  rootfs.img              true    JFFS2     rw      1      1        auto      auto
    APP     app.bin                 true    SQUASHFS  ro      1      1        auto      auto
    DATA    NULL                    false   JFFS2     rw      0      0       0xDC0000      auto
    ```

  - rootfs：

    ```shell
        DATA="/dev/mtdblock"`cat /proc/mtd | busybox grep -i data | cut -d ":" -f 1 | tr -cd "[0-9]"`
        # 此处的 minifs 换成 jffs2
        mount -t jffs2 $DATA /home/gx
    
    ```

    



## mtd_utils：











# Q&A



- Q：gxloader中编译debug版本时的`cmdline`传递的是使用nfs方式，那么在什么时候修改的这个`cmdline`，是在创建分区表的时候吗？

  ```
  mem=68M@0x00000000 videomem=60M@0x04400000 mem_end console=ttyS0,115200 init=/init root=/dev/mtdblock4 mtdparts=m25p80:128k@0m(BOOT),64k@128k(TABLE),64k@192k(LOGO),2560k@256k(KERNEL),3840k@2816k(ROOTFS),7424k@6656k(APP),2304k@14080k(DATA) mtdpart
  ```

- A：



- Q：使用 `netdown`的时候经常卡住，打印`retry xxx times`，是网络问题还是什么问题？
- A：



- Q：分区表是什么时候读的，从哪里直到的分区表的地址？



- Q：gxloader 中文件系统的驱动是用来干什么的？使用.boot 将整bin下载到flash，这个操作也不需要用到文件系统的驱动
- A：gxloader 中的文件系统驱动也都是通过调用 flash 的驱动，如果需要用 `mtd` 的方式的话，会在loader中构建`mtd`设备及`mtd->function`



- Q：flash 的 linux 驱动中除了构建 `mtd`设备，还做了两个`opt，nor misc`设备，这两个有什么用，以及怎么用？
- A：用来访问otp的功能，和`nor`的相关功能，包括获取各种info，使用方法是在启动linux 之后通过应用程序访问`misc`设备来进行操作