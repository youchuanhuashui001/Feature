#waiting 


- [ ] DW_I2C tx 时 msg 只需要组建一个，rx 时 msg 需要组建两个
	- Linux I2C 驱动中对于 TX，老的驱动 4.9 使用两个 msg，但是 DW_I2C 驱动会在发完第一个 msg 之后，发第二个 msg 之前发一个 restart 信号。这样的话可能某些外设就会不通。
		- 因此修改了 i2c 原始驱动 https://git.nationalchip.com/gerrit/#/c/43412/
	- Linux4.19 已经在 gx_layer 将 TX 时的 msg 改成了一个，要发的数据
		- 因此不需要修改 i2c 原始驱动 https://git.nationalchip.com/gerrit/#/c/122907/2
	- 查看 linux 中的 i2c device driver，tx 时均使用 1 个 msg，rx 时均使用 2 个 msg
