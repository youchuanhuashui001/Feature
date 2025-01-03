# 需求
- 目前显示的样式如下：
![[微信图片_20241227095620.jpg]]
- 需要修改成新的样式：
![[微信图片_20241227095528.jpg]]



# 处理

- 使用 `shutil.copy2` 拷贝源文件到目标文件
	- 这个时候已经有了 `output.excel`，格式和原来的一模一样
- 


### excel 需要修改：

- [x] `PMC(常亿) PM25LQ032` 驱动中是不支持 UID 的，但是 excel 中描述是 UID 单元格的内容是支持，需要修改成不支持
- [x] TH25Q32HA 的 OTP 漏掉了 Byte，写成了 `3*2048` 
- [x] XM25QH64D 的 ID 写成了 0x204018，正确的应该是 0x204017
- [x] JY25VQ128 需要修改成 JY25VQ128A

### python 脚本需要修改：

- [x] 对于相同 ID，但不同 Name 的 flash 设备，不会作为新增设备
- [x] 新增设备的时候，填充 otp 信息不能填成 `3*256 Byte` 格式了，需要填成 `支持(3*256 Byte)`


### 策略
#### XXXX/XXXX 类型的 name flash 作为两款型号放到 excel 表 

- [x] GD25Q127C/GD25Q128E 作为两款型号 
- [x] MX25L128/KH25L128 作为两款型号 
	- 修改正则表达式：
	- ![[Pasted image 20250102151158.png]]
- [x] EN25QH64A/PN25F64B 作为两款型号 
- [x] ZB25VQ32/ZB25VQ32B/T25S32 作为三款型号
- [x] ZB25VQ32D 是新的型号
- [x] ZB25VQ64/ZB25VQ64A 作为两款型号
- [x] ZB25VQ64B/ZB25VQ64C/ZB25VQ64D 作为三款型号
- [x] ZB25VQ128A   ZB25VQ128D 作为新的型号
- [x] BY25Q32BS/BH25Q32BS/BY25Q32ES 作为三款型号
- [x] BY25Q128AS/BY25Q128ES 作为两款型号
- [x] XM25QH32B/XM25QH32C/XM25QE32C 作为三款型号
- [x] FS25Q064/FM25Q64/FM25Q64AI3 作为三款新型号
	- [x] 也是修改正则表达式 
- [x] FS25Q032/FM25Q32BI3 作为两款新型号
- [x] ZD25Q32 应该在驱动中改成 ZD25Q32A


#### 对于重复的型号，需要用 jedec_id_map 来滤除



# 结果

- XXXX/XXXX 类型的 name flash 作为两款型号放到 excel 表 
- 对于相同 ID，但不同 Name 的 flash 设备，会作为新增设备
- excel 中的一些信息修改好了 



# 备注 