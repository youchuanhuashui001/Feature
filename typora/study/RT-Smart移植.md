# RT-Smart移植

1. 复制`kernel/bsp`目录下的qemu，然后修改名称为`imx6ull`
2. 在`imx6ull`文件夹中加入修改后的`rt-config.py,mkimage.py`
3. 修改内存地址映射，`scons --menuconfig`，修改KERNEL_VADDR_START = 0xC0000000

4. 根据单板资源确定内存大小，修改`rt-smart/kernel/bsp/imx6ull/drivers/board.c`
   ![image-20220521185454731](image/image-20220521185454731.png)

5. 修改`rt-smart/kernel/bsp/imx6ull/drivers/realview.h`

![image-20220521190530913](image/image-20220521190530913.png)

6. 修改console的硬件地址和中断号
   ![image-20220521201537964](image/image-20220521201537964.png)

7. 配置内核禁止UART1

8. 在`kernel/bsp/imx6ull/drivers/serial.c`添加代码
   ![image-20220521202139737](image/image-20220521202139737.png)

   ![image-20220521202107698](image/image-20220521202107698.png)