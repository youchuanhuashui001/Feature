
#doing 
#备注：两个ir的问题等待芯片给出解决方法


# 测试项：
- 标准协议带载波发送
- 标准协议带载波发送 repeat
	- 依次发送 9012、NEC、RC5、RC6 协议波形，使用 Apus 板卡接收，测试正常。
```
Done(0),recv type: 9012 , data: 23456789
Done(0),recv type: 9012 , data: eeff1122
Done(0),recv type: 9012 , data: 1b2b3b4b
Done(0),recv type: 9012 , data: 90441c02
Done(0),recv type: NEC , data: 23456789
Done(0),recv type: NEC , data: eeff1122
Done(0),recv type: NEC , data: 1b2b3b4b
Done(0),recv type: NEC , data: 90441c02
Done(0),recv type: RC5 , data: 50009
Done(0),recv type: RC5 , data: 1f0022
Done(0),recv type: RC5 , data: b000b
Done(0),recv type: RC5 , data: 40002
Done(0),recv type: RC6 , data: 450089
Done(0),recv type: RC6 , data: ff0022
Done(0),recv type: RC6 , data: 2b004b
Done(0),recv type: RC6 , data: 440002

Done(10),recv type: 9012 REPEAT, data: 23456789
Done(10),recv type: NEC REPEAT, data: 23456789
Done(10),recv type: RC5 REPEAT, data: 50009
Done(10),recv type: RC6 REPEAT, data: 450089
```
- 9012
![[Screenshot_2025-08-14-13-33-43.png]]

repeat：
![[Screenshot_2025-08-14-13-39-14 1.png]]



- NEC
![[Screenshot_2025-08-14-13-36-36.png]]

repeat
![[Screenshot_2025-08-14-13-39-41 1.png]]



- RC5
![[Screenshot_2025-08-14-13-37-17.png]]

repeat
![[Screenshot_2025-08-14-13-39-56.png]]



- RC6
![[Screenshot_2025-08-14-13-37-25.png]]


repeat

![[Screenshot_2025-08-14-13-40-08.png]]




- 标准协议带载波接收
- 标准协议带载波接收 repeat
- 自动识别接收的协议类型
	- 使用不同的协议 NEC、9012、RC5、RC6 依次在遥控器上按下按键1~9，再依次按下 repeat，能够正常接收数据、repeat、识别协议。
```
Done(0),recv type: NEC , data: bf40fe01
Done(0),recv type: NEC , data: bf40fd02
Done(0),recv type: NEC , data: bf40fc03
Done(0),recv type: NEC , data: bf40fb04
Done(0),recv type: NEC , data: bf40fa05
Done(0),recv type: NEC , data: bf40f906
Done(0),recv type: NEC , data: bf40f807
Done(0),recv type: NEC , data: bf40f708
Done(0),recv type: NEC , data: bf40f609

Done(0),recv type: NEC , data: bf40fe01
Done(10),recv type: NEC REPEAT, data: bf40fe01
Done(10),recv type: NEC REPEAT, data: bf40fe01
Done(10),recv type: NEC REPEAT, data: bf40fe01
Done(10),recv type: NEC REPEAT, data: bf40fe01
Done(10),recv type: NEC REPEAT, data: bf40fe01
Done(10),recv type: NEC REPEAT, data: bf40fe01
Done(10),recv type: NEC REPEAT, data: bf40fe01
Done(10),recv type: NEC REPEAT, data: bf40fe01
Done(10),recv type: NEC REPEAT, data: bf40fe01
Done(10),recv type: NEC REPEAT, data: bf40fe01
Done(0),recv type: NEC , data: bf40fd02
Done(10),recv type: NEC REPEAT, data: bf40fd02
Done(10),recv type: NEC REPEAT, data: bf40fd02
Done(10),recv type: NEC REPEAT, data: bf40fd02
Done(10),recv type: NEC REPEAT, data: bf40fd02
Done(10),recv type: NEC REPEAT, data: bf40fd02
Done(10),recv type: NEC REPEAT, data: bf40fd02


Done(0),recv type: RC5 , data: 50001
Done(0),recv type: RC5 , data: d0002
Done(0),recv type: RC5 , data: 50003
Done(0),recv type: RC5 , data: d0004
Done(0),recv type: RC5 , data: 50005
Done(0),recv type: RC5 , data: d0006
Done(0),recv type: RC5 , data: 50007
Done(0),recv type: RC5 , data: d0008
Done(0),recv type: RC5 , data: 50009
Done(0),recv type: RC5 , data: 50001
Done(10),recv type: RC5 REPEAT, data: 50001
Done(10),recv type: RC5 REPEAT, data: 50001
Done(0),recv type: RC5 , data: d0002
Done(10),recv type: RC5 REPEAT, data: d0002
Done(10),recv type: RC5 REPEAT, data: d0002
Done(10),recv type: RC5 REPEAT, data: d0002
Done(10),recv type: RC5 REPEAT, data: d0002
Done(10),recv type: RC5 REPEAT, data: d0002


Done(0),recv type: 9012 , data: e0eff00
Done(0),recv type: 9012 , data: e0efe01
Done(0),recv type: 9012 , data: e0efd02
Done(0),recv type: 9012 , data: e0efc03
Done(0),recv type: 9012 , data: e0efb04
Done(0),recv type: 9012 , data: e0efa05
Done(0),recv type: 9012 , data: e0ef906
Done(0),recv type: 9012 , data: e0ef807
Done(0),recv type: 9012 , data: e0ef708
Done(0),recv type: 9012 , data: e0eff00
Done(10),recv type: 9012 REPEAT, data: e0eff00
Done(10),recv type: 9012 REPEAT, data: e0eff00
Done(10),recv type: 9012 REPEAT, data: e0eff00
Done(10),recv type: 9012 REPEAT, data: e0eff00
Done(0),recv type: 9012 , data: e0efe01
Done(10),recv type: 9012 REPEAT, data: e0efe01
Done(10),recv type: 9012 REPEAT, data: e0efe01
Done(10),recv type: 9012 REPEAT, data: e0efe01
Done(0),recv type: 9012 , data: e0efd02
Done(10),recv type: 9012 REPEAT, data: e0efd02

Done(0),recv type: RC6 , data: 1
Done(0),recv type: RC6 , data: 2
Done(0),recv type: RC6 , data: 3
Done(0),recv type: RC6 , data: 4
Done(0),recv type: RC6 , data: 5
Done(0),recv type: RC6 , data: 6
Done(0),recv type: RC6 , data: 7
Done(0),recv type: RC6 , data: 8
Done(0),recv type: RC6 , data: 9
Done(0),recv type: RC6 , data: 1
Done(10),recv type: RC6 REPEAT, data: 1
Done(10),recv type: RC6 REPEAT, data: 1
Done(10),recv type: RC6 REPEAT, data: 1
Done(0),recv type: RC6 , data: 2
Done(10),recv type: RC6 REPEAT, data: 2
Done(10),recv type: RC6 REPEAT, data: 2
Done(0),recv type: RC6 , data: 3
Done(10),recv type: RC6 REPEAT, data: 3


```

- 长按左上角的 SET，然后按下 OK
- 输入以下对应 code，切换到对应协议后按下 EXIT 来退出
```
万能遥控器编码TV模式 
5008为RC5协议
2246为9012协议
2290为NEC协议
0020 RC6协议
```




- 发送载波频率及占空比
	- 遥控器发送 NEC 协议波形，解析出来为 37.04Khz，占空比为 15%，偏差较大

- 中断触发
	* 使用中断模式接收标准协议，中断可正常触发接收。
	* 使用中断模式发射标准协议，中断可正常触发发射。
	* 使用中断模式接收自由模式数据，中断可正常触发接收，FIFO空中断可正常上报。
	* 使用中断模式发送自由模式数据，发送结束中断可正常触发，发送阈值中断可正常上报。


- 带载波自由模式发送


- 带载波自由模式接收

- 不带载波标准协议发送

- 不带载波标准协议发送 repeat

- 不带载波自由模式发送

- 接收载波频率和占空比
	- 遥控器配置为 NEC 协议，按下 2 键，Sagitta 使用自由模式接收，计算得到的频率为 37.04k，占空比为 12%，误差较大。
```
boot> gx_ir recv_ud_carr_it
gx_ir_context_src_freq=8000000
ud rx done.
0:data 5460384, h 9000 us, l 4500 us
1:data 6d0038, h 560 us, l 530 us
2:data 6d0038, h 560 us, l 530 us
3:data 6c0038, h 560 us, l 520 us
4:data 6d0039, h 570 us, l 520 us
5:data 6d0038, h 560 us, l 530 us
6:data 6c0038, h 560 us, l 520 us
7:data df0039, h 570 us, l 1660 us
8:data 6e0038, h 560 us, l 540 us
9:data de0038, h 560 us, l 1660 us
10:data df0039, h 570 us, l 1660 us
11:data de0038, h 560 us, l 1660 us
12:data de0038, h 560 us, l 1660 us
13:data df0038, h 560 us, l 1670 us
14:data de0038, h 560 us, l 1660 us
15:data 6d0038, h 560 us, l 530 us
16:data de0038, h 560 us, l 1660 us
17:data 6d0038, h 560 us, l 530 us
18:data de0038, h 560 us, l 1660 us
19:data 6d0038, h 560 us, l 530 us
20:data 6c0038, h 560 us, l 520 us
21:data 6d0039, h 570 us, l 520 us
22:data 6d0038, h 560 us, l 530 us
23:data 6d0038, h 560 us, l 530 us
24:data 6c0038, h 560 us, l 520 us
25:data df0039, h 570 us, l 1660 us
26:data 6c0038, h 560 us, l 520 us
27:data df0039, h 570 us, l 1660 us
28:data de0038, h 560 us, l 1660 us
29:data de0038, h 560 us, l 1660 us
30:data de0038, h 560 us, l 1660 us
31:data df0039, h 570 us, l 1660 us
32:data e00038, h 560 us, l 1680 us
33:data 3a990038, h 560 us, l 149450 us
carr=d80019
carry freq: 37.04KHz, duty:12%
```



- 标准模式空闲电平


# 问题
## 【已解决】通路不通
- 配置 P2_0 为发射脚，P4_3 为接收脚，工作频率配置为 8MHz
- recv_ud 的时候立马就有了数据，之后再也收不到数据
### 【已解决】send_std 通过外围电路发射，看起来波形是正常的，带32k的载波
![[b720f81dd023a88bd9ac930187a0f87d.jpg]]


- 电压看起来只有 2v，量红外发射管两端，一段有很小的电压，一段没有波形
![[20f775aee03c43fa6e24cebb09497a6c.jpg]]
- 把驱动强度配到最大，能够拉到 3.3v 了，但是量红外管脚上的电压，这里有问题，空闲的时候应该是高电平吧？
	- 这里空闲的时候是低电平，并且在三极管导通之后，从0v拉低到了-0.3v 左右。
	- fpga 阶段的波形是空闲时为高电平，三极管导通时会把电压从3.3v拉到0v左右。
	- fornax 的时候量过，红外管的阳极在未导通时是高电平，导通后会有0.7v左右的压降；阴极在未导通时也是高电平，导通后会到0v左右。
- 原因：红外管焊反了，重新焊接后发射正常。








### 【已解决】recv_ud：遥控器发射 NEC 协议，量接收脚上的波形不带载波，按道理说这里只过了红外发射管，不会把载波滤掉吧？
![[699e75d6e6e029a6a5ea1046ac322910.jpg]]

- 错误，这里是因为红外管焊反了。

- 测试发现：
	-  IRC_TEST 配成 0x3，IRC_ANA 配成 0x1107
	- ud 模式没有阈值中断上来，std 模式没有 rx done 中断上来
	- 测量 IR_RX 脚有波形
![[895a3818917083786065f98a2b834452.jpg]]

使用 systemi2c 进入测试模式查看接收时的模拟部分是否正常
- 按照问题号 #397596 以及 sysi2c 目录下的 README 开始使用
- write、read 寄存器之前需要先执行 enable_test_mode 0
- 寄存器修改后，P1_4 引脚上没有波形

- 芯片建议修改 0x3C 的配置：
	- 0x104
		- vcm = 1
		- isel = 0
		- ana_rx_en = 1
	- 0x12c
		- vcm = 3
		- isel = 1
		- ana_rx_en = 1
	- 0x312c
		- vcm = 3
		- isel = 1


- 添加前面的去除漏电配置
```
输入命令: write 0xb5000038 4 0xf0000055
写入完成

输入命令: write 0xb500003c 4 0x410b0000
写入完成

输入命令: write 0xb5000040 4 0x11001701
写入完成

输入命令: write 0xb5000044 4 0x12000E31
写入完成

输入命令: write 0xb0910030 4 0x1
写入完成

// 配成 1 或 0
输入命令: write 0x0000009c 4 0x1
写入完成

// 配成 0x48 或 0x58 或 0x18
输入命令: write 0x000000a0 4 0x58
写入完成

输入命令: write 0xb5000034 4 0x300
写入完成

输入命令: write 0xb040003c 4 0x128

```


- 上面 P1_4 不出波形和P4_3 收不到数据的问题是，**需要把这两个脚配置成模拟脚，配置完成后 P1_4 可以输出模拟波形，P4_3 收得到阈值中断和数据，但是数据有问题。**
	- NEC 协议发出：0000 0010 1111 1101  0100 0000 1011 1111 1
	-                           0        2        f      d       4        0       b      f
	- 模拟波形看起来是对的，但是最后多了一个 1，软件从 fifo 中取出来的数据不对
- 发现问题原因了：接收时不能配置反相，修改后正常。
- **内部数字要求无信号时为低电平，如果模拟给数字的信号，IDLE时是高电平，那就需要反相**



## 【已解决】板子奇怪的的地方，每次焊了东西之后，都需要重新烧一遍程序，否则会不太正常
- 这里好像是因为 USB33，新的补丁已经解决了这个问题，需要把 P4_3 配成1，也就是 IRRX 的功能


## 测试模式
进入测试流程，执行下面的sysi2c指令，并且需要配置 P1_4 为模拟引脚(0xb0730130 bit14 = 1)：  
```

-> % sudo -E python3 sysi2c.py /dev/ch34x_pis2 sagitta -s 400
Open device /dev/ch34x_pis2 succeed, fd: 3
Driver version: V1.5 On 2024.12
Chip version: 0x30
Vendor ID: 0x1a86, Product ID: 0x5512
Device operating has function [SPI+I2C+PARALLEL+GPIO].
成功打开设备: /dev/ch34x_pis2
当前芯片: sagitta
I2C速度设置为: 400kHz

输入命令: enable_test_mode 0
test sagitta
已启用测试模式: 0x00

输入命令: write 0xb0910030 4 1
写入完成

输入命令: write 0x0000009c 4 1
写入完成

输入命令: write 0x000000a0 4 0x18
写入完成

输入命令: write 0xb5000034 4 0x300
写入完成

输入命令: write 0xb040003c 4 0x128
写入完成
```


## 获取载波频率和占空比误差比较大
- 4个载波周期，配成 3
```
boot> gx_ir recv_ud_carr_it
gx_ir_context_src_freq=8000000
ud rx done.
0:data db00d7, h 2150 us, l 40 us
1:data a0004, h 40 us, l 60 us
2:data aa007f, h 1270 us, l 430 us
3:data 60005, h 50 us, l 10 us
4:data 970060, h 960 us, l 550 us
5:data 61005e, h 940 us, l 30 us
6:data 410008, h 80 us, l 570 us
7:data 910053, h 830 us, l 620 us
8:data 1e70026, h 380 us, l 4490 us
9:data 6d0039, h 570 us, l 520 us
10:data 6d0038, h 560 us, l 530 us
11:data 6d0038, h 560 us, l 530 us
12:data 6c0038, h 560 us, l 520 us
13:data 6d0038, h 560 us, l 530 us
14:data 6d0038, h 560 us, l 530 us
15:data de0038, h 560 us, l 1660 us
16:data 6e0038, h 560 us, l 540 us
17:data df0039, h 570 us, l 1660 us
18:data de0038, h 560 us, l 1660 us
19:data de0038, h 560 us, l 1660 us
20:data df0038, h 560 us, l 1670 us
21:data de0038, h 560 us, l 1660 us
22:data de0038, h 560 us, l 1660 us
23:data 6d0038, h 560 us, l 530 us
24:data de0038, h 560 us, l 1660 us
25:data 6d0038, h 560 us, l 530 us
26:data de0038, h 560 us, l 1660 us
27:data 6d0038, h 560 us, l 530 us
28:data 6d0038, h 560 us, l 530 us
29:data 6d0038, h 560 us, l 530 us
30:data 6c0038, h 560 us, l 520 us
31:data 6d0038, h 560 us, l 530 us
32:data 6d0038, h 560 us, l 530 us
33:data de0038, h 560 us, l 1660 us
34:data 6d0038, h 560 us, l 530 us
35:data de0038, h 560 us, l 1660 us
36:data de0038, h 560 us, l 1660 us
37:data de0038, h 560 us, l 1660 us
38:data df0039, h 570 us, l 1660 us
39:data de0038, h 560 us, l 1660 us
40:data e00038, h 560 us, l 1680 us
41:data 3a990038, h 560 us, l 149450 us
carr=d80008
carry freq: 37.04KHz, duty:4%
```

## Apus发射，Sagitta 接收不到，遥控器发射，Sagitta 可以正常接收
