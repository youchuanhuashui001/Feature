

## SPI_NOR


### 功能需求
1. `python` 脚本执行方法：`spinor_support_list.py -i xxx.xlsx -o xxx.xlsx` 
2. 自动更新版本号
3. 从 `spinor_ids.c` 中根据 `jedec_id` 来判断，如果与原表格中的 `MID、DID` 不同，则新增到对应的位置
4. 新增 `flash` 设备的时候根据对应的厂商名称来插入，相同容量的插入到最后一个
5. `OTP` 功能能够正确读取，并填充
6. 写保护功能能够正常读取范围，并以批注的形式填充
7. `UID` 等需要手动填充的，请在执行时打印出来 



### 实现方法
1. 主要包含三个核心类
	- `FlashParser`：解析 `Flash` 信息
		- `parse_flash_info()` ：解析所有 `flash` 信息 
		- `_build_flash_info()`：构建单个 `Flash` 信息 
		- `_parse_features()`：解析单双四倍速 
		- `_parse_protect_type()`：解析写保护，这里只解析到 `xm_typey`
		- `_prase_protect_range()`：解析写保护范围 
		- `_parse_otp_info()`：解析 `otp` 信息，直接把所有的 `otp` 信息都解析出来了 
		- `_parse_brand()`：根据 `Name` 的前两个名称解析出对应的厂商 
	- `ExcelHandler`：处理 `Excel` 文件
		- `compare_and_update_excel` ：根据 MID、DID 比较，如果有不同的，则认为是新加的，根据容量大小和厂商名称来插入新增的 flash，打印一下新增的 flash 信息 
		- `upgrade_version`：找到备注列，然后拷贝 4 行，添加一个新版本，更新版本号 
	- 主程序流程控制 
2. 如何确认是否新增一个 `flash` 设备？
	- 如果 `ID、Name` 均不同，允许添加 
	- 如果 `ID` 相同，但 `Name` 不同，允许添加
	- 如果 `ID、Name` 均相同，不允许添加 



## SPI_NAND
- 与 `Nor` 基本相同，并且简单 

## 其它 