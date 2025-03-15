
- SR:
	- 读：`0x05 SR0; 0x35 SR1`
	- 写：`0x01 SR0; 0x31 SR1; 0x01 SR0 SR1`
- Quad：
	- `0x31 SR1` 支持
- UID:
	- `0x48 + Dummy` 支持
- 写保护：
	- 支持
- otp:
	- 

# leo mini 测试：

- 测试项：
	- 单页多次写：ok
	- uid：ok
	- 写保护：ok


fornax 要测一款 GT25Q16B 跑 130MHz 的情况，但是 fornax 的 flash 都是内封的，并且给到外设的 pll 频率不能超过 200M，


leo_mini 默认起来的时候就会跑 stage2 的 flash 初始化？
相位配成 1：id 读不到
相位配成 2：可以读到 id，但是读写 flash 有问题
相位配成 3：可以
相位配成 4：可以
- 测的时间是 7ns 一个周期，2 分频大约是在 150Mhz 左右；pll 配的是 240Mhz，但是实际可能到了 300MHz


130Mhz 测试情况：测试了两颗没问题，都是只有相位 3 可用
- 相位：
	- 2 不可用
	- 3 可用
	- 4 不可用
- 双线：ok
```
********** FLASH SPEED TEST *********
>>>>>read       8 kb from flash speed: 3060 KB/s
>>>>>erase      8 kb from flash speed: 1277 KB/s
>>>>>write      8 kb from flash speed: 267 KB/s
flash comtest finished.

32bit 开 dma：
********** FLASH SPEED TEST *********
>>>>>read       8 kb from flash speed: 24242 KB/s
>>>>>erase      8 kb from flash speed: 1277 KB/s
>>>>>write      8 kb from flash speed: 280 KB/s
flash comtest finished.
```
- 四线：
```
********** FLASH SPEED TEST *********
>>>>>read       8 kb from flash speed: 3056 KB/s
>>>>>erase      8 kb from flash speed: 1276 KB/s
>>>>>write      8 kb from flash speed: 267 KB/s
flash comtest finished.


8bit 开 dma：
********** FLASH SPEED TEST *********
>>>>>read       8 kb from flash speed: 22099 KB/s
>>>>>erase      8 kb from flash speed: 1277 KB/s
>>>>>write      8 kb from flash speed: 282 KB/s
flash comtest finished.

31bit 开 dma：
********** FLASH SPEED TEST *********
>>>>>read       8 kb from flash speed: 38647 KB/s
>>>>>erase      8 kb from flash speed: 1277 KB/s
>>>>>write      8 kb from flash speed: 282 KB/s
flash comtest finished.

```
- xip：ok


# fornax 测试：
- nor_test case:ok
- wp:ok
- uid:ok
- multiprogramtest:ok