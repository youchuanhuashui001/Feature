# 链接

[https://gitee.com/progit/2-Git-%E5%9F%BA%E7%A1%80.html#](https://gitee.com/progit/2-Git-基础.html#) --git手册

http://www.ruanyifeng.com/blog/2020/04/git-cherry-pick.html --cherry-pick

https://zhuanlan.zhihu.com/p/413646220	--wechat

http://www.deansys.com/doc/ldd3/index.html	--linux driver

http://git.nationalchip.com/redmine/issues/304240 --- boot命令如何使用的redmine

支持 #262662：gxmisc已发布版本后续Flash支持更新记录

功能 #258756：添加串并行 nand 坏块标记

http://git.nationalchip.com/redmine/projects/robot_os/wiki/Bootx工具  — Bootx工具



http://git.nationalchip.com/redmine/issues/284584   -- Canopus FPGA 验证

https://git.nationalchip.com/redmine/issues/308817 -- Canopus 芯片测试

http://git.nationalchip.com/redmine/issues/270875   -- Cygnus FPGA 验证

https://git.nationalchip.com/redmine/issues/323205 -- Cygnus 芯片测试

https://git.nationalchip.com/redmine/issues/361528 -- Vega 芯片测试

https://git.nationalchip.com/redmine/issues/366005 -- Scorpio FPGA测试

https://git.nationalchip.com/redmine/issues/385274 -- Scorpio 芯片测试







https://www.iczhiku.com/videoDetail/2czzy14sVzI3tjHR4@ly3mg== -- ICZhiku

https://git.nationalchip.com/redmine/issues/342422  评估 XIP 连续模式和Wrap Aroud 功能





# LinkCube 备用订阅：

```
trojan://iynvtav4yJ@backup4free.gotmpxray.tk:16888?security=tls&sni=backup4free.gotmpxray.tk&type=tcp#free
```



# Samba 服务器：

smb失败：由软件导致的连接断开

由于20.04把smb协议升级了，造成旧协议无法使用

编辑`/etc/samba/smb.conf  ` 在`[global]`后面添加
 `client min protocol = NT1`

```text
smb://192.168.0.254
名称：tanxzh
域名：guoxin
密码：联想网盘的密码
```

# GIT

- git reset HEAD^ --hard 回到上一次的提交
- :%s/源字符串/目标字符串/g ---全局替换
- {{patch(Gerrit号)}}
- commit -m "302932: issue"
- `smb://192.168.110.253/public/ftp/stb/GoXceed_Bus_Manual_v1.9-test/125/taurus_6605H1_linux_dvbs`
- 切换分支 `repo init -b 分支名（如 v1.0.2-9）如果想要查看仓库信息可以看 .repo/manifest.xml`
- 查看可切换的分支 cd .repo/manifests git branch -a | cut -d / -f 3

```Bash
repo init -b imx-4.1.15-1.0.0_ga
repo sync   # 如果不需要与服务器数据一致，可以不运行该步
repo start imx-4.1.15-1.0.0_ga --all 
```

- 查看切换结果 `repo branches`

- 前进或回退： `git reflog` 定义：查看命令操作的历史 查找到你要的 操作id，依旧使用 上文说的 `git reset --hard id`。又回退到当初一模一样的版本啰！

- git场景 很有可能你是你本地修改还没有提交 可以使用 

  ```
  git stash
  ```

   缓存当前工作

  ```
  repo sync
  ```

   同步成功

  ```
  git stash apply
  ```

   恢复前面的工作 本方法同样适用于分支切换 该问题最大可能是 git 版本不对，根据实验 1.9.1 版本可能会出该问题，但是 1.7.1 或者 2.9.5 没有发生该问题，可以通过重新安装 git 来解决。

  - repo upload 以后后悔了，怎么办？ 在原有分支上修改文件 git commit –amend repo upload –replace 404 出现文件以后去掉第 1 行的 #，中括号里面填上 change-id
  - repo upload 的时候出错或者忘记 repo start 的时候怎么办？ 重新 repo init 一个工程

```text
  repo start xxx project
  cd project
  git fetch path branch:test ---path是原来工程的路径
  git merge test
  repo upload
- repo sync 同步不成功
- `repo upload` 报 missing blob 或者 missing tree 怎么办
```

- 要提交代码，但是本地代码不是最新
  - `git stash`：暂存
  - `repo sync`：同步代码
  - `git stash pop`：恢复暂存的代码
  - `repo upload`：上传代码
  
- 要在补丁的基础上提交：`git cherry-pick`

- 要把补丁合并到本地：`git pull`

- 回退到某个版本：`git reset — hard commit_id`

- 暂存一个文件的修改以及恢复

  - ```shell
    git stash push <file_name>
    git stash apply
    ```

- 工作流程：依赖其他补丁，并提交其他补丁
  - 首先 pull 需要依赖的补丁，然后修改，修改完成之后 -m 提交补丁，后续使用 --amend

- 回退当前的 commit

  ```shell
  git reset HEAD~
  ```

  



# c语言规范：

- 函数名：小写单词之间用下划线 (is_prime)
- ,;后面要有一个空格
- 循环/判断语句后有一个空格 {前有一个空格
- 变量初始化赋初值
- 变量定义与程序空一行
- 双目运算符前后加空格
- 单目运算符前后无空格
- 不允许出现多余的空白字符
- 空行不能多

# Jtag

1. 开JTAG的后台 `DebugServerConsole -setclk 2`
2. 打开minicom `sudo minicom`
3. 打开gdb调试 `csky-abiv2-elf-gdb gxscpu.elf`
4. 开启csky的jtag服务 `target jtag jtag://127.0.1.1:1025`
5. 加载elf文件 `load out.elf`
6. break到main函数
7. 运行

- `sudo minicom -D /dev/ttyUSB0`
- `minicom -D /dev/ttyUSB0 -C 1.log`
- 命令`./boot -b gx.boot -c usbdown 0 1.bin -d /dev/ttyUSB0`

# nfs做根

- 直连网线：eth0
- usb转以太网：eth1
- usb网卡：WLAN
- 在接触一个新的工程时，首先要知道工程的功能、编译方法

arm:

- nfs出来的目录做根(/root)
  - make menuconfig中开nfs的root
- 不用手动指定ip，直接在loader中指定好，然后挂载
- 然后把动态库加载到/root目录下去，(gdbserver需要一个.so-->/root)
- git 可以打补丁 但是打完补丁之后再次提交就提交在补丁上了

# 修改 /bin/sh 为 bash

- `sudo dpkg-reconfigure dash`

# 工作流程：

- repo init，repo sync工程
- repo start 创建分支
- code
- upload之后可以新建一个分支
- repo sync开始下一个工作

# 编译脚本

# 编译csky ecos报错

- opt目录下的文件没有清除掉，可能使用的是其他版本的文件
  - 删除掉opt目录下的文件，重新编译
- 编译ecos_shell之前先`make clean`

![img](https://secure2.wostatic.cn/static/t7HG2pcR5q8h9N8GBoCLe/image.png?auth_key=1675903961-aR21LiMbKxMSq4PBjNbL5E-0-cc45787e7cc0df53fd1740bd723946dd)

# ecos使用gdb调试

- 打断点到命令前缀名处

![img](https://secure2.wostatic.cn/static/nmdmZer8pKqFVhLnZaZYSV/image.png?auth_key=1675903961-51ngHuyeAnUXEgj1D5LZc1-0-e37ddae45053b155dcc1ca9a3d7aa498)

- debug跳不进去，程序被优化了
- 要编译ecos3.0的时候指定optimize=O0

```Bash
optimize=O0 ./build csky
```

- 调试的时候直接s不进去，就多打断点然后s
- 编译gxmisc的时候要修改`scripts/rule_ecos.mk`编译选项`MY_CFLAGS = -c -O0 -g`

![img](https://secure2.wostatic.cn/static/9UD8CX4Z7w7UGx6faerXV7/image.png?auth_key=1675903961-r4RvGGdYctTLHL9C9WqsWx-0-38f05fbb8ad3ab71e125a5a819c34e35)

可能是未找到这整个数据所在位置

- 这个`dev→funs→flash_info(dev,buf)`调到哪里去了？

![img](https://secure2.wostatic.cn/static/cXqfYu5VJPQr6TfNfnx9Dp/image.png?auth_key=1675903961-mLXd4o6jfzUoNVYJc6MXFh-0-47ef3eddb48de7f7eb4a1d8a12a1887f)

![img](https://secure2.wostatic.cn/static/cnMqSYpSiHdPbmTAAduZzj/image.png?auth_key=1675903961-kA4vULuV7SrFrSyqW3TM9Z-0-f50201d7898994c7c0c2c9c6ffabce41)

- 在`gxmisc`中`ecos`目录下也定义了相同的结构体，并进行了赋值

![img](https://secure2.wostatic.cn/static/nUbciyzm6mivcP8P5C2XUH/image.png?auth_key=1675903961-7461wzjWYfyviKujauK12a-0-633b314b6a6fd5cbe76bed12bb45fb05)

# ecos中find_dev是查找`/dev/flash`设备

# gxloader是独立的，不依赖`/opt`目录下的文件

# SPI Nand Flash的坏块表管理

- 读取Flash最后4个Block

- 判断头部是否有主表或镜像表的标记

  - 若有标记则找到坏块表，并获取坏块表的版本号

  - 若无标记则创建坏块表

    - 关掉ecc
    - 查找所有的blocks
    - 读取blocks的oob区域
    - 将读取出来的数据与badblock→pattern进行比较
    - 如果相等(0xff)，则不是坏块，不相等则为坏块
    - 为坏块则进行坏块标记
    - 打开ecc

  - 标记bbt区域

    ```
    (td,md)
    ```

    - 更新bbt区域
    - 写bbt
    - 查找td和md，找到pages[]中不等于-1的，即有坏块的block
    - 把td→patter(Bbt0)放在坏块的第一个page
    - 把crc数据拷贝到bbt
    - 把是坏块的block擦除掉
    - 标记page被使用

  - rd：如果rd存在，代表至少有一个bbt表存在

# SPI NAND和SPI NOR的OTP

- SPI NAND的otplock会锁掉全部的OTP Region`(真正的OTP空间)`
- SPI NOR的otplock只会锁单个OTP Region`(OTP寄存器)`
- SPI NAND的write只能写一次，且只从`1→0`，故出厂时的flash使用flash otpread的数据应为`全f`

# flash掉电测试：

- ssh [spd@192.168.108.153](mailto:spd@192.168.108.153)
- 查看`daizhi/heqi.sh`
- 拷贝要测试的.boot和.bin到目标主机

```Bash
scp sirius-6633H1-spinand.boot output/loader-spinand.bin spd@192.168.108.153:/home/spd/daizhi/flash_test_package/fw_heqi
```

- 运行脚本`./heqi.sh`

# quartus

- `/opt/gxtools/intelFPGA_17.1/intelFPGA_17.1/intelFPGA/17.1/quartus/bin/quartus`

# SRAM和BOOTROM

BOOTROM：固化在芯片中的一片内存

BOOTROM的代码由由SOC来放置，主要做初始化硬件、初始化SPI FLASH控制器等

首先启动`BOOTROM`中的代码，来初始化`SPI FLASH`控制器，初始化完成之后将`stage1`的代码拷贝到`SRAM`中，然后跳转到`SRAM`中执行`stage1`的代码，`stage1`的代码执行一些初始化然后将`stage2`的代码拷贝到`DDR`中，进行执行。

```
.boot`文件就是直接运行在DDR中的一段代码(应用)，用于使用串口将`.bin`文件拷贝到`flash
```

`COPY_SIZE(STAGE1_SIZE)`：**不能修改**

因为bootrom中的固化代码已经指定了stage1的大小

stage1是拷贝到SRAM中的，而SRAM也只有那么大所以不能修改

# cygnus测试：

- stage1相关的在cpu/copy.c
- stage2相关的在driver/spi/dw_spi.c

# USB-HUB有问题，能够在`/dev`中查看到设备

![img](https://secure2.wostatic.cn/static/39K4EFECp2gJ4g4SZbMTWR/image.png?auth_key=1675903961-uR4irA1XcN8Y8egsRGSapV-0-6fef306df000ed21cd0953bcaa929e6e)

![img](https://secure2.wostatic.cn/static/jeqxJXuqQ25osBw2YXsVgu/image.png?auth_key=1675903961-g81YxASGBir8Z6kt8DqWV6-0-bbd683141d91604bf0076dc8196c454d)

```Bash
$ sudo boot -b cygnus-6705-spinand.boot -c serialdown 0x0 output/loader-spinand.bin 
Found serial: /dev/ttyUSB0
wait ROM request... please power on or restart the board...
test boot_head:
phead->magic: 626f6f74
phead->version: 100
phead->chip: 6705
phead->baudrate: 1c200

write size 142444 byte ...
[===================>]  100.00%Before exec the first command, failed to receive "boot>" from stb in 3s, will EXIT_FAILURE
```

- serialdump
  - dump之后的文件是.bin

# 机顶盒板子 GDB 调试方法

- gdb的调试方法：1(查看程序最后死在哪里)
  - 在gxloader中修改Makefile编译选项为`O0`
  - 在gxmisc中修改编译选项为`O0`(方便gdb进入gxmisc的函数)
  - 编译gxmisc：`./build csky gxloader`
  - 编译gxloader：`./build cygnus 6705 debug`
  - 烧写进flash：`sudo boot -b cygnus_xxx.boot -c serialdown 0x0 output/xxx.bin`
  - 重启
  - 打开串口：`sudo minicom`
  - 编辑.gdbinit

```Bash
target remote 127.0.0.1:1025
- 使用gdb查看当前的bt
- 可以手动添加断点在程序中`unsigned stop_flag = 1; while(stop_flag);`
```

- gdb的调试方法：2
  - 在gxmisc中修改编译选项为O0(方便gdb进入gxmisc的函数)
  - 在gxloader中编译出能正常启动的.boot和.bin并烧到开发板中

```Bash
./build cygnus 6705 boot
cp output/xxx.boot .
./build cygnus 6705 debug
sudo boot -b xxx.booot -c serialdown 0x0 ***.bin
- 修改gxloader中conf/目录下的debug.config，打开GDB选项
- 查看board_init.c中的引脚复用是否需要开启GDB功能
- 开启DebugServer：`DebugServerConsole -setclk 2`
- 开启minicom：在倒计时结束前按下回车
- 编辑.gdbinit
target remote 127.0.0.1:1025
load
- 开启gdb：`csky-elf-gdb xxx.elf`
```



# ARM 调试双核

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

  



# V1.9.6-4-lts 分支 gdbserver 连不上

- 使用 gx3211 板子，烧完 loader 后连不上 gdbserver

- 修改`board/gx3211/board-6622-dvbs2/board-init.c` 中的 `104、105、106、107、108` 将 1 修改为 0

  ```diff
  --- a/board/gx3211/board-6622-dvbs2/board-init.c
  +++ b/board/gx3211/board-6622-dvbs2/board-init.c
  @@ -92,11 +92,11 @@ struct mulpin_config_s mulpin_table[] = {
           {88,        75,     107,    MP_INV_V,  1},           // NC          | SC2RST_PORT43
           {90,        76,     108,    MP_INV_V,  1},           // NC          | SC2DATC8_PORT44
           {92,        77,     109,    MP_INV_V,  1},           // NC          | SC2DATC7_PORT45
  -        {104,       0,      32,     MP_INV_V,  1},           // 29          | DBGTDI_PORT0
  -        {105,       1,      33,     MP_INV_V,  1},           // 30          | DBGTDO_PORT1
  -        {106,       2,      34,     MP_INV_V,  1},           // 31          | DBGTMS_PORT2
  -        {107,       3,      35,     MP_INV_V,  1},           // 32          | DBGTCK_PORT3
  -        {108,       4,      36,     MP_INV_V,  1},           // 33          | DBGTRST_PORT4
  +        {104,       0,      32,     MP_INV_V,  0},           // 29          | DBGTDI_PORT0
  +        {105,       1,      33,     MP_INV_V,  0},           // 30          | DBGTDO_PORT1
  +        {106,       2,      34,     MP_INV_V,  0},           // 31          | DBGTMS_PORT2
  +        {107,       3,      35,     MP_INV_V,  0},           // 32          | DBGTCK_PORT3
  +        {108,       4,      36,     MP_INV_V,  0},           // 33          | DBGTRST_PORT4
           {110,       128,    132,    MP_INV_V,  0},           // 34          | SPISCK_NFRDY0_NFRDY0_SPISCKDW
           {111,       129,    133,    MP_INV_V,  0},           // 35          | SPIMOSI_NFOE_NFWE_SPIMOSIDW
           {112,       130,    134,    MP_INV_V,  0},           // 36          | SPICSn_NFCLE_NFALE_SPICSnDW
  ```

  



# SIRIUS GDB

- sirius 使用 gdb 的时候，所有的 jlink线 都要接上
- 需要在 `xxx.config` 文件中打开 `ENABLE_GDB_DEBUG` 选项，否则会出现断点打上，但不正常停止的情况

- 最新的测试方法：
  - develop 分支编出来的 .boot 烧录 bin 会出现 crc 校验失败，用 v1.9.8.5-tvos 分支的 .boot 烧录
  - 修改 Makefile 成 O0 -g
  - 烧到 flash 中启动
  - gdb server 开起来，.gdbinit 不加载，直接调 stage2



# Leo VSP 工程 GDB

- 拷贝 `configs/` 目录下的一个 `gx8010` 的配置文件到 `.config` 

- `vsp` 目录下执行 `make menuconfig` 自动生成一些文件

- `vsp` 目录下执行 `make menuconfig` 修改编译优化等级，以及打开 `Jtag`

  ```
  MCU settings  --> 
  	Build Schema (Optimized for Size (-Os))  --->
  		( ) Debug (-O0 -g) 
      [ ] Enbale JTAG debug (NEW)
  ```

- 擦除掉 flash 中的程序

  ```shell
  sudo bootx -m leo -t s -c flash eraseall
  ```

- 开启 `DebugServer`

  ```shel
  DebugServerConsole -setclk 4
  ```

- 执行 `gdb`

  ```shell
  csky-abiv2-elf-gdb output/mcu.elf
  ```

  





# SPI_INT_TXEI：要发送的数据少于fifo的阈值就会触发这个中断

# Ubuntu20.04编译linux4.9.y报错：

```
error while loading shared libraries: libmpc.so.3: cannot open shared object file: No such file or directory
```

- `sudo apt install libmpfr6:i386`
- `sudo ln -s /usr/lib/i386-linux-gnu/libmpfr.so.6 /usr/lib/i386-linux-gnu/libmpfr.so.4`

# CPU/CHIP/BOARD

- `cpu`：内核ARM/CK
- `chip`：cygnus/sirius/taurus
- `board`：6705/6705H1

一种内核可以做出多款芯片，一款芯片可以根据pcb画出多款板子

# 启动流程

![img](https://secure2.wostatic.cn/static/n3wvJyZfZwimQf3HrNXCHy/image.png?auth_key=1675903961-d2U5zkKU2iRWkp6M8DTDSK-0-a58c5a8a7f125c68f1ce882c1caaa7b8)

### Stage1

BootRom通过串口或者flash加载Gxloader的Stage1到[SRAM](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#sram)之后跳转运行。虽然BootRom已经 初始化了[CPU](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#cpu)，但是Gxloader的Stage1会根据自己软件的需求重新在初始化。主要功能还是 初始化[CPU](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#cpu)准备好C语言执行环境。明显这个Gxloader的stage1的实现比BootRom的stage1复杂很多，这个也是因为Gxloader需要更多功能。

这个阶段是不同系统的汇编加C语言代码代码实现。下面分别介绍csky和arm两个[cpu](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#cpu)的初始化过程。

#### CSKY( Taurus )

对应的代码在[cpu](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#cpu)/ck/taurus/taurus_start.S

- 配置中断向量表。

- 设置stage1 sp(r0)栈指针。

- 初始化PLL和DDR。

- 配置[Cache](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#cache "高速缓冲存储器，是位于CPU和主存储器DRAM（Dynamic Random Access Memory）之间，规

  模较小，但速度很高的存储器，通常由SRAM（Static Random Access Memory 静态存储器）

  组成。它是位于CPU与内存间的一种容量较小但速度很高的存 储器。CPU的速度远高于内存

  ，当CPU直接从内存中存取数据时要等待一定时间周期，而 Cache则可以保存CPU刚用过或循

  环使用的一部分数据，如果CPU需要再次使用该部分数据时 可从Cache中直接调用，这样就

  避免了重复存取数据，减少了CPU的等待时间，因而提高了系 统的效率。Cache又分为

  L1Cache（一级缓存）和L2Cache（二级缓存），L1Cache主要是集成 在CPU内部，而L2Cache

  集成在主板上或是CPU上。")和[MMU](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#mmu "MMU是Memory Management Unit的缩写，中文名是内存管理单元，它是中央处理器（CPU）中

  用来管理虚拟存储器、物理存储器的控制线路，同时也负责虚拟地址映射为物理地址，以及

  提供硬件机制的内存访问授权，多用户多进程操作系统。")。

- 设置stage2 sp(r0)栈指针。

- 使能中断。

- copy stage2到[dram](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#dram "DRAM（Dynamic Random Access Memory），即动态随机存取存储器，最为常见的系统内存。

  DRAM 只能将数据保持很短的时间。为了保持数据，DRAM使用电容存储，所以必须隔一段时

  间刷新（refresh）一次，如果存储单元没有被刷新，存储的信息就会丢失。 （关机就会丢

  失数据）")。

- 跳转到C程序。

#### ARM( Sirius )

对应的代码在[cpu](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#cpu)/arm/sirius/sirius_start.S

- 配置中断向量表。

- 关闭多核。

- 关闭看门狗。

- 清数据、指令[Cache](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#cache "高速缓冲存储器，是位于CPU和主存储器DRAM（Dynamic Random Access Memory）之间，规

  模较小，但速度很高的存储器，通常由SRAM（Static Random Access Memory 静态存储器）

  组成。它是位于CPU与内存间的一种容量较小但速度很高的存 储器。CPU的速度远高于内存

  ，当CPU直接从内存中存取数据时要等待一定时间周期，而 Cache则可以保存CPU刚用过或循

  环使用的一部分数据，如果CPU需要再次使用该部分数据时 可从Cache中直接调用，这样就

  避免了重复存取数据，减少了CPU的等待时间，因而提高了系 统的效率。Cache又分为

  L1Cache（一级缓存）和L2Cache（二级缓存），L1Cache主要是集成 在CPU内部，而L2Cache

  集成在主板上或是CPU上。")。

- 设置stage1 sp(r0)栈指针。

- 初始化PLL和DDR。

- 设置[TLB](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#tlb "TLB(Translation Lookaside Buffer)转换检测缓冲区是一个内存管理单元,用于改进虚拟地

  址到物理地址转换速度的缓存。

  TLB是一个小的，虚拟寻址的缓存，其中每一行都保存着一个由单个PTE(Page Table Entry,

  页表项)组成的块。如果没有TLB，则每次取数据都需要两次访问内存，即查页表获得物理地

  址和取数据。")。

- 配置[MMU](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#mmu "MMU是Memory Management Unit的缩写，中文名是内存管理单元，它是中央处理器（CPU）中

  用来管理虚拟存储器、物理存储器的控制线路，同时也负责虚拟地址映射为物理地址，以及

  提供硬件机制的内存访问授权，多用户多进程操作系统。")。

- 使能[Cache](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#cache "高速缓冲存储器，是位于CPU和主存储器DRAM（Dynamic Random Access Memory）之间，规

  模较小，但速度很高的存储器，通常由SRAM（Static Random Access Memory 静态存储器）

  组成。它是位于CPU与内存间的一种容量较小但速度很高的存 储器。CPU的速度远高于内存

  ，当CPU直接从内存中存取数据时要等待一定时间周期，而 Cache则可以保存CPU刚用过或循

  环使用的一部分数据，如果CPU需要再次使用该部分数据时 可从Cache中直接调用，这样就

  避免了重复存取数据，减少了CPU的等待时间，因而提高了系 统的效率。Cache又分为

  L1Cache（一级缓存）和L2Cache（二级缓存），L1Cache主要是集成 在CPU内部，而L2Cache

  集成在主板上或是CPU上。")。

- 使能中断。

- copy stage2到[dram](http://gitbook.guoxintech.com/SPD/training/GLOSSARY.html#dram "DRAM（Dynamic Random Access Memory），即动态随机存取存储器，最为常见的系统内存。

  DRAM 只能将数据保持很短的时间。为了保持数据，DRAM使用电容存储，所以必须隔一段时

  间刷新（refresh）一次，如果存储单元没有被刷新，存储的信息就会丢失。 （关机就会丢

  失数据）")。

- 设置stage2 sp(r0)栈指针。

- 跳转到C程序。

- boot工具：

  - 使用.boot文件，加载到ddr中运行
  - 起来之后运行serialdown命令
  - 要下载的数据通过串口作为输入，输出是下载到flash

- 正常启动：

  - BootRom(SOC)中的代码开始运行，将stage1代码拷贝到SRAM中运行
  - stage1代码进行一些配置，配置PLL、初始化spi控制器、将stage2代码从flash中拷贝到DRAM，跳转到stage2代码开始运行
  - 进入entry函数

```C
static uint32_t tx_max(spi_xfer_t *pf)
{
  /* SPIM_TXFLR:tx_fifo的数据长度 */
  uint32_t fifo_left = (pf)->fifo_len - dw_readl((pf)->regs, SPIM_TXFLR);
  /* 要发送的数据数量 */
  uint32_t xfer_left = (pf)->tx_end - (pf)->tx;

  return (fifo_left < xfer_left) ? fifo_left : xfer_left;
}

static void spi_write(spi_xfer_t *priv)
{
  uint32_t max = 0;
  uint16_t txw = 0;

  /* 当发送fifo数据满，就等待发送完成,当fifo有空闲，就继续开始发送 */
  while(tx_max(priv) == 0);
  while ((dw_readl((priv)->regs, SPIM_SR) & SR_BUSY) == SR_BUSY);
  max = tx_max(priv);
  while(max--){
    if((priv)->tx_end - (priv)->len)
      txw = *((priv)->tx);
    dw_writel((priv)->regs, SPIM_TXDR, txw);
    ++(priv)->tx;
  }
}
```

### Stage2

Stage2支持可裁剪，主要功能是从uart、net、usb、flash引导内核。

- 清BSS。
- 初始化堆（malloc）。
- 初始化uart、中断。
- 读取分区表，打印分区表。
- 配置板级。
- 进入引导模式/命令模式。
  - flash模式，读取内核引导，如果有验签，先验签在引导。
  - usb模式，从u盘读取数据，可以升级flash或者直接引导。
  - net模式，从tft盘读取数据，可以升级flash或者直接引导。

#### CMD

支持命令扩展，在这个阶段芯片所有模块都可以使用，根据实际需求增加命令。

```text
-c command list:netdown <flash addr|partition name> <file name>

  :serialdown <flash addr|partition name> <file name>

  :usbdown <flash addr|partition name> <file name>

  :serialdump <flash addr|partition name> <length> <file name>

  :flash erase <flash addr> <length>

  :flash erase <partition name>

  :flash eraseall

  :load_conf_down <config file name|merge bin name> <serialdown|netdown|usbdown> [usb bin path]

  :gx_otp read <gx otp address> <length> <file name>

  :gx_otp tread <gx otp address> <length> 

  :gx_otp write <gx otp address> <file name> 

  :gx_otp twrite <gx otp address> <hex digits string> 

  :eeprom read <eeprom address> <length> <file name>

  :eeprom write <eeprom address> <file name> 

  :compare <src_file> <dst_file>
```

# stage1 、 stage2

.boot：包括stage1和stage2

- stege1：初始化硬件(spi、uart)
  - 初始化pll(配时钟)
  - 初始化ddr
  - 拷贝stage2到ddr
  - 跳转到stage2运行

使用gdb load是把.elf加载到ddr中运行，这种方式能够启动证明stage1初始化ddr已经完成

# flash ecctest

- 方式1：
  - 擦除块
  - 读取数据，判断是否为0xff，若不是则属于使用过程位翻转
  - 关闭ecc
  - 写入伪造数据(其中test_bits为数据不为1)
  - 开启ecc
  - 读取数据查看是否为全f(即ecc是否纠正了伪造的数据)
- 方式2：
  - 擦除
  - 读取数据，判断是否存在位翻转
  - 开ecc
  - 写入带有ecc的随机数据
  - 关ecc
  - 写入伪造数据(不带ecc且test_bits位数据为0)
  - 开ecc
  - 读取数据查看是否为全f(即ecc是否纠正了伪造的数据)

写入伪造数据—> 开启ecc —> 读取数据校验 —> 写入伪造数据(bit+1) —> 开启ecc —> 读取数据校验

写入带ecc 的数据—> 关ecc —> 写伪造数据 —> 开ecc —> 读取数据校验 —> 写入伪造数据(bit+1) —> 开启ecc —> 读取数据校验

# SPI NAND状态寄存器C0

- ECCS1、ECCS0
  - 00：执行PAGE_READ(13H)命令时没有检测到错误
  - 01：检测并纠正错误
  - 10：检测到不可纠正的错误
  - 11：8位错误被检测和纠正。超过8位的错误无法纠正

# 历史版本无gxmisc

- 使用主线的gxmisc
- 回退到对应的gxmisc版本(common/gxm_version.c, .a中的信息)
- 打上历史版本gxmisc的补丁
- 提交.a

# 启动三步骤：

- rom代码启动：初始化spi控制器
- stage1：copybootloader
- stage2：entry

# makefile修改O0来开启DebugServer



# scpu 编译下载

1. 开发板：gx8008b

2. .boot：leo_mini

3. 编译：

   1. 编译.boot
      1. `make leo_mini_boot_defconfig`
      2. `make clean;make`
   2. 编译.bin
      1. `make leo_mini_gx8008b_dev_1v_defconfig `
      2. `make menuconfig`
      3. `make clean;make`

4. 下载：`sudo bootx -m leo_mini -t u -c download 0 output/spi_nor/gxscpu.bin`或者`sudo bootx -b gxscpu.boot -t u -c download 0 output/spi_nor/gxscpu.bin`

   gxscpu.boot用为更新后的.boot，leo_mini为bootx工具中自带的.boot，其中是不包含新添加的Flash信息的，所以无法正常下载，需先编译更新.boot，再使用-b的方式来烧写.bin

5. 测试读写擦命令：

   1. 使用

      ```
      sf
      ```

      命令

      1. `sf probe`
      2. `md.b 0x20027000 2048`
      3. `sf erase 0x0 0x100`
      4. `sf read 0x20027000 0x0 0x100`
      5. `sf write 0x20000000 0x0 0x100`
      6. `sf read 0x20027000 0x0 0x100`
      7. `md.b 0x20027000 0x100`
      8. `md.b 0x20000000 0x0 0x100`

   2. 使用

      ```
      flash_test comtest
      ```

      1. `make menuconfig`配置打开MTD的TEST选项
      2. COMMAND LINE中打开`flash_test`选项
      3. 重新烧录
      4. 运行`flash_test comtest`

6. 验证四倍速

   make menuconfig

   Device Drivers(legacy)  --->

   ```
    Memory Technology Device (MTD) support --->
   
      FLASH_SPI frame format (Quad speed )  --->
   ```

7. 验证XIP

   1. 0x3000 0000 / 0x5000 0000

   make menuconfig

   Device Drivers(legacy)  --->

   ```
    Memory Technology Device (MTD) support --->
   
      [*]     Support flash spi xip
   ```

# vsp编译下载

1. 开发板：gx8008b

2. 在mcu目录下进行flash信息的添加

   `mcu/drivers/flash/spinor/flash_spi.c`：有宏修饰的为leo_mini

   `mcu/drivers/flash/spinor/spi_nor.c`

3. 拷贝.config到vsp目录下(make menuconfig)

4. mcu目录下编译：`make clean;make`

5. 下载，使用scpu编译的.boot

   `sudo bootx -b ../../scpu/gxscpu.boot -t s -c download 0 output/spi_nor/mcu_nor.bin`

6. 重启开发板，验证打印信息是否能够识别新添加的Flash型号

# eCos、eCos_shell、gxmisc

- eCos是一个os

- gxmisc是驱动，为eCos提供驱动

- eCos_shell是appliocation，其中包括很多测试用例

  用户使用eCos_shell编写测试用例完成测试功能，测试用例的编写依赖于ecos内部os的接口和驱动的接口。

# DMA控制器

- ahb_dma：单独的dma ip
- axi_dma：spi ip中自带的dma ip
- leo中是用的独立的dma ip
- apus才是使用spi ip中自带的dma ip
- 访问DMA时是绕过CPU的，所以是用物理地址访问的
- 其余凡是通过CPU去访问的寄存器都是经过了MMU进行地址转换之后的虚拟地址
- MMU存在在CPU的内核中

# copy.c

- copy.c都是在ram中跑的
- ram中可以进行管脚复用配置，也可以在board_init.c中进行管脚复用配置

# 烧录.boot时无法识别flash

- 此时stage1已经起来，stage读取flash id时挂掉了
- 挂在 main.c —> flash_init —> ops.init —> gxmisc中的spinor_init()
- 读取flash id为0x0000，用逻辑分析仪查看下载过程的clk，无波形
- copy.c中的代码都运行在ram，此时spi控制器只有配置，还没用到spi控制器，只有开始读取flash id的时候才是开始用到spi控制器，已经运行到了读取id的函数，此时不管是否读到id，逻辑分析仪都应该能抓到读取id这个操作的clk电平变化

```Bash
$ sudo boot -b output/canopus-fpga-sflash.boot
Found serial: /dev/ttyUSB0
wait ROM request... please power on or restart the board...
test boot_head:
phead->magic: 626f6f74
phead->version: 100
phead->chip: 6612
phead->baudrate: 1c200

write size 162944 byte ...
[===================>]  100.00%      data transmit ok.

warning : use default dac gain = 0x1d !

warning : use default dac gain = 0x1d !

warning : use default dac gain = 0x1d !

warning : use default dac gain = 0x1d !
error: gxflash init failed.
error: flash get type failed.
error: flash get type failed.
error: flash driver switch to spinor failed.
error: flash get type failed.
Before exec the first command, failed to receive "boot>" from stb in 3s, will EXIT_FAILURE
```

# DMA链表传输

- DMA传输，总的来说就是：

  硬件上，会有对应的控制[寄存器](https://so.csdn.net/so/search?q=寄存器&spm=1001.2101.3001.7020)ctrl和配置寄存器config，

  比如你想要从[内存](https://so.csdn.net/so/search?q=内存&spm=1001.2101.3001.7020)一个地址addr传输，N个字word(32bit)的数据到设备dev上，

  那么你就要先去根据你的请求，去配置config寄存器，首先是传输方向，

  是DMA_TO_DEVICE,然后是源地址source address是你的内存地址addr，

  和目标destination address是你的dev的DATA寄存器地址，

  然后transfer size是N个，每个位宽是32bit，

  - 将源地址，目标地址，位宽，DMA传输方向设置好，
  - 整理成一个结构，专有名称叫做LLI(Link List Item)，把这个LLI设置到ctrl里面。
  - 然后去enable DMA，DMA就可以按照你的要求，把数据传输过去了。
  - 这样的DMA叫做single DMA传输，LLI中的next lli的域设置为空，表示就一个LLI要传输。
  - 如果源地址或目标地址是多个分散的地址，叫做[scatter](https://so.csdn.net/so/search?q=scatter&spm=1001.2101.3001.7020)/gather DMA，
  - 就要将这些LLI组合一下，即将第一个LLI的next lli那个域，设置成下一个LLI的地址，
  - 这样一个个链接起来，最后一个LLI的nex lli的域为空，这样设置好后，将第一个LLI的值写入到ctrl中，DMA就会自动地去执行第一个LLI的数据传输，传完后，发现next lli不为空，就找到next lli的位置，
  - 找到对应的配置，开始这个lli的数据传送，直至传完所有的数据为止。

## 通过链表操作分散/聚集

一系列链表定义了源数据区域和目标数据区域。每个 LLI 控制一个数据块的传输，然后可选地加载另一个 LLI 以继续 DMA 操作，或停止 DMA 流。第一个 LLI 被编程到 DMAC 中。

由 LLI 描述的要传输的数据（称为数据包）通常需要一个或多个 DMA 突发到每个源和目标。

[图 C.1](https://developer.arm.com/documentation/ddi0196/g/scatter-gather/scatter-gather-through-linked-list-operation?lang=en)显示了 LLI 的一个示例。一个矩形的内存必须转移到外围设备。每行数据的地址以十六进制形式在图的左侧给出。描述传输的 LLI 将从 address 连续存储`0x20000`。

**图 C.1。LLI 示例**

![img](https://secure2.wostatic.cn/static/kphYd98Ue5csPhmr3h4zNB/%E5%9B%BE%E7%89%87.png?auth_key=1675903964-5DgZCUhTvEBRPXHn9cqJVB-0-109a8c22f214128f4b4453bd22b22c7a)

存储在 的第一个 LLI`0x20000`定义了要传输的第一个数据块。这是地址`0x0A200`和之间存储的数据`0x0AE00`：

- 源起始地址`0x0A200`
- 目标地址设置为目标外设地址
- 传输宽度，字，32 位
- 传输大小，3072 字节，`0xC00`
- 源和目标突发大小，16 次传输
- 下一个 LLI 地址，`0x20010`.

存储在 的第二个 LLI`0x20010`定义要传输的下一个数据块：

- 源起始地址`0x0B200`
- 目标地址设置为目标外设地址
- 传输宽度，字，32 位
- 传输大小，3072 字节，`0xC00`
- 源和目标突发大小，16 次传输
- 下一个 LLI 地址，`0x20020`.

建立了一系列描述符，每个描述符都指向系列中的下一个。为了初始化 DMA 流，第一个 LLI`0x20000`被编程到 DMAC 中。当第一个数据包传输完毕后，会自动加载下一个 LLI。

最终的 LLI 存储在`0x20070`并包含：

- 源起始地址`0x11200`
- 目标地址设置为目标外设地址
- 传输宽度，字，32 位
- 传输大小，3072 字节，`0xC00`
- 源和目标突发大小，16 次传输
- 下一个 LLI 地址，`0x0`.

因为下一个 LLI 地址设置为零，这是最后一个描述符，并且在传输最后一项数据后禁用 DMA 通道。此时通道可能已设置为生成中断，以向 ARM 处理器指示该通道可以重新编程。

# flash 的 suspend功能

- XIP应用模式下，有中断发生要去擦写flash，但此时又需要读取flash
- 通过发送`erase suspend` 来暂停擦除动作，读取完成后，`resume erase`操作

# SPI NAND 的 ID 在定义的时候是反的

# SPI与总线

- spi的数量对应在soc内部是相同数量的ip数的
- 即外部有spi0、spi1、spi2可以用的话，soc内部就有三个spi的ip
- soc内部的ip挂载在ahb/apb总线上，对外提供spi通信接口，用户可以使用spi通信协议(总线)来连接设备进行通信
- spi总线也可以称为spi通信协议

# OS之下的设备驱动：

- 操作系统的存在给设备驱动究竟带来了什么实质性的好处？
- 简而言之，操作系统通过给驱动制造麻烦来达到给上层应用提供便利的目的
- 首先，无操作系统时设备驱动的硬件操作工作仍然是必不可少的，没有这一部分，驱动

不可能与硬件打交道。

- 其次，我们还需要将驱动融入内核。为了实现这种融合，必须在所有设备的驱动中设计

面向操作系统内核的接口，这样的接口由操作系统规定，对一类设备而言结构一致，独立于

具体的设备。

![img](https://secure2.wostatic.cn/static/mxDMvj5cW7Xy2RWzTpa2gx/%E5%9B%BE%E7%89%87.png?auth_key=1675903964-8WfpF3A8Tp3yyTreegsuC-0-54017c6547fee3fd21239e25c52ff257)

# 使用 XM25QH128A 烧写 .bin 报错：

```Bash
ERROR: xm25qh128a BT(OTP) bit was set
unlock error in file: bootmenu.c func: flash_download line: 510
```

- XM25QH128A的写保护BT位设置在OTP区域内，因此置位后不能修改为0.
- 使用未测试过的flash没有这个报错

![img](https://secure2.wostatic.cn/static/iN1yXi26SpDZoJJ9Z6PpzV/%E5%9B%BE%E7%89%87.png?auth_key=1675903964-MWUgAFfQx2HfRXaHxVU8v-0-7e45947e220dc2cc7c64d026e202e045)

# 编译 ecos_shell，运行出错

```Bash
BOOM! Creating the world...

eCosShell> sflash
[Warning]: Unsupported flash type, id: 0x1740.
/dev/flash/0/0x0 handle:0x9005ec60

flash_size:     1       [0x1]
sector_size:    1       [0x1]

##### Kernel Panic: 14 #####
  r0: 0x900ad5f8
  r1: 0x00000058
  r2: 0x9005b2e4
  r3: 0x900ad678
  r4: 0x00000014
  r5: 0x900ad638
  r6: 0x900677ec
  r7: 0x00000000
```

- 使用gdb打断点查看： `b sflash`
- gxmisc中查看打印信息：`grep "Unsupported flash type" -rn`
- 确认是使用的spi nand 初始化
- 查看`/opt/goxceed/csky-ecos/include`中的文件，env.sh中的chip无意义
- 修改`prj/spi_cramfs_jffs2/include/gxcore_ecos_module.hxx`文件

```Diff
diff --git a/ecos_shell/prj/spi_cramfs_jffs2/include/gxcore_ecos_module.hxx b/ecos_shell/prj/spi_cramfs_jffs2/include/gxcore_ecos_module.hxx
index 0a6def98..72552adf 100755
--- a/ecos_shell/prj/spi_cramfs_jffs2/include/gxcore_ecos_module.hxx
+++ b/ecos_shell/prj/spi_cramfs_jffs2/include/gxcore_ecos_module.hxx
@@ -20,8 +20,8 @@ cramfs_init_class         cramfs0    CYGBLD_ATTRIB_INIT_PRI(60360);
 //cyg_serial_init_class     gxserial   CYGBLD_ATTRIB_INIT_PRI(60600);
 //cyg_norflash_init_class   norflash0  CYGBLD_ATTRIB_INIT_PRI(60800);
 //cyg_gx_spi_nor_flash_init_class spiflash0  CYGBLD_ATTRIB_INIT_PRI(60900);
-//cyg_gx_dw_spi_nor_flash_init_class spiflash0  CYGBLD_ATTRIB_INIT_PRI(60900);
-cyg_spinand_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
+cyg_gx_dw_spi_nor_flash_init_class spiflash0  CYGBLD_ATTRIB_INIT_PRI(60900);
+//cyg_spinand_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
 //cyg_dw_spinand_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
 cyg_flashio_init_class    flashio    CYGBLD_ATTRIB_INIT_PRI(61000);
 //nand_init_class           nandflash0 CYGBLD_ATTRIB_INIT_PRI(61110);
```

- `make clean;./build flash`
- 必须`make clean;`
- `include/cyg/io/gx_dev_dw_spi_init.hxx`
- 实现一个类，并定义构造函数

```C++
class cyg_gx_dw_spi_nor_flash_init_class {
public:
  cyg_gx_dw_spi_nor_flash_init_class(void) {
    cyg_uint32 chip_type;
        chip_type = gx_chip_probe();
                switch(chip_type){
                        case 0x6705:   /* cygnus */
                        case 0x6631:   /* canopus */
                                gx_devs_dw_spi_init();
                                gx_spiflash_init_v2(&dw_spi_master);
                                diag_printf("spi flash init gx%x \n",chip_type);
                                break;
                        default:
                                gx_devs_dw_spi_init();
                                gx_spiflash_init_v2(&dw_spi_master);
                                diag_printf("spi flash not support gx%x now\n",chip_type);
                                break;
                }   
        }   
};
```

- prj/spi_crafs_jffs2/include/gxcore_ecos_module.hxx
- 创建这个类的对象，创建之后就会执行类的构造函数

```C++
//cyg_wdt_init_class        wdt0       CYGBLD_ATTRIB_INIT_PRI(60000);
//cyg_irr_init_class        irr0       CYGBLD_ATTRIB_INIT_PRI(60100);
//cyg_sci_init_class        sci0       CYGBLD_ATTRIB_INIT_PRI(60200);
//fatfs_init_class          fatfs0     CYGBLD_ATTRIB_INIT_PRI(60300);
//ntfs_init_class                 ntfs0          CYGBLD_ATTRIB_INIT_PRI( 60330);
jffs2_init_class          jffs20     CYGBLD_ATTRIB_INIT_PRI(60350);
cramfs_init_class         cramfs0    CYGBLD_ATTRIB_INIT_PRI(60360);
//minifs_init_class         minifs0    CYGBLD_ATTRIB_INIT_PRI(60361);
//romfs_init_class          romfs0     CYGBLD_ATTRIB_INIT_PRI(60370);
//ramfs_init_class          ramfs0     CYGBLD_ATTRIB_INIT_PRI(60380);
//cyg_sd_init_class         sd0        CYGBLD_ATTRIB_INIT_PRI(60400);
//cyg_av_init_class         av0        CYGBLD_ATTRIB_INIT_PRI(60500);
//cyg_serial_init_class     gxserial   CYGBLD_ATTRIB_INIT_PRI(60600);
//cyg_norflash_init_class   norflash0  CYGBLD_ATTRIB_INIT_PRI(60800);
//cyg_gx_spi_nor_flash_init_class spiflash0  CYGBLD_ATTRIB_INIT_PRI(60900);
cyg_gx_dw_spi_nor_flash_init_class spiflash0  CYGBLD_ATTRIB_INIT_PRI(60900);
//cyg_spinand_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
//cyg_dw_spinand_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
cyg_flashio_init_class    flashio    CYGBLD_ATTRIB_INIT_PRI(61000);
//nand_init_class           nandflash0 CYGBLD_ATTRIB_INIT_PRI(61110);
//cyg_gxnetdev_init_class   net0       CYGBLD_ATTRIB_INIT_PRI(61100);
//cyg_usb_init_class        gxusb      CYGBLD_ATTRIB_INIT_PRI(61200);
```

# SPI 驱动

- spi master：代表的是soc中的spi ip，`struct spi_master`对象就对应了一个spi控制器
- spi device：代表的是挂载在spi 总线上的spi从设备，可能是flash、声卡等
- dw_spi_v2.c中gx_spi_probe中的CS要进行配置的目的是为了能够切换SPI 控制器(SOC 中一般有3个SPI 控制器)

# SPI NAND oob_free

![img](https://secure2.wostatic.cn/static/vUmanaKWbWrYUgTmmkkupe/%E5%9B%BE%E7%89%87.png?auth_key=1675903964-udradt7ZXBiwVnADGy8PAt-0-c24a325a555b2ab820a33d3303f692c1)

- 有8 byte的空闲空间
- 为什么在程序里面表示只有4byte的空闲空间
  - 留作坏块表标记

# SPI NAND读UID步骤：

1. 获取状态寄存器(0Fh)
2. 设置状态寄存器(1Fh)OTP_EN位为1
3. 发送命令page read to cache(13h)，地址为10
4. 获取状态寄存器(0Fh)中c0寄存器中的OIP位为0
5. 发送命令read from cachex2(3Bh)，地址为0
6. 设置状态寄存器恢复到OTP_EN = 0的状态

通过测试：

- 江波龙
  - FS35ND01G
  - FS35SQA001G
- 华邦
  - W25N02KVZEIR
  - W25N01GVZEIT
  - W25N04KVZEIR
- 镁光
  - MT29F1G01ABAFD
  - MT29F2G01ABAGD

# gxmisc 与 gxloader：

- gxloader:
  - spi：控制器IP驱动
    - 包括如何配置(spi_probe)，类似mcu中的spi_init
    - 包括如何传输数据(spi_transfer)，类似mcu中的send_data、receive_data
  - flash：spi 外设
    - 调用spi驱动与外设通信
    - 提供测试用例
  - main.c
    - 调用flash提供的测试用例
- gxmisc：
  - 实现各类flash驱动，作为库提供给gxloader中的flash部分使用
- SPI主机驱动
  - 写在linux中，由SOC厂商编写，SPI IP不同对应的主机驱动也不同
  - `drivers/spi/spi-dw-gx-v2.c`
  - Linux内核使用spi_master结构表示SPI主机驱动：`include/linux/spi/spi.h`
  - SPI 主机驱动的核心就是申请spi_master，然后初始化spi_master，最后向Linux内核注册spi_master
    - spi_master申请与释放

```C
struct spi_master *spi_alloc_master(struct device *dev,
                                    unsigned size)
/*
 *  函数参数和返回值含义如下：
 *  dev：设备，一般是 platform_device 中的 dev 成员变量。
 *  size：私有数据大小，可以通过 spi_master_get_devdata 函数获取到这些私有数据。
 *  返回值：申请到的 spi_master。
 */

/* 
 *  spi_master 的释放通过 spi_master_put 函数来完成，
 *  当我们删除一个 SPI 主机驱动的时候就需要释放掉前面
 *  申请的 spi_master，spi_master_put 函数原型如下：
 */
void spi_master_put(struct spi_master *master)
/*
 *  函数参数和返回值含义如下：
 *  master：要释放的 spi_master。
 *  返回值：无。
 */
    - spi_master的注册与注销:
        - 当 spi_master 初始化完成以后就需要将其注册到 Linux 内核
int spi_register_master(struct spi_master *master)
/*
 *  master：要注册的 spi_master。
 *  返回值：0，成功；负值，失败。
 */
```

- SPI设备驱动
  - 写在gxmisc中，通过库的形式提供给linux
  - `drivers/spinor/linux/spinor.c`
  - Linux内核使用spi_driver结构来表示SPI设备驱动：`include/linux/spi/spi.h`
  - 当SPI设备与驱动匹配成功以后probe函数就会执行
  - spi_driver初始化，然后向Linux内核注册
  - spi_driver注册与注销函数：

```C
int spi_register_driver(struct spi_driver *sdrv)
/*
 *  sdrv:要注册的spi_driver
 *  return:0，success;负值:error
 */

void spi_unregister_driver(struct spi_driver *sdrv)
/*
 *  sdrv:要注销的spi_driver
 *  return : 无
 */
- spi_driver注册实例程序：
static int xxx_probe(struct spi_device *spi)
{
    /* 具体函数内容 */
    return 0;
}

static int xxx_remove(struct spi_device *spi)
{
    /* 具体函数内容 */
    return 0;
}

/* 传统匹配方式ID列表 */
static const struct spi_device_id xxx_id[] = {
    {"xxx", 0},
    {}
};

/* 设备树匹配列表 */
static const struct spi_device_id xxx_id[] = {
    {"xxx", 0},
    {}
};

/* spi_driver 结构 --> spi_register_driver时使用 */
static struct spi_driver xxx_driver = {
    .probe = xxx_probe,
    .remove = xxx_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "xxx",
        .of_match_table = xxx_of_match,
    },
    .id_table = xxx_id,
};

static int __init xxx_init(void)
{
    return spi_register_driver(&xxx_driver);
}

static void __exit xxx_exit(void)
{
    spi_unregister_driver(&xxx_driver);
}

module_init(xxx_init);
module_exit(xxx_exit);

static struct spi_driver xxx_driver = {
    .probe = xxx_probe,
    .remove = xxx_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "xxx",
        .of_match_table = xxx_of_match,
    },
    .id_table = xxx_id,
};

static int __init xxx_init(void)
{
    return spi_register_driver(&xxx_driver);
}

static void __exit xxx_exit(void)
{
    spi_unregister_driver(&xxx_driver);
}

module_init(xxx_init);
module_exit(xxx_exit);
```

- SPI设备和驱动匹配过程
  - SPI设备和驱动匹配是通过SPI总线来完成的
  - drivers/spi/spi.c

```C
static int spi_match_device(struct device *dev,
                            struct device_driver *drv)
{
    const struct spi_device *spi = to_spi_device(dev);
    const struct spi_driver *sdrv = to_spi_driver(drv);
    
    /* 用于完成设备树设备和驱动匹配，
       用于比较SPI设备节点的compatible属性
       和of_device_id中的compatible属性是否相等 */
    if (of_driver_match_device(dev, drv))
        return 1;
        
    /* 用于ACPI形式的匹配 */
    if (acpi_driver_match_device(dev, drv))
        return 1;
        
    /* 用于传统的、无设备树的SPI设备和驱动匹配过程 */
    if (sdrv->id_table)
        return !!spi_match_id(sdrv->id_table, spi);
      
    /* 比较spi_device中modalias成员变量
      和device_driver中的name成员变量是否相等 */
    return strcmp(spi->modalias, drv->name) == 0;
}
```

- SPI设备数据收发处理数据流程
  - 当SPI设备向Linux内核注册成功spi_driver以后就可以使用SPI核心层提供的API函数来对设备进行读写操作。
  - spi_transfer结构体用于描述SPI传输信息，其中：tx_buf保存要发送的数据，rx_buf保存要接收的数据，len是要传输的数据长度，由于SPI是全双工通信，因此在一次通信中发送和接收的字节数都是要一样的，所以就不必区分发送长度和接收长度
  - spi_transfer需要组织成spi_message
  - 在使用spi_message之前需要对其进行初始化，初始化后需要将spi_transfer添加到spi_message队列，spi_message准备好以后可以进行数据传输了，可以使用同步(阻塞等待传输完成)/异步传输(不需阻塞等待SPI数据传输完成，传输完成后自动调用complete回调)

```C
/*
 * m:要初始化的spi_message
 * return:无
 */
void spi_message_init(struct spi_message *m)


/*
 * t:要添加到队列中的spi_transfer
 * m:spi_trnasfer要加入的spi_message
 * return:无
 */

void spi_message_add_tail(struct spi_transfer *t,
                         struct spi_message *m)
                         
                         
/*
 *  description：阻塞等待 SPI 数据传输完成
 *  parameter - spi:要进行数据传输的spi_device
 *  parameter - message:要传输的spi_message
 *  return : 无
 */
int spi_sync(struct spi_device *spi, struct spi_message *message)

/* 非阻塞等待 SPI 数据传输完成，
   异步传输需要设置spi_message 中的 complete 回调函数 */
int spi_async(struct spi_device *spi, struct spi_message *message)
```

# FLASH 支持列表更新

- flash 支持列表随着版本发布一起发布出去
- flash 支持列表中 v3.0.0 以 release 分支的支持列表为准

# Vivado启动方法：

```Bash
cd /home/tanxzh/gx/Xilinx/Vivado_Lab/2020.3/bin
sudo ./vivado_lab
```

# 上溢和下溢

在调试芯片的某个外设的时候，如果该外设带有FIFO，那么一般情况下都有FIFO上溢和FIFO下溢的错误标志位，用于表示驱动对外设的FIFO操作是否正确，下面就是FIFO上溢和FIFO下溢对应的概念：

- FIFO上溢：写满fifo后继续写则导致上溢;
- FIFO下溢：读空fifo后继续读则导致下溢;

# DMA和 MMU

- MMU：内存管理单元，能够将虚拟地址转换成物理地址
- DMA：全称Direct Memory Access，即直接存储器访问。
  - DMA传输将数据从一个地址空间复制到另一个地址空间，提供在外设和存储器之间或者存储器和存储器之间的高速数据传输。当CPU初始化这个传输动作，传输动作本身是由DMA控制器来实现和完成的。DMA传输方式无需CPU直接控制传输，也没有中断处理方式那样保留现场和恢复现场过程，通过硬件为RAM和IO设备开辟一条直接传输数据的通道，使得CPU的效率大大提高。
- CPU 访问的地址都是虚拟地址
  - 访问寄存器的时候是物理地址
  - 访问内存的时候是虚拟地址

# 编译、链接

- 编译器在编译程序时，以源文件为单位，将一个个源文件编译生成一个个目标文件。在编译过程中，编译器都会按照默认规则(.bss/.data/.text)，将函数、变量分别放在不同的section中，最后将各个section组成一个目标文件。
- 链接器会将编译生成的各个目标文件组装合并、重定位，生成一个可执行文件。



### 链接

- SECTION：`SECTIONS` 命令告诉链接器如何把输入节映射到输出节，并如何把输出节放入到内存中。

- LMA：load memory address 也就是程序和数据在bin文件中的存储地址

- VMA：virtual memory address 也就是data和bss段在RAM中的运行地址

  - 此后用户在执行这些代码的时候，是要把代码搬运到对应的 VMA 地址才能正常运行的
  - 例如：Stage1 代码的 VMA 放在 SRAM，Stage2 代码的 VMA 放在 DDR，那么 ROM 会拷贝 Stage1 到 SRAM 执行，Stage1 执行的时候会拷贝 Stage2 到 DDR 执行。

- MEMORY：`MEMORY` 命令描述目标平台上内存块的位置和长度，用来描述哪些内存区域可以被链接器使用，哪些内存区域是要避免使用的。用户就可以在链接脚本中把节分配到特定的内存区域。链接器会基于内存区域设置节的地址，如果超出大小，会提示警告信息。链接器不会为了适应区域大小而修改节的排布。

- 输出节区域：可以通过 `> REGION` 的方式把一个节赋给前面已经定义的一个内存区域

  - 以下示例用于将 ROM 节的 VMA 地址放在 rom 区域

  ```
  MEMORY {
  	rom: ORIGIN = 0x1000, LENGTH = 0x1000
  }
  
  SECTION {
  	ROM: {
  		*(.text)
  	} > rom
  }
  ```

- 参考资料：

  - http://www.51testing.com/html/13/186213-809596.html
  - https://github.com/cisen/blog/issues/887





# GDB 查看连续数组

```
p/x data8[0]@10
```

查看数组 data8[0]开始连续的10个数组内容

# Uboot 重置环境变量

- 由于uboot的修改环境变量之后使用`saveenv`即将环境变量写入到flash中了，有时需要重置环境变量

```text
env default -a
```

# Uboot 查看内存

```
md.b 0x100000 0x100
```

- 查看 `0x100000`内存地址开始的0x100 个字节

# 在Linux中，不能直接操作基地址，需要使用ioremap()映射。



# Cygnus 开启 nor flash 四倍速

* gxmisc：
  * `ids.c`中确保配置了四倍速选项
* gxloader：
  * `conf/chip/board`中使能`QUAD`宏	
  * `board/xxx_init.c`中配置引脚复用



# Vim 复制文件到系统剪切板

* 复制整个文件到系统剪贴板：`% y +`
* 复制指定行到系统剪切板：`l:[start_line_number],[end_line_number]y +`， 
  * 例如，要复制第5到第10行的内容到系统剪贴板，可以使用命令： `:5,10y +`



# leo、leo_mini、grus 驱动梳理

## leo

* leo 用的是老的 designware ip，没有四倍速、xip
* leo 用的驱动是 `drivers/legacy/mtd/spinor/spinor.c`



## leo_mini

- leo_mini 有四倍速、xip
- leo_mini 用的驱动是 `drivers/legacy/mtd/spinor/flash_spi.c`



## grus

- grus 是内封的 flash，目前只支持 puya 的几颗小容量 flash
- grus 用的驱动是 `drivers/legacy/mtd/spinor/flash_spi.c`



# linux 使用 dd 命令不显示速度

- 需要在编译busybox 时打开`coreutils/dd/CONFIG_FEATURE_DD_THIRD_STATUS_LINE=y`选项
- 使用 release 的 buildroot 编译时：
  - 在 buildroot 目录下使用 `make help` 查看编译帮助
  - 打开 `make busybox_menuconfig`，选中`coreutils/dd/CONFIG_FEATURE_DD_THIRD_STATUS_LINE=y`
  - 保存退出
- 重新使用`make.sh`脚本编译



# hexdump 和 xxd

- hexdump：主要用来查看`二进制`文件的十六进制编码

- xxd：用于读取文件内容输出其二进制或十六进制的内容，如果没有指定outfile参数，则把结果显示在屏幕上，如果指定了outfile，则把结果输出到outfile中



# gxloader 使用ubifs的时候要开启ubifs的宏

- 由于gxloader中打开了ubifs的宏，就相当于修改了cmdline，指定root=/dev/xxx 
- 读文件系统是在kernel的驱动里读的
- gxloader 中 ubifs 的驱动并不是用来读取 文件系统的





# 堆栈概念

- 堆栈通常被用来存储函数调用的信息，例如返回地址、函数参数、局部变量等。
- 堆栈的实现通常使用堆栈指针来维护堆栈空间。
  - 堆栈指针是一个指向堆栈顶部的指针，当有新的数据需要压栈时，堆栈指针会向下移动，当需要弹出堆栈中的数据时，堆栈指针会向上移动。
  - 当函数被调用时，函数的返回地址和函数参数等信息会被压入堆栈中，同时一个新的堆栈帧会被创建。堆栈帧是一个包含了函数局部变量、函数参数和返回地址等信息的内存区域，它随着函数调用的开始和结束而动态地压入和弹出堆栈。
  - 当函数执行完毕时，堆栈帧会被弹出堆栈，同时堆栈指针也会回到原来的位置，以便从返回地址继续执行代码。
  - 堆栈指针在函数调用过程中是实时变化的。当一个函数被调用时，堆栈指针会向下移动，将返回地址和函数参数等数据压入堆栈中。随着函数执行过程中局部变量和临时数据等的不断产生，堆栈指针会继续向下移动。当函数执行完毕时，栈指针会向上移动，将堆栈中的数据弹出，恢复原来的状态。
  - 压栈和出栈的操作有CPU自动完成。

- 压栈过程中的局部变量和临时数据保存到了哪里？
  - 函数调用过程中，局部变量和临时数据都是保存在堆栈空间的，当函数调用时，系统会为该函数分配一段固定大小的堆栈空间，用于存储该函数的局部变量、临时数据和其它与函数调用相关的数据。



# 编译选项 `-static`

- 编译时使用`-static`参数表示使用静态编译，即编译时将库的代码全部包含在可执行文件中

- 不添加`-static`参数表示使用动态编译，即程序在运行的时候动态加载共享库

- 可以通过`file` 命令查看编译出的可执行文件采用的什么编译方式：

  ```shell
  -> % file hello         
  hello: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=052027a0a045abf419a7c865f9f0224ee798365a, for GNU/Linux 3.2.0, not stripped
  
  -> % file hello_static 
  hello_static: ELF 64-bit LSB executable, x86-64, version 1 (GNU/Linux), statically linked, BuildID[sha1]=9ca64b2784268cc113fea44f0fb1022756af0540, for GNU/Linux 3.2.0, not stripped
  ```

  编译后生成了.o文件，然后把所有的.o都链接到 flash_utils 上，怎么用 flash_utils

  把那些静态链接的文件拷贝过去之后可以直接运行





# GDB 查看程序挂在哪里

- 将程序烧到flash，然后重启或不重启

- 打开 `debugserver`

  ```
  jtag/jlink
  ```

  

- 打开 gdb，不打开 load：

  ```shell
  jtag/jlink
  # load
  
  arm-none-eabi-gdb loader.elf
  ```

  

# 包含头文件时不知道包含路径？

- 查看编译时 -I 指定的路径，然后包含相对路径(包含库也是一样的)



# 内存屏障、内存栅栏

- 内存屏障，也称内存栅栏等，是一类同步屏障指令，是CPU或编译器在对内存随机访问的操作中的一个同步点，使得此点之前的所有的读写操作都执行后才可以开始执行此点之后的操作
  - 就类似喝茶时需要先把水煮开(限定条件)，然后再泡茶，而这一整到流程都是限定特定环节的先后顺序(内存屏障)



## 为什么有内存屏障？

![7ae28b21275560265328784c0d92a8ea.png](https://img-blog.csdnimg.cn/img_convert/7ae28b21275560265328784c0d92a8ea.png)

- 现在cpu存在多级缓存和多核场景，为保证读取到的数据一致性以及并行运行时所计算出来的结果一致，在硬件层面实现一些指令来保证执行指令的先后顺序。
  - 例如上图：双核 cpu，每个核都有独立的 L1、L2 cache，而 cache 和 cache 之间需要保证数据的一致性所以这里才需要添加屏障来保证数据的一致性。三级 cache 为 cpu 共享，最后是主内存，所以这些存在交互的 cpu 都需要通过屏障手段来保证数据的一致性



## 内存屏障解决了什么问题？

- 内存配置主要解决重排序导致的内存乱序访问问题，由于 cpu 存在多级缓存，所以为了保障一些程序指令的顺序执行不会因为重排优化导致乱序的情况，所以硬件厂商引入了读、写屏障，主要就是保证了执行的顺序能够根据需要的特定场景进行标识防止重排顺序。



## DMB/DSB/ISB 关系

- DMB 与 DSB 的区别在于 DMB 可以继续执行之后的指令，只要这条指令不是内存访问指令
- DSB 不管后面是什么指令，都会强迫 CPU 等待它之前的指令执行完毕
- ISB 不仅做了 DSB 所做的事情，还将流水线清空





# DMA 与 cache 的关系

- 背景知识：
  - cache：为了 cpu 快速访问数据，先把数据从内存缓存到 cache 中
  - dma：替代 cpu 来读写数据
  - 刷 cache：将 cache 中的数据刷新到 内存，也叫 clean cache
  - 无效 cache：无效掉 cache 中的数据，也叫 invalidate cache

- 直接说结论：dma 写内存的时候需要`无效 cache`，dma 读内存的时候需要 `刷 cache`

  - dma 写内存：dma 将数据从源地址搬运到 内存，这个操作是不经过 cache 的，所以 内存中的数据是最新的，如果 cpu 需要访问这些数据，而这些数据之前被放到了 cache 中，那么访问的数据可能是错的，所以要让 cache 和 内存中的数据保持一致，由于内存中的数据是最新的，所以需要无效掉 cache，cpu要用的时候从内存取到cache，然后根据需要放到 cache 中。

  - dma 读内存：dma 将内存的数据搬运到目的地，这个操作中 cache 中的数据可能比内存中的数据要新(见下注解)，所以要先刷 cache，把cache中的数据刷到内存，然后 dma 再去操作内存中的数据，保证数据正确。

    - 注：引用自 chatgpt

      - 在具有缓存的环境下，CPU操作内存中的数据通常首先操作的是缓存中的数据，而不是直接操作内存。这是由于缓存的快速访问速度比内存更快，可以提高数据访问的效率。
      - 当CPU执行读操作时，它首先会检查缓存中是否存在所需的数据。如果缓存中存在数据（即缓存命中），CPU会直接从缓存中读取数据，而不需要去访问内存。这称为缓存命中（Cache Hit）。
      - 如果缓存中不存在所需的数据（即缓存未命中），CPU就需要从内存中获取数据。这时，CPU会发出一个请求，将数据从内存读取到缓存中，并将所需的数据返回给CPU。在这个过程中，CPU会等待数据从内存中加载到缓存中。为了确保数据的一致性，缓存中的修改数据在读取新数据之前通常会被写回到内存中。

      - 当CPU执行写操作时，它也会首先写入缓存中的数据，而不是直接写入内存。CPU会将修改后的数据写入缓存，并且在适当的时候将数据写回到内存中。这可以提高写入操作的速度，因为缓存的写入操作通常比内存更快。

      - 然而，为了保持数据的一致性，当需要将数据持久化到内存或与其他设备共享数据时，需要进行刷缓存的操作。刷缓存的操作会将缓存中被修改的数据写回到内存中，以确保数据的一致性。

      - 需要注意的是，缓存的策略和实现可能因处理器和系统架构而有所不同。不同的处理器和缓存体系结构可能具有不同的缓存一致性协议和刷缓存机制。因此，在具体的系统中，刷缓存的具体行为可能会有所差异。



# Cache 一致性问题

- 所谓的 cache 一致性问题，主要指的是由于 cache 存在时，当有多个 master(典型的如 多核、 dma、pcie等)访问同一块内存时，由于数据会缓存在 cache 中而没有更新实际的物理内存，导致的问题。
- 一致性问题解决方案：
  - 所有的共享存储器都定义为共享属性，共享意味着需要硬件保证一个内存位置中的内容对一定范围内可访问该位置的多个处理器是一致的
  - 通过软件进行cache的维护，例如：使用 `cache invalidate` 和 `cache clean` 维护





# CPU 乱序执行

- 程序中每行代码的执行顺序，有可能会被编译器和 `cpu`根据某种策略，给打乱掉，目的是为了性能的提升，让指令的执行能够尽可能的并行起来。
- 知道指令的乱序策略很重要，这样我们就能够通过 `barrier` (内存屏障) 等指令，在正确的位置告诉 cpu 或编译器，这里我可以接收乱序，那里我不接受乱序等等。从而，能够在保证代码正确性的前提下，最大限度的发挥机器的性能。

- 在 cpu 中为了能够让指令的执行尽可能地并行，发明了流水线技术。但是如果两条指令的前后存在依赖关系，比如数据依赖、控制依赖等，此时后一条语句就必须等到前一条指令完成后，才能开始执行。

- cpu 为了提高流水线的运行效率，会做出比如：

  - <font color=red>对无依赖的前后指令做适当的乱序和调度</font>
  - 对控制依赖的指令做分支预测
  - 对读取内存等的耗时操作，做提前预读

  以上操作都可能会导致指令乱序执行。



- c 语言中的 volatile 关键字：
  - 易变性：volatile 告诉编译器，某个变量是易变的，当编译器遇到这个变量的时候，只能从变量的内存地址中读取这个变量，不可以从 cache、寄存器、或其它任何地方读取。
  - 顺序性：两个包含 volatile 修饰的指令，编译后不可以乱序。<font color=red>注意是编译后不可以乱序，但是在执行的过程中还是有可能乱序的，这点需要其它机制来保证，例如 memory-barriers(内存屏障)。</font>>





# 可重入函数





# 终端十六进制颜色：#262626



# 安装 lunarvim 过程报错：

- `cp: 无法创建普通文件'/home/tanxzh/.local/share/icons/hicolor/16x16/apps/lvim.svg': 权限不够`

  - 查看当前用户是否具有对此文件夹的操作权限：

    ```
    ls -ld /home/tanxzh/.local/share/icons/hicolor/16x16/apps/
    ```

  - 如果此目录的所有者是 `root`用户，则更改目录的所有者为当前用户

    ```
    sudo chown -R tanxzh:tanxzh /home/tanxzh/.local/share/icons/hicolor/16x16/apps/
    ```

    

# ##### TODO：确认 ##### Canopus 6631SH1D 网络下载引脚复用

```diff
@@ -94,8 +94,8 @@ struct mulpin_config_s mulpin_table[] = {
         {4,        0},          //NC          |GPIO04(PMUPORT04) |RMTXD[1]     |SDIO_DATA[2] |ADCQDATA[3] |           |            |            |HDMI_CH1_G[8]   |
         {5,        0},          //NC          |GPIO05(PMUPORT05) |RMTXD[0]     |SDIO_DATA[3] |ADCQDATA[2] |           |            |            |HDMI_CH1_G[7]   |
         {6,        0},          //NC          |GPIO06(PMUPORT06) |RMCLK        |SDIO_PWR     |ADCQDATA[1] |           |            |            |HDMI_CH1_G[6]   |
-        {7,        0},          //NC          |GPIO07(PMUPORT07) |RMRXD[1]     |SDIO_CD      |ADCQDATA[0] |UART1_CTS  |MD          |            |                |
-        {8,        0},          //NC          |GPIO08(PMUPORT08) |RMRXD[0]     |SDIO_WP      |ADCCLKOUT   |UART1_RTS  |MDC         |            |                |
+        {7,        5},          //NC          |GPIO07(PMUPORT07) |RMRXD[1]     |SDIO_CD      |ADCQDATA[0] |UART1_CTS  |MD          |            |                |
+        {8,        5},          //NC          |GPIO08(PMUPORT08) |RMRXD[0]     |SDIO_WP      |ADCCLKOUT   |UART1_RTS  |MDC         |            |                |
         {9,        1},          //NC          |GPIO09(PMUPORT09) |UART1TX      |SUARTTX      |AUARTTX     |           |            |            |                |
         {10,       1},          //NC          |GPIO10(PMUPORT10) |UART1RX      |SUARTRX      |            |           |            |            |                |
         {11,       1},          //NC          |GPIO11(PMUPORT11) |IR           |             |            |           |            |            |                |
@@ -166,8 +166,8 @@ struct mulpin_config_s mulpin_table[] = {
         {74,       1},          //NC          |GPIO74            |HPD          |             |            |           |            |            |                |
         {75,       1},          //NC          |GPIO75            |SPDIF        |             |            |           |            |            |                |
         {76,       0},          //NC          |GPIO76            |PHY_CLK      |             |            |           |            |            |                |
-        {78,       0},          //NC          |GPIO78            |RMCRSDV      |             |            |           |            |            |                |
-        {81,       0},          //NC          |GPIO81            |RMTXEN       |             |            |           |            |            |                |
+        {78,       1},          //NC          |GPIO78            |RMCRSDV      |             |            |           |            |            |                |
+        {81,       1},          //NC          |GPIO81            |RMTXEN       |             |            |           |            |            |                |
         {82,       1},          //NC          |GPIO82            |RMTXD[1]     |             |            |           |            |            |                |
         {83,       1},          //NC          |GPIO83            |RMTXD[0]     |             |            |           |            |            |                |
         {84,       1},          //NC          |GPIO84            |RMCLK        |             |            |           |            |            |                |
```



https://git.nationalchip.com/gerrit/#/c/117138/1/board/vega/board-6631SH1D/board-init.c





# nand flash

- erase：

  - chiperase:
    - nand_erase_opts:
      - 擦坏块标记：将 chip->bbt 直接 free 掉，chip->bbt = NULL
      - 擦数据区：循环中检测当前块是否是坏块，若是则跳过，调用 nand_erase 擦除一个block，若擦除失败查看当前块是否是坏块，或进行标记坏块
      - nand_erase：
        - 检查地址必须要块对齐、长度也要块对齐，擦除大小不能超过 flash 大小
        - 使用控制器发送 erase 命令给 flash，控制器调用 chip->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page); chip->cmdfunc(mtd, NAND_CMD_ERASE2, -1, -1);然后等待 flash ready
        - nfc_v0:
          - 控制器发命令： 拉低 CLE，调用 write_cmd(dev, command) 发送命令，拉低 ALE，调用 write_addr(dev, culumu) 发送地址，先发列地址，再发行地址
          - write_cmd：regs->cmd_reg = cmd，然后等状态ready
          - write_addr：regs->addr_reg = addr，然后等状态ready
        - nfc_v1:与v0一致，只有write_cmd和write_addr 的实现不一致
          - write_cmd：写CMD(宏)到TYPE寄存器，写cmd到数据/地址寄存器，写START寄存器1开始，等待状态ready
          - write_addr：发送ADDR(宏)到TYPE寄存器，发CMD到数据/地址寄存器，写START寄存器1开始，等待状态ready
        - 如果是状态错误或者某些命令的时候，需要适当的添加延时
      - 若将坏块标记擦除了，便调用 chip->scan_bbt 重新构建一张坏块表
  - scruberase：nand_erase_remap：将地址和长度都进行block对齐，调用 nand_erase_opts
    - nand_erase_opts：跟上面一样的流程，多了一个 擦除 bbt 和 重构扫描构建 bbt 的过程
  - block_erase：nand_erase_remap、nand_erase_opts，跟上面的流程一样
- pageprogram：

  - 先判断当前的地址是否为页的首地址，若不是，则先把不满一页的写完，再整页整页写
  - 不满 page 的数据，发送写 page 指令
    - 这种情况，主机知道要发送的数据多少，而从机接收数据都是在 clk 的边沿上接收的，主机将要发送的数据发送完之后就不发 clk 了，从机接收不到 clk 也就不会继续写数据
  - 调用 nand_write_skip_bad 根据地址和长度写入数据
    - nand_write_skop_bad：若为 YAFFS 系统的话，则blocksize就等于 main+oob，否则一般的blocksize就只有 main 区域大小，调用 check_skip_len(nand, offset, *length)，检查当前地址是否为坏块，调用 nand_write ，如果写失败，则标记该块为坏块，循环写入
      - check_skip_len：检查要写的这段长度中是否存在坏块
      - nand_write：传递 databuf，无oobbuf，调用 nand_do_write_ops
        - nand_do_write_ops：
          - nfc_v1：若oobbuf不为空，则调用 nand_fill_oob 组织好oob数据，如果模式为 PLACE_OOB，就是同时要写oob和数据，就把刚刚组建好的oob buffer全部置为0xff，然后调用 chip->ecc.write_oob 写oob区域，若databuf 不为空，根据 flag 来确认是否开启 ECC，调用 chip->write_data 来写入数据
            - nand_fill_oob：若 ops->mode 为 PLACE_OOB或RAW模式，则直接将oob数据拷贝到要写入的buffer处，若为 AUTO_OOB 模式，找oob中空闲的区域， 最后也是将 oob 数据拷贝到 buffer 处
            - chip->ecc.write_oob：使用HW模式，调用nand_write_oob
              - nand_write_oob：调用 chip->cmdfunc 调用 gxnfc_nand_command调用 write_cmd 发送 0x80(写page)，列地址、行地址，调用 chip->write_buf，调用chip->cmdfunc发送 0x10(flash 要求的 0x80  地址 0x10)指令，等待 ready
                - chip->write_buf：控制器发写使能到TYPE寄存器，然后循环发数据到数据/地址寄存器，写1到启动寄存器，等待 ready
            - chip->write_data：控制器实现，调用 gxnfc_nand_write_data
              - gxnfc_nand_write_data：先写不满 page，然后再page写，调用接口nandflash_programpage来写
                - nandflash_programpage：先失能控制器，根据ecc的位数来配置TYPE寄存器，写page地址到ROW_ADDR寄存器，写配置寄存器，使能控制器，数据写到寄存器，把要传输的长度写到XFER_SIZE寄存器，等待ready
          - nfc_v0：根据要写到的地址计算行地址和列地址，读取flash的状态寄存器WP位是否为1，循环调用写oob和写data，如果有oobbuf，则将oob数据填充到buffer，调用chip->write_page接口，更新当前的写入长度
            - chip->write_page：v0.c没有实现这个接口，由base.c中实现的nand_write_page：写0x80,列地址(哪一个page)，行地址(0x00 page起始)，在调用 chip->ecc,write_page 写数据，发一个0x10，等待ready
              - chip->ecc.write_page:nand_write_page_hwecc：先配置好控制器的 ECC 寄存器，然后调用 chip->write_buf 将数据写到 flash，控制器会根据数据生成ecc码，将ecc码从控制器中读出来保存到buffer，然后调用 chip->write_buf 将 ecc码 写入到 flash 的oob 区域
              - chip->write_buf：将buf写入控制器的 buf 寄存器
  - write_oob:
    - 传递的参数是 oobbuf， 调用`nand_do_write_oob`，先把 oobbuf 按照 oob 区域排布好到 oobbuffer，然后调用控制器的驱动将buffer写到 oob 区域
  - write_data：no_ecc
    - 传递的参数是 databuf，调用 `nand_do_write_ops`，涉及两种方式：nfc_v0 、nfc_v1
      - nfc_v1：依次判断传递的参数是 oob 或 data，如果有 oobbuf，就填充好 oob 信息，然后写到 oob 区域；如果有 databuf，看是否需要关闭 ecc，然后调用 chip->write_data 接口写入
      - nfc_v0：判断是否无 oobbuf，如果没有 oobbuf 的话，就直接将 oob 信息全部填充为 0xff，然后调用 chip->write_page 接口，在接口中会将填充好的 oob 信息和 databuf 全部写入，由于 oob 信息是全 0xff，所以写入数据后还是原数据
- read：

  - read_data：先读不满一个page的，再整页整页的读
    - 这种方式是不读 oob 区域的，只读 main 区域的数据，nand_do_read_ops
      - nfc_v1.c：依次判断传入的参数，有databuf，则读databuf，有oobbuf，则读oobbuf
      - nfc_v0.c：判断是否开启 ecc 读取的标志，开启ecc读就调用 chip->ecc.read_page，关闭ecc读就调用chip->ecc.read_page_raw，再读取 oob nand_transfer_oob
  - read_oob：
    - 只传递oobbuf下去，mode=RAW，调用 nand->read_oob(nand_read_oob)
      - nand_read_oob:若传递的参数包含databuf，则调用 nand_do_read_ops，若不包含databuf，则调用 nand_do_read_oob
        - nand_do_read_oob：判断 ops->mode 若为AUTO_OOB，则长度是ecc.layout->oobavail，否则就是mtd->oobsize，调用 chip->ecc.read_oob，调用 nand_transfer_oob
          - nfc_v0:
            - chip->ecc.read_oob：控制器驱动实现，控制器先发送 0x00，列地址是page_size，行地址，然后读取数据寄存器的数据到临时buffer，然后memcpy到oob_buffer
            - nand_transfer_oob：把oob_buffer中的数据拷贝到buf中
          - nfc_v1:
            - chip->ecc.read_oob：控制器驱动实现，控制器配置好TYPE寄存器，发列地址和行地址，命令控制器自己会发，使能控制器即可，读数据寄存器到临时buffer，然后memcpu到 oob_buffer
        - nand_do_read_ops：
          - nfc_v1:若databuf存在，在调用 chip->read_data 接口读取 data 数据，若oobbuf存在，则调用 chip->read_oob 接口、nand_transfer_oob 接口读取 oob 数据
            - chip->read_data：控制器驱动实现 gxnfc_nand_read_data，先调用 nandflash_readpage 读不满一个page的，再循环调用 nandflash_readpage 按 page 循环读取
              - nandflash_readpage：先读取控制器的状态寄存器，等待ready，配置一些寄存器，根据 oob->size 配置好 TYPE 寄存器，写行地址，写一些配置到寄存器，传输data到控制器内部的DMA控制器，配置长度、结束页、配置开始，等ready
          - nfc_v0：先发 0x00，再发page行地址，列地址是0，如果ops->mode是RAW的话，就使用chip->ecc.read_page_raw读取数据，否则调用chip->ecc.read_page读取数据，剩余不对齐的数据再单独传一次，有oobdata存在的时候则需要再调用 nand_transfer_oob 来将 oobbuffer 中的数据读取回来
            - chip->ecc.read_page_raw:调用 chip->ecc.hwctl 关掉ecc，然后调用 chip->read_buf 读 buffer 和 oob
              - chip->read_buf：从控制器的 RX_FIFO 中读取数据
            - chip->ecc.read_page：调用 nand_read_page_hwecc，检测 sector的擦除标记，若全为true，则不读取page数据，否则就先发0x05(random data output)，开启ecc，调用 chip->read_buf，等 ecc ready，根据read_buf读到的ecc码对数据进行校验，
              - chip->read_buf：调用 read_data_bulk，从控制器的 RX_FIFO 中读取数据到buffer



## nfc_v1

- 开 dma，并且是 linux：使用 dma + 中断 + 信号量

- 不开 dma，无论是否是 linux：使用 cpu 四字节取数据

  

- `gxnfc_nand_read_data`：

  ```c
      data：	|       ...|..........|..........|....      |
  	要读的数据不是 page 对齐，先读一个 page 到 buffer，将 buffer 后部分有用的数据拷贝回去
  	再读中间整个 page 的数据，linux 系统下要读到的数据地址不是 cache_line 对齐；非 linux 系统下要读到的数据地址不是 4 字节对齐，先读到临时 buffer，然后拷贝回去。对齐的话就直接读。
      再读后面的数据，读一个 page 到临时 buffer，把临时 buffer 中前部分有用的数据拷贝回去
      
  static int gxnfc_nand_read_data(struct gx_mtd_info *mtd, unsigned int from, unsigned char *data, unsigned int len, unsigned int* readlen)
  {
  	struct gx_nand_dev *dev = mtd_to_dev(mtd);
  	uint8_t *readbuf = NULL;
  	int page_size, i, ret = 0;
  	unsigned int row, col, remain;
  	int leading, major, major_rows, trailing;
  	struct gx_nand_chip *chip = &dev->chip;
  
  	*readlen = 0;
  	page_size = mtd->writesize;
  	row = from / page_size;
  	col = from % page_size;
  	remain = page_size - col;
  	leading = min(remain, len);
  	if (leading != page_size) {
  		readbuf = chip->buffers->databuf;
  		ret = nandflash_readpage(mtd, row, readbuf, page_size);
  		memcpy(data, readbuf + col, leading);
  		from += leading;
  		data += leading;
  		len  -= leading;
  		*readlen += leading;
  		row++;
  		if(ret < 0)
  			goto final;
  	}
  
  	major_rows = len / page_size;
  	major = major_rows * page_size;
  	if (major) {
  #if NAND_DMA_ENABLE
  #ifdef CONFIG_GX_LINUX
  		/* 控制器要求4字节对齐, Linux系统cache line对齐 */
  		if (IS_ALIGNED((unsigned int)data, ARCH_DMA_MINALIGN)) {
  #else
  		/* dma模式要求4字节对齐，gxloader要求cache line对齐 */
  		if (((uint32_t)data % CACHE_LINE_SIZE) == 0x00) {
  #endif
  #else
  		/* cpu模式要求4字节对齐 */
  		if (((uint32_t)data & 0x3) == 0x00) {
  #endif
  			ret = nandflash_readpage(mtd, row, data, major);
  			data += major;
  			row  += major_rows;
  			from += major;
  			len  -= major;
  			*readlen += major;
  			if(ret < 0)
  				goto final;
  		} else {
  			readbuf = chip->buffers->databuf;
  			for (i = 0; i < major_rows; i++) {
  				ret = nandflash_readpage(mtd, row, readbuf, page_size);
  				memcpy(data, readbuf, page_size);
  				data += page_size;
  				*readlen += page_size;
  				row++;
  				if(ret < 0)
  					goto final;
  			}
  			from += major;
  			len  -= major;
  		}
  	}
  
  	trailing = len;
  	if (trailing) {
  		readbuf = chip->buffers->databuf;
  		ret = nandflash_readpage(mtd, row, readbuf, page_size);
  		memcpy(data, readbuf, trailing);
  		from += trailing;
  		len  -= trailing;
  		*readlen += trailing;
  		if(ret < 0)
  			goto final;
  	}
  
  final:
  	return ret;
  }
  ```

  - `nandflash_readpage`

    ```c
    static int nandflash_readpage(struct gx_mtd_info *mtd, unsigned int page, unsigned char *data, unsigned int len)
    {
    	struct gx_nand_dev *dev = mtd_to_dev(mtd);
    	struct gx_nand_chip *chip = &dev->chip;
    	uint32_t     regs = (uint32_t)dev->regs;
    	uint32_t     ctrl_config, reg_val;
    	uint32_t page_size = mtd->writesize;
    	struct gx_mtd_ecc_stats stats = mtd->ecc_stats;
    	int end_page;
    	int max_bits_err;
    #if !NAND_DMA_ENABLE || !defined(CONFIG_GX_LINUX)
    	unsigned int bkpt_continue, bkpt_status;
    	int err_page, subpage_ofs, fifo_count;
    	int subpage_num = mtd->writesize / GX_NAND_512_PAGESIZE;
    #endif
    
        // 等待退出全模块空闲
    	do {
    		reg_val = NAND_READ_REG(INT_STATUS);
    	} while(!(reg_val & 0x01));
    	/* disable NAND Controller */
    	reg_val = NAND_READ_REG(ENABLE_STATUS);
    	NAND_WRITE_REG(reg_val & (~0x01), ENABLE_STATUS);
        // 中断使能总开关、中断使能：断点事件
    	NAND_WRITE_REG((1<<24) | (1<< 20), INT_ENABLE);
        // 不可纠错断点使能
    	NAND_WRITE_REG((1<<8), BKPT_ENABLE);
        // ecc 由 nand flash 控制器使用、ecc 能力、数据块长度 512、译码
    	NAND_WRITE_REG(0<<28 | mtd->ecc_strength<<8 | 1 << 4 | 0, ECC_CONFIG);
    
    	if(ecc_enable_flag) {
    		switch (mtd->oobsize) {
    		case 64:
    			nandflash_ecc4(dev);
    			break;
    		case 112:
    			nandflash_oob112(dev);
    			break;
    		case 128:
    			nandflash_ecc8(dev);
    			break;
    		case 256:
    			nandflash_ecc16(dev);
    			break;
    		default:;
    		}
    	} else {
    		nandflash_noecc(dev);
    	}
    
        // 清零：最多的一次的错误个数
    	NAND_WRITE_REG(0x3f << 24, ECC_ENABLE_STATUS);
        // 起始页号
    	NAND_WRITE_REG(page, START_ROW_ADDR);
    	if (len > page_size)
    		ctrl_config = (NAND_DMA_ENABLE << 28) | (4<<16) | (mtd->ecc_strength<<8) | (ecc_enable_flag<<5) | (1<<4) | (chip->cache_read_flag<<1) | (0<<0);
    	else
    		ctrl_config = (NAND_DMA_ENABLE << 28) | (4<<16) | (mtd->ecc_strength<<8) | (ecc_enable_flag<<5) | (1<<4) | (0<<1) | (0<<0);
    
    #if NAND_DMA_ENABLE && defined(CONFIG_GX_LINUX)
    	if (is_vmalloc_addr(data))
            // dma 地址由链表描述符指定  / dma 地址由 dma_start_addr 指定
    		ctrl_config |= 1 << 3;
    #endif
    
        // 是否使能 DMA、两字节列地址+三字节行地址、ecc 纠错能力、是否使能ecc、每次读写32bit、是否使用 cache read、读 flash
    	NAND_WRITE_REG(ctrl_config, CTRL_CONFIG);
    	/* enable NAND Controller */
    	NAND_WRITE_REG(reg_val | 0x01, ENABLE_STATUS);
        // 默认开 dma
    #if NAND_DMA_ENABLE && defined(CONFIG_GX_LINUX)
    	struct gx_mtd_dev *mtd_dev = mtd->mtd_dev;
    	struct device *ndev = mtd_dev->mtd.dev.parent;
    	//struct page *page;
    	struct sg_table sgt;
    	struct scatterlist *sg;
    	struct scatterlist *sgl;
    	int ret, i, dma_desc_len, sgl_len, sgs;
    	int half_buffer_size, desc_len;
    	dma_addr_t dma_addr;
    	struct nfc_dma_desc *dma_desc;
    	dma_addr_t dma_desc_addr;
    	bool is_link_malloc = false;
    
    	memset(&sgt, 0, sizeof(sgt));
    
    	if (is_vmalloc_addr(data)) {
    		ASSERT(IS_ALIGNED((unsigned int)data, ARCH_DMA_MINALIGN));
    		ASSERT(IS_ALIGNED((unsigned int)len, ARCH_DMA_MINALIGN));
    
    		unsigned int max_seg_size = dma_get_max_seg_size(ndev);
    		desc_len = min_t(int, max_seg_size, PAGE_SIZE);
    		sgs = DIV_ROUND_UP(len + offset_in_page(data), desc_len);
    
    		/* 尝试使用write buffer作为链表储存空间，如果buffer过小，则申请内存 */
    		/* NFC DMA链表尾部需要添加一个额外的结束描述符 */
    		dma_desc_len = (sgs + 1) * sizeof(*dma_desc);
    		dma_desc_len = round_up(dma_desc_len, ARCH_DMA_MINALIGN);
    		sgl_len = sgs  * sizeof(*sgl);
    		half_buffer_size = mtd->writesize / 2;
    
    		if (dma_desc_len > half_buffer_size || sgl_len > half_buffer_size) {
    			dma_desc = kmalloc(dma_desc_len, GFP_KERNEL);
    			if (dma_desc == NULL)
    				return -ENOMEM;
    
    			sgl = kmalloc(sgl_len, GFP_KERNEL);
    			if (sgl == NULL) {
    				kfree(dma_desc);
    				return -ENOMEM;
    			}
    
    			is_link_malloc = true;
    
    			ret = nand_map_buf(mtd, ndev, &sgt, data, len, sgl, sgs, DMA_FROM_DEVICE);
    			if (ret != 0)
    				return -ENOMEM;
    			dma_desc_addr = dma_map_single_attrs(ndev, dma_desc, dma_desc_len, DMA_TO_DEVICE, DMA_ATTR_SKIP_CPU_SYNC);
    		} else {
    			dma_desc = (struct nfc_dma_desc*)chip->buffers->databuf;
    			sgl      = (struct scatterlist*)((uint32_t)(chip->buffers->databuf) + mtd->writesize / 2);
    			ret = nand_map_buf(mtd, ndev, &sgt, data, len, sgl, sgs, DMA_FROM_DEVICE);
    			if (ret != 0)
    				return -ENOMEM;
    			dma_desc_addr = dma_map_single_attrs(ndev, dma_desc, half_buffer_size, DMA_TO_DEVICE, DMA_ATTR_SKIP_CPU_SYNC);
    		}
    
    		for_each_sg(sgt.sgl, sg, sgt.nents, i) {
    			dma_desc[i].buf = (void*)sg_dma_address(sg);
    			dma_desc[i].len = sg_dma_len(sg);
    			dma_desc[i].llp = (void*)((unsigned int)dma_desc_addr + sizeof(*dma_desc) * (i + 1));
    		}
    		dma_desc[i].buf = 0;
    		dma_desc[i].len = 0;
    		dma_desc[i].llp = NULL;
    
    		/* 刷描述符内存 */
    		dma_sync_single_for_device(ndev, dma_desc_addr, DIV_ROUND_UP(dma_desc_len, ARCH_DMA_MINALIGN) * ARCH_DMA_MINALIGN, DMA_TO_DEVICE);
    		NAND_WRITE_REG(dma_desc_addr, DMA_START_ADDR);
    	} else {
    		dma_addr = dma_map_single(ndev, data, len, DMA_FROM_DEVICE);
    		if (dma_mapping_error(ndev, dma_addr)) {
    			gxlog_e("dma map error\n");
    		}
    		NAND_WRITE_REG(dma_addr, DMA_START_ADDR);
    	}
    #else
        // DMA 地址由 DMA_START_ADDR 指定
    	NAND_WRITE_REG(data - GX_DRAM_VIRTUAL_OFFSET, DMA_START_ADDR);
    #endif
        // 写：一批传输的总长度，单位是字节。
        // 读：还没有被 buffer 确认的字节数。注意：只要 buffer 认为可以接收数据，本寄存器就会减少，即使数据还没有开始传输
    	NAND_WRITE_REG(len, XFER_TOTAL_SIZE);
    	end_page = page + ((len-1) / page_size);
        // 并行 cache 读时，除了块尾以外，还需要发 0x3f 命令的页号
    	NAND_WRITE_REG(end_page, C3F_ROW_ADDR);
    
        // 1表示 dma 还未配置，即未写入过 dma_start_addr 或链表已耗尽，一般无需关心，不用 DMA 时完全不用关心
    	do {
    		reg_val = NAND_READ_REG(INT_STATUS);
    	} while(!(reg_val & 0x0100));
    #if NAND_DMA_ENABLE
    #ifndef CONFIG_GX_LINUX
    	dcache_flush();
    #endif
    #endif
        // enable_status.enable 置1的前提下，写1表示通过xfer_total_size告知新的一批数据的数量
        // 读到0表示可以通过xfer_total_size 告知新的一批数据的数量。读到0只表示数据数量被硬件知晓，不表示数据已经读写完成
    	NAND_WRITE_REG(1, XFER_START);
        // 没使能 dma
    #if !NAND_DMA_ENABLE
    	int i = 0, fifo_amount_remain;
    	uint32_t *p = (uint32_t*)data;
    	while ((i*4) != len) {
    		reg_val = NAND_READ_REG(ENABLE_STATUS);
            // 读到1时，表示可以填充或读走至少1字节。如果使用DMA，那么此位仅供调试观测
    		if (reg_val & (1<<4)) {
                // fifo 还需要填入的数据的数量？
    			fifo_amount_remain = (NAND_READ_REG(ENABLE_STATUS) >> 16) & 0x3ff;
    			while (fifo_amount_remain) {
    				p[i++] = NAND_READ_REG(DATA_REGISTER);
    				fifo_amount_remain -= 4;
    			}
    		} else {
    			reg_val = NAND_READ_REG(INT_STATUS);
                // 断点事件
    			if (reg_val & (0x01 << 20)) {
    				bkpt_continue = NAND_READ_REG(BKPT_CONTINUE);
                    // 有断点事件发生
    				if (bkpt_continue& 0x01) {
    					bkpt_status = NAND_READ_REG(BKPT_STATUS);
                        // fifo0 不可纠错
    					if (bkpt_status & 0x0100) {
                            // fifo0 工作次数
    						reg_val = NAND_READ_REG(TASK_CNT_0);
    						fifo_count = (reg_val - 1) * 2;
    						err_page = page + fifo_count / subpage_num;
    						subpage_ofs = fifo_count % subpage_num;
    						mtd->ecc_stats.failed++;
    						gxlog_e("ecc error page id = %d[%d]\n", err_page, subpage_ofs);
    					}
    					// 清除断点事件
    					NAND_WRITE_REG(bkpt_continue, BKPT_CONTINUE);
    				}
                    // fifo1 有断点事件发生
    				if (bkpt_continue & 0x10000) {
    					bkpt_status = NAND_READ_REG(BKPT_STATUS);
                        // fifo1 不可纠错
    					if (bkpt_status & 0x01000000) {
                            // fifo1 工作次数
    						reg_val = NAND_READ_REG(TASK_CNT_1);
    						fifo_count = (reg_val - 1) * 2  + 1;
    						err_page = page + fifo_count / subpage_num;
    						subpage_ofs = fifo_count % subpage_num;
    						mtd->ecc_stats.failed++;
    						gxlog_e("ecc error page id = %d[%d]\n", err_page, subpage_ofs);
    					}
    
    					NAND_WRITE_REG(bkpt_continue, BKPT_CONTINUE);
    				}
    			}
    		}
    	}
    #endif
    
    // 不使能 DMA 或 不是 Linux 或 既不使能 DMA 又不是 Linux
    #if !(NAND_DMA_ENABLE && defined(CONFIG_GX_LINUX))
    	while(1) {
    		reg_val = NAND_READ_REG(INT_STATUS);
    		if (reg_val & 0x01)  // wait nfc ready
    			break;
            // 断点事件
    		else if (reg_val & (0x01 << 20)) {
    			bkpt_continue = NAND_READ_REG(BKPT_CONTINUE);
                // fifo0 有断点事件
    			if (bkpt_continue& 0x01) {
    				bkpt_status = NAND_READ_REG(BKPT_STATUS);
                    // fifo0 不可纠错
    				if (bkpt_status & 0x0100) {
    					reg_val = NAND_READ_REG(TASK_CNT_0);
    					fifo_count = (reg_val - 1) * 2;
    					err_page = page + fifo_count / subpage_num;
    					subpage_ofs = fifo_count % subpage_num;
    					mtd->ecc_stats.failed++;
    					gxlog_e("ecc error page id = %d[%d]\n", err_page, subpage_ofs);
    				}
    
                    // 清除断点事件
    				NAND_WRITE_REG(bkpt_continue, BKPT_CONTINUE);
    			}
                // fifo1 有断点事件
    			if (bkpt_continue & 0x10000) {
    				bkpt_status = NAND_READ_REG(BKPT_STATUS);
    				if (bkpt_status & 0x01000000) {
    					reg_val = NAND_READ_REG(TASK_CNT_1);
    					fifo_count = (reg_val - 1) * 2  + 1;
    					err_page = page + fifo_count / subpage_num;
    					subpage_ofs = fifo_count % subpage_num;
    					mtd->ecc_stats.failed++;
    					gxlog_e("ecc error page id = %d[%d]\n", err_page, subpage_ofs);
    				}
    
    				NAND_WRITE_REG(bkpt_continue, BKPT_CONTINUE);
    			}
    		}
    	}
    #endif
    
        // 使能 DMA 并且是 Linux
    #if NAND_DMA_ENABLE && defined(CONFIG_GX_LINUX)
    	uint32_t reg = NAND_READ_REG(INT_ENABLE);
        // 中断使能
    	NAND_WRITE_REG(reg | 0x01, INT_ENABLE);
        // 获取信号量
    	down(&mtd_dev->sem);
    
    	if (is_vmalloc_addr(data)) {
    		dma_unmap_single(ndev, dma_desc_addr, mtd->writesize, DMA_TO_DEVICE);
    
    		if (sgt.orig_nents) {
    			dma_unmap_sg(ndev, sgt.sgl, sgt.orig_nents, DMA_FROM_DEVICE);
    		}
    
    	} else {
    		dma_unmap_single(ndev, dma_addr, len, DMA_FROM_DEVICE);
    	}
    
    	if (is_link_malloc == true) {
    		kfree(dma_desc);
    		kfree(sgl);
    	}
    #endif
    #if NAND_DMA_ENABLE
    #ifndef CONFIG_GX_LINUX
    	dcache_flush();
    #endif
    #endif
    
    	if(ecc_enable_flag) {
    		reg_val = NAND_READ_REG(ECC_ENABLE_STATUS);
            // 最多的一次的错误个数
    		max_bits_err = (reg_val >> 24) & 0x3f;
    		//gxlog_e("ecc error max_bits_err:%u\n", max_bits_err);
    
    		if (mtd->ecc_stats.failed - stats.failed)     /* ecc error occured */
    			return -EBADMSG;
    	} else {
    		max_bits_err = 0;
    		if(chip->ecc_buildin)
    			return chip->ecc.read_ecc_status(mtd, chip, page);
    	}
    
    	mtd->ecc_stats.corrected += max_bits_err;
    	return max_bits_err;
    }
    ```

    







## 问题：

1. 使用 chip->cmd_ctrl(mtd, NAND_CTL_SETCLE, 0) 是让 CLE 有效，但是函数 chip->cmd_ctrl 是一个空函数，怎么操作 CLE 的呢？

2. eccsteps是什么东西？nand_page_sectos mark 是什么东西？

   - ```c
     	chip->ecc.steps = mtd->writesize / chip->ecc.size;
     // 根据 writesize / ecc.size 算出来的，2048/512=4
     ```

   - 因为数据是全0xff，所以采用直接将buff赋值为0的方式，而不去读 flash

3. nfc 模块功能
4. v1 nfc 写数据时，如何生成的 ecc 数据？生成的 ecc 数据写入到了哪里？
   - 







# ARM CPU 投机操作

- 包括投机读取、投机执行(推测)





# Grus bbt 板 Training 数据恢复

- 编译打开 `pmu osc test` 测试命令、打开 `GDB` 选项
- 使用 `gdb` 加载 `.elf`
- 执行 `osc write` 命令重新写入 `training` 值
- 再次配置编译选项，关闭`GDB`选项
- 能够重新正常下载





# Grus BBT 板使用 JTAG 调试步骤

硬件环境：

- 硬件板：GRUS_GX8002_BBT_V1.0
- JTAG：CKLink-Lite-V1.0-GX
- 串口：串口 1



操作步骤如下：

1. 编译命令：

   - 需要配置打开 `Jtag 模式`、选择串口为 `串口1`

   ```
   make grus_gx8002t_dev_1v_defconfig
   make menuconfig
   	hacking -->
   		[*] jtag debug
       Device Drivers(legacy)  --->
       	Serial drivers  ---> 
       		select serial port (1)  --->
   ```

2. 编译出来生成 `gxscpu.elf` 

3. 打开 `DebugServer` ：

   ```shell
   DebugServerConsole -setclk 4
   ```

4. 编辑 `.gdbinit` 

   ```
   target remote 127.0.0.1:102
   load
   ```

5. 开启 gdb

   ```shell
   csky-abiv2-elf-gdb gxscpu.elf
   ```

6. 正常执行调试



注意：

1. 如果当前 flash 中有数据，需要先破坏掉前面的 stage1 代码，目的是让 gdb 能够正常连接上
   - 可以使用 sf 命令擦掉 flash 0 地址的几十个字节
   - 对于一颗新的 芯片 也无法链接 gdb，因为可能芯片内已经烧录了 bbt 程序，这些程序把 jtag 管脚复用成其它功能了，所以无法连接 jtag
2. 进行擦除操作时不要擦掉整个 flash，因为 flash 的末尾存储了 training 数据用于启动
   - 如果擦除掉 `training` 数据后，会发生无法下载、无法启动的情况，更换一颗芯片解决问题
   - `training` 数据是由于 grus 没有使用外部晶振，使用的是芯片内部晶振，内部晶振存在不稳定的情况，这些 `training` 数据帮助晶振稳定



# OBJDUMP 反汇编查看源文件

```shell
xxx-objdump -S source_code | less
```

- example：loader.elf

  ```assembly
  # SRAM base_addr：0x80400000
  
  loader.elf:     file format elf32-littlearm
  
  
  Disassembly of section .text:
  
  80400000 <ResetEntry>:
          .global ResetEntry
          .extern entry
  #define CR_V                         (1 << 13)  /* Vectors relocated to 0xffff0000      */
  
  ResetEntry:
          b       reset
  # 内存地址		 对应的二进制
  80400000:       ea000020        b       80400088 <reset>
          ldr pc, _undefined_instruction
  80400004:       e59ff014        ldr     pc, [pc, #20]   ; 80400020 <_undefined_instruction>
          ldr pc, _software_interrupt
  80400008:       e59ff014        ldr     pc, [pc, #20]   ; 80400024 <_software_interrupt>
          ldr pc, _prefetch_abort
  8040000c:       e59ff014        ldr     pc, [pc, #20]   ; 80400028 <_prefetch_abort>
          ldr pc, _data_abort
  80400010:       e59ff014        ldr     pc, [pc, #20]   ; 8040002c <_data_abort>
          ldr pc, _not_used
  80400014:       e59ff014        ldr     pc, [pc, #20]   ; 80400030 <_not_used>
          ldr pc, _irq
  80400018:       e59ff014        ldr     pc, [pc, #20]   ; 80400034 <_irq>
          ldr pc, _fiq
  8040001c:       e59ff014        ldr     pc, [pc, #20]   ; 80400038 <_fiq>
  
  80400020 <_undefined_instruction>:
  80400020:       80400194        .word   0x80400194
  
  80400024 <_software_interrupt>:
  80400024:       80400194        .word   0x80400194
  
  
  ```

  

# ARM 的 PC 地址 = PC + 8

- Why ARM `pc = pc + 8` ?
  - 由于 arm 的三级流水线遗留问题，其他的处理器没有这个问题
- 什么是三级流水线？
  - 三级流水线：cpu 将指令的执行过程分为三个阶段：取指、译码、执行。每个阶段负责处理一部分指令，从而允许多个指令在不同阶段同时运行。
  - 这种设计让处理器在每个时钟周期内都能处理一条新指令，从而显著提高执行速度
  - 在经典的 arm 32 位架构中，pc 的值在取值时会自动增加
  - 第一个周期：取指--v
  - 第二个周期: 取指 译码--v
  - 第三个周期：取指 译码 执行
- 因此当当前代码被执行时其实已经过了两个指令周期了，而 arm 中的指令集是32位的，一条指令也就是4个字节, 两条指令就是 8 个字节，
- pc = pc + 8

- 在 ARM 架构中，PC 是用于存储当前正在执行的指令的地址，也就是下一条将要被执行的指令的地址
- ARM 的地址宽度是 4 个字节，所以每执行一条指令，PC 值就会 +4
- 而一条指令要被执行，由于流水线的缘故，需要进行取指、译码、执行，而由于执行当前指令的时候已经在前面两个周期进行了取值和译码操作，PC指针领先当前执行操作的地址，那么PC指针实际上就是执行处代码地址 = PC - 8
- 即 PC = PC + 8 指的是 PC 领先执行指令的周期 2 条指令

```
程序计数器 R15 也叫做 PC， R15 保存着当前执行的指令地址值加 8 个字节，这是因为 ARM
的流水线机制导致的。ARM 处理器 3 级流水线：取指->译码->执行，这三级流水线循环执行，
比如当前正在执行第一条指令的同时也对第二条指令进行译码，第三条指令也同时被取出存放
在 R15(PC)中。我们喜欢以当前正在执行的指令作为参考点，也就是以第一条指令为参考点，
那么 R15(PC)中存放的就是第三条指令，换句话说就是 R15(PC)总是指向当前正在执行的指令
地址再加上 2 条指令的地址。对于 32 位的 ARM 处理器，每条指令是 4 个字节，所以:
R15 (PC)值 = 当前执行的程序位置 + 8 个字节。


ins1 ins2 ins3

周期1：CPU 取值 ins1
周期2：CPU 取值 ins2；译码 ins1
周期3：CPU 取值 ins3；译码 ins2；执行 ins1

此时执行 ins1 时的 pc 就是取值 ins1 之后的 2 个周期的地址
```


# ARM 中断默认不支持嵌套

- 在进入某种异常(irq/undefine/...)之后，处理器自动配置 CPSR 的 I、F 位来关闭中断和 FIQ
- 所有异常都禁用 IRQ
- Reset、FIQ 阶段禁用 IRQ 和 FIQ
- 因此在进入中断之后，默认是不响应其它中断的，所以不支持中断嵌套
- 可以通过在进入中断之后手动打开中断来支持中断嵌套

# ARM 异常处理

- 发生异常时，ARM 内核会自动执行以下操作：
  - 将 CPSR 复制到 SPSR_<mode>, 比如发生 IRQ 异常时，当前 CPSR 就会被保存到 SPSR_IRQ 中
  - 将返回地址存储在新模式的连接寄存器 LR 中, 比如发生 IRQ 模式时，返回地址就保存到 LR_IRQ 中
  - 将 CPSR 模式位修改为与异常类型相关联的模式
  - 将 PC 设置为指向异常向量表中的相关指令
      - 在新模式下，CPU 将访问于该模式关联的寄存器，例如进入 IRQ 模式，则访问 lr 寄存器就是访问的 LR_IRQ 寄存器
      - 异常处理程序软件几乎总是需要在进入 handler 的时候将寄存器保存到堆栈中
      - ARMv6 及更高版本的 ARM 体系结构中，提供了一种特殊的汇编 SRS(store return state) 将 LR 和 SPSR 压入任何模式的堆栈
  - 从异常处理程序返回
     - 从保存的 SPSR 中恢复 CPSR
     - 将返回地址写入 PC 寄存器
     - ARM 中可以通过两类命令从异常返回：
	- RFE 指令：将连接寄存器和 SPSR 从当前模式堆栈弹出
        - 设置 PC 寄存器：例如 ``SUBS PC， LR， #offset``
        - LDMFD sp!, {pc} ^
        - LDMFD sp!, {R0-R12, pc} ^
        - ^ 表示 SPSR 同时复制到 CPSR
  
- 异常的返回地址：
Exception        Adjustment    Return instruction    Instruction returned to
SVC              0             MOVS PC, R14          Next instruction
Undef            0             MOVS PC, R14          Next instruction
Prefetch Abort   -4            SUBS PC, R14, #4      Aborting instruction
Data abort       -8            SUBS PC, R14, #8      Aborting instruction if precise
FIQ              -4            SUBS PC, R14, #4      Next instruction    
IRQ              -4            SUBS PC, R14, #4      Next instruction    



# ARM 汇编指令

- 当程序中有一些立即数，比如是 32 位的时候，而 arm 执行一条机器指令只有 32 位的地址，还要保存其它的东西，没法把 32 位都用来保存地址，用伪指令的方式来保存地址(偏移)

- `MCR` 指令

  ```
  MCR<c> <coproc>, <opc1>, <Rt>, <CRn>, <CRm>.{, <opc2>}
  ```

- `MRC`指令

  ```
  MRC<c> <coproc>, <opc1>, <Rt>, <CRn>, <CRm>{, <opc2>}
  ```

  





# ARM 中的 b 和 bl

- b 就是跳转指令
- bl 就是在跳转前会保存下一句话的地址到 lr 寄存器，等跳转之后的指令执行完成后，会将 lr 的值赋给 pc，跳回来继续运行







# ARM JLINK GDBSERVER 后台开启方法

```
sudo ./JLinkGDBServerCLExe -select USB -device Cortex-A7 -endian little -if JTAG -speed 1000 -noir -LocalhostOnly -port 2331
```



# Leo_mini 修改模块时钟频率

- 修改文件`board/nationalchip/leo_mini_gx8008b_dev_1v/include/board_config.h` 中的  `CONFIG_FLASH_SPI_CLK_SRC` 宏的定义
- 此定义在 `arch/csky/mach-leo_mini/clk.c` 中会使用到

- 修改所有 spi_flash 的频率，默认 200M 的时钟 /2 之后的值配过去就是 100M，默认  200M 的时钟 /4 之后的值配过去就是 50M 的模块频率





# TestLink 测试用例导出

- 目前 TestLink 上包含 canopus 的 spi 测试用例，需要导入到 vega

- 首先在 canopus 的产品需求中，选中 spi，点击设置、然后导出

  ![选区_223](image/选区_223.png)

- 在 canopus 的测试用例中，选中 spi，点击设置，然后导出

  ![选区_225](image/选区_225.png)

- 导出时勾选上所有选项

![选区_226](image/选区_226.png)

- 去到 vega 目录下，首先进入到产品需求，相同的操作方式，导入在 canopus 中导出的 .xml 文件
- 然后在测试用例目录下，导入在 canopus 中导出的 .xml 文件





# SPI NAND 内置 ECC

- SPI NAND 一般都有内置 ECC，用于保持数据完整性。内置 ECC 计算在 page program 期间完成，结果存储在每页额外的 64 字节区域中。
- 在数据读取操作器件，ECC engine 将根据先前存储的 ECC 信息来验证数据值，并在需要时进行必要的更正。
- 验证和纠正状态由 ECC 状态位指示。
- ECC 功能在上电时默认启用。
- 用户需要一次性编程一个扇区和一个spare区域(扇区0、spare0)，以正确且自动地编程 ECC 校验数据



# 列出目标文件中的符号

- 现在有一个 .bin，需要知道其中的各个符号所在地址
- 使用 nm 工具





# ecos 一键编译关闭双核

- ecos 单独编译，可以通过 `./build ecos` `./build ecos_smp` 来区分单双核

- 一键编译：`platform/gxbus/scripts/solution_build`

  ```diff
  --- a/scripts/solution_build
  +++ b/scripts/solution_build
  @@ -199,7 +199,7 @@ function build_ecos3.0() {
          cd ./platform/ecos3.0
  
          if [[ ${ARCH} == "arm" ]]; then
  -               ARCH_TMP=arm_smp
  +               ARCH_TMP=arm
  ```

  



# DMA 操作导致线程栈被冲

```shell

        cd gxtest/stb/goxceed
        ./build config -m file_system -e mount -k self_test -s location inside ismount unmount fs_type jffs2 flash_type nor
        cd ../../../
        ./make.sh gxtest-goxceed arm ecos canopus 6631SH1D dvbs nor bin withclean
        cd gxtest/stb/goxceed
        ./build arm ecos canopus 6631SH1D dvbs nor bin

```

- 先修改成单核，然后按照上面的步骤编译
- 按照上述描述信息先把 .bin 烧录，然后编译 debug 版本的 gxloader 烧进去
- 用 jlink 调试 out.elf(最后编译的时候把 bin 换成 debug)
- 如果只修改了 ecos 的代码，在ecos的 `work_arm` 目录下，make 一下就行了，然后执行最后两步



- 线程栈是在创建线程的时候 malloc 出来的一块空间，ecos 在这块内存空间的 顶部和底部 分别用 32 个字节的数据来保存特定的信息，用于检查线程的堆栈没有被破坏
- 目前发现一个线程的栈被冲掉了，cpu 模式不会出现，dma 模式必冲
- dma 是从 flash 中读数据到内存，对于内存来说是写操作，那么就有可能把数据写到栈所在的内存空间
- 查看 dma 读数据时的内存地址和长度，发现覆盖了栈空间，导致的冲栈





# Flash 特性

- Nor：
  - 擦除：按照 block size 擦除
  - 读：发命令、发地址、出数据，可以从任意地址读，读任意长度 第一个字节寻址可以位于任何位置
  - 写：发命令、发地址、写数据，地址可以不是 page 对齐的，任意page 的位置写，写任意长度，驱动里面会做兼容，从任意page 的起始地址写，并且不超过这个page的末尾，如果要写的长度超过了末尾，那么会再发新的地址
    - datasheet 描述：可以从 page 的开头写至少一个字节
  
- SPI Nand：
  - 擦除：按照 block size 擦除，擦除会将 main、oob 区域都擦除
  
  - 读：按照 page 读，驱动做了策略，能够保证非page 对齐读
    - flash 要求：`指令、页地址、dummy`，`page_id = addr >> info->page_shift(11)` 相当于地址除以 2048，则得到 `page_id`
    - 如果不是 page 对齐，则先发命令 `read_page_to_cache`，再在 `read_from_cache` 中发地址时将偏移地址发出去来完成读偏移地址的操作
    
  - 同时读 main、oob：buffer 大小是 `main + oob`，通过读命令能够一次读 `main + oob` 长度的数据，读 oob 区域的 addr 为 `2048、2049、2050...`
  
    ```c
    int spinand_read_ops(struct spinand_chip *chip, loff_t from, struct mtd_oob_ops *ops, int *ecc_stats_corrected, int *ecc_stats_failed)
    
        |...main...|.oob.|...main...|.oob.|...main...|.oob.|
        
        根据长度和偏移地址，计算要读几个 page，例如：偏移地址为 0x10，长度是 2*0x800 + 0x10，那么就要读 3 个page
    1. 如果是 main + oob 都要读：|.|.|...main...|.oob.|.|.|
        第一个 page 先将 main、oob 全都读出来，然后根据 main 的 offset 将读出的 buffer 根据偏移拷贝给用户；oob 也一样；
        第二个 page 将 main、oob 全都读出来，main、oob 的 offset 此时全为 0，直接拷贝整个 page 给用户；
        第三个 page 先将 main、oob 全都读出来，然后根据 size 拷贝部分 main、oob 数据给用户；
    2. 如果是 main 要读：|.|...main...|.|
    	第一个 page 先将 main - offset 长度读出来，然后根据 main 的 offset 将读出来的 buffer 拷贝给用户；
        第二个 page 将 main 全都读出来，main 的 offset 为 0，直接拷贝整个 page 给用户；
        第三个 page main 的 offset 为 0，将剩余长度从 main 中读出来，拷贝给用户；
    3. 如果是 oob 要读：|.|.oob.|.|
        第一个 page 将整个 oob 读出来，然后根据 oob 的 offset 将读出来的 buffer 拷贝给用户；
        第二个 page 将整个 oob 读出来，将整个 oob 拷贝给用户；
        第三个 page 将整个 oob 读出来，根据 size 拷贝给部分 oob 数据用户；
    ```
  
    
  
  - 写：按照 page 写，驱动做了策略，能够保证非 page 对齐写
  
    ```c
    int spinand_write_ops(struct spinand_chip *chip, loff_t to, struct mtd_oob_ops *ops)
    
        |...main...|.oob.|...main...|.oob.|...main...|.oob.|
        
        根据长度和偏移地址，计算要写几个 page，例如：偏移地址为 0x10，长度是 2*0x800 + 0x10，那么就要写 3 个 page
    1. 如果是 main + oob 都要写：
    
    ```
  
    
  
  - 同时写 main、oob：buffer 大小是 `main + oob`，将要写的数据组织好，然后通过 `0x32 0x10` 一次将 `main + oob` 大小的数据写到 flash，相当于此时的写 oob 区域的 addr 为 `2048、2049、2050...`
  
  - ROM 不支持跳坏块，Stage1 支持跳坏块
  
- Paraller Nand：
  - 擦除：按照 block size 擦除
  - 读：按照 page 读，驱动做了策略，能够保证非 page 对齐读
  - 写：按照 page 写，驱动做了策略，能够保证非 page 对齐写
  - ESMT 的芯片读出来的厂家 ID 是 GD 的，说明他们用的是 GD 的颗粒
  - 支持 Cache-read 的 Nand 需要在驱动中加一下，然后测试读写擦就会使用 Cache-read
  - ROM 不支持跳坏块，Stage1 支持跳坏块



# Sirius-PPI Nand Read

- 通过 `nfc_v0.c` 中的驱动先读 oob 区域：`0x00、col addr、page_size、0x30` 将 oob 数据都读出来
- 然后通过 `nand_base.c` 中的驱动读 main 区域：`0x05 修改列地址到 0 0xe0` 将 main 区域数据都读出来
- main 数据读出来之后做 ecc 纠正



# Cache-Read

有的并行 Nand 支持 Cache-read 指令，用于加快读取。Cache-read 可以在 read 功能完成后发生，数据输出总是从列地址 0x00 开始，顺序读下一页。

Cache-read 会将最近从 flash 读取的数据复制到 page register。

如果不需要读更多页时，发送 0x3f 命令将最后一页复制到 page register。

`发 0x00 address 0x30, 指令换成 0x31，不发地址读下一个 page 的 0x00 地址，一直要读就一直发 0x31 然后读。读最后一页的时候需要发 0x3f`



![2024-11-01_15-32](image/2024-11-01_15-32.png)



 # 芯片特性

### ROM 区分 SPI Nor、SPI Nand、并行 Nand

- ROM 首先使用 3字节地址的方式发 03命令，读 4 个字节数据
  - 若读到 `55AA55AA` 则是 SPI Nor Flash
  - 若读到 `55BB55BB` 则是 SPI Nand Flash
    - 若都不是，则使用 4 字节地址的方式发 03 命令，读 4 个字节数据
      - 若读到 `55AA55AA` 则是 4 字节的 SPI Nor Flash
      - 4字节模式的 Nor 有一些厂家是掉电丢失的，有一些默认就是 4 字节模式的，所以 ROM 需要支持 4 字节模式，否则可能无法启动
- 上面读取失败之后，则认为是 并行 Nand Flash



## Scorpio

- SPI：
  - 包括 GX_SPI、DW_SPI
  - ROM 使用 GX_SPI
  - Stage1、Stage2、ecos 都要验证 GX_SPI、DW_SPI
  - 四字节地址的 Nor 需要验证是否支持
  - 还需要一个寄存器用来存放 SPI_Mode



## Vega

- SPI Nand Flash Rom 和 Stage1 都是采用 NFC 控制器，Stage2 采用的是 SPI 控制器
- Nor：Rom 不支持 32MB 字节
- Nand：Rom 支持 4k page
  - Stage1、Stage2 采用512+16+512+16的方式放在page的main区域
  - Stage1 备份一份，默认的 stage1 放在 block0，block1 空闲，block2 放置 Stage1 的备份



## Canopus

- Nand：Rom 支持 4k page

  - Stage1 采用 512+28+512+28 的方式放在 page 的 main 区域，2k page 后面没放的填充为 0xFF

  - Stage2 采用 nor flash 的方式排布，不备份

  - Stage1 不备份，默认的 Stage1 放在 block0

  - 如何做 bin？

    - 2k Page

      - 首先确定所有的 .o，并通过链接脚本将这些 .o 全部都链接成一个可执行文件 .elf

      - 然后在 .elf 中通过 objcopy-flag 来指定链接时 elf 排布的地址，从而将 .efl 转换成一个包括 stage1 和 stage2 的 .bin

        ```
        OBJCOPY_FLAG=--change-section-lma .text=$(STAGE1_LMA_ADDR) --change-section-lma .data=$(STAGE2_LMA_ADDR)
        
        # 通过修改 .text 的 LMA 地址来修改整个 stage1 的地址
        # 通过修改 .data 的 LMA 地址来修改整个 stage2 的地址
        
        
        .text :
        {
        	all_stage1
        } > stage1
        .data :
        {
        	all_stage2
        } > stage2
        ```

      - 通过 loader_nand_ecc 工具将整 bin 的 stage1 做成 512+28+512+28 的格式排布(2k 中其余的位置填充为 0xFF)

      - 至此结束

    - LMA  和 VMA

      - LMA：逻辑映射地址，链接时的 elf 排布地址
      - VMA：虚拟逻辑地址，就是运行时候的地址

  - 如何读 Stage2？

    - 2k Page
      - 从 flash 32k + 32 的地址开始读 stage2，因为 rom 拷贝 stage1 会拷贝 16k，而 stage1 做 bin 的时候是只填充了 page 的一半，所以要从 flash 的 32k 处开始读 stage2，而 32 是手动添加的 head 标志(55BB55BB)
      - 读到 DDR 指定的地址，长度是 stage2 的长度

  - Example：Stage2 放到 block3

    ```
    Stage1 和 Stage2 挨着放：
    	链接脚本 Stage2 的 LMA:0x00003fe0
    	copy.c 读 Stage2 的地址：0x8020 
    		整 bin 中 Stage2 本来应该的位置是 0x000003fe0，但是会使用 nand_ecc 工具在 bin 开头修改 Stage1 的排布，2k page 只用 1k page 来放数据，导致 Stage1 占用 bin 的大小增大一倍，Stage2 的地址就变成了 0x4000 + 0x4000(Stage1) + 0x20(head)
    
    Stage2 放到 block3：
    	链接脚本 Stage2 的 LMA:0x0005ffe0
    	copy.c 读 Stage2 的地址: 0x60020+0x4000
    ```

- Nand：Rom 会发 reset 命令，然后等待 R/B 信号，如果超时，则直接返回，不执行后续代码



## SIRIUS

- NAND：ROM 支持 4k page
  - 写数据的时候，先把数据写到 main 区域，此时 ecc_module 的寄存器中会保存这些数据的 ecc_code，软件读取这些 ecc_code 后写入到 oob 区域
  - 读数据的时候，先读一个 page 的 oob 区域，oob 区域存储的就是 ecc_code，然后读 main 区域，根据 ecc_code 和 main 数据来判断是否有跳变，并进行纠错

- ROM：不支持读坏块表跳坏块
  - rom 不开启 ecc，烧在 flash 的 bin 是 stage1 + crc，rom 读 stage1 然后读 crc 进行校验，校验成功就启动成功，校验失败就读下一个备份的 stage1 + crc
- Stage1：支持读坏块表跳坏块

  - Stage1(16k = 0x4000)：按照 nor 的方式排布，备份 5 份，放在同一个 block 的不同 page
  - Stage2：按照 nor 的排布，不备份

- Example：Stage2 放到 block3

  ```
  Stage1 和 Stage2 挨着放：
  	链接脚本 Stage2 的 LMA：0x00003fe0
  	copy.c 读 Stage2 的地址：0x14020
  		Stage1 放 5 份，都在同一个 block 的不同 page
  		16k/2k * 2k *5 + 32
  
  	
  Stage2 放到 block3： 
  	链接脚本 Stage2 的 LMA:0x0005ffe0 
  	copy.c 读 Stage2 的地址: copy_start_addr = 0x60000 + 0x4000 * 4 + RESERVE_NUM; 
  		整 bin 中 Stage2 的地址本来应该是 0x0005ffe0，但是会备份 Stage1 五份，相当于在 bin 前面插入四份 Stage1，实际 Stage2 的地址就变成了 0x60000 + 0x40000 * 4 + 32
  ```




- Nand：Rom 会发 reset 命令，然后等待 R/B 信号，如果超时，则直接返回，继续执行代码

- bin 文件如何组织：

  - stage1：生成 nor 排布的 bin，备份 5 份，没有 ecc 码
  - stage2：生成 nor 排布的 bin，不备份，也没有 ecc 码

  

  

  - 控制器 ecc：首先生成 nor 排布的 bin，然后用 nand_ecc 工具按照每 512 多少个字节的 ecc 插入进去，无论是 2k page 或 4k page，bin 里面只有 1k 的有效数据，即 `512+oob+512+oob`。之所以只放 1k 有效数据是因为如果放 4 个会超过 2k page。1k 有效数据不会用到 oob 区域，全部都是放到 data 区。
  - 片上 ecc：首先生成 nor 排布的 bin，然后用 nand_ecc 工具按照每 512 多少个字节的 ecc 插入进去，无论是是 2k page 或 4k page，bin 里面只有 1k 的有效数据，即 `512+oob+512+oob`。之所以只放 1k 有效数据是因为如果放 4 个会超过 2k page。1k 有效数据不会用到 oob 区域，全部都是放到 data 区。
    - 这里 stage1 用到的还是控制器的 ecc。相当于有双重 ecc，控制器的 ecc 和 flash 内部的 ecc。
    - 到了 stage2 之后再决定用不用 flash 内部的 ecc。
  - stage2 的排布：data 放到 data 区，ecc 放到 oob 区域。









## Leo_mini

- SPI 模块频率通过宏 `CONFIG_FLASH_SPI_CLK_SRC` 配置，能够修改 200MHz 和 150MHz
- 修改文件`board/nationalchip/leo_mini_gx8008b_dev_1v/include/board_config.h` 中的  `CONFIG_FLASH_SPI_CLK_SRC` 宏的定义
- 此定义在 `arch/csky/mach-leo_mini/clk.c` 中会使用到

- 修改所有 spi_flash 的频率，默认 200M 的时钟 /2 之后的值配过去就是 100M，默认  200M 的时钟 /4 之后的值配过去就是 50M 的模块频率





### 新增 Flash

- 测试项：
  - [ ] 双线读写擦
    - `drivers/legacy/mtd/spinor/spi_nor_ids.c` 添加驱动
    - `make menuconfig` 中开启 `FLASH_SPI frame format (Dual speed )` 
  - [ ] 四线读写擦
    - `flash_spi.c` 中确认使能四倍速方法
    - `make menuconfig` 中开启 `FLASH_SPI frame format (Quad speed ) `
  - [ ] XIP
    - XIP 带 Cache 地址：0x30000000；XIP 不带 Cache 地址：0x50000000
    - `arch/csky/mach-leo_mini/spinor/spinor.c` stage1 代码去掉 xip 初始化
    - stage2 不开启 xip 初始化，访问 0x30000000、0x50000000 挂掉；开启 xip 初始化，能够正常访问 0x30000000、0x50000000
    - `make menuconfig` 中开启`[*]     Support flash spi xip`
  - [ ] 不需要 UID、写保护、OTP

- `make menuconfig` 中开启 `[*]   MTD tests support` 用于执行测试





## Leo

- SPI 模块频率和 MCU 的频率一致，是一起的，通过宏 `CONFIG_CLOCK_SYSTEM` 配置的，在 `arch/csky/mach-leo/clk.c`中实现的，用户要想改频率只需要在板级对应的头文件中修改宏的值即可



## Grus

- Grus 在 Flash 的尾部 4k 中保存了 `training` 数据，用于辅助晶振
- `get_flash_size` 接口获取到的 flash size 是 `flash_size - 4k`
- `training` 值被误破坏之后，可以通过 GDB 的方式重新烧写
  - 编译打开 `pmu osc test` 测试命令、打开 `GDB` 选项
  - 使用 `gdb` 加载 `.elf`
  - 执行 `osc write` 命令重新写入 `training` 值
  - 再次配置编译选项，关闭`GDB`选项
  - 能够重新正常下载





## Apus

- Apus Stage1 使用四倍速
- bootx 工具使用 `cmd/flash_download.c` 中的下载接口使用的 `legacy` 中的驱动，会进行 `detect` 
- 需要在 `drivers/hal/src/spi/dw_spi_flash_gx_hal_sf_ids.c` 、`drivers/legacy/mtd/spinor/spi_nor_ids.c` 两个文件中添加 id 信息
- Apus 使用的驱动是 `drivers/drv-apus` 目录下的驱动：
  - `drv-apus`：
    - `dw_spi.c`:调用 hal 中的 spi 驱动，通用 spi 使用
    - `dma_ahb.c`:外设通用 dma
    - `psram_spi.c`: 调用 hal 中的 psram 驱动，psram 使用
    - `spi_nor_flash.c`:调用 hal 中 flash 驱动，flash 使用
  - `hal/src`：
    - `psram/gx_hal_psram_spi.c`:独立的 psram 驱动，用于提供套接层
    - `spi/dw_spi_flash/gx_hal_sf.c`: 独立的 flash 驱动，用于提供套接层
    - `spi/dw_spi/gx_hal_spi.c`:独立的 spi 驱动，用于提供套接层



- cache
  - l1 icache
  - l2 icache
  - l1 dcache
  - 没有 l2 dcache，在 mpw 的时候有问题，后面直接删掉了









# FPGA 环境

- VU440
  - 更新 bit：sd 卡中有一个 image.json 文件中保存了要加载的 fpga 文件名，有很多个目录，需要查看 fpga 上硬件的旋钮开关选的是几就是哪个目录

- K7
  - ![apus_fpga_hardware](image/apus_fpga_hardware.jpg)





# 新增 Nand Flash 要做哪些测试

- 内置 ECC
  - 关掉 NFC 的 ECC，使用的是 buildin 
- 非内置 ECC
- Cache read

- Ecc test
- comtest



# XXD 将dump 出来的 2进制转换成 16 进制进行修改，修改完成后恢复成 2 进制

`xxd`命令是一个非常有用的工具，它可以用来生成文件的十六进制表示，也可以用来将十六进制表示转换回二进制数据。你可以使用`xxd`命令来查看和编辑二进制文件。

以下是使用`xxd`命令编辑二进制文件的基本步骤：

1. 首先，你可以使用`xxd`命令将二进制文件转换为十六进制表示，并将结果输出到一个文本文件中。例如，如果你的二进制文件名为`binaryfile`，你可以使用以下命令：

```bash
xxd binaryfile > hexfile.txt
```

这个命令会创建一个名为`hexfile.txt`的文本文件，其中包含了`binaryfile`的十六进制表示。

1. 然后，你可以使用你喜欢的文本编辑器（如`vim`、`nano`或`gedit`等）来编辑`hexfile.txt`。你可以修改文件中的十六进制值，但你需要确保你的修改不会破坏文件的格式。
2. 当你完成编辑后，你可以使用`xxd`命令将十六进制表示转换回二进制数据。你可以使用以下命令：

```bash
xxd -r hexfile.txt > newbinaryfile
```

这个命令会创建一个名为`newbinaryfile`的二进制文件，其中包含了你修改后的数据。

请注意，`xxd`命令是一个强大的工具，但也需要谨慎使用。在编辑二进制文件时，你需要确保你知道你在做什么，因为错误的修改可能会导致文件无法使用。





# Ubuntu20.04 按键无法打开应用，但是键盘可以正常使用

- 重启 GNOME Shell：如果你使用的是 GNOME 桌面环境，尝试重启 GNOME Shell 可能会解决问题。按下 Alt + F2 组合键，输入 "r" 并按下 Enter 键，以重新启动 GNOME Shell。



# arma7使用 bl 指令跳转汇编的子程序时，执行完子程序需要将 lr 寄存器的值恢复到 PC 中去，但是使用 bl 执行 c 语言函数的时候，不需要将 lr 寄存器的值恢复到 pc 中去吗？

- 在 ARM 架构中，`bl`（Branch with Link）指令用于跳转到一个子程序（或函数）并将返回地址保存在链接寄存器（LR）中。当执行完子程序后，通过将链接寄存器的值恢复到程序计数器（PC）中，可以实现返回到调用者的下一条指令。

- 在汇编语言中，当使用 `bl` 指令跳转到一个汇编子程序时，通常需要在子程序的结尾使用 `bx lr` 指令将链接寄存器的值恢复到程序计数器中，以实现返回到调用者。

- 然而，在 C 语言中，编译器会自动处理函数调用和返回的细节，包括保存和恢复链接寄存器的值。当使用 `bl` 指令调用 C 函数时，编译器会生成适当的汇编代码来处理函数调用和返回。因此，在 C 语言中，不需要手动将链接寄存器的值恢复到程序计数器中，编译器会自动处理这个过程。

- 总结起来，当使用 `bl` 指令跳转到汇编子程序时，需要手动将链接寄存器的值恢复到程序计数器中。而在 C 语言中，编译器会自动处理函数调用和返回，无需手动恢复链接寄存器的值。





# DW_SPI 操作寄存器的简单测试程序

- 读取 Flash Jedec

  ```
  set *(int*)0xa030a900 = 0x01        // 这两句应该是使能 CS
  set *(int*)0xa0e00404 = 0x0
  set *(int*)0xa0e00008 = 0x0         // SSIENR
  set *(int*)0xa0e00000 = 0x80000C07  // CTRLR0
  set *(int*)0xa0e00004 = 0x2         // CTRLR1
  set *(int*)0xa0e00014 = 0x4         // BAUDR
  set *(int*)0xa0e00018 = 0x0         // TXFTLR
  set *(int*)0xa0e000F4 = 0x0         // SPI_CTRLR0
  set *(int*)0xa0e00008 = 0x1         // SSIENR
  set *(int*)0xa0e00010 = 0x1         // SER
  set *(int*)0xa0e00080 = 0x9F        // DR
  ```

- 内部 DMA 发送数据

  ```
  set *(int*)0xa030a900 = 0x01
  set *(int*)0xa0e00404 = 0x0
  set *(int*)0xa0e00008 = 0x0
  set *(int*)0xa0e00000 = 0x80000407 | (0x1 << 22)
  set *(int*)0xa0e00018 = 0x0
  set *(int*)0xa0e000F4 = (1<<30) | (0x2 << 8) | 0x01
  set *(int*)0xa0e00004 = 0x10
  set *(int*)0xa0e00014 = 0x4
  set *(int*)0xa0e00018 = 0x0
  set *(int*)0xa0e0004C = 0x04 | (1 << 6)
  set *(int*)0xa0e00054 = 0x1 << 8
  set *(int*)0xa0e00128 = 0x17b06400
  set *(int*)0xa0e0012C = 0x00000000
  set *(int*)0xa0e00124 = 0x01
  set *(int*)0xa0e00120 = 0x11
  set *(int*)0xa0e00010 = 0x1
  set *(int*)0xa0e00008 = 0x1
  ```

- 内部 DMA 读取数据

  ```
  set *(int*)0xa030a900 = 0x01
  set *(int*)0xa0e00404 = 0x0
  set *(int*)0xa0e00008 = 0x0
  set *(int*)0xa0e00000 = 0x80000807 | (0x1 << 22)
  set *(int*)0xa0e00018 = 0x0
  set *(int*)0xa0e000F4 = (1<<30) | (8 << 11) | (0x2 << 8) | (0x6 << 2) | 0x00
  set *(int*)0xa0e00004 = 0x3
  set *(int*)0xa0e00014 = 0x4
  set *(int*)0xa0e00018 = 0x0
  set *(int*)0xa0e0004C = 0x04 | (1 << 6)
  set *(int*)0xa0e00054 = 0x1 << 8
  set *(int*)0xa0e00128 = 0x17b06400
  set *(int*)0xa0e0012C = 0x00000000
  set *(int*)0xa0e00124 = 0x00
  set *(int*)0xa0e00120 = 0x3B
  set *(int*)0xa0e00010 = 0x1
  set *(int*)0xa0e00008 = 0x1
  ```

- 双线、32bit、txonly 发数据

  ```c
  *((volatile unsigned int *) 0xfc400008) = 0;
  *((volatile unsigned int *) 0xfc40002c) = 0;
  *((volatile unsigned int *) 0xfc4000f4) = 0;
  *((volatile unsigned int *) 0xfc400018) = 0;
  *((volatile unsigned int *) 0xfc40001c) = 0;
  *((volatile unsigned int *) 0xfc401080) = 0x0f;
  *((volatile unsigned int *) 0xfc401080) = 0xf & ~(1 << 0);
  *((volatile unsigned int *) 0xfc400008) = 0;
  *((volatile unsigned int *) 0xfc4000f0) = 0x01;
  *((volatile unsigned int *) 0xfc400014) = 0x40;
  *((volatile unsigned int *) 0xfc400004) = 0x00;
  *((volatile unsigned int *) 0xfc40004c) = 0x00;
  *((volatile unsigned int *) 0xfc400008) = 0x00;
  *((volatile unsigned int *) 0xfc400010) = 0x01;
  *((volatile unsigned int *) 0xfc40004c) = 0x00;
  *((volatile unsigned int *) 0xfc4000f4) = (0 << 30) | (0x2 << 8) | (0x6 << 2) | 0x2;
  *((volatile unsigned int *) 0xfc400000) = (0x1 << 31) | (0x1 << 22) | (0x1 << 10) | ((0x20 - 1) << 0) ;
  *((volatile unsigned int *) 0xfc400004) = 0x2 - 1;
  *((volatile unsigned int *) 0xfc400008) = 0x01;
  //*((volatile unsigned int *) 0xfc400060) = 0xaabbccdd;
  //*((volatile unsigned int *) 0xfc400060) = 0x44332211;
  *((volatile unsigned int *) 0xfc400060) = 0xaa;
  *((volatile unsigned int *) 0xfc400060) = 0x332211;
  *((volatile unsigned int *) 0xfc400060) = 0x88776655;
  while ((*(volatile unsigned int *)(0xfc400028)) & (1 << 2));
  while ((*(volatile unsigned int *)(0xfc400028)) & (1 << 0));
  *((volatile unsigned int *) 0xfc401080) = 0x0f;
  ```

  



# AXI DMA 内存测试简单程序

```c
// reset dma
*((volatile unsigned long long *)(0x85300000 + 0x58)) = 0x1
*((volatile unsigned long long *)(0x85300000 + 0x118)) |= (0 << 18) | (0 << 14) | (2 << 11) | (2 << 8) | (0 << 6) | (0 << 4) | (0 << 2) | (0 << 0);
*((volatile unsigned long long *)(0x85300000 + 0x120)) |= (0 << 36) | (0 << 35) | (0 << 23);

// 使能 dma
*((volatile unsigned long long *)(0x85300000 + 0x10)) = 0x3
// 清除所有中断
*((volatile unsigned long long *)(0x85300000 + 0x198)) = 0xffffffff
// 使能状态：dma trf、src_suspend、irq_disable
*((volatile unsigned long long *)(0x85300000 + 0x180)) = (1 << 30) | (1 << 29) | (1 << 1) 

*((volatile unsigned long long *)(0x85300000 + 0x100)) = 0x100000
*((volatile unsigned long long *)(0x85300000 + 0x108)) = 0x110000
*((volatile unsigned long long *)(0x85300000 + 0x110)) = 0x3f

// channel enable
*((volatile unsigned long long *)(0x85300000 + 0x18)) = (1 << 8) | (1 << 0)

while ((0x8530000 + 0x188) & 0x2)
```









# 顺丰快递公司寄快件

- 前台处找一个快递袋，扫描二维码填写寄件信息
- 单子上半部分是二维码，下半部分贴在快递袋上(不需要签字)
- 扫描上半部分的二维码填写寄件信息，填写寄件信息时使用公司月结账户
- 上半部分不需要了，自己拿走



# copy.c 中无论什么 flash 都会跑 spi_init

- 如果是 .boot 的时候会使用串口启动，那么就跑不到 `copy_stage2_from_flash` 函数，导致没法配置一些引脚复用啥的
- 所以这个地方就配一下好了



# SCPU

- 通用 SPI 驱动：

- master:

  - 读操作：
    - 异步 dma：(异步就是用中断和回调的方式，同步就是死等)
      - 单线模式：
        - prepare 阶段：使用 `RX_ONLY` 的方式配置 SPI 寄存器、开启 DMA 
        - transfer 阶段：申请 DMA 通道、`clean cache`、注册回调(回调中就是超时等待，并且关掉 dma ，调用更上层的回调)、配置寄存器、开启中断、启动 DMA、往 TXDR 中发一个 0x00 来产生 CLK、等待发生 DMA 中断的时候执行上面注册的回调函数、从而调到最上层注册的回调函数
      - 多线模式：
        - prepare 阶段：使用 `RX_ONLY` 的方式配置 SPI 寄存器，`SPI_CTRLR0` 寄存器中的指令和地址长度都配置成0.
        - transfer 阶段：申请 DMA 通道、`clean cache`、注册回调(回调中就是超时等待，并且关掉 dma ，调用更上层的回调)、配置寄存器、开启中断、启动 DMA、等待发生 DMA 中断的时候执行上面注册的回调函数、从而调到最上层注册的回调函数
    - 不使用 dma：
      - 单线模式：
        - prepare阶段：使用 `RX_ONLY` 的方式配置 SPI 寄存器，如果使用了同步 DMA，则配置 SPI_DMACR 和 DMARDLR 寄存器。
        - transfer阶段：
          - 无 DMA：往 TXDR 中发一个 0x00 来产生 CLK，然后从 RXDR 中读要读的数据
          - 同步 DMA：申请一个 dma 通道、`invalid cache`、配置寄存器、启动 DMA、发一个 0x00 来产生 CLK、然后等待 DMA 搬运完成。
      - 多线模式：
        - prepare阶段：使用 `RX_ONLY` 的方式配置 SPI 寄存器，`SPI_CTRLR0` 寄存器中的 指令和地址长度都配置成0.，如果使用了同步 DMA，则配置 SPI_DMACR 和 DMARDLR 寄存器。
          - DMACR：使能 DMA RX
          - <font color=green>DMARDLR：接收数据水线。水线=DMARDLR+1。当 RXFIFO 中的有效数据 >= DMARDLR+1就产生 dma_rx_req 请求。用于设置 Receive FIFO 请求的RX FIFO 水线。就是 rxfio 中装满了多少数据就开始进行 DMA 传输。</font>
        - transfer阶段：
          - 无 DMA：根据 prepare 阶段配置好的模式从 RXDR 中读要读的数据
          - 同步 DMA：申请一个 dma 通道、`invalid cache`、配置寄存器、启动 DMA、等待 DMA 搬运完成。

  - 写操作：
    - 异步 dma：
      - 单线模式：
        - prepare 阶段：使用 `TX_ONLY` 的方式配置 SPI 寄存器
        - transfer 阶段：申请一个 dma 通道、`clean cache` 、注册回调、配置 dma 寄存器、开启中断、启动 dma、将所有数据发完、等待发生 dma 中断的时候执行上面注册的回调函数、从而调到最上层的回调函数。
      - 多线模式：
        - prepare 阶段：使用 `TX_ONLY` 的方式配置 SPI 寄存器，把第一个数据当做指令配置到 `SPI_CTRLR0` 寄存器，然后发出去
        - transfer 阶段：申请一个 dma 通道、`clean cache` 、注册回调、配置 dma 寄存器、开启中断、启动 dma、将剩余的所有数据发完、等待发生 dma 中断的时候执行上面注册的回调函数、从而调到最上层的回调函数。
    - 不使用 dma：
      - 单线模式：
        - prepare 阶段：使用 `TX_ONLY` 的方式配置 SPI 寄存器。如果使用了同步 DMA，则配置 SPI_DMCR 和 DMATDLR 寄存器。
        - transfer 阶段：
          - 无 DMA：根据 prepare 阶段配置好的模式将所有的数据发送到 `TXDR` 
          - 同步 DMA：申请一个 dma 通道、`clean cache`、配置 dma 寄存器、启动 dma、等待 dma 搬运完成
      - 多线模式：
        - prepare 阶段：将第一个数据组成指令、地址是空的，使用 `TX_ONLY` 的方式配置 SPI 寄存器，`SPI_CTRLR0` 寄存器中的指令和地址都配置好，然后把第一个数据当成指令发出去。如果使用了同步 DMA，则配置 SPI_DMCR 和 DMATDLR 寄存器。
        - transfer 阶段：
          - 无 DMA：根据 prepare 阶段配置好的模式将剩余的数据发送到 `TXDR`
          - 同步 DMA：申请一个 dma 通道、`clean cache`、配置 dma 寄存器、启动 dma、等待 dma 搬运完成



- slave：
  - init：
    - 与正常的 master 基本一致
  - 接收操作：
    - 单线/多线模式：使用`RX_ONLY` 的方式配置 SPI 寄存器，不配置 SER 寄存器，使能 ENR 寄存器
      - 同步 dma：申请 dma 通道、`invalid cache`、配置 dma 寄存器、开始 dma 传输、如果是标准模式，需要写一个`empty` <font color=green>(这里需要发一个 empty 的原因是 designware ip 如果要读必须要先写一个数据到 buffer，否则 buffer 一直为空。)</font>来发出 clk、等待 dma 传输完成
      - 不使用 dma：如果是单线模式，写一个 `empty` 发出 clk、使用 `RX_ONLY` 的模式接收数据
  - 发送操作：
    - 单线/多线模式：使用 `TX_ONLY` 的方式配置好 SPI 寄存器、不配置 SER 寄存器、使能 ENR 寄存器
      - 同步 dma：申请 dma 通道、`clean cache` 、配置 dma 寄存器、开始 dma 传输、等待 dma 传输完成
      - 不使用 dma：直接把数据往 TXDR 中写



- flash 驱动：

  - <font color=red>**读操作：用的是硬件 cs，即控制器自己来拉高/拉低 cs**</font>
    - 单线模式读：
      - prepare：先发 5 个字节的 0x00，使用 `EEPROM_RX` 的方式？？？？<font color=green>(这里的操作只是配置了一些寄存器(主要是配置水线，因为之前使用的是硬件 CS，如果长时间不操作的话 CS 会自动拉高的，这里把水线配置成5，只有等数据到了5个之后才会去把数据发出去)，并且表示当前是工作在单线模式)</font>，通过`gx_hal_spi_transfer_prepare` 也没看到往 TXBUF 写的操作，prepare 中只是配置了一些寄存器
      - transfer：把指令<font color=green>(0x0b)</font>和地址组好，使用 `EEPROM` 的方式先把指令和地址发送出去，然后再读数据
    - 多线模式读：
      - prepare：把指令和地址组织好，通过 `gx_hal_qspi_transfer_prepare` 配置一些寄存器，把指令和地址配置到 `SPI_CTRLR0` 寄存器，然后发出去
      - transfer：prepare 的时候已经把指令和地址发出去了，使用 `RX_ONLY` 的方式直接读数据
  - <font color=red>**写操作：用的是软件 cs，即配置寄存器来拉高/拉低 cs**</font>
    - 要写的地址和长度小于一个 page，则直接把所有的数据一次写完；如果要写的长度大于一个 page，则循环按照 page 写
    - 单线/双线模式写：
      - prepare：配置一些 spi 寄存器
      - transfer：先把命令和地址组好，发出去；再把数据发出去
    - 多线模式写：
      - prepare：配置一些 spi 寄存器，添加了指令和地址配置到 `SPI_CTRLR0` 寄存器，发指令和地址
      - transfer：由于在 prepare 的时候已经把指令和地址发出去了，这里直接发数据
  - 擦除操作：擦除按照 `Sector` 对齐，满足 `block` 擦除的先按照 `block` 擦，block 擦完之后按照 `sector` 擦
  - xip 初始化：
    - ddr_en：指令是 ddr 模式的指令 0xbd、0xed，这个指令是需要 flash 支持；比下面的多配置 ddr_en 到 `XIP_CTRL` 寄存器、ddr_driver_edge 到 `DDR_DRIVER_EDGE` 寄存器
    - no_ddr：指令是普通 xip 模式的指令 0xbb、0xeb
    - 命令配到 `INCR_INST、WRAP_INST` 寄存器；**命令长度、命令模式、addr 模式、spi 模式、连续传输、预取使能** 配到 `XIP_CTRL` 寄存器；mode_code 配到 `XIP_MODE_BITS` 寄存器
  
  - dma：
    - tx：申请 dma 通道、申请 dma 链表所需占用的内存空间、源是内存、目标是控制器的数据寄存器、把一系列配置到 dma 的寄存器、`clean cache`、<font color=red>(写的时候由于许多 flash 只有单倍速和四倍速，所以单线和双线模式下都使用 TX_ONLY 的标准模式把指令和地址发出去) / (多线模式时指令和地址在 prepare 阶段传输)</font>、开始 dma 传输(使能通道)、等待传输完成
    - rx：申请 dma 通道、申请 dma 链表所需占用的内存、源是控制器的数据寄存器，目标是内存地址、把一系列配置到 dma 的寄存器、`clean cache` 、开始传输(使能通道)、<font color=red>(单线模式使用 `EEPROM` 模式把指令、地址发出去) / (多线模式时指令和地址在 prepare 阶段传输)</font>、等待dma传输完成





- XIP_CTRL：该寄存器仅当SSIC_CONCURRENT_XIP_EN = 1时有效。该寄存器用来存储XIP传输将用作并发模式的控制信息。当控制器启动后，不可写入该寄存器。

| Name            | Bits  | Description                                                  |
| --------------- | ----- | ------------------------------------------------------------ |
| RXDS_VL_EN      | 30    | RXDS 可变延迟使能位。当该位设置时，DWC_ssi 在停止 SCLK_OUT 之前等待所有数据被采样。 |
| XIP_PREFETCH_EN | 29    | 使能XIP预取功能                                              |
| XIP_MBL         | 27:26 | XIP模式位长度；0x0:2 0x1:4 0x2:8 0x3:16                      |
| RXDC_SIG_EN     | 25    | Hyperbus传输的地址和命令阶段使能rxds信号                     |
| XIP_HYPERBUS_EN | 24    | SPI Hyperbus 帧格式XIP传输使能                               |
| CONT_XFER_EN    | 23    | XIP 模式使能连续传输                                         |
| INST_EN         | 22    | XIP指令使能位，XIP传输开启指令阶段；如果开启则xip传输会包含指令相位。指令的op-codes将会从INCR、WRAP中选择，取决于 AHB 传输类型 |
| RXDS_EN         | 21    | 读取数据选通使能位，一旦这bit配为1，DWC_ssi 将使用 rxds 去捕获接收数据 |
| INST_DDR_EN     | 20    | 双数据速率传输指令使能位                                     |
| DDR_EN          | 19    | 使能SPI的DUAL/QUAD/OCTAL格式的双数据速率传输                 |
| DFS_HC          | 18    | 修复XIP传输的数据帧大小                                      |
| WAIT_CYCLES     | 17:13 | 在控制帧传输和接收之间以DUAL/QUAD/OCTAL模式等待周期，指定为SPI时钟周期数 |
| MD_BITS_EN      | 12    | XIP模式下的模式位使能。为1时,XIP模式下控制器将在地址阶段之后插入模式位 |
| INST_L          | 10:9  | DUAL/QUAR/OCTAL模式指令长度；0x0:no 0x1:4bit 0x2:8bit 0x3:16bit |
| ADDR_L          | 7:4   | 传输地址长度；0x0:no 0x1:4bit 0x2:8bit 0x3:12bit 0x4:16bit 0x5:20bit 0x6:24bit |
| TRANS_TYPE      | 3:2   | 传输类型；0x0:指令地址标准模式 0x1:指令标准,地址按配置模式  0x2:两者都按配置模式 0x3:指令地址8线模式，数据16线 |
| FRF             | 1:0   | SPI帧格式; 0x0:标准 0x1:Dual  0x2:Quad  0x3:Octal            |



- XIP_MODE_BITS

| Name        | Bits | Description                                |
| ----------- | ---- | ------------------------------------------ |
| XIP_MD_BITS | 15:0 | XIP 传输时，xip 模式位将发送在地址相位之后 |



- SPI_CTRLR0

- 此寄存器用于控制增强型 SPI 操作模式下的串行数据传输。该寄存器仅在 SPI_FRF（在 CTRLR0 中）设置为 01 或 10 或 11 时相关。 DWC_ssi 启用（SSIC_EN = 1）无法写入。

| Name                  | Bits  | Description                                                  |
| --------------------- | ----- | ------------------------------------------------------------ |
| CLK_STRETCH_EN        | 30    | 时钟延展；当写的时候如果 FIFO 即将变空，控制器将停止 CLK 直到 FIFO 有足够的数据继续传输；当读的时候，如果接收 FIFO 即将变满，控制器将停止 CLK 直到数据已经从FIFO中读走。 |
| XIP_PREFETCH_EN       | 29    | 使能XIP 预取                                                 |
| XIP_MBL               | 27:26 | XIP模式位宽                                                  |
| SPI_RXDS_SIG_EN       | 25    | 超级总线 传输的地址和命令阶段启用 rxds 信号                  |
| SPI_DM_EN             | 24    | SPI数据屏蔽使能位                                            |
| RXDS_VL_EN            | 23    | RXDS 可变延迟使能位，当设置该位时，DWC_ssi 在停止 SCLK_OUT 之前等待所有的数据被采样。这使得 RXDS 传输中支持可变延迟的设备。 |
| SSIP_XIP_CONT_XFER_EN | 21    | 使能xip模式的连续传输                                        |
| XIP_INST_EN           | 20    | XIP指令使能位。如果此位设置为1，XIP 传输将会包含指令相位。根据 AHB 的类型从 XIP_INCR_INST 和 XIP_WRAP_INST 寄存器选择指令 op-codes 。 |
| XIP_DFS_HC            | 19    | 固定XIP传输的数据帧大小。如果此位设置为1，则XIP传输的数据帧大小将会固定成CTRLR0.DFS的编程值。 |
| SPI_RXDS_EN           | 18    | 读数据选通使能位。一旦这一bit设置为1，DWC_ssi 将使用 Read data strobe(rxds)去捕获读取数据。 |
| INST_DDR_EN           | 17    | 指令阶段启用双数据速率传输。                                 |
| SPI_DDR_EN            | 16    | 增强模式模式下启用DDR传输。                                  |
| WAIT_CYCLES           | 15:11 | 增强模式下控制帧发送和数据接收之间的等待周期。周期数是 SPI 的时钟周期。 |
| INST_L                | 9:8   | 增强模式下的指令长度。 0x0:无指令 0x1:4bit 0x2:8bit 0x3:16bit |
| XIP_MD_BIT_EN         | 7     | 模式位(XIP模式下才启用)。如果此位设置为1，在XIP 模式操作 DWC_ssi 时将会在地址相位之前插入mode bits。 |
| ADDR_L                | 5:2   | 传输地址长度                                                 |
| TRANS_TYPE            | 1:0   | 地址和指令传输格式                                           |

TRANS_TYPE:

0x0:指令和地址将在标准SPI模式发送

0x1:指令在标准SPI模式发送，地址依据CTRLR0.SPI_FRF(单倍速、两倍速、四倍速)的配置发送

0x2:指令和地址依据SPI_FRF的配置发送

模式位在 XIP 模式下启用。如果该位设置为 1，则在 XIP 操作模式下，DWC_ssi 将在地址阶段之后插入模式位，这些位在寄存器 XIP_MODE_BITS 寄存器中设置。模式位的长度始终设置为 8 位 .

Dual/Quad/Octal 模式下控制帧发送和数据接收之间的等待周期。指定为 SPI 时钟周期数。

指令阶段启用双数据速率传输。

修复 XIP 传输的 数据帧大小。 如果该位设置为 1，则 XIP 传输的数据帧大小将固定为 CTRLR0.DFS 中的编程值。要获取的数据帧数将由 HSIZE 和 HBURST 信号确定。如果该位设置为 0，则 数据帧大小和要获取的数据帧数将由 HSIZE 和 HBURST 信号确定。

在 Hyperbus 传输的地址和命令阶段启用 rxds 信号。

该位在命令、地址 (CA) 阶段启用超级总线从设备的 rxds 信号。如果在传输的 命令、地址 阶段将 rxds 信号设置为 1，则 DWC_ssi 在地址阶段后发送 (2*SPI_CTRLR0.WAIT_CYCLES - 1) 个等待周期 已经完成。

XIP模式位宽。

设置XIP模式操作位宽。只有在SPI_CTRLR0.XIP_MD_BIT_EN设置为1时有效。

在 DWC_ssi 中启用 XIP 预取功能。一旦启用 DWC_ssi 将从下一个连续位置预取数据帧，以减少即将到来的连续传输的延迟。如果下一个 XIP 请求不连续，则预取位将被丢弃。

在 SPI 传输中启用时钟延长功能。在写入的情况下，如果 FIFO 变空，DWC_ssi 将延长时钟，直到 FIFO 有足够的数据继续传输。在读取的情况下，如果接收 FIFO 变满，DWC_ssi 将停止时钟，直到 数据已从 FIFO 中读取。





- DDR_DRIVE_EDGE

| Name | bits | Description                                                  |
| ---- | ---- | ------------------------------------------------------------ |
| TDE  | 7:0  | TXD 驱动边沿寄存器，决定发送数据的驱动边沿。该寄存器的最大值=(BAUD/2)-1 |



- XIP_WRITE_INCR_INST

  - 此寄存器只有当 `SSIC_XIP_WRITE_REG_EN` 配置成 1 时才可用。此寄存器用于存储指令 op-code，当 AHB 接口上有相同的 XIP 写请求时用作 INCR 传输。如果`SSIC_EN == 1`时无法写入。

  | Name            | bits  | Description                                                  |
  | --------------- | ----- | ------------------------------------------------------------ |
  | INCR_WRITE_INST | 15：0 | XIP Write INCR 传输 opcode。当 XIP_WRITE_CTRL.INST_L 不等于 0 时，DWC_ssi 发送指令给所有的 XIP 写传输，当一个 INCR 类型的 XIP Write 传输在 AHB 总线上发生请求，这个寄存器字段存储的指令 op-code 将发送。发送多少位由XIP_WRITE_CTRL.INST_L 字段决定。 |



- XIP_WRITE_CTRL

  - 此寄存器仅当 `SSIC_XIP_WRITE_REG_EN == 1` 时才有效。此寄存器用作存储一些 XIP 模式下的 XIP write 传输的信息。当`SSIC_EN=1`时不能写入此寄存器。

  | Name              | bits   | Description                                                  |
  | ----------------- | ------ | ------------------------------------------------------------ |
  | XIPWR_DFS_HC      | 21     | XIP 传输时固定的 DFS.配为1时则XIP 传输是将使用固定 CTRLR0.DFS 编程值。 |
  | XIPWR_WAIT_CYCLES | 20：16 | 增强模式下在控制帧发送和数据接收之间等待周期。这个值是SPI 时钟源的周期。 |
  | XIPWR_DM_EN       | 14     | 数据掩码使能位。当使能之后 txd_dm 信号用作屏蔽 txd 数据线上的数据。 |
  | XIPWR_RXDS_SIG_EN | 13     | 在 Hyperbus 传输地址和命令相位阶段使能 rxds 信号。此 bit 在命令-地址相位期间启用 hyperbus 目标设备的 rxds 信号。如果rxds 信号在CA相位传输期间设置成1，DWC_ssi 传输 (2*WAITCYCLES-1) wait cycles 之后地址相位完成。 |
  | XIPWR_HYPERBUS_EN | 12     | XIP Write 传输  Hyperbus 帧格式使能。选择  XIP Write 传输的数据帧格式是否处于 Hyperbus 模式。仅当 CTRLR0.FRF 设置为 SPI 帧格式时，该字段才有效。 |
  | WR_INST_DDR_EN    | 11     | 指令 DDR 使能位。使能指令阶段 DDR 传输。                     |
  | WR_SPI_DDR_EN     | 10     | SPI DDR 使能位。使能 DDR 传输在增强 SPI 帧格式。             |
  | WR_INST_L         | 9：8   | 增强 SPI 模式指令长度位。0x0:0  0x1:4bit 0x2:8bit 0x3:16bit  |
  | WR_ADDR_L         | 7：4   | 此字段定义地址将传输的长度。只有这些bits编程完成后 FIFO 传输才可以开始。<br />0x0:Reserved 0x1:4bit address 0x2:8bit 0x3:12bit 0x4:16bit 0x5:20bit 0x6:24bit 0x7:28bit 0x8:32bit |
  | WR_TRANS_TYPE     | 3：2   | 地址和指令传输格式。选择 DWC_ssi 是否在标准 SPI 模式或 XIP_WRITE_CTRL.FRF 字段中选择的 SPI 模式下传输指令和地址。<br />0x0:指令、地址都在标准模式；0x1:指令标准模式，地址在XIP_WRITE_CTRLR.FRF模式<br />0x2:指令、地址都在 XIP_WRITE_CTRL.FRF 模式；0x3:Dual Octal 模式，地址、指令都传输在8线，数据在16线 |
  | WR_FRF            | 1：0   | SPI 帧格式。选择传输数据时的数据帧格式。<br />0x0:标准；0x1:Dual；0x2:Quad；0x3:Octal(8 线) |

  





- psram 驱动：
  - master：
    - init 和普通的 spi 初始化一致
    - 读操作：组成 message，第一个 message 包括指令、地址、dummy，第二个 message 就是读数据，先往 psram 写指令、地址，然后读数据
    - 写操作：组成 message，先往  psram 写指令、地址，然后继续写数据
  - xip：
    - init ：
      - sdr 模式：如果是 qpi 模式下指令、地址、数据都是 qspi 模式，配置一些普通的 SPI 寄存器，配置 `INCR_INST、WRAP_INST、XIP_CTRL` 寄存器，(如果开启了 ddr 模式还需要配置  `DDR_DRIVE_EDGE` 寄存器)，配置`WRITE_INCR_INST、WRITE_WRAP_INST、WRITE_CTRL、XIP_MODE_BITS` 寄存器，(如果开启了 wrap，则需要通过发指令的方式配置 psram 的一些寄存器)，(如果开启了 qpi 模式，也需要通过发指令的方式使能 psram 的qpi 模式)
      - ddr 模式：跟 sdr 模式比较也只是多配了几个寄存器的某几 bit，具体的寄存器都是上面的那些
    - 读写：直接操作内存
    - dma：跑普通的 dma 流程，只不过源和目标都是内存





> cmd --> drv-apus/xxxx.c  --> hal/xxx.c
>
> hal 就是一些 ip 最基础的接口配置
>
> drv-apus 中没有实际的代码，都是通过将 hal 中一些基础的功能组合起来封装一个简单可用的功能
>
> cmd 下面调用 drv-apus 再来实现应用程序
>
> hal 就是属于真正的驱动，drv-apus、cmd 都是属于应用



## cmd/spi_test.c：通用 spi 测试程序

- 支持单倍速、双倍速、四倍速的测试，支持异步 DMA，支持8bit，不支持32bit

- init：dw_spi_probe、spi_setup、异步 DMA(注册回调)
- read：创建一个 message，sync   --> drivers/drv-apus/dw_spi.c transfer pump message 这套
  - 不使用 DMA：用 8bit 的方式，如果单线就把寄存器配好，多线也是把寄存器配好，然后使用 `RX_ONLY` 的方式读数据
  - 异步 DMA：用 8bit 的方式，走 dma 开中断那一套流程
- write：创建一个 message，sync  --> drivers/drv-apus/dw_spi.c  transfer pump message 这套
  - 不使用 DMA：用 8bit 的方式，如果单线就把寄存器配好，多线就把第一个数据作为指令发出去(用多线模式)，然后用 `TX_ONLY` 的方式发数据
  - 异步 DMA：用 8bit 的方式，如果单线就把寄存器配好，多线就把第一个数据作为指令发出去(用多线模式)，然后走 dma 开中断那一套流程
- format：配置单线、双线、四线模式，根据重复跑 spi_setup 实现的



## cmd/spi_nor_flash_test.c：flash spi 测试程序

- 支持单倍速、双倍速、四倍速测试，支持 dma，支持 ddr 模式，支持 8bit、32bit 模式，支持 xip 模式
- 依赖 hal 下 flash、spi 的驱动
- init：gx_nor_init --> drv-apus/spi_nor_flash.c，配一些基地址、信息、函数指针、探测 tx/rx fifo 深度、detect、使能四倍速
- xip_init：drv-apus/spi_nor_flash.c，配寄存器，主要是 XIP 相关的寄存器
- case：循环遍历每种场景，包括：单倍速、双倍速、四倍速、dma、ddr、8bit、32bit、xip 等混合组合进行读写擦、速度测试





## drivers/drv-apus/spi_nor_flash.c：flash spi 驱动

- 没有用 message 那一套，使用的是 prepare、transfer 的一套
- init：调用 hal 下面的 spi 接口来探测 tx/rx fifo 深度，函数指针赋值，这些值都由 `drv-apus/spi_nor_flash.c` 中实现，`hal` 里面就是通过掉这些值来实现功能的。做 detect、protect_init、enable_quad
- read_reg：prepare `EEPROM`模式配寄存器，transfer 传输数据，相当于每次只要调用 prepare 都会重复配置 spi 寄存器，包括 sample_delay 等
- write_reg：prepare `TX_ONLY`模式配寄存器，transfer 把命令发出去，继续transfer 把数据发出去
- write_then_read：prepare `EEPROM`模式配寄存器，transfer 先发再读

- nor_dma_tx：申请 dma 通道，申请 dma 保存链表所需的内存空间，配置 dma 寄存器，刷 cache，如果是单线或双线写的话需要单独发指令和地址，四线的会在 prepare 的时候把指令和地址长度配到 `SPI_CTRLR0` 寄存器，然后发走，开始 dma 传输，等待 dma 传输完成
- nor_dma_rx：申请 dma 通道，申请 dma 保存链表所需的内存空间，配置 dma 寄存器，刷 cache，启动 dma 传输，如果是单线读的话需要单独把指令、地址、dummy 发出去，然后等待数据被接收完成
- read_prepare：单线模式下需要使用 `EEPROM` 模式配一下水线、各种寄存器，其他模式的话需要把指令、地址、dummy 都准备好，使用 `RX_ONLY` 的方式配置好 `SPI_CTRLR0` 寄存器，然后发出去
- read：单线模式下需要把指令、地址、dummy 组好，使用 `EEPROM` 模式配置好寄存器，多线模式直接配置成 `RX_ONLY` 模式，开始读数据

- write_prepare：单线或双线模式下使用 `TX_ONLY` 的方式配置好 spi 寄存器，主要是配置水线，如果是多线模式把指令、地址的长度配置到 `SPI_CTRLR0` 寄存器，然后发出去
- write：单线或双线模式下使用 `TX_ONLY` 的方式把指令、地址发出去，多线模式不管，直接开始发数据
- irq：
- xip_init：配置 spi 寄存器，主要是 xip 那部分
- nor_read：包括 prepare、read，提供给 cmd/spi_nor_test.c 测试程序使用
- write_page：包括 写使能、prepare、write
- write：将要写的数据按照 page 切片，单次/多次调用 write_page。提供给 cmd/spi_nor_test.c 测试程序使用



## drivers/drv-apus/dw_spi.c：通用 spi 驱动

- 此驱动中维护的 `struct spi_data、spi_master` 独立，用作通用 spi
- 同步 dma tx：`clean cache`，配置 dma 寄存器，`clean cache`，启动 dma，等待 dma 传输完成
- 同步 dma rx：`invalid cache`，配置 dma 寄存器，`clean cache`，启动 dma，如果是单线的时候，需要发一个 dummy 到 `TXDR`<font color=green>(控制器必须要这么做，发一个 dummy 到 TXDR，否则 RXFIFO 一直是 empty 会卡住)</font>，等待 dma 传输完成
- inti：调用 probe，如果是 async 的情况注册tx和rx的回调
- setup：

- pump_transfer_async：
  - tx：多线模式下把第一个数据当成指令，按照多线`TX_ONLY` 的方式组好
    - dw_qspi_xfer_async：
      - gx_hal_qspi_transfer_prepare：把组好的第一个数据在 prepare 中发出去
      - dw_spi_sendmsg_dma_async：申请 dma 通道，把回调注册好，配置好 dma 寄存器，并打开 dma 中断，`clean cache`，启动 dma
    - dw_spi_xfer_async：
      - gx_hal_spi_transfer_prepare：不用组数据，在 prepare 中配置好 spi 控制器
      - dw_spi_sendmsg_dma_async：申请 dma 通道，把回调注册好<font color=green>( dma_tx_callback 回调的内容：等tx fifo empty、busy、关掉 SPI 的 dma、关掉 dma 通道、调用 `struct dw_spi_async_tran *device->tx_complete_cb` 也就是在 init probe 之后注册的回调函数)</font>，配置好 dma 寄存器，并打开 dma 中断，`clean cache`，启动 dma
  - rx：多线模式下把指令和地址的长度配成0，相当于和单线一样
    - dw_qspi_xfer_async：
      - gx_hal_qspi_transfer_prepare：在 prepare 中配置到 spi 控制器
      - dw_spi_recvmsg_dma_async：申请 dma 通道，把回调注册好，配置好 dma 寄存器，并打开 dma 中断，`clean cache`，启动 dma
    - dw_spi_xfer_async：
      - gx_hal_spi_transfer_prepare：不用组数据，在 prepare 中配置好 spi 控制器
      - dw_spi_recvmsg_dma_async：申请 dma 通道，把回调注册好，配置好 dma 寄存器，并打开 dma 中断，`clean cache`，启动 dma

- irq

- message

- transfer

- probe









## drivers/hal/src/spi/dw_spi_flash/gx_hal_sf.c：flash 驱动

- hal 之间不直接调用，通过调用到 `drv-apus/spi_nor_flash.c` 注册的驱动调到 `hal/src/spi/dw_spi/gx_hal_spi.c` 

- 一部分函数，都是上面那种方式实现的
- init：根据`drv-apus/spi_nor_flash.c` 中配置的回调都赋值到这边，可以直接使用。
- xip init：配置 xip 相关的 spi 寄存器
- 写保护功能
- uid 功能



## drjvers/hal/src/spi/dw_spi/gx_hal_spi.c：spi 驱动

- 这个驱动由很多其它的驱动(通用 spi、flash)使用，其它驱动在使用的时候会把自己的结构传进来，有了基地址之后就可以做区分了，就可以多个驱动同时使用这一套 spi 驱动

- 使能、失能控制器、配置分频、配置分频和Sample_delay、获取tx/rx fifo 深度、获取一些控制器的当前状态、配置`CTRLR1`寄存器(长度)、配置 tx fifo(深度)
- 传输：tx_and_rx、tx_only、rx_only、eeprom_rx
- spi_transfer_prepare：配置一些 spi 寄存器，单线模式用
- qspi_transfer_prepare：配置一些 spi 寄存器，多线模式用。多了指令、地址、dummy的长度配置到`SPI_CTRLR0`寄存器，然后把指令、地址发出去
- set_dma：开启 TX DMA、开启 RX DMA
- cs：各个芯片的 cs 操作方式不同
- 复位 spi
- spi_hw_init：硬件初始化，探测 `tx/rx fifo` 深度，一些 spi 寄存器
- irq：



# APUS 事业部板级烧录时，串口和 usb 烧录都要按住 boot 键再重启





# Nor Flash DDR模式

- SPI 控制器需要开启 DDR 模式，并且需要配置好 `DDR_DRIVE_EDGE` 来选择采样边沿

- Nor flash 目前只支持 DDR Read，不支持 DDR Write

- Flash 指令序列：

  - ![image-20240225162707531](image/image-20240225162707531.png)
  - ![image-20240225162637097](image/image-20240225162637097.png)

  - 1个字节的指令 + 3个字节的地址(ddr 模式) + 1 个字节的模式位(ddr 模式) + 4 dummy cycles(ddr 模式)
    - 2 + 4 其中的 2 可能指的是 模式位会占用 2 个 cycles
    - 1 + 7 指的是在 quad 的模式下一个 cycles 可以发 8bit 数据，所以一个字节的模式位可以在一个 cycles 发完，后面的 7 个 cycles 都当做 dummy
    - 总共的时钟周期就是：8(指令) + 6(地址) + 2(模式) + 4(dummy) = 20

  - 数据最起码要是一个字节

- 指令(单线) + 3字节地址(ddr 模式) + 1字节模式位(ddr 模式) + 4 dummy cycles
  - XIP 模式：指令和地址通过 `XIP_CTRL` 寄存器，模式位通过 `XIP_MODE_BITS`，`dummy cycle` 通过 `XIP_CTRL` 寄存器
  - 普通模式：指令和地址通过 `SPI_CTRLR0` 寄存器，地址长度配置成 32bit，实际的地址左移 8 bit，后面的 8bit 用来放模式位，`dummy cycle` 也通过 `SPI_CTRLR0` 寄存器配置


- 控制器在单线模式下不支持 DDR 模式，但 Flash 支持单线模式下的 DDR 模式指令







# Flash XIP 驱动问题

- 测试 XIP 的时候，把 icache 关掉，然后把数据读到 xip 地址上来测试，测试完了之后把 icache 打开
  - 为什么要是 icache？为什么不是 dcache？ 为什么不两个都关掉？
    - 因为 apus 上使用 xip 只映射了 Icache 的地址，没有映射 Dcache 的地址
    - xip 访问也都是通过 Icache 映射的地址访问的，所以只关 Icache 即可。如果访问 Dcache 映射的地址，那么就要关 Dcache
  - 怎么用的 xip？xip 上跑的是什么代码？
    - 跟 xip 上跑的代码无关
  
- memcpy 为什么可以用来测试 xip？
  - 因为 memcpy 函数是相当于从内存某个位置搬运数据到内存另一个位置
  - 相当于 memcpy 的源数据是从 xip 的地址取得，也就相当于 cpu 访问 xip

- dma tx 的时候为什么 dma 传输水线要设置成 4？

- nor_single_test 中的一段代码有什么意义？

  - 当 ` len > 0` 时用作 `single test` 测试指定的地址和长度；当长度为 `0` 是用于测试固定地址、固定长度

  ```c
  	if( cfg->len > 0 ) {
  
  		addr_tmp = cfg->addr;
  		len_tmp = cfg->len;
  
  		if(cfg->data_32bit && (cfg->addr % 4 != 0 || cfg->len % 4 !=0))
  			return CMD_RET_FAILURE;
  
  		if(cfg->is_print)
  			printf(">>>>>single test:addr=0x%x, len = %d\n", addr_tmp, len_tmp);
  		if(cfg->addr==0 && cfg->len==100)
  			cfg->len = len_tmp;
  		if(nor_single_rw(cfg))
  		{
  			printf("*****sflash_test_failed:addr=0x%x, len = %d\n\n\n", \
  					addr_tmp, len_tmp);
  			return CMD_RET_FAILURE;
  		}
  
  		if(cfg->is_print)
  			printf(">>>>>sflash_test_success.\n\n\n");
  		else
  			nor_test_show_process(i + 1, count);
  
  		printf("\n########## SINGLE TEST FINISH ########\n");
  
  		return 0;
  	}
  ```

- XIP 模式里面有一个 `MD_BITS_EN` 是用于在 XIP 模式下 Nor Flash 指令要求：

  - 指令(单线) + 地址(多线) + 模式位(多线) + 数据(多线)

- 什么是 XIP 的 Prefetch？

- 什么是 XIP 的 Continue？





# SPI 中断测试

- 注释掉发送FIFO的满判断，能产生FIFO溢出中断，中断测试正常

```diff
diff --git a/drivers/mtd/spinor/flash_spi.c b/drivers/mtd/spinor/flash_spi.c
index 7950ef6d..fcb39ec2 100644
--- a/drivers/mtd/spinor/flash_spi.c
+++ b/drivers/mtd/spinor/flash_spi.c
@@ -792,7 +792,7 @@ static int sflash_standard_1pageprogram(unsigned int addr, const void *buf, unsi
        writel(0x01, SPIM_SER);

        for (i = 0; i < len; i++) {
-               while(!(readl(SPIM_SR) & SR_TF_NOT_FULL));
+               //while(!(readl(SPIM_SR) & SR_TF_NOT_FULL));
                val = ((uint8_t*)buf)[i];
                writel(val, SPIM_TXDR_LE);
        }
```





# 右移 N 位 = 除以 2^N；左移 M 位 = 乘以 2^M







# GDB load xxx.elf

- gdb 在 load xxx.elf 的时候并不是把整个 .elf 都 load 过去，而是去找到 .elf 中的 text、data、bss 段然后加载到内存
- ecos 的 cmdline 会描述 mem 总大小，能够 malloc 的空间大小 = mem 总大小 - 程序的大小(.elf 的 text、data、bss)


# 并行 Nand Canopus 起得来，Sirius 起不来

- Canopus 和 Sirius 在 Stage1 都是将 oobsize 识别成了 64
- Stage2 驱动将 oobsize 做成了 128，boot 工具烧写是用的 Stage2 代码
- Canopus 在 Stage1 读 Stage2 的时候能够起来
- Sirius 在 Stage1 读 Stage2 的时候起不来



# ROM 读 Stage1 代码，以及同一款芯片的不同封装运行代码是否相同

1. rom 怎么读的 stage1 代码？比如 canopus 芯片，3215B 它从两个位置启动，在 rom 里把两个位置都配置成 spi？
	- 先配一组引脚复用，读一下数据看是否正确，然后再配一组引脚复用，再读数据是否正确
2. 比如哈 3215B、6631SH1D 两款芯片，配置引脚复用都是一样的？
    - 是的，因为跑的都是同一个 rom，程序都是一样的


# .h 调试手段，使用 #error 在编译时生成一个错误信息，用在不确定宏是否有效时使用

#error 是一个预处理指令，用于在编译时生成一个错误消息。当编译器遇到 #error 指令时，会立即停止编译，并显示指定的错误消息。这通常用于在特定条件下终止编译，或者提醒开发者注意某些重要信息。

以下是一个简单的示例，演示如何在代码中使用 #error：

```c
#ifdef CONFIG_DEBUG
    // 一些调试相关的代码
#else
    #error "Debugging is disabled. Enable CONFIG_DEBUG for debugging."
#endif
```

在上面的示例中，如果 CONFIG_DEBUG 宏未定义，则编译器会遇到 #error 指令，停止编译，并显示错误消息 "Debugging is disabled. Enable CONFIG_DEBUG for debugging." 这样可以确保在必要的情况下，开发者会收到明确的错误提示，以便采取相应的措施。





#                 Cannot enable the TUI: error opening terminal

解决方法：

```shell
-> % echo $TERM    
xterm-256color

-> % echo $TERMINFO
空的
```

- 问题就出在TERMINFO变量为空值，正常应该设置为xterm所在的目录

```shell
-> % whereis terminfo
terminfo: /usr/lib/terminfo /etc/terminfo /usr/share/terminfo /usr/share/man/man5/terminfo.5.gz
```

- 找到`xterm`在`/lib/terminfo/x/`目录下 `$sudo vi /etc/profile`

- 在尾部增加如下命令 `export TERMINFO=/lib/terminfo/`









# boot 工具烧 .boot 打印：

```shell
-> % sudo boot -b output/vega-fpga-sflash.boot -d /dev/ttyUSB1
[sudo] tanxzh 的密码： 
Found serial: /dev/ttyUSB1
wait ROM request... please power on or restart the board...
test boot_head:
phead->magic: 626f6f74
phead->version: 100
phead->chip: 6612
phead->baudrate: 1c200

write size 173648 byte ...
[===================>]  100.00%      data transmit ok.
�XXXBefore exec the first command, failed to receive "boot>" from stb in 3s, will EXIT_FAILURE
```



- boot 工具 `-b` 选项会先读这个文件，读到 malloc 的buffer，整个文件都读完
- 确认 buffer 开头是否为 `boot`，并创建一个新的 `boothead`
- 打印一下 `boothead`
- 等待接收 `X`，这个时候重启了板子，ROM 会发 `X`
- 将数据拷贝到新的 `boothead` 
- 发 `Y` 和 整个 bin 的大小写到串口
- 等待 `GET` 



- 烧 .boot 的时候，ROM 先启动以不同的晶振频率打 xxx
- boot 工具收到`X` 之后通过串口发 `Y`、整个 bin的大小写到串口，rom 就知道现在是 `boot` 工具，开始用串口启动
- boot 工具会发送 `stage1` 
- rom 收到之后，就开始跑 `stage1`，然后发送 `GET`

- 等待板子发 `GET` ，板子什么时候跑的 stage1？

  - rom 收到 `Y` 之后就从串口启动，接收 stage1，然后使用函数指针的方式将 PC 跑到 stage1

    ```c
    void (*stage1_entry)(struct loader_boot_info *) = (void (*)(struct loader_boot_info *))0x80400030;
    stage1_entry(&boot_info);
    ```

    

```
ROM:
	/* 使用 boot 工具 -b 指定 .boot 启动流程 */
	芯片启动后，设置不同的晶振频率(板子上会贴不同的晶振)来发送字符 `X`
	从串口获取字符 `Y`，如果获取到说明不是从 flash 启动
	从串口获取四个字节数据，作为整个 .boot 的大小
	从串口获取整个 bin 大小，保存到 sram
	flush cache & invalidate cache
	定义一个函数指针，指向保存 bin 数据的 sram 地址
	执行这个函数指针，相当于执行一个函数，将 pc 指针跳到 sram 地址开始执行
	
	执行 stage1 代码使用串口启动，从 .boot 中读 stage2 到 ddr
	汇编跳到 entry 执行
	
	
	/* 使用 flash 启动流程 */
	读取芯片 otp，若 uart 被 disable，则设置不同的晶振频率来发送字符 `X`
	读 flash 到 sram
		循环判断，Nor、SPI Nand、PNand，配置引脚复用
		如果是 nor flash 启动：
			SPI 初始化，配置一些寄存器，基址、cs
			用 4 字节模式先从 flash 0 地址读 4 个字节数据，看看是否是 0x55aa55aa
				如果是 55aa55aa 那么就是 4字节模式；如果不是再用 3 字节模式读
			flag 为 55aa55aa
			从 flash 0 地址读 16k 数据到 sram
			拉低 cs，发 03 命令，发 24 字节地址，读数据，拉高 cs
			将读到的数据拿出来做 crc 校验，是否正确
		如果是 spi nand 启动：
			用 nfc 读 spi nand flash 中的 stage1 数据到 sram
		如果是 pnand 启动：
			用 nfc 读 pnand 中的 stage1 数据到 sram
    flush cache & invalidate cache
    定义函数指针指向 sram 地址，pc 跳转到 sram 地址开始执行
    
    此时开始跑 stage1，stage1 中的代码会执行拷贝 stage2 到 ddr
	

boot:
	读取 -b 指令后跟着的文件到 buffer
	确认 buffer 开头是否为 `boot` 字符
	等待接收字符 `X`，收到后通过串口发送字符 `Y`
	继续发送整个 bin 的大小(4个字节)到串口
	继续发生整个 bin 到串口
```







# 代码在 xip 上运行，同时需要执行 flash 读写操作

## 情景1：所有代码都放在 xip 上运行，执行 flash 读写操作

- 当代码全部运行在 XIP 上，如果执行 flash 读写操作，会遇到以下问题：
  - 因为 xip 和 flash 读写操作都需要通过 spi 总线进行通信，当代码在 xip 上运行并且尝试 flash 读写时，会导致 spi 总线被同时占用。
  - 此时先执行 xip 操作，xip 获取了总线权，然后 flash 执行操作又尝试去获取总线权，无法获取就会卡死，因为 xip 没执行完，所以一直不会释放总线

## 情景2：flash 读写代码放在 sram，其它代码放在 xip，flash 读写代码中 某个接口放在 xip 上

- 首先执行 xip 操作，正常运行。然后执行 flash 读写操作，在执行的时候会先占用 spi 总线，然后又跳到 xip 操作了，xip 想要去占用总线，但是发现怎样都获取不到总线，而 flash 读写操作又没做完，所有不会释放总线。

## 情景3：flash 读写代码放在 sram，flash 读写代码中调用的所有代码都放在 sram，其它代码放在 xip

- 首先执行 xip 操作，正常运行。然后执行 flash 读写操作，由于在读写操作的时候先占用总线，然后执行所有的操作都是在 sram 中进行的，然后顺利执行完操作，释放 xip 总线。再进行 xip 操作，xip 占用总线，执行 xip 操作，执行完释放总线。



## 链接脚本将特定函数放在 ram

```makefile
SECTIONS
{
    .ram_text : AT(ADDR(.ram_text))
    {
        _sram_text = . ;

        . = ALIGN(4) ;

        KEEP(*(.app_hdr))

        *(.vectors)

        OBJ_DIR/platform/mcu/vectors.o(.text*)
        OBJ_DIR/platform/mcu/start.o(.text*)
        OBJ_DIR/platform/mcu/trap_c.o(.text*)
        OBJ_DIR/platform/mcu/system.o(.text*)
        OBJ_DIR/platform/mcu/clk*.o(.text*)
        OBJ_DIR/platform/utils/boot_env.o(.text*)
        OBJ_DIR/platform/hal/src/clock/apus/gx_hal_clk_apus*.o(.text*)
        OBJ_DIR/platform/hal/src/mmu/gx_hal_mmu.o(.text*)
        OBJ_DIR/platform/hal/src/spi/?*.o(.text*)
        OBJ_DIR/platform/hal/src/timer/gx_timer/gx_hal_gx_timer.o(.text*)
        OBJ_DIR/platform/drv-apus/src/vic.o(.text*)
        OBJ_DIR/platform/drv-apus/src/spi_nor_flash.o(.text*)
        OBJ_DIR/platform/drv-apus/src/timer.o(.text*)

#if (BUILD_ISMBB_ENABLE > 0)
        OBJ_DIR/platform/drv-apus/ismbb/?*.o(.text*)
#endif
        *(.ram_text)

        . = ALIGN(ALIGNOF(.ram_nocache_text)) ;
        _eram_text = . ;
    } > iram

    .ram_rodata : AT(LOADADDR(.ram_nocache_text) + SIZEOF(.ram_nocache_text))
    {
        _sram_rodata = .;
        . = ALIGN(4) ;
        OBJ_DIR/platform/mcu/vectors.o(.rodata*)
        OBJ_DIR/platform/mcu/start.o(.rodata*)
        OBJ_DIR/platform/mcu/trap_c.o(.rodata*)
        OBJ_DIR/platform/mcu/system.o(.rodata*)
        OBJ_DIR/platform/mcu/clk*.o(.rodata*)
        OBJ_DIR/platform/utils/boot_env.o(.rodata*)
        OBJ_DIR/platform/hal/src/clock/apus/gx_hal_clk_apus*.o(.rodata*)
        OBJ_DIR/platform/hal/src/mmu/gx_hal_mmu.o(.rodata*)
        OBJ_DIR/platform/hal/src/spi/?*.o(.rodata*)
        OBJ_DIR/platform/hal/src/timer/gx_timer/gx_hal_gx_timer.o(.rodata*)
        OBJ_DIR/platform/drv-apus/src/vic.o(.rodata*)
        OBJ_DIR/platform/drv-apus/src/spi_nor_flash.o(.rodata*)
        OBJ_DIR/platform/drv-apus/src/timer.o(.rodata*)

#if (BUILD_ISMBB_ENABLE > 0)
        OBJ_DIR/platform/drv-apus/ismbb/?*.o(.rodata*)
#endif
        *(.ram_rodata)
        *(.srodata*)

        . = ALIGN(ALIGNOF(.ram_data)) ;
        _eram_rodata = .;
    } > dram
}
```





## 使用宏将特定函数放在特定段

```c
#include "flash_driver.h"

void __attribute__((section(".ramfunc"))) Flash_Read(uint32_t address, uint8_t *buffer, uint32_t length) {
    FlashDriver_Read(address, buffer, length);
}

void __attribute__((section(".ramfunc"))) Flash_Write(uint32_t address, uint8_t *data, uint32_t length) {
    FlashDriver_Erase(address, length);
    FlashDriver_Write(address, data, length);
}
```







# Linux 编译 nfs 选项会打开 Root NFS

- 编译 linux 时使用 `./build gx3211 nfs` 编译方式默认打开使用 nfs 做根文件系统
- gxloader 中打开命令行选项使用 `nfs` 做根文件系统则会使用 nfs 作为根文件系统





# SCPU link.ld

![选区_536](image/选区_536.png)

# XIP 预取问题

- 什么是预取?
    使能 DWC_ssi 的 XIP 预取功能。一旦使能 DWC_ssi 将从下一个连续的位置预取数据帧，减少即将到来的连续传输的延迟。如果下一个 XIP 请求不是连续的，则预取会被丢掉。
- 什么是连续读?
    当 DWC_ssi 接收到一个 XIP 请求，地址从 AHB 接口传输到 SPI 接口方向。
    每个新的传输(XIP Read)在 AHB 接口都以相同的方式处理。因此，对于每个请求，一个新的地址必须发送到设备而
    如果一个内存设备允许在XIP传输处理片选信号，则 DWC_ssi 可以编程为连续模式来激活高性能模式。在这个模式，主机fuses 2个或多个 AHB burst 请求到 XIP 模式到单个 SPI 命令来确保在这些 burst 中命令和地址不是再次传输和主机控制器不需要等待任何临时 cycles
    在 XIP 模式下使能连续传输。如果此 bit 设置为 1，则 XIP 将会使能连续传输，在这个模式 DWC_ssi 将持续选中目标直到在 AHB 接口上有非 XIP 操作被选中。
        DWC_ssi 可以通过编程 SPI_CTRLR0.XIP_CONT_XFER_EN bit 为 1 来使能连续 XIP 模式。当次 bit 设置为 1，则 DWC_ssi 功能是连续 XIP 模式 as soon as 第一个 XIP 命令接收。
        对于第一个 XIP 传输，地址(和指令 如果 SPI_CTRLR0.XIP_INST_EN = 1)是发送到 SPI 接口。之后接收请求数据，DWC_ssi 连续保持目标设备选中，clock 保持默认状态。对于第二个 XIP 传输在 AHB 接口，DWC_ssi 恢复 clock 以及 命令地址传输到 SPI 接口和数据从设备获取。
        DWC_ssi 连续模式终止通过以下两种行为：
            XIP 接口上接收到非 XIP 命令
            当 AHB 传输是 non-consecutive 地址，片选移除和DWC_ssi 初始化一个新的 XIP 传输
- 什么是 flash 的连续 XIP 读?
    双线/四线模式下 XIP 读数据都有一个字节的模式位，如果模式位(M5-4 = 1,0)设置为对应的值，则下一次双线/四线读命令(CS 拉高再拉低)不需要(0xBB/0xEB)命令，只需要地址、模式位，即可读数据。

- 预取的限制：
    - XIP Prefetch 模式下所有传输的 HISZE 应该保持固定。
    - HSIZE:总线上传输的位宽

- 示例：Aquila 安全核，无 Cache
    - XIP 初始化完成后，使用 memcpy 函数拷贝 5 个字节数据，memcpy 会将数据先用 4 字节的方式拷贝，再用 1 字节的方式拷贝
    - 此时第一个访问 xip 时 HSIZE 为 32，第二次访问 xip 时 HSIZE 为 8
    - HSIZE 发生了改变导致拷贝数据出错
    - 注：Apus 也开启了预取，同样有此问题却未触发，是由于 APUS 默认有 Cache，每次访问都会从 Cache 获取数据，而 Cache 每次取数据都是 Cache Line，所以拷贝的位宽一直是一致的





# Gxloader GPIO 操作

- `board-init` 将要操作的 gpio 复用成 gpio 模式
- 在下面的数组中配置物理 gpio 对应的 虚拟 gpio，以及模式和初始电平
- 使用接口 `gx_gpio_setio(gpio, mode)`，mode = 0 是输入，mode = 1 是输出
- 使用接口 `gx_gpio_setlevel(gpio, level)`，levle = 0 是拉低，level = 1 是拉高



# Gxloader 验证 GXAPI Nos 接口

- GxAPI 是提供给事业部的接口，用于屏蔽系统层，调用统一的接口，我们在下面分别实现 nos、ecos、linux 的接口

- ecos 的 gxapi 接口可以用 ecos_shell 来验证

- linux 的 gxapi 接口可以用 `gxtools/test_case/flash_test/` 下的测试程序来验证

- nos 的 gxapi 接口需要在 gxloader 中来验证

  ```
  gxapi 是通过调用 gxloader 中的接口来实现的，所以需要用到 gxloader 的库
  gxloader 中调用 gxapi 需要依赖 gxcore.a
  编译思路如下：
  	1. gxloader 编译成库给到 gxapi 用
  	2. 编译 gxapi、gxosal、gxfrontend、gxavdev 等的 nos 库: ./build arm nos
  	3. gxloader 在 .config 中添加 ENABLE_THIRDLIB=y 来使得在编译时会调用 script/thirdlib.mk 来链接三方库
  	4. thirdlib.mk 中可以去掉一些不需要用到的库
  	5. gxloader 中使用 gxapi 的接口来实现测试程序
  ```

  



# 检测 Ubuntu 上是否识别到 usb 设备

`tail -f /var/log/syslog` 

`cat /dev/kmsg` 







# 软件实现一个类似 watch 的功能

- 可以随机的去检测要 watch 的地址，如果发生了改变，就打印这个值，或者做一个 `while(stop_flag)`



# GDB dump text 段

```
riscv64-unknown-elf-objdump -h apus.elf 
riscv64-unknown-elf-objcopy -O binary apus.elf output_file --only-section .dump_section
dump binary memory dump.file _sxip_text _exip_text    (gdb)
```





# Python 局域网传输文件

- Host 端：`python3 -m http.server 8080` 
  - 在要分享的目录下执行，或者使用 `-d` 指定目录
- Slave 端：访问网页`http://192.168.108.149:8080`





# 添加全局变量，保证指令顺序一致

- 测试代码：

  ```c
  config.sample_delay  = _get_nor_spi_sample_delay(config.freq_src);
  + 
  config.sample_delay  = 4;
  ```

  - 加了下面那句之后指令地址 + 4 了，相当于改变了指令顺序







# GDB 打断点到文件的某一行

- 行号是绝对行号

```
b drivers/spi/dw_spi/dw_spi_v2.c:458
```

指令`" b * if cond`设置条件断点，只要条`cond`满足，就可以触发断点







# 汇编测试：while(flag) 和 while(1)

- while(1) 编译器认为后面的代码都不会跑到，所以会编译器优化掉
- while(flag) 编译器不会优化

![image-20240923193605303](image/image-20240923193605303.png)





# XIP 启动的时候出现中断会不会打断 xip 取指

- 代码放在 xip 上，中断也放在 xip 上，xip 读 flash 数据的时候，发生了中断，此时中断不会打断前面的 xip 操作，这是由总线来保证的



# Canopus 6631SH1D SPINand Linux Ubifs 启动出现 kernel panic

启动失败` log `如下：

```
[    0.000000] Kernel command line: mem=68M@0x00000000 videomem=60M@0x04400000 mem_end console=ttyS0,115200 init=/init rw ubi.mtd=3 root=ubi0_0 rootfstype=ubifs mtdparts=spi_nand:512k@0m(BOOT),512k@512k(TABLE),4m@1m(KERNEL),123m@5m(ROOT) mtdparts 

[    1.046364] 4 cmdlinepart partitions found on MTD device spi_nand
[    1.052614] Creating 4 MTD partitions on "spi_nand":
[    1.057737] 0x000000000000-0x000000080000 : "BOOT"
[    1.064313] 0x000000080000-0x000000100000 : "TABLE"
[    1.070829] 0x000000100000-0x000000500000 : "KERNEL"
[    1.077473] 0x000000500000-0x000008000000 : "ROOT"

[    2.508992] ubi0: attaching mtd3
[    2.648554] ubi0: scanning is finished
[    2.685789] ubi0: volume 0 ("rootfs") re-sized from 496 to 960 LEBs
[    2.692935] ubi0: attached mtd3 (name "ROOT", size 123 MiB)
[    2.698674] ubi0: PEB size: 131072 bytes (128 KiB), LEB size: 126976 bytes
[    2.705736] ubi0: min./max. I/O unit sizes: 2048/2048, sub-page size 2048
[    2.712691] ubi0: VID header offset: 2048 (aligned 2048), data offset: 4096
[    2.719835] ubi0: good PEBs: 972, bad PEBs: 12, corrupted PEBs: 0
[    2.726094] ubi0: user volume: 1, internal volumes: 1, max. volumes count: 128
[    2.733505] ubi0: max/mean erase counter: 1/0, WL threshold: 4096, image sequence number: 183541576
[    2.742773] ubi0: available PEBs: 0, total reserved PEBs: 972, PEBs reserved for bad PEB handling: 8
[    2.752163] ubi0: background thread "ubi_bgt0d" started, PID 652
[    2.752360] VCC3P3: disabling
[    2.761608] ttyS0 - failed to request DMA
[    2.766086] VFS: Cannot open root device "ubi0_0" or unknown-block(0,0): error -19
[    2.773876] Please append a correct "root=" boot option; here are the available partitions:
[    2.782440] 0100           65536 ram0 [    2.786119]  (driver?)
[    2.788548] 0101           65536 ram1 [    2.792210]  (driver?)
[    2.794647] 0102           65536 ram2 [    2.798310]  (driver?)
[    2.800736] 0103           65536 ram3 [    2.804411]  (driver?)
[    2.806838] 0104           65536 ram4 [    2.810499]  (driver?)
[    2.812925] 0105           65536 ram5 [    2.816618]  (driver?)
[    2.819046] 0106           65536 ram6 [    2.822709]  (driver?)
[    2.825151] 0107           65536 ram7 [    2.828814]  (driver?)
[    2.831239] 0108           65536 ram8 [    2.834912]  (driver?)
[    2.837338] 0109           65536 ram9 [    2.841000]  (driver?)
[    2.843438] 010a           65536 ram10 [    2.847190]  (driver?)
[    2.849616] 010b           65536 ram11 [    2.853378]  (driver?)
[    2.855805] 010c           65536 ram12 [    2.859556]  (driver?)
[    2.861981] 010d           65536 ram13 [    2.865745]  (driver?)
[    2.868172] 010e           65536 ram14 [    2.871923]  (driver?)
[    2.874360] 010f           65536 ram15 [    2.878111]  (driver?)
[    2.880545] 1f00             512 mtdblock0 [    2.884660]  (driver?)
[    2.887087] 1f01             512 mtdblock1 [    2.891194]  (driver?)
[    2.893629] 1f02            4096 mtdblock2 [    2.897736]  (driver?)
[    2.900162] 1f03          125952 mtdblock3 [    2.904276]  (driver?)
[    2.906703] Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(0,0)
```

**使用新编译的 `kernel` 能够解决问题：**

- 与使用的 `flash.conf` 还是 `nand_flash.conf` 无关
- 与 `.conf` 中配置的 `kernel` 使用 `dtb + uImage` 还是 `kernel.img` 无关





# Linux、Gxmisc、Gxapi 调试

需求场景：需要调试 gxapi 驱动，涉及到 gxmisc、gxapi、linux kernel、test case，test case 可以通过 nfs 的方式更新，更新了 gxmisc 需要更新 linux kernel，gxapi 即使不需要重新编，那么也要更新 linux kernel，也就是说要重新烧录 bin，**有没有什么方法能够直接更新 `kernel`，最好是可以 gdb ?**

uboot 支持通过网络的方式加载 kernel，gxloader 不支持这种方式，所以用 jtag 加载 kernel 的方式比较方便





# Sirius GDB 打开 GDB_INFO 之后 load elf 过程中会失败

修改 `.gdbinit` 中的速度`10000 --> 5000` 后正常

失败 log 如下：

```
Downloading 4096 bytes @ address 0x00824000
Downloading 4096 bytes @ address 0x00825000
Downloading 4096 bytes @ address 0x00826000
Starting target CPU...
Reading all registers
WARNING: Failed to read memory @ address 0x00699EC8
WARNING: Failed to read memory @ address 0x00699EC4
WARNING: Failed to read memory @ address 0x00699EC8
WARNING: Failed to read memory @ address 0x00699EC4
WARNING: Failed to read memory @ address 0x00699EC8
WARNING: Failed to read memory @ address 0x00699EC6
WARNING: Failed to read memory @ address 0x00699EC4
WARNING: Failed to read memory @ address 0x00699EC8
WARNING: Failed to read memory @ address 0x00699EC6
WARNING: Failed to read memory @ address 0x00699EC4
WARNING: Failed to read memory @ address 0x00699EC8
WARNING: Failed to read memory @ address 0x00699EC4
WARNING: Failed to read memory @ address 0x00699EC8
WARNING: Failed to read memory @ address 0x00699EC4
WARNING: Failed to read memory @ address 0x00699EC8
WARNING: Failed to read memory @ address 0x00699EC8
WARNING: Failed to read memory @ address 0x00699EC8
GDB closed TCP/IP connection
ERROR: Failed to open remote socket. GDBServer will be closed.
Restoring target state and closing J-Link connection...
Shutting down...
Remote connection failed.%   
```





# Vega 修改 GPIO 驱动能力

查看文档`管脚复用`章节，查找要修改的 `gpio` 对应的寄存器偏移地址，其中 **gpioDS 输出电流控制** 表示驱动强度，`00` 最小，`11`最大

对应的寄存器基址为：`芯片架构-->模块地址--> pin_config`



# 读 flash 操作使用 DMA 操作流程为什么要 invalidate cache

- 使用 dma 之前先 `clean buffer`，这里涉及到 buffer 的所有权问题，buffer 在交给 dma 之前，可能数据还在 cache 中，不在内存中，所以在交给 dma 之前，cpu 需要将自己的事情都干完，把数据从 cache 刷到内存
- 在 dma 操作的时候，有可能 dma 操作还没完成，但是 cpu 会有投机访问的机制，会把 dma 所在的 buffer 先投机访问到 cache 中，认为可能会使用到，那么等 dma 操作完成之后，cache 里面的数据缓存的还是 dma 操作之前的，cpu 要用的话就是用的老的数据，所以这里需要 `invalidate`







# gdbserver 调试 linux 应用

`Sirius 6633H1 Socket` 测试板，编译好 `gxloader`，并且在 `gxloader` 中关掉 `cache、mmu`，烧到板子中

板子起来之后，连接上 `GDB_SERVER`，然后使用 `gdb` 加载 `linux`

此时 `linux` 已经可以正常起来了，但是想要调试应用断点是打不到应用代码上的，就需要使用应用的 `gdbserver`

- 将工具链的 `gdbserver：arm-nationalchip-linux-uclibcgnueabihf/debug-root/usr/bin/gdbserver` 拷贝到 `nfs` 目录

- 板子上打开 `server`：

  ```shell
  # 1234:端口号
  # flash.elf 要监听的程序
  ./gdbserver 192.168.108.149:1234 flash.elf
  ```

- PC 上开始执行程序：

  ```shell
  arm-linux-uclibcgnueabihf-gdb flash.elf
  
  # 连上 server 的 ip 以及端口号
  target remote 192.168.108.150:1234
  load
  ```

- 开始在 gdb 中调试程序，建议打断点到 `main`，因为应用程序跑完了之后 gdb 就自己断了，要再跑就要重新连接 gdb

- 如果需要参数，可以起来之后配置

  ```shell
  set argc = 2
  set argv[1] = "comtest"
  ```

  



# Apus 引脚复用

- Apus 的引脚可以进行全功能复用，也就是说每个 gpio 都可以复用成任何功能
- flash 引脚默认就是 flash 功能，不需要引脚复用，并且也改不成其它功能
- 在引脚复用全功能表格上看，如果提供了配置的方法，那就需要配引脚复用。如果没提供，就说明默认就是那个功能
- pinmux.c 用于配置引脚复用，如果 pinmux.c 无法配置，那么就说明是默认的功能



# Sample_delay

- 为什么需要 Sample_delay?
  - 由于主控发出 clk，需要经过电路板上的延迟，从设备准备数据的延迟，然后返回数据到主控的延迟，导致无法发出 clk 就立即采数据，需要等待一定的时间来采数据
- Sample_delay：用来做采样延迟，用 spi 的模块频率为基准来延迟
- 假如 spi 模块频率工作在 160MHz，flash 工作在 80MHz，相位范围是 2、3、4，那么对应在波形上的延迟大概是 12.25ns ~ 22.5ns
  - 在读 id 或读数据的时候，可以从波形上抓出来
  - 写的时候没有 sample_delay 的问题
  - 有待验证！！



# Linux shell 中自动获取 IP 地址

- `udhcpc`



# Gen_SPI 芯片测试

- master 对接 flash
  - 取消注释 `dw_spi_v2.c` 中的宏，正常测试
- master 对接 gen spi slave
  - 8bit、32bit
    - cpu 传输(ok)、同步 dma(ok)、异步 dma(ok)
- slave 对接 gen spi master
  - 8bit、32bit
    - cpu 传输(ok)、同步 dma(ok)

> - gen spi master 频率调到 100k
> - master 测试的时候打开打印，slave 测试的时候不要打开打印



Master 向 Slave 发/收数据的过程中 CLK 来源都是 Master，所以 Slave 内部会去采样 Master 的工作时钟，这里是用的 Slave 的 SPI 模块频率采样。那么只需要保证 Slave 的模块频率是 Master 的工作频率的 6~8 倍(或者更高) 就可以保证数据传输过程的 CLK 是稳的。



Master 和 Slave 互相发送/接收数据，Master 在发数据的时候，Slave 读数据需要考虑 Sample delay；Slave 发数据的时候，Master 需要考虑 Sample delay；并且这里的 Sample delay 和 Master 的工作时钟相关。

Master 的测试 case 需要考虑 Slave 是否准备好数据，因为 Master 可以空读和空写，也就是说如果在发送完成之后 delay 的时间过短，Slave 还没有把数据发出来就已经开始读了，这里会读到错误的数据，所以可以增大等待时间。

Slave 的测试 case 会刷 cache 和 无效 cache 等操作很耗时，会导致 Slave 处理数据速度很慢，不用 dma 时不需要这些操作。

对于测试没有收完的情况，可以考虑是由于 Slave 或 Master 处理数据过慢或过快导致的。

- 增大 Master 测试 case 中的延时时间
- 去掉 Slave 测试 case 中的 cache 操作

对于测试读写错误的情况，可以考虑是由于 Slave 或 Master 的 Sample delay 不对应导致的。

- 修改 Master 的 Sample delay
- 修改 Slave 的 Sample delay





# 函数参数是指针还是结构体变量

- 如果函数的参数以结构体变量的形式传递，在 b 到函数之前会先把所有参数都拷贝通用寄存器，然后再调用函数，函数中会从通用寄存器中取出数据到内存再使用
- 如果函数的参数以结构体指针的形式传递，在 b 到函数之前只需要将地址拷贝到通用寄存器 R0 即可，然后调用函数，函数中会直接从 R0 指向的内存地址直接取数据用

```c
struct test_st {
	int a;
	int b;
	int c;
};

void func_a(struct test_st st_a)
{
	int x;
	x = st_a.a + st_a.b + st_a.c;
}

void func_b(struct test_st *st_b)
{
	int y;
	y = st_b->a + st_b->b + st_b->c;
}

struct test_st ab = {
	.a = 10,
	.b = 20,
	.c = 30,
};

void main(void)
{

	func_a(ab);

	func_b(&ab);
}

arm-none-eabi-gcc test.c -o test -specs=nosys.specs
arm-none-eabi-objdump -S test > test.dump
    
0000821c <func_a>:
    821c:	e52db004 	push	{fp}		; (str fp, [sp, #-4]!)
    8220:	e28db000 	add	fp, sp, #0
    8224:	e24dd01c 	sub	sp, sp, #28
    8228:	e24b3018 	sub	r3, fp, #24
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    822c:	e8830007 	stm	r3, {r0, r1, r2}
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    8230:	e51b2018 	ldr	r2, [fp, #-24]
    8234:	e51b3014 	ldr	r3, [fp, #-20]
    8238:	e0822003 	add	r2, r2, r3
    823c:	e51b3010 	ldr	r3, [fp, #-16]
    8240:	e0823003 	add	r3, r2, r3
    8244:	e50b3008 	str	r3, [fp, #-8]
    8248:	e24bd000 	sub	sp, fp, #0
    824c:	e49db004 	pop	{fp}		; (ldr fp, [sp], #4)
    8250:	e12fff1e 	bx	lr

00008254 <func_b>:
    8254:	e52db004 	push	{fp}		; (str fp, [sp, #-4]!)
    8258:	e28db000 	add	fp, sp, #0
    825c:	e24dd014 	sub	sp, sp, #20
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    8260:	e50b0010 	str	r0, [fp, #-16]
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    8264:	e51b3010 	ldr	r3, [fp, #-16]
    8268:	e5932000 	ldr	r2, [r3]
    826c:	e51b3010 	ldr	r3, [fp, #-16]
    8270:	e5933004 	ldr	r3, [r3, #4]
    8274:	e0822003 	add	r2, r2, r3
    8278:	e51b3010 	ldr	r3, [fp, #-16]
    827c:	e5933008 	ldr	r3, [r3, #8]
    8280:	e0823003 	add	r3, r2, r3
    8284:	e50b3008 	str	r3, [fp, #-8]
    8288:	e24bd000 	sub	sp, fp, #0
    828c:	e49db004 	pop	{fp}		; (ldr fp, [sp], #4)
    8290:	e12fff1e 	bx	lr

00008294 <main>:
    8294:	e92d4800 	push	{fp, lr}
    8298:	e28db004 	add	fp, sp, #4
    829c:	e59f3018 	ldr	r3, [pc, #24]	; 82bc <main+0x28>
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    82a0:	e8930007 	ldm	r3, {r0, r1, r2}
    82a4:	ebffffdc 	bl	821c <func_a>
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    82a8:	e59f000c 	ldr	r0, [pc, #12]	; 82bc <main+0x28>
    82ac:	ebffffe8 	bl	8254 <func_b>
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    82b0:	e24bd004 	sub	sp, fp, #4
    82b4:	e8bd4800 	pop	{fp, lr}
    82b8:	e12fff1e 	bx	lr
    82bc:	0001072c 	.word	0x0001072c

```





# Nor flash 4字节地址测试方法

- Nor flash 由于 3 字节最大能寻址的范围是：`2 ^ 24 = 0x1000000` 也就是 16MB，那么对于超过 16MB 的 flash，就需要用 4 字节地址的方式来寻址
- 一般通过配置某个寄存器的某一 bit 来达到进入四字节地址模式，一般这个 bit 都是`易失性的`，也就是掉电会重置，所以每次上电都要设置一下 
- 那么可以通过进入四字节地址模式后，使用 **看门狗复位** 的方式来重启软件，如果重启失败，则说明 rom 不支持，否则 rom 支持四字节地址









# Gxmisc 坏块处理逻辑

![2024-11-06_11-04](image/2024-11-06_11-04.png)









# 并行 Nand 烧录器 bin

- 烧录器的 ecc 算法：烧录器支持一些标准的 ecc 算法，或者一些大的厂商的 ecc 算法，我们的算法是不被烧录器所支持的，需要采用直烧的方式来烧录

- 烧录器和 `boot` 工具的差异：

  - `boot` 工具：用的是我们自己的 nfc 控制器，烧录的 bin 是只包含 main 区域的数据，在烧录的时候是用的 nfc 控制器来写 main 区域，同时 nfc 控制器自己会往 oob 区域填充 ecc 码
  - 烧录器：用的是烧录器厂家的控制器，烧录 bin 时只包含 main 区域的数据时不会往 oob 区域填充 ecc 码
  - 因此用烧录器直接烧只包含 main 区域的 bin 是无法起来的，因为芯片起来的时候会依赖 oob 区域的 ecc 码，而这样是没有 ecc 码的

- 由于 烧录器直烧的方式无法启动，所以我们做了一个 nand_calc_ecc 工具，通过输入`包含 main 数据的 bin`，在这个 bin 中根据 main 数据计算出 ecc 码然后插入到对应的 oob 区域，然后生成一个 `包含 main + oob 数据的 bin`，以及一个 `烧录器用到的 .def` 文件

- 客户烧录方法：

  - 用我们的工具将`原始 bin` 生成 `目标 bin + .def 文件`
  - `main bin --> nand_calc_tool --> main + oob bin + .def`
  - 客户采用直烧的方式烧录这个生成的 bin

  ```
  原始 bin 文件（仅含 main 数据）
      ↓
  nand_calc_ecc 工具处理
      ↓
  生成两个文件：
  1. main + OOB 的 bin 文件
  2. .def 文件
  ```

  

> 对于并行 Nand 自带片上 ecc：之前的烧录器做法是直接烧 `main + oob 的 bin 文件`，让 flash 自己来处理





