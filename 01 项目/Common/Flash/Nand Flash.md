

## 什么是 Nand Flash 的 Cache read?

Nand Flash 的读取方式主要有两种:

1. 普通读取 (Normal Read)
- 每次读取都需要先将数据从存储阵列读到页寄存器
- 然后再从页寄存器读出数据
- 读取过程需要等待 tR (读取时间)
- 适合随机读取

2. Cache Read 模式
- 利用了双缓冲页寄存器
	- 数据输出缓存：用于存放当前页数据输出
	- 感应放大器缓存：用于下一页数据预读取
- 当读取第一页时, 数据会同时存入两个页寄存器
- 读取下一页时, 一个寄存器输出数据, 另一个寄存器同时预读下一页
- 大大减少了等待时间, 提高了连续读取速度
- 适合连续读取大量数据

Cache Read 使用方法:
```
第1页读取：
1. 发送00h (读取命令)
2. 发送地址
3. 发送31h (Cache read命令)
4. 等待R/B就绪
5. 读取第1页数据
   同时，内部开始预读第2页到感应放大器缓存

第2页及后续页读取：
1. 发送31h命令
2. 等待R/B就绪
3. 读取当前页数据
   同时，预读下一页
```

```c
// 示例代码
void NAND_CacheRead(uint32_t startPage, uint32_t numPages) {
    // 第一页读取
    NAND_SendCommand(0x00);    // 读取命令
    NAND_SendAddress(startPage); // 发送地址
    NAND_SendCommand(0x31);    // Cache read命令
    NAND_WaitReady();          // 等待就绪
    NAND_ReadData();           // 读取数据

    // 后续页读取
    for(uint32_t i = 1; i < numPages; i++) {
        NAND_SendCommand(0x31); // 只需发送Cache read命令
        NAND_WaitReady();
        NAND_ReadData();
    }

    // 最后一页需要结束Cache read模式
    NAND_SendCommand(0x3F);    // 结束Cache read
}
```