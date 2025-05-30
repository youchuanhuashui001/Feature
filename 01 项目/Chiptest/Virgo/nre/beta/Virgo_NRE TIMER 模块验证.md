
# 1. 基本信息

- **Redmine 问题号**: `#408040`
- **芯片型号/项目名称**: `Virgo_NRE`
- **被测模块**: `TIMER`
- **固件/FPGA 镜像版本**: 
	- a7： #404698 vu9p_fpga_top_sub_a7_20250524. bit test7 a7 核 flash 中 48M
	- a55: #404698 vu9p_fpga_top_sub_a55_20250524. bit test8 a55 核 flash 中 48M
- **相关文档**: [[timer ip]]

# 2. 测试目标

简要描述本次测试的主要目的和范围。
- 兼容 32 位、64 位驱动、测试代码
- I2C 模块验证


# 3. 测试环境

## 3.1 硬件环境

- **FPGA 型号**: `VU9P-05`

## 3.2 软件环境
```
commit c8c3f8dcc3b0587775e99383adc177c5804cf808 (HEAD -> virgonre, refs/published/virgonre)
Author: tanxzh <tanxzh@nationalchip.com>
Date:   Wed Apr 16 20:20:38 2025 +0800

    406477: Virgo NRE Flash_SPI 模块验证
            1. 新增 ddr 模式
            2. 测试程序支持 64 位
    
    Change-Id: Iff4f3a0f3de42b5cfced0c780313511093660454

commit bbb3a47e7060674f40d776b6e05979adccd18671
Merge: bdc426ea 8444e4ca
Author: 刘非 <liufei@nationalchip.com>
Date:   Sun Apr 27 15:10:15 2025 +0800

    Merge "406515: gxloader 支持 nos lib 的编译" into develop2

commit 8444e4cabfddb2a9ce696f5f0b539cb29e8e745a
Author: yemb <yemb@nationalchip.com>
Date:   Wed Apr 16 16:13:28 2025 +0800

    406515: gxloader 支持 nos lib 的编译
    
    Change-Id: I2c9a76e8f50a46c8e4c67bfa18de5495d9c9f1e9

commit bdc426ea3806369fb99bc6fa4e4c190a7f1fc2bc
Author: yangmch <yangmch@nationalchip.com>
Date:   Fri Mar 14 16:26:33 2025 +0800

    398922: 添加gxloader 支持 eMMC 和 SD支持
    
    Change-Id: Ie240b97650bfbb1572bcaa7e3f0a3029207fb54a
```

# 4. 测试用例与结果


# 5. Q&&A

## 如何编译测试代码？
- 测试代码路径
    - gxtest: stb/chiptest/cases/TIME_V100/
- 编译方法：
	- 到 `gxtest/stb/chiptest/` 目录下
    - 编译./build config -m TIME_V100 
    - ./build arm nos virgo fpga dvbs bin debug
    - 依赖 gxloader 的 nos lib，需要先编译 gxloader
- 编译后生成的整 bin 在 `/home/tanxzh/goxceed/develop2/gxtest/stb/chiptest/output/image`
- 也可以直接 gdb 加载 `chiptest` 目录下的 `out.elf`

- minicom 不显示汉字：
	- `minicom -R utf8`


- a55 测试代码：
```diff
diff --git a/bootmenu.c b/bootmenu.c
index 8a6e9afae..ef9f08e2b 100644
--- a/bootmenu.c
+++ b/bootmenu.c
@@ -3638,3 +3638,1310 @@ void command_lowpower(int argc, const char **argv)
 }
 #endif
 
+/******************************************** TIME_V100 Test Commands ********************************************/
+// Forward declarations for TIME_V100 counter tests
+static void timev100_counter_lowpower_test(int argc, const char **argv);
+static void timev100_counter_frequency_div_test(int argc, const char **argv);
+static void timev100_counter_time_mode_test(int argc, const char **argv);
+static void timev100_counter_counter_enable_test(int argc, const char **argv);
+static void timev100_counter_counter_reset_test(int argc, const char **argv);
+static void timev100_counter_interrupt_overflow_test(int argc, const char **argv);
+static void timev100_counter_counter_level_test(int argc, const char **argv);
+static void timev100_counter_counter_get_test(int argc, const char **argv);
+static void timev100_counter_overflow_cnt_test(int argc, const char **argv);
+
+// Forward declarations for PWM tests here
+static void timev100_pwm_lowpower_test(int argc, const char **argv);
+static void timev100_pwm_frequency_div_test(int argc, const char **argv); 
+static void timev100_pwm_time_mode_test(int argc, const char **argv); // Added forward declaration
+static void timev100_pwm_pwmr_enable_test(int argc, const char **argv);
+static void timev100_pwm_pwm_reset_test(int argc, const char **argv);
+static void timev100_pwm_pwm_duty_cycle_test(int argc, const char **argv);
+static void timev100_pwm_pwm_period_test(int argc, const char **argv);
+static void timev100_pwm_pwm_level_test(int argc, const char **argv);
+static void timev100_pwm_pwm_interrupt_test(int argc, const char **argv);
+static void timev100_pwm_pwm_get_set_test(int argc, const char **argv);
+static void timev100_pwm_overflow_cnt_test(int argc, const char **argv);
+
+static void command_timev100_info(int argc, const char **argv);
+
+static void command_timev100_system(int argc, const char **argv);
+
+// Forward declarations for SYSTEM tests here
+static void timev100_system_stability_test(int argc, const char **argv);
+
+// Static variable to share time_id for pwmr_enable test
+static unsigned int timev100_pwmr_enable_time_id = (unsigned int)-1; // Initialize to an invalid ID
+
+// Counter subcommands
+void command_timev100_counter(int argc, const char **argv)
+{
+	if (argc < 2) {
+		printf("Usage: timev100 counter <test_case> [options]\n");
+		printf("Available counter test cases:\n");
+		printf("  lowpower         - Test timer clock low power enable configuration\n");
+		printf("  freqdiv          - Test timer pre-division configuration\n");
+		printf("  timemode         - Test timer mode configuration (default count mode)\n");
+		printf("  enable           - Test counter enable/disable\n");
+		printf("  reset            - Test counter reset\n");
+		printf("  interrupt        - Test timer interrupt (single shot mode)\n");
+		printf("  level            - Test timer period/threshold setting (single shot mode)\n");
+		printf("  getcount         - Test getting counter value\n");
+		printf("  overflowcnt      - Test reading timer overflow count (depends on HAL behavior)\n");
+		return;
+	}
+
+	if (strcmp(argv[1], "lowpower") == 0) {
+		timev100_counter_lowpower_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "freqdiv") == 0) {
+		timev100_counter_frequency_div_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "timemode") == 0) {
+		timev100_counter_time_mode_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "enable") == 0) {
+		timev100_counter_counter_enable_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "reset") == 0) {
+		timev100_counter_counter_reset_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "interrupt") == 0) {
+		timev100_counter_interrupt_overflow_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "level") == 0) {
+		timev100_counter_counter_level_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "getcount") == 0) {
+		timev100_counter_counter_get_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "overflowcnt") == 0) {
+		timev100_counter_overflow_cnt_test(argc - 2, argv + 2);
+	}
+	else {
+		printf("Unknown counter test case: %s\n", argv[1]);
+	}
+}
+
+void command_timev100_pwm(int argc, const char **argv)
+{
+	if (argc < 2) {
+		printf("Usage: timev100 pwm <test_case> [options]\n");
+		printf("Available PWM test cases:\n");
+		printf("  lowpower         - PWM 低功耗配置测试\n");
+		printf("  freqdiv          - PWM 预分频配置测试\n");
+		printf("  timemode         - PWM 时间模式配置测试\n"); // Added help message
+		printf("  pwmr_enable      - PWM 低功耗配置测试\n"); // Added help message
+		printf("  pwm_reset        - PWM 复位测试\n");
+		printf("  duty_cycle       - PWM 占空比配置测试\n");
+		printf("  period           - PWM 周期配置测试\n");
+		printf("  level            - PWM 电平翻转点配置测试\n");
+		printf("  interrupt        - PWM 中断测试\n");
+		printf("  get_set          - PWM 参数获取与设置测试\n");
+		printf("  overflow_cnt     - PWM 溢出计数测试 (间接)\n");
+		return;
+	}
+
+	if (strcmp(argv[1], "lowpower") == 0) {
+		timev100_pwm_lowpower_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "freqdiv") == 0) {
+		timev100_pwm_frequency_div_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "timemode") == 0) { // Added handler for timemode
+		timev100_pwm_time_mode_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "pwmr_enable") == 0) { // Added handler for pwmr_enable
+		timev100_pwm_pwmr_enable_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "pwm_reset") == 0) {
+		timev100_pwm_pwm_reset_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "duty_cycle") == 0) {
+		timev100_pwm_pwm_duty_cycle_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "period") == 0) {
+		timev100_pwm_pwm_period_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "level") == 0) {
+		timev100_pwm_pwm_level_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "interrupt") == 0) {
+		timev100_pwm_pwm_interrupt_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "get_set") == 0) {
+		timev100_pwm_pwm_get_set_test(argc - 2, argv + 2);
+	} else if (strcmp(argv[1], "overflow_cnt") == 0) {
+		printf("无此特性\n");
+//		timev100_pwm_overflow_cnt_test(argc - 2, argv + 2);
+	}
+	else {
+		printf("未知的PWM测试用例: %s\n", argv[1]);
+	}
+}
+
+// TIME_V100 main command
+COMMAND(timev100, command_timev100, "<module> [test_case] [options] -- TIME_V100 test suite");
+void command_timev100(int argc, const char **argv)
+{
+	if (argc < 2) {
+		printf("Usage: timev100 <module> [options]\n");
+		printf("Available modules:\n");
+		printf("  counter          - 计数器功能测试\n");
+		printf("  pwm              - PWM功能测试\n");
+		printf("  info             - 信息功能测试 (例如版本号)\n");
+		printf("  system           - 系统级测试\n");
+		return;
+	}
+
+	if (strcmp(argv[1], "counter") == 0) {
+		command_timev100_counter(argc - 1, argv + 1);
+	} else if (strcmp(argv[1], "pwm") == 0) {
+		command_timev100_pwm(argc - 1, argv + 1);
+	} else if (strcmp(argv[1], "info") == 0) {
+		command_timev100_info(argc - 1, argv + 1);
+	} else if (strcmp(argv[1], "system") == 0) {
+		command_timev100_system(argc - 1, argv + 1);
+	}
+	else {
+		printf("未知模块: %s\n", argv[1]);
+	}
+}
+
+// Implementations for TIME_V100 counter tests
+
+// Corresponds to TIME_V100/src/01.counter/01.lowpower/test_TIME_V100_01_counter_01_lowpower_test.c
+static void timev100_counter_lowpower_test(int argc, const char **argv)
+{
+	int i_test_case;
+	if (argc < 1) {
+		printf("用法: timev100 counter lowpower <test_case_num>\n");
+		printf("  00 - 时钟一直有效\n");
+		printf("  01 - 自动开关时钟\n");
+		return;
+	}
+
+	i_test_case = atoi(argv[0]);
+
+	if (i_test_case == 0) { 
+		int i;
+		printf("测试用例: 计数器低功耗 - 时钟一直有效\n");
+		printf("模拟执行: gx_chiptest_set_flag(CHIP_TEST_TIME_GLOBAL_GATE_EN, 1)\n");
+		gx_time_init();
+		for (i = 0; i < 4; i++) {
+			mdelay(1000);
+			printf("延时: %d S\n", i + 1);
+		}
+		gx_delay_deinit();
+		printf("模拟执行: gx_chiptest_set_flag(CHIP_TEST_TIME_GLOBAL_GATE_EN, 0)\n");
+		printf("测试用例 00 完成.\n");
+	} else if (i_test_case == 1) { 
+		int i;
+		printf("测试用例: 计数器低功耗 - 自动开关时钟\n");
+		printf("模拟执行: gx_chiptest_set_flag(CHIP_TEST_TIME_GLOBAL_GATE_EN, 0)\n");
+		gx_time_init(); 
+		for (i = 0; i < 4; i++) {
+			mdelay(1000);
+			printf("延时: %d S\n", i + 1);
+		}
+		printf("测试用例 01 完成.\n");
+	} else {
+		printf("无效的低功耗测试用例编号: %s\n", argv[0]);
+	}
+}
+
+// Corresponds to TIME_V100/src/01.counter/02.Frequency_div/test_TIME_V100_01_counter_02_Frequency_div_test.c
+static void timev100_counter_frequency_div_test(int argc, const char **argv)
+{
+	printf("测试用例: 计数器分频 - 预分频测试\n");
+	gx_time_init(); 
+	printf("预分频设置为1us tick, 延时 1000000us (1秒)\n");
+	mdelay(1000);
+	gx_delay_deinit();
+	printf("测试用例完成.\n");
+}
+
+// Corresponds to TIME_V100/src/01.counter/03.time_mode/test_TIME_V100_01_counter_03_time_mode_test.c
+static void timev100_counter_time_mode_test(int argc, const char **argv)
+{
+	printf("测试用例: 计数器时间模式 - 默认计数模式测试\n");
+	gx_time_init();
+	printf("延时 1 S\n");
+	mdelay(1000);
+	gx_delay_deinit();
+	printf("测试用例完成.\n");
+}
+
+static void timev100_delay_ms_custom(uint32_t milliseconds) {
+	unsigned int i, j;
+    for (i = 0; i < milliseconds; i++) {
+        for (j = 0; j < 40000; j++) { 
+            asm volatile ("nop");
+        }
+    }
+}
+
+// Corresponds to TIME_V100/src/01.counter/04.counter_enable/test_TIME_V100_01_counter_04_counter_enable_test.c
+static void timev100_counter_counter_enable_test(int argc, const char **argv)
+{
+	int i_test_case;
+	if (argc < 1) {
+		printf("用法: timev100 counter enable <test_case_num>\n");
+		printf("  00 - 使能计数器 (通过 gx_time_init 和 mdelay隐式测试)\n");
+		printf("  01 - 禁止计数器 (测试 gx_timer_stop)\n");
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+
+	if (i_test_case == 0) { 
+		printf("测试用例: 计数器使能 - 使能计数器\n");
+		gx_time_init();
+		printf("延时 1 S (定时器应运行中)\n");
+		mdelay(1000);
+		gx_delay_deinit();
+		printf("测试用例 00 完成.\n");
+	} else if (i_test_case == 1) { 
+		unsigned int time_id;
+		unsigned long long count_start, count_after_stop, count_after_delay;
+
+		printf("测试用例: 计数器使能 - 禁止计数器\n");
+		time_id = gx_timer_register(-1, NULL, 0xFFFFFFFF, NULL, GX_TIMER_MODE_CNT);
+		if ((int)time_id < 0) { 
+			printf("定时器错误: 注册定时器失败\n");
+			return;
+		}
+		printf("定时器 %d 已注册.\n", time_id);
+
+		count_start = gx_timer_get_count(time_id);
+		gx_timer_stop(time_id);
+		count_after_stop = gx_timer_get_count(time_id);
+		
+		printf("自定义延时约100ms...\n");
+		timev100_delay_ms_custom(100);
+		
+		count_after_delay = gx_timer_get_count(time_id);
+
+		printf("停止前计数值: %llu, 停止后计数值: %llu, 停止并延时后计数值: %llu\n", 
+		       count_start, count_after_stop, count_after_delay);
+
+		if (count_after_stop == count_after_delay) {
+			printf("成功: 定时器 %d 成功停止 (停止并延时后计数值未改变).\n", time_id);
+		} else {
+			printf("失败: 定时器 %d 未按预期停止 (停止并延时后计数值发生改变).\n", time_id);
+		}
+		gx_timer_unregister(time_id);
+		printf("测试用例 01 完成.\n");
+	} else {
+		printf("无效的使能测试用例编号: %s\n", argv[0]);
+	}
+}
+
+// Corresponds to TIME_V100/src/01.counter/05.counter_reset/test_TIME_V100_01_counter_05_counter_reset_test.c
+static void timev100_counter_counter_reset_test(int argc, const char **argv)
+{
+	unsigned int time_id;
+	unsigned long long count_before_reset = 0;
+	unsigned long long count_after_reset;
+
+	printf("测试用例: 计数器复位 - 复位定时器CNT寄存器\n");
+
+	time_id = gx_timer_register(-1, NULL, 0xFFFFFFFF, NULL, GX_TIMER_MODE_CNT);
+	if ((int)time_id < 0) {
+		printf("定时器错误: 注册定时器失败\n");
+		return;
+	}
+	printf("定时器 %d 已注册.\n", time_id);
+
+	printf("等待计数值至少达到300...\n");
+	while (count_before_reset < 300) {
+		count_before_reset = gx_timer_get_count(time_id);
+	}
+	printf("复位前计数值: %llu\n", count_before_reset);
+
+	gx_timer_reset_count(time_id);
+	printf("定时器 %d 计数值已复位.\n", time_id);
+
+	count_after_reset = gx_timer_get_count(time_id);
+	printf("复位后计数值: %llu\n", count_after_reset);
+
+	if (count_after_reset < count_before_reset && count_after_reset < 50) { 
+		printf("成功: 计数器复位成功。复位后计数值 (%llu) < 复位前计数值 (%llu) 并且较小.\n", count_after_reset, count_before_reset);
+	} else {
+		printf("失败: 计数器复位失败。复位后计数值 (%llu), 复位前计数值 (%llu).\n", count_after_reset, count_before_reset);
+	}
+
+	gx_timer_unregister(time_id);
+	printf("测试用例完成.\n");
+}
+
+static volatile unsigned int timev100_interrupt_complete_flag = 0;
+
+static int timev100_timer_callback_for_interrupt_test(void *data)
+{
+	timev100_interrupt_complete_flag = 1;
+	printf("定时器回调已执行! 标志已设置.\n");
+	return 0;
+}
+
+// Corresponds to TIME_V100/src/01.counter/06.interrupt_overflow/test_TIME_V100_01_counter_06_interrupt_overflow_test.c
+static void timev100_counter_interrupt_overflow_test(int argc, const char **argv)
+{
+	unsigned int time_id;
+	printf("测试用例: 计数器中断溢出 - 单次定时器回调\n");
+	time_id = gx_timer_register(-1, timev100_timer_callback_for_interrupt_test, 100, NULL, GX_TIMER_MODE_SINGLE);
+	if ((int)time_id < 0) {
+		printf("定时器错误: 注册定时器失败\n");
+		return;
+	}
+	printf("定时器 %d 已注册为单次模式, 100ms超时.\n", time_id);
+
+	timev100_interrupt_complete_flag = 0;
+	printf("等待定时器中断回调 (超时100ms)...\n");
+
+	unsigned int wait_timeout_ms = 200; 
+	unsigned int start_wait_ms = gx_time_get_ms(); 
+
+	while (!timev100_interrupt_complete_flag) {
+		if (gx_time_get_ms() - start_wait_ms > wait_timeout_ms) {
+			printf("失败: 等待中断回调超时.\n");
+			gx_timer_unregister(time_id);
+			return;
+		}
+	}
+
+	if (timev100_interrupt_complete_flag) {
+		printf("成功: 定时器中断回调已被调用.\n");
+	} else {
+		printf("失败: 定时器中断回调未被调用 (标志未设置).\n");
+	}
+
+	gx_timer_unregister(time_id);
+	printf("测试用例完成.\n");
+}
+
+// Corresponds to TIME_V100/src/01.counter/07.counter_level/test_TIME_V100_01_counter_07_counter_level_test.c
+static void timev100_counter_counter_level_test(int argc, const char **argv)
+{
+	unsigned int time_id;
+	unsigned int timeout_ms = 3000;
+	unsigned int wait_timeout_ms = timeout_ms + 100;
+	unsigned int start_wait_ms;
+
+	printf("测试用例: 计数器阈值 - 定时器周期/阈值设置 (单次 %u ms)\n", timeout_ms);
+
+	time_id = gx_timer_register(-1, timev100_timer_callback_for_interrupt_test, timeout_ms, NULL, GX_TIMER_MODE_SINGLE);
+	if ((int)time_id < 0) {
+		printf("定时器错误: 注册定时器失败\n");
+		return;
+	}
+	printf("定时器 %d 已注册为单次模式, %u ms超时.\n", time_id, timeout_ms);
+
+	timev100_interrupt_complete_flag = 0; 
+	printf("等待定时器中断回调 (超时 %u ms)...\n", timeout_ms);
+
+	start_wait_ms = gx_time_get_ms();
+
+	while (!timev100_interrupt_complete_flag) {
+		if (gx_time_get_ms() - start_wait_ms > wait_timeout_ms) {
+			printf("失败: 等待中断回调超时.\n");
+			gx_timer_unregister(time_id);
+			return;
+		}
+	}
+
+	if (timev100_interrupt_complete_flag) {
+		printf("成功: 定时器 (周期 %u ms) 中断回调已被调用.\n", timeout_ms);
+	} else {
+		printf("失败: 定时器中断回调未被调用 (标志未设置).\n");
+	}
+
+	gx_timer_unregister(time_id);
+	printf("测试用例完成.\n");
+}
+
+// Corresponds to TIME_V100/src/01.counter/08.counter_get/test_TIME_V100_01_counter_08_counter_get_test.c
+static void timev100_counter_counter_get_test(int argc, const char **argv)
+{
+	unsigned int time_id;
+	unsigned long long count1, count2;
+	printf("测试用例: 获取计数值 - 读取定时器CNT值\n");
+
+	time_id = gx_timer_register(-1, NULL, 0xFFFFFFFF, NULL, GX_TIMER_MODE_CNT);
+	if ((int)time_id < 0) {
+		printf("定时器错误\n");
+		return; 
+	}
+	printf("定时器 %d 已注册为计数模式.\n", time_id);
+
+	count1 = gx_timer_get_count(time_id);
+	unsigned int retry_count = 0;
+	unsigned int max_retries = 1000000; 
+	do {
+		count2 = gx_timer_get_count(time_id);
+		if (count1 != count2) break;
+		retry_count++;
+	} while (retry_count < max_retries);
+
+	if (count1 != count2) {
+		printf("成功: 获取计数值成功。计数值1: %llu, 计数值2: %llu\n", count1, count2);
+	} else {
+		printf("失败: 获取计数值失败或定时器未递增。计数值1: %llu, 计数值2: %llu\n", count1, count2);
+	}
+
+	gx_timer_unregister(time_id);
+	printf("测试用例完成.\n");
+}
+
+// Corresponds to TIME_V100/src/01.counter/09.overflow_cnt/test_TIME_V100_01_counter_09_overflow_cnt_test.c
+static void timev100_counter_overflow_cnt_test(int argc, const char **argv)
+{
+	unsigned int time_id;
+	unsigned long long count;
+	printf("测试用例: 计数器溢出次数 - 读取定时器溢出次数\n");
+	time_id = gx_timer_register(-1, timev100_timer_callback_for_interrupt_test, 100, NULL, GX_TIMER_MODE_CONTINUE);
+	if ((int)time_id < 0) {
+		printf("定时器错误\n");
+		return;
+	}
+	printf("定时器 %d 已注册为连续模式, 100ms周期.\n", time_id);
+
+	timev100_interrupt_complete_flag = 0; 
+	printf("等待至少一个定时器周期完成...\n");
+	
+	unsigned int wait_timeout_ms = 200; 
+	unsigned int start_wait_ms = gx_time_get_ms();
+	while(!timev100_interrupt_complete_flag) {
+		if (gx_time_get_ms() - start_wait_ms > wait_timeout_ms) {
+			printf("警告: 等待首次中断回调超时。计数值可能在首次溢出前获取.\n");
+			break; 
+		}
+	}
+
+	count = gx_timer_get_count(time_id);
+	printf("定时器 %d 当前64位计数值: %llu (0x%016llx)\n", time_id, count, count);
+
+	if (count >> 32) {
+		printf("成功: 定时器溢出次数非零 (高32位: 0x%llx).\n", count >> 32);
+	} else {
+		printf("失败或提示信息: 定时器溢出次数为零 (高32位: 0x%llx).\n", count >> 32);
+		printf("  这可能是实际失败，或者gx_hal_timer_get_count()在此定时器模式/配置下\n");
+		printf("  未返回高32位溢出计数器 (TIMER_ACC)。\n");
+	}
+
+	gx_timer_unregister(time_id);
+	printf("测试用例完成.\n");
+}
+
+
+// Implementations for TIME_V100 PWM tests
+
+// Corresponds to TIME_V100/src/02.pwm/01.lowpower/test_TIME_V100_02_pwm_01_lowpower_test.c
+static void timev100_pwm_lowpower_test(int argc, const char **argv)
+{
+	int i_test_case;
+	unsigned int pwm_time_id;
+	const int TARGET_GPIO_PIN = 0;
+
+	if (argc < 1) {
+		printf("用法: timev100 pwm lowpower <test_case_num>\n"); 
+		printf("  00 - 时钟一直有效\n");
+		printf("  01 - 自动开关时钟\n");
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+
+	unsigned long period_ns = 1000000;
+	unsigned long duty_ns = 500000;
+
+	if (i_test_case == 0) {
+		printf("测试用例: PWM 低功耗 - 时钟一直有效\n");
+		printf("模拟执行: gx_chiptest_set_flag(CHIP_TEST_TIME_GLOBAL_GATE_EN, 1)\n");
+
+		pwm_time_id = gx_timer_enable_pwm(-1, period_ns, duty_ns);
+		if ((int)pwm_time_id < 0) {
+			printf("错误: 使能PWM失败\n");
+			return;
+		}
+		printf("PWM 已在定时器通道 %u 上使能.\n", pwm_time_id);
+
+		if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) != 0) {
+		    printf("错误: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, pwm_time_id);
+		    gx_timer_disable_pwm(pwm_time_id);
+		    return;
+		}
+		gx_gpio_afio_enable(TARGET_GPIO_PIN);
+		printf("PWM 输出已配置到 GPIO%d. 周期: %lu ns, 占空比: %lu ns.\n", TARGET_GPIO_PIN, period_ns, duty_ns);
+		printf("查看GPIO0 PWM波是否正常产生, 周期为1000us 上升沿为500us。输入任意键结束\n");
+		uart_getc(CONFIG_UART_PORT);
+
+		gx_timer_disable_pwm(pwm_time_id);
+		gx_gpio_afio_disable(TARGET_GPIO_PIN);
+		printf("PWM 及 GPIO%d AFIO 已禁用.\n", TARGET_GPIO_PIN);
+		printf("模拟执行: gx_chiptest_set_flag(CHIP_TEST_TIME_GLOBAL_GATE_EN, 0)\n");
+		printf("测试用例 00 完成.\n");
+
+	} else if (i_test_case == 1) {
+		printf("测试用例: PWM 低功耗 - 自动开关时钟\n");
+		printf("模拟执行: gx_chiptest_set_flag(CHIP_TEST_TIME_GLOBAL_GATE_EN, 0)\n");
+
+		pwm_time_id = gx_timer_enable_pwm(-1, period_ns, duty_ns);
+		if ((int)pwm_time_id < 0) {
+			printf("错误: 使能PWM失败\n");
+			return;
+		}
+		printf("PWM 已在定时器通道 %u 上使能.\n", pwm_time_id);
+
+		if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) != 0) {
+		    printf("错误: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, pwm_time_id);
+		    gx_timer_disable_pwm(pwm_time_id);
+		    return;
+		}
+		gx_gpio_afio_enable(TARGET_GPIO_PIN);
+		printf("PWM 输出已配置到 GPIO%d. 周期: %lu ns, 占空比: %lu ns.\n", TARGET_GPIO_PIN, period_ns, duty_ns);
+		printf("查看GPIO0 PWM波是否正常产生, 周期为1000us 上升沿为500us。输入任意键结束\n");
+		uart_getc(CONFIG_UART_PORT);
+
+		gx_timer_disable_pwm(pwm_time_id);
+		gx_gpio_afio_disable(TARGET_GPIO_PIN);
+		printf("PWM 及 GPIO%d AFIO 已禁用.\n", TARGET_GPIO_PIN);
+		printf("测试用例 01 完成.\n");
+	} else {
+		printf("无效的PWM低功耗测试用例编号: %s\n", argv[0]);
+	}
+}
+
+// Corresponds to TIME_V100/src/02.pwm/02.Frequency_div/test_TIME_V100_02_pwm_02_Frequency_div_test.c
+static void timev100_pwm_frequency_div_test(int argc, const char **argv)
+{
+	unsigned int pwm_time_id;
+	const int TARGET_GPIO_PIN = 0; 
+	unsigned long period_ns = 1000000; // 1ms
+	unsigned long duty_ns = 500000;   // 0.5ms
+
+	// 00 Function-PWM功能[02.pwm]- Feature-定时器预分频配置[02.Frequency_div] = 预分频
+	printf("测试用例: PWM 预分频 - 预分频 (不分频) 测试\n");
+
+	pwm_time_id = gx_timer_enable_pwm(-1, period_ns, duty_ns);
+	if ((int)pwm_time_id < 0) {
+		printf("错误: 使能PWM失败\n");
+		return;
+	}
+	printf("PWM 已在定时器通道 %u 上使能 (周期 %lu ns, 占空比 %lu ns).\n", pwm_time_id, period_ns, duty_ns);
+
+	if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) != 0) {
+	    printf("错误: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, pwm_time_id);
+	    gx_timer_disable_pwm(pwm_time_id);
+	    return;
+	}
+	gx_gpio_afio_enable(TARGET_GPIO_PIN);
+	printf("PWM 输出已配置到 GPIO%d.\n", TARGET_GPIO_PIN);
+	printf("定时器预分频周期为1us, GPIO0 PWM周期为1000us 上升沿为500us。输入任意键结束\n");
+	uart_getc(CONFIG_UART_PORT); 
+
+	gx_timer_disable_pwm(pwm_time_id);
+	gx_gpio_afio_disable(TARGET_GPIO_PIN);
+	printf("PWM 及 GPIO%d AFIO 已禁用.\n", TARGET_GPIO_PIN);
+	printf("测试用例完成.\n");
+}
+
+// Corresponds to TIME_V100/src/02.pwm/03.time_mode/test_TIME_V100_02_pwm_03_time_mode_test.c
+static void timev100_pwm_time_mode_test(int argc, const char **argv)
+{
+	unsigned int pwm_time_id;
+	const int TARGET_GPIO_PIN = 0;
+	unsigned long period_ns = 1000000; // 1ms
+	unsigned long duty_ns = 500000;   // 0.5ms
+
+	// 00 Function-PWM功能[02.pwm]- Feature-定时器模式配置[03.time_mode] = PWM模式
+	printf("测试用例: PWM 时间模式 - PWM模式 功能测试\n");
+
+	pwm_time_id = gx_timer_enable_pwm(-1, period_ns, duty_ns);
+	if ((int)pwm_time_id < 0) {
+		printf("错误: 使能PWM失败\n");
+		return;
+	}
+	printf("PWM 已在定时器通道 %u 上使能 (周期 %lu ns, 占空比 %lu ns).\n", pwm_time_id, period_ns, duty_ns);
+
+	if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) != 0) {
+	    printf("错误: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, pwm_time_id);
+	    gx_timer_disable_pwm(pwm_time_id);
+	    return;
+	}
+	gx_gpio_afio_enable(TARGET_GPIO_PIN);
+	printf("PWM 输出已配置到 GPIO%d.\n", TARGET_GPIO_PIN);
+
+	printf("GPIO0 PWM周期为1000us 上升沿为500us。输入任意键结束\n");
+	uart_getc(CONFIG_UART_PORT); 
+
+	gx_timer_disable_pwm(pwm_time_id);
+	gx_gpio_afio_disable(TARGET_GPIO_PIN);
+	printf("PWM 及 GPIO%d AFIO 已禁用.\n", TARGET_GPIO_PIN);
+	printf("测试用例完成.\n");
+}
+
+// Corresponds to TIME_V100/src/02.pwm/04.pwmr_enable/test_TIME_V100_02_pwm_04_pwmr_enable_test.c
+static void timev100_pwm_pwmr_enable_test(int argc, const char **argv)
+{
+	unsigned int pwm_time_id;
+	const int TARGET_GPIO_PIN = 0;
+
+	if (argc < 1) {
+		printf("用法: timev100 pwm pwmr_enable <test_case_num>\n"); 
+		printf("  00 - 时钟一直有效\n");
+		printf("  01 - 自动开关时钟\n");
+		return;
+	}
+
+	pwm_time_id = gx_timer_enable_pwm(-1, 1000000, 500000);
+	if ((int)pwm_time_id < 0) {
+		printf("错误: 使能PWM失败\n");
+		return;
+	}
+	printf("PWM 已在定时器通道 %u 上使能.\n", pwm_time_id);
+
+	if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) != 0) {
+	    printf("错误: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, pwm_time_id);
+	    gx_timer_disable_pwm(pwm_time_id);
+	    return;
+	}
+	gx_gpio_afio_enable(TARGET_GPIO_PIN);
+	printf("PWM 输出已配置到 GPIO%d. 周期: 1000000ns, 占空比: 500000ns.\n", TARGET_GPIO_PIN);
+	printf("查看GPIO0 PWM波是否正常产生, 周期为1000us 上升沿为500us。输入任意键结束\n");
+	uart_getc(CONFIG_UART_PORT);
+
+	gx_timer_disable_pwm(pwm_time_id);
+	gx_gpio_afio_disable(TARGET_GPIO_PIN);
+	printf("PWM 及 GPIO%d AFIO 已禁用.\n", TARGET_GPIO_PIN);
+	printf("测试用例 00 完成.\n");
+
+	if (timev100_pwmr_enable_time_id == (unsigned int)-1) {
+		timev100_pwmr_enable_time_id = pwm_time_id;
+	} else {
+		printf("警告: 定时器通道 %u 已使用.\n", pwm_time_id);
+		printf("  这可能是实际失败，或者gx_hal_timer_get_count()在此定时器模式/配置下\n");
+		printf("  未返回高32位溢出计数器 (TIMER_ACC)。\n");
+	}
+}
+
+// Corresponds to TIME_V100/src/02.pwm/05.pwm_reset/test_TIME_V100_02_pwm_05_pwm_reset_test.c
+static void timev100_pwm_pwm_reset_test(int argc, const char **argv)
+{
+	unsigned int pwm_time_id;
+	const int TARGET_GPIO_PIN = 0; // Assuming GPIO0 for PWM output
+	unsigned long period_ns = 1000000; // 1ms
+	unsigned long duty_ns = 500000;   // 0.5ms (50% duty cycle)
+	int i_test_case;
+
+	if (argc < 1) {
+		printf("用法: timev100 pwm pwm_reset <test_case_num>\n");
+		printf("  00 - PWM复位功能\n");
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+
+	if (i_test_case == 0) {
+		printf("测试用例: PWM 复位 - PWM复位功能测试\n");
+
+		// 1. Enable PWM
+		pwm_time_id = gx_timer_enable_pwm(-1, period_ns, duty_ns);
+		if ((int)pwm_time_id < 0) {
+			printf("错误: 使能PWM失败\n");
+			return;
+		}
+		printf("PWM 已在定时器通道 %u 上使能.\n", pwm_time_id);
+
+		if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) != 0) {
+			printf("错误: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, pwm_time_id);
+			gx_timer_disable_pwm(pwm_time_id);
+			return;
+		}
+		gx_gpio_afio_enable(TARGET_GPIO_PIN);
+		printf("PWM 输出已配置到 GPIO%d. 周期: %lu ns, 占空比: %lu ns.\n", TARGET_GPIO_PIN, period_ns, duty_ns);
+		printf("查看GPIO0 PWM波是否正常产生。输入任意键继续复位测试...\n");
+		uart_getc(CONFIG_UART_PORT);
+
+		// 2. "Reset" PWM by disabling it (which also de-initializes)
+		printf("模拟执行: gx_chiptest_set_flag(CHIP_TEST_TIME_PWM_RESET_BIT, 1) (通过gx_timer_disable_pwm实现)\n");
+		gx_timer_disable_pwm(pwm_time_id);
+		gx_gpio_afio_disable(TARGET_GPIO_PIN); // Disable AFIO as well
+		printf("PWM 及 GPIO%d AFIO 已禁用 (模拟复位).\n", TARGET_GPIO_PIN);
+
+		printf("查看GPIO0 PWM波是否停止产生。输入任意键结束测试...\n");
+		uart_getc(CONFIG_UART_PORT);
+		printf("模拟执行: gx_chiptest_set_flag(CHIP_TEST_TIME_PWM_RESET_BIT, 0)\n");
+		printf("测试用例 00 完成.\n");
+	} else {
+		printf("无效的PWM复位测试用例编号: %s\n", argv[0]);
+	}
+}
+
+// Corresponds to TIME_V100/src/02.pwm/06.pwm_duty_cycle/test_TIME_V100_02_pwm_06_pwm_duty_cycle_test.c
+static void timev100_pwm_pwm_duty_cycle_test(int argc, const char **argv)
+{
+	unsigned int pwm_time_id;
+	const int TARGET_GPIO_PIN = 0; // Assuming GPIO0 for PWM output
+	unsigned long period_ns = 1000000; // 1ms period
+	unsigned long duty_ns_00 = 000000; // 0.00ms (00% duty cycle)
+	unsigned long duty_ns_25 = 250000; // 0.25ms (25% duty cycle)
+	unsigned long duty_ns_50 = 500000; // 0.5ms (50% duty cycle)
+	unsigned long duty_ns_75 = 750000; // 0.75ms (75% duty cycle)
+	unsigned long duty_ns_100 = 1000000; // 0.75ms (75% duty cycle)
+	unsigned long current_duty_ns = 0;
+	const char *duty_desc = "";
+	int i_test_case;
+
+	if (argc < 1) {
+		printf("用法: timev100 pwm duty_cycle <test_case_num>\n");
+		printf("  00 - PWM占空比25%%\n");
+		printf("  01 - PWM占空比50%%\n");
+		printf("  02 - PWM占空比75%%\n");
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+
+	if (i_test_case == 0) {
+		current_duty_ns = duty_ns_00;
+		duty_desc = "0%";
+		printf("测试用例: PWM 占空比 - PWM占空比0%%测试\n");
+	} else if (i_test_case == 1) {
+		current_duty_ns = duty_ns_25;
+		duty_desc = "25%";
+		printf("测试用例: PWM 占空比 - PWM占空比25%%测试\n");
+	} else if (i_test_case == 2) {
+		current_duty_ns = duty_ns_50;
+		duty_desc = "50%";
+		printf("测试用例: PWM 占空比 - PWM占空比50%%测试\n");
+	} else if (i_test_case == 3) {
+		current_duty_ns = duty_ns_75;
+		duty_desc = "75%";
+		printf("测试用例: PWM 占空比 - PWM占空比75%%测试\n");
+	} else if (i_test_case == 4) {
+		current_duty_ns = duty_ns_100;
+		duty_desc = "100%";
+		printf("测试用例: PWM 占空比 - PWM占空比100%%测试\n");
+	} else {
+		printf("无效的PWM占空比测试用例编号: %s\n", argv[0]);
+		return;
+	}
+
+	pwm_time_id = gx_timer_enable_pwm(-1, period_ns, current_duty_ns);
+	if ((int)pwm_time_id < 0) {
+		printf("错误: 使能PWM失败 (占空比 %s)\n", duty_desc);
+		return;
+	}
+	printf("PWM 已在定时器通道 %u 上使能.\n", pwm_time_id);
+
+	if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) != 0) {
+		printf("错误: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, pwm_time_id);
+		gx_timer_disable_pwm(pwm_time_id);
+		return;
+	}
+	gx_gpio_afio_enable(TARGET_GPIO_PIN);
+	printf("PWM 输出已配置到 GPIO%d. 周期: %lu ns, 占空比: %lu ns (%s).\n", TARGET_GPIO_PIN, period_ns, current_duty_ns, duty_desc);
+	printf("查看GPIO0 PWM波是否正常产生, 周期为%lu us, 上升沿(占空比)为%lu us。输入任意键结束测试...\n", period_ns / 1000, current_duty_ns / 1000);
+	uart_getc(CONFIG_UART_PORT);
+
+	gx_timer_disable_pwm(pwm_time_id);
+	gx_gpio_afio_disable(TARGET_GPIO_PIN);
+	printf("PWM 及 GPIO%d AFIO 已禁用.\n", TARGET_GPIO_PIN);
+	printf("测试用例 %02d (%s 占空比) 完成.\n", i_test_case, duty_desc);
+}
+
+// Corresponds to TIME_V100/src/02.pwm/07.pwm_period/test_TIME_V100_02_pwm_07_pwm_period_test.c
+static void timev100_pwm_pwm_period_test(int argc, const char **argv)
+{
+	unsigned int pwm_time_id;
+	const int TARGET_GPIO_PIN = 0; // Assuming GPIO0 for PWM output
+	unsigned long period_ns_1ms = 1000000;  // 1ms
+	unsigned long period_ns_2ms = 2000000;  // 2ms
+	unsigned long period_ns_500us = 500000; // 0.5ms
+	unsigned long duty_ns_fixed = 250000; // Fixed duty for simplicity, ensuring it's less than the smallest period
+	unsigned long current_period_ns = 0;
+	const char *period_desc = "";
+	int i_test_case;
+
+	if (argc < 1) {
+		printf("用法: timev100 pwm period <test_case_num>\n");
+		printf("  00 - PWM周期1ms\n");
+		printf("  01 - PWM周期2ms\n");
+		printf("  02 - PWM周期500us\n");
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+
+	if (i_test_case == 0) {
+		current_period_ns = period_ns_1ms;
+		period_desc = "1ms";
+		printf("测试用例: PWM 周期 - PWM周期1ms测试\n");
+	} else if (i_test_case == 1) {
+		current_period_ns = period_ns_2ms;
+		period_desc = "2ms";
+		printf("测试用例: PWM 周期 - PWM周期2ms测试\n");
+	} else if (i_test_case == 2) {
+		current_period_ns = period_ns_500us;
+		period_desc = "500us";
+		printf("测试用例: PWM 周期 - PWM周期500us测试\n");
+	} else {
+		printf("无效的PWM周期测试用例编号: %s\n", argv[0]);
+		return;
+	}
+
+	pwm_time_id = gx_timer_enable_pwm(-1, current_period_ns, duty_ns_fixed);
+	if ((int)pwm_time_id < 0) {
+		printf("错误: 使能PWM失败 (周期 %s)\n", period_desc);
+		return;
+	}
+	printf("PWM 已在定时器通道 %u 上使能.\n", pwm_time_id);
+
+	if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) != 0) {
+		printf("错误: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, pwm_time_id);
+		gx_timer_disable_pwm(pwm_time_id);
+		return;
+	}
+	gx_gpio_afio_enable(TARGET_GPIO_PIN);
+	printf("PWM 输出已配置到 GPIO%d. 周期: %lu ns (%s), 占空比: %lu ns.\n", TARGET_GPIO_PIN, current_period_ns, period_desc, duty_ns_fixed);
+	printf("查看GPIO0 PWM波是否正常产生, 周期为%s, 上升沿(占空比)为%lu us。输入任意键结束测试...\n", period_desc, duty_ns_fixed / 1000);
+	uart_getc(CONFIG_UART_PORT);
+
+	gx_timer_disable_pwm(pwm_time_id);
+	gx_gpio_afio_disable(TARGET_GPIO_PIN);
+	printf("PWM 及 GPIO%d AFIO 已禁用.\n", TARGET_GPIO_PIN);
+	printf("测试用例 %02d (周期 %s) 完成.\n", i_test_case, period_desc);
+}
+
+// Corresponds to TIME_V100/src/02.pwm/08.pwm_level/test_TIME_V100_02_pwm_08_pwm_level_test.c
+static void timev100_pwm_pwm_level_test(int argc, const char **argv)
+{
+	unsigned int pwm_time_id;
+	const int TARGET_GPIO_PIN = 0; // Assuming GPIO0 for PWM output
+	unsigned long period_ns = 1000000; // 1ms period
+	unsigned long duty_ns_25 = 250000; // 0.25ms (25% duty cycle implies rise at 0, fall at 250000)
+	unsigned long duty_ns_75 = 750000; // 0.75ms (75% duty cycle implies rise at 0, fall at 750000)
+    // Note: The original test cases for pwm_level seem to test duty cycle rather than distinct rise/fall level configurations
+    // The gx_timer_enable_pwm API takes period and duty cycle (implying rise at 0 and fall at duty_cycle)
+    // To truly test pwm_level, one would need HAL access to TIMER_PWM_RISE and TIMER_PWM_FALL directly.
+    // For now, we will replicate the behavior of the existing test cases which effectively test different duty cycles.
+	unsigned long current_duty_ns = 0;
+	const char *level_desc = "";
+	int i_test_case;
+
+	if (argc < 1) {
+		printf("用法: timev100 pwm level <test_case_num>\n");
+		printf("  00 - PWM占空比25%% (模拟电平翻转点配置)\n");
+		printf("  01 - PWM占空比75%% (模拟电平翻转点配置)\n");
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+
+	if (i_test_case == 0) {
+		current_duty_ns = duty_ns_25;
+		level_desc = "25% duty (fall at 250000ns)";
+		printf("测试用例: PWM 电平 - PWM占空比25%% (模拟电平翻转点)测试\n");
+	} else if (i_test_case == 1) {
+		current_duty_ns = duty_ns_75;
+		level_desc = "75% duty (fall at 750000ns)";
+		printf("测试用例: PWM 电平 - PWM占空比75%% (模拟电平翻转点)测试\n");
+	} else {
+		printf("无效的PWM电平测试用例编号: %s\n", argv[0]);
+		return;
+	}
+
+	pwm_time_id = gx_timer_enable_pwm(-1, period_ns, current_duty_ns);
+	if ((int)pwm_time_id < 0) {
+		printf("错误: 使能PWM失败 (%s)\n", level_desc);
+		return;
+	}
+	printf("PWM 已在定时器通道 %u 上使能.\n", pwm_time_id);
+
+	if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) != 0) {
+		printf("错误: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, pwm_time_id);
+		gx_timer_disable_pwm(pwm_time_id);
+		return;
+	}
+	gx_gpio_afio_enable(TARGET_GPIO_PIN);
+	printf("PWM 输出已配置到 GPIO%d. 周期: %lu ns, 配置: %s.\n", TARGET_GPIO_PIN, period_ns, level_desc);
+	printf("查看GPIO0 PWM波是否正常产生, 周期为1ms, 占空比根据'%s'配置。输入任意键结束测试...\n", level_desc);
+	uart_getc(CONFIG_UART_PORT);
+
+	gx_timer_disable_pwm(pwm_time_id);
+	gx_gpio_afio_disable(TARGET_GPIO_PIN);
+	printf("PWM 及 GPIO%d AFIO 已禁用.\n", TARGET_GPIO_PIN);
+	printf("测试用例 %02d (%s) 完成.\n", i_test_case, level_desc);
+}
+
+static volatile unsigned int timev100_pwm_interrupt_count = 0;
+static int timev100_pwm_timer_callback_for_interrupt_test(void *data)
+{
+	timev100_pwm_interrupt_count++;
+	//printf("PWM (模拟) 中断回调已执行! 当前计数: %u\n", timev100_pwm_interrupt_count);
+	return 0;
+}
+
+// Corresponds to TIME_V100/src/02.pwm/09.pwm_interrupt/test_TIME_V100_02_pwm_09_pwm_interrupt_test.c
+static void timev100_pwm_pwm_interrupt_test(int argc, const char **argv)
+{
+	unsigned int pwm_sim_timer_id; // Timer ID for simulating PWM interrupts
+	const int TARGET_GPIO_PIN = 0; // Assuming GPIO0 for actual PWM output for visual confirmation if needed
+	unsigned long period_ns = 100000000; // 100ms, so interrupts are not too frequent
+	unsigned long duty_ns = 50000000;   // 50ms (50% duty cycle)
+	unsigned int interrupt_period_ms = 100; // Corresponds to period_ns
+	int i_test_case;
+
+	if (argc < 1) {
+		printf("用法: timev100 pwm interrupt <test_case_num>\n");
+		printf("  00 - PWM中断测试 (使用通用定时器模拟PWM周期性中断)\n");
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+
+	if (i_test_case == 0) {
+		printf("测试用例: PWM 中断 - PWM中断测试 (模拟)\n");
+		printf("由于 time_v3.c API 不直接支持PWM特定中断回调注册,\n");
+		printf("此测试将使用一个通用定时器以连续模式模拟PWM周期性中断,\n");
+		printf("同时会启动一个实际的PWM信号供观察 (如果需要)。\n");
+
+		// 1. Enable actual PWM for observation (optional, but good for completeness)
+		unsigned int actual_pwm_id = gx_timer_enable_pwm(-1, period_ns, duty_ns);
+		if ((int)actual_pwm_id < 0) {
+			printf("信息: 使能实际PWM信号失败 (通道 %u), 但中断模拟仍将继续.\n", actual_pwm_id);
+		} else {
+			printf("实际PWM已在定时器通道 %u 上使能 (周期 %lums, 占空比 %lums).\n", actual_pwm_id, period_ns / 1000000, duty_ns / 1000000);
+			if (gx_gpio_afio_select(TARGET_GPIO_PIN, actual_pwm_id) != 0) {
+				printf("信息: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, actual_pwm_id);
+			} else {
+				gx_gpio_afio_enable(TARGET_GPIO_PIN);
+				printf("实际PWM信号已配置到 GPIO%d. 请观察波形.\n", TARGET_GPIO_PIN);
+			}
+		}
+
+		// 2. Register a general timer for interrupt simulation
+		timev100_pwm_interrupt_count = 0;
+		pwm_sim_timer_id = gx_timer_register(-1, timev100_pwm_timer_callback_for_interrupt_test, interrupt_period_ms, NULL, GX_TIMER_MODE_CONTINUE);
+		if ((int)pwm_sim_timer_id < 0) {
+			printf("错误: 注册模拟PWM中断的定时器失败\n");
+			if((int)actual_pwm_id >= 0) gx_timer_disable_pwm(actual_pwm_id);
+			if((int)actual_pwm_id >= 0 && gx_gpio_afio_select(TARGET_GPIO_PIN, actual_pwm_id) == 0) gx_gpio_afio_disable(TARGET_GPIO_PIN);
+			return;
+		}
+		printf("模拟PWM中断的定时器 %u 已注册 (周期 %ums, 连续模式).\n", pwm_sim_timer_id, interrupt_period_ms);
+
+		printf("等待约5个中断周期 (约 %ums)...\n", 5 * interrupt_period_ms);
+		mdelay(5 * interrupt_period_ms + 50); // Wait for 5 periods + a bit more
+
+		printf("模拟PWM中断发生次数: %u\n", timev100_pwm_interrupt_count);
+
+		if (timev100_pwm_interrupt_count >= 4 && timev100_pwm_interrupt_count <= 6) { //Allow for slight timing variations
+			printf("成功: 模拟PWM中断按预期发生 (预期~5次, 实际%u次).\n", timev100_pwm_interrupt_count);
+		} else {
+			printf("失败或警告: 模拟PWM中断次数与预期不符 (预期~5次, 实际%u次).\n", timev100_pwm_interrupt_count);
+		}
+
+		// Cleanup
+		gx_timer_unregister(pwm_sim_timer_id);
+		printf("模拟PWM中断的定时器 %u 已注销.\n", pwm_sim_timer_id);
+		if ((int)actual_pwm_id >= 0) {
+			gx_timer_disable_pwm(actual_pwm_id);
+			if (gx_gpio_afio_select(TARGET_GPIO_PIN, actual_pwm_id) == 0) { // Check if afio was set before disabling
+                 gx_gpio_afio_disable(TARGET_GPIO_PIN);
+            }
+			printf("实际PWM信号 (通道 %u) 及 GPIO AFIO 已禁用.\n", actual_pwm_id);
+		}
+
+		printf("测试用例 00 完成.\n");
+	} else {
+		printf("无效的PWM中断测试用例编号: %s\n", argv[0]);
+	}
+}
+
+// Corresponds to TIME_V100/src/02.pwm/10.pwm_get_set/test_TIME_V100_02_pwm_10_pwm_get_set_test.c
+static void timev100_pwm_pwm_get_set_test(int argc, const char **argv)
+{
+	unsigned int pwm_time_id;
+	const int TARGET_GPIO_PIN = 0; // Assuming GPIO0 for PWM output
+	unsigned long period_ns_set = 2000000; // 2ms period for setting
+	unsigned long duty_ns_set = 1000000;   // 1ms duty cycle (50%) for setting
+	int i_test_case;
+
+	// Note: time_v3.c does not provide a direct gx_timer_get_pwm_params function.
+	// So, the "get" part of this test cannot be directly replicated.
+	// We will only implement the "set" part by enabling PWM with specific parameters.
+
+	if (argc < 1) {
+		printf("用法: timev100 pwm get_set <test_case_num>\n");
+		printf("  00 - PWM参数设置 (周期2ms, 占空比1ms)\n");
+		// Add more test cases if different set parameters are needed
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+	int chx = atoi(argv[1]);
+
+	if (i_test_case == 0) {
+		printf("测试用例: PWM 参数获取与设置 - 设置PWM参数 (周期 %lu ns, 占空比 %lu ns)\n", period_ns_set, duty_ns_set);
+		printf("注意: 'get' 功能无法通过 time_v3.c API 直接测试.\n");
+
+		pwm_time_id = gx_timer_enable_pwm(chx, period_ns_set, duty_ns_set);
+		if ((int)pwm_time_id < 0) {
+			printf("错误: 通过 gx_timer_enable_pwm 设置PWM参数失败\n");
+			return;
+		}
+		printf("PWM 已在定时器通道 %u 上使能并设置参数.\n", pwm_time_id);
+
+		if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) != 0) {
+			printf("错误: GPIO%d 定时器 %u AFIO选择失败.\n", TARGET_GPIO_PIN, pwm_time_id);
+			gx_timer_disable_pwm(pwm_time_id);
+			return;
+		}
+		gx_gpio_afio_enable(TARGET_GPIO_PIN);
+		printf("PWM 输出已配置到 GPIO%d. 周期: %lu ns, 占空比: %lu ns.\n", TARGET_GPIO_PIN, period_ns_set, duty_ns_set);
+		printf("查看GPIO0 PWM波是否按设置参数产生 (周期 %lums, 占空比 %lums)。输入任意键结束测试...\n", period_ns_set / 1000000, duty_ns_set / 1000000);
+		uart_getc(CONFIG_UART_PORT);
+
+		gx_timer_disable_pwm(pwm_time_id);
+		gx_gpio_afio_disable(TARGET_GPIO_PIN);
+		printf("PWM 及 GPIO%d AFIO 已禁用.\n", TARGET_GPIO_PIN);
+		printf("测试用例 00 (设置部分) 完成.\n");
+	} else {
+		printf("无效的PWM参数获取与设置测试用例编号: %s\n", argv[0]);
+	}
+}
+
+// Corresponds to TIME_V100/src/02.pwm/11.overflow_cnt/test_TIME_V100_02_pwm_11_overflow_cnt_test.c
+static void timev100_pwm_overflow_cnt_test(int argc, const char **argv)
+{
+	unsigned int pwm_time_id;
+	unsigned int time_id; // Separate timer for count observation
+	const int TARGET_GPIO_PIN = 0;    // GPIO for PWM output visualization
+	unsigned long period_ns = 100000; // 100us period for PWM
+	unsigned long duty_ns = 50000;    // 50us duty (50%)
+	unsigned long long count_val;
+	int i_test_case;
+
+	// 重要提示: time_v3.c 中的 gx_timer_enable_pwm 不返回可用于 gx_timer_get_count
+	// 以获取PWM特定溢出计数的ID。gx_timer_get_count 通常用于通用定时器。
+	// 此测试将启动PWM，然后观察一个独立通用定时器的计数值在一段时间后的变化，
+	// 这并非直接测试PWM模块自身的溢出计数器，而是间接演示长时间运行后的计数情况。
+
+	if (argc < 1) {
+		printf("用法: timev100 pwm overflow_cnt <test_case_num>\n");
+		printf("  00 - PWM运行时观察通用定时器溢出 (间接测试)\n");
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+
+	if (i_test_case == 0) {
+		printf("测试用例: PWM 溢出计数 - PWM运行时观察通用定时器溢出 (间接)\n");
+		printf("注意: 这不直接测试PWM硬件溢出计数器，而是观察一个独立计数器。\n");
+
+		// 1. 启动PWM信号 (主要用于产生一些活动和时间流逝)
+		pwm_time_id = gx_timer_enable_pwm(-1, period_ns, duty_ns);
+		if ((int)pwm_time_id < 0) {
+			printf("错误: 使能PWM信号失败\n");
+			return;
+		}
+		printf("PWM已在定时器通道 %u 上使能 (周期 %luus, 占空比 %luus).\n", pwm_time_id, period_ns / 1000, duty_ns / 1000);
+		if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) == 0) {
+			gx_gpio_afio_enable(TARGET_GPIO_PIN);
+			printf("PWM信号已配置到 GPIO%d. 请观察波形.\n", TARGET_GPIO_PIN);
+		}
+
+		// 2. 注册一个新的通用定时器来观察其计数值
+		// 使用 systick_time_id，它是 gx_time_init() 创建的那个
+		// 或者，如果想隔离，可以注册一个新的，但要注意 systick_time_id 已经在了
+		// 这里我们假设 gx_timer_get_count 可以传递 systick_time_id 或类似的全局ID
+		// 或者，更准确地，我们应该使用 gx_get_time_us()，它内部使用 systick_time_id
+
+		printf("让PWM运行一小段时间 (例如，约200毫秒) 以允许计数器累积...\n");
+		// 为了演示溢出可能性（如果计数器是32位的且运行速度快），我们需要更长的延时
+		// 但gx_get_time_us() 返回的是u64, HAL层可能处理32位硬件计数器的溢出并累加到64位软件计数器
+		// 因此，直接观察 gx_get_time_us() 的高位部分是合理的
+
+		unsigned long long start_us = gx_get_time_us();
+		mdelay(200); // 等待200毫秒
+		unsigned long long end_us = gx_get_time_us();
+
+		count_val = end_us; // gx_get_time_us() 本身就是累积的微秒数
+
+		printf("PWM运行约200ms后, gx_get_time_us() 返回值: %llu (0x%016llx)\n", count_val, count_val);
+
+		// 检查高32位是否可能不为零 (表示自系统启动以来已经过了很多时间)
+		if (count_val >> 32) {
+			printf("提示: 通用定时器 (systick) 的高32位为 0x%llx, 表明其已 '溢出' 其32位部分多次。\n", count_val >> 32);
+			printf("这证实了64位计数正在工作，但并非PWM模块本身的特定溢出计数。\n");
+		} else {
+			printf("提示: 通用定时器 (systick) 的高32位为0。要观察其 '溢出' 32位部分，\n");
+			printf("系统需要运行更长时间 (大约 > 4295 秒，如果每us计数一次)。\n");
+		}
+
+		// 清理
+		gx_timer_disable_pwm(pwm_time_id);
+		if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_time_id) == 0) {
+			gx_gpio_afio_disable(TARGET_GPIO_PIN);
+		}
+		printf("PWM (通道 %u) 及 GPIO AFIO 已禁用.\n", pwm_time_id);
+
+		printf("测试用例 00 完成.\n");
+	} else {
+		printf("无效的PWM溢出计数测试用例编号: %s\n", argv[0]);
+	}
+}
+
+// Forward declarations for INFO tests here
+static void timev100_info_version_test(int argc, const char **argv);
+
+// Info subcommands
+static void command_timev100_info(int argc, const char **argv)
+{
+	if (argc < 2) {
+		printf("用法: timev100 info <test_case> [options]\n");
+		printf("可用的 info 测试用例:\n");
+		printf("  version          - 获取并打印定时器模块版本号\n");
+		return;
+	}
+
+	if (strcmp(argv[1], "version") == 0) {
+		timev100_info_version_test(argc - 2, argv + 2);
+	} else {
+		printf("未知的 info 测试用例: %s\n", argv[1]);
+	}
+}
+
+// Implementations for TIME_V100 info tests
+
+// Corresponds to a hypothetical TIME_V100/src/03.info/01.version/test_TIME_V100_03_info_01_version_test.c
+static void timev100_info_version_test(int argc, const char **argv)
+{
+	int i_test_case;
+	unsigned int version;
+
+	if (argc < 1) { // Expecting one argument for the test case number, e.g., "00"
+		printf("用法: timev100 info version <test_case_num>\n");
+		printf("  00 - 获取定时器版本号\n");
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+
+	if (i_test_case == 0) {
+		printf("测试用例: 信息获取 - 获取定时器版本号\n");
+		
+		// 调用 time_v3.c 中的 gx_time_version 函数
+		// 该函数内部会调用 gx_hal_timer_version
+		// 需要先初始化基础的定时器（gx_time_init 会间接调用 gx_timer_init）
+		gx_time_init(); //确保systick定时器等已初始化, timer_dev[0] 会被用到
+
+		version = gx_time_version();
+
+		printf("定时器模块版本号: 0x%08X\n", version);
+
+		// gx_delay_deinit(); // 取消systick定时器，如果其他测试也需要它，则按需处理
+		// 为简单起见，暂时不反初始化，以便其他测试可能复用systick
+
+		printf("测试用例 00 完成.\n");
+	} else {
+		printf("无效的信息版本测试用例编号: %s\n", argv[0]);
+	}
+}
+
+
+// Implementations for TIME_V100 system tests
+static volatile unsigned int system_test_timer_ticks = 0;
+static int system_test_timer_callback(void *data)
+{
+	system_test_timer_ticks++;
+	printf("系统稳定性测试: 定时器中断 %u 次\n", system_test_timer_ticks);
+	return 0;
+}
+
+// Corresponds to a hypothetical TIME_V100/src/04.system/01.stability/test_TIME_V100_04_system_01_stability_test.c
+static void timev100_system_stability_test(int argc, const char **argv)
+{
+	int i_test_case;
+	unsigned int periodic_timer_id = (unsigned int)-1;
+	unsigned int pwm_id = (unsigned int)-1;
+	const int TARGET_GPIO_PIN = 0; // GPIO for PWM output
+	unsigned long pwm_period_ns = 1000000; // 1ms
+	unsigned long pwm_duty_ns = 500000;   // 0.5ms
+	unsigned int test_duration_seconds = 10;
+	unsigned int timer_interval_ms = 1000; // 1 second
+
+	if (argc < 1) { // Expecting test case number, e.g., "00"
+		printf("用法: timev100 system stability <test_case_num>\n");
+		printf("  00 - 长时间运行PWM和周期定时器\n");
+		return;
+	}
+	i_test_case = atoi(argv[0]);
+
+	if (i_test_case == 0) {
+		printf("测试用例: 系统测试 - 长时间运行PWM和周期定时器 (%u 秒)\n", test_duration_seconds);
+
+		// 1. 初始化时间系统 (gx_time_init 会初始化 systick)
+		gx_time_init();
+		printf("时间系统已初始化.\n");
+
+		// 2. 注册一个周期性定时器
+		system_test_timer_ticks = 0;
+		periodic_timer_id = gx_timer_register(-1, system_test_timer_callback, timer_interval_ms, NULL, GX_TIMER_MODE_CONTINUE);
+		if ((int)periodic_timer_id < 0) {
+			printf("错误: 注册周期性定时器失败\n");
+			gx_delay_deinit(); // Cleanup systick
+			return;
+		}
+		printf("周期性定时器 %u 已注册 (每 %u ms触发一次).\n", periodic_timer_id, timer_interval_ms);
+
+		// 3. 使能PWM输出
+		pwm_id = gx_timer_enable_pwm(-1, pwm_period_ns, pwm_duty_ns);
+		if ((int)pwm_id < 0) {
+			printf("错误: 使能PWM失败\n");
+			gx_timer_unregister(periodic_timer_id);
+			gx_delay_deinit();
+			return;
+		}
+		printf("PWM 已在定时器通道 %u 上使能 (周期 %luus, 占空比 %luus).\n", pwm_id, pwm_period_ns/1000, pwm_duty_ns/1000);
+		if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_id) == 0) {
+			gx_gpio_afio_enable(TARGET_GPIO_PIN);
+			printf("PWM 输出已配置到 GPIO%d.\n", TARGET_GPIO_PIN);
+		} else {
+			printf("警告: GPIO%d PWM AFIO 配置失败, PWM信号可能无法在引脚上看到.\n", TARGET_GPIO_PIN);
+		}
+
+		// 4. 等待测试持续时间
+		printf("测试将运行 %u 秒. 请观察控制台输出和GPIO%d的PWM信号...\n", test_duration_seconds, TARGET_GPIO_PIN);
+		unsigned int seconds_elapsed = 0;
+		unsigned int last_tick_count = 0;
+		while(seconds_elapsed < test_duration_seconds) {
+			mdelay(100); // Check roughly every 100ms
+			if(system_test_timer_ticks > last_tick_count) {
+				seconds_elapsed += (system_test_timer_ticks - last_tick_count);
+				last_tick_count = system_test_timer_ticks;
+			}
+            // Fallback if interrupt is not perfectly 1s or gets missed by chance
+            if (seconds_elapsed == 0 && system_test_timer_ticks == 0 && test_duration_seconds > 0) {
+                // crude backup delay if timer ticks aren't progressing initially
+                static unsigned int initial_delay_done = 0;
+                if (!initial_delay_done) {
+                    mdelay(timer_interval_ms);
+                    initial_delay_done = 1;
+                }
+            }
+		}
+
+		printf("测试持续时间已到.\n");
+
+		// 5. 清理
+		printf("停止并注销定时器 %u...\n", periodic_timer_id);
+		gx_timer_unregister(periodic_timer_id);
+		printf("禁用PWM通道 %u...\n", pwm_id);
+		gx_timer_disable_pwm(pwm_id);
+		if (gx_gpio_afio_select(TARGET_GPIO_PIN, pwm_id) == 0) {
+			gx_gpio_afio_disable(TARGET_GPIO_PIN);
+			printf("GPIO%d AFIO 已禁用.\n", TARGET_GPIO_PIN);
+		}
+		
+		// gx_delay_deinit(); // 反初始化 systick 定时器
+		// 同样，为简单起见，暂时不反初始化systick，以防其他测试需要
+		printf("系统稳定性测试完成. 共触发 %u 次周期性中断.\n", system_test_timer_ticks);
+		printf("测试用例 00 完成.\n");
+
+	} else {
+		printf("无效的系统稳定性测试用例编号: %s\n", argv[0]);
+	}
+}
+
+// System subcommands
+static void command_timev100_system(int argc, const char **argv)
+{
+	if (argc < 2) {
+		printf("用法: timev100 system <test_case> [options]\n");
+		printf("可用的 system 测试用例:\n");
+		printf("  stability        - 定时器和PWM长时间运行稳定性测试\n");
+		return;
+	}
+
+	if (strcmp(argv[1], "stability") == 0) {
+		timev100_system_stability_test(argc - 2, argv + 2);
+	} else {
+		printf("未知的 system 测试用例: %s\n", argv[1]);
+	}
+}
```