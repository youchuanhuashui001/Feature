---

mindmap-plugin: basic

---

# dw_uart

## init
- main.c 中会调用初始化接口 uart_init(115200, 0);
- dw_uart.c 中会定义 uart_dev，有几个串口，就定义几个变量，主要包括：port 号、基址、中断号、is_init
- uart_init_port
    - uart_set_baudrate(baudrate, port); 根据波特率计算寄存器要如何配置；并直接将配置更新到了对应的寄存器
    - 根据传入的 port 号就可以找到对应的 uart_dev 信息
        - if (p_uart_dev->is_init >0)return; p_uart_dev->is_init = 1;
    - dw_serial_set_pin_multip 配置引脚复用，但是 virgo、Canopus、Vega 这些 arm 架构的芯片没有配置
    - 如果使能了中断，申请一个 kfifo，并且注册 irq handler

## tx
- puts
- putc
    - 一直读 DSW_LSR 等一个状态，等到之后就把数据往 DSW_THR 寄存器写
- compatible_putc
    - 如果要发\n，则先发一个 \r，再发 \n；因为 windows 中 \r\n 是换行符。unix 中 \n 是换行符。

## rx
- 开中断(差异就是一个在 poll 中读，一个在 irq handler 中读)
    - 从 kfifo 中去一个字节数据
        - kfifo 的数据是在 irq handler 中从 DSW_RBR 寄存器中读出来的
- 不开中断
    - 一直读 DSW_LSR 等一个状态，等到之后就从 DSW_RBR 寄存器中读数据

## try_getc
- 开中断
    - 如果 kfifo 中有数据，就取一个字节出来，没有就返回 -1
- 不开中断
    - 如果 LSR 中的状态显示有数据，就从 RBR 寄存器读一个字节数据出来，没有就返回 -1

## reset
- 就是重新初始化了一把串口

## flow
- 如果使能了自动流控，读出 MCR 寄存器的值，或上 DSW_MCR_AFCE | DSW_MCR_RTS 位；如果没使能，清空这两个 bit
- 将刚刚的值读出来写回去到 DSW_MCR 寄存器
- DSW_FCR 寄存器写入 0xC7

## break
- start_break
    - 读出 LCR 寄存器的值，或上 DSW_LCR_BC，重新写回去
- end_bread
    - 读出 LCR 寄存器的值，清除掉 DSW_LCR_BC 位，重新写回去

## irq
- DSW_LSR 非空，从 DSW_RBR 寄存器读数据出来，如果 rx_fifo 非空，把读到的数据放到 kfifo
- tx 没有 irq

## dma
- 如果是 uart0，不会使用 dma，其它的 uart 才会用 dma
- uart_raw_dma_putc
- uart_dma_getc