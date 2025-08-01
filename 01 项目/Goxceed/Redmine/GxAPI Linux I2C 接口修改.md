
老的逻辑：tx是2个 msg，在发了地址之后，会发一个restart，然后再写后面的数据
新的逻辑：tx是1个msg，在发了地址之后，直接再发后面的数据

## kernel

- ioctl
- cmd:I2C_RDWR
	- 从用户空间拷贝 struct i2c_rdwr_ioctl_data32 到内核空间来
	- malloc rdwr_arg.nmsgs 个 struct i2c_msg
	- 一个指针 p，指向用户控件传来的 msgs
	- 拷贝 rdwr_arg.nmsgs 个 struct i2c_msg，从 p，也就是用户空间的 msgs 拷贝到内核空间来 
	- 上面只是临时的操作，实际要做的事情是把 user space 的结构体拷贝到内核空间来
		- 由于 user space 传了指针，所以要二级拷贝
	- 调用 i2cdev_ioctl_rdwr()
		- 调用 i2c_transfer()
			- i2c_dw_xfer
		- 拷贝到 user space


## gxtools
- 源码：
```c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gxhal/misc/gxi2c_api.h>

static int EEPROM_test(void)
{
	GxI2CSetParam param = {0};
	GxI2CUserData user_data = {0};
	unsigned char tx_data[10];
	unsigned char rx_data[10];
	int i;
	int ret;

	memset(tx_data, 0, sizeof(tx_data));
	memset(rx_data, 0, sizeof(rx_data));


	param.chip_address = 0xa0;
	param.address_width = 2;
	param.send_noack = 1;
	param.send_stop = 0;

	user_data.count = 2;
	user_data.rtx_data = &tx_data[0];

	tx_data[0] = 0x33;
	tx_data[1] = 0x44;
	tx_data[2] = 0x55;
	tx_data[3] = 0x66;
	tx_data[4] = 0x77;
	tx_data[5] = 0x88;
	tx_data[6] = 0x99;
	tx_data[7] = 0xaa;
	tx_data[8] = 0xbb;
	tx_data[9] = 0xcc;
	ret = GxI2C_Write(0, 0x00, &param, &user_data);
	if (ret < 0) {
		printf("Read failed.\n");
		return ret;
	}

	usleep(100 * 1000);

	user_data.rtx_data = &rx_data[0];
	ret = GxI2C_Read(0, 0x00, &param, &user_data);
	if (ret < 0) {
		printf("Read failed.\n");
		return ret;
	}


	for (i = 0; i < 10; i++) {
		printf("data[%d]:0x%x\n", i, rx_data[i]);
	}

	return 0;
}

static int RDA5815M_test(void)
{
	GxI2CSetParam param = {0};
	GxI2CUserData user_data = {0};
	unsigned char tx_data[10];
	unsigned char rx_data[10];
	int i;
	int ret;

	memset(tx_data, 0, sizeof(tx_data));
	memset(rx_data, 0, sizeof(rx_data));

	param.chip_address = 0x18;
	param.address_width = 1;
	param.send_noack = 1;
	param.send_stop = 0;

	user_data.count = 2;
	user_data.rtx_data = &rx_data[0];

	ret = GxI2C_Read(0, 0x00, &param, &user_data);
	if (ret < 0) {
		printf("Read failed.\n");
		return ret;
	}

	for (i = 0; i < 10; i++) {
		printf("data[%d]:0x%x\n", i, rx_data[i]);
	}

	tx_data[0] = 0x33;

	user_data.count = 1;
	user_data.rtx_data = &tx_data[0];
	ret = GxI2C_Write(0, 0x0b, &param, &user_data);

	user_data.rtx_data = &rx_data[1];
	ret = GxI2C_Read(0, 0x0b, &param, &user_data);
	for (i = 0; i < 10; i++) {
		printf("data[%d]:0x%x\n", i, rx_data[i]);
	}

	return 0;
}

static void help(void)
{
	printf("./i2c_api.elf 1     (RDA5815M_test)\n");
	printf("./i2c_api.elf 2     (EEPROM_test)\n");
}

int main(int argc,char *argv[])
{
	int ret = 0;

	if (strcmp(argv[1], "1") == 0) {
		RDA5815M_test();
	} else if (strcmp(argv[1], "2") == 0) {
		EEPROM_test();
	} else {
		help();
	}


	return 0;
}
```
- 编译命令
```shell
aarch64-none-linux-gnu-gcc -static -o i2c.elf i2c_api.c -I ../../../../library/goxceed/arm64-linux/include -L../../../../library/goxceed/arm64-linux/lib -lgxcore -lpthread
```


- 测试
- 修改前的写波形：
![[Pasted image 20250731202145.png]]
- 修改前的读波形：
![[Pasted image 20250731202203.png]]




- 修改后的写波形：
![[Pasted image 20250731201008.png]]






- 修改后的读波形：
![[Pasted image 20250731195649.png]]

![[Pasted image 20250731200805.png]]
