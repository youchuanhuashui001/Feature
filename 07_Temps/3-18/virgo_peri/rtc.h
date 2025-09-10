#ifndef __RTC_H__
#define __RTC_H__

/**
 * RTC(实时时钟)驱动接口
 */

/**
 * 初始化RTC设备
 */
void gx_rtc_init(void);

/**
 * 启动RTC计时
 */
void gx_rtc_start_tick(void);

/**
 * 停止RTC计时
 */
void gx_rtc_stop_tick(void);

/**
 * 设置RTC时间
 *
 * @param time_duration 时间值(秒)
 */
void gx_rtc_set_sec(unsigned int time_duration);

/**
 * 获取当前RTC时间
 *
 * @param time_duration 输出参数,用于存储当前时间值(秒)
 */
void gx_rtc_get_sec(unsigned int *time_duration);

/**
 * 设置RTC tick值
 *
 * @param time_duration tick值
 */
void gx_rtc_set_tick(unsigned int time_duration);

/**
 * 获取当前RTC tick值
 *
 * @param time_duration 输出参数,用于存储当前tick值
 */
void gx_rtc_get_tick(unsigned int *time_duration);

/**
 * 设置RTC中断回调函数
 *
 * @param callback 中断触发时的回调函数
 * @param priv 传递给回调函数的私有数据
 * @return 0表示成功,其他值表示失败
 */
int gx_rtc_set_interrupt(int (*callback)(void*), void *priv);

/**
 * 设置RTC闹钟时间
 *
 * @param alarm_after_s 多少秒后触发闹钟
 * @return 0表示成功,其他值表示失败
 */
int gx_rtc_set_alarm_time_s(int alarm_after_s);

/**
 * 退出RTC,释放相关资源
 *
 * @return 0表示成功,其他值表示失败
 */
int gx_rtc_exit(void);

#endif /* __RTC_H__ */
