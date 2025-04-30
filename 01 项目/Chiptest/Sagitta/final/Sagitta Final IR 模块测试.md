# 问题 

## 【已解决】问题 1：收不到数据，使用老的 2-26 号的 bit 可以收到数据
用老的 bit 发数据，机顶盒板子也收不到数据
分别查看发送波形：
- 2-26 号 bit 的发送波形：
![[f79b84596c780d2e1676f783ccbfa3d8.jpg]]

- 4-18b bit 的发送波形：
![[af1c270d08cabf7989dc4f61d5264c37.jpg]]
![[68d22080721f3386f91d83dc5580aeb6.jpg]]

- 芯片将 IR_REC2 (IR_EN) 引脚拉给了 boot 按键
- 重新出的 bit 将 IR_REC2 放到了 p2_0，可以发送数据，但是感觉很难才能发对一次，看波形是没啥问题的

## 【已解决】问题 2：NEC 协议带载波接收，红外遥控器需要按许多次才会正确读到数据 
目前测试 IR 现象是正常的，但是不响应的次数太多了，比如按个 5 次或者更多次才有一次中断上报。
红外遥控器按下按键之后，有中断上报后主要是这两种情况：
- rx_done 产生了，此时正确解析到数据，正确解析了协议
- rx_done 没产生，看 INT_STA 寄存器没有正确解析协议

咨询一下红外的一个问题，标准模式接收，没开中断，红外遥控器发 NEC 协议的时候，要按 5 次或者更多次遥控器的按键才能有一次中断上报，并且上报上来的中断许多时候没有正确解析到协议。

开中断的模式：其实每次按键按下都收到了，但是被认为是 time out，读 INT_STA 寄存器经测试 timeout
```
boot> std rx tov
time out!
data:bf40fd02, type:NEC.
std rx tov
time out!
det err
port_det_error!
det err
port_det_error!
det err
port_det_error!
det err
port_det_error!
det err
port_det_error!
std rx tov
time out!
std rx tov
time out!
det err
port_det_error!
std rx tov
time out!
std rx tov
time out!
std rx tov
time out!
std rx tov
time out!
std rx tov
time out!
std rx tov
time out!
std rx tov
time out!
std rx tov
time out!
std rx tov
time out!
data:bf40fd02, type:NEC.
data:bf40fd02, type:NEC.
data:bf40fd02, type:NEC.
data:bf40fd02, type:NEC.
data:bf40fd02, type:NEC.
data:bf40fd02, type:NEC.
data:bf40fd02, type:NEC.
data:bf40fd02, type:NEC.
```


nec 有一个 108ms 的保护机制：在第一次按下按键之后的 108ms 如果再次有按键按下，不会被响应。

之前的测试情况是：按下按键，立马抬起；再次快速按下按键，立马抬起。两轮按键大概 200ms。
新的测试情况：按下按键，立马抬起；稍等一会，再次按下按键，立马抬起。两轮按键大概 1s。
nec 协议可以正常解析。
再次测试：按下立马抬起，再次快速按下，立马抬起。也没问题。
应该还是遥控器没有对准红外管。

## 【已解决】问题 3：Sagitta 发送 NEC 协议，Gx6605s 接收数据，两者如何对应？
- Sagitta 按照每 16bit  LSB 的方式发送
- Gx6605s 按照 MSB 的顺序进行解析，并且会对数据进行取反
```
Sagitta 发送：0x23456789 0xeeff1122 0x1b2b3b4b 0x90441c02
Gx6605s 接收：0x5d3b6e19 0x0088bb77 0x2b272d23 0xddf6bfc7

0x5d3b6e19: 0101 1101 0011 1011    0110 1110 0001 1001
			按位取反：
            1010 0010 1100 0100    1001 0001 1110 0110

			每16bit调整大小端：			
			0010 0011 0100 0101    0110 0111 1000 1001
		0x  2    3    4    5       6    7    8    9
		
		
0xeeff1122：1110 1110 1111 1111   0001 0001 0010 0010
			
			0001 0001 0000 0000   1110 1110 1101 1101
			
			0000 0000 1000 1000   1011 1011 0111 0111
		0x	0    0    8    8      b    b    7    7
			
```

## 【已解决】问题 4：Sagitta 发送 RC6 协议，Gx6605s 接收数据，两者如何对应？
- Sagitta 发送RC5协议的数据，向tx FIFO中写入的值为0x23456789，RC5 协议的custom code为5bit，command code 6bit，所以实际发出的数值为00101 001001。  
- Gx6605s 接收到的值为0x149，RC5协议为MSB，收到的值与发送的值一致，发送RC5数据正常。
```
00101 001001

0x149:
001 0010 1001
```


## 【已解决】问题 5：fpga 核心板上的红色灯可能影响红外传输
Sagitta 发送数据，Apus 接收数据，很容易出现无法收到正确数据的情况。
用贴纸把核心板上的红色 led 灯贴上，效果好一些。



# 测试用例
### 标准协议带载波发送
- 9012
	- sagitta 发送波形
		-  ![[Pasted image 20250424135545.png]]
		-  ![[Pasted image 20250424140816.png]]
	- apus 接收数据
```
Done(0),recv type: 9012 , data: 23456789
1111 1111 0111 0111   0100 0100 1000 1000
ee ff                 11 22
Done(0),recv type: 9012 , data: eeff1122
1101 0100 1101 1000   1101 0010 1101 1100
1b 1b                 3b 4b
Done(0),recv type: 9012 , data: 1b2b3b4b
0010 0010 0000 1001   0100 0000 0011 1000
90 44                 1c 02
Done(0),recv type: 9012 , data: 90441c02
```
- nec：
	- sagitta 发送波形
		- ![[Pasted image 20250424153051.png]]
	- apus 接收数据
```
Done(0),recv type: NEC , data: 23456789
Done(0),recv type: NEC , data: eeff1122
Done(0),recv type: NEC , data: 1b2b3b4b
Done(0),recv type: NEC , data: 90441c02
```
- rc5
	- sagitta 发送波形：
		- 发送RC5协议的数据，向tx FIFO中写入的值为0x23456789，RC5 协议的custom code为5bit，command code 6bit，所以实际发出的数值为00101 001001。
		-  ![[2025-04-24_14-57.png]]
	- apus 接收数据
```
6631 解析为 0x149: 001 0100 1001


Done(0),recv type: RC5 , data: 50009
Done(0),recv type: RC5 , data: 1f0022
Done(0),recv type: RC5 , data: b000b
Done(0),recv type: RC5 , data: 40002

```
- rc6
	- sagitta 发送波形：
		- RC6协议的数据为8bit control和8bit information，所以写入FIFO的数据只有bit0～bit7和bit16～bit23有效，数据为MSB。写入FIFO数据为0x23456789，实际发出的数据为0x4589。
		-  ![[Pasted image 20250424144844.png]]
	- apus 接收数据：
```
0 1 0 0 0 1 0 1    1 0 0 0 1 0 0 1
0x45               0x89

Done(0),recv type: RC6 , data: 450089
Done(0),recv type: RC6 , data: ff0022
Done(0),recv type: RC6 , data: 2b004b
Done(0),recv type: RC6 , data: 440002

```

### 标准协议带载波发送 repeat
- 9012
	- saigtta 发送波形：
		- ![[Pasted image 20250424153920.png]]
	- apus 接收数据：
```
Done(10),recv type: 9012 REPEAT, data: 23456789
```
- nec
	- sagitta 发送波形：
		- ![[Pasted image 20250424154432.png]]
	- apus 接收数据：
```
Done(10),recv type: NEC REPEAT, data: 90441c02
```
- rc5
	- sagitta 发送波形：
		- ![[Pasted image 20250424160042.png]]
```
Done(10),recv type: RC5 REPEAT, data: 50009
```
- rc6
	- sagitta 发送波形：
		- ![[Pasted image 20250424160100.png]]
```
Done(0),recv type: RC6 , data: 450089
```

### 标准协议带载波接收
### 标准协议带载波接收 repeat
遥控器按下 2 键发送：
- 9012  -- 2246
```
Done(0),recv type: 9012 , data: e0efe01
Done(10),recv type: 9012 REPEAT, data: e0efe01
```
- nec -- 2290
```
Done(0),recv type: NEC , data: bf40fd02
Done(10),recv type: NEC REPEAT, data: bf40fd02
```
示波器抓取到的波形：
引导码 + 0000 0010 1111 1101 0100 0000 1011 1111
引导码 + 0 2 f d 4 0 b f
- rc5 -- 5008
```
Done(0),recv type: RC5 , data: d0002
Done(10),recv type: RC5 REPEAT, data: d0002
```
- rc6 -- 0020
```
Done(0),recv type: RC6 , data: 2
Done(10),recv type: RC6 REPEAT, data: 2
```


### 自动识别接收的协议类型
done


### 发送载波频率及占空比
- 50k 80 占空比：实测 50k 13/16 占空比
- 50k 40 占空比：实测 50k 54/57 占空比
- 38k 40 占空比：实测 38k 55/57 占空比


### 中断触发
```
boot> gx_ir recv_std_it
```
- 9012
```
data:0e0efe01, type:9012.


0111 0000 0111 0000     1000 0000 0111 1111  1
0e0e fe01

0:data 38201c1, h 4490 us, l 4490 us
1:data 6e0039, h 570 us, l 530 us
2:data de0039, h 570 us, l 1650 us
3:data de0039, h 570 us, l 1650 us
4:data df0039, h 570 us, l 1660 us

5:data 6e0039, h 570 us, l 530 us
6:data 6e0039, h 570 us, l 530 us
7:data 6f0039, h 570 us, l 540 us
8:data 6e0039, h 570 us, l 530 us

9:data 6e0039, h 570 us, l 530 us
10:data df0039, h 570 us, l 1660 us
11:data de0039, h 570 us, l 1650 us
12:data de0039, h 570 us, l 1650 us

13:data 6f0039, h 570 us, l 540 us
14:data 6e0039, h 570 us, l 530 us
15:data 6e0039, h 570 us, l 530 us
16:data 6f0039, h 570 us, l 540 us

17:data de0039, h 570 us, l 1650 us
18:data 6e0039, h 570 us, l 530 us
19:data 6e0039, h 570 us, l 530 us
20:data 6f0039, h 570 us, l 540 us

21:data 6e0039, h 570 us, l 530 us
22:data 6e0039, h 570 us, l 530 us
23:data 6f0039, h 570 us, l 540 us
24:data 6e0039, h 570 us, l 530 us

25:data 6e0039, h 570 us, l 530 us
26:data df0039, h 570 us, l 1660 us
27:data de0038, h 560 us, l 1660 us
28:data de0039, h 570 us, l 1650 us

29:data df0039, h 570 us, l 1660 us
30:data de0038, h 560 us, l 1660 us
31:data de0038, h 560 us, l 1660 us
32:data de0039, h 570 us, l 1650 us

33:data 3a990039, h 570 us, l 149440 us

```
- nec
```
data:bf40fd02, type:NEC.

0000 0010 1111 1101    0100 0000 1011 1111 1
bf 40                  fd 02

0:data 5450385, h 9010 us, l 4480 us
1:data 6d0039, h 570 us, l 520 us
2:data 6c0038, h 560 us, l 520 us
3:data 6d0039, h 570 us, l 520 us
4:data 6d0039, h 570 us, l 520 us

5:data 6d0038, h 560 us, l 530 us
6:data 6d0038, h 560 us, l 530 us
7:data de0038, h 560 us, l 1660 us
8:data 6d0038, h 560 us, l 530 us

9:data de0038, h 560 us, l 1660 us
10:data de0038, h 560 us, l 1660 us
11:data de0038, h 560 us, l 1660 us
12:data df0039, h 570 us, l 1660 us

13:data de0038, h 560 us, l 1660 us
14:data de0038, h 560 us, l 1660 us
15:data 6d0038, h 560 us, l 530 us
16:data e00038, h 560 us, l 1680 us

17:data 6c0038, h 560 us, l 520 us
18:data df0039, h 570 us, l 1660 us
19:data 6c0038, h 560 us, l 520 us
20:data 6d0039, h 570 us, l 520 us

21:data 6d0038, h 560 us, l 530 us
22:data 6c0038, h 560 us, l 520 us
23:data 6d0039, h 570 us, l 520 us
24:data 6f0039, h 570 us, l 540 us

25:data de005f, h 950 us, l 1270 us
26:data 6d0038, h 560 us, l 530 us
27:data de0038, h 560 us, l 1660 us
28:data de0038, h 560 us, l 1660 us

29:data de0038, h 560 us, l 1660 us
30:data df0039, h 570 us, l 1660 us
31:data de0038, h 560 us, l 1660 us
32:data de0038, h 560 us, l 1660 us

33:data 3a990039, h 570 us, l 149440 us
```
- rc5
```
data:d0002, type:RC5.



0:data a80057, h 870 us, l 810 us
1:data a90057, h 870 us, l 820 us
2:data 15200ac, h 1720 us, l 1660 us
3:data a90057, h 870 us, l 820 us
4:data 15300ac, h 1720 us, l 1670 us
5:data fd00ac, h 1720 us, l 810 us
6:data a80057, h 870 us, l 810 us
7:data a80057, h 870 us, l 810 us
8:data fe0057, h 870 us, l 1670 us
9:data 3a9900ac, h 1720 us, l 148290 us

```
- rc6
```
data:2, type:RC6.

```
```
ud rx done.
0:data a80057, h 870 us, l 810 us
1:data a90057, h 870 us, l 820 us
2:data 15200ac, h 1720 us, l 1660 us
3:data a90057, h 870 us, l 820 us
4:data 15300ac, h 1720 us, l 1670 us
5:data fd00ac, h 1720 us, l 810 us
6:data a70057, h 870 us, l 800 us
7:data a80058, h 880 us, l 800 us
8:data ff0058, h 880 us, l 1670 us
9:data 3a9900ac, h 1720 us, l 148290 us
```


### 带载波自由模式发送
- 发送自定义波形，参考 mnec 协议，发送数据为：
	- `0111 1001 1100 0000 1111 1100 1001 1100`
- 使用机顶盒 gx6605s 板子接收，能够被识别为 nec 协议，判断接收到的数据是否与发送的数据相同
- 解析的值为发送值取反且 MSB，结果正常
```
key value : 0x863f0363
1000 0110 0011 1111    0000 0011 0110 0011

0111 1001 1100 0000    1111 1100 1001 1100




03 9e 393f
```

灵活模式发射将待发射数据填入 tx fifo 中，其中高 16bit 为两次上升沿之间的计数值，低 16bit 为高电平的时长计数值。
配置 tx_trig = 1 触发发射后，用 irc_clk 分频出 100kHz 时钟，根据 tx fifo 的内容将数据发出。
帧与帧之间的间隔由全 1 表示，例如 fifo 位宽 32 位时，间隔符为 0xffffffff。
模拟 mnec 协议：
```c
int gx_ir_test_send_ud(GX_HAL_IR *ir_dev)
{
	GX_HAL_IR_TX_CFG_T tx;
	uint32_t data[64] = {0x04b00320,0x00640032,0x00c80032,0x00c80032,0x00c80032, /* 0111 */
					0x00c80032,0x00640032,0x00640032,0x00c80032, /* 1001 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00640032,0x00640032,0x00640032,0x00640032, /* 0000 */
					0x00c80032,0x00c80032,0x00c80032,0x00c80032, /* 1111 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00c80032,0x00640032,0x00640032,0x00c80032, /* 1001 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00c80032};
	gx_hal_ir_ud_tx_flush(ir_dev);
	memset(&tx, 0, sizeof(tx));
	tx.modulation_en = 1;
	tx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	tx.ud_txfifo_thr = 3;
	tx.data = (void*)data;
	tx.data_len = 34;
	gx_hal_ir_send_data_ud(ir_dev, &tx);

	return 0;
}
```
分频出 100kHz 的时钟，则 0x04b00320 表示周期时长为：0x4b0 = 1200 * 10us = 12ms，其中高电平时长为 0x320 = 800 * 10us = 8ms，低电平时长为 12 - 8 = 4ms。占空比为 1/3。
1 用 0x00640032 表示，周期时长为 0x64 = 100 * 10us = 1ms，其中高电平时长为 0x32 = 50 * 10us = 500us。
0 用 0x00c80032 表示，周期时长为 0xc8 = 200 * 10us = 2ms，其中高电平时长为 0x32 = 50 * 10us = 500us。



### 带载波自由模式接收
遥控器按下 2 键：0x0e0efe01 9012
```
0111 0000 0111 0000   1000 0000 0111 1111
0e0e                  fe01


0:data 38201c1, h 4490 us, l 4490 us

1:data 6e0039, h 570 us, l 530 us
2:data de0039, h 570 us, l 1650 us
3:data de0039, h 570 us, l 1650 us
4:data df003a, h 580 us, l 1650 us

5:data 6e0039, h 570 us, l 530 us
6:data 6e0039, h 570 us, l 530 us
7:data 6f003a, h 580 us, l 530 us
8:data 700039, h 570 us, l 550 us

9:data 6e0039, h 570 us, l 530 us
10:data de0039, h 570 us, l 1650 us
11:data de0039, h 570 us, l 1650 us
12:data df0039, h 570 us, l 1660 us

13:data 6e0039, h 570 us, l 530 us
14:data 6e0039, h 570 us, l 530 us
15:data 6f0039, h 570 us, l 540 us
16:data 700039, h 570 us, l 550 us

17:data de0039, h 570 us, l 1650 us
18:data 6e0039, h 570 us, l 530 us
19:data 6e0039, h 570 us, l 530 us
20:data 6f0039, h 570 us, l 540 us

21:data 6e0039, h 570 us, l 530 us
22:data 6e0039, h 570 us, l 530 us
23:data 6f0039, h 570 us, l 540 us
24:data 700039, h 570 us, l 550 us

25:data 6e0061, h 970 us, l 130 us
26:data de008e, h 1420 us, l 800 us
27:data de0066, h 1020 us, l 1200 us
28:data df0039, h 570 us, l 1660 us

29:data de0039, h 570 us, l 1650 us
30:data de0039, h 570 us, l 1650 us
31:data de0039, h 570 us, l 1650 us
32:data df0039, h 570 us, l 1660 us

33:data 27110039, h 570 us, l 99440 us

```
遥控器按下 2 键：0xbf40fd02 nec
```
0xbf40fd02
0000 0010 1111 1101   0100 0000 1011 1111 1
bf40                  fd02

0:data 5450384, h 9000 us, l 4490 us
1:data 6d0039, h 570 us, l 520 us
2:data 6d0038, h 560 us, l 530 us
3:data 6d0038, h 560 us, l 530 us
4:data 6c0038, h 560 us, l 520 us

5:data 6d0039, h 570 us, l 520 us
6:data 6d0039, h 570 us, l 520 us
7:data de0038, h 560 us, l 1660 us
8:data 6d0038, h 560 us, l 530 us

9:data de0038, h 560 us, l 1660 us
10:data de0038, h 560 us, l 1660 us
11:data df0039, h 570 us, l 1660 us
12:data de0038, h 560 us, l 1660 us

13:data de0038, h 560 us, l 1660 us
14:data df0039, h 570 us, l 1660 us
15:data 6c0038, h 560 us, l 520 us
16:data df0039, h 570 us, l 1660 us

17:data 6c0038, h 560 us, l 520 us
18:data de0039, h 570 us, l 1650 us
19:data 6d0039, h 570 us, l 520 us
20:data 6d0039, h 570 us, l 520 us

21:data 6d0038, h 560 us, l 530 us
22:data 6d0038, h 560 us, l 530 us
23:data 6c0038, h 560 us, l 520 us
24:data 6d0039, h 570 us, l 520 us

25:data de0038, h 560 us, l 1660 us
26:data 6d0039, h 570 us, l 520 us
27:data de0038, h 560 us, l 1660 us
28:data df0039, h 570 us, l 1660 us

29:data de0038, h 560 us, l 1660 us
30:data de0038, h 560 us, l 1660 us
31:data de0038, h 560 us, l 1660 us
32:data df0039, h 570 us, l 1660 us

33:data 27110038, h 560 us, l 99450 us

```
遥控器按下 2 键：5008 rc5
```
ret 10,recv data:
0:data a80057, h 870 us, l 810 us
1:data a90057, h 870 us, l 820 us
2:data 15200ac, h 1720 us, l 1660 us
3:data a90057, h 870 us, l 820 us
4:data 15300ac, h 1720 us, l 1670 us
5:data fd00ac, h 1720 us, l 810 us
6:data a80057, h 870 us, l 810 us
7:data a80057, h 870 us, l 810 us
8:data fe0057, h 870 us, l 1670 us
9:data 271100ac, h 1720 us, l 98290 us


```


## 不带载波标准协议发送
9012：
![[Pasted image 20250425093503.png]]
nec：
![[Pasted image 20250425094148.png]]
rc5：
- 发送RC5协议的数据，向tx FIFO中写入的值为0x23456789，RC5 协议的custom code为5bit，command code 6bit，所以实际发出的数值为00101 001001。
![[Pasted image 20250425094730.png]]
rc6：
RC6协议的数据为8bit control和8bit information，所以写入FIFO的数据只有bit0～bit7和bit16～bit23有效，数据为MSB。写入FIFO数据为0x23456789，实际发出的数据为0x4589。
![[Pasted image 20250425100213.png]]




## 不带载波标准协议发送 repeat
9012：
![[Pasted image 20250425100148.png]]
nec:
![[Pasted image 20250425100718.png]]
rc5: 翻转位保持不变
![[Pasted image 20250425100711.png]]
rc6: 翻转位保持不变
![[Pasted image 20250425100701.png]]


## 不带载波自由模式发送
发送自定义波形参考mnec协议  
发送数据为：  
0111 1001 1100 0000 1111 1100 1001 1100  
测量发送的波形，从波形看，值与期望的一致，波形解析参考带载波模式
![[Pasted image 20250425101944.png]]


## 接收载波频率和占空比
遥控器按下 2 键：5008 rc5
```
boot> gx_ir recv_ud_carr_it
ud rx done.
0:data aa0057, h 870 us, l 830 us
1:data fc00ac, h 1720 us, l 800 us
2:data fd0057, h 870 us, l 1660 us
3:data a90057, h 870 us, l 820 us
4:data 15300ac, h 1720 us, l 1670 us
5:data fd00ac, h 1720 us, l 810 us
6:data a80057, h 870 us, l 810 us
7:data a80057, h 870 us, l 810 us
8:data fe0057, h 870 us, l 1670 us
9:data 3a9900ac, h 1720 us, l 148290 us
carr=d40028
carry freq: 37.74KHz, duty:19%




boot> gx_ir recv_ud_carr_it
ud rx done.
0:data 5450385, h 9010 us, l 4480 us
1:data 6c0038, h 560 us, l 520 us
2:data 6d0039, h 570 us, l 520 us
3:data 6d0039, h 570 us, l 520 us
4:data 6d0038, h 560 us, l 530 us
5:data 6c0038, h 560 us, l 520 us
6:data 6d0039, h 570 us, l 520 us
7:data de0038, h 560 us, l 1660 us
8:data 6d0039, h 570 us, l 520 us
9:data de0038, h 560 us, l 1660 us
10:data df0039, h 570 us, l 1660 us
11:data de0038, h 560 us, l 1660 us
12:data de0038, h 560 us, l 1660 us
13:data de0038, h 560 us, l 1660 us
14:data df0039, h 570 us, l 1660 us
15:data 6c0038, h 560 us, l 520 us
16:data df0039, h 570 us, l 1660 us
17:data 6c0038, h 560 us, l 520 us
18:data df0039, h 570 us, l 1660 us
19:data 6c0038, h 560 us, l 520 us
20:data 6d0039, h 570 us, l 520 us
21:data 6d0038, h 560 us, l 530 us
22:data 6d0038, h 560 us, l 530 us
23:data 6c0038, h 560 us, l 520 us
24:data 6d0039, h 570 us, l 520 us
25:data de0038, h 560 us, l 1660 us
26:data 6d0039, h 570 us, l 520 us
27:data de0038, h 560 us, l 1660 us
28:data df0039, h 570 us, l 1660 us
29:data de0038, h 560 us, l 1660 us
30:data de0038, h 560 us, l 1660 us
31:data de0038, h 560 us, l 1660 us
32:data de0039, h 570 us, l 1650 us
33:data 3a990039, h 570 us, l 149440 us
carr=d80028
carry freq: 37.04KHz, duty:19%

```


## 标准模式空闲电平
```diff
--- a/drivers/hal/src/ir/gx_hal_ir.c
+++ b/drivers/hal/src/ir/gx_hal_ir.c
@@ -345,7 +345,7 @@ int gx_hal_ir_send_data_std(GX_HAL_IR *ir_dev, const GX_HAL_IR_TX_CFG_T *tx,
                gx_hal_ir_set_ana_reg(ir_dev, &ana_reg);
        }
 
-       IR_WRITE_REG(ir_dev, (0<<8) | 0x1, GX_HAL_IRC_INTERVAL);
+       IR_WRITE_REG(ir_dev, (10<<8) | 0x1, GX_HAL_IRC_INTERVAL);
```
 用于配置发送时的帧间空闲电平时长：
配置间隔为10Tm，即5.6ms，9012协议波形总时间等于108ms+5.6ms=113.6ms，测量发送9012协议波形，时长与配置值基本一致
![[Pasted image 20250425104156.png]]