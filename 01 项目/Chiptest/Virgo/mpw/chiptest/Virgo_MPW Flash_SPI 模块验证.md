---
tags:
  - Virgo_MPW模块验证
---
# 验证进度：
- 总验证点:: 8
- 已完成:: 8
- 备注:: 需要把 gxloader 的工作频率提上去，linux4.9 也可以跑 50MHz
	- gxloader 工作频率为 25MHz，uboot 也为 25MHz、linux4.19 也为 25MHz。


# 关联问题号
- canopus： https://git.nationalchip.com/redmine/issues/308817
- vega： https://git.nationalchip.com/redmine/issues/361528


# 功能测试

## 单线
- spinor：
	- 读写擦 40 多次

- spinand：
	- 读写擦 25 次未出错
	- 读写擦 103 次未出错

## 双线
- spinor：
	- 读写擦测试 1489 次，未出错。
	- 100MHz 读写擦 134 次，未出错

- spinand:
	- spinand 50MHz 读写擦1000次未出错。
	- spinand 74MHz 测试9362 次未出错。


## 四线
- spinor:
	- 读写擦 80 多次
	- 297MHz 模块频率，6 分频到接近 50MHz，跑一晚上看看
		- 1660 次未出错

- spinand:
	- spinand 641 次未出错


## 八线
- 硬件接的是 CS1

- 读写测试
	- 单线读写 10次
	- 八线读写
		- gd：331 次
		- mxic：

- 性能测试：
```

## GD

# 模块频率 396MHz 4分频 DMA 八线
boot> flash speedtest
erase flash size 16MiB
program 16MiB elapse 9328ms, 1756 KB/S
read 16MiB elapse 173ms, 94705 KB/S
erase 16MiB elapse 25455ms, 643 KB/S

# 模块频率 396MHz 4分频 CPU 单线
boot> flash speedtest
erase flash size 16MiB
program 16MiB elapse 9885ms, 1657 KB/S
read 16MiB elapse 1356ms, 12082 KB/S
erase 16MiB elapse 25468ms, 643 KB/S

# 模块频率 297MHz 4分频 DMA 八线
boot> flash speedtest
erase flash size 16MiB
program 16MiB elapse 9025ms, 1815 KB/S
read 16MiB elapse 229ms, 71545 KB/S
erase 16MiB elapse 23903ms, 685 KB/S


# 模块频率 297MHz 6分频 DMA 八线
boot> flash speedtest
erase flash size 16MiB
program 16MiB elapse 9463ms, 1731 KB/S
read 16MiB elapse 342ms, 47906 KB/S
erase 16MiB elapse 25508ms, 642 KB/S


# 模块频率 297MHz 6分频 CPU 八线
boot> flash speedtest
erase flash size 16MiB
program 16MiB elapse 8802ms, 1861 KB/S
read 16MiB elapse 734ms, 22321 KB/S
erase 16MiB elapse 25484ms, 642 KB/S


# 模块频率 297MHz 6分频 CPU 单线
boot> flash speedtest
erase flash size 16MiB
program 16MiB elapse 11030ms, 1485 KB/S
read 16MiB elapse 2712ms, 6041 KB/S
erase 16MiB elapse 25426ms, 644 KB/S


# 模块频率 297MHz 8分频 DMA 八线
boot> flash speedtest
erase flash size 16MiB
program 16MiB elapse 9282ms, 1765 KB/S
read 16MiB elapse 455ms, 36008 KB/S
erase 16MiB elapse 23931ms, 684 KB/S


# mxic

# 模块频率 396MHz 4分频 DMA 八线
boot> flash speedtest
erase flash size 16MiB
program 16MiB elapse 11954ms, 1370 KB/S
read 16MiB elapse 172ms, 95255 KB/S
erase 16MiB elapse 56713ms, 288 KB/S




```



## 中断
- spinor:
	- 能够触发 tx_empty、rx_full 中断

- spinand:
	- spinand 测试ok

## DMA
- spinor:
	- 双线 rx
	- 四线 tx、rx

- spinand:
	- 双线 rx
	- 四线 tx、rx





## 多 CS
- 测试通过



## 相位测试

### 模块频率：297MHz

- 4 分频：74.25MHz
- 硬件板：4 号板

| 分频  | 频率       | nor                         | nand                    |
| --- | -------- | --------------------------- | ----------------------- |
| 4   | 74.25MHz | 6, 7, 8, 9, 10, 11, 12, 13, | 6, 7, 8, 9, 10, 11, 12, |



- 6 分频：49.5MHz
- 硬件板：4 号板

| 分频   | 频率      | nor                                    | nand                                  |
| ---- | ------- | -------------------------------------- | ------------------------------------- |
| 6 分频 | 49.5MHz | 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, | 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, |



- 8 分频：37.125MHz
- 硬件板：4 号板

| 分频   | 频率        | nor                                                  | nand                                                |
| ---- | --------- | ---------------------------------------------------- | --------------------------------------------------- |
| 8 分频 | 37.125MHz | 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, | 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, |




- 12 分频：24.75MHz
- 硬件板：4 号板

| 分频  | 频率       | nor                                                                       | nand                                                                      |
| --- | -------- | ------------------------------------------------------------------------- | ------------------------------------------------------------------------- |
| 12  | 24.75MHz | 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, | 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, |





### 模块频率：396Mhz

- 模块频率：400MHz
- 4 分频：100MHz
- 硬件板：4 号板

| 分频  | 频率       | 可用相位范围                  |
| --- | -------- | ----------------------- |
| 12  | 24.75MHz | 10, 11, 12, 13, 14, 15, |



# 性能测试

## 2 分频
- 手册最高频率：133MHz，超频至 150MHz
```
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 25528ms, 641 KB/S
read 16MiB elapse 733ms, 22351 KB/S
erase 16MiB elapse 44013ms, 372 KB/S




```

## 4 分频
- spinor
```
# 模块频率 297MHz CPU 单线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 28435ms, 576 KB/S
read 16MiB elapse 1808ms, 9061 KB/S
erase 16MiB elapse 46354ms, 353 KB/S


#  模块频率 297MHz CPU 双线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 26243ms, 624 KB/S
read 16MiB elapse 905ms, 18103 KB/S
erase 16MiB elapse 44231ms, 370 KB/S


# 模块频率 297MHz CPU 四线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 25471ms, 643 KB/S
read 16MiB elapse 755ms, 21700 KB/S
erase 16MiB elapse 44011ms, 372 KB/S


# 模块频率 297MHz DMA 双线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 26966ms, 607 KB/S
read 16MiB elapse 907ms, 18063 KB/S
erase 16MiB elapse 43899ms, 373 KB/S


# 模块频率 297Mhz DMA 四线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 26510ms, 618 KB/S
read 16MiB elapse 455ms, 36008 KB/S
erase 16MiB elapse 44013ms, 372 KB/S









# 模块频率 376MHz CPU 单线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 27948ms, 586 KB/S
read 16MiB elapse 1356ms, 12082 KB/S
erase 16MiB elapse 46292ms, 353 KB/S


# 模块频率 376MHz CPU 双线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 26371ms, 621 KB/S
read 16MiB elapse 735ms, 22291 KB/S
erase 16MiB elapse 44255ms, 370 KB/S


# 模块频率 376MHz CPU 四线 (可能 CPU 目前跑的频率不高)
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 25342ms, 646 KB/S
read 16MiB elapse 733ms, 22351 KB/S
erase 16MiB elapse 44244ms, 370 KB/S


# 模块频率 376MHz DMA 双线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 28051ms, 584 KB/S
read 16MiB elapse 681ms, 24058 KB/S
erase 16MiB elapse 46125ms, 355 KB/S



# 模块频率 376MHz DMA 四线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 26121ms, 627 KB/S
read 16MiB elapse 342ms, 47906 KB/S
erase 16MiB elapse 44324ms, 369 KB/S

```

- spinand
```
# 模块频率 297MHz 4分频 CPU 四线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 1793ms, 4568 KB/S
read 8MiB elapse 614ms, 13342 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 358ms, 22882 KB/S

# 模块频率 297MHz 4分频 DMA 四线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 1861ms, 4401 KB/S
read 8MiB elapse 583ms, 14051 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 359ms, 22818 KB/S


# 模块频率 297MHz 4分频 CPU 双线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 2510ms, 3263 KB/S
read 8MiB elapse 699ms, 11719 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 358ms, 22882 KB/S



# 模块频率 297MHz 4分频 DMA 双线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 2519ms, 3252 KB/S
read 8MiB elapse 812ms, 10088 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 359ms, 22818 KB/S


# 模块频率 297MHz 4分频 CPU 单线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 2507ms, 3267 KB/S
read 8MiB elapse 1148ms, 7135 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 360ms, 22755 KB/S




# 模块频率 396Mhz 4分频 CPU 四线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 1766ms, 4638 KB/S
read 8MiB elapse 604ms, 13562 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 359ms, 22818 KB/S


# 模块频率 396MHz 4分频 DMA 四线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 1794ms, 4566 KB/S
read 8MiB elapse 527ms, 15544 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 361ms, 22692 KB/S

# 模块频率 396MHz 4分频 CPU 双线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 2268ms, 3611 KB/S
read 8MiB elapse 625ms, 13107 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 361ms, 22692 KB/S

# 模块频率 396MHz 4分频 DMA 双线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 2281ms, 3591 KB/S
read 8MiB elapse 705ms, 11619 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 360ms, 22755 KB/S

# 模块频率 396MHz 4分频 CPU 单线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 2266ms, 3615 KB/S
read 8MiB elapse 916ms, 8943 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 361ms, 22692 KB/S


```


## 6 分频
spinor：
```
# 模块频率 297MHz 4分频 CPU 单线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 29298ms, 559 KB/S
read 16MiB elapse 2711ms, 6043 KB/S
erase 16MiB elapse 46560ms, 351 KB/S


# 模块频率 297MHz 4分频 CPU 双线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 29294ms, 559 KB/S
read 16MiB elapse 1356ms, 12082 KB/S
erase 16MiB elapse 46625ms, 351 KB/S



# 模块频率 297MHz 4分频 CPU 四线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 27376ms, 598 KB/S
read 16MiB elapse 735ms, 22291 KB/S
erase 16MiB elapse 46817ms, 349 KB/S



# 模块频率 297MHz 4分频 DMA 双线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 27965ms, 585 KB/S
read 16MiB elapse 1359ms, 12055 KB/S
erase 16MiB elapse 43970ms, 372 KB/S


# 模块频率 297MHz 4分频 DMA 四线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 28484ms, 575 KB/S
read 16MiB elapse 681ms, 24058 KB/S
erase 16MiB elapse 46967ms, 348 KB/S

```

spinand:
```

# 模块频率 297MHz 6分频 DMA 四线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 1984ms, 4129 KB/S
read 8MiB elapse 697ms, 11753 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 358ms, 22882 KB/S

# 模块频率 297MHz 6分频 CPU 四线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 1908ms, 4293 KB/S
read 8MiB elapse 622ms, 13170 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 357ms, 22946 KB/S


# 模块频率 297MHz 6分频 DMA 双线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 3004ms, 2727 KB/S
read 8MiB elapse 1036ms, 7907 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 357ms, 22946 KB/S



# 模块频率 297MHz 6分频 CPU 双线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 2991ms, 2738 KB/S
read 8MiB elapse 923ms, 8875 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 358ms, 22882 KB/S


# 模块频率 297MHz 6分频 CPU 单线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 2990ms, 2739 KB/S
read 8MiB elapse 1592ms, 5145 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 360ms, 22755 KB/S


```


## 8 分频
- spinor
```
模块频率 297MHz，8分频，CPU 单线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 30199ms, 542 KB/S
read 16MiB elapse 3615ms, 4532 KB/S
erase 16MiB elapse 46430ms, 352 KB/S



模块频率 297MHz，8分频，CPU 双线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 30199ms, 542 KB/S
read 16MiB elapse 1808ms, 9061 KB/S
erase 16MiB elapse 46376ms, 353 KB/S



模块频率 297MHz，8分频，CPU 四线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 27517ms, 595 KB/S
read 16MiB elapse 904ms, 18123 KB/S
erase 16MiB elapse 46365ms, 353 KB/S



模块频率 297MHz，8分频，DMA 双线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 30289ms, 540 KB/S
read 16MiB elapse 1810ms, 9051 KB/S
erase 16MiB elapse 46288ms, 353 KB/S



模块频率 297MHz，8分频，DMA 四线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 28528ms, 574 KB/S
read 16MiB elapse 906ms, 18083 KB/S
erase 16MiB elapse 46406ms, 353 KB/S


```

- spinand
```
# 模块频率 297MHz 8分频 DMA 四线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 2100ms, 3900 KB/S
read 8MiB elapse 820ms, 9990 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 360ms, 22755 KB/S



# 模块频率 297MHz 8分频 CPU 四线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 2035ms, 4025 KB/S
read 8MiB elapse 696ms, 11770 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 359ms, 22818 KB/S


# 模块频率 297MHz 8分频 DMA 双线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 3489ms, 2347 KB/S
read 8MiB elapse 1268ms, 6460 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 360ms, 22755 KB/S


# 模块频率 297Mhz 8分频 CPU 双线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 3476ms, 2356 KB/S
read 8MiB elapse 1142ms, 7173 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 359ms, 22818 KB/S


# 模块频率 297Mhz 8分频 CPU 单线
boot> flash speedtest
erase flash size 8MiB
Erasing at 0x007e0000 -- 100% complete.
erase OK
program 8MiB elapse 3475ms, 2357 KB/S
read 8MiB elapse 2050ms, 3996 KB/S
Erasing at 0x007e0000 -- 100% complete.
erase OK
erase 8MiB elapse 360ms, 22755 KB/S


```


## 12 分频
```
# CPU 双线
boot> flash speedtest
protect len: 0x0
erase flash size 16MiB
program 16MiB elapse 29948ms, 547 KB/S
read 16MiB elapse 2712ms, 6041 KB/S
erase 16MiB elapse 44089ms, 371 KB/S



```



# 性能对比：Cygnus、Canopus、Vega
- Canopus、Vega 729MHz
- Virgo：500 多 MHz

# ecos、linux
- spinor
- 读写擦测试
	- linux 双线、四线读写擦测试通过
- 性能测试
```
4分频:74.25MHz  4倍速 开 dma
[root@gx6602 gx]# dd if=/dev/mtd4 of=/dev/null bs=1024 count=1024
1024+0 records in
1024+0 records out
1048576 bytes (1.0MB) copied, 0.333217 seconds, 3.0MB/s
[root@gx6602 gx]# dd if=/dev/mtd4 of=/dev/null bs=1MB count=1
1+0 records in
1+0 records out
1000000 bytes (976.6KB) copied, 0.035759 seconds, 26.7MB/s
```


- spinand
- 读写擦测试
	- linux 双线、四线读写擦测试通过
- 性能测试
```
4分频：74.25MHz 2倍速 开 dma
[root@gx6602 tmp]# dd if=/dev/mtd3 of=/dev/null bs=1M count=5
5+0 records in
5+0 records out
5242880 bytes (5.0MB) copied, 1.125873 seconds, 4.4MB/s
[root@gx6602 tmp]# dd if=/dev/mtd3 of=/dev/null bs=1M count=5
5+0 records in
5+0 records out
5242880 bytes (5.0MB) copied, 1.123456 seconds, 4.5MB/s


4分频：74.25MHz 4倍速 开 dma
[root@gx6602 ~]# dd if=/dev/mtd3 of=/dev/null bs=1M count=5
5+0 records in
5+0 records out
5242880 bytes (5.0MB) copied, 0.996193 seconds, 5.0MB/s
[root@gx6602 ~]# dd if=/dev/mtd3 of=/dev/null bs=1M count=5
5+0 records in
5+0 records out
5242880 bytes (5.0MB) copied, 0.977621 seconds, 5.1MB/s

```


# 32MB nor flash 支持 uboot、linux4.19

- gxloader 不需要修改
- gxmisc 需要支持 linux4.19 的编译
- uboot 不需要修改，需要确认设备树如何配置
- linux4.19 需要添加 spi、flash 的驱动


- uboot 用的老的，导致网络不通
- linux 跑 spi 驱动会挂掉，说是访问了  0 地址
	- 设备树缺少属性，用的是 uboot 的设备树，把 4.9 上的信息加上去之后没这个问题
	- 编完 linux 之后需要重新烧 dtb，dtb 用的是 4.19 目录下的 virgo-mpw. dtsi
	- 需要在 compatible 中把 uboot、linux 两个的都写上
- linux 需要增加 Kconfig
	- 先编译生成 .config，然后 make menuconfig 修改之后将这个 config 作为默认的 config
	- 操作流程：
		```shell
		make menuconfig
		make savedefconfig
		
		cp defconfig arch/<your_arch>/configs/aaa_defconfig
		```
- 使用 mtd_debug 工具读写 flash 出现了 Receive FIO overflow，查看下去发现是用的 fast_read 接口，这是单线才会用到的
	- 打印看下，为什么会用单线的接口，确认下 SPI_READ 的宏


## 修改
- linux：
- gxmisc：
	- 增加 spinor_linux_arm64_defconfig
	- spinor. c 修改编译报错
	- build 增加 linux 的编译
	- Makefile 修改 linux 的目录
- 工作频率：
	- gxloader 跑的就是 25MHz，后面会升到 50MHz
	- uboot 中 spi 模块频率是 297MHz，flash 跑 25MHz
	- linux 里面不会重新配频率，接着跑 25MHz
- 设备树：
	- 时钟频率只有 uboot 会用到，所以 uboot 需要配置一个可靠的频率，和相位，linux 不会重新配置频率和相位
	- 需要配置 spi-max-frequency，这是说明 spi 工作的最高频率
	- flash 的最高工作频率看起来好像不用配置
	- 时钟关系：
		- spi-max-frequency 频率在 spi_post_probe 的时候会配到 `struct dm_spi_bus spi->max_hz`，这里就是 25MHz
		- slave->max_hz 为空，所以 speed = spi->max_hz，就也是 25MHz 了，然后调用 set_speed 回调配置 baud 寄存器
		- 还有一个问题：flash 的节点看上去好像并不需要配置频率啊
			- 需要配置，因为 spi_slave_of_to_plat 接口中回去解析这些属性
```c
int spi_slave_of_to_plat(struct udevice *dev, struct dm_spi_slave_plat *plat)
{
	int mode = 0;
	int value;

	plat->cs = dev_read_u32_default(dev, "reg", -1);
	plat->max_hz = dev_read_u32_default(dev, "spi-max-frequency",
					    SPI_DEFAULT_SPEED_HZ);
	if (dev_read_bool(dev, "spi-cpol"))
		mode |= SPI_CPOL;
	if (dev_read_bool(dev, "spi-cpha"))
		mode |= SPI_CPHA;
	if (dev_read_bool(dev, "spi-cs-high"))
		mode |= SPI_CS_HIGH;
	if (dev_read_bool(dev, "spi-3wire"))
		mode |= SPI_3WIRE;
	if (dev_read_bool(dev, "spi-half-duplex"))
		mode |= SPI_PREAMBLE;

	/* Device DUAL/QUAD mode */
	value = dev_read_u32_default(dev, "spi-tx-bus-width", 1);
	switch (value) {
	case 1:
		break;
	case 2:
		mode |= SPI_TX_DUAL;
		break;
	case 4:
		mode |= SPI_TX_QUAD;
		break;
	case 8:
		mode |= SPI_TX_OCTAL;
		break;
	default:
		warn_non_spl("spi-tx-bus-width %d not supported\n", value);
		break;
	}

	value = dev_read_u32_default(dev, "spi-rx-bus-width", 1);
	switch (value) {
	case 1:
		break;
	case 2:
		mode |= SPI_RX_DUAL;
		break;
	case 4:
		mode |= SPI_RX_QUAD;
		break;
	case 8:
		mode |= SPI_RX_OCTAL;
		break;
	default:
		warn_non_spl("spi-rx-bus-width %d not supported\n", value);
		break;
	}

	plat->mode = mode;

	return 0;
}
```



代码移到 nationalchip 目录下：
- 无法注册 mtd 设备，看样子好像是执行了 linux 内核中的 m25p80 注册函数去了
	- 难道是 spi 的驱动和 flash 的驱动不能变成一个 .o 吗？
- 看起来是 spi0 被谁占用了，把设备树去掉，就正常了
- 应该是顺序问题，**即使 `compatible` 属性没有匹配到任何驱动，这个设备树节点依然会成功注册为一个 `spi_device`，并且占用其 `reg` 属性中指定的CS号**
- spinand 的玩法：
	- 设备树也是指定 0
	- 打开了 spinand 驱动的时候就没打开 spinor 驱动，那么 spinor 的驱动没跑，spinand 就能拿到
	- 打开了 spinor 驱动的时候就没打开  spinand 驱动，那么 spinor 驱动先跑的，spinor 就拿到了
- spinor 根据 linux 版本号的方式，使用设备树

- ddr4 的板子有一块起不来
	- 需要用 gxbin 目录下的 bl3x. bin


## 测试
- gxloader
	- 双线读写擦测试：137 次 ok

- uboot
	- 单线读写擦测试：
	- 速度：ok
	- 保存 env：ok
	- 没有 mtdparts：ok
- linux
	- 双线读写擦测试：
	- 速度：



# todo
- 和 4.9 的 spi 驱动对比一下
	- 没什么差异



# 问题

## 【已解决】SPINand 启动 Linux4.9 时报访问了 0 地址
- 更新了 gxmisc 库就好了



## MXIC MX66UM2G45G 测试改动
- gxmisc 打补丁： https://git.nationalchip.com/gerrit/#/c/119584/
- gxloader 改动：引脚复用使用 GPIO_B01 ~ GPIO_B10
```diff
-- a/conf/virgo/3502-D4/boot.config
+++ b/conf/virgo/3502-D4/boot.config
@@ -150,8 +150,8 @@ CONFIG_UART_TYPE = DW
 CONFIG_MULPIN_VERIFY = y
 
 # if you want to use quad/octal mode to read/write flash, please enable quad or octal
-ENABLE_SPI_QUAD = y
-ENABLE_SPI_OCTAL = n
+ENABLE_SPI_QUAD = n
+ENABLE_SPI_OCTAL = y
 
 CONFIG_24M_XTAL = y
 CONFIG_SARADC = n
diff --git a/drivers/flash/flash.c b/drivers/flash/flash.c
index cda294758..a331a20c1 100644
--- a/drivers/flash/flash.c
+++ b/drivers/flash/flash.c
@@ -34,7 +34,7 @@ inline int gxflash_init(void)
 {
        int magic = 0;
        int ret     = -1;
-       int cs = 0;
+       int cs = 1;
 
 #ifdef CONFIG_ENABLE_EMMCFLASH
        if (gx_sdmmc_init(GX_SDMMC_DEV_EMMC) == 0)

```
