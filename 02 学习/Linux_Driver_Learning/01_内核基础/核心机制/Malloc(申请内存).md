---
tags:
  - Linux_Driver/Kernel_Fundamentals
aliases: []
---
# Malloc

#### 1. 概述与首选建议

Linux 内核提供了多种内存分配 API，适用于不同场景：
- **小块内存**: `kmalloc` 或 `kmem_cache_alloc` 系列。
- **大块虚拟连续内存**: `vmalloc` 系列。
- **直接分配物理页**: `alloc_pages`。
- **更专业的分配器**: `cma_alloc` 或 `zs_malloc`。

大多数分配 API 都使用 **GFP (Get Free Pages) 标志** 来控制分配行为。

尽管 API 众多，但在绝大多数情况下，你的首选应该是：
```c
kzalloc(<size>, GFP_KERNEL);
```
`kzalloc` 不仅分配内存，还会将其清零，更加安全。

---

#### 2. GFP (Get Free Page) 标志详解

GFP 标志控制着分配器的行为，例如使用哪个内存域、尝试获取内存的努力程度、是否允许用户空间访问等。

**常用 GFP 标志推荐用法：**

| 标志 (Flag) | 推荐使用场景 | 核心特性 |
| :--- | :--- | :--- |
| `GFP_KERNEL` | **绝大多数情况下的首选**。用于内核数据结构、DMA 内存、inode 缓存等。 | 允许睡眠，意味着在内存压力下可能会触发直接内存回收 (direct reclaim)。 |
| `GFP_NOWAIT` | **原子上下文**，如中断处理程序、软中断、Tasklet 等。 | **不允许睡眠**。不进行直接内存回收、IO 或文件系统操作。在内存紧张时**极易失败**，调用者必须能处理分配失败的情况。 |
| `GFP_ATOMIC` | 与 `GFP_NOWAIT` 类似，但可以用在**需要不惜一切代价获取内存**的场景。 | **不允许睡眠**。可以访问为紧急情况保留的内存，比 `GFP_NOWAIT` 成功率稍高。 |
| `GFP_KERNEL_ACCOUNT` | **由用户空间触发的、需要进行内核内存记账的分配**。 | `GFP_KERNEL` 的变种，增加了 `__GFP_ACCOUNT` 标志位。 |
| `GFP_USER` | 用于**用户空间**的分配。 | 内存**不可移动**，且**必须**能被内核直接访问。 |
| `GFP_HIGHUSER` | 用于**用户空间**的分配。 | 内存**不可移动**，但**不要求**能被内核直接访问。 |
| `GFP_HIGHUSER_MOVABLE` | 用于**用户空间**的分配。 | 内存**可移动**，也**不要求**能被内核直接访问。这是限制最少的用户空间标志。 |

**历史遗留标志 (应避免使用):**

* `GFP_NOIO` / `GFP_NOFS`: 曾用于防止文件系统或 IO 路径中的递归死锁。自内核 4.12 版本起，已有新的作用域 API (scope APIs) 来解决此问题，应优先使用新方法。
* `GFP_DMA` / `GFP_DMA32`: 用于为有寻址能力限制的旧硬件（如只能访问低端 16MB 或 4GB 内存的设备）分配内存。现在应优先使用 `dma_alloc*` 系列 API。

---

#### 3. GFP 标志与内存回收 (Reclaim) 行为

GFP 标志直接影响内存分配器尝试回收内存的“努力程度”。

| 标志组合 | 回收行为描述 |
| :--- | :--- |
| `GFP_KERNEL & ~__GFP_RECLAIM` | **最轻量级**。完全不尝试释放内存，甚至不唤醒后台回收进程。可能会快速耗尽内存。 |
| `GFP_KERNEL & ~__GFP_DIRECT_RECLAIM` (或 `GFP_NOWAIT`) | **乐观分配**。不进行直接回收，但如果内存水位低于 low watermark，会唤醒 `kswapd` 进行后台回收。适用于原子上下文或性能优化路径。 |
| `(GFP_KERNEL\|__GFP_HIGH)` (即 `GFP_ATOMIC`) | **不睡眠的分配**。带有昂贵的备用策略，可以动用部分内存保留区。 |
| `GFP_KERNEL` | **默认行为**。允许后台回收和直接回收。对于不大的内存请求，基本不会失败，但调用者仍需检查失败情况。 |
| `GFP_KERNEL \| __GFP_NORETRY` | **尽早失败**。覆盖默认行为，只进行一轮回收尝试，如果失败则立即返回，不会调用 OOM Killer。 |
| `GFP_KERNEL \| __GFP_RETRY_MAYFAIL` | **努力尝试**。会非常努力地进行回收，直到回收无法取得任何进展时才会失败。不会调用 OOM Killer。 |
| `GFP_KERNEL \| __GFP_NOFAIL` | **永不失败**。会无限循环地尝试，直到分配成功。**非常危险**，尤其对于大块内存请求，可能导致系统卡死。 |

---

#### 4. 如何选择内存分配器

| 分配器类型 | 适用场景 | 核心特性 | 释放函数 |
| :--- | :--- | :--- | :--- |
| **`kmalloc` / `kzalloc`** | **最常用**。分配**小于页大小**的**物理连续**内存。 | 分配速度快。最大分配尺寸受限。地址对齐有保证。 | `kfree` / `kvfree` |
| **`kcalloc` / `kmalloc_array`** | 分配**数组**。 | 内部处理了 size * count 的溢出检查，更安全。 | `kfree` / `kvfree` |
| **`krealloc` / `krealloc_array`** | **重新调整**已分配内存的大小。 | - | `kfree` / `kvfree` |
| **`vmalloc` / `vzalloc`** | 分配**大块**内存。 | 分配的内存**虚拟地址连续**，但**物理地址不一定连续**。 | `vfree` / `kvfree` |
| **`kvmalloc`** | 不确定用 `kmalloc` 还是 `vmalloc` 时。 | **智能选择**。会先尝试 `kmalloc`，如果失败（通常因为尺寸太大），则自动用 `vmalloc` 重试。 | `kvfree` |
| **Slab Allocator** (`kmem_cache_create`, `kmem_cache_alloc`) | 需要频繁分配和释放**大量相同大小的对象**。 | 效率高，可以减少内存碎片。需要先创建缓存 (`kmem_cache`)。 | `kmem_cache_free`, `kfree`, `kvfree` |

**重要提示：**
* 使用 `struct_size()`, `array_size()` 等辅助宏来安全地计算分配大小，防止整数溢出。
* 所有分配的内存，在不再需要时**必须**被释放，以避免内存泄漏。
* `kvfree` 是一个方便的通用释放函数，它可以正确处理由 `kmalloc`、`vmalloc` 或 `kvmalloc` 分配的内存。
