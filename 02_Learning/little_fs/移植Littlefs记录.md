# 移植 Littlefs

## 1. 简介

Littlefs 是一个轻量级的文件系统，适用于嵌入式系统。它具有以下特点：

- 小巧：Littlefs 的代码量非常小，适合资源受限的嵌入式设备。
- 快速：Littlefs 的读写速度非常快，适合需要频繁读写数据的场景。
- 可靠：Littlefs 提供了数据完整性和一致性保护，能够有效防止数据损坏和丢失。

- Github：https://github.com/littlefs-project/littlefs

## 2. 移植

### 2.1 需要移植的文件

- 移植到 Linux 需要移植的文件有：
  - `lfs.c`
  - `lfs.h`
  - `lfs_util.c`
  - `lfs_util.h`
- 不需要移植的文件有：
  - `fs/littlefs/bd/`
    这些都是模拟的文件(模拟的磁盘、ram 等)，不需要移植

### 2.2 移植步骤

- 修改 Makefile
  - 在 `Makefile` 文件中，添加 .c 文件编译：
    ```makefile
    obj-$(CONFIG_LITTLEFS) += lfs.o lfs_util.o
    ```
  - 在 `Makefile` 文件中，添加 .h 文件查找路径
    ```makefile
    EXTRA_CFLAGS += -I ./fs/littlefs
    ```
- 解决编译问题
  - `fs/littlefs/lfs_util.h` 中去掉 `#include <inttypes.h>`
  - `littlefs` 默认用的是 `C99` 标准，而我们没有用标准 C 库，所以需要修改 `lfs.c` 文件中的一些在 `for` 循环中使用 `int` 类型变量的问题，以及一些其它宏的问题

- 添加测试程序
  - 在 `main.c` 文件中，添加如下内容：
    ```diff
    --- a/main.c
    +++ b/main.c
    @@ -230,6 +230,112 @@ static void gx_print_info(void)
    
     	flash_switch_type(type);
     }
    +#include "lfs.h"
    +
    +// variables used by the filesystem
    +lfs_t lfs;
    +lfs_file_t file;
    +
    +int lfs_spi_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    +    // check if read is valid
    +    LFS_ASSERT(off % cfg->read_size == 0);
    +    LFS_ASSERT(size % cfg->read_size == 0);
    +    LFS_ASSERT(block < cfg->block_count);
    +    gxflash_readdata(0x300000+block * cfg->block_size + off, (uint8_t *)buffer, size);
    +    return LFS_ERR_OK;
    +}
    +
    +/*
    + * @brief 将数据写入指定块内的某区域。该区域必须已经先被擦除过，可以返回 LFS_ERR_CORRUPT 表示该块已损坏
    + * @param [in] lfs_config格式参数
    + * @param [in] block 逻辑块索引号，从0开始
    + * @param [in] off 块内偏移，该值需能被rprog_size整除
    + * @param [in] 写入数据的缓冲区
    + * @param [in] size 要写入的字节数，该值需能被read_size整除，lfs在读取时会确保不会跨块；
    + * @retval 0 成功, < 0 错误码
    + */
    +int lfs_spi_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    +    // check if write is valid
    +    LFS_ASSERT(off % cfg->prog_size == 0);
    +    LFS_ASSERT(size % cfg->prog_size == 0);
    +    LFS_ASSERT(block < cfg->block_count);
    +    gxflash_pageprogram(0x300000+block * cfg->block_size + off, (uint8_t *)buffer, size);
    +    return LFS_ERR_OK;
    +}
    +
    +/*
    + * @brief 擦除指定块。块在写入之前必须先被擦除过，被擦除块的状态是未定义
    + * @param [in] lfs_config格式参数
    + * @param [in] block 要擦除的逻辑块索引号，从0开始
    + * @retval 0 成功, < 0 错误码
    + */
    +int lfs_spi_flash_erase(const struct lfs_config *cfg, lfs_block_t block) {
    +    // check if erase is valid
    +    LFS_ASSERT(block < cfg->block_count);
    +    gxflash_erasedata(0x300000+block * cfg->block_size, 64*1024);
    +    return LFS_ERR_OK;
    +}
    +
    +/*
    + * @brief 对底层块设备做同步操作。若底层块设备不没有同步这项操作可以直接返回
    + * @param [in] lfs_config格式参数;
    + * @retval 0 成功, < 0 错误码
    + */
    +int lfs_spi_flash_sync(const struct lfs_config *cfg) {
    +    return LFS_ERR_OK;
    +}
    +
    +
    +// configuration of the filesystem is provided by this struct
    +const struct lfs_config cfg = {
    +    // block device operations
    +    .read  = lfs_spi_flash_read,
    +    .prog  = lfs_spi_flash_prog,
    +    .erase = lfs_spi_flash_erase,
    +    .sync  = lfs_spi_flash_sync,
    +
    +    // block device configuration
    +    .read_size = 256,
    +    .prog_size = 256,
    +    .block_size = 64*1024,
    +    .block_count = 128,
    +    .cache_size = 512,
    +    .lookahead_size = 512,
    +    .block_cycles = 500,
    +};
    +
    +// entry point
    +void test_littfs(void) {
    +    // mount the filesystem
    +    int err = lfs_mount(&lfs, &cfg);
    +
    +    // reformat if we can't mount the filesystem
    +    // this should only happen on the first boot
    +    if (err) {
    +        lfs_format(&lfs, &cfg);
    +        lfs_mount(&lfs, &cfg);
    +    }
    +
    +    // read current count
    +    uint32_t boot_count = 0;
    +    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    +    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));
    +
    +    // update boot count
    +    boot_count += 1;
    +    lfs_file_rewind(&lfs, &file);
    +    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));
    +
    +    // remember the storage is not updated until the file is closed successfully
    +    lfs_file_close(&lfs, &file);
    +
    +    // release any resources we were using
    +    lfs_unmount(&lfs);
    +
    +    // print the boot count
    +    printf("boot_count: %d\n", boot_count);
    +}
    +
    
     void __attribute__((section(".entry_text")))entry(uint32_t ddr_size)
     {
    @@ -263,6 +369,7 @@ void __attribute__((section(".entry_text")))entry(uint32_t ddr_size)
     #ifdef CONFIG_ENABLE_RELEASE_CHECK
     	gx_release_check();
     #endif
    +	test_littfs();
    
     #ifdef CONFIG_ENABLE_CMD
     	bootmenu();

    ```

## 3. 运行测试

- 编译
  - 编译时需要添加 `CONFIG_LITTLEFS=y` 选项
- 运行
  - 运行后可以看到打印的 `boot_count` 值，每次重启都会读出 `boot_count` 值并加一存储到文件中
  ```
  1183 lfs_dir_fetchmatch
  1183 lfs_dir_fetchmatch
  boot_count: 43609

  1183 lfs_dir_fetchmatch
  1183 lfs_dir_fetchmatch
  boot_count: 43610
  ```


## 4. 整理

- 移植过程中遇到的问题：
  - 由于 `loader` 会存在 0x0 开始的地址，所以需要将文件系统操作的地址都偏移到 `loader` 用不到的地址处
  - 由于 `loader` 默认不会开启 flash 的 `write and erase` 功能，所以需要手动开启

- 相关代码保存到 `littlefs` 文件夹
