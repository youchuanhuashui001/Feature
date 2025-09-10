#doing
#备注：gxloader需要周龙提供补丁，在PMU域正确操作LED

- C08
	- standby，待机
	- 两种情况都加
	- 是否在pmu域？是的
- D09
	- running，运行
	- 打印 Loading 的时候亮


# Gxloader 
- 将宏放到 board_config.h 来处理
- pmu 域需要正确处理 gpio 的亮灭

# Linux 


## 设备树
```d
/ {
    leds {
        compatible = "gpio-leds";
        
        led-status {
            label = "status";
            gpios = <&gpio 0 GPIO_ACTIVE_HIGH>;
            linux,default-trigger = "heartbeat";
            default-state = "on";
        };
        
        led-power {
            label = "power";
            gpios = <&gpio 1 GPIO_ACTIVE_LOW>;
            linux,default-trigger = "default-on";
            default-state = "off";
        };
    };
};

gpio: gpio-controller@0 {
    compatible = "vendor,gpio-controller";
    reg = <0x0 0x1000>;
    gpio-controller;
    #gpio-cells = <2>;
};
```

## 配置
```
# LED核心支持
CONFIG_NEW_LEDS=y
CONFIG_LEDS_CLASS=y

# GPIO LED驱动
CONFIG_LEDS_GPIO=y
CONFIG_LEDS_GPIO_REGISTER=y

# GPIO子系统
CONFIG_GPIOLIB=y
CONFIG_OF_GPIO=y

# 可选：LED触发器支持
CONFIG_LEDS_TRIGGERS=y
CONFIG_LEDS_TRIGGER_HEARTBEAT=y
CONFIG_LEDS_TRIGGER_DEFAULT_ON=y
CONFIG_LEDS_TRIGGER_TIMER=y
```

## sysfs 测试
```shell
# 查看所有LED设备
ls /sys/class/leds/

# 查看LED属性
cat /sys/class/leds/led-status/brightness
cat /sys/class/leds/led-status/max_brightness
cat /sys/class/leds/led-status/trigger

# 控制LED亮度
echo 1 > /sys/class/leds/led-status/brightness  # 点亮
echo 0 > /sys/class/leds/led-status/brightness  # 熄灭

# 设置触发器
echo heartbeat > /sys/class/leds/led-status/trigger
echo timer > /sys/class/leds/led-status/trigger

# 配置闪烁参数
echo 500 > /sys/class/leds/led-status/delay_on
echo 500 > /sys/class/leds/led-status/delay_off
```


# 问题
## 【已解决】测试发现 Linux 起来之后可以控制 Running 灯，但是 Standby 灯也错误的亮了
- 怀疑是开关了控制器的时钟，导致 gpio 控制器重置了，把 C08 复位成 input 模式了？
	- 也不会啊，input 模式不会亮的
```
[    0.841893] [gx_gpio_request] bank:1, offset:b
[    0.845568] vdd-cpu: supplied by regulator-dummy
[    0.846533] vdd-gpu-core: supplied by regulator-dummy
[    0.847566] cpu cpu0: Linked as a consumer to regulator.2
[    0.848135] cpu cpu0: Dropping the link to regulator.2
[    0.848837] cpu cpu0: Linked as a consumer to regulator.2
[    0.849750] energy_model: pd0: invalid cap. state: -22
[    0.850302] cpu_cooling: cpu0 is not running as cooling device: -22
[    0.851235] [gx_gpio_request] bank:1, offset:16
```
- 谁在用 gpio？
	- 16 是 16进制的，也就是 22，对应的是 54 --> D09
	- b 是 regulator-pcie，这里是 43，刚好就是 C08
		- 状态改成 disable 之后可以了

## 【已解决】uboot poweroff 之后灯不亮，无法唤醒
- 灯不亮，看看点压
- 将 gxbin 下面的 lowpower.bin 烧到 0x3000，再按键唤醒
	- 正常了

## 【已解决】换成 mmc0 作为 trigger，找不到 mmcblock0
- 更新 loader 可以了，但是 saradc 占用了 D09 引脚
- gxloader 将 D09 配成 gpio 可以了

- 不是只会有 mmc0，如果还插入sd卡，就会有mmcblk1，sdiowifi，就会有mmcblk2

## 【已解决】默认在 running 目录下就会有 mmc0、mmc1
- sdhci 驱动在 add_host 时会注册 led 类，这里注册的是 `mmc0::`，ott 中只用了 emmc 和 sdcard，所以有两个 mmc


## 【已解决】MUTE_CTRL 确认是谁来调？
- 谁要用谁来调， 我这边提供 gpio 的接口即可


## 【已解决】USB两个引脚需要确保是高电平，不能有人修改
- 这两个引脚默认是输入模式，需要 linux 时配置这两个引脚为输出模式，输出高电平