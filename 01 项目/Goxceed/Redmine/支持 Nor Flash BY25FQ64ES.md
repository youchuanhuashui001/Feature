
# 阅读手册

- Jedec ID：0x684017
- otp ：`3*1024 `
- uid ：128bit
- 写保护：
	- 四线使能：`模式0 0x31 写 SR1` `模式1 0x01 写 SR0、SR1`
	- 写保护：`0x05 读 SR0，0x35 读 SR1`
- quad 使能方式：
	- 是否支持 0x01 read sr？
		- 支持




# 修改代码




# 测试用例 
- 单线读写擦：
- 双线读写擦：ok
- 四线读写擦：ok
	- BY25Q64AS 四线测试 1000 次 ok
- UID：ok
- OTP：ok
- WP：ok
- 单页多次写：ok
- 掉电测试 ：ok

```

* 测试项与测试结果：
|测试项|测试结果|
|4K Sector擦除|ok|
|单倍速读写擦1000次|ok|
|双倍速读写擦1000次|ok|
|四倍速读写擦1000次|ok|
|写保护测试|ok|
|otp|ok|
|uid|ok|ls
|单页多次写测试|ok|
|300次掉电测试|ok|


* develop 分支补丁：{{patch(124317)}}
```

- [x] 手册上传至云盘 


# 其它 