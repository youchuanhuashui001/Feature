# GDB调试GRUS—8002环境搭建

1. 使用默认配置来配置.config文件`make grus_gx8002t_dev_1v_defconfig`

2. 编译工程时使用`make menuconfig`选中配置jtag debug

3. `make clean;make`：若不使用make clean，由于.config改变了但.c文件没有变，.o文件也被默认为未更新，输出的.elf文件就是使用未更新的.o文件来输出的，故要加上make clean

4. 编译出了gxscup.elf之后，要把开发板上flash的程序(gxscpu.bin)全部清空掉`sudo bootx -m grus -t s -c flash eraseall`，然后重启一下(因为清空flash实质上使用串口下载程序，所以要重新启动开发板)

5. 在gxscpu.elf目录下创建一个.gdbinit文件，内容为：
   
   ```
   target jtag jtag://127.0.1.1:1025
   load
   ```

6. 打开三个窗口：
   
   1. `DebugServerConsole -setclk 2`：GDB的后台
   
   2. `sudo minicom`：方便观察串口打印
   
   3. `csky-abiv2-elf-gdb gxscpu.elf`：在gxscup.elf目录下使用gdb
   
   4. 打开gdb后就可以b、c、n等操作

7. 注意：
   
   - 要在make menuconfig中打开jtag debug
   
   - 打开jtag debug生成.config后，要重新编译程序make clean;make
   
   - 要使用bootx工具把flash中的程序清空，保证开发板内flash为空，然后重启开发板
   
   - 在烧写bin文件到开发板后，直接load gxscup.elf文件，其实bin文件中除了.elf文件的部分程序已经开始运行，已经一直卡在读取串口处，所以此时load gxscup.elf并不能将开发板置于最初的位置，也就无法正常使用gdb
   
   - 使用bootx把flash清空之后要把开发板重启，因为bootx工具擦除flash实质是使用串口下载了一遍程序