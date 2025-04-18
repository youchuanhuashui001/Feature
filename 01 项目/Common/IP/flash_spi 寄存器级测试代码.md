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




# dtr
```
set  *((volatile unsigned int *) 0xf8400008) = 0
set  *((volatile unsigned int *) 0xf840002c) = 0
set  *((volatile unsigned int *) 0xf84000f4) = 0
set  *((volatile unsigned int *) 0xf8400018) = 0
set  *((volatile unsigned int *) 0xf840001c) = 0
set  *((volatile unsigned int *) 0xfa490080) = 0x0f
set  *((volatile unsigned int *) 0xfa490080) = 0xf & ~(1 << 0)
set  *((volatile unsigned int *) 0xf8400008) = 0

set  *((volatile unsigned int *) 0xf84000f0) = 0x01

set  *((volatile unsigned int *) 0xf8400014) = 0x40
set  *((volatile unsigned int *) 0xf8400004) = 0x00
set  *((volatile unsigned int *) 0xf840004c) = 0x00
set  *((volatile unsigned int *) 0xf8400008) = 0x00

set  *((volatile unsigned int *) 0xf8400010) = 0x01
set  *((volatile unsigned int *) 0xf840004c) = 0x00

set  *((volatile unsigned int *) 0xf84000f4) = (0 << 30) | (0x2 << 8) | (0x6 << 2) | 0x2
set  *((volatile unsigned int *) 0xf84000f4) |= (1 << 16) | (1 << 17)
set  *((volatile unsigned int *) 0xf84000f8) = 1

set  *((volatile unsigned int *) 0xf8400000) = (0x1 << 31) | (0x2 << 22) | (0x1 << 10) | ((0x20 - 1) << 0)

set  *((volatile unsigned int *) 0xf8400018) = (0x3 << 16)
set  *((volatile unsigned int *) 0xf8400004) = 0x3

set  *((volatile unsigned int *) 0xf8400008) = 0x01


set  *((volatile unsigned int *) 0xf8400060) = 0x12
set  *((volatile unsigned int *) 0xf8400060) = 0x345678
set  *((volatile unsigned int *) 0xf8400060) = 0xa5b5a5b5
set  *((volatile unsigned int *) 0xf8400060) = 0x12345678

set  *((volatile unsigned int *) 0xfa490080) = 0x0f
```


```shell
set  *((volatile unsigned int *) 0xf8400008) = 0
set  *((volatile unsigned int *) 0xf840002c) = 0
set  *((volatile unsigned int *) 0xf84000f4) = 0
set  *((volatile unsigned int *) 0xf8400018) = 0
set  *((volatile unsigned int *) 0xf840001c) = 0
set  *((volatile unsigned int *) 0xfa490080) = 0x0f
set  *((volatile unsigned int *) 0xfa490080) = 0xf & ~(1 << 0)
set  *((volatile unsigned int *) 0xf8400008) = 0

set  *((volatile unsigned int *) 0xf84000f0) = 0x01

set  *((volatile unsigned int *) 0xf8400014) = 0x40
set  *((volatile unsigned int *) 0xf8400004) = 0x00
set  *((volatile unsigned int *) 0xf840004c) = 0x00
set  *((volatile unsigned int *) 0xf8400008) = 0x00

set  *((volatile unsigned int *) 0xf8400010) = 0x01
set  *((volatile unsigned int *) 0xf840004c) = 0x00

set  *((volatile unsigned int *) 0xf84000f4) = (0 << 30) | (0x2 << 8) | (0x8 << 2) | 0x1
set  *((volatile unsigned int *) 0xf84000f4) |= (1 << 16)

set  *((volatile unsigned int *) 0xf84000f8) = 1

set  *((volatile unsigned int *) 0xf8400000) = (0x1 << 31) | (0x2 << 22) | (0x1 << 10) | ((0x20 - 1) << 0)

set  *((volatile unsigned int *) 0xf8400018) = (0x2 << 16)
set  *((volatile unsigned int *) 0xf8400004) = 0x0

set  *((volatile unsigned int *) 0xf8400008) = 0x01


set  *((volatile unsigned int *) 0xf8400060) = 0xed
set  *((volatile unsigned int *) 0xf8400060) = 0x345678
set  *((volatile unsigned int *) 0xf8400060) = 0xaa55bb55

set  *((volatile unsigned int *) 0xfa490080) = 0x0f
```
![[Pasted image 20250417161309.png]]




```
set  *((volatile unsigned int *) 0xf8400008) = 0
set  *((volatile unsigned int *) 0xf840002c) = 0
set  *((volatile unsigned int *) 0xf84000f4) = 0
set  *((volatile unsigned int *) 0xf8400018) = 0
set  *((volatile unsigned int *) 0xf840001c) = 0
set  *((volatile unsigned int *) 0xfa490080) = 0x0f
set  *((volatile unsigned int *) 0xfa490080) = 0xf & ~(1 << 0)
set  *((volatile unsigned int *) 0xf8400008) = 0

set  *((volatile unsigned int *) 0xf84000f0) = 0x01

set  *((volatile unsigned int *) 0xf8400014) = 0x40
set  *((volatile unsigned int *) 0xf8400004) = 0x00
set  *((volatile unsigned int *) 0xf840004c) = 0x00
set  *((volatile unsigned int *) 0xf8400008) = 0x00

set  *((volatile unsigned int *) 0xf8400010) = 0x01
set  *((volatile unsigned int *) 0xf840004c) = 0x00

set  *((volatile unsigned int *) 0xf84000f4) = (0 << 30) | (0x2 << 8) | (0x8 << 2) | 0x1
set  *((volatile unsigned int *) 0xf84000f4) |= (1 << 16)

set  *((volatile unsigned int *) 0xf84000f8) = 1

set  *((volatile unsigned int *) 0xf8400000) = (0x1 << 31) | (0x2 << 22) | (0x1 << 10) | ((0x8 - 1) << 0)

set  *((volatile unsigned int *) 0xf8400018) = (0x2 << 16)
set  *((volatile unsigned int *) 0xf8400004) = 0x0

set  *((volatile unsigned int *) 0xf8400008) = 0x01


set  *((volatile unsigned int *) 0xf8400060) = 0xed
set  *((volatile unsigned int *) 0xf8400060) = 0x12345678



set  *((volatile unsigned int *) 0xfa490080) = 0x0f
```

![[Pasted image 20250417163631.png]]

```
(gdb) x/x 0xf8400028
0xf8400028:     0x00000006
(gdb) x/20x 0xf8400000
0xf8400000:     0x00800407      0x00000000      0x00000001      0x00000000
0xf8400010:     0x00000001      0x00000040      0x00020000      0x00000000
0xf8400020:     0x00000000      0x00000000      0x00000006      0x00000000
0xf8400030:     0x00000000      0x00000001      0x00000000      0x00000000
0xf8400040:     0x00000000      0x00000000      0x00000000      0x00000000
(gdb) x/x 0xf84000f8
0xf84000f8:     0x00000001
(gdb) x/x 0xf84000f4
0xf84000f4:     0x00010221



(gdb) x/x 0xf8400028
0xf8400028:     0x00000006
(gdb) x/x 0xf84000f0
0xf84000f0:     0x00000001
(gdb) x/x 0xf84000f4
0xf84000f4:     0x00013a21
(gdb) x/x 0xf84000f8
0xf84000f8:     0x00000001
(gdb) x/20x 0xf8400000
0xf8400000:     0x00800407      0x00000000      0x00000001      0x00000000
0xf8400010:     0x00000001      0x00000040      0x00020000      0x00000000
0xf8400020:     0x00000000      0x00000000      0x00000006      0x00000000
0xf8400030:     0x00000000      0x00000001      0x00000000      0x00000000
0xf8400040:     0x00000000      0x00000000      0x00000000      0x00000000




(gdb) x/20x 0xf8400000
0xf8400000:     0x00400407      0x00000000      0x00000001      0x00000000
0xf8400010:     0x00000001      0x00000040      0x00010000      0x00000000
0xf8400020:     0x00000001      0x00000000      0x00000002      0x00000000
0xf8400030:     0x00000000      0x00000000      0x00000000      0x00000000
0xf8400040:     0x00000000      0x00000000      0x00000000      0x00000000
(gdb) x/x 0xf84000f0
0xf84000f0:     0x00000001
(gdb) x/x 0xf84000f1
0xf84000f1:     0x21000000
(gdb) x/x 0xf84000f4
0xf84000f4:     0x00012221
(gdb) x/x 0xf84000f8
0xf84000f8:     0x00000001



```




- 发 1 地址波形：
![[Pasted image 20250417172105.png]]




## 测试四线四字节地址：4 字节地址+1 字节模式位+7dummy
```shell
set  *((volatile unsigned int *) 0xf8400008) = 0
set  *((volatile unsigned int *) 0xf840002c) = 0
set  *((volatile unsigned int *) 0xf84000f4) = 0
set  *((volatile unsigned int *) 0xf8400018) = 0
set  *((volatile unsigned int *) 0xf840001c) = 0
set  *((volatile unsigned int *) 0xfa490080) = 0x0f
set  *((volatile unsigned int *) 0xfa490080) = 0xf & ~(1 << 0)
set  *((volatile unsigned int *) 0xf8400008) = 0

set  *((volatile unsigned int *) 0xf84000f0) = 0x01

set  *((volatile unsigned int *) 0xf8400014) = 0x40
set  *((volatile unsigned int *) 0xf8400004) = 0x00
set  *((volatile unsigned int *) 0xf840004c) = 0x00
set  *((volatile unsigned int *) 0xf8400008) = 0x00

set  *((volatile unsigned int *) 0xf8400010) = 0x01
set  *((volatile unsigned int *) 0xf840004c) = 0x00

set  *((volatile unsigned int *) 0xf84000f4) = (0 << 30) | (0x2 << 8) | (0xa << 2) | 0x1 | (7 << 11)
set  *((volatile unsigned int *) 0xf84000f4) |= (1 << 16)

set  *((volatile unsigned int *) 0xf84000f8) = 1

set  *((volatile unsigned int *) 0xf8400000) = (0x1 << 31) | (0x2 << 22) | (0x1 << 10) | ((0x8 - 1) << 0)

set  *((volatile unsigned int *) 0xf8400018) = (0x2 << 16)
set  *((volatile unsigned int *) 0xf8400004) = 0x0

set  *((volatile unsigned int *) 0xf8400008) = 0x01


set  *((volatile unsigned int *) 0xf8400060) = 0xed
set  *((volatile unsigned int *) 0xf8400060) = 0x12345678
set  *((volatile unsigned int *) 0xf8400060) = 0x9a
```
![[Pasted image 20250417201514.png]]

## 测试四线三字节地址：3 字节地址+1 字节模式位+7dummy
```shell
set  *((volatile unsigned int *) 0xf8400008) = 0
set  *((volatile unsigned int *) 0xf840002c) = 0
set  *((volatile unsigned int *) 0xf84000f4) = 0
set  *((volatile unsigned int *) 0xf8400018) = 0
set  *((volatile unsigned int *) 0xf840001c) = 0
set  *((volatile unsigned int *) 0xfa490080) = 0x0f
set  *((volatile unsigned int *) 0xfa490080) = 0xf & ~(1 << 0)
set  *((volatile unsigned int *) 0xf8400008) = 0

set  *((volatile unsigned int *) 0xf84000f0) = 0x01

set  *((volatile unsigned int *) 0xf8400014) = 0x40
set  *((volatile unsigned int *) 0xf8400004) = 0x00
set  *((volatile unsigned int *) 0xf840004c) = 0x00
set  *((volatile unsigned int *) 0xf8400008) = 0x00

set  *((volatile unsigned int *) 0xf8400010) = 0x01
set  *((volatile unsigned int *) 0xf840004c) = 0x00

set  *((volatile unsigned int *) 0xf84000f4) = (0 << 30) | (0x2 << 8) | (0x8 << 2) | 0x1 | (7 << 11)
set  *((volatile unsigned int *) 0xf84000f4) |= (1 << 16)

set  *((volatile unsigned int *) 0xf84000f8) = 1

set  *((volatile unsigned int *) 0xf8400000) = (0x1 << 31) | (0x2 << 22) | (0x1 << 10) | ((0x8 - 1) << 0)

set  *((volatile unsigned int *) 0xf8400018) = (0x2 << 16)
set  *((volatile unsigned int *) 0xf8400004) = 0x0

set  *((volatile unsigned int *) 0xf8400008) = 0x01


set  *((volatile unsigned int *) 0xf8400060) = 0xed
set  *((volatile unsigned int *) 0xf8400060) = 0x12345678
```
![[Pasted image 20250417201802.png]]


## 测试四线三字节地址时，数据使用单字节的方式往 fifo 填不会按照意愿的顺序发出去

^61a92a

```shell
set  *((volatile unsigned int *) 0xf8400008) = 0
set  *((volatile unsigned int *) 0xf840002c) = 0
set  *((volatile unsigned int *) 0xf84000f4) = 0
set  *((volatile unsigned int *) 0xf8400018) = 0
set  *((volatile unsigned int *) 0xf840001c) = 0
set  *((volatile unsigned int *) 0xfa490080) = 0x0f
set  *((volatile unsigned int *) 0xfa490080) = 0xf & ~(1 << 0)
set  *((volatile unsigned int *) 0xf8400008) = 0

set  *((volatile unsigned int *) 0xf84000f0) = 0x01

set  *((volatile unsigned int *) 0xf8400014) = 0x40
set  *((volatile unsigned int *) 0xf8400004) = 0x00
set  *((volatile unsigned int *) 0xf840004c) = 0x00
set  *((volatile unsigned int *) 0xf8400008) = 0x00

set  *((volatile unsigned int *) 0xf8400010) = 0x01
set  *((volatile unsigned int *) 0xf840004c) = 0x00

set  *((volatile unsigned int *) 0xf84000f4) = (0 << 30) | (0x2 << 8) | (0x8 << 2) | 0x1 | (7 << 11)
set  *((volatile unsigned int *) 0xf84000f4) |= (1 << 16)

set  *((volatile unsigned int *) 0xf84000f8) = 1

set  *((volatile unsigned int *) 0xf8400000) = (0x1 << 31) | (0x2 << 22) | (0x1 << 10) | ((0x8 - 1) << 0)

set  *((volatile unsigned int *) 0xf8400018) = (0x2 << 16)
set  *((volatile unsigned int *) 0xf8400004) = 0x0

set  *((volatile unsigned int *) 0xf8400008) = 0x01


set  *((volatile unsigned char *) 0xf8400060) = 0xed
set  *((volatile unsigned char *) 0xf8400060) = 0x12
set  *((volatile unsigned char *) 0xf8400060) = 0x34
set  *((volatile unsigned char *) 0xf8400060) = 0x56
set  *((volatile unsigned char *) 0xf8400060) = 0x78
```
![[Pasted image 20250417202113.png]]

- 看起来发的 clk 周期一致，但是发地址的时候和要发的数据不一样


## 测试双线三字节地址：3 字节地址 + 1 字节模式位 + 4 dummy
```shell
set  *((volatile unsigned int *) 0xf8400008) = 0
set  *((volatile unsigned int *) 0xf840002c) = 0
set  *((volatile unsigned int *) 0xf84000f4) = 0
set  *((volatile unsigned int *) 0xf8400018) = 0
set  *((volatile unsigned int *) 0xf840001c) = 0
set  *((volatile unsigned int *) 0xfa490080) = 0x0f
set  *((volatile unsigned int *) 0xfa490080) = 0xf & ~(1 << 0)
set  *((volatile unsigned int *) 0xf8400008) = 0

set  *((volatile unsigned int *) 0xf84000f0) = 0x01

set  *((volatile unsigned int *) 0xf8400014) = 0x40
set  *((volatile unsigned int *) 0xf8400004) = 0x00
set  *((volatile unsigned int *) 0xf840004c) = 0x00
set  *((volatile unsigned int *) 0xf8400008) = 0x00

set  *((volatile unsigned int *) 0xf8400010) = 0x01
set  *((volatile unsigned int *) 0xf840004c) = 0x00

set  *((volatile unsigned int *) 0xf84000f4) = (0 << 30) | (0x2 << 8) | (0x8 << 2) | 0x1 | (4 << 11)
set  *((volatile unsigned int *) 0xf84000f4) |= (1 << 16)

set  *((volatile unsigned int *) 0xf84000f8) = 1

set  *((volatile unsigned int *) 0xf8400000) = (0x1 << 31) | (0x1 << 22) | (0x1 << 10) | ((0x8 - 1) << 0)

set  *((volatile unsigned int *) 0xf8400018) = (0x1 << 16)
set  *((volatile unsigned int *) 0xf8400004) = 0x0

set  *((volatile unsigned int *) 0xf8400008) = 0x01


set  *((volatile unsigned int *) 0xf8400060) = 0xed
set  *((volatile unsigned int *) 0xf8400060) = 0x12345678

```
![[Pasted image 20250418140617.png]]