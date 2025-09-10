# Linux Watchdog 驱动学习笔记

## 1. 什么是看门狗 (Watchdog)？

看门狗定时器 (Watchdog Timer, WDT) 是一种硬件电路，其核心功能是在系统出现软件故障（如死循环、内核恐慌、进程卡死等）时，通过**自动重启（Reset）计算机系统**来恢复系统的可用性。

它就像一个安全计时器：

- 系统正常运行时，一个专门的程序（守护进程）会定期“喂狗”（即重置计时器）。
    
- 如果系统异常，导致“喂狗”程序无法正常工作，计时器就会超时。
    
- 计时器超时后，硬件会强制重启系统。
    

交互模型:

用户空间守护进程 <---> 内核驱动 (/dev/watchdog) <---> 硬件看门狗

## 2. 基本工作流程

所有驱动程序都支持最基本的操作模式。

- **激活看门狗**:
    
    - 通过 `open()` 系统调用打开特殊设备文件 `/dev/watchdog`。一旦打开，看门狗定时器就立即开始倒计时。
        
- **“喂狗” (Ping/Feed the Watchdog)**:
    
    - 在超时周期内，周期性地重置看门狗计时器，以防止系统重启。
        
- **禁用看门狗 (默认情况)**:
    
    - 通过 `close()` 关闭文件描述符。在默认情况下，这会禁用看门狗定时器。
        

> 示例代码可以参考内核源码中的 `samples/watchdog/watchdog-simple.c`。

## 3. 用户空间 API 详解

与看门狗设备交互主要通过 `write` 和 `ioctl` 系统调用。

### 3.1 `write` API

这是最简单的“喂狗”方式。向已打开的 `/dev/watchdog` 文件描述符写入**任意数据**都会重置看门狗计时器。

```
// 向设备写入任意字符来“喂狗”
write(fd, "k", 1); 
```

### 3.2 `ioctl` API

`ioctl` 提供了更丰富、更精细的控制能力。所有符合标准的驱动都支持此接口。要使用 `ioctl` 命令，需要包含头文件 `<linux/watchdog.h>`。

#### `WDIOC_KEEPALIVE` - 喂狗

此命令的作用与 `write` 操作完全相同，都是用于重置看门狗计时器。`ioctl` 的第三个参数会被忽略。

```
#include <linux/watchdog.h>

// 使用 ioctl 发送 KEEPALIVE 命令来“喂狗”
ioctl(fd, WDIOC_KEEPALIVE, 0);
```

#### `WDIOC_SETTIMEOUT` / `WDIOC_GETTIMEOUT` - 设置与获取超时

- **设置超时 (`WDIOC_SETTIMEOUT`)**
    
    - 对于支持动态修改超时的驱动（驱动会设置 `WDIOF_SETTIMEOUT` 标志），可以使用此命令。
        
    - 参数是一个指向整数的指针，该整数表示期望的超时时间（单位：秒）。
        
    - **注意**：由于硬件限制，驱动程序会将**实际设置**的超时值写回到该整数变量。因此，调用后需要检查该变量以获取确切的超时值。
        
    
    ```
    int timeout = 45; // 期望设置45秒
    ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
    // 如果硬件的超时粒度是分钟，那么timeout的值现在可能变成了60
    printf("The timeout was set to %d seconds\n", timeout);
    ```
    
- **获取超时 (`WDIOC_GETTIMEOUT`)**
    
    - 用于查询当前的看门狗超时时间（单位：秒）。
        
    - 参数是一个指向整数的指针，用于存放返回的超时值。
        
    
    ```
    int timeout;
    ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
    printf("The current timeout is %d seconds\n", timeout);
    ```
    

#### `WDIOC_SETPRETIMEOUT` / `WDIOC_GETPRETIMEOUT` - 设置与获取预超时

某些看门狗硬件支持在**真正重启系统之前**，先触发一个早期警报（如 NMI 中断）。这使得系统有机会在最终重启前记录下崩溃信息（如 kernel coredump）。

- **设置预超时 (`WDIOC_SETPRETIMEOUT`)**
    
    - 用于设定预超时警报。参数是一个整数指针，表示在**主超时发生前多少秒**触发预超时。
        
    - **重要**：这个值不是从现在开始计时，而是相对于主超时的偏移量。例如，主超时为 60 秒，预超时设为 10 秒，那么预超时警报将在第 50 秒触发。
        
    - 将预超时设置为 0 可以禁用它。并非所有驱动都支持此功能。
        
    
    ```
    int pretimeout = 10; // 在主超时发生前10秒触发
    ioctl(fd, WDIOC_SETPRETIMEOUT, &pretimeout);
    ```
    
- **获取预超时 (`WDIOC_GETPRETIMEOUT`)**
    
    - 用于查询当前设置的预超时值。
        
    
    ```
    int pretimeout;
    ioctl(fd, WDIOC_GETPRETIMEOUT, &pretimeout);
    printf("The pretimeout is %d seconds\n", pretimeout);
    ```
    

#### `WDIOC_GETTIMELEFT` - 获取剩余时间

- 某些驱动能够报告距离下一次系统重启还剩多少秒。
    
- 此 ioctl 命令用于获取该剩余时间。
    

```
int timeleft;
ioctl(fd, WDIOC_GETTIMELEFT, &timeleft);
printf("Time left before reboot is %d seconds\n", timeleft);
```

## 4. 核心风险与高级保障机制

**核心风险**: 如果看门狗的**守护进程本身**崩溃了，它打开的文件会被系统自动关闭，这将意外地**禁用看门狗**，从而使整个安全机制失效。

为了应对这个风险，Linux 提供了两种更强大的机制。

### 机制一: `nowayout` (无路可退)

`nowayout` 是一种非常强硬的策略，确保看门狗一旦启动就无法被停止。

- **实现方式**:
    
    1. **编译时**: 在内核配置中设置 `CONFIG_WATCHDOG_NOWAYOUT=Y`。
        
    2. **运行时**: 某些驱动支持在加载模块时传递 `nowayout=1` 参数来启用。
        
- **工作模式**:
    
    - 当 `nowayout` 启用后，`open("/dev/watchdog")` 会启动看门狗。
        
    - **关键点**: `close("/dev/watchdog")` **不再会**禁用看门狗。
        
    - 唯一的停止方式就是系统重启。
        
- **优点**: 极度可靠。只要守护进程失效，系统重启是必然的。
    
- **缺点**: 缺乏灵活性。在调试或某些特定场景下，可能会导致不必要的重启。
    

### 机制二: "Magic Close" (魔法关闭)

`Magic Close` 是一种更智能、更灵活的策略，它通过一个“暗号”来区分是“计划内关闭”还是“意外崩溃”。

- **实现方式**: 由驱动程序自身支持此功能。
    
- **工作模式**:
    
    - **意外崩溃**: 如果守护进程崩溃，`/dev/watchdog` 文件被动关闭。驱动没有收到“暗号”，会假定系统已陷入故障，**并保持看门狗激活**。由于不再有进程来“喂狗”，系统将在超时后重启。
        
    - **计划内关闭**: 如果是正常的、计划内的关闭（例如系统正常关机或服务停止），守护进程必须执行以下两步：
        
        1. 向 `/dev/watchdog` 写入一个特殊的魔法字符 `V`。
            
        2. 然后 `close()` 文件。
            
    - 驱动收到 `V` 字符后，就理解这是一个合法的关闭请求，于是安全地禁用看门狗。
        

## 5. 机制对比总结

|   |   |   |   |
|---|---|---|---|
|**特性**|**默认行为**|**nowayout 模式**|**Magic Close 模式**|
|**如何禁用**|`close()` 文件|**无法禁用** (只能重启)|写入'V'字符，然后`close()`|
|**守护进程崩溃后**|看门狗被**禁用**，系统**不重启**|看门狗**激活**，系统**重启**|看门狗**激活**，系统**重启**|
|**灵活性**|高|低|中|
|**可靠性**|低|非常高|高|
|**适用场景**|简单应用，开发测试|需要极高可靠性的生产环境|大多数需要可靠性又兼顾灵活性的场景|

## 6. Watchdog 核心框架 API (驱动开发)

Linux 内核提供了一个看门狗核心框架，极大地简化了看门狗驱动的编写。开发者无需重复实现与用户空间交互的逻辑（如创建设备文件、处理 ioctl 等），只需专注于实现与具体硬件交互的操作即可。

### 6.1 注册与注销

驱动程序通过以下函数向核心框架注册或注销一个看门狗设备。必须包含 `<linux/watchdog.h>`。

- `int watchdog_register_device(struct watchdog_device *wdd)`: 注册一个看门狗设备。成功返回 0，失败返回负的 errno。
    
- `void watchdog_unregister_device(struct watchdog_device *wdd)`: 注销一个已注册的设备。
    

### 6.2 `watchdog_device` 结构

这是描述一个看门狗设备的核心结构体，驱动开发者需要填充它并传递给注册函数。

```
struct watchdog_device {
      int id;
      const struct watchdog_info *info;
      const struct watchdog_ops *ops;
      unsigned int timeout;
      // ... 其它字段
};
```

**关键字段说明**:

- **`info`**: 指向 `watchdog_info` 结构的指针，描述硬件的静态信息（如唯一名称、支持的功能 `WDIOF_*` 标志等）。
    
- **`ops`**: 指向 `watchdog_ops` 结构的指针，包含了驱动实现的一系列硬件操作函数（如启动、停止、喂狗等）。
    
- **`timeout`**: 当前的超时时间（秒）。
    
- **`pretimeout`**: 当前的预超时时间（秒）。
    
- **`min_timeout`, `max_timeout`**: 硬件支持的最小和最大超时时间。
    
- **`max_hw_heartbeat_ms`**: 硬件能承受的最大喂狗间隔（毫秒）。如果驱动没有实现 `stop` 操作，则必须设置此值。
    
- **`status`**: 一组标志位，用于表示看门狗的动态状态（如 `WDOG_ACTIVE`, `WDOG_NO_WAY_OUT` 等）。
    
- **`driver_data`**: 用于存储驱动的私有数据，通过 `watchdog_set/get_drvdata` 访问。
    

### 6.3 `watchdog_ops` 结构

这个结构体包含了一系列函数指针，驱动程序需要根据硬件能力实现它们。

```
struct watchdog_ops {
      struct module *owner;
      /* 强制要求实现 */
      int (*start)(struct watchdog_device *);
      /* 可选操作 */
      int (*stop)(struct watchdog_device *);
      int (*ping)(struct watchdog_device *);
      int (*set_timeout)(struct watchdog_device *, unsigned int);
      int (*restart)(struct watchdog_device *);
      // ... 其它可选操作
};
```

**关键操作说明**:

- **`owner`**: **必须**设置为 `THIS_MODULE`，用于在设备打开时锁定模块，防止模块被意外卸载。
    
- **`start` (强制)**: 启动硬件看门狗。
    
- **`stop` (可选)**: 停止硬件看门狗。如果硬件不支持停止，则无需实现此函数，但必须设置 `max_hw_heartbeat_ms`。
    
- **`ping` (可选)**: 向硬件“喂狗”。如果未实现，核心框架会调用 `start` 函数来代替“喂狗”。
    
- **`set_timeout` (可选)**: 设置硬件的超时时间。
    
- **`restart` (可选)**: 如果硬件支持，使用看门狗来重启系统。
    
- **`ioctl` (可选)**: 用于处理驱动自定义的 `ioctl` 命令。
    

### 6.4 状态位 (Status Bits)

这些位在 `watchdog_device->status` 字段中，用于跟踪设备状态。

- **`WDOG_ACTIVE`**: 表示 `/dev/watchdog` 已被用户空间打开，用户程序应该正在“喂狗”。
    
- **`WDOG_NO_WAY_OUT`**: 表示 `nowayout` 模式已激活，看门狗无法被停止。
    
- **`WDOG_HW_RUNNING`**: 由驱动设置，表示硬件定时器正在运行。如果硬件无法停止，或在设备打开前就已运行，则必须设置此位。
    

### 6.5 常用辅助函数

- `watchdog_set_nowayout()`: 设置 `nowayout` 状态。
    
- `watchdog_set/get_drvdata()`: 设置/获取驱动私有数据。
    
- `watchdog_init_timeout()`: 从模块参数或设备树初始化超时值。
    
- `watchdog_stop_on_reboot()`: 注册一个回调，在系统重启时停止看门狗。
    
- `watchdog_notify_pretimeout()`: 由驱动在预超时发生时调用，以通知核心框架。


## 7. 测试代码
- watchdog (busybox)
