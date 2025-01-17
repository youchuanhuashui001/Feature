
# 阅读手册

- Jedec ID：0xc46017
- otp ：`3*1024` 
- uid ：`128bit 16Byte` 
- 写保护：
- quad 使能方式：SR2 bit1
	- 是否支持 0x01 read sr？
	- 是否支持？


# 修改代码
```diff
diff --git a/drivers/spinor/spinor_base.c b/drivers/spinor/spinor_base.c
index 2a616c2..e262961 100644
--- a/drivers/spinor/spinor_base.c
+++ b/drivers/spinor/spinor_base.c
@@ -282,7 +282,8 @@ int spi_nor_read_uniqueid(struct spi_nor_flash *flash, unsigned char *buf, unsig
 			cmd_dummy_len = 5;
 		if (((jedec_id == 0x5e4017) && (flash->info->markid == ZB25VQ64B_C_D)) ||
 				((jedec_id == 0x5e4016) && (flash->info->markid == ZB25VQ32D)) ||
-				((jedec_id == 0x5e4018) && (flash->info->markid == ZB25VQ128D)))
+				((jedec_id == 0x5e4018) && (flash->info->markid == ZB25VQ128D)) ||
+				(jedec_id == 0xc46017)) /* GT25Q64A */
 			len = 16;
 		else
 			len = 8;
diff --git a/drivers/spinor/spinor_ids.c b/drivers/spinor/spinor_ids.c
index d625055..ae342f6 100644
--- a/drivers/spinor/spinor_ids.c
+++ b/drivers/spinor/spinor_ids.c
@@ -658,6 +658,7 @@ static struct spi_nor_otp otp_type20 = {0x7FD000, 0x001000, 512,  3, };
 static struct spi_nor_otp otp_type21 = {0x7FF000, 0x000000, 512,  1, };
 static struct spi_nor_otp otp_type22 = {0xFFD000, 0x001000, 512,  3, };
 static struct spi_nor_otp otp_type23 = {0xFFF000, 0x000000, 512,  1, };
+static struct spi_nor_otp otp_type18 = {0x000000, 0x000800, 1024, 3, };
 #endif
 
 #ifdef CONFIG_GX_MTD_SPI_NOR_32M_SIZE
@@ -2408,6 +2409,21 @@ spi_nor_info g_spi_nor_data[] = {
 	},
 #endif
 
+#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
+	{
+		"GT25Q64A",
+		0x00c46017,
+		0x800000,
+		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
+		NULL,
+		0,
+		&protect_8m_type1,
+#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
+		&otp_type18,
+#endif
+	},
+#endif
+
 #ifdef CONFIG_GX_MTD_SPI_NOR_4M_SIZE
 	{
 		"TH25Q32HA",
```



# 测试用例 
- 单线读写擦：
- 双线读写擦：
- 四线读写擦：
	- 之前测试 GT25Q32A 的时候 50M 四倍速才可以
	- 用 75M 左右的时钟测试 
- UID：手册描述 16 个字节，实际只有 8 个字节，后面 8 个字节是重复的
	- `30 72 10 0c 01 79 c0 07 30 72 10 0c 01 79 c0 07`
	- ok
- OTP：ok
- WP：ok
- 单页多次写：ok
- 掉电测试 ：


# 手册上传至云盘 


# 其它 
- `-S` 指的是 `2.3 ~ 3.6v`
- 75MHz 四线跑读写擦 2775 次失败：
```
>>>>>sflash_test:addr=0xa5900, len = 0x1ffa00
sflash_gx_readl finish.
sflash erase finish.
read addr=0xa5900, len=0x1ffa00
sflash_gx_readl finish.
sflash page program finish.
sflash_gx_readl finish.
cmp error. i = 1383235. read=0x91, orig=0x93
*****sflash_test_failed:addr=0xa5900, len = 0x1ffa00, times=2775




```
- 50MHz 四线跑读写擦 2775 次失败：
```
>>>>>sflash_test:addr=0xa5900, len = 0x1ffa00
sflash_gx_readl finish.
sflash erase finish.
read addr=0xa5900, len=0x1ffa00
sflash_gx_readl finish.
sflash page program finish.
sflash_gx_readl finish.
cmp error. i = 1383235. read=0x91, orig=0x93
*****sflash_test_failed:addr=0xa5900, len = 0x1ffa00, times=2775
```
