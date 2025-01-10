def parse_memory_data():
        # GDB中看到的原始数据
        mem_data = [
                0x00042021, 0x04080601, 0x9dca8c5, 0x1255994f,
                0x8ef917d1, 0x2c6f5bd0, 0x25b2331a, 0x19f91cb2,
                0x77877125, 0xadd02374, 0x9e6002cb, 0x591c9737,
                0xb4b84b8a, 0x04e3f8ae, 0x8536aff5, 0xc9c495b1,
                0x35219908, 0xc613db08, 0x13223391, 0xff571e19
        ]
        
        # 转换为字节序列 (小端序)
        mem_bytes = []
        for word in mem_data:
                mem_bytes.extend([
                        (word >> 0) & 0xFF,
                        (word >> 8) & 0xFF,
                        (word >> 16) & 0xFF,
                        (word >> 24) & 0xFF
                ])
        
        # 写入GDB数据到文件
        with open('gdb_data.txt', 'w') as f:
                for byte in mem_bytes:
                        f.write(f"0x{byte:02X}\n")
        
        return mem_bytes

def parse_memory_bytes():
        # 从文件中读取最后一列的十六进制数据,返回字节列表
        mem_bytes = []
        with open('8线4096.txt', 'r') as f:
                for line in f:
                        if ',' in line:  # 跳过空行和无效行
                                parts = line.strip().split(',')
                                if len(parts) >= 3:  # 确保有足够的列
                                        hex_byte = parts[-1].strip()
                                        # 跳过标题行和非十六进制数据
                                        if hex_byte.lower().replace('0x', '').isalnum():
                                                try:
                                                        mem_bytes.append(int(hex_byte, 16))
                                                except ValueError:
                                                        continue
        
        # 写入文件数据到文件
        with open('file_data.txt', 'w') as f:
                for byte in mem_bytes:
                        f.write(f"0x{byte:02X}\n")
        
        return mem_bytes

if __name__ == "__main__":
        try:
                parse_memory_data()
                parse_memory_bytes()
                print("数据已分别写入 gdb_data.txt 和 file_data.txt")
        except Exception as e:
                print(f"发生错误: {e}")
