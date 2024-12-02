# Uboot 驱动开发流程



# 概念

- DM：

  - global_data:

    ```c
    typedef struct global_data {
    ...
    #ifdef CONFIG_DM
    	struct udevice	*dm_root;	/* Root instance for Driver Model */
    	struct udevice	*dm_root_f;	/* Pre-relocation root instance */
    	struct list_head uclass_root;	/* Head of core tree */
    #endif
    ...
    }
    ```

    - global_data：管理着整个 Uboot 的全局变量
      - dm_root：DM 模型的根设备
      - dm_root_f：重定向前的根设备
      - uclass_root：uclass 链表的头，会链接所有的 uclass

  - Uclass：一组以相同方式运行的设备。uclass 提供了一种访问组内各个设备的方法，但始终使用相同的接口。例如：GPIO uclass 提供获取、设置值的操作。

    - 声明:

      ```c
      UCLASS_DRIVER(demo) = {
          .id = UCLASS_DEMO,
      };
      
      struct uclass {
      	void *priv;
      	struct uclass_driver *uc_drv;
      	struct list_head dev_head;
      	struct list_head sibling_node;
      };
      ```

  - Uclass_driver:

  - Uclass_id:uclass 代表驱动的一个类别，uclass_driver 是 uclass 的驱动程序，为 uclass 提供统一操作接口。而对于不同类型的驱动，就需要 uclass_id 进行区分。意思就是说：每一种类型的设备 uclass 都有对应的 uclass_id，例如 spi 是一个 uclass，那么 显示屏、adc、flash 等都是不同的 uclass_id。

  - Device:绑定到特定端口或外设的驱动程序的实例

    ```c
    /**
     * struct udevice - An instance of a driver
     *
     * This holds information about a device, which is a driver bound to a
     * particular port or peripheral (essentially a driver instance).
     *
     */
    struct udevice {
    	const struct driver *driver;		//device 对应的driver
    	const char *name;					//device 的名称
    	void *platdata;
    	void *parent_platdata;
    	void *uclass_platdata;
    	ofnode node;						//设备树节点
    	ulong driver_data;
    	struct udevice *parent;				//父设备
    	void *priv;							// 私有数据的指针
    	struct uclass *uclass;				//驱动所属的uclass
    	void *uclass_priv;
    	void *parent_priv;
    	struct list_head uclass_node;
    	struct list_head child_head;
    	struct list_head sibling_node;
    	uint32_t flags;
    	int req_seq;
    	int seq;
    #ifdef CONFIG_DEVRES
    	struct list_head devres_head;
    #endif
    };
    ```

  - Driver：一些与外设对接并为其提供更高级别接口的代码

    ```c
    struct driver {
    	char *name;							//驱动名称
    	enum uclass_id id;					//驱动所对应的uclass_id	
    	const struct udevice_id *of_match;	//匹配函数
    	int (*bind)(struct udevice *dev);	//绑定函数
    	int (*probe)(struct udevice *dev);	//注册函数
    	int (*remove)(struct udevice *dev);
    	int (*unbind)(struct udevice *dev);
    	int (*ofdata_to_platdata)(struct udevice *dev);
    	int (*child_post_bind)(struct udevice *dev);
    	int (*child_pre_probe)(struct udevice *dev);
    	int (*child_post_remove)(struct udevice *dev);
    	int priv_auto_alloc_size;
    	int platdata_auto_alloc_size;
    	int per_child_auto_alloc_size;
    	int per_child_platdata_auto_alloc_size;
    	const void *ops;	/* driver-specific operations */
    	uint32_t flags;
    #if CONFIG_IS_ENABLED(ACPIGEN)
    	struct acpi_ops *acpi_ops;
    #endif
    };
    ```

- udevice 和 udriver 匹配：

  - 绑定阶段：根据设备树的字符串和 `U_BOOT_DRIVER()` 中的 `of_match` 表来查找每个节点的正确驱动程序。如果发现有相同的，则调用 `device_bind()` 创建一个新设备，初始化设备对象的每个核心字段。此时，所有设备都是已知的，并且绑定到它们的驱动程序，为所有设备分配了一个 `struct udevice` ，但是没有任何内容被激活。

  - 读取数据：从设备树中读数据到平台数据结构中，这会比 probe 先执行。

  - probe：执行 `device_probe()` 。先激活父设备，再调用设备的 probe 方法。

  - 运行阶段：该设备现在已经激活并开始使用。从现在开始直到它被删除为止，所有上述结构都是可以访问的。该设备出现在 uclass 的设备列表中。(如果是 UCLASS GPIO 的话，将显示为 GPIO uclass 中的设备)。

  - 移除阶段：当不再需要该设备时，可以调用 `device_remove()` 将其移除。

- udevice 和 uclass 的绑定：

  - udevice 内的 driver 下的 uclass_id，来与 uclass 对应的 uclass_driver 的 uclass_id 进行匹配

- uclass 和 uclass_driver 的绑定：

  - 已知 udevice 内的 driver 下的 uclass_id，创建 uclass 的同时，通过 uclass_id 找到对应的 uclass_driver 对象，然后将 uclass_driver 绑定到 uclass 上
  - uclass 是 uboot 自动生成的，并且不是所有的 uclass 都会生成，有对应 `uclass_driver` 并且有被 `udevice` 匹配到的 `uclass` 才会生成




![img](image/c0737f663229e3ed585ea36127a51566.png)



![img](image/20161119201742692.png)



# 代码

- 设备：设备树或者通过宏 `U_BOOT_DEVICE`

- 驱动：通过宏 `U_BOOT_DRIVER`

- 测试程序：cmd/demo.c

  - 调用位于 demo-uclass.c 中的接口

  ```c
  struct udevice *demo_dev;
  
  ret = uclass_get_device(UCLASS_DEMO, devnum, &demo_dev);
  
  return demo_hello(demo_dev, ch);
  ```

  

- demo-uclass.c:

  - 只是调用相关的驱动程序方法，具体的实现在 demo-simple.c

  ```c
  int demo_hello(struct udevice *dev, int ch)
  {
      const struct demo_ops *ops = device_get_ops(dev);
      
      if (!ops->hello)
          return -ENOSYS;
      
      return ops->hello(dev, ch);
  }
  ```

- demo-simple.c

  - 具体实现的驱动

  ```c
  static int simple_hello(struct udevice *dev, int ch)
  {
      const struct dm_demo_pdata *pdata = dev_get_plat(dev);
      
      printf("Hello from:....");
      
      return 0;
  }
  ```



### dm_init

DM 的初始化分为两个部分，一个是在 `relocate` 重定向之前的初始化：`initf_dm`，一个是在 `relocate` 重定向之前的初始化：`initr_dm`

```c
static int initf_dm(void)
{
#if defined(CONFIG_DM) && CONFIG_VAL(SYS_MALLOC_F_LEN)
	int ret;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");
	ret = dm_init_and_scan(true);					//这里为true
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);
	if (ret)
		return ret;
#endif
#ifdef CONFIG_TIMER_EARLY
	ret = dm_timer_init();
	if (ret)
		return ret;
#endif

	return 0;
}

static int initr_dm(void)
{
	int ret;

	/* Save the pre-reloc driver model and start a new one */
	gd->dm_root_f = gd->dm_root;
	gd->dm_root = NULL;
#ifdef CONFIG_TIMER
	gd->timer = NULL;
#endif
	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_scan(false);						//这里为false
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	if (ret)
		return ret;

	return 0;
}
```

- 两个均调用了 `dm_init_and_scan` 这个接口，这两个的关键区别在于参数的不同。

  - dts 节点中的 `u-boot,dm-pre-reloc` 属性，当设置了这个属性时，则表示这个设备在`relocate` 之前就需要使用
  - 当`dm_init_and_scan` 的参数为`true` 时，只会对带有`u-boot,dm-pre-reloc` 属性的节点进行解析。而当参数为`flase` 的时候，则会对所有节点进行解析





#### 创建 udevice、绑定 uclass和udriver

- `device_bind_by_name` 接口
  - 在所有的`struct driver`段中查找对应的 name
  - 找到 driver 之后根据 driver->id 找到对应的 uclass
  - malloc 一块内存用于存放 struct udevice
  - 将 udevice 的各个信息填充上，其中 `udevice->driver = 找到的 drv，udevice->uclass = 刚刚找到的 uclass`


#### uclass 和 device 绑定

- 接口 `uclass_bind_device` 绑定 uclass 和 udevice

- 把这个 udevice 链接到 uclass

  ```c
  int uclass_bind_device(struct udevice *dev)
  {
  	struct uclass *uc;
  	int ret;
  
  	uc = dev->uclass;
  	list_add_tail(&dev->uclass_node, &uc->dev_head);
  
  	if (dev->parent) {
  		struct uclass_driver *uc_drv = dev->parent->uclass->uc_drv;
  
  		if (uc_drv->child_post_bind) {
  			ret = uc_drv->child_post_bind(dev);
  			if (ret)
  				goto err;
  		}
  	}
  
  	return 0;
  err:
  	/* There is no need to undo the parent's post_bind call */
  	list_del(&dev->uclass_node);
  
  	return ret;
  }
  ```




#### udevice 和 udriver 绑定

- 根据传进来的 device，获取到对应的 driver(这一步在创建 udevice 设备的时候就指定了对应的 driver，而 driver 是通过 name 在段里面找到的)
- 执行一些 malloc、pre_probe、parent->probe 等等
- 执行 drv->probe





### dm_scan_platdata(老的做法)

#### lists_bind_drivers

- 找到所有的 `struct driver_info` 段
- 循环执行 `device_bind_by_name`
- 根据 name，找到对应的 struct driver
- 根据 drier  执行 `device_bind_common`
- 根据 struct driver 的 id 找到对应的 uclass
- calloc struct udevice 结构的空间
- 把name、driver、uclass 等都赋值给 struct udevice
- 执行 uclass_bind_device ，把 这个 udevice 链接到 uclass 上去

### dm_scan_fdt

#### dm_scan_fdt_node

- 遍历所有的设备树节点
- 如果没有 `u-boot,dm-pre-reloc` 宏就直接 continue
- 如果 `status` 不是 `okay` 也直接 continue
- 遍历所有的 struct driver
- driver_check_compatible
- 根据 struct driver->of_match->compatible 字符串
- fdt_node_check_compatible
- 从设备树中的 compatible 中找字符串是否和 `struct driver->of_match->compatible` 相同
- 相同则说明能够匹配，并返回一个 `struct udevice_id 的东西`
- 从 `fdt_get_name` 中拿到设备树中的 `name`
- `device_bind_with_driver_data`
- `device_bind_common`
- 现在有 `struct driver、dts->compatible`
- 根据 struct driver 的 id 找到对应的 uclass
- 如果这个 uclass 没有找到，那么就会新创建一个 uclass，根据 id 找到对应的 `struct uclass_driver`，然后 malloc `struct uclass` ，把 `struct uclass_driver` 和 `struct uclass` 链接上，并且执行 `struct uclass_driver->init`
- calloc struct udevice 结构的空间
- 把name、driver、uclass 等都赋值给 struct udevice
- 执行 uclass_bind_device ，把 这个 udevice 链接到 uclass 上去



![image-20220122134027436](image/aeaa1c89ee019fc929d3324e78826b12.png)



![](image/111.png)

# 实例
