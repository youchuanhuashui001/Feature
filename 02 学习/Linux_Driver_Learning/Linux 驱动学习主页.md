---
tags:
  - Linux_Driver
aliases:
  - Linux 驱动学习
---
# Linux 驱动学习蓝图

这是一个围绕 Linux 内核驱动开发构建的知识网络。请从这里开始你的学习之旅。

---

## 内核基础 (Kernel Fundamentals)


-   **核心机制 (Core Mechanisms)**
    -   [[Mutex(互斥锁)|Mutex (互斥锁)]]
    -   [[Spinlock(自旋锁)|Spinlock (自旋锁)]]
    -   [[Completion(完成量)|Completion (完成量)]]
    -   [[Wait_Queue(等待队列)|Wait Queue (等待队列)]]
    -   [[Timer(内核定时器)|Kernel Timer (内核定时器)]]
    -   [[Tasklet(任务队列)|Tasklet (任务队列)]]
-   **数据结构 (Data Structures)**
    -   [[01_内核基础/数据结构/Kfifo|Kfifo]]
    -   [[01_内核基础/数据结构/List_Head_(内核链表)|List Head (内核链表)]]
-   **设备模型 (Device Model)**
	-   [[01_内核基础/设备模型/字符设备驱动|字符设备驱动]]
	-   [[01_内核基础/设备模型/块设备驱动|块设备驱动]]
-   **核心子系统 (Core Subsystems)**
    -   [[01_内核基础/子系统/Input_子系统|Input 子系统]]
    -   [[01_内核基础/子系统/MTD_子系统|MTD 子系统]]
    -   [[01_内核基础/子系统/DMAEngine_子系统|DMAEngine 子系统]]

---

##  驱动分析
-   [[Linux_i2c_driver]]
-   [[Linux_irr_driver]]
-   [[Linux_watchdog_driver]]
-   [[Linux_uart_driver]]