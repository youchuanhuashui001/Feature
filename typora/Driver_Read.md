# gxloader

## bootmenu.c


### flash_oob_test

- `gxflash_write_protect_unlock` 解锁 Flash 写保护
- `flash_bbt_block_test`
    - 调用 gxmisc 提供的接口 `gxflash_badinfo` 判断返回结果是否正常
        - gx_spinand_badinfo
            - 遍历每个 block，每个 block 都从坏块表中读当前 block 的状态是否是坏块
            - 如果有坏块就打印
    - 调用 gxmisc 接口 `gxflash_get_info` 获取 block_size
    - 让 block5 模拟成 badblock
    - 计算 block5 对应的地址:bad_addr = 5 * block_size
    - 遍历 20 个测试 block
        - 每个 block 测试长度 0x2000，相当于申请了 20个 src_buffer 和 20 个 dst_buffer，每个 buffer 长度为 0x2000
    - 循环：将 src_buffer[i] 都赋值成 i
    - 将 dst_buffer[i] 都赋值成 0x5a
    - 调用 gxmisc 接口 `gxflash_erasedata_noskip`:擦除 block，不跳过坏块
        - 先将擦除的地址和长度做 block 对齐
        - 组织好 addr、len，调用 `spinand_erase_opts(&g_spinand_chip, &opts)`
            - 计算要擦除的长度：erase_length
            - 循环做擦除操作，每次增加的地址是 block_size
                - 判断当前地址是否是 bad_block
                    - 如果 `opts->scrub == 0 放弃当前 block 的擦除操作，继续遍历`
                    - 如果 `opts->scrub == 1` 调用 `chip->erase_block(chip, erase_addr/chip->info->block_main_size)
                        - write_enable
                        - 擦除 block
                        - wait_ready
                    - 如果擦除失败
                        - 如果 `opts->scrub == 0` 标记坏块表，并更新到 flash
            - 如果 opts->scrub == 1，并且内存中存在 bbt 表的话，先把 bbt 表释放掉，再重新构建 bbt 表
    - 调用 `imitate_block(bad_block)` 标记 block5 为 bad block
    - 从 block0 到 block6，循环每个 block
        - 将 src_buffer 的数据写入到每个 block
        - 将每个 block 的数据读到 dst_buffer
    - 调用 `gxflash_erasedata_noskip` 擦除 block6，不跳过坏块
    - 从 block6 开始，测试 20 个 block
        - 将 src_buffer 写入到每个 block
        - 将每个 block 的数据读到 dst_buffer
    - 遍历 20 个 block
        - 比较 src_buffer 和 dst_buffer 的数据是否相同
    - ==读写测试通过==
- ·flash_bbt_partition_test`




# Nand

## nand_util.c

### nand_erase_opts

```c
/* Usage */
void gx_nand_chiperase(void)
{
    int ret = 0;
    int quiet = 1;
    struct nand_erase_options opts;
    struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
    memset(&opts, 0, sizeof(opts));
    opts.offset = 0;
    opts.length = nand->size;
    opts.quiet = quiet;
    opts.scrub = 0;

    ret = nand_erase_opts(nand, &opts);
    gxlog_i("%s\n", ret ? "ERROR" : "OK");
}
```

- 赋值 struct gx_erase_info 成员，包括 `len、addr`，以及要擦除的长度(block 对齐)
- `if (opts->scrub) { if (chip->bbt) kfree(chip->bbt) chip->bbt = NULL;}`
- 循环，初值为0，步长为 block_size，限制为 已擦除的长度 < 要擦除的长度
    - `if (!opts->scrub)` 检查当前要擦除的地址是否为坏块，如果是坏块则跳坏块，继续下一次循环;如果 `opts->spread` 为 0 则表示当前的坏块已经被擦除
    - 调用 `nand_erase(mtd, &erase)` 擦除单个 block
    - 如果擦除失败，则调用 `mtd->block_markbad(mtd, erase.addr)` 标记当前地址所在的 block 为坏块;如果 `opts->spread` 为 0 则表示当前的坏块已经被擦除
    - 已擦除的长度递增
- `if (opts->scrub) chip->scan_bbt(mtd)`



### nand_write_skip_bad

```c
int gx_nand_pageprogram(uint32_t addr, uint8_t *data, uint32_t len)
{
	int ret = 0;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	unsigned int page_size = nand->writesize;
	int block_size = nand->erasesize;

	unsigned int page_offset = addr & (page_size - 1);
	if (page_offset) {
		unsigned int tmp_len;
		unsigned char *buf = malloc(page_size);
		while (gx_nand_block_isbad(addr)) {
			addr +=  block_size;
		}
		tmp_len = min(page_size - page_offset, len);
		memset(buf, 0xff, page_size);
		memcpy(buf + page_offset, data, tmp_len);
		ret = nand_write_skip_bad(nand, addr - page_offset, &page_size, buf, 0);
		addr += tmp_len;
		data += tmp_len;
		len -= tmp_len;
		free(buf);
	}
	if (len)
		ret = nand_write_skip_bad(nand, addr, &len, (uint8_t *)data, 0);

	return ret;
}
```

==一次最多写一个 block==

- 变量 left_to_write = *length 保存需要写的长度
- 限制 offset 必须 page_size 对齐
- `ifdef CONFIG_CMD_NAND_YAFFS`  blocks = main_size + oob_size
- 其他模式下，blocks = main_size
- 调用函数 `check_skip_len(nand, offset, *length)` 根据跳坏块的规则，计算要写的 offset 和 length 中是否需要跳坏块
- `while (left_to_write > 0)` 循环
    - 计算当前地址相对于 block 的偏移：block_offset
    - 如果当前地址是坏块，则 offset += block_size - block_offset(相当于到下一个 block)
    - 如果当前要写的地址+要写的长度小于当前这个 block，则write_size = left_to_write
    - 否则 write_size = blocksize - block_offset，相当于先把偏移写掉
    -  `ifdef CONFIG_CMD_NAND_YAFFS`
    - 调用 `nand_write(nand, offset, &write_size, p_buffer)` 把前面的 buffer 写掉
    - 如果写失败，调用 `nand->block_markbad` 标记为坏块
    - `offset += write_size; p_buffer += write_size;`
    - 更新要写的长度：`left_to_write -= write_size`


### nand_read_skip_bad

```c
int gx_nand_readdata(uint32_t addr, uint8_t *data, uint32_t len)
{
	int ret = 0;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	unsigned int page_size = nand->writesize;
	int block_size = nand->erasesize;

	unsigned int page_offset = addr & (page_size - 1);
	if (page_offset) {
		unsigned int tmp_len;
		unsigned char *buf = malloc(page_size);
		while (gx_nand_block_isbad(addr)) {
			addr +=  block_size;
		}
		tmp_len = min(page_size - page_offset, len);
		ret = nand_read_skip_bad(nand, addr - page_offset, &page_size, buf);
		memcpy(data, buf + page_offset, tmp_len);
		addr += tmp_len;
		data += tmp_len;
		len -= tmp_len;
		free(buf);
	}
	if (len)
		ret = nand_read_skip_bad(nand, addr, &len, data);
	return ret;
}
```

==可以一次读多个 block(无 badblock);可以一次读单个 block(或者不满 block);==

- offset 必须 page 对齐
- 计算要读的数据长度：`left_to_read = *length`
- 调用 `check_skip_len(nand, offset, *length)` 根据地址和长度检查其中是否有坏块
- 如果不需要跳：
    - 调用 `nand_read(nand, offset, length, buffer)` 把所有的数据一次读完
- 如果需要跳：
    - 计算 offset 对于 block 的偏移：block_offset
    - 检查当前 offset 对应 block 是否为坏块，如果为坏块，则 `offset += page_size - block_offset`
    - 如果要读的长度不满当前 block
        - 则 `read_length = left_to_read`
    - 否则：`read_length = block_size - block_offset`
    - 调用 `nand_read(nand, offset, &read_length, p_buffer)` 读数据(最大一个 block)
    - 更新 `left_to_read -= read_length; offset += read_length; p_buffer += read_length;`


### nand_write_no_skip_bad

- 写与 `skip bad` 一致，少了跳坏块的部分


### nand_read_no_skip_bad

- 读一次全部读完所有数据，少了跳坏块的部分



## nand_base.c


```c
	/* Fill in remaining MTD driver data */
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->erase = nand_erase;
	mtd->read = nand_read;
	mtd->write = nand_write;
	mtd->read_oob = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->sync = nand_sync;
	mtd->lock = NULL;
	mtd->unlock = NULL;
	mtd->block_isbad = nand_block_isbad;
	mtd->block_markbad = nand_block_markbad;
```



### nand_block_isbad

- 调用 `nand_block_checkbad(mtd, offs, 1, 0)`
    -  如果内存中的坏块表 `chip->bbt` 不存在，则调用 `chip->block_bad(mtd, ofs, getchip)`
    - 否则调用 `nand_isbad_bbt(mtd, ofs, allowbbt)`
        - 计算当前地址所在的 block
        - 调用 `bbt_get_entry()`



### nand_block_markbad

==使用 PLACE_OOB 的方式把 0x00 写入到 to 所在 page 的 OOB 区域第一个字节==

==只写 OOB 区域一个字节，不写 main 区域==


- 调用 `struct gx_mtd_info *nand->block_markbad(nand, addr);`
    - `nand_scan_tail:mtd->block_markbad = nand_block_markbad`
    - 调用 `nand_block_isbad(mtd, ofs)` 检查此偏移地址所在的 block 是否为坏块
    - 调用 `nand_block_markbad_lowlevel(mtd, ofs)` ---------->|
                                                              |
                                                          <-------------------------------------------------------------|
- 擦除坏块所在 block
- 调用 `chip->block_markbad(mtd, ofs)`
    - 使用 `PLACE_OOB` 的方式写 oob 区域 0 地址，长度是 1
    - 如果定义宏 `NAND_BBT_SCANLASTPAGE` 则将写入最后一个 page
    - 否则 ofs = 0
    - 调用 `nand_do_write_oob(mtd, ofs, &ops)`
        - 根据要写的地址计算出 page(to >> chip->page_shift)
        - 向 flash 发送 reset 命令
        - 调用 `nand_check_wp(mtd)` 检查是否被 wp
        - 将变量 `chip->oob_poi` memset 成 0xff，长度是 oobsize
        - 调用 `nand_fill_oob(chip, ops->oobbuf, ops)`，将要写到 oob 区域的 buffer 拷贝到 `chip->oob_poi` 处
            - memcpy(chip->oob_poi + ops->ooboffs, oob, len);
        - 调用 `chip->ecc.write_oob(mtd, chip, page&chip->pagemask)`
            - 调用 `chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page)`
                - 发命令 0x80，列地址是 mtd->writesize(表示 oob 区域开头),page 是 to 所在的page
            - 调用 `chip->write_buf(mtd, buf, length)`
                - 配置控制器，把要写的数据写入到 buffer，开启控制器
            - 调用 `chip->cmdfunc(mtd, NAND_MTD_PAGEPROG, -1, -1)`
                - 发命令 0x10，无列地址，无 page
            - 调用 `chip->waitfunc(mtd, chip)`
                - 调用 `gxnfc_nand_dev_ready(mtd)`
                    - 等待控制器 ready
                - 调用 `chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1)`
                    - 发命令 0x70，无列地址，无 page
                - 调用 `gxnfc_nand_read_byte(mtd)`
                    - 配控制器，读一个字节数据
    - 如果定义了宏 `NAND_BBT_SCAN2NDPAGE` 则在 page0、page1 的 oob 区域分别写上坏块标记
- 如果内存中有 `chip->bbt` 坏块表
    - 调用 `nand_markbad_bbt(mtd, ofs)`
        - 计算 ofs 所在 `block：offs >> this->bbt_erase_shift`
        - 调用 `bbt_mark_entry(this, block, BBT_BLOCK_WORN`
            - 标记 offs 所在的 block 为坏块, 内存中的坏块表更新为 0x01
        - 如果有宏 `NAND_BBT_USE_FLASH`
            - 调用 `nand_updata_bbt(mtd, offs)`
                - 申请一块 buffer
                - `td->version++, md->version++`
                - 调用 `write_bbt(mtd, buf, td, md, chipsel)`
                    - 使用 `PLACE_OOB` 的方式
                    - 调用 `get_bbt_block(this, td, md, chip)`  找出坏块表所在的 block
                    - 根据 block 算出 page：`page = block << (this->bbt_erase_shift - this->page_shift)`
                    - bits = 2
                    - `sft = 2; sft_mask = 0x06; msk[0] = 0x00; msk[1] = 0x01; msk[3] = 0x03;`
                    - 根据 page 计算出 to：`to = page << this->page_shift`
                    - td->options 有宏 `NAND_BBT_NO_OOB`
                        - offs = td->len = 4
                        - offs++
                        - offs += 2 + 1
                        - len = numblocks >> 2  所有 block 占多长
                        - len += offs
                        - len 向上对齐到 page_size
                        - memcpy(buf, td->pattern, td->len)  td->pattern：Bbt0
                    - td->options 没有宏 `NAND_BBT_NO_OOB`
                        - len = numblocks >> 2
                        - len 向上对齐到 page_size
                        - offs = 0
                        - ooboffs = len
                        - memcpy(&buf[ooboffs + td->offs], td->pattern, td->len)
                    - buf[ooboffs + td->veroffs] = td->version[0];
                    - buf[offs-2] = numblocks >> 2
                    - buf[offs-1] = (numblocks >> 2) >> 8 存放总共有多少个 block，用 2 个字节存
                    - 循环 numblocks 次
                        - 获取当前 block 是否为好块、坏块、工厂坏块
                        - 将块的信息放到 buf[offs + i >> 2]
                    - 如果有宏 `NAND_BBT_NO_OOB`
                        - 将 buf 中的 offs+ numblocks >> 2 的长度算 crc
                    - 将 4 个字节的 crc 放到 buf[offs+numblocks>>2]
                    - 擦除 to 所在 block
                    - 如果擦除失败
                        - 标记成坏块
                    - 调用 `scan_write_bbt(mtd, to, len, buf, NULL/&buf[len])`
                        - 使用 PLACE_OOB 方式把 buf 写进去，oob 区域不需要写
                        - 调用 `mtd->write_oob(mtd, offs, &ops)`
                            - 如果 `ops->datbuf` 为空则调用 `nand_do_write_oob(mtd, to, ops)`
                            - 否则调用 `nand_do_write_ops(mtd, to, ops)`
                                - 如果是 NFC V1
                                    - 如果 oob 要写：
                                    - 如果 buf 要写：
                                        - buildin/RAW 模式关掉 ECC，其他模式开启 ECC
                                        - 调用 `chip->write_data(mtd, to, buf, ops->len, &writelen)`
                                            - 算出 row、col 地址
                                            - 把 data 也就是 buf 拷贝到 writebuf
                                            - 调用 `nandflash_programpage(mtd, row, writebuf, page_size)`
                                            - 配置寄存器、ECC 能力、TYPE、地址、启动
                                - 如果是 NFC V0
                                    - 如果 oob 要写：
                                        - 把 oob 数据填充到 `chip->oob_poi[ops->ooboffs]`
                                        - 调用 `chip->write_page(mtd, chip, wbuf, page, cached, (ops->mode == GX_MTD_OPS_RAW)`
                                            - 调用 `chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page)` 把命令 0x80、地址和 page 发出去
                                            - 如果是 raw 模式，调用 `chip->ecc.write_page_raw(mtd, chip, buf)`
                                            - 否则，调用 `chip->ecc.write_page(mtd, chip, buf)`
                                                - V1:
                                                    - 控制器 ECC 配置为 ECC_WRITE
                                                    - 调用 `chip->write_buf` 把 main 数据写进去
                                                    - 调用 `chip->ecc.calculate(mtd, p, &ecc_calc[i])` 算出 ecc
                                                - V0:
                                                    - 关掉控制器 ECC
                                                    - 调用 `chip->write_buf` 把 main 数据写进去
                                                    - 根据 main 数据计算出 ecc(从控制器的寄存器中读出)
                                                    - 将 ecc 放到 `chip->oob_poi[eccpos]`
                                                - 调用 `chip->write_buf(mtd, chip->oob_poi, mtd->oob_size)` 把 oob 数据写入到 oob 区域
                                            - 调用 `chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1)` 发命令 0x10
                                            - 调用 `chip->waitfunc(mtd, chip)` 发命令 0x70 读 flash 的状态寄存器
                    - 如果失败
                        - 标记成坏块
                    - 标记当前页存储为坏块表：td->pages[0] = page

### nand_read

==支持同时写(单独写) main 数据和 oob 数据==

==支持 V1、V0;V1 配好控制器之后，由控制器发指令、地址，V0 通过配控制器之后，手动发指令、地址、拉高拉低 CLE、ALE等==

```c
struct gx_mtd_oob_ops {
	unsigned int mode;
	size_t       len;
	size_t       retlen;
	size_t       ooblen;
	size_t       oobretlen;
	uint32_t     ooboffs;
	uint8_t      *datbuf;
	uint8_t      *oobbuf;
};


static int nand_read(struct gx_mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uint8_t *buf)
{
	struct gx_nand_chip *chip = mtd_to_chip(mtd);
	int ret;

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	chip->ops.len = len;
	chip->ops.datbuf = buf;
	chip->ops.oobbuf = NULL;

	ret = nand_do_read_ops(mtd, from, &chip->ops);
	*retlen = chip->ops.retlen;

	return ret;
}
```

- 调用 `nand_do_read_ops(mtd, from, &chip->ops)` 完成读数据操作
    - 根据 from 地址计算出所在 page
    - 根据 from 地址计算出列地址 col
    - **V1:**
        - buf:
            - 如果是 RAW/buildin 模式就关掉控制器的 ECC，否则配置控制器的 ECC 相关寄存器
            - 调用 `chip->read_data(mtd, from, buf, ops->len, &readlen);` 配置好控制器的 TYPE、START 等等啥的寄存器，配好之后控制器自己发指令、地址，然后读数据
        - oob:
            - 调用 `chip->ecc.read_oob(mtd, chip, page, sndcmd);` 配置好控制器的寄存器，把地址配置到 main 区域的结尾(oob)，将 oob 数据读到 `chip->oob_poi[]`
            - 调用 `nand_transfer_oob(chip, oob, pos, toread)` 将数据从 `chip->oob_poi[]` 拷贝到 `oob`
    - **V0:**
        - oob/buf: 不区分 oob/buf，一次都读过来
            - 数据地址四字节对齐，长度 page_size 对齐
            - V1:
            - V0:
                - (1)如果是 RAW/buildin 模式，调用 `chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page)` 发命令 0x00，page，列地址 0x00，发 0x30
                - (2)否则调用 `chip->ecc.read_oob(mtd, chip, page, sndcmd)` 仍然是发命令 0x00，page，列地址变为 oob 开头，发 0x30，将 oob 数据读到 `chip->oob_poi[]`
                - (1)如果是 RAW 模式，调用 `chip->ecc.read_page_raw(mtd, chip, bufpoi, page)` 先关掉控制器的 ECC，读 main 区域数据，读 oob 区域数据到 `chip->oob_poi[]`
                - (2)否则调用 `chip->ecc.read_page(mtd, chip, bufpoi, page)`
                    - V1:
                    - V0:
                        - 扫描每个 sector(512bytes) 其中的 bit1 位数，如果 bit1 很多代表被擦除过了
                        - 如果四个 sector 都是擦除过，则说明这个 page 为全 0xFF，就不读
                        - 如果不是全 0xFF，则需要读数据
                            - 调用 `chip->cmdfunc(mtd, NAND_CMD_RNDOUT, 0, -1)` 发命令 0x05，列地址是 0x0，行地址没有，发 0xE0
                            - 开启控制器的 ECC
                            - 调用 `chip->read_buf(mtd, p, mtd->writesize)` 读 main 区域数据
                            - 调用 `chip->ecc.hwctl(mtd, NAND_ECC_WAIT_READY)` 等控制器 ready
                - 如果存在 oob，则将 `chip->oob_poi[]` 的数据都拷贝到 oob buffer

### nand_write

```c
static int nand_write(struct gx_mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const uint8_t *buf)
{
	struct gx_nand_chip *chip = mtd_to_chip(mtd);
	int ret;

	/* Do not allow writes past end of device */
	if ((to + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	chip->ops.len = len;
	chip->ops.datbuf = (uint8_t *)buf;
	chip->ops.oobbuf = NULL;

	ret = nand_do_write_ops(mtd, to, &chip->ops);

	*retlen = chip->ops.retlen;

	return ret;
}
```

- 调用 `nand_do_write_ops(mtd, to, &chip->ops)` 完成写操作
- **V1:**
    - oob
        - 计算 to 要写入的地址所在的 page
        - 将 oob buffer 填充到 `chip->oob_poi[]`
        - 如果模式是 `PLACE_OOB` 将 `chip->oob_poi[]` 中用于存放 ecc 位置的数据都赋值为 0xff
        - 调用 `chip->ecc.write_oob(mtd, chip, page & chip->pagemask)` 发命令0x80,列地址是 oob 区域，page，然后把 oob 数据都写掉，发 0x10，等控制器 ready
    - buf
        - 如果是 RAW/buildin 模式，关掉控制器的 ECC
        - 否则配置好控制器的 ECC 相关寄存器
        - 调用 `chip->write_data(mtd, to, buf, ops->len, &writelen)` 配置好控制器的 TYPE、Start_addr、等等寄存器，控制器自己发指令、地址、数据、指令
- **V0:**
    - oob/buf
        - 根据 to 地址计算要写的列地址
        - 根据 to 地址计算要写的页地址
        - 如果 oob buffer 为空，则将 `chip->oob_poi` 都配为 0xff
        - while (1)
        - 如果列地址非0，或者要写的长度小于 page，或者要写的 buffer(wbuf) 不是 4 字节对齐
            - 先将临时 buffer memset 成 0xff，长度是 page_size
            - 将要写的 buffer 拷贝到临时 buffer, wbuf = 临时 buffer
        - 如果 oob buffer 不为空
            - 将 oob buffer 拷贝到 `chip->oob_poi[]`
        - 调用 `chip->write_page(mtd, chip, wbuf, page, cached, (ops->mode == GX_MTD_OPS_RAW)` 发0x80，列地址0x00，行地址 page
            - 如果是 raw 模式：关掉控制器的 ecc，写 main 数据，将 `chip->oob_poi[]` 中的数据写到 oob 区域
            - V1
            - V0:
                - 配置控制器的 ECC 相关寄存器，将 main 数据写掉，根据 main 数据计算出 ecc，将算出来的 ecc 填充到 `chip->oob_poi[]` ，然后把 `chip->oob_poi[]` 都写到 oob 区域
        - 调用 `chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1)` 发送 0x10，等待控制器 ready


### nand_erase

```c
struct gx_erase_info {
	uint32_t addr;
	uint32_t len;
	uint32_t fail_addr;
	u_char state;
	int scrub;
};

int nand_erase(struct gx_mtd_info *mtd, struct gx_erase_info *instr)
{
	return nand_erase_nand(mtd, instr, 0);
}
```

- 地址、长度都要 block 对齐
- 根据要擦除的地址算出 page
- while (len)
    - 检查当前 page 是否是坏块，如果是坏块直接退出
    - 调用 `chip->erase_cmd(mtd, page & chip->pagemask)` 发命令 0x60、行地址、0xD0
    - 调用 `chip->waitfunc(mtd, chip)` 发命令 0x70，读控制器状态是否擦除成功
    - len -= block_size


### nand_read_oob

```c
int gx_nand_readoob(uint32_t addr, uint8_t *data, uint32_t len)
{
	int ret;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	/* out-of-band data */
	struct gx_mtd_oob_ops ops =
	{
		.oobbuf = (u8 *)data,
		.ooblen = len,
		.mode = GX_MTD_OPS_RAW
	};

	ret = nand->read_oob(nand, addr, &ops);

	return ret;
}
```

==接口支持同时读 oob、buf，也支持仅读 oob==

==只读 oob 时用的是 RAW 模式==

- **如果同时读 oob、buf：`nand_do_read_ops(mtd, from, ops)`**
    - 根据 from 地址计算出 page、列地址
    - `oob = ops->oobbuf; buf = ops->darbuf`
    - ==V1:==
        - buf:
            - 如果是 RAW/buildin 模式，关掉控制器的 ECC；否则配置控制器的 ECC 相关寄存器
            - 调用 `chip->read_data(mtd, from, buf, ops->len, &readlen)` 把控制器配好，然后读数据到 buf
        - oob:
            - 调用 `chip->ecc.read_oob(mtd, chip, page, sndcmd)` 配置控制器只有一个 TYPE，读 oob 区域数据到 `chip->oob_poi[]`
            - 调用 `nand_transfer_oob(chip, oob, ops, toread)` 将 `chip->oob_poi[]` 的数据拷贝到 oob
    - ==V0:==
        - buf/oob:
            - ==V1:==
            - ==V0:==
                - 如果模式是 RAW/buildin 发命令 0x00、行地址 page、列地址 0x00、发命令 0x30
                - 否则发命令 0x00、行地址 page、列地址 oob、发命令 0x30
                - 如果模式是 RAW ，关掉控制器 ECC，读 main 数据，读 oob 数据
                - 否则
                    - ==V1：==
                    - ==V0：==
                        - 扫描每个 sector(512bytes) 其中的 bit1 位数，如果 bit1 很多代表被擦除过了
                        - 如果四个 sector 都是擦除过，则说明这个 page 为全 0xFF，就不读
                        - 如果不是全 0xFF，则需要读数据
                            - 调用 `chip->cmdfunc(mtd, NAND_CMD_RNDOUT, 0, -1)` 发命令 0x05，列地址是 0x0，行地址没有，发 0xE0
                            - 开启控制器的 ECC
                            - 调用 `chip->read_buf(mtd, p, mtd->writesize)` 读 main 区域数据
                            - 调用 `chip->ecc.hwctl(mtd, NAND_ECC_WAIT_READY)` 等控制器 ready
                - 如果存在 oob，则将 `chip->oob_poi[]` 的数据都拷贝到 oob buffer
- **如果仅读 oob：`nand_do_read_oob(mtd, from, ops)`**
    - 如果模式是 `AUTO_OOB` 则 len = chip->ecc.layout->oobavail
    - 否则 len = mtd->oobsize
    - 根据 from 地址计算出所在的 page
    - while (1)
        - 调用 `chip->ecc.read_oob(mtd, chip, page, sndcmd)` 
            - V1:配控制器的 TYPE，只有一个 TYPE 就是 oob 区域，然后启动传输，读数据到 `chip->oob_poi[]`
            - V0:发命令 0x00，列地址是 oob 区域，行地址是 page，命令 0x30,配寄存器，读数据到 ecc_code,将数据拷贝到 `chip->oob_poi[]`
        - 将 `chip->oob_poi[]` 的数据拷贝到 buf
        - 如果读完，退出循环


### nand_write_oob

```c
int gx_nand_writeoob(uint32_t addr, uint8_t *data, uint32_t len)
{
	int ret;
	struct gx_mtd_info *nand = dev_to_mtd(&gx_nand_dev);
	struct gx_mtd_oob_ops ops = {
		.oobbuf = (u8 *)data,
		.ooblen = len,
		.mode = GX_MTD_OPS_RAW
	};

	ret = nand->write_oob(nand, addr, &ops);

	return ret;
}
```

==只写 oob 时用的是 RAW 模式==

==只写 data 时用的也是 RAW 模式==

- **如果同时要写 dat、oob：`nand_do_write_ops(mtd, to, ops)`**
    - ==V1:==
        - oob:
            - 根据 to 地址计算出所在的 page
            - 将要写的 oob 数据填充到 `chip->oob_poi[]`
            - 如果模式是 `PLACE_OOB` 则将 `chip->oob_poi[]` 中用于存放 ECC 的位置赋值为 0xff
            - 调用 `chip->ecc.write_oob(mtd, chip, page & chip->pagemask)` 发命令 0x80,列地址是 oob，行地址是 page，写数据，发命令 0x10，发 0x70 查写状态
        - buf:
            - 如果模式是 RAW/buildin，关掉控制器的 ECC 功能，否则配置控制器的 ECC 相关功能
            - 调用 `chip->write_data(mtd, to, buf, ops->len, &writelen)` 配置控制器的 TYPE、Start、等等相关寄存器，启动传输，将数据发送出去(命令、地址之类的都是控制器自己发)
    - ==V0:==
        - oob/buf:
            - 根据 to 地址计算出列地址、page
            - while (1)
                - 如果数据不是 page 对齐，或者要写的长度不是 page 对齐，或者要写的 buffer 不是四字节对齐
                    - 将要写的 buffer 转移到一个全为 0xff 的 临时 buffer，有数据的区域为数据，没有数据的区域为 0xff，并且此 buffer 还是四字节地址对齐
                - 将 oob buffer 填充到 `chip->oob_poi`
                - 调用 `chip->write_page(mtd, chip, wbuf, page, cached, (ops->mode == GX_MTD_OPS_RAW))`
                    - 发命令 0x80，行地址 page，列地址 0x00
                    - 如果是 RAW 模式，关掉控制器 ECC，写 main 区域，写 oob 区域
                    - 否则：配置控制器 ECC 相关寄存器、写 main 区域、根据 main 区域写的数据计算出 ecc(从寄存器读)、填充擦除标记、将 ecc 填充到 `chip->oob_poi` 后写出去
                    - 发命令 0x10，发 0x70 查写状态
                - 递增 page，直到写完为止
- **如果仅写 oob：`nand_do_write_oob(mtd, to, ops)`**









# SPINand

## spinand.c

### gx_spinand_writedata

==用户写不满一个 page 数据时，把后面都补成0xff，补满page==

```c
int gx_spinand_writedata(unsigned int addr, unsigned char *data, unsigned int len)
{
	size_t ret = 0;
	unsigned int spinand_erasesize = g_spinand_chip.info->block_main_size;
	unsigned int spinand_writesize = g_spinand_chip.info->page_main_size;
	unsigned int page_offset = addr & (spinand_writesize - 1);

	if (page_offset) {
		unsigned int tmp_len;
		unsigned char *buf = malloc(spinand_writesize);
		while (spinand_block_isbad(addr & ~(spinand_erasesize -1))) {
			gxlog_w("warning: skip bad block at 0x%x\n", addr & ~(spinand_erasesize -1));
			addr +=  spinand_erasesize;
		}
		tmp_len = min(spinand_writesize - page_offset, len);
		memset(buf, 0xff, spinand_writesize);
		memcpy(buf + page_offset, data, tmp_len);
		ret = spinand_write_skip_bad(addr - page_offset, &spinand_writesize, buf, 0);
		addr += tmp_len;
		data += tmp_len;
		len -= tmp_len;
		free(buf);
	}
	if (len)
		ret = spinand_write_skip_bad(addr, &len, data, 0);
	return ret;
}
```


### spinand_write_skip_bad

- 根据要写的地址和长度，查看这段空间内是否有坏块，是否需要跳坏块
- `while (left_to_write > 0)` 循环按 page 写数据
    - 检查当前地址是否是坏块，如果是坏块，offset += block_size - block_offset
    - write_size = page_size
    - 调用 `spinand_write(offset, page_size, &truncated_write_size, p_buffer)` 写一个 page
    - 更新 `offset、p_buffer`

### spinand_write

- 调用 `spinand_write_ops(&g_spinand_chip, to, &ops)` 写数据

### gx_spinand_block_markbad

==标记坏块：更新内存坏块表 `chip->bbt`，标记坏块的 page0 的 oob[0] oob[1] 写成 0x01==

```c
int gx_spinand_block_markbad(uint32_t addr)
{
	struct spinand_chip *chip = &g_spinand_chip;

    /* 标记坏块 */
	spinand_markbad_bbt(chip, addr);

	return spinand_block_markbad(addr);
}
```

### spinand_markbad_bbt

- 根据偏移地址计算出所在的 block
- 调用 `bbt_mark_entry(chip, block, 0x01)` 标记内存中的坏块表 `chip->bbt[]`，将当前 block 标记为 0x01
- 如果有宏 `NAND_BBT_USE_FLASH`，调用 `nand_update_bbt(chip, offs)` 更新 flash 中的坏块表


### nand_update_bbt

- 如果内存中没有坏块表`(chip->bbt)`则退出
- len = block_size; malloc 一块 len 长度的 buf
- 主表和镜像表的 version++
- 更新主表，调用 `write_bbt(chip, buf, td, md, chipsel)`
- 更新副表，调用 `write_bbt(chip, buf, md, td, chipsel)`


### write_bbt

==用于(Re)write 坏块表==

- td->reserved_block_code 如果不存在就赋值为 0xff
- 计算总共有多少个 block：numblocks
- 调用 `get_bbt_block(chip, td, md, chip_nor)` 获取当前坏块表所在的 block
    - 从 td->pages[] 中获取存储的 page 并转换成 block
- 根据 block 转换成 page
- msk[2] = ~rcode = 0x00
- 用 2 bit 表示一个 block 的好坏
- `sft = 2; sftmsk = 0x06; msk[0] = 0x00; msk[1] = 0x01; msk[3] = 0x03`
- 根据 page 计算对应的地址 to(page * 0x800)
- 有宏 `NAND_BBT_NO_OOB`,`ooboffs = 0; offs = td->len = `

### 标记坏块表

```
 * The table uses 2 bits per block
 * 11b:		block is good
 * 00b:		block is factory marked bad
 * 01b, 10b:	block is marked bad due to wear
```
- 更新 `chip->bbt[]` 对应 block 为 0x01 表示坏块
- 如果使用 flash 保存坏块表，则更新 flash 中存储的坏块表
- 更新坏块所在的 block 第一个 page 的 oob 区域第一个字节为 0x00

### 0x13(Page read data)

- The Page Data Read instruction will transfer the data of the specified memory page into the 2,112-Byte Data Buffer
- 一次将 `main+oob` 的所有数据加载到 flash 的 buffer 中
