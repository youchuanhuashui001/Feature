# 新增SPI NorFlash SX25Q64A

1. `g_spi_nor_data`中添加：

   ```
   ID：0x008f4017
   ```

   ![image-20220620105245907](image/image-20220620105245907.png)

![image-20220620105221995](image/image-20220620105221995.png)



2. 修改otp：`0100 0000 0000 0000:4000`、 `1000 0000 0000 0000:8000`、 `1100 0000 0000 0000:C000`

   start：4000，step：4000

   ![image-20220620110630717](image/image-20220620110630717.png)

![image-20220620110737029](image/image-20220620110737029.png)

![image-20220620110558320](image/image-20220620110558320.png)

3. 修改`spi_nor_flash_protect_range`

   ![image-20220620124022079](image/image-20220620124022079.png)

   ![image-20220620124000017](image/image-20220620124000017.png)

   ```c
   /* 查看程序中使用数据的方式 */
   if(dev_mask[0] != 0){
   	tmp_status[0] = read_sr1(spi);
   	tmp_status[0] &= ~dev_mask[0];
   	tmp_status[0] |= dev_status[0];
   	write_sr(spi, &tmp_status[0], 1, 1);
   	wait_till_ready(spi);
   }
   
   if(dev_mask[1] != 0){
   	tmp_status[1] = read_sr2(spi);
   	tmp_status[1] &= ~dev_mask[1];
   	tmp_status[1] |= dev_status[1];
   	wait_till_ready(spi);
   	write_sr(spi, &tmp_status[1], 1, 2);
   
   
   /* 
    * 根据程序来查看数组中的数据
    * tmp_status[0]对应SR寄存器中的低8位
    * tmp_status[1]对应SR寄存器中的高8位
    * 查看SR寄存器CMP位和BP4~BP0位
   */
   tmp_status[0] &= 1110 0011 &= ~0x1c	none blocks
   tmp_status[0] |= 0000 0000 |= 0x00
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 0
   tmp_status[1] |= 0100 0000 |= 0x00
    
   /* status1 mask1 status2 mask2 */
   {0x00, 0x1c, 0x00, 0x40, {WP_BOTTOM, 0 * 0x10000}}		none blocks
       
   
   tmp_status[0] &= 0000 0011 &= ~0xfc	2 blocks
   tmp_status[0] |= 0010 0100 |= 0x24
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 0
   tmp_status[1] |= 0100 0000 |= 0x00
       
   {0x24, 0xfc, 0x00, 0x40, {WP_BOTTOM, 2 * 0x10000}}		2 blocks
       
   
   tmp_status[0] &= 0000 0011 &= ~0xfc	4 blocks
   tmp_status[0] |= 0010 1000 |= 0x28
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 0
   tmp_status[1] |= 0000 0000 |= 0x00
       
   {0x28, 0xfc, 0x00, 0x40, {WP_BOTTOM, 4 * 0x10000}}		4 blocks
       
   
   tmp_status[0] &= 0000 0011 &= ~0xfc	8 blocks
   tmp_status[0] |= 0010 1100 |= 0x2c
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 0
   tmp_status[1] |= 0000 0000 |= 0x00
       
   {0x2c, 0xfc, 0x00, 0x40, {WP_BOTTOM, 8 * 0x10000}}		8 blocks
       
       
   tmp_status[0] &= 0000 0011 &= ~0xfc	16 blocks
   tmp_status[0] |= 0011 0000 |= 0x30
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 0
   tmp_status[1] |= 0000 0000 |= 0x00
       
   {0x30, 0xfc, 0x00, 0x40, {WP_BOTTOM, 16 * 0x10000}}		16 blocks
       
   tmp_status[0] &= 0000 0011 &= ~0xfc	32 blocks
   tmp_status[0] |= 0011 0100 |= 0x34
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 0
   tmp_status[1] |= 0000 0000 |= 0x00
       
   {0x34, 0xfc, 0x00, 0x40, {WP_BOTTOM, 32 * 0x10000}}		32 blocks
       
   
   tmp_status[0] &= 0000 0011 &= ~0xfc	64 blocks
   tmp_status[0] |= 0011 1000 |= 0x38
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 0
   tmp_status[1] |= 0000 0000 |= 0x00
       
   {0x38, 0xfc, 0x00, 0x40, {WP_BOTTOM, 64 * 0x10000}}		64 blocks
       
       
   tmp_status[0] &= 0110 0011 &= ~0x9c	128 blocks
   tmp_status[0] |= 0001 1100 |= 0x1c
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 0
   tmp_status[1] |= 0000 0000 |= 0x00
       
   {0x1c, 0x9c, 0x00, 0x40, {WP_BOTTOM, 128 * 0x10000}}		128 blocks
       
       
   tmp_status[0] &= 0000 0011 &= ~0xfc	96 blocks
   tmp_status[0] |= 0001 0100 |= 0x14
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 1
   tmp_status[1] |= 0100 0000 |= 0x40
       
   {0x14, 0xfc, 0x40, 0x40, {WP_BOTTOM, 96 * 0x10000}}		96 blocks
       
       
   tmp_status[0] &= 0000 0011 &= ~0xfc	112 blocks
   tmp_status[0] |= 0001 0000 |= 0x10
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 1
   tmp_status[1] |= 0100 0000 |= 0x40
       
   {0x10, 0xfc, 0x40, 0x40, {WP_BOTTOM, 112 * 0x10000}}		112 blocks
       
       
   tmp_status[0] &= 0000 0011 &= ~0xfc	120 blocks
   tmp_status[0] |= 0000 1100 |= 0x0c
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 1
   tmp_status[1] |= 0100 0000 |= 0x40
       
   {0x0c, 0xfc, 0x40, 0x40, {WP_BOTTOM, 112 * 0x10000}}		120 blocks
       
       
   tmp_status[0] &= 0000 0011 &= ~0xfc	124 blocks
   tmp_status[0] |= 0000 0100 |= 0x04
   
   tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 1
   tmp_status[1] |= 0100 0000 |= 0x40
       
   {0x04, 0xfc, 0x40, 0x40, {WP_BOTTOM, 112 * 0x10000}}		124 blocks
   ```



# 问题

1. read_uniqueid：

   ​	A7~A0:怎么设置值  00

   ​	len：要接收的数据长度(Byte)   读取uid的时候确认

2. calc_range:这个是跟protect region有关的吗？就是protect region保护时用了哪几个寄存器

3. ![image-20220620143853644](image/image-20220620143853644.png)

- ![image-20220620150941906](image/image-20220620150941906.png)

![image-20220620150956662](image/image-20220620150956662.png)

- 芯片手册写状态寄存器(SR)要分别发不同的指令：第一次发01来写SR1，第二次发31来写SR2



4. 执行断电测试：

   ![image-20220620153156709](image/image-20220620153156709.png)









# 新增 Nor Flash



## GoXceed



## Robots



## Leo_mini

- 功能：ID、单双四线读写、XIP
  - 无写保护，无 otp，无 uid

- 测试用例：
  - `flash_test comtest`
  - `md 0x30000000(带 cache); md 0x50000000(不带 cache)`

- 需要关注：

  - 使能四倍速的方式`(0x01 写单个寄存器 或 0x01 写两个寄存器)`

  - XIP 测试方法：关掉 stage1 的 xip 初始化，关掉 stage2 的 xip 初始化，md xip 地址直接卡死

    ```diff
    --- a/arch/csky/mach-leo_mini/spinor/spinor.c
    +++ b/arch/csky/mach-leo_mini/spinor/spinor.c
    @@ -504,6 +504,7 @@ static int sflash_init(void)
     
     #ifdef CONFIG_FLASH_SPI_QUAD
            sflash_enable_quad(jedec);
    +#if 0
            if (jedec == 0xe53118) { /* lc25q4aa */
                    spl_xip_init(0xeb, 8, 1,
                                    24, 4,
    @@ -516,7 +517,9 @@ static int sflash_init(void)
                                    0x00, 1,
                                    4, 4);
            }
    +#endif
     #else
    +#if 0
            if (jedec == 0xe53118) { /* lc25q4aa */
                    spl_xip_init(0xbb, 8, 1,
                                    24, 2,
    @@ -529,6 +532,7 @@ static int sflash_init(void)
                                    0x00, 1,
                                    0, 2);
            }
    +#endif
     #endif
    
    
    --- a/drivers/legacy/mtd/spinor/flash_spi.c
    +++ b/drivers/legacy/mtd/spinor/flash_spi.c
    @@ -1894,6 +1894,7 @@ static GX_FLASH_DEV * sflash_init(int sample_delay,int cs, int speed, int mode)
            g_flash_info.program_page = sflash_standard_1pageprogram;
            g_flash_info.read_data = sflash_dual_read;
     #ifdef CONFIG_FLASH_SPI_XIP
    +#if 0
            if (g_flash_info.info->jedec_id == 0xe53118) { /* lc25q4aa */
                    gx_xip_init(0xbb, 8, 1,
                                    24, 2,
    @@ -1907,12 +1908,14 @@ static GX_FLASH_DEV * sflash_init(int sample_delay,int cs, int speed, int mode)
            }
     #endif
     #endif
    +#endif
    ```

    



### Apus

- 功能：ID、单双四线读写、XIP、UID、写保护

- 测试用例：

  - 读写擦、XIP
  - UID 
  - 写保护
  - 单页多次写测试
  - 掉电数据是否异常测试
  - 不同频率下的相位范围：`board/nationalchip/apus_nre_1v/clk.c`

- 跟帖模板：

  ```
  * 补丁：{{patch()}}
  |/6. |4,2,1倍速读写擦测试||
  |xip读写测试||
  |写保护测试||
  |uid读取||
  |单页多次写测试||
  |掉电300次测试||
  
  ---
  
  * 不同频率下的 sample_delay 范围测试：
  | core电压 | 实测VDD09电压(V) | 时钟源pll | cpu分频系数 | cpu频率 | xip时钟源分频系数 | xip内部分频 | xip工作频率 | xip sample delay范围值 | xip sample delay建议值 | 备注                                   |
  | 1.0       |                 | 440       | 4           | 110     | 2                 | 2           | 110         |                        |                        |    无可用相位                          |
  | 1.1       |                 | 160       | 1           | 160     | 1                 | 2           | 80          |      3, 4, 5           |                        |    读写擦测试通过                      |
  | 1.1       |                 | 144       | 1           | 144     | 1                 | 2           | 72          |      3, 4, 5           |                        |    读写擦测试通过                      |
  | 1.1       |                 | 128       | 1           | 128     | 1                 | 2           | 64          |      2, 3, 4, 5        |                        |    读写擦测试通过                      |
  | 1.0       |                 | 96        | 1           | 96      | 1                 | 2           | 48          |         2, 3, 4        |                        |    读写擦测试通过                      |
  | 0.95      |                 | 64        | 1           | 64      | 1                 | 2           | 32          |         1, 2, 3        |                        |    左侧为 1.0v 测试情况，读写擦测试通过|
  | 0.9       |                 | 32        | 1           | 32      | 1                 | 2           | 16          |      0, 1, 2           |                        |    读写擦测试通过                      |
  
  ```

- 更新 `wiki` 页面支持列表