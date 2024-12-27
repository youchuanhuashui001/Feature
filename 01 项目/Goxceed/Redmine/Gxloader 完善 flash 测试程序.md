

## 增加不对齐地址和不对齐内存测试程序


### 确认读写擦接口，nor、nand 是否都会自己处理不对齐的地址和长度 
- nor 会内部处理
- spinand 也会内部处理
- nand 也会内部处理 


### 如果内存地址不对齐，真实的按照不对齐的方式操作了内存吗？
- 如果指针使用 `u8 *` 的方式访问单个字节，由于所有的地址都可以被 `u8 *` 对齐，所以不存在非对齐访问
- 如果指针使用 `u32 *` 的方式访问四个字节，由于某些地址不被 `u32 *` 对齐，那么就存在非对齐访问 

- 是的，底层 spi 的驱动操作的就是这个内存
	- `32bit` 位宽的时候是从 fifo 将数据先取到一个 `uint32_t` 类型的变量，再按 `u8 *` 的方式写到内存的，不会直接将 `fifo` 的数据按 `4字节` 的方式写到内存，所以不会出现内存不对齐的情况，因为全是用的 `u8 *` 操作的非对齐的内存地址 
	- 对于 `dma` 如果内存不是 `DCACHE_LINE` 对齐的话，不对齐的部分会用 `CPU` 传掉，所以也不会有操作非对齐的内存地址 
- spi nand 的驱动：
	- 读：把这个 buf 给到了另一个结构体中的 buffer，然后传给 spi 的，spi 读到这个 buffer，然后用 `memcpy` 拷贝到内存地址的 
	- 写：和读类似的处理方式 
- 并行 nand 的驱动呢？
	- V1：
		- 读：
			- CPU：4 字节对齐，
			- DMA：64 字节对齐 
			- 如果不对齐，先读到一个临时 `buffer`，然后使用 `memcpy` 拷贝到内存 

		- 写：
			- 一样的处理方式 
	- V0：
		- 读：如果不是 4 字节对齐，会用单字节的方式写到内存
		- 写：如果不是 4 字节对齐，会用单字节的方式从内存取数据 

- memcpy 会存在非对齐访问的问题吗？
	- 不存在，因为 `memcpy` 会在进行内存拷贝之前先判断 `src、dst` 是否是四字节对齐，如果非四字节对齐的地址，会采用 `u8 *` 的方式拷贝 
	```c
	#define UNALIGNED2(X, Y) \
 	(((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

void *memcpy(void *dst0, const void *src0, size_t len0)
 {
 	char *dst = dst0;
 	const char *src = src0;
 	long *aligned_dst;
 	const long *aligned_src;
 	int   len =  len0;
 
 	/* If the size is small, or either SRC or DST is unaligned,
 	   then punt into the byte copy loop.  This should be rare.  */
 	if (!TOO_SMALL(len) && !UNALIGNED2 (src, dst))
 	{
 		aligned_dst = (long*)dst;
 		aligned_src = (long*)src;
 
 		/* Copy 4X long words at a time if possible.  */
 		while (len >= BIGBLOCKSIZE)
 		{
 			*aligned_dst++ = *aligned_src++;
 			*aligned_dst++ = *aligned_src++;
 			*aligned_dst++ = *aligned_src++;
 			*aligned_dst++ = *aligned_src++;
 			len -= BIGBLOCKSIZE;
 		}
 
 		/* Copy one long word at a time if possible.  */
 		while (len >= LITTLEBLOCKSIZE)
 		{
 			*aligned_dst++ = *aligned_src++;
 			len -= LITTLEBLOCKSIZE;
 		}
 
 		/* Pick up any residual with a byte copier.  */
 		dst = (char*)aligned_dst;
 		src = (char*)aligned_src;
 	}
 
 	while (len--)
 		*dst++ = *src++;
 
 	return dst0;
 }
	```




## 增加掉电测试程序，增加打印错误的地址和错误的数据 


