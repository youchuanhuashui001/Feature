# Nor Flash、Nand Flash、并行Nand

​		FLASH是一种存储芯片，全名叫Flash EEPROM Memory，通过程序可以修改数据，即平时所说的“闪存”。

​		Flash可以在软件的控制下写入和擦写数据。其存储空间被分割成相对较大的可擦除单元，成为擦除块（erase block）。



## Nor Flash：

1. 支持xip
2. 读取速度很快，但擦除和写入新数据的速度较慢
3. 数据线、地址线分开，具有随机寻址功能







## Nand Flash：

1. 存在坏块
2. 需要专门的控制器
3. 写的时候，最小单位是页page，对也进行写操作，也称作“页编程”，page programming

4. 擦除的最小单位是块block
5. 数据线、地址线复用，不支持随机寻址，要按页读取