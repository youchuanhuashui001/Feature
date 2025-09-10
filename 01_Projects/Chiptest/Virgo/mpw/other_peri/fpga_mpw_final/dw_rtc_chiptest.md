# DW RTC 芯片测试文档

## 1. 测试环境
- FPGA 版本：`382541 9p_peri_newcode_FPGA_2024-09-03.tar`
- VU9P

## 2. 测试用例

### 2.1 1Hz 计数测试
1. 测试步骤：
	- 配置 `ENABLE_RTC = y;ENABLE_IRQ = y`
	- `rtc init`
	- `rtc read`
	- 观察打印变化
2. 预期结果：
	- RTC 内部计数器每秒计数一次

### 2.2 分频配置测试
1. 测试步骤：
	- 配置 `ENABLE_RTC = y;ENABLE_IRQ = y`
	- `rtc init`
	- `rtc read`
	- 观察打印变化
2. 预期结果：
	- 默认开启预分频
	- 计数器每秒变化一次

### 2.3 闹钟功能测试
1. 测试步骤：
	- 配置 `ENABLE_RTC = y;ENABLE_IRQ = y`
	- `rtc init`
	- `rtc set_alarm 10`
	- 观察闹钟中断打印
2. 预期结果：
	- 闹钟可以正常触发
	- 触发时间准确
3. 测试命令：
	```bash
	rtc init
	rtc set_alarm 10
	```
4. 注意事项：
	- 需要修改闹钟测试代码代码
	- Virgo 有预分频，不需要乘以 `GX_RTC_FRE`
	- 中断只会调用一次，因为闹钟的设定是单次的
```diff
--- a/bootmenu.c
+++ b/bootmenu.c
@@ -288,6 +288,12 @@ void command_wdtest(int argc, const char **argv)
 
 #ifdef CONFIG_ENABLE_RTC
 COMMAND(rtc,command_rtc, "[init/read/write/set_alarm]");
+static int test_rtc_callback(void)
+{
+       printf("rtc callback.\n");
+       return 0;
+}
+
 void command_rtc(int argc, const char **argv)
 {
        unsigned int time = 0;
@@ -303,6 +309,7 @@ void command_rtc(int argc, const char **argv)
                gx_rtc_set_tick(time);
        }else if(strcmp(argv[1], "set_alarm") == 0){
                time = atoi(argv[2]);
+               gx_rtc_set_interrupt(test_rtc_callback, NULL);
                gx_rtc_set_alarm_time_s(time);
        }

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


## 3. 测试结果
- [x] 1Hz 计数测试
- [x] 分频配置测试
- [x] 闹钟功能测试


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


