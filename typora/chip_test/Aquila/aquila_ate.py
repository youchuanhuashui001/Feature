import serial
import sys

speed_20k = [0xaa, 0x60, 0x00]
speed_100k = [0xaa, 0x61, 0x00]
speed_400k = [0xaa, 0x62, 0x00]
apus_addr = 0x54
start = [0xaa, 0x74]
end = [0x75, 0x00]
read_start = [0x74, 0x81]
read_end = [0xc0]

# ip_name  base_addr  offset_read_addr  read_value  offset_write_addr1  write_value1  offset_write_addr2  write_value2
test_info = dict(   i2c0 =        [0xa0000000, 0x14,   0x0190,     0x24,   0x5a5a,     0x28,   0xa5a5],
					i2c1 =        [0xa0010000, 0x14,   0x0190,     0x24,   0x5a5a,     0x28,   0xa5a5],
					i2c2 =        [0xa0020000, 0x14,   0x0190,     0x24,   0x5a5a,     0x28,   0xa5a5],
					uart0 =       [0xa0100000, 0x08,   0xc1,       0x0c,   0x5a,       0x1c,   0xa5],
					uart1 =       [0xa0110000, 0x08,   0x01,       0x0c,   0x5a,       0x1c,   0xa5],
					ismbb =       [0xa0300000, 0x0124, 0x01ffff,   0x24,   0x5a5a5a5a, 0x28,   0xa5a5a5a5],
					spi0 =        [0xa1000000, 0x1004, 0x235,      0x1080, 0x102,      0x2c,   0xa],
					spi1 =        [0xa2000000, 0x1004, 0x2bd,      0x1080, 0x102,      0x2c,   0xa],
					spi2 =        [0xa3000000, 0x1004, 0x233,      0x1080, 0x2c,       0x2c,   0xa],
					wdt =         [0xb0000000, 0x08,   0xffff,     0x00,   0x0a,       0x04,   0x05],
					timer =       [0xb0100000, 0x00,   0x10000,    0x04,   0x5a,       0x100c, 0xa5a5],
					gpio =        [0xb0200000, 0x00,   0x10000,    0x08,   0x5a5a5a5a, 0x48,   0xa5a5a5a5],
					sar_adc =     [0xb0300000, 0x00,   0x10101,    0x0c,   0xa5,       0x18,   0x5a5a5a5a],
					sar_ana =     [0xb0300200, 0x14,   0x10004,    0x10,   0x5a,       0x18,   0x5],
					ir =          [0xb0400000, 0x18,   0x7ff07ff,  0x0c,   0x5a5a5a5a, 0x28,   0xa5a5a5a5],
					quad_dec =    [0xb0500000, 0x10,   0x160,      0x0c,   0xa5,       0x18,   0x5a5a],
					##pmu_ao =      [0xb0700000, 0x3c, 0x5a5a,     0x6c, 0xa5a5a5a5],
					rtc =         [0xb0710000, 0x1c,   0x3230372a, 0x08,   0x5a5a5a5a, 0x04,   0xa5a5a5a5],
					key_scan =    [0xb0800000, 0x00,   0x201,      0x04,   0x5a5a5a5a, 0x0c,   0xa5a5],
					io_matrix =   [0xb0900000, 0x28,   0x61,       0x00,   0x1a,       0x04,   0x25],
					io_test =     [0xb0910000, 0xd0,   0x07,       0x94,   0x5a5a5a5a, 0x9c,   0xa5a5a5a5],
					rng =         [0xb0a00000, 0x00,   0x10000,    0x04,   0x1a,       0x18,   0x01],
					efuse =       [0xb0c00000, 0x28,   0x13f001,   0x04,   0x5a5a5a5a, 0x00,   0x5],
					##pmu_core =    [0xb0d00000, 0x04, 0x5a5a5a5a, 0x08, 0xa5a5a5a5a],
					ga_lite =     [0xb0f00000, 0x0c,   0x22150,    0x28,   0x5a,       0x40,   0xa5],
					sdio =        [0xb1000000, 0x1004, 0x8c,       0x04,   0x5a,       0x08,   0x5a5a5a5a],
					usb_slave =   [0xb2000000, 0x100000, 0x10200,  0x18,   0x5a505a,   0x100008, 0x01],
					dma =         [0xb3000000, 0x1000, 0x61707573, 0x00,   0x5a5a5a5a, 0x08,   0xa5a5a5a5],
					bluetooth =   [0xb4000000, 0x30010, 0x24,      0x3000c,0x01,       0x30008,0x05],
					mcu_config =  [0xb5000000, 0x44,   0xf0,       0x20,   0x5a,       0x2c,   0xa5],
					audio_in =    [0xb6000000, 0x40,   0x3fdc20,   0x3c,   0x5a5a5a5a, 0x0c,   0x5a],
					i2s =         [0xb6100000, 0x00,   0x10101,    0x14,   0x5a5a,     0x3c,   0x5a5a5a5a],
					audio_out =   [0xb6200000, 0x28,   0x4000000,  0x14,   0x5a5a5a5a, 0x24,   0x5a],
					audio_e =     [0xb6400000, 0x00,   0x10200,    0x1c,   0x5a,       0x20,   0x5a5a5a00],
					lodac =       [0xb6500000, 0x00,   0x10002,    0x08,   0x5a5a,     0x10,   0xa5a5a5a5],
					audio_codec = [0xb6600000, 0x00,   0x01,       0x44,   0xa5,       0x4c,   0x05],
					l2icache =    [0xf0100000, 0x10,   0x39b8,     0x00,   0x28,       0x08,   0x02],
					mmu =         [0xf0200000, 0x00,   0x00,       0x04,   0x5a5a0000, 0x10,   0xa5a50000],
					sar_dma =     [0xb8000000, 0x14,   0x100, 0x10, 0x5a5a5a5a, 0x0c, 0xa5a5],
					#rcm =         [0xb0600000, 0x24,   0xf07ff,   0x0c,    0x5a00,     0x00,   0xa5a5a5a5],
				)

# ao_info[0]: base_addr  reg_num
# others: offset_addr  default_value  write_value
ao_info = [
			[0xb0700000, 36],
			[0x00, 0x00,       0x00],
			[0x04, 0x01,       0x01],
			[0x08, 0x00,       0x00],
			[0x0c, 0x00,       0x00],
			[0x10, 0x3d,       0x00],
			[0x14, 0x01,       0x3c],
			[0x18, 0x00,       0x18],
			[0x1c, 0xffffffff, 0x00],
			[0x20, 0x00,       0x5a5a5a5a],
			[0x24, 0x00,       0x5a5a5a5a],
			[0x28, 0x00,       0x00],
			[0x2c, 0x01,       0x02],
			[0x30, 0x03,       0x00],
			[0x34, 0x00,       0x1a],
			[0x38, 0x00,       0x5aa0],
			[0x3c, 0xff00,     0x5a5a],
			[0x44, 0x208,      0x100],
			[0x48, 0x20280,    0x300],
			[0x50, 0x08,       0x50a],
			[0x58, 0x412,      0x0a],
			[0x5c, 0x40b,      0x15],
			[0x60, 0x10106,    0x01],
			[0x64, 0x10004,    0x03],
			[0x68, 0x03,       0x00],
			[0x6c, 0x1000,     0x5a5a5a5a],
			[0x70, 0x00,       0xa5],
			[0x74, 0x00,       0x5a],
			[0x78, 0x00,       0xa5a5a5a5],
			[0x7c, 0x00,       0x5a],
			[0x80, 0x00,       0x00],
			[0x84, 0x00,       0x00],
			[0x88, 0x00,       0x00],
			[0x8c, 0x00,       0x01],
			[0x90, 0x4006,     0x00],
			[0x94, 0x301,      0x00],
			[0x98, 0x00,       0x300]
		]

core_info = [
			[0xb0d00000, 13],
			[0x00, 0x01,     0x01],
			[0x04, 0x00,     0x5a5a5a5a],
			[0x08, 0x00,     0xa5a5a5a5],
			[0x0c, 0x00,     0x5a5a5a5a],
			[0x10, 0x00,     0x00],
			[0x14, 0x800040, 0x1a],
			[0x18, 0x1ff,    0x00],
			[0x1c, 0x00,     0x5000],
			[0x20, 0x00,     0x11],
			[0x24, 0x00,     0x1000],
			[0x28, 0x00,     0x1000],
			[0x2c, 0x342,    0x10000],
			[0x30, 0x00,     0x105]
			]

def write_data (addr, value):
	data = []
	data.clear()
	data.extend(start)
	data.append(0x80 + 1 + 4 + 4)
	data.append(apus_addr)

	data.append(addr & 0xff)
	data.append(addr >> 8 & 0xff)
	data.append(addr >> 16 & 0xff)
	data.append(addr >> 24 & 0xff)

	data.append(value & 0xff)
	data.append(value >> 8 & 0xff)
	data.append(value >> 16 & 0xff)
	data.append(value >> 24 & 0xff)

	data.extend(end)

	#print(ip, hex(addr))
	#hex_list = [format(num, '02x') for num in data]
	#print(hex_list)

	send_data = bytes(data)
	ser.write(send_data)


def read_data (addr):
	data = []
	data.clear()
	data.extend(start)
	data.append(0x80 + 1 + 4)
	data.append(apus_addr)

	data.append(addr & 0xff)
	data.append(addr >> 8 & 0xff)
	data.append(addr >> 16 & 0xff)
	data.append(addr >> 24 & 0xff)

	data.extend(read_start)
	data.append(apus_addr + 1)
	data.append(0xc0 + 3)
	data.extend(read_end)
	data.extend(end)

	#print(ip, hex(addr))
	#hex_list = [format(num, '02x') for num in data]
	#print(hex_list)

# read: send data
	send_data = bytes(data)
	ser.write(send_data)

# receive data
	receive_data = ser.read(4)
	if len(receive_data) < 4:
		return [0,0,0,0]
	return receive_data


# open serial dev
dev = '/dev/ttyUSB0'
# dev = input('dev name:')
ser = serial.Serial(dev, 9600, timeout=1)

send_data = bytes(speed_400k)
ser.write(send_data)

# enable test mode
write_data(0x00, 0xa5)

tmp = read_data(0x00)
print(hex(tmp[0]), hex(tmp[1]), hex(tmp[2]), hex(tmp[3]))

write_data(0x24, 0x5a)

tmp = read_data(0x24)
print(hex(tmp[0]), hex(tmp[1]), hex(tmp[2]), hex(tmp[3]))

write_data(0x4, 0xc)

tmp = read_data(0x4)
print(hex(tmp[0]), hex(tmp[1]), hex(tmp[2]), hex(tmp[3]))

write_data(0x30, 0x0)
tmp = read_data(0x30)
print(hex(tmp[0]), hex(tmp[1]), hex(tmp[2]), hex(tmp[3]))

write_data(0xc, 0x1)

tmp = read_data(0xc)
print(hex(tmp[0]), hex(tmp[1]), hex(tmp[2]), hex(tmp[3]))

print("config finish.")


# close serial dev
ser.close()