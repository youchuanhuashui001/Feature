# Linux 驱动中使用 platform_get_irq 返回的IRQ号与设备树中定义的IRQ号不一致

- dts 配置：中断号从32开始，中断号实际为 32 + 63 = 95

```text
interrupts = <GIC_SPI 63 IRQ_TYPE_LEVEL_HIGH>;
```

- 驱动：打印dws→irq = 0x1d

```C
dws->irq = platform_get_irq(pdev, 0);
```

- 由于`platform_get_irq()`返回Linux IRQ号(同类资源进行中心编号后的下标编号)，而不是实际的硬件号。

# Linux 驱动和设备树中中断相关的全部配置正确，但中断仍无法使用

- 可能是 uboot 阶段没有配置 中断，需要在uboot 中打开uboot 的总开关和 IRQ等等

### Linux 中 flash的驱动能够正常启动，但是没有mtd分区信息

- 由于uboot中传递的mtd信息不正确，kernel中flash的驱动会解析命令行信息，需要mtd名称和flash驱动名称相同才能正常启动mtd分区。

# Linux 内核根据根节点中的 compatible 属性来启动内核

## 无设备树：

- 无设备树时，uboot会向Linux内核传递一个叫做`machine id`的值，`machine id`也就是设备ID，告诉Linux内核自己是个什么设备，看看Linux内核是否支持。
- Linux 内核是支持很多设备的，针对每一个设备，Linux内核都用`MACHINE_START、MACHINE_END`来定义一个`machine_desc`结构体来描述这个设备。
- uboot就是向Linux 内核传递`machine id`这个参数，Linux 内核会检查这个，如果与`MACH_TYPE_XXX`匹配成功，就代表Linux内核支持这个设备，如果不支持的话就无法启动Linux内核。

## 有设备树：

- 当Linux内核引入设备树以后就不再使用`MACHINE_START`了，而是换为`DT_MACHINE_START`。
- 定义在文件`arch/arm/include/asm/arch.h`

```C
#define DT_MACHINE_START(_name, _namestr)
static const struct machine_desc __mach_desc_##_name
\
__used \
__attribute__((__section__(".arch.info.init"))) = { \
.nr
= ~0,
.name
\
= _namestr,
```

- 设备树方面：
  - 设备树通过在根节点的compatible属性值中定义开发板名称。
- 驱动方面：
  - 驱动在`arch/xxx/mach-xxx/mach-xxx.c`中的`DT_MACHINE_START`中定义`.dt_compat`值，来定义开发板名称。
- 只要某个设备(板子)根节点的`compatible`属性值与驱动中的任何一个值相等，那么就表示Linux内核支持此设备。

## Linux 内核如何根据设备树根节点的 compatible属性来匹配出对应的machine_desc？

- Linux内核调用`start_kernel`来启动内核，进而调用`setup_arch`函数来匹配`machine_desc`

![img](https://secure2.wostatic.cn/static/bzonV3qSZUvaCydgXD1686/image.png?auth_key=1675904067-nqYudvEqkwV47MYL1oTTbf-0-ccbbc13b4fdf0e1acd630368ec86bcb7)

# 设备树追加或修改节点内容

- 若需要在i2c节点下新增一个device，不能直接在`.dtsi`文件中新增，由于其它所有使用到这颗SOC的板子都会引用`.dtsi`，这就相当于其它所有的板子都有这个device，需要单独的对`.dts`文件进行数据追加。

```C
&i2c1 {
  /* 要追加或修改的内容 */
}；
```

- 通过`&label`的方式来访问节点，然后直接在里面编写要追加或者修改的内容。
- 需要在特定的`.dts`文件中修改，不能在`.dtsi`文件中修改。

# 设备树在系统中的体现

- Linux内核启动的时候会解析设备树中各个节点的信息，并且在根文件系统的`/proc/device-tree`目录下根据节点名字创建不同文件夹，如下所示：

  ![img](https://secure2.wostatic.cn/static/ecvPMJyU1VRYVXg4FzsjeN/image.png?auth_key=1675904067-5Ct8GcvaXVcK9JWSo2b98K-0-4a57715b300777afebf3dc74b7b3f026)

  - 根节点

    ```
    /
    ```

    各个属性：

    - 根节点属性表现为一个个的文件，其中包含设备树中定义的属性，可以通过`cat`命令查看，就是在设备树中定义的值。

  - 根节点

    ```
    /
    ```

    各个子节点：

    - 根节点的子节点就是进入根节点下的属性值，就能查看到子节点，同样是树形结构组织的。

      ![img](https://secure2.wostatic.cn/static/veXCuTJnXgFhFqJB2QgDPr/image.png?auth_key=1675904067-rQNfh3kJLECRn1ob9doC2c-0-6f4b790ff6a5f023ecf7eca91f73f93c)

  - 特殊节点：

    - aliases子节点：就是定义别名，后续方便访问节点

```text
aliases {
    can0 = &flexcan1;
    can1 = &flexcan2;
}
    - chosen子节点：主要是为了uboot向Linux内核传递参数，重点是`bootargs`参数。一般`.dts`文件中chosen节点的内容为空或者很少。
chosen {
    stdout-path = &uart1;
};
        - 可以在启动之后的`proc/device-tree/chosen`目录中，可以如下属性：

            ![](https://secure2.wostatic.cn/static/am6oE5mpgaTCeBEFUa5fML/image.png?auth_key=1675904067-hgMuzwn8DrSRDKfsorxh3N-0-84ef3357be8aadf686180e886039792c)

            - 可以看到多了一个 `bootargs`属性，但是在`chosen`节点中并没有配置`bootargs`属性。
                - 设备树中并没有设置，那么`bootargs`属性值是如何产生的？
                    - uboot会向`chosen`节点添加`bootargs`属性，并且设置了`bootargs`的属性值
int fdt_chosen(void *fdt)
{
        int   nodeoffset;
        int   err; 
        char  *str;             /* used to set string properties */

        err = fdt_check_header(fdt);
        if (err < 0) { 
                printf("fdt_chosen: %s\n", fdt_strerror(err));
                return err; 
        }    

        /* find or create "/chosen" node. */
        nodeoffset = fdt_find_or_add_subnode(fdt, 0, "chosen");
        if (nodeoffset < 0) 
                return nodeoffset;

        str = getenv("bootargs");
        if (str) {
                err = fdt_setprop(fdt, nodeoffset, "bootargs", str, 
                                ┆ strlen(str) + 1);
                if (err < 0) { 
                        printf("WARNING: could not set bootargs %s.\n",
                        ┆      fdt_strerror(err));
                        return err; 
                }    
        }    

        return fdt_fixup_stdout(fdt, nodeoffset);
}
                        ![](https://secure2.wostatic.cn/static/3eUbqn4M6mxFNg9C3wdfL9/image.png?auth_key=1675904067-bMPU88MUAyUKwwtj7P263S-0-d442acb832b6c8b1d3ebfa04dab0e913)
```

# Linux 内核解析DTB文件

- Linux内核在启动的时候会解析DTB文件，然后在`proc/device-tree`目录下生成相应的设备树节点文件。简单分析Linux内核是如何解析DTB文件的，`start_kernel`函数中完成了设备数节点解析的工作，最终实际工作的函数为`unflatten_dt_node`。流程如下所示：

![img](https://secure2.wostatic.cn/static/g9jf2qSR5tJKuGRUhf96Y1/image.png?auth_key=1675904067-eoMnB9U6nUtJahsh9nx7F1-0-680641960afd303388a17f093199e92b)

## 查找节点的OF函数

- 设备都是以节点的形式挂在设备树上的，因此要向获取这个设备的其它属性信息，必须先获取到这个设备的节点。
- LInux内核使用`device_node`结构体来描述一个节点，此结构体定义在文件`include/linux/of.h`中，定义如下：

```C
struct device_node {
    const char *name;
    const char *type;
    phandle phandle;
    const char *full_name;
    struct fwnode_handle fwnode;
    
    struct property *properties;
    struct property *deadprops;
    struct device_node *parent;
    struct device_node *child;
    struct device_node *sibling;
    struct kobject kobj;
    unsigned long _flags;
    void *data;
#if defined(CONFIG_SPARC)
    const char *path_component_name;
    unsigned int unique_id;
    struct of_irq_controller *irq_trans;
#endif
};
- 与查找节点有关的OF函数：
/*
 * 通过节点名字查找指定的节点
 * from：开始查找的节点，如果为NULL表示从根节点开始查找整个设备树
 * name：要查找的节点名称
 * 返回值：找到的节点，如果为NULL表示查找失败
 */
struct device_node *of_find_node_by_name(struct device_node *from, 
                                         const char *name);
                                         
                                         
/*
 * 根据 device_type 属性查找指定的节点
 * from：开始查找的节点，如果为NULL表示从根节点开始查找整个设备树
 * type：要查找的节点对应的type字符串，也就是device_type属性值
 * 返回值：找到的节点，如果为NULL表示查找失败
 */
struct device_node *of_find_node_by_type(struct device_node *from,
                                        const char *type)
 
 
 /*
  * 根据 device_type 和 compatible 这两个属性查找到指定的节点
  * from：开始查找的节点，如果为NULL表示从根节点开始查找整个设备树
  * type：要查找的节点对应的type字符串，也就是device_type属性值，可以为NULL，
  * 表示忽略 device_type 属性
  * compatible：要查找的节点所对应的compati属性列表
  * 返回值：找到的节点，如果为NULL表示查找失败
  */
struct device_node *of_find_compatible_node(struct device_node *from,
                                            const char *type,
                                            const char *compatible)
```

- 主要看 

  ```
  of_address_to_resource
  ```

  函数

  - IIC、SPI、GPIO等这些外设都有对应的寄存器，这些寄存器其实就是一组内存空间，Linux内核使用resource结构体来描述一段内存空间，因此用resource结构体描述的都是设备资源信息，resource结构体定义在文件`include/linux/ioport.h`中定义如下：

```C
struct resource {
    resource_size_t start;
    resource_size_t end;
    const char *name;
    unsigned long flags;
    struct resource *parent, *sibling, *child;
};
- 对于32位的SOC来说，resource_size_t是u32类型的。其中start表示开始地址，end表示结束地址，name是这个资源的名称，flags是资源标志位，一般表示资源类型(常用的是IORESOURCE_MED、IORESOURCE_REG、IORESOURCE_IRQ等)
```

# Linux 驱动的分层与分离

！！！！！！ 注意：这里的驱动分层与分离是控制器的驱动和设备信息(设备树)匹配！！！！！！

所谓的platform驱动并不是独立于字符设备驱动、块设备驱动和网络设备驱动之外其它种类驱动。 platform只是为了驱动的分层和分离而提出来的一种框架，其驱动的具体实现还是需要字符设备驱动、块设备驱动或网络设备驱动。

## 驱动的分离

```
    对于Linux这样一个成熟、庞大、复杂的操作系统，代码的重用性非常重要，否则的话就会在Linux内核中存在大量无意义的重复代码。尤其是驱动程序，因为驱动程序占用了Linux内核代码量的大头，如果不对驱动程序加以管理，任由重复的代码肆意增加，那么用不了多久Linux内核的文件数量就庞大到无法接受的地步。

    假如现在有三个平台A、B、C，这三个平台(三种SOC)上都有MPU6050这个I2C接口的六轴传感器，按照我们写逻辑I2C驱动的逻辑，每个平台都有一个MPU6050的驱动，因此编写出来的最简单的驱动框架如图所示：
```

![img](https://secure2.wostatic.cn/static/eQpTu6pw8jrsHLLzcgi9Ru/image.png?auth_key=1675904067-oAFp7JdivyZtmDjPFpxbCJ-0-440d0feb6d3b5cd3f3ae4471a73a4297)

```
    从上图看出来，每个平台下都有一个主机驱动和设备驱动，主机驱动肯定是必须要的，毕竟不同的平台其I2C控制器可能使用的不同。但是右侧的设备驱动就没必要每个平台都写一个，因为不管对于哪个SOC来说，MPU6050都是一样的，通过I2C接口读写数据就行了，只需要一个MPU6050的驱动即可。如果再来几个I2C设备，比如AT24C02、FT5206(电容触摸屏)等，按照上图的写法，那么设备端的驱动将会重复的编写好几次。显然在Linux驱动程序中这种写法是不推荐的，最好的做法就是每个平台的I2C控制器都提供一个统一的接口(也叫做主机驱动)，每个设备也都提供一个驱动程序(设备驱动)，每个设备通过统一的I2C接口驱动来访问，这样就可以大大简化驱动文件，如下所示：
```

![img](https://secure2.wostatic.cn/static/t2nRop3CWdVFokKHLhpRUY/image.png?auth_key=1675904067-wCe9B5Em5Yframbi7GDJrj-0-623f6ea5cb06c0290467b8db08b4b2be)

```
    实际的I2C驱动设备肯定有很多种，不止MPU6050这一种，那么实际的驱动框架如图所示：
```

![img](https://secure2.wostatic.cn/static/m1hjqBK3hVP5CbdYN7E1jD/image.png?auth_key=1675904067-ne674iwPRe2AUBM2SA51bL-0-3b14b97995837b64294eb66b514a3daa)

```
    这就完成了驱动的分隔，也就是将主机驱动和设备驱动分隔开来，比如I2C、SPI等等都会采用驱动分隔的方式来简化驱动的开发。在实际的驱动开发中，一般I2C主机控制器驱动由半导体厂家编写，而设备驱动有设备器件的厂家编写，用户只需要提供设备信息即可，比如I2C设备的话就提供设备连接到了那个I2C接口上，I2C的速度是多少等等。相当于将设备信息从设备驱动中剥离开来，驱动使用标准方法去获取到设备信息(比如从设备树中获取)，然后根据获取到的设备信息来初始化设备。这样就相当于驱动只负责驱动，设备只负责设备，想办法将两者进行匹配即可。这就是Linux中的总线(bus)、驱动(driver)和设备(device)模型，也就是常说的驱动分离。

    总线(统一接口API)就是驱动和设备信息的月老，负责给两者牵线搭桥，如图所示：
```

![img](https://secure2.wostatic.cn/static/4TqFoPm4jNFEKAWAf47Ztq/image.png?auth_key=1675904067-vu8wEAaFuGnUqdiPMxP3zm-0-4e9d7d18a68a2b6b1cdbb809118b69b5)

```
    当我们向系统注册一个驱动的时候，总系那就会在右侧的设备中查找，看看有没有与之匹配的设备，如果有的话就将两者联系起来。同样的，当向系统中注册一个设备的时候，总线就会在左侧的驱动中查找是否有与之匹配的设备。Linux内核中大量的驱动程序都采用总线、驱动和设备模式。
```

## 驱动的分层

```
    前面讲解设备驱动的分离，引出了总线、驱动和设备模型，比如I2C、SPI、USB等总线。但是在SOC中有些外设是没有总线这个概念的，但是又要使用总线、驱动和设备模型该怎么办呢？为了解决这个问题，Linux提出了platform这个虚拟总线，相应的就有`platform_driver`和`plarform_device`
```

### platform总线

```
Linux系统内核使用bus_type结构体表示总线，此结构体定义在文件`include/linux/device.h`，`bus_type`结构体内容如下：
struct bus_type {
        const char              *name;/* 总线名称 */
        const char              *dev_name;
        struct device           *dev_root;
        struct device_attribute *dev_attrs;     /* use dev_groups instead */
        const struct attribute_group **bus_groups;/* 总线属性 */
        const struct attribute_group **dev_groups;/* 设备属性 */
        const struct attribute_group **drv_groups;/* 驱动属性 */

        int (*match)(struct device *dev, struct device_driver *drv);
        int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
        int (*probe)(struct device *dev);
        int (*remove)(struct device *dev);
        void (*shutdown)(struct device *dev);

        int (*online)(struct device *dev);
        int (*offline)(struct device *dev);

        int (*suspend)(struct device *dev, pm_message_t state);
        int (*resume)(struct device *dev);

        const struct dev_pm_ops *pm; 

        const struct iommu_ops *iommu_ops;

        struct subsys_private *p;
        struct lock_class_key lock_key;
};
match 函数用来完成设备和驱动之间匹配的，总线就是使用match函数来根据注册的设备来查找对应的驱动，或者根据注册的驱动来查找对应的设备，因此每一条总线都必须实现此函数。match函数有两个参数：dev和drv，这两个参数分别为device和device_driver类型，也就是设备和驱动。

platform总线是bus_type的一个具体事例，定义在`driver/base/platform.c`，platform总线定义如下：
struct bus_type platform_bus_type = {                                                                          
        .name           = "platform",
        .dev_groups     = platform_dev_groups,
        .match          = platform_match,
        .uevent         = platform_uevent,
        .pm             = &platform_dev_pm_ops,
};

static int platform_match(struct device *dev, struct device_driver *drv)
{
        struct platform_device *pdev = to_platform_device(dev);
        struct platform_driver *pdrv = to_platform_driver(drv);

        /* When driver_override is set, only bind to the matching driver */
        if (pdev->driver_override)
                return !strcmp(pdev->driver_override, drv->name);

        /* Attempt an OF style match first */
        if (of_driver_match_device(dev, drv))
                return 1;

        /* Then try ACPI style match */
        if (acpi_driver_match_device(dev, drv))
                return 1;

        /* Then try to match against the id table */
        if (pdrv->id_table)
                return platform_match_id(pdrv->id_table, pdev) != NULL;

        /* fall-back to driver name match */
        return (strcmp(pdev->name, drv->name) == 0);
}
```

- 驱动和设备的匹配有四种方法：
  - OF类型的匹配，也就是设备树采用的匹配方式，`of_driver_match_device`函数定义在文件`include/linux/of_device.h`中。`device_driver`结构体(表示设备驱动)中有个名为`of_match_table`的成员变量，此成员变量保存着驱动的`compatible`驱动表，设备树中的每个设备节点的`compatible`属性会和`of_match_table`表中的所有成员比较，查看是否有相同的名称。如果有的话就表示设备和此驱动匹配，设备和驱动匹配成功以后probe函数就会执行。
  - ACPI匹配方式
  - id_table匹配，每个`platform_driver`结构体有一个`id_table`成员变量，保存了很多id信息。这些id信息存放着这个platform驱动所支持的驱动类型。
  - 如果第三种匹配方式的id_table不存在的话就直接比较驱动和设备的name字段，查看是否相等，如果相等则匹配成功。
  - 对于支持设备树的Linux版本，一般设备驱动为了兼容性都支持设备数和无设备树两种匹配方式。也就是第一种匹配方式一般都会存在，第三种和第四种只要存在一种就可以，一般用的最多的还是第四种，也就是直接比较驱动和设备的name字段。

### platform驱动

```
    platform_driver结构体表示platform驱动，此结构体定义在文件`include/linux/platform_device.h`中，内容如下：
struct platform_driver {
        int (*probe)(struct platform_device *); 
        int (*remove)(struct platform_device *); 
        void (*shutdown)(struct platform_device *);                                                            
        int (*suspend)(struct platform_device *, pm_message_t state);
        int (*resume)(struct platform_device *); 
        struct device_driver driver;
        const struct platform_device_id *id_table;
        bool prevent_deferred_probe;
};
```

- probe函数，当驱动和设备匹配成功以后probe函数就会执行。一般驱动的提供者会编写，如果自己要编写一个全新的驱动，那么probe就需要自行实现。
- driver成员，为`device_driver`结构体变量，Linux内核里面大量使用到了面向对象的思想，device_driver相当于基类，提供了最基础的驱动框架。`platform_driver`继承了这个基类，然后在此基础上有添加了一些特有的成员变量。
- id_table表，也就是platform总线匹配驱动和设备的时候采用的的三种方法， id_table是个表，每个元素的类型为platform_device_id.
- device_driver结构体定义在`include/linux/device.h`中，device_driver结构体内容如下：

```C
struct device_driver {
        const char              *name;
        struct bus_type         *bus;

        struct module           *owner;
        const char              *mod_name;      /* used for built-in modules */

        bool suppress_bind_attrs;       /* disables bind/unbind via sysfs */
        enum probe_type probe_type;

        const struct of_device_id       *of_match_table;
        const struct acpi_device_id     *acpi_match_table;

        int (*probe) (struct device *dev);
        int (*remove) (struct device *dev);
        void (*shutdown) (struct device *dev);
        int (*suspend) (struct device *dev, pm_message_t state);
        int (*resume) (struct device *dev);
        const struct attribute_group **groups;

        const struct dev_pm_ops *pm;

        struct driver_private *p;
};
- id_match_table就是采用设备树的时候驱动使用的匹配表，同样是数组，每个驱动向都为`of_device_id`结构体类型。
- 通过设备节点的compatible属性值和of_match_table中每个成员的compatible成员变量进行比较，如果有相等的就表示设备和此驱动匹配成功。

        在编写platform驱动的时候，首先定义一个`platform_driver`结构体变量，然后实现结构体中的各个成员变量，重点是实现匹配方法以及probe函数。当驱动盒设备匹配成功以后probe函数就会执行，具体的驱动程序在probe函数里面编写。

    当定义并初始化好`platform_driver`结构体变量以后，需要在驱动入口函数里面调用`platform_driver_register`函数向Linux内核注册一个platform驱动，platform_driver_register函数原型如下：
int platform_driver_register(struct platform_driver *driver)
- platform驱动框架如下所示：
struct xxx_dev{
    struct cdev cdev;
};

struct xxx_dev xxxxdev;

static int xxx_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t xxx_write(struct file *filp, const char __user *buf,
                          size_t cnt, loff_t *offt)
{
    return 0;
}

static int xxx_probe(struct platform_device *dev)
{
    .......
    cdev_init(&xxxdev.cdev, &xxx_fops);
    return 0;
}

static int xxx_remove(struct platform_device *dev)
{
    ......
    cdev_del(&xxxdev.cdev);
    return 0;
}

static struct platform_driver xxx_driver = {
    .driver = {
        .name = "xxx",
        .of_match_table = xxx_of_match,
    },
    .probe = xxx_probe,
    .remove = xxx_remove,
};

static int __init xxxdriver_init(void)
{
    return platform_driver_register(&xxx_driver);
}

static void __exit xxxdriver_exit(void)
{
    platform_driver_unregister(&xxx_driver);
}

module_init(xxxdriver_init);
module_exit(xxxdriver_exit);
```

### platform设备

```
    platform驱动已经准备好了，还需要platform设备。如果内核支持设备树的话就不要在使用`platform_device`描述设备了，改用设备树描述就可以了。
```

- 使用`platform_device`描述设备信息：`platform_device`结构体定义在`include/linux/platform_device.h`中，结构体内容如下：

```C
struct platform_device {
        const char      *name;
        int             id;
        bool            id_auto;
        struct device   dev;
        u32             num_resources;
        struct resource *resource;

        const struct platform_device_id *id_entry;
        char *driver_override; /* Driver name to force a match */

        /* MFD cell pointer */
        struct mfd_cell *mfd_cell;

        /* arch specific additions */
        struct pdev_archdata    archdata;
};
- name：设备名称，要和所用的platform驱动的name字段相同，否则的话设备就无法匹配到对应的驱动。
- num_resources：资源数量，一般是指resource资源的大小
- resource：资源，表示设备信息，比如外设寄存器等。
```

- 不支持设备树的Linux版本中，用户需要编写`platform_device`变量来描述设备信息，然后使用`platform_device_register`函数将设备信息注册到Linux内核中。如果不再使用platform的话可以通过platform_device_unregister函数注销掉相应的platform设备。
- platform设备信息框架如下所示：

```C
#define PERIPH1_REGISTER_BASE  (0x20000000)
#define PERIPH2_REGISTER_BASE  (0x020E0068)
#define REGISTER_LENGTH 4

static struct resource xxx_resources[] = {
    [0] = {
        .start = PERIPH1_REGISTER_BASE,
        .end = (PERIPH1_REGISTER_BASE = REGISTER_LENGTH - 1),
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = PERIPH2_REGISTER_BASE,
        .end = (PERIPH2_REGISTER_BASE = REGISTER_LENGTH - 1),
        .flags = IORESOURCE_MEM,
    },
}

static struct platform_device xxxdevice = {
    .name = "xxx-gpio",
    .id = -1,
    .name_resources = ARRAY_SIZE(xxx_resources),
    .resource = xxx_resources,
};

static int __init xxxdevice_init(void)
{
    return platform_deice_register(&xxxdevice);
}

static void __exit xxx_resourcedevice_exit(void)
{
    platform_device_unregister(&xxxdevice);
}

module_init(xxxdevice_init);
module_exit(xxxdevice_exit);
```

上述代码是在不支持设备树的Linux版本中使用的，当Linux内核支持了设备树以后就不需要用户手动去注册platform设备了。因为设备信息都放到了设备树中去描述，Linux内核启动的时候会从设备数中读取设备信息，然后将其组织称platform_device形式。

### platform设备与驱动程序编写

#### paltform设备文件：leddevice.c

按照platform设备框架编写驱动

#### platform驱动文件：leddriver.c

按照platform驱动框架编写驱动

加载设备模块、驱动模块，两个模块加载成功以后platform总线就会进行匹配，当驱动和设备匹配成功以后就会执行probe函数。

## 设备树下的platform驱动编写

### 设备树下的platform驱动简介

```
    platform驱动框架分为总线、设备和驱动，其中总线有Linux内核提供，在编写驱动时只需要关注设备和驱动的具体实现即可。在没有设备树的Linux内核下，需要分别编写并注册platform_device和platform_driver，分别代表设备和驱动。在使用设备树的时候，设备的描述被放到了设备树中，因此`platform_device`就不需要手动编写了，只需要实现`platform_driver`即可。
```

#### 1. 在设备树中创建设备节点

```
    首先需要在设备树中创建设备节点来描述设备信息，重点是要设置好compatible属性的值，因为platform总线需要通过设备节点的compatible属性值来匹配驱动。例如：
gpioled {
    #address-cells = <1>;
    #size-cells = <1>;
    compatible = "atkalpha-gpioled";
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_led>;
    led-gpio = <&gpio1 3 GPIO_ACTIVE_LOW>;
    status = "okay";
};
```

#### 2. 编写platform驱动

```
    在使用设备树的时候platform驱动会通过`of_match_table`来保存兼容性值，也就是表明此驱动兼容哪些设备。所以，`of_match_table`将会尤为重要。
static const struct of_device_id leds_of_match[] = {
    {.compatible = "atkalpha-gpioled"},
    {}
};

static struct platform_driver leds_platform_driver = {
    .driver = {
        .name = "imx6ul-led",
        .of_match_table = leds_of_match,
    },
    .probe = leds_probe,
    .remove = les_remove,
};

static int __init leddriver_init(void)
{
    return platform_driver_register(&led_driver);
}

static int __exit leddriver_exit(void)
{
    return platform_driver_unregister(&led_driver);
}

module_init(leddriver_init);
module_exit(leddriver_exit);
```

将上面的platform_driver.c编译成ko，然后加载。 驱动模块加载完成以后到/sys/bus/platform/driver/目录下查看驱动是否存在。在/sys/bus/platform/devices/目录下也存在led的设备文件。 当驱动和设备匹配成功以后就会执行probe函数。

### Linux 自带的led驱动

```C
#define module_platform_driver(__platform_driver) \
        module_driver(__platform_driver, platform_driver_register, \
        platform_driver_unregister)


#define module_driver(__driver, __register, __unregister, ...) \
static int __init __driver##_init(void) \
{ \
return __register(&(__driver) , ##__VA_ARGS__); \
} \
module_init(__driver##_init); \
static void __exit __driver##_exit(void) \
{ \
__unregister(&(__driver) , ##__VA_ARGS__); \
} \
module_exit(__driver##_exit);

/* 上面采用 module_platform_driver 注册驱动，相当于是下面的方式注册驱动 */

static int __init gpio_led_driver_init(void)
{
    return platform_driver_register (&(gpio_led_driver));
}
module_init(gpio_led_driver_init);

static void __exit gpio_led_driver_exit(void)
{
    platform_driver_unregister (&(gpio_led_driver) );
}
module_exit(gpio_led_driver_exit);
```

# Linux SPI驱动框架

```
    对于SPI的大框架，主要分为两层，控制器驱动程序层(spi_master)，主要提供transfer函数，进行SPI协议的数据手法。spi_master基于platform模型，注册spi_master时会扫描一个链表进行注册设备。(就是上面Linux 驱动的分层与分离中描述 device采用设备树，driver采用platform_driver的方式)
```

另一层是设备驱动层，基于`spi_bus_type`，需要提供名称、片选、最大速率、模式、中断号等等。在driver里则使用spi_read、spi_write等函数，最终也会调用到master->transfer函数进行数据手法。

## 控制器层：

- 设备：采用设备树来描述或者使用platform_device结构描述
- 驱动：采用platform_driver结构来进行描述，通过与设备在platform总线层进行匹配，匹配完成后执行probe函数。
- 如何与设备层交互：
  - 在platform_driver中会调用`spi_register_master` —> `scan_boardinfo` —> 扫描 board_list ，取出每一个 boardinfo ，比对，如果 boardinfo 里的 bus_num 和 master 的 bus_num 相等，则认为这个spi设备在硬件物理连接上是接到这个控制器的，则使用 spi_new_device 创建 spi 设备

```C
int spi_register_master(struct spi_master *master)
{
    .....
    device_add(&master->dev);
    
    spi_match_master_to_boardinfo(master, &bi->board_info);
}

static void spi_match_master_to_boardinfo(struct spi_master *master,
                                struct spi_board_info *bi) 
{
        struct spi_device *dev;

        if (master->bus_num != bi->bus_num)
                return;

        dev = spi_new_device(master, bi);                                                                                                                             
        if (!dev)
                dev_err(master->dev.parent, "can't create new device for %s\n",
                        bi->modalias);
}


/* 如果 board_info 提供的bus_num 和 master—>bus_num 一致，则调用 spi_new_device  */
```

## 设备层

### device

```
- 分配一个`spi_board_info`结构体
- 设置`spi_board_info`中的名称、最大频率、控制器编号、模式、片选
- 注册`spi_register_board_info`
```

### driver:

```
- 分配一个`spi_driver`结构
- 设置名称、probe函数
- 注册`spi_register_driver`函数
- 使用`spi_write`等系统调用，使用 `spi_transfer spi_message`收发数据
```

# Uboot DM分析

## 1. 描述u-boot驱动模型的数据结构

```
u-boot有一个功能强大的驱动模型，这一点与Linux内核一致。驱动模型对设备驱动相关操作做了一个抽象：使用uclass来描述设备类，使用driver来描述驱动，使用udevice来描述设备。
```

uclass是一个类，udevice是一个设备，driver是udevice的驱动。一个类下面的udevice连接到uclass上，driver是对应的udevice的具体的驱动。一个类可以有多个udevice。

### uclass

- uclass表示以相同特征方式运行的一组devicc。
- uclass提供一种以相同接口方式访问组内单个设备的方式。
- 该结构由`struct uclass`表示`(include/dm/uclass.h)`

```C
struct uclass {
        void *priv;// private data for this uclass
        struct uclass_driver *uc_drv;// 对应的uclass driver
        struct list_head dev_head;//该类中的设备列表（当设备的绑定方法被调用时，它们会被附加到它们的类上）。
        struct list_head sibling_node;//链表节点，用于把uclass连接到uclass_root链表上，类链表中的下一个类
}; 
```

- uclass driver：`spi-uclass.c`用于连接到uc_drv

### driver

- 用于提供与外设交互的高级接口
- 连接到`udevice→driver`

```C
struct driver {
        char *name;//device name
        enum uclass_id id;//该驱动属于哪个uclass                                                                                                                                                                                                      
        const struct udevice_id *of_match;//要匹配的兼容字符串列表
        int (*bind)(struct udevice *dev);//调用该函数将设备绑定到其驱动程序
        int (*probe)(struct udevice *dev);//用于探测设备，即激活设备
        int (*remove)(struct udevice *dev);//用于移除一个设备
        int (*unbind)(struct udevice *dev);//调用该函数来接触设备与驱动程序的绑定
        int (*ofdata_to_platdata)(struct udevice *dev);//在probe之前调用该函数以解析设备树
        int (*child_post_bind)(struct udevice *dev);//在一个新的子设备被绑定后调用
        int (*child_pre_probe)(struct udevice *dev);//在一个新的子设备被probe之前调用
        int (*child_post_remove)(struct udevice *dev);//在子设备被移除后调用
        int priv_auto_alloc_size;//如果非零，这是在设备的->priv指针中分配的私有数据大小
        int platdata_auto_alloc_size;//设备->platdata中分配的平台数据的大小，通常对设备树有效
        int per_child_auto_alloc_size;
        int per_child_platdata_auto_alloc_size;
        const void *ops;//由驱动定义的函数指针列表，实现uclass所需要的驱动函数 */
        uint32_t flags;//驱动标志
};
```

### udevice

- 与特定端口或外围设备绑定的驱动程序实例
- 连接到uclass中的 `dev_head`

```C
struct udevice {
        const struct driver *driver;
        const char *name;
        void *platdata;
        void *parent_platdata;
        void *uclass_platdata;
        int of_offset;
        ulong driver_data;
        struct udevice *parent;
        void *priv;
        struct uclass *uclass;
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

## 2. 声明驱动

```
分析u-boot/drivers目录下的文件，驱动程序声明一般具有如下类似的结构`(drivers/demo/demo-shape.c)`
static const struct demo_ops shape_ops = {
    .hello = shape_hello,
    .status = shape_status,
};

U_BOOT_DRIVER(demp_shape_drv) = {
    .name = "demo_shape_drv",
    .id   = UCLASS_DEMO,
    .ops  = &shape_ops,
    .priv_data_size = sizeof(struct shape_data),
};
```

- 上述代码所示：

  - 首先创建xxx_ops结构，该结构与具体的设备驱动有关
  - 然后使用`U_BOOT_DRIVER`宏将其声明为u-boot驱动
  - 在`U_BOOT_DRIVER`中还可以指定用于绑定和解绑定的方法，这些方法在适当的时候被调用。对于大多数驱动程序来说，一般只会使用到probe和remove方法

- 在u-boot中，让一个设备工作的顺序是：

  ![img](https://secure2.wostatic.cn/static/soWiHHSvSZ6fRXgPkQQYp9/image.png)

- `U_BOOT_DRIVER`宏创建了一个可从c访问的数据结构，因此驱动模型可以找到可用的驱动程序

## 3. U_BOOT_DRIVER宏分析

`U_BOOT_DRIVER`宏定义在`include/dm/device.h`文件宏：通过`__attribute__`的方式将所有的`U_BOOT_DRIVER`存放在`.u_boot_list_2_driver`这样单独的段中

```C
/* Declare a new U-Boot driver */
#define U_BOOT_DRIVER(__name)      \
 ll_entry_declare(struct driver, __name, driver)
 
 #define ll_entry_declare(_type, _name, _list)    \
 _type _u_boot_list_2_##_list##_2_##_name __aligned(4)  \
   __attribute__((unused,    \
   section(".u_boot_list_2_"#_list"_2_"#_name)))
    许多这样的`U_BOOT_DRIVER`声明出来的驱动会形成一张表(实质为数组),u-boot会使用这个表
```

- u-boot提供一些宏接口来访问这些定义出来的东西

```C
#define ll_entry_start(_type, _list)     \
({         \
 static char start[0] __aligned(4) __attribute__((unused, \
  section(".u_boot_list_2_"#_list"_1")));   \
 (_type *)&start;      \
})


#define ll_entry_end(_type, _list)     \
({         \
 static char end[0] __aligned(4) __attribute__((unused,  \
  section(".u_boot_list_2_"#_list"_3")));   \
 (_type *)&end;       \
})

#define ll_entry_count(_type, _list)     \
 ({        \
  _type *start = ll_entry_start(_type, _list);  \
  _type *end = ll_entry_end(_type, _list);  \
  unsigned int _ll_result = end - start;   \
  _ll_result;      \
 })
```

- 则实际上所有的驱动会以以下的形式组成：

  ![img](https://secure2.wostatic.cn/static/9BVwafdN2Q9t2M8fQmdATf/image.png)

## 4. udevice

```
    u-boot中用`struct udevice`来描述一个具体设备。该数据结构保存着一个设备的具体信息，这个设备是一个绑定到一个特定端口或外设(本质上是一个驱动实例)的驱动。
struct udevice {
        const struct driver *driver;//此设备使用的驱动程序
        const char *name;//设备名称，通常为FDT节点的名称
        void *platdata;//这个设备的配置数据
        void *parent_platdata;//父总线对此设备的配置数据
        void *uclass_platdata;//该uclass设备的配置数据
        int of_offset;//在设备树节点中的偏移量
        ulong driver_data;//驱动数据字，用于匹配该设备的驱动
        struct udevice *parent;//此设备的父设备，当为NULL时表示此设备为顶级设备
        void *priv;//私有数据
        struct uclass *uclass;//指向该设备所属类的指针
        void *uclass_priv;//uclass为该设备提供的私有数据
        void *parent_priv;//父设备为此设备提供的私有数据
        struct list_head uclass_node;//用于uclass链接到此设备
        struct list_head child_head;//设备子节点链表
        struct list_head sibling_node;//设备列表中的下一个设备
        uint32_t flags;//设备的标志
        int req_seq;//标志请求此设备的序列号
        int seq;//为该设备分配的虚列报。该只是在设备被探测时设置，并且在设备的类中是唯一的
#ifdef CONFIG_DEVRES
        struct list_head devres_head;
#endif
};
```

- 一个设备通过调用bind产生，分为两种方式：
  1. 由`U_BOOT_DEVICE`宏
  2. 通过设备树中的节点产生
- 如果是通过设备树中节点产生，则会将设备树信息转化为平台数据，通过驱动程序将数据转化为平台的实现方法(如果设备有设备树节点，则会在probe函数之前调用)
- 在驱动模型相关数据结构创建过程中，`platdata/priv/uclass_priv`可以由驱动分配，也可以使用`struct driver`和`struct uclass_driver`的`auto_alloc_size`成员来让驱动模型自动完成分配操作

## 5. 驱动模型分析

```
    u-boot启动会从`board_init_f`到`board_init_r`。这里只分析`board_init_r`中的`initr_dm()`函数。
#ifdef CONFIG_DM
static int initr_dm(void)
{
        int ret; 

        /* Save the pre-reloc driver model and start a new one */
        gd->dm_root_f = gd->dm_root;
        gd->dm_root = NULL;
#ifdef CONFIG_TIMER
        gd->timer = NULL;
#endif                                                                                                                                                                                                                          
        // 用于初始化驱动模型的结构并扫描设备
        ret = dm_init_and_scan(false);
        if (ret)
                return ret; 
#ifdef CONFIG_TIMER_EARLY
        ret = dm_timer_init();
        if (ret)
                return ret; 
#endif

        return 0;
}
#endif
```

- `dm_init_and_scan`

```C
int dm_init_and_scan(bool pre_reloc_only)
{
        int ret;

        ret = dm_init();
        if (ret) {
                debug("dm_init() failed: %d\n", ret);
                return ret;
        }   
        ret = dm_scan_platdata(pre_reloc_only);
        if (ret) {
                debug("dm_scan_platdata() failed: %d\n", ret);
                return ret;
        }   

        if (CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)) {
                ret = dm_scan_fdt(gd->fdt_blob, pre_reloc_only);
                if (ret) {
                        debug("dm_scan_fdt() failed: %d\n", ret);
                        return ret;
                }   
        }   

        ret = dm_scan_other(pre_reloc_only);
        if (ret)
                return ret;

        return 0;
}

//在u-boot的驱动模型中，驱动和类是以树形来构建和管理的，树根定义如下：
/* This is the root driver - all drivers are children of this */
U_BOOT_DRIVER(root_driver) = { 
        .name   = "root_driver",
        .id     = UCLASS_ROOT,
        .priv_auto_alloc_size = sizeof(struct root_priv),
};

/* This is the root uclass */
UCLASS_DRIVER(root) = {
        .name   = "root",
        .id     = UCLASS_ROOT,
};   

// 在dm_init_and_scan()函数中会初始化树和类似树的根，然后扫描并绑定来自平台数据和FDT的可用设备。
// 调用dm_init()来建立驱动模型结构
```

- `dm_init`

```C
int dm_init(void)
{
 int ret;
 
 //判断dm_root是否存在，如果存在则会返回。
 if (gd->dm_root) {
  dm_warn("Virtual root driver already exists!\n");
  return -EINVAL;
 }
  
  //初始化((gd_t *)gd)->uclass_root链表。
 INIT_LIST_HEAD(&DM_UCLASS_ROOT_NON_CONST);

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
 fix_drivers();
 fix_uclass();
#endif

//创建设备并绑定驱动程序。
 ret = device_bind_by_name(NULL, false, &root_info, &DM_ROOT_NON_CONST);
 if (ret)
  return ret;
#if CONFIG_IS_ENABLED(OF_CONTROL)
 DM_ROOT_NON_CONST->of_offset = 0;
#endif
//探测（probe）一个设备，并激活它
 ret = device_probe(DM_ROOT_NON_CONST);
 if (ret)
  return ret;

 return 0;
}
```

- ```
  dm_scan_platdata
  ```

  - u-boot有两种方式描述设备：
    - 平台数据
    - 设备树
  - 此函数会扫描所有的平台数据并绑定驱动程序，扫描所有可用的平台数据并未每个数据创建驱动程序

```C
int dm_scan_platdata(bool pre_reloc_only)
{
 int ret;
 //搜索并将所有驱动程序绑定到父驱动程序
 ret = lists_bind_drivers(DM_ROOT_NON_CONST, pre_reloc_only);
 if (ret == -ENOENT) {
  dm_warn("Some drivers were not found\n");
  ret = 0;
 }

 return ret;
}
```

- ```
  dm_scan_fdt
  ```

  - 如果u-boot配置支持设备树，就会执行此函数
  - 次函数用于扫描设备书，绑定驱动程序。浙江扫描设备书并未每个节点创建一个驱动程序(只检查顶级子节点)

```C
int dm_scan_fdt_node(struct udevice *parent, const void *blob, int offset,
       bool pre_reloc_only)
{
 int ret = 0, err;

// 通过for循环从第一个子节点开始遍历到最后一个子节点
 for (offset = fdt_first_subnode(blob, offset);
      offset > 0;
      offset = fdt_next_subnode(blob, offset)) {
  if (pre_reloc_only &&
      !fdt_getprop(blob, offset, "u-boot,dm-pre-reloc", NULL))
   continue;
   
   // status = "okay"
  if (!fdtdec_get_is_enabled(blob, offset)) {
   dm_dbg("   - ignoring disabled device\n");
   continue;
  }
  
  err = lists_bind_fdt(parent, blob, offset, NULL);
  if (err && !ret) {
   ret = err;
   debug("%s: ret=%d\n", fdt_get_name(blob, offset, NULL),
         ret);
  }
 }

 if (ret)
  dm_warn("Some drivers failed to bind\n");

 return ret;
}

int dm_scan_fdt(const void *blob, bool pre_reloc_only)
{
 return dm_scan_fdt_node(gd->dm_root, blob, 0, pre_reloc_only);
}
```

- `lists_bind_fdt`

```C
int lists_bind_fdt(struct udevice *parent, const void *blob, int offset,
                ┆  struct udevice **devp)
{
        struct driver *driver = ll_entry_start(struct driver, driver);
        const int n_ents = ll_entry_count(struct driver, driver);
        const struct udevice_id *id;
        struct driver *entry;
        struct udevice *dev;
        bool found = false;
        const char *name;
        int result = 0;
        int ret = 0;

        dm_dbg("bind node %s\n", fdt_get_name(blob, offset, NULL));
        if (devp)
                *devp = NULL;
        // 查找所有使用 U_BOOT_DRIVER 宏定义的 struct driver
        // 使用 U_BOOT_DRIVER 中的 compatible 和 dts中的 compatible 进行匹配
        // blod：device tree point
        for (entry = driver; entry != driver + n_ents; entry++) {
                ret = driver_check_compatible(blob, offset, entry->of_match,
                                        ┆     &id);
                name = fdt_get_name(blob, offset, NULL);
                if (ret == -ENOENT) {
                        continue;
                } else if (ret == -ENODEV) {
                        dm_dbg("Device '%s' has no compatible string\n", name);
                        break;
                } else if (ret) {
                        dm_warn("Device tree error at offset %d\n", offset);
                        result = ret;
                        break;
                }   

                dm_dbg("   - found match at '%s'\n", entry->name);
                ret = device_bind_with_driver_data(parent, entry, name,
                                                ┆  id->data, offset, &dev);
                if (ret == -ENODEV) {
                        dm_dbg("Driver '%s' refuses to bind\n", entry->name);
                        continue;
                }   
                if (ret) {
                        dm_warn("Error binding driver '%s': %d\n", entry->name,
                                ret);
                        return ret;
                } else {
                        found = true;
                        if (devp)
                                *devp = dev;
                }   
                break;         
        }

        if (!found && !result && ret != -ENODEV) {
                dm_dbg("No match for node '%s'\n",
                ┆      fdt_get_name(blob, offset, NULL));
        }

        return result;
}
                
```

- `driver_check_compatible`

```C
/**
 * driver_check_compatible() - Check if a driver is compatible with this node
 *
 * @param blob:         Device tree pointer
 * @param offset:       Offset of node in device tree
 * @param of_match:     List of compatible strings to match
 * @param of_idp:       Returns the match that was found
 * @return 0 if there is a match, -ENOENT if no match, -ENODEV if the node
 * does not have a compatible string, other error <0 if there is a device
 * tree error
 */
static int driver_check_compatible(const void *blob, int offset,
                                ┆  const struct udevice_id *of_match,
                                ┆  const struct udevice_id **of_idp)
{
        int ret;

        *of_idp = NULL;
        if (!of_match)
                return -ENOENT;

        while (of_match->compatible) {
                ret = fdt_node_check_compatible(blob, offset,
                                                of_match->compatible);
                if (!ret) {
                        *of_idp = of_match;
                        return 0;
                } else if (ret == -FDT_ERR_NOTFOUND) {
                        return -ENODEV;
                } else if (ret < 0) {
                        return -EINVAL;
                }   
                of_match++;
        }   

        return -ENOENT;
}

int fdt_node_check_compatible(const void *fdt, int nodeoffset,
                        ┆     const char *compatible)
{
        const void *prop;
        int len;

        // 将设备树中该节点的 所有 compatible 字符串返回回来
        prop = fdt_getprop(fdt, nodeoffset, "compatible", &len);
        if (!prop)
                return len;
        // 将保存下来的 compatible 字符串与 U_BOOT_DRIVER.compatible 进行字符串比较
        if (fdt_stringlist_contains(prop, len, compatible))                                                                                                                                                                     
                return 0;
        else
                return 1;
}

int fdt_stringlist_contains(const char *strlist, int listlen, const char *str)                                                                                                                                                  
{               
        int len = strlen(str);
        const char *p; 

        while (listlen >= len) {
                if (memcmp(str, strlist, len+1) == 0)
                        return 1; 
                p = memchr(strlist, '\0', listlen);
                if (!p)
                        return 0; /* malformed strlist.. */
                listlen -= (p-strlist) + 1;
                strlist = p + 1;
        }
        return 0;
}

// !!!! 到这里只是将设备树中的 compatibe 和 U_BOOT_DRIVER 中的 compatible 匹配上，还未绑定
```

- `device_bind_with_driver_data`

```C
int device_bind_with_driver_data(struct udevice *parent,                                                                                                                                                                        
                                ┆const struct driver *drv, const char *name,
                                ┆ulong driver_data, int of_offset,
                                ┆struct udevice **devp)
{
        return device_bind_common(parent, drv, name, NULL, driver_data,
                                ┆ of_offset, 0, devp);
}


// 创建 device，通过 struct udevice **devp 带回去
// 将 device 连接到 uclass 链表上
// 执行一些 auto_malloc
// 执行一些  bind 函数，如果定义了的话
static int device_bind_common(struct udevice *parent, const struct driver *drv,
                        ┆     const char *name, void *platdata,
                        ┆     ulong driver_data, int of_offset,
                        ┆     uint of_platdata_size, struct udevice **devp)
{
        struct udevice *dev;
        struct uclass *uc;
        int size, ret = 0;

        if (devp)
                *devp = NULL;
        if (!name)
                return -EINVAL;

        // 根据 drv->id 找到对应的 uclass 类别
        ret = uclass_get(drv->id, &uc);
        if (ret) {
                debug("Missing uclass for driver %s\n", drv->name);
                return ret;
        }

        dev = calloc(1, sizeof(struct udevice));
        if (!dev)
                return -ENOMEM;

        INIT_LIST_HEAD(&dev->sibling_node);
        INIT_LIST_HEAD(&dev->child_head);
        INIT_LIST_HEAD(&dev->uclass_node);
#ifdef CONFIG_DEVRES
        INIT_LIST_HEAD(&dev->devres_head);
#endif
        dev->platdata = platdata;
        dev->driver_data = driver_data;
        dev->name = name;
        dev->of_offset = of_offset;
        dev->parent = parent;
        dev->driver = drv;
        dev->uclass = uc;

        dev->seq = -1;
        dev->req_seq = -1;
        if (CONFIG_IS_ENABLED(OF_CONTROL) && CONFIG_IS_ENABLED(DM_SEQ_ALIAS)) {
                /*
                ┆* Some devices, such as a SPI bus, I2C bus and serial ports
                ┆* are numbered using aliases.
                ┆*
                ┆* This is just a 'requested' sequence, and will be
                ┆* resolved (and ->seq updated) when the device is probed.  
                ┆*/
                if (uc->uc_drv->flags & DM_UC_FLAG_SEQ_ALIAS) {
                        if (uc->uc_drv->name && of_offset != -1) {
                                fdtdec_get_alias_seq(gd->fdt_blob,
                                                uc->uc_drv->name, of_offset,
                                                &dev->req_seq);
                        }
                }
        }

        // 处理 platdata，如果在U_BOOT_DRIVER宏中定义存在platdata的话，就分配一块空间
        if (drv->platdata_auto_alloc_size) {
                bool alloc = !platdata;

                if (CONFIG_IS_ENABLED(OF_PLATDATA)) {
                        if (of_platdata_size) {
                                dev->flags |= DM_FLAG_OF_PLATDATA;
                                if (of_platdata_size <
                                                drv->platdata_auto_alloc_size)
                                        alloc = true;
                        }
                }
                if (alloc) {
                        dev->flags |= DM_FLAG_ALLOC_PDATA;
                        dev->platdata = calloc(1,
                                        ┆      drv->platdata_auto_alloc_size);
                        if (!dev->platdata) {
                                ret = -ENOMEM;
                                goto fail_alloc1;
                        }
                        if (CONFIG_IS_ENABLED(OF_PLATDATA) && platdata) {
                                memcpy(dev->platdata, platdata,
                                ┆      of_platdata_size);
                        }
                }
        }

        // 如果UCLASS_DRIVER宏中定义了per_device_platdata_auto_alloc_size，就malloc一块空间
        size = uc->uc_drv->per_device_platdata_auto_alloc_size;
        if (size) {
                dev->flags |= DM_FLAG_ALLOC_UCLASS_PDATA;
                dev->uclass_platdata = calloc(1, size);
                if (!dev->uclass_platdata) {
                        ret = -ENOMEM;
                        goto fail_alloc2;
                }
        }
        // parent：gd->dm_root
        if (parent) {
                size = parent->driver->per_child_platdata_auto_alloc_size;
                if (!size) {
                        size = parent->uclass->uc_drv->
                                        per_child_platdata_auto_alloc_size;
                }
                if (size) {
                        dev->flags |= DM_FLAG_ALLOC_PARENT_PDATA;
                        dev->parent_platdata = calloc(1, size);
                        if (!dev->parent_platdata) {
                                ret = -ENOMEM;
                                goto fail_alloc3;
                        }
                }
        }

        /* put dev into parent's successor list */
        // 所有的dt都链接起来
        if (parent)
                list_add_tail(&dev->sibling_node, &parent->child_head);

        // uclass 和 device 进行绑定，其实就是将 udevice 连接到uclass的链表上
        ret = uclass_bind_device(dev);
        if (ret)
                goto fail_uclass_bind;

        /* if we fail to bind we remove device from successors and free it */
        // 执行 U_BOOT_DRIVER 中的 bind 函数
        if (drv->bind) {
                ret = drv->bind(dev);
                if (ret)
                        goto fail_bind;
        }
        if (parent && parent->driver->child_post_bind) {
                ret = parent->driver->child_post_bind(dev);
                if (ret)
                        goto fail_child_post_bind;
        }
        if (uc->uc_drv->post_bind) {
                ret = uc->uc_drv->post_bind(dev);
                if (ret)
                        goto fail_uclass_post_bind;
        }

        if (parent)
                dm_dbg("Bound device %s to %s\n", dev->name, parent->name);
        // 通过二级指针，将dev带回去
        if (devp)
                *devp = dev;
                                                                                                                                                                                                                                
        dev->flags |= DM_FLAG_BOUND;

        return 0;

fail_uclass_post_bind:
        /* There is no child unbind() method, so no clean-up required */
fail_child_post_bind:
        if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
                if (drv->unbind && drv->unbind(dev)) {
                        dm_warn("unbind() method failed on dev '%s' on error path\n",
                                dev->name);
                }
        }

fail_bind:
        if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
                if (uclass_unbind_device(dev)) {
                        dm_warn("Failed to unbind dev '%s' on error path\n",
                                dev->name);
                }
        }
fail_uclass_bind:
        if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
                list_del(&dev->sibling_node);
                if (dev->flags & DM_FLAG_ALLOC_PARENT_PDATA) {
                        free(dev->parent_platdata);
                        dev->parent_platdata = NULL;
                }
        }
fail_alloc3:
        if (dev->flags & DM_FLAG_ALLOC_UCLASS_PDATA) {
                free(dev->uclass_platdata);
                dev->uclass_platdata = NULL;
        }
fail_alloc2:
        if (dev->flags & DM_FLAG_ALLOC_PDATA) {
                free(dev->platdata);
                dev->platdata = NULL;
        }
fail_alloc1:
        devres_release_all(dev);

        free(dev);

        return ret;
}
```

int device_bind_with_driver_data(struct udevice *parent,

┆const struct driver *drv, const char *name,

┆ulong driver_data, int of_offset,

┆struct udevice **devp)

{

return device_bind_common(parent, drv, name, NULL, driver_data,

```
┆ of_offset, 0, devp);
```

}

- `uclass_bind_device`

```C
int uclass_bind_device(struct udevice *dev)
{
        struct uclass *uc;
        int ret;

        uc = dev->uclass;
        // 把 udevice 连接到对应的 uclass上
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





# 交叉编译 minicom

## 移植 ncurses

- 下载`ncurses`，并进行解压

- 配置 `ncurses`

  - configure是配置脚本
  - --prefix指定编译结果的保存目录
  - --host指定编译器前缀
  - --target用于指定目标

  ```
  ./configure --prefix=/home/zuozhongkai/linux/IMX6ULL/tool/ncurses --host=arm-linux-gnueabihf --target=arm-linux-gnueabihf --with-shared --without-profile --disable-stripping --without-progs --with-manpages --without-tests
  ```

- make

- make install：将编译出来的结果拷贝到`--prefix`指定的目录

- 拷贝`include、lib、share`下的文件都拷贝到开发板，注意有些动态库拷贝的时候要保持链接

  - ```shell
    sudo cp lib/* /home/zuozhongkai/linux/nfs/rootfs/usr/lib/ -rfa
    sudo cp share/* /home/zuozhongkai/linux/nfs/rootfs/usr/share/ -rfa
    sudo cp include/* /home/zuozhongkai/linux/nfs/rootfs/usr/include/ -rfa
    ```

- 开发板的`etc/profile`目录用于指定链接库目录路径

## 移植 minicom

- 下载minicom，并进行解压
- 配置minicom
  - configure是配置脚本
  - CC指定要使用的gcc交叉编译器
  - --prefix指定编译出来的文件存放目录
  - --host指定交叉编译器前缀
  - CPPFLAGS指定ncurses的头文件路径
  - LDFLAGS指定ncurses的库文件路径

- make
- make install：将编译出来的结果拷贝到`--prefix`指定的目录
- 将--prefix目录下的bin目录中的文件都拷贝到开发板的`usr/bin`

- 开发板中新建`etc/passwd`文件，并输入以下内容

  `root:x:0:0:root:/root:/bin/sh`

- 重启开发板



一旦传输开始，读该寄存器的值就是已经读从源外设数据项总数





# Flush_cache and Invalidata_cache

- flush_dcache_all
  - 相当于是把`cache`中的数据回写到内存中
  - dma 读取 flash 的时候：数据流为从外设的fifo到内存，cpu看到的是cache中的数据，dma直接看到的是内存中的数据，所以此时要刷cache，同步cache和内存的数据，避免cache中的数据丢失
- invalidata_dcache_range
  - 相当于是先无效`cache`，下次操作的时候再从内存读取数据到`cache`
  - dma 写到 flash 的时候：数据流为从内存到外设的fifo，cpu看到的是cache中的数据，dma看到的是内存中的数据，此时要无效cache，直接从内存中拿数据到 flash



# SPI nand flash 坏块标记

- 新片子中第一次启动，会先读取所有block的第一个page的oob区域，判断是否为0xff
  - 若不为0xff，则表明该块为坏块，并进行标记，标记完成后将其写入到最后的四个block
- 第二次启动的时候，会读最后四个block中的第一个page的main区域，看是否有坏块标记头，若最后的四个block都是坏的，则重新扫描所有block的第一个page的oob区域，在内存中构建坏块表，并且以后每次使用都是重新扫描一遍，并构建在内存中使用坏块表。



# Linux 的根文件系统和 jffs2 有什么区别？

- Linux 的根文件系统是 Linux 操作系统中的一个基础概念，它是操作系统在启动时所需要的最基本的文件系统，包含了一些必要的文件和目录，如/bin、/sbin、/etc 等，以及内核模块和驱动程序等
- Jffs2 是一种针对闪存设计的文件系统，主要用于嵌入式系统中。与传统的文件系统不同，Jffs 是一种基于日志的文件系统，它可以很好地支持山存储器的特性，如快速擦除、读写等。
- 在 Linux 系统中，根文件系统可以使用多种文件系统格式，如`Ext2、Ext3、Ext4、XFS`等。`Jffs2`可以作为一种可选的文件系统格式，用于嵌入式系统中。通常，嵌入式系统中的根文件系统会使用一种特定的文件系统格式，以满足该系统的需求。例如，一些嵌入式系统可能会使用`SquashFS`作为跟文件系统，而另一些系统坑你会使用`Jffs2`。
- 总之，Linux 的根文件系统和JFFS2是两个不同的概念，它们有着不同的用途和特点，但在嵌入式系统中，JFFS2可以作为一种可选的文件系统格式，用于实现根文件系统。





# Linux 的根文件系统使用 Jffs2，并且只修改一个一个文件中的一个字符

- 如果在Linux系统中使用 `Jffs2` 文件格式作为根文件系统，那么对文件的读写操作实际上是对闪存继续读写操作。
- 在JFFS2中，文件系统的元数据和文件数据是分开存储的，因此对文件的读写操作实际上是对闪存储器中的元数据和文件数据进行读写操作。
- 在使用JFFS2文件系统格式作为根文件系统时，文件系统会被加载到内存中，而不是被挂载在硬盘等其他存储介质上。因此，对文件的读写操作实际上是在内存中进行的，而不是在硬盘上进行的。当文件被修改后，系统会将修改的数据写回到闪存储器中，从而实现文件的持久化存储。

- 如果只修改了一个文件中的一个字符，对于闪存存储器来说，需要进行以下操作：

  1. 读取文件所在的页（Page）到内存缓冲区中；
  2. 修改缓冲区中对应位置的字符；
  3. 将修改后的缓冲区数据写回到闪存存储器中；
  4. 更新文件系统元数据，标记文件所在的页已经被修改过，这样在读取文件时可以直接读取缓存中的数据。

  > 在进行步骤3时，JFFS2文件系统不会直接覆盖闪存存储器中原有的数据，而是将修改后的数据写入一个新的页中，然后将新的页链接到原有的页链表中。这样做的目的是为了避免频繁地在同一个物理地址上进行擦除和写入操作，从而减少闪存存储器的磨损和延长使用寿命。



# ECC 磨损测试

1. 随机选取一个block
2. 如果这个block已经是坏块的话，就用下一个block，循环到找到一个非坏块
3. 做一些随机数，大小为page_size
4. 擦除整个block
5. 读取这个block的每个page，查看是否为0xff
6. 将随机数写入这个block的每个page
7. 读取每个page，比较是否与写入的随机数不同
8. 有的page会出现5bit的翻转或者7bit的翻转

具体操作过程：

1. 对整个块进行擦除
2. 逐个对 64 个数据页执行以下操作：
   1. 写入随机生成的数据
   2. 读出数据并计算比特翻转次数
   3. 判断比特翻转次数是否等于 0，若不是则说明该页发生了比特翻转
   4. 若发生了比特翻转，则记录最小和最大比特翻转次数，用于后续比较和输出
   5. 比较读出的数据和写入的数据，计算比特翻转次数
   6. 判断比特翻转次数是否等于 0，若不是则说明该页发生了比特翻转
   7. 若发生了比特翻转，则记录最小和最大比特翻转次数，用于后续比较和输出
3. 输出每轮测试的结果

该程序的目的是通过 ECC 机制的校验结果来判断 NAND Flash 的质量和稳定性，以便决定是否适合用于产品。





# 多线程系统的优先级倒置

:::tips
线程的优先级倒置问题是在多线程操作系统中可能出现的一种现象，其中一个优先级较低的线程占用了一个优先级较高的线程所需的共享资源，导致优先级较高的线程无法及时执行。

这个问题通常发生在具有不同优先级的线程之间共享某些资源的情况下。当一个优先级较低的线程持有某个资源时，一个优先级较高的线程需要等待该资源的释放才能执行。如果持有资源的线程长时间不释放，高优先级线程可能会一直被阻塞，从而导致优先级倒置。

以下是一个简化的示例，说明了线程优先级倒置问题：

假设有三个线程：高优先级线程 A、中优先级线程 B 和低优先级线程 C，它们需要共享一个关键资源 R。

1. 线程 A 被分配了最高的优先级，它需要使用资源 R 进行关键计算。
2. 线程 B 被分配了较低的优先级，它也需要使用资源 R 进行一些计算。
3. 线程 C 被分配了最低的优先级，它也需要使用资源 R，但是它的任务相对较轻。

由于线程 C 的优先级较低，当线程 A 和线程 B 同时竞争资源 R 时，线程 C 可能会被线程 B 长时间阻塞，从而导致线程 A 也无法及时执行，造成优先级倒置。这种情况下，高优先级线程 A 受到低优先级线程 C 的干扰，无法按照预期的高优先级立即执行。

为解决线程优先级倒置问题，可以采取以下措施：

1.  使用合适的调度策略：操作系统可以采用适当的调度算法，如抢占式调度，以确保高优先级线程能够及时获得执行机会。 
2.  优先级继承/优先级反转：引入优先级继承或优先级反转机制来解决优先级倒置问题。优先级继承将低优先级线程的优先级提升为与其竞争资源的高优先级线程相同，以确保资源的及时访问。优先级反转是一种类似的概念，但它只在需要时临时提升低优先级线程的优先级。 
3.  合理设计线程间通信和同步机制：通过使用适当的线程间通信和同步机制，如信号量、互斥 
    :::


# 自旋锁：Spinlock

:::tips
自旋锁（Spin Lock）是一种同步机制，用于在多线程环境中保护共享资源的互斥访问。与互斥锁（Mutex Lock）不同，自旋锁采用忙等待的方式，即线程在获取锁之前会一直循环检查锁的状态，直到成功获取锁为止，而不是将线程阻塞挂起。

自旋锁的工作原理如下：

1. 当一个线程需要访问被自旋锁保护的共享资源时，它尝试获取自旋锁。
2. 如果自旋锁处于空闲状态，线程会立即获取锁，并进入临界区执行相应的操作。
3. 如果自旋锁已经被其他线程占用，当前线程会进入自旋状态，不断循环检查锁的状态。
4. 当占用自旋锁的线程释放锁时，其他线程会在循环中检测到自旋锁的状态变为可用，然后立即获取锁并进入临界区。

自旋锁的优点是对于锁的持有时间较短的情况下，可以避免线程上下文切换所带来的开销，因为线程会一直循环等待，不会主动放弃 CPU 的执行权。因此，自旋锁适用于对共享资源的访问时间较短且竞争不激烈的情况。

然而，自旋锁也存在一些缺点。如果线程在自旋等待期间一直无法获取锁，会导致浪费CPU资源。因此，在竞争激烈或临界区执行时间较长的情况下，使用自旋锁可能会降低系统的性能。此外，自旋锁在单核处理器上并不适用，因为只有一个核心，无法进行真正的并行执行。

需要注意的是，自旋锁只能在多核系统或支持超线程技术的系统上发挥作用，因为需要多个执行上下文来实现并行执行和自旋等待。
:::

- 不应在单处理器系统上使用自旋锁。
  - 考虑一个高优先级线程，它试图申请一个已被低优先级线程持有的自旋锁：它将永远循环，低优先级线程永远不会再有机会运行和释放自旋锁。即使这两个线程以相同的优先级运行，试图申请自旋锁的线程也会自旋直到它被时间片化，这样就会浪费大量的 CPU 时间。如果中断处理程序试图申请线程拥有的自旋锁，中断处理程序将永远循环。因此，自旋锁仅适用于自旋锁的当前所有者可以继续在不同处理器上运行的 SMP 系统。

# 串行 NAND OOB 区域排布

- 举例一款 华邦 串行 NAND 手册描述的 OOB 区域：
  - ![选区_119.png](https://cdn.nlark.com/yuque/0/2023/png/36177780/1686880014922-e24267ea-aa95-4688-a502-44a3de5006c5.png#averageHue=%23f2f1f0&clientId=u058fc5c5-9f07-4&from=drop&id=ucb316232&originHeight=855&originWidth=790&originalType=binary&ratio=1&rotation=0&showTitle=false&size=187786&status=done&style=none&taskId=uf745ff3b-0212-43d5-9c75-eb6d974c20b&title=)
  - 其中描述了 page 包括 2048的数据区+64的oob区域
    - oob 区域包括四个 spare 区域，每个 spare区域 16 字节
    - 每 16 个字节的数据分布如下：前2个字节用于坏块表及，2~3字节用于用户，但没有ecc保护；4~7可以提供给用户使用，且包含ecc保护，8~D 字节用于存放主区域0 512字节的ecc码；E~F字节用于存放oob区域的4~D字节的ecc码
    - 总结一下：用户可以用的 oob 区域，每个 spare 区域只有 User Data I 的 4个字节
- 使用 scrub 擦除，会把坏块表擦掉，但是会不会擦掉 oob 区域呢？什么操作会擦掉 oob 区域呢？
  - 会擦掉所有的 oob 区域，然后重新构建坏块表

- flash 的擦除操作会把数据区和 oob 区同时擦除
- nand singletest 的时候如果不指定数据区，只指定 oob 区，则相当于在操作某一个page的oob区域，那么擦除操作的时候必须要擦掉这个page的oob区域，否则下一次进行 singletest 的时候，相当于 oob 区域没擦就再写了，或者再读了
- gx_spinand_read_data_and_oob：接口的时候，会指定 数据区的地址和长度，oob区域的地址和长度，然后模式，这个数据区的地址和长度相当于指定了所在的 block 和 page，oob 区域也就是对应的 page
  - 测试的时候指定：`flash nandsingletest 0x0 0x0 0x0 oob_size mode`这个操作不行，每次擦除的时候相当于没擦，然后下一次测试 oob 区域的时候就相当于没擦就写了
  - 换成`flash nandsingletest 0x300000 len 0x0 oob_size mode`这个操作，相当于找到一个 block 、page，然后 数据区域不操作，oob 区域操作，擦除的时候是根据 起始地址 + 长度来擦的，如果主数据区域的长度配成0的话，那擦除操作永远是不会进行的



# SPI Nand 坏块表

- 坏块表
  - g_spinand_chip.bbt_options |= NAND_BBT_USE_FLASH |NAND_BBT_NO_OOB;
    - 将 BBT 放置在  Flash 中
    - BBT 标记不放置在 OOB 区域
  - spinand_default_bbt(&g_spinand_chip)
    - 如果chip->bbt_td 没有配置的话，那么就配置 chip->bbt_td和 chip->bbt_md，并且根据是否有 NAND_BBT_NO_OOB 标记来选择是否将 OOB 标记放置在 OOB 区域
      - chip->bbt_td：配置坏块表的标志
        - .options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP | NAND_BBT_NO_OOB
        - .len = 4
        - .veroffs = 4
        - maxblocks = 4
        - .pattern = "B' 'b' 't' '0'
      - chip->bbt_md：配置镜像表的标志
        - .options 跟主表一致
        - .len = 4
        - .veroffs = 4
        - .maxblocks = 4
        - .pattern = '1' 't' 'b' 'B'
      - nand_create_badblock_pattern(chip):构建 struct nand_bbt_desrc 结构，并将其挂载到 chip 结构中去
        - bd->options = chip->bbt_options & BADBLOCK_SCAN_MASK
        - bd->offs = 0
        - bd->len = 1
        - bd->pattern = scan_ff_pattern (0xff 0xff)
        - bd->options |- NAND_BBT_DYNAMICSTRUCT
        - chip->badblock_pattern = bd
      - nand_scan_bbt(chip, chip->badblock_pattern) --> 相当于传了chip.和刚刚创建的 struct nand_bbt_desrc 结构
        - 计算长度：每个block占2bit，malloc 这么长的空间用来存放 bbt(chip->bbt)
        - 判断 chip->bbt_td 是否存在，若不存在的话就调用 nand_memory_bbt(chip, bd)
          - nand_memory_bbt(chip, bd)
        - 校验 主表描述信息和镜像表描述信息：verify_bbt_desrc(chip, td) verify_bbt_desrc(chip,md)
          - verify_bbt_desrc(chip, td)：
        - malloc 一个临时的buffer，大小是 block_size
        - 如果bbt在给定的page页，也就是有标志 NAND_BBT_ABSPAGE，就调用 read_abs_bbts
          - read_abs_bbts(chip, buf, td, md)
        - 否则调用 search_read_bbts(chip, buf, td, md)：使用 oob 中的模式搜索坏块表
          - search_read_bbts(chip, buf, td, md)：buf是刚刚malloc的，大小是block_size，td和md是主坏块表和镜像坏块表
            - search_bbt(chip, buf, td)：
              - 根据 NAND_BBT_LASTBLOCK 宏来确定查找的起始 block 和 方向，计算是否有多个 chip，目前是只有一个，然后循环
              - 重置版本信息，包括 主表的 td->version td->pages[0] ，扫描td->maxblocks也就是四个block
              - 读取每个block的第一个page，scan_read(chip, buf, offs, chip->info->page_main_size, td)
                - scan_read(chip, buf, offs, chip->info->page_main_size, td)，td->options包含了 NAND_BBT_NO_OOB 也就是BBT标记不放在OOB区域，调用 scan_read_data(chip, buf, offs, td)，buf还是之前malloc的buf，offs是要读哪一个page，长度就忽略，td就是主镜像表
                - scan_read_data：如果有版本号，长度再+1，调用spinand_read_ops去读数据，长度是len，
                - spinand_read_ops：依次读取数据区和oob区(如果有的话)，调用chip->read_page
                - chip->read_page：发指令给flash，然后读到的数据到了buf
                - 读到的数据到了buf中，调用check_pattern(buf, scanlen, chip->info->page_main_size, td)：
                - check_pattern：oob区域不用来存放坏块标记，调用check_pattern_no_oob(buf, td)
                - check_pattern_no_oob：调用 memcmp(buf, td->pattern, td->len)，比较buf中的数据是否是bbt的描述信息(.pattern = "B' 'b' 't' '0'),如果找到分区表就返回0，否则返回-1
                - 如果是0，找到坏块表，就打印td->pages[0] = 实际block对应的page，调用 bbt_get_ver_offs(chip, td)获取版本号在buf中的偏移，然后将 td->version[i] 进行更新
                - bbt_get_ver_offs：根据之前定义时td中的版本号所在偏移，返回即可
            - search_bbt(chip, buf, md)：跟上面主表一样的操作方式，就是在最后的四个block中的第一个page读出来，然后对比里面的数据是不是定义的坏块表标识符来确认当前flash中是否有坏块表
        - 调用 check_create(chip, buf, bd)：buf是刚创建的一个block_size空间，bd是前面的 struct nand_bbt_desrc 结构，其中的 pattern 是 0xff 0xff
        - 将 tab 放到前面，否则后面放不下了
- check_create(chip, buf, bd):buf这个时候已经是读到的最后block的第一个page，根据 NAND_BBT_PERCHIP 判断有多少个 chip，目前只有 1个
- 如果md存在：
  - 主坏块表和镜像坏块表都没有找到，create=1，writeops = 0x03
  - 主坏块表没有找到，rd = md,writeops = 0x01
  - 镜像坏块表没有找到，rd=td; writeops = 0x02
  - 如果主表和镜像表的版本号相同，rd = td，如果用了版本 NAND_BBT_VERSION 宏，rd2 = md
  - 如果主表版本号大于镜像版本号，rd = td; writeops = 0x02
  - 否则 rd = md; writeops = 0x01
- 如果镜像表不存在：如果主表没有找到，create = 1; writeops = 0x01, 否则 rd = 主表
  - 只有主表，如果主表找到了就用主表，如果主表没找到，就需要创建
- 如果需要创建：
  - 判断创建主表的时候描述信息中是否有 NAND_BBT_CREATE 宏
  - 如果没有 NAND_BBT_CREATE_EMPTY 宏，则调用 create_bbt(chip, buf, bd, chipsel)，这里buf是开始malloc的，然后用来读最后四个block的page的，bd是开始创建的描述信息，pattern 是0xff 0xff的那个，chipsel不用想，只有一个
    - create_bbt(chip, buf, bd, chipsel)：如果定义了宏 NAND_BBT_SCAN2NDPAGE就扫描 2 个page，否则就扫描 1 个page
    - 判断 chip_nr 是否是 -1，如果是 -1 ，就是没有 NAND_BBT_PERCHIP 宏，其实 startblock 都是 0，from 也都是 0
    - 如果有 NAND_BBT_SCANLASTPAGE 这里的 from 就到 block 的最后一个page
    - 关闭 ecc
    - 循环，从起始block也就是0，到最后一个block
      - 调用 scan_block_fast(chip, bd, from, buf, numpages)，参数bd是前面创建的 0xff 0xff 的那个，from 是哪个page，buf 应该是用来存储的，numpages 是1
        - ***  这里是在读厂商的坏块标记，读每个 block 的第一个page，如果不是 0xff 就是坏块 ***
        - scan_block_fast(chip, bd, from,buf, numpages)：初始化一个 读的 ops，要读oob区域，长度是oobsize，模式不开ecc，读一个 page 的oob数据，调用 spinand_read_ops()，
          - spinand_read_ops()：调用 chip->read_page，其实就是发命令给flash，然后地址是到oob区域，读到buf
        - check_short_pattern(buf, bd)：调用 memcmp(buf + td->oofs, td->pattern, td->len)，比较 buf 中的td偏移(0)，长度是1，比较是不是0xff，如果相等则返回0，不相等则返回-1
        - 如果 check_short_pattern 返回的值是0 表示不是坏块，就加偏移继续找，如果返回的是-1，则是坏块，则返回-1；找到最后都没有坏块就说明这个块是好块
      - 如果scan_block_fast 返回的是 1，就是有坏块，调用 bbt_mark_entry(chip, i, BBT_BLOCK_FACTORY_BAD(0x03))，其中参数 i 是 哪一个 block，后面的是宏，
        - bbt_mark_entry(chip, i, BBT_BLOCK_FACTORY_BAD)：
          - msk = (0x03 & 0x03) << (block & 0x03)*2);
          - chip->bbt[block>>2] |= msk
          - 算一下：
            - 坏块 0：chip->bbt[0] |= 0x03
            - 坏块1： chip->bbt[0] |= 0x0c
            - 坏块2： chip->bbt[0] |= 0x30
            - 坏块3： chip->bbt[0] |= 0xc0
            - 坏块4： chip->bbt[1] |= 0x03
            - 坏块5： chip->bbt[1] |= 0x0c
      - 就这样循环找每个block，打开 ecc
  - 然后给主表和镜像表一个版本号，版本号为1
  - 上面是需要创建坏块表的情况，只是创建了一个坏块表，但是还没有写入到 Flash 中存储下来
  - 如果找到了坏块表的话，如果 rd 存在，调用 read_abs_bbt(chip, buf, rd, chipsel)
    - read_abs_bbt(chip, buf, rd, chipsel)：如果有宏 NAND_BBT_PERCHIP的话就循环每个 CHIP，其实就 1 个，调用 read_bbt(chip, buf, td->pages[i], chip->info->usable_size >> chip->info->block_shift, td, offs)参数 buf 是用来存放读到的 bbt 的，td->pages[i]是存放的bbt，block数量，主表，偏移是0
      - read_bbt(chip, buf, td->pages[i], blocks, td, 0)：
        - 计算总共要读的长度：block 数量 * 坏块表中几bit用来表示一个块的好坏 然后 >> 3
        - 调用 add_marker_len(td)：td 是 pattern 为 0xff 0xff 的东西，长度 += td的长度 + 坏块表数据长度信息字段长度 + 坏块表 MARK 中的保留字段长度，实际就是算出来一个长度
        - 在读的时候，总长度减去算出来的长度，地址+=这个长度
        - 组建 ops，调用 spinand_read_ops(chip, from, &ops, &ecc_stats_corrected, &ecc_stats_failed)
          - 根据要读的是 data 区域还是 oob 区域，调用 chip->read_page ，实际就是调用 SPI 的驱动发指令给 Flash
        - 数据读到了 ops 中的 buf，然后分析数据，每2bit代表一个block，11 代表是好块，继续扫描下2bit数据，如果读到的数据是 0，那就标记是工厂坏块，否则标记 01 使用中产生的坏块，这里也只是写到了 chip->bbf[] 这个数组里，也就是内存里面；循环的扫描总长度就能够更新所有的坏块
    - 如果 rd2 存在，再读 read_abs_bbt(chip, buf, td2, chipsel)
    - 如果数据有跳变，则 writeops = 0x03
    - 如果 镜像表存在，主表的版本号就是 主表和镜像表的版本号中 较大的值，并且镜像表的版本号也=主表的版本号
    - 如果 writeops & 0x01，且 td->options & NAND_BBT_WRITE 那么就调用 write_bbt(chip, buf, td, md, chipsel)来写坏块表，参数buf是临时buf，td是主表，md是镜像表，chipsel = 0
      - write_bbt(chip, buf, td, md, chipsel)：如果保留的block_code 为0，那么就赋值为 0xff
      - 如果td->options & NAND_BBT_PERCHIP 宏，计算 numblocks 为总共的blocks
      - 循环所有的 chip，调用 get_bbt_block(chip, td, md, chip_nr) 获取第一个适合存储 bbt 的有效擦除块
        - get_bbt_block(chip, td, md, chip_nr)：如果主表已经存在，那么return  td->pages[chip_nr] >> (chip->info->block_shift - chip->info->page_shitf) 算出 block
        - 否则的话计算出所有的 block，如果有 NAND_BBT_LASTBLOCK 宏，起始block为最后一个block，dir  = -1 
        - 循环次数为td->maxblocks 也就是 4，计算 block = startblock + dir * i;调用 bbt_get_entry(chip, block)）
          - bbt_get_entry：首先拿到 chip->bbt[]数组，然后做&操作，如果是坏块则跳过，不是坏块则计算这个block所在的page，并且看一下这个page是否被用作镜像表
        - 这里是在最后四个block 里面找一个好块
      - 然后计算出来找到的好快对应的 page，bits  为 td->结构中定义的位数，msk[2] = ~rcode = 0x00
      - 根据 bits(用几bit表示一个块的好坏)进行填充值：
        - bits:1
          - sft = 3;
          - sft_msk = 0x07;
          - msk[0] = 0x00;
          - msk[1] = 0x01;
          - msk[3] = 0x01;
        - bits:2
          - sft = 2;
          - sft_msk = 0x06;
          - msk[0] = 0x00;
          - msk[1] = 0x01;
          - msk[3] = 0x03;
      - 计算要写入的地址 to，也就是根据page来转换
      - 如果 不用 OOB 区域来存坏块表，offs = td->len = 4；如果有版本号NAND_BBT_VERSION，offs++;offs += BBT_MARK_LENGTH_LEN + BBT_MARK_RESERVED_LEN;
      - 长度是 numblocks >> sft；长度 += offs;进行页对齐，长度变成page_main_size；把td->pattern 拷贝到buf，长度是td->len；这里把 buf 的头变成了 td->pattern
      - 如果 td->options & NAND_BBT_VERSION ，那么就吧 buf[ooboffs + td->veroffs] = td->version[chip_nr]，也就是 buf[td->veroffs] = td->version[0]
      - buf[offs-2] = (numblocs >> sft)  ： 低8位
      - buf[offs-1] = (numblocks >> sft) >> 8 ： 高 8 位 共起来就是block的总数量，总长度
      - 遍历所有的 block，调用 bbt_get_entry 查看是否是坏块，然后把 2bit 数据写到 buf 中
      - 如果不在 OOB 区域放坏块表，那么调用 bbt_crc32 计算 crc，然后把 crc 值写到 buf 中去
        - bbt_crc32(buf, offs+(numblocs >> sft)：计算出数据对应的  crc 值，然后返回回去
      - 调用 chip->erase_block 擦除掉坏块表将要存放的block
      - 调用 scan_write_bbt(chip, to, len, buf, td-<options & NAND_BBT_NO_OOB?NULL:&bul[len]
        - scan_write_bbt()：将buf、offs、len 传下去，调用 spinand_write_ops(chip, offs, &ops) 将buf写入到 offs 地址去，长度为 len
          - 根据 main或oob 调用 chip->program_page
      - td->pages[0] = page; 把 page 标记到td->page[0] 中，表示坏块表存在的page
    - 这样一整个坏块表就已经都写到 flash 中了，上面只会调用一次，也就是写坏块表只有写 1 次，所有的 block 好坏已经版本等，都在 buf 中，只要把 buf 写到 Flash 中
- 如果调用 check_create 返回值 !=0 或者 != ENOSPC 就 free 掉 buf，再 free 掉 chip->bbf
- 调用 mark_bbt_region(chip, td) 防止 bbt 区域擦除\写入，相当于把 bbt 所在的块描述成坏块，让用户不能使用
  - mark_bbt_region(chip, td)：td 是主坏块表，还是先算有多少个 chip、blocks，其实只有 1个chip
  - block = 所有的block - td->maxblocks，也就是 block = 所有的 block - 4
  - 循坏 td->maxblocks 次，调用 bbt_get_entry(chip, block)， 然后调用 bbt_mark_entry(chip, block, BBT_BLOCK_RESERVED)
    - bbt_get_entry(chip, block)：返回代表这个 block 好坏的 2bit 数据
    - bbt_mark_entry(chip, block, RESERVED)：标记 这个block的值为 RESERVED
  - 如果 get_entry 返回的值 不是 RESERVED ，那就表示需要更新
- 至此，SPI Nand 的坏块表初始化告一段落.

![](image/1687681551406-02d10a65-e7ad-4689-a63a-0588e37d15e9.jpeg)






# eCos 驱动相关知识：

- eCos disk 驱动分为设备驱动层和一般 IO 抽象层。前者对应 `/devs/disk`目录，后者对应`io/disk` 目录。
- ![image.png](https://cdn.nlark.com/yuque/0/2023/png/36177780/1687758695334-0374a4dd-5b61-421b-9cbd-baf7ef755501.png#averageHue=%23f2f0e7&clientId=ub142e10d-9333-4&from=paste&height=521&id=u88ff542a&originHeight=521&originWidth=402&originalType=binary&ratio=1&rotation=0&showTitle=false&size=15705&status=done&style=none&taskId=u36e80419-f6af-4636-9a16-292b4b520d6&title=&width=402)
  - eCos disk IO 层：
    - 如上所述，eCos disk IO 层抽象出了一般的 disk 设备。向上提供 bread、bwrite 等API 接口的实现，向下提供了统一的数据结构体和接口，大大简化了设备驱动层的设计。
    - 块设备 API 接口实现：
      - 位于 disk.c 源文件中，它向上提供了 read/write 等方法，如下代码所示：

```c
BLOCK_DEVIO_TABLE(cyg_io_disk_devio,
                  disk_bwrite,
                  disk_bread,
                  disk_select,
                  disk_get_config,
                  disk_set_config
);

```







## Flash 驱动流程：

### 设备驱动程序基础知识：

#### 概述

- 在 eCos 系统中，设备驱动程序是以包 (Package) 的形式存在的，当需要使用该设备时，通过 eCos 环境配置工具或 .db 把设备驱动程序包加载到内核中，编译后生成一个库文件，用户使用时，只需要打开该设备，直接调用接口API 函数即可

#### cdl 脚本

- eCos 的设备驱动程序一般是以包的形式存在。而 eCos 中所有的包都必须具有至少一个 cdl 脚本对该包进行描述。这种 cdl 脚本中包含了该保重所有配置选项的详细信息，并且提供了如何对该报进行编译的信息。
- **当编写一个设备的驱动程序时，必须编写相应的 cdl 脚本文件**


#### ecos.db

- 配置工具使用 eCos 库的数据库来理解库中的组件。库的数据库中包的描述包含在文件 ecos.db 中，该文件在 `ecos/packages`中
- ecos.db 文件使用 cdl 描述所有包和数据库中的目标硬件
- 一个设备驱动程序包在用 cdl 脚本对其进行描述后，必须加载到 eCos 的数据库才能正常使用，这时候需要通过 ecos.db 对该设备驱动程序包进行描述

#### 编写设备驱动的一般步骤

- 在`packages/devs`目录下建立设备驱动程序目录
- 建立 cdl 文件和驱动源程序，一般是 .c
- 创建头文件(如果需要)
- 编写 cdl 脚本文件，建立设备驱动程序组件包，定义配置选项
- 编写驱动源程序文件，编写其中的设备表入口、驱动程序接口函数等
- 必要时编写头文件
- 把设备驱动程序组件包加载到 eCos 数据库中，在 ecos.db 中将包添加到数据库
- 在数据库中找到该设备驱动程序组件包，加载到开发平台
- 编译生成 *.ecc 库文件
- 编写测试程序测试驱动程序


### 前提条件：target --> .db --> .cdl

1. target：编译的目标文件，需要描述有哪些包存在

```c
    package -hardware CYGPKG_IO_SPI v3_0 ;
    package -hardware CYGPKG_IO_DISK v3_0 ;
    package -hardware CYGPKG_IO_FLASH v3_0 ;
    package -hardware CYGPKG_IO_NAND v3_0 ;
    package -hardware CYGPKG_DEVS_GX_EEPROM v3_0 ;
    package -hardware CYGPKG_DEVS_GX_OTP v3_0 ;
    package -hardware CYGPKG_DEVS_GX_MTC v3_0 ;
    package -hardware CYGPKG_DEVS_GX_SCPU v3_0 ;
    package -hardware CYGPKG_DEVS_GX_SPI v3_0 ;
    package -hardware CYGPKG_DEVS_GX3110_IRR v3_0 ;
    package -hardware CYGPKG_DEVS_GX3110_WDT v3_0 ;
    package -hardware CYGPKG_DEVS_GX_LOWPOWER v3_0 ;
    package -hardware CYGPKG_DEVS_GX3110_I2C v3_0 ;
    package -hardware CYGPKG_DEVS_GX_SERIAL v3_0 ;
    package -hardware CYGPKG_DEVS_GX3110_DISK_SD v3_0 ;
    package -hardware CYGPKG_DEVS_GX3110_DISK_IDE v3_0 ;
    package -hardware CYGPKG_DEVS_SPI_NOR_FLASH v3_0 ;
```

2. cdl 文件，需要添加 cdl 文件来让 eCos 知道有哪些源文件

```c
cdl_package CYGPKG_DEVS_GX_SPI {
    display     "GX SPI driver"

    parent      CYGPKG_IO_SPI
    active_if   CYGPKG_IO_SPI

    include_dir cyg/io
    compile     -library=libextras.a gx_spi.c dw_spi_v2.c
}
```

3. db 文件：用于告知 eCos 到哪个目录下去查找 packeg

```c
package CYGPKG_DEVS_GX_SPI {
    alias         { "GX SPI driver" }
    directory     devs/spi/gx
    script        gx_spi.cdl
    description "This package provides a driver for the GX SPI unit
                 controller family."
}

package CYGPKG_DEVS_SPI_NOR_FLASH {
	alias           { "SPI nor Flash driver" flash_spi_nor }
	directory       devs/flash/spinor
	script          spi_nor_flash.cdl
	description "
	Support for SPI nor flash devices."
}
```

### 设备驱动层

- 提供 spi 、flash 的初始化、具体操作等函数
- `packegs/devs/spi/gx/v3_0/include/gx_dev_dw_spi_init.hxx`：dw_spi
- `packegs/devs/spi/gx/v3_0/include/gx_dev_spi_init.hxx`：gx_spi
- 在头文件中实现了一个类的构造函数，构造函数里面包括 spi 和  flash 的初始化函数
- 用户在定义类的时候就会调用这些初始化函数了，用户在 应用程序中定义这个类

### IO 抽象层

- 提供 flash 向上的接口，包括 注册 block设备、提供 bwrite、bread、close、get_config 、set_config 等接口给用户使用
- `packages/io/flash/v3_0/include/gx_flashio_init.hxx`：flash 的 io 抽象层
- 在头文件中实现了一个类的构造函数，构造函数里面实现 flash 的 block_dev_register
- 用户在应用程序中定义这个类：`ecos_shell/prj/xx/include/gxcore_ecos_module.hxx`

### 用户用法

- 在应用中声明 spi 、flash 的两个类，声明 spi 类的时候会初始化 spi和flash
- 声明 flash 类的时候会注册 `block_dev_register`
- `ecos_shell/prj/spi_cramfs_jffs2/include/gxcore_ecos_module.hxx`

```c
//cyg_gx_spinor_flash_init_class   spinorflash0   CYGBLD_ATTRIB_INIT_PRI(60900);
cyg_dw_spinor_flash_init_class   spinorflash0   CYGBLD_ATTRIB_INIT_PRI(60900);
//cyg_gx_spinand_flash_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
//cyg_dw_spinand_flash_init_class  spinandflash0  CYGBLD_ATTRIB_INIT_PRI(60910);
cyg_flashio_init_class    flashio    CYGBLD_ATTRIB_INIT_PRI(61000);
//nand_init_class           nandflash0 CYGBLD_ATTRIB_INIT_PRI(61110);
```

- 初始化 spi，在 eCos 中有相关的 SPI 驱动
- 初始化 flash，在 gxmisc 中有相关的 Flash 驱动，其中会调用 `packages/io/dev_manage/v3_0/dev_register.cxx`中的 `flash_dev_register`，就是在 `cyg_flashdevab[0] --> cyg_flashdevtab_end`中找，如果 其中的 ->funcs 是空的，就把 gxmisc 中注册的一些东西拷贝到这个地址来
- `cyg_flashdevtab[0]`这些其实就是 `packages/io/flash/v3_0/src/flash.c`
  - 宏 `CYG_HAL_TABLE_BEGIN(cyg_flashdevtab, cyg_flashdev);`
  - `CYG_HAL_TABLE_END(cyg_flashdevtab_end, cyg_flashdev);`
- cyg_flashdev：`packages/io/flash/v3_0/include/flash_dev.h`

```c
struct cyg_flash_dev _name_ CYG_HAL_TABLE_ENTRY(cyg_flashdev) = \
{                                                               \
    .funs               = _funs_,                               \
    .flags              = _flags_,                              \
    .start              = _start_,                              \
    .end                = _end_,                                \
    .num_block_infos    = _num_block_infos_,                    \
    .block_info         = _block_info_,                         \
    .priv               = _priv_,                               \
    .jedec_id           = _jedec_id_,                           \
}
```

```c
struct cyg_flash_dev gxspi_flash_device =
{
	.funs               = &cyg_devs_gxspi_funs,
	.flags              = 0,
	.start              = 0,
	.end                = 0,
	.num_block_infos    = 1,
	.block_info         = &gxspi_flash_device_block_info,
	.priv               = &m25p_dev,
	.jedec_id           = 0
};

extern struct spi_master gx_spi_master;
__externC void gx_spiflash_init(void)
{
	m25p_dev.flash.spi->master = &gx_spi_master;
	flash_dev_register(&gxspi_flash_device);
}

__externC void gx_spiflash_init_v2(struct spi_master *master)
{
	m25p_dev.flash.spi->master = master;
	flash_dev_register(&gxspi_flash_device);
}
```


- 总结：flash 驱动的初始化
  - 首先定义 cyg_flash_dev，其中的接口是什么 .func .start .end .jedec_id 什么的，这样一个结构，其实是 CYG_HAL_TABLE_ENTRY 修饰的一个东西，这个宏是注册一个块设备必须使用宏 BLOCK_DEVTAB_ENTRY 进行定义
  - 由于可能存在多款 Flash 在同时运行，所以有一个 TABLE，就是一块空间，包括 CYG_HAL_TABLE_BEGIN、CYG_HAL_TABLE_END
  - 然后初始化的时候，gxmisc 会调用注册 flash 设备，在 TABLE 中找一块空地方，然后拷贝过来
  - 结束


### 插入 eCos 驱动程序结构：

![选区_121.png](https://cdn.nlark.com/yuque/0/2023/png/36177780/1687769625513-6fa24999-fdba-4368-9a07-3c00eb2845ad.png#averageHue=%23f4f4f4&clientId=ub142e10d-9333-4&from=drop&id=u55b5848d&originHeight=543&originWidth=580&originalType=binary&ratio=1&rotation=0&showTitle=false&size=36299&status=done&style=none&taskId=u6d83b545-660d-4eec-aeb8-771069890a9&title=)

- 设备驱动程序只不过是一个支持基本 IO 功能的、有名称的实体，支持 read、write、get_config、set_config。
- 设备驱动程序一般可分为三部分，分别为设备表入口 `DEVTAB_ENTRY`、设备I/O 函数表 `DEVIO_TAB`、和设备 I/O 函数
- 设备驱动程序的主要宏可以在 `cyg/io/devtab.h`中找到
- eCos 中所有设备驱动程序均有设备表 `cyg_devtab_tnery_t`类型描述，使用 `DEVTAB_ENTRY()`宏创建设备表入口：

```c
/*
 l: 该设备表入口的 C 标识符
 name： 设备的 C 字符串名称
 dep_name： 对于分层设备，该设备所基于的设备的 C 字符串名称
 handlers： 指向 IO 函数句柄的指针
 init：     eCos 初始化时调用的函数。该函数可以查询设备、设置硬件等
 lookup：   当为此设备调用 cyg_io_lookup() 时调用的函数
 priv：     驱动程序所需的任何设备特定数据的占位符。驱动所需的专用数据存放位置
*/
DEVTAB_ENTRY(l, name, dep_name, handlers, init, lookup, priv)




/* 上面定义的设备表入口只适用于字符设备 */
/* 对于块设备，要使用宏 BLOCK_DEVTAB_ENTRY 进行定义 */
BLOCK_DEVTAB_ENTRY(l, name, dep_name, handlers, init, lookup, priv)
```

- 设备表入口函数中的句柄 handlers 提供了一组设备驱动程序接口函数。 handlers 是设备IO函数表 `DEVIO_TAB`的指针：

```c
/*
 l: 该设备表条目的 C 标签
 write：  作为 cyg_io_write() 调用的函数
 read:    作为 cyg_io_read()  调用的函数
 select:  作为 cyg_io_select() 调用的函数
 get_config： 作为 cyg_io_get_config() 调用的函数
 set_config： 作为 cyg_io_set_config() 调用的函数
*/
DEVIO_TABLE(l, write, read, select, get_config, set_config)
```

- 设备 IO 函数：


- 再继续执行的是 flash io 层的初始化：
  - 同样是由 ecso_shell 中的头文件定义一个 class，然后这个class 的构造函数在 `packages/io/flash/v3_0/include/gx_flashio_init.hxx`中实现

```c
class cyg_flashio_init_class {
public:
    cyg_flash_init_class(void)
    {
        flashio_init():
    }
}
```

   - flash io 层的初始化会提供出来各种 io 操作接口
     - 为什么文档描述使用的是 BLOCK_DEVICE_TABLE，而我们驱动里面用的是 CYG_HAL_TABLE_ENTRY
       - 用宏的最后相当于定义了一个结构体，然后填充了里面的成员，gx 用自己的方式，也是先配这些结构，然后调用了 gx_devtab_copy 把结构填充好，并且调用了 init 方法
       - 最后的实现都是一样的，gx 自己做了封装

```c
#define BLOCK_DEVTAB_ENTRY(_l,_name,_dep_name,_handlers,_init,_lookup,_priv)  \
cyg_devtab_entry_t _l CYG_HAL_TABLE_ENTRY(devtab) = {                   \
   _name,                                                               \
   _dep_name,                                                           \
   _handlers,                                                           \
   _init,                                                               \
   _lookup,                                                             \
   _priv,                                                               \
   CYG_DEVTAB_STATUS_BLOCK                                              \
};

#define BLOCK_DEVIO_TABLE(_l,_bwrite,_bread,_select,_get_config,_set_config)    \
cyg_devio_table_t _l = {                                        \
    cyg_devio_cwrite,                                           \
    cyg_devio_cread,                                            \
    _bwrite,                                                    \
    _bread,                                                     \
    _select,                                                    \
    _get_config,                                                \
    _set_config												\
};

/* BLOCK_DEVICE_TABLE 宏对应的 */
cyg_devio_table_t cyg_io_flashdev_ops={
	cyg_devio_cwrite,
	cyg_devio_cread,
	&flashiodev_bwrite,
	&flashiodev_bread,
	NULL, // no select
	&flashiodev_get_config,
	&flashiodev_set_config,
	&flashiodev_ioctl,
	flashiodev_close
};

/* BLOCK_DEVIO_TABLE 宏对应的，其中的 handlers 成员就是指向上面的 cyg_io_flashdev_ops */
static cyg_devtab_entry_t cyg_io_flashdev={
	"/dev/flash/",
	NULL,
	&cyg_io_flashdev_ops,
	&flashiodev_init,
	&flashiodev_lookup,
	NULL 
};

EMPTY_DEVTAB_ENTRY(cyg_io_flashdev_void);
__externC void flashio_init(void)
{
	block_dev_register(&cyg_io_flashdev, "/dev/flash/", NULL);
}


// gx 实现了 block_dev_register、char_dev_register 等接口
 Cyg_ErrNo block_dev_register(cyg_devtab_entry_t *ops, char *name, void *priv_dev)
{
	cyg_devtab_entry_t *tab;

	if (ops == NULL)
		return EINVAL;

	tab = ops;
	tab->name = name;
	tab->priv = priv_dev;

	tab->status = CYG_DEVTAB_STATUS_BLOCK | CYG_DEVTAB_STATUS_AVAIL;

	return gx_devtab_copy(tab);
}

gx_devtab_copy(struct cyg_devtab_entry *tab)
{
	struct cyg_devtab_entry *t;
	bool ret_value;

	if (tab == NULL)
		return EINVAL;

    for (t = &__DEVTAB__[0]; t != &__DEVTAB_END__; t++) {
		if(t->status == CYG_DEVTAB_STATUS_EMPTY) {
			break;
		}
	}

    if(t == &__DEVTAB_END__)
	{
		diag_printf("there is no empty entry\n");
		return ENOMEM;
    }

	t->dep_name = tab->dep_name;
	t->handlers = tab->handlers;
	t->init = tab->init;
	t->lookup = tab->lookup;
	t->name = tab->name;
	t->priv = tab->priv;
	t->status = tab->status;
	if(t->init !=NULL)
	{
		ret_value=t->init(t);
		if(ret_value==true)
		{
			//diag_printf("dynamic device \'%s\' init finished \n",t->name);
		}else{
			diag_printf("dynamic device \'%s\' init error \n",t->name);
			return EINITERR;
		}
	}
	return ENOERR;
}
```

      - flash 的 io 层怎么操作的

   - flashio_init() 会调用 block_dev_register(&cyg_io_flashdev, "/dev/flash/", NULL) ，在这个接口里面的 `cyg_io_flashdev`会包括io层的各种操作包括 bwrite、bread，以及 init 函数和 lookup 函数
   - 调用 block_dev_register 的时候会调用 io 层的 init 函数
   - io 层的 init 函数又会调用 dev->funcs->flash_init(dev) 函数，相当于调用了 gxmisc 中的 flash 初始化函数
   - 初始化就完了
- 用户执行 lookup 来查找设备：
  - err = cyg_io_lookup(devname, &flash_dev)
    - `packages/io/common/ve_0/src/iosys.c` 中是 cyg_io_lookup 函数的实现，其中会在 __DEV_TAB[0] --> **DEV_TAB_END** 中循环调用 cyg_io_compare(name, t->name, &name_ptr) 来比较名称，查找 flash 设备，当相等的时候就会执行 t->lookup 也就是  flashiodev_lookup 函数，这个函数的实现在 `packages/io/flash/v3_0/src/flashiodev.c`
    - 在 flashio_lookup 函数中调用 cyg_flash_scan_ident(flashdevno) 就调到了 gxmisc  中 dev->funs->flash_scan_ident(dev)，这个在gxmisc 中是空的
    - 其实就是找到这个设备，然后从 dev 中获取一些信息，后面其它的接口都要使用这个 dev 来做参数
- 用户执行测试程序：
  - 通过 flash io 层提供的 read、write、get_config、set_config 等
  - 通过调用 cyg_io_get_config 调到 eCos 的 io 抽象层，`packages/io/flash/v3_0/src/flashiodev.c`然后调用 t->handlers->get_config 调到 flash io 层提供的 get_config 接口，也就是  flashiodev_get_config 接口
- 讲一下结构：
  - flash  io 层的文件结构：
    - flashiodev.c 里面是 block_device_register，提供 io 层的接口，包括 read、write、ioctl、get_config、set_config 这种东西的实现，其中这些接口的实现会调用到 下面的读写接口，还是 io 层的读写接口，io 层的读写接口做一些限制时候再去调用 底层，也就是gxmisc 的 dev->funs->xxx 接口
      - ![选区_123.png](https://cdn.nlark.com/yuque/0/2023/png/36177780/1687833006369-9b2763f6-7112-4726-b8fd-23bb9c65b74c.png#averageHue=%23312f2d&clientId=ubf1d99c5-71c5-4&from=drop&id=ua1a3b23e&originHeight=297&originWidth=411&originalType=binary&ratio=1&rotation=0&showTitle=false&size=19121&status=done&style=none&taskId=u8e263a92-dbf2-47ca-bbb2-9e1b919cb16&title=)
    - flash.c 就是给 flash io层提供一些接口的，封装了一层，下层是 gxmisc，上层是给 flash io 层的 ops 实现用
      - ![选区_124.png](https://cdn.nlark.com/yuque/0/2023/png/36177780/1687833032058-4e9ab8d7-d915-4148-8d6a-a4da853325b5.png#averageHue=%23322f2d&clientId=ubf1d99c5-71c5-4&from=drop&id=u822a233c&originHeight=512&originWidth=404&originalType=binary&ratio=1&rotation=0&showTitle=false&size=29225&status=done&style=none&taskId=u4dd3bb20-6e5e-4155-ab09-aee014c0f18&title=)
    - gxmisc ：
      - ![选区_125.png](https://cdn.nlark.com/yuque/0/2023/png/36177780/1687833065797-aabd119a-7015-4220-9193-ed623daee264.png#averageHue=%2332302d&clientId=ubf1d99c5-71c5-4&from=drop&id=u80e01a37&originHeight=527&originWidth=406&originalType=binary&ratio=1&rotation=0&showTitle=false&size=25755&status=done&style=none&taskId=u21f27a05-ce44-4b62-b794-e5e5bae073b&title=)









# eCos 一些相关配置文件 .db  .cdl  .ecc

## .db

> `Package` 修饰一个设备驱动，相当于加载到数据库，用 `target` 表示目标(板级/架构)，`target` 下的 `package` 代表要加载的设备驱动程序包。

​		一个设备驱动程序包在用 cdl 脚本对其进行描述后，必须加载到 eCos 库的数据库中才能正常使用，这时候需要通过 ecos.db 文件对该设备驱动程序包进行描述。 在写好一个驱动程序包的 cdl 脚本后，添加该包到数据库中的常见方法如下：

1. 定义包：用 package 命令定义设备驱动程序包，其包名必须与 cdl 文件中 cdl_package 定义的 包名相同。

2. 定义包体：在包体内用 alias 属性描述该包的功能，此属性描述的内容会显示在 eCos 库的数 据库里；另外用 directory 属性指定该包的 cdl 脚本文件所在路径；用 script 属性说明该包的 cdl 脚本 文件名；最后用 description 属性描述该包的详细功能。 

3. 假设有一个设备 dev1 的驱动程序包，其 cdl 脚本文件名为 dev1_spce3200.cdl，其典型的 ecos.db 中包的描述如下： 

   ```
   package CYGPKG_DEVS_DEV1_SPCE3200 {
    alias { "dev1support for Score SPCE3200"}
    directory devs/dev1/score/spce3200
    script dev1_spce3200.cdl
    description "
    This package contains hardware support for the dev1
    on the Score SPCE3200 DEV Board."
   } 
   ```

   - CYGPKG_DEVS_DEV1_SPCE3200 为设备dev1 在驱动cdl文件中已经定义好的cdl包名；

   - alias是 对于这个包的功能简要概述，作为dev1 设备的驱动程序名，显示在eCos配置工具的eCos数据库中；

   - directory指定dev1 设备驱动程序的源路径；
   - script指定驱动程序cdl文件的文件名
   - 在对内核编译的时候，编译工具通过directory指定的路径和script指定的cdl文件名找到设备驱动程序的cdl文 件，通过cdl文件描述的内容把设备驱动程序编译到内核中并生成库；
   - description是对dev1 设备驱动 程序的简单描述。

## .cdl

> `.cdl` 文件用于描述设备驱动程序包。

​		eCos 的设备驱动程序一般是以包的形式存在。而 eCos 中所有的包都必须具有至少一个 cdl 脚本对该包进行描述。这种 cdl 脚本文件中包含了该包中所有配置选项的详细信息，并且提供了如何对该报进行编译的信息。同样，当编写一个设备的驱动程序的时候，必须编写相应的 cdl 脚本文件。

​		在驱动程序包的脚本 cdl 文件中，需要描述如下几方面内容：

1. 用 cdl_package 命令定义驱动程序包，驱动程序包中含 display、include_dir、compile、description 等属性。 
2. 用 cdl_component 命令描述驱动程序包中的组件。 
3. 用 cdl_option 命令描述驱动程序包或者组件中的选项。 
4. 描述其他内容。

## .ecc



## 编写设备驱动的一般步骤

![选区_154](image/选区_154.png)





# Linux 内核为什么能够使用 printk 函数

- printk 函数在 Linux 内核中定义并且对模块可用；它与标准 C 库函数 printf 的行为相似。
- 内核需要它自己的打印函数，因为它靠自己运行，没有 C 库的帮助。
- 模块能够调用 printf 是因为 在 insmod 加载了它之后，模块被连接到内核并且可存取内核的共用符号。
- 字串 `KERNEL_ALERT` 是消息的优先级。



# Linux 符号表

- 所谓 `符号` 就是内核中的函数名、全局变量等。`符号表` 就是用来记录这些 `符号` 的文件。

- 驱动程序可以编译成 `.ko`，每个 `.ko` 是相互独立的，也就是说模块之间无法相互访问，这时就通过符号表来访问要调用其它模块的函数

  ```c
  EXPORT_SYMBOL()
  EXPORT_SYMBOR_GPL()
  ```

- 使用方法：

  - A 模块使用 `EXPORT_SYMBPL()` 导出函数
  - B 模块声明一下此函数，即可使用 `extern type name()`



# module_init

- 在单片机中，程序都是从 main 函数开始运行的，那么在加载 `ko` 文件的时候，程序是怎么运行的呢？

  - 使用 `module_init` 的时候查看具体实现，宏 `MODULE` 由顶层 Makefile 决定是否有值，顶层 Makefile 中如果要编译的是模块，那么为空， 如果编译进内核，那么有值，所以这里 `module_init` 就等价于 `__initcall(fn)`  --> `__define_intcall("1", fn)` 

  - 就是定义了一些段，然后在启动的时候会循环找哪些段里面的函数，然后执行这些段

    ```c
    #define __define_initcall(level,fn) \
    	static initcall_t __initcall_##fn __used \
    	__attribute__((__section__(".initcall" level ".init"))) = fn
    ```

    ![选区_130](image/选区_130.png)

  - 相当于搞了一些段，专门用来存放这些函数，并且每个等级都有宏的起始和结束位置

  - 在 `init/main.c` 中会拿到这些位置：

    ​		![选区_131](image/选区_131.png)

    ```
    start_kernel()
    	--> rest_init()
    		--> kernel_thread(kernel_init, NULL, CLONE_FS)
    			--> kernel_init
    				--> kernel_init_freeable()
    					--> do_basic_setup()
    						--> do_initcalls()
    							--> do_init_level(level)
    								--> parse_args(initcall_level_names[level], initcall_command_line,xxx)
    								--> for (fn = initcall_levels[level]; fn < initcall_levels[level+1]; fn++)
    									--> do_one_initcall(*fn)
    ```






# 字符设备

## 主次编号

​		字符设备通过文件系统中的名字来存取，通常位于 `/dev` 目录。字符驱动使用 `ls-l` 查看输出的第一列 `c` 表示，块设备是`b` 表示；修改日期前面的两个数字代表主设备号、次设备号。

```shell
crw-rw-rw-   1 root   root       10, 229 7月  10 13:00 fuse
crw-------   1 root   root       89,   0 7月  10 13:00 i2c-0
brw-rw----   1 root   disk        7,   0 7月  10 13:00 loop0
drwxr-xr-x   2 root   root            60 7月  10 13:00 net
crw-rw-rw-   1 root   root        1,   3 7月  10 13:00 null
crw-rw-rw-   1 root   tty         5,   0 7月  11 10:26 tty
```

​		传统上，主编号表示设备相连的驱动。主设备号相同的表示一类设备。次设备号用来决定这一类设备中的哪一个设备。

### 设备编号的内部表示

​		在内核中，`dev_t` 类型是 `32bit`，其中 `12bit` 用作主设备号，`20bit` 用作次设备号。并且内核提供了两个宏用来获得一个 `dev_t` 类型的主/次设备号。相反的，如果有主次设备号，可以将其转换成 `dev_t` 类型：

```c
MAJOR(dev_t dev);
MINOR(dev_t dev);
MKDEV(int major, int minor);

#define MINORBITS	20
#define MINORMASK	((1U << MINORBITS) - 1)

#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))
```

​	

#### 分配和释放设备编号

​		在建立一个字符设备的时候需要获取一个或多个设备编号使用。函数`register_chrdev_region`会用到设备号。

```c
/*
 * first：要分配的设备编号
 * count：要连续请求的设备编号总数
 */
int register_chrdev_region(dev_t first, unsigned int count, char *name);
```

​		如果事先直到要使用哪个设备编号，可以直接使用上面的函数；那么如果事先不知道的时候，使用`alloc_chrdev_region` 内核动态分配一个主设备号；使用 `unregister_chrdev_regopon` 函数来释放设备编号。

```c
/*
 * dev:获取主设备号，通过指针的形式带回来
 * firstminor：请求的第一个要用的次设备号，通常是0
 * count：要连续请求的设备编号总数
 */
int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name);

void unregister_chrdev_region(dev_t first, unsigned int count);
```



### 主设备号的动态分配

​		系统默认会分配一些主设备号给固定的设备使用，在编写驱动的时候建议多使用 `alloc_chrdev_region` 的方式来获取设备号，避免与系统默认分配的造成冲突。

```
一个典型的 /proc/devices 文件看来如下:

Character devices:
 1 mem
 2 pty
 3 ttyp
 4 ttyS
 6 lp
 7 vcs
 10 misc
 13 input
 14 sound 
 21 sg
 180 usb

Block devices:
 2 fd
 8 sd
 11 sr
 65 sd
 66 sd 
```



### 一些重要数据结构

- 文件操作：到现在，我们已经保留了一些设备编号给我们的驱动使用，但是还没有任何操作设备的函数连接到设备编号上。`struct file_operation` 结构是一个字符设备如何建立这个链接。此结构是一个函数指针的集合。
- 文件结构：文件结构代表一个打开的文件。它在使用 open 是创建，并且传递给在文件上操作的任何函数，直到最后的关闭。
  - 通过 `struct file_operations *f_op` 链接上面的文件操作结构体
  - 通过 `void *private_data` 配置私有数据
- inode 结构：inode 结构由内核在内部用来表示文件。inode 结构包含大量关于文件的信息，此结构中只有两个成员对于编写驱动代码有用：
  - `dev_t i_rdev` ：实际的设备编号
  - `struct cdev *i_cdev` ：代表字符设备



## 字符设备注册

​		内核使用类型 `struct cdev` 的结构来代表字符设备。在内核调用设备之前，需要分配并注册一个或多个这些结构。

​		有两种方法分配和注册这些结构。如果想在运行时获得一个独立的 `cdev` 结构，可以为此使用：

```c
/* 第一种注册字符设备的方式：新版本使用 */
void cdev_init(struct cdev *cdev, struct file_operations *fops);

/*
 * dev:cdev 结构
 * num：设备相应的第一个设备号
 * count：关联到设备的设备号的数量
 */
int cdev_add(struct cdev *dev, dev_t num, unsigned int count);

/* 第二种注册字符设备的方式：老版本使用 */
register_chrdev(unsigned int major, const char *name, struct file_operations *fops);
unregister_chrdev(unsigned int major, const char *name);


/* 设备节点 */

/* 第一种创建设备节点方法：手动创建 */
mknod
    
/* 第二种创建设备节点方法：自动创建 */

/* 创建类*/
class = class_create(THIS_MODULE, "xxx");

/*
 * class:在哪一个类下面创建设备
 * NULL：无父设备
 * devid：设备编号
 * NULL：私有数据
 * xxx：设备名称
 */
/* 创建设备 */
device = device_create(class, NULL, devid, NULL, "xxx");
```

![image-20230712141138224](image/image-20230712141138224.png)





# Linux 设备模型

## 总线：`sys/bus`



![/sys/bus目录](image/bus.jpg)



- 一个总线是处理器和一个或多个设备之间的通道。为设备模型的摸底，所有的设备都通过一个总线连接

  ```c
  /* include/linux/device.h */
  struct bus_type {
      const char              *name;
      const struct attribute_group **bus_groups;
      const struct attribute_group **dev_groups;
      const struct attribute_group **drv_groups;
  
      int (*match)(struct device *dev, struct device_driver *drv);
      int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
      int (*probe)(struct device *dev);
      int (*remove)(struct device *dev);
  
      int (*suspend)(struct device *dev, pm_message_t state);
      int (*resume)(struct device *dev);
  
      const struct dev_pm_ops *pm;
  
      struct subsys_private *p;
  
  };
  ```



1. 总线注册

   一个新的总线必须注册到系统，通过 `bus_register` 来注册。

   ```c
   struct bus_type ldd_bus_type = { .name = "ldd", .match = ldd_match, .hotplug = ldd_hotplug, };
   
   ret = bus_register(&ldd_bus_type);
   if (ret)
    return ret; 
   ```

2. 总线方法

   - 包括 `match` ：当添加一个新设备或新驱动到此总线时此方法被调用

3. 列举设备和驱动

   - 列举驱动：

     ```c
     int bus_for_each_drv(struct bus_type *bus, struct device_driver *start, void *data, int (*fn)(struct device_driver *, void *));
     ```

## 设备:`sys/bus/devices  or  sys/devices`

​		在最底层，Linux 系统中的每个设备由一个 `struct device` 代表：

```c
/* include/linux/device.h */

struct device {
		const char *init_name;
        struct device           *parent;
        struct bus_type *bus;
        struct device_driver *driver;
        void            *platform_data;
        void            *driver_data;
        struct device_node      *of_node;
        dev_t                   devt;
        struct class            *class;
		void (*release)(struct device *dev);
        const struct attribute_group **groups;  /* optional groups */
		struct device_private   *p;
};
```

1. 设备注册

​		通用的注册和注销函数：注册完成的设备连接在 `sys/devices/xxx` 

```c
int device_register(struct device *dev);
void device_unregister(struct device *dev);

static void ldd_bus_release(struct device *dev)
{
 printk(KERN_DEBUG "lddbus release\n");
}

struct device ldd_bus = {
 .bus_id = "ldd0",
 .release = ldd_bus_release

}; 

ret = device_register(&ldd_bus);
if (ret)
 printk(KERN_NOTICE "Unable to register ldd0\n");
```







## 设备驱动：`sys/bus/drivers  or  sys/drivers` 就是 驱动

​		设备模型跟踪所有对系统已知的驱动，主要是使驱动核心能匹配驱动和设备。

```c
/* include/linux/device.h */

struct device_driver {
        const char              *name;
        struct bus_type         *bus;

        struct module           *owner;
        const char              *mod_name;      /* used for built-in modules */

        bool suppress_bind_attrs;       /* disables bind/unbind via sysfs */

        const struct of_device_id       *of_match_table;
        const struct acpi_device_id     *acpi_match_table;

        int (*probe) (struct device *dev);
        int (*remove) (struct device *dev);

        const struct attribute_group **groups;
        struct driver_private *p;

};
```

- 注册完成之后可以在 `sys/bus/xxx/drivers` 下面看到这个设备区驱动



## `attribute` 属性文件

​		`/sys` 目录有各种子目录及文件，当我们注册新的总线、设备或驱动时，内核会在对应的地方创建一个新的目录，目录名为各自结构体的 `name` 成员，每个子目录下的文件，都是内核导出到用户控件，用于控制设备的。内核中以 `struct attribute` 结构来描述 `/sys` 目录下的文件，如下所示：

```c
/* include/linux/sysfs.h */

/*  
 * name：指定文件的文件名
 * mode：指定文件的权限
 */
struct attribute {
    const char              *name;
    umode_t                 mode;
};
```

​		`bus_type、device、device_driver` 结构体中都包含了一种数据类型`struct attribute_group`，如下所示，它是多个`attribute`文件的集合， 利用它进行初始化，可以避免一个个注册`attribute`。

```c
/* include/linux/sysfs.h */

struct attribute_group {
    const char              *name;
    umode_t                 (*is_visible)(struct kobject *,
                        struct attribute *, int);
    struct attribute        **attrs;
    struct bin_attribute    **bin_attrs;
};
```



## 设备属性文件

​		在开发单片机的时候，如果想要读取某个寄存器的值，你可能需要加入一些新的代码，并重新编译。但对于Linux内核来讲，每次都需要编译一遍源码， 实在太浪费时间和精力了。为此，Linux提供以下接口，来注册和注销一个设备属性文件。我们可以通过这些接口直接在用户层进行查询/修改，避免了重新编译内核的麻烦。 <font color=red> !!!!!!!!!!!!!!!!!!!!!!!!! WTF !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!</font>

```c
/* include/linux/device.h */

struct device_attribute {
    struct attribute        attr;
    ssize_t (*show)(struct device *dev, struct device_attribute *attr,
            char *buf);
    ssize_t (*store)(struct device *dev, struct device_attribute *attr,
            const char *buf, size_t count);
};

/*
DEVICE_ATTR宏 定义用于定义一个device_attribute类型的变量，##表示将##左右两边的标签拼接在一起，因此， 我们得到变量的名称应该是带有dev_attr_前缀的。该宏定义需要传入四个参数_name，_mode，_show，_store，分别代表了文件名， 文件权限，show回调函数，store回调函数。show回调函数以及store回调函数分别对应着用户层的cat和echo命令， 当我们使用cat命令，来获取/sys目录下某个文件时，最终会执行show回调函数；使用echo命令，则会执行store回调函数。 参数_mode的值，可以使用S_IRUSR、S_IWUSR、S_IXUSR等宏定义，更多选项可以查看读写文件章节关于文件权限的内容。

device_create_file 函数用于创建文件，它有两个参数成员，第一个参数表示的是设备，前面讲解device结构体时，其成员中有个bus_type变量， 用于指定设备挂载在某个总线上，并且会在总线的devices子目录创建一个属于该设备的目录，device参数可以理解为在哪个设备目录下，创建设备文件。 第二个参数则是我们自己定义的device_attribute类型变量。

device_remove_file 函数用于删除文件，当我们的驱动注销时，对应目录以及文件都需要被移除。 其参数和device_create_file函数的参数是一样。
*/

#define DEVICE_ATTR(_name, _mode, _show, _store) \
        struct device_attribute dev_attr_##_name = __ATTR(_name, _mode, _show, _store)
extern int device_create_file(struct device *device,
                const struct device_attribute *entry);
extern void device_remove_file(struct device *dev,
                const struct device_attribute *attr);
```



## 驱动属性文件

​		驱动属性文件，和设备属性文件的作用是一样，唯一的区别在于函数参数的不同，函数接口如下：

```c
/* include/linux/device.h */

struct driver_attribute {
    struct attribute attr;
    ssize_t (*show)(struct device_driver *driver, char *buf);
    ssize_t (*store)(struct device_driver *driver, const char *buf,
            size_t count);
};

/*
DRIVER_ATTR_RW、DRIVER_ATTR_RO 以及 DRIVER_ATTR_WO 宏定义用于定义一个driver_attribute类型的变量，带有driver_attr_的前缀，区别在于文件权限不同， RW后缀表示文件可读写，RO后缀表示文件仅可读，WO后缀表示文件仅可写。而且你会发现，DRIVER_ATTR类型的宏定义没有参数来设置show和store回调函数， 那如何设置这两个参数呢？在写驱动代码时，只需要你提供xxx_store以及xxx_show这两个函数， 并确保两个函数的xxx和DRIVER_ATTR类型的宏定义中名字是一致的即可。

driver_create_file 和 driver_remove_file 函数用于创建和移除文件，使用driver_create_file函数， 会在/sys/bus/<bus-name>/drivers/<driver-name>/目录下创建文件。
*/

#define DRIVER_ATTR_RW(_name) \
    struct driver_attribute driver_attr_##_name = __ATTR_RW(_name)
#define DRIVER_ATTR_RO(_name) \
    struct driver_attribute driver_attr_##_name = __ATTR_RO(_name)
#define DRIVER_ATTR_WO(_name) \
    struct driver_attribute driver_attr_##_name = __ATTR_WO(_name)

extern int __must_check driver_create_file(struct device_driver *driver,
                                    const struct driver_attribute *attr);
extern void driver_remove_file(struct device_driver *driver,
                const struct driver_attribute *attr);
```



## 总线属性文件

```c
/* include/linux/device.h */

struct bus_attribute {
    struct attribute        attr;
    ssize_t (*show)(struct bus_type *bus, char *buf);
    ssize_t (*store)(struct bus_type *bus, const char *buf, size_t count);
};

/*
BUS_ATTR宏定义用于定义一个bus_attribute变量，

使用bus_create_file函数，会在/sys/bus/<bus-name>下创建对应的文件。

bus_remove_file则用于移除该文件。
*/

#define BUS_ATTR(_name, _mode, _show, _store)       \
        struct bus_attribute bus_attr_##_name = __ATTR(_name, _mode, _show, _store)
extern int __must_check bus_create_file(struct bus_type *,
                    struct bus_attribute *);
extern void bus_remove_file(struct bus_type *, struct bus_attribute *);
```



## 测试代码

功能：创建一个虚拟总线 `xbus`，分别挂载了驱动 `xdrv` 以及设备 `xdev`.

步骤：

1. 编写Makefile文件
2. 声明一个总线结构体并创建一个总线xbus，实现match方法，对设备和驱动进行匹配
3. 声明一个设备结构体，挂载到我们的xbus总线中
4. 声明一个驱动结构体，挂载到xbus总线，实现probe、remove方法
5. 将总线、设备、驱动导出属性文件到用户空间。



### Makefile

```makefile
KERNEL_DIR=../ebf_linux_kernel/build_image/build

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export  ARCH  CROSS_COMPILE

obj-m := xdev.o xbus.o xdrv.o

all:
    $(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
modules clean:
    $(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
```



### 总线

- 定义新的总线

  ```c
  int xbus_match(struct device *dev, struct device_driver *drv)
  {
      printk("%s-%s\n",__FILE__, __func__);
      if(!strncmp(dev_name(dev), drv->name, strlen(drv->name))){
          printk("dev & drv match\n");
          return 1;
      }
      return 0;
  }
  
  static struct bus_type xbus = {
      .name = "xbus",
      .match = xbus_match,
  };
  EXPORT_SYMBOL(xbus);
  ```

- 导出总线属性文件

  - 我们通过BUS_ATTR宏，将我们自定义的变量导出到/sys目录，方便用户查询。

  ```c
  static char *bus_name = "xbus";
  
  ssize_t xbus_test_show(struct bus_type *bus, char *buf)
  {
      return sprintf(buf, "%s\n", bus_name);
  }
  
  BUS_ATTR(xbus_test, S_IRUSR, xbus_test_show, NULL);
  ```

- 注册总线

  - 内核的驱动代码，都是基于内核模块，我们在模块初始化的函数中注册总线，在模块注销的函数中注销该总线。

  ```c
  static __init int xbus_init(void)
  {
      printk("xbus init\n");
  
      bus_register(&xbus);
      bus_create_file(&xbus, &bus_attr_xbus_test);
      return 0;
  }
  module_init(xbus_init);
  
  
  static __exit void xbus_exit(void)
  {
      printk("xbus exit\n");
      bus_remove_file(&xbus, &bus_attr_xbus_test);
      bus_unregister(&xbus);
  }
  module_exit(xbus_exit);
  
  MODULE_AUTHOR("embedfire");
  MODULE_LICENSE("GPL");
  ```

  

- 当成功加载该内核模块式，内核变回出现一种新的总线 `xbus`，如图所示：

  ![xbus目录](image/xbus.jpg)

  - 可以看到，总线的devices和drivers目录都是空的，并没有什么设备和驱动挂载在该总线下。红框处便是我们自定义的总线属性文件，当我们执行命令“cat xbus_test”时，可以看到终端上会打印一行字符串：xbus。
  - cat --> show



### 设备

- Linux设备模型中，总线已经注册好了，还缺少设备和驱动。注册一个新的设备，主要完成这两个工作：一个是名字， 这是总线匹配的依据；另一个就是总线，该设备挂载在哪个总线上。

- 这里，我们注册一个设备xdev，并且定义一个变量id，将该变量导出到用户空间，使得用户可以通过sysfs文件系统来修改该变量的值。

- 定义新的设备：

  ```c
  extern struct bus_type xbus;
  
  void xdev_release(struct device *dev)
  {
      printk("%s-%s\n", __FILE__, __func__);
  }
  
  
  static struct device xdev = {
      .init_name = "xdev",
      .bus = &xbus,
      .release = xdev_release,
  };
  ```

- 导出设备属性文件

  ```c
  unsigned long id = 0;
  ssize_t xdev_id_show(struct device *dev, struct device_attribute *attr,
                  char *buf)
  {
      return sprintf(buf, "%d\n", id);
  }
  
  ssize_t xdev_id_store(struct device *dev, struct device_attribute *attr,
                  const char *buf, size_t count)
  {
      kstrtoul(buf, 10, &id);
      return count;
  }
  
  
  DEVICE_ATTR(xdev_id, S_IRUSR|S_IWUSR, xdev_id_show, xdev_id_store);
  ```

- 注册设备

  - 调用device_register函数以及device_create_file函数，将上面的设备结构体以及属性文件结构体注册到内核即可。

  ```c
  static __init int xdev_init(void)
  {
      printk("xdev init\n");
      device_register(&xdev);
      device_create_file(&xdev, &dev_attr_xdev_id);
      return 0;
  }
  module_init(xdev_init);
  
  
  static __exit void xdev_exit(void)
  {
      printk("xdev exit\n");
      device_remove_file(&xdev, &dev_attr_xdev_id);
      device_unregister(&xdev);
  }
  module_exit(xdev_exit);
  
  MODULE_AUTHOR("embedfire");
  MODULE_LICENSE("GPL");
  ```

  

- 此处，加载内核模块后，就可以在 `/sys/bus/xbus/devices/` 中多了个设备 `xdev`，它是个链接文件，最终指向了 `sys/devices` 中的设备。

  ![xdev目录](image/xdev.jpg)

  - 切换到 `xdev` 目录下，可以看到自定义的属性文件`xdev_id` 

    ![xdevid文件](image/xdevid.jpg)

  - 通过 echo 或 cat 命令，可以进行修改和查询

    ![修改xdev_id文件](image/exec.jpg)





### 驱动



- 定义新的驱动：

  ```c
  extern struct bus_type xbus;
  
  int xdrv_probe(struct device *dev)
  {
      printk("%s-%s\n", __FILE__, __func__);
      return 0;
  }
  
  int xdrv_remove(struct device *dev)
  {
      printk("%s-%s\n", __FILE__, __func__);
      return 0;
  }
  
  static struct device_driver xdrv = {
      .name = "xdev",
      .bus = &xbus,
      .probe = xdrv_probe,
      .remove = xdrv_remove,
  };
  ```

- 导出驱动属性文件：

  ```c
  char *name = "xdrv";
  ssize_t drvname_show(struct device_driver *drv, char *buf)
  {
      return sprintf(buf, "%s\n", name);
  }
  
  DRIVER_ATTR_RO(drvname);
  ```

- 注册驱动：

  ```c
  static __init int xdrv_init(void)
  {
      printk("xdrv init\n");
      driver_register(&xdrv);
      driver_create_file(&xdrv, &driver_attr_drvname);
      return 0;
  }
  module_init(xdrv_init);
  
  static __exit void xdrv_exit(void)
  {
      printk("xdrv exit\n");
      driver_remove_file(&xdrv, &driver_attr_drvname);
      driver_unregister(&xdrv);
  }
  module_exit(xdrv_exit);
  
  MODULE_AUTHOR("embedfire");
  MODULE_LICENSE("GPL");
  ```

  

成功加载驱动后，可以看到/sys/bus/xbus/driver多了个驱动xdev目录，如图所示：在该目录下存在一个我们自定义的属性文件， 使用cat命令读该文件的内容，终端会打印字符串“xdrv”。

![drivers目录](image/xdrv.jpg)

使用命令 `dmesg | tail` 来查看模块加载过程的打印信息，当我们加载完设备和驱动之后，总线开始进行匹配，执行match函数， 发现这两个设备的名字是一致的，就将设备和驱动关联到一起，最后会执行驱动的probe函数。

![drivers目录](image/dmesg.jpg)













## 总结：

https://doc.embedfire.com/linux/imx6/driver/zh/latest/linux_driver/linux_device_model.html

![设备驱动模型](image/linux_device_model02.png)

在总线上管理着两个链表，分别管理着设备和驱动，当我们向系统注册一个驱动时，便会向驱动的管理链表插入我们的新驱动， 同样当我们向系统注册一个设备时，便会向设备的管理链表插入我们的新设备。在插入的同时总线会执行一个bus_type结构体中match的方法对新插入的设备/驱动进行匹配。 (它们之间最简单的匹配方式则是对比名字，存在名字相同的设备/驱动便成功匹配)。 在匹配成功的时候会调用驱动device_driver结构体中probe方法(通常在probe中获取设备资源，具体的功能可由驱动编写人员自定义)， 并且在移除设备或驱动时，会调用device_driver结构体中remove方法。

> 注册驱动或设备时，通过bus_type 中的 match 方法匹配设备和驱动的名称，匹配成功就执行 device_driver 中的 probe，移除设备或驱动的时候就执行 device_driver 中的 remove







# Linux 启动的时候如何获取到设备树所在内存中的地址

- Linux 启动是从 `init/main.c` 中的 `start_kerken` 函数启动的

  - 执行 `arch` 相关的初始化 `setup_arch(&command_line)`

  - 获取 设备树在内存中的地址 `setup_machine_fdt(__atags_pointer)`，其中 `_atags_pointer` 就是保存着设备树在内存中的地址

    - `__atags_points` 的值是通过 `arch/arm/kernel/head-common.S` 进行传递的，其中使用 r2 保存着这个地址

    - 将 `r2` 的地址拷贝到 `r6` 所在的地址，而 `r6` 所在的地址就是 `__atags_pointer` 

      ```assembly
      	str	r2, [r6]			@ Save atags pointer
      	cmp	r7, #0
      	strne	r0, [r7]			@ Save control register values
      	b	start_kernel
      ENDPROC(__mmap_switched)
      
      	.align	2
      	.type	__mmap_switched_data, %object
      __mmap_switched_data:
      	.long	__data_loc			@ r4
      	.long	_sdata				@ r5
      	.long	__bss_start			@ r6
      	.long	_end				@ r7
      	.long	processor_id			@ r4
      	.long	__machine_arch_type		@ r5
      	.long	__atags_pointer			@ r6
      ```

  - 由于系统启动到 kernel 阶段，`mmu` 已开启，需要将这个物理地址转换成虚拟地址，并将这块虚拟内存预留下来，防止被破坏

    ```c
    setup_machine_tags(phys_addr_t __atags_pointer, unsigned int machine_nr)
    {
    	/* xxxxxx */
    	if (__atags_pointer)
    		tags = phys_to_virt(__atags_pointer);
    }
    
    phys_addr_t __init_memblock memblock_phys_mem_size(void)
    {
    	return memblock.memory.total_size;
    }
    
    
    const struct machine_desc * __init setup_machine_fdt(unsigned int dt_phys)
    {
    	/* 把虚拟地址转换城物理地址 */
    	if (!dt_phys || !early_init_dt_verify(phys_to_virt(dt_phys)))
    		return NULL;
    
    	mdesc = of_flat_dt_match_machine(mdesc_best, arch_get_next_mach);
    
    
    	/* 扫描设备树节点 */
    	early_init_dt_scan_nodes();
    
    	/* Change machine number to match the mdesc we're using */
    	__machine_arch_type = mdesc->nr;
    
    	return mdesc;
    }
    
    ```
  
  - 然后扫描设备树节点，包括 `chosen、root、memory` 
  
    ```c
    void __init early_init_dt_scan_nodes(void)
    {
    	/* Retrieve various information from the /chosen node */
    	of_scan_flat_dt(early_init_dt_scan_chosen, boot_command_line);
    
    	/* Initialize {size,address}-cells info */
    	of_scan_flat_dt(early_init_dt_scan_root, NULL);
    
    	/* Setup memory, calling early_init_dt_add_memory_arch */
    	of_scan_flat_dt(early_init_dt_scan_memory, NULL);
    }
    ```
  
  - 创建`device_nodes` 结构，扫描设备树的头(设备树的地址已经通过.s拿到了)，获取设备树大小、调用 `unflaten_dt_nodes --> populate_node`创建节点
  
  - `device_node` 转换成 `platform_device` 结构，设备树替换了平台总线模型当中对硬件描述的device部分。
  
    - 根节点下所有包含 `compatible` 属性的子节点会转换成 `platform_device`









# Nor、Nand Flash Linux 驱动框架分析

- 平台总线(platform_bus)：Linux 系统内核使用`bus_type`结构体表示总线，由于`SOC`中有些外设没有总线这个概念，但是又要使用总线，所以虚拟出来了一个平台总线。
- SPI 总线：spi_bus



## 串行 Nor 

- 设备：通过 `struct spi_board_info` 结构体的方式来描述设备硬件信息
- 驱动：通过 `struct spi_driver` 结构体的方式来描述驱动
- 总线：挂载在 `SPI` 总线下，使用 `SPI` 总线的 `match` 规则匹配设备和驱动，匹配成功后执行驱动的`probe` 函数



### 设备注册过程：

1.  注册 `struct spi_board_info` 结构体，描述设备相关的信息
2. 使用接口 `spi_register_board_info` 函数来注册 `SPI 设备` 
   - 匹配`spi_master->bus_num` 和 `spi_board_info->bus_num`，如果相同则调用 `spi_new_device` 接口来根据 `struct spi_board_info`结构体描述的信息来注册`SPI 设备`
     - 调用 `spi_alloc_device`来申请一块内存空间，大小是 `struct spi_device`
     - 将`struct spi_board_info`结构描述的信息都赋值为`struct spi_device`
     - 调用 `spi_add_device`来注册设备
       - 调用`spi_dev_set_name` 来配置设备的名称
       - 执行 `spi_setup`函数
       - 执行`device_add`来注册设备
         - 实质上是通过`kobject_add`的方式来注册了一个设备，父设备应该是 `spi`



### 驱动注册过程：

1. 注册`struct spi_driver`结构体，描述驱动相关的信息
2. 使用接口`spi_register_driver`函数来注册`SPI 驱动`
   - 将驱动的总线配置为 `spi_bus_type`
   - 配置`probe、remove、shutdown` 的函数
   - 调用`driver_register`函数来注册驱动
     - 调用`driver_find`函数根据总线和驱动来查看当前驱动是否已被注册
     - 调用`bus_add_driver`函数来为总线注册驱动
     - 调用`driver_add_groups`添加一些驱动的辅助信息



### 总线匹配过程：

- 总线匹配通过`SPI 总线的 match`匹配规则进行匹配

  ```c
  struct bus_type spi_bus_type = {
  	.name		= "spi",
  	.dev_groups	= spi_dev_groups,
  	.match		= spi_match_device,
  	.uevent		= spi_uevent,
  };
  EXPORT_SYMBOL_GPL(spi_bus_type);
  ```

  - 共有4种方式进行匹配，目前采用的是设备的`modalias`属性和驱动的`name`进行匹配

    ```c
    static int spi_match_device(struct device *dev, struct device_driver *drv)
    {
    	const struct spi_device	*spi = to_spi_device(dev);
    	const struct spi_driver	*sdrv = to_spi_driver(drv);
    
    	/* Attempt an OF style match */
    	if (of_driver_match_device(dev, drv))
    		return 1;
    
    	/* Then try ACPI */
    	if (acpi_driver_match_device(dev, drv))
    		return 1;
    
    	if (sdrv->id_table)
    		return !!spi_match_id(sdrv->id_table, spi);
    
    	return strcmp(spi->modalias, drv->name) == 0;
    }
    ```

    

### 分区表创建过程：

- 采用 `cmdlinepart` 的方式来描述分区表，调用`parse_mtd_partitions`解析分区表
  - 先设置有几种解析分区表的类型，有`ofpart、cmdlinepart`，目前采用的是`cmdlinepart`
  - 调用函数`mtd_part_parset_get`获取解析什么类型的分区表
  - 调用类型的解析回调函数`*parser->parse_fn --> parse_cmdline_partitions` 
    - 调用`mtdpart_setup_real`解析`cmdline`
      - 先找到`:`号，然后使用 `newpart`函数来根据`cmdline`后面的部分创建分区描述信息
        - 按照分区表的描述要求解析描述分区表的字符串，依次循环解析每个分区，malloc 一块数据用于存放分区表，分区表的信息就来源于刚刚循环解析的数据，然后把这个分区表返回回去
      - 把创建好的分区表绑定到当前`mtd`设备上
    - 重新把分区表的偏移和大小组建一下，返回分区个数
- 调用`add_mtd_partitions`将解析出来的分区表依次创建成分区
  - 目前已经有了分区表，根据分区表创建分区即可
  - 根据分区的个数，分配足够大的分区空间
  - 把`struct mtd_info`的信息都配置到 `struct mtd_part`分区上，包括一些函数指针和信息
  - 把创建好的分区都链接到`mtd_partitions`链表上
  - 调用`add_mtd_device`创建`mtd`设备
  - 调用`mtd_add_partition_attrs`添加一些分区表的描述信息





## 串行 Nand

### 设备注册过程

- 使用设备树的方式创建设备，驱动文件中只需要放`compatible`属性来跟设备树进行匹配
- 设备树放在 `struct spi_driver  --> driver.of_match_table` 中描述



### 驱动注册过程

- 驱动采用 `module_spi_driver`的方式注册
  - `module_spi_driver` 的实质也是`spi_register_driver`跟上面 `nor` 的注册过程一致

### 总线匹配过程

- 总线匹配方式也是通过`SPI总线`进行匹配
  - 此次匹配是通过设备树的方式进行匹配设备与驱动的

### 分区表创建过程

- 调用 `mtd_device_parse_register` 的方式来解析`cmdline`并创建分区表
- 跟`nor`的解析过程一致





## 并行 Nand

- 这里只用到了 平台总线下的设备和驱动匹配，不像 串行Nor和串行Nand，SPI控制器驱动是通过平台总线下的设备和驱动匹配，然后FLASH驱动是通过SPI总线下的设备和驱动匹配。

### 设备注册过程

- 采用 设备树 的方式来注册设备



### 驱动注册过程

- 驱动采用 `module_platform_driver` 的方式注册，因为这个设备不是挂载在 `SPI 总线`下，所以要用 `platform 总线`



### 总线匹配过程

- 通过`platform`平台下的`match`方式来进行匹配

  ```c
  struct bus_type platform_bus_type = {
  	.name		= "platform",
  	.dev_groups	= platform_dev_groups,
  	.match		= platform_match,
  	.uevent		= platform_uevent,
  	.pm		= &platform_dev_pm_ops,
  };
  EXPORT_SYMBOL_GPL(platform_bus_type);
  ```

- 匹配规则有 5种，目前 驱动采用的是设备树的方式进行匹配，匹配设备树的`compatible`信息和驱动的`of_match_table->compatible`

  ```c
  static int platform_match(struct device *dev, struct device_driver *drv)
  {
  	struct platform_device *pdev = to_platform_device(dev);
  	struct platform_driver *pdrv = to_platform_driver(drv);
  
  	/* When driver_override is set, only bind to the matching driver */
  	if (pdev->driver_override)
  		return !strcmp(pdev->driver_override, drv->name);
  
  	/* Attempt an OF style match first */
  	if (of_driver_match_device(dev, drv))
  		return 1;
  
  	/* Then try ACPI style match */
  	if (acpi_driver_match_device(dev, drv))
  		return 1;
  
  	/* Then try to match against the id table */
  	if (pdrv->id_table)
  		return platform_match_id(pdrv->id_table, pdev) != NULL;
  
  	/* fall-back to driver name match */
  	return (strcmp(pdev->name, drv->name) == 0);
  }
  ```



### 分区表创建过程：

- 与串行 Nand 过程一致











# CPU 的指令执行和指令退休

- 指令执行：指令执行是CPU的一个主要阶段，它是指计算机在执行程序时，CPU逐条执行指令的过程。在指令执行阶段，CPU从内存或缓存中读取指令，并根据指令的操作码和操作数执行相应的操作。这些操作可能包括算术运算、逻辑运算、数据传输、条件分支等。指令执行是CPU完成任务的基本操作。
- 指令退休：指令退休是CPU中的另一个重要阶段，它是指完成指令执行并将指令结果提交到下一级流水线或<font color=red>回写寄存器文件</font>的过程。在流水线处理器中，指令执行通常是以流水线的形式进行的，即同时处理多条指令，而指令退休是值完成指令执行后，将指令的结果正确<font color=red>写回到目标寄存器或内存中</font>，确保程序的正确执行。
- 指令可能执行了，但是还没退休，就是没有把数据或指令结果写到寄存器或内存中。





# 现代处理器体系结构

- 传统的体系结构包括：冯诺依曼、哈佛

  - 哈佛：指令和数据分开存储，使用不同的数据线和地址线
  - 冯诺依曼：指令和数据一起存储，使用相同的数据线和地址线

- 现代处理器：

  ```
  
  |------------|
  |			 |
  |            |
  |  CPU       |   <---->   I-cache           RAM            Flash
  |            |   <---->  
  |            |
  |            |   <---->
  |      |-----|   <---->   D-cache        
  |      |-ROM-|
  |------------|
  
  
  
  
  ```






# Nor、Nand 读写擦基本单位

- Nor 的读写基本单位可以不是页的开头，也就是不是 256 对齐
  - 擦是按照 64k 擦除的(block) 或者使用 sector 擦除 4k
- Nand 的读写基本单位必须从页的开头读
  - 擦除的基本单位是 Block，0x2000，128k



# Nor 、Nand bin 构建方式

- 执行 `./build <chip> <board> <mode>` 即可编译出对应的 bin

- 把所有的文件都编译成.o，然后根据链接脚本链接好，链接好之后的bin，在头部添加 Nor、Nand 的标识，如果是 canopus 会生成三个 bin，一个原始的 bin，一个 pagesize 为2k 然后加上 ecc 的bin，一个 pagesize 为 4k 然后加上 ecc 的bin，然后重新组成好 bin

- ROM 启动的时候会去读 4k 的代码到 sram，校验头部标识，然后就开始执行 stage1 的代码<font color=red>(通过链接脚本指定让哪些文件放置在前 4k)</font>



```makefile
cp conf/canopus/3215B/boot.config .config
make clean
make
	all: checkcfg mkdir include/version_autogenerated.h $(EXTERN_PARAM_DEST_FILE) $(TARGET) checkbin


TARGET += loader.bin
TARGET += loader-flash.bin

checkcfg：Makefile

mkdir：
	@mkdir -p output
	
include/version_autogenerated.h:
	@echo "/*" > $@
	@echo " * Boot Loader Version" >> $@
	@echo " *" >> $@
	@echo " * This file is automatically generated from Makefile" >> $@
	.....

loader.bin : loader.elf
	@$(OBJCOPY) -Obinary $< $@
	@chmod a-x $@
	@mv $@ output/

loader.elf: $(COBJS-y) $(SOBJS-y) $(EXTRA-OBJS)
	@-rm -f $(LD_FILE_NAME)
	@$(CC) $(CPPFLAGS) -E -x assembler-with-cpp -P -o $(LD_FILE_NAME) $(LD_FILE)
ifeq ($(ENABLE_DENALI_ELF), y)
	@$(LD) $(LDFLAGS) -T$(LD_FILE_NAME) -o $@ $^ $(LIBS) $(LIBGCC)
	
	
##########################################
#######   至此 loader.elf 已生成    #######
##########################################
loader-flash.bin : loader.elf
	mv loader-flash.bin loader-sflash.bin
ifeq ($(ENABLE_SPIFLASH), y)
	@gcc tools/loader_write.c -o loader_write
else
ifeq ($(ENABLE_BOOT_TOOL), y)
	@gcc tools/loader_write.c -o loader_write
else
	@gcc tools/nand_ecc_cal.c -o loader_nand_ecc
endif
endif

nor:
	./loader_write loader-sflash.bin output/loader-sflash.bin $(STAGE1_DATA_START) $(ROM_COPY_SIZE) $(STAGE1_COPY_COUNT)
nand:
	cp -f loader-nand.bin output/loader-nand-noecc.bin
	./loader_nand_ecc -i loader-nand.bin -o output/loader-nand.bin -s $(STAGE1_DATA_START) -l $(ROM_COPY_SIZE) -c $(STAGE1_COPY_COUNT) -p 0x800
	./loader_nand_ecc -i loader-nand.bin -o output/loader-nand-4k.bin -s $(STAGE1_DATA_START) -l $(ROM_COPY_SIZE) -c $(STAGE1_COPY_COUNT) -p 0x1000

```



## gdb load .elf 的过程

- 在链接的时候根据链接脚本把所有的程序都链接成一个 .elf，链接的地址在 ddr 内
- gdb 执行 load 这个 .elf 的时候是在 ddr 中开始跑的，也就是链接脚本中指定的链接地址
- 那么这个时候 ddr 是在哪里初始化的？
  - 在跑 .elf 的时候都需要先烧录一个能启动的 loader，相当于是把 ddr 配好了，然后 gdb 来接管 cpu





# rom

- 


