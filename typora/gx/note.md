https://gitee.com/progit/2-Git-%E5%9F%BA%E7%A1%80.html# --git手册
http://www.ruanyifeng.com/blog/2020/04/git-cherry-pick.html --cherry-pick
https://zhuanlan.zhihu.com/p/413646220    --wechat
http://www.deansys.com/doc/ldd3/index.html    --linux driver

- git reset HEAD^ --hard 回到上一次的提交

- :%s/源字符串/目标字符串/g ---全局替换

- {{patch(Gerrit号)}}

- commit -m "302932: issue"

- c语言规范：
  
  - 函数名：小写单词之间用下划线 (is_prime)
  - ,;后面要有一个空格
  - 循环/判断语句后有一个空格 {前有一个空格
  - 变量初始化赋初值
  - 变量定义与程序空一行
  - 双目运算符前后加空格
  - 单目运算符前后无空格
  - 不允许出现多余的空白字符
  - 空行不能多

- scpu 下载命令：`sudo bootx -m grus -t s -c download 0 gxscpu.bin`
  
  - 重启k2开关

- Jtag
  
  1. 开JTAG的后台
      `DebugServerConsole -setclk 2`
  
  2. 打开minicom
      `sudo minicom`
  
  3. 打开gdb调试
      `csky-abiv2-elf-gdb gxscpu.elf`
  
  4. 开启csky的jtag服务
      `target jtag jtag://127.0.1.1:1025`
  
  5. 加载elf文件
      `load gxscpu.elf`
  
  6. break到main函数
  
  7. 运行
  
  8. 注意：
     
     - make menuconfig中要打开jtag debug
     
     - 编译之前要先clean，因为.c文件没变化默认.o文件是最新的
     
     - 运行前板子中程序要为空 boogx把flash文件擦除：`sudo bootx -m grus -t s -c flash eraseall`
     
     - 每次运行gdb前要重启板子
  
  9. 半自动运行：
     
     1. 在有.elf目录下创建.gdbinit文件
     
     2. 在.gdbinit文件中写入
        
        ```
        target jtag jtag://127.0.1.1:1025
        load
        ```
     
     3. 运行`csky-abiv2-elf-gdb gxscpu.elf`

- 文档阅读：`rdesktop 2rdc.guoxintech.com -z -a 16 -g 1200x830 -u tanxzh  -p Txz991103`

- `sudo minicom -D /dev/ttyUSB0`

- `minicom -D /dev/ttyUSB0 -C 1.log`

- 命令`./boot -b gx.boot -c usbdown 0 1.bin -d /dev/ttyUSB0` 

- `smb://192.168.110.253/public/ftp/stb/GoXceed_Bus_Manual_v1.9-test/125/taurus_6605H1_linux_dvbs`

- http://git.nationalchip.com/redmine/issues/304240 --- boot命令如何使用的redmine

- 切换分支
    `repo init -b 分支名（如 v1.0.2-9）如果想要查看仓库信息可以看 .repo/manifest.xml`

- 查看可切换的分支
    cd .repo/manifests
    git branch -a | cut -d / -f 3

```shell
repo init -b imx-4.1.15-1.0.0_ga
repo sync   # 如果不需要与服务器数据一致，可以不运行该步
repo start imx-4.1.15-1.0.0_ga --all 
```

- 查看切换结果
    `repo branches`

- 前进或回退：
    `git reflog`
  
    定义：查看命令操作的历史
  
    查找到你要的 操作id，依旧使用 上文说的 `git reset --hard id`。又回退到当初一模一样的版本啰！

- git场景
  
  - repo upload 以后后悔了，怎么办？
    
      在原有分支上修改文件
    
      git commit –amend
    
      repo upload –replace 404
    
      出现文件以后去掉第 1 行的 #，中括号里面填上 change-id
  
  - repo upload 的时候出错或者忘记 repo start 的时候怎么办？
    
      重新 repo init 一个工程
    
    ```
      repo start xxx project
      cd project
      git fetch path branch:test ---path是原来工程的路径
      git merge test
      repo upload
    ```
  
  - repo sync 同步不成功
    
    很有可能你是你本地修改还没有提交
    
    可以使用 `git stash` 缓存当前工作
    
    `repo sync` 同步成功
    
    `git stash apply` 恢复前面的工作
    
    本方法同样适用于分支接换
  
  - `repo upload` 报 missing blob 或者 missing tree 怎么办
    
    该问题最大可能是 git 版本不对，根据实验 1.9.1 版本可能会出该问题，但是 1.7.1 或者 2.9.5 没有发生该问题，可以通过重新安装 git 来解决。

- 直连网线：eth0

- usb转以太网：eth1

- usb网卡：WLAN

- 在接触一个新的工程时，首先要知道工程的功能、编译方法

- nfs出来的目录做根(/root)
  
  - make menuconfig中开nfs的root

- 不用手动指定ip，直接在loader中制定好，然后挂载

- 然后把动态库加载到/root目录下去，(gdbserver需要一个.so-->/root)

- git 可以打补丁 但是打完补丁之后再次提交就提交在补丁上了

repo 无法使用：

- python --> python2.7

- 在虚拟 anaconda 环境，在~/.zshrc中删除掉这个bin的PATH

- BootLoader通常保存在ROM中，为了避免改变BootLoader时需要改变ROM硬件，故在ROM中保留很小的BootLoader(stage1)，将完整功能的BootLoader存放在磁盘的启动块(stage2:ddr)

- 使用vivado时应加sudo

- gdb：查看外设寄存器：x /20x 0x12345678
  
  - x：16进制
  - 20：查看20个word
  - 0x12345678：寄存器地址

- 使用vscode时，点击跳转可能会跳到错误的位置，要使用全局搜索确认一下

- bc ：命令行进制转换工具
  
  - echo "ibase=16;obase=10;1F4" | bc
  - ibase：输入进制
  - obase：输出进制，10进制时不需要填写
  - 1F4：要转换的数据，必须为大写

- 请求中断前，要先从芯片手册中查看是否有对应的中断号

- `\\`：查看打开的所有文件

- `\ space`：`nohl`

- bootloader烧在flash上

- spi flash:
  
  - standard:cs/miso/mosi/clk
  - dual:miso/mosi:I/O
  - quad:miso/mosi/wp/hold:I/O

- git  打错补丁：
  
  - 删掉现在的代码，重新repo一份下来
  - 新建一个分支，这个分支就回到了主线上，没有打任何的补丁
  - 先回退到一个觉得正确的版本，然后repo sync新的版本

- \>：添加到文件中去

- \>>：追加到文件中去

- Flash存储特点：
  
  - 数据只能从1到0
  
  - 要写之前必须要先擦除



- 网盘链接更新：# 320283 新人培训



- 比较工具：`bcompare file1 file2`






#### Ubuntu20.04出现无法定位软件包：

- 一般安装包无法定位是安装源的问题，ubuntu中直接`sudo apt-get update`再重试
- 仍然报错，则需要修改下载源文件，`sudo vim /etc/apt/sources.list`文件
- 在文件末尾添加语句并保存退出`deb http://us.archive.ubuntu.com/ubuntu trusty main universe`
- 再次更新安装即可
- sudo apt-get update
- sudo apt-get install xxxx