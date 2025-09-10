


# GXAPI

## Gxloader
- uart_set_baudrate --> gxloader
- 需要打开小数波特率了，否则不准确

## eCos
- designware_serial.c 中提供了 set_config 接口，并且支持小数波特率
- 上层直接 ioctl 配置波特率，到下面的 set_config 接口就可以设置成功

## Linux
- linux 原生的 dw_uart 支持小数波特率
- 上层通过 get_config --> cfgetispeed 拿到现在的配置，然后用 set_config --> cfsetospeed  cfsetispeed 配置输入输出波特率



# Test_Case

## Linux
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <gxhal/misc/gxserial_api.h>

static void print_hex(const unsigned char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		printf("%02X%s", buf[i], (i + 1) % 16 == 0 ? "\n" : " ");
	}
	if (len % 16 != 0)
		printf("\n");
}

static int Serial_Loopback_Test(int port, int baudrate, int count)
{
	int fd = -1;
	int ret = 0;
	int i;
	int len;
	unsigned char data[1024] = {0};

	if (count <= 0) {
		printf("Invalid params\n");
		return -1;
	}

	fd = GxSerial_Open(port, GXSERIAL_MODE_BLOCK);
	if (fd < 0) {
		printf("Open %d failed, ret=%d\n", port, fd);
		return -1;
	}

	ret = GxSerial_SetBaudrate(fd, baudrate);
	if (ret < 0) {
		printf("Set baudrate %d failed\n", baudrate);
		GxSerial_Close(fd);
		return -1;
	}

	while (count--) {
		memset(data, 0x00, sizeof(data));
		for (i = 0; ;i++) {
			ret = GxSerial_Read(fd, &data[i], 1, 100);
			if ((data[i] == '\r' || data[i] == '\n')) {
				printf("Receive enter, stop recving.\n");
				break;
			}
			len += ret;
		}

		ret = GxSerial_Write(fd, data, len, 1000);
		if (ret < 0) {
			printf("Write failed, ret=%d\n", ret);
			break;
		}
	}

	GxSerial_Close(fd);
	return ret;
}

static int Serial_SendOnly_Test(int port, int baudrate, int count)
{
	int fd = -1;
	int ret;
	int len;
	unsigned char ch = 0;

	const char *message = "Hello GXSerial\r\n";

	fd = GxSerial_Open(port, GXSERIAL_MODE_NONBLOCK);
	if (fd < 0) {
		printf("Open %d failed, ret=%d\n", port, fd);
		return -1;
	}

	ret = GxSerial_SetBaudrate(fd, baudrate);
	if (ret < 0) {
		printf("Set baudrate %d failed\n", baudrate);
		GxSerial_Close(fd);
		return -1;
	}

	len = (int)strlen(message);
	while (count--) {
		ret = GxSerial_Write(fd, message, len, 1000);
		if (ret < 0) {
			printf("Write failed, ret=%d\n", ret);
			break;
		}

		/* 仅当实际读到 1 个字节并且为 'c'/'C' 时退出 */
		ret = GxSerial_Read(fd, &ch, 1, 100);
		if (ret == 1 && (ch == 'c' || ch == 'C')) {
			printf("Receive 'c', stop sending.\n");
			break;
		}

		usleep(200 * 1000);
	}

	GxSerial_Close(fd);
	return 0;
}


static void help(void)
{
	printf("Usage:\n");
	printf("  ./serial_api.elf 1 <dev> <baud> <count>    # Read_then_write test\n");
	printf("  ./serial_api.elf 2 <dev> <baud>            # Send-only test\n");
	printf("Example:\n");
	printf("  ./serial_api.elf 1 /dev/ttyS0 115200 32\n");
	printf("  ./serial_api.elf 2 /dev/ttyS0 115200\n");
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		help();
		return 0;
	}

	if (strcmp(argv[1], "1") == 0) {
		int port = atoi(argv[2]);
		int baud = atoi(argv[3]);
		int count = atoi(argv[4]);
		return Serial_Loopback_Test(port, baud, count);
	} else if (strcmp(argv[1], "2") == 0) {
		if (argc < 5) {
			help();
			return -1;
		}
		int port = atoi(argv[2]);
		int baud = atoi(argv[3]);
		int count = atoi(argv[4]);
		return Serial_SendOnly_Test(port, baud, count);
	} else {
		help();
	}

	return 0;
}
```

```shell
aarch64-none-linux-gnu-gcc -static -o serial_api.elf serial_api.c  -I ../../../library/goxceed/arm64-linux/include  -L  ../../../library/goxceed/arm64-linux/lib -lgxcore -lpthread
```


## eCos
```c
void serial_test1(void)
{
	cyg_io_handle_t handle;
    Cyg_ErrNo err;
	const char test_string[] = "1\n432\n4532\n~!@$#%^&*\n$^*()_++<>  ?\n";
    cyg_uint32 len = strlen(test_string);
	cyg_uint32 k = 5, times = 10;
	char receive_buf[5];
	int baudrate = 1500000;

	err=cyg_io_lookup("/dev/tty0",&handle);

	err=cyg_io_ioctl(handle, 0x0191, (void *)&baudrate);
	if (ENOERR == err) {
        diag_printf("Found /dev/haldiag. Test some special char....\n");
		err = cyg_io_write(handle, test_string, &len);
    }
	if (ENOERR == err) {
        diag_printf("I think I wrote the string. Did you see it?\n");
    }

	diag_printf("Start Interaction ....\n");
	diag_printf("Please Input 5 Chars once a time ....\n");
	while(times--) {
		cyg_io_read( handle, receive_buf,&k );
		diag_printf("\nJust Input String is:\n");
		cyg_io_write( handle, receive_buf, &k );
		diag_printf("\n");
		diag_printf("Please Input 5 Chars\n");
	}
	diag_printf("\n");
    diag_printf("Serial Interaction Finished\n");
}
```
