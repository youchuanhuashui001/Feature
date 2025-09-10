
# 概述
- drawio 流程图路径：linux_driver. drawio --> irr
- 代码路径：
	- `driver/nationalchip/irr/gx_irr.c`
	- `driver/nationalchip/irr/gx_irr_pt2264.c`

# 系统集成

##  .config
```
CONFIG_NATIONALCHIP=y

#
# NationalChip Device
#
CONFIG_GX_IRR=y
```

##  Kconfig
```
config NATIONALCHIP
	tristate "NationalChip Device Support"
        help
          This enable nationalchip driver

if NATIONALCHIP
    menu "NationalChip Device"

        config GX_IRR
            tristate "NationalChip IR Remote"
            default y

#        config GXOSAL
#            tristate "NationalChip OSAL"
            default y

    endmenu
endif
```

## Makefile
```
obj-$(CONFIG_GX_IRR)=gxirr.o
gxirr-objs += gx_irr.o gx_irr_pt2264.o
```

## dts
```
gx_irr: gx_irr@fa420000 {
    compatible  = "nationalchip,gx-irr";
    #address-cells = <1>;
    #size-cells = <1>;
    reg = <0 0xfa420000 0 0x10>;
    reg-names = "gx_irr.regs";
    interrupt-parent = <&gic>;
    interrupts = <GIC_SPI 94 IRQ_TYPE_LEVEL_HIGH>;
    interrupt-names = "gx_irr.irq";
    clock-frequency = <1000000>;
    system-clock-frequency = <27000000>;
    status = "okay";
};
```


# 驱动分析

## gx_irr_probe
![[Pasted image 20250618203824.png]]


## irr_keymap_get_count、irr_keymap_init
![[Pasted image 20250618203920.png]]



## irr_isr
![[Pasted image 20250618203941.png]]



## 对上层的接口：

### 字符设备：/dev/gxirr0
![[Pasted image 20250618204046.png]]

### input 设备：/dev/input/eventX
![[Pasted image 20250618204054.png]]


![[drawnix-1756276123438.png]]




# 关联知识

## Input 子系统
比如按键输入、键盘、鼠标、触摸屏等
等这些都属于输入设备，不同的输入设备所代表的含义不同，按键和键盘就是代表按键信息，
鼠标和触摸屏代表坐标信息，因此在应用层的处理就不同，对于驱动编写者而言不需要去关心
应用层的事情，我们只需要按照要求上报这些输入事件即可。

硬件的输入信息 --> 内核空间 --> 用户空间
![[Pasted image 20250618143058.png]]
左边是具体的硬件设备，中间部分属于 Linux 内核空间，分为驱动层、核心层和事件层，最右边的就是用户空间，所有的输入设备以文件的形式供用户应用程序使用。

驱动层：输入设备的具体驱动程序，比如按键驱动程序，向内核层报告输入内容。
核心层：承上启下，为驱动层提供输入设备注册和操作接口。通知时间曾对输入事件进行处理。
- input 核心层在 `drivers/input/input.c` 向 `Linux` 内核注册**批量的字符设备**以及类
	- `/dev/input, sys/class/input`
- input 子系统的所有设备主设备号都为 13，在使用 input 子系统处理输入设备的时候就不需要注册字符设备了，只需要向 input 子系统注册一个 input_device 即可。
	- `struct input_dev *input_allocate_device(void)`
	- `int input_register_device(struct input_dev *dev)`
- 使用流程：
	- 注册 input_dev
		- 使用 input_allocate_device 函数申请一个 input_dev。
		- 初始化 `input_dev` 的事件类型以及事件值
		- 使用 input_register_device 函数向 Linux 系统注册前面初始化好的 input_dev
		- 卸载时先 input_unregister_device 注销掉注册的 input_dev；然后使用 input_free_device 释放前面申请的 Input_dev。
	- 上报输入事件
		- 不同的事件，其上报事件的 API 函数不同
		- 通过函数 `input_evenv` 函数上报指定的事件以及对应的值
		- 上报结束后使用 `input_sync` 告诉 input 子系统上报结束
事件层：主要和用户空间进行交互。


# Q&&A

## 资源如何使用？

### mutex
- 读红外数据的时候会先锁住，读完再解锁
- 持锁的线程会被挂入等待队列并调度；未持锁的线程会休眠等待；保障同一时刻仅有一个执行流访问临界资源.

### completion
- `irr_isr` 中，如果是堵塞模式，会置位完成量
- `irr_read` 中，会一直等待完成量完成
- 线程 A 等待某个条件/事件完成，线程 B 触发事件，通知 A 继续。

### timer
- timer 来源及用法：
    - arm 存在系统定时器：ARM Genieic Timer; `jiffies` 是内核全局递增的 tick 计数；定时器硬件的中断最终触发 `softirq` 执行回调。
    - `timer_setup(struct timer_list *t, void (*cb)(struct timer_list *), unsigned flags)`
    - `mod_timer(t, expires)` 修改/启动
    - `del_timer(t)` 取消
- 驱动中的 timer 功能：
    - probe 创建一个 1000ms 的 timer, 定时事件到执行回调函数中会清除键值
    - isr 在每次收到按键中断后重新设定超时，修改 timer，定时 1s,确保最后一次按键后 1000ms 自动清零
    - 防抖 + 状态复位：若用户长时间不按键，定时器回调把键值、重复计数等清零，避免旧值影响下一次解析

### kfifo
- 驱动中的 fifo 功能：
    - kfifo1：
        - 大小为 2 个键值，使用 `info->queue` 保存
        - isr 中解析到键值后，通过 `kfifo_in` 将键值放入到 `info->queue`，供 `read()` 系统调用读取
        - 字符设备的 read 接口通过 `kfifo_out` 将键值从 `info->queue` 取出，拷贝给用户空间
    - kfifo2：
        - 大小为 2 个键值，使用 `info->event_queue` 保存
        - isr 中解析到键值后，通过 `kfifo_in` 将键值放入到 `info->evenv_queue`，供 tasklet 在 `irr_send_evenv()` 转发到 input 子系统
        - tasklet 回调通过 `kfifo_out` 将键值从 `info->evenv_queue` 取出，与键值进行匹配，匹配成功后向 input 子系统发起按键事件
    - kfifo_len：
        - 字符设备的 poll 接口配合 `kfifo_len(info->queue)` 判断是否有数据

### input_dev
- 驱动中的 input_dev 功能：
    - 通过接口 `input_allocate_device` 申请一个 input_dev，用 `info->input` 保存
    - 配置 input_dev 支持的事件类型(`input_dev->evbit`)：`KEY`、`REP`、`SYN`
    - 配置 input_dev 的其他信息
    - 通过接口 `input_register_device(input_dev)` 注册到 input 子系统
    - 在获取到键值之后通过 `input_evenv` 将事件类型和键值都上报给 input 子系统
    - 通过 `input_sync` 说明上报事件结束
- 上层如何获取 input 事件：
    - 上面注册 `input_device` 成功后，会在 `/dev/input` 目录下生成一个名为 `eventX` 的文件，可以通过读取该文件获取到输入事件信息，比如按键值之类的。使用 read 函数读取 `/dev/input/eventX`，读到的数据按照 `input_evenv` 结构体的方式解析出来，即可处理对应的事件
    - `fd = open (/dev/input/event1)`
    - `read(fd, &inputevenv, sizeof(inputevenv))`
    - `switch (inputevenv.type)`
        - `case EV_KEY: printf("Key %d %s \n", inputevenv.code, inputdev.valud ? "press" : "release")`

### tasklet
- tasklet 用法：
    - 属于软中断框架的轻量级下半部机制，让时间较长的工作避开硬中断，但仍保持在中断上下文执行
    - 初始化：`tasklet_init(struct tasklet_struct *, callback_func, data)`
    - 调度：`tasklet_schedule(&my_tl); tasklet_hi_schedule(&my_tl);`
    - 释放：`tasklet_kill(&my_tl);`
- 驱动中的 tasklet 功能：
    - probe 时注册 tasklet：`tasklet_init(&info->irr_tasklet, irr_send_event, (unsigned long)info);`
    - 调度位置：isr 中解析到键值后调用 `tasklet_hi_schedule(&info->irr_tasklet);`
    - 回调函数用于获取键值并上报给 input 子系统
    - remove 时移除 tasklet：`tasklet_kill`

### waitqueue
- waitqueue 用法：
    - 提供一种 **睡眠-唤醒** 机制：让进程在自愿条件下挂起，并由另一执行流在事件到来时把它唤醒。
    - 初始化：`init_waitqueue_head(&my_wq)`
    - 休眠/等待：`wait_evenv(wq, condition); wait_evenv_interruptible(wq, condition);`
    - 唤醒：`wake_up(&wq); wake_up_interruptible(&wq)`
    - 内核提供 `poll_wait(file, &wq, poll_table);` 在 poll 低依次调用时将当前任务挂入 wq，当事件发生后驱动之星 wake_up，poll 系统调用就会返回
- 驱动中的 waitqueue 功能：
    - probe 时初始化 waitqueue：`init_waitqueue_head(&info->irr_read_queue);`
    - poll 接口中将调用者挂入等待队列：`poll_wait(filp, &info->irr_read_quque, table);`
        - 若 fifo 为空，`poll_wait()` 会为当前进程创建 `wait_queue_entry` 并加入 `irr_read_queue` 然后返回；select/poll 系统调用会睡眠。
        - 如果已有数据，直接返回 `POLLIN`，应用立即得到可读通知。
    - isr 获取到键值后通过 `wake_up(&info->irr_read_quque);` 唤醒休眠的进程，select/poll 得以返回，再调用 `read()` 获取按键键值。
    - 仅服务于 select/poll 提供存在红外码值可读的异步通知。

### misc_dev
- `open`：将 `g_info` 放到 `filp->private_data`
- `release`：不处理
- `read`：
    - 上锁；
    - 如果 `kfifo(info->queue)` 为空，等待完成量完成；完成量完成后会重新判断 kfifo 是否非空；
    - 如果 `kfifo(info->queue)` 非空，调用 `kfifo_out` 从 `info->quque` 取出数据并通过 `copy_to_user` 返回数据给用户空间。
    - 解锁；
- `poll`：用户进程监视多个文件描述符，直到其中一个或多个就绪，或超时；如果所有 fd 都未就绪，则进程睡眠。
    - 把当前进程关联到驱动的 `irr_read_queue` 等待队列头，此时进程并不马上休眠，还会检查其余的 fd。
    - 若 fifo 非空，返回 `POLLIN`，`poll()` 系统调用立即返回，表示可读。
    - 若 fifo 为空，返回 0；如果所有 fd 都返回 0，用户进程开始睡眠。


# AI 交互历史

## 请深入分析 @gx_irr_pt2264.c 该驱动代码

以下内容基于你提供的 gx_irr_pt2264.c 完整源码（1-1443 行）逐段梳理驱动的整体架构、关键数据结构、核心执行路径以及各类红外协议解析算法。分析以“自上而下 + 关键路径”的思路展开，方便后续 debug / 二次开发 / 移植。

1. 驱动定位与整体框架  
   • 本文件是国芯(GX) SoC 上红外接收控制器（IRR，Infra-Red Receiver）驱动的主体，实现了：  
     - IR 硬件初始化、IRQ 注册与处理中断获取脉冲序列  
     - 多协议自适应判定 + 专用解析算法（NEC、RC5/RCMM、DVB-40、BESCON、Panasonic、XMP、PT2264）  
     - 去抖/简码过滤、FIFO 队列缓存  
     - 两种上层接口：字符设备 `/dev/gxirr0` + input 子系统按键事件  
     - 设备树 keymap 解析，支持多遥控器系统码映射到 Linux keycode  

2. 关键全局变量  
   96:115:gx_irr_pt2264.c
   ```c
   static volatile unsigned int g_irr_simple_ignore_num = 4;   //重复键忽略次数
   static volatile unsigned int g_irr_clear_code_interval = 1000; //自动清零定时器(ms)
   static struct irr_key_map_table *irr_key_map = NULL;        //DTS 构建的键值映射表
   static struct gx_irr_info *g_info = NULL;                   //全局设备实例
   ```
   • g_irr_simple_ignore_num 与 counter_simple 配合，用于长按过滤「简码(repeat code)」。  
   • g_irr_clear_code_interval 对应 timer 中断，在一定空闲时清除 last code，避免旧值残留。

3. 数据结构概览（定义在 gx_irr.h，本文档引用）  
   • struct irr_regs - 软映射的 IRR 寄存器集  
   • struct gx_irr_info - 驱动核心上下文：寄存器基址、时钟、kfifo、tasklet、timer、input_dev 等  
   • struct irr_algorithm - 当前所选协议解析回调  
   • struct irr_code_info_t - 传递到用户态/上层的解析结果（code、proto、is_simcode 等）  
   • struct irr_key_map_table - keymap，60 键/遥控器上限

4. 硬件资源申请与初始化  
   • gx_irr_probe() → gx_irr_chip_probe()：解析 DTS，映射寄存器(IORESOURCE_MEM)、获取时钟/IRQ。  
   • gx_irr_inithw() 设置 irr_cntl/irr_div：不同 CPU 品牌（默认 / pt2264）分支有差异。  
   • request_irq → ISR irr_isr()；timer_setup → irr_clear_code()；kfifo_alloc 两套队列(queue / event_queue)。  
   • input_allocate_device() 注册 input-event；misc_register() 暴露字符设备。  

5. 中断处理主线  
   1135:1205:gx_irr_pt2264.c
   ```c
   irr_isr():
       - 读取 FIFO 获取 pulse_num / pulse_val[]                 --> irr_get_fifo_data()
       - 自适应判断协议                                      --> irr_protocol_detect()
       - 执行对应解析算法                                     --> info->algo->functionality()
       - 将结果封装入 irr_code_info_t，写入 kfifo            --> kfifo_in()
       - 唤醒阻塞进程 / poll / tasklet 发送 input_event        --> tasklet_hi_schedule()
       - 启动/刷新 “清零” 定时器                              --> mod_timer()
   ```
   解析失败路径统一跳转 exit，最后重新开启 IRR 模块。

6. 协议判定与解析算法  
   6.1 协议快速判定 irr_protocol_detect()  
       - 基于 pulse_num、首脉冲宽度 & 低电平长短等特征选择回调。  
       - special case：17 脉冲 → RCMM 或 XMP。  
       - ≤26 且 ≥22 脉冲 → PT2264。  
   6.2 典型解析流程（以 NEC 为例）  
       • IGNORE：num <= 2 → simcode 计数; 完整码 32-pulse → 循环左移/时宽比获取 bit；最后更新 info->key_code。  
   6.3 其它协议要点  
       • Philips RC5：双相位编码，每脉冲拆成高/低再转 2bit。  
       • RCMM32：首 pulse = 416μs 高频；后续根据低电平 1T/2T/3T/4T 译 00/01/10/11。  
       • DVB 40bit：40 bit 数据分四段解析，注释“STB40”。  
       • BESCON：采用特殊 17bit 帧结构，支持缺失补偿。  
       • Panasonic：长按检测通过两次中断间的 jiffies 差判断。  
       • XMP：以 ns 为单位，计算 divider，把收到的 16 个 nibbles 校验两段 checksum=0xF。  
       • PT2264：适用于无线门铃/PT2264，脉冲数 21-26，pair 检测、二次去抖。

7. 简码(Simple-code)过滤  
   - 多数协议均在解析后判断简码标志位/toggle 或前后帧对比，配合全局 g_irr_simple_ignore_num 实现。  
   - 用户可通过 ioctl IRR_SIMPLE_IGNORE_CONFIG 动态修改阈值。

8. 用户空间接口  
   • 字符设备 `/dev/gxirr0`  
     - read(): 若队列为空则阻塞/非阻塞返回; struct irr_code_info_t 直接拷给用户。  
     - poll(): 支持 select/epoll.  
     - ioctl(): 模式切换(O_NONBLOCK)、enable/disable、忽略阈值/清零间隔配置。  
   • Input 子系统  
     - irq → tasklet irr_send_event()  
     - 根据用户 keymap 把 code 映射为 Linux key_* 并 input_event()；同步后长按由 input-core 处理。

9. DTS keymap 支持  
   • irr_keymap_get_count() 遍历子节点统计遥控器数量；每子节点需提供属性  
     - gxirr,syscode : uint32_t  
     - gxirr,key_table : <irrcode keycode …>  
   • irr_keymap_init() 解析并保存至 irr_key_map。  
   • 用户可通过 `code_print=1`/`keymap_print=1` 模块参数打印解码与 keymap。

10. 典型调用链 (开机后一次按键)  
   ```
   probe → request_irq
        ↳ 用户按键 →
            → 硬件 FIFO & 中断 →
                irr_isr() →
                    irr_get_fifo_data()
                    irr_protocol_detect()
                    ▷ irr_parse_xxx() 成功
                    kfifo_in(queue/event_queue)
                    tasklet_hi_schedule(irr_send_event)
                    mod_timer(clear_code)
                → tasklet(rtl) →
                    irr_send_event()
                        input_event(EV_KEY press/release)
                        → /dev/input/eventX 被上层捕捉
   ```

11. 特色与可改进点  
   • 特色  
     - 多协议单文件实现，便于固件级裁剪。  
     - 简码过滤 + 定时清零保证长按/抖动体验。  
     - 双接口设计兼顾传统 ioctl 应用与新 input-layer。  
   • 改进建议  
     1. 多协议 if-else 判定可改为 table-driven，提高可维护性。  
     2. g_* 全局变量较多，可收拢进 gx_irr_info 或 static 局部，减少并发风险。  
     3. 解析算法中大量 magic number，建议统一宏命名或注释来源。  
     4. XMP/Panasonic 等协议计算采用 ns/μs 混合，最好统一单位以免平台移植误差。  
     5. 输入键码映射失败仅打印日志，可在 debug 选项下 fallback 发送 raw scancode。  

如果你需要：  
   • 指定协议的时序草图 / Mermaid 时序图  
   • DTS 示例片段  
   • 代码修改或移植到新内核版本  
请告诉我，我可以进一步细化。

## 1.  如何对外提供接口？ 2. 对外提供的接口有哪些？如何实现的？用户空间如何进行调用？

Ran tool

Ran tool

Read file: irr/gx_irr_pt2264.c

Read file: gx_irr_pt2264.c

Ran tool

Read file: gx_irr.h

Read file: gx_irr_pt2264.c

Read file: gx_irr.h

下面分两部分回答：  
A) 内核端究竟“暴露”了哪些对外接口、在代码中怎样实现  
B) 用户空间如何使用这些接口，包括典型调用示例  

────────────────────────────────────  
A) 驱动对外提供的接口 & 实现方式  
1. 字符设备接口（miscdevice → /dev/gxirr0）  
   • 注册位置  
     ```1278:1285:gx_irr_pt2264.c
     struct file_operations irr_fops = {
         .owner          = THIS_MODULE,
         .open           = irr_open,
         .read           = irr_read,
         .poll           = irr_poll,
         .unlocked_ioctl = irr_ioctl,
         .release        = irr_release,
     };
     1287:1292:gx_irr_pt2264.c
     static struct miscdevice irr_miscdev = {
         .minor = MISC_IRR_MINOR,   //156
         .name  = "gxirr0",
         .fops  = &irr_fops,
     };
     ```
   • `gx_irr_probe()` 最后调用 `misc_register(&irr_miscdev)` 将其创建为 `/dev/gxirr0`。  
   • 数据通道：  
     - 中断 → `kfifo_in(INFO_QUEUE, …)` 把解析后的 `irr_code_info_t` 写进 FIFO  
     - 用户调用 `read()` 从 FIFO 取数据，结构体定义如下：  
       ```45:60:gx_irr.h
       struct irr_code_info_t {
           unsigned int code;            //32bit 原始键值
           uint64_t     code_extended;   //某些协议的扩展码
           unsigned int is_simcode;      //1=重复键
           irr_protocol_t irr_proto;     //协议枚举
       };
       ```  
   • `ioctl()` 提供运行时参数配置，宏值见头文件：  
     ```52:58:gx_irr.h
     #define IRR_SET_MODE                   0x1000
     #define IRR_SIMPLE_IGNORE_CONFIG       0x3000
     #define IRR_DISABLE                    0x4000
     #define IRR_ENABLE                     0x5000
     #define IRR_CLEAR_CODE_INTERVAL_CONFIG 0x6000
     ```  
   • `poll()`/`select()` 通过 `wait_queue_head_t irr_read_queue` 支持异步事件通知。  

2. Input-event 接口（/dev/input/eventX，属于 Linux input 子系统）  
   • 创建流程  
     - `input_dev = input_allocate_device();`  
     - 设置事件位：`set_bit(EV_KEY/EV_REP/EV_SYN, input_dev->evbit);`  
     - `input_register_device(input_dev);`  
   • 事件产生  
     ```1089:1105:gx_irr_pt2264.c
     static void irr_send_event(unsigned long data)
     {
         … kfifo_out(INFO_EVENT_QUEUE,&irr_data,…);
         key_event = irr_keycode_get(&irr_data, info->keymap_num); //根据 DTS keymap 转 Linux keycode
         if (key_event >= 0) {
             input_event(info->input, EV_KEY, key_event, 1); //按下
             input_event(info->input, EV_KEY, key_event, 0); //松开
             input_sync(info->input);
         }
     }
     ```  
     IRQ → `tasklet_hi_schedule()` 触发该 tasklet，最终向 input core 报送 EV_KEY。  
   • 该接口屏蔽了协议细节，直接向用户空间呈现标准键盘按键事件。  

3. sysfs/调试接口  
   本文件未单独创建 sysfs 节点；调试开关通过模块参数实现：  
   - `code_print`、`keymap_print` 可 `echo 1 > /sys/module/gx_irr_pt2264/parameters/…`  

────────────────────────────────────  
B) 用户空间调用示例  
1. 直接读取原始解码结果（字符设备 /dev/gxirr0）  
```c
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include "gx_irr.h"      //拷贝头文件中 ioctl/结构体 定义

int main(void)
{
    int fd = open("/dev/gxirr0", O_RDONLY);
    struct irr_code_info_t info;

    while (1) {
        if (read(fd, &info, sizeof(info)) == sizeof(info)) {
            printf("code=0x%08x proto=%d repeat=%d\n",
                   info.code, info.irr_proto, info.is_simcode);
        }
    }
}
```
可选 ioctl 调整行为：  
```c
int mode = O_NONBLOCK;
ioctl(fd, IRR_SET_MODE, &mode);                //切换阻塞/非阻塞
int ignore = 2;
ioctl(fd, IRR_SIMPLE_IGNORE_CONFIG, &ignore);  //重复键过滤次数
```

2. 通过 input-event 读取（推荐上层应用）  
   • 用 `evtest`, `libinput debug-events` 或自己写 evdev 程序监听：  
```bash
$ evtest /dev/input/eventX
```
   • 当按下遥控器上的“POWER”键时，内核会发送  
```
Event: time 123.456, type 1 (EV_KEY), code 116 (KEY_POWER), value 1
Event: time 123.456, type 0 (EV_SYN), code 0 (SYN_REPORT), value 0
Event: time 123.556, type 1 (EV_KEY), code 116 (KEY_POWER), value 0
Event: time 123.556, type 0 (EV_SYN), code 0 (SYN_REPORT), value 0
```
   • 不需要关心协议/键值转换逻辑，驱动已根据 DTS keymap 做了映射。  

────────────────────────────────────  
总结  
• 驱动同时提供“底层原始数据字符设备接口”与“高层标准 input-event 接口”两条通路；前者适合需要完整脉冲解码信息的实验/调试程序，后者则直接兼容 Linux 输入框架，可被 X11/Wayland、Android InputReader 等系统组件自动识别。  
• 接口实现核心在 `gx_irr_probe()` 中一次性完成注册；数据交互依赖中断 → kfifo → read/tasklet/input_event 这条链路。  
如需进一步演示 DTS keymap 格式或自定义 ioctl 的用法，随时告知！



## 请你为我生成一份用户空间的测试代码
下面给出一份完整的 C 语言用户空间示例——`gx_irr_test.c`。  
它直接访问字符设备 `/dev/gxirr0`，通过 `poll + read` 获取解析后的按键信息，并演示常用 ioctl 配置。  

```c
/*
 * gx_irr_test.c  —  用户态示例程序
 *
 * 功能：
 *   1. 轮询 /dev/gxirr0，读取 struct irr_code_info_t 并打印
 *   2. 演示 IRR_SET_MODE / IRR_SIMPLE_IGNORE_CONFIG 等 ioctl 用法
 *
 * 编译：
 *   # arm-none-linux-gnueabi-gcc -Wall -O2 gx_irr_test.c -o gx_irr_test
 *   （在 x86 主机测试可直接 gcc）
 *
 * 运行：
 *   # ./gx_irr_test            //默认 /dev/gxirr0
 *   # ./gx_irr_test /dev/gxirr0 8   //指定设备并把重复键忽略阈值调为 8 次
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>

/*-------------------------------------------
 * 把驱动头文件里的关键信息拷到用户态
 *------------------------------------------*/
#define IRR_SET_MODE                   0x1000
#define IRR_SIMPLE_IGNORE_CONFIG       0x3000
#define IRR_DISABLE                    0x4000
#define IRR_ENABLE                     0x5000
#define IRR_CLEAR_CODE_INTERVAL_CONFIG 0x6000

typedef enum {
    IRR_PROTO_UNKNOWN,
    IRR_PROTO_NEC,
    IRR_PROTO_RC5,
    IRR_PROTO_BESCON,
    IRR_PROTO_PANASONIC,
    IRR_PROTO_DVB_40BIT,
    IRR_PROTO_PT2264,
    IRR_PROTO_XMP,
    IRR_PROTO_RCMM32,
} irr_protocol_t;

struct irr_code_info_t {
    uint32_t  code;            /* 32-bit 原始键值 */
    uint64_t  code_extended;   /* XMP 等协议扩展码 */
    uint32_t  is_simcode;      /* 1 = 重复键 */
    irr_protocol_t irr_proto;  /* 协议类型 */
};

/* 协议枚举 → 字符串 */
static const char *proto2str(irr_protocol_t p)
{
    switch (p) {
        case IRR_PROTO_NEC        : return "NEC";
        case IRR_PROTO_RC5        : return "RC5";
        case IRR_PROTO_BESCON     : return "BESCON";
        case IRR_PROTO_PANASONIC  : return "PANASONIC";
        case IRR_PROTO_DVB_40BIT  : return "DVB40";
        case IRR_PROTO_PT2264     : return "PT2264";
        case IRR_PROTO_XMP        : return "XMP";
        case IRR_PROTO_RCMM32     : return "RCMM32";
        default                   : return "UNKNOWN";
    }
}

int main(int argc, char *argv[])
{
    const char *dev = "/dev/gxirr0";
    int ignore_cnt  = -1;          /* <0 表示不设置 */

    if (argc >= 2) dev = argv[1];
    if (argc >= 3) ignore_cnt = atoi(argv[2]);

    /* 1. 打开字符设备（非阻塞，方便 poll） */
    int fd = open(dev, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* 2. 切换非阻塞模式（其实 open 时已指定，这里只做演示） */
    int mode = O_NONBLOCK;
    if (ioctl(fd, IRR_SET_MODE, &mode) < 0)
        perror("ioctl: IRR_SET_MODE");

    /* 3. 配置重复键忽略次数（可选） */
    if (ignore_cnt >= 0) {
        if (ioctl(fd, IRR_SIMPLE_IGNORE_CONFIG, &ignore_cnt) < 0)
            perror("ioctl: IRR_SIMPLE_IGNORE_CONFIG");
    }

    /* 4. poll 主循环 */
    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    printf("Listening on %s …  Ctrl-C 退出\n", dev);

    for (;;) {
        int ret = poll(&pfd, 1, 5000);   /* 5 秒超时 */
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }
        if (ret == 0) {                  /* 超时 */
            printf("[timeout] no key\n");
            continue;
        }

        if (pfd.revents & POLLIN) {
            struct irr_code_info_t info;
            ssize_t n = read(fd, &info, sizeof(info));
            if (n < 0) {
                if (errno == EAGAIN) continue;
                perror("read");
                break;
            } else if (n != sizeof(info)) {
                fprintf(stderr, "short read: %zd bytes\n", n);
                continue;
            }

            printf("code=0x%08x  proto=%s  repeat=%u",
                   info.code, proto2str(info.irr_proto),
                   info.is_simcode);

            if (info.irr_proto == IRR_PROTO_XMP)
                printf("  ext=0x%016llx", (unsigned long long)info.code_extended);

            putchar('\n');
        }
    }

    close(fd);
    return 0;
}
```

编译与运行小结  
1. 交叉编译（假设 arm-none 工具链已在 PATH）：  
   ```bash
   $ arm-none-linux-gnueabi-gcc -Wall -O2 gx_irr_test.c -o gx_irr_test
   ```  
2. 拷贝到开发板后运行：  
   ```bash
   # ./gx_irr_test               # 监听 /dev/gxirr0
   # ./gx_irr_test /dev/gxirr0 8 # 把简码忽略次数设为 8
   ```  
3. 终端将持续打印：  
```
code=0x20DF10EF  proto=NEC  repeat=0
code=0x20DF10EF  proto=NEC  repeat=1
…
```

若想监听 input-event 而非字符设备，可使用现成工具 `evtest`，或参考以下极简示例（不含错误处理）：  

```c
#include <linux/input.h>
…
int fd = open("/dev/input/event2", O_RDONLY);
struct input_event ev;
while (read(fd, &ev, sizeof ev) == sizeof ev) {
    if (ev.type == EV_KEY && ev.value == 1)      /* 按下 */
        printf("KEY %d DOWN\n", ev.code);
}
```

这样即可全面验证驱动在用户空间的两种接口。


## 1. 介绍 linux 中的 timer 用法，并说明该 timer 来自哪里？由哪里提供硬件计数？
2. @gx_irr_pt2264.c 中如何使用 timer？作用是什么？
3. 介绍 kfifo 的用法，并说明 @gx_irr_pt2264.c 中 kfifo 是如何使用的？

Read file: gx_irr_pt2264.c

Ran tool
Read file: gx_irr_pt2264.c

Read file: gx_irr_pt2264.c

下面分三节说明 timer 与 kfifo 的概念及在 `gx_irr_pt2264.c` 中的具体使用。  

────────────────────────────────────  
一、Linux 内核中的 timer 基础  
1. 类型  
   • `struct timer_list` —— 基于 jiffies 的“低分辨率”定时器（非 hrtimer）。  
   • 重要字段：`expires`（到期 jiffies）、`function`（回调）、`data`（回调参数）。  
2. 常见 API（4.15 内核后新接口）  
   • `timer_setup(struct timer_list *t, void (*cb)(struct timer_list *), unsigned flags)`  
   • `mod_timer(t, expires)` 修改/启动  
   • `del_timer(t)`       取消  
3. 计数来源  
   • 并非驱动私有硬件，而是由内核“时基”统一驱动：  
     - CONFIG_HZ 产生的周期性 tick 或 high-res hrtimer signal；  
     - 不同架构用各自系统定时器（ARM 通常是 ARM Generic Timer，x86 可以是 LAPIC / HPET 等）。  
   • `jiffies` 是内核全局递增的 tick 计数；定时器硬件的中断最终触发 softirq 执行回调。  

────────────────────────────────────  
二、`gx_irr_pt2264.c` 中 timer 的用法与作用  
1. 初始化与启动  
```1329:1334:gx_irr_pt2264.c
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
    timer_setup(&info->timer, irr_clear_code, 0);
    mod_timer(&info->timer, jiffies + msecs_to_jiffies(g_irr_clear_code_interval));
#else
    …   /* 旧内核 init_timer + add_timer */
#endif
info->timer_added = 1;
```  
2. 回调实现  
```759:772:gx_irr_pt2264.c
static void irr_clear_code(struct timer_list *t)
{
    struct gx_irr_info *info = from_timer(info, t, timer);
    info->key_code          = 0;
    info->counter_simple    = 0;
    g_irr_philip_rc_mm_temp = 0;
}
```  
3. 重新装载  
   在每次收到按键中断后重新设定超时，保证“最后一次按键后 N ms 自动清零”。  
```1145:1152:gx_irr_pt2264.c
/* IRQ 处理尾部 */
mod_timer(&info->timer,
          jiffies + msecs_to_jiffies(g_irr_clear_code_interval));
```  
4. 释放  
```1409:1416:gx_irr_pt2264.c
if (info->timer_added)
    del_timer(&info->timer);
```  
5. 作用  
   • 防抖 + 状态复位：若用户长时间不按键，定时器回调把 `key_code`、重复计数等清零，避免旧值影响下一次解析。  

────────────────────────────────────  
三、kfifo 的用法 & 本驱动中的应用  
1. kfifo 概述  
   • 内核 lock-less 环型缓冲区；支持一次性存/取字节或结构体；头文件 `<linux/kfifo.h>`.  
   • 常用接口：  
     - `kfifo_alloc(struct kfifo *fifo, unsigned int size, gfp_t gfp)`  
     - `kfifo_in(fifo, buf, len)` 写入  
     - `kfifo_out(fifo, buf, len)` 读取并删除  
     - `kfifo_len(fifo)`    查询已有数据字节数  
     - `kfifo_free(fifo)`     释放  
2. 驱动中的分配  
```1340:1346:gx_irr_pt2264.c
err = kfifo_alloc(&info->queue,        IRR_KFIFO_SIZE, GFP_KERNEL);
err = kfifo_alloc(&info->event_queue,  IRR_KFIFO_SIZE, GFP_KERNEL);
#define INFO_QUEUE       (&info->queue)
#define INFO_EVENT_QUEUE (&info->event_queue)
```  
   • `IRR_KFIFO_SIZE = sizeof(struct irr_code_info_t)*2`，一次可缓存两帧键值。  
3. 数据写入（IRQ 上下文）  
```1153:1158:gx_irr_pt2264.c
if (kfifo_size(INFO_QUEUE) - kfifo_len(INFO_QUEUE) >= sizeof(struct irr_code_info_t))
    kfifo_in(INFO_QUEUE, (u8 *)&irr_data, sizeof(struct irr_code_info_t));
if (kfifo_size(INFO_EVENT_QUEUE) - kfifo_len(INFO_EVENT_QUEUE) >= sizeof(struct irr_code_info_t))
    kfifo_in(INFO_EVENT_QUEUE, (u8 *)&irr_data, sizeof(struct irr_code_info_t));
```  
   • 把解析好的结果推入两个 FIFO：  
     - `queue` 供 `read()` 系统调用读取；  
     - `event_queue` 供 tasklet `irr_send_event()` 转发到 input 子系统。  
4. 数据读取  
   • 用户进程 `read()`：  
```1199:1216:gx_irr_pt2264.c
ret = kfifo_out(INFO_QUEUE, (u8 *)&irr_data, sizeof(struct irr_code_info_t));
… copy_to_user()
```  
   • tasklet ：  
```1094:1102:gx_irr_pt2264.c
kfifo_out(INFO_EVENT_QUEUE, (u8 *)&irr_data, sizeof(struct irr_code_info_t));
```  
5. 辅助接口  
   • `kfifo_len()` 配合 `poll_wait()` 判断是否有数据，实现 select/epoll 支持。  

总结  
• 本驱动利用内核统一的 `timer_list` 机制实现“按键空闲超时复位”；底层计数依托 SoC 公用系统定时器，无须自备硬件。  
• 通过两条 `kfifo` 队列在不同上下文（中断→内核线程/用户进程）之间安全传递解码结果，避免自实现锁或繁琐同步，简洁高效。


## 1. 请详细介绍 mutex、completetion 的使用方法，以及功能
2. 请分析代码中的 mutex、completetion 是如何配置的，并完成了什么功能

下文分为两大部分：  
A) Linux 内核中 mutex 与 completion 的概念、典型 API 及使用注意事项；  
B) 结合 `gx_irr_pt2264.c` 源码说明两者的实战配置与实现效果。  

────────────────────────────────────  
A. 基础知识  

1. mutex（互斥锁）  
   • 作用：在进程上下文中实现“睡眠式”互斥，保障同一时刻仅有一个执行流访问临界资源。  
   • 关键 API  
     - 定义：`struct mutex my_lock;`  
     - 初始化：  
       - 静态：`DEFINE_MUTEX(my_lock);`  
       - 动态：`mutex_init(&my_lock);`  
     - 加锁：`mutex_lock(&my_lock);`（可休眠，不能在原子上下文/中断中使用）  
     - 解锁：`mutex_unlock(&my_lock);`  
     - 可中断/超时版本：`mutex_lock_interruptible()`、`mutex_lock_killable()` 等。  
   • 特点  
     - 若锁已被占用，持锁线程会被挂入等待队列并调度出去；自旋锁则忙等。  
     - 内核保证 unlock 后按 FIFO 唤醒等待者，防止饥饿。  

2. completion（一次性事件同步原语）  
   • 作用：线程 A 等待“某个条件/事件”完成，线程 B 触发事件，通知 A 继续；等价于“计数=1 的信号量”。  
   • 核心结构：`struct completion comp;`  
   • 常用 API  
     - 初始化  
       - 静态：`DECLARE_COMPLETION(comp);`  
       - 动态：`init_completion(&comp);`  
     - 等待  
       - `wait_for_completion(&comp);`  (不可中断)  
       - `wait_for_completion_interruptible()` / `_timeout()`  
     - 唤醒  
       - `complete(&comp);`       唤醒 1 个等待者  
       - `complete_all(&comp);`     唤醒所有等待者  
     - 重置  
       - `reinit_completion(&comp);` 再次复用  
   • 特点  
     - 内部仅包含一个 wait_queue_head_t + done 计数；轻量、无数据传递。  
     - 常见于“驱动启动等待固件加载完成”“阻塞 read 等待硬件中断”等场景。  

────────────────────────────────────  
B. 在 `gx_irr_pt2264.c` 中的具体使用  

1. 对象创建  
```1320+:gx_irr_pt2264.c
mutex_init(&info->lock);
init_completion(&info->comp);
```  
   • `info->lock`：保护 kfifo 等共享资源的并发访问。  
   • `info->comp`：实现阻塞 read 的“事件等待”机制。  

2. 加锁/解锁路径  
   ① `irr_read()` —— 用户态 read 入口  
```1191+:gx_irr_pt2264.c
mutex_lock(&info->lock);
/* 检查 FIFO   若无数据 → 休眠等待 completion */
if (kfifo_len(INFO_QUEUE) == 0) {
    if (info->irr_mode & O_NONBLOCK) { ... }
    wait_for_completion(&info->comp);
    goto retry;
}
/* 取数据 copy_to_user */
mutex_unlock(&info->lock);
```  
   • 目的：  
     - 防止 read 与 IRQ/tasklet 并发访问 FIFO；  
     - 若无数据则在释放 CPU 的同时睡眠等待。  

   ② `irr_poll()` —— select/poll 入口  
   • 同样 `mutex_lock()` → 检查 `kfifo_len()` → `poll_wait()` → `mutex_unlock()`，防止并发。  

3. completion 触发点  
```1122+:gx_irr_pt2264.c
static irqreturn_t irr_isr(...)
{
    ...
    if (kfifo_in(...)成功) {
        if ((info->irr_mode & O_NONBLOCK) != O_NONBLOCK)
            complete(&info->comp);   /* 唤醒阻塞的 read 进程 */
    }
    ...
}
```  
   • 当硬件中断解析出一帧数据并成功写入 FIFO 后：  
     - 若驱动工作在阻塞模式 (`O_NONBLOCK` 未设置)，调用 `complete()` 将等待中的进程唤醒。  
     - read() 被唤醒后再次循环读取数据，流程闭合。  

4. 整体时序  
```
用户进程 read() ──> (FIFO empty) wait_for_completion()
                                   ↑
           中断 → 解析 → kfifo_in  ┘ complete()
```
5. 为什么 mutex + completion 而非自旋锁？  
   • 互斥发生在进程上下文（read/poll），可休眠；  
   • 中断 handler 不能睡眠，因此对 FIFO 写入时未持 mutex，但写前先检查剩余空间且数据结构（kfifo）自身对生产者/消费者是 lock-less 安全。只有慢速读侧需要保护与 poll_wait 的交互。  

6. 功能小结  
   • mutex：序列化用户进程对队列的读操作 + 与 poll_wait 配合保证状态一致；  
   • completion：为阻塞 read 提供“有数据可读”事件信号，避免忙等，提高效率。  

这样，驱动在并发安全与等待/唤醒两方面分别运用了最合适的内核同步原语。

## 1. 请详细介绍 tasklet 的使用方法，以及功能
2. 请分析 @gx_irr_pt2264.c  中 tasklet 是如何配置的，功能是什么？

下面分两部分说明：

A) Linux 内核 tasklet 的概念、使用 API、与其它下半部机制的区别  
B) 结合 `gx_irr_pt2264.c` 源码，说明 tasklet 如何被初始化、调度以及它在驱动中承担的具体功能  

────────────────────────────────────  
A. Tasklet 基础知识  

1. 定位  
   • 属于软中断（softirq）框架的一种轻量级“下半部”机制。  
   • 设计目标：让时间较长的工作避开硬中断（IRQ 上半部），但仍保持在中断上下文执行（不可睡眠）。  

2. 主要 API  
   • 定义对象  
     ```c
     struct tasklet_struct my_tl;
     ```  
   • 初始化  
     ```c
     tasklet_init(&my_tl, callback_func, data);
     // callback 原型：void callback_func(unsigned long data);
     ```  
     - Linux 4.7 起推荐 `DECLARE_TASKLET(name, func, data)`（静态）或 `DECLARE_TASKLET_DISABLED`。  
   • 调度  
     - 普通优先级：`tasklet_schedule(&my_tl);`  
     - 高优先级：`tasklet_hi_schedule(&my_tl);`（对应 HI_SOFTIRQ，比普通 tasklet 更早执行）  
   • 取消 / 释放  
     - `tasklet_kill(&my_tl);` 等待当前回调执行完毕并禁止再次调度  
     - `tasklet_disable/enable` 允许临时屏蔽  

3. 执行特点  
   • 仍在中断上下文；不能调用可能睡眠的 API（如 mutex_lock）。  
   • 同一 tasklet 在同一时刻只会在一个 CPU 上运行（内核内部使用 per-tasklet 自旋锁保护）。  
   • 适合“短小、不能阻塞，但又不想阻塞硬中断”的场景——典型如网络包处理、输入事件上报。  

4. 与工作队列(workqueue)对比  
   • tasklet：软中断环境、无进程上下文，不可睡眠，延迟极短；  
   • workqueue：在专用/共享内核线程中运行，可睡眠，调度延迟相对大，适合耗时或需要调用可能阻塞 API 的工作。  

────────────────────────────────────  
B. `gx_irr_pt2264.c` 中 tasklet 的实战分析  

1. 成员声明  
```33:88:gx_irr.h          // gx_irr_info 结构体
    struct tasklet_struct irr_tasklet;
```

2. 初始化（probe 阶段）  
```1359:1366:gx_irr_pt2264.c
tasklet_init(&info->irr_tasklet, irr_send_event,
             (unsigned long)info);
```
   • 回调函数 `irr_send_event()` 定义见下文；  
   • `data` 参数指向 `struct gx_irr_info`，方便回调获取设备上下文。

3. 调度位置（中断服务程序尾部）  
```1140:1148:gx_irr_pt2264.c
/* 硬件 FIFO -> 协议解析成功后 */
tasklet_hi_schedule(&info->irr_tasklet);
```
   • 选择 **高优先级 tasklet**（`tasklet_hi_schedule`），确保输入事件能尽快从软中断层转发到上层，而不会被普通 tasklet 拖延。  

4. 回调内容——发送 input 事件  
```1075:1104:gx_irr_pt2264.c
static void irr_send_event(unsigned long data)
{
    struct gx_irr_info *info = (struct gx_irr_info *)data;
    struct irr_code_info_t irr_data;
    kfifo_out(INFO_EVENT_QUEUE, &irr_data, sizeof(irr_data));

    int key_event = irr_keycode_get(&irr_data, info->keymap_num);
    if (key_event >= 0) {
        input_event(info->input, EV_KEY, key_event, 1); // 按下
        input_event(info->input, EV_KEY, key_event, 0); // 松开
        input_sync(info->input);
    }
    if (gx_irr_print_keymap)
        …
}
```
   • 步骤  
     1. 从 `event_queue` 取出刚写入的 `irr_code_info_t`  
     2. 根据 keymap 将红外码转换为 Linux keycode  
     3. 调用 `input_event()` + `input_sync()` 上报到 input 子系统  
   • 此代码没有睡眠调用，完全符合 tasklet 运行限制。

5. 释放（remove 阶段）  
```1405:1408:gx_irr_pt2264.c
tasklet_kill(&info->irr_tasklet);
```
   • 确保模块卸载时无残留回调执行，避免 use-after-free。

6. 功能总结  
   • 将“比较耗时但不允许阻塞硬中断”的**键值映射与 input 事件上报**从 IRQ 上半部剥离出来。  
   • 这样做的好处：  
     - 中断处理时间更短，仅完成协议解析和 kfifo 写入；  
     - 输入事件上报仍很快（高优先级 tasklet），不会延迟用户体验；  
     - 不必创建额外内核线程，节省资源。  

整体调用链  
```
硬件中断 → irr_isr()
          ├─ 解析脉冲 → kfifo_in(event_queue)
          └─ tasklet_hi_schedule(irr_tasklet)
                  ↓
          irr_send_event() (tasklet context)
                  ↓
          input_event(...) → /dev/input/eventX
```

通过以上配置，驱动实现了**上半部快、中间桥 tasklet、最终同步到 input-core**的高效三级流水。

## 1. 请详细介绍 waitqueue 的使用方法以及功能
2. 请详细分析 @gx_irr_pt2264.c 中 waitqueue 是如何配置的，功能是什么？

Ran tool
Read file: gx_irr_pt2264.c

Read file: gx_irr_pt2264.c

Ran tool
Read file: gx_irr_pt2264.c

下面给出两部分内容：  
A) waitqueue 的概念、常用 API 与典型使用流程  
B) 针对 `gx_irr_pt2264.c` 的源码解析——waitqueue 在其中如何初始化、挂接、唤醒以及它承担的具体职责  

────────────────────────────────────  
A. Linux waitqueue 基础  

1. 功能  
   • 提供一种“睡眠-唤醒”机制：让进程在自愿条件下挂起（进入 TASK\_INTERRUPTIBLE / TASK\_UNINTERRUPTIBLE），并由另一执行流在事件到来时把它唤醒。  
   • 典型场景：设备驱动阻塞 I/O、poll/select、条件等待等。  

2. 数据结构  
```c
wait_queue_head_t my_wq;          /* 等待队列头 */
struct wait_queue_entry { … };    /* 每个睡眠任务对应一个 entry，由内核自动维护 */
```

3. 核心 API  
   • 初始化  
     - 静态：`DECLARE_WAIT_QUEUE_HEAD(name);`  
     - 动态：`init_waitqueue_head(&my_wq);`  
   • 休眠/等待（宏把 add_wait_queue + schedule() 封装起来）  
     - `wait_event(wq, condition);`                     // 不可被信号打断  
     - `wait_event_interruptible(wq, condition);`       // 可被信号中断  
     - `wait_event_timeout()` / `wait_event_interruptible_timeout()`  
   • 手动方式  
     ```c
     DEFINE_WAIT(wait);
     prepare_to_wait(&wq, &wait, TASK_INTERRUPTIBLE);
     if (!condition)
         schedule();        /* 让出 CPU，进入睡眠 */
     finish_wait(&wq, &wait);
     ```  
   • 唤醒  
     - `wake_up(&wq);`               // 唤醒全部 TASK\_UNINTERRUPTIBLE | TASK\_INTERRUPTIBLE  
     - `wake_up_interruptible(&wq);` // 仅唤醒 TASK\_INTERRUPTIBLE  
     - `wake_up_all()`               // 全部等待者  
   • 与 poll/select 的衔接  
     - 内核提供 `poll_wait(file, &wq, poll_table);`  
       poll 栈在第一次调用时把当前任务挂入 `wq`，并把 wq 地址缓存到 poll\_table 里；事件发生后驱动执行 `wake_up()`，select/poll 系统调用就会返回。  

4. 特点  
   • 只能在进程上下文使用（因为可能调用 schedule() 休眠）。  
   • 不传递数据，仅实现同步；取数一般在唤醒后由调用者再次检查条件。  

────────────────────────────────────  
B. `gx_irr_pt2264.c` 中 waitqueue 的实现细节  

驱动作者为 **select/epoll** 支持专门加了一条等待队列 `irr_read_queue`。流程如下。

1. 声明 & 成员位置  
```88:93:gx_irr.h
wait_queue_head_t irr_read_queue;   // 包含在 struct gx_irr_info
```

2. 初始化（probe 中完成）  
```1395:1399:gx_irr_pt2264.c
/* add by huangjb:select init */
init_waitqueue_head(&info->irr_read_queue);
```

3. 将调用者挂入等待队列 —— `poll` 方法  
```1220:1225:gx_irr_pt2264.c
unsigned int irr_poll(struct file *filp, struct poll_table_struct *table)
{
    …
    if (kfifo_len(INFO_QUEUE) == 0)
        poll_wait(filp, &info->irr_read_queue, table); // 把当前进程加入 wq
    else
        mask |= POLLIN;  // 立即可读
    …
}
```
说明：  
• 当 FIFO 为空时，`poll_wait()` 会为当前进程创建 `wait_queue_entry` 并加入 `irr_read_queue`，然后返回；select/poll 系统调用会睡眠。  
• 如果已有数据，直接返回 `POLLIN`，应用立即得到“可读”通知。

4. 事件到来后的唤醒 —— 中断服务程序尾部  
```1164:1169:gx_irr_pt2264.c
// add by huangjb:select wakeup
wake_up(&info->irr_read_queue);
```
• 硬件中断完成解码并把数据写入 kfifo 后调用 `wake_up()`；  
• 所有在 `irr_read_queue` 上睡眠的进程被唤醒，select/poll 返回，应用随后调用 `read()` 取得按键数据。  

5. 作用总结  
   • waitqueue 只服务于 **select/poll/epoll** 路径——提供“有新红外码可读”的异步通知；  
   • 与 `completion comp` 的分工：  
     - block read() 使用 `wait_for_completion()`（一次只能唤醒一个阻塞读者）；  
     - poll/select 使用 waitqueue，可同时挂多个进程，`wake_up()` 一次性广播所有等待者。  
   • 这样驱动同时兼容两种 I/O 风格：阻塞 read 及事件驱动式 poll/epoll。  

总结  
waitqueue 是 Linux 最基础的睡眠-唤醒原语之一。在本驱动里，它被整合进 `irr_poll()`，配合 `wake_up()` 完成 **“驱动内部事件 → 用户空间 epoll/select”** 的通知链路；与 mutex/completion/tasklet 等同步手段各司其职，共同保证红外数据从硬件到用户态的高效、可靠传递。



## 1. 请详细介绍 poll 的使用方法以及功能
2. 请详细分析 @gx_irr_pt2264.c  中 poll 接口是如何实现的，功能是什么？
3. 请分析上层应用是如何调用 poll 接口的？为什么要调用 poll 接口？调用完之后需要做什么？获取键值的流程是怎样的？


下面分三步详细解答 poll 机制：  
A) 用户空间 `poll()` 系统调用及驱动侧 `poll` 回调的工作原理  
B) `gx_irr_pt2264.c` 中 `irr_poll()` 的源码剖析  
C) 典型的用户空间 poll → read 工作流  

────────────────────────────────────  
A. poll 机制概述  

1. 用户态 poll()  
   • 作用：允许进程监视**多个文件描述符（fd）**，直到其中一个或多个“就绪”（可读、可写、出错等），或超时。  
   • 核心 API:  
     ```c
     #include <poll.h>
     int poll(struct pollfd fds[], nfds_t nfds, int timeout);
     struct pollfd {
         int   fd;         /* 要监视的 fd */
         short events;     /* 请求的事件，如 POLLIN, POLLOUT */
         short revents;    /* 返回的事件 */
     };
     ```  
   • 对比 select  
     - poll() 解除了 fd 数量限制（select 受限于 `FD_SETSIZE`）  
     - fd 集合更紧凑，大文件描述符场景下效率更高  

2. 驱动侧 `.poll` 回调  
   • `file_operations` 成员：`unsigned int (*poll) (struct file *, struct poll_table_struct *);`  
   • 流程：  
     1. 把当前进程挂入设备特定的 waitqueue → `poll_wait()`  
     2. 检查设备状态（如 buffer 是否有数据）  
     3. 返回表示“当前就绪状态”的掩码（POLLIN | POLLOUT | POLLERR...）  
   • 内核 VFS 会在 poll 系统调用时遍历所有 fd，调用其 `.poll` 回调；如果所有 fd 都未就绪，则进程睡眠。  

────────────────────────────────────  
B. `gx_irr_pt2264.c` 中 poll 的实现  
```1220:1225:gx_irr_pt2264.c
unsigned int irr_poll(struct file *filp, struct poll_table_struct *table)
{
    struct gx_irr_info *info = filp->private_data;
    unsigned int mask = 0;

    mutex_lock(&info->lock);
    /* 1. 注册等待队列 */
    poll_wait(filp, &info->irr_read_queue, table);

    /* 2. 检查条件 */
    if (kfifo_len(INFO_QUEUE) > 0)
        mask |= POLLIN; // FIFO 有数据 → 可读

    mutex_unlock(&info->lock);
    return mask;
}
```  
1. 做了什么  
   • `mutex_lock()` → 保证 `kfifo_len()` 的原子性。  
   • `poll_wait()`：  
     - 把当前进程关联到驱动的 `irr_read_queue` 等待队列头；  
     - 此时进程**并不会马上睡眠**，VFS 会继续检查其它 fd。  
   • `kfifo_len()`：  
     - 若 FIFO 非空，返回 `POLLIN`，`poll()` 系统调用立即返回，表示“可读”。  
     - 若为空，返回 0；如果所有 fd 都返回 0，用户进程才开始睡眠。  
2. 功能  
   • 为 `/dev/gxirr0` 提供**非阻塞 I/O 事件通知**能力，即 `select/epoll/poll` 支持。  

────────────────────────────────────  
C. 用户空间 poll 调用流程  

1. 为什么要用 poll？  
   • 避免**忙等**：若直接循环调用 `read()`，在没数据时会浪费大量 CPU。  
   • **多路复用**：如果程序需要同时处理红外、网络、终端输入等多个 I/O 源，poll 是最高效的模型。  
   • 替代 `read()` 阻塞：可实现带超时的读取。  

2. poll → read 完整流程  
```
用户进程
  ┌───────────────────────────────┐
  │   1. poll([ {fd:irr,ev:POLLIN},… ]) │   // 发起监听
  └─────────────────┬─────────────┘
                    │
   (FIFO is empty)  │
                    ↓
                   内核
  ┌─────────────────────────────────┐
  │ 2. 调用 gx_irr_pt2264.c::irr_poll │
  │ 3. poll_wait() 把进程挂入等待队列   │
  │ 4. 返回 0，进程睡眠               │
  └─────────┬───────────────────────┘
            │
            │
            │ <───── 6. wake_up(irr_read_queue)
            │
          硬件中断
  ┌─────────────────────────────────┐
  │ 5. irr_isr() → kfifo_in() → ...   │   // 新数据到达
  └─────────────────────────────────┘
                    │
   (进程被唤醒)      │
                    ↓
  ┌─────────────────────────────────┐
  │ 7. poll() 系统调用返回，revents=POLLIN │
  └─────────────────┬───────────────┘
                    │
                    ↓
  ┌───────────────────────────────┐
  │   8. 调用 read() 读取 FIFO 数据    │
  └───────────────────────────────┘
```
3. 代码示例  
   已在“生成用户空间测试代码”中提供，核心循环如下：  
```c
struct pollfd pfd = { .fd = fd, .events = POLLIN };

for (;;) {
    /* 1. 调用 poll，带 5s 超时 */
    int ret = poll(&pfd, 1, 5000);
    if (ret <= 0) continue; // 超时或错误

    /* 2. poll 返回 > 0，说明 fd 就绪 */
    if (pfd.revents & POLLIN) {
        /* 3. 调用 read 读取数据 */
        struct irr_code_info_t info;
        read(fd, &info, sizeof(info));
        /* 4. 处理键值 */
    }
}
```
总结  
驱动中的 `poll` 接口是连接 **内核事件(新数据到达)** 与 **用户空间 I/O 多路复用** 的桥梁。用户通过它实现高效的事件驱动编程，避免了阻塞或忙等，这对于需要同时响应多种输入的复杂应用（如媒体中心、智能家居网关）至关重要。

