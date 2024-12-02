# ZB35Q01A ECC 测试流程及现象



## 测试case：

- case1:数据未纠正，ecc未报0x20
  - 随机选取一个block，擦除整个block
  - 读取这个block的64个page，并比较每个page的数据是否为0xff
  - 读取每个page的oob区域
  - 打印page0的前100个数据及oob
  - 将数据做bit翻转，即将1变0(例如0xff变为0xfe)
  - 关闭ecc，将翻转后的数据写入到64个page
  - 读取每个page的oob区域
  - 打印写入page0的前100个数据及oob数据
  - 开启ecc，读取64个page的数据，查看数据是否纠正回来
  - 读取每个page的oob区域
  - 打印page0的前100个数据及oob
  - 循环递增bit翻转数
- case2:数据已纠正，ecc报0x20
  - 随机选取一个block，擦除整个block
  - 读取这个block的64个page，并比较每个page的数据是否为0xff
  - 创建一个包含随机数据的buffer，大小是pagesize
  - 开启ecc，将随机数据分别写入到block的64个page
  - 读取oob区域
  - 打印写入page0的前100个数据和读取到的oob数据
  - 将随机数据做bit翻转，即将1变0(例如0xff变为0xfe)
  - 关闭ecc，将做了翻转的随机数据再次写入每个page
  - 读取oob区域
  - 打印写入page0的翻转后的数据及oob数据
  - 开启ecc，读取每个page的数据，查看数据是否纠正回来
  - 读取oob区域
  - 打印page0的前100个数据及oob数据
  - 循环递增bit翻转数
- case3:数据未纠正，且错乱，ecc报0x20
  - 随机选取一个block，擦除整个block
  - 读取这个block的64个page，比较每个page的数据是否为0xff
  - 生成pagesize的随机数据
  - 开启ecc，将随机数据写入page0
  - 读取oob区域
  - 打印page0的前100个数据及oob数据
  - 将随机数据做bit翻转
  - 关闭ecc，将翻转后的数据写入到page1~page63
  - 将开ecc时读出来的oob数据写入到page1~page63的oob区域
  - 开启ecc，读取page1~page63，与原始数据进行比较，查看数据是否纠正回来
  - 循环递增bit翻转数