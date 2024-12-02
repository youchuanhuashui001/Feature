# 新增SPI NorFlash SX25Q64A

1. 涉及到的 `base.c` 中的函数：

   spi_nor_read_uniqueid
   
   spi_nor_calc_range  -

   spi_nor_write_protect_lock -
   
   spi_nor_otp_lock
   
   spi_nor_otp_status
   
   spi_nor_otp_erase
   
   spi_nor_otp_write
   
   spi_nor_otp_read
   
2. [v1.9.8-8.1](http://git.nationalchip.com/gerrit/#/q/status:open+project:spd/gxmisc+branch:v1.9.8-8.1)版本

   - 包括gxmisc，在gxmisc库中进行修改，修改完成后在gxloader中进行测试

3. v1.9.8-7版本

   - ```
     make menuconfig
     ```

     - 选择 `csky/arm`
     - 打开 nor flash 支持

4. ecos3.0 编译

   - 编译csky ecos报错：删除/opt/kernel-ecos
   - [修改env.sh](http://xn--env-5n0el78g.sh)，source一下
   - 编译ecos_shell：先编译gxlowpower，修改编译选项，只编译os和flash
   - 将开发板烧写能启动的.boot，然后用gdb load ecos_shell编译出的.elf文件

5. 串口打印xxx，bin没烧进去

   - 调整flash的座子

6. **DebugServerConsole连不上：**

   - **在board/gx3211/board-6622-dvbs2/board-init.c中打开DEBUG 复用(0)**

7. ecos_shell的命令配置：

   - 在编译的时候编进去的就会有命令，通过.c来实现，不是用.config实现的

8. ecos_shell跑起来之后无法输入、显示kernel问题

   - 重启开发板
   - 打开DebugServer
   - 连接gdb
   - 因为ecos已经起来了，没办法起2次，需要loader来对ecos进行一些初始化，所以要重启

9. ecos_shell：otp 测试

   - sflash otp test命令存在，但帮助信息没显示

10. 查看ecos的otp test函数如何编写的

    - 对照gxloader

11. 重新拉取lts版本的代码测试一遍

## 问题：

- 运行命令 flash wptest时cmp=1时都错误

  - 由于芯片手册中SR 命令需要分两次发送，发送SR1 + 命令，发送SR2 + 命令

  ![img](https://secure2.wostatic.cn/static/42VdMfe7KP5LE1vb58T1y2/image.png?auth_key=1676893627-jxRNdaaVn7sNtPH9SrikPC-0-91498f0f65f5a99855e8b5cf98d978bd)

![img](https://secure2.wostatic.cn/static/iobJrB1VtpEWGBNHRTxYL/image.png?auth_key=1676893627-r6gwopXaceG3QTnQxYdMt4-0-70c89805b9e19571ad892d68c5368254)

- 读取UID时不知道 A7~A0 如何设置，以及数据返回的大小

  - 将 A7~A0 设为F8读取UID，返回数据大小为64bit = 8Byte

  ![img](https://secure2.wostatic.cn/static/5soPjhatMUSsZJ4v61HNti/image.png?auth_key=1676893627-bhxsZZrMAAMH9DUugQGCMK-0-b84a66d339d6383e303fd97617a63e84)

![img](https://secure2.wostatic.cn/static/6AHUwemQMEuzUkKup8sdWo/image.png?auth_key=1676893627-cCiMGv8GajTSYszUuK2YVS-0-1a7d2f30d8c39834e173417226f73ebb)

![img](https://secure2.wostatic.cn/static/BHfsxBsWXULsEQhTyivgB/image.png?auth_key=1676893627-qxDonxRmcagtfhN5thKTiZ-0-e7630ac751469e585db9f04780ff57e8)

192.168.110.254

192.168.0.221

[git.nationalchip.com](http://git.nationalchip.com)

[gitbook.guoxintech.com](http://gitbook.guoxintech.com)

```C
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
tmp_status[0] &= 0110 0011 &= ~0x9c    none blocks
tmp_status[0] |= 0000 0000 |= 0x00

tmp_status[1] &= 1011 1110 &= ~0x41 cmp = 0
tmp_status[1] |= 0100 0000 |= 0x00

/* status1 mask1 status2 mask2 */
{0x00, 0x1c, 0x00, 0x40, {WP_BOTTOM, 0 * 0x10000}}        none blocks

tmp_status[0] &= 0000 0011 &= ~0xfc    2 blocks
tmp_status[0] |= 0010 0100 |= 0x24

tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 0
tmp_status[1] |= 0100 0000 |= 0x00

{0x24, 0xfc, 0x00, 0x40, {WP_BOTTOM, 2 * 0x10000}}        2 blocks

tmp_status[0] &= 0000 0011 &= ~0xfc    
tmp_status[0] |= 0010 1000 |= 0x28    2 blocks

tmp_status[0] |= 0010 1100 |= 0x2c    4
tmp_status[0] |= 0011 0000 |= 0x30    8
tmp_status[0] |= 0011 0100 |= 0x34    16
tmp_status[0] |= 0011 1000 |= 0x38    32
tmp_status[0] |= 0001 0100 |= 0x14    48
tmp_status[0] |= 0001 0000 |= 0x10    56
tmp_status[0] |= 0000 1100 |= 0x0c    60
tmp_status[0] |= 0000 1000 |= 0x08    62
tmp_status[0] |= 0000 0100 |= 0x04    63
tmp_status[0] |= 0001 1100 |= 0x1c    64

tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 0
tmp_status[1] |= 0000 0000 |= 0x00

tmp_status[0] &= 0000 0011 &= ~0xfc    96 blocks
tmp_status[0] |= 0001 0100 |= 0x14

tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 1
tmp_status[1] |= 0100 0000 |= 0x40

{0x14, 0xfc, 0x40, 0x40, {WP_BOTTOM, 96 * 0x10000}}        96 blocks

tmp_status[0] &= 0000 0011 &= ~0xfc    112 blocks
tmp_status[0] |= 0001 0000 |= 0x10

tmp_status[1] &= 1011 1111 &= ~0x40 cmp = 1
tmp_status[1] |= 0100 0000 |= 0x40

{0x10, 0xfc, 0x40, 0x40, {WP_BOTTOM, 112 * 0x10000}}        112 blocks
```