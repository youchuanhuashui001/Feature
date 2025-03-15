https://zhuanlan.zhihu.com/p/398126847
https://docs.beeware.org/zh-cn/latest/tutorial/tutorial-0.html


## # 安装

根据 BeeWare 的文档说明，在 Windows 上使用，我们首先需要安装 Git 和 WiX Toolset，根据给出的网址，下载安装即可。

然后，我们使用 pip 工具安装 BeeWare：

```text
pip install briefcase
```

## # 创建应用

BeeWare 安装完成之后，我们就可以通过`briefcase`命令在命令行终端进行 BeeWare 应用的管理，比如新建、运行、构建、打包等等。

我们先使用命令`briefcase new`创建一个应用。

命令输入之后，会让我们输入「应用的正式名称」、「应用程序名称」、「域名」、「项目名称」等等信息，在这里出于演示，我们统统使用默认值。

  

![](https://pic1.zhimg.com/v2-83c1b21821a7abd0353ba4dd57f12c30_1440w.jpg)

  

输入完成之后，BeeWare会开始创建应用，创建完成之后，会有如下提示：

  

![](https://pic4.zhimg.com/v2-9cc1833d4bd11a992554763828b45fc3_1440w.png)

  

同时目录下多出了一个与应用程序名称同名的目录：
![](https://picx.zhimg.com/v2-a49b7319a42d344fb7f31fba4f5ee50b_1440w.jpg)
我们的程序的主要代码都将在 app.py 里面编写，默认 app.py 文件内已经有一个demo代码，我们可以直接运行项目：

```text
briefcase dev
```

在命令行输入上述命令，会生成一个如下图所示的窗口：

  

![](https://pic2.zhimg.com/v2-aa1b16a8f8cf507c0970f8249d8a20fd_1440w.jpg)


## # 打包为安卓APP

如果我们要将应用打包为安卓APP，过程也是类似的。

首先，创建应用的安卓脚手架：

```text
briefcase create android
```

接着，构建安卓应用：

```text
briefcase build android
```

  

![](https://pic4.zhimg.com/v2-c35b03f18b7754fe40fc07b81b6ddceb_1440w.png)

  

然后，我们运行一下构建好的安卓应用：

```text
briefcase run android
```

在这里会让我们选择设备，可以选择 BeeWare 提供的安卓虚拟机或者是在电脑上连接自己的手机，在这里，我们选择安卓虚拟机：

  

![](https://pica.zhimg.com/v2-a5cd35cd9be33777f4587bd2ca99453a_1440w.jpg)

  

最后，打包安卓应用：

```text
briefcase package android
```

打包完成之后，我们可以在 .\android\gradle\Hello World\app\build\outputs 找到打包好的文件：

  

![](https://pica.zhimg.com/v2-57130784c9ab62f289f1e4d663dff86e_1440w.jpg)

  

BeeWare 提供了两种打包好的文件，一种是用于上架Google Play 的.aab 格式文件，

  

![](https://pic1.zhimg.com/v2-fa686fc01d0b63f1a445db26cd756292_1440w.jpg)

  

一种是用于调试的 .apk 文件:

  

![](https://pica.zhimg.com/v2-a142c51bb7cbc85fb8e014c0efa48088_1440w.jpg)

  

apk 文件咱们的手机可以直接安装，所以就用QQ把它传到手机上：

  

![](https://pic2.zhimg.com/v2-3e27a0b387b9fe2d8fadebbd17499451_1440w.jpg)




# 遇到的问题：
## cmake 版本过低：
--> % briefcase dev

[helloworld] Installing requirements...
Looking in indexes: https://pypi.doubanio.com/simple/
Collecting toga-gtk~=0.4.5
  Downloading https://mirrors.cloud.tencent.com/pypi/packages/da/76/217d16438483626c8c494ee7674246fe1dbe0d82143573b215d288ecd775/toga_gtk-0.4.7-py3-none-any.whl (60 kB)
Collecting pytest
  Using cached https://mirrors.cloud.tencent.com/pypi/packages/30/3d/64ad57c803f1fa1e963a7946b6e0fea4a70df53c1a7fed304586539c2bac/pytest-8.3.5-py3-none-any.whl (343 kB)
Collecting pycairo>=1.17.0 (from toga-gtk~=0.4.5)
  Using cached pycairo-1.26.1-cp38-cp38-linux_x86_64. whl
INFO: pip is looking at multiple versions of toga-gtk to determine which version is compatible with other requirements. This could take a while.
Collecting toga-gtk~=0.4.5
  Downloading https://mirrors.cloud.tencent.com/pypi/packages/27/38/2aa64f466dfb61f2580294ae9c7a50d4e2a1e83df6d778c7599db3261da8/toga_gtk-0.4.6-py3-none-any.whl (60 kB)
Collecting gbulb>=0.5.3 (from toga-gtk~=0.4.5)
  Downloading https://mirrors.cloud.tencent.com/pypi/packages/f6/8e/8055b9cc6c47e125b4758d716e12dfbf5f146a83d6e1f0417a3c8d4be269/gbulb-0.6.6-py3-none-any.whl (19 kB)
Collecting pygobject>=3.46.0 (from toga-gtk~=0.4.5)
  Downloading https://mirrors.cloud.tencent.com/pypi/packages/f9/9e/6bab1ed3bd878f0912d9a0600c21f3d88bab0e8a8d4c3ce50f5cf336faaf/pygobject-3.48.2.tar.gz (715 kB)
     ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ 715.2/715.2 kB 715.8 kB/s eta 0:00:00
  Installing build dependencies: started
  Installing build dependencies: finished with status 'done'
  Getting requirements to build wheel: started
  Getting requirements to build wheel: finished with status 'done'
  Installing backend dependencies: started
  Installing backend dependencies: finished with status 'done'
  Preparing metadata (pyproject. toml): started
  Preparing metadata (pyproject. toml): finished with status 'error'
  error: subprocess-exited-with-error
  
  × Preparing metadata (pyproject. toml) did not run successfully.
  │ exit code: 1
  ╰─> [24 lines of output]
      + meson setup /tmp/pip-install-mwvkt2lj/pygobject_b3d53f13f0de4d2d900cc1b088e98c5a /tmp/pip-install-mwvkt2lj/pygobject_b3d53f13f0de4d2d900cc1b088e98c5a/. mesonpy-1kadj40i -Dbuildtype=release -Db_ndebug=if-release -Db_vscrt=md -Dtests=false -Dwheel=true --wrap-mode=nofallback --native-file=/tmp/pip-install-mwvkt2lj/pygobject_b3d53f13f0de4d2d900cc1b088e98c5a/. mesonpy-1kadj40i/meson-python-native-file. ini
      The Meson build system
      Version: 1.7.0
      Source dir: /tmp/pip-install-mwvkt2lj/pygobject_b3d53f13f0de4d2d900cc1b088e98c5a
      Build dir: /tmp/pip-install-mwvkt2lj/pygobject_b3d53f13f0de4d2d900cc1b088e98c5a/. mesonpy-1kadj40i
      Build type: native build
      Project name: pygobject
      Project version: 3.48.2
      C compiler for the host machine: cc (gcc 9.4.0 "cc (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0")
      C linker for the host machine: cc ld. bfd 2.34
      Host machine cpu family: x86_64
      Host machine cpu: x86_64
      Program python3 found: YES (/home/tanxzh/tanxzh/code/python/bee_ware/helloworld/myenv/bin/python3)
      Found pkg-config: YES (/usr/bin/pkg-config) 0.29.1
      Run-time dependency python found: YES 3.8
      Did not find CMake 'cmake'
      Found CMake: NO
      Run-time dependency gobject-introspection-1.0 found: NO (tried pkgconfig and cmake)
      Not looking for a fallback subproject for the dependency gobject-introspection-1.0 because:
      Use of fallback dependencies is disabled.
      
      ../meson.build:29:9: ERROR: Dependency 'gobject-introspection-1.0' is required but not found.
      
      A full log can be found at /tmp/pip-install-mwvkt2lj/pygobject_b3d53f13f0de4d2d900cc1b088e98c5a/.mesonpy-1kadj40i/meson-logs/meson-log.txt
      [end of output]
  
  note: This error originates from a subprocess, and is likely not a problem with pip.
error: metadata-generation-failed

× Encountered error while generating package metadata.
╰─> See above for output.

note: This is an issue with the package mentioned above, not pip.
hint: See above for details.
Installing dev requirements... errored

Unable to install requirements. This may be because one of your
requirements is invalid, or because pip was unable to connect
to the PyPI server.

Log saved to /home/tanxzh/tanxzh/code/python/bee_ware/helloworld/logs/briefcase. 2025_03_10-13_55_15. dev. log

(myenv) tanxzh@tanxzh-Lenovo [13 时 55 分 16 秒] [~/tanxzh/code/python/bee_ware/helloworld] 
-> % cmake --version
zsh: command not found: cmake
(myenv) tanxzh@tanxzh-Lenovo [13 时 55 分 33 秒] [~/tanxzh/code/python/bee_ware/helloworld] 
-> % sudo apt install -y software-properties-common lsb-release
[sudo] tanxzh 的密码： 
对不起，请重试。
[sudo] tanxzh 的密码： 
正在读取软件包列表... 完成
正在分析软件包的依赖关系树       
正在读取状态信息... 完成       
lsb-release 已经是最新版 (11.1.0ubuntu2)。
lsb-release 已设置为手动安装。
software-properties-common 已经是最新版 (0.99.9.12)。
下列软件包是自动安装的并且现在不需要了：
  cmake-data librhash0 libssl1.0.0
使用'sudo apt autoremove'来卸载它 (它们)。
升级了 0 个软件包，新安装了 0 个软件包，要卸载 0 个软件包，有 143 个软件包未被升级。
(myenv) tanxzh@tanxzh-Lenovo [13 时 56 分 39 秒] [~/tanxzh/code/python/bee_ware/helloworld] 
-> % wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | sudo apt-key add -
--2025-03-10 13:56:42--  https://apt.kitware.com/keys/kitware-archive-latest.asc
正在连接 127.0.0.1:7890... 已连接。
已发出 Proxy 请求，正在等待回应... 200 OK
长度： 3963 (3.9K) [application/pgp-keys]
正在保存至: “STDOUT”

-                                                             100%[================================================================================================================================================>]   3.87K  --.-KB/s    用时 0s    

2025-03-10 13:56:43 (389 MB/s) - 已写入至标准输出 [3963/3963]

OK
(myenv) tanxzh@tanxzh-Lenovo [13 时 56 分 44 秒] [~/tanxzh/code/python/bee_ware/helloworld] 
-> % sudo add-apt-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
命中: 1 http://mirrors.ustc.edu.cn/ubuntu focal InRelease                                                                                                                                                                                              
获取: 2 http://mirrors.ustc.edu.cn/ubuntu focal-updates InRelease [128 kB]                                                                                                                                                                             
获取: 3 http://mirrors.ustc.edu.cn/ubuntu focal-backports InRelease [128 kB]                                                                                                                                                                           
命中: 4 http://ppa.launchpad.net/bookworm-team/bookworm/ubuntu focal InRelease                                                                                                                                                                         
获取: 5 http://mirrors.ustc.edu.cn/ubuntu focal-security InRelease [128 kB]                                                                                                                                                                            
命中: 6 http://ppa.launchpad.net/jonathonf/vim/ubuntu focal InRelease                                                                                                                                                                                  
获取: 7 http://mirrors.ustc.edu.cn/ubuntu focal-updates/main amd64 DEP-11 Metadata [276 kB]                                                                                                                                                            
获取: 8 http://mirrors.ustc.edu.cn/ubuntu focal-updates/restricted amd64 DEP-11 Metadata [212 B]                                                                                                                                                       
获取: 9 http://mirrors.ustc.edu.cn/ubuntu focal-updates/universe amd64 DEP-11 Metadata [446 kB]                                                                                                                                                        
命中: 10 http://ppa.launchpad.net/linuxuprising/libpng12/ubuntu focal InRelease                                                                                                                                                                        
命中: 11 https://repo.whale.naver.com/stable/deb stable InRelease                                                                                                                                                                                      
获取: 12 http://mirrors.ustc.edu.cn/ubuntu focal-updates/multiverse amd64 DEP-11 Metadata [940 B]                                                                                                                                                      
获取: 13 http://mirrors.ustc.edu.cn/ubuntu focal-backports/main amd64 DEP-11 Metadata [7,996 B]                                                                                                                                                        
获取: 14 http://mirrors.ustc.edu.cn/ubuntu focal-backports/restricted amd64 DEP-11 Metadata [216 B]                                                                                                                                                    
获取: 15 http://mirrors.ustc.edu.cn/ubuntu focal-backports/universe amd64 DEP-11 Metadata [30.5 kB]                                                                                                                                                    
获取: 16 http://mirrors.ustc.edu.cn/ubuntu focal-backports/multiverse amd64 DEP-11 Metadata [212 B]                                                                                                                                                    
获取: 17 http://mirrors.ustc.edu.cn/ubuntu focal-security/main amd64 DEP-11 Metadata [65.4 kB]                                                                                                                                                         
获取: 18 http://mirrors.ustc.edu.cn/ubuntu focal-security/restricted amd64 DEP-11 Metadata [212 B]                                                                                                                                                     
命中: 19 https://dl.google.com/linux/chrome/deb stable InRelease                                                                                                                                                                                       
获取: 20 http://mirrors.ustc.edu.cn/ubuntu focal-security/universe amd64 DEP-11 Metadata [160 kB]                                                                                                                                                      
获取: 21 http://mirrors.ustc.edu.cn/ubuntu focal-security/multiverse amd64 DEP-11 Metadata [940 B]                                                                                                                                                     
忽略: 22 https://deepin-wine.i-m.dev  InRelease                                                                                                                                                                                                        
获取: 23 https://deepin-wine.i-m.dev  Release [1,611 B]                                                                                                                                                                                                
命中: 24 http://ppa.launchpad.net/neovim-ppa/unstable/ubuntu focal InRelease                                                                                                                                                                           
忽略: 25 https://deepin-wine.i-m.dev  Release. gpg                                                                                                                                                                                                      
获取: 26 https://apt.kitware.com/ubuntu focal InRelease [15.5 kB]                                                                                                                                                      
获取: 27 https://deepin-wine.i-m.dev  Packages [156 kB]                                                                                                                                                                           
获取: 28 https://download.sublimetext.com apt/stable/ InRelease [2,536 B]                                                                                                                                                                    
忽略: 29 https://www.scootersoftware.com bcompare4 InRelease                                                                                                                                                                                           
获取: 30 https://www.scootersoftware.com bcompare4 Release [1,717 B]                                                                                                                                                    
命中: 31 https://packages.microsoft.com/repos/edge stable InRelease                                                                                       
命中: 32 https://windsurf-stable.codeiumdata.com/wVxQEIWkwPUEAGf3/apt stable InRelease                                  
命中: 33 https://packages.microsoft.com/repos/code stable InRelease                                                     
获取: 34 https://apt.kitware.com/ubuntu focal/main i386 Packages [51.7 kB]                                              
获取: 35 https://www.scootersoftware.com bcompare4 Release. gpg [836 B]                                          
命中: 36 https://download.docker.com/linux/ubuntu focal InRelease                                                            
错误: 35 https://www.scootersoftware.com bcompare4 Release. gpg                             
  由于没有公钥，无法验证下列签名： NO_PUBKEY EC34ED227AFAE3F2
获取: 37 https://apt.kitware.com/ubuntu focal/main amd64 Packages [94.3 kB]
获取: 38 https://apt.kitware.com/ubuntu focal/main amd64 Contents (deb) [338 kB]                                                                                                                                                                       
获取: 39 https://apt.kitware.com/ubuntu focal/main i386 Contents (deb) [333 kB]                                                                                                                                                                        
已下载 2,367 kB，耗时 33 秒 (71.4 kB/s)                                                                                                                                                                                                                
正在读取软件包列表... 完成
W: 校验数字签名时出错。此仓库未被更新，所以仍然使用此前的索引文件。GPG 错误：https://www.scootersoftware.com bcompare4 Release: 由于没有公钥，无法验证下列签名： NO_PUBKEY EC34ED227AFAE3F2
W: 无法下载 https://www.scootersoftware.com/dists/bcompare4/Release.gpg  由于没有公钥，无法验证下列签名： NO_PUBKEY EC34ED227AFAE3F2
W: 部分索引文件下载失败。如果忽略它们，那将转而使用旧的索引文件。
(myenv) tanxzh@tanxzh-Lenovo [13 时 57 分 34 秒] [~/tanxzh/code/python/bee_ware/helloworld] 
-> % sudo apt update
命中: 1 http://mirrors.ustc.edu.cn/ubuntu focal InRelease
命中: 2 http://mirrors.ustc.edu.cn/ubuntu focal-updates InRelease                                                                                                                                                                                      
命中: 3 http://mirrors.ustc.edu.cn/ubuntu focal-backports InRelease                                                                                                                                                                                    
命中: 4 http://mirrors.ustc.edu.cn/ubuntu focal-security InRelease                                                                                                                                                                                     
命中: 5 https://dl.google.com/linux/chrome/deb stable InRelease                                                                                                                                                                                        
命中: 6 https://packages.microsoft.com/repos/edge stable InRelease                                                                                                                                                                                     
命中: 7 https://packages.microsoft.com/repos/code stable InRelease                                                                                                                                                                                     
忽略: 8 https://deepin-wine.i-m.dev  InRelease                                                                                                                                                                                                         
命中: 9 https://deepin-wine.i-m.dev  Release                                                                                                                                                                                                           
命中: 10 http://ppa.launchpad.net/bookworm-team/bookworm/ubuntu focal InRelease                                                                                                                                                                        
忽略: 11 https://deepin-wine.i-m.dev  Release. gpg                                                                                                                                                                                                      
命中: 12 http://ppa.launchpad.net/jonathonf/vim/ubuntu focal InRelease                                                                                                                                                                                 
命中: 13 https://download.sublimetext.com apt/stable/ InRelease                                                                                                                                                         
命中: 14 https://apt.kitware.com/ubuntu focal InRelease                                                                                                                                                                 
命中: 15 http://ppa.launchpad.net/linuxuprising/libpng12/ubuntu focal InRelease                                                                                                                    
命中: 16 https://windsurf-stable.codeiumdata.com/wVxQEIWkwPUEAGf3/apt stable InRelease                                                                                        
命中: 17 http://ppa.launchpad.net/neovim-ppa/unstable/ubuntu focal InRelease                                                                             
命中: 18 https://repo.whale.naver.com/stable/deb stable InRelease                                                       
命中: 19 https://download.docker.com/linux/ubuntu focal InRelease 
忽略: 20 https://www.scootersoftware.com bcompare4 InRelease
获取: 21 https://www.scootersoftware.com bcompare4 Release [1,717 B]
获取: 22 https://www.scootersoftware.com bcompare4 Release. gpg [836 B]
错误: 22 https://www.scootersoftware.com bcompare4 Release. gpg
  由于没有公钥，无法验证下列签名： NO_PUBKEY EC34ED227AFAE3F2
已下载 2,553 B，耗时 2 秒 (1,050 B/s)
正在读取软件包列表... 完成
正在分析软件包的依赖关系树       
正在读取状态信息... 完成       
有 144 个软件包可以升级。请执行‘apt list --upgradable’来查看它们。
W: 校验数字签名时出错。此仓库未被更新，所以仍然使用此前的索引文件。GPG 错误：https://www.scootersoftware.com bcompare4 Release: 由于没有公钥，无法验证下列签名： NO_PUBKEY EC34ED227AFAE3F2
W: 无法下载 https://www.scootersoftware.com/dists/bcompare4/Release.gpg  由于没有公钥，无法验证下列签名： NO_PUBKEY EC34ED227AFAE3F2
W: 部分索引文件下载失败。如果忽略它们，那将转而使用旧的索引文件。
(myenv) tanxzh@tanxzh-Lenovo [13 时 57 分 41 秒] [~/tanxzh/code/python/bee_ware/helloworld] 
-> % sudo apt install cmake
正在读取软件包列表... 完成
正在分析软件包的依赖关系树       
正在读取状态信息... 完成       
下列软件包是自动安装的并且现在不需要了：
  librhash0 libssl1.0.0
使用'sudo apt autoremove'来卸载它 (它们)。
将会同时安装下列软件：
  cmake-data
建议安装：
  ninja-build
下列【新】软件包将被安装：
  cmake
下列软件包将被升级：
  cmake-data
升级了 1 个软件包，新安装了 1 个软件包，要卸载 0 个软件包，有 143 个软件包未被升级。
需要下载 15.9 MB 的归档。
解压缩后会消耗 50.5 MB 的额外空间。
您希望继续执行吗？ [Y/n] Y
获取: 1 https://apt.kitware.com/ubuntu focal/main amd64 cmake-data all 3.31.6-0kitware1ubuntu20.04.1 [1,503 kB]
获取: 2 https://apt.kitware.com/ubuntu focal/main amd64 cmake amd64 3.31.6-0kitware1ubuntu20.04.1 [14.4 MB]                                                                                                                                            
获取: 2 https://apt.kitware.com/ubuntu focal/main amd64 cmake amd64 3.31.6-0kitware1ubuntu20.04.1 [14.4 MB]                                                                                                                                            
已下载 10.7 MB，耗时 13 分 14 秒 (13.4 kB/s)                                                                                                                                                                                                            
(正在读取数据库 ... 系统当前共安装有 390917 个文件和目录。)
准备解压 .../cmake-data_3.31.6-0kitware1ubuntu20.04.1_all. deb  ...
正在解压 cmake-data (3.31.6-0kitware1ubuntu20.04.1) 并覆盖 (3.16.3-1ubuntu1.20.04.1) ...
正在选中未选择的软件包 cmake。
准备解压 .../cmake_3.31.6-0kitware1ubuntu20.04.1_amd64. deb  ...
正在解压 cmake (3.31.6-0kitware1ubuntu20.04.1) ...
正在设置 cmake-data (3.31.6-0kitware1ubuntu20.04.1) ...
正在设置 cmake (3.31.6-0kitware1ubuntu20.04.1) ...
正在处理用于 man-db (2.9.1-1) 的触发器 ...


## 缺少 libgirepository1.0-dev
```
-> % sudo apt-get install libgirepository1.0-dev
[sudo] tanxzh 的密码： 
正在读取软件包列表... 完成
正在分析软件包的依赖关系树       
正在读取状态信息... 完成       
下列软件包是自动安装的并且现在不需要了：
  librhash0 libssl1.0.0
使用'sudo apt autoremove'来卸载它(它们)。
将会同时安装下列软件：
  gobject-introspection
建议安装：
  libgirepository1.0-doc
下列【新】软件包将被安装：
  gobject-introspection libgirepository1.0-dev
升级了 0 个软件包，新安装了 2 个软件包，要卸载 0 个软件包，有 143 个软件包未被升级。
需要下载 1,045 kB 的归档。
解压缩后会消耗 12.0 MB 的额外空间。
您希望继续执行吗？ [Y/n] Y
获取:1 http://mirrors.ustc.edu.cn/ubuntu focal-updates/main amd64 gobject-introspection amd64 1.64.1-1~ubuntu20.04.1 [282 kB]
获取:2 http://mirrors.ustc.edu.cn/ubuntu focal-updates/main amd64 libgirepository1.0-dev amd64 1.64.1-1~ubuntu20.04.1 [762 kB]
已下载 1,045 kB，耗时 2秒 (695 kB/s)            
正在选中未选择的软件包 gobject-introspection。
(正在读取数据库 ... 系统当前共安装有 391999 个文件和目录。)
准备解压 .../gobject-introspection_1.64.1-1~ubuntu20.04.1_amd64.deb  ...
正在解压 gobject-introspection (1.64.1-1~ubuntu20.04.1) ...
正在选中未选择的软件包 libgirepository1.0-dev:amd64。
准备解压 .../libgirepository1.0-dev_1.64.1-1~ubuntu20.04.1_amd64.deb  ...
正在解压 libgirepository1.0-dev:amd64 (1.64.1-1~ubuntu20.04.1) ...
正在设置 gobject-introspection (1.64.1-1~ubuntu20.04.1) ...
正在设置 libgirepository1.0-dev:amd64 (1.64.1-1~ubuntu20.04.1) ...
正在处理用于 man-db (2.9.1-1) 的触发器 ...
正在处理用于 libc-bin (2.31-0ubuntu9.17) 的触发器 ...

```

## briefcase create android 后需要手动下载文件
- 手动下载：`gradle-8.2-bin.zip`
	- `https\://services.gradle.org/distributions/gradle-8.2-bin.zip`
- 修改 `~/tanxzh/code/python/bee_ware/helloworld/build/helloworld/android/gradle/gradle/wrapper/gradle-wrapper.properties`，将其中的 file 链接修改如下：
```
distributionUrl=file:///home/tanxzh/tanxzh/code/python/thirtyone_days/thirtyone_days/build/thirtyone_days/android/gradle/gradle/wrapper/gradle-8.2-bin.zip
```

## build 之后运行的不是新的代码
```shell
briefcase update android
briefcase build android
briefcase run android
```

- build 的时候需要看下代码有没有更新