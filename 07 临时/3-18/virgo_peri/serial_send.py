#!/usr/bin/env python3
import serial
import time
import argparse

def send_byte(port_name, baud_rate, byte_value, interval=0.01):
    """
    向指定串口持续发送特定字节
    
    参数:
    port_name -- 串口名称
    baud_rate -- 波特率
    byte_value -- 要发送的字节值(十六进制)
    interval -- 发送间隔时间(秒)
    """
    try:
        # 打开串口连接
        ser = serial.Serial(
            port=port_name,
            baudrate=baud_rate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=1
        )
        
        print(f"成功打开串口 {port_name}，波特率: {baud_rate}")
        print(f"开始持续发送字节: 0x{byte_value:02x}")
        
        # 创建要发送的单字节数据
        data_byte = bytes([byte_value])
        
        try:
            # 持续发送字节
            while True:
                ser.write(data_byte)
                ser.flush()  # 确保数据被写出
#                time.sleep(interval)  # 控制发送频率
                
        except KeyboardInterrupt:
            # 处理用户中断(Ctrl+C)
            print("\n用户中断，停止发送")
        
        finally:
            # 确保关闭串口
            if ser.is_open:
                ser.close()
                print(f"串口 {port_name} 已关闭")
    
    except serial.SerialException as e:
        print(f"串口错误: {e}")
        
if __name__ == "__main__":
    # 设置命令行参数
    parser = argparse.ArgumentParser(description='向串口持续发送特定字节')
    parser.add_argument('-p', '--port', type=str, default='/dev/ttyUSB0',
                      help='串口设备名 (默认: /dev/ttyUSB0)')
    parser.add_argument('-b', '--baud', type=int, default=9600,
                      help='波特率 (默认: 9600)')
    parser.add_argument('-v', '--value', type=lambda x: int(x, 0), default=0xef,
                      help='要发送的字节值，十六进制 (默认: 0xef)')
    parser.add_argument('-i', '--interval', type=float, default=0.01,
                      help='发送间隔时间，单位秒 (默认: 0.01)')
    
    args = parser.parse_args()
    
    # 调用函数开始发送
    send_byte(args.port, args.baud, args.value, args.interval)

