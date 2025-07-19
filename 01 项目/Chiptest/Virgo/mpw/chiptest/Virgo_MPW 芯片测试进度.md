

# 总览
```dataview
TABLE
    总验证点,
    已完成,
    备注,
    (总验证点 - 已完成) AS "未完成",
    "<progress value='" + (已完成 / 总验证点 * 100) + "' max='100'></progress>" + " " + round(已完成 / 总验证点 * 100) + "%" AS "验证进度"
FROM #Virgo_MPW模块验证 
SORT 已完成 / 总验证点 DESC
```




# Flash_SPI 模块
- 6.22
	- 简单过了单线、双线、四线读写擦，297MHz 模块频率，12 分频，跑在 24.75MHZ 的频率下，测试一把相位范围：0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
- 6.30
	- nor flash gxloader 剩下多 cs、8 线 flash 没有测试，性能测试都测过了，需要 ecos、linux 都跑一下
	- gxloader 需要切频率到 50MHz，现在跑的是 25MHz



# GEN_SPI 模块
- 6.30
	- 简单通了下通路




# AXI-DMA 模块



# IR 模块
验证进度：100%

- 7.1
	- 模块验证完成




# UART 模块



# I2C 模块
- 6.26
	- gxloader 剩下 3.4MHz、1MHz 没有测试，提交了补丁
	- ecos 应用测试就可以了，提交了补丁，可以用了
	- linux 提交了补丁，使用 i2c-tools 可以正常用
- 7.1
	- 除高速模式 3.4MHz 无硬件无法验证外，其余功能都验证完成


# WATCHDOG 模块



# TIMER 模块
- 6.22
	- 输出 pwm 控制 dc-dc 电路给 cpu 供电，要求能够动态调整 cpu 工作电压


# RTC 模块


# GPIO 模块
- 需要全功能板
