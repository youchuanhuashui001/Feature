# gxmisc+gxloader

## 编译：

1. gxloader

   1. 编译gxmisc：`./build csky gxloader`

   2. 使用`git diff`查看gxloader工程下flash相关的库是否更新

   3. 编译gxloader的.boot和.bin：

      `./build gx3211 6622-dvbs2 boot `

      `./build gx3211 6622-dvbs2 debug `

   4. 使用boot工具烧写bin文件到开发板：

      `sudo boot -b gx3211-6622-dvbs2-sflash.boot -c serialdown 0x0 output/loader-sflash.bin -d /dev/ttyUSB0`

   5. 修改.config配置：`conf/gx3211/6622-dvbs2/debug.config`
   
   6. 修改复用：`board/gx3211/board-6622-dvbs2/board-init.c`





## 程序：

1. GUN C编译器扩展关键字：`__attribute__`，用来申明一个函数、变量或类型的特殊属性，主要用途就是指导编译器在编译程序时进行特定方面的优化或代码检查。

2. 可以使用`__attribute__`来声明一个section属性，主要作用是：在程序编译时，将一个函数或变量放到指定的段，即放到指定的section中。

3. 程序的入口在main.c中,由于使用section来指定该函数为entry_text段，并在链接脚本中指定链接顺序，让此函数最先运行

   ![image-20220523194457514](image/image-20220523194457514.png)

![image-20220523194406872](image/image-20220523194406872.png)

4. 用户若需进行某些配置，只需在`user/user_config.c`中的user_init函数中编写，由于entry函数中调用了loader_init函数，loader_init函数进行了硬件的初始化，并调用`user_init`函数。

5. 命令的配置都在bootmenu.c中

6. spi flash

   ```c
   struct sflash_mater{
       int bus_num;  
       
       int bus_num;
       unsigned int num_chipselect;
       int (*setup)(struct spi_device *spi);
       int (*transfer)(struct spi_device *spi, struct spi_message *mesg);
       int (*cleanup)(struct spi_device *spi);
       void *data;
       void *driver_data;
       struct list_head list;
       
       unsigned int max_speed_hz;
       unsigned char chip_select;
       unsigned char bits_per_word;
       unsigned short mode;
       void *controller_state;
       
       struct list_head list;
   };
   
   
   spi_write_then_read()
       
   x[0].len = n_tx;
   x[1].len = n_rx;
   
   spi_message_add_tail(&x[i], &message);
   
   	*		   buf[31]
   local_buf --> buf
    
   memcpy(local_buf, txbuf, n_tx);
   txbuf --> local_buf --> buf[31]
   x[0].tx_buf = local_buf = buf[31]
   x[1].rx_buf = local_buf + n_tx
       
   spi_sync() --> message transfer/receive
       
    
   int flash_get_type(void)
   {
   #ifdef CONFIG_ENABLE_ADAPTIVE_
       
   #else
   
   #ifdef CONFIG_ENABLE_SFLASH
       if (flash_ops == &gxflash_spi_ops)
           	return 0;
   #endif
   
   #ifdef CONFIG_ENABLE_SPINAN
       if (flash_ops == &gxflash_sipnand_ops)
           	return 0;
   #endif
       
       
       
       
   int flash_get_used_device_map(char *map)
   {
   #ifdef CONFIG_ENABLE_SFLASH
       map[0] = 1;
   #endif
   #ifdef CONFIG_ENABLE_SPINAND
       map[1] = 1;
   #endif
   #ifdef CONFIG_ENABLE_NANDFLASH
       map[2] = 1;
   #endif
   }
       
       
   }
   
   ```

7. cmd执行过程：

   ```c
   entry --> bootmenu()
       --> while (1) -> readline("boot >") print boot> ，read cmd
           --> exec_string(cmd)
           	--> do_command
           
           
    
   #define COMMAND(cmd_, func_, help_)
           
   void func_(int argc, const char **);
   struct bootldr_command __attribute__ ((__section__(".data.commands"))) bootldr_cmd flash = {
       help,
       NULL,
       command_new_help
       " " help_,
       0
       
   }
           
           
   struct bootldr_command __commanddata bootldr_cmd_ ## cmd_ = { #cmd_, NULL, func_, #cmd_ " " help_, 0 }
   ```

   8. 在ops中实现spi nor、nand、并行nand的各个接口
      ![image-20220524151345506](image/image-20220524151345506.png)

   ![image-20220524151501695](image/image-20220524151501695.png)

9. 使用flash comtest命令会把flash中的数据用随机数的方式读写擦，可能会误擦除存放.boot和.bin文件的区域，重启后无法正常工作，要重新烧写bin文件

9. 程序写法：

   ```c
   unsigned      cs_change:1;
   unsigned若省略后一个关键字，大多数编译器都会认为是unsigned int
   cs_change:1：表示位域，即只有1位用来保存数据
   ```

   





## 命令：

1. gxloader中的bootmenu.c中所存在的命令，通过ops管理，实质是调用了gxmisc中实现的接口

   ```c
   static struct flash_op *flash_ops = NULL;
   
   extern struct flash_op gxflash_nand_ops;
   extern struct flash_op gxflash_spi_ops;
   extern struct flash_op gxflash_nor_ops;
   extern struct flash_op gxflash_spinand_ops;
   
   在gxflash_init(void)函数中通过宏定义来选择将哪一个ops赋值给flash_ops，在使用命令时，调用ops对应的接口(实现于gxmisc)
   
   inline int gxflash_readdata(unsigned int addr, unsigned char *data, unsigned int len)
   {
   ▸       if(flash_ops->readdata != NULL)
   ▸       ▸       return flash_ops->readdata(addr, data, len);
   ▸       else
   ▸       ▸       return -1;
   }
   
    struct flash_op gxflash_spi_ops = {
    ▸       .init                = spi_nor_init,
    #if defined(CONFIG_ENABLE_GXSPI)
    ▸       .readdata            = gx_spi_nor_readdata,
    #else
    ▸       .readdata            = gx_spi_nor_fast_readdata,
    #endif
    ▸       .gettype             = gx_spi_nor_gettype,
    ▸       .getinfo             = spi_nor_get_info,
    #ifdef CONFIG_ENABLE_FLASH_FULLFUNCTION
    ▸       .write_protect_mode  = gx_spi_nor_write_protect_mode,
    ▸       .write_protect_status= gx_spi_nor_write_protect_status,
    ▸       .write_protect_lock  = gx_spi_nor_write_protect_lock,
    ▸       .write_protect_unlock= gx_spi_nor_write_protect_unlock,
    ▸       .set_lock_sr         = gx_spi_nor_set_lock_reg,
    ▸       .get_lock_sr         = gx_spi_nor_get_lock_reg,
    ▸       .chiperase           = spi_nor_chiperase,
    ▸       .scruberase          = NULL,
    ▸       .erasedata           = spi_nor_erasedata,
    ▸       .erasedata_nospread  = spi_nor_erasedata,
    ▸       .pageprogram         = gx_spi_nor_pageprogram,
    ▸       .sync                = gx_spi_nor_sync,
    ▸       .test                = NULL,
    ▸       .calcblockrange      = gx_spi_nor_calcblockrange,
    ▸       .badinfo             = NULL,
    ▸       .pageprogram_yaffs2  = NULL,
    ▸       .readoob             = NULL,
    ▸       .writeoob            = NULL,
    ▸       .otp_lock            = gx_spi_nor_otp_lock,
    ▸       .otp_status          = gx_spi_nor_otp_status,
    ▸       .otp_erase           = gx_spi_nor_otp_erase,
    ▸       .otp_read            = gx_spi_nor_otp_read,
    ▸       .otp_write           = gx_spi_nor_otp_write,
    ▸       .otp_get_regions     = gx_spi_nor_otp_get_regions,
    ▸       .otp_set_region      = gx_spi_nor_otp_set_region,
    ▸       .otp_get_region_size = gx_spi_nor_otp_get_region_size,
    ▸       .otp_get_current_region = gx_spi_nor_otp_get_current_region,
    ▸       .uid_read            = gx_spi_nor_uid_read,
    #endif
    };
   ```

   2. flash comtest的执行

      ```c
      #ifdef CONFIG_ENABLE_FLASH_TEST
      ▸       else if (strcmp(argv[1], "comtest") == 0) {
      ▸       ▸       flash_complete_test();
      ▸       }
      
      flash_complete_test() --> gxflash_get_info
      	gxflash_write_protect_unlock --> flash_ops->write_protect_unlock--> (gxmisc)gx_spi_nor_write_protect_unlock
      --> sflash_test_single()  进行特定的测试
          --> while (1) sflash_test_single() 进行随机测试	
        
      sflash_test_sigle --> 
      	-->	gxflash_readdata()
          --> gxflash_erasedata()
          --> gxflash_readdata()
          --> cmp 数据是否全为0xff
          --> gxflash_pageprogram()
          --> gxflash_readdata()
      ```
      
   3. flash的初始化
   
      ```c
      entry()
          --> loader_init()
          --> gxflash_init()
          --> gx_spi_probe()
          --> spi_register_master(&gx_spi_master)
      
      static struct spi_master gx_spi_master = {
          .bus_num        = 0,
          .num_chipselect = 3,
          .setup          = gx_spi_setup,
          .transfer       = gx_spi_transfer,
          .cleanup        = gx_spi_cleanup,
          .data           = (void *)REG_BASE_SPI,
      }
      
      gx_flash_init()
          --> 根据宏定义来初始化对应的flash
          --> gxflash_spi_ops.init()
          --> .init = spi_nor_init
          --> spi_nor_init
          --> gx_spi_nor_init()
          --> spi_setup()
          --> return spi->master->setup(spi)
          --> gx_spi_setup
      ```
   
   4. 命令执行
   
      ```c
      (*command->cmdfunc)(argc - command->offset, argv + command->offset);
      
      
      struct bootldr_command {
      ▸       const char *cmdstr;
      ▸       const char *subcmd;
      ▸       void (*cmdfunc)(int argc, const char **);
      ▸       const char *helpstr;
      ▸       int offset;   /* number of args to remove from the left side */
      };
                                                                                                                                                        
      #define __commanddata▸  __attribute__ ((__section__ (".data.commands")))
      
      #define COMMAND(cmd_, func_, help_) \
      ▸       void func_(int argc, const char **); \
      struct bootldr_command __commanddata bootldr_cmd_ ## cmd_ = { #cmd_, NULL, func_, #cmd_ " " help_, 0 }
      
      #define SUBCOMMAND(cmd_, sub_, func_, help_, offset_) \
      ▸       void func_(int argc, const char **); \
      struct bootldr_command __commanddata bootldr_cmd_ ## cmd_ ## sub_ = { #cmd_, #sub_, func_, #cmd_ " " #sub_ " " help_, offset_ }
      
      #define CMD(cmd_) \
      ▸       &bootldr_cmd_ ## cmd_
      
      #define SUBCMD(cmd_, sub_) \
      ▸       &bootldr_cmd_ ## cmd_ ## sub_
      
      
      COMMAND(flash, command_flash, "<command> [args...] - flash operation");
      
      #defeine COMMAND(cmd_, func_, help_)
      	void func_(int argc, const char **);
      	struct bootldr_command __commanddata bootldr_cmd_ ## cmd_ = {
              #cmd_, 
              NULL, 
              func_, 
              #cmd_ " " help_,
              0
          }
      
      
      #defeine COMMAND(cmd_, func_, help_)
      	void func_(int argc, const char **);
      	struct bootldr_command __commanddata bootldr_cmd_ ## cmd_ = {
              #cmd_, 
              NULL, 
              func_, 
              #cmd_ " " help_,
              0
          }
      
      
      #defeine COMMAND(flash, command_flash, "<command> [args...] - flash operation")                                                                   
      
      ▸       void command_flash(int argc, const char **);
      
      ▸       struct bootldr_command __attribute__ ((__section__ (".data.commands"))) bootldr_flash ## cmd_ = {
              flash,
              NULL,
              command_flash,
              " flash" "<command> [args...] - flash operation",
              0
          }
      ```
   
      



## 问题：

1. boot工具

   使用时先拷贝到`/usr/bin`目录，然后使用时要添加`sudo`

2. 无法烧写程序：

   芯片正常启动时会在串口打印X字符，若未正常打印可能是串口小板的问题，拨动开关后再试

2. 芯片正常启动但无法通过串口输入

   由于minicom的配置，应关闭流控

4. gx_get_chip_id中的PUBLIC_ID从哪来的
   ![image-20220524134037609](image/image-20220524134037609.png)

![image-20220524134057028](image/image-20220524134057028.png)

5. 使用gdbserver调试

   1. 下载调试器，无法打开，可能是调试器硬件接线有问题

   2. 调试时要打开在.config中打开gdb调试，并且打开代码io口复用

   3. 缺少库：`libexpat.so.0`

      首先您需要安装 32 位的`libexpat-dev`软件包：

      ```
      sudo apt-get install libexpat-dev:i386
      ```

      现在您可以创建一个符号链接`libexpat.so.0`作为链接名称和`libexpat.so.1`目标：

      ```
      sudo ln -s /lib/i386-linux-gnu/libexpat.so.1 /lib/i386-linux-gnu/libexpat.so.0
      ```

   4. 在flash中烧写可以正常运行的bin，并在boot倒计时阶段敲回车

   5. 打开J-Tag后台：`DebugServerConsole -setclk 2`

   6. 在包含要加载的.elf文件的目录下创建.gdbinit文件，并添加以下内容：

      ```shell
      target jtag jtag://127.0.1.1:1025
      load
      ```

   7. 使用命令`csky-elf-gdb loader.elf`，即可正常使用gdb调试

      注：使用打开gdb调试的bin去烧写，无法进入系统

6. 烧写bin文件后，程序能够正常运行，但重启之后就只打印xxx
   1. 由于烧写完之后运行的还是.boot文件，还没有使用.bin，断电重启之后使用的是.bin所以只打印xxx
   2. `sudo boot -b gx3211-6622-dvbs2-sflash.boot -c serialdown 0x0 output/loader-sflash.bin -d /dev/ttyUSB0`
   3. 先加载.boot文件到ddr，然后执行命令`serialdown`去下载bin文件到flash中，这个过程boot文件只负责发送spi命令，不管flash是否接收到
   4. 下载完成之后关闭掉当前的串口消息发送，并继续执行`boot> `，所以能够在烧写完成后看到程序是正常运行的，但是重启之后只打印xxx
   
7. 使用打开gdb调试的bin去烧写，无法进入系统

8. 使用otplock后进行otptest，只锁了区域2
   ![image-20220526093355021](image/image-20220526093355021.png)

9. 初始化：

   ```c
   flash_init()
       --> gx_spi_setup
       --> writel(CONTROL_VALUE, &regs->SPI_CR)
   ```

   

   ![image-20220526105203625](image/image-20220526105203625.png)



![image-20220526110046625](image/image-20220526110046625.png)

10. gx_spi_transfer中的`struct spi_regs *regs=(struct spi_regs *)spi->master->data;`，是怎么获取的地址？
    首先，spi控制器要有基地址，然后这些寄存器是在这些基地址上的偏移吧

    regval = readl(&regs->SPI_CR);  (括号中的应该是一个地址)

    writel(regval, &regs->SPI_CR); (括号中是对)

    也没有看见具体的spi相关的配置

11. spi_write_then_read、spi_sync

    ```
    spi_write()
    	--> spi_sync
    	--> gx_spi_transfer
    ```

    