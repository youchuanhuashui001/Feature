
# 介绍
主要用于解决两个问题：
- 什么时候调频调压
- 怎么调频调压
cpufreq driver 提供调频调压的基址，cpufreq govanor 提供不同的策略，cpufreq core 对通用的调频逻辑做抽象，为上层提供功能、接口封装，为下层调用抽象封装的硬件控制接口。此外，

还借助频率电压对opp(operation performance points)功能，clk driver、regulator driver 对频率及电压做时钟及电压的调整。




# 框架

## linux regulator

> 参考链接：
> - https://zhuanlan.zhihu.com/p/565532795   regulator
> - https://zhuanlan.zhihu.com/p/585218834   device dvfs
> - http://www.wowotech.net/pm_subsystem/regulator_framework_overview.html
> - https://doc.embedfire.com/linux/rk356x/driver/zh/latest/linux_driver/subsystem_power_management.html


Linux regulator framework的主要目的：提供标准的内核接口，控制系统的voltage/current regulators，并提供相应的开关、大小设置的机制。


2）利用[struct regulation_constraints](https://zhida.zhihu.com/search?content_id=213806427&content_type=Article&match_order=1&q=struct+regulation_constraints&zhida_source=entity) 描述regulator的物理限制，比如：

- 输出电压的最大值和最小值（voltage regulator）；
- 输出电流的最大值和最小值（current regulator）；
- 允许的操作（修改电压值、修改电流限制、enable、disable等）；
- 输入电压是多少（当输入是另一个regulator时）；
- 是否不允许关闭（always_on）；
- 是否启动时就要打开（boot_on）；



regulator driver指的是regulator设备的驱动，主要包含如下结构：

1）使用[struct regulator_desc](https://zhida.zhihu.com/search?content_id=213806427&content_type=Article&match_order=1&q=struct+regulator_desc&zhida_source=entity)，描述regulator的静态信息，包括：名字、supply regulator的名字、中断号、操作函数集（struct regulator_ops）、使用regmap时相应的寄存器即bitmap等。

2）使用[struct regulator_config](https://zhida.zhihu.com/search?content_id=213806427&content_type=Article&match_order=1&q=struct+regulator_config&zhida_source=entity)，描述regulator的动态信息（所谓的动态信息，体现在struct regulator_config变量都是局部变量，因此不会永久保存），包括struct regulator_init_data指针、设备指针、enable gpio等。

3）提供regulator的注册接口（regulator_register/devm_regulator_register），该接口接受描述该regulator的两个变量的指针：struct regulator_desc和struct regulator_config，并分配一个新的数据结构（[struct regulator_dev](https://zhida.zhihu.com/search?content_id=213806427&content_type=Article&match_order=1&q=struct+regulator_dev&zhida_source=entity)，从设备的角度描述regulator），并把静态指针（struct regulator_desc）和动态指针（struct regulator_config）提供的信息保存在其中。

4）regulator driver以struct regulator_dev（代表设备）指针为对象，对regulator进行后续的操作。



**2.3 regulator consumer**

regulator consumer抽象出regulator设备（struct regulator），并提供regulator操作相关的接口。包括：regulator_get/regulator_put/regulator_enable/regulator_disable/ regulator_set_voltage/regulator_get_voltage等。

**2.4 regulator core**

regulator core负责上述regulator driver/consumer/machine逻辑的具体实现，对底层的硬件进行封装，并提供接口给内核中其他的consumer（使用当前regulator设备的驱动）提供操作接口，并以[sysfs](https://zhida.zhihu.com/search?content_id=213806427&content_type=Article&match_order=1&q=sysfs&zhida_source=entity)的形式，向用户空间提供接口。


### 数据结构

![[Pasted image 20250714150922.png]]

- `struct regulator_dev *rdev` 每一路电压或电流输出会抽象成一个 regulator_dev
	- cpu_voltage 是一路，则会有一个 regulator_dev
		- continuous_voltage_range 表示可以在一定范围输出连续电压
		- 其中的 n_voltages 表示可以支持的电压值个数
		- 其中的 struct regultor_desc 对应的 struct regulator_ops 包含以下函数回调：
			- set_voltage
			- get_voltage
			- enable
			- disable
			- is_enable
	- gpu_voltage 是一路，则会有一个 regulator_dev
	- struct regulation_constraints 是安全约束结构体，包含一些具体的信息
		- min_uV：最小电压
		- max_uV：最大电压
		- uV_offset：consumer 看到的电压和实际电压之间的偏差值
		- ramp_delay：rdev 电压变化后稳定时间
		- enable_time：rdev 开启时间
		- always_on：rdev 是否一直打开



### 流程

#### 初始化
![[Pasted image 20250714151753.png]]





#### 调压(逐步调压)
![[Pasted image 20250714151800.png]]



## Consumer
### Consumer Regulator Access
- 访问 regulator
	- regulator = regulator_get(dev, "Vcc")
- 释放 regulator
	- regulator_put(regulator)


- consumer 可以由多个 regulator 供电
	- digital = regulator_get(dev, "Vcc")
	- analog = regulator_get(dev, "Avdd")
	- 这些 regulator_get regulator_put 通常会在设备驱动 probe、remove 中调用


### Regulator output enable & disable
- regulator_enable(regulator)
- regulator_is_neable(regulator)
- regulator_disable(regulator)
	- 如果电源与其他用电设备共享，则可能无法禁用电源；
	- 只有当启用参考计数为0时，regulator 才会被禁用
- regulator_force_disable(regulator)
	- 强制禁用 regulator
	- 立即强制关闭 regulator 输出。

### Regulator Voltage Control & Status
- regulator_set_voltage(regulator, min_uV, max_uV);
	- 可接受的最小电压和最大电压，单位是微伏
- regulator_get_voltage(regulator)
	- 获取配置的电压输出
	- 无论 regulator 启用还是禁用，都会返回配置的输出电压。因此需要与 is_enable 结合使用


### Regulator Current limit
- regulator_set_current_limit(regulator, min_uA, max_uA)
	- 可接受的最小电流和最大电流，单位是微安
- regulator_get_current_limit(regulator)
	- 获取电流限制
	- 无论 Regulator 启用还是禁用，都会返回电流限制


### Regulator Operating Mode Contorl & Status
用于改变 Regulator 的工作模式进一步节省系统功耗。例如空闲模式、正常模式、高性能模式。
- `regulator_set_load(struct regulator *regulator, int load_uA)`
	- 核心重新计算 Regulator 上的总负载并改变操作模式以最佳匹配当前操作负载
	- 间接操作模式控制
- `regulator_set_mode(struct regulator *regulator, unsigned int mode);`
- `regulator_get_mode(struct regulator *regulator)`
	- 直接配置工作模式和获取工作模式


### Regulator Events
Regulator 可以通知Consumer 外部事件。在 Regulator 压力或者故障情况下，Consumer 可能会收到事件。
- `regulator_register_notifier(struct regulator *regulator, struct notifier_block *nb)`
	- Consumer 可以通过调用注册通知来表达对 Regulator event 表示兴趣
- `regulator_unregister_nofier(struct regulator *regulator, struct notifier_block *nb)`
	- 取消订阅



### Regulator Direct Register Access
7. 调节器直接寄存器访问 =====================================  
  
Some kinds of power management hardware or firmware are designed such that
they need to do low-level hardware access to regulators, with no involvement
from the kernel. Examples of such devices are:  
某些类型的电源管理硬件或固件被设计成需要对调节器进行低级硬件访问，而无需内核的参与。此类设备的示例包括：  
  
- clocksource with a voltage-controlled oscillator and control logic to change
  the supply voltage over I2C to achieve a desired output clock rate
- thermal management firmware that can issue an arbitrary I2C transaction to
  perform system poweroff during overtemperature conditions  
- 带有压控振荡器和控制逻辑的时钟源，用于通过 I2C 改变电源电压，从而实现所需的输出时钟速率 
- 热管理固件，可以发出任意 I2C 事务以在过热条件下执行系统关机  
  
To set up such a device/firmware, various parameters like I2C address of the
regulator, addresses of various regulator registers etc. need to be configured
to it. The regulator framework provides the following helpers for querying
these details.  
要设置这样的设备/固件，需要配置各种参数，例如调节器的 I2C 地址、各种调节器寄存器的地址等。调节器框架提供了以下帮助程序来查询这些详细信息。  
  
Bus-specific details, like I2C addresses or transfer rates are handled by the
regmap framework. To get the regulator's regmap (if supported), use::  
特定于总线的详细信息（例如 I2C 地址或传输速率）由 regmap 框架处理。如需获取调节器的 regmap（如果支持），请使用：  
  
`struct regmap *regulator_get_regmap(struct regulator *regulator);  `

  
To obtain the hardware register offset and bitmask for the regulator's voltage
selector register, use::  
要获取调节器电压选择器寄存器的硬件寄存器偏移量和位掩码，请使用：  
```

int regulator_get_hardware_vsel_register(struct regulator *regulator,
						 unsigned *vsel_reg,
						 unsigned *vsel_mask);  
int regulater_get_hardware_vsel_register(结构调节器*调节器，无符号*vsel_reg，无符号*vsel_mask)；  
```

To convert a regulator framework voltage selector code (used by
regulator_list_voltage) to a hardware-specific voltage selector that can be
directly written to the voltage selector register, use::  
要将调节器框架电压选择器代码（由 regulate_list_voltaic 使用）转换为可直接写入电压选择器寄存器的硬件特定电压选择器，请使用：  
```

int regulator_list_hardware_vsel(struct regulator *regulator,
					 unsigned selector);  
int regulater_list_hardware_vsel(struct regulater *regulator，无符号选择器)；  

```
To access the hardware for enabling/disabling the regulator, consumers must
use regulator_get_exclusive(), as it can't work if there's more than one
consumer. To enable/disable regulator use::  
要访问硬件以启用/禁用调节器，消费者必须使用 regulator_get_exclusive()，因为如果有多个消费者，该函数将无法工作。要启用/禁用调节器，请使用：  
```

int regulator_hardware_enable(struct regulator *regulator, bool enable);
  
int regulater_hardware_enable(struct regulater *regulator, bool enable);
``` 



## Regulator Machine Driver Interface
Regulator machine 驱动程序接口用于 board/machine 特定的初始化代码来配置 regulator subsystem。
```
Regulator-1     --->  Consumer A @1.8 - 2.0v
                --->  Consumer B @3.3v
```
为了控制电源，Consumer A 和 B 的驱动器必须映射到正确的 Regulator。此映射可以在 machine init 代码中通过为每个 Regulator 创建一个 struct regulator_consumer_supply 来实现：
```c
struct regulator_consumer_supply {
	const char *dev_name; // consumer dev_name()
	const char *supply;   // consumer supply - eg "vcc"
}

static struct regulator_consumer_supply regulator1_consumers[] = {
	REGULATOR_SUPPLY("Vcc", "consumer B"),
  };

  static struct regulator_consumer_supply regulator2_consumers[] = {
	REGULATOR_SUPPLY("Vcc", "consumer A"),
  };
```
这将Regilator 1 映射到Consumer B 的“Vcc”电源，并将Regulator 2 映射到Consumer A 的“Vcc”电源。


可以通过为每个调节器电源域定义一个 struct regulater_init_data 来注册约束。该结构还将用电设备映射到其供电调节器：
```c
  static struct regulator_init_data regulator1_data = {
	.constraints = {
		.name = "Regulator-1",
		.min_uV = 3300000,
		.max_uV = 3300000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(regulator1_consumers),
	.consumer_supplies = regulator1_consumers,
  };
```


最后，调节器设备必须以通常的方式进行注册：  
```c
  static struct platform_device regulator_devices[] = {
	{
		.name = "regulator",
		.id = DCDC_1,
		.dev = {
			.platform_data = &regulator1_data,
		},
	},
	{
		.name = "regulator",
		.id = DCDC_2,
		.dev = {
			.platform_data = &regulator2_data,
		},
	},
  };
  /* register regulator 1 device */
  platform_device_register(&regulator_devices[0]);
  ```



# 驱动
**driver**

driver模块的功能，是从regulator driver的角度，抽象regulator设备。

1）使用struct regulator_desc描述regulator的静态信息，包括：名字、supply regulator的名字、中断号、操作函数集（struct regulator_ops）、使用regmap时相应的寄存器即bitmap等等。

2）使用struct regulator_config，描述regulator的动态信息（所谓的动态信息，体现在struct regulator_config变量都是局部变量，因此不会永久保存），包括struct regulator_init_data指针、设备指针、enable gpio等等。

3）提供regulator的注册接口（regulator_register/devm_regulator_register），该接口接受描述该regulator的两个变量的指针：struct regulator_desc和struct regulator_config，并分配一个新的数据结构（struct regulator_dev，从设备的角度描述regulator），并把静态指针（struct regulator_desc）和动态指针（struct regulator_config）提供的信息保存在其中。

4）最后，regulator driver将以为struct regulator_dev指针为对象，对regulator进行后续的操作。



**consumer模块向用户空间consumer提供的接口**

用户空间程序可以通过sysfs接口，使用regulator，就像内核空间consumer一样。这些接口由“drivers/regulator/userspace-consumer.c”实现，主要包括：

> sysfs目录位置：/sys/devices/platform/reg-userspace-consumer。
> 
> name，读取可以获取该regulator的名字。
> 
> state，读取，可以获取该regulator的状态（enabled/disabled）；写入可以改变regulator的状态（enabled或者1使能，disabled或者0禁止）。


**core模块向用户空间提供的sysfs接口**

regulator设备在内核中是以regulator class的形式存在的，regulator core通过class->dev_groups的方式，提供了一些默认的attribute，包括：

> name，读取可以获取该regulator的名字；
> 
> num_users，读取可获取regulator的使用者数目；
> 
> type，读取可以获取该regulator的类型（voltage或者current）。

另外，如果regulator driver需要提供更多的attribute（如状态、最大/最小电压等等），可以调用add_regulator_attributes接口，主动添加。



系统中注册的regulator和consumer之间关系或者regulator之间的关系，可以查看下/sys/kernel/debug/regulator/regulator_summary文件



## 参考驱动
### pwm-regulator.c
```d
    pwm_reg: regulator {
        compatible = "pwm-regulator";
        pwms = <&pwm1 0 50000>; /* PWM句柄 */
        regulator-name = "vdd_core";
        regulator-min-microvolt = <1200000>;
        regulator-max-microvolt = <3300000>;

        /* 定义 (电压_uV, 占空比) 对 */
        voltage-table = <1200000 90>,  /* 1.2V -> 90% duty-cycle */
                        <1800000 60>,  /* 1.8V -> 60% duty-cycle */
                        <3300000 10>;  /* 3.3V -> 10% duty-cycle */
    };
```
- probe
	- malloc struct pwm_regulator_data
	- 将 pwm_regulator_desc 拷贝给 &drvdata->desc，相当于是一个默认值
	- 设备树
		- `voltage-table`：先获取这个属性有多少个元素，然后获取每个成员，保存到duty_cycle_table
			- duty_cycle_table 是 malloc 出来的
			- 将 duty_cycle_table 放到 drvdata->duty_cycle_table
			- 将 pwm_regulator_voltage_table_ops 拷贝到 drvdata->ops
			- 将  drvdata->desc.ops = &drvdata->ops
			- drvdata->desc.n_voltages = 元素的个数，每个代表一个电压
			- drvdata 是 struct pwm_regulator_data，自己设计的，其中包含各种需要的属性
	- of_get_regulator_init_data
		- 从设备数获取标准的 regulator 初始化数据，如电压约束、boot_on 等
	- devm_pwm_get
		- <font color="#ff0000">获取可用的 pwm</font>
			- `devm_pwm_get(&pdev->dev, NULL)`
			- 当第二个参数为 NULL，会在该设备的设备树节点中查找一个名为 pwms 的属性，并使用该属性中指定的第一个 pwm 资源
		- 如何指定 pwm？
			- 假设有两个 pwm 控制器，每个控制器有8个通道，现在需要指定pwm1的channel0 作为 regulator
			- 查看上面的设备树，`pwms = <&pwm1 0 50000>`就是使用 pwm1 的 0 通道，pwm 周期是 50000
	- gpio 拉高拉低，这里用不到 gpio
	- pwm 矫正
	- devm_regulator_register
		- 将填充好的 regulator_desc和 regulator_config 注册到 regulator 框架。
		- 之后，这个 pwm regulator 就可以被其它 consumer 驱动使用了

- set_voltage
	- ops->set_voltage_sel
		- pwm_init_state
		- pwm_set_relative_duty_cycle
		- pwm_apply_state
	- ops->get_voltage_sel
		- return drvdata->state

### Documentation/devicetree/bindings/regulator/pwm-regulator.txt

```d
Continuous Voltage With Enable GPIO Example:
	pwm_regulator {
		compatible = "pwm-regulator";
		pwms = <&pwm1 0 8448 0>;
		enable-gpios = <&gpio0 23 GPIO_ACTIVE_HIGH>;
		regulator-min-microvolt = <1016000>;
		regulator-max-microvolt = <1114000>;
		regulator-name = "vdd_logic";
		/* unit == per-mille */
		pwm-dutycycle-unit = <1000>;
		/*
		 * Inverted PWM logic, and the duty cycle range is limited
		 * to 30%-70%.
		 */
		pwm-dutycycle-range = <700 300>; /* */
	};

Voltage Table Example:
	pwm_regulator {
		compatible = "pwm-regulator";
		pwms = <&pwm1 0 8448 0>;
		regulator-min-microvolt = <1016000>;
		regulator-max-microvolt = <1114000>;
		regulator-name = "vdd_logic";

			      /* Voltage Duty-Cycle */
		voltage-table = <1114000 0>,
				<1095000 10>,
				<1076000 20>,
				<1056000 30>,
				<1036000 40>,
				<1016000 50>;
	};
```




## GPIO
> 参考： https://docs.kernel.org/driver-api/gpio/driver.html

- 目前使用 linux4.9 的老驱动，可以拉高拉低 gpio，后面使用 gpiolib
- 复用成 pwm 的功能，就放在 dts 里面，probe 的时候就复用掉


### Provider
#### rk9.dtsi
```d
 		gpio0: gpio0@ff210000 {
 			compatible = "rockchip,gpio-bank";
 			reg = <0x0 0xff210000 0x0 0x100>;
 			interrupts = <GIC_SPI 51 IRQ_TYPE_LEVEL_HIGH>;
 			clocks = <&cru PCLK_GPIO0>;

			// 文档要求：每个 GPIO 控制器节点必须包含一个空的 gpio-controller 属性
 			gpio-controller;
 			#gpio-cells = <2>;
 
 			interrupt-controller;
 			#interrupt-cells = <2>;
 		};
```
- 定义 gpio 控制器，看起来并没有引用的地方
```d
bank：指的是一个 ip 模块；通常，每个 bank 在设备树中都作为一个单独的 gpio-controller 节点暴露出来
ngpios：可选。指明了可用 GPIO 插槽中正在被使用的数量。
gpio-line-names：可选。这是一个字符串数组，定义了 GPIO 控制器输出的 GPIO 线的名称。

```

### virgo_mpw.dtsi
```d
    gx_gpio: gx_gpio@fa400000 {
        compatible = "nationalchip,gx-gpio";
        #address-cells = <1>;
        #size-cells = <1>;
        reg-num = <4>;
        reg = <0x0 0xfa400000 0x0 0x1000>,
              <0x0 0xfa401000 0x0 0x1000>,
              <0x0 0xfa402000 0x0 0x1000>,
              <0x0 0xfa403000 0x0 0x1000>,
              <0x0 0xf0404000 0x0 0x1000>;
        clocks = <&apb2_0>;
        interrupt-parent = <&gic>;
        interrupts = <GIC_SPI 50 IRQ_TYPE_LEVEL_HIGH>,
                     <GIC_SPI 51 IRQ_TYPE_LEVEL_HIGH>,
                     <GIC_SPI 52 IRQ_TYPE_LEVEL_HIGH>,
                     <GIC_SPI 53 IRQ_TYPE_LEVEL_HIGH>;
        interrupt-names = "gx_gpio.irq0",
                          "gx_gpio.irq1",
                          "gx_gpio.irq2",
                          "gx_gpio.irq3";
        output_read_support = <1>;
        gpio_clock = <198000000>;
        status = "okay";
    };


    gpio0: gpio0@fa400000 {
        compatible = "nationalchip,gpio-bank";
        reg = <0x0 0xfa400000 0x0 0x1000>,
        gpio-sram-base = <0xf0404000>;
        //TODO:
//        interrupt-parent = <&gic>;
        interrupts = <GIC_SPI 50 IRQ_TYPE_LEVEL_HIGH>,
        interrupt-names = "gx_gpio.irq0";
        clocks = <&apb2_0>;
        output_read_support = <1>;

        gpio-controller;
        #gpio-cells = <2>;

        interrupt-controller;
        interrput-cells = <2>;
    };
```


#### rk-gpio.c

```c
/**
 * @dev: device of the gpio bank
 * @reg_base: register base of the gpio bank
 * @reg_pull: optional separate register for additional pull settings
 * @clk: clock of the gpio bank
 * @db_clk: clock of the gpio debounce
 * @irq: interrupt of the gpio bank
 * @saved_masks: Saved content of GPIO_INTEN at suspend time.
 * @pin_base: first pin number
 * @nr_pins: number of pins in this bank
 * @name: name of the bank
 * @bank_num: number of the bank, to account for holes
 * @iomux: array describing the 4 iomux sources of the bank
 * @drv: array describing the 4 drive strength sources of the bank
 * @pull_type: array describing the 4 pull type sources of the bank
 * @of_node: dt node of this bank
 * @drvdata: common pinctrl basedata
 * @domain: irqdomain of the gpio bank
 * @gpio_chip: gpiolib chip
 * @grange: gpio range
 * @slock: spinlock for the gpio bank
 * @route_mask: bits describing the routing pins of per bank
 */
struct rockchip_pin_bank {
	struct device *dev;

	void __iomem			*reg_base;
	struct regmap			*regmap_pull;
	struct clk			*clk;
	struct clk			*db_clk;
	int				irq;
	u32				saved_masks;
	u32				pin_base;
	u8				nr_pins;
	char				*name;
	u8				bank_num;
	struct rockchip_iomux		iomux[4];
	struct rockchip_drv		drv[4];
	enum rockchip_pin_pull_type	pull_type[4];
	struct device_node		*of_node;
	struct rockchip_pinctrl		*drvdata;
	struct irq_domain		*domain;
	struct gpio_chip		gpio_chip;
	struct pinctrl_gpio_range	grange;
	raw_spinlock_t			slock;
	const struct rockchip_gpio_regs	*gpio_regs;
	u32				gpio_type;
	u32				toggle_edge_mode;
	u32				recalced_mask;
	u32				route_mask;
};

static const struct gpio_chip rockchip_gpiolib_chip = {
	.request = gpiochip_generic_request,
	.free = gpiochip_generic_free,
	.set = rockchip_gpio_set,
	.get = rockchip_gpio_get,
	.get_direction	= rockchip_gpio_get_direction,
	.direction_input = rockchip_gpio_direction_input,
	.direction_output = rockchip_gpio_direction_output,
	.set_config = rockchip_gpio_set_config,
	.to_irq = rockchip_gpio_to_irq,
	.owner = THIS_MODULE,
};

static int rockchip_gpio_probe(struct platform_device *pdev)
{

	bank->bank_num =  bank_id;
	bank->dev = dev;

	bank->reg_base = ioremap_resource(pdev, 0);

	rockchip_gpio_get_ver(bank);

	raw_spin_lock_init(&bank->slock);

	bank->clk = devm_clk_get(dev, "bus");
	clk_prepare_enable(bank->clk);

	ret = rockchip_gpio_parse_irqs(pede, bank);
	// not use
	rockchip_gpio_init_cpuhp();

	mutex_lock(&bank->deferred_lock);

	// 关键的地方
	ret = rockchip_gpiolib_register(bank);

	// 关键的地方
	ret = gpiochip_add_pin_range(gc, dev_name(pctldev->dev), 0,
				     gc->base, gc->ngpio);

	// 根据设备树中的 gpio 参数，配置gpio为输出或输入

	mutex_unlock(&bank->deferred_lock);

	platform_set_drvdata(pdev, bank);

	return 0;
}

static int rockchip_interrupts_register(struct rockchip_pin_bank *bank)
{

	ret = irq_alloc_domain_generic_chips(bank->domain, 32, 1,
					     "rockchip_gpio_irq",
					     handle_level_irq,
					     clr, 0, 0);

	irq_set_chained_handler_and_data(bank->irq[i],
					 rockchip_irq_demux,
					 bank);
}

static int rockchip_gpiolib_register(struct rockchip_pin_bank *bank)
{
	struct gpio_chip *gc;

	bank->gpio_chip = rockchip_gpiolib_chip;

	// init struct gpio_chip;
	gc = &bank->gpio_chip;
	gc->base = bank->pin_base;
	gc->ngpio = bank->nr_pins;
	gc->label = bank->name;

	ret = gpiochip_add_data(gc, bank);


	// 注册中断相关
	ret = rockchip_interrupts_register(bank);

	return 0;
}


static struct platform_driver rockchip_gpio_driver = {
	.probe		= rockchip_gpio_probe,
	.remove		= rockchip_gpio_remove,
	.driver		= {
		.name	= "rockchip-gpio",
		.of_match_table = rockchip_gpio_match,
	},
};
```

----------------

probe 流程如下：
- 定义 `struct gpio_chip`
- 配置 gpio_chip 的一些参数和回调函数
- 使能 clk
- 通过 gpiochip_add_data 就将 struct gpio_chip 注册到了 gpio subsystem
- 如果需要支持 irq，通过 irq_alloc_domain_generic_chips 注册到 gpio subsystem
- 通过 gpiochip_add_pin_range 配置 gpio 的基地址和个数


remove 流程如下：
- 释放中断
- 关掉 clk
- `gpiochip_remove(&bank->gpio_chip);`


### consumer-kernelspace

```d
// 在您的设备节点中
my_device@cafebabe {
    compatible = "my-vendor,my-device";
    ...
    // "reset-gpios" 是您自定义的属性名
    // <&my_gpio_controller 5 GPIO_ACTIVE_LOW>
    // 引用 my_gpio_controller 的第 5 个引脚，低电平有效
    reset-gpios = <&my_gpio_controller 5 1>;
};


    gpiod_example {
        compatible = "gpiod-example,test";

        // 单个输出, 使用 gpio1 的第 14 脚
        output-gpios = <&gpio1 14 GPIO_ACTIVE_HIGH>;


        // 中断输入, 使用 gpio1 的第 16 脚
        interrupt-gpios = <&gpio1 15 GPIO_ACTIVE_HIGH>;
    };
```

对于 api 使用，存在两个版本：
- gpiod API
	- 其实内部也是调用的基于整数的 GPIO API，建议使用这个
- 基于整数的 GPIO API

```c
struct gpio_desc *gpiod_get(struct device *dev, const char *con_id, enum gpiod_flags flags);

struct gpio_desc *gpiod_get_index(struct device *dev,
                                  const char *con_id, unsigned int idx,
                                  enum gpiod_flags flags)

struct gpio_desc *gpiod_get_optional(struct device *dev,
                                     const char *con_id,
                                     enum gpiod_flags flags)

struct gpio_desc *gpiod_get_index_optional(struct device *dev,
                                           const char *con_id,
                                           unsigned int index,
                                           enum gpiod_flags flags)


struct gpio_desc *reset_gpio;

reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);

ret = gpiod_direction_output(reset_gpio, 0);

ret = gpiod_set_value(reset_gpio, 1);


ret = gpiod_direction_input(reset_gpio);
status = gpiod_get_value;



void gpiod_put(struct gpio_desc *desc)





int gpiod_direction_input(struct gpio_desc *desc)
int gpiod_direction_output(struct gpio_desc *desc, int value)
int gpiod_get_direction(const struct gpio_desc *desc)

int gpiod_get_value(const struct gpio_desc *desc);
void gpiod_set_value(struct gpio_desc *desc, int value);

int gpiod_get_value_cansleep(const struct gpio_desc *desc)
void gpiod_set_value_cansleep(struct gpio_desc *desc, int value)

```


### userspace

存在两种方式：字符设备接口以及 sysfs 接口(/sys/class/gpio)，推荐使用字符设备接口

- 字符设备接口`/dev/gpiochipX`
- libgpiod 库，使用 `/dev/gpiochipX` 设备的开源程序
	- 安装 libgpiod、gpiod-tools



- sysfs
```shell
$ echo 46 >  /sys/class/gpio/export
[  235.550331] [gx_gpio_request] bank:1, offset:e
[  235.557066] [gx_gpio_get_direction] bank:1, offset:e, data:1
$ ls
active_low  direction   power       uevent
device      edge        subsystem   value
$ cd ../
$ ls
export      gpio46      gpiochip32  gpiochip96
gpio14      gpiochip0   gpiochip64  unexport
$ cd gpio46/
$ ls
active_low  direction   power       uevent
device      edge        subsystem   value
```









## PWM
![[Pasted image 20250715165835.png]]

```c

struct pwm_device 表示 pwm channel，每个channle就是一个 pwm_device
struct pwm_chip 表示 pwm controller，每个controller就是一个 pwm_chip
	包含 struct pwm_device、struct pwm_ops
	以及 npwm、设备树相关的 pwm
struct pwm_ops 表示各种操作函数，包括 request、free、config、enable、disable...



------

// user api
pwm_request
pwm_free
pwm_apply_state
pwm_adjust_config


// provider apis
pwm_capture
pwm_set_chip_data
pwmchip_add
pwmchip_remove
pwm_get
of_pwm_get
devm_pwm_get
...




// pwm-nationalchip.c
// probe
struct pwm_chip *chip = malloc;
chip->ops = 
chip->xxx = 

if (pwmchip_add(chip))
	return -1;



// pwm-regulator.c
// probe
drvdata->pwm = devm_pwm_get(&ped->dev, NULL);

pwm_adjust_config(drvdata->pwm);
	pwm->chip->ops->apply(pwm->chip, pwm, state);


// get_voltage
return drvdata->state;

// set_voltage
pwm_init_state(drvdata->pwm, &pstate);
pwm_set_relative_duty_cycle(&pstate, dutycycle, duty_unit);
pwm_apply_state(drvdata->pwm, &pstate);
	pwm->chip->ops->apply(pwm->chip, pwm, state);

// is_enable
pwm_is_enabled(drvdata->pwm);

// enable
pwm_enable(drvdata->pwm);
	pwm_get_state(pwm, &state);
	if (state.enabled)
		reurn 0;
	state.enabled = true;
	return pwm_apply_state(pwm, &state);

// disable
pwm_disable(drvdata->pwm);
	pwm_get_state(pwm, &state);
	if (!state.enabled)
		return;
	state.enabled = false;
	pwm_apply_state(pwm, &state);




// pwm-nationalchip.c
.get_state = nationalchip_pwm_get_state,
.apply = nationalchip_pwm_apply,
.owner = THIS_MODULE,


```

- pwm-cells
	- 决定在设备树中引用该 pwm 控制器时，每个 pwm 通道需要提供的参数个数
	- 若为2，则需要提供通道号和周期值
	- 若为3，则需要额外参数，例如极性或标志位
	- `pws = <&pwm0 0 25000 0>`：表示pwm0、通道0、周期25000ns、极性0



- sysfs 测试
```shell
$ echo 3 > export
$ cd pwm3/
$ ls
capture     enable      polarity    uevent
duty_cycle  period      power
$ cat enable
0
$ cat capture
cat: read error: Function not implemented
$ cat duty_cycle
0
$ cat period
0
$ echo 10000 > period
$ echo 5000 > duty_cycle
$ echo 1 > enable

```

## Regulator
> - http://www.wowotech.net/pm_subsystem/regulator_framework_overview.html
> - http://www.wowotech.net/pm_subsystem/regulator_driver.html


需求：
- cpu-regulator
- gpu-regulator
- pcie-12v-fix-regulator


- 初始时 pwm 输出为 min_voltage


- sysfs
```shell
$ cd /sys/device/platform/
$ cd test_vol_user_ctl/
$ ls
driver           name             state            voltage
driver_override  of_node          subsystem
modalias         power            uevent
$ cat name
test_vol_userspace
$ cat voltage
1020000
$ cat state
enabled


```


# 添加 ADC 回采功能
需要咨询老司机的问题：
- 要设定的电压值还是按照档位来吗？
- 当温度升高导致电压产生漂移，这时候电压的设定是由谁来处理？
	- pwm-regulator 驱动来处理，那么内部就要一个线程专门来处理电压设置
	- 线程实时读取当前的电压值，以及要配置的值，然后实时调整电压


# 问题

## 【已解决】gpio ioctl 无法进入函数
- 32位应用程序，64位内核需要使用 compat-ioctl



## 【已解决】PWM无法通过 sysfs 配置 period
```
$ echo 1 > /sys/class/pwm/pwmchip0/pwm2/enable
[ 1450.634409] gx_pwm_apply: pwm 2, enabled: 1
[ 1450.641821] gx_pwm_config: pwm 2, period: 10000000000, duty_cycle: 4000000000
[ 1450.649009] gx_pwm_enable: pwm 2, enable: 1
[ 1450.653263] gx_pwm_get_state: pwm 2
[ 1450.656819] period: 327483376614, duty_cycle: 130993350646, enabled: 0
$ echo 10000 > /sys/class/pwm/pwmchip0/pwm2/period
sh: write error: Invalid argument
$ echo 100000 > /sys/class/pwm/pwmchip0/pwm2/period
sh: write error: Invalid argument
$ echo 100000 ^C/sys/class/pwm/pwmchip0/pwm2/period

$ echo 50000 > /sys/class/pwm/pwmchip0/pwm2/period
sh: write error: Invalid argument
```
- 发现是不能去关闭 前三路 timer
- get_state 函数有问题，不需要更新 `pwm->state   period, duty_cycle`


```
mount -t nfs -o nolock 192.168.108.149:/opt/nfs /tmp
```



## 【已解决】PWM-regulator 无法通过 sysfs 配置 state
```
$ echo enabled > /sys/class/regulator/regulator.1/state
sh: can't create /sys/class/regulator/regulator.1/state: Permission denied
$ sudo echo enabled > /sys/class/regulator/regulator.1/state
sh: can't create /sys/class/regulator/regulator.1/state: Permission denied
```
- 打开了 CONFIG_REGULATOR_DEBUG 也不行，查看这些属性都是 read-only
```
$ cd /sys/class/regulator/regulator.1/
$ ls
device                 num_users              suspend_disk_state
max_microvolts         of_node                suspend_mem_state
microvolts             power                  suspend_standby_state
min_microvolts         state                  type
name                   subsystem              uevent
$ ls -lh
total 0
lrwxrwxrwx    1 root     root           0 Jan 10 15:59 device -> ../../../pwm_regulator
-r--r--r--    1 root     root        4.0K Jan 10 15:59 max_microvolts
-r--r--r--    1 root     root        4.0K Jan 10 15:59 microvolts
-r--r--r--    1 root     root        4.0K Jan 10 15:59 min_microvolts
-r--r--r--    1 root     root        4.0K Jan 10 15:59 name
-r--r--r--    1 root     root        4.0K Jan 10 15:59 num_users
lrwxrwxrwx    1 root     root           0 Jan 10 15:59 of_node -> ../../../../../firmware/devicetree/base/pwm_regulator
drwxr-xr-x    2 root     root           0 Jan 10 15:59 power
-r--r--r--    1 root     root        4.0K Jan 10 15:59 state
lrwxrwxrwx    1 root     root           0 Jan 10 15:59 subsystem -> ../../../../../class/regulator
-r--r--r--    1 root     root        4.0K Jan 10 15:59 suspend_disk_state
-r--r--r--    1 root     root        4.0K Jan 10 15:59 suspend_mem_state
-r--r--r--    1 root     root        4.0K Jan 10 15:59 suspend_standby_state
-r--r--r--    1 root     root        4.0K Jan 10 15:59 type
-rw-r--r--    1 root     root        4.0K Jan 10 15:59 uevent

```
- 设备树中添加属性`regulator-boot-on、regulator-always-on`可以看到 regulator 的输出了，默认输出值为最小电压所对应的占空比。
- 使用 userspace_consumer，也就是一个应用者来测试
	- 需要使用主线的 userspace_consumer.c 来测试，并且需要添加set_get voltage 接口




## 【已解决】libgpiod 编译方法
- gpio 采用 linux 的标准接口 gpiod，用户空间可以使用 libgpiod 来进行测试，libgpiod 尝试了好几个版本最后发现有一个版本可用
- 其中 v2.1.1 版本的 libgpiod 用的是 linux 中的 gpiod_v2 版本接口，目前 4.19 用的是 v1 版本的接口，所以使用起来会出错
- 其中 v1.4、v1.5、v1.6 版本的 libgpiod 在执行 configure 时都会报错 `libgpiod needs linux headers version >= v5.5.0`，因此不可用
- 在 github 上找到一个帖子： [https://github.com/aquaticus/nexus433/issues/21](https://github.com/aquaticus/nexus433/issues/21) 上面说可以 v1.1.1 版本可用，实际编译下来确实可用！
- 记录 v1.1.1 版本编译方法：  
```shell
# 用于生成 configure
./autogen.sh --enable-tools

# 配置脚本
export CROSS_COMPILE=aarch64-none-linux-gnu-

./configure \
	--host=aarch64-none-linux-gnu \
	--prefix=/home/tanxzh/tanxzh/tools/libgpio-1.1.1/test \
	--enable-tools \
	--enable-static \
	--disable-shared \
	--disable-bindings-cxx \

# 将工具切成静态编译
libgpiod-1.1.1/src/tools/Makefile.am 中的 AM_CFLAGS 添加 --static

# 将 config.h 中的 #define malloc rpl_malloc 换成 #define malloc malloc

# 编译和安装
make
```


## 【已解决】gpio 硬件是否支持先配置电平值，再配置输出模式？
配置输出模式的时候需要同步配置输出模式的初始电平，我们控制器支持先配置电平值，再配置成输出模式吗？
linux 里面其它厂商大多都是先配置初始电平，再配置为输出模式；这样可以有效的避免毛刺，假如现在是输入模式，引脚上是低电平，先配成输出模式的话会导致内部上拉可能变成高电平，再配置电平的话，这期间会有一个短暂的高电平产生。

- 支持


## pwm 需要改成有步进值的类型
- pwm-regulator 后续会使用 sar adc 回采电压，那么 pwm-regulator 的 set_voltage 接口需要做成一个闭环，设定电压、回采电压直到设置为准确的电压
- 由于温度或者负载的改变，可能会导致电压改变，所以需要做一个定时器，1S 或其它时间来设定一次电压