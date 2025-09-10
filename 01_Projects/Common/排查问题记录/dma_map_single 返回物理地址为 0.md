### Linux DMA 疑难问题调试案例：从 `dma_map_single` 返回错误到定位根因

#### 1. 问题背景

在 `arm64` 架构平台上，为 DesignWare SPI 控制器驱动 `spi-dw-gx-v2.c` 添加并调试 DMA 功能时，遇到了一个棘手的问题。

- **平台与架构:** ARM64
    
- **驱动程序:** `spi-dw-gx-v2.c`
    
- **核心问题函数:** `dw_half_duplex_dma`
    
- **直接现象:** 在进行 DMA 读写操作时，调用 `dma_map_single()` 后返回的 `dma_addr_t` 句柄值为 0。紧接着调用 `dma_mapping_error()` 返回 `true`，驱动程序因此判断 DMA 映射失败，返回 `-ENOMEM` 错误，导致整个 SPI 传输失败。
    

```
// spi-dw-gx-v2.c
// ...
dma_handle = dma_map_single(..., dws->buffer, ...);
pr_info("phy_address:0x%llx", dma_handle); // 打印出 dma_handle 为 0

if (dma_mapping_error(..., dma_handle)) {
    // 此处条件成立，进入错误处理流程
    return -ENOMEM;
}
```

#### 2. 初步排查与错误的假设

面对 DMA 映射失败，调试过程首先从最常见的几个原因入手。

##### 假设 1: 32位 DMA 地址寻址限制

- **线索:**
    
    1. 硬件代码 `dw_writel(dws, SPIM_AXIAR0, (uint32_t)dma_handle);` 暗示了该 SPI 控制器的 AXI 接口可能只能处理 32 位的物理地址。
        
    2. `arm64` 系统可以轻松管理超过 4GB 的物理内存。内核的内存分配器 (`kmalloc`) 完全可能从高于 4GB 的物理内存区域（高位内存）分配 DMA 缓冲区。
        
- **怀疑:** 驱动程序没有向内核声明其硬件只能进行 32 位寻址的限制。当内核分配了一个位于高位内存的缓冲区后，`dma_map_single` 尝试为其创建映射，但由于地址超出了硬件的能力范围（> 32位），映射失败并返回 0。
    
- **尝试的解决方案:** 在驱动的 `probe` 函数中，调用 `dma_set_mask_and_coherent()` 明确告知内核该设备只支持 32 位的 DMA 地址。
    
    ```
    // dw_spi_v2_probe()
    ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
    if (ret) {
        // ... 错误处理
    }
    ```
    
    这个操作的目的是让内核 DMA 子系统在遇到高位内存的缓冲区时，能自动启用“Bounce Buffer”机制（在低 4GB 内存中分配一个临时中转缓冲区）。
    
- **结果:** **问题依旧。** 即使设置了 DMA mask，`dma_map_single` 返回的 `dma_handle` 仍然是 0。这说明问题的原因比预想的更深层。
    

##### 假设 2: IOMMU / SMMU 配置问题

- **线索:** 在现代 `arm64` SoC 中，IOMMU（在 ARM 平台常被称为 SMMU）是标准的硬件组件，负责管理所有外设的 DMA 请求。
    
- **怀疑:** 当系统中存在并启用了 IOMMU 时，如果一个设备没有在设备树 (Device Tree) 中通过 `iommus` 属性被正确地注册到 IOMMU 的管理之下，IOMMU 驱动会拒绝为该设备创建任何地址映射，导致 `dma_map_single` 返回失败。
    
- **分析:** 这是一个非常常见且合理的怀疑方向。但在本案中，进一步的调试发现，流程甚至没有正确地走到与 IOMMU 驱动交互的那一步。一个更基础的错误提前导致了失败。
    

#### 3. 关键突破：定位到错误的设备实例 (`struct device`)

调试过程的转折点来自于一个关键的观察：**系统其他部分在进行 DMA 操作时，使用的是有效的 `dma_map_ops`（如 `arm64_swiotlb_dma_ops`），而 `spi-dw-gx-v2.c` 在调用 `dma_map_single` 时，最终却使用了 `dummy_dma_ops`。**

`dummy_dma_ops` 是内核中的一个“空壳”操作集，它的所有函数都不执行任何有效操作，只会立即返回错误。这解释了为什么 `dma_handle` 总是 0。

- **根因分析:** 为什么会用到 `dummy_dma_ops`？答案在于驱动中存在**两个**不同的 `struct device` 实例，而代码错误地使用了未经正确初始化的那一个。
    
    1. **`platform_device->dev` (`pdev->dev`)**: 这个 `struct device` 代表了在设备树中描述的**物理平台设备**。当内核的平台总线在启动时匹配到该设备时，会创建它并为其**正确地设置好 DMA 操作函数集** (`dma_ops`)。这个 `dma_ops` 指向 `arm64_swiotlb_dma_ops` 或 IOMMU 提供的 `ops`。
        
    2. **`spi_master->dev` (`dws->master->dev`)**: 这个 `struct device` 代表的是一个**逻辑上的 SPI 主控制器功能**。它是在驱动的 `probe` 函数中，通过调用 `spi_alloc_master()` 创建的一个全新的、独立的**子设备**。**关键在于，这个新创建的子设备的 `dma_ops` 字段在初始化时是 `NULL`！**
        
- **错误的调用链:**
    
    1. 驱动代码错误地将 `&dws->master->dev` 传入了 `dma_map_single()`。
        
    2. `dma_map_single()` 内部调用 `get_dma_ops(&dws->master->dev)` 来获取操作函数集。
        
    3. `get_dma_ops()` 发现 `dws->master->dev.dma_ops` 是 `NULL`。
        
    4. 为防止内核因空指针解引用而崩溃，`get_dma_ops()` 返回一个全局的、安全的 `dummy_dma_ops` 作为后备。
        
    5. 最终，`dummy_dma_ops` 中的 `map_page` 函数被调用，直接返回了 0。
        

#### 4. 最终解决方案与验证

既然定位到了根因，解决方案就变得非常清晰：**DMA 操作必须使用那个代表物理硬件、且已被内核正确配置过 DMA 能力的 `struct device` 实例。**

- **正确实现:** 如何在 `dw_half_duplex_dma` 函数中获取到正确的 `pdev->dev` 呢？Linux 设备模型提供了标准的父子关系指针。 当 `spi_alloc_master(&pdev->dev, ...)` 被调用时，`pdev->dev` 被自动设置为了新创建的 `master->dev` 的**父设备**。 因此，正确的 `struct device` 实例可以通过 `dws->master->dev.parent` 来访问。
    
- **最终代码修复:** 将 `dw_half_duplex_dma` 函数中所有 `dma_*` 系列 API 的第一个参数，由 `&dws->master->dev` 修改为 `dws->master->dev.parent`。
    
    ```
    --- a/spi/spi-dw-gx-v2.c
    +++ b/spi/spi-dw-gx-v2.c
    @@ -671,15 +671,15 @@
     ...
                    page_ofs  = offset_in_page (dws->buffer);
                    page_left = PAGE_SIZE - page_ofs;
                    block_len = min_t (uint32_t, block_len, page_left);
    -               dma_handle = dma_map_page (&dws->master->dev, vm_page, \
    +               dma_handle = dma_map_page (dws->master->dev.parent, vm_page, \
                            page_ofs, block_len, DMA_FROM_DEVICE);
                } else {
                    pr_info ("%s %d\n", __func__, __LINE__);
    -               dma_handle = dma_map_single (&dws->master->dev, dws->buffer, \
    +               dma_handle = dma_map_single (dws->master->dev.parent, dws->buffer, \
                            block_len, DMA_FROM_DEVICE);
                }
    
    -           if (dma_mapping_error (&dws->master->dev, dma_handle)) {
    +           if (dma_mapping_error (dws->master->dev.parent, dma_handle)) {
                    pr_info ("%s %d\n", __func__, __LINE__);
                    return -ENOMEM;
                }
    ```
    
- **验证与参考:** 通过分析另一个行为正确的驱动 `spi-zynqmp-gqspi.c`，可以印证该方案的正确性。该驱动采取了另一种更清晰的实现方式：在 `probe` 函数中，就将 `&pdev->dev` 指针显式地保存到其私有数据结构中 (`xqspi->dev = &pdev->dev;`)，并在后续所有 DMA 操作中始终使用这个保存好的指针。这两种方法殊途同归，都遵循了同一个核心原则。
    

#### 5. 案例总结与启示

1. **核心教训:** 在编写涉及父子设备关系（如 platform_device 与 spi_master）的 Linux 驱动时，必须严格区分哪个 `struct device` 实例代表**物理硬件**（通常是父设备），哪个代表**逻辑功能**（通常是子设备）。所有与硬件直接交互的操作（尤其是 DMA），都必须使用代表物理硬件的设备实例。
    
2. **关键调试技巧:** 当遇到 `dma_map_single` 返回 0 或失败时，排查路径应为：
    
    - **第一层：** 检查传入 `dma_map_single` 的 `dev` 参数是否正确。通过 `gdb` 或 `printk` 确认 `get_dma_ops(dev)` 返回的是预期的 `dma_map_ops`，而不是 `dummy_dma_ops`。
        
    - **第二层：** 检查 DMA mask 是否已根据硬件能力正确设置 (`dma_set_mask_*`)。
        
    - **第三层：** 检查设备树中 IOMMU/SMMU 的相关配置 (`iommus` 属性) 是否正确。
        
3. **驱动设计最佳实践:** 为了避免混淆，推荐在 `probe` 函数中，就将正确的平台设备指针 (`&pdev->dev`) 缓存到驱动的私有数据结构中。这种做法明确了依赖关系，使得后续代码更清晰、更健壮。


# Linux4.9. y 为什么没有此问题
- 4.9. y 时传的 device 也是错的，但是由于 arm 架构的代码，导致默认可以找到 arm_dma_ops 使用，所以没问题
- arm64 由于多了一个 dummy_dma_ops，所以传入错的 device 时，会导致拿到 dummy_dma_ops 来使用
- dma_ops 何时配置给 platform_device->dev?
	- platform_bus_type 有一个回调用于配置 dma_ops
	- 在系统启动时，device_add 时会调用该回调，所以 platform_device->dev 就和 dma_ops 绑上了


```c
//arm：

static struct dma_map_ops *arm_get_dma_map_ops(bool coherent)
{
	return coherent ? &arm_coherent_dma_ops : &arm_dma_ops;
}



//arm64：

int of_dma_configure(struct device *dev, struct device_node *np, bool force_dma)
{
	u64 dma_addr, paddr, size = 0;
	arch_setup_dma_ops(dev, dma_addr, size, iommu, coherent);

	return 0;
}

#define arch_setup_dma_ops	arch_setup_dma_ops

void arch_setup_dma_ops(struct device *dev, u64 dma_base, u64 size,
			const struct iommu_ops *iommu, bool coherent)
{
	if (!dev->dma_ops) {
		if (dev->removed_mem)
			set_dma_ops(dev, &removed_dma_ops);
		else
			dev->dma_ops = &arm64_swiotlb_dma_ops;
	}

	dev->archdata.dma_coherent = coherent;
	arm_iommu_setup_dma_ops(dev, dma_base, size);

#ifdef CONFIG_XEN
	if (xen_initial_domain()) {
		dev->archdata.dev_dma_ops = dev->dma_ops;
		dev->dma_ops = xen_dma_ops;
	}
#endif
}


static inline const struct dma_map_ops *get_dma_ops(struct device *dev)
{
    if (dev && dev->dma_ops)
        return dev->dma_ops;
    return get_arch_dma_ops(dev ? dev->bus : NULL);
}

static inline const struct dma_map_ops *get_arch_dma_ops(struct bus_type *bus)
{
    /*
     * We expect no ISA devices, and all other DMA masters are expected to
     * have someone call arch_setup_dma_ops at device creation time.
     */
    return &dummy_dma_ops;
}
```