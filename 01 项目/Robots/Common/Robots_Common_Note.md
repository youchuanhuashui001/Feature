
## Leo_mini、Leo、Apus 获取 block_size 实际为 4K
- `apus、leo_mini`  等芯片的驱动在使用接口 ` getinfo(BLOCK_SIZE) ` 获取 `block_size` 时获取到的 `block_size`  为 `4k` ，而不是 `64k` 



## 各芯片 Flash 不同阶段工作频率

| Chip     | ROM  | Stage1                  | Stage2 | 备注                                                                        |
| -------- | ---- | ----------------------- | ------ | ------------------------------------------------------------------------- |
| Leo_mini | 3Mhz | 50Mhz<br>(读 ID : 25MHz) | 50MHz  | Stage1、Stage2 同频，可以配置为 200MHz、150MHz、100MHz。<br>目前配置为 200MHz，4分频，相位范围：1~4 |





## Apus 配置 P2_0 ~ P2_5 为通用 SPI 功能
- Apus 的 `Gen SPI` 可以配置成 `Master/Slave`，并且分别是两组引脚
```diff
diff --git a/board/nationalchip/apus_nre_1v/pinmux.c b/board/nationalchip/apus_nre_1v/pinmux.c
index 67a02638..429bd85f 100644
--- a/board/nationalchip/apus_nre_1v/pinmux.c
+++ b/board/nationalchip/apus_nre_1v/pinmux.c
@@ -3,6 +3,8 @@
 #include <gx_clock.h>
 #include <misc.h>
 
+#define CONFIG_VIRGO_TEST_MASTER
+
@@ -57,12 +59,21 @@ static const GX_PIN_CONFIG pin_table[] = {
     /* {13,  2  }, // P1_5 */
     {14,  33 }, // P1_6
     {15,  6  }, // P1_7
-    {16,  29 }, // P2_0
-    {17,  10 }, // P2_1
-    {18,  97 }, // P2_2
-    {19,  98 }, // P2_3
-    {20,  13 }, // P2_4
-    {21,  14 }, // P2_5
+#ifdef CONFIG_VIRGO_TEST_MASTER
+    {16,  62 }, // P2_0
+    {17,  71 }, // P2_1
+    {18,  72 }, // P2_2
+    {19,  73 }, // P2_3
+    {20,  74 }, // P2_4
+    {21,  75 }, // P2_5
+#else
+    {16,  76 }, // P2_0
+    {17,  77 }, // P2_1
+    {18,  78 }, // P2_2
+    {19,  79 }, // P2_3
+    {20,  80 }, // P2_4
+    {21,  81 }, // P2_5
+#endif
     {22,  0  }, // P2_6
     {23,  50 }, // P2_7
     {24,  51 }, // P3_0
```




# Apus、Aquila、Fornax 等芯片 Stage1 会读 ID，Stage1 用的是四倍速读 flash
- `arch/riscv/mach-apus/spl/spl_spinor.c` 中初始化 flash 时会读 id，并使能 quad，以及读接口是用的 quad 模式。xip 模式也是四线模式 
- Aquila 不知道 Stage1 有没有 QUAD 的宏，是不是用的四线
- Fornax Stage1 用的是四倍速 