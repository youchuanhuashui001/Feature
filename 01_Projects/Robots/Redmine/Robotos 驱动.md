#note

# IODMA
## 连续调用两次 start，会导致中断第一次传输，重新开始第二次传输
- 驱动中的逻辑有点问题，想做的事情是如果传输还没结束，那么就会return -1
- 实际处理的时候只考虑了 trans.total_len，这些东西都是四组寄存器对传输不完才会赋值的，如果用四组寄存器对就传输完成了，那么这里就判断不了
- trans每次都会重新赋值，这里不应该。实际做了一个 static 修饰，所以这里不需要赋值成0
```c
	trans.mode = config->mode;
	// static 修饰了，这里不需要重新初始化，或者初始化也行，下面就不用这个判断了
//	trans.total_len = 0;
//	trans.next_addr = 0;

	gx_hal_iodma_writel(dev, IODMA_DMALITE_EN, 0);
	gx_hal_iodma_writel(dev, IODMA_EN, 0);

	printf("trans.total_len:0x%x, trans.next_addr:0x%x\n", trans.total_len, trans.next_addr);
	printf("config->len:0x%x, config->addr:0x%x\n", config->len, config->addr);

	if (config->mode == GX_HAL_IODMA_MODE_FIXED) {
		//TODO: 判断上一次传输未完成
		if (trans.total_len || trans.next_addr) {
			iodma_debug("Error: Previous transfer still in progress, please wait for completion\n");
			return -1;
		}
```
