#todo


- iodma
	- suspend：关时钟
		- 无尽模式没有处理
	- resume：开时钟
- ir
	- suspend：
		- 把 dev 中保存的占空比和频率赋给 hal_cfg
		- 关时钟
	- resume
		- 开时钟



- 测试的时候还需要开启  CONFIG_ENABLE_PM 宏


后续在 gxaiot 项目上测试