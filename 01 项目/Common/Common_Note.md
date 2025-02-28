
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



# Cache

## 刷 Cache 地址应该传虚拟地址，DMA 操作应该用物理地址

^83f3c5

- 对于刷 Cache 操作，应该使用虚拟地址
- 对于 DMA 操作，应该使用物理地址
- CPU 访问的都是虚拟地址，DMA 访问的都是物理地址






# 芯片拖线


## 访问内存、或者往内存写数据，流程都是先发出请求，总线会处理，处理完成后会返回响应，如果没响应就会把总线卡死 
- 由于总线设计时会出现异步读写总线的情况，而 `cpu` 需要直到总线何时操作完，总线会给一个 `ready` 信号返回给 `cpu` 




# 调制解调

## 调制

**1. 什么是调制 (Modulation)？**

你可以把调制想象成**翻译**或者**编码**的过程。

- **信息的本质：** 我们想要传输的原始信息，例如你想发送的文字、声音、图像等等，通常是**“低频”或者“基带”信号**。 你可以简单理解为，这些信号的频率范围比较低，不适合直接远距离传输。 例如，声音的频率范围是20Hz-20kHz，人的说话声音频率更低。
    
- **传输的媒介：** 为了将信息有效地传输到远方，我们需要借助一个**载体**。这个载体通常是**高频信号**，也叫做**载波 (Carrier Wave)**。载波就像一辆货车，可以装载我们的信息，并将其运送到目的地。例如，无线电广播使用的电磁波就是一种高频载波。
    
- **调制的过程：** 调制就是**用原始信息去改变载波的某些特性**的过程。就像把你要发送的货物（原始信息）装载到货车（载波）上一样。我们通过改变载波的**幅度、频率或相位**等特性，来“编码”我们的原始信息。
    

**举个生活中的例子：**

假设你想给远方的朋友发送一条消息：“我今天很开心！”

- **原始信息：** 文字信息 “我今天很开心！” (低频信号)
- **载波：** 手电筒发出的光 (可以理解为一种高频载波)
- **调制过程：** 你用手电筒**有节奏地闪烁**来传递信息。例如，可以用 “长闪” 代表 “开心”，用 “短闪” 代表 “今天”，用 “闪一下” 代表 “我”。这样，你通过改变手电筒光线的**节奏 (类似于改变载波的特性)**，把文字信息 “编码” 成了一系列闪烁的光信号。

[![Person using Morse code with a flashlight to send a message的图片](https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcRY4UANx5tzx1g65o1PeTL_JS7Iqjdl-R55o_pmYizs4FED7FclxroKb7uILdSw)在新窗口中打开](https://www.wikihow.com/I-Love-You-in-Morse-Code)[![](https://encrypted-tbn3.gstatic.com/favicon-tbn?q=tbn:ANd9GcQm2PLs9PeO4CzLq55oOCJiDKZ92RwiimaOsMHwkCv_Xi6Ic3Lqr7mFu8Q2fmKIC0xPPnB5BNgP1h0WAmmVz6E5ikMFUtHo9yn_)www.wikihow.com](https://www.wikihow.com/I-Love-You-in-Morse-Code)

Person using Morse code with a flashlight to send a message

**更专业的解释：**

调制是指用基带信号（包含原始信息的信号）去控制载波信号的某个或某些参数，使载波信号的参数按照基带信号的规律变化。 常用的载波是高频的正弦波，可以表示为：

`c(t) = A * cos(2πf_c*t + φ)`

其中：

- `A` 是载波的幅度
- `f_c` 是载波的频率
- `φ` 是载波的相位

调制就是通过改变 `A`、`f_c` 或 `φ` 中的一个或多个参数来加载信息。 常见的调制方式有：

- **幅度调制 (AM - Amplitude Modulation):** 用基带信号控制载波的幅度。 幅度随信息信号变化。
    
    [![Amplitude Modulation (AM) waveform的图片](https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSBjXbUDITafpkAp_HuBrvDKiEh3ItzGE4FLkrUDFELD3BEk9UvcCL6MRLWOmw4)在新窗口中打开](https://www.geeksforgeeks.org/amplitude-modulation-definition-types-expression/)[![](https://encrypted-tbn1.gstatic.com/favicon-tbn?q=tbn:ANd9GcRhozPFEmg8f-ipzeQviUztsisaCXzxmMiAMrRliVhPQtJyEMMq_0b1osZy_EdZwXvGx2s17j-bfdCt94tYx_ITDG-xKXoZ_BxsQE05uyyO)www.geeksforgeeks.org](https://www.geeksforgeeks.org/amplitude-modulation-definition-types-expression/)
    
    Amplitude Modulation (AM) waveform
    
- **频率调制 (FM - Frequency Modulation):** 用基带信号控制载波的频率。 频率随信息信号变化。
    
    [![Frequency Modulation (FM) waveform的图片](https://encrypted-tbn2.gstatic.com/images?q=tbn:ANd9GcTmao9roWF-y3q6nE9euAgZUekNgpyP1BIPP6215vebtPM47gRa-nMniXYyDC81)在新窗口中打开](https://byjus.com/jee/frequency-modulation/)[![](https://encrypted-tbn1.gstatic.com/favicon-tbn?q=tbn:ANd9GcQuXRl-9syImP0bZ4t-SYIjqIu2lZiuZGgNul0a2Iaf2Lsij5Bzz5k3na5lYBFGiPDDMIQLgGZboK-pzxErbIE0n2wm)byjus.com](https://byjus.com/jee/frequency-modulation/)
    
    Frequency Modulation (FM) waveform
    
- **相位调制 (PM - Phase Modulation):** 用基带信号控制载波的相位。 相位随信息信号变化。
    
    [![Phase Modulation (PM) waveform的图片](https://encrypted-tbn1.gstatic.com/images?q=tbn:ANd9GcShYlPwwQDqMWS09yup7i2P8ClO4oA9wRxi4JpuzkWXftEj2k0LA8xsPtUERQ3M)在新窗口中打开](https://electronicscoach.com/phase-modulation.html)[![](https://encrypted-tbn1.gstatic.com/favicon-tbn?q=tbn:ANd9GcQktfvBj4b0Xl690abKt_-WUIiL2S6j9_jz1CkdDdyS_iHHfnWzQ_GyamR-fcNgwKa6H9kQxM2MOFxOyXliEDMB7OQe767N0MDMFpukMjU)electronicscoach.com](https://electronicscoach.com/phase-modulation.html)
    
    Phase Modulation (PM) waveform
    

**为什么需要调制？**

- **提高传输效率，减小天线尺寸 (对于无线通信):** 低频信号波长长，如果直接用低频信号发射无线电波，需要非常巨大的天线才能有效辐射出去，这在实际应用中是不可行的。 将低频信号调制到高频载波上后，波长变短，可以使用尺寸较小的天线进行发射和接收。 (这个可能更多针对无线通信，但可以帮助理解)。
    
- **实现频率复用，提高信道利用率:** 通过调制，可以将多个不同的基带信号调制到不同的载波频率上，这样就可以在同一传输介质（例如空气、电缆）中同时传输多个信号，而不会互相干扰。 这就像不同的广播电台使用不同的频率进行广播一样。
    
- **改善信号传输特性，提高抗干扰能力:** 有些调制方式（例如FM、相位调制）比幅度调制具有更好的抗噪声和抗干扰能力。 选择合适的调制方式可以提高通信的可靠性。
    
- **匹配信道特性:** 不同的传输信道 (例如无线信道、有线电缆、光纤) 对不同频率的信号有不同的特性。 调制可以将信号频率调整到信道特性较好的频段，以获得更好的传输效果。

## 解调
**2. 什么是解调 (Demodulation)？**

解调是调制的**逆过程**，也就是**解码**或者**反翻译**的过程。

- **接收到的信号：** 接收端接收到的信号是经过调制后的**已调信号**，其中包含了原始信息，但信息是“隐藏”在载波的特性变化中的。
    
- **解调的过程：** 解调就是**从已调信号中提取出原始信息**的过程。就像从装载货物的货车上把货物卸下来一样。接收端需要根据发送端采用的调制方式，设计相应的解调器，将载波中携带的信息“解码”出来，还原成我们需要的原始基带信号。
    

**继续手电筒的例子：**

- **接收到的信号：** 你的朋友在远处看到了你用手电筒发出的**闪烁的光信号** (已调信号)。
- **解调过程：** 你的朋友需要**观察光信号的闪烁节奏**，然后根据你预先约定的规则 (例如，长闪=开心，短闪=今天，闪一下=我) ，将光信号**解码** 成文字信息 **“我今天很开心！”** 这样，你的朋友就完成了对手电筒信号的解调，获取了你想传递的原始信息。

**更专业的解释：**

解调是调制的反过程，也称为检波。其作用是将接收到的已调信号进行处理，去除载波，恢复出原始的基带信号。解调器的设计需要与调制器相对应，例如，对于幅度调制 (AM) 信号，可以使用包络检波器进行解调；对于频率调制 (FM) 信号，可以使用鉴频器进行解调。






# 流控

## 流控概念：
**1. 什么是“流控”？（Flow Control 的基本概念）**

首先，我们需要理解“流控”本身是什么意思。 想象一下水管里的水流，或者高速公路上的车流。 “流控” 的目的就是 **控制数据流动的速度和数量**，以避免出现问题。

在计算机和网络中，“数据流” 指的是信息在不同设备之间传输的过程。 如果没有流控，可能会出现以下问题：

- **发送方速度过快，接收方处理不过来：** 就好比你用消防水管往一个小杯子里倒水，水肯定会溢出来。 在数据传输中，如果发送数据的速度太快，接收方的计算机或设备可能来不及处理这些数据，导致数据丢失或者错误。
- **网络拥塞：** 如果很多设备同时快速发送数据，就像很多车辆同时涌上高速公路，就会造成交通堵塞。 在网络中，这会导致数据传输速度变慢，甚至数据包丢失。

**流控的目的就是为了解决这些问题，确保数据传输平稳可靠。**

**2. 什么是“自动”流控？ （Automatic Flow Control 的核心）**

“自动” 流控意味着 **流控的过程是自动进行的，不需要人工干预**。 系统或协议会自动监测数据传输的情况，并根据情况动态地调整数据发送的速度。

与 “自动” 相对的是 “手动” 流控。 手动流控需要人为地去设置和调整数据传输的参数，这不仅麻烦，而且效率低下，也很难适应复杂和变化的环境。

**自动流控的优点：**

- **高效性：** 自动流控能够实时监控数据传输情况，并快速做出反应，保证数据传输效率最大化。
- **可靠性：** 通过自动调节数据流速，可以有效避免数据丢失、错误和网络拥塞，提高数据传输的可靠性。
- **智能化：** 自动流控系统通常会采用一些智能算法，能够根据网络的实际情况，动态地优化流控策略，提高整体性能。
- **降低管理成本：** 由于是自动的，所以减少了人工干预的需求，降低了管理和维护成本。

**3. 自动流控是如何工作的？ （工作原理）**

不同的系统和协议采用的自动流控机制可能有所不同，但它们的基本原理是相似的：

- **监测：** 系统会不断监测数据传输过程中的各种参数，例如：
    - **接收方的处理能力：** 接收方是否能够及时处理收到的数据？
    - **网络拥塞程度：** 网络是否繁忙？数据传输是否延迟？
    - **数据包丢失率：** 是否有数据包在传输过程中丢失？
- **反馈：** 接收方或网络会将监测到的信息反馈给发送方。 例如，接收方可以告诉发送方：“我处理速度有点慢了，你发慢一点”。 或者网络可以发出拥塞信号。
- **调整：** 发送方根据接收到的反馈信息，自动调整数据发送的速度。 如果接收方处理不过来，或者网络拥塞，发送方就会减慢发送速度。 反之，如果一切正常，发送方可以适当提高发送速度。


## Designware Uart 流控：
**1. UART 中的流控 (Flow Control in UART)**

首先，让我们聚焦到 UART (Universal Asynchronous Receiver/Transmitter，通用异步收发传输器) 这种串行通信接口。 UART 常用于微控制器、计算机和其他设备之间进行短距离、低速的数据通信。

在 UART 通信中，同样会遇到数据流速不匹配的问题：

- **数据溢出 (Data Overflow):** 如果发送端 (例如，微控制器) 向 UART 发送数据的速度太快，而接收端 (例如，计算机) 的 UART 接收数据的速度跟不上，接收端的 UART 内部的接收缓冲区 (FIFO) 就可能被填满，导致后续接收到的数据 **丢失**。 这就是数据溢出。

为了避免数据溢出，UART 也需要流控机制。 UART 的流控主要分为两种：

- **软件流控 (Software Flow Control)：** 通过在数据流中插入特定的控制字符 (例如，XON/XOFF 字符) 来控制数据传输的暂停和恢复。 这种方式简单易实现，但效率较低，并且会占用数据传输带宽。
- **硬件流控 (Hardware Flow Control)：** 通过使用额外的硬件信号线 (通常是 RTS 和 CTS 信号线) 来进行流控。 这种方式效率更高，更可靠，是 UART 流控的主流方式。

**2. UART 中的硬件自动流控 (Hardware Automatic Flow Control in UART)**

你提到的“自动流控”在 UART 语境下，通常指的是 **硬件自动流控**。 它利用了 UART 接口上的两条额外的信号线：

- **RTS (Request To Send, 请求发送)：** 这是一个 **输出信号线**，由 **数据接收方** (例如，接收数据的 UART) 控制。 接收方通过 RTS 信号线 **告知发送方** 是否准备好接收数据。
    - **RTS 低电平 (Low)：** 表示接收方 **准备好** 接收数据，发送方可以发送数据。
    - **RTS 高电平 (High)：** 表示接收方 **繁忙，无法接收** 数据，发送方应该 **暂停** 发送数据。
- **CTS (Clear To Send, 允许发送)：** 这是一个 **输入信号线**，由 **数据发送方** (例如，发送数据的 UART) 监听。 发送方通过 CTS 信号线 **接收来自接收方的流控信息**。
    - **CTS 低电平 (Low)：** 表示接收方 **允许** 发送方发送数据。 (对应接收方的 RTS 为低电平)
    - **CTS 高电平 (High)：** 表示接收方 **不允许** 发送方发送数据。 (对应接收方的 RTS 为高电平)

**硬件自动流控的工作流程 (使用 RTS/CTS 信号线):**

1. **接收方 UART 监测自身接收缓冲区 (FIFO) 的状态。** 当接收缓冲区快要满时 (例如，达到预设的阈值)，接收方 UART 会 **自动将 RTS 信号线拉高 (变为高电平)**。
2. **接收方 UART 的 RTS 信号线连接到发送方 UART 的 CTS 信号线。** 因此，发送方 UART 会检测到 CTS 信号线变为高电平。
3. **发送方 UART 检测到 CTS 信号线为高电平后，会立即暂停数据发送**。
4. **当接收方 UART 的接收缓冲区中的数据被处理 (例如，被 CPU 读取走) 后，接收缓冲区不再拥挤。** 接收方 UART 会 **自动将 RTS 信号线拉低 (变为低电平)**。
5. **发送方 UART 检测到 CTS 信号线变为低电平后，会恢复数据发送**。

( _这里应该插入一张示意图，展示两个 UART 设备通过 RTS 和 CTS 信号线连接，以及数据流向和信号线的电平变化，帮助理解 RTS/CTS 的工作原理_)

[![UART Hardware Flow Control RTS CTS signals的图片](https://encrypted-tbn1.gstatic.com/images?q=tbn:ANd9GcRHkcmuU3ASZorimAObZrsKhZfICbItmPL_yHwme8XvXIkjGEWqkGhdu8J3RLYc)在新窗口中打开](https://community.silabs.com/s/article/x-reference-using-or-bypassing-flow-control-with-uart-communication)[![](https://encrypted-tbn3.gstatic.com/favicon-tbn?q=tbn:ANd9GcRc9x41EWMRaBlIWltn1XABCdWCOnSWVOlDGH6M7OWfTBgyLZfMZ9CPdVlJjmPhMXZxMRtEh-CppTmrD00gx7gBZVI0hWzB9waiSTsUrDw)community.silabs.com](https://community.silabs.com/s/article/x-reference-using-or-bypassing-flow-control-with-uart-communication)

UART Hardware Flow Control RTS CTS signals

**简单总结 RTS/CTS 硬件流控:**

- **接收方控制发送速度：** 接收方通过 RTS 信号线“告诉”发送方何时可以发送数据。
- **全自动过程：** 整个流控过程由 UART 硬件自动完成，无需软件干预。
- **双向通信：** RTS/CTS 流控既可以用于控制单向数据传输，也可以用于全双工双向数据传输 (两个方向都需要流控)。

**3. Designware UART 中的自动流控**

Designware UART 作为一个通用的 UART IP 核，通常都会 **支持硬件自动流控 (RTS/CTS)**。 当你查看 Designware UART 手册时，关于自动流控的章节应该会详细介绍：

- **如何使能和配置硬件流控：** 通常会涉及到一些寄存器的配置，例如使能流控功能、设置 RTS 信号的触发阈值 (接收缓冲区的空闲程度达到多少时拉高 RTS 信号) 等。
- **RTS/CTS 信号线的连接方式：** 手册会指导如何将 Designware UART 的 RTS 和 CTS 引脚连接到外部设备 (例如，连接到另一个 UART 设备的 CTS 和 RTS 引脚)。
- **流控相关的寄存器和状态信息：** 手册会描述哪些寄存器可以用来监控流控的状态，例如查看当前 RTS 信号的状态、CTS 信号的状态、流控是否正在激活等。

**4. 为什么需要 UART 自动流控？**

在 UART 通信中使用自动流控有很多好处，尤其是在以下场景中：

- **连接到速度较慢的设备：** 如果你的 Designware UART 要连接到一个处理速度较慢的外部设备，使用流控可以避免数据发送过快导致对方接收溢出。
- **数据突发 (Data Burst)：** 如果发送端会突然发送大量数据 (例如，DMA 传输)，使用流控可以保证接收端能够可靠地接收所有数据，而不会因为缓冲区瞬间被填满而丢失数据。
- **高波特率通信：** 在高波特率下，数据传输速度更快，更容易出现数据溢出的问题。 硬件流控可以更有效地保证数据传输的可靠性。
- **资源受限的接收端：** 如果接收端的处理能力有限，例如是一个低功耗的微控制器，使用流控可以减轻接收端的负担，避免因数据接收压力过大而导致系统崩溃。

## uart 指定了波特率为什么还需要流控？

UART 通信在配置时需要指定波特率，这决定了理论上的数据传输速率。 但仅仅指定波特率**并不能完全解决所有数据传输可靠性的问题**，这就是为什么流控仍然非常重要，甚至在某些情况下是必不可少的原因。

为了更好地理解这个问题，我们可以从以下几个方面来展开解释：

**1. 波特率定义的是 _传输能力_，而不是 _处理能力_**

- **波特率 (Baud Rate) 就像公路的速度限制：** 波特率规定了 UART 线路在单位时间内可以传输多少个bit。 比如，115200 bps 的波特率意味着每秒可以传输 115200 个 bit。 这就像高速公路上的限速 120 km/h，它定义了这条公路 _能够_ 达到的最高速度。
- **处理能力 (Processing Capacity) 就像车辆的行驶速度：** 实际数据传输还需要接收方来 _处理_ 这些数据。 接收方的处理能力是指它在单位时间内能够 _接收并妥善处理_ 多少数据。 这就像车辆在高速公路上实际的行驶速度，它可能受到车辆性能、路况、驾驶员技术等多种因素的影响，不一定总是能达到 120 km/h 的限速。

**关键点在于： 即使发送方和接收方 _都_ 按照指定的波特率进行数据收发，但如果接收方的 _处理能力_ 跟不上，仍然会出现问题。**

**2. 接收方可能“忙不过来”的情况**

即使波特率固定，接收方在某些情况下仍然可能无法及时处理接收到的数据，导致数据积压甚至丢失。 常见的情况包括：

- **接收方 CPU 负载过高：** 接收方设备 (例如，单片机、计算机) 的 CPU 除了处理 UART 数据，可能还需要处理其他任务，例如：
    - 执行复杂的计算
    - 控制其他外设
    - 响应其他中断
    - 运行操作系统 当 CPU 资源紧张时，分配给 UART 数据处理的时间就会减少，可能导致 UART 接收缓冲区的数据来不及被及时读取和处理。
- **中断延迟 (Interrupt Latency)：** UART 数据接收通常是通过中断方式进行的。 当 UART 接收到数据时，会触发中断，CPU 响应中断并处理数据。 但是，中断响应是需要时间的，并且可能会被更高优先级的中断或任务延迟。 如果数据接收速率很快，而中断响应延迟较长，就可能导致在数据被处理之前，新的数据已经到达，最终导致接收缓冲区溢出。
- **接收缓冲区 (FIFO) 容量有限：** UART 硬件通常会有一个接收缓冲区 (FIFO，First-In First-Out 队列) 来临时存储接收到的数据。 但 FIFO 的容量是有限的 (例如，几十个字节到几百个字节)。 如果数据接收速度持续高于数据处理速度，FIFO 最终会被填满，后续到达的数据将被 **丢弃**，造成数据丢失 (Overrun Error)。
- **突发数据 (Data Burst)：** 即使平均数据速率不高，但如果发送方突然发送大量数据 (例如，DMA 突发传输)，接收方可能在短时间内无法及时处理如此多的数据，导致缓冲区溢出。
- **软件处理速度瓶颈：** 即使硬件接收没问题，接收到的数据最终还需要被软件 (驱动程序、应用程序) 处理。 软件处理速度可能受到算法效率、代码实现、系统调度等因素的限制，如果软件处理速度跟不上硬件接收速度，仍然会导致数据积压。

**3. 流控的作用： 动态调节发送速率，适应接收能力**

流控 (Flow Control) 的核心作用就是 **协调发送方和接收方的数据传输速率，使发送速率能够动态适应接收方的处理能力，防止数据过载和丢失。**

流控就像是高速公路上的 **交通管制系统** (例如，红绿灯、可变限速标志)。 即使高速公路的最高限速是 120 km/h，交通管制系统也会根据 _实际的路况_ (例如，车流量、拥堵程度) 动态地调整车辆的行驶速度，例如：

- **车流量大时：** 可能会降低限速，甚至实施分流，避免交通拥堵。 这就像流控在接收方繁忙时，让发送方 _减慢_ 发送速度。
- **路况良好时：** 车辆可以按照最高限速行驶，甚至可以稍微提高速度 (在安全范围内)。 这就像流控在接收方空闲时，允许发送方 _全速_ 甚至 _稍微提速_ 发送数据。

**通过流控，UART 通信可以更加可靠，尤其是在以下场景中：**

- **接收方处理能力不稳定或较低：** 例如，低功耗设备、资源受限的嵌入式系统。
- **数据传输对可靠性要求很高：** 例如，工业控制、医疗设备等，数据丢失可能导致严重后果。
- **复杂系统环境：** 在多任务、多中断的复杂系统中，流控可以提高 UART 通信的鲁棒性。

## 一般 uart 不会打开流控，不会出现丢数据吗？

**在很多常见的串口通信应用中，确实并不会启用流控**。 这是因为在很多情况下，即使没有流控，串口通信也能工作得相对可靠。 但是，**不启用流控确实会增加数据丢失的风险**，只是这个风险在不同的应用场景下大小不同。

为了更清楚地理解这个问题，我们可以从以下几个方面来分析：

**1. 为什么很多情况下不启用流控也能“正常”工作？**

- **低波特率和低数据量：** 如果串口通信的波特率不高 (例如，9600 bps, 115200 bps)，并且每次传输的数据量也不大，那么接收方通常有足够的时间来处理接收到的数据。 在这种情况下，即使没有流控，接收缓冲区也不太容易溢出。
- **接收方处理能力较强：** 如果接收方设备的处理能力很强 (例如，PC 机)，它可以很快地从 UART 接收缓冲区中读取数据，即使数据传输速率稍快一些，也不容易出现数据积压。
- **数据丢失可以容忍：** 在某些应用中，少量的数据丢失是可以接受的，或者可以通过上层协议的重传机制来弥补 (例如，某些基于串口的简单应用，即使偶尔丢几个字符，用户可能也感知不到)。
- **简单的点对点通信：** 如果只是两个设备简单的点对点通信，通信环境比较稳定，干扰较少，数据丢失的概率相对较低。

在这些情况下，为了简化硬件连接和软件配置，并且因为数据丢失的风险相对较低，开发者可能会选择不启用流控，让串口通信“裸奔”。

**2. 不启用流控时，数据丢失的风险仍然存在，尤其是在以下情况下：**

- **高波特率和大数据量：** 当波特率提高 (例如，几 Mbps 甚至更高)，并且需要传输大量数据时 (例如，传输文件、图像数据)，数据传输速率会大大增加。 如果接收方处理速度跟不上，接收缓冲区就很容易被填满，导致数据溢出和丢失。
- **接收方处理能力有限或不稳定：** 如果接收方是资源受限的嵌入式系统 (例如，低功耗单片机)，或者接收方 CPU 还需要处理其他高优先级的任务，那么分配给串口数据处理的时间就可能不足，更容易出现数据丢失。
- **数据突发性强：** 即使平均数据速率不高，但如果数据传输具有突发性 (例如，突然发送一大段数据)，接收方可能在短时间内无法及时处理，导致缓冲区溢出。
- **复杂的通信环境：** 在噪声干扰较强的环境中，或者在多设备竞争总线的场景中，数据传输的可靠性会下降，更容易出现错误和数据丢失。
- **对数据可靠性要求高的应用：** 在一些关键应用领域，例如工业控制、医疗设备、航空航天等，数据丢失是绝对不允许的，或者会造成严重后果。 在这种情况下，必须采取措施保证数据传输的可靠性，流控通常是必要的手段之一。

**3. 不启用流控时，可能出现的数据丢失现象：**

- **接收数据不完整或乱码：** 由于数据溢出导致部分数据字节被丢弃，接收方接收到的数据可能是不完整的，或者由于字节错位而出现乱码。
- **Overrun Error (溢出错误)：** 许多 UART 硬件会在发生接收缓冲区溢出时设置一个状态标志 (例如，Overrun Error Flag)。 如果软件没有及时检测和处理这个错误，后续的数据接收可能会持续出错。
- **程序运行异常：** 如果串口通信的数据是应用程序的关键输入，数据丢失可能会导致程序逻辑错误，甚至程序崩溃。


# 波特率

波特率，简单来说，就是**串行通信中信号每秒钟变化的次数**，也常被理解为**每秒钟传输的码元 (symbol) 的数量**。

- **码元 (Symbol):** 在数字通信中，码元是代表信息的信号基本单元。在最简单的二进制系统中，一个码元通常代表 1 位 (bit) 的信息，例如电压的高低变化可以表示 0 或 1。更复杂的调制技术中，一个码元可以代表多个比特。但在我们讨论的 UART 场景中，通常可以简化理解为一个码元就代表一个比特。
- **每秒钟变化次数:** 指的是信号在传输介质上，例如电线或光纤中，每秒钟电压、电流、频率或相位等参数变化的次数。每一次变化就代表传输了一个码元。

**波特率的单位：**

波特率的单位是 **波特 (baud)**，也常用 **bps (bits per second)** 来近似表示，尤其是在简单的二进制 UART 通信中。

- **波特 (baud):** 严格意义上来说，波特是指 **码元/秒 (symbols per second)**。
- **bps (bits per second):** 比特每秒，指 **比特/秒 (bits per second)**。
