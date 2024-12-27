

## 需求：根据 spi_nor_ids. c 的信息自动生成 flash 支持列表 
![[Pasted image 20241221103303.png]]
```python
import pandas as pd
import re
import openpyxl
from openpyxl.styles import Font, PatternFill
import os

def parse_c_file(file_path):
    """解析C文件中的flash信息"""
    flash_info = {}
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    # 使用正则表达式匹配flash信息
    pattern = r'{\s*"([^"]+)",\s*0x([0-9a-fA-F]+),.*?}'
    matches = re.finditer(pattern, content, re.DOTALL)
    
    for match in matches:
        name = match.group(1)
        jedec_id = int(match.group(2), 16)
        if name != "Unsupported Flash Type":
            flash_info[jedec_id] = name
            
    return flash_info

def get_latest_version(excel_path):
    """获取Excel中最新的版本号"""
    wb = openpyxl.load_workbook(excel_path)
    sheet = wb.active
    
    # 查找备注列
    note_col = None
    for col in range(1, sheet.max_column + 1):
        if sheet.cell(row=1, column=col).value == "备注":
            note_col = col
            break
    
    if note_col is None:
        raise Exception("找不到备注列")
        
    # 获取版本号列
    version_col = note_col - 1
    
    # 找到最新版本
    latest_version = None
    for row in range(2, sheet.max_row + 1):
        version = sheet.cell(row=row, column=version_col).value
        if version and isinstance(version, str) and version.startswith('V'):
            latest_version = version
            
    return latest_version

def increment_version(version):
    """增加版本号"""
    # 从V3.3.0格式解析版本号
    major, minor, patch = map(int, version[1:].split('.'))
    
    # 增加minor版本号
    minor += 1
    patch = 0
    
    return f"V{major}.{minor}.{patch}"

def create_new_version(excel_path, c_file_path):
    """创建新版本的支持列表"""
    # 读取Excel文件
    wb = openpyxl.load_workbook(excel_path)
    sheet = wb.active
    
    # 获取C文件中的flash信息
    c_flash_info = parse_c_file(c_file_path)
    
    # 获取Excel中的flash信息
    excel_flash_info = {}
    jedec_id_col = None
    name_col = None
    
    # 查找Jedec ID和Name列
    for col in range(1, sheet.max_column + 1):
        header = sheet.cell(row=1, column=col).value
        if header == "Jedec ID":
            jedec_id_col = col
        elif header == "Name":
            name_col = col
            
    if not (jedec_id_col and name_col):
        raise Exception("找不到必要的列")
        
    # 读取Excel中的flash信息
    for row in range(2, sheet.max_row + 1):
        jedec_id = sheet.cell(row=row, column=jedec_id_col).value
        if jedec_id:
            try:
                jedec_id = int(jedec_id, 16)
                name = sheet.cell(row=row, column=name_col).value
                excel_flash_info[jedec_id] = name
            except:
                continue
                
    # 找出新的flash
    new_flash = {}
    for jedec_id, name in c_flash_info.items():
        if jedec_id not in excel_flash_info:
            new_flash[jedec_id] = name
            
    if not new_flash:
        print("没有发现新的flash")
        return
        
    # 获取最新版本并创建新版本
    latest_version = get_latest_version(excel_path)
    new_version = increment_version(latest_version)
    
    # 复制最新版本的数据并添加新的flash
    version_row = None
    for row in range(2, sheet.max_row + 1):
        if sheet.cell(row=row, column=name_col-1).value == latest_version:
            version_row = row
            break
            
    if version_row is None:
        raise Exception("找不到最新版本所在行")
        
    # 插入新版本
    sheet.insert_rows(version_row)
    new_row = version_row
    
    # 复制格式和数据
    for col in range(1, sheet.max_column + 1):
        source_cell = sheet.cell(row=version_row+1, column=col)
        target_cell = sheet.cell(row=new_row, column=col)
        
        # 复制值和格式
        target_cell.value = source_cell.value
        if source_cell.has_style:
            target_cell.font = Font(name=source_cell.font.name, size=source_cell.font.size)
            target_cell.fill = PatternFill(start_color=source_cell.fill.start_color, 
                                         end_color=source_cell.fill.end_color,
                                         fill_type=source_cell.fill.fill_type)
    
    # 更新版本号
    sheet.cell(row=new_row, column=name_col-1).value = new_version
    
    # 添加新的flash
    for jedec_id, name in new_flash.items():
        sheet.insert_rows(new_row + 1)
        sheet.cell(row=new_row + 1, column=jedec_id_col).value = f"0x{jedec_id:06x}"
        sheet.cell(row=new_row + 1, column=name_col).value = name
        
    # 保存文件
    wb.save(excel_path)
    print(f"已创建新版本 {new_version} 并添加了 {len(new_flash)} 个新的flash")

if __name__ == "__main__":
    excel_path = "SPI_NOR_FLASH_V19.xlsx"
    c_file_path = "drivers/spinor/spinor_ids.c"
    
    create_new_version(excel_path, c_file_path)
```

![[Pasted image 20241221103435.png]]

### 此时我发现它生成的这些代码并没有保存到一个新的文件中，我希望它直接将这些代码保存到另一个文件中  
![[Pasted image 20241221103544.png]]

- 此时我再去看，就发现确实有这个文件了，里面也是它生成好的代码 

### 按照它提供的方法来运行，发现有报错，继续让它来处理
![[Pasted image 20241221103845.png]]

![[Pasted image 20241221103949.png]]


### 依旧产生相同的报错，可能是它了解的信息太少了，我们让它先分析下提供的源文件 (spinor_ids. c、excel)，让它了解更多的信息
![[Pasted image 20241221104553.png]]


### 把输出的打印作为输入提供给它 
```
-> % python3 update_flash_list.py

成功从drivers/spinor/spinor_ids.c解析到77个flash:
JEDEC ID: 0x1c3115, 型号: EN25F16
JEDEC ID: 0x1c3015, 型号: EN25Q16
JEDEC ID: 0x1c7015, 型号: EN25QH16
JEDEC ID: 0x1c3116, 型号: EN25F32

...


Excel文件列标题:
列 1: 品牌
列 2: 主型号
列 3: 料号
列 4: 封装
列 5: MID
列 6: DID
列 7: 容量
(MB)
列 8: 是否支持多倍速
列 9: 最高时钟频率
列 10: 备注
列 11: V3.3.0
列 15: V3.2.0
列 18: V3.1.0
列 21: V3.0.0
列 24: V1.9.8-9
列 27: V1.9.8-8.1
列 30: V1.9.8-8
列 33: V1.9.8-7
列 36: v1.9.8-6
列 39: v1.9.8-5
```
![[Pasted image 20241221105044.png]]


### 根据它修改后的代码，重新运行，发现仍然产生了报错
![[Pasted image 20241221105129.png]]


### 此时运行没有报错了，但是找到的最新版本号还是错误的。之前分析 excel 的时候找到的版本号都是对的，因此我把之前的分析结果输入给它，让它重新解析 
![[Pasted image 20241221105854.png]]

### 这次解析到的最新版本号是对的了，但是并没有生成新的 excel 表格，让它将新的版本填充到新的 excel 表中
![[Pasted image 20241221110414.png]]

### 继续运行，发现产生了新的 excel 表格，但是新表格中并没有新版本的 flash 信息
![[Pasted image 20241221110549.png]]

- excel 表格的格式不太好描述，我就直接截图发给它，让它来分析 
![[Pasted image 20241221111012.png]]


### 生成的新表格中对于版本的处理还是有问题
![[Pasted image 20241221111148.png]]

此时，我就把原来正确的 excel 表格和错误的 excel 表格都发给它，然后问它是怎么处理这些单元格的
![[Pasted image 20241221111729.png]]



### 运行生成的程序，理想中生成的版本应该是 v3.4.0，但它却生成了 V1.10.8
![[Pasted image 20241221112840.png]]

此时，我告诉它一些隐含的规则来帮助它解析版本号所在的位置。
![[Pasted image 20241221113228.png]]




### 生成的 excel 表中出现了 v3.4.0，但是原来的版本号都被清空了 
![[Pasted image 20241221113216.png]]

- 我告诉它一种方法来避免原来的版本号信息被破坏

![[Pasted image 20241221113425.png]]


### 但是生成的 excel 表中还是原来的版本号被清空了
![[Pasted image 20241221113615.png]]
![[Pasted image 20241221113744.png]]



### 运行改正后的程序，生成的  excel 表版本号还是错误的
![[Pasted image 20241221122456.png]]


- 换一种提问方式
![[Pasted image 20241221122719.png]]

一系列的运行报错 ...

### 总算是生成出来了，但是格式不太一样
![[Pasted image 20241221123519.png]]

![[Pasted image 20241221123834.png]]



### 生成了格式一样的 excel 表格
![[Pasted image 20241221123823.png]]

现在可以开始让它替我在 `spi_nor_ids.c` 中创建几个虚拟的 `flash` 设备，来测试脚本中存在的问题
![[Pasted image 20241221124111.png]]


### 但是在新的 excel 中并没有看到这些新增的 flash 设备
- cursor 并不知道哪些 flash 设备是新增的 flash 设备，要如何添加这些 flash 设备到 excel 中，接下来，我将告诉它规则
![[Pasted image 20241221125439.png]]

### 新的 excel 中没有生成 v3.4.0 版本
![[Pasted image 20241221131244.png]]


一系列的报错...

### 解决了所有的报错之后，出现两个问题：1. 所有的 flash 都被当做新的 flash 设备了 2. 破坏了原有的版本号
![[Pasted image 20241221132017.png]]


![[Pasted image 20241221133037.png]]


### 只有新的 flash 被填充到了 excel 中，并且版本号也好了
![[Pasted image 20241221140704.png]]



- 后面是一些细节上的处理，例如 otp、写保护信息的解析，以及 excel 表格中的字体、格式等的处理，基本也和上面的步骤类似



### 添加测试 flash 设备并演示 
##### 添加测试 flash
```diff
diff --git a/drivers/spinor/spinor_ids.c b/drivers/spinor/spinor_ids.c
index ebe3315..25bd152 100644
--- a/drivers/spinor/spinor_ids.c
+++ b/drivers/spinor/spinor_ids.c
@@ -2453,6 +2453,51 @@ spi_nor_info g_spi_nor_data[] = {
 	},
 #endif
 
+#ifdef CONFIG_GX_MTD_SPI_NOR_8M_SIZE
+	{
+		"GD25Q32",
+		0x00c86016,
+		0x400000,
+		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
+		NULL,
+		0,
+		&protect_8m_type1,
+#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
+		&otp_type13,
+#endif
+	},
+#endif
+
+#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
+	{
+		"PY25Q128HB",
+		0x00856018,
+		0x1000000,
+		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
+		NULL,
+		0,
+		&protect_16m_type4,
+#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
+		&otp_type13,
+#endif
+	},
+#endif
+
+#ifdef CONFIG_GX_MTD_SPI_NOR_16M_SIZE
+	{
+		"XX25Q128HB",
+		0x00856018,
+		0x1000000,
+		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_QUAD_WRITE,
+		NULL,
+		0,
+		&protect_16m_type4,
+#ifdef CONFIG_GX_MTD_SPI_NOR_OTP
+		&otp_type13,
+#endif
+	},
+#endif
+
 };
 
 int g_spi_nor_data_count = (sizeof(g_spi_nor_data) / sizeof((g_spi_nor_data)[0]));
```

##### 演示结果 
``` shell
警告: 输出文件 SPI_NOR_FLASH_V20.xlsx 已存在。是否覆盖？(y/N): y

警告: 未找到Flash型号 XX25Q128HB 对应的品牌映射
请在 brand_map 中添加相应的品牌映射
版本升级: V3.3.0 -> V3.4.0

=== 新增的Flash设备 ===
----------------------------------------------------------------------
型号                     JEDEC ID      容量         Excel 行号
----------------------------------------------------------------------
GD25Q32                  0xC86016      4MB              25
PY25Q128HB               0x856018      16MB            135
----------------------------------------------------------------------

请注意：
1. 所有新增设备默认封装为 SOP，如果实际封装不是 SOP，请在 Excel 中手动修改
2. 需要为新增设备补充 料号、最高时钟频率、UID
```

![[Pasted image 20241221143120.png]]
![[Pasted image 20241221143050.png]]


