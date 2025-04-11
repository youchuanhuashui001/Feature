


# 测试用例
- 复位：
	- fpga 无法产生复位，验证可以产生中断即可
	- **我测试用 fpga 可以重启，这里的复位不是我理解的复位？**
	- 是的，可以重启就是复位了
- 喂狗：
	- 没有代码喂狗
	- 打开中断后，把代码在 bootmenu. c 中调用一下就行了
- 使能中断：
	- 配置 `CONFIG_ENABLE_IRQ`
- 读取定时器值：
	- 没有驱动
	- 手动看寄存器，有在实时递增
	- 不会溢出吗？
		- 每次喂狗，也就是写 0x76 的时候会喂狗，会把计数器清 0
		- 如果没有喂狗会溢出，溢出了就会产生中断或这复位，默认配的是最大值，一直计数



# Q&&A
## Q：gdb 跑不了，实际烧程序进去也跑不起来
```
X23eRUNFPGA0xF090001C : 0x0
0xfa4900cc : 0x4
chip spdif powercfg not config
```
- 回退后可以正常启动：
```
commit 0de76170c39058e69f6169ed12a0004df590b712 (HEAD -> final)
Author: tanxzh <tanxzh@nationalchip.com>
Date:   Thu Nov 7 16:05:56 2024 +0800

    377384: Virgo IRR 新增低功耗模式
    
    Change-Id: I5b197023a2294eeb3ab39c948c358c13dd3d4b22

commit ab4774558b5bb25fc1f2c0e0c9049c72a155a573
Author: zhoulong <zhoulong@nationalchip.com>
Date:   Fri Nov 29 17:13:42 2024 +0800

    395033: gxloader 支持管脚复用配置
    
    Change-Id: I6e1059accfa95f70635e5f9ced6b43f48521dec7

commit 0d06f03802cece9cc16604609da1587dacfb46ff
Merge: 01834daf 54dc38ad
Author: 叶妙兵 <yemb@nationalchip.com>
Date:   Wed Dec 4 13:32:38 2024 +0800

    Merge "362611: virgo eth final 寄存器地址变更" into develop2

commit 01834daf76eb850a353f5a9a5b8cde184a5b54f0
Author: yangmch <yangmch@nationalchip.com>
Date:   Tue Dec 3 14:07:12 2024 +0800

    374659: 1. 添加dma配置block size参数，适配saradc传输
            2. 关掉 dma lowpower 功能
    
    Change-Id: I55fcbd53cbf28545c23058d32ebedfa41e53dc80
```



## Q：irq 时间不准 
- 5s 的时候还挺准的
- 8s 的时候不准
![[Pasted image 20250313155808.png]]

- 正常的，定时时间是这样算的
```
(1 << (16 + top)) / dev->pclk
```
- 上面的值大于要设定的定时值即可：
	- 8s 的时候：top = 12，所以计算出来的值是 `((1<<12+16)) / 24000000 = 11.1848`，实际确实是 11.18s 左右
	- 如果 top = 11，计算出来的值 `(1<<27) / 24000000 = 5.5924` 不够要求