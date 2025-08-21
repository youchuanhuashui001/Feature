
#  virgo
- [x] 外设对接
- [x] rom spinor 驱动优化，用双倍速。spinand 驱动整理
	- [x] rom、stage1 spinand 驱动需要修改代码，避免从 0 地址读写数据
		- [x] 这里根本不存在这个问题，理解错了需求
- [x] flash spi 要加 ddr 模式 [completion:: 2025-05-06]
- [x] Virgo Flash SPI SPI_CTRLR0 在普通模式下会起作用，在测试时发现用新的 virgo_nre bit 测试，出现 Receive Over Flow，读不到 Flash ID。原因是读 id 读错了，需要修改 linux 中的 spi 驱动，在普通模式下需要配置 `SPI_CTRLR0` 为 0。 [completion:: 2025-04-27]
- [x] stage1 和 rom 中的 spinand 驱动，需要解决编译报错的问题，使用变量的方式来修改 [completion:: 2025-05-06]
- [x] 小组会议讨论： ✅ 2025-08-05
	- [x] spinand 是否不需要备份？spinand 都自带 ecc，对于多备份是不是没什么必要？如果数据跳变太多了，纠正不了就让客户换 flash 好了。 ✅ 2025-08-05
	- [x] 是的，不备份好了。如果需要备份的话，stage1、stage2 都需要备份，因为启动的时候都存在跳变导致无法启动的问题。 ✅ 2025-08-05
	- [x] stage1 需要跳坏块的功能，而 rom 不需要跳坏块，是因为 stage1 固定放在 block0，是好块，所以不需要跳坏块。而 stage2 放在任何位置，可能出现坏块，因此需要跳坏块。 ✅ 2025-08-05
- [x] 了解 axi 总线对于读请求、写请求，地址是否递增来整成一个 burst 传输的相关内容
	- [x] 已迁移至独立的问题页面 [[AXI 总线学习]]
- [x] i2c gx_i2c_async_tx 中的接口，对于 align_buf 需要按照 dcache_line 对齐，从 ai 移植过来的代码由于 apus cache line 为 32，所以直接写成了 32，需要改成 dcache_line_size
- [x] i2c 缺少 1MHz Fast plus 模式测试
- [x] uart brc 是用来检测波特率的，但是测试 case 是用来检测模块频率的
	- [x] brc 是很早的功能，因为 uart 时钟不稳，brc 用于根据 rxd 线上的波特率来调整自己，适配对方传输
- [x] rtc 代码里面是单次的闹钟，但是实际测试好像是周期性的，需要确认
	- [x] 确实是单次的闹钟，中断里会关掉 rtc
- [x] loader 中的 spi 驱动，需要处理一下 speed_mode_reg，目前只支持 flash_spi 有寄存器可以保存
	- [x] gen_spi 不需要保存 speed_mode
- [x] 需要支持 Uboot 的 Flash 驱动
	- [x] 已完成，用的 uboot 原生 flash 驱动 ✅ 2025-07-24
- [x] MPW 芯片回来了需要红外驱动，目前 4.19 内核里面已经有了，需要调试一下是不是通的
	- [x] 已经是通的了
- [x] 确认下 ai 的代码有没有用 tx_fifo_addr_sync_ok 位
	- [x] fornax 开始有的
- [x] 关注 Virgo_NRE Flash_SPI 的用户寄存器
	- [x] 确认下 tx_fifo_addr_sync_ok 位是否需要
	- [x] 不会频繁开关 spi，所以不需要这个 bit
- [x] Virgo_MPW 关注下 GEN_SPI 做 slave，和上位机通信
	- [x] 已经基本调通了，后面需要再改一版，把代码合并
	- [x] 已迁移至独立的页面 [[gen_spi 支持 gxvlink]]
- [x] Virgo 关注 ecos dwi2c 的代码，路径 `devs/i2c/gx3110/src/`，gxi2c 和 dwi2c 做了相同的兼容接口，上面调用的时候传同一套参数即可。基地址、模块频率等信息都在这里配置。init、tx、rx 都在这里实现的。
	- [x] 事业部用的接口是 `io/i2c` 目录下的，`gx_i2c_tx、gx_i2c_rx` ✅ 2025-08-05
	- [x] 需要增加 i2c 的测试代码 ✅ 2025-08-05
	- [x] 关注下 slave 不响应的时候，tx 接口不会失败，rx 接口会失败的问题
	- [x] 已迁移至独立的页面 [[Virgo ecos i2c 驱动读失败会报错，写失败不会报错]]
- [x] flash_spi 模块在 linux 4.19 reboot 的时候，会打印一遍 flash 的 id，好像是跑到了 gxmisc 里面 ✅ 2025-08-01
	- [x] 原因是 flash 在 remove 的时候会再读一下 flash id，但不知道为啥这么干 ✅ 2025-08-01
		- [x] 只是为了退出四字节地址模式；和雄哥确认后不需要再读 flash id了，可以直接根据 size 来决定是否退出四字节地址模式 ✅ 2025-08-01
	- [x] 还需要 suspend 和 resume 支持 ✅ 2025-08-05



- [x] Virgo Watchdog Linux4.19 修改频率 ✅ 2025-08-02
- [x] `GoXceed 5.0.0 GX_API` 接口将由 `Gx_HAL` 接口替代：
	https://git.nationalchip.com/redmine/projects/goxceed/wiki/GxHAL_%E7%BC%96%E7%A8%8B%E8%A7%84%E8%8C%83
- [x] Virgo_NRE ✅ 2025-08-05
	- SPINand flash 讨论是否需要备份，由于其自带 ecc，如果ecc无法纠错失败后就启动失败
	- 要求 flash 确保 block0 是好块
- [x] I2C eCos 添加测试代码 ✅ 2025-08-05
- [x] Virgo_NRE ROM flash 驱动通过 magic  aa55 这种方式，先双线读，再单线读，就可以同时支持4线flash和8线flash
	- [设备树支持 SPINor、SPINand]
	- [x] Virgo_NRE ROM 目前用的是双线模式读数据，还需要兼容单线读数据；测试时发现 GD 64MBit 的 flash 只支持单线和四线，不支持单线。但 rom 考虑的是只支持双线，所以就会起不来。
	- [x] 已迁移至独立的页面 [[rom spi nor flash 倍速确认]]







- [x] [[Virgo_MPW 芯片测试进度]]
- [x] Virgo_MPW GXAPI I2C 接口参考 i2c-tools 获取 i2c 控制器数量 ✅ 2025-07-24
	[[Virgo_MPW  BBT I2C 测试开发]]
- [x] Virgo_MPW Flash_SPI 模块验证 ✅ 2025-08-05
	- 8线 nor flash
		- 需要用 d4 的板子
		- gxloader 测试
		- linux 测试
	- 多 CS
		- 也用 d4 的板子测吧
	- spinand
		- 等 uart 测试完后再换
- [x] Virgo_MPW 所有外设模块低功耗测试[[低功耗测试]]
	- [x] GPIO 功耗测试时，需要把内部上拉全部配成 0
	- [x] flash_spi linux 驱动需要去掉 remove 时候的读 id ✅ 2025-08-01
	- [x] 需要测试开时钟时的功耗，以及关时钟时的功耗；以及 idle 模式的功耗，至于什么是 idle 需要向 soc 确认
	- 已迁移至独立的页面 [[外设模块低功耗测试]]










# 待整理

- [x] Linux MTD 分区：
	- nor flash 擦除大小为 64KB
	- 分区时大小和地址都要 64KB 对齐，但事业部经常会只分 4KB，或 512B，这种情况下擦除分区会影响相邻分区。
	- linux mtd 要求分区大小是  erase_size，在 64KB 擦除时，只分4KB 的分区就会是 Read only，但要确保 rootfs 分区是 64KB 对齐；
	- 如果linux 打开了 4K 擦除的选项，那么 mtd 分区就可以 以 4K 对齐了
	- 已迁移至独立的页面 [[Linux MTD 分区]]
- [x] DW_I2C tx 时 msg 只需要组建一个，rx 时 msg 需要组建两个
	- Linux I2C 驱动中对于 TX，老的驱动 4.9 使用两个 msg，但是 DW_I2C 驱动会在发完第一个 msg 之后，发第二个 msg 之前发一个 restart 信号。这样的话可能某些外设就会不通。
		- 因此修改了 i2c 原始驱动 https://git.nationalchip.com/gerrit/#/c/43412/
	- Linux4.19 已经在 gx_layer 将 TX 时的 msg 改成了一个，要发的数据
		- 因此不需要修改 i2c 原始驱动 https://git.nationalchip.com/gerrit/#/c/122907/2
	- 查看 linux 中的 i2c device driver，tx 时均使用 1 个 msg，rx 时均使用 2 个 msg
	- 已迁移至独立的页面 [[DW_I2C msg]]





# Goxceed 版本发布
- [x] virgo-5.0.0 新增 PPI Nand：https://git.nationalchip.com/redmine/issues/413923?issue_count=305&issue_position=13&next_issue_id=406477&prev_issue_id=414085
	- [x] 已迁移至独立的页面 [[Goxceed 版本发布]]