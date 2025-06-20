

# 环境搭建

总共有三颗芯片：
- Fornax：待测芯片
- Leo_mini：用于启动 Fornax，Fornax 的程序是放在 Leo_mini 中的，当 Leo_mini 启动的时候会把 Fornax 的程序发到 Fornax 的内存，然后跑起来
- Apus：用于与 Fornax 交互，Fornax 跑什么测试程序是由 Apus 决定的；Apus 与 Fornax 的串口是通的

# 测试流程
- Fornax 使用标准模式中断方式发送一个 NEC 数据，在发送完成之后的回调中开始接收数据，接收完数据后比较数据是否为发出去的数据

- 目前看下来是发送中断有，但是接收中断没有

# 波形分析

![[Pasted image 20250612103418.png]]

## 切换成 P15

### P1
![[297a68b6a961054ba91c4c1e83bb9cb5.jpg]]

![[2d7bede228d5678cf38e047f1f9ab06d.jpg]]

### P2
![[a594ee988edecb3fe0265b4d99532d1a.jpg]]

### P3
![[4e160d5023bb98a925531931ee6a0125.jpg]]

![[348caf8d86c75b0a1a02b3fc8ad138f3.jpg]]

### P4
![[c6628c1fae2fe68ac8108bd93254474c.jpg]]
![[fbd5b464ceb7c75cc919b17ddbaec2ef.jpg]]


## 切换回 P8
### P1

补充抓：
![[d757329d2d2de788511abbc28b8f2273.jpg]]

驱动强度提升之后的波形：
![[91a0272161a18ee44d540c8eb44bdd6e.jpg]]




![[994884c744ce319875159c5db871677a.jpg]]
![[e94d71fe5ed41747a7833d96ef24da52.jpg]]


### P2
![[3ca1813513378cf0feb1acb6baffd6fe.jpg]]


提高驱动强度之后：
![[ced9f35b8ea5b1cd8394214e6dedd8ce.jpg]]




### P3

补充抓：
![[d1bd4fea6e1381d82bb69971fdfceabf.jpg]]


提高驱动强度之后再抓：
![[21c69096fe4eb46c624b84f90f8da05e.jpg]]



![[ef8b23df610104ebbc4c2789f90c4221.jpg]]

![[99868c5aab86eeb3c1895e08f60f38e3.jpg]]


### P4
补充抓：
![[b3b45bed07c21174a6454cbfc976a745.jpg]]


P8 驱动能力由 6.6ma 增强到 10.9 ma 之后的波形：
![[0990186cd2c28b4f8d7873dd3234aa2a.jpg]]





![[e4829c278db4d64092897a5babe279c5.jpg]]



# 结论：
- IO 驱动能力缺陷，P8 引脚驱动能力偏弱，无法驱动 IR 外围电路
