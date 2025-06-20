
# X25Q128ES：芯一代
- JedecID: 手册未描述
- 最大工作频率：
	- 0x03 100MHz
- OTP：3 * 1024
	- 手册没写 otp 地址
- UID：手册未描述
- 命令：
	- 0x03、0x01 等命令都没有描述可以操作几个字节
- datasheet 没有提供 JedecID，需要咨询 Flash 厂商



# YXW25Q64E：GD25Q64
- jedecid：0xc84017
- 最大工作频率：
	- 0x03：80MHz
	- DC=0 104MHz
	- DC=1 3.0~3.6v 供电 133MHz
- otp：3 * 1024
	- 0x001000
	- 0x002000
	- 0x003000
- uid：128bit
	- 0x4b + 24bit address + dummy
- 命令：
	- 0x05:read_sr1
	- 0x35:read_sr2
	- 0x15:read_sr3
	- 0x01: write sr1
	- 0x31: write sr2
	- 0x11: write sr3


- 测试用例：

| 测试项     | 测试结果 |
| ------- | ---- |
| 双线读写擦测试 | ok   |
| 四线读写擦测试 | ok   |
| XIP 测试  | ok   |
| 掉电测试    | ok   |
| 单页多次写测试 | ok   |

```
相位测试：leo_mini
* 测试条件：读写擦测试 10000 次不出错
* flash 型号：YXW25Q64E
* 硬件：leo_mini socket 板

|模块频率|模块分频|工作频率|相位范围|
|200M   |4     |50M    |1、2、3|
|150M   |2     |75M    |1、2|
|150M   |4     |37.5M  |0、1、2、3 |




```