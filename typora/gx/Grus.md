# Grus

- GPIO模块
1. 每个GPIO模块控制32个IO口

2. 寄存器GPIO_MASK可以屏蔽对选定IO的输入输出操作，即不能设置输入输出，不能设置输出值，读到的管脚值始终是0

3. 寄存器中的gpio_dir：每一个代表一个io口

4. PWM发生器
   
   1. 8路16位PWM发生器
   2. 每路可分配至32个IO中的任意一个
   3. 每路周期独立配置
   4. 各路公用一个预分频器

5. 中断
   
   1. 每个IO都支持中断
   2. 中断类型有：H、L、L2H、H2L

6. 工作模式
   
   1. 普通IO和PWM，每个IO的工作模式都有单独的bit控制`GPIO_USE_PWM_PWMx`
   2. 普通IO模式可以选择作为输入输出
   3. PWM模式有寄存器选择某个IO输出哪路PWM
   4. 八路PWM共用一个预分频器，由`GPIO_PWM_PREDIV`配置，每路PWM可以单独配置周期`GPIO_PWM_CYCx`和占空比`GPIO_PWM_DOUTYx`
      1. 周期由寄存器GPIO_PWM_CYCx配置，`GPIO_PWM_DUTYx.PWMx_POS：PWM高电平起始位置`，`GPIO_PWM_DUTYx.PWMx_NEG：PWM高电平结束位置`

7. 中断
   
   1. 在IO模式和PWM模式均可以产生中断
   2. 配置寄存器`GPIO_INIT_HI_EN`等配置成对应的中断(H、L、H2L、L2H)
- gx8002 芯片的GPIO模块有一组，总共32个，其中每个GPIO还提供了管脚复用来使用其它功能

- #define readl(addr) ({ unsigned int __v = (*(volatile unsigned int *) (addr)); __v; }) ：从addr的地址开始读取long长度的数据，把addr强转成volatile unsinged int * 然后取值，并返回这个值

- #define writel(b,addr) (void)((*(volatile unsigned int *) (addr)) = (b)) ：从addr的地址开始写unsigned int 长度的数据b，把addr强转成volatile unsigned int *然后把b赋给\*addr

- 每个IO口均支持中断

- gpio.callback是一个函数指针，在要使用的时候通过回调函数的方式来调用中断服务函数

- 在选择输出通道时，可选pwm通道和时钟源

- 使用strtoul函数实现将命令行输入的字符串转换成unsigned long形式

- 输出pwm：例如`gx_gpio_enable_pwm(i, 1000, 500);`将周期设为1000，占空比设为500，gpio的时钟频率首先进行1000的分频，然后进行500的分频决定最终的输出频率，占空比为：500/1000-->50%

- 疑惑：gpio.callback 是一个函数指针，应该在使用的时候指向一个函数
  
  - 也就是当中断产生，这个指针会指向对应的中断服务函数

- gpio pwm映射以及通道(pwm只有两个通道)

- gpio周期和占空比的命名中ns是代表什么?

- 时钟树：gpio的时钟是24.576MHz(25MHz)和最高频率时钟

# I2C

- i2c是串行、同步、半双工的通信方式，通信只需要一根scl线和一跟sda线

- gx8002开发板上有两个i2c硬件接口，当硬件接口缺少时，可以使用gpio模拟i2c

- gpio模拟i2c不支持异步读写

- dw_readl也是调用readl宏来读取数据的

- 包含了tx和rx的fifo缓冲区

- i2c支持dma传输， 并且在dma传输时要先进行配置数据源和数据目标

- 在函数 `static void i2c_dw_set_dma_rx_slave(struct dw_i2c_dev *dev)`中对dev进行dma的src和dst进行结构体指针的方式来配置

- 用回调函数的方式将寄存器地址和寄存器地址(数据+数据长度)和数据组合到一起作为数据发送
  
  ```c
                            要传输的寄存器地址  寄存器地址的字节数 存放临时数据
  status = gx_set_reg_address_buf(reg_address, address_width, buf);
      if (status < 0)
          return -1;
                  组合到buf中        要传输的数据
      memcpy(&buf[address_width], tx_data, len);
                                     将组合起来的数据通过i2c发出去
      status =  gx_i2c_write(bus_id, slv_addr, buf, address_width + len);
  ```

- 时钟延展：由于主机和从机的速率不相同，可能存在主机发送读命令后，从机还未准备好数据，就会将主机的scl拉低来等待从机的数据准备好，准备好后再进行下一次的数据传输

- GDB：运行之前会加载程序的指令，然后gdb将接管cpu的运行，由gdb来把指令一条一条的送给cpu来执行，例如：打断点-->相当于在程序的相应位置指令换成cpu运行一条汇编即会停止的指令

- 数据传输必须带响应。相关的响应时钟脉冲由主机产生，在响应的时钟脉冲期间，发送器
  释放SDA线(H),在响应的时钟脉冲期间始终脉冲期间，接收器必须将SDA线拉低，使它在这
  个时钟脉冲的高电平期间保持稳定的低电平

- 产生的同步SCL时钟的`低电平周期由低电平始终周期最长的器件决定`，而`高电平周期由高
  电平时钟周期最短的期间决定`

- 起始条件后面立即跟着一个停止条件(报文为空)是一个不合法的格式

- i2c总线的寻址过程是通过在起始条件后的第一个字节决定主机选择哪一个从机

- 广播呼叫地址是用来寻址连接到i2c总线上的每个器件。但是，如果器件在广播呼叫结构
  中不需要任何数据，可以通过不发出响应来忽略这个地址。如果器件要求从广播呼叫地址
  得到数据，它会响应这个地址并作为从机-接收器工作。

- 10位寻址的格式：当起始条件后有10位地址时，每个从机将从机地址第一个字节的头七位
  (11110xx)与自己的地址比较，并测试看第8位(R/W方向位)是否为0.此时，很可能超过一
  个器件发现地址相同，并产生一个响应(A1)。所有发现地址相同的从机地址第2个字节的8位
  (xxxxxxxx)与自己的地址比较，此时只有一个主机发现地址相同并产生一个响应(A2).匹配
  的从机将保持被主机寻址，直到接收到停止条件(P)或从机地址不同的重复起始条件(Sr)

issue:

- 对与i2c的理解和基本使用(at24c02为例)

- cmd line的使用：
  
  - 把底层的接口抽象出来了
  - set speed：dw_i2c_info 中的hight_time、low_time代表什么
  - 模拟gpio的打开方式
    - 提示未使用config、进行config后报程序多重定义(应该是用法错误)
      - 要关闭掉硬件i2c的配置
    - 模拟i2c不需要进行复用，任意两个io口即可，因为是使用的gpio端口来模拟i2c的波形
    - 硬件i2c需要提前在pinmux中配置好复用为i2c模式，然后去使用
  - 硬件i2c的命令发送：要跟器件通信怎么使用
    - 后面的addr 和 addr_width都是对数据抽象
  - 两机通信的步骤：
    - 主机发送器件地址，从机设置好自己的期间地址，然后监听总线上是否有与自己器件地址相同的地址，相同则通信成功
    - 先使用master，配置成no ask查看波形是否正确
  - i2c_master 0x30 write 0x01 8 0x55
  - i2c_master 0x30 write 0x2345 2 0x33 0x34

```c
i2c_master使用方法：
    1. make menuconfig的cmd line和device and drivers中打开i2c master和i2c slave模式
    2. 在boot cmd line ：i2c_master slv_addr write addr addr_width <data>
                                    从机地址   读/写 要发的数据  数据占几字节 要发送的data(可以发送多个 程序会根据命令行的长度计算出data的长度)
    3.     addr --> argv[3]
        address_width --> argv[4]
        buf --> argv[5 + i]
        len --> argc - 5
       4. argv[i]的值作为func-->gx_i2c_tx(回调函数)的参数
    5. gx_i2c_tx --> gx_set_reg_address_buf：将addr和data组合成buf --> gx_i2c_write(bus_id, slv_addr, buf, address_width + len)
    6. gx_i2c_write --> gx_i2c_open(bus_id)：打开时钟和进行一些配置，将slv_addr,buf,address_width+len组合成一个msg --> gx_i2c_transger(i2c_dev, &msgs, 1)：通过open打开i2c设备获取文件句柄，根据文件句柄传输msg，传输完成后gx_i2c_close(i2c_dev)
    7. gx_i2c_transger --> 根据传入的i2c_dev判断同步/异步传输，调用i2c_dw_async_xfer/i2c_dw_xfer(i2c_dev, msgs, num) ：进行数据的传输
    8. i2c_dw_xfer --> i2c_dw_xfer_prepare(dev, msgs, num):配置i2c设备的使用，msgs，msgs_num,cmd_err,msg_write_idx,status等 --> 等待i2c总线空闲，然后进行硬件的配置(配置是否为10bit格式，清除中断，打开i2c设备)
    9. i2c_dw_xfer 调用prepare完成了准备工作后就一直循环检测当前的中断标志位，当前发送缓冲区为空则传输数据

   i2c_slave使用方法：
    1. bus_id slv_addr --> 通过命令行传入bus_id和slv_addr
    2. 调用gx_i2c_slave_open(bus, slv_addr, i2c_slave_cb, &priv)：设置i2c设备和从机地址，并调用回调函数，priv作为回调函数参数传入
    3. i2c_slave_cb回调函数中读取发生的中断事件：I2C_SLAVE_STOP(检测到STOP信号)、I2C_SLAVE_REQUESTED_DATA（检测到读请求）、I2C_SLAVE_RECEIVE_DATA(检测到写请求)，分别在各个事件中完成想要的功能
```

```c
# i2c主从机通信：

## i2c_slave 0 0x33    设置从机i2c0地址为0x33
## i2c_master bus 0    设置主机使用i2c0
## i2c_master 0x33 write 0x112233 3 0x44 0x55 0x66    发送六个字节
## i2c_master 0x33 read 0x12 1 4    从0x12处读取4个字节
```

# RTC

1. 使用DW_RTC
   
   1. get_tick ：获取当前时钟计数器 操作硬件寄存器
   2. set_tick：设置当前时钟计数器
   3. set_alarm：设置闹钟  --> 要先设置中断函数，再设置闹钟，闹钟时间到则发生中断

2. 使用GX_RTC
   
   1. 编译失败，缺少宏SCPU_APB2_CLOCK
   2. grep SCPU_APB2_CLOCK -rn：查找在grus的board_config.h中未定义该宏

3. 有些rtc的寄存器是以bcd码或不是bcd码表示的转换方法：
   
   ```c
   /* bcd码转10进制 */
   unsigned char bcd2bin(unsigned char bcd)
   {
       return ((bcd >> 4)*10 + (bcd & 0x0F));
   }
   
   /* 非bcd码转10进制 */
   int bcd4bin(int val)
   {
       u8 tmp[4];
       tmp[0] = (val & 0xf000) >> 12;
       tmp[1] = (val & 0x0f00) >> 8;
       tmp[2] = (val & 0x00f0) >> 4;
       tmp[3] = val & 0x000f;
       return tmp[0] * 1000 + tmp[1] * 100 + tmp[2] * 10 + tmp[3];
   }
   ```

# SPI

1. 4线SPI器件四个信号
   
   1. 时钟 --> SPI CLK , SCLK
   2. 片选 --> CS
   3. 主机输出、从机输入 --> MOSI
   4. 主机输入、从机输出 --> MISO

2. 产生时钟信号的器件称为主机。主机和主机之间传输的数据与主机产生的始终同步

3. 要开始SPI通信，主机必须发送时钟信号，并通过使能CS信号选择从机。片选信号通常是低电平有效信号

4. SIP是全双工接口，主机和从机可以通过MOSI和MISO线同时发送数据和接受数据

5. SPI接口允许用户灵活选择时钟的上升沿或下降沿来采样或移位数据

6. 时钟极性和相位
   
   1. CPOL：在空闲状态期间，设置时钟的极性。CPOL=0：SCK在空闲状态时为0。CPOL：SCK在空闲状态时为1。
   2. CPHA：选择时钟相位，根据CPHA位的状态使用时钟上升沿或下降沿来采样或移位数据。CPHA=0：在SCK第一个边沿时输入输出数据有效。CPHA=1：在SCK第二个边沿时输入输出数据有效。
   
   ![](/home/tanxzh/txz/my/note/image/445456.png)

![](/home/tanxzh/txz/my/note/image/菊花链.png)

4. 菊花链数据传输：所有从机的片选信号连接在一起，数据从第一个从机传输到下一个从机。在此配置中，所有从机同时接受同一个SPI时钟，来自主机的数据直接送到第一个从机，该从机将数据提供给下一个从机。

![](/home/tanxzh/txz/my/note/image/菊花链数据传播.png)

1. 写法：在struct spi_transfer中unsigned cs_change:1;
2. grus只有一个SPI Master接口
3. `spi 0:3.1 2 3322`：一直卡在`static void sflash_scan_boardinfo(struct sflash_master *master)`函数中的list_for_each循环
