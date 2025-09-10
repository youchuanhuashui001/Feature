
# Provider
### 第一部分：设备树 (Device Tree) - 硬件的“蓝图”

设备树是第一步，它告诉内核“这里有一个GPIO控制器，它的硬件资源是什么”。内核会根据这些信息去匹配并加载您的驱动。

**一个典型的GPIO Provider设备树节点如下：**
```d
/* 在您的 soc.dtsi 文件中 */
gpio_controller0: gpio@fa400000 {
    /* 1. 兼容性字符串，用于内核匹配驱动 */
    compatible = "your-company,your-soc-gpio";

    /* 2. 硬件寄存器地址和大小 */
    reg = <0xfa400000 0x1000>;

    /* 3. 声明这是一个GPIO控制器 */
    gpio-controller;

    /* 4. 定义消费者如何引用这个控制器的GPIO。
     * <2> 表示需要两个cell：第一个是pin号，第二个是标志(如高/低电平有效)
     */
    #gpio-cells = <2>;

    /* --- 以下是中断相关的部分 (如果支持) --- */

    /* 5. 连接到主中断控制器(如GIC)的中断号 */
    interrupt-parent = <&gic>;
    interrupts = <0 90 4>; /* 示例：SPI中断，中断号90，触发方式4(上升沿) */

    /* 6. 声明这也是一个中断控制器 */
    interrupt-controller;

    /* 7. 定义消费者如何引用这个控制器提供的中断。
     * <2> 表示需要两个cell：第一个是pin号(作为中断源)，第二个是触发类型
     */
    #interrupt-cells = <2>;
};
```

---

### 第二部分：驱动程序代码 - 核心实现

通常，GPIO控制器驱动以**平台驱动 (Platform Driver)** 的形式存在。

#### A. 驱动框架 - Platform Driver

这是您驱动的骨架。
```c
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/driver.h>
#include <linux/interrupt.h>
// ... 其他头文件

// 1. 定义驱动支持的 "compatible" 字符串
static const struct of_device_id my_gpio_of_match[] = {
    { .compatible = "your-company,your-soc-gpio", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_gpio_of_match);

// 2. 定义 probe 和 remove 函数
static int my_gpio_probe(struct platform_device *pdev);
static int my_gpio_remove(struct platform_device *pdev);

// 3. 定义平台驱动结构体
static struct platform_driver my_gpio_driver = {
    .driver = {
        .name = "my-gpio-driver",
        .of_match_table = my_gpio_of_match,
    },
    .probe = my_gpio_probe,
    .remove = my_gpio_remove,
};

// 4. 使用 module_platform_driver 宏来注册和注销驱动
module_platform_driver(my_gpio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("GPIO provider driver for My SoC");
```

#### B. 自定义数据结构

定义一个私有结构体来保存驱动的所有状态，这是一种良好的编程实践。
```c
struct my_gpio_chip {
    struct gpio_chip chip;      // 内核的 gpio_chip 结构体
    void __iomem *base;       // 寄存器基地址
    spinlock_t lock;            // 用于保护寄存器访问的自旋锁
    // 如果支持中断，还需要 irq_chip 等
    struct irq_chip irqchip;
};
```

#### C. `probe` 函数 - 初始化入口

`probe` 函数是驱动的核心，当内核找到匹配的设备树节点时会调用它。
```c
static int my_gpio_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct my_gpio_chip *my_chip;
    int ret;

    /* 1. 分配并初始化自定义结构体 */
    my_chip = devm_kzalloc(dev, sizeof(*my_chip), GFP_KERNEL);
    if (!my_chip)
        return -ENOMEM;
    
    platform_set_drvdata(pdev, my_chip); // 将私有数据与设备关联

    /* 2. 映射MMIO寄存器 */
    my_chip->base = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(my_chip->base))
        return PTR_ERR(my_chip->base);

    /* 3. 初始化自旋锁 */
    spin_lock_init(&my_chip->lock);

    /* 4. 填充 gpio_chip 结构体 (关键步骤!) */
    my_chip->chip.label = dev_name(dev);
    my_chip->chip.parent = dev;
    my_chip->chip.owner = THIS_MODULE;
    my_chip->chip.base = -1; // -1 表示让内核动态分配GPIO编号
    my_chip->chip.ngpio = 32; // 您的控制器支持的GPIO数量
    my_chip->chip.can_sleep = false; // 如果寄存器访问不能休眠，则为false

    // 绑定回调函数 (这些是您需要根据硬件手册实现的函数)
    my_chip->chip.direction_input = my_gpio_direction_input;
    my_chip->chip.direction_output = my_gpio_direction_output;
    my_chip->chip.get = my_gpio_get;
    my_chip->chip.set = my_gpio_set;
    my_chip->chip.set_config = my_gpio_set_config; // 用于上拉/下拉等配置

    /* 5. (可选) 填充并关联 irq_chip */
    // 稍后详细介绍

    /* 6. 向内核注册 gpio_chip */
    ret = devm_gpiochip_add_data(dev, &my_chip->chip, my_chip);
    if (ret < 0) {
        dev_err(dev, "Failed to add gpiochip\n");
        return ret;
    }
    
    dev_info(dev, "GPIO driver registered successfully for %u gpios\n", my_chip->chip.ngpio);

    return 0;
}
```

#### D. 实现 `gpio_chip` 的回调函数

这些函数是您的驱动与物理硬件交互的桥梁。您需要根据芯片的数据手册来读写相应的寄存器。
```c
// 设置GPIO为输入方向
static int my_gpio_direction_input(struct gpio_chip *chip, unsigned int offset)
{
    struct my_gpio_chip *my_chip = gpiochip_get_data(chip);
    unsigned long flags;

    spin_lock_irqsave(&my_chip->lock, flags);
    // TODO: 写硬件寄存器，将 offset 对应的pin配置为输入模式
    // 例如: writel(new_value, my_chip->base + DIRECTION_REG);
    spin_unlock_irqrestore(&my_chip->lock, flags);

    return 0;
}

// 设置GPIO为输出方向并设置初始值
static int my_gpio_direction_output(struct gpio_chip *chip, unsigned int offset, int value)
{
    struct my_gpio_chip *my_chip = gpiochip_get_data(chip);
    unsigned long flags;

    spin_lock_irqsave(&my_chip->lock, flags);
    // TODO: 1. 写硬件寄存器，将 offset 对应的pin配置为输出模式
    // TODO: 2. 写硬件寄存器，将 offset 对应的pin的电平设置为 value
    spin_unlock_irqrestore(&my_chip->lock, flags);
    
    // 或者先调用 set 函数
    // my_gpio_set(chip, offset, value);

    return 0;
}

// 获取GPIO电平值
static int my_gpio_get(struct gpio_chip *chip, unsigned int offset)
{
    struct my_gpio_chip *my_chip = gpiochip_get_data(chip);
    u32 reg_val;

    // TODO: 读硬件寄存器，获取 offset 对应的pin的电平值
    // reg_val = readl(my_chip->base + DATA_IN_REG);
    
    // return (reg_val & BIT(offset)) ? 1 : 0;
    return 0; // 示例返回值
}

// 设置GPIO电平值
static void my_gpio_set(struct gpio_chip *chip, unsigned int offset, int value)
{
    struct my_gpio_chip *my_chip = gpiochip_get_data(chip);
    unsigned long flags;
    u32 reg_val;

    spin_lock_irqsave(&my_chip->lock, flags);
    // TODO: 实现“读-改-写”原子操作来设置电平
    // reg_val = readl(my_chip->base + DATA_OUT_REG);
    // if (value)
    //     reg_val |= BIT(offset);
    // else
    //     reg_val &= ~BIT(offset);
    // writel(reg_val, my_chip->base + DATA_OUT_REG);
    spin_unlock_irqrestore(&my_chip->lock, flags);
}

// (可选) 配置上拉/下拉等
static int my_gpio_set_config(struct gpio_chip *chip, unsigned int offset, unsigned long config)
{
    // enum pin_config_param param = pinconf_to_config_param(config);
    // u32 arg = pinconf_to_config_argument(config);
    // switch (param) {
    // case PIN_CONFIG_BIAS_PULL_UP:
    //     // TODO: 配置硬件为上拉
    //     break;
    // ...
    // }
    return -ENOTSUP;
}
```

---

### 第三部分：中断处理实现

如果您的GPIO控制器还管理中断，您需要将它注册为一个`irq_chip`。

**1. 在 `probe` 函数中添加中断初始化**
```c
// ... 在 devm_gpiochip_add_data 之前添加 ...

struct irq_chip *irqchip = &my_chip->irqchip;

irqchip->name = dev_name(dev);
irqchip->irq_mask = my_irq_mask;       // 屏蔽中断
irqchip->irq_unmask = my_irq_unmask;   // 使能中断
irqchip->irq_set_type = my_irq_set_type; // 设置触发类型(边沿/电平)

// 将 gpiochip 和 irqchip 关联起来
ret = gpiochip_irqchip_add(&my_chip->chip,
                             irqchip,
                             0, // 第一个IRQ从哪个GPIO开始
                             handle_level_irq, // 中断处理方式，通常是 handle_level_irq
                             IRQ_TYPE_NONE);
if (ret) {
    dev_err(dev, "could not add irqchip\n");
    return ret;
}

// 获取父中断号，并设置中断链
int parent_irq = platform_get_irq(pdev, 0);
if (parent_irq < 0)
    return parent_irq;

gpiochip_set_chained_irq_handler(&my_chip->chip,
                                 my_chained_irq_handler, // 您的级联中断处理函数
                                 parent_irq);

```

**2. 实现 `irq_chip` 回调和级联中断处理函数**
- `my_irq_mask`/`my_irq_unmask`: 实现对单个中断源的使能/屏蔽。
- `my_irq_set_type`: 根据内核要求，设置硬件的触发方式。
- `my_chained_irq_handler`: 这是当中断线上有信号时，内核首先调用的函数。您需要在这个函数里：
	1. 读取中断状态寄存器，判断是哪个GPIO触发了中断。
	2. 循环处理所有触发了中断的GPIO。
	3. 对每一个触发中断的GPIO，调用 `generic_handle_irq()` 将中断上报给内核。
	4. 清除中断标志位。


# Consumer
#### A. 核心思想：设备树是“连接器”

消费者驱动不应该知道GPIO的具体编号，而应该通过设备树来请求一个具有特定功能的GPIO。

#### B. 步骤一：在设备树 (Device Tree) 中声明连接

这是实现解耦和硬件无关性的关键。在您的**消费者设备**的节点中，通过 `xxx-gpios` 属性来引用您的GPIO控制器提供的GPIO。

**示例**：假设您的GPIO控制器节点叫 `gpio_controller`，您有一个LED设备需要连接到该控制器的第15号引脚 (高电平点亮)。


```d
// 这是您的GPIO控制器节点 (生产者)
gpio_controller: gpio@fa400000 {
    compatible = "your-company,your-gpio-controller";
    reg = <0xfa400000 0x1000>;
    gpio-controller;
    #gpio-cells = <2>; // <pin_number flags>
    ...
};

// 这是您的LED设备节点 (消费者)
heartbeat_led: led {
    compatible = "gpio-leds";
    label = "heartbeat";
    
    // "led-gpios" 是一个自定义的名字，指明了用途
    // <&gpio_controller 15 GPIO_ACTIVE_HIGH> 
    // - &gpio_controller: 引用您的GPIO控制器
    // - 15: 您控制器上的第15号引脚
    // - GPIO_ACTIVE_HIGH: 表明高电平有效
    led-gpios = <&gpio_controller 15 0>; // 0 代表 GPIO_ACTIVE_HIGH

    // --- 核心的中断请求 --- 
    interrupt-parent = <&gpio_controller0>; // 指明中断的提供者是谁   
    interrupts = <17 IRQ_TYPE_EDGE_RISING>; // 请求中断的具体细节
};
```

#### C. 步骤二：在消费驱动 (Consumer Driver) 的C代码中获取和使用

在消费者驱动（例如 `gpio-leds.c` 或您自己的驱动）的 `probe` 函数中，通过 `gpiod` API 来获取和操作GPIO。

**1. 获取GPIO描述符 (Descriptor)**

使用`devm_gpiod_get()`函数来获取描述符。`devm_`前缀表示这是受设备管理的资源，当驱动卸载时会自动释放，能有效防止资源泄漏。
后续一直使用`struct gpio_desc`作为参数来调用接口。

```c
#include <linux/gpio/consumer.h>

static int my_driver_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct gpio_desc *led_gpio;

    // 从设备树的 "led-gpios" 属性中获取GPIO描述符
    // GPIOD_OUT_HIGH 表示获取后立即设为输出高电平
    led_gpio = devm_gpiod_get(dev, "led", GPIOD_OUT_HIGH);
    if (IS_ERR(led_gpio)) {
        dev_err(dev, "Failed to get led-gpios\n");
        return PTR_ERR(led_gpio);
    }
    
    // ... 将 led_gpio 保存起来，供后续使用 ...
    
    return 0;
}


// devm_gpiod_get 接口获取到描述符后，在设备卸载时会自动释放
// 也可以通过接口 gpiod_get、gpiod_put 手动获取、释放
// gpiod_request --> chip->request --> chip->get_direction
// gpiod_configure_flags --> direction_output / direction_input
```

**2. 控制GPIO输出**


```c
// 设置GPIO为输出方向，并设置初始值 (0为低, 1为高)
gpiod_direction_output(led_gpio, 1);

// 后续在代码中改变电平
// chip->set
gpiod_set_value(led_gpio, 0); // 设置为低
mdelay(500);
gpiod_set_value(led_gpio, 1); // 设置为高
```

**3. 读取GPIO输入**

如果GPIO用作输入（例如一个按键）。


```c
struct gpio_desc *button_gpio;

// 获取GPIO并设置为输入
button_gpio = devm_gpiod_get(dev, "button", GPIOD_IN);
if (IS_ERR(button_gpio)) {
    // ... 错误处理 ...
}

// 读取当前值
// chip->get
int value = gpiod_get_value(button_gpio);
if (value) {
    // 按键被按下 (假设高电平有效)
} else {
    // 按键未被按下
}
```

**4. 使用中断**

如果需要用GPIO作为中断源。


```c
int irq;

// 1. 获取GPIO描述符
button_gpio = devm_gpiod_get(dev, "button", GPIOD_IN);
// ... 错误检查 ...

// 2. 将GPIO描述符转换为中断号
irq = gpiod_to_irq(button_gpio);
if (irq < 0) {
    return irq;
}

// 3. 注册中断处理函数
// IRQF_TRIGGER_RISING: 上升沿触发
// my_irq_handler: 你的中断处理函数
if (devm_request_threaded_irq(dev, irq, NULL, my_irq_handler,
                              IRQF_TRIGGER_RISING | IRQF_ONESHOT,
                              "my-button", dev)) {
    dev_err(dev, "Failed to request IRQ %d\n", irq);
    return -EINVAL;
}

// 中断处理函数
static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    printk(KERN_INFO "Button pressed!\n");
    return IRQ_HANDLED;
}
```



# Userspace

#### A. 核心思想：通过标准库 `libgpiod` 访问设备

`libgpiod` 提供了稳定的API和一系列命令行工具，是现代Linux系统下用户空间访问GPIO的标准方式。

#### B. 方案一：使用 `libgpiod` 命令行工具 (用于测试、脚本)

这是最简单快捷的方式。

- **`gpiodetect`**: 列出系统中所有的GPIO控制器。
```shell
$ ./gpiodetect
gpiochip0 [your-gpio-controller] (32 lines)
gpiochip1 [another-chip] (16 lines)
```

- **`gpioinfo`**: 查看某个控制器的所有GPIO线路详情。
```shell
$ ./gpioinfo gpiochip0
```
    
- **`gpioset`**: 设置某个GPIO为输出并改变其电平。
```shell
# 将 gpiochip0 的第15号线设为高电平
$ ./gpioset gpiochip0 15=1
```
    
- **`gpioget`**: 读取某个GPIO的输入电平。
```shell
# 读取 gpiochip0 的第16号线电平
$ ./gpioget gpiochip0 16
```
    
- **`gpiomon`**: 监控GPIO的边沿事件（中断）。
```shell
# 监控 gpiochip0 第17号线的上升沿和下降沿事件
$ ./gpiomon gpiochip0 17
```
    

#### C. 方案二：在C/C++程序中使用 `libgpiod` 库 (用于应用程序)

当您需要编写自己的应用程序来控制GPIO时，直接调用`libgpiod`的API。

**示例：一个简单的C程序来闪烁一个LED**
```c
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    const char *chipname = "gpiochip0";
    unsigned int line_num = 15; // GPIO 15
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret, i;

    // 1. 打开GPIO控制器
    chip = gpiod_chip_open_by_name(chipname);
    if (!chip) {
        perror("Open chip failed");
        return 1;
    }

    // 2. 获取GPIO线路
    line = gpiod_chip_get_line(chip, line_num);
    if (!line) {
        perror("Get line failed");
        gpiod_chip_close(chip);
        return 1;
    }

    // 3. 请求将该线路用作输出，并设置初始值为低
    // "my-led-app" 是消费者名称，方便调试
    ret = gpiod_line_request_output(line, "my-led-app", 0);
    if (ret < 0) {
        perror("Request line as output failed");
        gpiod_line_release(line);
        gpiod_chip_close(chip);
        return 1;
    }

    // 4. 闪烁5次
    printf("Blinking LED on line %u...\n", line_num);
    for (i = 0; i < 5; i++) {
        gpiod_line_set_value(line, 1); // 设置为高
        usleep(500000);
        gpiod_line_set_value(line, 0); // 设置为低
        usleep(500000);
    }

    // 5. 释放资源
    gpiod_line_release(line);
    gpiod_chip_close(chip);

    return 0;
}
```

**编译命令**： `gcc your_program.c -o your_program -lgpiod`

#### D. 方案三：旧的 `sysfs` 接口 (不推荐)

在非常老的内核或特定场景下，您可能会看到通过 `/sys/class/gpio/` 目录操作GPIO的方式。

**此方法已被废弃！** 原因包括：

- **有竞争条件**：`export`之后设置`direction`和`value`不是原子操作。
    
- **性能差**：每次操作都是一次文件读写，开销大。
    
- **功能有限**：无法处理“高开漏/高开集”等配置，无法原子地操作多条线路。





# Example
## Consumer


```d
    gpiod_example {
        compatible = "gpiod-example,test";

        // 单个输出, 使用 gpio1 的第 14 脚
        output-gpios = <&gpio1 14 GPIO_ACTIVE_HIGH>;

        // 单个输入, 使用 gpio1 的第 15 脚
//        input-gpios = <&gpio1 15 GPIO_ACTIVE_HIGH>;

        // 中断输入, 使用 gpio1 的第 16 脚
        interrupt-gpios = <&gpio1 15 GPIO_ACTIVE_HIGH>;
    };
```

```makefile
obj-m += gpiod_example.o
```


```c
// gpiod_example.c

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

struct gpiod_example_data {
	struct gpio_desc *output_gpio;
	struct gpio_descs *array_gpios;
	struct gpio_desc *input_gpio;
	struct gpio_desc *interrupt_gpio;
	int irq;
};

/* 中断处理函数 */
static irqreturn_t gpiod_example_irq_handler(int irq, void *dev_id)
{
	struct device *dev = dev_id;
	struct gpiod_example_data *data = dev_get_drvdata(dev);
	int value;

	pr_info("gpiod-example: Interrupt %d triggered!\n", irq);

	/* 在中断中翻转输出GPIO的电平，以产生可视化效果 */
	if (data->output_gpio) {
		value = gpiod_get_value_cansleep(data->output_gpio);
		gpiod_set_value_cansleep(data->output_gpio, !value);
		pr_info("gpiod-example: Toggled output GPIO to %d\n", !value);
	}

	return IRQ_HANDLED;
}

static int gpiod_example_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct gpiod_example_data *data;
	int ret;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	platform_set_drvdata(pdev, data);

	pr_info("gpiod-example: Probe started...\n");

	/* 1. 获取并配置单个输出 GPIO */
	pr_info("gpiod-example: --- Testing single GPIO I/O ---\n");
	// GPIOD_OUT_LOW: 获取GPIO并将其配置为输出，初始值为低电平
	data->output_gpio = devm_gpiod_get(dev, "output", GPIOD_OUT_LOW);
	if (IS_ERR(data->output_gpio)) {
		dev_err(dev, "Failed to get 'output-gpios'\n");
		return -1;
	}
	pr_info("gpiod-example: Got output GPIO. Initial value is LOW.\n");
	msleep(500); // 延迟以便观察
	gpiod_set_value(data->output_gpio, 1); // 设置为高电平
	pr_info("gpiod-example: Set output GPIO to HIGH.\n");
	msleep(500); // 延迟以便观察

	gpiod_set_value(data->output_gpio, 0); // 设置为高电平
	msleep(500); // 延迟以便观察
	gpiod_set_value(data->output_gpio, 1); // 设置为高电平
	msleep(500); // 延迟以便观察
	gpiod_set_value(data->output_gpio, 0); // 设置为高电平
	msleep(500); // 延迟以便观察
	gpiod_set_value(data->output_gpio, 1); // 设置为高电平
	msleep(500); // 延迟以便观察
	gpiod_set_value(data->output_gpio, 0); // 设置为高电平
	msleep(500); // 延迟以便观察
	gpiod_set_value(data->output_gpio, 1); // 设置为高电平
	msleep(500); // 延迟以便观察
	gpiod_set_value(data->output_gpio, 0); // 设置为高电平
	msleep(500); // 延迟以便观察
	gpiod_set_value(data->output_gpio, 1); // 设置为高电平
	msleep(500); // 延迟以便观察
	gpiod_set_value(data->output_gpio, 0); // 设置为高电平
	msleep(500); // 延迟以便观察
	gpiod_set_value(data->output_gpio, 1); // 设置为高电平
	msleep(500); // 延迟以便观察


//	/* 2. 获取并配置 GPIO 数组 */
//	pr_info("gpiod-example: --- Testing GPIO array I/O ---\n");
//	// 获取所有名为 "array" 的GPIO，并配置为输出，初始值为高电平
//	data->array_gpios = devm_gpiod_get_array(dev, "array", GPIOD_OUT_HIGH);
//	if (IS_ERR(data->array_gpios)) {
//		return dev_err_probe(dev, PTR_ERR(data->array_gpios),
//				     "Failed to get 'array-gpios'\n");
//	}
//	pr_info("gpiod-example: Got %zu GPIOs in array. Initial value is HIGH.\n",
//		data->array_gpios->ndescs);
//	msleep(500);
//
//	// 创建一个值数组，将 GPIO 数组设置为交替的 0 和 1
//	if (data->array_gpios->ndescs > 0) {
//		int *values = kcalloc(data->array_gpios->ndescs, sizeof(int), GFP_KERNEL);
//		if (values) {
//			for (int i = 0; i < data->array_gpios->ndescs; i++)
//				values[i] = i % 2;
//			gpiod_set_array_value(data->array_gpios->ndescs,
//					      data->array_gpios->desc, values);
//			pr_info("gpiod-example: Set GPIO array to alternating pattern.\n");
//			kfree(values);
//		}
//	}


//	/* 3. 获取并读取单个输入 GPIO */
//	pr_info("gpiod-example: --- Testing single GPIO input ---\n");
//	// GPIOD_IN: 获取GPIO并将其配置为输入
//	data->input_gpio = devm_gpiod_get(dev, "input", GPIOD_IN);
//	if (IS_ERR(data->input_gpio)) {
//		dev_err(dev, "Failed to get 'input-gpios'\n");
//		return -1;
//	}
//	// 查询方向和值
//	pr_info("gpiod-example: Got input GPIO. Is active low: %d\n",
//		gpiod_is_active_low(data->input_gpio));
//	pr_info("gpiod-example: Current direction: %s\n",
//		(gpiod_get_direction(data->input_gpio) == 1) ? "IN" : "OUT");
//	pr_info("gpiod-example: Current value: %d\n",
//		gpiod_get_value(data->input_gpio));


	/* 4. 获取 GPIO 并配置为中断 */
	pr_info("gpiod-example: --- Testing GPIO interrupt ---\n");
	// GPIOD_IN: 先获取为输入
	data->interrupt_gpio = devm_gpiod_get(dev, "interrupt", GPIOD_IN);
	if (IS_ERR(data->interrupt_gpio)) {
		dev_err(dev, "Failed to get 'interrupt-gpios'\n");
		return -1;
	}

	// 将 descriptor 转换为 IRQ 编号
	data->irq = gpiod_to_irq(data->interrupt_gpio);
	if (data->irq < 0) {
		dev_err(dev, "Failed to get IRQ from GPIO: %d\n", data->irq);
		return data->irq;
	}
	pr_info("gpiod-example: Mapped GPIO to IRQ %d\n", data->irq);
	
	// 为 IRQ 请求一个带线程的处理器
	// IRQF_TRIGGER_RISING: 上升沿触发
	// gpiod_example_irq_handler: 中断处理函数
	// IRQF_ONESHOT: 防止中断风暴，在处理函数完成前，中断线是屏蔽的
	ret = devm_request_threaded_irq(dev, data->irq,
					NULL, /* primary handler */
					gpiod_example_irq_handler,
					IRQF_TRIGGER_RISING | IRQF_ONESHOT,
					"gpiod-example", dev);
	if (ret) {
		dev_err(dev, "Failed to request IRQ %d: %d\n", data->irq, ret);
		return ret;
	}

	pr_info("gpiod-example: Probe finished successfully.\n");
	return 0;
}

static int gpiod_example_remove(struct platform_device *pdev)
{
	pr_info("gpiod-example: Remove called. Resources will be freed automatically by devm.\n");
	return 0;
}

static const struct of_device_id gpiod_example_of_match[] = {
	{ .compatible = "gpiod-example,test" },
	{},
};
MODULE_DEVICE_TABLE(of, gpiod_example_of_match);

static struct platform_driver gpiod_example_driver = {
	.driver = {
		.name = "gpiod-example",
		.of_match_table = gpiod_example_of_match,
	},
	.probe = gpiod_example_probe,
	.remove = gpiod_example_remove,
};

module_platform_driver(gpiod_example_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AI Assistant");
MODULE_DESCRIPTION("A comprehensive example of using the gpiod interface in the kernel");
```



## Test

### sysfs
```shell
$ echo 45 > /sys/class/gpio/export
[  307.284415] [gx_gpio_request] bank:1, offset:d
[  307.288955] [gx_gpio_get_direction] bank:1, offset:d, data:1
$ [  311.511646] random: crng init done
$ cd /sys/class/gpio/
$ ls
export      gpiochip0   gpiochip64  unexport
gpio45      gpiochip32  gpiochip96
$ cd gpio45/
$ ls
active_low  direction   power       uevent
device      edge        subsystem   value
$ cat value
[  329.198039] [gx_gpio_get] bank:1, offset:d, value:1
1
[  329.203107] [gx_gpio_get] bank:1, offset:d, value:1
[  329.208302] [gx_gpio_get] bank:1, offset:d, value:1
$ echo 0 > value
[  338.146761] [gx_gpio_set] bank:1, offset:d, value:0
$ echo 1 > value [  358.730509] [gx_gpio_set] bank:1, offset:d, value:1

$ echo 0 > value [  362.300854] [gx_gpio_set] bank:1, offset:d, value:0

$ ls
active_low  direction   power       uevent
device      edge        subsystem   value
$ cat direction
[  368.662160] [gx_gpio_get_direction] bank:1, offset:d, data:1
out
$ echo in > direction
[  372.685603] [gx_gpio_direction_input] bank:1, offset:d
$ cat value
[  377.320681] [gx_gpio_get] bank:1, offset:d, value:1
1
[  377.325746] [gx_gpio_get] bank:1, offset:d, value:1
[  377.330918] [gx_gpio_get] bank:1, offset:d, value:1
$ cat value
[  385.326007] [gx_gpio_get] bank:1, offset:d, value:0
0
[  385.331129] [gx_gpio_get] bank:1, offset:d, value:0
[  385.336305] [gx_gpio_get] bank:1, offset:d, value:0

$ echo 45 > /sys/class/gpio/unexport
```

### tools/gpio
```shell
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- LDFLAGS=-static -C tools/gpio
```
- 拷贝测试程序到 nfs 目录
- 执行测试
```shell
./gpio-hammer -n gpiochip1 -o 13


$ ./gpio-event-mon -n gpiochip 1 -o 13 -r -f
Failed to open /dev/gpiochip
Failed to close GPIO character device file: Bad file descriptor
$ ./gpio-event-mon -n gpiochip1 -o 13 -r -f
[ 1086.676281] [gx_gpio_request] bank:1, offset:d
[ 1086.680806] [gx_gpio_get_direction] bank:1, offset:d, data:1
[ 1086.686507] [gx_gpio_direction_input] bank:1, offset:d
[ 1086.691930] [gx_gpio_get_direction] bank:1, offset:d, data:0
[ 1086.697834] [gx_gpio_get] bank:1, offset:d, value:1
Monitoring line 13 on gpiochip1
Initial line value: 1
GPIO EVENT 839958097313647: falling edge
[ 1109.228361] [gx_gpio_get] bank:1, offset:d, value:1
GPIO EVENT 839958388170439: rising edge
[ 1109.635426] [gx_gpio_get] bank:1, offset:d, value:0
GPIO EVENT 839958795247147: falling edge
[ 1109.640388] [gx_gpio_get] bank:1, offset:d, value:0
GPIO EVENT 839958800214981: falling edge
[ 1109.863962] [gx_gpio_get] bank:1, offset:d, value:1
GPIO EVENT 839959023788064: rising edge
[ 1110.269708] [gx_gpio_get] bank:1, offset:d, value:0
[ 1110.274691] [gx_gpio_get] bank:1, offset:d, value:0
GPIO EVENT 839959429515814: falling edge
GPIO EVENT 839959434512981: falling edge
[ 1110.450420] [gx_gpio_get] bank:1, offset:d, value:0
GPIO EVENT 839959610243439: falling edge
[ 1110.455374] [gx_gpio_get] bank:1, offset:d, value:1
GPIO EVENT 839959615202314: rising edge
```

### libgpiod
```shell
$ ./gpioget gpiochip1 13
[ 1175.812532] [gx_gpio_request] bank:1, offset:d
[ 1175.817074] [gx_gpio_get_direction] bank:1, offset:d, data:1
[ 1175.822782] [gx_gpio_direction_input] bank:1, offset:d
[ 1175.827986] [gx_gpio_get] bank:1, offset:d, value:0
[ 1175.832923] [gx_gpio_free] bank:1, offset:d
0
$ ./gpioget gpiochip1 13
[ 1176.943337] [gx_gpio_request] bank:1, offset:d
[ 1176.947944] [gx_gpio_get_direction] bank:1, offset:d, data:1
[ 1176.953653] [gx_gpio_direction_input] bank:1, offset:d
[ 1176.958861] [gx_gpio_get] bank:1, offset:d, value:1
[ 1176.963845] [gx_gpio_free] bank:1, offset:d
1


$ ./gpioset gpiochip1 13=0
[ 1231.228845] [gx_gpio_request] bank:1, offset:d
[ 1231.233369] [gx_gpio_get_direction] bank:1, offset:d, data:1
[ 1231.239100] [gx_gpio_direction_output] bank:1, offset:d, data:0
[ 1231.245101] [gx_gpio_free] bank:1, offset:d
$ ./gpioset gpiochip1 13=1
[ 1240.430628] [gx_gpio_request] bank:1, offset:d
[ 1240.435252] [gx_gpio_get_direction] bank:1, offset:d, data:1
[ 1240.440987] [gx_gpio_direction_output] bank:1, offset:d, data:1
[ 1240.446977] [gx_gpio_free] bank:1, offset:d
$ ./gpioset gpiochip1 13=0
[ 1243.000019] [gx_gpio_request] bank:1, offset:d
[ 1243.004537] [gx_gpio_get_direction] bank:1, offset:d, data:1
[ 1243.010256] [gx_gpio_direction_output] bank:1, offset:d, data:0
[ 1243.016243] [gx_gpio_free] bank:1, offset:d


$ ./gpiomon -r -f gpiochip1 13
[ 1294.725526] [gx_gpio_request] bank:1, offset:d
[ 1294.730078] [gx_gpio_get_direction] bank:1, offset:d, data:1
[ 1294.735801] [gx_gpio_direction_input] bank:1, offset:d
[ 1294.741260] [gx_gpio_get_direction] bank:1, offset:d, data:0
[ 1294.747077] [gx_gpio_get] bank:1, offset:d, value:1
event:  RISING EDGE offset: 13 timestamp: [  840143.906861777]
[ 1301.634167] [gx_gpio_get] bank:1, offset:d, value:0
event: FALLING EDGE offset: 13 timestamp: [  840150.793969197]
[ 1301.931636] [gx_gpio_get] bank:1, offset:d, value:0
[ 1301.936600] [gx_gpio_get] bank:1, offset:d, value:1
```

