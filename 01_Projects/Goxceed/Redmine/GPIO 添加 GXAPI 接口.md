#todo

- Virgo GXAPI 需要封出来 gpio 的用户态接口，不需要考虑 ecos，需要使用 gxhal 的方式，需要同时兼容 32、64 位(编译时用32、64位工具链编译测试都可用即可) 🛫 2025-9-15

- uart 需要在 loader 中打开 dlf 支持小数波特率
- 先执行board_init
- 再执行gx_gpio_init

- [ ] `GoXceed 5.0.0 GX_API` 接口将由 `Gx_HAL` 接口替代：
	https://git.nationalchip.com/redmine/projects/goxceed/wiki/GxHAL_%E7%BC%96%E7%A8%8B%E8%A7%84%E8%8C%83

# Userspace 用法

## libgpiod

## tools/gpio
- [[linux_gpio_driver]]
	- getio
	- setio
	- getlevel
	- setlevel
	- enable_trigger
	- disable_trigger

# GXHal
## 如何兼容多种操作系统
- 看下来好像只有 Linux_OS 和 NO_OS
- 如何区分 Linux_OS 和 NO_OS

## 操作 IOCTL 的时候需要用到不同的宏，这个宏是内核决定的，需要在 include 中套接一层？


## 获取电平和设置电平接口是否需要先将 GPIO 配置成输入和输出模式？

## 中断如何处理？
- 老的处理方式：
	- 打开设备
	- 执行 ioctl，配置gpio和edge
	- 设置一个信号，只设置一次，这个信号能够代表所有的 gpio
	- 设置回调函数
		- 回调函数可以获取到是什么信号，根据这个信号找到 gpio 对应的 callback 来执行
- 新的处理方式：
	- 打开设备
	- 执行 ioctl，配置 gpio和edge
	- 设置回调函数
	- 创建一个线程，每个 gpio 使能了都会注册一个专属的线程
		- 线程中执行 poll 操作，等待触发event
		- 触发event后通过read函数读取事件
		- 如果正常，则执行callback


## 通路
- hdmi
```
gxhal_hdmi.c
	GxHalStatus GxHal_HdmiInit(void)
		GxCore_DeviceOpen("/dev/gxav/hdmi0", O_RDWR|O_DSYNC);
			定义在 include 目录下的头文件中
		GX_HAL_AV_SET(_get_vout(), GxVoutPropertyID_OutputSelect, OutputSelect);
		
```

- 每个模块必须有 `GxHAL_ModuleGetCapability` 接口
	- 例如 hdmi：
```c
GxHalStatus GxHAL_AvGetProperty(GxHalHandle dev, unsigned long request, void *args, size_t size)
{
	GxAvIOCTL_PropertyGet param = { 0 };

	param.prop_id = request;
	param.prop_val = u64_cast(args);
	param.prop_size = size;

	if (GxHAL_AvIoctl(dev, GXAV_IOCTL_PROPERTY_GET, &param, sizeof(param)) >= 0) {
		return param.ret;
	}

	return GXHAL_ERR;
}

GxHalStatus GxHAL_HdmiGetCapability(GxHalHdmiCapability *capability)
{
	GxHalStatus ret = GXHAL_ERR;
	GxHalHdmiCapability cap = {0};

	CHECK_NULL(capability);

	ret = GXHAL_AV_GET(_get_hdmi(), GxAVGenericPropertyID_GetCapability, cap);
	CHECK_RET(ret);
	memcpy(capability, &cap, sizeof(GxHalHdmiCapability));

	return ret;
}

//                               hdmi_fd         GXAV_IOCTL_PROPERTY_GET   cap       sizeof(cap)
GxHalStatus GxHAL_AvGetProperty(GxHalHandle dev, unsigned long request, void *args, size_t size)
{
	GxAvIOCTL_PropertyGet param = { 0 };

	param.prop_id = request;           // GXAV_IOCTL_PROPERTY_GET
	param.prop_val = u64_cast(args);   // cap
	param.prop_size = size;            // sizeof(cap)

	if (GxHAL_AvIoctl(dev, GXAV_IOCTL_PROPERTY_GET, &param, sizeof(param)) >= 0) {
		return param.ret;
	}

	return GXHAL_ERR;
}

GxHalStatus GxHAL_AvIoctl(GxHalHandle dev, unsigned long request, void *args, size_t size)
{
	return GxCore_DeviceIoctl(dev, request, args, size);
}

int GxCore_DeviceIoctl(int fd, unsigned long request, ...)
{
	int result;
	va_list args;
	va_start(args, request);

	void *argp = va_arg(args, void *);
	unsigned int size = va_arg(args, unsigned int);

#if defined(TEE_OS) || defined(GXTEE_OS)
    // 在 TEE 环境下，期望获取 param 和 size 参数
    result = ioctl(fd, request, argp, size);
#else
    // 在普通环境下，只使用一个指针参数
    //     hdmi_fd   GXAV_IOCTL_PROPERTY_GET  
    result = ioctl(fd, request, argp);
#endif

	va_end(args);

	return result;
}
```


# 各驱动处理方式
## gxsecure 和 gxavdev 的处理方式
- 由 gxsecure 和 gxavdev 来处理各个操作系统的差异，gxhal 调用统一的接口，不需要区分操作系统
- 发布版本的时候更新库，中间过程有问题就打补丁，自己更新库
![[Pasted image 20250905153348.png]]


## Flash 处理方式
- 由 gxapi 来区分系统，gxmisc 中由于要走标准框架，所以必须各个操作系统实现一份，但最终都调用统一的接口
![[Pasted image 20250905153305.png]]


## GPIO 处理方式
- 由 gxapi 来区分系统，各个操作系统放一份 gpio 驱动代码
![[Pasted image 20250905153938.png]]

- GXAPI 接口放到 os 目录下，根据不同的操作系统来进行差异化处理，不开放出去
- GXHal 接口直接调用 GxAPI 接口