
# 系统调用
- i386 使用 `int 0x80` 来进入系统调用
- arm v7 使用 `svc` 指令来进入系统调用
	- **跑裸机的时候，执行一下 svc 指令看看现象
