# 测试进度： **aquila_top_FPGA_2024-03-20bit0.bit**



```
|SPIx|核|功能|测试结果|备注|
|/21. SPI1 (0xAA000000)|
|/5. bt 核|8/32bit 标准、双倍速、四倍速|通过|标准模式会发生 Receive FIFO 溢出|
|dma 开/关|通过||
|ddr 开/关|通过||
|xip 开/关|通过||
|中断 开/关|失败||
|/5. sensor 核|8/32bit 标准、双倍速、四倍速|通过|标准模式会发生 Receive FIFO 溢出|
|dma 开/关|通过||
|ddr 开/关|通过||
|xip 开/关|通过||
|中断 开/关|通过||
|/5. ap 核|8/32bit 标准、双倍速、四倍速|通过|标准模式会发生 Receive FIFO 溢出|
|dma 开/关|通过||
|ddr 开/关|通过||
|xip 开/关|通过||
|中断 开/关|通过||
|/5. secure 核|8/32bit 标准、双倍速、四倍速|通过|标准模式会发生 Receive FIFO 溢出|
|dma 开/关|通过||
|ddr 开/关|通过||
|xip 开/关|失败||
|中断 开/关|失败||
|/5. SPI0 (0x82000000)|
|/1. bt 核|||访问不到 0x82000000 寄存器|
|/1. sensor 核||||
|/1. ap 核||||
|/1. secure 核|||访问 0x82000000 寄存器，DebugServer 就会挂掉|
```







# 测试进度： **aquila_top_k7_bit0_FPGA_2024-03-22.bit**



* 此 bit 用于测试 flash spi0，flash spi0 和 flash spi1 无法同时测试

```
|SPIx|核|功能|测试结果|备注|
|/21. SPI0 (0x82000000)|
|/5. bt 核|||访问不到 0x82000000 寄存器|
|/5. sensor 核|||访问不到 0x82000000 寄存器|
|/5. ap 核|||访问不到 0x82000000 寄存器|
|/5. secure 核|8/32bit 标准、双倍速、四倍速|通过||
|dma 开/关|通过|使用同步 DMA，未使用异步 DMA|
|ddr 开/关|通过||
|xip 开/关|通过|dma master 源和目的都要使用 MASTER3|
|中断 开/关|失败|secure 核无法触发中断|
```





22号的bit测试情况，这样是正常的吗？
1. 除 secure 核外其他核都无法访问 0x82000000 寄存器。
2. secure 核无法触发 spi 中断;无法测试 xip(不能关闭 icache，否则会触发 cpu 异常);dma 需要使用同步 dma，不能使用异步 dma(无中断号，是不是也没法触发 dma 的中断?)







# 新增测试项



1. 使用另一套寄存器配置 XIP

   ```
   ENR
   INCR
   WRAP
   CTRL
   SER
      timeout
   	DRIVE_EDGE
   	mode_bits
   ```



- 需要配置 分频、sample_delay

- 配置完 ao 这一套之后，再去配置 ssi 会触发异常

  ```
  (gdb) x/20x 0x83200040
  0x83200040:	0x28c01465	0x00000001	0x00000001	0x00000001
  0x83200050:	0x00000000	0x00000000	0x00000000	0x00000000
  0x83200060:	0x00000000	0x00000000	0x00000000	0x00000000
  0x83200070:	0x00000000	0x00000000	0x00000000	0x00000000
  0x83200080:	0x00000000	0x00000000	0x00000000	0x00000000
  (gdb) set *0x8320004c=0
  (gdb) x/20x 0x83200040 
  0x83200040:	0x28c01465	0x00000001	0x00000001	0x00000000
  0x83200050:	0x00000000	0x00000000	0x00000000	0x00000000
  0x83200060:	0x00000000	0x00000000	0x00000000	0x00000000
  0x83200070:	0x00000000	0x00000000	0x00000000	0x00000000
  0x83200080:	0x00000000	0x00000000	0x00000000	0x00000000
  (gdb) x/20x 0x82000000
  0x82000000:	0xffffffff	0x82000004	Remote communication error.  Target disconnected.: 连接被对方重设.
  (gdb) 
  ```

  

1. 测试模式烧写 flash






# 测试进度： **aquila_top_FPGA_2024-03-20bit0.bit**
```
|DMAx|核|功能|测试结果|备注|
|/33. DMA2 (0xC2000000)|
|/8. bt 核|||只能 secure 核访问|
|/8. sensor 核|||只能 secure 核访问|
|/8. ap 核|||只能 secure 核访问|
|/8. secure 核|不同内存区域的DMA传输|通过||
|不同的DMA传输宽度|通过||
|不同源和目的msize的DMA传输|通过||
|不同master的DMA传输|失败|dma2 只能用 Master2 访问内存|
|不同channel 的DMA传输|通过||
|DMA中断||没有中断号|
|flash spi xip 地址到内存的DMA传输|通过||
```





* DMA2 只测了内存到内存，外设使用的是异步 dma，但是文档没有给 dma 中断号，需要咨询芯片




msp0 flash 测试模式下，通过特定的引脚灌数据来完成烧写 flash，目前方案如下：

1. 采用两块板子，一块 apus 板子，一块 fpga 板。将 apus 板子上接到 flash 上的几个脚跳到 fpga 板上的测试脚 (应该要共地)
2. 采用 sysi2c 来配置 fpga 进入测试模式 
3. 执行 apus 的烧写命令，相当于 apus 烧录 flash，但是 flash 并不是放在 apus 板子上，而是放在 fpga 板 
4. 确认 flash 数据是否烧写成功

ate 测试模式：

1. flash 的几个引脚和 mspi0 的几个引脚不是同一组，是吧？
2. i2c 和 flash 的引脚对应 fmc 子板上的是哪几个脚？

ate 测试模式：

1. flash 的几个引脚(PH5~PH10)和 mspi0 的几个引脚(PH20~PH25)不是同一组，是吧？
2. i2c(PH11~PH12) 和 flash 的引脚(PH5~PH10)对应 fmc 子板上的是哪几个脚？ 



# 单线测试模式：
1. fpga1 进入测试模式。fpga0 读 id，能否读到 ID？
    - 能够读到 ID（有时需要读多次才能读到 ID，错误时读到的都是 0xff）
    - 逻辑分析仪抓 flash 上的引脚都看得到数据
    - 示波器看 测试引脚(MISO)、flash 上的 MISO 引脚都有数据
2. fpga1 的 flash 有数据，进入测试模式。fpga0 直接读，是否能读到数据？
    - 读不到数据，全为 0xff
    - 逻辑分析仪抓 flash 上的 CS、CLK、MOSI 引脚都有数据，MISO引脚看不到数据
    - 示波器看 测试引脚(MISO)、flash 上的 MISO 引脚没有数据
3. fpga1 的 flash 没有数据，进入测试模式。fpga0 直接写，能够写进去数据？
    - 烧进去之后，立马用测试模式读，读到的都是 0xff。
    - 退出测试模式，看 flash 中的数据为：08310621 20244922 491221c6 44883108。实际的数据为 11223344 22334455 22334455 11223344
    - 逻辑分析仪抓 flash 上的 CS、CLK、MOSI、MISO 引脚都有数据，MOSI 引脚上看起来是有数据写出去的
    - 示波器抓 测试引脚(MOSI) 也有波形

# 四线测试模式：
1. fpga1 进入测试模式。fpga0 使用四线模式写，能否写进去数据？
    - 退出测试模式，查看 flash 数据是否正确
    - flash 中的数据：
    b3410113 34011073 116000ef 32c050ef    ..A.s..4..`..P.2
    034050ef 10001ac0 10283809 10307711    .P@......8(..w0.

b3410113 34011073 116000ef 32c050ef    ..A.s..4..`..P.2
3c4050ef 0001a001 01818293 00017317    .P@<.........s..


<pre>
80824505 6c121104 2c626c20 e80aac42    .E.....l lb,B...
30365345 04e15100 2478161c 9ef9186f    ES60.Q....x$o...
1a431438 007630a9 704d3e26 1f743ac8    8.C..0v.&>Mp.:t.
</pre>
    - 实际的数据:
<pre>
80824505 c2261141 c422c606 84aec02a    .E..A.&...".*...
00633355 41101e05 468267c1 e3998ff1    U3c....A.g.F....
aa314481 02600793 0cd7e463 fa41a783    .D1...`.c.....A.
</pre>

- 另一个方案：
    - 将测试引脚映射到 generic spi
    - 既然是要通过外部引脚灌信号到 flash，那么通过 generic spi 灌信号到 flash 其实也可以验证逻辑


- 写了多次都没写进去，当前分频 512，改成 32 试试
- 降频不行，把逻辑分析仪去掉看看

1. 数据块长度：512Byte;校验码长度:14Byte;纠错能力8bits
2. 写是一页一页写 page 的方式，用的 0x80、0x10 的方式。对于随机长度，如果不满 page 的会填充 0xff 至整个 page 操作


# ddr 2 分频

- 修改 cs 操作接口，2分频时需要使能 0x1080 的 bit[11:10]
- 2分频时需要使能 rxds
- 配置 0x1090 的 dll 来使得 clk 在 data 的中间





## quad 8bit 卡死问题

- 5月23号bit：

```
ratio1 = 0,ratio0 = 128,master = 0   读到 xip 数据全对，ap、sensor 核全对
ratio1 = 0,ratio0 = 128,master = 1   读到 xip 数据全对，ap、sensor 核全对
ratio1 = 0,ratio0 = 128,master = 2   读到 xip 数据全对，ap、sensor 核全对
ratio1 = 0,ratio0 = 128,master = 3   读到 xip 数据全对，ap、sensor 核全对
ratio1 = 0,ratio0 = 128,master = 4   ap 核几个字节数据读错，sensor 核读的数据全错，一个字节有数据，一个字节为 0
ratio1 = 0,ratio0 = 128,master = 5   ap 核几个字节数据读错，sensor 核读的数据全错，一个字节有数据，一个字节为 0
        xip test error! flash addr:0x1e15, xip data 0x80, rand_data 0xc6766782
        xip test error! flash addr:0x1e16, xip data 0x60, rand_data 0xc6766782
        xip test error! flash addr:0x1e18, xip data 0x0, rand_data 0x5a427ae1
        xip test error! flash addr:0x1e19, xip data 0xe0, rand_data 0x5a427ae1
        xip test error! flash addr:0x1e1a, xip data 0x70, rand_data 0x5a427ae1
        xip test error! flash addr:0x1e1b, xip data 0x0, rand_data 0x5a427ae1
        xip test error! flash addr:0x1e1c, xip data 0x0, rand_data 0xb6d45dae
ratio1 = 0,ratio0 = 128,master = 6   读到 xip 数据全是 0x0
ratio1 = 0,ratio0 = 128,master = 7   读到 xip 数据全是 0x0



master = 127,ratio1 = 0,ratio0 = 1    0xaa001090:	0x31057f11	0x00000001  ap 核正确
master = 127,ratio1 = 1,ratio0 = 1    0xaa001090:	0x31057f11	0x00000101  ap 核正确
master = 127,ratio1 = 2,ratio0 = 1    0xaa001090:	0x31057f11	0x00000201  ap 核正确
master = 127,ratio1 = 3,ratio0 = 1    0xaa001090:	0x31057f11	0x00000301  ap 核几个字节数据出错
master = 127,ratio1 = 4,ratio0 = 1    0xaa001090:	0x31057f11	0x00000401  ap 核几个字节数据出错
master = 127,ratio1 = 5,ratio0 = 1    ap 核读了1/2个字节数据后就卡死，卡死后 gdb 查看寄存器
		(gdb) x/20x 0xaa000000
		0xaa000000:	0xaa000000	Remote communication error.  Target disconnected.: 连接被对方重设.
master = 127,ratio1 = 6,ratio0 = 1    ap 核读了1/2个字节数据后就卡死，卡死后 gdb 查看寄存器(和 5 一样的情况)

master = 127,ratio1 = 10,ratio0 = 1    ap 核读了10多个个字节数据后就卡死，卡死后 gdb 查看寄存器(和 5 一样的情况)
master = 127,ratio1 = 20,ratio0 = 1    ap 核读了0x1e6多个个字节数据后就卡死，卡死后 gdb 查看寄存器(和 5 一样的情况)
```









## ddr 模式

    两个寄存器都会使能 DDR 模式，猜测 SPI_CTRLR0 寄存器控制的是发命令的时候用 0x0d、0xbd、0xed 命令，XIP_CTRLR寄存器控制的是
使用 XIP 操作的时候用的是 0x0d、0xbd、0xed 命令。(默认 XIP 操作用的是 0xbb、0xeb，xip 只有 dual、quad 才支持)

- SPI_CTRLR0
    - bit[]
- XIP_CTRL



![选区_425](image/选区_425.png)





# ate 测试

- [通过] 从 0 地址开始读写擦
- [通过] 从 0 地址开始读写擦，不写满整个 page 也可以
- [失败] 从 0x10 地址开始就不行，0x110 也不行，表现在写的时候逻辑分析仪抓到有波形，读的时候读的是全 0xff
- [通过] flash 中有数据，从 0 地址开始读，数据都是对的
- [失败] flash 中有数据，从 0x10 地址开始读,数据全是 0xff


正常情况下去写，测试模式下去读： 
- [通过] flash 中有数据，从 0 地址开始读，数据都是对的
- [失败] flash 中有数据，从 0x10 地址开始读,数据全是 0xff

测试模式写：正常模式读：
- [通过] 从 0 地址开始写，读到的数据是对的
- [通过] 从 0x110 地址开始写，读到的数据也是对的

Flash 正常模式写满数据，测试模式下去读：
- [通过] 从 0 地址开始读，数据都是对的
- [失败] 从 0x10 地址开始读，数据全是 0xff


# scpu code
- gx_hal_spi_hw_init 中的配置 `DDR_DRIVE_EDGE` 永远不会执行到
```c
	if(config->ddr_en)
		gx_hal_dw_writel(dws, GX_HAL_SPI_DDR_DRIVE_EDGE, config->ddr_drive_edge);
	dws->freq_src = config->freq_src;
```
ate 测试单线模式已经测通。
ate 四线模式全部都是输入引脚，获取不了 flash 的状态(写/擦除时需要等待 flash  busy)。
通过 i2c 配置引脚输入输出，存在的问题是 i2c 不知道何时需要切状态，i2c 是跑在 pc 上的程序，无法获取当前程序跑在什么状态(是否写完，是否擦完)。
后面还要不要使用四线模式？

1. ate 单线模式已经测通。

2. ate 四线模式中所有引脚均为输入，无法获取 Flash 的状态（在写入或擦除操作时需要等待 Flash 忙状态）。通过 I2C 配置引脚的输入和输出存在一个问题，即 I2C 不知道何时需要改变状态。由于 I2C 是在 PC 上运行的程序，无法获取当前程序的运行状态（例如是否完成写入或擦除操作）。

3. 后续还要不要使用四线模式?

i2c0
padmux_set(39, 56)
padmux_set(38, 57)

i2c1
padmux_set(39, 66 + bus * 2 - 6);
padmux_set(38, 67 + bus * 2 - 6);

进了单线测试模式，如何退出切四线？

+
+       unsigned char tx_data[4] = {0};
+       unsigned char rx_data[4] = {0};
+       unsigned int slv_addr = 0x2a;
+
+       padmux_set(39, 56);
+       padmux_set(38, 57);
+
+       tx_data[0] = 0xa5;
+       gx_i2c_tx(0, slv_addr, 0x0, 4, tx_data, 4);
+       gx_i2c_rx(0, slv_addr, 0x0, 4, rx_data, 4);
+
+       printf("rx_data:0x%x 0x%x 0x%x 0x%x. \n", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
+
+       tx_data[0] = 0x5a;
+       gx_i2c_tx(0, slv_addr, 0x24, 4, tx_data, 4);
+       gx_i2c_rx(0, slv_addr, 0x24, 4, rx_data, 4);
+
+       printf("rx_data:0x%x 0x%x 0x%x 0x%x. \n", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
+
+       tx_data[0] = 0xc;
+       gx_i2c_tx(0, slv_addr, 0x4, 4, tx_data, 4);
+       gx_i2c_rx(0, slv_addr, 0x4, 4, rx_data, 4);
+
+       printf("rx_data:0x%x 0x%x 0x%x 0x%x. \n", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
+
+       tx_data[0] = 0x0;
+       gx_i2c_tx(0, slv_addr, 0x30, 4, tx_data, 4);
+       gx_i2c_rx(0, slv_addr, 0x30, 4, rx_data, 4);
+
+       printf("rx_data:0x%x 0x%x 0x%x 0x%x. \n", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
+
+       tx_data[0] = 0x1;
+       gx_i2c_tx(0, slv_addr, 0xc, 4, tx_data, 4);
+       gx_i2c_rx(0, slv_addr, 0xc, 4, rx_data, 4);
+
+       printf("rx_data:0x%x 0x%x 0x%x 0x%x. \n", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
+
+       gx_i2c_rx(0, slv_addr, 0x0, 4, rx_data, 4);
+       printf("rx_data:0x%x 0x%x 0x%x 0x%x. \n", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
+       gx_i2c_rx(0, slv_addr, 0x24, 4, rx_data, 4);
+       printf("rx_data:0x%x 0x%x 0x%x 0x%x. \n", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
+       gx_i2c_rx(0, slv_addr, 0x4, 4, rx_data, 4);
+       printf("rx_data:0x%x 0x%x 0x%x 0x%x. \n", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
+       gx_i2c_rx(0, slv_addr, 0x30, 4, rx_data, 4);
+       printf("rx_data:0x%x 0x%x 0x%x 0x%x. \n", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
+

# 其它核访问安全核 xip



- 已测试：

  ```
  ate10
  ate11
  ddr/aquila_top.bit
  iopmp:4-15.bit
  pr_1:aquila_top_k7.bit
  3-20bit0.bit
  aquila_top_k7_bit0_FPGA_2024-03-21a.bit
  aquila_top_k7_bit0_FPGA_2024-03-21.bit
  aquila_top_k7_bit0_FPGA_2024-03-22.bit
  aquila_top_k7_bit0_FPGA_2024-03-29.bit
  ```

  





- 安全核编译一个 elf
- 其它核编译一个 bin，烧到 flash
- 启动安全核，配置 xip，配好之后什么都不干
- 设置其它核的 pc 到 xip 地址开始启动，如果启动成功，那就验到了其它核访问安全核的 xip

现在测 ate 退出测试模式有点问题，大概情况是这样：

fpga0 i2c:
    fpga0 的 i2c 发波形进入测试模式, ok
    进测试模式后，要退出测试模式，fpga0 的 i2c 收不到 fpga1 的应答

usb -- i2c 工具:
    usb -- i2c 工具发波形进入测试模式, ok
    进测试模式后，usb -- i2c 工具发波形退出测试模式, ok


目前有问题的点：
    1. 能够进测试模式，但是不能退出测试模式。进测试模式的时候 i2c 响应，但是退出测试模式的时候 i2c 不响应。测试模式和非测试模式有什么差异？
    2. usb -- i2c 工具能响应，fpga0 的 i2c 不能响应，对 fpga0 上的 i2c 波形时序有什么要求？

- 用 fpga0 的 i2c 发波形进入测试模式，ok
- 进了测试模式之后，要退出测试模式，此时 fpga1 的没有应答信号
- 用 usb 转 i2c 模块进入测试模式之后，退出测试模式，fpga1 有应答信号
- fpga1 的 i2c 对退出测试模式时的 sda 波形有什么要求吗？

ate 测试用一块 fpga 板(fpga0)上的 i2c 发信号给另一块 fpga 板(fpga1) 的 system i2c 进入测试模式(system i2c 有应答信号)
成功进入测试模式后，继续使用 fpga0 的 i2c 发命令来退出测试模式，fpga1 上的 system i2c 无应答信号
采用 usb 转 i2c 工具能够退出模式，因为工具发完地址之后即使 system i2c 无应答，仍会继续发数据给 system i2c，并且能够成功写入 system i2c(海波仿真：不响应仍可继续写入数据到 system i2c)
软件发完地址，system i2c 无应答信号则会退出发送。并且软件无法修改为无应答继续发数据的模式，由于无应答时 i2c ip 会上报一个错误状态，并且 ip 自己发了 stop 信号
fpga0 的 i2c 频率改成 400k、100k、50k、20k 测试情况均一致

ate 测试的时候发现 system i2c 存在两个问题：
    1. 进入测试模式后，system i2c 无法返回 ack
        - 用一块 fpga 板(fpga0) 上的 i2c 发信号给另一块 fpga 板(fpga1) 的 system i2c 用于进入测试模式(此时 system i2c 有应答信号)
        - 进入测试模式后，用 fpga0 的 i2c 发信号来退出测试模式，fpga1 的 system i2c 不会应答
            - fpga0 的 i2c 频率试过 400k、100k、50k、20k 都是一样的情况，system i2c 没有应答
    2. system i2c 在不返回 ack 的情况下，仍可继续向 system i2c 写数据
        - 用 usb 转 i2c 工具向 fpga1 的 system i2c 发信号，无应答后继续发数据，能够成功退出测试模式
            - 海波仿真 system i2c 不响应的情况下确实可以写入数据到 system i2c
        - 软件没法做到不返回 ack 的情况下继续向 system i2c 写数据。因为无应答的时候 i2c 会上报一个错误状态，ip 自己发了 stop 信号


```diff
 /* SPI FLASH */
 #define CONFIG_FLASH_SPI_CLK_SRC        48000000
-#define CONFIG_SF_DEFAULT_CLKDIV        4  /* 分频数必须为偶数且非0 */
+#define CONFIG_SF_DEFAULT_CLKDIV        2  /* 分频数必须为偶数且非0 */
 #define CONFIG_SF_DEFAULT_SAMPLE_DELAY  1
 #define CONFIG_SF_DEFAULT_SPEED         (CONFIG_FLASH_SPI_CLK_SRC / CONFIG_SF_DEFAULT_CLKDIV)
 
diff --git a/cmd/flash/spi_nor_flash_test.c b/cmd/flash/spi_nor_flash_test.c
index 3cd48154..0efe1eaa 100644
--- a/cmd/flash/spi_nor_flash_test.c
+++ b/cmd/flash/spi_nor_flash_test.c
@@ -10,7 +10,7 @@
 extern unsigned int rand(void);
 
 #define CMD_NOR_TEST_MALLOC_MAX_LEN (20*1024)
-#define CMD_NOR_TEST_RAND_CNT       1000
+#define CMD_NOR_TEST_RAND_CNT       1
 
 #define CMD_NOR_CACHE_LINE_SIZE DCACHE_LINE_SIZE
 #define CMD_NOR_CACHE_LINE_MASK ((uint32_t)(CMD_NOR_CACHE_LINE_SIZE-1))
@@ -120,7 +120,7 @@ static int nor_test_read(CMD_NOR_TEST_RW_CFG_T *cfg)
        return (readlen == r.len) ? 0 : -1;
 }
 
-static void nor_test_write(CMD_NOR_TEST_RW_CFG_T *cfg)
+static int nor_test_write(CMD_NOR_TEST_RW_CFG_T *cfg)
 {
        uint32_t writelen = 0;
        GX_HAL_SF_RW_T w = {0};
diff --git a/drivers/hal/src/spi/dw_spi/gx_hal_spi.c b/drivers/hal/src/spi/dw_spi/gx_hal_spi.c
index 964d173e..9a3cd475 100644
--- a/drivers/hal/src/spi/dw_spi/gx_hal_spi.c
+++ b/drivers/hal/src/spi/dw_spi/gx_hal_spi.c
@@ -462,6 +462,12 @@ int gx_hal_qspi_transfer_prepare(struct gx_hal_dw_spi *dws, struct gx_hal_spi_tr
        {
                gx_hal_dw_writel(dws,GX_HAL_SPI_DDR_DRIVE_EDGE, transfer->ddr_drive_edge);
                val |= 1 << GX_HAL_ESPI_CTRLR0_SPI_DDR_EN;
+#if 1
+               val |= 1 << 10;
+               gx_hal_dw_writel(dws, 0x1080, 1<<11 | 1<<10);
+               gx_hal_dw_writel(dws, 0x1090, 5<<8 | 1<<4 | 1);
+               gx_hal_dw_writel(dws, 0x1094, 0<<8 | 128);
+#endif
        }
        gx_hal_dw_writel(dws,GX_HAL_SPI_SPI_CTRLR0, val);
```


- 0xaa001090: 8:dll_master_set   4:1 0:1
- 0xaa001094: 8:dll_ratio_1(ddr 模式下对 flash 时钟管脚换回的时钟进行延时，用于数据采样)  0:dll_ratio_0(对输出的 flash 线上时钟进行延时)
    ratio_1 慢时钟时不需要配置


- ratio1，2，3配置255，dll_master_value配置1，延时0ns，读写数据全部出错   .
```
(gdb) p w
$3 = {addr = 16, len = 256, 
  data = 0x20020ce0 <nor_test_buff+10240> ")#\276\204\341l֮R\220I\361\361\273\351볦\333<\207\f>\231$^\r\034\006\267G\336\263\022M\310C\273\213\246\037\003Z}\t8%\037]\324\313\374\226\365E;\023\r\211\n\034ۮ2 \232P\356@x6\375\022I2\366\236}IܭO\024\362D@f\320k\304\060\267\062;\241\"\366\"\221\235\341\213\037ڰʙ\002\271r\235I,\200~ř\325進\352\311\314S\277gֿ\024\326~-\334\216f\203\357WIa\377i\217a\315\321\036\235\234\026rr\346\035\360\204OJw\002\327\350\071,S\313\311\022\036\063t\236\f\364\325ԟԤY~5\317\062\"\364\314\317Ӑ-Hӏu\346\331\035*"..., dma_en = 0 '\000', bus_mode = 2 '\002', data_width_8or32bit = 1 '\001', espi_ddr_en = 0 '\000'}
(gdb) x/20x 0x20020ce0
0x20020ce0 <nor_test_buff+10240>:	0x84be2329	0xaed66ce1	0xf1499052	0xebe9bbf1
0x20020cf0 <nor_test_buff+10256>:	0x3cdba6b3	0x993e0c87	0x1c0d5e24	0xde47b706
0x20020d00 <nor_test_buff+10272>:	0xc84d12b3	0xa68bbb43	0x7d5a031f	0x1f253809
0x20020d10 <nor_test_buff+10288>:	0xfccbd45d	0x3b45f596	0x0a890d13	0x32aedb1c
0x20020d20 <nor_test_buff+10304>:	0xee509a20	0xfd367840	0xf6324912	0xdc497d9e
(gdb) n
216		if(cfg->is_print)
(gdb) n
217			printf("sflash page program finish %d.\n", i);
(gdb) n
219		i = gx_nor_read(&r);
(gdb) n
220		if(cfg->is_print)
(gdb) p r
$4 = {addr = 16, len = 256, 
  data = 0x2001e4e0 <nor_test_buff> "\t#\276\204\341l֮R\220I\361\361\273\351볦\333<\207\f>\231$^\r\034\006\267G\336\061$܄;\270\272a5\247Г\202Q\365ݿ\311oTS\261\060ءͺ\343\"\t\245\016\a\203o\321$\223/iԝ\312\324\361O$Dm\006\274C\vs#\272/b)\031\336\030\261\375\f\251\220+\227)Ԓ\a\354Y\235^\230\v.\234\305;\366}k\361M\342\335\310\346h>\365t\037\366\230\366\034\335\021\351\301g'.a\337\bD\247p-~\203\222\305<\221!\343\067I\340\317MI\375JE\227\343\\\363/L\314\375\071\002ԍ\367^m\221Ү\\\017\267\210\030t@\345", <incomplete sequence \365>, dma_en = 0 '\000', bus_mode = 2 '\002', data_width_8or32bit = 1 '\001', espi_ddr_en = 1 '\001'}
(gdb) x/20x 0x2001e4e0
0x2001e4e0 <nor_test_buff>:	0x84be2309	0xaed66ce1	0xf1499052	0xebe9bbf1
0x2001e4f0 <nor_test_buff+16>:	0x3cdba6b3	0x993e0c87	0x1c0d5e24	0xde47b706
0x2001e500 <nor_test_buff+32>:	0x84dc2431	0x61bab83b	0x93d0a735	0xddf55182
0x2001e510 <nor_test_buff+48>:	0x546fc9bf	0xd830b153	0xe3bacda1	0x0ea50922
0x2001e520 <nor_test_buff+64>:	0xd16f8307	0x692f9324	0xd4ca9dd4	0x44244ff1
```

- ratio1，2，3配置128，dll_master_value配置3，延时2ns，读写数据偶尔出错，并且比延时4ns是出错频繁 

```
(gdb) p w
$3 = {addr = 16, len = 256, 
  data = 0x20020ce0 <nor_test_buff+10240> ")#\276\204\341l֮R\220I\361\361\273\351볦\333<\207\f>\231$^\r\034\006\267G\336\263\022M\310C\273\213\246\037\003Z}\t8%\037]\324\313\374\226\365E;\023\r\211\n\034ۮ2 \232P\356@x6\375\022I2\366\236}IܭO\024\362D@f\320k\304\060\267\062;\241\"\366\"\221\235\341\213\037ڰʙ\002\271r\235I,\200~ř\325進\352\311\314S\277gֿ\024\326~-\334\216f\203\357WIa\377i\217a\315\321\036\235\234\026rr\346\035\360\204OJw\002\327\350\071,S\313\311\022\036\063t\236\f\364\325ԟԤY~5\317\062\"\364\314\317Ӑ-Hӏu\346\331\035*"..., dma_en = 0 '\000', bus_mode = 2 '\002', data_width_8or32bit = 1 '\001', espi_ddr_en = 0 '\000'}
(gdb) x/20x 0x20020ce0
0x20020ce0 <nor_test_buff+10240>:	0x84be2329	0xaed66ce1	0xf1499052	0xebe9bbf1
0x20020cf0 <nor_test_buff+10256>:	0x3cdba6b3	0x993e0c87	0x1c0d5e24	0xde47b706
0x20020d00 <nor_test_buff+10272>:	0xc84d12b3	0xa68bbb43	0x7d5a031f	0x1f253809
0x20020d10 <nor_test_buff+10288>:	0xfccbd45d	0x3b45f596	0x0a890d13	0x32aedb1c
0x20020d20 <nor_test_buff+10304>:	0xee509a20	0xfd367840	0xf6324912	0xdc497d9e
(gdb) n
216		if(cfg->is_print)
(gdb) n
217			printf("sflash page program finish %d.\n", i);
(gdb) n
219		i = gx_nor_read(&r);
(gdb) n
220		if(cfg->is_print)
(gdb) p r
x$4 = {addr = 16, len = 256, 
  data = 0x2001e4e0 <nor_test_buff> "\t#\276\204\341l֮R\220I\361\361\273\351볦\333<\207\f>\231$^\r\034\006\267G\336\061$܄;\270\272a5\247Г\202Q\365ݿ\311oTS\261\060ءͺ\343\"\t\245\016\a\203o\321$\223/iԝ\312\324\361O$Dm\006\274C\vs#\272/b)\031\336\030\261\375\f\251\220+\227)Ԓ\a\354Y\235^\230\v.\234\305;\366}k\361M\342\335\310\346h>\365t\037\366\230\366\034\335\021\351\301g'.a\337\bD\247p-~\203\222\305<\221!\343\067I\340\317MI\375JE\227\343\\\363/L\314\375\071\002ԍ\367^m\221Ү\\\017\267\210\030t@\345", <incomplete sequence \365>, dma_en = 0 '\000', bus_mode = 2 '\002', data_width_8or32bit = 1 '\001', espi_ddr_en = 1 '\001'}
(gdb) x/20x 0x2001e4e0
0x2001e4e0 <nor_test_buff>:	0x84be2309	0xaed66ce1	0xf1499052	0xebe9bbf1
0x2001e4f0 <nor_test_buff+16>:	0x3cdba6b3	0x993e0c87	0x1c0d5e24	0xde47b706
0x2001e500 <nor_test_buff+32>:	0x84dc2431	0x61bab83b	0x93d0a735	0xddf55182
0x2001e510 <nor_test_buff+48>:	0x546fc9bf	0xd830b153	0xe3bacda1	0x0ea50922
0x2001e520 <nor_test_buff+64>:	0xd16f8307	0x692f9324	0xd4ca9dd4	0x44244ff1
(gdb) x/20x 0xaa001080
0xaa001080:	0x00000c00	0x00000002	0x00040201	0x00201008
0xaa001090:	0x01050311	0x00008080	0x00000008	0x00000008
0xaa0010a0:	0x00000008	0x00000008	0x00000008	0x00000008
0xaa0010b0:	0x00000000	0x00000000	0x00000000	0x00000000
0xaa0010c0:	0x00000000	0x00000000	0x00000000	0x00000000
```



- ratio1，2，3配置128，dll_master_value配置5，延时4ns，cpu 16bit 32bit读写，随机读写测试通过 
```
(gdb) p w
$2 = {addr = 16, len = 256, 
  data = 0x20020ce0 <nor_test_buff+10240> ")#\276\204\341l֮R\220I\361\361\273\351볦\333<\207\f>\231$^\r\034\006\267G\336\263\022M\310C\273\213\246\037\003Z}\t8%\037]\324\313\374\226\365E;\023\r\211\n\034ۮ2 \232P\356@x6\375\022I2\366\236}IܭO\024\362D@f\320k\304\060\267\062;\241\"\366\"\221\235\341\213\037ڰʙ\002\271r\235I,\200~ř\325進\352\311\314S\277gֿ\024\326~-\334\216f\203\357WIa\377i\217a\315\321\036\235\234\026rr\346\035\360\204OJw\002\327\350\071,S\313\311\022\036\063t\236\f\364\325ԟԤY~5\317\062\"\364\314\317Ӑ-Hӏu\346\331\035*"..., dma_en = 0 '\000', bus_mode = 2 '\002', data_width_8or32bit = 1 '\001', espi_ddr_en = 0 '\000'}
(gdb) x/20x 0x20020ce0
0x20020ce0 <nor_test_buff+10240>:	0x84be2329	0xaed66ce1	0xf1499052	0xebe9bbf1
0x20020cf0 <nor_test_buff+10256>:	0x3cdba6b3	0x993e0c87	0x1c0d5e24	0xde47b706
0x20020d00 <nor_test_buff+10272>:	0xc84d12b3	0xa68bbb43	0x7d5a031f	0x1f253809
0x20020d10 <nor_test_buff+10288>:	0xfccbd45d	0x3b45f596	0x0a890d13	0x32aedb1c
0x20020d20 <nor_test_buff+10304>:	0xee509a20	0xfd367840	0xf6324912	0xdc497d9e
(gdb) n
216		if(cfg->is_print)
(gdb) l
211		w.data = (void *)p2;
212		w.bus_mode = cfg->bus_mode;
213		w.dma_en = cfg->dma_en;
214		w.data_width_8or32bit = cfg->data_32bit;
215		i = gx_nor_write(&w);
216		if(cfg->is_print)
217			printf("sflash page program finish %d.\n", i);
218	
219		i = gx_nor_read(&r);
220		if(cfg->is_print)
(gdb) n
217			printf("sflash page program finish %d.\n", i);
(gdb) n
219		i = gx_nor_read(&r);
(gdb) n
220		if(cfg->is_print)
(gdb) p r
x$3 = {addr = 16, len = 256, 
  data = 0x2001e4e0 <nor_test_buff> "\t#\276\204\341l֮R\220I\361\361\273\351볦\333<\207\f>\231$^\r\034\006\267G\336\061$܄;\270\272a5\247Г\202Q\365ݿ\311oTS\261\060ءͺ\343\"\t\245\016\a\203o\321$\223/iԝ\312\324\361O$Dm\006\274C\vs#\272/b)\031\336\030\261\375\f\251\220+\227)Ԓ\a\354Y\235^\230\v.\234\305;\366}k\361M\342\335\310\346h>\365t\037\366\230\366\034\335\021\351\301g'.a\337\bD\247p-~\203\222\305<\221!\343\067I\340\317MI\375JE\227\343\\\363/L\314\375\071\002ԍ\367^m\221Ү\\\017\267\210\030t@\345", <incomplete sequence \365>, dma_en = 0 '\000', bus_mode = 2 '\002', data_width_8or32bit = 1 '\001', espi_ddr_en = 1 '\001'}
(gdb) x/20x 0x2001e4e0
0x2001e4e0 <nor_test_buff>:	0x84be2309	0xaed66ce1	0xf1499052	0xebe9bbf1
0x2001e4f0 <nor_test_buff+16>:	0x3cdba6b3	0x993e0c87	0x1c0d5e24	0xde47b706
0x2001e500 <nor_test_buff+32>:	0x84dc2431	0x61bab83b	0x93d0a735	0xddf55182
0x2001e510 <nor_test_buff+48>:	0x546fc9bf	0xd830b153	0xe3bacda1	0x0ea50922
0x2001e520 <nor_test_buff+64>:	0xd16f8307	0x692f9324	0xd4ca9dd4	0x44244ff1
(gdb) x/20x 0xaa001080
0xaa001080:	0x00000c00	0x00000002	0x00040201	0x00201008
0xaa001090:	0x01050511	0x00008080	0x00000008	0x00000008
0xaa0010a0:	0x00000008	0x00000008	0x00000008	0x00000008
0xaa0010b0:	0x00000000	0x00000000	0x00000000	0x00000000
0xaa0010c0:	0x00000000	0x00000000	0x00000000	0x00000000

```

- ratio1，2，3配置128，dll_master_value配置6，延时6ns，读写数据偶尔出错
```
(gdb) p w
$3 = {addr = 16, len = 256, 
  data = 0x20020ce0 <nor_test_buff+10240> ")#\276\204\341l֮R\220I\361\361\273\351볦\333<\207\f>\231$^\r\034\006\267G\336\263\022M\310C\273\213\246\037\003Z}\t8%\037]\324\313\374\226\365E;\023\r\211\n\034ۮ2 \232P\356@x6\375\022I2\366\236}IܭO\024\362D@f\320k\304\060\267\062;\241\"\366\"\221\235\341\213\037ڰʙ\002\271r\235I,\200~ř\325進\352\311\314S\277gֿ\024\326~-\334\216f\203\357WIa\377i\217a\315\321\036\235\234\026rr\346\035\360\204OJw\002\327\350\071,S\313\311\022\036\063t\236\f\364\325ԟԤY~5\317\062\"\364\314\317Ӑ-Hӏu\346\331\035*"..., dma_en = 0 '\000', bus_mode = 2 '\002', data_width_8or32bit = 1 '\001', espi_ddr_en = 0 '\000'}
(gdb) x/20x 0x20020ce0
0x20020ce0 <nor_test_buff+10240>:	0x84be2329	0xaed66ce1	0xf1499052	0xebe9bbf1
0x20020cf0 <nor_test_buff+10256>:	0x3cdba6b3	0x993e0c87	0x1c0d5e24	0xde47b706
0x20020d00 <nor_test_buff+10272>:	0xc84d12b3	0xa68bbb43	0x7d5a031f	0x1f253809
0x20020d10 <nor_test_buff+10288>:	0xfccbd45d	0x3b45f596	0x0a890d13	0x32aedb1c
0x20020d20 <nor_test_buff+10304>:	0xee509a20	0xfd367840	0xf6324912	0xdc497d9e
(gdb) p r
x$4 = {addr = 16, len = 256, 
  data = 0x2001e4e0 <nor_test_buff> "\t#\276\204\341l֮R\220I\361\361\273\351볦\333<\207\f>\231$^\r\034\006\267G\336\061$܄;\270\272a5\247Г\202Q\365ݿ\311oTS\261\060ءͺ\343\"\t\245\016\a\203o\321$\223/iԝ\312\324\361O$Dm\006\274C\vs#\272/b)\031\336\030\261\375\f\251\220+\227)Ԓ\a\354Y\235^\230\v.\234\305;\366}k\361M\342\335\310\346h>\365t\037\366\230\366\034\335\021\351\301g'.a\337\bD\247p-~\203\222\305<\221!\343\067I\340\317MI\375JE\227\343\\\363/L\314\375\071\002ԍ\367^m\221Ү\\\017\267\210\030t@\345", <incomplete sequence \365>, dma_en = 0 '\000', bus_mode = 2 '\002', data_width_8or32bit = 1 '\001', espi_ddr_en = 1 '\001'}
(gdb) x/20x 0x2001e4e0
0x2001e4e0 <nor_test_buff>:	0x84be2309	0xaed66ce1	0xf1499052	0xebe9bbf1
0x2001e4f0 <nor_test_buff+16>:	0x3cdba6b3	0x993e0c87	0x1c0d5e24	0xde47b706
0x2001e500 <nor_test_buff+32>:	0x84dc2431	0x61bab83b	0x93d0a735	0xddf55182
0x2001e510 <nor_test_buff+48>:	0x546fc9bf	0xd830b153	0xe3bacda1	0x0ea50922
0x2001e520 <nor_test_buff+64>:	0xd16f8307	0x692f9324	0xd4ca9dd4	0x44244ff1
(gdb) x/20x 0xaa001080
0xaa001080:	0x00000c00	0x00000002	0x00040201	0x00201008
0xaa001090:	0x01050611	0x00008080	0x00000008	0x00000008
0xaa0010a0:	0x00000008	0x00000008	0x00000008	0x00000008
0xaa0010b0:	0x00000000	0x00000000	0x00000000	0x00000000
0xaa0010c0:	0x00000000	0x00000000	0x00000000	0x00000000
```

```1
正确数据：
20 9a 50 ee 40 78 36 fd 12 49 32 f6 9e 7d 49 dc 
ad 4f 14 f2 44 40 66 d0 6b c4 30 b7 32 3b a1 22 
f6 22 91 9d e1 8b 1f da b0 ca 99 02 b9 72 9d 49 
2c 80 7e c5 99 d5 e9 80 b2 ea c9 cc 53 bf 67 d6



先写0地址，0x40个数据，数据内容是： 
5a 5a 5a 5a 5a 5a 5a 5a a5 a5 a5 a5 a5 a5 a5 a5 
5a 5a 5a 5a 5a 5a 5a 5a a5 a5 a5 a5 a5 a5 a5 a5 
5a 5a 5a 5a 5a 5a 5a 5a a5 a5 a5 a5 a5 a5 a5 a5 
5a 5a 5a 5a 5a 5a 5a 5a a5 a5 a5 a5 a5 a5 a5 a5  
然后读0地址，0x40个数据


5a 5a 5a 5a 5a 5a 5a 5a a5 a5 a5 a5 a5 a5 a5 a5 
5a 5a 5a 5a 5a 5a 5a 5a a5 a5 a5 a5 a5 a5 a5 a5 
5a 5a 5a 5a 5a 5a 5a 5a a5 a5 a5 a5 a5 a5 a5 a5 
5a 5a 5a 5a 5a 5a 5a 5a a5 a5 a5 a5 a5 a5 a5 a5

29 23 be 84 e1 6c d6 ae 52 90 49 f1 f1 bb e9 eb 
b3 a6 db 3c 87 0c 3e 99 24 5e 0d 1c 06 b7 47 de 
b3 12 4d c8 43 bb 8b a6 1f 03 5a 7d 09 38 25 1f 
5d d4 cb fc 96 f5 45 3b 13 0d 89 0a 1c db ae 32

master从0->30
   ratio1 = 0;ratio0 = 128;
   ratio1 = 128;ratio0 = 0;
   ratio1 = 0;ratio0 = 255;
   ratio1 = 255;ratio0 = 0;
   ratio1 = 128; ratio0 = 255;
   ratio1 = 255; ratio0 = 128;
   ratio1 = 128; ratio0 = 128;
   ratio1 = 255; ratio0 = 255;


正确数据：
6b 83 29 d5 e9 a8 d6 4d 0c 55 b3 81 25 b0 46 89 
04 cf 05 b7 b6 46 4c d2 9b d4 cf 09 95 6b c5 98 
8e f8 4d 83 a0 f2 d8 b7 a6 6a 18 98 00 e0 d0 3c 
68 fd ce 3a f9 74 12 49 df b0 27 f6 e1 48 b6 07

读到的数据n：
6b 83 29 d5 e9 a8 d6 4d 0c 55 b3 81 25 b0 46 89 
04 cf 05 b7 b6 46 4c d2 9b d4 cf 09 95 6b c5 98 
ef 84 d8 3a 0f 2d 8b 7a a1 89 80 0e 0d 03 c6 8f 
e3 af 97 41 24 9d fb 02 6e 14 8b 60 73 4f 9e b1

20/15/10/8/6/4/2:
ratio0:128 ratio1:128 master:2
ratio0:128 ratio1:128 master:4
ratio0:128 ratio1:128 master:6
ratio0:128 ratio1:128 master:10

29 23 be 84 e1 6c d6 ae 52 90 49 f1 f1 bb e9 eb 
b3 a6 db 3c 87 0c 3e 99 24 5e 0d 1c 06 b7 47 de 
31 24 dc 84 3b b8 ba 61 35 a7 d0 93 82 51 f5 dd 
bf c9 6f 54 53 b1 30 d8 a1 cd ba e3 22 09 a5 0e


ratio0:128 ratio1:128 master:20/18/16
22 92 3b e8 4e 16 cd 6a e5 29 04 9f 1f 1b be 9e 
bb 3a 6d b3 c8 70 c3 e9 92 45 e0 d1 c0 6b 74 7d 
b3 12 4d c8 43 bb 8b a6 03 5a 7d 09 38 25 1f 5d 
cb fc 96 f5 45 3b 13 0d 0a 1c db ae 32 20 9a 50

ratio0:128 ratio1:128 master:14
21 92 3b e8 4e 1c cf 6a e5 29 04 9d 1d 1b be 9e 
bb 3e 6f b3 c8 70 c3 e9 90 45 e0 d1 c0 6f 77 7f 
b3 32 4d c8 43 bb 8b e6 03 fa dd 09 38 25 1f dd 
cb dc b6 f5 45 3b 13 dd 0a cc fb ae 32 20 9a 50%

ratio0:128 ratio1:128 master:2

ratio0 = 255，1 = 0，dll_master_value配置1，延时0ns，读写数据全部出错
ratio0 = 128，1 = 0，dll_master_value配置3，延时2ns，读写数据偶尔出错，并且比延时4ns是出错频繁
ratio0，1配置128，dll_master_value配置5，延时4ns，cpu 16bit 32bit读写，随机读写测试通过
ratio0，1配置128，dll_master_value配置6，延时6ns，读写数据偶尔出错

dll0_delay  = ratio0 /256 * master
            = 255/256 * 1 = 0ns
            = 128/256 * 1 =  0ns
            = 128/256 * 2 = 1 * 2ns = 2ns
            = 128/256 * 3 = 1.5 * 2ns = 
            = 128/256 * 4 = 2 * 2ns = 4ns

# ratio0 输出的flash时钟进行延时:表现在 cs 拉低之后，clk 开始动作的延时
# ratio1 采数据的时候进行延时

# 修改 ratio0，逻辑分析仪抓到的波形一致，数据也是一样的(只有第一个字节从20 --> f0)
# 修改 ratio0 = 128，ratio1 = 0, master = 6，和什么都不配是一样的
# 修改 ratio0 = 0， ratio1 = 128, master = 6，MOSI 波形晚了 2.5ns，数据有一个字节变了
# 修改 ratio0 = 0， ratio1 = 128, master = 16，MOSI 波形晚了 2.5ns，数据有一个字节变了, 看寄存器 16 配进去了


(0*128)/256 = 0       0*2ns = 0ns       45ns  165ns 820ns
(6*128)/256 = 3       3*2ns = 6ns       50ns
(10*128)/256 = 5     5*2ns = 10ns       55ns  170ns 830ns
(20*128)/256 = 10   10*2ns = 20ns       65ns  170ns 
(18*128)/256 = 9     9*2ns = 18ns       65ns  170ns 

主核flash spi的测试，能不能增加一个dma xip和打开l2 icache进行交叉访问的case？

我理解的是：开启 l2 icache，进行 dma xip 的操作(用 dma 搬运 xip 的数据到内存)
但是这里有一个问题，dma 是不过 cache 的，cpu 访问 xip 地址才过 cache

那就分别做两个测试:

1. 开启 l2 icache，用 dma 搬运 xip 的数据到内存，验证数据是否正确
2. 开启 l2 icache，往 flash 里写一些数据，cpu 访问 xip 地址读数据，验证是否与写入的相同

这样就可以分别测试 dma 和 l2 icache 的交叉访问


# ddr 测试：

1. 读正确的一组配置：comtest 4100 时会出错

```c
	#if 1
			val |= 1 << 18;
			gx_hal_dw_writel(dws, 0x1080, 1<<11 | 1<<10);
			gx_hal_dw_writel(dws, 0x1090, 127<<8 | 1<<4 | 1);
			gx_hal_dw_writel(dws, 0x1094, 0<<8 | 4);
	#endif
```


# 4.22 测试情况：

* 读写命令方式:
** master = 0, ratio0 = 255, ratio1 = 0,延时 0ns, 读数据偶尔出错
** master = 3, ratio0 = 128, ratio1 = 0,延时 (128 * 3) / 256 = 1 * 2ns = 2ns, 读数据偶尔出错
** master = 5, ratio0 = 128, ratio1 = 0,延时 (128 * 5) / 256 = 2 * 2ns = 4ns, 读数据正确
** master = 7, ratio0 = 128, ratio1 = 0,延时 (128 * 7) / 256 = 3 * 2ns = 6ns, 读数据正确
** master = 9, ratio0 = 128, ratio1 = 0,延时 (128 * 9) / 256 = 4 * 2ns = 8ns, 读数据出错
** master = 11, ratio0 = 128, ratio1 = 0,延时 (128 * 11) / 256 = 5 * 2ns = 10ns, 读数据出错
* XIP 方式:
** master =  0, ratio0 = 128, ratio1 = 0,延时 0ns, 读数据出错
** master =  3, ratio0 = 128, ratio1 = 0,延时 (128 *  3) / 256 = 1 * 2ns =  2ns, 读数据出错
** master =  5, ratio0 = 128, ratio1 = 0,延时 (128 *  5) / 256 = 2 * 2ns =  4ns, 读数据正确
** master =  7, ratio0 = 128, ratio1 = 0,延时 (128 *  7) / 256 = 3 * 2ns =  6ns, 读数据正确
** master =  9, ratio0 = 128, ratio1 = 0,延时 (128 *  9) / 256 = 4 * 2ns =  8ns, 读数据出错
** master = 11, ratio0 = 128, ratio1 = 0,延时 (128 * 11) / 256 = 5 * 2ns = 10ns, 读数据出错


即将发布的 v3.3.0 版本中不会支持钰创的这两款 SPI Nand.

EM73D044VCS-H 这款存在两个问题：
1. main 区域写完数据后，关掉 ecc 读 oob 区域会读到错误的数据(非 0xff，且每次都是相同的数据)
2. 不关闭 ecc，进行读写擦测试失败

EM73C044VCG 没有给过样片

secure 核：

-> % riscv64-unknown-elf-gdb -x scripts/aquila/gdbinit.core3.secure gxscpu.elf
Reading symbols from gxscpu.elf...
0x00000000 in ?? ()
(gdb) load 
Loading section .stage1_sram_text, size 0x2b4 lma 0x10000000
        section progress: 100.0%, total progress:  0.60% 
Loading section .stage2_sram_text, size 0x1a0c8 lma 0x10002000
        section progress: 100.0%, total progress: 93.48% 
Loading section .stage2_sram_data, size 0x1d40 lma 0x1001c0c8
        section progress: 100.0%, total progress: 100.00% 
Start address 0x10000218, load size 114876
Transfer rate: 59 KB/sec, 3705 bytes/write.
(gdb) c
Continuing.
^C
Program received signal SIGINT, Interrupt.
0x10004da4 in hal_uart_readl (dev=0x1001d8ec <uart_config>, reg=20) at drivers/hal/src/uart/dw_uart/gx_hal_dw_uart.c:21
21	}
(gdb) x/20x 0x83200000
0x83200000:	0x00000000	0x00011111	0x00000000	0xffffffff
0x83200010:	0x00000000	0xffffffff	0x00000000	0xffffffff
0x83200020:	0x00000000	0xffffffff	0x00000000	0xffffffff
0x83200030:	0x00000000	0x00000000	0x00000000	0x00000000
0x83200040:	0x00000000	0x00000000	0x00000000	0x00000000


sensor 核:
-> % riscv64-unknown-elf-gdb -x scripts/aquila/gdbinit.core1.sensor gxscpu.elf
Reading symbols from gxscpu.elf...
0x00000002 in ?? ()
(gdb) x/20x 0x83200000
0x83200000:	0x83200000	0x83200004	0x83200008	0x8320000c
0x83200010:	0x83200010	0x83200014	0x83200018	0x8320001c
0x83200020:	0x83200020	0x83200024	0x83200028	0x8320002c
0x83200030:	0x83200030	0x83200034	0x83200038	0x8320003c
0x83200040:	0x83200040	0x83200044	0x83200048	0x8320004c
(gdb) x/20x 0x82000000
0x82000000:	0x82000000	0x82000004	0x82000008	0x8200000c
0x82000010:	0x82000010	0x82000014	0x82000018	0x8200001c
0x82000020:	0x82000020	0x82000024	0x82000028	0x8200002c
0x82000030:	0x82000030	0x82000034	0x82000038	0x8200003c
0x82000040:	0x82000040	0x82000044	0x82000048	0x8200004c



```



# IOPMP



- 访问 0x80 可以读到寄存器
- 访问 0x81 读到的全都是相同的数据
- 访问 0x82 读不到寄存器





代码修改：

```diff
--- a/common/main.c
+++ b/common/main.c
@@ -140,6 +140,70 @@ void loader_init(void)
 int main(int argc, char **argv)
 {
        loader_init();
+
+//#define SECURE
+
+#if 0
+#ifdef SECURE
+       volatile unsigned int *p = (void *)0x83200004;
+       *p = (1 << 16 | 1 << 12 | 1 << 8 | 1 << 4 | 1 << 0);
+
+       p = (void *)0x83200008;
+       *p = 0x0;
+
+       p = (void *)0x8320000c;
+       *p = 0xffffffff;
+
+       p = (void *)0x83200010;
+       *p = 0x0;
+
+       p = (void *)0x83200014;
+       *p = 0xffffffff;
+
+       p = (void *)0x83200018;
+       *p = 0x0;
+
+       p = (void *)0x8320001c;
+       *p = 0xffffffff;
+
+       p = (void *)0x83200020;
+       *p = 0x0;
+
+       p = (void *)0x83200024;
+       *p = 0xffffffff;
+
+       p = (void *)0x83200028;
+       *p = 0x0;
+
+       p = (void *)0x8320002c;
+       *p = 0xffffffff;
+
+       printf("iompm config finish.\n");
+#else
+       volatile unsigned int *ptr = (volatile unsigned int *)0x83200000;
+       int i;
+
+       for (i = 0; i < 24; i++) { // 每个寄存器占4个字节，所以只需要遍历24个地址
+               printf("Register at address 0x%08X contains: 0x%08X\n", (unsigned int)(ptr + i), *(ptr + i));
+       }
+
+       ptr = (volatile unsigned int *)0x82000000;
+       for (i = 0; i < 24; i++) { // 每个寄存器占4个字节，所以只需要遍历24个地址
+               printf("Register at address 0x%08X contains: 0x%08X\n", (unsigned int)(ptr + i), *(ptr + i));
+       }
+#endif
+#endif
+
```





# memcpy 7 个字节，后三个字节错误

- 开启 xip 预取，先 4 字节访问 xip，再单字节访问 xip

- 手册规定：在开启预取时 HSIZE 需要固定

- 开启 xip 的目的是为了加快效率，那么分析波形和测试读速度如下：

  - 分析波形：

    ![选区_479](image/选区_479.png)

  - 读速度：

    - 开预取：

      ```shell
      boot> nor_test xipspeed 2 0 0x1000
      init flash: done.
      id: ef4018, name: w25q128
      init xip
      
      [xip speed test result]
      test_time = 1339us
      test_len= 0x1000
      test_speed = 2987KB/s = 2.9169MB/S
      ```

    - 不开预取：

      ```shell
      boot> nor_test xipspeed 2 0 0x1000
      init flash: done.
      id: ef4018, name: w25q128
      init xip
      
      [xip speed test result]
      test_time = 2235us
      test_len= 0x1000
      test_speed = 1789KB/s = 1.7470MB/S
      ```













最近在测试 aquila flash spi 的 xip 时，开启预取功能之后使用 memcpy 拷贝 7 个字节数据到内存，发现前4个字节数据是对的，后3个字节数据是错的。关掉预取功能之后就正常了。

芯片回复：ip 在开启 xip 预取时使用是有限制的。<font color=red>**在预取模式下，HSIZE 应该保持恒定**。</font>memcpy 在拷贝的时候会先用 四字节的方式拷，然后用单字节拷，相当于修改了 HSIZE。

![微信图片_20240520152854](image/微信图片_20240520152854.png)

之所以要使用 xip 预取的功能是为了提高效率，所以测试了开启 xip 和关闭 xip 的读速度：

```
开预取：
boot> nor_test xipspeed 2 0 0x1000
init flash: done.
id: ef4018, name: w25q128
init xip

[xip speed test result]
test_time = 1339us
test_len= 0x1000
test_speed = 2987KB/s = 2.9169MB/S

不开预取：
boot> nor_test xipspeed 2 0 0x1000
init flash: done.
id: ef4018, name: w25q128
init xip

[xip speed test result]
test_time = 2235us
test_len= 0x1000
test_speed = 1789KB/s = 1.7470MB/S
```

然后看了波形，发现开预取的读间隔时间很短，而不开预取的读间隔时间较长。(无 L1 icache)

![选区_479](image/选区_479-1716190755359-4.png)



apus nre 上搭建环境测试也会出现相同的问题， 而之所以 apus nre 在使用的时候没有问题是因为 apus nre 的 L1 cache 是一定会开的，L1 cache 开了，所以每次读的数据都会是 cache line 对齐的(也就四字节对齐了，HSIZE 就一样)，所以才没有问题。而 aquila 的蓝牙核和 sensor 核、secure 核都没有 L1 cache，就会存在这样的问题。

芯片给出建议：在没有 L1 cache 的时候必须打开 L2 icache，这样就可以避免 HSIZE 改变的情况。

软件上对 xip 预取这个功能要怎么处理？规定使用场景必须是用相同的 HSIZE，或者在没有 L1 cache 的时候必须打开 L2 icache，或者关掉 xip 预取功能。

- 可以看下 XIP_DFS_HC 的功能
- 然后关掉 xip 连续读的时候，读4个字节、读1个字节，会是几次 cs 操作




# 安全核写 flash，主核读 xip，卡死

目前在测试 aquila flash、xip 的时候发现一个问题要咨询一下，安全核写 flash，主核读 xip，有两种情况：
1. 安全核挂掉，触发未定义指令异常
2. DebugServer 挂掉，重连 DebugServer 只看得到 bt 核
这样会挂掉的情况是正常的吗？






