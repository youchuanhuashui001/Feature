
# TODO
- [ ] 留下 bit 流和验证用的 bin
- [ ] gdb 调试内核时的相关内容都记录下来
- [ ] 验证 4 字节模式时的 dtr
- [ ] 目前 gxloader 有一个补丁，gxmisc 有两个补丁：一个用于支持 64 位编译，一个用于支持 dtr 模式，测试时需要都打上

# 环境搭建



# 新增  DDR Support
## 控制器支持
### DDR 模式支持

*   DDR 模式支持 SPI 协议中的 mode0 (SCPH=0 && SCPOL=0) 和 mode3 (SCPH=1 && SCPOL=1)。

### 支持两种方式

*   地址和数据以 DDR 格式传输 (或接收)，而指令以标准格式传输。
*   指令、地址和数据均以 DDR 格式发送和接收。

### 相关寄存器配置

*   `SPI_DDR_EN` (`SPI_CTRLR0[16]`) 位用于确定是否必须以 DDR 模式传输地址和数据。
*   `INST_DDR_EN` (`SPI_CTRLR0[17]`) 位用于确定是否必须以 DDR 模式传输指令。
*   这些位仅在 `CTRLR0.SPI_FRF` 位设置为 Dual、Quad 或 Octal 模式时有效。

### DDR 采样

*   DDR 模式下数据在两个边沿传输，因此很难正确采样数据。IP 使用内部寄存器来确定数据传输的边沿，这确保了接收器在采样时能够获得稳定的数据。
*   `DDR_DRIVER_EDGE` 决定了数据传输的边沿。最大值为 (BAUDR/2)-1。因此 DDR 模式最大分频值为 4。

### 相关寄存器

*   `CTRLR0`
*   `SPI_CTRLR0`
*   `DDR_DRIVER_EDGE`


## flash 支持

### flash ddr 模式
*   **单线：**
    *   0x0d (单边沿) + 3 字节地址 (双边沿) + 6 dummy cycle，读数据时双边沿读
    *   ![[Pasted image 20250416140455.png]]
*   **双线：**
    *   0xbd (单边沿) + 3 字节地址 (双边沿) + 8 bit 模式位 + 4 dummy cycles，读数据时双边沿读
    *   ![[Pasted image 20250416140440.png]]
*   **四线：**
    *   0xed (单边沿) + 3 字节地址 (双边沿) + 8 bit 模式位 + 7 dummy cycles，读数据时双边沿读
    *   ![[Pasted image 20250416140450.png]]

## 问题记录
### 【已解决】问题 1：看起来数据出来的早了，此时 DDR_DRIVER_EDGE 配的是 1，分频是 4
![[Pasted image 20250416151319.png]]
读到的数据如果按照波形来看是正确的，这里的波形是错误的波形，不是从这里读的，这里是 4个 dummy

考虑到可能是 DDR_DRIVER_EDGE 导致的问题，四分频下的范围是 (4/2)-1 = 1，八分频下的范围是 (8/2)-1 = 3，依次查看不同的 DDR_DRIVER_EDGE 配置下波形的情况：
- 配置为 2
![[Pasted image 20250416151854.png]]
- 配置为 3
![[Pasted image 20250416151952.png]]


- 把模式位都配置成 0 再发

- 看起来是波形来早了一个周期
	- 手册要求发 6 个字节
	- 发 6 个字节，数据来早了一个周期
	- 发 5 个字节，数据来早了两个周期
	- 发 7 个字节，看起来此时若是发 6 个字节，则采样的数据就是对的
		- ![[Pasted image 20250416155628.png]]
	- 数据来早了，采样采晚了
		- 双线  ![[Pasted image 20250416160021.png]]
		- 四线：![[Pasted image 20250416170845.png]]
- 将 sample_delay 修改为 0 后数据读取正确，但实际的波形依然和上面的波形一致。


### 【已解决】问题 2：只能发 0 地址，其它地址发出去在线上的显示不正确
gxloader 中的发送方式，分作三个 message：
- `message[0]` 单线发指令
- `message[1]` ddr、多线发地址和模式位 + dummy
- `message[2]` ddr、多线发数据或收数据
目前是 `message[1]` 发的地址位不正确。

scpu 中的发送方式，没有 message 的概念，但大致相同：
- prepare：8bit 指令、32bit 地址(真实的地址放到高位，mode 放在低位)、指令用单线发，地址用增强模式，使能 `SPI_CTRLR0 bit16`，往 txdr_be 写指令，往 txdr_be 写地址


开始测试：
- 将地址发成 0x384257，实际的波形也根本都挨不上边，试试看将换成往 txdr_be 寄存器写数据
	- 看起来写的数据有点相关了
	- 换成地址 0x14000 也还是一样的
- 指令不用 dtr 模式发，应该会有问题吧，这样地址波形就多了一倍

- 发地址：0x1 时实际读到了 0x11
```shell
boot> flash read 0x1 0x200000 0x10
a6 db 3c 87 0c 3e 99 24 5e 0d 1c 06 b7 47 de b3


----
# 实际读到的数据
boot> flash read 0x11 0x200000 0x10
a6 db 3c 87 0c 3e 99 24 5e 0d 1c 06 b7 47 de b3

# 发地址 0x100 实际读到了 0x1100，说明数据没有按照双边沿发送
boot> flash read 0x100 0x200000 0x10
a2 5c 28 0a 02 ed a6 41 8b 07 79 a6 05 e5 41 67


boot> flash read 0x1100 0x200000 0x10
a2 5c 28 0a 02 ed a6 41 8b 07 79 a6 05 e5 41 67

# 发地址 0x10000 实际读到了 0x110000

boot> flash read 0x10000 0x20000 0x10
5f cb 3a c5 cc 33 37 70 28 46 c9 9a 43 65 5b 5a

# 发地址 0x123456 实际读到了 0x224466 旁边的一个位置
boot> flash read 0x123456 0x200000 0x10
e9 0f 48 0e 12 5d 73 5e bc 9d f7 b4 19 35 db 1d


boot> flash read 0x224466 0x200000 0x10
e4 90 f4 80 e1 25 d7 35 eb c9 df 7b 41 93 5d b1


```


gxmisc：
0xed
0
0x12
0x34
0x56

new:
0xed
0x12
0x34
0x56
0x0

gxloader:
inst = 0xed
addr = 0x12 << 24 | 0x34 << 16 | 0x56 << 8 | 0x0


new:
inst = 0xed
addr = 0x12 << 24 | 0x34 << 16 | 0x56 << 8 | 0x0


解决方法：分成 2 个 message 发出去，第一个 message 包括地址+指令+模式位+dummy，第二个 message 专门读数据。
注：第一个 message 发的时候配置 `SPI_CTRLR0` 为 8bit 指令、32/40bit 地址、wait dummy，此时不能用 8bit 位宽的操作往寄存器写，发出去的波形不正常。需要一次把数据都写进寄存器。
[[flash_spi 寄存器级测试代码#^61a92a]]



### 【已解决】问题 3：SPINand 读不到 ID，但是线上有返回正确的 ID
- 使用 rc 的 bit 也不行
- 更换子板后可以了：
![[c88bce4eb52107b60582917bbf54328f.jpg]]



### 【已解决】问题 4：A55 平台 Flash 不通
- 依次验证读写擦操作：
	- 独立的擦除、读出、写入、读出都是正确的数据
	- 很奇怪，连起来之后，擦除完了再去读数据就是 0，并且逻辑分析仪抓到的也是 0
	- 驱动中的擦除操作：在擦除之前等 ready，擦除命令发完了就不管了；读之前做 ready
	- ![[Pasted image 20250428104312.png]]
	- `0x0 0x11 0x0 0x11 0x22 0x33 0x22 0x33 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff`
	- 双线模式读数据，实际出来的是单线的数据
		- ![[Pasted image 20250428105352.png]]

### 【已解决】问题 4-1：JLink 连不上
- JLink 连不上
	- 4 线 flash 的 bit 连不上
	- pcie 的 bit 也连不上
	- 用可以连上 Jlink 的板子放 x4 的 bit 跑不起来，sd 卡中的目录是 suit1，并且拨码也是拨到了 suit1
- JLink 连上了，但是跑的时候打印 345，就没有了
	- 此时应该跳去了 bl31，但是连上 JLink 看在 X32_READ 函数，si 走不下去，一直在 0xf0200e68 地址；并且无法访问任何内存
	- JLINK 无论是否执行了代码，都无法访问 DDR、SRAM
- 将 JLINKServer 的速度降低至 1000 后可以了，但是感觉还是不太正常
- 用妙兵的短 JLINK 线，可以正常调试，妙兵说是时序的问题


---
继续调试 flash 不通的问题：
打上 gxmisc 支持 64 位的补丁，flash 读写擦可以了
- gxmisc 有两个补丁：支持 dtr 模式和支持 64 位编译器，需要两个都打上


### 【已解决】问题 5：dtr 模式只有一个相位
dtr 模式下，若 flash 工作频率很低，则采样的右边在 0，左边到了负值，所以一直只有一个相位；若 flash 工作频率变高，则采样的右边有值，左边也有值，所以有多个相位。
- 实际测试：48MHz 模块频率时相位有：0、1


### 【已解决】问题 6：换成 spinand ，读不到 flash id
- gxmisc 没有打补丁：支持 dtr 模式
	- 打了之后，cs 依然不拉低
- 由于 `spi_nand_init` 中的宏应该是 `CONFIG_ARCH_ARM64_VIRGO2`，实际写成了 `CONFIG_ARCH_ARM_VIRGO2`，修改正确后可以读到 id。


### 【已解决】问题 7：spinand 开启 dma 时，用 boot 工具烧录不成功
- 由于开了 dma 之后，刷 cache 的时间很久，但 boot 工具有超时机制，因此烧录失败
- 用 gdb 加载是可以的



## 测试项
- [x] 三字节双倍速
- [x] 三字节四倍速
- [ ] dtr 模式的相位： 
- 4 分频只有相位 0，8 分频也只有相位 0，16 分频也只有相位 0
- 4 分频时寄存器范围为 1把 DDR_DRIVER_EDGE 配成 0，一个相位都没有
- 8 分频时寄存器范围为 8/2-1= 3，
	- 1 的时候只有 0
	- 2 的时候只有 0
	- 3 的时候只有 0
- 16 分频时寄存器范围为 16/2-1=7
	- 1 的时候只有 0
	- 2 的时候也只有 0
- [ ] 四字节双倍速，有一颗 w25q256 的 flash 看手册支持 dtr，但是 spinand 的 fmc 板子不通
- [ ] 四字节四倍速




| \4=. 测试项       | 位宽    | dma    | irq  | Nor Flash 测试结果 | NAND Flash 测试结果 |
| -------------- | ----- | ------ | ---- | -------------- | --------------- |
| 单倍速发送          | 8bit  | 不开 dma | 不开中断 |                |                 |
| 单倍速接收          | 8bit  | 不开 dma | 不开中断 |                |                 |
| 单倍速接收          | 32bit | 不开 dma | 不开中断 |                |                 |
| 双倍速接收          | 8bit  | 不开 dma | 不开中断 |                |                 |
| 双倍速接收          | 32bit | 不开 dma | 不开中断 |                |                 |
| 双倍速接收          | 8bit  | 开 dma  | 不开中断 |                |                 |
| 双倍速接收          | 32bit | 开 dma  | 不开中断 |                |                 |
| 双倍速接收          | 8bit  | 不开 dma | 开中断  |                |                 |
| 双倍速接收          | 32bit | 不开 dma | 开中断  |                |                 |
| 双倍速接收          | 8bit  | 开 dma  | 开中断  |                |                 |
| 双倍速接收          | 32bit | 开 dma  | 开中断  |                |                 |
| 四倍速接收          | 8bit  | 不开 dma | 不开中断 |                |                 |
| 四倍速接收          | 32bit | 不开 dma | 不开中断 |                |                 |
| 四倍速接收          | 8bit  | 开 dma  | 不开中断 |                |                 |
| 四倍速接收          | 32bit | 开 dma  | 不开中断 |                |                 |
| 四倍速接收          | 8bit  | 不开 dma | 开中断  |                |                 |
| 四倍速接收          | 32bit | 不开 dma | 开中断  |                |                 |
| 四倍速接收          | 8bit  | 开 dma  | 开中断  |                |                 |
| 四倍速接收          | 32bit | 开 dma  | 开中断  |                |                 |
| 四倍速发送          | 8bit  | 不开 dma | 不开中断 |                |                 |
| 四倍速发送          | 8bit  | 开 dma  | 不开中断 |                |                 |
| 四倍速发送          | 8bit  | 不开 dma | 开中断  |                |                 |
| 四倍速发送          | 8bit  | 开 dma  | 开中断  |                |                 |
| \4=. 分频配置      |       |        |      |                |                 |
| \4=. 输入时钟延时    |       |        |      |                |                 |
| \4=. 输入时钟上升沿采样 |       |        |      |                |                 |
| \4=. 输入时钟下降沿采样 |       |        |      |                |                 |
| \4=. 双 CS      | \2=.  |        |      |                |                 |
|                |       |        |      |                |                 |
| ddr 模式         |       |        |      |                |                 |



```
v2.0 更新好像支持 单线模式 增强寄存器啥的


功能测试：
* Flash 型号： <notextile>W25Q128、GD55LX02GEBIRY(NOR) GD5F1GM7UEYIG(NAND)</notextile>
* *rom 目前不支持 spi nand 启动，用 gdb 将 bin 加载进去验证的 spinand*
|模式| 位宽 | Nor Flash 测试结果| NAND Flash 测试结果|
| 发送单倍速，无内部DMA     | 8bit  | OK  |  |
| 接收单倍速，无内部DMA     | 8bit  | OK  |  |
| 接收单倍速，无内部DMA     | 32bit | OK  |  |
| 接收双倍速，开/关内部DMA  | 8bit  | OK  |  |
| 接收双倍速，开/关内部DMA  | 32bit | OK  |  |
| 发送四倍速，开/关内部DMA  | 8bit  | OK  |  |
| 接收四倍速，开/关内部DMA  | 8bit  | OK  |  |
| 接收四倍速，开/关内部DMA  | 32bit | OK  | OK |
| 发送八倍速，开/关内部DMA  | 8bit  | OK  | 无8线flash，不支持 |
| 接收八倍速，开/关内部DMA  | 8bit  | OK  | 无8线flash，不支持 |
| 接收八倍速，开/关内部DMA  | 32bit | OK  | 无8线flash，不支持 |
| 分频配置                | 8bit  | OK | OK |
| 输入时钟延时             | 8bit  | OK | OK |
| 输入时钟上升沿采样        | 8bit  | OK | OK |
| 输入时钟下降沿采样        | 8bit  | OK | OK |
| 中断                   | 8bit  | OK | OK |
| 双 CS                  | 8bit  |\2=. OK  |  

|ddr 模式|ok|


sdr 模式相位：
2分频 ：
sample_delay correct number is 0, 1, 2,
sample_delay the best number is 1

dtr 模式：
4分频、8分频、16分频都只有相位 0 可用
flash_spi 48MHz 之后，4分频有相位0、1可用

性能测试：双线、四线、八线、双线 dtr、四线 dtr

模块频率 48MHz，4分频(12MHz)

八线：开 dma
boot> flash speedtest
erase flash size 16MiB
read 16MiB elapse 2003ms, 8179 KB/S

四线 dtr：开 dma
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
read 16MiB elapse 2002ms, 8183 KB/S

四线：开 dma
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
read 16MiB elapse 3400ms, 4818 KB/S

双线 dtr：开 dma
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
read 16MiB elapse 3401ms, 4817 KB/S


双线：开 dma
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
read 16MiB elapse 6197ms, 2643 KB/S


模块频率 48MHz，2分频(24MHz)
八线：开 dma
boot> flash speedtest
erase flash size 16MiB
read 16MiB elapse 1304ms, 12564 KB/S


四线：开 dma
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
read 16MiB elapse 2003ms, 8179 KB/S


四线：不开 dma
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
read 16MiB elapse 18322ms, 894 KB/S


双线：开 dma
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
read 16MiB elapse 3401ms, 4817 KB/S

双线：不开 dma
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
read 16MiB elapse 18324ms, 894 KB/S


------
spinand

48MHz(4分频，12MHz)

双线：不开 dma


双线：开 dma
boot> flash speedtest
erase flash size 8MiB
read 8MiB elapse 23938ms, 342 KB/S

四线：不开 dma
boot> flash speedtest
erase flash size 8MiB
read 8MiB elapse 10884ms, 752 KB/S

四线：开 dma
boot> flash speedtest
erase flash size 8MiB
read 8MiB elapse 22545ms, 363 KB/S


|测试项 |测试结果|

|标准模式读写 | |

|Dual 模式读 | |

|Quad 模式读写 | |

|Octal 模式读写 | |

|DDR 模式读 | |

|片选 CS | |

|DMA | |

|延时采样 | |

|中断 | |

|分频 | |

|标准模式读写擦速度 | |

|Dual 模式读写擦速度 | |

|Quad 模式读写擦速度 | |

|Octal 模式读写擦速度 | |

|兼容性测试 | |


```


| 测试项           | 测试结果 |
| ------------- | ---- |
| 标准模式读写        |      |
| Dual 模式读      | ok   |
| Quad 模式读写     | ok   |
| Octal 模式读写    |      |
| DDR 模式读       | ok   |
| 片选 CS         |      |
| DMA           | ok   |
| 延时采样          |      |
| 中断            |      |
| 分频            |      |
| 标准模式读写擦速度     |      |
| Dual 模式读写擦速度  |      |
| Quad 模式读写擦速度  |      |
| Octal 模式读写擦速度 |      |
| 兼容性测试         |      |



# 修改
## gxmisc 编译选项. S、. C 分开
- .S 使用 mcpu 指定
- .c 使用 march 指定，搞清楚各种编译选项，以及是否对现有的编译选项有影响

### 参考链接：
https://gcc.gnu.org/onlinedocs/gcc/AArch64-Options.html

### 编译选项：
```makefile
-Wpointer-arith -Wundef -Wall -Wstrict-prototypes -pipe -fno-builtin -fstrict-volatile-bitfields -mlittle-endian
```
- -Wpointer-arith:
	- 警告任何依赖于函数类型或 `void` 的“大小”的操作。GNU C 将这些类型的大小指定为 1，以便于使用 `void *` 指针和指针进行计算。函数。在 C++ 中，当算术运算涉及 `NULL` 。此警告也由 -Wpedantic 启用。
	- 当对 `void *` 指针或函数指针进行算术运算时，如果 GCC 扩展被使用，此选项会发出警告。这些操作在 ISO C 标准中是不允许的。
- -Wundef
	- 如果在 `#if` 指令中计算未定义的标识符，则发出警告。此​​类标识符将被替换为零。
- -Wall
	- 启用所有关于某些用户认为有问题的构造的警告。
- -Wstrict-prototypes 
	- 如果声明或定义函数时未指定参数类型，则发出警告。（如果旧式函数定义前面有指定参数类型的声明，则允许使用旧式函数定义，而不会发出警告。）
- -pipe
	- 使用管道而不是临时文件在编译的各个阶段之间进行通信。在某些汇编器无法从管道读取数据的系统上，这种方法会失败；但 GNU 汇编器却没有遇到任何问题。
- -fno-builtin
	- 无法识别不以 built-in 开头的内置函数 __builtin__  作为前缀。
- -fstrict-volatile-bitfields
	- 如果访问易失性位字段（或其他结构体字段，尽管编译器通常也支持这些类型），需要使用与字段类型宽度相同的单次访问，​​并尽可能进行自然对齐，则应使用此选项。例如，具有内存映射外设寄存器的目标系统可能要求所有此类访问的宽度均为 16 位；使用此标志，您可以将所有外设位字段声明为 `unsigned short` （假设这些目标系统上的 short 为 16 位），以强制 GCC 使用 16 位访问， 而不是更高效的 32 位访问。
- -mlittle-endian
	- 为以小端模式运行的处理器生成代码。这是所有标准配置的默认设置。


---

```makefile
MY_CFLAGS += -Wno-format-security -ffreestanding -fshort-wchar -fno-strict-aliasing
MY_CFLAGS += -march=armv8-a
MY_CFLAGS += -std=gnu99
MY_CFLAGS += -mstrict-align
```
- -Wno-format-security
	- 如果指定了 -Wformat ，还会对可能存在安全问题的格式化函数的使用发出警告。目前，这会对 `printf` 和 `scanf` 函数的调用发出警告，其中格式字符串不是字符串文字，并且没有格式参数，例如 `printf (foo);` 。如果格式字符串来自不受信任的输入并且包含“ %n ”，则可能存在安全漏洞。
	- 此选项用于禁止上面的警告
- -ffreestanding
	- 断言编译目标为独立环境。这意味着 -fno-builtin 。独立环境是指标准库可能不存在，程序启动也不一定在 `main` 中。最明显的例子是操作系统内核。这相当于 -fno-hosted 。
- -fshort-wchar
	- 将 wchar_t 的底层类型覆盖为 short unsigned int，而不是目标的默认类型。此选项对于构建在 WINE 下运行的程序很有用。
- -fno-strict-aliasing
	- 允许编译器采用适用于所编译语言的最严格的别名规则。对于 C（和 C++），这将根据表达式的类型激活优化。具体来说，除非类型几乎相同，否则一种类型的对象被假定永远不会与另一种类型的对象位于同一地址。例如， `unsigned int` 可以为 `int` 指定别名，但不能为 `void*` 或 `double` 。字符类型可以是任何其他类型的别名。

---

- -std=gnu99
    - 对于带有 GNU 扩展的 C99
- -mstrict-align
	- 允许生成可能未按照架构规范中描述的自然对象边界对齐的内存访问

- -mcpu=name
	- 指定目标处理器的名称，可选地添加一个或多个功能修饰符作为后缀。
	- 目前使用 cpu 是 cotrex-a55
- -march=name
	- 指定目标 ARM 架构的名称




## gxmisc 只需要增加 DDR 特性，flash 支持 DDR 特性，能够和 DUAL、QUAD 交叉
- gxmisc 中对于 flash 的特性增加了一个 DTR，可以和 DUAL、QUAD 一起匹配


## gxloader 中对于 message 的处理需要支持单线写、多线写、ddr 多线写
- gxmisc 中处理 ddr 模式时分成 3 个 message，第一个 message 使用单线将指令发掉，第二个 message 使用多线 ddr 将地址、模式、dummy 发掉，第三个 message 使用多线 ddr 模式读数据

- 四线 dtr 模式，可以读到数据，但是读的数据看起来有点问题，不太正确
	- 因为逻辑分析仪的线导致的干扰

dual_dtr_addr 改成 7

## 测试
- [x] 双线 dtr
- [x] 四线 dtr
- [x] 双线 dma dtr
- [x] 四线 dma dtr
- [x] 双线
- [x] 四线
- [x] 双线 dma
- [x] 四线 dma