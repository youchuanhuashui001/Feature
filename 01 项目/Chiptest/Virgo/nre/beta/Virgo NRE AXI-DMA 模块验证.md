
## 1. 基本信息

- **Redmine 问题号**: `408503`
- **芯片型号/项目名称**: `Virgo_NRE`
- **被测模块**: `AXI_DMA`
- **固件/FPGA 镜像版本**: 
	- a7： #404698 vu9p_fpga_top_sub_a7_20250524. bit test7 a7 核 flash 中 48M
	- a55: #404698 vu9p_fpga_top_sub_a55_20250524. bit test8 a55 核 flash 中 48M
- **相关文档**: [[01 项目/Common/IP/dw_i2c ip]]


# 测试用例与测试结果

dma comtest 会覆盖下面的所有测试用例，comtest 包括：
- 内存到内存
- 不同传输位宽
- 不同源和目的 msize
- 链表、非链表
- 每个通道
- 不同 block_ts

## 1.内存到内存传输
- 打开 `.config` 中的 `ENABLE_AXI_DMA_TEST = y, ENABLE_IRQ = y`，因为测试 case 会涉及同步、异步
- 执行 `dma comtest`

## 2.DMA 传输位宽
- `dma comtest` 会测试位宽 8bit、16bit、32bit、64bit

## 3.不同源和目的 msize
- `dma comtest` 会测试源、目的 msize：1、4、8、16、32、64

## 4.链表传输
- `dma comtest` 会测到

##  5.非链表传输
- `dma comtest` 会测到

## 6. 低功耗测试
- <font color="#ff0000">nre 这版低功耗 gate 未开，后续后出一版 bit 补测</font>


## 7. 性能测试




# 问题

## 对接外设不通
- mpw 测试时使用的桥时 designware 的，由于高安有需求，自己配置出了一个桥，但该桥有问题。
- 芯片决定改版沟通：改版成对接外设时可以地址递增和地址固定，对接内存时只能地址递增，不能地址固定；
	- 但若对接内存时配置为地址固定，会导致总线挂掉；因此芯片决定修改桥的设计。
- 芯片改版后对接 uart ok

## 【已解决】测试不同的 block_ts
目前 llp_size 仅支持 4 个链表，所以测试 case 中的长度和链表大小需要对应调整。
```
#define DMA_CHANNEL_LLP_SIZE     384    // (16*1024*1024/(4*1024*1024-1)+1)*sizeof(dw_lli_t)+DCACHE_LINE_SIZE

```
测试长度为 0x10000，那么 block_ts 就只能测试 0x0, 0x4000，0x8000，0xc000，0x10000

测试时出现许多个 dma callback 打印，可能是驱动里面把所有的 int_status 打开了，但是这个开的又不是 int_single 应该不会触发中断吧？
- 是的，把使能的 all 去掉之后就只会触发一次中断
- 打开使能的 all 之后，也只会触发一次中断了，和设计是一致的