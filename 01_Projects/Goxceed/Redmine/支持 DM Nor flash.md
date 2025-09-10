
# 阅读手册

- Jedec ID：
	- 0xc22016，原来已支持这款 flash，需要看下 markid，确认与 MX25L3233F 一致
		- 0x64 = 0x9e，则为 MX25L3233F
		- 0x64 = 0xF6，则为 MX25L3206E
		- 否则为 MX25L32
	- 0xc22017，原来已支持这款 flash，需要看下 markid
		- MX25L6433F
		- 手册没有描述 SFDP 区域
- otp ：
	- 0xc22016: 4096bit，512Byte，0x0~0x1ff
	- 0xc22017: 2 * 512Byte,`0x0 ~ 0x1ff; 0x200 ~ 0x3ff`
- uid ：
	- 0xc22016：存在 sfdp 区域，但是没有描述如何读取
		```
		boot> flash uid
		uid: 
		c2 f2 05 1b 03 03 04 01 00 00 06 59 52 02 04 ff
		```
	- 0xc22017: 存在 sfdp 区域，但是没有描述如何读取 uid
		```
		boot> flash uid
		uid: 
		c2 f2 05 1b 09 07 08 05 00 00 06 33 26 02 05 55
		```
- 写保护：
	-  四线使能：`模式0 0x31 写 SR1` `模式1 0x01 写 SR0、SR1`
	- 写保护：`0x05 读 SR0，0x35 读 SR1`
- quad 使能方式：
	- 是否支持 0x01 read sr？
	- 是否支持？




# 修改代码




# 测试用例 
- 单线读写擦：
	- 32Mbit：测试中 560 次测试通过
	- 64Mbit：单线测试ok，4K擦除测试ok
- 双线读写擦：
	- 32Mbit
	- 64MBit：读写擦 ok
- 四线读写擦：
	- 32MBit
	- 64MBit：读写擦 ok
- UID：
	- 32MBit：ok
	- 64Mbit：ok
- OTP：
	- 32MBit：ok
	- 64MBit：ok
- WP：
	- 32MBit：ok
	- 64MBit：ok
- 单页多次写：
	- 32MBit：ok
	- 64Mbit：ok
- 掉电测试 ：

```

* 测试项与测试结果：
|测试项|测试结果|
|4K Sector擦除||
|单倍速读写擦1000次||
|双倍速读写擦1000次||
|四倍速读写擦1000次||
|写保护测试||
|otp||
|uid||
|单页多次写测试||
|300次掉电测试||


* develop 分支补丁：{{patch()}}
```

- [ ] 手册上传至云盘

# 其它 