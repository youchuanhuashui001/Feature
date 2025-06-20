# 1. 概述
- drawio 流程图路径：`linux_driver.drawio --> dw-uart`
- 代码路径：
	- `driver/tty/serial/8250/8250_core.c`
	- `driver/tty/serial/8250/8250_port.c`
	- `driver/tty/serial/8250/8250_dw.c`



# 2. 系统集成

## 2.1 .config
```d
# Virgo_MPW
CONFIG_SERIAL_8250=y
# CONFIG_SERIAL_8250_DEPRECATED_OPTIONS is not set
// 可以将 8250_serial 作为系统控制台
CONFIG_SERIAL_8250_CONSOLE=y
// 定义所有的串口数量，以及初始化时需要初始化的串口数量
CONFIG_SERIAL_8250_NR_UARTS=2
CONFIG_SERIAL_8250_RUNTIME_UARTS=2
CONFIG_SERIAL_8250_DW=y


# Virgo_NRE
CONFIG_SERIAL_EARLYCON=y
CONFIG_SERIAL_8250=y
CONFIG_SERIAL_8250_DEPRECATED_OPTIONS=y
CONFIG_SERIAL_8250_PNP=y
# CONFIG_SERIAL_8250_FINTEK is not set
CONFIG_SERIAL_8250_CONSOLE=y
CONFIG_SERIAL_8250_DMA=y
CONFIG_SERIAL_8250_PCI=y
CONFIG_SERIAL_8250_EXAR=y
CONFIG_SERIAL_8250_NR_UARTS=4
CONFIG_SERIAL_8250_RUNTIME_UARTS=4
CONFIG_SERIAL_8250_EXTENDED=y
# CONFIG_SERIAL_8250_MANY_PORTS is not set
# CONFIG_SERIAL_8250_ASPEED_VUART is not set
CONFIG_SERIAL_8250_SHARE_IRQ=y
# CONFIG_SERIAL_8250_DETECT_IRQ is not set
# CONFIG_SERIAL_8250_RSA is not set
CONFIG_SERIAL_8250_BCM2835AUX=y
CONFIG_SERIAL_8250_FSL=y
CONFIG_SERIAL_8250_DW=y
# CONFIG_SERIAL_8250_RT288X is not set
CONFIG_SERIAL_8250_MT6577=y
CONFIG_SERIAL_8250_UNIPHIER=y
# CONFIG_SERIAL_8250_MOXA is not set
CONFIG_SERIAL_OF_PLATFORM=y
```

## 2.2 Kconfig


## 2.3 Makefile
```d
obj-$(CONFIG_SERIAL_8250)		+= 8250.o 8250_base.o
8250-y					:= 8250_core.o
8250_base-y				:= 8250_port.o
8250_base-$(CONFIG_SERIAL_8250_DMA)	+= 8250_dma.o
obj-$(CONFIG_SERIAL_8250_DW)		+= 8250_dw.o
```

## 2.4 dts
```d
# Virgo_MPW
uart0: serial0@0xFC880000 {
	compatible = "snps,dw-apb-uart";
	reg = <0xfc880000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <GIC_SPI 40 IRQ_TYPE_LEVEL_HIGH>;
	reg-shift = <2>;
	reg-io-width = <4>;
	clocks = <&apb2_2>;
};

# Virgo_NRE
uart0: serial@fc880000 {
    compatible = "snps,dw-apb-uart";
    reg = <0x00 0xFC880000 0x00 0x2000>;
    reg-io-width = <0x04>;
    reg-shift = <0x02>;
    clock-frequency = <27000000>;
    clock-names = "baudclk", "apb_pclk";
    clocks = <&sysclk &sysclk>;
    interrupt-parent = <&gic>;
    interrupts = <GIC_SPI 40 IRQ_TYPE_LEVEL_HIGH>;
};
```


# 3. 驱动分析

## 3.1 数据结构

- `struct dw8250_data`

```c
struct dw8250_data {
	u8			usr_reg;     // IP 的 USR 寄存器(8250 是一套很标准的串口 IP 规定，许多寄存器都是固定的，这里相当于是允许 IP 厂商自己配置的内容)
	u8			dlf_size;    // 用作小数波特率
	int			line;        // 由 8250 核心层分配的串口线路号(eg. 0 for ttyS0)
	int			msr_mask_on; // 硬件信号模拟相关的，强制 MSR(Modem Status Register) 某些位为1的掩码
	int			msr_mask_off;// 硬件信号模拟相关的，强制 MSR(Modem Status Register) 某些位为0的掩码
	struct clk		*clk;        // 工作时钟
	struct clk		*pclk;       // APB 时钟
	struct reset_control	*rst;        // 指向复位控制器的指针，用于在初始化时解除对UART IP 的复位
	struct uart_8250_dma	dma;         // 包含DMA相关的配置和回调函数

	unsigned int		skip_autocfg:1; // 如果为1，则跳过在 probe 阶段对 DW UART 特性的自动探测
	unsigned int		uart_16550_compatible:1; // 表示是否兼容 16550
};
```

- struct uart_port (泛化)
- 定义在 <linux/serial_core.h>。
- 是串口核心层的抽象，代表一个通用的串口端口，与具体 UART 芯片无关。
- 包含最基本的硬件资源（mapbase, membase, irq）、状态（flags, type）、操作函数集（struct uart_ops *ops），以及一个 void *private_data 指针。

- struct uart_8250_port (特化)
- 定义在 <linux/serial_8250.h>。
- 是为 8250 系列芯片 特化的结构。
- 它的第一个成员就是 struct uart_port port;。这是C语言中实现“继承”的常用技巧。这意味着一个 uart_8250_port 的指针可以直接强制转换为 uart_port 指针，并且它们指向相同的内存地址。
在 uart_port 的基础上，它增加了 8250 芯片特有的字段，如 struct uart_8250_dma *dma、unsigned int capabilities (FIFO, AFE能力) 等。


- struct dw8250_data (私有化)
- 定义在 8250_dw.c 中。
- 是具体驱动的私有数据，用于存储 Synopsys DW UART 特有的信息，如 dlf_size, clk, rst 等。

- 当驱动通过`serial8250_register_8250_port(&uart)` 时，将 `struct uart_8250_prot` 注册到 8250 核心层
- 之后，当串口核心或8250核心调用某个操作函数(例如：serial8250_set_termios) 时，会传入 `struct uart_port *p` 参数。驱动内部通过`struct dw8250_data *d = p->private_data;` 获取到自己的私有数据，从而操作`clk, dlf_size` 等特有成员
- ??? 有待商榷



struct uart_8250_port uart
struct uart_port *p = &uart.port;
struct device *dev = &pdev->dev

struct dw8250_data *data = devm_kzalloc();

p->xxx =  data->xxx;
p->private_data = data;

pdev->dev->driver_data = data;


有效的数据结构是 struct dw8250_data，其余的都是局部变量

## 3.2 probe

从设备树获取寄存器基址并进行映射
spin_lock_init(&p->lock);
使用 struct uart_port 保存寄存器基地址、irq号、irq回调等信息 (这些在哪里会调用到？)
p->irq = irq;
p->handle_irq = dw8250_handle_irq;
p->serial_in = dw8250_serial_in;(每次修改一个字节)
p->serial_out = dw8250_serial_out;
p->private_data = data;

获取设备树信息：
data->uart_16550_compatible = 0;
p->regshift = 0x02;
p->iotype = UPIO_MEM32;
p->serial_in = dw8250_serial_in32;(每次修改一个 32 位寄存器)
p->serial_out = dw8250_serial_out32;
p->uartclk = 27000000; 晶振时钟
data->clk = 24000000;
p->uartclk = 24000000;

获取 reset 子系统
reset uart ip

硬件特性探测：
对一些特殊的 uart ip 做的特殊操作，designware uart 不需要处理

配置端口：通过操作寄存器获取一些信息
读版本寄存器，拿到 version
获取 dlf 寄存器的位宽
获取 fifosize

如果存在 fifo：
配置 dma 的 tx、rx burst 为 fifosize/4
uart.dma = &dma->dma;

注册端口：将配置好的 uart 端口注册到 8250 核心层；成功时返回一个非负整数(0, 1...)，此时 `/dev/ttySn` 设备就会被创建
data->line = serial8250_register_8250_port(&uart);

pdev->dev->driver_data = data;

电源管理：
没关注

## 3.3 对上层的接口
非常好的问题，这触及了 Linux 驱动分层模型的核心。`8250_dw.c`, `8250_port.c`, 和 `8250_core.c` 作为一个整体（我们称之为 **8250 层**），共同向上层的 **`serial_core` 层**提供了一套定义良好、高度结构化的接口。

这里的接口不是传统意义上的一系列可直接调用的函数 API，而是通过**两种主要机制**实现的：

1.  **操作函数集 (Callback-based Operations)**: 通过填充一个名为 `uart_ops` 的结构体，提供一组标准的回调函数。
2.  **端口注册函数 (Registration Function)**: 提供一个 `EXPORT_SYMBOL` 导出的函数，让下层平台驱动（如 `8250_dw.c`）可以将自己发现的端口注册到 8250 核心层。

下面我们来详细分析这两种接口。

---

### 3.3.1. 接口一：`struct uart_ops` 操作函数集

这是 8250 层向上层提供的**最主要、最核心**的接口。它告诉 `serial_core`：“对于一个 8250 类型的串口，你应该如何启动它、关闭它、设置波特率、发送数据……”

#### a. 接口是什么？

这个接口就是一个名为 `serial8250_pops` 的 `uart_ops` 结构体实例，它在 `8250_port.c` 中定义：

```c
// 8250/8250_port.c
static const struct uart_ops serial8250_pops = {
	.tx_empty	= serial8250_tx_empty,
	.set_mctrl	= serial8250_set_mctrl,
	.get_mctrl	= serial8250_get_mctrl,
	.stop_tx	= serial8250_stop_tx,
	.start_tx	= serial8250_start_tx,
	.stop_rx	= serial8250_stop_rx,
	.enable_ms	= serial8250_enable_ms,
	.break_ctl	= serial8250_break_ctl,
	.startup	= serial8250_startup,
	.shutdown	= serial8250_shutdown,
	.set_termios	= serial8250_set_termios,
    // ... 等等
};
```
这个结构体里的每一个成员都是一个函数指针，指向一个在 `8250_port.c` 中实现的、用于完成特定功能的函数。

#### b. 如何提供这个接口？

这个接口不是被直接调用的，而是通过一种“注册”和“关联”的机制提供给上层：

1.  **初始化端口**: 在 `8250_port.c` 中，`serial8250_init_port` 函数会将每个 `uart_port` 的 `ops` 成员指向 `&serial8250_pops`。
    ```c
    // 8250/8250_port.c
    void serial8250_init_port(struct uart_8250_port *up)
    {
        // ...
        port->ops = &serial8250_pops;
    }
    ```
2.  **注册驱动**: 在 `8250_core.c` 中，`serial8250_init` 函数会调用 `uart_register_driver` 将 `serial8250_reg` 这个 `uart_driver` 注册到 `serial_core`。
3.  **添加端口**: 当一个端口（如 `8250_dw.c` 探测到的端口）被 `uart_add_one_port` 函数添加到 `serial_core` 时，`serial_core` 就知道了这个端口属于 `serial8250_reg` 这个驱动，并且它应该使用的操作函数集是 `port->ops`（即 `&serial8250_pops`）。

#### c. 上层如何使用？

`serial_core` 层在需要对端口执行操作时，会通过 `port->ops` 来调用这些函数。它完全不知道函数的具体实现，只关心调用这个标准接口。

**示例：启动一个串口**
1.  用户空间程序 `open("/dev/ttyS0")`。
2.  调用最终会到达 `serial_core` 的 `uart_open` 函数。
3.  `uart_open` 会调用 `uart_startup(state)`。
4.  `uart_startup` 进而调用 `port->ops->startup(port)`。
5.  由于 `port->ops` 指向 `&serial8250_pops`，所以 `serial8250_startup` (`8250_port.c`) 函数被调用。
6.  `serial8250_startup` 函数内部会检查 `if (port->startup)`。这个 `port->startup` 是一个**可被下层平台驱动覆盖**的钩子。如果 `8250_dw.c` 没有覆盖它，那么就会调用通用的 `serial8250_do_startup` 来执行硬件初始化、探测、请求中断等操作。

---

### 3.3.2. 接口二：`serial8250_register_8250_port` 端口注册函数

这是 8250 层为**下层平台驱动**（如 `8250_dw.c`, `8250_pci.c`）提供的核心接口。它允许这些驱动将它们发现和初始化的物理端口“告知”并“上交”给 8250 核心层进行统一管理。

#### a. 接口是什么？

它是一个在 `8250_core.c` 中定义的全局函数：
```c
// 8250/8250_core.c
int serial8250_register_8250_port(struct uart_8250_port *up) { ... }
EXPORT_SYMBOL(serial8250_register_8250_port);
```
`EXPORT_SYMBOL` 关键字将这个函数导出，使其对内核中的其他模块（如 `8250_dw.c`）可见。

#### b. 如何提供和使用这个接口？

这是一个典型的**生产者-消费者**模型：
*   **生产者 (`8250_dw.c`)**: `8250_dw.c` 的 `probe` 函数是“生产者”。它负责：
    1.  探测硬件，获取物理地址、中断号等资源。
    2.  在**栈上**创建一个临时的 `uart_8250_port` 结构体 `uart`。
    3.  用探测到的信息填充 `uart`。
    4.  **关键的定制化**: 它可以**覆盖** `uart.port` 中的某些函数指针，以提供平台特有的功能。例如，`dw8250_probe` 就会设置 `uart.port.set_termios = dw8250_set_termios;`。
    5.  作为最后一步，它调用 `serial8250_register_8250_port(&uart)` 来“生产”并“上交”这个端口。

*   **消费者 (`8250_core.c`)**: `serial8250_register_8250_port` 函数是“消费者”。它接收到这个端口模板后：
    1.  在自己的全局 `serial8250_ports` 数组中找一个空位。
    2.  将 `uart` 的内容**完整地复制**到全局数组中，从而解决了栈上局部变量的生命周期问题。
    3.  调用 `uart_add_one_port()` 将这个新端口通知给上层 `serial_core`。

### 3.3.3 总结

**`8250_dw.c` (平台驱动) -> `8250_core.c` / `8250_port.c` (8250 层) -> `serial_core.c` (串口核心层)**

这个关系可以这样理解：

1.  **`8250_dw.c` 对 `8250_core.c` 说**：
    > “你好，我是 `8250_dw.c`，我发现了一个 DesignWare UART。这是它的配置信息（`struct uart_8250_port`），我已经填好了。另外请注意，设置波特率的方法 (`set_termios`) 比较特殊，请用我提供的 `dw8250_set_termios` 函数来代替标准的函数。现在，请你调用 `serial8250_register_8250_port` 来接管它。”

2.  **`8250_core.c` 对 `serial_core.c` 说**：
    > “你好，我是 8250 核心驱动，我刚接手了一个新端口，已经配置好了。它的所有标准操作（启动、关闭等）都在 `serial8250_pops` 这个操作集里。现在，请你调用 `uart_add_one_port` 把它作为 `ttyS_` 设备呈现给用户空间。”

3.  **当 `serial_core.c` 需要设置波特率时，它说**：
    > “好的，用户要求改变波特率。我要调用这个端口的 `ops->set_termios` 函数。”
    >
    > (它调用的是 `serial8250_set_termios` in `8250_port.c`)

4.  **`8250_port.c` 内部逻辑说**：
    > “收到 `set_termios` 请求。我先检查一下 `port->set_termios` 这个钩子有没有被平台驱动设置过。啊，它被 `8250_dw.c` 设置成了 `dw8250_set_termios`。好的，那我调用 `dw8250_set_termios`。”
    >
    > (调用 `dw8250_set_termios` in `8250_dw.c`)

5.  **`8250_dw.c` 的 `dw8250_set_termios` 说**：
    > “收到！我来处理 DesignWare 特有的时钟速率调整 (`clk_set_rate`)。处理完特殊部分后，我再调用 `serial8250_do_set_termios` (`8250_port.c`) 来完成剩下那些标准的寄存器写入工作。”

这个精巧的、层层嵌套的函数指针和回调机制，是 Linux 设备驱动模型强大灵活性和可扩展性的完美体现。





## 3.4 8250_core.c && 8250_port.c
好的，我们来深入剖析 `8250` 层的这两个核心文件：`8250_core.c` 和 `8250_port.c`。在 `tty/serial/` 目录下，这两个文件共同构成了对 8250/16550 及其兼容 UART 的通用驱动逻辑。它们是平台驱动（如 `8250_dw.c`）和更上层的 `serial_core` 之间的桥梁。

### 核心思想：职责分离

首先，理解这两个文件关系的关键是**职责分离**。Linux 内核开发者将 8250 的通用逻辑拆分成了两个部分：

*   **`8250_port.c`**: **端口操作层 (Port Operations Layer)**。这个文件定义了对**单个** 8250 串口进行操作的所有具体“方法”。你可以把它想象成一个 "8250 端口类" 的实现，它知道如何读写寄存器、如何配置硬件、如何处理中断等。它关心的是**一个端口的内部工作原理**。

*   **`8250_core.c`**: **核心管理层 (Core Management Layer)**。这个文件负责**管理系统内所有**的 8250 串口。它维护一个所有已注册 8250 端口的列表，并处理它们的注册、注销、中断共享等系统级事务。它不关心单个端口的 FIFO 如何工作，而是关心如何将一个新发现的端口集成到系统中。

现在，我们来详细分析每个文件的具体内容。

---

### 1. `8250_port.c` - 端口操作的实现者

这个文件是 8250 驱动的“工人”，它干所有和硬件寄存器打交道的脏活累活。

**a. 关键职责：**

1.  **实现标准的 `uart_ops`**: 它定义了 `serial8250_pops` 这个 `uart_ops` 结构体，里面填充了大量的函数指针，如 `serial8250_startup`, `serial8250_shutdown`, `serial8250_set_termios` 等。这些是 8250 驱动向上层 `serial_core` 提供的标准接口。
2.  **硬件寄存器访问**: 定义了各种 I/O 类型的访问函数，如 `io_serial_in`/`out` (用于 I/O 端口)、`mem_serial_in`/`out` (用于内存映射 I/O)、`mem32_serial_in`/`out` 等。`set_io_from_upio` 函数会根据 `uart_port` 的 `iotype` 字段，选择并设置正确的读写函数指针。
3.  **硬件自动探测 (`autoconfig`)**: 这是此文件中非常核心和复杂的部分。当一个新端口被注册时，`autoconfig` 函数会被调用，通过一系列精巧的“魔法”操作来探测 UART 的真实类型：
    *   **存在性测试**: 写入 IER 寄存器并回读，判断是否有响应。
    *   **LOOPBACK 测试**: 置位 MCR 的 LOOP 位，然后检查 MSR 的状态，确认 UART 是否能正常工作。
    *   **FIFO 检测**: 通过 IIR 寄存器的值来判断是 8250/16450 (无 FIFO)，还是 16550 (有bug的FIFO)，还是 16550A (有正常FIFO)。
    *   **高级特性探测**: 对于 16550A 及以上的芯片，它会进一步探测是否有 EFR (Enhanced Feature Register)、是否是 National Semiconductor、TI、Oxford 等厂商的特定型号，并据此设置 `port->type` 和 `up->capabilities`。
4.  **中断处理逻辑 (`serial8250_handle_irq`)**: 这是实际的中断服务函数。它被 `8250_core.c` 的顶层中断处理函数调用后，负责：
    *   读取 IIR (中断标识寄存器) 以确定中断来源。
    *   根据中断来源，调用具体的处理函数：`serial8250_rx_chars` (处理接收)、`serial8250_tx_chars` (处理发送)、`serial8250_modem_status` (处理 Modem 状态变化)。

**b. 核心数据结构：`uart_config[]`**

```c
static const struct serial8250_config uart_config[] = { ... };
```
这是一个静态常量数组，是 8250 驱动的“知识库”。`autoconfig` 函数在确定了 UART 的类型后（例如 `PORT_16550A`），就会从这个数组中查找对应的条目，并将 `fifo_size`, `fcr` 默认值, `flags` (即 `capabilities`) 等信息设置到 `uart_8250_port` 结构中。

**小结**: `8250_port.c` 定义了一个 8250 兼容端口应该“如何行为”。它提供了所有与硬件直接交互的底层函数。

---

### 2. `8250_core.c` - 所有端口的管理者

这个文件是 8250 驱动的“经理”，它负责组织和管理所有的“工人”（`8250_port`）。

**a. 关键职责：**

1.  **维护端口数据库**: 这是它的核心职责。它定义了一个全局的端口数组：
    ```c
    static struct uart_8250_port serial8250_ports[UART_NR];
    ```
    这个数组就是所有 8250 端口的**持久化存储位置**。`dw8250_probe` 中的局部变量 `uart` 的内容最终被**复制**到这个数组的某个元素中。`UART_NR` 在内核配置时定义，决定了系统最多能支持多少个 8250 串口。

2.  **端口注册与注销**:
    *   **`serial8250_register_8250_port(struct uart_8250_port *up)`**: 这是平台驱动（如 `8250_dw.c`）调用的入口。它的工作流程是：
        1.  加锁 `serial_mutex` 以保证线程安全。
        2.  调用 `serial8250_find_match_or_unused()` 在 `serial8250_ports` 数组中找到一个空闲的槽位。
        3.  **执行核心的复制操作**: 将传入的 `up`（即 `dw8250_probe` 中的局部变量）的**内容**逐一复制到 `serial8250_ports[i]` 中。这解决了局部变量生命周期的问题。
        4.  调用 `uart_add_one_port(&serial8250_reg, &uart->port)`，将这个配置好的端口**注册到上层的 `serial_core`**，使其对 TTY 系统可见。
    *   **`serial8250_unregister_port(int line)`**: 执行反向操作，从 `serial_core` 移除端口，并清空在 `serial8250_ports` 中的槽位。

3.  **中断管理**:
    *   **中断共享**: 实现了复杂的中断共享机制。当多个端口共享同一个 IRQ 时，它会创建一个 `irq_info` 结构，并将所有共享此 IRQ 的 `uart_8250_port` 链接成一个链表。
    *   **`serial8250_interrupt(int irq, void *dev_id)`**: 这是向内核注册的**顶层中断处理函数**。当一个中断发生时，内核会调用它。它的工作是：
        1.  根据 `irq` 号找到对应的 `irq_info` 结构。
        2.  遍历链表中的每一个 `uart_8250_port`。
        3.  对每一个端口，调用 `port->handle_irq(port)`。这个 `handle_irq` 函数指针通常指向 `8250_port.c` 中实现的 `serial8250_default_handle_irq`，后者再去调用 `serial8250_handle_irq`。

4.  **驱动注册**:
    *   它定义了 `serial8250_reg` 这个 `uart_driver` 结构体，这是整个 8250 驱动的代表。
    *   在 `serial8250_init` 函数中，通过 `uart_register_driver(&serial8250_reg)` 将整个 8250 子驱动注册到 `serial_core`。

**b. 核心数据结构：`serial8250_ports[]`**

如上所述，这是整个 8250 核心层的基石。它是所有 8250 串口实例的“家”。任何对特定串口的操作（比如 `serial8250_suspend_port(line)`），都是通过 `line` 作为索引在这个数组中找到对应的端口实例，然后对其进行操作。

**小结**: `8250_core.c` 负责宏观的管理和集成工作。它不知道 LCR 寄存器的具体位定义，但它知道如何安全地注册一个端口，如何管理共享中断，以及如何与上层 `serial_core` 对话。

### 总结：协作流程

让我们把 `dw8250_probe` 的过程和这两个文件串起来：

1.  **`8250_dw.c` (`dw8250_probe`)**:
    *   发现硬件，获取资源（地址、IRQ）。
    *   创建一个**临时的、栈上的** `uart_8250_port` 结构体 `uart`。
    *   填充 `uart`，设置 `iotype`, `regshift`，并**重载**一些函数指针，比如 `handle_irq` 指向 `dw8250_handle_irq`。
    *   调用 `serial8250_register_8250_port(&uart)`。

2.  **`8250_core.c` (`serial8250_register_8250_port`)**:
    *   收到 `&uart` 指针。
    *   在全局 `serial8250_ports` 数组中找到一个空位 `i`。
    *   **将 `uart` 的内容完整复制到 `serial8250_ports[i]`**。
    *   调用 `univ8250_setup_irq`，它会进一步调用 `serial_link_irq_chain` 来为此端口设置中断处理链。
    *   调用 `uart_add_one_port` 通知 `serial_core`：“嘿，我有一个新端口 `i`，它的配置在 `&serial8250_ports[i].port`”。

3.  **当用户写入 `/dev/ttyS_`**:
    *   请求通过 VFS -> TTY Core -> Serial Core 最终到达 `serial8250_pops.start_tx` (`8250_port.c`)。
    *   `serial8250_start_tx` (`8250_port.c`) 调用 `serial_port_out` (`8250_port.c`)。
    *   `serial_port_out` 调用 `port->serial_out`，这个指针在第1步已经被 `8250_dw.c` 设置为 `dw8250_serial_out32`。
    *   最终，`8250_dw.c` 中的函数被调用，数据被写入硬件。

4.  **当硬件中断发生**:
    *   内核调用 `serial8250_interrupt` (`8250_core.c`)。
    *   `serial8250_interrupt` 遍历中断链表，找到对应的端口，调用 `port->handle_irq`。
    *   这个指针指向 `dw8250_handle_irq` (`8250_dw.c`)。
    *   `dw8250_handle_irq` 处理 DW 特有的中断，然后可能会调用 `serial8250_handle_irq` (`8250_port.c`) 来处理标准的 8250 中断。

通过这种精巧的、层层递进的结构，8250 框架实现了高度的通用性和可扩展性。希望这个分析能帮助您清晰地理解 8250 层的工作模式。


# 4. 数据流

### 4.1 中断方式

#### a. 默认方式：中断 (Interrupt-Driven)

对于正常的串口通信，**默认的数据传输方式是中断驱动的**。这是现代操作系统中的标准做法，因为它在 CPU 占用和响应延迟之间取得了最佳平衡。

*   **工作原理**:
    1.  **初始化**: 在 `dw8250_probe` 中，驱动获取了一个中断号 (`p->irq`)。在 `serial8250_do_startup` (`8250_port.c`) 中，驱动通过向 **IER (Interrupt Enable Register)** 写入 `UART_IER_RDI` (接收数据中断) 等位来使能中断。
    2.  **发送 (Write)**: 当用户程序写入数据时，数据被快速拷贝到内核的 `xmit` 环形缓冲区。`serial8250_start_tx` 函数会使能 **THRE (Transmit Holding Register Empty)** 中断。此时 `write` 系统调用可以立即返回，用户程序无需等待。当 UART 的发送 FIFO 有空间时，硬件会触发一个 THRE 中断。中断服务程序 (`serial8250_tx_chars`) 会从 `xmit` 缓冲区取出一些数据，写入硬件 FIFO，直到缓冲区为空或硬件 FIFO 变满。
    3.  **接收 (Read)**: 当外部数据到达 UART 的接收 FIFO 并达到预设的触发水平时，硬件会触发一个接收中断。中断服务程序 (`serial8250_rx_chars`) 会被调用，它从硬件 FIFO 读取所有可用数据，并将其放入 TTY 核心的 `flip` 缓冲区，然后唤醒等待数据的用户进程。

*   **优点**: CPU 不需要持续检查硬件状态，只有在有实际工作（发送完成或接收到数据）时才介入，大大降低了 CPU 占用。
*   **缺点**: 中断处理本身有上下文切换的开销，对于极高波特率的持续数据流，中断风暴可能会成为瓶颈。

#### b. 特殊场景：轮询 (Polling)

轮询模式，即 CPU 不断地主动查询硬件状态，一般不用于常规数据传输，因为它非常消耗 CPU。但它在以下几种特殊场景中被使用：

1.  **无中断端口**: 如果一个串口硬件没有连接中断线（`port->irq == 0`），驱动会退化为基于内核定时器的轮询模式。`univ8250_setup_irq` (`8250_core.c`) 会设置一个定时器，周期性地触发 `serial8250_timeout`，模拟中断的发生。这种方式效率很低。
2.  **早期控制台/调试**: 在系统引导早期（`earlycon`）或使用内核调试器（如 `kgdb`）时，完整的中断系统可能还未初始化或不可靠。此时，控制台输出会使用轮询方式，如 `serial8250_console_putchar` (`8250_port.c`)，它内部调用 `wait_for_xmitr`，在一个紧凑循环里等待 LSR 寄存器的 `UART_LSR_THRE` 位置位，然后再发送字符。

#### c. 高性能方式：DMA (Direct Memory Access)

当硬件支持且驱动配置了 DMA 后，数据传输的效率会得到极大提升。`8250_dw.c` 支持 DMA。

*   **工作原理**:
    1.  **初始化**: `dw8250_probe` 会检查端口的 `fifosize`，如果大于0，它会配置 `uart.dma` 结构。在 `serial8250_do_startup` 中，`serial8250_request_dma` 会尝试申请 DMA 通道。
    2.  **发送 (Write)**: 当用户写入数据后，`serial8250_start_tx` 会调用 `up->dma->tx_dma()` (`serial8250_tx_dma`)。这个函数**不再是只使能一个中断**，而是配置 DMA 控制器：
        *   **源地址**: 内核的 `xmit` 缓冲区。
        *   **目标地址**: UART 的发送数据寄存器 (`UART_TX`) 的物理地址。
        *   **传输长度**: `xmit` 缓冲区中的数据量。
        然后启动 DMA 传输。DMA 控制器会接管数据搬运工作，自动将数据从内存搬到 UART FIFO，**CPU 在此期间完全解放**，可以去处理其他任务。当 DMA 传输完成后，DMA 控制器会触发一个**完成中断**，通知 CPU 可以准备下一次传输。
    3.  **接收 (Read)**: 驱动会配置一个 DMA 通道，持续地将数据从 UART 的接收数据寄存器 (`UART_RX`) 搬运到一个内核的环形 DMA 缓冲区。当接收到一定量的数据或发生超时后，DMA 会触发一个中断。中断服务程序 (`handle_rx_dma`) 会将 DMA 缓冲区的数据一次性拷贝到 TTY 的 `flip` 缓冲区，并唤醒用户进程。

*   **优点**: 将 CPU 从繁重的字节拷贝工作中解放出来，极大地降低了高波特率下的 CPU 负载，非常适合大块数据的传输。
*   **缺点**: DMA 的设置和完成中断处理有一定延迟（overhead），不适合传输非常零散的、单个字节的数据。

#### 数据流向分析 (以中断模式为例)

**`write()` 数据流 (用户 -> 硬件):**
1.  **User App**: 调用 `write(serial_port, "Hello...", 19);`
2.  **VFS/TTY Core**: 系统调用进入内核，`tty_write` 将数据放入 `uart_port->state->xmit` 环形缓冲区。
3.  **Serial/8250 Core**: `serial8250_start_tx` 被调用，它通过向 IER 寄存器写入 `UART_IER_THRI` 位，**使能“发送保持寄存器空中断”**。
4.  **`8250_dw.c`**: 这个 IER 写操作通过 `port->serial_out`（指向 `dw8250_serial_out32`）最终写入到 DW UART 硬件。
5.  **硬件中断**: 当 UART 发送 FIFO 有可用空间时，硬件触发中断。
6.  **`8250_dw.c/8250_port.c` (ISR)**: 内核中断子系统最终调用到 `serial8250_tx_chars`。此函数从 `xmit` 缓冲区取出字符，通过 `port->serial_out` (`dw8250_serial_out32`) 写入 UART 的 `UART_TX` 寄存器。
7.  **循环**: 只要 `xmit` 缓冲区不为空，硬件就会在有空间时不断产生 THRE 中断，驱动就不断地在中断上下文中搬运数据，直到数据全部发送完毕。

**`read()` 数据流 (硬件 -> 用户):**
1.  **硬件**: 外部数据通过 RX 引脚进入 DW UART，硬件将其存入接收 FIFO。
2.  **硬件中断**: 当 FIFO 中的数据量达到触发点，或接收数据超时，硬件触发一个“接收数据可用中断”。
3.  **`8250_dw.c` (ISR)**: 内核中断子系统调用 `dw8250_handle_irq`。
4.  **`8250_port.c` (ISR)**: `dw8250_handle_irq` 调用通用的 `serial8250_rx_chars`。该函数通过 `port->serial_in` (`dw8250_serial_in32`) 从 UART 的 `UART_RX` 寄存器读取数据。
5.  **TTY Core**: 读取到的字符被 `uart_insert_char` 放入 TTY 的 `flip` 缓冲区。
6.  **唤醒**: `tty_flip_buffer_push` 检查是否有进程在等待数据（即阻塞在 `read` 系统调用上），如果有，就唤醒它。
7.  **User App**: 用户程序的 `read()` 调用从阻塞中返回，内核将 `flip` 缓冲区的数据拷贝到用户提供的 `read_buf` 中，`read` 函数返回读取到的字节数。



### 4.2 DMA 方式

好的，我们来深入分析 `8250_dw.c` 和 `8250_dma.c` 是如何协同工作的，然后在此基础上，完整地阐述 DMA 的传输方式。

#### 1. 源码分析：`8250_dw.c` 与 `8250_dma.c` 的协作

这两个文件是典型的**平台驱动 (Platform Driver)** 与**通用库 (Generic Library)** 的协作模式。

*   **`8250_dw.c` (平台驱动)**: 它不实现具体的 DMA 传输逻辑。它的**职责是为 DMA 传输做准备和配置**。它了解 DesignWare UART IP 所连接的具体 DMA 控制器的特性，并将这些信息传递给通用层。

*   **`8250_dma.c` (通用库)**: 它包含了所有 8250 兼容端口进行 DMA 传输的**通用逻辑**。它通过 Linux DMA Engine API 与 DMA 控制器交互，但它不关心具体是哪家的 DMA 控制器。它依赖于平台驱动提供给它的配置信息。

##### `8250_dw.c` 中的 DMA 配置

在 `dw8250_probe` 函数中，我们可以看到 DMA 的配置过程：
```c
// 8250_dw.c, line 603
static int dw8250_probe(struct platform_device *pdev)
{
    // ...
    struct dw8250_data *data;
    // ...
    data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
    // ...
    // (1) 设置默认的DMA过滤器
	data->dma.fn = dw8250_fallback_dma_filter;
    p->private_data = data;
    // ...
    
    // (2) 针对特定平台（如Intel LPSS）重写过滤器
	if (platform_get_resource_byname(to_platform_device(p->dev),
					 IORESOURCE_MEM, "lpss_priv")) {
		data->dma.rx_param = p->dev->parent;
		data->dma.tx_param = p->dev->parent;
		data->dma.fn = dw8250_idma_filter;
	}

    // ...

    // (3) 如果硬件有FIFO，则启用DMA
	if (p->fifosize) {
		data->dma.rxconf.src_maxburst = p->fifosize / 4;
		data->dma.txconf.dst_maxburst = p->fifosize / 4;
		uart.dma = &data->dma; // (4) 将配置好的dma结构关联到端口
	}

	data->line = serial8250_register_8250_port(&uart);
    // ...
}
```

**分析**:
1.  **设置默认过滤器 `dw8250_fallback_dma_filter`**: 这个函数直接返回 `false`，它的作用是防止在没有明确配置的情况下，DMA Engine 框架随意分配一个不兼容的 DMA 通道给这个 UART。这是一个安全的默认设置。
2.  **设置特定平台过滤器 `dw8250_idma_filter`**: 对于 Intel LPSS (Low Power Subsystem) 这样的平台，DW UART 使用其内部的 iDMA (internal DMA)。这个过滤器函数会检查 DMA 通道的设备指针是否与 UART 的父设备匹配，确保只使用正确的、专用的 DMA 通道。`rx_param` 和 `tx_param` 传递了用于匹配的设备指针。
3.  **配置 Burst Size**: 根据 UART FIFO 的大小来配置 DMA 的 `maxburst`（最大突发传输长度），这是一个性能优化，可以提高总线效率。
4.  **关联 `uart_8250_dma` 结构**: 最关键的一步，将配置好的 `data->dma` 结构体的指针赋值给 `uart.dma`。当此端口注册到 8250 核心后，`8250_dma.c` 中的通用函数就能通过 `up->dma` 访问到这些平台相关的配置（尤其是 `fn` 过滤器函数）。

**结论**: `8250_dw.c` 扮演了**配置者**的角色。它告诉 `8250_dma.c`：“如果你要为我这个端口申请 DMA 通道，请使用我提供的这个 `fn` 过滤函数，确保找到正确的 DMA 控制器。”

##### `8250_dma.c` 中的通用逻辑

这个文件是实际的**执行者**。它实现了 DMA 传输的所有步骤。

**a. 资源申请: `serial8250_request_dma()`**
*   **请求通道**: 使用 `dma_request_slave_channel_compat` 来请求 RX 和 TX 两个 DMA 通道。在这个函数内部，DMA Engine 框架会调用 `8250_dw.c` 提供的 `dma->fn` (即 `dw8250_idma_filter` 或 `dw8250_fallback_dma_filter`) 来判断每个可用的 DMA 通道是否适用。
*   **配置通道**: 设置 DMA 传输的方向（内存到设备/设备到内存）、源/目的地址（UART 的 RX/TX 寄存器物理地址）、总线宽度等。
*   **分配缓冲区**:
    *   **RX**: 使用 `dma_alloc_coherent` 分配一个对 DMA 和 CPU 都一致的接收缓冲区 `dma->rx_buf`。
    *   **TX**: 使用 `dma_map_single` 将内核的 `xmit` 环形缓冲区的物理地址映射给 DMA 控制器，使其可以直接访问。

**b. 发送过程: `serial8250_tx_dma()` & `__dma_tx_complete()`**
*   `serial8250_tx_dma` 准备一个 DMA 传输描述符 (`dmaengine_prep_slave_single`)，描述了要从 `xmit` 缓冲区发送多少数据。
*   它为这个描述符注册一个完成回调函数 `__dma_tx_complete`。
*   `dmaengine_submit` 提交描述符，`dma_async_issue_pending` 启动传输。
*   当硬件传输完成，DMA 控制器触发中断，最终 `__dma_tx_complete` 被调用。
*   回调函数会更新 `xmit` 缓冲区的 `tail` 指针，并**立即尝试启动下一次 DMA 传输** (`serial8250_tx_dma(p)`)，形成一个自我驱动的数据发送链。

**c. 接收过程: `serial8250_rx_dma()` & `dma_rx_complete()` & `serial8250_rx_dma_flush()`**
*   `serial8250_rx_dma` 准备一个描述符，让 DMA 控制器将数据从 UART RX 寄存器搬运到 `dma->rx_buf`。
*   当 DMA 缓冲区满或者超时中断（由 `serial8250_rx_dma_flush` 触发）发生时，`dma_rx_complete` 被调用。
*   `__dma_rx_complete` 检查实际收到的数据量 (`dma->rx_size - state.residue`)。
*   使用 `tty_insert_flip_string` 将收到的整块数据一次性推送到 TTY flip 缓冲区，然后 `tty_flip_buffer_push` 唤醒用户进程。
*   `serial8250_rx_dma_flush` 的存在至关重要，它允许在 DMA 缓冲区未满的情况下，通过 UART 的接收超时中断来处理已经收到的数据，保证了低延迟。

---

#### 2. DMA 传输方式的完整流程

下面将 DMA 的完整数据流串起来，形成一幅清晰的图景。

#### **发送 (Write) 数据流**

1.  **用户空间**: `write(fd, buffer, len)` 系统调用将数据从用户缓冲区拷贝到内核。
2.  **TTY/Serial/8250 Core**: 数据被放入 `uart_port->state->xmit` 环形缓冲区。`serial8250_start_tx()` 被调用。
3.  **8250 DMA 层 (`8250_dma.c`)**:
    *   `serial8250_start_tx` 发现 `up->dma` 已配置，于是调用 `serial8250_tx_dma()`。
    *   `serial8250_tx_dma()` 向 DMA Engine 框架提交一个请求：“请将 `xmit` 缓冲区从 `tail` 指针开始的一段数据，搬运到 DW UART 的 TX 寄存器物理地址”。
    *   CPU 的工作到此结束，`write()` 系统调用可以很快返回。
4.  **DMA 控制器 & DW UART 硬件**:
    *   DMA 控制器获得总线控制权，从内存中读取一个字节（或一个 burst）。
    *   将数据写入 DW UART 的 TX 寄存器。
    *   DW UART 硬件像中断模式一样，将数据发送出去。
    *   DMA 控制器不断重复此过程，直到预设的数据量全部搬运完毕。
5.  **DMA 完成中断**:
    *   DMA 控制器向 CPU 发送一个“传输完成”中断。
    *   DMA Engine 框架调用 `__dma_tx_complete` 回调函数。
    *   `__dma_tx_complete` 更新 `xmit` 缓冲区的 `tail` 指针，表示这部分数据已成功移交硬件。
    *   **关键**: 它会立即检查 `xmit` 缓冲区是否还有待发送的数据。如果有，它会**再次调用 `serial8250_tx_dma()`**，无缝地启动下一次 DMA 传输，形成高效的数据流管道。

#### **接收 (Read) 数据流**

1.  **初始化**: 在 `serial8250_startup()` 时，`serial8250_request_dma()` 已经分配好了 `rx_buf` 并准备好了 DMA 通道。
2.  **启动接收**: `serial8250_rx_dma()` 被调用，向 DMA Engine 提交一个请求：“请将数据从 DW UART 的 RX 寄存器物理地址，持续搬运到我的 `rx_buf` 缓冲区”。
3.  **硬件 & DMA 控制器**:
    *   外部数据到达 DW UART 的 RX FIFO。
    *   DMA 控制器检测到 UART 有数据，便从 RX 寄存器读取数据，然后写入内存中的 `rx_buf`。
    *   这个过程持续进行，`rx_buf` 被逐渐填满，CPU 完全不参与。
4.  **数据处理触发 (两种情况)**:
    *   **情况A：DMA 缓冲区满**: `rx_buf` 被写满，DMA 控制器触发“传输完成”中断。
    *   **情况B：接收超时**: 数据流中断，`rx_buf` 未满。但 DW UART 的接收超时功能触发了一个**普通的 UART 中断**。`dw8250_handle_irq` -> `handle_rx_dma` -> `serial8250_rx_dma_flush` 被调用。`flush` 函数会暂停当前的 DMA 传输。
5.  **DMA 完成处理 (`8250_dma.c`)**:
    *   无论是哪种情况，最终都会执行到 `__dma_rx_complete`。
    *   它通过 `dmaengine_tx_status` 获取到实际接收的数据量。
    *   调用 `tty_insert_flip_string()` 将**整块数据**一次性放入 TTY 的 flip 缓冲区。
    *   `tty_flip_buffer_push()` 唤醒阻塞在 `read()` 上的用户进程。
6.  **用户空间**: `read()` 调用返回，数据从内核的 flip 缓冲区被拷贝到用户提供的 `buffer` 中。
7.  **重新启动**: 驱动会再次调用 `serial8250_rx_dma()` 启动下一次 DMA 接收，为接收后续数据做好准备。

**总结**: DMA 模式将数据传输从“**CPU 逐字节搬运（中断模式）**”的模式，转变为“**CPU 下达批量搬运指令，由专业硬件（DMA控制器）执行**”的模式，极大地降低了 CPU 的负载，是实现高速、大批量数据串口通信的关键。


# 应用程序
```c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>   // Contains File Control definitions
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // UNIX standard function definitions
#include <errno.h>   // Error number definitions

int main() {
    // 1. 打开串口设备文件
    const char *port_name = "/dev/ttyS0"; // 根据实际情况修改
    int serial_port = open(port_name, O_RDWR);

    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return 1;
    }

    // 2. 配置串口参数
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    // 设置波特率 (例如 115200)
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag &= ~PARENB; // 清除校验位
    tty.c_cflag &= ~CSTOPB; // 清除停止位
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;     // 8位数据位
    tty.c_cflag &= ~CRTSCTS; // 关闭硬件流控
    tty.c_cflag |= CREAD | CLOCAL; // 开启读取，忽略DCD信号线

    // 设置为非经典模式 (raw mode)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;

    // 关闭软件流控
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    // 关闭实现定义的输出处理
    tty.c_oflag &= ~OPOST;

    // 设置读取超时
    tty.c_cc[VMIN] = 0; // 非阻塞读取
    tty.c_cc[VTIME] = 10; // 1秒的读取超时

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    printf("Serial port %s configured.\n", port_name);

    // 3. 写入数据
    unsigned char msg[] = "Hello from 8250_dw!\n";
    write(serial_port, msg, sizeof(msg) - 1);
    printf("Sent: %s", msg);

    // 4. 读取数据
    char read_buf[256];
    int n = 0;
    printf("Receiving...\n");
    while (1) {
        n = read(serial_port, &read_buf, sizeof(read_buf));
        if (n > 0) {
            read_buf[n] = '\0';
            printf("Received %d bytes: %s\n", n, read_buf);
        }
    }

    close(serial_port);
    return 0;
}
```




# 关联知识

- [[Spinlock(自旋锁)]]



# Q&&A

## 【已解决】设备树中为什么要描述 clock-frequency, baudclk, apb_pclk 这些信息？
- 看驱动中 clock-frequency 和 baudclk 后面会用 baudclk 覆盖 clock-frequency
- apb_pclk 会做分频吗？

- clock-frequency:
	- 角色: 这是一个简单、直接的遗留属性。它直接用一个32位整数值告诉驱动：“这个 UART 的输入时钟频率是固定的 XXXX Hz”。
	- 用途: 对于那些硬件设计简单、时钟源固定不变的平台，这是一种非常方便的描述方式。驱动直接读取这个值，存入 p->uartclk，然后用它来计算波特率分频系数 (divisor = uartclk / (16 * baud))。
	- 限制: 它无法表达时钟是可变的，也无法让驱动去动态控制时钟频率。

- baudclk (通过 clocks 和 clock-names 属性):
	- 角色: 这是基于通用时钟框架 (CCF) 的现代、灵活的属性。它不是一个固定的频率值，而是一个指向系统中某个时钟提供者（如某个 PLL 的输出）的句柄。
	- 用途: 在复杂的 SoC 中，时钟通常由专门的时钟控制器管理，可以动态开启、关闭、改变频率。通过 CCF，8250_dw.c 驱动可以：
		- clk_get_rate(): 查询 baudclk 当前的频率。
		- clk_set_rate(): 请求 CCF 将 baudclk 的频率调整到某个值，以更精确地匹配所需的波特率。
		- clk_prepare_enable() / clk_disable_unprepare(): 在使用 UART 前开启时钟，在不使用时（如运行时挂起）关闭时钟以节省功耗。

- apb_pclk: 这是另一个通过 CCF 管理的时钟。它的作用与 baudclk 不同。apb_pclk 是 APB (Advanced Peripheral Bus) 总线的时钟，驱动访问 UART 内部寄存器（读/写）需要这个时钟。而 baudclk 是专门用来生成串行比特流波特率的基准时钟。它们通常可以来自不同的时钟源。
优先级与工作方式:

在 dw8250_probe 函数中，驱动被设计为优先使用现代的 CCF 方式：
1. 它首先尝试用 devm_clk_get(dev, "baudclk") 获取 baudclk 的句柄。
2. 如果成功获取，它就会调用 clk_get_rate() 来得到当前频率，并赋值给 p->uartclk。
3. 如果获取 baudclk 失败（例如设备树中没有定义 clocks 属性），它会回退 (fallback) 到使用 device_property_read_u32 来读取 clock-frequency 这个简单的固定值。

所以，在您的设备树示例中，clock-frequency 的值实际上被 baudclk (即 &sysclk) 的实际频率所覆盖了。


## 【已解决】使用 serial8250_register_8250_port(&uart) 的时候用的参数是一个局部变量，probe 函数结束之后不就失去生命周期了吗？
- serial8250_register_8250_port 中会将 uart 的信息都拷贝到 `serial8250_ports[UART_NR]` 中保存下来
- 8250_core.c 中会定义 `static struct uart_8250_port serial8250_ports[UART_NR]` 保存所有的串口信息
- 注册完成之后会返回其所在 `serial8250_ports[]` 数组中的位置


## 用户空间的测试程序如何调度到 8250_dw.c

这是一个经典的 Linux I/O 栈调用路径。我们以一个用户程序调用 write() 发送数据为例，来追踪整个流程：
用户空间 -> VFS -> TTY 核心 -> Serial 核心 -> 8250 核心 -> 8250_dw.c -> 硬件
1. 用户空间:
- 一个程序（如 minicom 或你自己的测试程序）打开 /dev/ttyS0 文件，得到一个文件描述符 fd。
- 调用 write(fd, "hello", 5);。
2. VFS (虚拟文件系统) & 系统调用:
- write() 触发一个系统调用，进入内核态。
- VFS 根据 fd 找到对应的 file 结构体，该结构体中有一个 file_operations 指针，指向 TTY 驱动提供的操作集。
- VFS 调用 f_op->write(...)。
3. TTY 核心 (drivers/tty/tty_io.c):
- 这个 write 操作被路由到 tty_write 函数。
- TTY 核心层负责处理线路规程 (line discipline)。默认的 N_TTY 规程会处理字符转换（例如 \n 转 \r\n）、回显等。
- 处理完毕后，线路规程会调用 TTY 驱动注册的 ops->write 函数，将数据真正地交给下一层。
4. Serial 核心 (drivers/tty/serial/serial_core.c):
- 对于串口设备，这个 TTY 驱动就是串口核心。它的 write 函数是 uart_write。
- uart_write 负责将数据放入串口的缓冲区。它会遍历要发送的字符串，一次处理一个字符。
- 它会调用 uart_put_char(port, ch) 将字符放入端口的发送循环缓冲区。
- 然后调用 uart_ops->start_tx(port) 来启动或唤醒发送过程。
5. 8250 核心 (drivers/tty/serial/8250/8250_port.c):
- start_tx 操作在 8250 框架中对应 serial8250_start_tx。
- 这个函数会检查发送 FIFO 是否有空间。如果有，它会从循环缓冲区中取出字符，并通过 serial_port_out(port, UART_TX, ch) 将字符写入发送寄存器。
- serial_port_out 是一个宏，它最终会调用 port->serial_out(port, UART_TX, ch)。
6. 8250_dw.c 平台驱动:
- 关键连接点：在 dw8250_probe 期间，port->serial_out 这个函数指针已经被设置为了 DW UART 特定的实现。根据设备树的 reg-io-width 和 reg-shift，它可能被设置为：
	- dw8250_serial_out (8位访问)
	- dw8250_serial_out32 (32位访问)
	- dw8250_serial_out32be (32位大端访问)
	- ...
- 假设它被设置为 dw8250_serial_out32。
7. 硬件访问:
- dw8250_serial_out32 函数被调用。
- 它执行 writel(value, p->membase + (offset << p->regshift));。
- p->membase 是 UART 的基地址（0xFC880000 映射后的虚拟地址）。
- offset 是 UART_TX (值为0)。
- p->regshift 是 2。
- 最终，内核执行一条处理器指令，将字符数据写入到 membase + (0 << 2) 的物理地址，也就是 UART 的发送保持寄存器 (THR)。
8. 硬件:
- UART 硬件检测到 THR 被写入新数据，自动将数据移入发送移位寄存器，然后按照设定的波特率、校验位、停止位，将数据一位一位地通过 TXD 引脚发送出去。



## 如何对接 AXI-DMA 的通道
好的，我们来深入解析您提供的这段 AXI DMA 驱动中的代码。这部分代码是 DMA Engine 框架中非常核心的机制，它解释了 DMA 控制器驱动（Provider）是如何响应设备驱动（Consumer）的请求的。

`of_dma_xlate` 回调函数是整个基于设备树的 DMA 请求流程中的**“最后一公里”**，它扮演着**“翻译官”和“分配器”**的角色。

---

### 1. 宏观视角：`of_dma_controller_register` 的作用

首先，我们看 `of_dma_controller_register` 这个函数。

```c
ret = of_dma_controller_register(pdev->dev.of_node,
                                 dw_axi_dma_of_xlate, dw);
```

*   **作用**: AXI DMA 驱动在自己的 `probe` 函数中调用它，目的是向 **DMA Engine 核心**注册自己，声明：“**我（这个 AXI DMA 实例）是一个有能力提供 DMA 服务的控制器**”。
*   **参数**:
    *   `pdev->dev.of_node`: AXI DMA 自己的设备树节点。这让 DMA Engine 核心知道哪个设备树节点对应哪个 DMA 控制器。
    *   `dw_axi_dma_of_xlate`: **关键！** 这是一个函数指针，是 AXI DMA 驱动提供的 `xlate` 回调函数。`xlate` 是 "translate" 的缩写，意为“翻译”或“转换”。
    *   `dw`: AXI DMA 驱动的私有数据结构实例。这个指针会被保存在 `struct of_dma` 的 `of_dma_data` 成员中，以便回调时使用。

注册成功后，DMA Engine 核心的内部链表 `of_dma_controllers` 中就会增加一个节点，这个节点包含了 AXI DMA 的设备树节点信息和它的 `xlate` 回调函数地址。

---

### 2. 微观视角：`dw_axi_dma_of_xlate` 回调的触发与功能

现在，我们把场景切换回 `8250` 驱动请求 DMA 通道的时候。

1.  `8250` 驱动调用 `dma_request_slave_channel(dev, "rx")`。
2.  DMA Engine 核心通过设备树发现 `8250` 驱动需要 `axi_dma_0` 的服务，并且设备树中指定的 DMA 说明符（dma specifier）是 `<&axi_dma_0 1>`。
3.  DMA Engine 核心此时会：
    *   找到 `axi_dma_0` 对应的 `of_dma` 结构体。
    *   从这个结构体中取出 `of_dma_xlate` 函数指针，这个指针就指向我们正在分析的 `dw_axi_dma_of_xlate`。
    *   调用这个回调函数：`dw_axi_dma_of_xlate(dma_spec, ofdma);`

**`dw_axi_dma_of_xlate` 的功能详解**:

`xlate` 回调函数的**核心作用**是：**将设备树中通用的、抽象的 DMA 请求（`dma_spec`），翻译成对该 DMA 控制器内部具体物理通道的分配和配置，并返回一个标准的 `dma_chan` 指针。**

```c
static struct dma_chan *dw_axi_dma_of_xlate(struct of_phandle_args *dma_spec,
					    struct of_dma *ofdma)
{
    // (1) 获取私有数据
	struct dw_axi_dma *dw = ofdma->of_dma_data;
	struct axi_dma_chan *chan;
	struct dma_chan *dchan;

    // (2) 从本DMA控制器中获取一个可用的从设备通道
	dchan = dma_get_any_slave_channel(&dw->dma);
	if (!dchan)
		return NULL;

    // (3) 将通用的 dma_chan 转换为本驱动的私有通道结构
	chan = dchan_to_axi_dma_chan(dchan);
    
    // (4) 关键的“翻译”步骤
	chan->hw_handshake_num = dma_spec->args[0];
    
    // (5) 返回标准通道指针
	return dchan;
}
```

**分步解析**:

*   **(1) 获取私有数据**: `ofdma->of_dma_data` 中保存着注册时传入的 AXI DMA 驱动的实例指针 `dw`。通过它，回调函数可以访问到该 DMA 控制器的所有状态和资源。

*   **(2) 获取可用通道**: `dma_get_any_slave_channel(&dw->dma)`。这个函数会在 `dw->dma` 这个 `dma_device` 的所有通道中，寻找一个**当前未被占用**的、类型为 `DMA_SLAVE` 的通道。这确保了每个请求都能获得一个独占的物理通道。

*   **(3) 转换为私有结构**: `dchan_to_axi_dma_chan(dchan)` 是一个宏或内联函数，它通过 `container_of` 技巧，从标准的 `dma_chan` 成员指针获取到包含它的、AXI DMA 特有的 `axi_dma_chan` 结构体的指针。这使得驱动可以访问到该通道的特有配置字段。

*   **(4) 关键的“翻译”步骤**: `chan->hw_handshake_num = dma_spec->args[0];`
    *   `dma_spec`: 这个结构体包含了从设备树的 `dmas` 属性中解析出的 DMA 说明符。
    *   `dma_spec->args[0]`: 这就是 `dmas` 属性中，跟在 `phandle` 后面的第一个参数。在我们的例子 `<&axi_dma_0 1>` 中，`dma_spec->args[0]` 的值就是 **`1`**。
    *   **翻译**: 这行代码的意义是：“将设备树中请求的参数（硬件握手信号线号 `1`），配置到我刚刚分配的这个物理 DMA 通道的 `hw_handshake_num` 字段中。”
    *   **作用**: 这就完成了从抽象请求到具体硬件配置的映射。当这个 `chan` 后续被用来传输数据时，AXI DMA 驱动会知道它应该使用 `1` 号硬件握手信号线来与外设（DW-UART）进行同步。

*   **(5) 返回标准指针**: 函数最后返回的是一个标准的 `struct dma_chan *` 指针。调用者（DMA Engine 核心）并不需要知道任何 `axi_dma_chan` 的内部细节，它只通过这个标准指针来与通道交互。

### **总结：`xlate` 的角色**

可以把 `of_dma_xlate` 回调想象成一个**“租车公司的柜台服务员”**:

*   **DMA 请求 (来自 8250 驱动)**: 一个客户（8250 驱动）来到柜台，出示一张预订单（`dma_spec`），上面写着“我需要一辆与 **1 号** 登机口驳接的穿梭巴士”。
*   **xlate 回调被调用**: 服务员（`xlate` 回调）开始处理这张订单。
*   **`dma_get_any_slave_channel`**: 服务员查看车库（`dw->dma`），找到一辆空闲的穿梭巴士（`dchan`）。
*   **“翻译”和配置**: 服务员拿出对讲机，告诉司机（配置 `chan->hw_handshake_num`）：“你的任务是去 **1 号** 登机口接人”。
*   **返回 `dma_chan`**: 服务员将这辆巴士的钥匙（`dchan` 指针）交给客户。

客户不需要知道巴士的型号、司机的名字，他只需要用这把标准钥匙就能使用这辆车。同样，8250 驱动也只需要用标准的 `dma_chan` 指针就能使用这个 DMA 通道，而具体的硬件配置细节已经被 `xlate` 回调函数在幕后处理掉了。这就是 `of_dma_xlate` 的核心作用：**解耦设备驱动和 DMA 控制器驱动，并完成从抽象请求到具体硬件配置的转换**。






# AI 交互历史

## 1. 请详细介绍 Linux 中的 tty/serial/8250 
2. 请详细分析 @8250_dw.c 

好的，我们来详细探讨一下 Linux 内核中的 `tty/serial/8250` 驱动以及 `8250_dw.c` 这个具体的驱动文件。

### 1. Linux tty/serial/8250 驱动框架介绍

Linux 的 `tty/serial/8250` 是一个层次化、模块化的驱动架构，用于处理基于 8250/16550 UART (通用异步收发器) 芯片的串行通信。这个架构是 TTY 子系统的一部分，为用户空间提供了标准的 TTY 设备接口（例如 `/dev/ttyS0`）。

其结构可以大致分为以下几个层次：

**a. TTY核心层 (TTY Core)**
*   这是 Linux TTY 子系统的最顶层，为所有类型的终端设备（物理终端、伪终端、串口等）提供了一个统一的接口。
*   它处理 TTY 线路规程 (line discipline)、流控 (flow control)，并将用户空间的 `read()`/`write()`/`ioctl()` 等操作转换为对下层驱动的调用。

**b. 串口核心层 (Serial Core - `drivers/tty/serial/serial_core.c`)**
*   这一层为所有的串口驱动提供了一个通用的框架，抽象了串口设备的共性操作。
*   它定义了 `struct uart_driver` 和 `struct uart_port` 这两个核心数据结构，并负责管理它们的注册与注销。
    *   `uart_driver`: 描述一个串口驱动，包含驱动名称、设备主/次设备号、TTY 驱动指针等。
    *   `uart_port`: 描述一个独立的串口端口，包含 I/O 地址或内存映射地址、中断号、时钟、FIFO 大小以及一组操作函数指针 `uart_ops`。
*   串口核心层将来自 TTY 核心的请求（如 `open`, `close`, `set_termios`）分发到具体的 UART 驱动。

**c. 8250 核心层 (8250 Core - `drivers/tty/serial/8250/8250_core.c`, `8250_port.c`)**
*   这一层是专门为 8250 及其兼容系列 (16450, 16550A 等) UART 实现的通用驱动逻辑。由于 8250 UART 是一个非常经典和广泛使用的设计，内核为它提供了一个强大的核心支持。
*   `8250_core.c` 实现了大部分 `uart_ops` 中的函数，如中断处理、模式设置、FIFO 控制等。它处理了所有 8250 兼容 UART 的标准寄存器操作。
*   它还提供了一个注册接口 `serial8250_register_8250_port()`，允许下层的硬件平台驱动将探测到的串口注册到 8250 核心层。

**d. 平台/总线/硬件驱动层 (e.g., `8250_pci.c`, `8250_dw.c`)**
*   这是最底层，直接面向具体的硬件。这些驱动负责：
    *   **探测硬件**: 通过 PCI、PNP、ACPI 或设备树 (Device Tree) 等机制发现串口硬件的存在。
    *   **获取资源**: 获取硬件的 IO 端口、内存地址、中断号、时钟等资源。
    *   **处理硬件差异**: 不同的 8250 兼容芯片可能有细微的差异或额外的功能。这些驱动负责处理这些 "quirks"（特性或怪癖）。
    *   **注册端口**: 将探测到的端口信息填充到一个 `uart_8250_port` 结构中，并调用 `serial8250_register_8250_port()` 将其注册到 8250 核心层。

**总结一下**：当用户操作 `/dev/ttyS0` 时，请求会经过 TTY核心 -> Serial核心 -> 8250核心。8250 核心会使用特定硬件驱动（如 `8250_dw.c`）提供的 I/O 函数和配置来与硬件通信。这种分层结构极大地提高了代码的复用性，使得添加对一款新 8250 兼容 UART 的支持变得非常简单。

---

### 2. `8250_dw.c` 驱动源码分析

`8250_dw.c` 就是上述结构中**第四层**的一个典型例子。它是一个平台驱动 (platform driver)，专门用于支持 **Synopsys DesignWare APB UART**。DesignWare (DW) 是 Synopsys 公司提供的可配置 IP (知识产权) 核，被广泛用于各种 SoC (片上系统) 中。DW APB UART 在兼容 8250 标准的同时，增加了很多扩展功能。

下面我们来详细分析这个文件：

**a. 驱动的角色和目标**

*   **目标**: 驱动基于 Synopsys DesignWare IP 的 UART 控制器。
*   **功能**: 它作为 8250 核心层和具体 DW UART 硬件之间的“粘合剂”，主要负责：
    1.  初始化并注册 DW UART 端口到 8250 核心。
    2.  处理 DW UART 特有的寄存器和功能 (如分数波特率分频器、特性寄存器等)。
    3.  实现平台特有的时钟管理和电源管理。
    4.  处理硬件的特定 "quirks"。

**b. 关键数据结构 `dw8250_data`**

这是该驱动为每个串口定义的私有数据结构，用于存储 DW UART 的特有信息：
```c
struct dw8250_data {
    u8          usr_reg;            // 特有的 "UART Status Register" 寄存器偏移
    u8          dlf_size;           // "Divisor Latch Fraction" 寄存器的大小，用于分数波特率
    struct clk      *clk;               // 主时钟 (baudclk)
    struct clk      *pclk;              // APB 总线时钟
    struct reset_control *rst;            // 复位控制器
    struct uart_8250_dma dma;        // DMA 配置
    // ...
    unsigned int    uart_16550_compatible:1; // 标记硬件是否是纯16550兼容模式
};
```

**c. 驱动注册与探测 (`dw8250_platform_driver`)**

驱动通过一个 `platform_driver` 结构体向内核注册自己。
```c
static struct platform_driver dw8250_platform_driver = {
    .driver = {
        .name       = "dw-apb-uart",
        .pm     = &dw8250_pm_ops,
        .of_match_table = dw8250_of_match,
        .acpi_match_table = dw8250_acpi_match,
    },
    .probe          = dw8250_probe,
    .remove         = dw8250_remove,
};
```
*   `.name`: 驱动名称。
*   `.of_match_table`: 定义了此驱动支持的设备树 `compatible` 字符串，如 `"snps,dw-apb-uart"`。当设备树中节点的 `compatible` 属性匹配时，内核就会调用此驱动的 `probe` 函数。
*   `.acpi_match_table`: 定义了支持的 ACPI ID，如 `"INT33C4"`。
*   `.probe`: `dw8250_probe` 是驱动的核心入口点，在设备匹配成功后被调用。
*   `.remove`: `dw8250_remove` 在设备被移除时调用，用于释放资源。

**d. 核心探测函数 `dw8250_probe`**

这个函数执行了设备初始化的所有关键步骤：
1.  **获取资源**: 通过 `platform_get_resource` 获取内存映射地址 (`regs->start`)，通过 `platform_get_irq` 获取中断号。
2.  **初始化 `uart_8250_port`**: 创建一个 `uart_8250_port` 结构体，并填充大部分字段。
3.  **重载函数指针**: 将 `uart_port` 中的一些函数指针指向 DW 特定的实现，例如：
    *   `handle_irq`: 指向 `dw8250_handle_irq`，处理 DW 特有的中断。
    *   `set_termios`: 指向 `dw8250_set_termios`，处理时钟和波特率设置。
    *   `pm`: 指向 `dw8250_do_pm`，实现电源管理。
    *   `serial_in`/`serial_out`: 指向 DW 特定的寄存器读写函数。
4.  **分配私有数据**: 使用 `devm_kzalloc` 分配 `dw8250_data` 结构体。
5.  **读取平台属性**: 从设备树或 ACPI 读取 `reg-shift`, `reg-io-width`, `clock-frequency` 等平台相关配置。
6.  **时钟和复位管理**: 使用 `devm_clk_get` 获取 `baudclk` 和 `apb_pclk` 并使能它们；获取并控制复位信号。这是保证 UART 正常工作的前提。
7.  **处理硬件Quirks**: 调用 `dw8250_quirks` 函数，根据 `compatible` 字符串应用特定于平台的修复或配置（例如，为 Cavium OCTEON 平台设置 64 位访问函数）。
8.  **探测DW特性**: 调用 `dw8250_setup_port`，通过读取 `DW_UART_CPR` (Component Parameter Register) 和 `DW_UART_UCV` (UART Component Version) 等特有寄存器，自动检测硬件特性，如 FIFO 深度、是否支持自动流控 (AFE)、是否支持红外 (IrDA) 等。
9.  **注册端口**: 最关键的一步，调用 `serial8250_register_8250_port(&uart)` 将配置好的端口注册到 8250 核心。一旦成功，该串口就会出现在系统中（如 `/dev/ttyS_`）。
10. **使能电源管理**: 开启运行时电源管理。

**e. DesignWare 特有功能实现**

`8250_dw.c` 最重要的工作就是处理标准 8250 所没有的那些功能：

*   **分数波特率分频器 (Fractional Baud-Rate Divisor)**:
    *   DW UART 有一个 `DW_UART_DLF` 寄存器，允许设置一个分数分频值，从而可以更精确地生成各种非标准波特率。
    *   `dw8250_get_divisor` 和 `dw8250_set_divisor` 函数被用来计算和设置整数部分（写入标准 `DLL`/`DLM` 寄存器）和分数部分（写入 `DLF` 寄存器）。

*   **LCR 写保护和忙检测 (Busy Detect)**:
    *   DW UART 有一个特性：当 UART 忙于发送数据时，对 LCR（线路控制寄存器）的写操作可能会被忽略。
    *   如果发生这种情况，会产生一个“忙检测”中断 (`UART_IIR_BUSY`)。
    *   `dw8250_handle_irq` 会捕获此中断并清除状态。
    *   `dw8250_check_lcr` 函数会在写入 LCR 后，回读并检查值是否正确写入，如果不正确会重试，确保配置生效。

*   **时钟控制**:
    *   `dw8250_set_termios` 在设置波特率时，会通过 `clk_set_rate` 动态调整 `baudclk` 的频率，以匹配所需的波特率，这比仅仅依赖固定时钟和分频器要灵活得多。

*   **电源管理**:
    *   驱动实现了完整的运行时电源管理 (`dw8250_runtime_suspend`/`dw8250_runtime_resume`)。
    *   在挂起时，它会通过 `clk_disable_unprepare` 关闭 `pclk` 和 `clk` 来节省功耗。
    *   在恢复时，重新使能时钟。

**总结**: `8250_dw.c` 是一个优秀的驱动范例，它清晰地展示了如何在一个通用驱动框架（8250 核心）下，通过实现平台相关的探测、资源管理、Quirks 处理和扩展功能，来适配一款功能丰富的特定硬件 (Synopsys DesignWare UART)。它充分利用了内核的 Device-Driver 模型、时钟管理框架和电源管理框架。

希望这份详细的介绍和分析对您有帮助！如果您对某个具体函数或细节有更多疑问，我们可以继续深入探讨。


## 1. 请详细介绍 struct dw8250_data 结构体
2.  设备树内容如下：
uart0: serial@fc880000 {
    compatible = "snps,dw-apb-uart";
    reg = <0x00 0xFC880000 0x00 0x2000>;
    reg-io-width = <0x04>;
    reg-shift = <0x02>;
    clock-frequency = <27000000>;
    clock-names = "baudclk", "apb_pclk";
    clocks = <&sysclk &sysclk>;
    interrupt-parent = <&gic>;
    interrupts = <GIC_SPI 40 IRQ_TYPE_LEVEL_HIGH>;
};
3. 详细介绍 probe 函数，以及如何从设备树解析内容，解析后的值是多少
4. 详细介绍 struct uart_8250_port，sutrct uart_port，然后介绍它们与 dw8250_data 之间的关联
5. 详细说明 如何向 8250 中添加一款新的 uart ip 驱动


好的，我们来逐一深入探讨这些问题。这是一个非常好的提问，涵盖了从硬件描述到驱动实现再到框架关联的整个流程。

### 1. `struct dw8250_data` 结构体详解

这个结构体是 `8250_dw.c` 驱动为每个 DesignWare UART 端口定义的**私有数据**。它存储了标准 8250 模型之外、属于 DesignWare IP 特有的信息。`uart_port` 结构体中的 `private_data` 指针会指向这个结构体的实例。

```c
// drivers/tty/serial/8250/8250_dw.c

struct dw8250_data {
    // DW 特有寄存器和配置
    u8          usr_reg;            // UART Status Register (USR) 的寄存器偏移量。
                                    // 这是一个 DW 特有的状态寄存器，默认是 DW_UART_USR (0x1f)。

    u8          dlf_size;           // Divisor Latch Fraction (DLF) 寄存器的大小（位数）。
                                    // DW UART 支持分数波特率分频器，此字段记录分数部分寄存器的能力，
                                    // 在 dw8250_setup_port 中通过写-读硬件来探测。

    // 8250 核心层交互
    int         line;               // 由 8250 核心层分配的串口线路号 (e.g., 0 for ttyS0)。
                                    // 在 serial8250_register_8250_port() 成功后被赋值。

    // 硬件信号模拟 (Quirks)
    int         msr_mask_on;        // 用于软件强制 MSR (Modem Status Register) 某些位为1的掩码。
    int         msr_mask_off;       // 用于软件强制 MSR 某些位为0的掩码。
                                    // 这通常用于在设备树中通过 "dcd-override" 等属性来模拟
                                    // DCD/DSR/CTS 等 Modem 信号，绕过硬件限制。

    // 平台资源
    struct clk      *clk;               // 指向 "baudclk" 的指针。这是UART的核心时钟，用于产生波特率。
                                    // 驱动可以通过时钟框架调整它的频率。

    struct clk      *pclk;              // 指向 "apb_pclk" 的指针。这是APB总线时钟，用于访问UART寄存器。

    struct reset_control *rst;      // 指向复位控制器的指针，用于在初始化时解除对UART IP的复位。

    // DMA 功能
    struct uart_8250_dma  dma;      // 包含DMA相关的配置和回调函数。

    // 行为标志
    unsigned int    skip_autocfg:1; // 一个比特位标志，如果为1，则跳过在 probe 阶段对 DW UART
                                    // 特性的自动探测（dw8250_setup_port）。某些平台需要硬编码配置。

    unsigned int    uart_16550_compatible:1; // 一个比特位标志，如果为1，则表示该硬件虽然由本驱动匹配，
                                           // 但行为上是纯粹的16550兼容UART，没有DW的扩展功能
                                           // (如忙检测中断)，需要跳过相关处理。
};
```

---

### 2. & 3. `dw8250_probe` 函数与设备树解析

`dw8250_probe` 函数是驱动的核心。当内核的设备-驱动匹配机制发现设备树中有一个节点的 `compatible` 属性与本驱动的 `of_match_table` 中的条目相匹配时，就会调用此函数。

下面，我们以您提供的设备树片段为例，一步步解析 `dw8250_probe` 函数是如何工作的，以及解析出的值是多少。

**设备树节点:**
```dts
uart0: serial@fc880000 {
    compatible = "snps,dw-apb-uart"; // (A)
    reg = <0x00 0xFC880000 0x00 0x2000>; // (B)
    reg-io-width = <4>; // (C)
    reg-shift = <2>; // (D)
    clock-frequency = <27000000>; // (E)
    clock-names = "baudclk", "apb_pclk"; // (F)
    clocks = <&sysclk &sysclk>; // (G)
    interrupt-parent = <&gic>;
    interrupts = <GIC_SPI 40 IRQ_TYPE_LEVEL_HIGH>; // (H)
};
```

**`dw8250_probe` 执行流程:**

1.  **匹配与调用**: 内核发现 `compatible = "snps,dw-apb-uart"` **(A)**，与 `dw8250_of_match` 表匹配，调用 `dw8250_probe`。

2.  **获取内存资源 (B)**:
    *   `regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);`
    *   此函数解析 `reg` 属性。
    *   **解析结果**: `regs->start` (物理基地址) = `0xFC880000`，`resource_size(regs)` = `0x2000` (8KB)。
    *   `p->mapbase` 被设为 `0xFC880000`。
    *   `p->membase` 通过 `devm_ioremap` 将物理地址映射为内核虚拟地址。

3.  **获取中断资源 (H)**:
    *   `irq = platform_get_irq(pdev, 0);`
    *   此函数解析 `interrupts` 属性。
    *   **解析结果**: 内核的中断子系统会将 `GIC_SPI 40` 转换为一个 Linux IRQ 编号（一个整数）。`p->irq` 被设置为这个 IRQ 编号。

4.  **初始化 `uart_port` 结构体**:
    *   创建一个 `uart_8250_port uart` 局部变量。
    *   `p` 指向 `&uart.port`。
    *   设置 `p->handle_irq = dw8250_handle_irq;` 等函数指针，覆盖8250核心的默认实现。

5.  **分配私有数据 `dw8250_data`**:
    *   `data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);`
    *   分配 `dw8250_data` 结构体，并让 `p->private_data = data;` 指向它。

6.  **解析 `reg-shift` (D)**:
    *   `device_property_read_u32(dev, "reg-shift", &val);`
    *   **解析结果**: `val` = `2`。
    *   `p->regshift` 被设为 `2`。这表示寄存器地址之间的偏移是对数（`2^2 = 4` 字节）。例如，`UART_LSR` 在 `UART_RX` 之后，它的地址是 `membase + (UART_LSR << 2)`。

7.  **解析 `reg-io-width` (C)**:
    *   `device_property_read_u32(dev, "reg-io-width", &val);`
    *   **解析结果**: `val` = `4`。
    *   因为 `val == 4`，驱动会设置 `p->iotype = UPIO_MEM32;`，并使用32位宽的读写函数 `dw8250_serial_in32` 和 `dw8250_serial_out32`。

8.  **解析时钟 (E, F, G)**:
    *   `device_property_read_u32(dev, "clock-frequency", &p->uartclk);`
        *   **解析结果**: `p->uartclk` 被**临时**设为 `27000000`。这是一个备用值。
    *   `data->clk = devm_clk_get(dev, "baudclk");`
        *   此函数根据 `clock-names` **(F)** 找到名为 `"baudclk"` 的时钟（第一个），然后从 `clocks` **(G)** 属性中获取对应的时钟提供者（第一个 `&sysclk`）。
        *   **解析结果**: `data->clk` 指向 `sysclk` 的时钟结构体。
        *   随后，`p->uartclk = clk_get_rate(data->clk);` 会用 `baudclk` 的实际频率**覆盖** `clock-frequency` 的值。
    *   `data->pclk = devm_clk_get(dev, "apb_pclk");`
        *   类似地，找到名为 `"apb_pclk"` 的时钟（第二个），并从 `clocks` 获取其提供者（第二个 `&sysclk`）。
        *   **解析结果**: `data->pclk` 指向 `sysclk` 的时钟结构体。
    *   驱动接着会调用 `clk_prepare_enable()` 来使能这两个时钟。

9.  **硬件特性探测**:
    *   `dw8250_quirks(p, data);` 被调用，但对于 `"snps,dw-apb-uart"`，此函数内没有特殊操作。
    *   `dw8250_setup_port(p);` 被调用。它会通过读写 `DW_UART_CPR` 和 `DW_UART_UCV` 寄存器来探测 FIFO 深度、是否支持分数波特率等，并更新 `p->fifosize` 和 `data->dlf_size` 等字段。

10. **注册端口**:
    *   `data->line = serial8250_register_8250_port(&uart);`
    *   这是最关键的一步。它将完全配置好的 `uart` 端口注册到 8250 核心层。如果成功，`data->line` 会被赋值为一个非负整数（如 0, 1, ...），一个 `/dev/ttySn` 设备就会被创建。

---

### 4. `uart_port`, `uart_8250_port`, `dw8250_data` 关联

这三个结构体体现了驱动框架的层次化设计思想：**泛化 -> 特化 -> 私有化**。

*   **`struct uart_port` (泛化)**
    *   定义在 `<linux/serial_core.h>`。
    *   是**串口核心层**的抽象，代表一个通用的串口端口，与具体 UART 芯片无关。
    *   包含最基本的硬件资源（`mapbase`, `membase`, `irq`）、状态（`flags`, `type`）、操作函数集（`struct uart_ops *ops`），以及一个 `void *private_data` 指针。

*   **`struct uart_8250_port` (特化)**
    *   定义在 `<linux/serial_8250.h>`。
    *   是为 **8250 系列芯片** 特化的结构。
    *   它的**第一个成员就是 `struct uart_port port;`**。这是C语言中实现“继承”的常用技巧。这意味着一个 `uart_8250_port` 的指针可以直接强制转换为 `uart_port` 指针，并且它们指向相同的内存地址。
    *   在 `uart_port` 的基础上，它增加了 8250 芯片特有的字段，如 `struct uart_8250_dma *dma`、`unsigned int capabilities` (FIFO, AFE能力) 等。

*   **`struct dw8250_data` (私有化)**
    *   定义在 `8250_dw.c` 中。
    *   是**具体驱动**的私有数据，用于存储 Synopsys DW UART 特有的信息，如 `dlf_size`, `clk`, `rst` 等。

**它们之间的关联如下：**

```
   +---------------------------------------+
   |        struct uart_8250_port          |
   |  (在 8250_dw.c 的 probe 函数中创建)   |
   |                                       |
   |  +---------------------------------+  |
   |  |      struct uart_port port      |  |
   |  |---------------------------------|  |
   |  | void *private_data; -----------|--+
   |  | ... (通用串口成员) ...          |  | |
   |  +---------------------------------+  | |
   |                                       | |
   |  ... (8250 专用成员) ...              | |
   |                                       | |
   +---------------------------------------+ |
                                             |
     (p->private_data = data)                |
                                             v
                                  +-----------------------+
                                  |   struct dw8250_data  |
                                  |  (dw8250.c 私有数据)  |
                                  |-----------------------|
                                  | struct clk *clk;      |
                                  | struct clk *pclk;     |
                                  | u8 dlf_size;          |
                                  | ...                   |
                                  +-----------------------+
```

**关联流程:**
1.  在 `dw8250_probe` 中，`struct uart_8250_port uart;` 在栈上被创建。
2.  `struct dw8250_data *data` 通过 `devm_kzalloc` 在堆上被创建。
3.  通过 `uart.port.private_data = data;` 这条语句，将两者关联起来。
4.  当驱动调用 `serial8250_register_8250_port(&uart)` 时，它将 `uart_8250_port` 注册到 8250 核心。
5.  之后，当串口核心或8250核心调用某个操作函数（例如 `dw8250_set_termios`）时，会传入一个 `struct uart_port *p` 参数。驱动内部可以通过 `struct dw8250_data *d = p->private_data;` 很方便地获取到自己的私有数据，从而操作 `clk`、`dlf_size` 等特有成员。

---

### 5. 如何向 8250 框架添加一款新的 UART IP 驱动

假设你要为一款名为 "MyUART" 的、与8250兼容但有自己特色的IP添加驱动，可以遵循以下步骤：

1.  **创建驱动文件**:
    *   在 `drivers/tty/serial/8250/` 目录下创建新文件 `8250_myuart.c`。
    *   在 `drivers/tty/serial/8250/Kconfig` 中添加一个 `CONFIG_SERIAL_8250_MYUART` 选项。
    *   在 `drivers/tty/serial/8250/Makefile` 中添加 `obj-$(CONFIG_SERIAL_8250_MYUART) += 8250_myuart.c`。

2.  **定义平台驱动结构**:
    *   在 `8250_myuart.c` 中，定义一个 `platform_driver`。
    ```c
    static const struct of_device_id myuart_of_match[] = {
        { .compatible = "mycorp,myuart" }, // 你的设备树 compatible 字符串
        { /* Sentinel */ }
    };
    MODULE_DEVICE_TABLE(of, myuart_of_match);

    static struct platform_driver myuart_platform_driver = {
        .driver = {
            .name = "myuart",
            .of_match_table = myuart_of_match,
        },
        .probe  = myuart_probe,
        .remove = myuart_remove,
    };
    module_platform_driver(myuart_platform_driver); // 注册驱动
    ```

3.  **实现 `probe` 函数 (`myuart_probe`)**:
    *   这是工作的核心，可以大量参考 `8250_dw.c`。
    *   **基础设置**: 获取 `reg`, `irq` 资源，创建 `uart_8250_port` 实例。
    *   **处理差异点 (关键！)**:
        *   **私有数据**: 如果 MyUART 有特殊状态需要保存，定义一个 `struct myuart_data`，并像 `dw8250` 一样分配它并链接到 `private_data`。
        *   **寄存器访问**: 如果寄存器位宽或偏移不是标准的 `regshift=0, width=1`，则根据设备树设置 `p->iotype`, `p->regshift`，并选择合适的 `serial_in`/`serial_out` 函数（或者自己实现）。
        *   **时钟/复位**: 如果 MyUART 有自己的时钟或复位信号，使用 `devm_clk_get` 和 `devm_reset_control_get` 从设备树获取并管理它们。
        *   **特殊功能**: 如果 MyUART 有特殊功能（比如一个独特的波特率设置方法），你需要：
            *   覆盖 `uart_port` 中的相关函数指针，例如 `p->set_termios = myuart_set_termios;`。
            *   在你的 `myuart_set_termios` 函数中实现独特的逻辑。
        *   **Quirks**: 如果硬件有怪癖，在 `probe` 函数中进行处理。

4.  **实现 `remove` 函数 (`myuart_remove`)**:
    *   这个函数在驱动卸载时被调用，负责释放 `probe` 中申请的资源。
    *   主要工作是调用 `serial8250_unregister_port()`，并禁用时钟、释放中断等。使用 `devm_*` 系列函数可以大大简化这个过程。

5.  **编写设备树节点**:
    *   在你的板级设备树文件 (`.dts`) 中，为 MyUART 添加一个节点，提供所有必要的属性。
    ```dts
    myuart_node: serial@deadbeef {
        compatible = "mycorp,myuart";
        reg = <0xdeadbeef 0x1000>;
        interrupts = <...>;
        clocks = <...>;
        clock-names = <...>;
        /* 其他自定义属性 */
    };
    ```

通过以上步骤，你就可以将一个新的 UART IP 无缝地集成到健壮的 Linux 8250 串口子系统中。

## 1. probe 函数中所有的信息基本都保存在了 struct uart_port p 中，但是 p = &uart.port，struct uart_8250_port uart 是一个局部变量，当 probe 函数结束之后就没有用了，那么这些信息在后面使用的时候不就丢失了吗？请详细分析这里的处理方式
2. 设备树中为什么还要区分 clock-frequency、baudclk？请依次介绍这些 clk 的作用
3. 请介绍 uart 的交互流程，用户空间的测试程序如何调度到 @8250_dw.c 


