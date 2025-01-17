- 返回 -1


- gxapi --> mtd --> gxmisc


- SYS_write-->

![[Pasted image 20250107162509.png]]
![[Pasted image 20250107162603.png]] ![[Pasted image 20250107162630.png]]


![[Pasted image 20250107162730.png]]



# copy_from_user 返回了一个奇怪的地址
- 使用 asan 工具来帮助排查是否有内存错误
- uclibc 不支持 asan 工具，需要用 lib 工具链
	- 需要重新编译 linux 和 rootfs
- 报错：找不到 /usr/bin/sh 是因为找不到库
- 报错：permition denied 是因为找到库了，但是库没有可执行权限 


```
app/

net_upgrage/
app. c
app_config. c
app_file_list. c
app_full_screen. c
app_main_menu. c
app_sysinfo. c
app_system_setting_opt. c
app_upgrade. c
full_screen. c
main. c



make clean;
拷贝之前的 .config
make
make bin


需要换一个新的 loader 的 bin
拷贝之前的 loader
make
make bin
```

- 没有编译 thirdlib 中的 omx
- 没有编译 busybox 中的 wpa package

# solution 中添加一个 aaa. c 测试
- 添加上 `-fsanitize=address -g` 之后报错：
- 问 cursor 说需要指定动态库，感觉是找不到动态库，用的是静态编译 
	- asan 不能用静态编译，需要动态编译，然后将需要用到的动态库拷贝到根文件系统 
- 已经把 app 目录下的文件都加上了 asan，编译过了
	- 问题是：不是一个 .elf，而是一个 .bin，运行的时候可能缺少哪些动态库应该怎么找 
	- .bin 是通过 .elf 
- 因为需要把动态库拷贝过来，所以删了很多东西 


- 抓到现场了
	- 用 addr-to-len 看一下具体出错的几个位置
	- 在加 .c
	- 抓到的是一个 read 越界，需要添加选项不检测 read 越界 
		- `--param asan-instrument-reads=0` 选项没用
		- https://github.com/google/sanitizers/issues/752
- data 分区太小，换大 flash
	- 烧进去的 bin 和升级的 bin 大小要相同
	- 烧进去的 bin 是 24MB，升级的 bin 尾部加上 8M 的 0xff，合起来也是 24MB，可以从 U 盘读到 recrover_os. bin 和要升级的 bin，但是失败了
		- 不判断 flash_size 必须和要升级的 bin 大小一样，就升级 16M 的 bin 试试 
		- bin 就做成 17M 试试
			- 打印 flash size 是 4M，升级的数据 size 是 17M，注释掉判断
			- 这里是需要 flash size 是 4 的倍数，不然就是 4M
			- 可以正常读到两个 bin，但是 umount 失败，kill 掉占用的进程
				- 好像不能 kill 进程，umount 失败不管
				- 或者后面不判断 umount 的返回值
				- 可以跑下来了，到了写 flash head 的位置了，显示只有 400 多字节，但是要读 4k
					- 换成一个奇怪的值，看看是不是还是要读这么多，下一步顺便看看 partition 的值是多少，看了 buffer 不是错的地址
					- 是的还是错误的地址，看看 partition 的地址是多少
				- malloc 675 个字节，write 4k，读越界 