---
tags:
  - Virgo_MPW模块验证
---
# 验证进度：
- 总验证点:: 4
- 已完成:: 4
- 备注:: 无


# 修改项
- 模块频率：49.5MHz


# 测试项


## 复位
- 可以重启


## 喂狗
- 49.5MHz 的时钟，定时 10s 喂狗
```
[2025-07-04 20:47:58] wdt callback.
[2025-07-04 20:48:09] wdt callback.
[2025-07-04 20:48:19] wdt callback.
```


## 使能中断
- 能够触发中断


## 读取定时器值
- 使能 watchdog 后，查询计数值，依次递减
```
(gdb) x/x 0xfa450008
0xfa450008:     0x0a133d71
(gdb) x/x 0xfa450008
0xfa450008:     0x086d18c7
(gdb) x/x 0xfa450008
0xfa450008:     0x06ff8a92
(gdb) x/x 0xfa450008
0xfa450008:     0x05bb6967
(gdb) x/x 0xfa450008
0xfa450008:     0x0479a21e
(gdb) x/x 0xfa450008
0xfa450008:     0x0342a1ca
(gdb) x/x 0xfa450008
0xfa450008:     0x0218e00d
(gdb) x/x 0xfa450008
0xfa450008:     0x00df65e0
```