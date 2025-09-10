#include <stdio.h>
#include <config.h>
#include <io.h>
#include <types.h>
#include <interrupt.h>
#include <div64.h>

#include "gx_hal_rtc.h"

#define GX_RTC_FRE 32768

#if defined(CONFIG_ARCH_ARM_VIRGO)
#define IRQ_NUM_RTC 59
#endif

static GX_HAL_RTC rtc_dev = {
	.regs = (void *)REG_BASE_RTC,
	.pclk = GX_RTC_FRE,
	.cpsr_enable = 1, // apus 没有该功能，只能配置为 0
};

static int (*rtc_callback_entry)(void*) = NULL;

static enum interrupt_return rtc_isr(uint32_t irq, void *pdata)
{
	gx_hal_rtc_irq_handler(&rtc_dev);

	return 0;
}

static int rtc_callback(GX_HAL_RTC *dev, void *pdata)
{
	if(rtc_callback_entry)
		rtc_callback_entry(pdata);

	return 0;
}

void gx_rtc_start_tick(void)
{
	gx_hal_rtc_start(&rtc_dev);
}

void gx_rtc_stop_tick(void)
{
	gx_hal_rtc_stop(&rtc_dev);
}

void gx_rtc_set_sec(unsigned int time_duration)
{
	gx_hal_rtc_set_time(&rtc_dev, time_duration);
}

void gx_rtc_get_sec(unsigned int *time_duration)
{
	gx_hal_rtc_get_time(&rtc_dev, time_duration);
}

void gx_rtc_set_tick(unsigned int time_duration)
{
	gx_hal_rtc_set_tick(&rtc_dev, time_duration);
}

void gx_rtc_get_tick(unsigned int *time_duration)
{
	gx_hal_rtc_get_tick(&rtc_dev, time_duration);
}


void gx_rtc_init(void)
{
	gx_request_interrupt(IRQ_NUM_RTC, IRQ, rtc_isr, NULL);
	gx_hal_rtc_init(&rtc_dev);
}

int gx_rtc_set_interrupt(int (*callback)(void*), void *priv)
{

	rtc_dev.alarm_ctrl = GX_HAL_RTC_ALARM_CTRL_CLOSE;
	gx_hal_rtc_set_alarm(&rtc_dev);

	// 填充中断回调函数
	rtc_callback_entry = callback;
	rtc_dev.alarm_callback = rtc_callback;
	rtc_dev.pdata = priv;

	return 0;
}

int gx_rtc_set_alarm_time_s(int alarm_after_s)
{
	hal_status hal_ret = HAL_OK;
	unsigned int cur_time = 0;

	gx_rtc_get_tick(&cur_time);
	rtc_dev.alarm_ctrl = GX_HAL_RTC_ALARM_CTRL_OPEN;
	if (rtc_dev.cpsr_enable)
		rtc_dev.alarm_time = cur_time + alarm_after_s;
	else
		rtc_dev.alarm_time = cur_time + alarm_after_s * GX_RTC_FRE;
	hal_ret = gx_hal_rtc_set_alarm(&rtc_dev);

	return hal_ret;
}

int gx_rtc_exit(void)
{
	gx_hal_rtc_deinit(&rtc_dev);

	return 0;
}
