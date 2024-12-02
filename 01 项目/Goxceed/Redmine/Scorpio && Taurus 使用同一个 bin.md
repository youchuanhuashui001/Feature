#doing 

事业部想要使用同一个软件能够同时在 scorpio、taurus 两颗芯片上跑。


## gxloader

目前 Scorpio 的 Rom、Stage1、Stage2 分别是用的什么 SPI？
- Rom：gx_spi
- Stage1：DW_SPI 
- Stage2：DW_SPI
目前 Taurus 的 Rom、Stage1、Stage2 分别是用的什么 SPI？
- Rom：gx_spi
- Stage1：gx_spi
- Stage2：gx_spi

|               | Scorpio          | Taurus     | 备注                                               |
| ------------- | ---------------- | ---------- | ------------------------------------------------ |
| GX_SPI        | 0xa0302000       | 0xa0302000 |                                                  |
| DW_SPI        | 0xa0e00000       | -          |                                                  |
| DW_SPI_CS     | 0xa030a000+0x708 | -          | dw_spi 的 cs 由用户寄存器控制；<br>gx_spi 的 cs 由 SPI_CR 控制 |
| DW_SPI/GX_SPI | 0xa030a000+0x70c | -          |                                                  |

- Scorpio 的 Stage1、Stage2 都修改成使用 GX_SPI 即可
- Stage2 没什么差异，改了宏就可以
	- `.config` 中的 `CONFIG_SPI_TYPE = DW` 修改为 `GX`
	- 配置 `0xa030a000+0x70c = 0x0`，要关注下 `taurus` 中对应的寄存器的作用
- Stage1 确认一下，可以编译 Scorpio 的时候 undef scorpio，define Taurus 这样测试
	- 配置 `0xa030a000+0x70c = 0x0`，要关注下 `taurus` 中对应的寄存器的作用

- taurus 芯片用 scorpio 的驱动：
	- 配置 `0xa030a000+0x70c = 0x0`，要关注下 `taurus` 中对应的寄存器的作用
- scorpio 芯片用 taurus 的驱动：
	- `.config` 中的 `CONFIG_SPI_TYPE = DW` 修改为 `GX`
	- 配置 `0xa030a000+0x70c = 0x0`，要关注下 `taurus` 中对应的寄存器的作用



> [!info]
> taurus、scorpio 共用 stage1、stage2 flash 驱动：
> 
> 1. 如果编的是 scorpio 需要修改 .config 中的 CONFIG_SPI_TYPE = DW 为 GX；编的是 taurus 的话没关系
> 2. scorpio 芯片需要配置 0xa030a000+0x70c = 0x0(用于选择 GXSPI/DWSPI)；taurus 中这个寄存器没被使用



## ecos


- taurus ecos 用的是 gx_spi
- scorpio ecos 用的是 dw_spi，没支持 gx_spi
- 直接用 taurus 的方式注册就可以用了 scorpio 的 gx_spi 了