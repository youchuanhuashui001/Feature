- 0x90 0x00 手册描述 5 个字节分别是：`0x98 0xda 0x90 0x15 0x76` 
	- pagesize: 2048 + 128
	- 手册描述需要控制器提供 ecc，但是第三个字节的 0x90 代表的就是自带 ecc
		- 第五个字节代表是否自带 ecc
	- 对于自带 ecc 的 nand flash，需要支持命令 0x7a 来获取 ecc 状态，手册中也没有描述此命令

- 不带 ecc，正常测就行了 
- 带 cache read 
- 不符合 onfi 标准

- 读写擦 9000 多次都没问题 
- `flash nandcomtest` 在 ``


- flash nandcomtest
	- `0x910ab04 0x1f0 0x1c 0x1 1`
	- 挂了之后读对应的 oob 地址看数据是什么
```
>>>>>flash_nand_test:addr=0x910ab04, len = 0x1f0, oob_addr=0x1c, oob_len=0x1, mode=2
sflash erase finish.
read addr=0x910ab04, len=0x1f0, oob_addr=0x1c, oob_len=0x1, mode=2
sflash_gx_readl finish.
sflash page program finish.
sflash_gx_readl finish.
cmp finish. src equal dst.
cmp oob finish. src equal dst.
>>>>>flash_nand_test:addr=0x910ab04, len = 0x1f0, oob_addr=0x1c, oob_len=0x1, mode=1
sflash erase finish.
read addr=0x910ab04, len=0x1f0, oob_addr=0x1c, oob_len=0x1, mode=1
sflash_gx_readl finish.
sflash page program finish.
sflash_gx_readl finish.
cmp finish. src equal dst.
cmp oob error. i = 0. read=0x73, orig=0x63




cmp oob error. i = 0. read=0xbf, orig=0xb7


flash nandsingletest 0x8e48fd9 0x4c5 0x11 0x1 1

```

- oob 区域会被 ecc 保护吗？
	- 不会被 ecc 保护，只会保护 main 区域 + oob 区域的 ecc 码 
- 直接读写 oob 一个字节不会出错 
	- 用的是 `nand_do_write_oob` 接口 
- 同时读写 main + oob 才会出错 
	- 用的是 `nand_do_write_ops` 接口 
- 先跑一下 `flash nandsingletest 0x910ab04 0x0 0x1c 0x1   ok`
	- `flash nandsingletest 0x910ab04 0x10 0x1c 0x1 ok`
	- `flash nandsingletest 0x910ab04 0x100 0x1c 0x1 ok`
	- `flash nandsingletest 0x910ab04 0x1f0 0x1c 0x1 ok`

- 用 nfc_v0 的方式写，没问题 
	- 确认下两种方式的差异 
	- flash 擦除后全是 0xff，所以用户在写操作的时候只能 `1-->0`
	- 现在看起来是写进去的是有 `1bit` 成了 `0-->1` 
	- `nfc_v1`
		- oob 区域会被写两次，但是现在看来第一次就只写 oob 就会失败
		- oob: `0x80 行地址 列地址 data 0x10   wait` 写整个 oob 区域 
			- 根据要写的地址找到对应的 page
			- `memset (chip->oob_poi, 0xff, mtd->oobsize)`
			- `oob = nand_fill_oob(chip, oob, ops)`
			- 把 ops->oob_buf 拷贝到 chip->oob_poi
			- `chip->ecc.write_oob(mtd, chip, page & chip->pagemask)` 
				- 0x80 行地址列地址    data   0x10 
				- wait
		- dat：配置控制器，main + ecc，自己就写了 
	- `nfc_v0`
		- 先把 main 写完，再写 oob，oob 只会写一次 
		- 填充 oob 到 `chip->oob_poi`
		- `chip->write_page`
			- `0x80  行地址列地址(0x00)`
			- 配 ecc 为写模式
			- 写 512
			- 根据数据计算 ecc
			- 循环上面 3 步，直到 2k 做完
			- 手动填充 ecc 到 chip->oob_poi
			- 把 chip->oob_poi 写到 flash 的 oob 区域
			- 写 `0x10  wait` 
	