#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "driver/gx_delay.h"
#include "gx_timer.h"
#include "gx_ws2812.h"
#include "gx_delay.h"
#include "gx_padmux.h"
#include "cmd_parser.h"

// 声明 atoi 函数，避免编译警告
extern int atoi(const char *str);

// LED 配置
#define LED_NUM    7    // 默认LED数量

// 全局变量
static int ws2812_init_flag = 0;

// HSV颜色结构体
typedef struct {
    int h;    // 色相 (0-360)
    int s;    // 饱和度 (0-100)
    int v;    // 明度 (0-100)
} HSV_COLOR;

// 预定义颜色 (HSV格式)
static const HSV_COLOR preset_colors[] = {
    {0, 100, 100},     // 红色
    {60, 100, 100},    // 黄色
    {120, 100, 100},   // 绿色
    {180, 100, 100},   // 青色
    {240, 100, 100},   // 蓝色
    {300, 100, 100},   // 紫色
    {0, 0, 100},       // 白色
    {30, 100, 100},    // 橙色
    {330, 100, 100},   // 粉色
    {90, 100, 100},    // 酸橙色
    {160, 100, 100},  // 青绿色
    {270, 100, 100},  // 薰衣草色
    {45, 100, 100},   // 金色
    {30, 20, 100},    // 暖白色
    {210, 5, 100},    // 冷白色
};

// 按键LED状态结构体（用于按键测试）
typedef struct {
    int status;                // 1 表示按下，0 表示松开
    int brightness;            // 记录亮度
} KEY_STATUS;

typedef struct {
	KEY_STATUS key[LED_NUM];
	int h;
	int s;
	int v;
	int steps;
} KEY_PRESS;

KEY_PRESS keypress;

/**
 * @brief HSV转RGB
 */
static GX_WS2812_RGB hsv_to_rgb(int hue, int sat, int val)
{
	GX_WS2812_RGB rgb = {0};

	// 处理边界情况
	if (hue >= 360) {
		hue %= 360;
	}

	// 特殊情况处理
	if (sat == 0) {
		// 灰度值
		rgb.r = rgb.g = rgb.b = (val * 255) / 100;
		return rgb;
	}

	// 将饱和度和明度转换为0-1范围的浮点数
	float f_sat = sat / 100.0f;
	float f_val = val / 100.0f;

	// 色相区间(0-5)
	float f_hue = hue / 60.0f;
	int i = (int)f_hue;
	float f = f_hue - i;

	// 计算RGB的辅助值
	float p = f_val * (1.0f - f_sat);
	float q = f_val * (1.0f - (f_sat * f));
	float t = f_val * (1.0f - (f_sat * (1.0f - f)));

	// 将浮点值转换为0-255范围的整数
	uint8_t p_byte = (uint8_t)(p * 255 + 0.5f);
	uint8_t q_byte = (uint8_t)(q * 255 + 0.5f);
	uint8_t t_byte = (uint8_t)(t * 255 + 0.5f);
	uint8_t v_byte = (uint8_t)(f_val * 255 + 0.5f);

	// 根据色相区间计算RGB值
	switch (i) {
		case 0: rgb.r = v_byte; rgb.g = t_byte; rgb.b = p_byte; break;
		case 1: rgb.r = q_byte; rgb.g = v_byte; rgb.b = p_byte; break;
		case 2: rgb.r = p_byte; rgb.g = v_byte; rgb.b = t_byte; break;
		case 3: rgb.r = p_byte; rgb.g = q_byte; rgb.b = v_byte; break;
		case 4: rgb.r = t_byte; rgb.g = p_byte; rgb.b = v_byte; break;
		default: rgb.r = v_byte; rgb.g = p_byte; rgb.b = q_byte; break;
	}

	return rgb;
}

/**
 * @brief WS2812模块初始化
 */
static int ws2812_test_init(int led_num)
{
    int ret;

    if (led_num <= 0) {
        printf("Invalid LED number: %d.\n", led_num);
        return -1;
    }

    // 配置管脚
#if defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
    padmux_set(25, 46); // P3_1 IODMA[0]
#elif defined(CONFIG_TARGET_SAGITTA_NRE_V1)
    padmux_set(2, 46);
#else
    // 默认配置，可根据实际硬件调整
    padmux_set(25, 46);
#endif

    ret = gx_ws2812_init(led_num);
    if (ret != 0) {
        printf("WS2812 initialization failed: %d\n", ret);
        return ret;
    }

    printf("WS2812 initialized with %d LEDs\n", led_num);

    return 0;
}

/**
 * @brief 确保WS2812已初始化
 */
static int ensure_ws2812_initialized(void)
{
    if (!ws2812_init_flag) {
        if (ws2812_test_init(LED_NUM) == 0) {
            ws2812_init_flag = 1;
            printf("WS2812 module auto-initialized\n");
        } else {
            printf("Failed to initialize WS2812 module\n");
            return -1;
        }
    }
    return 0;
}

/**
 * @brief 固定颜色测试
 */
static int test_fix_color(int h, int s, int v, int mode)
{
    int ret = ensure_ws2812_initialized();
    if (ret != 0) return ret;

    if (mode == 0) {
        // 没有指定HSV，显示预设颜色
        printf("Displaying preset colors...\n");

        int num_colors = sizeof(preset_colors) / sizeof(preset_colors[0]);
        for (int i = 0; i < num_colors; i++) {
            GX_WS2812_RGB color = hsv_to_rgb(preset_colors[i].h,
                                            preset_colors[i].s,
                                            preset_colors[i].v);

            gx_ws2812_setall(color);
            gx_ws2812_update();

            printf("Color %d: HSV(%d, %d, %d)\n",
                   i+1, preset_colors[i].h, preset_colors[i].s, preset_colors[i].v);

            gx_mdelay(1000);
        }
    } else if (mode == 1) {
        // 指定了HSV值
        if (h < 0 || h > 360 || s < 0 || s > 100 || v < 0 || v > 100) {
            printf("Invalid HSV values. H: 0-360, S: 0-100, V: 0-100\n");
            return -1;
        }

        GX_WS2812_RGB color = hsv_to_rgb(h, s, v);
        gx_ws2812_setall(color);
        gx_ws2812_update();

        printf("Fixed color: HSV(%d, %d, %d)\n", h, s, v);
        gx_mdelay(3000); // 显示3秒
    }

    // 熄灭所有灯
    gx_ws2812_stop();
    printf("Fixed color test completed, all LEDs turned off\n");

    return 0;
}

/**
 * @brief 呼吸灯效果测试
 */
static int test_breath(int h, int s, int v, int period_ms, int cycles)
{
    int ret = ensure_ws2812_initialized();
    if (ret != 0) return ret;
    // 每个亮度级别之间的延时时间，亮度由0~100，100~0
    uint16_t delay_per_step = period_ms / 200;

    if (h < 0 || h > 360 || s < 0 || s > 100 || v < 0 || v > 100) {
        printf("Invalid HSV values. H: 0-360, S: 0-100, V: 0-100\n");
        return -1;
    }

    if (period_ms < 500) {
        printf("Period too short, minimum 500ms\n");
        return -1;
    }

    if (cycles < 1) cycles = 1;

    printf("Breathing effect: HSV(%d, %d, %d), period: %dms, cycles: %d\n",
           h, s, v, period_ms, cycles);

    for (int cycle = 0; cycle < cycles; cycle++) {
        // 从暗到亮
        for (uint8_t brightness = 0; brightness <= 100; brightness++) {
            GX_WS2812_RGB color = hsv_to_rgb(h, s, brightness);
            gx_ws2812_setall(color);
            gx_ws2812_update();
            gx_mdelay(delay_per_step);
        }

        // 从亮到暗
        for (uint8_t brightness = 100; brightness > 0; brightness--) {
            GX_WS2812_RGB color = hsv_to_rgb(h, s, brightness);
            gx_ws2812_setall(color);
            gx_ws2812_update();
            gx_mdelay(delay_per_step);
        }

        printf("Cycle %d/%d completed\n", cycle + 1, cycles);
    }

    // 熄灭所有灯
    gx_ws2812_stop();
    printf("Breathing effect completed, all LEDs turned off\n");

    return 0;
}

/**
 * @brief 彩虹效果测试
 */
static int test_rainbow(int h, int s, int v, int speed, int time)
{
    int ret = ensure_ws2812_initialized();
    if (ret != 0) return ret;
    int base_h = h;

    if (h < 0 || h > 360 || s < 0 || s > 100 || v < 0 || v > 100) {
        printf("Invalid HSV values. H: 0-360, S: 0-100, V: 0-100\n");
        return -1;
    }

    if (speed < 1 || speed > 360) {
        printf("Invalid speed. Range: 1-360\n");
        return -1;
    }

    printf("Rainbow effect: Base HSV(%d, %d, %d), speed: %d, duration: %dms\n",
           h, s, v, speed, time);

    int total_steps = time / 100; // 每100ms一步
    int hue_step = speed; // 每步色相增加量

    for (int step = 0; step < total_steps; step++) {
        for (int i = 0; i < LED_NUM; i++) {
            int hue = (base_h + step * hue_step + i * 360 / LED_NUM) % 360;
            GX_WS2812_RGB color = hsv_to_rgb(hue, s, v);
            gx_ws2812_setcolor(i, color);
        }
        gx_ws2812_update();
        gx_mdelay(100);
    }

    // 熄灭所有灯
    gx_ws2812_stop();
    printf("Rainbow effect completed, all LEDs turned off\n");

    return 0;
}

static int keypress_task(void *priv)
{
	int period = 5;

	for (int i = 0; i < LED_NUM; i++) {
		if (key_status[i].status == 1) {
			key_status[i].brightness = 100;
		} else if (key_status[i].status == 0) {
			key_status[i].brightness -= period;
		}

		if (key_status[i].brightness <= 0) {
			key_status[i].brightness = 0;
			key_status[i].status = 0;
		}
	}

	for (int i = 0; i < LED_NUM; i++) {
		gx_ws2812_setcolor(i, hsv_to_rgb(hsv.h, hsv.s, key_status[i].brightness));
	}

	gx_ws2812_update();
}


/**
 * @brief 按键效果测试
 */
static int test_keypress(int h, int s, int v, int fade_time)
{
    int ret = ensure_ws2812_initialized();
    if (ret != 0) return ret;

    if (h < 0 || h > 360 || s < 0 || s > 100 || v < 0 || v > 100) {
        printf("Invalid HSV values. H: 0-360, S: 0-100, V: 0-100\n");
        return -1;
    }

    if (fade_time < 100) {
        printf("Fade time too short, minimum 100ms\n");
        return -1;
    }

    printf("Keypress effect: HSV(%d, %d, %d), fade_time: %dms\n",
           h, s, v, fade_time);

    memset(keypress, 0, sizeof(KEY_PRESS));

    int timer_id = gx_timer_register(keypress_task, 10, NULL, GX_TIMER_MODE_CONTINUE);

    // 模拟按键序列测试
	keypress.key[0].status = 1;
	keypress.key[0].status = 1;
	keypress.key[0].status = 1;

	gx_mdelay(300);

	keypress.key[1].status = 0;
	keypress.key[2].status = 0;

	gx_mdelay(300);

	keypress.key[0].status = 0;
	keypress.key[3].status = 1;
	keypress.key[4].status = 1;
	keypress.key[5].status = 1;

	gx_mdelay(300);
	key_status[3].status = 1;
	key_status[4].status = 1;
	key_status[5].status = 1;

	gx_mdelay(2000);
	gx_timer_unregister(timer_id);
    // 熄灭所有灯
    gx_ws2812_stop();
    printf("Keypress effect completed, all LEDs turned off\n");

    return 0;
}

/**
 * @brief 流水灯效果测试
 */
static int test_flow(int h, int s, int v, int cycles)
{
    int ret = ensure_ws2812_initialized();
    if (ret != 0) return ret;

    if (h < 0 || h > 360 || s < 0 || s > 100 || v < 0 || v > 100) {
        printf("Invalid HSV values. H: 0-360, S: 0-100, V: 0-100\n");
        return -1;
    }

    printf("Flow effect: HSV(%d, %d, %d), 5 rounds with 1s idle between rounds\n",
           h, s, v);

    GX_WS2812_RGB color = hsv_to_rgb(h, s, v);
    GX_WS2812_RGB off = {0, 0, 0};

    for (int round = 0; round < cycles; round++) {
        printf("Round %d/%d\n", round + 1, cycles);

        for (int pos = 0; pos < LED_NUM; pos++) {
            // 清除所有LED
            gx_ws2812_setall(off);

            // 点亮当前位置
            gx_ws2812_setcolor(pos, color);

            gx_ws2812_update();
            gx_mdelay(100); // 每个位置停留100ms
        }

        // 清除所有LED
        gx_ws2812_setall(off);
        gx_ws2812_update();

        if (round < 4) { // 最后一轮不需要等待
            printf("Idle 1s before next round...\n");
            gx_mdelay(100); // 每轮空闲100ms
        }
    }

    // 熄灭所有灯
    gx_ws2812_stop();
    printf("Flow effect completed, all LEDs turned off\n");

    return 0;
}

/**
 * @brief 关闭所有LED
 */
static int test_stop(void)
{
    int ret = ensure_ws2812_initialized();
    if (ret != 0) return ret;

    gx_ws2812_stop();
    printf("All LEDs turned off\n");

    return 0;
}

/**
 * @brief 重新初始化
 */
static int test_reinit(int led_num)
{
    // 先退出原有初始化
    if (ws2812_init_flag) {
        gx_ws2812_exit();
        ws2812_init_flag = 0;
    }

    // 重新初始化
    int ret = ws2812_test_init(led_num);
    if (ret == 0) {
        ws2812_init_flag = 1;
    }

    return ret;
}

/**
 * @brief 显示帮助信息
 */
static void show_help(void)
{
    printf("WS2812 Test Commands:\n");
    printf("  init <led_num>                     - Initialize with specified LED count\n");
    printf("  fix_color [h] [s] [v]              - Fixed color (preset if no HSV)\n");
    printf("  breath [h] [s] [v] [period_ms] [cycles] - Breathing effect (default: 5 cycles)\n");
    printf("  rainbow [h] [s] [v] [speed] [time_ms]        - Rainbow effect (5s duration)\n");
    printf("  keypress [h] [s] [v] [fade_time_ms]   - Key press simulation effect\n");
    printf("  flow [h] [s] [v] [cycles]                   - Flow effect (5 rounds, 1s idle)\n");
    printf("  auto [loops]                       - Auto test all effects (default: 1 loop)\n");
    printf("  stop                               - Turn off all LEDs\n");
    printf("  help                               - Show this help\n");
    printf("\n");
    printf("HSV Parameters:\n");
    printf("  h: Hue (0-360)\n");
    printf("  s: Saturation (0-100)\n");
    printf("  v: Value/Brightness (0-100)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  ws2812 init 7                     - Initialize with 7 LEDs\n");
    printf("  ws2812 fix_color                  - Show preset colors\n");
    printf("  ws2812 fix_color 240 100 80       - Blue color at 80%% brightness\n");
    printf("  ws2812 breath 0 100 80 2000 5     - Red breathing, 2s period, 5 cycles\n");
    printf("  ws2812 rainbow 120 100 60 10 5000 - Green-based rainbow, speed 10, 5s duration\n");
    printf("  ws2812 keypress 240 100 100 1000  - Blue keypress, 1s fade\n");
    printf("  ws2812 flow 180 100 70 5          - Cyan flow effect, 5 rounds\n");
    printf("  ws2812 auto                       - Run all effects demo (1 loop)\n");
    printf("  ws2812 auto 3                     - Run all effects demo (3 loops)\n");
}

// 命令处理函数声明
static int handle_init(int argc, char **argv)
{
    if (argc == 3) {
        return test_reinit(atoi(argv[2]));
    } else {
        printf("Usage: ws2812 init <led_num>\n");
        return -1;
    }
}

static int handle_fix_color(int argc, char **argv)
{
    if (argc == 2) {
        return test_fix_color(0, 0, 0, 0);
    } else if (argc == 5) {
        return test_fix_color(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), 1);
    } else {
        printf("Usage: ws2812 fix_color [h] [s] [v]\n");
        return -1;
    }
}

static int handle_breath(int argc, char **argv)
{
    int h = 300;
    int s = 100;
    int v = 100;
    int period_ms = 4000;
    int cycles = 5;

    if (argc == 2) {
        return test_breath(h, s, v, period_ms, cycles);
    } else if (argc == 7) {
        return test_breath(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    } else {
        printf("Usage: ws2812 breath [h] [s] [v] [period_ms] [cycles]\n");
        return -1;
    }
}

static int handle_rainbow(int argc, char **argv)
{
    int h = 300;
    int s = 100;
    int v = 100;
    int speed = 30;
    int time = 3000;

    if (argc == 2) {
        return test_rainbow(h, s, v, speed, time);
    } else if (argc == 7) {
        return test_rainbow(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    } else {
        printf("Usage: ws2812 rainbow [h] [s] [v] [speed] [time_ms]\n");
        return -1;
    }
}

static int handle_keypress(int argc, char **argv)
{
    int h = 300;
    int s = 100;
    int v = 100;
    int fade_time = 2000;

    if (argc == 2) {
        return test_keypress(h, s, v, fade_time);
    } else if (argc == 6) {
        return test_keypress(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
    } else {
        printf("Usage: ws2812 keypress [h] [s] [v] [fade_time_ms]\n");
        return -1;
    }
}

static int handle_flow(int argc, char **argv)
{
    int h = 300;
    int s = 100;
    int v = 100;
    int cycles = 5;

    if (argc == 2) {
        return test_flow(h, s, v, cycles);
    } else if (argc == 6) {
        return test_flow(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
    } else {
        printf("Usage: ws2812 flow [h] [s] [v] [cycles]\n");
        return -1;
    }
}

static int handle_stop(int argc, char **argv)
{
    return test_stop();
}

/**
 * @brief 自动测试程序 - 展示所有WS2812灯效
 */
static int test_auto(int loops)
{
    int ret = ensure_ws2812_initialized();
    if (ret != 0) return ret;

    if (loops <= 0) loops = 1;

    printf("WS2812 Auto Test Starting - %d loop(s)\n", loops);
    printf("===========================================\n");

    for (int loop = 0; loop < loops; loop++) {
        if (loops > 1) {
            printf("\n>>> Loop %d/%d <<<\n", loop + 1, loops);
        }

        // 1. 固定颜色测试 - 显示预设颜色
        printf("\n1. Fixed Color Test - Preset Colors\n");
        printf("------------------------------------\n");
        ret = test_fix_color(0, 0, 0, 0);  // mode = 0 表示预设颜色
        if (ret != 0) {
            printf("Fixed color test failed\n");
            return ret;
        }
        gx_mdelay(1000);

        // 2. 固定颜色测试 - 自定义颜色（蓝色）
        printf("\n2. Fixed Color Test - Custom Blue\n");
        printf("----------------------------------\n");
        ret = test_fix_color(240, 100, 80, 1);  // 蓝色，80%亮度
        if (ret != 0) {
            printf("Custom color test failed\n");
            return ret;
        }
        gx_mdelay(1000);

        // 3. 呼吸灯效果 - 红色
        printf("\n3. Breathing Effect - Red\n");
        printf("-------------------------\n");
        ret = test_breath(0, 100, 80, 2000, 3);  // 红色，2秒周期，3次循环
        if (ret != 0) {
            printf("Breathing effect test failed\n");
            return ret;
        }
        gx_mdelay(500);

        // 4. 彩虹效果
        printf("\n4. Rainbow Effect\n");
        printf("-----------------\n");
        ret = test_rainbow(120, 100, 60, 20, 3000);  // 绿色基础，速度20，3秒
        if (ret != 0) {
            printf("Rainbow effect test failed\n");
            return ret;
        }
        gx_mdelay(500);

        // 5. 按键效果测试
        printf("\n5. Keypress Effect\n");
        printf("------------------\n");
        ret = test_keypress(300, 100, 100, 800);  // 紫色，800ms渐变
        if (ret != 0) {
            printf("Keypress effect test failed\n");
            return ret;
        }
        gx_mdelay(500);

        // 6. 流水灯效果 - 青色
        printf("\n6. Flow Effect - Cyan\n");
        printf("---------------------\n");
        ret = test_flow(180, 100, 70, 3);  // 青色，3轮
        if (ret != 0) {
            printf("Flow effect test failed\n");
            return ret;
        }
        gx_mdelay(500);

        // 7. 流水灯效果 - 橙色（快速）
        printf("\n7. Flow Effect - Orange (Fast)\n");
        printf("-------------------------------\n");
        ret = test_flow(30, 100, 90, 2);  // 橙色，2轮
        if (ret != 0) {
            printf("Flow effect test failed\n");
            return ret;
        }
        gx_mdelay(500);

        if (loop < loops - 1) {
            printf("\nWaiting 2 seconds before next loop...\n");
            gx_mdelay(2000);
        }
    }

    printf("\n===========================================\n");
    printf("WS2812 Auto Test Completed Successfully!\n");
    printf("All %d loop(s) finished, all effects tested.\n", loops);

    return 0;
}

static int handle_auto(int argc, char **argv)
{
    int loops = 1;  // 默认1次循环

    if (argc == 3) {
        loops = atoi(argv[2]);
        if (loops < 1) loops = 1;
    }

    return test_auto(loops);
}

static int handle_help(int argc, char **argv)
{
    show_help();
    return 0;
}

// 命令表
static const struct cmd_entry ws2812_cmd_table[] = {
    {"init",      "ws2812 init <led_num>",                          2, handle_init},
    {"fix_color", "ws2812 fix_color [h] [s] [v]",                   2, handle_fix_color},
    {"breath",    "ws2812 breath [h] [s] [v] [period_ms] [cycles]", 2, handle_breath},
    {"rainbow",   "ws2812 rainbow [h] [s] [v] [speed] [time_ms]",   2, handle_rainbow},
    {"keypress",  "ws2812 keypress [h] [s] [v] [fade_time_ms]",     2, handle_keypress},
    {"flow",      "ws2812 flow [h] [s] [v] [cycles]",               2, handle_flow},
    {"auto",      "ws2812 auto [loops]",                            2, handle_auto},
    {"stop",      "ws2812 stop",                                    2, handle_stop},
    {"help",      "ws2812 help",                                    2, handle_help},
    {NULL, NULL, 0, NULL}  // 结束标记
};

/**
 * @brief WS2812测试程序入口
 */
int gx_ws2812_test_entry(int argc, char **argv)
{
    return cmd_parse_and_run(argc, argv, ws2812_cmd_table, show_help);
}
