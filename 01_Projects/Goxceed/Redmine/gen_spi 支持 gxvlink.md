


现有的补丁已经支持了 gxvlink，需要将功能移到 cmd 层。
能合并的补丁都合并。

# 使用方法补丁：
- board-init 打开 SPI0 引脚复用
- gx_dw_spi.c 从 SPI1 切成 SPI0
![[Pasted image 20250903105928.png]]
- .config 打开 ENABLE_GEN_SPI_TEST

