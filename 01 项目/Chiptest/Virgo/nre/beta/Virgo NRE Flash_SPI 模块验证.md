
# TODO
- [ ] 留下 bit 流和验证用的 bin
- [ ] gdb 加载时的命令都记录下来
- [ ] 验证 4 字节模式时的 dtr

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


功能测试：
* bit：#394932 问题号下的 peri_final_FPGA_2024-11-29
* Flash 型号： <notextile>BY25Q128AS(NOR) (NAND)</notextile>
* *rom 支持了 spi nand 启动*


| 模式             | 位宽    | Nor Flash 测试结果 | NAND Flash 测试结果 |
| -------------- | ----- | -------------- | --------------- |
| 发送单倍速，无内部DMA   | 8bit  |                |                 |
| 接收单倍速，无内部DMA   | 8bit  |                |                 |
| 接收单倍速，无内部DMA   | 32bit |                |                 |
| 接收双倍速，开/关内部DMA | 8bit  | ok             |                 |
| 接收双倍速，开/关内部DMA | 32bit | ok             |                 |
| 发送四倍速，开/关内部DMA | 8bit  |                |                 |
| 接收四倍速，开/关内部DMA | 8bit  |                |                 |
| 接收四倍速，开/关内部DMA | 32bit |                |                 |
| 分频配置           | 8bit  |                |                 |
| 输入时钟延时         | 8bit  |                |                 |
| 输入时钟上升沿采样      | 8bit  |                |                 |
| 输入时钟下降沿采样      | 8bit  |                |                 |
| 中断             | 8bit  |                |                 |
| 双 CS           | 8bit  | \2=.           |                 |

注：cs1 需要配置 GPIOB01 为 func3






