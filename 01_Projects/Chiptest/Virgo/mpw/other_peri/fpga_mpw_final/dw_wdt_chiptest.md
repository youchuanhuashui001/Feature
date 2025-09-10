# DW WDT 芯片测试文档

## 1. 测试环境
- FPGA 版本：`382541 9p_peri_newcode_FPGA_2024-09-03.tar`
- VU9P

## 2. 测试用例

### 2.1 复位功能测试
1. 测试步骤：
	- `.config` 中打开配置 `CONFIG_WDT = y`
	- `reboot`
	- 观察系统是否重启
2. 预期结果：
	- FPGA 可以产生复位
	- 系统可以正常重启
3. 注意事项：
	- 默认配置为最大值，会一直计数
	- 每次喂狗（写 0x76）会将计数器清零
	- 如果不喂狗会溢出，溢出后会产生中断或复位

### 2.2 喂狗功能测试
1. 测试步骤：
	- `.config` 中打开配置 `CONFIG_WDT = y;ENABLE_IRQ=y`
	- `wdt ping <ms>`
	- 观察系统打印
2. 预期结果：
	- 系统正常运行，打印 wdt callback
	- gdb 打断程序不喂狗，系统复位
3. 修改代码：
```diff
--- a/bootmenu.c
+++ b/bootmenu.c
@@ -284,10 +284,33 @@ void command_wdtest(int argc, const char **argv)
                gx_wdt_reset_soon(time);
        }
 }
+
+COMMAND(wdt,command_wdt, "<ping> <ms>");
+void command_wdt(int argc, const char **argv)
+{
+       extern int gx_wdt_timer(int32_t ms);
+       
+       if (argc != 3){
+               printf("wdt <ping> <ms>\n");
+               return;
+       }
+
+       if (strcmp(argv[1], "ping") == 0){
+               unsigned int time;
+               time = atoi(argv[2]);
+               gx_wdt_timer(time);
+       }
+}
 #endif


--- a/drivers/watchdog/dw_wdt.c
+++ b/drivers/watchdog/dw_wdt.c
@@ -80,6 +80,7 @@ static enum interrupt_return wdt_interrupt_isr(uint32_t vector, void* pdata)
 
 static int wtd_callback(GX_HAL_WDT *dev, void *pdata)
 {
+       printf("wdt callback\n");
     wdt_interrupt_isr(WDT_ISR_VEC, NULL);
     return 0;
 }

```



### 2.3 中断使能测试
1. 测试步骤：
	- `.config` 中打开配置 `CONFIG_WDT = y;ENABLE_IRQ=y`
	- `wdt ping <ms>`
	- 观察系统打印
2. 预期结果：
	- 中断可以正常使能
	- 可以正常触发中断

### 2.4 定时器值读取测试
1. 测试步骤：
	- `.config` 中打开配置 `CONFIG_WDT = y;ENABLE_IRQ=y`
	- `wdt ping <ms>`
	- 观察寄存器是否在递减 `x/x 0xfa450008`
2. 预期结果：
	- 寄存器值实时递减
	- 喂狗后计数器回绕到设定值开始递减
3. 注意事项：
	- 目前没有驱动代码
	- 需要手动读取寄存器，递减到 0 之后会根据所选的输出相应模式，将发生系统复位或中断




## 3. 测试结果
- [x] 复位功能测试
- [x] 喂狗功能测试
- [x] 中断使能测试
- [x] 定时器值读取测试





## 4. 测试记录
### 1 . GDB 调试问题：gdb 跑不了，实际烧程序进去也跑不起来
```
X23eRUNFPGA0xF090001C : 0x0
0xfa4900cc : 0x4
chip spdif powercfg not config
```
- 回退到以下版本可以正常启动：
```
commit 0de76170c39058e69f6169ed12a0004df590b712 (HEAD -> final)
Author: tanxzh <tanxzh@nationalchip.com>
Date:   Thu Nov 7 16:05:56 2024 +0800

    377384: Virgo IRR 新增低功耗模式
    
    Change-Id: I5b197023a2294eeb3ab39c948c358c13dd3d4b22

commit ab4774558b5bb25fc1f2c0e0c9049c72a155a573
Author: zhoulong <zhoulong@nationalchip.com>
Date:   Fri Nov 29 17:13:42 2024 +0800

    395033: gxloader 支持管脚复用配置
    
    Change-Id: I6e1059accfa95f70635e5f9ced6b43f48521dec7

commit 0d06f03802cece9cc16604609da1587dacfb46ff
Merge: 01834daf 54dc38ad
Author: 叶妙兵 <yemb@nationalchip.com>
Date:   Wed Dec 4 13:32:38 2024 +0800

    Merge "362611: virgo eth final 寄存器地址变更" into develop2

commit 01834daf76eb850a353f5a9a5b8cde184a5b54f0
Author: yangmch <yangmch@nationalchip.com>
Date:   Tue Dec 3 14:07:12 2024 +0800

    374659: 1. 添加dma配置block size参数，适配saradc传输
            2. 关掉 dma lowpower 功能
    
    Change-Id: I55fcbd53cbf28545c23058d32ebedfa41e53dc80
```


### 2 . 中断时间精度问题：irq 时间不准
- 5s 定时较为准确
- 8s 定时会有偏差（约 11.18s）
- 这是由于定时器配置的计算方式导致的 
- 定时时间计算公式：`(1 << (16 + top)) / dev->pclk`
- 8s 配置：
	- top = 12：`((1<<12+16)) / 24000000 = 11.1848s`
	- top = 11：`(1<<27) / 24000000 = 5.5924s`（不够要求）


