# Linux 编译烧写



## 1. 所需文件

- bootloader：gxloader 编译生成 .boot + .bin
- kernel：Linux  Kernel 编译生成 uImage + .dtb
- rootfs.bin：busybox 编译生成的 rootfs 使用 mkcramfs 制作 rootfs.bin
- table.bin：genflash 工具根据 flash.conf 生成 table.bin
- app.bin：应用程序(可选)
- all.bin：包含上述 loader.bin + kernel.bin + rootfs.bin + table.bin 的整bin文件
- 烧录命令：`sudo boot -b xxx.boot -c serialdown all.bin`



## 2. 编译流程

### 2.1 编译 bootloader

​		目标：编译 gxloader 生成 .boot 和 .bin

1. gxmisc 编译新的.a 提供给 gxloader (platform/gxmisc)

   ```bash
   ./build arm gxloader
   ```

2. 编译gxloader 生成 .boot 和 .bin (platform/gxloader)

   - 修改 conf/sirius/6633H1/debug.config 中的 CMD_LINE

     ```diff
     --- a/conf/sirius/6633H1/debug.config
     +++ b/conf/sirius/6633H1/debug.config
     @@ -35,8 +35,8 @@ USB_RESISTANCE = 0
      USB_EYE_DIAGRAM_TEST = n
      
      # cmdline config, if CMDLINE_VALUE = "" will not ENABLE_CMDLINE
     -#CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init"
     -CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init root=/dev/nfs rw nfsroot=192.168.101.200:/opt/nfs,v3 ip=192.168.101.3"
     +CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init rw"
     +#CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init root=/dev/nfs rw nfsroot=192.168.101.200:/opt/nfs,v3 ip=192.168.101.3"
      #CMDLINE_VALUE = "mem=163M videomem=93M mem_end console=ttyS0,115200 init=/init root=/dev/nfs rw nfsroot=192.168.101.200:/opt/nfs,v3 ip=192.168.101.3"
     ```

   - 编译生成.boot

     ```bash
     ./build sirius 6633H1 boot
     cp output/sirius-6633H1-sflash.boot ../.boot	# 拷贝备用
     ```

   - 编译生成.bin

     ```bash
     ./build siruis 6633H1 debug
     cp output/loader-sflash.bin ../.boot		# 拷贝备用
     ```



### 2.2 编译 kernel

​		目标：编译 Linux Kernel 生成 uImag 和 .dtb

1. 编译 Linux Kernel 提供头文件给 gxmisc 使用 (buildroot/kernel/4.9.y)

   ```bash
   ./build sirius
   ```

2. 编译 gxmisc 提供库文件给 Linux Kernel(platform/gxmisc)

   ```bash
   ./build arm linux
   ```

3. 编译 Linux Kernel 生成 uImage 和 .dtb

   ```bash
   ./build sirius
   ```

4. 将生成的 uImage 和 .dtb 拷贝备用

   ```bash
   cp /opt/goxceed/kernel-arm/arch/arm/boot/uImage xxx/.boot	# 拷贝备用
   cp /opt/goxceed/kernel-arm/arch/arm/boot/dts/sirius_generic.dtb xxx/.boot	# 拷贝备用
   ```

   

### 2.3 编译 rootfs

​		目标：编译 busybox 生成 rootfs，并使用 mkcramfs 制作 rootfs.bin

1. 编译 busybox 生成根文件系统，生成的目录在`_install`  (目前编译的 rootfs 无法启动，显示缺少 init，使用 make.sh 编译出来的 rootfs 目录)

   ```bash
   ./build arm-linux-uclibcgnueabihf-
   ```

2. 在编译出的目录下创建需要的目录，删除 linuxrc 

   ```bash
   mkdir home
   rm linuxrc
   ```

3. 新建文件 init(由于 bootloader 中的 cmdline 参数为：init = /init，此处文件名也应为 init)，并添加以下内容：

   ```
   !/bin/sh                                                                                                                                                                                                                       
   
   echo "Welcome to Linux."
   
   mount -t proc proc /proc
   mount -t sysfs sysfs /sys
   mount -t tmpfs mdev /dev
   mkdir -p /dev/pts
   mount -t devpts devpts /dev/pts
   mount -t tmpfs tmpfs /tmp
   mount -t tmpfs tmpfs /media
   mount -t tmpfs tmpfs /var
   mkdir -p /var/lock
   mkdir -p /var/run
   
   mknod /dev/console c 5 1 
   mknod /dev/ptmx c 5 2 
   mknod /dev/null c 1 3 
   
   echo "2" > /proc/cpu/alignment 2
   exec /sbin/init </dev/console >/dev/console 2>/dev/console
   ```

4. 此时将显示 缺少 rcS 文件，继续新增 `etc/init.d/rcS` 文件

   ```
   #!/bin/sh                                                                                                                                                                                                                       
   PATH=$PATH:/usr/local/bin
   PATH=$PATH:/usr/local/sbin
   
   # Run preset scripts
   echo ""
   echo "###preS.d start###"
   if test -d /etc/preS.d/; then
           for RCS in /etc/preS.d/S*; do
                   if test -r "$RCS"; then
                           . "$RCS" start
                   fi
           done
           unset RCS
   fi
   
   echo ""
   echo "###preS.d ended###"
   cat /proc/uptime
   echo "###"
   
   # Run system scripts
   echo ""
   echo "###rcS.d start###"
   if test -d /etc/rcS.d/; then
           for RCS in /etc/rcS.d/S*; do
                   if test -r "$RCS"; then
                           . "$RCS" start
                   fi
           done
           unset RCS
   fi
   
   echo ""
   echo "###rcS.d ended###"
   cat /proc/uptime
   echo "###"
   
   # Run application scripts
   if test -d /dvb/etc/rcS.d/; then
           for RCS in /dvb/etc/rcS.d/S*; do
                   if test -r "$RCS"; then
                           . "$RCS" start
                   fi
           done
           unset RCS
   fi
   ```

   

5. 使用 mkfs.squash 工具将 `rootdir-arm`下的根文件系统生成 rootfs.bin

   ```bash
   ../../tools/mkfs.squash rootdir-arm rootfs.bin -all-root -noappend -no-duplicates
   cp rootfs.bin xxx/.boot 	# 拷贝备用
   ```



### 2.4 编译 table.bin

​		目标：使用 genflash 工具根据 flash.conf 文件编译生成 table.bin(生成的整bin 中已包含 table.bin)

1. 拷贝 flash.conf 文件到.boot 目录

   ```bash
   cp flash.conf xxx/.boot
   ```

2. 配置 flash.conf，取消 logo.bin 和 app.bin

   ```
   flash_size    0x1000000                                                                                                                                                                                                         
   #flash_size    0x1000000
   block_size    0x010000
   #write_protect true
   crc32         true
   dtb_file      sirius_generic.dtb
   
   # 1) NAME field:
   #       "BOOT"   : gxloader
   #       "TABLE"  : partition table
   #       "LOGO"   : gxloader show logo
   #       "KERNEL" : application program
   #       "ROOT"   : root file system
   #
   # 2) The FS field have these types:
   #           "RAW": self definition file system type;
   #           "CRAMFS":  cram_file_system type;
   #           "MINIFS":  mini_file_system type;
   # 3) The partition have 2 mode "ro" and "rw":
   #           "ro": means that this partition could not been modified at runtime;
   #           "rw": means that this partition could modify at runtime
   # 4) UPDATE:
   #       0: don't update
   #       1: always update
   #       2: auto update, while version > old version
   #       3: clear partition data
   #
   # 5) VERSION 0 - 65535 (0x0000 - 0xFFFF)
   # 6) The size of "auto" means the size of partition is determined by source file size, but for
   #    the final partition (example "DATA") it means the size is last to the end of flash.
   #
   ## NOTE:
   #
   # 1) The size of source file could not exceed the max size of partition.
   # 2) The start addr of "BOOT" and "LOGO" should never be modified whenever you are
   #    modifying this file (configure file) or running a program.
   # 3) The 1K bytes blank befor the "LOGO" partition is reserved for partition table,
   #    so do not use these space.
   # 4)
   #
   #KERNEL  uImage                  true    RAW       ro      1      1        0x040000  4096k
   #ROOTFS  rootfs.bin              true    SQUASHFS  ro      1      1        auto      5120k
   # NAME  FILE                    CRC     FS      MODE    UPDATE VERSION  ADDRESS   SIZE
   #-------------------------------------------------------------------------------------------
   BOOT    loader-sflash.bin       false   RAW       ro      0      0        0x000000  auto
   TABLE   NULL                    true    RAW       ro      2      1        0x020000  64k
   #LOGO    logo.bin                false   RAW       ro      0      0        0x030000  64k
   KERNEL  uImage                  true    RAW       ro      1      1        0x040000  auto
   ROOTFS  rootfs.bin              true    SQUASHFS  ro      1      1        auto      auto
   #APP     app.bin                 true    SQUASHFS  ro      1      1        auto      auto
   DATA    NULL                    false   MINIFS     rw      0      0       0xDC0000      auto
   ```

4. 使用 genflash 工具根据 flash.conf 生成整 bin(solution/tools/genflash)

   ```bash
   ./genflash mkflash ../../../platform/.boot/flash.conf ../../../platform/.boot/all.bin
   ```



## 3. 烧录

​		烧录有两种方式：使用 boot 工具的 serialdown 命令烧录整bin 或者 使用 boot 工具的 load_conf_down 命令烧录：

1. 使用 serialdown 命令下载整 bin：

   ```bash
   boot -b xxx.boot -c serialdown 0x0 all.bin
   ```

2. 使用 load_conf_down 命令按照 flash.conf 烧录：

   ```bash
   sudo boot -b xxx.boot -c load_conf_down flash.conf serialdown
   ```

   



## Q && A

1. 使用 boot 工具 load_conf_down 命令下载时：是只下载分区中描述的 loader-sflash.bin 、 logo.bin 、 uImage 、 rootfs.bin 、 app.bin 吗？
   - 分区描述的是 uImage，但是在编译 Linux Kernel 时是将 uImage 和 .dtb 合并成 kernel.bin 了，这里下载的 uImage 是指什么? --> 由于老版的 flash.conf 描述是只有 kernel.bin ，没有.dtb的所以可以这样合并下载
   - 我理解的是应该把  uImage 和 .dtb 直接拷贝过来，名称和 flash.conf 中描述的一样，使用 load_conf_down 下载的时候就会根据 flash.conf 下载

2. goxceed 文档中描述将 Linux Kernel 编译生成的 uImage 和 .dtb 合成 kernel.img，而 flash.conf 中描述的是 uImage 和 xxx.dtb?
   1. 文档中描述的是老的 flash.conf ，上文提供的是新的 flash.conf
   2. genflash 工具只是解析 flash.conf 中的描述信息，用户可以手动修改 flash.conf 来实现是使用 kernel.img 或 使用 uImage 和 xxx.dtb，只需将 flash.conf 描述的文件和要下载的文件保持一直即可

3. 怎么测驱动？在烧写进去之后使用 lsmod 并没有看到 flash 相关的驱动？要手动加载吗？
4. misc 设备的测试应用程序放在哪里？

5. 文件系统`Jffs2、ubifs、Yaffs`这些使用工具就能够将 busybox 生成的目录转换成对应的文件系统，那么是不是这些文件系统也要对接 flash 的操作接口？如 Jffs2 的读操作实质也是要调用 gxmisc 中提供的 read接口

6. Linux Nor Flash 驱动中添加分区信息，分区信息是由 gxloader CMD_LINE 传递过来的。
   1. 默认配置的 CMD_LINE 会在后续进行填充
   2. boot.c ：中会读取 mtdparts 信息，并将其添加到 CMD_LINE
7. 使用 gxloader：board-init.c 中的分区信息和使用 genflash 工具生成的 table.bin 有什么区别？
   1. board-init.c 是自测试使用的
   2. genflash 工具是 loader 去到存储 table.bin 的地址去读取分区信息
   3. table.bin 可能在每个版本中所存放在 flash 上的地址都不一样，大小也可能不一样

