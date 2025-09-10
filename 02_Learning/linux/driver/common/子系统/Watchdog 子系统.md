
# 测试方法

## Userspace
- 通过 ioctl 配置超时时间，超时时间内不喂狗，就会产生 reset
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>

// watchdog 只要一直打开设备节点不喂，然后等待设定的时间结束引发reset。
int main(void)
{

    int fd;
    fd = open("/dev/watchdog", O_WRONLY);

    if (fd == -1) {
        fprintf(stderr, "Watchdog device not enabled.\n");
        fflush(stderr);
        exit(-1);
    }

    int timeout = 5;
    ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
    printf("The timeout was set to %d seconds\n", timeout);

    int timeleft = timeout;
    while((timeleft--) >= 0) {
        printf("The timeout left %d seconds\n", timeleft);
        sleep(1);
    }
}
```

- 通过 ioctl 配置超时时间，超时时间内持续喂狗，不会产生 reset
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>

// watchdog 只要一直打开设备节点不喂，然后等待设定的时间结束引发reset。
int main(void)
{

    int fd;
    fd = open("/dev/watchdog", O_WRONLY);

    if (fd == -1) {
        fprintf(stderr, "Watchdog device not enabled.\n");
        fflush(stderr);
        exit(-1);
    }

    int timeout = 5;
    ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
    printf("The timeout was set to %d seconds\n", timeout);

    int timeleft = timeout;
    int dummy;
    while(1) {
        printf("The timeout left %d seconds\n", timeleft);
	ioctl(fd, WDIOC_KEEPALIVE, &dummy);
        sleep(1);
    }
}
```
