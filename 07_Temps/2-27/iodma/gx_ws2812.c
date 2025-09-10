#include <stdio.h>
#include <stdint.h>
#include <soc.h>
#include <base_addr.h>
#include "gx_ws2812.h"
#include "gx_iodma.h"
#include "gx_delay.h"
#include "gx_hal_iodma.h"
#include <string.h>

#define WS2812_BITS_PER_LED    24
#define WS2812_BIT_0           0x000002aa
#define WS2811_BIT_1           0x0000aaaa

#define WS2812_RESET_BITS      0x00000000

// 发送 GRB 数据之前先发送 300 个 Reset 信号，确保发送 GRB 时的环境干净
static uint32_t led_buffer[WS2812_MAX_LEDS * WS2812_BITS_PER_LED + 301] __attribute__((aligned(4)));
static GX_WS2812_RGB led_colors[WS2812_MAX_LEDS];
static uint8_t brightness = 100;
static GX_IODMA_TX_CONFIG config = {0};

static uint32_t ws2812_convert_bit(uint8_t bit)
{
	return bit ? WS2811_BIT_1 : WS2812_BIT_0;
}

static void ws2812_convert_byte(uint8_t byte, uint32_t *buffer)
{
	int i;

	for (i = 0; i < 8; i++) {
		buffer[7-i] = ws2812_convert_bit((byte >> i) & 0x01);
	}
}

static void ws2812_convert_rgb(uint8_t r, uint8_t g, uint8_t b, uint32_t *buffer)
{
	r = (r * brightness) / 100;
	g = (g * brightness) / 100;
	b = (b * brightness) / 100;

	ws2812_convert_byte(g, buffer);
	ws2812_convert_byte(r, buffer + 8);
	ws2812_convert_byte(b, buffer + 16);
}

static uint32_t ws2812_get_delay_by_freq(uint32_t freq_hz)
{
	switch (freq_hz) {
		case 32000000: return 60;
		case 48000000: return xx;
		case 64000000: return xx;
		default: return 60;
	}
}

void gx_ws2812_init(void)
{
	int i = 0;

	gx_iodma_init();
	for (i = 0; i < 300; i++) {
		led_buffer[i] = WS2812_RESET_BITS;
	}

	for (i = 300; i < WS2812_MAX_LEDS * WS2812_BITS_PER_LED; i++) {
		led_buffer[i] = WS2812_BIT_0;
	}

	led_buffer[WS2812_MAX_LEDS * WS2812_BITS_PER_LED] = WS2812_RESET_BITS;

	memset(led_colors, 0, sizeof(led_colors));

	config.io_width = GX_IODMA_IO_WIDTH_2BIT;
	config.mode = GX_IODMA_MODE_FIXED;
	config.addr = (uint32_t)led_buffer;
	// TODO: 这里的周期需要确认
//	config.delay = 60;  // 125ns delay  2 周期
	config.delay = ws2812_get_delay_by_freq(get_frequency(iodma));
	config.len = sizeof(led_buffer);

	gx_iodma_config(&config);
	gx_iodma_start();

	gx_mdelay(1);
}

void gx_ws2812_setcolor(int index, GX_WS2812_RGB color)
{
	if (index >= WS2812_MAX_LEDS) return;

	led_colors[index] = color;
	ws2812_convert_rgb(color.r, color.g, color.b,
		led_buffer + 300 + (index * WS2812_BITS_PER_LED));
}

void gx_ws2812_setall(GX_WS2812_RGB color)
{
	for (int i = 0; i < WS2812_MAX_LEDS; i++) {
		gx_ws2812_setcolor(i, color);
	}
}

void gx_ws2812_update(void)
{
//	led_buffer[WS2812_MAX_LEDS * WS2812_BITS_PER_LED] = WS2812_RESET_BITS;

	config.len = sizeof(led_buffer);

	gx_iodma_config(&config);
	gx_iodma_start();

	gx_mdelay(1);
}

void gx_ws2812_setbrightness(int percent)
{
	if (percent > 100) percent = 100;
	brightness = percent;

	for (int i = 0; i < WS2812_MAX_LEDS; i++) {
		gx_ws2812_setcolor(i, led_colors[i]);
	}
}

void gx_ws2812_breath(GX_WS2812_RGB base, int period_ms)
{
	uint8_t brightness;
	uint16_t steps = 100;
	uint16_t delay_per_step = period_ms / steps;

	while (1) {
		/* Fade in */
		for (brightness = 0; brightness <= 100; brightness++) {
			gx_ws2812_setbrightness(brightness);
			gx_ws2812_setall(base);
			gx_ws2812_update();
			gx_mdelay(delay_per_step);
		}

		/* Fade out */
		for (brightness = 100; brightness > 0; brightness--) {
			gx_ws2812_setbrightness(brightness);
			gx_ws2812_setall(base);
			gx_ws2812_update();
			gx_mdelay(delay_per_step);
		}
	}
}

static GX_WS2812_RGB hsv_to_rgb(uint16_t hue, uint8_t sat, uint8_t val)
{
	GX_WS2812_RGB rgb = {0};
	uint8_t hi = (hue / 60) % 6;
	uint8_t f = (hue % 60);
	uint8_t p = (val * (255 - sat)) / 255;
	uint8_t q = (val * (255 - (sat * f) / 60)) / 255;
	uint8_t t = (val * (255 - (sat * (60 - f)) / 60)) / 255;

	switch (hi) {
		case 0: rgb.r = val; rgb.g = t; rgb.b = p; break;
		case 1: rgb.r = q; rgb.g = val; rgb.b = p; break;
		case 2: rgb.r = p; rgb.g = val; rgb.b = t; break;
		case 3: rgb.r = p; rgb.g = q; rgb.b = val; break;
		case 4: rgb.r = t; rgb.g = p; rgb.b = val; break;
		case 5: rgb.r = val; rgb.g = p; rgb.b = q; break;
	}
	return rgb;
}

void gx_ws2812_rainbow(int speed)
{
	uint16_t hue = 0;

	while (1) {
		for (int i = 0; i < WS2812_MAX_LEDS; i++) {
			uint16_t led_hue = (hue + (i * 360 / WS2812_MAX_LEDS)) % 360;
			GX_WS2812_RGB color = hsv_to_rgb(led_hue, 255, 255);
			gx_ws2812_setcolor(i, color);
		}
		gx_ws2812_update();

		hue = (hue + speed) % 360;
		gx_mdelay(50);
	}
}
