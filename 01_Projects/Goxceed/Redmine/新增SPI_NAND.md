# 新增 SPI NAND



## 1. 所需新增文件：

- `drivers/spinand/spinand_ids.c`
- `drivers/spinand/spinand_base.c`
- `drivers/spinand/spinand_uid.c`
- `drivers/spinand/spinand_otp.c`



## 2. 新增内容



- spinand_id.c主要涉及oob区域ECC code排布表，ecc状态错误个数处理功能，flash基本属性（page、block、size、id、name、ecc等）的添加； 
  - spinand使用自带的硬件ECC功能，因此初始化时需要设置状态寄存器中ECC使能位，ecc code一般存储在page后面的oob区域；
  - 注意：spinand编写JEDECID时，华邦的是3个字节，其余时2个字节；排布为设备id，厂家id，与spinor的排布是反着的，厂家id、设备id、版本id；
  - flash基本功能属性：保护flash型号，flash JEDEC ID，大小，是否使用双倍速读等功能；



- spinand_uid.c主要涉及读取UID数据： 
  - 读取UID，需要根据flash厂家区分来使用不同的读取UID方式，UID一般存储在OTP中，因此部分厂家需要设置OTP_ENABLE,然后读取特定地址的page页数据；另外部分厂家是通过特定的读取UID指令来读取的；
  - spi nand 的UID一般为16个字节，读取otp区域中32个字节，前16个字节UID与后16个字节在数值上是互补的，每个字节相加（addr0+addr16）为0xff；



- spinand_otp.c主要涉及读写OTP、lock OTP操作： 
  - otp读写，spinand的OTP读写，需要先设置状态寄存器1中的otp_enable位，打开otp功能，然后使用page读写功能来读写特定otp page id的数据；此处的otp时真正的one time program，每个bit只能由1写为0；与spi  norflash的otp(security register)是可以擦除的，在otp lock之前；



- spinand_base.c文件中主要涉及page读、写，block erase，坏块标记，自动检测flash型号功能等修改适配； 
  - 所有需要根据JEDEC ID来做区分设计的地方都需要遍历适配；
  - spinand具有写保护功能，但是spinand没有开发写保护功能，也没有对外提供写保护功能；写保护功能需要注意配置状态寄存器BP0-BP4的值来设置写保护的大小，一般状态寄存器存在2个，分别进行配置；





### 注：

- 在添加 oobfree 内容时：在 offset = 2 时，会将最开始的 2 个字节预留出来，不提供给用户使用
  - 由于工厂存储的坏块标记存放在 oob 区域的前 2 个字节，一般这 2 个字节是不能给用户写的，而且 bbt 表的建立也是要基于这前 2 个字节
  - 在 offset 不是从头开始的情况下，是不需要预留 2 个字节出来的