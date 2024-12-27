
# C 语言 

## 使用变量和指针：
>  背景：`Virgo` 移植 `AXI-DMA` 驱动时，传递给某个函数的参数是结构体变量名，而不是结构体变量的地址 

- 使用变量的原因：
	- 存储简单的数值和状态
	- 数据大小固定且较小
	- 不需要共享给其他结构体使用
	- 生命周期和结构体本身绑定
- 使用指针的原因：
	- 需要指向其它完整的大型结构体
	- 指向动态内存的内存(指针都是需要动态分配出来的)
	- 指向内存映射的硬件寄存器
	- 表示一对多的关系(一个结构体可能被多个其它结构体引用)
	- 关注是动态分配还是静态分配

- 如果函数的参数以结构体变量的形式传递，在 b 到函数之前会先把<font color="#245bdb">所有参数都拷贝通用寄存器</font>，然后再调用函数，函数中会从通用寄存器中取出数据到内存再使用
- 如果函数的参数以结构体指针的形式传递，在 b 到函数之前<font color="#245bdb">只需要将地址拷贝到通用寄存器 R0</font> 即可，然后调用函数，函数中会直接从 R0 指向的内存地址直接取数据用

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



## memcpy 会存在非对齐访问的情况吗？
- 不存在，因为 `memcpy` 会在进行内存拷贝之前先判断 `src、dst` 是否是四字节对齐
	- 如果非四字节对齐的地址，会采用 `u8 *` 的方式拷贝 
	- 对于四字节对齐的地址，才会用 `long *` 的方式拷贝 
	```c
	#define UNALIGNED2(X, Y) \
 		(((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof 	(long) - 1)))

void *memcpy(void *dst0, const void *src0, size_t len0)
 {
 	char *dst = dst0;
 	const char *src = src0;
 	long *aligned_dst;
 	const long *aligned_src;
 	int   len =  len0;
 
 	/* If the size is small, or either SRC or DST is unaligned,
 	   then punt into the byte copy loop.  This should be rare.  */
 	if (!TOO_SMALL(len) && !UNALIGNED2 (src, dst))
 	{
 		aligned_dst = (long*)dst;
 		aligned_src = (long*)src;
 
 		/* Copy 4X long words at a time if possible.  */
 		while (len >= BIGBLOCKSIZE)
 		{
 			*aligned_dst++ = *aligned_src++;
 			*aligned_dst++ = *aligned_src++;
 			*aligned_dst++ = *aligned_src++;
 			*aligned_dst++ = *aligned_src++;
 			len -= BIGBLOCKSIZE;
 		}
 
 		/* Copy one long word at a time if possible.  */
 		while (len >= LITTLEBLOCKSIZE)
 		{
 			*aligned_dst++ = *aligned_src++;
 			len -= LITTLEBLOCKSIZE;
 		}
 
 		/* Pick up any residual with a byte copier.  */
 		dst = (char*)aligned_dst;
 		src = (char*)aligned_src;
 	}
 
 	while (len--)
 		*dst++ = *src++;
 
 	return dst0;
 }
	```







# 交叉编译工具链 

## GDB

### GDB dump 代码段 
> 背景：`Apus` 跑测试程序卡死，看 `SPI` 寄存器被修改了，怀疑 ` text ` 段在执行过程中被修改 

- `text` 段在程序执行时是不会被修改的，所以可以对比程序起来之后和出问题之后 `text` 段是否有差异 

```c
// 从 .elf 文件中获取 section 信息
riscv64-unknown-elf-objdump -h apus.elf 
riscv64-unknown-elf-objcopy -O binary apus.elf output_file --only-section .dump_section


// 从 .lds 中直接用 section 的标号来 dump：
// _stage2_text_start_、_stage2_text_end_

// 具体的命令
dump binary memory dump.file _sxip_text _exip_text
```


### GDB 访问 CP15 寄存器
- 具体命令格式来自 `jlink` 支持的拓展命令文档 [[#^22268b]]
![[Pasted image 20241216170244.png]]
```shell
# Reads or writes from/to cp15 register. If <data> is specified, this command writes the data to the cp15 register. Otherwise this command reads from the cp15 register. For further information please refer to the ARM reference manual.


cp15 <CRn>, <CRm>, <op1>, <op2> [= <data>]

# Read

#Reading CP15 register (1,2,6,7 = 0x0460B77D)
monitor cp15 1, 2, 6, 7


 #Write:
monitor cp15 1, 2, 6, 7 = 0xFFFFFFFF
```

- `Virgo` 测试读取 `GIC` 基地址：
```shell
(gdb) monitor cp15 15 0 4 0
Reading CP15 register (15,0,4,0 = 0xF0200000)
```


### GDB 命令文档

^22268b

- `GDB`  支持的命令：
```
https://sourceware.org/gdb/current/onlinedocs/gdb.html/Command-and-Variable-Index.html#Command-and-Variable-Index
```
- `JLink`  支持的拓展命令：
```
https://www.mouser.com/datasheet/2/737/UM08001_JLink_(1)-1228640. pdf? srsltid=AfmBOopq553q6isA0CIPqspTtsdHOTQWk28WI1TUJC-aE5Nm7F-TleAu
```




### FPGA 上没有编网络，没有 USB 如何 GDB 调试？
- 编译 `Linux` 时在 `def_config` 中打开对应的 `Debug` 选项
- 将整 `bin` 烧到 `flash`，等待 `Linux` 正常启动
- 启动完成后在 `Linux` 目录下，加载 `vmlinux`
	- **`.gdbinit` 中不要执行 `strore` 命令和 `set pc` 等命令，直接连上去就行了**

- 如果需要往文件系统放测试程序，可以在生成 `rootfs` 之前先丢到 `root` 文件夹中 



## objdump


### 反汇编 .elf
`arm-none-eabi-objdump -S loader.elf > dump.file`



## 编译优化


### 对某个函数使用 O0 编译 
> 背景：`Virgo SPINand` 开启中断后，如果用 `O0` 编译 `SPI` 传输接口就不会卡死；如果用 `Os` 编译会卡死

- 使用 `__attribute__(( optimize( "-O0" )))` 修饰特定函数以 `O0` 编译 
















# Linux

## Linux 如何以 O0 方式编译某个 .c 文件
- 移植高版本 `axi-dma` 驱动到 `Linux4.9` 时发现使用 `GDB` 调试某些变量看不到，Linux 默认是以比较高的优化等级编译的，可以单独以 `O0` 来编译某个 `.c` 文件来方便调试
- 通过修改 `Makefile` 实现：
	```Makefile
	CFLAGS_dw-axi-dmac-platform.o = -O0 -g
	obj-$(CONFIG_DW_AXI_DMAC) += dw-axi-dmac-platform.o
	```



## Linux 中的 pr_debug 如何打印？

> [!tips]
> - 如果不想那么麻烦，直接都改成 `pr_info`  好了 


- `前置条件1`：定义了 `CONFIG_DYNAMIC_DEBUG`
- `前置条件2`：文件的最开头定义 `-DDEBUG`
- 如此之后，则打印等级为 `KERN_DEBUG` 也就是比默认的 `控制台级别` 还是要低的，依然打不出来，需要从 `dmesg` 中查看 `pr_debug` 的输出
```
pr_debug()

    Some files call pr_debug(), which is ordinarily an empty macro that discards
    its arguments at compile time.  To enable debugging output, build the
    appropriate file with -DDEBUG by adding

      CFLAGS_[filename].o := -DDEBUG

    to the makefile.

    For example, to see all attempts to spawn a usermode helper (such as
    /sbin/hotplug), add to lib/Makefile the line:

        CFLAGS_kobject_uevent.o := -DDEBUG

    Then boot the new kernel, do something that spawns a usermode helper, and
    use the "dmesg" command to view the pr_debug() output.
```

- 如果消息的优先级比 `console_loglevel` 高，消息将被打印到控制台 
	- 数字越小，优先级越高 
```shell
case /proc/sys/kernel/printk
7 4 1 7
7:控制台打印等级，优先级高于该值的 printk 打印才会输出到控制台重定向的串口或屏幕上
4：默认的消息日志级别
1：最低的控制台日志级别
7：默认的控制台日志级别
```

![[Pasted image 20241218162323.png]]








# 芯片拖线


## 访问内存、或者往内存写数据，流程都是先发出请求，总线会处理，处理完成后会返回响应，如果没响应就会把总线卡死 
- 由于总线设计时会出现异步读写总线的情况，而 `cpu` 需要直到总线何时操作完，总线会给一个 `ready` 信号返回给 `cpu` 