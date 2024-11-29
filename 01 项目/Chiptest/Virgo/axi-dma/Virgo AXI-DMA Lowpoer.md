

## bit(dmac_1) 测试情况
- 先使能 dma 后配置 `0xfa49008c=0x1` 正常工作
- 先配置 `0xfa49008c=0x1` 再使能 `dma` 无法完成


## bit(dmac_2) 测试情况
- 先使能 dma 后配置 `0xfa49008c=0x1` 正常工作
	- 连续多次 dma 传输 ok
- 先配置 `0xfa49008c=0x1` 再使能 `dma` 正常工作
	- 连续多次 dma 传输 ok

| 测试项                        | 测试结果   | 备注                           |
| -------------------------- | ------ | ---------------------------- |
| 先使能 lowpoer，再进行 comtest    | ok     |                              |
| 不使能 Lowpower，直接进行 comtest  | ok     |                              |
| 先使能 Lowpower，再进行 lowpower  | ok     | 需要手动关闭 dw_dmac_xfer 中的通道使能接口 |
| 不使能 Lowpower，直接进行 Lowpower | ok<br> | 需要手动关闭 dw_dmac_xfer 中的通道使能接口 |
| 使能 Lowpower，对接外设           | ok     |                              |
| 不使能 Lowpower，对接外设          | ok     |                              |

## 驱动修改
- Lowpower 是属于驱动的一部分，需要在初始化的时候就配置好这个寄存器
- `axi_dma_lowpower_test` 测试用例应该修改名称为 `axi_dma_multi_channel_test`
- 通道时钟使能、关闭需要循环测到，这里在 comtest 里面会测到同一个通道打开、关闭




## todo
- [x] dma 驱动中需要清掉第二次从链表中取数据时配成的 last