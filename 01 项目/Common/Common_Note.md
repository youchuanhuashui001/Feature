
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




## 对某个函数使用 O0 编译 
> 背景：`Virgo SPINand` 开启中断后，如果用 `O0` 编译 `SPI` 传输接口就不会卡死；如果用 `Os` 编译会卡死

- 使用 `__attribute__(( optimize( "-O0" )))` 修饰特定函数以 `O0` 编译 





## 反汇编 .elf
`arm-none-eabi-objdump -S loader.elf > dump.file`





## GDB dump 代码段 
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



## 芯片拖线

### 访问内存、或者往内存写数据，流程都是先发出请求，总线会处理，处理完成后会返回响应，如果没响应就会把总线卡死 
