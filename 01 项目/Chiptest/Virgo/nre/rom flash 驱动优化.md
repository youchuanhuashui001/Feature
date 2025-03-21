
- [ ] 优化 spinor flash 驱动，使用双倍速
	- [ ] 四字节地址的问题
	- [ ] 需要考虑四倍速的情况，stage1 bbt 会用到四倍速
	- [ ] 四线的时候要根据芯片判断启不启动
- [ ] 添加 spinand flash 驱动


# spinor flash 驱动

## Stage1
直接用 scpu 中 apus 的 stage1 驱动，需要添加 4 字节地址即可




# Q&&A
## 写地址的时候直接用 `writel` 接口把地址一次写掉，会有问题吗？
- `writel` 接口一次最多可以写四个字节，当 spi 位宽配成 8bit 的时候，写四个字节的操作会被分成 4 次写入操作
- 但有一个问题：
	- 3 字节地址模式时，flash 需要收到 `0x3b + 3 byte address + dummy`：这样地址配了 3 个字节，最后一个字节会不会当成 dummy 发出去？
	- 4 字节地址模式的时候，就没有 dummy 了

### 读 magic 的时候用一个 `uint32_t` 变量来保存存在大小端问题
- 原来读 flash 的操作，是读一个字节，移位，再读一个字节，再移位
- 但是用 `uint32_t` 直接保存的时候，不会移位，所以需要手动调整大小端


### 发地址的时候，Virgo 和 Canopus/Cygnus 不一样
- Virgo 可以发 0x64
- Canopus 不能发 0x64，
- Cygnus 不能发 0x64，换成发 0x60 起来了 (develop 分支)


gdb 看 buf 数据
关 dmacr