#doing 
#备注：spinand超时需要兼容性测试得到timeout


# 策略

- ​**​NOR Flash部分​**​
	- ROM与Stage1共用同一份驱动代码，Stage1所需的相关参数将在ROM阶段预先写死；进入Stage1后，可从参数页读取这些值并重新配置。
	- ROM阶段需固定使用单线模式；Stage1需支持单线、双线及四线模式。
		
	
- ​**​NAND Flash部分​**​
	- ROM与Stage1同样共用驱动，Stage1所用参数也将在ROM中写死，后续由Stage1从参数页读取并重新配置。
	- 需为Stage1和ROM的read接口增加一个参数。当前默认从地址0开始计算坏块并定位物理地址，应改为基于传入参数中的地址进行坏块计算。
		

- rom stage1 驱动整理：
	- nor：
		- stage1和rom 共用一份驱动，stage1 中的相关参数，在rom中先写死，到了stage1可以从参数页读出来，然后再配置一遍
		- rom 需要改成固定用单线模式，stage1 需要单双四线模式都支持
	- nand：
		- stage1和rom共用一份驱动，stage1 中的相关参数，在rom中先写死，到了stage1可以从参数页读出来，然后再配置一遍
		- stage1 和 rom read接口需要添加一个参数，目前默认是从0 地址计算坏块，然后找到对应物理地址的，需要改成从参数传进来的地址计算坏块
		- 一份驱动，rom 和 stage1 完全相同，只是参数不同，所以这份驱动就要支持所有的功能
    



- [x] Virgo_NRE ROM flash 驱动通过 magic  aa55 这种方式，先双线读，再单线读，就可以同时支持4线flash和8线flash
	- [设备树支持 SPINor、SPINand]
	- [x] Virgo_NRE ROM 目前用的是双线模式读数据，还需要兼容单线读数据；测试时发现 GD 64MBit 的 flash 只支持单线和四线，不支持单线。但 rom 考虑的是只支持双线，所以就会起不来。
	- rom 不能同时先双线，再单线的方式读数据，高安认为这种尝试的方式是不安全的
		- 对于3字节和四字节，设计的是两种启动方式，而不是尝试的方法
	- 因此到时测试下单线和双线启动时的速度差异，如果不大的话就采用单线启动

# TODO
- spinand flash 的超时需要测试，在驱动中加了一个 printf，兼容性测试时测试两种情况：
	- 有 flash 时 timeout 一般是多少
	- 没有 flash 是  timeout 一般是多少
- 经过如此测试后，定一个 timeout 的值
