# 环境搭建：arm64


## linux
- 从清华源获取 linux 源码
- 编译方法：
```shell
cp arch/arm/configs/vexpress_defconfig .config
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- menuconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j32
```
 


## busybox
- 从 github 获取 busybox 源码
- 编译方法：
```shell
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j32
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnue- install
```
- 此时会在 `_install` 目录下生成根文件系统

## 制作 ext4 格式的根文件系统镜像
```shell
# 制作 1G 大小的 rootfs_ext4.img 镜像
dd if=/dev/zero of=rootfs_ext4.img bs=1M count=1024
# 将镜像格式化为 ext4 格式
mkfs.ext4 rootfs_ext4.img
# 创建一个临时目录
mkdir -p tmpfs
# 将 rootfs_ext4.img 以 ext4 格式挂载到 tmpfs 目录
sudo mount  -t ext4 rootfs_ext4.img  tmpfs/ -o loop
# 将 busybox 生成的文件都拷贝到 tmpfs 目录下
sudo cp -af busybox/busybox/_install/* tmpfs/
# 取消挂载
sudo umount  tmpfs
# 根文件系统增加权限
chmod 777 rootfs_ext4.img
```

## 运行：
```shell
qemu-system-aarch64 -machine virt -cpu cortex-a57 -machine type=virt -m 1024 -smp 4 \
    -kernel linux_master/linux-stable/arch/arm64/boot/Image \
    --append "noinitrd root=/dev/vda rw console=ttyAMA0 loglevel=8" \
    -nographic \
    -drive if=none,format=raw,file=rootfs_ext4.img,id=hd0 \
    -device virtio-blk-device,drive=hd0 \
    --fsdev local,id=kmod_dev,path=linux_master/linux-stable/kmodules,security_model=none \
    -device virtio-9p-device,fsdev=kmod_dev,mount_tag=kmod_mount
```


## 运行 + GDB：
- 在上面的选项中添加：`-s -S` 选项，其中 `-s` 表示默认使用 `1234` 端口
- 如果需要使用其他的端口，例如 9999，可以按如下方式指定：
```shell
-S -s ---> -S -gdb tcp::9999
```

- 执行完脚本之后，需要在另一个终端中执行如下命令：
```
gdb-multiarch vmlinux(这里需要根据当前路径来指定)
target remote localhost:9999
b start_kernel
c
```



# arm32
```
https://blog.csdn.net/tang_vincent/article/details/144562576
```


## linux
```shell
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- vexpress_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j$(nproc)
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- dtbs
# or
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- vexpress_defconfig
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- realview_defconfig
mcimx6ul-evk         Freescale i.MX6UL Evaluation Kit (Cortex A7)


make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- -j$(nproc)
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- dtbs
```


## busybox
```shell
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- defconfig
# 需要打开静态编译 Settings --> Build Options --> Build static binary(no shared libs)
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j$(nproc)
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- install
```


## 运行
```shell
qemu-system-arm \
  -M vexpress-a9 \
  -m 512M \
  -kernel linux_master/linux-stable/arch/arm/boot/zImage \
  -dtb linux_master/linux-stable/arch/arm/boot/dts/arm/vexpress-v2p-ca9.dtb \
  -drive file=rootfs_ext4.img,if=sd,format=raw \
  -append "root=/dev/mmcblk0 rootfstype=ext4 init=/sbin/init rw rootwait console=ttyAMA0" \
  -nographic \
  -serial mon:stdio

```