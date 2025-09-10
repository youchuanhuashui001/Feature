

# Binding && Unbinding
- 解除绑定命令 `(unbind)` 调用删除设备驱动程序回调并将设备与其驱动程序解除绑定。
- `bind` 命令将设备绑定到其驱动程序。

为了调试目的，从 U-Boot 命令行绑定或取消绑定驱动程序应该很有用。
  - **绑定命令**：`bind <node path> <driver>` 或 `bind <class> <index> <driver>`
  - **解除绑定命令**：`unbind <node path>` 或 `unbind <class> <index>` 或 `unbind <class> <index> <driver>`
- **参数说明**
  - `<node path>`：节点的设备树路径。
  - `<class>`：由“dm uclass”命令或“dm tree”命令第一列给出的类之一。
  - `<index>`：父节点的索引（“dm tree”输出的第二列）。
  - `<driver>`：由“dm drivers”命令或“dm tree”输出的第四列给出的驱动程序名称。
- **使用示例**
  - 绑定：`bind usb_dev_generic 0 usb_ether`
  - 解除绑定：`unbind usb_dev_generic 0 usb_ether` 或 `unbind eth 1`
  - 绑定：`bind /ocp/omap_dwc3@48380000/usb@48390000 usb_ether`
  - 解除绑定：`unbind /ocp/omap_dwc3@48380000/usb@48390000`

# Debugging driver model

- **通用调试技巧**
    - 在开发新功能时，考虑在沙盒环境中进行，便于调试且可编写常见设备的模拟器。
    - 在文件顶部添加 `#define DEBUG`，激活该文件中的 `debug()` 和 `log_debug()` 语句。
    - 调整日志级别，如在 SPL 中设置 `CONFIG_SPL_LOG_MAX_LEVEL=7` 和 `CONFIG_LOG_DEFAULT_LEVEL=7`。
    - 启用 `CONFIG_LOG_ERROR_RETURN=y`，通过 `log_msg_ret()` 查看错误发生的具体位置。
    - 确保启用了调试 UART（`CONFIG_DEBUG_UART`），以便在串行驱动运行前获取串行输出。
    - 使用 JTAG 模拟器设置断点并单步执行代码。
- **设备无法定位问题**
    - 若 `uclass_first_device_err()` 返回错误，可参考 `linux/errno.h` 查看错误含义。常见错误包括：
        - `-ENOMEM`：表示内存不足。若发生在 SPL 或 U-Boot 重定位前，需检查 `CONFIG_SPL_SYS_MALLOC_F_LEN` 和 `CONFIG_SYS_MALLOC_F_LEN` 是否足够大，并可在 `malloc_simple.c` 顶部添加 `#define DEBUG` 查看内存使用情况。
        - `-EINVAL`：通常意味着设备树节点缺少或存在错误。检查设备树是否正确，并查看驱动中的 `of_to_plat()` 方法。
    - 若无错误，需确认设备是否已绑定。在定位设备前调用 `dm_dump_tree()`，确保设备存在。
    - 若设备不存在，检查设备树兼容性字符串是否与驱动期望的（`struct udevice_id` 数组中的）一致。
    - 若使用 `of-platdata`（如 `CONFIG_SPL_OF_PLATDATA`），需确保驱动名称与设备树中的第一个兼容性字符串相同（无效变量字符转换为下划线）。
    - 若问题仍未解决，可在 `drivers/core/lists.c` 顶部添加 `#define LOG_DEBUG`，查看具体发生了什么。



# Design Details

### Terminology（术语）
- **Uclass（类）**
    - 是一组以相同方式运行的设备。uclass 提供了一种访问组内各个设备的方法，但始终使用相同的接口。例如，GPIO uclass 提供获取/设置值的操作。I2C uclass 可能有 10 个 I2C 端口，其中 4 个带有一个驱动程序，6 个带有另一个驱动程序。
- **Driver（司机）**
    - 是一些与外围设备对话并向其提供更高级别接口的代码。
- **Device（设备）**
    - 是与特定端口或外围设备绑定的驱动程序实例。

> 单个 uclass 下个能有多个 driver：
> 	demo uclass：
> 		simple：只打印一条问候消息
> 		 shape：打印很丰富的信息




## **demo 命令执行流程：**

1.  **命令入口：** 命令 `demo` 位于 `cmd/demo.c` 中。
2.  **设备查找：**
    *   通过 `uclass_get_device(UCLASS_DEMO, devnum, &demo_dev)` 函数，根据设备号在 `UCLASS_DEMO` uclass 中查找设备。
    *   找到的设备会自动激活，可以直接使用。
3.  **调用 uclass 方法：**
    *   调用 `demo_hello(demo_dev, ch)` 函数，该函数位于 `drivers/demo/demo-uclass.c` 中。
    *   `demo_hello` 函数负责调用相关驱动程序的 "hello" 方法。
```
int demo_hello(struct udevice *dev, int ch)
{
        const struct demo_ops *ops = device_get_ops(dev);

        if (!ops->hello)
                return -ENOSYS;

        return ops->hello(dev, ch);
}
```
4.  **驱动程序执行：**
    *   驱动程序的 "hello" 方法被调用，例如 `drivers/demo/demo-simple.c` 中的 `simple_hello` 函数。
    *   `simple_hello` 函数通过 `dev_get_plat(dev)` 获取平台数据，并执行具体操作。

**关键点：**

*   `uclass_get_device()`：根据设备号获取 `UCLASS_DEMO` 中的设备。
*   `demo_hello()`：调用相关驱动程序的 "hello" 方法。
*   `device_get_ops()`：获取设备的操作函数集。
*   `dev_get_plat()`：获取设备的平台数据。




## **驱动程序声明**

*   **结构体 `demo_ops`：** 定义驱动程序的操作函数。
    *   示例：
        ```c
        static const struct demo_ops shape_ops = {
            .hello = shape_hello,
            .status = shape_status,
        };
        ```
*   **宏 `U_BOOT_DRIVER`：** 用于声明驱动程序。
    *   `name`：驱动程序名称。
    *   `id`：驱动程序所属的 uclass。
    *   `ops`：指向 `demo_ops` 结构体的指针，包含驱动程序的操作函数。
    *   `priv_data_size`：驱动程序私有数据的大小。
    *   示例：
        ```c
        U_BOOT_DRIVER(demo_shape_drv) = {
            .name   = "demo_shape_drv",
            .id     = UCLASS_DEMO,
            .ops    = &shape_ops,
            .priv_data_size = sizeof(struct shape_data),
        };
        ```
*   **`dev_get_priv(dev)`：** 用于访问驱动程序的私有数据。

**驱动程序方法**

*   `bind`：使驱动程序模型知道设备，将其绑定到驱动程序。
*   `unbind`：使驱动程序模型忘记设备。
*   `of_to_plat`：将设备树数据转换为平台数据（platform data）。
*   `probe`：使设备准备好使用。
*   `remove`：移除设备，直到再次探测时才能使用。

**设备工作流程**

1.  `bind`
2.  `of_to_plat` (如果使用设备树)
3.  `probe`

**要点**

*   `U_BOOT_DRIVER` 宏创建了一个可从 C 访问的数据结构，使驱动程序模型可以找到可用的驱动程序。
*   驱动程序方法在 `device.h` 头文件中定义。
*   `probe` 和 `remove` 方法是驱动程序中最常用的方法。




## **平台数据 (Platform Data)**

*   **定义：** 一种 C 结构，用于向驱动程序传递特定于平台的设置，例如寄存器地址、总线速度等。
*   **作用：** 提供启动设备所需的主板特定信息。
*   **目的：** 使驱动程序具有通用性，可以在具有相同类型设备的不同主板上运行。
*   **示例：** UART 驱动程序可以通过不同的平台数据实例化多次，每个平台数据提供 UART 实例的寄存器地址等信息。
*   **存储位置：**
    *   编译到 U-Boot 中的结构体。
    *   从设备树中解析。
*   **访问方式：** 驱动程序可以通过 `dev->info->plat` 访问平台数据。
*   **示例代码：**
    ```c
    static const struct dm_demo_pdata red_square = {
        .colour = "red",
        .sides = 4.
    };

    static const struct driver_info info[] = {
        {
            .name = "demo_shape_drv",
            .plat = &red_square,
        },
    };

    demo1 = driver_bind(root, &info[0]);
    ```
*   **缺点：** 相对不灵活，已被设备树取代。

## **设备树 (Device Tree)**

*   **定义：** 一种描述硬件的树状数据结构。
*   **优点：**
    *   更灵活，允许同一个板文件支持多种类型的板。
    *   可以重用 Linux 设备树，简化板级支持工作。
*   **替代方案：** 替代了在板文件中使用大量 `U_BOOT_DRVINFO()` 声明的方式。
*   **示例代码：**
    ```dts
    red-square {
        compatible = "demo-shape";
        colour = "red";
        sides = <4>;
    };
    ```
*   **驱动程序中的使用：**
    *   `.plat_auto = sizeof(struct dm_test_pdata)`：自动分配平台数据空间。
    *   `.of_to_plat = testfdt_of_to_plat`：解析设备树节点，并将数据存储到 `dev->plat` 中。
*   **方法调用顺序：** 激活期间先调用 `of_to_plat()`，然后调用 `probe()`。
*   **内存管理：**
    *   如果使用 `plat_auto`，则自动分配内存。
    *   如果不使用 `plat_auto`，则需要在 `of_to_plat` 或 `probe` 中使用 `malloc` 分配内存，并在 `remove` 中使用 `free` 释放内存。
*   **驱动模型树：** 驱动模型树旨在镜像设备树的结构。
*   **设备树有效性：** 设备树的内容必须符合设备树规范和设备树绑定。


### 设备树规范：https://www.devicetree.org/specifications/



## **声明 Uclass**

*   使用 `UCLASS_DRIVER` 宏声明 Uclass。
    ```c
    UCLASS_DRIVER(demo) = {
        .id             = UCLASS_DEMO,
    };
    ```
*   `id` 成员指定 Uclass 的 ID，定义在 `include/dm/uclass-id.h` 中。
*   可以为 Uclass 指定 `probe` 等特殊方法。
*   添加新的 Uclass 时，需要在 `include/dm/uclass-id.h` 的枚举末尾添加新的 ID。

### **设备序列号**

*   U-Boot 在许多情况下从 0 开始对设备进行编号，例如 I2C、SPI 总线和串口。
*   驱动模型支持按序列号定位设备。
*   同一 Uclass 中的设备序列号必须唯一。
*   序列号从 0 开始，但允许有间隙。
*   设备如何获取序列号由 `DM_SEQ_ALIAS` Kconfig 选项控制。
*   即使启用了 `CONFIG_DM_SEQ_ALIAS`，Uclass 仍然必须设置 `DM_UC_FLAG_SEQ_ALIAS` 标志，才能按别名对设备进行排序。
*   如果设置了 `CONFIG_DM_SEQ_ALIAS` 和 `DM_UC_FLAG_SEQ_ALIAS`，具有别名的设备将获得相应的序列号，其他设备将获得所有别名和现有编号之后的下一个可用编号。
*   如果未设置 `CONFIG_DM_SEQ_ALIAS` 或 `DM_UC_FLAG_SEQ_ALIAS`，所有设备都将以从 0 开始的简单顺序获得序列号。
*   设备序列号在设备绑定时解析，并且在设备的使用寿命内不会改变。
*   在某些情况下，Uclass 必须分配序列号，例如 PCI 总线。在这种情况下，可以设置 Uclass 的 `DM_UC_FLAG_NO_AUTO_SEQ` 标志。设置此标志后，只有具有别名的设备才会按驱动程序模型分配编号，其余部分留给 Uclass 来整理。

### **设备树中的别名**

*   可以使用设备树中的 `aliases` 节点为设备指定序列号。
    ```dts
    aliases {
        serial2 = "/serial@22230000";
    };
    ```
*   更常见的是使用节点引用：
    ```dts
    aliases {
        serial2 = &serial_2;
    };

    serial_2: serial@22230000 {
        ...
    };
    ```




## **总线驱动 (Bus Drivers)**

*   **定义：** 一种特殊的驱动程序，用于实现总线，提供对其他设备的访问。
*   **示例：** SPI、I2C、USB、SATA 等。
*   **作用：** 提供某种传输或转换，使与总线上的设备进行通信成为可能。

### **驱动模型对总线驱动的支持**

1.  **父数据 (Parent Data)：** 总线可以请求其子设备存储一些父数据，用于跟踪子设备的状态。
    *   通过 `dev->parent_plat` 访问。
    *   可以通过设置总线驱动或 Uclass 的 `per_child_plat_auto` 为非零值来自动分配。
2.  **子设备方法 (Child Methods)：** 总线可以定义在探测或移除子设备时调用的方法。
    *   `child_pre_probe()`：在子设备激活之前执行。
    *   `child_post_remove()`：在子设备停用之后执行。
3.  **子设备平台数据 (Per-Child Platform Data)：** 可以为每个子设备提供平台数据，用于指定子设备在总线上的地址等信息。
    *   在子设备的探测和移除周期中持续有效。

### **总线 Uclass**

*   可以指定每个子设备的平台数据，以便该 Uclass 中总线的所有子设备都可以使用相同的数据。
*   可以定义 `child_post_bind()` 方法，从设备树中获取每个子设备的平台数据并进行设置。
*   可以提供 `child_pre_probe()` 方法。
*   通常由总线 Uclass 控制这些功能，以避免每个驱动程序都进行相同的处理。

### **总线与 Uclass 的关系**

*   一个 USB 总线可以连接多个设备，每个设备来自不同的 Uclass。
*   总线设备需要存储每个设备的信息，例如地址和总线速度。
*   总线驱动控制子设备的总线方法，子设备可能不知道自己连接到总线。
*   同一个子设备可以在不同的总线类型上使用，或者不连接到任何总线。
*   设备的 Uclass 还可以包含该 Uclass 的私有数据，但这与总线上每个子设备的子数据无关。

### **示例**

```
xhci_usb (UCLASS_USB)
   flash (UCLASS_FLASH_STORAGE)  - parent data/methods defined by USB bus

sata (UCLASS_AHCI)
   flash (UCLASS_FLASH_STORAGE)  - parent data/methods defined by SATA bus

flash (UCLASS_FLASH_STORAGE)  - no parent data/methods (not on a bus)
```

**总结**

*   总线驱动是一种重要的驱动程序类型，用于实现总线，提供对其他设备的访问。
*   驱动模型提供了一些有用的功能来帮助实现总线驱动，例如父数据、子设备方法和子设备平台数据。
*   总线 Uclass 可以控制子设备的行为，以避免每个驱动程序都进行相同的处理。

Bind
Reading ofdata  of_to_plat
probe
running
remove
unbind




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
