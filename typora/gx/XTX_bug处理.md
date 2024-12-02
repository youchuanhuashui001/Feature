# XTX 新增 FLASH





## XT25F64F：

​		xt25f64f 临时使用 xt25f64b-s 进行修改，差异如下：(后续还需向厂家确认这两款 flash 如何进行区分)

```diff
diff --git a/drivers/spinor/spinor_base.c b/drivers/spinor/spinor_base.c
index 17cbf1e..1890ae7 100644
--- a/drivers/spinor/spinor_base.c
+++ b/drivers/spinor/spinor_base.c
@@ -350,13 +350,17 @@ int spi_nor_read_uniqueid(struct spi_nor_flash *flash, unsigned char *buf, unsig
                len = 17;
                break;
        case 0x0b: /* xtx */
-               cmd_dummy[0] = 0x5A;
-               cmd_dummy[1] = 0x00;
-               if (jedec_id == 0x0b4018)
-                       cmd_dummy[2] = 0x00;
-               else
-                       cmd_dummy[2] = 0x01;
-               cmd_dummy[3] = 0x94;
+               if (jedec_id == 0x0b4017) {
+                       cmd_dummy[0] = 0x4B;
+               } else {
+                       cmd_dummy[0] = 0x5A;
+                       cmd_dummy[1] = 0x00;
+                       if (jedec_id == 0x0b4018)
+                               cmd_dummy[2] = 0x00;
+                       else
+                               cmd_dummy[2] = 0x01;
+                       cmd_dummy[3] = 0x94;
+               }
                cmd_dummy_len = 5;
                len = 16;
                break;
@@ -1472,7 +1476,7 @@ int spi_nor_otp_lock(struct spi_nor_flash *flash)
                                        || (flash->info->jedec_id == 0xa12816)
                                        || (flash->info->jedec_id == 0xa14017)
                                        || (flash->info->jedec_id == 0xa14018))
-                               status[1] |= 1<<2;
+                               status[1] |= 1<<(3 + region);
                        else if (flash->info->jedec_id == 0x0b4018)
                                        status[1] |= 3<<2;
                        else
@@ -1585,7 +1589,7 @@ int spi_nor_otp_status(struct spi_nor_flash *flash, unsigned char *buf)
                                        || (flash->info->jedec_id == 0xa12816)
                                        || (flash->info->jedec_id == 0xa14017)
                                        || (flash->info->jedec_id == 0xa14018))
-                               otp_lock = (status[0] >> 2) & 0x01;
+                               otp_lock = (status[0] >> (3 + region)) & 0x01;
                        else if (flash->info->jedec_id == 0x0b4018)
                                otp_lock = ((status[0] >> 2) & 0x03) == 0x03;
                        else
diff --git a/drivers/spinor/spinor_ids.c b/drivers/spinor/spinor_ids.c
index a518acc..baebe78 100644
--- a/drivers/spinor/spinor_ids.c
+++ b/drivers/spinor/spinor_ids.c
@@ -1460,7 +1460,7 @@ static struct spi_nor_flash_protect_range xt25f64b_ranges[] = {
        { 0x1c, 0x9c, 0x00, 0x41, {WP_BOTTOM, 128 * 0x10000} },
 };
 #endif
-static struct spi_nor_otp xt25f64b_otp = {0x000000, 0x000100, 256,  4, };
+static struct spi_nor_otp xt25f64b_otp = {0x001000, 0x001000, 1024,  3, };
 #endif
 
 #ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
(END)


```



## xt25f64f 复现掉电测试 bug：

发送uid：0x4B

```diff
diff --git a/drivers/flash/flash_test.c b/drivers/flash/flash_test.c
index 322aa9c7..a3767b07 100644
--- a/drivers/flash/flash_test.c
+++ b/drivers/flash/flash_test.c
@@ -17,6 +17,7 @@
 
 static uint64_t holdrand_test = 1;
 static uint32_t flash_random_seed = 1;
+void flash_uid_test(void);
 void rand_init(uint32_t seed)
 {
        flash_random_seed = seed;
@@ -1298,8 +1299,11 @@ void flash_power_cut_test(void)
                }
        }
 
+
        if (wrong_block_num > 1) {
                printf("wrong data block num: %d\n", wrong_block_num);
+               flash_uid_test();
+               while(1);
                goto EXIT;
        }
 
@@ -1331,6 +1335,7 @@ EXIT:
        printf("test exit\n");
        free(wb);
        free(rb);
+
 }
 

```







## XT25F128F:

​		gxmisc 支持列表中的 `XT25F128B` 删除掉，并新增 `XT25F128F`

```c
//spinor_ids.c

1491 #ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
1492 static struct spi_nor_flash_protect xt25f128b_protect = {NULL, 0};                                                                                                                                                         
1493 #ifdef CONFIG_GX_MTD_SPI_NOR_WRITE_PROTECT
1494 static struct spi_nor_flash_protect_range xt25f128b_ranges[] = {
1495         /*
1496         ┆* xt25f64b datasheet 16M byte,company is xtx;
1497         ┆*/
1498         { 0x00, 0x9c, 0x00, 0x41, {WP_BOTTOM, 0   * 0x10000} },
1499         { 0x24, 0xfc, 0x00, 0x41, {WP_BOTTOM, 4   * 0x10000} },
1500         { 0x28, 0xfc, 0x00, 0x41, {WP_BOTTOM, 8   * 0x10000} },
1501         { 0x2c, 0xfc, 0x00, 0x41, {WP_BOTTOM, 16  * 0x10000} },
1502         { 0x30, 0xfc, 0x00, 0x41, {WP_BOTTOM, 32  * 0x10000} },
1503         { 0x34, 0xfc, 0x00, 0x41, {WP_BOTTOM, 64  * 0x10000} },
1504         { 0x38, 0xfc, 0x00, 0x41, {WP_BOTTOM, 128 * 0x10000} },
1505         { 0x14, 0xfc, 0x40, 0x41, {WP_BOTTOM, 192 * 0x10000} },
1506         { 0x10, 0xfc, 0x40, 0x41, {WP_BOTTOM, 224 * 0x10000} },
1507         { 0x0c, 0xfc, 0x40, 0x41, {WP_BOTTOM, 240 * 0x10000} },
1508         { 0x08, 0xfc, 0x40, 0x41, {WP_BOTTOM, 248 * 0x10000} },
1509         { 0x04, 0xfc, 0x40, 0x41, {WP_BOTTOM, 252 * 0x10000} },
1510         { 0x1c, 0x9c, 0x00, 0x41, {WP_BOTTOM, 256 * 0x10000} },
1511 };
1512 #endif
1513 static struct spi_nor_otp xt25f128b_otp = {0x000000, 0x000100, 256,  4, }; 
1514 #endif									   {0x001000, 0x001000, 1024, 3, };


3295 #ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
3296         {
3297                 //note: XT25F128BSSIGU
3298                 "XT25F128B", --> XT25F128F
3299                 0x000b4018,
3300                 0x1000000,
3301                 SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ,
3302                 NULL,
3303                 0,
3304                 &xt25f128b_protect,
3305                 &xt25f128b_otp,
3306         },
3307 #endif


// spinor_base.c

// spi_nor_read_uniqueid
 288                 cmd_dummy[0] = 0x4B;                                                
 289                 cmd_dummy_len = 5;                                            
 290                 len = 16;

// spi_nor_calc_range
 994                 case 0x0b:  //xtx 
 995                 case 0x68:  //BYT  
 996                 case 0x85:  //p25q* 
 997                 case 0xba:  //zetta  
 998                 case 0xc8:  //Gigadevice  
 999                 case 0xb3: /* ucun */   
1000                 case 0x8f: /* sx */ 
1001                 case 0xc4:  //gt 
1002                         tmp_status[0] = read_sr1(spi); 
1003                         tmp_status[1] = read_sr2(spi); 
1004                         break;  

// spi_nor_otp_lock

                               status[1] |= 1<<(3 + region);

// spi_nor_otp_status
								otp_lock = (status[0] >> (3 + region)) & 0x01;
```





## SPI NAND

1. 开启ecc，擦除1个block                                                                                                                                                                                    
2. 读取该block数据，验证每个bit是否为1
3. 关闭ecc，将该块的第1个page的第1个字节其中1个bit修改为0
4. 将修改后的数据写入page，伪造出1bit翻转
5. 开启ecc，读取该page数据是为0xFE
6. 读取到的数据未被纠正回来，且读取 SR 寄存器中的 ECCS3、ECCS2、ECCS1、ECCS0 位为 0000

