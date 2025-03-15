# DW RTC 芯片测试文档

## 1. 测试环境
- FPGA 版本：`394932 peri_final3_gpio_FPGA_2024-12-03.tar`
- VU9P

## 2. 测试用例

### 2.1 1Hz 计数测试
1. 测试步骤：
	- 初始化 RTC
	- 观察内部计数器变化
2. 预期结果：
	- RTC 内部计数器每秒计数一次
3. 测试命令：
	```bash
	rtc init
	rtc read
	```

### 2.2 分频配置测试
1. 测试步骤：
	- 初始化 RTC
	- 观察计数器变化
2. 预期结果：
	- 默认开启预分频
	- 计数器每秒变化一次
3. 测试命令：
	```bash
	rtc init
	rtc read
	```

### 2.3 闹钟功能测试
1. 测试步骤：
	- 初始化 RTC
	- 设置闹钟时间
	- 等待闹钟触发
2. 预期结果：
	- 闹钟可以正常触发
	- 触发时间准确
3. 测试命令：
	```bash
	rtc init
	rtc set_alarm 10
	```
4. 注意事项：
	- 需要修改闹钟设置代码
	- Virgo 有预分频，不需要乘以 `GX_RTC_FRE`

## 3. 测试结果
- [ ] 1Hz 计数测试
- [ ] 分频配置测试
- [ ] 闹钟功能测试


## 4. 测试记录
### 1. 闹钟功能问题
1. 问题描述：
	- 设定闹钟后不会触发
2. 原因分析：
	- Virgo 有预分频，不需要乘以 `GX_RTC_FRE`
	- 需要根据 `cpsr_enable` 判断是否需要乘以 `GX_RTC_FRE`
3. 解决方案：
	- 修改 `gx_rtc_set_alarm_time_s` 函数
	- 根据 `cpsr_enable` 判断是否需要乘以 `GX_RTC_FRE`
	- 代码修改如下：
```diff
--- a/drivers/rtc/dw_rtc.c
+++ b/drivers/rtc/dw_rtc.c
@@ -94,7 +94,10 @@ int gx_rtc_set_alarm_time_s(int alarm_after_s)

		gx_rtc_get_tick(&cur_time);
		rtc_dev.alarm_ctrl = GX_HAL_RTC_ALARM_CTRL_OPEN;
-       rtc_dev.alarm_time = cur_time + alarm_after_s * GX_RTC_FRE;
+       if (rtc_dev.cpsr_enable)
+               rtc_dev.alarm_time = cur_time + alarm_after_s;
+       else
+               rtc_dev.alarm_time = cur_time + alarm_after_s * GX_RTC_FRE;
```


