```c
static struct uart_driver serial8250_reg = {
	.owner			= THIS_MODULE,
	.driver_name		= "serial",
	.dev_name		= "ttyS",
	.major			= TTY_MAJOR,
	.minor			= 64,
	.cons			= SERIAL8250_CONSOLE,
};

static const struct tty_operations uart_ops = {
	.open		= uart_open,
	.close		= uart_close,
	.write		= uart_write,
	.put_char	= uart_put_char,
	.flush_chars	= uart_flush_chars,
	.write_room	= uart_write_room,
	.chars_in_buffer= uart_chars_in_buffer,
	.flush_buffer	= uart_flush_buffer,
	.ioctl		= uart_ioctl,
	.throttle	= uart_throttle,
	.unthrottle	= uart_unthrottle,
	.send_xchar	= uart_send_xchar,
	.set_termios	= uart_set_termios,
	.set_ldisc	= uart_set_ldisc,
	.stop		= uart_stop,
	.start		= uart_start,
	.hangup		= uart_hangup,
	.break_ctl	= uart_break_ctl,
	.wait_until_sent= uart_wait_until_sent,
#ifdef CONFIG_PROC_FS
	.proc_fops	= &uart_proc_fops,
#endif
	.tiocmget	= uart_tiocmget,
	.tiocmset	= uart_tiocmset,
	.get_icount	= uart_get_icount,
#ifdef CONFIG_CONSOLE_POLL
	.poll_init	= uart_poll_init,
	.poll_get_char	= uart_poll_get_char,
	.poll_put_char	= uart_poll_put_char,
#endif
};

static const struct tty_port_operations uart_port_ops = {
	.carrier_raised = uart_carrier_raised,
	.dtr_rts	= uart_dtr_rts,
	.activate	= uart_port_activate,
	.shutdown	= uart_tty_port_shutdown,
};
```
```c
// 向 uart 核心层注册一个驱动程序
// 向核心驱动注册注册一个 uart 驱动。我们通过 tty 层依次注册，并且根据每个端口的状态初始化核心驱动
// 我们有一个 proc 文件：/proc/tty/driver，在稍后命名为普通驱动
// drv->port 应该为 NULL；并且在此调用成功后通过uart_add_one_port注册每个端口的结构体
int uart_register_driver(struct uart_driver *drv)

drv->state = kzalloc(sizeof(struct uart_state) * drv->nr, GFP_KERNEL);

normal = alloc_tty_driver(drv->nr);
drv->tty_driver = normal;
normal->xxx = drv->xxx;
normal->ops = &uart_ops;


// init tty _port
port->xxx = xxx;

// 将 struct tty_driver normal 注册到系统
retval = tty_register_driver(normal);

```
一个 uart_driver 可以包含多个串口，对应的 tty_driver 也是一个, cdev 也只有一个
依次为每个串口(struct tty_port)初始化 buffer，队列, ops
那么 struct uart_port 指什么？

serial8250_init 的时候每个串口都调用了 uart_add_one_port 填充了默认信息，在 dw8250_probe 的时候会先 remove，然后填充特有的信息，再去 uart_add_one_port
