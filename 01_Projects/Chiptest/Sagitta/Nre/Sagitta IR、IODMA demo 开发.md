

# IR
- 固定模式发射
	- 9012
		- ok
	- NEC
		- ok
	- RC5
		- ok
	- RC6
		- ok
- 固定模式接收
	- 5008为RC5协议
		- ok
	- 2246为9012协议
		- ok
	- 2290为NEC协议
		- ok
	- 0020 RC6协议
		- ok
- 灵活模式发射
	- ok
- 灵活模式接收
	- ok
- 固定模式中断发射
	- ok
- 固定模式中断接收
	- ok
- 灵活模式中断发射
	- ok
- 灵活模式中断接收
	- ok
- 载波频率和占空比设置需要测一下
	- ok
- apus 发射，sagitta 接收需要测一下
	- ok

### 驱动 bug
- std_tx 的时候没有关 tx，ud tx 的时候关了
	- std_tx 的时候不能关 tx，因为 `TX_CTRL`寄存器关闭后，RC5、RC6 的翻转位会被复位
- rx close 的接口只关了控制器的 rx，没有管 dev 的状态，这会导致先执行了 std_it，再执行 ud_it 的时候，两个 it 都会开着
- std_rx_it 和 ud_rx_it 的时候都是从 INTEN 寄存器取出状态，直接或的，这样会导致 std 的时候，产生 ud 模式的 irq
	- 但这里很奇怪，在 stop 的时候，会关掉 INTEN 的
	- 换成在配置 INTEN 寄存器读状态时，清除掉另一种模式的中断状态。


# IODMA

## IODMA
- 固定模式，单线输出
- 固定模式，八线输出
- 无尽模式，单线输出
- 无尽模式，八线输出
- 固定模式，WS2812 点亮一盏灯
- 互补PWM输出(无尽模式)

## ws2812

- 第一盏灯：188us
- 第二盏灯：158us
- 第三盏灯：128us
- 第四盏灯：99us
- 第五盏灯：69us
- 第六盏灯：39us
- 第七盏灯：

- 有一些串色
	- 手指用力按住灯珠，效果会好很多
	- 重新补焊后可以了
- basic_light
	- 指定灯
	- 所有灯

- basic
- all
- fix_color
	- 如果指定 hsv，就显示固定的颜色；如果不指定 hsv，就显示预设的颜色
	- 显示完固定的颜色后，熄灭所有灯
- breath
	- 参数包含：hsv，period_ms，cycles
	- 如果没有指定 cycles，默认显示5次后熄灭所有灯
- rainbow
	- 参数包含：hsv，speed
	- 默认5s后熄灭所有灯
- key_press
	- 参数包含：hsv，fade_time
	- 默认所有测试完成后熄灭所有灯
- flow
	- 参数包含：hsv
	- 每轮空闲1s
	- 默认五轮后熄灭所有灯
- auto

### ws2812 测试灯效时需要烧到 flash 测试，如果用 gdb 加载灯效会有问题