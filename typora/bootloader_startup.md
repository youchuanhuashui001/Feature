# Startup



## Start.S

1. 配置stage1 的时钟
2. 初始化ddr
3. 初始化SPI，拷贝stage2到ddr
   - stage1 阶段 SPI时钟频率为 396M，读取flash的速度约为7.5M/S(不同的flash此时间可能不同)，stage2大小为1.2M，则读取stage2时间约为0.16S
4. 读ddr中128*1024大小的数据，确保ddr工作稳定
5. 跳到stage2执行

