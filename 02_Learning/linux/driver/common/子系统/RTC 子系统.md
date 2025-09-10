
# 测试
参考链接：
> https://www.cnblogs.com/waterzhm/p/13210846.html


- 先看内核打印驱动开起来了没有
	- `dmesg | grep -i rtc`
- 显示当前时间
	- `date`
- 设置当前时间
```shell
date -s "2021-1-1 10:10:10"  时间格式形如xxxx-xx-xx xx:xx:xx`
root@zzz-VirtualBox:/# date -s "2021-1-1 10:10:10"
2021年 01月 01日 星期五 10:10:10 CST
```
- 同步系统时间到rtc时间
	- `hwclock -w`
- 设置闹钟为当前rtc时间的20s后
	- `echo +20 /sys/class/rtc/rtc0/wakealarm`
- 查看RTC属性
	- `cat /proc/driver/rtc`


```shell
mount debugfs /sys/kernel/debug/ -t debugfs
cat /sys/kernel/debug/clk/clk_summary | grep rtc
       rtc                            1        1        0       31970          0     0  50000


[2025-08-02 14:21:11.604] [    3.413554] xgene-rtc fa460000.rtc: setting system clock to 1970-01-01 00:00:00 UTC (0)

# cat /proc/driver/rtc 
[2025-08-02 14:44:00.318] rtc_time      : 00:22:48
[2025-08-02 14:44:00.318] rtc_date      : 1970-01-01
[2025-08-02 14:44:00.319] alrm_time     : 00:00:00
[2025-08-02 14:44:00.319] alrm_date     : 1970-01-01
[2025-08-02 14:44:00.319] alarm_IRQ     : no
[2025-08-02 14:44:00.319] alrm_pending  : no
[2025-08-02 14:44:00.320] update IRQ enabled    : no
[2025-08-02 14:44:00.320] periodic IRQ enabled  : no
[2025-08-02 14:44:00.320] periodic IRQ frequency        : 1
[2025-08-02 14:44:00.320] max user IRQ frequency        : 64
[2025-08-02 14:44:00.320] 24hr          : yes
# cat /proc/driver/rtc 
[2025-08-02 14:44:00.606] rtc_time      : 00:22:49

```



- 用户空间测试代码
```c
#include <stdio.h> /*标准输入输出定义*/
#include <stdlib.h> /*标准函数库定义*/
#include <unistd.h> /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h> /*文件控制定义*/
#include <linux/rtc.h> /*RTC支持的CMD*/
#include <errno.h> /*错误号定义*/
#include <string.h>

#define RTC_DEVICE_NAME "/dev/rtc0"

int set_rtc_timer(int fd)
{
	struct rtc_time rtc_tm = {0};
	struct rtc_time rtc_tm_temp = {0};

	rtc_tm.tm_year = 2020 - 1900; /* 需要设置的年份，需要减1900 */
	rtc_tm.tm_mon = 11 - 1; /* 需要设置的月份,需要确保在0-11范围*/
	rtc_tm.tm_mday = 21; /* 需要设置的日期*/
	rtc_tm.tm_hour = 10; /* 需要设置的时间*/
	rtc_tm.tm_min = 12; /* 需要设置的分钟时间*/
	rtc_tm.tm_sec = 30; /* 需要设置的秒数*/

	/* 设置RTC时间*/
	if (ioctl(fd, RTC_SET_TIME, &rtc_tm) < 0) {
		printf("RTC_SET_TIME failed\n");
		return -1;
	}

	/* 获取RTC时间*/
	if (ioctl(fd, RTC_RD_TIME, &rtc_tm_temp) < 0) {
		printf("RTC_RD_TIME failed\n");
		return -1;
	}
	printf("RTC_RD_TIME return %04d-%02d-%02d %02d:%02d:%02d\n",
	rtc_tm_temp.tm_year + 1900, rtc_tm_temp.tm_mon + 1, rtc_tm_temp.tm_mday,
	rtc_tm_temp.tm_hour, rtc_tm_temp.tm_min, rtc_tm_temp.tm_sec);
	return 0;
}

int set_rtc_alarm(int fd)
{
	struct rtc_time rtc_tm = {0};
	struct rtc_time rtc_tm_temp = {0};

	rtc_tm.tm_year = 0; /* 闹钟忽略年设置*/
	rtc_tm.tm_mon = 0; /* 闹钟忽略月设置*/
	rtc_tm.tm_mday = 0; /* 闹钟忽略日期设置*/
	rtc_tm.tm_hour = 10; /* 需要设置的时间*/
	rtc_tm.tm_min = 13; /* 需要设置的分钟时间*/
	rtc_tm.tm_sec = 30; /* 需要设置的秒数*/

	/* set alarm time */
	if (ioctl(fd, RTC_ALM_SET, &rtc_tm) < 0) {
		printf("RTC_ALM_SET failed\n");
		return -1;
	}

	if (ioctl(fd, RTC_AIE_ON) < 0) {
		printf("RTC_AIE_ON failed!\n");
		return -1;
	}

	if (ioctl(fd, RTC_ALM_READ, &rtc_tm_temp) < 0) {
		printf("RTC_ALM_READ failed\n");
		return -1;
	}

	printf("RTC_ALM_READ return %04d-%02d-%02d %02d:%02d:%02d\n",
	rtc_tm_temp.tm_year + 1900, rtc_tm_temp.tm_mon + 1, rtc_tm_temp.tm_mday,
	rtc_tm_temp.tm_hour, rtc_tm_temp.tm_min, rtc_tm_temp.tm_sec);
	return 0;
}

int main(int argc, char *argv[])
{
	int fd;
	int ret;

	/* open rtc device */
	fd = open(RTC_DEVICE_NAME, O_RDWR);
	if (fd < 0) {
		printf("open rtc device %s failed\n", RTC_DEVICE_NAME);
		return -ENODEV;
	}

	/* 设置RTC时间*/
	ret = set_rtc_timer(fd);
	if (ret < 0) {
		printf("set rtc timer error\n");
		return -EINVAL;
	}

	/* 设置闹钟*/
	ret = set_rtc_alarm(fd);
	if (ret < 0) {
		printf("set rtc alarm error\n");
		return -EINVAL;
	}

	close(fd);
	return 0;
}
```


# 问题

## RTC 在 Linux 每次启动的时候，读到的值都是 0
- linux启动之后跑 rtc 驱动，发现 CCVR 每次都是 0
- 启动的逻辑是这样的：
	- 先执行 rtc 驱动的 probe，会执行 rtc_register
	- 从 rtc 中读取当前的时间，然后把这个时间转换成 day 的形式
- 在 loader 中进行测试：
	- 开关 rtc 模块，依然会计数
	- 开关 rtc 模块的 clk 和 pclk，依然会计数
```shell
###### 关闭 clk_gate ######
(gdb) set *0xf0a06024=0x107
(gdb) c
Continuing.
^C
Program received signal SIGTRAP, Trace/breakpoint trap.
uart_getc (port=port@entry=0) at drivers/serial/dw_uart.c:600
600             }while (!(state & (1 << DSW_LSR_DR)));
(gdb) x/x 0xf0a06024
0xf0a06024:     0x00000107
###### 打开 clk_gate ######
(gdb) set *0xf0a06024=0x100
(gdb) c
Continuing.
^C

###### 关闭 clk_gate 和 pclk ######
Program received signal SIGTRAP, Trace/breakpoint trap.
uart_getc (port=port@entry=0) at drivers/serial/dw_uart.c:600
600             }while (!(state & (1 << DSW_LSR_DR)));
(gdb) set *0xf0a06024=0x107
(gdb) x/x 0xf0a06008
0xf0a06008:     0x00000000
(gdb) set *0xf0a06008=0x800
(gdb) c
Continuing.
^C
Program received signal SIGTRAP, Trace/breakpoint trap.
uart_getc (port=port@entry=0) at drivers/serial/dw_uart.c:600
600             }while (!(state & (1 << DSW_LSR_DR)));
###### 打开 clk_gate 和 pclk ######
(gdb) set *0xf0a06008=0x000
(gdb) set *0xf0a06024=0x100
```
- 看门狗复位之后，rtc 的数值丢了
- 需要咨询芯片
