# DW WDT 芯片测试文档

## 1. 测试环境
- FPGA 版本：`394932 peri_final3_gpio_FPGA_2024-12-03.tar`
- VU9P

## 2. 测试用例

### 2.1 复位功能测试
1. 测试命令：
```c
reboot
```
2. 预期结果：
	- FPGA 可以产生复位
	- 系统可以正常重启
3. 注意事项：
	- 默认配置为最大值，会一直计数
	- 每次喂狗（写 0x76）会将计数器清零
	- 如果不喂狗会溢出，溢出后会产生中断或复位

### 2.2 喂狗功能测试
1. 测试步骤：
	- 在 `bootmenu.c` 中添加喂狗代码
	- 使能 WDT 中断
	- 观察系统行为
2. 预期结果：
	- 系统正常运行
	- 不会触发复位

### 2.3 中断使能测试
1. 测试步骤：
	- 配置 `CONFIG_ENABLE_IRQ`
	- 使能 WDT 中断
2. 预期结果：
	- 中断可以正常使能
	- 可以正常触发中断

### 2.4 定时器值读取测试
1. 测试步骤：
	- 手动读取寄存器值
	- 观察计数器变化
2. 预期结果：
	- 寄存器值实时递增
	- 喂狗后计数器清零
3. 注意事项：
	- 目前没有驱动代码
	- 需要手动读取寄存器




## 3. 测试结果
- [ ] 复位功能测试
- [ ] 喂狗功能测试
- [ ] 中断使能测试
- [ ] 定时器值读取测试
- [ ] 定时精度测试




## 4. 测试记录
### 1 . GDB 调试问题：gdb 跑不了，实际烧程序进去也跑不起来
```
X23eRUNFPGA0xF090001C : 0x0
0xfa4900cc : 0x4
chip spdif powercfg not config
```
- 回退到以下版本可以正常启动：
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


### 2 . 中断时间精度问题：irq 时间不准
- 5s 定时较为准确
- 8s 定时会有偏差（约 11.18s）
- 这是由于定时器配置的计算方式导致的 
- 定时时间计算公式：`(1 << (16 + top)) / dev->pclk`
- 8s 配置：
	- top = 12：`((1<<12+16)) / 24000000 = 11.1848s`
	- top = 11：`(1<<27) / 24000000 = 5.5924s`（不够要求）


