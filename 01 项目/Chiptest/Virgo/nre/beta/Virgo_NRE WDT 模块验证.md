# WDT 模块测试

## 1. 基本信息

- **Redmine 问题号**: `#408040`
- **芯片型号/项目名称**: `Virgo_NRE  Beta` 
- **被测模块**: `WDT`
- **固件/FPGA 镜像版本**:  
	- a7： #404698 vu9p_fpga_top_sub_a7_20250524.bit test7 a7核 flash中48M
	- a55: #404698 vu9p_fpga_top_sub_a55_20250524.bit test8 a55核 flash中48M
- **相关文档**: [[dw_wdt ip]]

## 2. 测试目标
简要描述本次测试的主要目的和范围。
- 兼容 32 位、64 位驱动、测试代码
- WDT 模块验证

## 3. 测试环境

### 3.1 硬件环境

- **FPGA 型号**: `VU9P-05` (或在此处填写您的FPGA型号)

### 3.2 软件环境
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


## 4. 测试用例与结果

### 测试用例 1: 复位功能测试

- **测试命令：**
    - 打开 `.config` 中的 `ENABLE_WDT = y`
    - 输入命令：`reboot`
- **预期结果**:
    - 系统复位
- **实际结果**:
    - 芯片未连接 wdt 的复位到 fpga 的复位，无法产生复位信号
- **备注**:


---

### 测试用例 2: 喂狗功能测试

- **测试命令：**
	- 打开 `.config` 中的 `ENABLE_WDT = y； ENABLE_IRQ = y`
	- dw_wdt. c 中的 wdt_callback 添加打印
	- 输入命令：`reboot ping 5000`
	- 查看打印是否为 5s 后喂狗一次
- **测试步骤**:
    1. 初始化 WDT，配置模式为中断模式，配置超时间和回调函数
    2. 注册中断服务函数
    3. 启动 wdt
    4. wdt 内部计数器计数达到设定值后触发中断
- **预期结果**:
    - 在正确喂狗的情况下，系统不会复位
    - 功能正常
- **实际结果**:
    - nre a7 测试通过
    - nre a55 手动喂狗(详细测试流程见备注)，测试通过
- **备注/观察**:
```
[2025-05-14 10:58:34] boot> reboot ping 5000
[2025-05-14 10:58:37] boot> watchdog is alive.
[2025-05-14 10:58:46] watchdog is alive.
[2025-05-14 10:58:52] watchdog is alive.
[2025-05-14 10:58:57] watchdog is alive.
```

```diff
 #include "drivers/intc/nationalchip/nc.h"
@@ -273,7 +274,7 @@ void command_wdtest(int argc, const char **argv)
 {
        int time = 0;
 
-       if(argc > 2){
+       if(argc > 3){
                printf("too many parameters, please read help.\n");
                return;
        }
@@ -283,12 +284,18 @@ void command_wdtest(int argc, const char **argv)
                time = atoi(argv[1]);
                gx_wdt_reset_soon(time);
        }
+#if defined(CONFIG_ENABLE_IRQ)
+       else if(strcmp(argv[1], "ping") == 0){
+               time = atoi(argv[2]);
+               gx_wdt_timer(time);
+       }
+#endif
 }
 #endif
 
 
 --- a/drivers/watchdog/dw_wdt.c
+++ b/drivers/watchdog/dw_wdt.c
@@ -78,10 +78,11 @@ static enum interrupt_return wdt_interrupt_isr(uint32_t vector, void* pdata)
        return HANDLED;
 }
 
-static int wtd_callback(GX_HAL_WDT *dev, void *pdata)
+static int wdt_callback(GX_HAL_WDT *dev, void *pdata)
 {
-    wdt_interrupt_isr(WDT_ISR_VEC, NULL);
-    return 0;
+       printf("watchdog is alive.\n");
+       wdt_interrupt_isr(WDT_ISR_VEC, NULL);
+       return 0;
 }
 
 void gx_wdt_timer(int32_t ms)
@@ -89,7 +90,7 @@ void gx_wdt_timer(int32_t ms)
        wdt_dev.mode = GX_HAL_WDT_MODE_INTER;
        wdt_dev.pclk = WDT_SYS_CLK;
        wdt_dev.timeout = ms/1000;
-       wdt_dev.wdt_callback = wtd_callback;
+       wdt_dev.wdt_callback = wdt_callback;
        gx_hal_watchdog_init(&wdt_dev);
        gx_request_interrupt(WDT_ISR_VEC, IRQ, wdt_isr, NULL);
        gx_hal_watchdog_start(&wdt_dev);

```
- a55 手动 ping
	- 启动看门狗，查看计数器会一直递减计数，递减到 0 后重新开始计数
	- 在递减到 0 之前手动喂狗 (写 0x76)，能够重置计数器，直接重新计数
```diff
@@ -283,6 +283,13 @@ void command_wdtest(int argc, const char **argv)
        }else if(argc == 2){
                time = atoi(argv[1]);
                gx_wdt_reset_soon(time);
+//             while (1) {
+//                     printf("*(volatile uintptr_t *0xfa450008 = 0x%x)\n", *(volatile uintptr_t *)0xfa450008);
+//             }
+       }else if(strcmp(argv[1], "get") == 0){
+               printf("*(volatile uintptr_t *0xfa450008 = 0x%x)\n", *(volatile uintptr_t *)0xfa450008);
+       }else if(strcmp(argv[1], "ping_dog") == 0){
+               gx_wdt_ping();
        }
```

---

### 测试用例 3: 使能中断测试

- **测试命令：**
	- 与测试用例 2 一致
- **实际结果**:
    - nre a7 测试通过
    - nre a55 中断基础环境没有准备好，暂时不可测

- **备注/观察**:



---

### 测试用例 4: 读取定时器值测试

- **测试命令：**
	- 打开 `.config` 中的 `ENABLE_WDT = y； ENABLE_IRQ = y`
	- dw_wdt. c 中的 wdt_callback 添加打印
	- 输入命令：`reboot ping 5000`
	- 查看寄存器：0xfa450000+0x8
- **预期结果**:
    - 可以正确读取计数值
    - 计数值随时间正确递减
    - 功能正常
- **实际结果**:
    - nre a7 测试通过
    - nre a55 手动查看计数值，测试通过
- **状态**: 
- **备注:**
```
(gdb) x/x 0xfa450008
0xfa450008:     0x002b9a18
(gdb) x/x 0xfa450008
0xfa450008:     0x076ba4e3
(gdb) x/x 0xfa450008
0xfa450008:     0x065c922b
(gdb) x/x 0xfa450008
0xfa450008:     0x04b67720
```
- 手动查看当前的计数值：
	- 启动看门狗
	- 去掉 gx_wdt_reset_soon () 接口中的 while (1)
	- 查看寄存器 0xfa450008 寄存器的值，能够逐渐递减，递减到 0 后重新从配置的值开始计数
```diff
--- a/bootmenu.c
+++ b/bootmenu.c
@@ -283,6 +283,13 @@ void command_wdtest(int argc, const char **argv)
        }else if(argc == 2){
                time = atoi(argv[1]);
                gx_wdt_reset_soon(time);
+//             while (1) {
+//                     printf("*(volatile uintptr_t *0xfa450008 = 0x%x)\n", *(volatile uintptr_t *)0xfa450008);
+//             }
+       }else if(strcmp(argv[1], "get") == 0){
+               printf("*(volatile uintptr_t *0xfa450008 = 0x%x)\n", *(volatile uintptr_t *)0xfa450008);
+       }else if(strcmp(argv[1], "ping_dog") == 0){
+               gx_wdt_ping();
        }
```
---




## 5. 问题与缺陷记录




## 6. 测试总结与结论

- **测试覆盖率评估 (简述)**: A7 复位功能不可测，A55 使能中断、复位功能不可测
- **总体测试结果**: 通过


## 7. 附件

- `[[链接到Log文件]]`
- `[[链接到配置文件]]`
- `[[链接到逻辑分析仪捕获文件.sal 或 示波器截图]]`
- `[[链接到其他相关截图或数据]]`
