


## 测试1：关掉 CONFIG_BLK_DEV_NVME=y
![[Pasted image 20241120165756.png]]



## 测试2：关掉 DMA 相关的
![[Pasted image 20241120170314.png]]

## 测试3：关掉 PCI 相关的

ok

## 测试4：更换 rootfs，不换 kernel

![[Pasted image 20241120171834.png]]



## 测试5：关掉 pci 驱动中的 edma 初始化以及 spi 切换成用 cpu 模式
- 依然是 `Receive FIFO overflow`

## 测试6：spi 切成 cpu 模式，打开 spi 中断的方式取数据



## 测试7：loader 使用四倍速
- 依然是 `Receive FIFO overflow`


## 测试8：用默认的 flash.bin
ok
包括 pcie 的初始化

## 测试9：用默认的 flash.bin 只换 rootfs
ok
包括 pcie 的初始化


## 测试10：用默认的 flash.bin 换了 rootfs，用最新的 linux kernel
编译 kernel 使用 `./build virgo nfs`
出现了 `Receive FIFO overflow`

## 测试11：去掉 edma，spi 用 cpu 读
- 依然 `Receive FIFO overflow`
```diff
-CONFIG_DW_EDMA=y
-CONFIG_DW_EDMA_PCIE=y
+CONFIG_DW_EDMA=n
+CONFIG_DW_EDMA_PCIE=n
 # CONFIG_NATIONALCHIP_TIMER is not set
 CONFIG_RESET_CONTROLLER=y
 CONFIG_ANDROID=y
diff --git a/drivers/pci/host/pcie-designware.c b/drivers/pci/host/pcie-designware.c
index 742769935..7ea053947 100644
--- a/drivers/pci/host/pcie-designware.c
+++ b/drivers/pci/host/pcie-designware.c
@@ -965,9 +965,9 @@ int dw_pcie_host_init(struct pcie_port *pp)
                pp->num_viewport = 2;
 
        dw_pcie_version_detect(pp);
-       ret = dw_pcie_edma_detect(pp);
-       if(ret)
-               goto error;
+//     ret = dw_pcie_edma_detect(pp);
+//     if(ret)
+//             goto error;
 
        if (IS_ENABLED(CONFIG_PCI_MSI)) {
                if (!pp->ops->msi_host_init) {
diff --git a/drivers/spi/spi-dw-gx-v2.c b/drivers/spi/spi-dw-gx-v2.c
index a51bc2145..515b92237 100644
--- a/drivers/spi/spi-dw-gx-v2.c
+++ b/drivers/spi/spi-dw-gx-v2.c
@@ -683,9 +683,9 @@ static int dw_pump_transfers(struct dw_spi *dws)
        dw_writel(dws, SPIM_CTRLR1, 0);
        dw_writel(dws, SPIM_DMACR,  0);
 
-       if (dws->hw_version >= 0x3130332a)
-               return dw_half_duplex_dma(dws);
-       else
+//     if (dws->hw_version >= 0x3130332a)
+//             return dw_half_duplex_dma(dws);
+//     else
                return dw_half_duplex_poll_transfer(dws);
```

## 测试 12：在 11 的基础上，关掉 CONFIG_PCI_REALLOC_ENABLE_AUTO、CONFIG_BLK_DEV_NVME

- 仅更新 uImage，无法启动
	- 需要将 dtb 和 uImage cat 到一起烧到 kernel 分区
- 依旧 Receive FIFO Overflow

## 测试13：在 12 的基础上，如下修改：
```
CONFIG_HOTPLUG_PCI_PCIE=n
CONFIG_PCIE_DPC=n
CONFIG_PCIE_PTM=n
CONFIG_PCI_MSI=n
CONFIG_HOTPLUG_PCI=n
```

- 测试能够成功启动

## 测试14：在 13 的基础上，如下修改：
```
CONFIG_HOTPLUG_PCI_PCIE=n
CONFIG_PCIE_DPC=y
CONFIG_PCIE_PTM=y
CONFIG_PCI_MSI=y
CONFIG_HOTPLUG_PCI=n
```
- 出现 `Receive FIFO overflow`

## 测试15：在 14 的基础上，如下修改：
```
CONFIG_HOTPLUG_PCI_PCIE=n
CONFIG_PCIE_DPC=y
CONFIG_PCIE_PTM=y
CONFIG_PCI_MSI=n
CONFIG_HOTPLUG_PCI=n
```
- 可以正常启动

## 测试16：只关闭 `CONFIG_PCI_MSI`
```
CONFIG_PCI_MSI=n
```
- 可以正常启动

## 测试17：采用同一个 flash.bin 仅更新 kernel

- kernel 打开 `CONFIG_PCI_MSI` 选项，编译成整 bin 烧到 flash，无法启动
- kernel 关闭 `CONFIG_PCI_MSI` 选项，将 `uImage + .dtb` 合并成 kernel.img，烧到 flash，能够正常启动

# 综合上述测试：发现是由 `CONFIG_PCI_MSI` 引起的 Linux 无法启动。
- 打开 `CONFIG_PCI_MSI` 出现 `Receive FIFO overflow`
- 关闭 `CONFIG_PCI_MSI` 能够正常启动


- 关闭：烧录
	- 启动 ok
	- ![[Pasted image 20241120201203.png]]
- 打开：烧录 kernel
	- 启动 失败
	- ![[Pasted image 20241120202154.png]]








## 测试18：更新 gxmisc 库
- 由于测试 17 打开 `MSI` 的时候无法获取到 `PCI` 的设备树资源，这是不合理的
	- 经过测试发现是用了新的 uImage，老的 dtb，更换了新的 dtb 之后好了
- 考虑到 PCI 和 flash 无关，所以可能问题不在这里，更新 gxmisc 库尝试
	- 之前更新过一次库，但是用的是 `arm-linux-uclibc`工具链，对应的 kernel 也是 `arm-linux-uclibc` 工具链
		- 这个时候是 ok 的
	- kernel 换成了 `arm-none-linux`，gxmisc 无论是 `arm-linux` 还是 `arm-nene-linux` 都可以
	- kernel 换成老的 `arm-linux-uclibc`，gxmisc 库用老的，还是 `arm-linux-uclibc` 也还是会出现 `Receive FIFO overflow`


## 测试19：最后测试

- [x] 测试1：最新的 kernel，用我的 rootfs

- [x] 测试2：最近的 kernel，用 超哥的 rootfs