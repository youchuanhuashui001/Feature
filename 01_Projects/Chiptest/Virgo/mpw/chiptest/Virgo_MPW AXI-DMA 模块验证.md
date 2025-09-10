---
tags:
  - Virgo_MPW模块验证
---
# 验证进度：
- 总验证点:: 9
- 已完成:: 9
- 备注:: 低功耗测试失败



# 验证项

## 内存到内存传输测试
- 测试通过


## DMA 传输宽度测试
- 测试通过


## 不同源和目的 msize 的DMA传输测试
- 测试通过


## 链表传输
- 测试通过


## 非链表传输
- 测试通过


## 低功耗测试
- 使能低功耗后，第一次测试成功，第二次测试失败
```
boot> dma multi_channel
dma multi channel test success.
boot> dma multi_channel
```


## msize为1items的传输速度测试

```shell
boot> dma config 0 0 0 0 0 0 0 0
boot> dma speed 0x200000 0x300000 0x100000 0 100
[test env]
test_src_addr = 0x200000
test_dst_addr = 0x300000
test_len = 1048576B
test_count = 100B
test_total_size = 102400KB

[test result]
test_time = 2674ms
test_speed = 38294KB/s
boot> dma config 1 1 0 0 0 0 0 0
boot> dma speed 0x200000 0x300000 0x100000 0 100
[test env]
test_src_addr = 0x200000
test_dst_addr = 0x300000
test_len = 524288B
test_count = 100B
test_total_size = 102400KB

[test result]
test_time = 1362ms
test_speed = 75183KB/s
boot> dma config 2 2 0 0 0 0 0 0
boot> dma speed 0x200000 0x300000 0x100000 0 100
[test env]
test_src_addr = 0x200000
test_dst_addr = 0x300000
test_len = 262144B
test_count = 100B
test_total_size = 102400KB

[test result]
test_time = 688ms
test_speed = 148837KB/s
boot> dma config 3 3 0 0 0 0 0 0
boot> dma speed 0x200000 0x300000 0x100000 0 100
[test env]
test_src_addr = 0x200000
test_dst_addr = 0x300000
test_len = 131072B
test_count = 100B
test_total_size = 102400KB

[test result]
test_time = 339ms
test_speed = 302064KB/s

```

## msize为32items的传输速度测试
```shell
boot> dma config 0 0 4 4 0 0 0 0
boot> dma speed 0x200000 0x300000 0x100000 0 100
[test env]
test_src_addr = 0x200000
test_dst_addr = 0x300000
test_len = 1048576B
test_count = 100B
test_total_size = 102400KB

[test result]
test_time = 2674ms
test_speed = 38294KB/s
boot> dma config 1 1 4 4 0 0 0 0
boot> dma speed 0x200000 0x300000 0x100000 0 100
[test env]
test_src_addr = 0x200000
test_dst_addr = 0x300000
test_len = 524288B
test_count = 100B
test_total_size = 102400KB

[test result]
test_time = 1362ms
test_speed = 75183KB/s
boot> dma config 2 2 4 4 0 0 0 0
boot> dma speed 0x200000 0x300000 0x100000 0 100
[test env]
test_src_addr = 0x200000
test_dst_addr = 0x300000
test_len = 262144B
test_count = 100B
test_total_size = 102400KB

[test result]
test_time = 687ms
test_speed = 149053KB/s
boot> dma config 3 3 4 4 0 0 0 0
boot> dma speed 0x200000 0x300000 0x100000 0 100
[test env]
test_src_addr = 0x200000
test_dst_addr = 0x300000
test_len = 131072B
test_count = 100B
test_total_size = 102400KB

[test result]
test_time = 338ms
test_speed = 302958KB/s

```


| items | bits | speed(MB/S) |
|:------|:-----|:------------|
| 1     | 8    | 37.4        |
| 1     | 16   | 73.4        |
| 1     | 32   | 145.3       |
| 1     | 64   | 295.0       |
| 32    | 8    | 37.4        |
| 32    | 16   | 73.4        |
| 32    | 32   | 145.6       |
| 32    | 64   | 295.9       |
  


## 降低CPU频率的DMA传输速度




# 问题