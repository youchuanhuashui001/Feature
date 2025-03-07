

- Quad：
	- `0x31 SR1` 支持
- UID:
	- `0x48 + Dummy` 支持
- 写保护：
	- 支持



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