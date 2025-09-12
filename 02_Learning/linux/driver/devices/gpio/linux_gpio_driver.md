
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

# tools/gpio-utils

```c
int gpiotools_set(const char *device_name, unsigned int line,
		  unsigned int value)
{
	struct gpiohandle_data data;
	unsigned int lines[] = {line};

	data.values[0] = value;
	return gpiotools_sets(device_name, lines, 1, &data);
}

// gpiochip0 1 1 1: gpio1 set high level
int gpiotools_sets(const char *device_name, unsigned int *lines,
		   unsigned int nlines, struct gpiohandle_data *data)
{
	int ret;

	ret = gpiotools_request_linehandle(device_name, lines, nlines,
					   GPIOHANDLE_REQUEST_OUTPUT, data,
					   COMSUMER);
	if (ret < 0)
		return ret;

	return gpiotools_release_linehandle(ret);
}

#define GPIOHANDLE_REQUEST_INPUT	(1UL << 0)
#define GPIOHANDLE_REQUEST_OUTPUT	(1UL << 1)
	struct gpiohandle_request req;
	fd = open(chrdev_name, 0);
	if (fd == -1) {
		ret = -errno;
		fprintf(stderr, "Failed to open %s, %s\n",
			chrdev_name, strerror(errno));
		goto exit_close_error;
	}

	req.lineoffsets[i] = lines[i];
	req.flags = flag;
	strcpy(req.consumer_label, consumer_label);
	req.lines = nlines;
	if (flag & GPIOHANDLE_REQUEST_OUTPUT)
		memcpy(req.default_values, data, sizeof(req.default_values));

	ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	if (ret == -1) {
		ret = -errno;
		fprintf(stderr, "Failed to issue %s (%d), %s\n",
			"GPIO_GET_LINEHANDLE_IOCTL", ret, strerror(errno));
	}

	if (close(fd) == -1)
		perror("Failed to close GPIO character device file");
	free(chrdev_name);
	return ret < 0 ? ret : req.fd;


	ret = close(fd);
	if (ret == -1) {
		perror("Failed to close GPIO LINEHANDLE device file");
		ret = -errno;
	}



	// get_value
	fd = open(chrdev_name, 0);
	req.flags = GPIOHANDLE_REQUEST_INPUT;
	ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	ret = ioctl(fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, data);
	ret = close(fd);


	// enable_trigger
	struct gpioevent_request req;
	struct gpiohandle_data data;

	fd = open(chrdev_name, 0);

	req.lineoffset = line;
	req.handleflags = handleflags;
	req.eventflags = eventflags;
	strcpy(req.consumer_label, "gpio-event-mon");

	ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &req);
	ret = ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);

	ret = read(req.fd, &event, sizeof(event));
	switch (event.id) {
		case GPIOEVENT_EVENT_RISING_EDGE:
			fprintf(stdout, "rising edge");
			break;
		case GPIOEVENT_EVENT_FALLING_EDGE:
			fprintf(stdout, "falling edge");
			break;

	if (close(fd) == -1)

```

# 绘图
[[gpio.excalidraw]]

# GxHal

- getio
- setio
- getlevel
- setlevel
- enable_trigger
- disable_trigger



# 问题
## gpioevent_request 是否为每个 GPIO 引脚分配一个独立事件监听？

- ​**​是的。​**​ 每次成功调用 `gpioevent_request()` 函数时，内核内部都会执行 `gpioevent_create()` 操作。
- 在 `gpioevent_create()` 过程中：
    
    - 会为​**​此次特定的 GPIO 事件请求​**​创建一个专用的中断处理线程（IRQ thread）。
    - 会分配一个空闲的文件描述符（fd）。
    - 最终，这个文件描述符（fd）会被返回给发起请求的用户空间程序。

> [!note]
> _注意：_ 这个文件描述符（fd）代表了用户空间程序与这个特定 GPIO 事件监听通道的连接。用户程序可以通过对这个 fd 进行 `read()` 操作来获取发生的事件（`struct gpioevent_data`），并通过 `close()` 来释放该事件监听请求及其相关资源（包括那个 IRQ 线程）。


- 因此在 GxHal 接口中的封装接口会为每个使能 irq 的 gpio 都创建一个专属的线程，用于poll等待event发生