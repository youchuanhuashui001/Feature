
# bear


## 编译报错 
- 编译命令：`bear -- make CROSS_COMPILE="/home/tanxzh/goxceed/android/build/../toolchains/aarch64/bin/aarch64-linux-gnu-" ARCH=arm64 Image -j16`
- 解决方法：去掉 --
```shell
bear make CROSS_COMPILE="/home/tanxzh/goxceed/android/build/../toolchains/aarch64/bin/aarch64-linux-gnu-" ARCH=arm64 Image -j16
```



## 编译成功后，打开. c 文件报错 clangd: -32001: invalid AST
- 在编译目录下添加 `.clangd` 文件，内容如下：
```yaml
CompileFlags:
  Remove: -mabi=lp64
```