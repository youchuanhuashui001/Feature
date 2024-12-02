# Cache



​		Cache的引入，降低了内存访问时延，大幅提升内存访问带宽

## 1. Cache一致性

​		在多核系统中，一般每个CPU核都会使用一个本地L1和L2 Cache(称之为private cache)，以提升CPU核的整体性能，在多核间共享一个L3 Cache(LLC:last level Cacle)和一个全局的主存储器(如DDR3和DDR4)

![image-20220521171052819](image/image-20220521171052819.png)

​	假设主存储器中有变量A=17：

PS：

​	write through(写直)：同时写入到Cache和内存中

​	write back(写回)：只写入到Cache



1. CPU1试图读取全局变量A，由于其Cache中没有缓存该变量，故发生miss。于是从主存中加载该变量到其Cache，结果表现为在其Cache中有一个A=17的本地副本

2. 随后CPU4也试图读取全局变量A，同样发生miss，其Cache也存在一个A=17的本地副本

3. 此时CPU4经过计算，需要修改变量A的值，比如修改为A=42。此时如果采用wirite through策略，则A=42会同时反映到CPU4的Cache和主存中，如果采用write back策略，则A=42只会反映到CPU4的cache中(设置一个Dirty)位，主存中仍然为A=15

4. 此时CPU1继续使用变量A，由于CPU1的cache中以及缓存了变量A，所以无论CPU4采用write through或write back，CPU1看到的只是A=17

   此时就产生了Cache一致性问题，同一个变量在不同CPU看到的值不一样，通常有两种解决方案，总线snooping-based方案和Directory-based方案：

   Snoopying-based方案：每个Cache Block对应着一个共享的状态，系统中所有的Cache控制器通过共享总线进行互联，每个Cache控制器都监控着共享总线上的操作，根据共享总线上的命令来更新自己的共享状态

   Directory-based方案：在某个单一的位置(directory)对某个Cache line的共享状态进行跟踪

   或者先清一遍Cache或失能Cache