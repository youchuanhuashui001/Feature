# 测试用例
- 产生 1Hz 计数
	- rtc 内部计数器 1s 计数一次
- 分频配置
	- 默认开了预分频
	- `rtc init`
	- `rtc read` 1s 记一次数的话，这里两个测试项就都测到了
- 开启闹钟功能
	- `rtc init`
	- `rtc set_alarm 10`








# Q&&A

## Q：设定 alarm 不会触发闹钟
执行命令如下：
```
rtc init
rtc set_alarm 10
```
设定闹钟的代码需要修改，因为 apus 没有预分频，所以这里需要 `*GX_RTC_FRE`，但是 virgo 有预分频，这里就不能开
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
