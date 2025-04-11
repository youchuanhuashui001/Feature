
# 背景
事业部没有正确配置 dtb 的宏，导致 stage2 放到了 0 地址 (dram 首地址)。发现无法正常启动，排查问题时发现 spinand 驱动会往 0 地址读写数据。

# 修改
stage1 spinand 驱动使用 tx_and_rx 模式来进行数据传输，因此无论是 tx 或 rx，都需要发一个字节，读一个字节。
存在两种情况：
- tx 不为空时，需要从 tx buffer 一直取数据写到 fifo
- rx 不为空时，需要从 fifo 一直取数据写到 rx buffer
对于上面两种情况：
- 如果 rx buffer 为空时，会从 fifo 取数据写到 0 地址
- 如果 tx buffer 为空时，会从 0 地址取数据到 fifo

驱动的逻辑判断方法：
- tx：tx_end - len 
- rx：rx_end - len 一直有效

修改后，不允许对 0 地址读写数据



----

上面的描述错误!!！
实际代码修改是为了解决 a55 编译报错
```diff

```