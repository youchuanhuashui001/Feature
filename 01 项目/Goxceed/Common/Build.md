# Linux 4.9.y 



## 平台环境：

- 硬件：sirius 6633H1 socket + SPI NAND

- 软件：v1.9.8-7-rc3



## 所需文件：

- `bootloader`：`gxloader` 编译生成 `.boot + .bin`

  - `.boot `用来引导下载
  - `.bin` 用来组成整bin下载到开发板

- `kernel`：Linux  Kernel 编译生成 `uImage + .dtb`

  - 将 `uImage + .dtb` 组合成 `Kernel.img` 后用来组成整 bin 下载到开发板

- `rootfs.bin`：`busybox` 编译生成的 `rootfs` 使用 `mkcramfs` 制作 `rootfs.bin`

- `flash.con`：分区表，用来描述各个文件组成整 bin 时的地址、大小等信息

  - `Nor Flash` 使用 `flash.conf`
  - `Nand Flash` 使用 `nand_flash.conf`

- `table.bin`：描述的是 TABLE 分区的数据，已经包含在整 bin 中

  - `genflash` 工具根据 `flash.conf` 生成 `table.bin`

- `app.bin`：应用程序(可选)

- `all.bin`：包含上述 `loader.bin + kernel.bin + rootfs.bin + table.bin `的整bin文件

  - 使用 `genflash`工具根据 `flash.conf` 组成整 bin 文件

- 烧录命令：

  ```
  boot -b sirius-6613H1-sflash.boot -c serialdown 0x0 flash.bin
  boot -b sirius-6613H1-sflash.boot -c usbdown 0x0 flash.bin
  boot -b sirius-6613H1-sflash.boot -c netdown 0x0 flash.bin -p ubuntu_ip -s board_ip
  ```



## 编译流程：

### Gxloader 编译：

* 修改`conf/sirius/6633H1/boot.conf`

  ```diff
  -ENABLE_SPIFLASH = y
  -ENABLE_SPINAND = n
  +ENABLE_SPIFLASH = n
  +ENABLE_SPINAND = y
  ```

* 修改 `conf/sirius/6633H1/debug.conf`，默认不使用 nfs 作为根文件系统

  ```diff
  -ENABLE_SPIFLASH = y
  -ENABLE_SPINAND = n
  +ENABLE_SPIFLASH = n
  +ENABLE_SPINAND = y
  
  
  -#CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init"
  -CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init root=/dev/nfs rw nfsroot=192.168.101.200:/opt/nfs,v3 ip=192.168.101.3"
  +CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init rw"
  +#CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init root=/dev/nfs rw nfsroot=192.168.101.200:/opt/nfs,v3 ip=192.168.101.3"
  ```

* 编译.boot、.bin

  ```
  ./build sirius 6633H1 boot
  
  ./build sirius 6633H1 debug
  ```



### Linux 4.9 编译：

- 修改文件`arch/arm/configs/sirius_none_debug_defconfig`，默认为 NOR Flash，此处修改为 Nand Flash

  ```diff
  -CONFIG_MTD_SPI_NOR=y
  -CONFIG_MTD_MT81xx_NOR=y
  -CONFIG_SPI_CADENCE_QUADSPI=y
  +CONFIG_MTD_SPINAND=y
  +CONFIG_MTD_UBI=y
  ```

- 编译 Linux：

  ```
  ./build sirius
  ```

- 编译生成文件包括 sirius_generic.dtb，zImage, uImage。默认编译生成的文件路径放在 /opt/goxceed/kernel-arm/目录下。三个文件的路径分别是：

  ```
  /opt/goxceed/kernel-arm/arch/arm/boot/dts/sirius_generic.dtb
  /opt/goxceed/kernel-arm/arch/arm/boot/zImage
  /opt/goxceed/kernel-arm/arch/arm/boot/uImage
  ```

- sirius_generic.dtb 是设备树的镜像文件。

- zImage 是压缩过的内核镜像。

- uImage 是在 zImage的头前面加了一段头信息描述镜像的长度，压缩方式，内核的加载和运行地址等信息。

- 最后把dtb文件和uImage文件合成一个文件（可以用 zImage，但是建议用 uImage ）kernel.img

- 合成 `kernel.img`

  ```
   cat /opt/goxceed/kernel-arm/arch/arm/boot/dts/sirius_generic.dtb > kernel.img
   cat /opt/goxceed/kernel-arm/arch/arm/boot/uImage >> kernel.img
  ```

- 新版本合成 `kernel.img`，内核相关目录放在 ../../../library

  ```shell
  cat ../../../library/goxceed/kernel-arm/arch/arm/boot/dts/canopus_generic.dtb > kernel.img
  cat ../../../library/goxceed/kernel-arm/arch/arm/boot/uImage >> kernel.img
  ```

  

### 根文件系统编译：

- 编译 sirius 的根文件系统：

  ```
  ./build sirius 4.9.y
  ```

- 使用 squash 命令将根文件系统制作出 rootfs.img

  ```
  mksquashfs sirius_rootfs rootfs.img
  ```

  

### 分区表和整 bin 构建

- 到这一步，我们已经有了如下 5 个文件

  ```
  sirius-6613H1-sflash.boot
  
  loader-sflash.bin
  
  kernel.img
  
  app.img
  
  rootfs.img
  ```

- 当前除了.boot，另外 4 个文件需要下载到 Flash 里，把 4 个文件要按分区表的方式进行组织

- 使用 nand_flash.conf 文件：可能要修改 kernel 的size

  ```
  flash_size    0x1000000
  block_size    0x020000
  #write_protect true
  crc32         true
  #zlibmode      true
  #key_file      gx-rsa2048-keypair.pem
  table_version 2  
  
  # readme
  # 1)zlibmode: if true, the output binfile format is [8 bytes magic("zlibmode") + table + [zlibmode part1] + [zlibmode part2] + ... ]
  #             zlibmode part format is [partition name + compress size + zlib compress partition data bin]
  #
  # 2)key_file: indicate the private key for signature
  # 3)table_version: 
  #       1:old version
  #       2:add reserved size, table size is expanded to 1024B
  #
  
  # 1) NAME field:
  #	"BOOT"   : gxloader
  #	"TABLE"  : partition table
  #	"LOGO"   : gxloader show logo
  #	"KERNEL" : application program
  #	"ROOT"   : root file system
  #
  # 2) The FS field have these types:
  #           "RAW": self definition file system type;
  #           "CRAMFS":  cram_file_system type;
  #           "MINIFS":  mini_file_system type;
  # 3) The partition have 2 mode "ro" and "rw":
  #           "ro": means that this partition could not been modified at runtime;
  #           "rw": means that this partition could modify at runtime
  # 4) UPDATE:
  # 	0: don't update
  # 	1: always update
  # 	2: auto update, while version > old version
  # 	3: clear partition data
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
  # 4) extend_info:0 not enable; 1 enable crc; 2 enable signature; 3 enable crc&signature. crc/signature only
  #    valid in "ro" MODE partition.
  # 5)
  #
  # NAME	FILE                    CRC     FS    MODE UPDATE VERSION ADDRESS  TOTAL_SIZE  EXTEND RES_SIZE
  #----------------------------------------------------------------------------------------------------------
  BOOT    loader-spinand.bin      true	RAW    ro    0     0     0x000000  0x080000   0      0x20000
  TABLE   NULL                    true	RAW    ro    2     1     0x080000  0x080000   0      0x20000
  KERNEL  kernel.img              true	RAW    ro    1     1     0x100000  0x400000   0      0x60000
  ROOT    rootfs.img              false	YAFFS2 rw    1     1     0x500000  0x700000   0      0x100000
  DATA    NULL                    false	YAFFS2 rw    3     63    0xc00000  0x500000   0      0x80000
  ```



* 使用 genflash 工具根据 nand_flash.conf 文件来构建 flash.bin

  ```
  genflash mkflash nand_flash.conf flash.bin
  ```

  



## 镜像烧写：

- 镜像烧写使用 boot 工具配合 前面编译出来的.boot 使用。支持 串口、USB和NET烧写。

```
boot -b sirius-6613H1-sflash.boot -c serialdown 0x0 flash.bin
boot -b sirius-6613H1-sflash.boot -c usbdown 0x0 flash.bin
boot -b sirius-6613H1-sflash.boot -c netdown 0x0 flash.bin -p ubuntu_ip -s board_ip
```



- **serialdown**

  - 下载download.bin到flash的0x0地址处

    ```
    boot -b gx3211.boot -c serialdown 0x0 download.bin
    ```

- **usbdown**

  - 下载download.bin到flash的0x0地址处，download.bin位于U盘根目录

    ```
    boot -b gx3211.boot -c usbdown 0x0 download.bin
    ```

- **serialdump**

  - 将flash里的数据读出到文件dump.file，flash数据起始地址为0x0，数据长度为0x100000

    ```
    boot -b gx3211.boot -c serialdump 0x0 0x100000 dump.file
    ```

- **flash**

  - 将flash上地址为0x0长度为0x100的数据擦除，实际上将会以64KB为单位进行擦除

    ```
    boot -b gx3211.boot -c flash erase 0x0 0x100
    ```

- **load_conf_down**

  此命令一般用于 FLASH 芯片是 NAND 的情况。因为 NAND Flash 的容量很大，比如有的容量是 256  MBytes，但是内存总的也只有 256 MBytes， 在升级 256 MBytes 的整 bin  时，会因为内存不够大，无法升级，所以需要把整 bin 拆包下载，实际做法是boot 读取 flash.conf 文件，按照 flash.conf  读取各个分区的数据下载。

  使用load_conf_down下载文件前必须设置 flash.config 中每个分区的 UPDATE 标记，UPDATE 标记大于 0 表示升级该分区， UPDATE 标记为 0 表示不对该分区做任何操作，所以如果要升级 flash.conf 中所有分区需要把每个分区的  UPDATE 标记改为 1。用 UPDATE 标记的目的是在分区过多的情况下，有时候需要更新其中几个分区，这样可以通过更改不需要升级的分区的  UPDATE 标记为 0， 更改需要升级的分区的 UPDATE 标记为 1 来批量升级分区。

  - 根据flash.config文件用netdown下载数据

    ```
    boot -b gx3211.boot -c load_conf_down ./flash.config netdown -p pc_ip -s board_ip
    ```

  - 根据flash.config文件用usbdown下载数据，下载文件存放在usb中的usb_path目录下

    ```
    boot -b gx3211.boot -c load_conf_down ./flash.config usbdown usb_path
    ```

- **gx_otp**

  - 读取otp数据到终端，起始地址为0x310，长度为10

    ```
    boot -b gx3211.boot -c gx_otp tread 0x310 10
    ```

  - 从终端写数据到otp中，起始地址为0x310，十六进制数据为112233445566778899，注意写入的数据前不用加”0x”

    ```
    boot -b gx3211.boot -c gx_otp twrite 0x310 112233445566778899 
    ```

  - 将file文件中的数据写入otp地址0x310开始处

    ```
    boot -b gx3211.boot -c gx_otp write 0x310 file
    ```

  - 将otp中数据读到文件中file中，起始地址为0x310，数据长度为10

    ```
    boot -b gx3211.boot -c gx_otp read 0x310 10 file
    ```

- **compare**

  - 比较src.file和dst.file的内容是否相同

    ```
    boot -b gx3211.boot -c compare src.file dst.file
    ```

- **连续命令**

  - 先将flash上数据全部擦除，然后将download.bin下载到flash起始地址处，再将flash上地址为0x0，长度为65536的数据读到文件dump.file中，最后比较download.bin和dump.file是否相同

    ```
    boot -m gx3201 -c "flash eraseall;serialdown 0 download.bin;serialdump 0 65536 dump.file;compare download.bin dump.file" 
    ```

- **常用命令**

  - 出现boot>界面时，输入help可以查看常用命令，如flash的读写擦等。











## 测试程序执行：

- 由于采用的非 NFS 做根文件系统，所以需要手动挂载 NFS 来加载 `mtd_untils` 工具



### 开发板挂载 NFS

- NFS 主机环境搭建：

  - nfs server 安装

  ```
  sudo apt-get install nfs-kernel-server
  ```

  - nfs 共享目录配置，在/etc/exports文件中加入共享目录和属性信息

  ```
  sudo vim /etc/exports
  
  /opt/nfs *(rw,sync,no_root_squash)
  ```

  - 重启nfs 服务

  ```
  sudo service nfs-kernel-server restart
  ```

  - 测试nfs复位挂载是否成功

  ```
  mount -t nfs -o nolock 192.168.108.111:/opt/nfs /mnt
  ```

- 开发板配置 eth0

  ```
  ifconfig eth0 up
  ifconfig eth0 192.168.108.10
  ```

- 挂载 NFS：

  ```
  mount -t nfs -o nolock ubuntu_ip:nfs_path /mnt
  ```



- Q：执行挂载命令的时候显示无文件或目录
- A：编译 kernel 时可能未打开 NFS_client 功能





### 编译 `mtd_untils` 

- 文件目录：`~/goxceed/develop/platform/gxtools/mtd_utils`

- 修改 `Makefile`：

  ```diff
  -CONFIG_CROSS_COMPILE=csky-linux-
  +CONFIG_CROSS_COMPILE=arm-linux-uclibcgnueabihf-
  ```

- 编译：

  ```sh
  make clean;make
  ```

- 编译会生成以下工具：

  ```
  lrwxrwxrwx  1 tanxzh tanxzh     11 4月   3 20:12 flash_erase -> flash_utils
  lrwxrwxrwx  1 tanxzh tanxzh     11 4月   3 20:12 flash_eraseall -> flash_utils
  -rwxrwxr-x  1 tanxzh tanxzh 133348 4月   3 20:12 flash_utils
  lrwxrwxrwx  1 tanxzh tanxzh     11 4月   3 20:12 mtd_debug -> flash_utils
  lrwxrwxrwx  1 tanxzh tanxzh     11 4月   3 20:12 nanddump -> flash_utils
  lrwxrwxrwx  1 tanxzh tanxzh     11 4月   3 20:12 nandwrite -> flash_utils
  ```

- 使用方法：将上述生成的工具 通过 `NFS` 的方式拷贝到根文件系统中使用，例如：

  ```
  [@sirius ]# ./mtd_debug 
  usage: mtd_debug info <device>
         mtd_debug read <device> <offset> <len> <dest-filename>
         mtd_debug write <device> <offset> <len> <source-filename>
         mtd_debug erase <device> <offset> <len>
         mtd_debug comtest <device>
         mtd_debug oobtest <device>
         
         
  # Example
  ./mtd_debug comtest /dev/mtd4
  ```













# Ecos 3.0



## 平台环境：

- 硬件：Canopus 6631SH1A + SPI NAND

- 软件：release



## 所需文件：



```
drwxrwxr-x 2 tanxzh tanxzh     4096 4月  11 14:50 .
drwxrwxr-x 4 tanxzh tanxzh     4096 4月  11 14:42 ..
-rw-rw-r-- 1 tanxzh tanxzh   165536 4月  11 14:43 canopus-6631SH1A-spinand.boot
-rw-rw-r-- 1 tanxzh tanxzh 16777216 4月  11 14:52 all.bin
-rw-rw-r-- 1 tanxzh tanxzh   123760 4月  11 14:48 loader-spinand.bin
-rw-rw-r-- 1 tanxzh tanxzh     2669 4月  11 14:47 nand_flash.conf
-rwxrwxr-x 1 tanxzh tanxzh   466460 4月  11 14:44 shell.bin
-rw-rw-r-- 1 tanxzh tanxzh     1024 4月  11 14:52 table.bin
```



- `bootloader`：`gxloader` 编译生成 `.boot + .bin`

  - `.boot `用来引导下载
  - `.bin` 用来组成整bin下载到开发板

- `kernel`：ecos_shell 编译生成的 shell.bin

  - ecos_shell 将 `ecos` 编译生成的 .a 链接合成 `Kernel.img` 后用来组成整 bin 下载到开发板

- `flash.con`：分区表，用来描述各个文件组成整 bin 时的地址、大小等信息

  - `Nor Flash` 使用 `flash.conf`
  - `Nand Flash` 使用 `nand_flash.conf`

- `table.bin`：描述的是 TABLE 分区的数据，已经包含在整 bin 中

  - `genflash` 工具根据 `flash.conf` 生成 `table.bin`

- `app.bin`：应用程序(可选)

- `all.bin`：包含上述 `loader.bin + kernel.bin + table.bin `的整bin文件

  - 使用 `genflash`工具根据 `flash.conf` 组成整 bin 文件(包含 table.bin)

- 烧录命令：

  ```
  boot -b .boot -c serialdown 0x0 flash.bin
  boot -b .boot -c usbdown 0x0 flash.bin
  boot -b .boot -c netdown 0x0 flash.bin -p ubuntu_ip -s board_ip
  ```





## 编译流程



### 一键编译：make.sh

- 直接使用 make.sh 脚本一键编译

  ```
  ./make.sh solution arm ecos
  ```

- 生成的文件在 `solution/output`目录下，使用 boot 工具下载

  ```
  # 网络下载
  sudo boot -b canopus-6631SH1A-sflash.boot -c netdown 0x0 flash.bin -p pc_id -s board_ip
  
  # 串口下载
  sudo boot -b canopus-6631SH1A-sflash.boot -c serialdown 0x0 flash.bin
  ```

- 若只需要跑 ecos_shell，则需要使用 ecos_shell 编译出来的 `shell.bin` 替换 kernel

  - 由于 ecos 编译出来是.a，而 ecos_shell 中会调用 ecos 编译的 .a，所以 ecos_shell 包含 kernel+app

  - 修改 env.sh

    ```diff
    --- a/env.sh
    +++ b/env.sh
    @@ -14,6 +14,7 @@ CROSS_PATH=$ARCH-$OS
     export ENABLE_MEMWATCH=yes
     unset ENABLE_MEMWATCH
     
    +export GX_PREFIX=$(pwd)/../../library/goxceed
     if [ -z $GX_PREFIX ]; then
         GX_PREFIX=/opt/goxceed
     fi
    ```

  - 同步环境变量：

    ```
    source env.sh
    ```

  - 进入 `ecos_shell` 目录，编译 ecos_shell

    ```
    make clean
    ./build flash
    ```

  - 将编译完成的 `_build/shell.bin` 拷贝到`solution/output`目录

  - 修改 `flash.conf`文件

    ```
    BOOT    loader-sflash.bin       true        RAW     	     ro		0	       0         0x000000     auto 
    #I_OEM   invariable_oem.ini    true        RAW     	     ro		0	       0         0x00F000     auto
    TABLE   NULL                   true        RAW               ro         0              0         0x030000     512
    #LOGO    logo.bin               false       RAW     	     ro		0	       0         0x030200     59K
    #V_OEM   variable_oem.ini      true        RAW     	     rw		0	       0         0x03F000     auto
    KERNEL  shell.bin              true        RAW     	     ro   	0	       0         0x040000     auto
    ROOT    root_fs.img        false        SQUASHFS  	     ro   	0	       0     0x600000         auto 
    #DATA    minifs.img             false        MINIFS  	     rw 	0	       0     0x700000         auto
    ```

  - 使用`genflash`工具生成 table.bin 和 整bin

    ```
    genflash mkflash flash.conf flash.bin
    ```

  - 按照下载命令进行下载即可

注：ecos 不需要 rootfs 也可以正常运行



>Q:如果编译过程中出现报错：
>
>A:同步最新代码：`rpeo sync`，分别去每个目录下`make clean` 或 `./build clean`

>Q:下载时未找到.boot文件
>
>A: gxloader 目录下的 output 目录中会保留.boot文件，拷贝过来即可







### SPI NAND 编译：



#### gxloader 编译：

- 编译gxloader对应的.boot和.bin，不需要做任何修改，<font color=red>CMD_LINE也不做修改</font>

  - 修改 `conf/canopus/6631SH1A/boot.config(debug.config)`为 SPINAND

    ```diff
    --- a/conf/canopus/6631SH1A/boot.config
    +++ b/conf/canopus/6631SH1A/boot.config
    @@ -63,8 +63,8 @@ ENABLE_COMPRESS_ZLIB = n
     ##III Module & function configurations
     #
     #############################################################
    -ENABLE_SPIFLASH = y
    -ENABLE_SPINAND = n
    +ENABLE_SPIFLASH = n
    +ENABLE_SPINAND = y
     ENABLE_NANDFLASH = n
     ENABLE_IRQ = n
     ENABLE_UART_IRQ = n
    diff --git a/conf/canopus/6631SH1A/debug.config b/conf/canopus/6631SH1A/debug.config
    index 78a6a8b4..e79abc3b 100644
    --- a/conf/canopus/6631SH1A/debug.config
    +++ b/conf/canopus/6631SH1A/debug.config
    @@ -68,8 +68,8 @@ ENABLE_COMPRESS_ZLIB = n
     ##III Module & function configurations
     #
     #############################################################
    -ENABLE_SPIFLASH = y
    -ENABLE_SPINAND = n
    +ENABLE_SPIFLASH = n
    +ENABLE_SPINAND = y
    ```

  - 编译.boot

    ```
    ./build canopus 6631SH1A boot
    ```

  - 编译.bin

    ```
    ./build canopus 6631SH1A debug
    ```

  - 将编译生成的.boot 和 .bin 分别拷贝到镜像目录备用



#### ecos3.0 编译：

- 编译 ecos 内核

  ```
  ./build arm ecos
  ```



#### 编译 ecos_shell：

- 修改 env.sh

  ```diff
  +export GX_PREFIX=$(pwd)/../../library/goxceed
  +
   if [ -z $GX_PREFIX ]; then
       GX_PREFIX=/opt/goxceed
   fi
  ```

- 保存修改的环境变量：

  ```
  source env.sh
  ```

- 由于ecos_shell默认编译的是norflash，需要修改为 SPINAND

  ```diff
  --- a/ecos_shell/prj/spi_cramfs_jffs2/include/gxcore_ecos_module.hxx
  +++ b/ecos_shell/prj/spi_cramfs_jffs2/include/gxcore_ecos_module.hxx
  @@ -20,9 +20,9 @@ cramfs_init_class         cramfs0    CYGBLD_ATTRIB_INIT_PRI(60360);
   //cyg_serial_init_class     gxserial   CYGBLD_ATTRIB_INIT_PRI(60600);
   //cyg_norflash_init_class   norflash0  CYGBLD_ATTRIB_INIT_PRI(60800);
   //cyg_gx_spinor_flash_init_class   spinorflash0   CYGBLD_ATTRIB_INIT_PRI(60900);
  -cyg_dw_spinor_flash_init_class   spinorflash0   CYGBLD_ATTRIB_INIT_PRI(60900);
  +//cyg_dw_spinor_flash_init_class   spinorflash0   CYGBLD_ATTRIB_INIT_PRI(60900);
   //cyg_gx_spinand_flash_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
  -//cyg_dw_spinand_flash_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
  +cyg_dw_spinand_flash_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
  ```

- 编译 ecos_shell，生成 shell.bin

  ```
  make clean
  ./build flash
  ```

- 拷贝 `shell.bin` 到镜像目录备用



####  生成整bin：

- 复制一份`nand_flash.conf`文件，修改后的内容如下：

  ```
  flash_size    0x1000000
  block_size    0x020000
  #write_protect true
  crc32         true
  #zlibmode      true
  #key_file      gx-rsa2048-keypair.pem
  table_version 2  
  
  # readme
  # 1)zlibmode: if true, the output binfile format is [8 bytes magic("zlibmode") + table + [zlibmode part1] + [zlibmode part2] + ... ]
  #             zlibmode part format is [partition name + compress size + zlib compress partition data bin]
  #
  # 2)key_file: indicate the private key for signature
  # 3)table_version: 
  #       1:old version
  #       2:add reserved size, table size is expanded to 1024B
  #
  
  # 1) NAME field:
  #	"BOOT"   : gxloader
  #	"TABLE"  : partition table
  #	"LOGO"   : gxloader show logo
  #	"KERNEL" : application program
  #	"ROOT"   : root file system
  #
  # 2) The FS field have these types:
  #           "RAW": self definition file system type;
  #           "CRAMFS":  cram_file_system type;
  #           "MINIFS":  mini_file_system type;
  # 3) The partition have 2 mode "ro" and "rw":
  #           "ro": means that this partition could not been modified at runtime;
  #           "rw": means that this partition could modify at runtime
  # 4) UPDATE:
  # 	0: don't update
  # 	1: always update
  # 	2: auto update, while version > old version
  # 	3: clear partition data
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
  # 4) extend_info:0 not enable; 1 enable crc; 2 enable signature; 3 enable crc&signature. crc/signature only
  #    valid in "ro" MODE partition.
  # 5)
  #
  # NAME	FILE                    CRC     FS    MODE UPDATE VERSION ADDRESS  TOTAL_SIZE  EXTEND RES_SIZE
  #----------------------------------------------------------------------------------------------------------
  BOOT    loader-spinand.bin      true	RAW    ro    0     0     0x000000  0x080000   0      0x20000
  TABLE   NULL                    true	RAW    ro    2     1     0x080000  0x080000   0      0x20000
  KERNEL  shell.bin               true	RAW    ro    1     1     0x100000  0x400000   0      0x60000
  #ROOT    rootfs.img              false	YAFFS2 rw    1     1     0x500000  0x700000   0      0x100000
  #DATA    NULL                    false	YAFFS2 rw    3     63    0xc00000  0x500000   0      0x80000
  ```

- 使用 `genflash` 工具根据 `nand_flash.conf` 生成整bin

  ```
  genflash mkflash nand_flash.conf flash.bin
  ```



#### 烧录：

```
# 网络下载
sudo boot -b canopus-6631SH1A-sflash.boot -c netdown 0x0 flash.bin -p pc_ip -s board_ip

# 串口下载
sudo boot -b canopus-6631SH1A-sflash.boot -c serialdown 0x0 flash.bin
```









# Linux 4.9.y



## 平台环境：

- 硬件：Canopus 6631SH1A + SPI NAND

- 软件：release



## 所需文件：

```
-rw-rw-r-- 1 tanxzh tanxzh   165536 4月  11 16:03 canopus-6631SH1A-spinand.boot
-rw-rw-r-- 1 tanxzh tanxzh 16777216 4月  12 09:16 all.bin
-rw-rw-r-- 1 tanxzh tanxzh  2846685 4月  12 09:15 kernel.img
-rw-rw-r-- 1 tanxzh tanxzh   123760 4月  11 20:04 loader-spinand.bin
-rw-rw-r-- 1 tanxzh tanxzh     2668 4月  11 16:40 nand_flash.conf
-rw-r--r-- 1 tanxzh tanxzh  3006464 4月  11 16:39 rootfs.bin
-rw-rw-r-- 1 tanxzh tanxzh     1024 4月  12 09:16 table.bin
```



- `bootloader`：`gxloader` 编译生成 `.boot + .bin`
  - `.boot `用来引导下载
  - `.bin` 用来组成整bin下载到开发板
  
- `kernel`：linux 编译生成的 uImage + .dtb
  - 使用 cat 命令将 .dtb 和 uImage 合成整个 kernel.img

- `flash.conf`：分区表，用来描述各个文件组成整 bin 时的地址、大小等信息
  - `Nor Flash` 使用 `flash.conf`
  - `Nand Flash` 使用 `nand_flash.conf`
  
- `rootfs.bin`：busybox 编译生成
  - 将busybox 编译生成的文件件，使用 mkflash 命令将其转换成对应的 .bin
- `table.bin`：描述的是 TABLE 分区的数据，已经包含在整 bin 中
  - `genflash` 工具根据 `flash.conf` 生成 `table.bin`

- `app.bin`：应用程序(可选)

- `all.bin`：包含上述 `loader.bin + kernel.bin + table.bin `的整bin文件

  - 使用 `genflash`工具根据 `flash.conf` 组成整 bin 文件(包含 table.bin)



## 一键编译：make.sh

- 直接使用 make.sh 脚本一键编译

  ```
  ./make.sh solution arm linux
  ```

- 生成的文件在 `solution/output/image`目录下，使用 boot 工具下载

  ```
  # 网络下载
  sudo boot -b canopus-6631SH1A-sflash.boot -c netdown 0x0 download_linux.bin -p pc_id -s board_ip
  
  # 串口下载
  sudo boot -b canopus-6631SH1A-sflash.boot -c serialdown 0x0 download_linux.bin
  ```





## 开启 NFS mount

​		由于默认编译配置选项中未打开 NFS client 相关的功能，所以需要重新编译内核。

- 编译步骤：

  ```shell
  make ARCH=arm CROSS_COMPILE=arm-linux-uclibcgnueabihf- canopus_defconfig
  make ARCH=arm CROSS_COMPILE=arm-linux-uclibcgnueabihf- menuconfig
  
  File systems --->
      [*] Network File Systems --->
              <*> NFS client support(目录下的所有配置)
              
              
  make ARCH=arm CROSS_COMPILE=arm-linux-uclibcgnueabihf- -j12
  make ARCH=arm CROSS_COMPILE=arm-linux-uclibcgnueabihf- -j12 LOADADDR=0x00008000 uImage
  ```

  



## NFS Rootfs

#### loader.bin

- 编译gxloader，打开 CMDLINE 中的 nfs 选项，修改主机IP和板子IP：

  ```diff
  --- a/conf/canopus/6631SH1A/debug.config
  +++ b/conf/canopus/6631SH1A/debug.config
  @@ -34,8 +34,8 @@ USB_RESISTANCE = 0
   USB_EYE_DIAGRAM_TEST = n
   
   # cmdline config, if CMDLINE_VALUE = "" will not ENABLE_CMDLINE
  -#CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init"
  -CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init root=/dev/nfs rw nfsroot=192.168.101.200:/opt/nfs,v3 ip=192.168.101.3"
  +#CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init rw"
  +CMDLINE_VALUE = "mem=68M videomem=60M mem_end console=ttyS0,115200 init=/init root=/dev/nfs rw nfsroot=192.168.108.111:/opt/nfs,v3 ip=192.168.108.10"
   #CMDLINE_VALUE = "mem=163M videomem=93M mem_end console=ttyS0,115200 init=/init root=/dev/nfs rw nfsroot=192.168.101.200:/opt/nfs,v3 ip=192.168.101.3"
  ```

- 编译 .bin

  ```
  ./build canopus 6631SH1A debug
  ```

- 拷贝备用



**若需要编译 SPI NAND，则需要修改 gxloader 中的 Flash 类型：**

- 修改 `conf/board/xxx.config`中的 Flash 类型：

  ```diff
  --- a/conf/canopus/6631SH1A/boot.config
  +++ b/conf/canopus/6631SH1A/boot.config
  @@ -63,8 +63,8 @@ ENABLE_COMPRESS_ZLIB = n
   ##III Module & function configurations
   #
   #############################################################
  -ENABLE_SPIFLASH = y
  -ENABLE_SPINAND = n
  +ENABLE_SPIFLASH = n
  +ENABLE_SPINAND = y
  
  --- a/conf/canopus/6631SH1A/debug.config
  +++ b/conf/canopus/6631SH1A/debug.config
  -ENABLE_SPIFLASH = y
  -ENABLE_SPINAND = n
  +ENABLE_SPIFLASH = n
  +ENABLE_SPINAND = y
  ```



#### kernel.img

* buildroot 目录下配置 Linux 编译选项：	

  ```
  make help
  make linux_menuconfig
  ```

* menuconfig 中打开以下选项

  ```
  File systems --->
      [*] Network File Systems --->
              <*> NFS client support(目录下的所有配置)
              [*] Root file system on NFS (若没有，则开Networking support)
      # nfs 文件系统做根 方式还需要开启 Networking support->   
                              Networking options->  
                                  TCP/IP networking下IP:  
                                  kernel level autoconfiguration下全开(IP: advanced router下面要不要全开不确定)
  
  ```

* 执行一键编译：

  ```
  ./make.sh solution arm linux
  ```

* `solution/output/image/bin_linux`目录下保存生成的文件

* 将`.dts + uImage` 合成 `kernel.img`

  ```
  cat canopus_generic.dtb > kernel.img
  cat uImage >> kernel.img
  ```

* 拷贝 kernel.img 备用



**若需要编译 SPI NAND，则需要修改 Linux 中的 Flash 类型：**

```diff
--- a/arch/arm/configs/canopus_defconfig
+++ b/arch/arm/configs/canopus_defconfig
@@ -51,10 +51,10 @@ CONFIG_MTD=y
 CONFIG_MTD_CMDLINE_PARTS=y
 CONFIG_MTD_BLOCK=y
 CONFIG_MTD_DATAFLASH=y
-CONFIG_MTD_M25P80=y
 CONFIG_MTD_SST25L=y
 CONFIG_MTD_BLOCK2MTD=y
-CONFIG_MTD_SPI_NOR=y
+CONFIG_MTD_SPINAND=y
+CONFIG_MTD_UBI=y
```



#### 生成整 bin：

- 使用 `genflash` 工具根据 `xxx.conf` 生成整bin

  - norflash 使用 flash.conf
  - nandflash 使用 nand_flash.conf

  ```
  genflash mkflash nand_flash.conf flash.bin
  ```



####  烧录：

```
# 网络下载
sudo boot -b canopus-6631SH1A-sflash.boot -c netdown 0x0 download_linux.bin -p pc_id -s board_ip

# 串口下载
sudo boot -b canopus-6631SH1A-sflash.boot -c serialdown 0x0 download_linux.bin
```







## 编译根文件系统为其它文件格式



### Jffs2：

- loader：正常编译

- kernel：打开 Jffs2 编译选项：

  - `buildroot`目录下使用 `make help` 查看编译帮助信息

  - 使用命令`make linux_menuconfig`配置linux

    ```
    File system --->
    　　[*] Miscellaneous filesystems --->
    　　　　<*> Journalling Flash File System v2 (JFFS2) support
    ```

  - 配置完保存退出，按下y确认保存到buildroot

- rootfs：将编译出来的根文件系统使用工具打包成`jffs2`格式

- Table.bin:

  



### UBI：

- loader：打开loader 的宏是用于修改`cmdline`

  - 打开 UBI 宏：

```diff
    -ENABLE_SPIFLASH = y
    -ENABLE_SPINAND = n
    +ENABLE_SPIFLASH = n
    +ENABLE_SPINAND = y
     ENABLE_NANDFLASH = n
     ENABLE_IRQ = n
     ENABLE_UART_IRQ = n
    diff --git a/conf/sirius/6633H1/debug.config b/conf/sirius/6633H1/debug.config
    index 1c01463a..aae6e1c5 100644
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
     
     # if you set <=0, you'll enter prompt directly
    @@ -56,7 +56,7 @@ ENABLE_UIMAGE = y
     ENABLE_ROOTFS_YAFFS2 = n
     ENABLE_ROOTFS_ROMFS = y
     ENABLE_ROOTFS_CRAMFS = y
    -ENABLE_ROOTFS_UBIFS = n
    +ENABLE_ROOTFS_UBIFS = y
```

- kernel：使用 `sirius_nand_defconfig`文件，默认已打开 UBI 及 UBIFS 编译选项，需要用到 `nfs`

```diff
--- a/arch/arm/configs/sirius_nand_defconfig
+++ b/arch/arm/configs/sirius_nand_defconfig
@@ -167,7 +167,9 @@ CONFIG_SQUASHFS_XATTR=y
CONFIG_SQUASHFS_LZ4=y
CONFIG_SQUASHFS_LZO=y
CONFIG_SQUASHFS_XZ=y
-# CONFIG_NETWORK_FILESYSTEMS is not set
+CONFIG_NFS_FS=y
+CONFIG_NFS_V3_ACL=y
+CONFIG_NFS_V4=y
```

- rootfs：

  - `mkfs.ubifs`用于将根文件系统打包成 UBIFS 文件系统镜像

```shell
mkfs.ubifs -r root -m 0x1000 -e 0x3e000 -c 2048 -x none -o root.img
mkfs.ubifs -r root -m 2048 -e 126976 -c 160 -x none -o root.img
#-m最小输入输出大小为2KiB(2048bytes)，一般为页大小 
#-e逻辑可擦除块大小为124KiB=(每块的页数-2)*页大小=（64-2）*2KiB=124KiB 
#-c最多逻辑可擦除块数目为160(160*128KiB=20MiB),这个可根据ubi volume来设置，并非指定文件系统的大小，而是限制其最大值
```
  
|         OPTIONS         |                          说明                           |            备注            |
| :---------------------: | :---------------------------------------------------: | :----------------------: |
|   -r, -d, --root=DIR    |                     从目录DIR构建文件系统                      |                          |
| -m, --min-io-size=SIZE  |                       最小IO单元大小                        | flash编程的最小单位，一般=pagesize |
|   -e, --leb-size=SIZE   |                        逻辑擦除块大小                        |                          |
| -c, --max-leb-cnt=COUNT |                        最大逻辑块数量                        |   并非指定文件系统的大小，而是限制其最大值   |
|    -o, --output=FILE    |                         输出到文件                         |                          |
|   -j, --jrn-size=SIZE   |                         日志大小                          |                          |
|   -R, --reserved=SIZE   |                     应该为超级用户保留多少空间                     |                          |
|    -x, --compr=TYPE     | 压缩算法 “lzo”, “favor_lzo”, “zlib” or “none” (默认: “lzo”) |                          |
|    -f, --fanout=NUM     |                     扇出 NUM（默认值：8）                     |                          |
|    -F, --space-fixup    |          文件系统可用空间必须在首次挂载时修复（需要内核版本 3.0 或更高版本          |                          |
|  -p, --orph-lebs=COUNT  |                   孤立块的擦除块计数（默认值：1）                    |                          |
|   -D, --devtable=FILE   |                       使用设备表FILE                       |                          |
|    -U, --squash-uids    |                squash所有者使所有文件都归root所有                 |                          |
|  -l, --log-lebs=COUNT   |                    日志的擦除块计数（仅用于调试）                    |                          |
|        -y, --yes        |                   假设所有问题的答案都是“yes”                    |                          |
|    -g, --debug=LEVEL    | 显示调试信息（0 - 无，1 - 统计信息，2 - 文件，3 - 更多详细信息） |                                          |
  
  - `ubinize`用于将 UBIFS 文件系统镜像转换成可直接在FLASH上烧写的格式（带有UBI文件系统镜像卷标)
  
```shell
ubinize -o rootfs.bin -m 2048 -p 128KiB -s 2048 -O 2048 ./ubinize.cfg
#-o：输出文件名
#-m：最小输入输出大小为2KiB(2048bytes)，一般为页大小 
#-p：物理可擦出块大小为128KiB=每块的页数*页大小=64*2KiB=128KiB 
#-s：用于UBI头部信息的最小输入输出单元，一般与最小输入输出单元(-m参数)大小一样
#-O：VID头部偏移量,默认是512
```
  
  - `ubinize.cfg`
  
```
[ubifs]
mode=ubi
image=root.img
vol_id=0
vol_size=20MiB
vol_type=dynamic
vol_name=rootfs
vol_flags=autoresize
```
  

  - flash.conf：修改 data 分区为 UBIFS
  
```
flash_size    0x8000000
#flash_size    0x1000000
block_size    0x020000
#write_protect true
crc32         true
dtb_file      sirius_generic.dtb
table_version 2

# 1) NAME field:
#	"BOOT"   : gxloader
#	"TABLE"  : partition table
#	"LOGO"   : gxloader show logo
#	"KERNEL" : application program
#	"ROOT"   : root file system
#
# 2) The FS field have these types:
#           "RAW": self definition file system type;
#           "SQUASHFS":  cram_file_system type;
#           "MINIFS":  mini_file_system type;
# 3) The partition have 2 mode "ro" and "rw":
#           "ro": means that this partition could not been modified at runtime;
#           "rw": means that this partition could modify at runtime
# 4) UPDATE:
# 	0: don't update
# 	1: always update
# 	2: auto update, while version > old version
# 	3: clear partition data
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
# NAME  FILE                    CRC     FS      MODE    UPDATE VERSION  ADDRESS   SIZE   RES_SIZE
#---------------------------------------------------------------------------------------------------
BOOT    loader-spinand.bin      false   RAW       ro      1      0        0x0000000  0x100000  0xc800
TABLE   NULL                    true    RAW       ro      2      1        0x0100000  0x100000  0xc800
LOGO    logo.bin                false   RAW       ro      1      0        0x0200000  0x100000  0xc800
KERNEL  uImage                  true    RAW       ro      1      1        0x0300000  0x1000000  0x80000
ROOTFS  rootfs.bin              true    UBIFS     rw      1      1        auto       0x2800000  0x140000
APP     app.bin                 true    UBIFS     rw      1      1        auto       0x2800000  0x140000
DATA    NULL                    false   UBIFS     rw      1      0        auto       auto       0x100000
```
  
  - 重新生成分区表：table.bin
  
```
genflash mkflash flash.conf flash.bin
```
  
  - 烧写：
  
```
sudo boot -b sirius-6633H1-spinand.boot -c load_conf_down flash.conf netdown -p 192.168.108.112 -s 192.168.108.10
```
  
    



## 注：

- Q: 需要修改 busybox、kernel 配置，应该如何修改？

- A：在buildroot目录下 使用 make help ，查看 xxx menuconfig 的命令



- Q：ubi0 error: 0xc038a364: too large reserved_pebs 91, good PEBs 88，启动 Linux 时 UBI 报错

- A：因为 ubi 文件系统制作过大，需要修改 `mkfs.ubifs` 时的 `-c `选项



# Solution 仅更新 ecos.bin

- 若需要先更新 gxmisc 的库

```
./build arm ecos
```

- 更新 ecos

```
./build arm
```

- 更新应用

```
./build xxx
```





# 关闭 ecos 多核

- 编译ecos时选择 不带 smp



# 调试双核

- GDB_SERVER 脚本：

```
void InitTarget(void) {
Report("Connect to 2nd A7 core");
JTAG_DRPre             = 0;
JTAG_DRPost            = 0;
JTAG_IRPre             = 0;
JTAG_IRPost            = 0;
JTAG_IRLen             = 4;
CPU                    = CORTEX_A7;
JTAG_AllowTAPReset     = 1;
// Base address of debug registers differs from device to device, so this needs to be adapted
CORESIGHT_CoreBaseAddr = 0x80112000;  // Base address of debug registers of second core.
// Manually configure which APs are present on the CoreSight device
//CORESIGHT_AddAP(0, CORESIGHT_AHB_AP);
CORESIGHT_AddAP(1, CORESIGHT_APB_AP);
}
```



- 开启 两个 gdb_server：

```shell
# 第一个
sudo ./JLinkGDBServerCLExe -select USB -device Cortex-A7 -endian little -if JTAG -speed 4000 -noir -LocalhostOnly -port 2331

# 第二个
sudo ./JLinkGDBServerCLExe -select USB -device Cortex-A7 -endian little -if JTAG -speed 4000 -noir -LocalhostOnly -jlinkscriptfile ./ConnectSecondCore.JLinkScript -port 2341
```

  

- 开启 gbd：

```shell
# 第一个
arm-none-eabi-gdb
target remote 127.0.0.1:2331

# 第二个
arm-none-eabi-gdb
target remote 127.0.0.1:2341
```

  

- 指令：

```
# 加载符号表
file out.elf

# 查看寄存器
i r
```

  









# Canopus 6631SHNG 下载需要修改引脚复用

LQFP156 测试板，6631SHD(SHNG) 芯片 管脚复用： 

- NFC: 17~29 配 3
- NET： 76~90 配 1





GX3215B板级，BGA封装，nfc控制器管脚复用如下： 

- nfc控制器有2路管脚复用： 
  - 91-103管脚配置成1，为nfc控制器1路功能，默认；
  - 17-29管脚配置成3，为nfc控制器2路功能，备用；





# Canopus 6631SHNG 编译 UBIFS

- 使用 `make.sh` 一键编译脚本编译生成 `solution` 代码

```shell
./make.sh solution arm linux
```

  - 选择板级为 `canopus_pnand`

- 进入`solution/output/image`目录，发现 `boot` 工具根据 `flash.conf`文件下载后无法启动

  - 将 `gxloader` 中`6631SHNG `板级的引脚复用成修改成并行nand，打开 `debug.config`中的 `ENABLE_ROOTFS_UBIFS=y、CMDLINE_VALUE`，然后重新编译 `.boot` 和 `.bin`

- 将新编译的  `.boot` 和 `.bin` 拷贝到`solution/output/image/bin_linux`，替换一键编译生成的文件

- 修改 `flash.conf` 中的`boot`文件名称为 `loader-nand.bin`，然后使用 `genflash` 工具生成`整bin`，发现报错`BOOT区域`过大，修改 `flash.conf`，继续生成`整 bin`

```
BOOT    loader-nand.bin         false   RAW       ro      0      0        0x000000  auto
TABLE   NULL                    true    RAW       ro      2      1        0x030000  64k
LOGO    logo.bin                false   RAW       ro      0      0        0x040000  64k
KERNEL  uImage                  true    RAW       ro      1      1        0x050000  auto
ROOTFS  rootfs.bin              true    SQUASHFS  ro      1      1        auto      auto
APP     app.bin                 true    SQUASHFS  ro      1      1        auto      auto
DATA    NULL                    false   MINIFS     rw      0      0       0xDC0000      auto
```

- 使用 `.boot` 根据 `flash.conf` 文件下载`整bin`

- 烧写完成后重新启动，发现启动失败，因为`gxloader`中打开了 `UBIFS` 的选项，但此时的 `rootfs` 不是 `UBIFS`格式，先修改 `gxloader` 为 `SQUASHFS` 格式后再进行尝试，然后新生成的用 `.bin` 重新生成`整bin`，下载查看现象
- 此时以 `squashfs` 启动成功





- 将 SQUASHFS 修改成使用 UBIFS 启动

- `gxloader` 打开 `UBIFS` 选项，编译后拷贝到`solution/outpug/image/bin_linux`

- 修改 `flash.conf` 文件：

  - 修改 `flash_size：0x8000000`、`block_size：0x20000`
  - 修改 `ROOTFS` 分区为 `UBIFS`，名称为`rootfs.bin`，注释掉 `APP区域`、`DATA 区域`也使用 `UBIFS`，修改 `DATA 区域`的地址和大小为 `auto`

- 根据根文件系统生成的`root`目录制作 `ubifs` 格式的根文件系统

```shell
mkfs.ubifs -r root -m 2048 -e 126976 -c 2048 -x none -o root.img
ubinize -o rootfs.bin -m 2048 -p 128KiB -s 2048 -O 2048 ./ubinize.cfg
```

```
[ubifs]
mode=ubi
image=root.img
vol_id=0
vol_size=30MiB
vol_type=dynamic
vol_name=rootfs
vol_flags=autoresize
```

- 根据新生成的 `loader、rootfs.bin`生成`整 bin`，然后使用命令下载

```shell
sudo boot -b canopus-6631SHNG-nand.boot -c load_conf_down flash.conf netdown -p 192.168.108.118 -s 192.168.108.10
```

  - 下载后无法启动 `loader`，使用串口重新下载 `loader`

  - 下载成功启动 `loader` 之后发现没有分区，修改 `flash.conf` 中的分区表名称为生成`整bin`时的 `table.bin`

  - 重新下载，还是无法启动

  - 更换 `flash.conf` 为` 版本2` 的再试一下，启动成功

```shell
flash_size    0x8000000
#flash_size    0x1000000
block_size    0x020000
#write_protect true
crc32         true
dtb_file      canopus_generic.dtb
table_version 2

# NAME  FILE                    CRC     FS      MODE    UPDATE VERSION  ADDRESS   SIZE   RES_SIZE
#---------------------------------------------------------------------------------------------------
BOOT    loader-nand-ubifs.bin         false   RAW       ro      1      0        0x0000000  0x100000  0xc800
TABLE   NULL                    true    RAW       ro      2      1        0x0100000  0x100000  0xc800
#LOGO    logo.bin                false   RAW       ro      1      0        0x0200000  0x100000  0xc800
KERNEL  uImage                  true    RAW       ro      1      1        0x0300000  0x1000000  0x80000
ROOTFS  rootfs.img              true    UBIFS     rw      1      1        auto       0x2800000       0x140000
##APP     app.bin                 true    UBIFS     rw      1      1        auto       0x2800000  0x140000
DATA    NULL                    false   UBIFS     rw      1      0        auto       auto       0x100000
```

    

# 手动编译 linux 替换 kernel

- 在 `kernel/4.9.y` 目录下使用 `build` 脚本进行编译
  - 编译完成后的文件存放在 `library/goxceed/chip` 目录下
- 若需要使用 `nfs` 相关功能，则使用 `build` 脚本编译时，模式选择为 `nfs`
  - 通过 `nfs` 模式编译时，是通过添加配置到`.config` 文件中实现的，`.config`文件存放地址在`library/goxceed/kernel/.config`







# GDB 加载 eCos

```shell
sudo ./jlink_gdb_server -select USB -device Cortex-A7 -endian little -if JTAG -speed 4000 -noir -LocalhostOnly -port 2331
```







# sirius SPINAND eCos

1. gxloader 修改 `boot.config` `debug.config` 中的 Flash 类型

2. 编译出 `.boot + .bin`

3. 删除掉 `library/goxceed` 目录下 `ecos` 相关的文件

4. 编译 `ecos3.0`

5. 修改 `env.sh`

6. 修改 `ecos_shell/scriput/config.mk`

```diff
--- a/ecos_shell/prj/spi_cramfs_jffs2/include/gxcore_ecos_module.hxx
+++ b/ecos_shell/prj/spi_cramfs_jffs2/include/gxcore_ecos_module.hxx
@@ -20,8 +20,8 @@ cramfs_init_class         cramfs0    CYGBLD_ATTRIB_INIT_PRI(60360);
//cyg_serial_init_class     gxserial   CYGBLD_ATTRIB_INIT_PRI(60600);
//cyg_norflash_init_class   norflash0  CYGBLD_ATTRIB_INIT_PRI(60800);
//cyg_gx_spinor_flash_init_class   spinorflash0   CYGBLD_ATTRIB_INIT_PRI(60900);
-cyg_dw_spinor_flash_init_class   spinorflash0   CYGBLD_ATTRIB_INIT_PRI(60900);
-//cyg_gx_spinand_flash_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
+//cyg_dw_spinor_flash_init_class   spinorflash0   CYGBLD_ATTRIB_INIT_PRI(60900);
+cyg_gx_spinand_flash_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
//cyg_dw_spinand_flash_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
cyg_flashio_init_class    flashio    CYGBLD_ATTRIB_INIT_PRI(61000);
//nand_init_class           nandflash0 CYGBLD_ATTRIB_INIT_PRI(61110);
diff --git a/ecos_shell/profile/flash b/ecos_shell/profile/flash
index 1df1fd86..2ce3da3e 100644
--- a/ecos_shell/profile/flash
+++ b/ecos_shell/profile/flash
@@ -3,4 +3,3 @@ i2c
spi_cramfs_jffs2
gpio
uart
-partition
diff --git a/ecos_shell/script/config.mk b/ecos_shell/script/config.mk
index 271636cb..b40e07a9 100755
--- a/ecos_shell/script/config.mk
+++ b/ecos_shell/script/config.mk
@@ -1,4 +1,4 @@
-ARCH = csky
+ARCH = arm
#########################

ifeq ($(ARCH),csky)
diff --git a/env.sh b/env.sh
index d80f735c..54ea9024 100644
--- a/env.sh
+++ b/env.sh
@@ -14,6 +14,7 @@ CROSS_PATH=$ARCH-$OS
export ENABLE_MEMWATCH=yes
unset ENABLE_MEMWATCH

+export GX_PREFIX=$(pwd)/../../library/goxceed
if [ -z $GX_PREFIX ]; then
	GX_PREFIX=/opt/goxceed
```

7. 编译出来 `shell.bin`

8. 可以使用 gdb 的方式加载：

```shell
sudo ./jlink_gdb_server -select USB -device Cortex-A7 -endian little -if JTAG -speed 4000 -noir -LocalhostOnly -port 2331

arm-none-eabi-gdb _build/shell.elf
```

9. 或者使用下载到 flash 中的方式加载：

   - 修改 `flash.conf`

 ```
 flash_size    0x8000000
 block_size    0x020000
 write_protect true
 crc32         true
 
 # name   source file           CRC        file_system_type mode    update   version 	start addr   size
 #---------------------------------------------------------------------------------------------------------
 BOOT    loader-sflash.bin      true         RAW     	     ro		0	       0         0x000000     1024k
 TABLE   table.bin              false        RAW              ro         0              0         0x0100000     1024k
 LOGO    logo.bin               false        RAW     	     ro		0	       0         0x0200000     1024k
 KERNEL  shell.bin              true         RAW     	     ro   	0	       0         0x0300000     16384k
 ROOT    root_cramfs.img        false        CRAMFS  	     ro   	0	       0         auto         40960k
 DATA    data.bin               false        YAFFS2  	     rw 	0	       0         auto         auto
 ```

   - 根据 `flash.conf` 生成整 bin

 ```shell
 genflash mkflash flash.conf flash.bin
 ```

   - 烧写

 ```shell
 sudo boot -b sirius-6633H1-spinand.boot -c netdown 0x0 flash.bin -p 192.168.108.118 -s 192.168.108.110
 ```

     

# GDB 加载 Linux

## csky

- gxloader：

  - 编译成 debug 版本，在 `board/xxx/boar-init.c` 中打开分区表
  - 将编译生成的 .bin 烧录到板子，这里 bootloader 的目的只是初始化一些设备，主要是初始化好 ddr

- linux：

  - 编译成 debug 版本，需要修改设备树中的`chosen {bootargs = “xxx”}`，linux 内核的 cmdline 配置有两种方式，一种是 bootloader 通过 bootargs 传递，一种是通过内核传递。
  - 编译完会生成 .dtb vmlinux 放在 library 目录下

- rootfs：

  - 使用 busybox 编译出 csky 架构的根文件系统，然后创建一些文件夹，添加一些文件...
  - 使用命令 `mksquashfs root squashfs.img`

- 烧写：

  - 将 loader 烧到 0 地址
  - kernel 可以不用烧写，因为会使用 gdb 将 kernel 加载到 ddr
  - 将 rootfs 根据 loader 的分区表烧到对应的位置

- 启动：

  - loader 启动到倒计时，然后按下回车

  - 开启 `DebugServerConsole`

  - 执行 `csky-linux-gdb vmlinux` 加载内核镜像，这里需要有 .gdbinit

```
# csky linux
target jtag jtag://127.0.1.1:1025
set endian little
restore arch/csky/boot/dts/gx3211_generic.dtb binary 0x96200000
load vmlinux
file vmlinux
set $r3=0x96200000
```

  - 然后即可正常执行 gdb 指令



## arm

- gxloader：

  - 需要在 `main.c` 中调用 `boot_close_module()` 接口来关掉 `cache、mmu` 等，来保证启动 Linux 时是干净的环境

```diff
diff --git a/main.c b/main.c
index a30c84ac..34c69272 100644
--- a/main.c
+++ b/main.c
@@ -252,6 +252,7 @@ void __attribute__((section(".entry_text")))entry(uint32_t ddr_size)
				g_board_extend.func_usb_update();
		}
 
+       boot_close_module();
 #ifdef CONFIG_ENABLE_LOGO
		gx_show_logo();
 #endif
```

- linux:

	- 编译成 debug 版本，修改设备树中的 `chosen {bootargs = “xxx”}`

	- 将 loader 烧到 flash，在 linux kernel 目录下执行：`arm-none-eabi-gdb` ，然后 `c` 即可

	  - `.gdbinit`：

```
# arm linux
target remote 127.0.0.1:2331
set endian little
monitor speed 10000
restore /home/tanxzh/goxceed/develop2/library/goxceed/kernel-arm/arch/arm/boot/dts/virgo_generic.dtb binary 0x00000100
restore /home/tanxzh/goxceed/develop2/library/goxceed/kernel-arm/arch/arm/boot/Image binary 0x8000
file /home/tanxzh/goxceed/develop2/library/goxceed/kernel-arm/vmlinux
set $r2=0x00000100
set $pc=0x00008000
```

- 如果没有分区表，是因为 cmdline 中没有 mtdparts，在设备树中修改一下即可

```dtd
	bootargs = "mem=68M videomem=48M fbmem=12M console=ttyS0,115200 init=/init root=/dev/nfs rw nfsroot=192.168.108.149:/opt/nfs/arm,v3 ip=192.168.108.150 \
		mtdparts=m25p80:128k@0m(BOOT),64k@128k(TABLE),2880k@192k(KERNEL),1792k@3m(ROOTFS),2368k@4864k(USER),960k@7232k(DATA) mtdparts_end";

```

- 如果打不上断点，可以在 def_config 文件中加入以下配置：

```diff
+CONFIG_DEBUG_INFO=y
+CONFIG_DEBUG_VM=y
```

  



# IRR 模块 Linux API 测试

- 使用 `gxtest/stb/goxceed/cases/service/periphery/irr/cmd/` 目录下的测试 case
- 在 `gxtest/stb/goxceed` 目录下先打开 `confs/def_config` 文件中的 `USE_SHELL_CLI_SHELL=y` 选项
- `./build config -m irr`
- `./build arm linux virgo fpga dvbs bin`
- 生成一个整 bin，将整 bin 烧到 flash，启动之后会启动测试 shell
- **注：需要使用 cli shell，而不是 gxshell**





# Linux 启动后配置好 IP，能够 ping 通主机但无法 mount 上

- 编译 kernel 时使用的是 `debug` 模式，更换为 `nfs` 模式即可









# IRR Virgo 测试编译

## Gxloader:

  - 打开 `.config` 中的 `ENABLE_IRR=y`选项和 `ENABLE_IRQ=y` 选项
  - v0:`drivers/irr/gx_irr.c`
    - 中断方式获取键值
  - v1:


## Linux：

  - 挑选用例：在 `gxtest/stb/goxceed` 目录下打开 `confs/def_config` 文件中的 `USE_SHELL_CLI_SHELL=y` 选项
  - 编译用例模块：`./build config -m irr`
  - 编译 gxtest：`./build arm linux virgo fpga dbvs bin`
  - 编译完成后会生成整 bin，将整 bin 烧到 flash，启动之后会启动测试 shell
  - 要求能够正确读到键值

  > 编译 gxtest 之前需要先将基础库都编译通过，建议使用 `./make.sh gxtest-bus arm virgo fpga linux dvbs bin`


## eCos:
  - 与 linux 相同的测试方法，只是编译更改成 ecos 即可





# v1.9.6-4-lts 分支编译 Linux-2.6.27.55
编译方法：
- 修改 .zshrc 切换成编译 linux-2. x 版本的工具链，并重起一个 shell 窗口
- `make ARCH=csky CROSS_COMPILE=csky-linux- gx3211-debug_defconfig`
- `make ARCH=csky CROSS_COMPILE=csky-linux- -j$(nproc)`

