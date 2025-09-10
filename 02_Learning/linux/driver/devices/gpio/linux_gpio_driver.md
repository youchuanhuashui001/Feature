
# 设备树可以实现配置GPIO输出高电平的功能吗？
- 标准的做法还是通过设备树定义gpio，驱动来处理拉高拉低
#### ​**​步骤 1：设备树中定义 GPIO​**​

```
/ {
    my_device {
        compatible = "my,gpio-device";
        power-gpios = <&gpio1 6 GPIO_ACTIVE_HIGH>; // 定义 GPIO1_6 为输出
        enable-gpios = <&gpio1 7 GPIO_ACTIVE_LOW>;  // 定义 GPIO1_7（低电平有效）
    };
};
```

#### ​**​步骤 2：驱动中设置电平​**​

在对应的驱动代码中（如 Platform Driver）：

```
#include <linux/gpio/consumer.h>

static int my_probe(struct platform_device *pdev) {
    struct gpio_desc *power_gpio, *enable_gpio;

    // 获取设备树中定义的 GPIO
    power_gpio = devm_gpiod_get(&pdev->dev, "power", GPIOD_OUT_HIGH);
    enable_gpio = devm_gpiod_get(&pdev->dev, "enable", GPIOD_OUT_LOW);

    // 动态调整电平
    gpiod_set_value(power_gpio, 1); // 输出高电平
    gpiod_set_value(enable_gpio, 0); // 输出低电平
    return 0;
}
```
