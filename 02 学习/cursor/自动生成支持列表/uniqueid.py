import os
import subprocess
from ctypes import *
import re

def extract_c_function():
    """从spinor_base.c中提取spi_nor_read_uniqueid函数"""
    try:
        with open('drivers/spinor/spinor_base.c', 'r') as f:
            content = f.read()
        
        # 查找函数
        start = content.find('int spi_nor_read_uniqueid(')
        if start == -1:
            print("未找到 spi_nor_read_uniqueid 函数")
            return None
        
        # 找到完整函数体
        brace_count = 0
        end = start
        for i in range(start, len(content)):
            if content[i] == '{':
                brace_count += 1
            elif content[i] == '}':
                brace_count -= 1
                if brace_count == 0:
                    end = i + 1
                    break
        
        return content[start:end]
    except Exception as e:
        print(f"提取C函数时出错: {e}")
        return None

def create_c_wrapper(c_code):
    """创建C包装文件"""
    # 提取结构体定义
    structs = []
    struct_pattern = r'struct\s+(\w+)\s*{[^}]+}'
    struct_matches = re.finditer(struct_pattern, c_code)
    for match in struct_matches:
        structs.append(match.group(0))
    
    # 创建包装代码
    wrapper_code = """
#include <stdint.h>
#include <string.h>

// 必要的结构体定义
struct flash_info {
    const char *markid;
    unsigned int jedec_id;
};

struct spi_nor_flash {
    void *spi;
    struct flash_info *info;
    int addr_width;
};

// 包装函数
int get_uid_length(unsigned int jedec_id, const char* markid) {
    struct spi_nor_flash flash = {0};
    struct flash_info info = {0};
    unsigned char buf[32] = {0};
    unsigned int id_len = 0;
    int len = 0;
    
    // 设置参数
    info.jedec_id = jedec_id;
    info.markid = markid;
    flash.info = &info;
    flash.addr_width = 3;  // 默认地址宽度
    
    // 从原始函数提取的核心逻辑
"""
    
    # 提取switch语句
    switch_pattern = r'(switch\s*\([^)]+\)\s*{.*?})(?=\s*if\s*\(buf_len)'
    switch_match = re.search(switch_pattern, c_code, re.DOTALL)
    if switch_match:
        switch_code = switch_match.group(1)
        # 调整缩进
        switch_code = "\n    ".join(switch_code.splitlines())
        wrapper_code += f"    {switch_code}\n"
    
    # 添加返回语句
    wrapper_code += """
    return len;
}
"""
    
    # 写入文件
    with open('spinor_wrapper.c', 'w') as f:
        f.write(wrapper_code)
    
    return True

def compile_shared_library():
    """编译共享库"""
    try:
        cmd = ['gcc', '-shared', '-fPIC', '-o', 'libspinor.so', 'spinor_wrapper.c']
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        #print("共享库编译成功")
        return True
    except subprocess.CalledProcessError as e:
        print(f"编译失败: {e.stderr}")
        return False

def load_spinor_library():
    """加载共享库"""
    try:
        lib = cdll.LoadLibrary('./libspinor.so')
        lib.get_uid_length.argtypes = [c_uint, c_char_p]
        lib.get_uid_length.restype = c_int
        return lib
    except Exception as e:
        print(f"加载库失败: {e}")
        return None

def get_uid_length(jedec_id, markid=""):
    """获取指定Flash的UID长度
    
    Args:
        jedec_id: JEDEC ID (整数或十六进制字符串)
        markid: Flash的markid字符串
    
    Returns:
        int: UID长度，失败返回-1
    """
    try:
        # 如果jedec_id是字符串，转换为整数
        if isinstance(jedec_id, str):
            jedec_id = int(jedec_id, 16)
        
        # 提取C函数
        c_code = extract_c_function()
        if not c_code:
            return -1
        
        # 创建包装文件
        if not create_c_wrapper(c_code):
            return -1
        
        # 编译共享库
        if not compile_shared_library():
            return -1
        
        # 加载共享库
        lib = load_spinor_library()
        if not lib:
            return -1
        
        # 调用函数获取UID长度
        markid_bytes = markid.encode('utf-8')
        return lib.get_uid_length(jedec_id, markid_bytes)
        
    except Exception as e:
        print(f"获取UID长度时出错: {e}")
        return -1

def main():
    """测试函数"""
    # 测试用例
    test_cases = [
        (0xef4019, ""),  # Winbond
        (0x5e4017, "ZB25VQ64B_C_D"),  # Zbit
        (0x1c4116, ""),  # Eon
    ]
    
    for jedec_id, markid in test_cases:
        print(f"\n测试 JEDEC ID: 0x{jedec_id:06x}, MarkID: {markid}")
        uid_len = get_uid_length(jedec_id, markid)
        if uid_len > 0:
            print(f"UID长度: {uid_len} 字节")
        else:
            print("获取UID长度失败")

if __name__ == '__main__':
    main()
