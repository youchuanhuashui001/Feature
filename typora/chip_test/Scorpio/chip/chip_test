* DW_SPI Stage1 频率：198MHz, 4 分频(50MHz)
* DW_SPI Stage2 频率：198MHz, 4 分频(50MHz)
* GX_SPI Stage1 频率：118.8MHz, 6 分频(20MHz)
* GX_SPI Stage2 频率：118.8MHz, 6 分频(20MHz)

* 补丁：
** release
** develop


dw_spi 功能测试：(无四倍速、无内部 DMA)

* 11 号板
* Flash 型号： <notextile>W25Q128(NOR) (NAND)</notextile>
* 时钟及相位：SPI 模块时钟 198MHz，4 分频，相位 5
| 模式               | 位宽 | Nor Flash 测试结果| NAND Flash 测试结果|
| 发送单倍速         | 8bit | 通过              |                    |
| 接收单倍速         | 8bit | 通过              |                    |
| 接收单倍速         | 32bit| 通过              |                    |
| 接收双倍速         | 8bit | 通过              |                    |
| 接收双倍速         | 32bit| 通过              |                    |
| 分频配置           | 8bit | 通过              |                    |
| 输入时钟延时       | 8bit | 通过              |                    |
| 输入时钟上升沿采样 | 8bit | 通过              |                    |
| 输入时钟下降沿采样 | 8bit | 通过              |                    |
| 中断               | 8bit | 通过              |                    |

* 相位测试：
|模块频率    |模块分频|工作频率    |  相位范围               |
|/3. 198MHz  |  2     | 99 MHz     |    4,5,6                |
             |  4     | 50 MHz     |   2,3,4,5,6,7,8         |
             |  6     | 33 MHz     | 0,1,2,3,4,5,6,7,8,9,10  |

gx_spi 功能测试：

* 11 号板
* Flash 型号： <notextile>W25Q128(NOR) (NAND)</notextile>
* 时钟：SPI 模块时钟 118.8MHz， 6分频
| 模式       | Nor Flash 测试结果| NAND Flash 测试结果|
| 发送单倍速 |    通过           |                    |
| 接收单倍速 |    通过           |                    |
| 接收双倍速 |    通过           |                    |
| 接收双倍速 |    通过           |                    |
| 分频配置   |    通过           |                    |
| 中断       |    通过           |                    |

* gx_spi 打开中断后，能够进入中断服务函数，关闭中断后，不会进入中断服务函数


SPINOR 性能测试: DW

* SPI 模块频率：198MHz
* Flash 型号：W25Q128
* 分频：2分频 99MHz
** SAMPLE_DLY：5(可配范围：4~6)
|模式  |读取       |写入      |擦除      |
|两倍速|19.9248MB/S|0.6240MB/S|0.2841MB/S|
|单倍速|11.7988MB/S|0.6259MB/S|0.2822MB/S|

* 分频：4分频 50MHz
** SAMPLE_DLY：5(可配范围：2~8)
|模式  |读取       |写入      |擦除      |
|两倍速|11.7988MB/S|0.5917MB/S|0.2832MB/S|
|单倍速| 5.8994MB/S|0.5908MB/S|0.2822MB/S|

* 分频：6分频 33MHz
** SAMPLE_DLY：5(可配范围：0~10)
|模式  |读取       |写入      |擦除      |
|两倍速| 7.8662MB/S|0.5625MB/S|0.2802MB/S|
|单倍速| 3.9335MB/S|0.5625MB/S|0.2812MB/S|


SPINOR 性能测试: GX

* SPI 模块频率：118.8MHz
* Flash 型号：W25Q128
* 分频：2分频 60MHz
|模式  |读取       |写入      |擦除      |
|两倍速| 4.4414MB/S|0.5693MB/S|0.2792MB/S|
|单倍速| 3.3564MB/S|0.5683MB/S|0.2802MB/S|

* 分频：4分频 30MHz
|模式  |读取       |写入      |擦除      |
|两倍速| 3.3564MB/S|0.5234MB/S|0.2792MB/S|
|单倍速| 2.2539MB/S|0.5234MB/S|0.2792MB/S|

* 分频：6分频 20MHz
|模式  |读取       |写入      |擦除      |
|两倍速| 2.6972MB/S|0.4843MB/S|0.2802MB/S|
|单倍速| 1.6777MB/S|0.4843MB/S|0.2802MB/S|


* 还需要测试 ecos、linux 的读写擦、性能

ecos：

* 2 分频 99MHz
|模式  |读取       |写入      |擦除      |
|两倍速|20.3779MB/S|0.6230MB/S|0.3017MB/S|
|单倍速|12.0820MB/S|0.6230MB/S|0.2939MB/S|

* 4 分频 50MHz

|模式  |读取       |写入      |擦除      |
|两倍速|12.0820MB/S| 0.5908B/S|0.3076MB/S|
|单倍速| 6.0429MB/S| 0.5976B/S|0.2988MB/S|

linux:

* 4 分频 50MHz
** 双倍速
<pre>
[root@github gx]# dd if=/dev/mtd3 of=/dev/null bs=1024 count=1024
1024+0 records in
1024+0 records out
1048576 bytes (1.0MB) copied, 0.194606 seconds, 5.1MB/s

[root@github gx]# dd if=/dev/mtd3 of=/dev/null bs=1MB count=1
1+0 records in
1+0 records out
1000000 bytes (976.6KB) copied, 0.098975 seconds, 9.6MB/s
</pre>
** 单倍速起 kernel 时会出现 Receive FIFO Overflow
* 2 分频 99MHz
** 双倍速
<pre>
[root@github gx]# dd if=/dev/mtd3 of=/dev/null bs=1024 count=1024
1024+0 records in
1024+0 records out
1048576 bytes (1.0MB) copied, 0.155613 seconds, 6.4MB/s

[root@github gx]# dd if=/dev/mtd3 of=/dev/null bs=1MB count=1
1+0 records in
1+0 records out
1000000 bytes (976.6KB) copied, 0.066174 seconds, 14.4MB/s
</pre>
** 单倍速起 kernel 时会出现 Receive FIFO Overflow






SPINand 性能测试: DW

* SPI 模块频率：198MHz
* Flash 型号：GD5F1GM7UEYIG
* 分频：2分频 99MHz
** SAMPLE_DLY：5(可配范围：4~6)
|模式  |读取       | 写入      | 擦除      |
|两倍速|13.6279MB/S| 3.6591MB/S|20.4072MB/S|
|单倍速| 8.6669MB/S| 3.6591MB/S|20.4072MB/S|

* 分频：4分频 50MHz
** SAMPLE_DLY：5(可配范围：2~9)
|模式  |读取       | 写入      | 擦除      |
|两倍速| 8.5830MB/S| 2.7480MB/S|20.4599MB/S|
|单倍速| 4.9687MB/S| 2.7480MB/S|20.4599MB/S|

* 分频：6分频 33MHz
** SAMPLE_DLY：5(可配范围：0~11)
|模式  |读取       | 写入      | 擦除      |
|两倍速| 6.2597MB/S| 2.2050MB/S|20.4599MB/S|
|单倍速| 3.4853MB/S| 2.2050MB/S|20.4599MB/S|


SPINand 性能测试: GX

* SPI 模块频率：118.8MHz
* Flash 型号：GD5F1GM7UEYIG
* 分频：2分频 60MHz
|模式  |读取       | 写入      | 擦除      |
|两倍速| 3.9521MB/S| 2.2509MB/S|20.4072MB/S|
|单倍速| 3.0683MB/S| 2.2509MB/S|20.4599MB/S|

* 分频：4分频 30MHz
|模式  |读取       | 写入      | 擦除      |
|两倍速| 3.0615MB/S| 1.6669MB/S|20.4072MB/S|
|单倍速| 2.1171MB/S| 1.6660MB/S|20.4072MB/S|

* 分频：6分频 20MHz
|模式  |读取       | 写入      | 擦除      |
|两倍速| 2.5039MB/S| 1.3125MB/S|20.4599MB/S|
|单倍速| 1.6005MB/S| 1.3125MB/S|20.4072MB/S|


ecos 性能测试：DW

* 2 分频 99MHz
|模式  |读取       | 写入      | 擦除      |
|两倍速|13.5625MB/S| 3.7031MB/S|45.5107MB/S|
|单倍速| 8.6406MB/S| 3.7031MB/S|45.5107MB/S|

* 4 分频 50MHz

|模式  |读取       | 写入      | 擦除      |
|两倍速| 8.6044MB/S| 2.7841MB/S|45.5107MB/S|
|单倍速| 4.9951MB/S| 2.7861 B/S|45.5107MB/S|

* 读写擦 1000 次通过

linux 性能测试: DW

* 2 分频 99MHz
** 双倍速
<pre>
[root@gx6602 gx]# dd if=/dev/mtd3 of=/dev/null bs=1024 count=1024
1024+0 records in
1024+0 records out
1048576 bytes (1.0MB) copied, 0.238579 seconds, 4.2MB/s
[root@gx6602 gx]# dd if=/dev/mtd3 of=/dev/null bs=1MB count=1
1+0 records in
1+0 records out
1000000 bytes (976.6KB) copied, 0.127022 seconds, 7.5MB/s
</pre>
** 单倍速起 kernel 时会出现 Receive FIFO Overflow

* 4 分频 50MHz
** 双倍速
<pre>
[root@gx6602 ~]# dd if=/dev/mtd3 of=/dev/null bs=1024 count=1024
1024+0 records in
1024+0 records out
1048576 bytes (1.0MB) copied, 0.281659 seconds, 3.5MB/s
[root@gx6602 ~]# dd if=/dev/mtd3 of=/dev/null bs=1MB count=1
1+0 records in
1+0 records out
1000000 bytes (976.6KB) copied, 0.167698 seconds, 5.7MB/s
</pre>
** 单倍速起 kernel 时会出现 Receive FIFO Overflow

