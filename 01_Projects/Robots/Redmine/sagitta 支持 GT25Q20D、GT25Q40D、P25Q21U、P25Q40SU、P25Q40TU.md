sagitta 需要内封这些 flash 用于可选项。由于 sagitta 目前还在 fpga 阶段，所以纯粹是为了测试 flash ，因此放到 leo_mini 上进行测试。
sagitta 是 3.3v 的 flash，目前已有的平台：leo_mini、apus、fornax。其中 apus 是 3.3v，fornax 是内封的 flash，因此只能用 leo_mini 来测试，需要跑极限速度 120MHz。

 - [x] **sagitta 到时候内封会封 1.8v 的 flash，因此需要重新测试下 1.8v 的工作情况**
	 - [x] 提交补丁，直接 repo upload
	 - [x] 上传手册
	 - [x] 修改支持列表，描述是 1.8v


# flash 信息

- Name：P25Q40SU
- JEDEC_ID：0x856013
- OTP：`3*512`
- UID：128bit
- Write_Protect：
- Quad：
	- [x] 0x05：Read SR1
	- [x] 0x35：Read SR2
	- [x] 0x01：Read SR1+SR2
	- [x] 0x31：Write SR2
- 最高频率：104MHz
- 

- Name：P25Q40TU
- JEDEC_ID：0x856013
- OTP：`3*512`
- UID：128bit
- Write_Protect：
- Quad：
	- [x] 0x05：Read SR1
	- [x] 0x35：Read SR2
	- [x] 0x01：Read SR1+SR2
	- [x] 0x31：Write SR2
- 最高频率：120MHz

- Name：P25Q21U
- JEDEC_ID：0x856012
- OTP：`3*512`
- UID：128bit
- Write_Protect：
- Quad：
	- [x] 0x05：Read SR1
	- [x] 0x35：Read SR2
	- [x] 0x01：Read SR1+SR2
	- [x] 0x31：Write SR2
- 最高频率：104MHz
- 1.8v 最高能跑 70MHz

- Name：GT25Q40D
- JEDEC_ID：0xc44013
- OTP：`4*256  0x0, 0x100, 0x200, 0x300 `
- UID：64bit  0x4b + 4dummy
- Write_Protect：
- Quad：
	- [x] 0x05：Read SR1
	- [x] 0x35：Read SR2
	- [x] 0x01：Read SR1+SR2
	- [x] 0x31：Write SR2
- 最高频率：104MHz
- 跑不到 80MHz，改了驱动强度也没有可用的相位
- 可以跑 72MHz，测试下可用的相位，再读写擦看看
	- 可用相位：4，5


- Name：GT25Q20D
- JEDEC_ID：0xc44012
- OTP：`4*256  0x0, 0x100, 0x200, 0x300 `
- UID：64bit  0x4b + 4dummy
- Write_Protect：
- Quad：
	- [x] 0x05：Read SR1
	- [x] 0x35：Read SR2
	- [x] 0x01：Read SR1+SR2
	- [x] 0x31：Write SR2
- 最高频率：104MHz
- 72MHz，可用相位 4，5

# 问题 ：
## 【已解决】编译报错：添加 types. h 可以解决
```
drivers/legacy/mtd/flash_test.c:18:2: 错误： 未知的类型名‘u32’
  u32 test_dram_base;
  ^~~
drivers/legacy/mtd/flash_test.c:19:2: 错误： 未知的类型名‘u32’
  u32 malloc_max_len;
  ^~~
drivers/legacy/mtd/flash_test.c:21:2: 错误： 未知的类型名‘u32’
  u32 flash_size;
  ^~~
drivers/legacy/mtd/flash_test.c:22:2: 错误： 未知的类型名‘u32’
  u32 flash_block_size;
  ^~~
drivers/legacy/mtd/flash_test.c:23:2: 错误： 未知的类型名‘u32’
  u32 flash_page_size;
  ^~~
drivers/legacy/mtd/flash_test.c:24:2: 错误： 未知的类型名‘u32’
  u32 rand_cnt;
  ^~~
drivers/legacy/mtd/flash_test.c:28:2: 错误： 未知的类型名‘u32’
  u32 addr;
  ^~~
drivers/legacy/mtd/flash_test.c:29:2: 错误： 未知的类型名‘u32’
  u32 len;
  ^~~
drivers/legacy/mtd/flash_test.c:77:8: 错误： 未知的类型名‘u64’
 static u64 holdrand_test = 1;
        ^~~
drivers/legacy/mtd/flash_test.c:78:12: 错误： expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘inline’
 static u32 inline rand_test(void)
            ^~~~~~
drivers/legacy/mtd/flash_test.c: 在函数‘sflash_test_single’中:
drivers/legacy/mtd/flash_test.c:86:2: 错误： 未知的类型名‘u8’
  u8 *p, *p2, *p_src, *p_dst, *p_tmp;
  ^~
drivers/legacy/mtd/flash_test.c:103:37: 警告： 传递‘gx_spi_flash_readdata’的第 3 个参数时在不兼容的指针类型间转换 [-Wincompatible-pointer-types]
  gx_spi_flash_readdata(flash, addr, p, len);
                                     ^
In file included from drivers/legacy/mtd/flash_test.c:6:0:
include/gx_flash.h:84:5: 附注： 需要类型‘unsigned char *’，但实参的类型为‘int *’
 int gx_spi_flash_readdata(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);
     ^~~~~~~~~~~~~~~~~~~~~
drivers/legacy/mtd/flash_test.c:112:37: 警告： 传递‘gx_spi_flash_readdata’的第 3 个参数时在不兼容的指针类型间转换 [-Wincompatible-pointer-types]
  gx_spi_flash_readdata(flash, addr, p, len);
                                     ^
In file included from drivers/legacy/mtd/flash_test.c:6:0:
include/gx_flash.h:84:5: 附注： 需要类型‘unsigned char *’，但实参的类型为‘int *’
 int gx_spi_flash_readdata(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);
     ^~~~~~~~~~~~~~~~~~~~~
drivers/legacy/mtd/flash_test.c:124:40: 警告： 传递‘gx_spi_flash_pageprogram’的第 3 个参数时在不兼容的指针类型间转换 [-Wincompatible-pointer-types]
  gx_spi_flash_pageprogram(flash, addr, p2, len);
                                        ^~
In file included from drivers/legacy/mtd/flash_test.c:6:0:
include/gx_flash.h:134:5: 附注： 需要类型‘unsigned char *’，但实参的类型为‘int *’
 int gx_spi_flash_pageprogram(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);
     ^~~~~~~~~~~~~~~~~~~~~~~~
drivers/legacy/mtd/flash_test.c:127:37: 警告： 传递‘gx_spi_flash_readdata’的第 3 个参数时在不兼容的指针类型间转换 [-Wincompatible-pointer-types]
  gx_spi_flash_readdata(flash, addr, p, len);
                                     ^
In file included from drivers/legacy/mtd/flash_test.c:6:0:
include/gx_flash.h:84:5: 附注： 需要类型‘unsigned char *’，但实参的类型为‘int *’
 int gx_spi_flash_readdata(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);
     ^~~~~~~~~~~~~~~~~~~~~
drivers/legacy/mtd/flash_test.c: 在文件作用域：
drivers/legacy/mtd/flash_test.c:187:27: 错误： 未知的类型名‘u32’
 static int flash_get_rand(u32 *addr, u32 *len)
                           ^~~
drivers/legacy/mtd/flash_test.c:187:38: 错误： 未知的类型名‘u32’
 static int flash_get_rand(u32 *addr, u32 *len)
                                      ^~~
drivers/legacy/mtd/flash_test.c: 在函数‘flash_speed_test’中:
drivers/legacy/mtd/flash_test.c:210:2: 错误： 未知的类型名‘u32’
  u32 test_len = flash_test.flash_block_size * 2;
  ^~~
drivers/legacy/mtd/flash_test.c:211:2: 错误： 未知的类型名‘u8’
  u8 *p;
  ^~
drivers/legacy/mtd/flash_test.c:226:34: 警告： 传递‘gx_spi_flash_readdata’的第 3 个参数时在不兼容的指针类型间转换 [-Wincompatible-pointer-types]
  gx_spi_flash_readdata(flash, 0, p, test_len);
                                  ^
In file included from drivers/legacy/mtd/flash_test.c:6:0:
include/gx_flash.h:84:5: 附注： 需要类型‘unsigned char *’，但实参的类型为‘int *’
 int gx_spi_flash_readdata(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);
     ^~~~~~~~~~~~~~~~~~~~~
drivers/legacy/mtd/flash_test.c:240:37: 警告： 传递‘gx_spi_flash_pageprogram’的第 3 个参数时在不兼容的指针类型间转换 [-Wincompatible-pointer-types]
  gx_spi_flash_pageprogram(flash, 0, p, test_len);
                                     ^
In file included from drivers/legacy/mtd/flash_test.c:6:0:
include/gx_flash.h:134:5: 附注： 需要类型‘unsigned char *’，但实参的类型为‘int *’
 int gx_spi_flash_pageprogram(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);
     ^~~~~~~~~~~~~~~~~~~~~~~~
drivers/legacy/mtd/flash_test.c: 在函数‘flash_complete_test’中:
drivers/legacy/mtd/flash_test.c:252:2: 错误： 未知的类型名‘u32’
  u32 addr_tmp, len_tmp;
  ^~~
drivers/legacy/mtd/flash_test.c:298:3: 警告： 隐式声明函数‘flash_get_rand’ [-Wimplicit-function-declaration]
   flash_get_rand(&addr_tmp, &len_tmp);
   ^~~~~~~~~~~~~~
drivers/legacy/mtd/flash_test.c: 在函数‘flash_otp_test’中:
drivers/legacy/mtd/flash_test.c:619:17: 警告： 隐式声明函数‘rand_test’ [-Wimplicit-function-declaration]
   writebuf[j] = rand_test();
                 ^~~~~~~~~
make: *** [Makefile:127：drivers/legacy/mtd/flash_test.o] 错误 1
```

### 【已解决】编译 .boot 时找不到宏：CONFIG_SF_MSPI0_BUS、CONFIG_SF_MSPI1_BUS
- 修改代码：
```c
if (spi_channel == 0)
	bus_num = CONFIG_SF_MSPI0_BUS;
else if (spi_channel == 1)
	bus_num = CONFIG_SF_MSPI1_BUS;
else
	bus_num = CONFIG_SF_DEFAULT_BUS;
	
	--------------------------------
	
if (spi_channel == 0)
	bus_num = 0;
else if (spi_channel == 1)
	bus_num = 1;
else
	bus_num = CONFIG_SF_DEFAULT_BUS;
```


### 【已解决】掉电测试时 flash 很小，需要修改预留的 block
测试 256k flash 时，只有 4 个 block，但是预留了 3 个 block，所以一直测试 block3，修改代码，只预留 block0 来存放 bin 文件 (仅 59k)
```diff
diff --git a/board/nationalchip/leo_mini_gx8008b_dev_1v/include/board_config.h b/board/nationalchip/leo_mini_gx8008b_dev_1v/include/board_config.h
index ea30d358..392d99b9 100644
--- a/board/nationalchip/leo_mini_gx8008b_dev_1v/include/board_config.h
+++ b/board/nationalchip/leo_mini_gx8008b_dev_1v/include/board_config.h
@@ -95,7 +95,7 @@
 
 /* SPI FLASH */
 #define CONFIG_FLASH_SPI_CLK_SRC        240000000    /* 200MHz */
-#define CONFIG_SF_DEFAULT_CLKDIV        4            /* 分频数必须为偶数且非0 */
+#define CONFIG_SF_DEFAULT_CLKDIV        2            /* 分频数必须为偶数且非0 */
 #define CONFIG_SF_DEFAULT_SAMPLE_DELAY  3
 #define CONFIG_SF_DEFAULT_SPEED         (CONFIG_FLASH_SPI_CLK_SRC / CONFIG_SF_DEFAULT_CLKDIV)
 
diff --git a/drivers/legacy/mtd/flash_test.c b/drivers/legacy/mtd/flash_test.c
index c12f14bb..c63fdb0c 100644
--- a/drivers/legacy/mtd/flash_test.c
+++ b/drivers/legacy/mtd/flash_test.c
@@ -1038,6 +1038,10 @@ void flash_power_cut_test_init(void)
        reserve_block_num = (fw_size + erase_size + 1) / erase_size;
        reserve_size = reserve_block_num * erase_size;
 
+       fw_size = 64*1024;
+       reserve_block_num = 1;
+       reserve_size = 64*1024;
+
        data = gx_spi_flash_getinfo(flash, GX_FLASH_CHIP_TYPE);
        printf("flash type: %s \n", data == GX_FLASH_CHIP_TYPE_NOR ? "spi nor":
                        data == GX_FLASH_CHIP_TYPE_SPINAND ? "spi nand": "nand" );
@@ -1100,6 +1104,10 @@ void flash_power_cut_test(void)
        reserve_block_num = (fw_size + erase_size + 1) / erase_size;
        reserve_size = reserve_block_num * erase_size;
 
+       fw_size = 64*1024;
+       reserve_block_num = 1;
+       reserve_size = 64*1024;
+
        printf("firmware size: %d, reserve_size: 0x%x\n", fw_size, reserve_size);

```

### 【已解决】gt25q40d、gt25q20d 四线读写擦出错
- 修改驱动能力为 10% 后可以正常读写
- 60MHz 可以正常读写，120MHz 没有可用的相位
- 厂商描述最高只能到 104Mhz


# Apus
```
* 补丁：{{patch()}}
|/7.ZB25LQ16A |4,2,1倍速读写擦测试||
|xip读写测试||
|写保护测试||
|uid读取||
|单页多次写测试||
|掉电300次测试||
|休眠唤醒测试||

---

* 不同频率下的 sample_delay 范围测试：
| core电压 | 实测VDD09电压(V) | 时钟源pll | cpu分频系数 | cpu频率 | xip时钟源分频系数 | xip内部分频 | xip工作频率 | xip sample delay范围值 | xip sample delay建议值 | 备注                                   |
| 1.0       |                 | 440       | 4           | 110     | 2                 | 2           | 110         |                        |                        |    无可用相位                          |
| 1.1       |                 | 160       | 1           | 160     | 1                 | 2           | 80          |      3, 4, 5           |         4              |    读写擦测试通过                      |
| 1.1       |                 | 144       | 1           | 144     | 1                 | 2           | 72          |      3, 4, 5           |         4              |    读写擦测试通过                      |
| 1.1       |                 | 128       | 1           | 128     | 1                 | 2           | 64          |      2, 3, 4, 5        |         3              |    读写擦测试通过                      |
| 1.0       |                 | 96        | 1           | 96      | 1                 | 2           | 48          |         2, 3, 4        |         3              |    读写擦测试通过                      |
| 0.95      |                 | 64        | 1           | 64      | 1                 | 2           | 32          |         1, 2, 3        |         2              |    左侧为 1.0v 测试情况，读写擦测试通过|
| 0.9       |                 | 32        | 1           | 32      | 1                 | 2           | 16          |      0, 1, 2           |         1              |    读写擦测试通过                      |


* 注：此处的相位使用 apus socket 板测得，仅供参考；具体客户板子的相位需要重新在客户板子上测试。




* 手册描述掉电至 xxx v 后保持 xxx us 即可完成复位

* 手册描述最高工作频率为：xxx MHz
```

- [ ] 手册上传至云盘
- [ ] 更新支持列表： https://git.nationalchip.com/redmine/projects/robot_os/wiki/FLASH%E6%94%AF%E6%8C%81%E5%88%97%E8%A1%A8



# Leo/Leo_mini/grus
```
* scpu 补丁：{{patch()}}
* vsp 补丁：{{patch()}}


需要补测 otp
130MHz：测试情况
|/6.P25Q40TU |4,2倍速读写擦测试||
|xip测试||
|写保护测试|ok|
|uid读取|ok|
|单页多次写测试|失败|
|掉电300次测试||

相位：
0不行
1不行
2可以
3不行
4不行

需要补测 otp
130MHz：测试情况
|/6.P25Q40SU |4,2倍速读写擦测试|ok|
|xip测试|ok|
|写保护测试|ok|
|uid读取|ok|
|单页多次写测试|失败|
|掉电300次测试||
相位：
0不行
1不行
2可以
3不行
4不行


需要补测 otp
130MHz：测试情况
|/6.P25Q21U |4,2倍速读写擦测试|ok|
|xip测试|ok|
|写保护测试|ok|
|uid读取|ok|
|单页多次写测试|失败|
|掉电300次测试||
相位：
0不行
1不行
2不行
3可以
4不行


130MHz：测试情况
|/6.GT25Q20D |4,2倍速读写擦测试|四线失败|
|xip测试|ok|
|写保护测试|ok|
|uid读取|ok|
|单页多次写测试|ok|
|掉电300次测试||
|otp测试|不测|
相位：
3可用
看起来好像四线模式不能超过 50MHz
130MHz：
2不可用
3不可用
4不可用

65MHz:
1不可用
2不可用
3不可用
4不可用


40D 65MHz 也会读写擦出错
1不可用
2不可用
3不可用
4不可用


也没有可用的相位值，换成之前支持的 GT25Q80A 可以正常读写擦

```

- [x] 手册上传至云盘
- [x] 更新支持列表： https://git.nationalchip.com/redmine/projects/robot_os/wiki/FLASH%E6%94%AF%E6%8C%81%E5%88%97%E8%A1%A8

