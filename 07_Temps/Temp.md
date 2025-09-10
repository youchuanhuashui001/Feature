---

mindmap-plugin: basic

---

# start_kernel

## rest_init
- kernel_init
    - kernel_init_freeble
        - do_basic_setup
            - do_initcalls()
                - arch_init()
                - module_init()
                    - dw_spi_v2_probe()
                - xxx_init()
                - __setup("root=", root_dev_setup)
                    - 从 cmdline 中取到 root=xxxxx --> saved_root_name
                        - 还是在 do_initcall_level 中执行的
        - sys_open("/dev/console")
        - ramdisk = "/init"
        - if (sys_access(ramdisk)
            - 尝试从  initramfs/initrd 目录下找到 init，如果找得到就用这个init脚本，如果找不到就自行挂载根文件系统
        - prepare_namespace()
            - if(saved_root_name[0)   root_device_name = saved_root_name
                - 这里的的 saved_root_name 来自 __setup("root", root_dev_setup) 取到的字符串
            - 找到了 mtd 或 ubi 字符串
                - mount_block_root(root_device_name, root_mountflags)
                    - 这里和下面不同的地方时，name 是 mtdblockx，而下面的 name 是 /dev/root
                    - 由于 bootargs 中只指定了 root=/dev/mtdblockx，而没有指定 rootfstype，所以会在 get_fs_names 中遍历所有支持的 fstype
                    - 调用 sys_mount 来进行根文件系统的 mount
                        - sys_mount 是一个系统调用会到 fs/namespace.c  SYSCALL_DEFINE5(mount, xxx)
                        - type、name、data --> do_mount(kernel_name, dir_name, kernel_type, flags, options)
                            - user_patch(dir_name, &path) 从 dir_name 中找到 path？还是从 path 中找到 dir_name？
                            - security_sb_mount(dev_name, &patch, type_page, flags, data_page)
                                - 检查安全性
                            - 。。。根据 flag，进行其它的挂载
                            - do_new_mount   常规挂载
                                - get_fs_type
                                - vfs_kern_mount
                                    - mount_fs
                                        - type->mount
                                - do_add_mount
                                    - 确认下是否挂载成功
            - initrd_load()
                - create_dev("/dev/ram", Root_RAM0)
                    - 如果用的是 initrd ，这里会创建 /dev/ram
                    -
                - rd_load_image("/initrd.image")
                    - 从 initrd 中读 initrd.image
            - if (is_floppy && rd_doload && rd_load_disk(0))
                 ROOT_DEV = Root_RAM0
            - mount_root()
                - 如果定义了 CONFIG_ROOT_NFS，会调用 mount_nfs_root() 来将 nfs 作为 rootfs
                - rd_load_disk(1) 从 ramdisk 挂载根文件系统
                - mount_block_root 从 mtdblock 挂载根文件系统，会尝试支持的所有根文件系统
                    - __setup("rootfstype=", fs_name_setup)
                    - get_fs_names(fs_names)
                        - 将所有的 rootfstype 拷贝到 char *page 保存下来
                    - for 循环调用 do_mount_root()，尝试挂载支持的所有文件系统
                    - 如果挂载成功，会退出循环；如果挂载失败会打印所有的分区，然后调用 panic 来产生一个 kernel_panic
                        - panic 会关掉中断，然后打印。。。一系列的事情，然后吧中断打开,创建一个 soft_watchdog 然后一直循环 delay

## squashfs
- fs/squashfs/super.c
    static struct file_system_type squashfs_fs_type = {
    .owner = THIS_MODULE,
    .name = "squashfs",
    .mount = squashfs_mount,
    .kill_sb = kill_block_super,
    .fs_flags = FS_REQUIRES_DEV
    };
- squashfs_fill_super 将 fs 需要用到的信息从 mtdblock 中读到 super
- fs/squashfs/file.c
    - 数据在 buffer 中， squashfs/file_cache.c
         squashfs_readpage_block 从 buffer 中取
    - 数据不在 buffer 中，
        suqashfs/file_direct.c
        - suqashfs_read_data
            - ll_rw_block(REQ_OP_READ, 0, b, bh)
                - submit_bh(op, op_flags, bh)
                    - submit_bh_wbc
                        - submit_bio
                            - generic_make_request
                                - add_mtd_blktrans_dev
                                    - INIT_WORK(&new->work, mtd_blktrans_work)
                                        - static struct mtd_blktrans_ops mtdblock_tr = {
                                             .name = "mtdblock",
                                             .major = MTD_BLOCK_MAJOR,
                                             .part_bits = 0,
                                             .blksize = 512,
                                             .open = mtdblock_open,
                                             .flush = mtdblock_flush,
                                             .release = mtdblock_release,
                                             .readsect = mtdblock_readsect,
                                             .writesect = mtdblock_writesect,
                                             .add_mtd = mtdblock_add_mtd,
                                             .remove_dev = mtdblock_remove_dev,
                                             .owner = THIS_MODULE,
                                            };
                                        - do_blktrans_request(dev->tr, dev, req);
                                            - 根据 req 判断是要进行什么操作，进而调到 mtdblock.c 中对应的回调，例如是读操作：tr->readsect(dev, block, buf);
                                                或者写操作：tr->writesect(dev, block, buf);
                                    - new->rq = blk_init_queue(mtd_blktrans_request, &new->queue_lock)
                                        - queue_work(dev->wq, &dev->work) 唤醒上面初始化的 work
                                - q->make_request_fn

## dma_map_single
dma_unmap_single
- arch/arm/mm/dma-mapping.c
   struct dma_map_ops arm_dma_ops; 非一致性
   struct dma_map_ops arm_coherent_dma_ops; 一致性
   struct dma_map_ops arm_iommu_dma_ops; 带 iommu 非一致性
   struct dma_map_ops arm_iommu_coherent_dma_ops; 带 iommu 非一致性
   (这样可以让上层用户不用关心下面到底有没有一致性、带不带 iommu，直接调统一的接口就行了)
- driver/of/device.c
   设备树中是否有定义 dma-coherent、iommu
   arch_setup_dma_ops(dev, dma_addr, size, iommu, coherent)
- arm_dma_map_page
    - __dma_page_cpu_to_dev
        - dir = DMA_FROM_DEVICE;   会 invalidate range cache
           dir = 其它的，会 clean range cache
    - return pfn_to_dma(dev, page_to_pfn(page)) + offset;
- arm_dma_unmap_page
    - __dma_page_dev_to_cpu
        - phys_addr_t paddr = page_to_phys(page) + off;
        - 如果不是写外设，会先 invalidate cache