# 进程
## 进程表示
- PID：Unix 进程总是会分配一个号码用于在其命名空间中唯一的标识它们。该号码被称作进程 PID 号。
	- 用 fork 或 clone 产生的每个进程都由内核自动地分配了一个新的唯一的 PID 值。
- 全局 PID 和 TGID 直接保存在 `task_struct` 中，分别是 `task_struct` 的 pid 和 tgid 成员：
```
<sched.h>
struct task_struct {
...
	pid_t pid;
	pid_t tgid;
...
}
```

### 管理 PID
- 使用 PID 分配器用于加速新 ID 的分配。查看代码：
	```
	<pid_namespace.h>
	struct pid_namespace {
		struct task_struct *child_reaper;
	...
		int level;
		struct pid_namespace *parent;
	}
	
	<pid.h>
	struct upid {
		int nr;
		struct pid_namespace *ns;
		struct hlist_head pid_chain;
	};
	
	struct pid {
		atomic_t count; // 引用计数器，如果使用了就把这个变量值+1，不用了就-1
		struct hlist_head tasks[PIDTYPE_MAX]; // (结构体数组)每个数组项对应一个 ID 类型，每个类型的都通过链表组合起来
		int level; // 表示当前命名空间等级
		struct upid numbers[1];
	};
	
	enum pid_type {
		PIDTYPE_PID,
		PIDTYPE_PGID,
		PIDTYPE_SID,
		PIDTYPE_MAX
	}
	```
	- 一个进程可能在多个命名空间中可见，而其在各个命名空间中的局部 ID 各不相同。level 表示可以看到该进程的命名空间的数目。
	- 所有共享同一 ID 的 task_struct 实例都按进程存储在一个散列表中:
	```
	<sched.h>
	struct task_struct {
	...
		struct pid_link pids[PIDTYPE_MAX];
	...
	};
	
	<pid.h>
	struct pid_link {
		struct hlist_node node;
		struct pid *pid;
	};
	
	kernel/pid.c
	/* 通过这个函数把 struct task_struct 和 struct pid 双向链接上了，两者都可以互相找到 */
	/* pid:遍历 task */
	/* task_struct:通过 task->pids[type] */
	int fast call attach_pid(struct task_struct *task, enum pid_type type, struct pid *pid)
	{
		struct pid_link *link;
	
		link = &task->pids[type];
		link->pid = pid;
		hlist_add_head_rcu(&link->node, &pid->tasks[type]);
	}
	```
- 使用位图生成唯一的 PID。为跟踪已经分配和仍然可用的 PID，内核使用一个大的位图，其中每个 PID 由一个 bit 标识。PID 的值\
	可通过对应 bit 在位图中的位置计算而来.
	- 申请一个 PID、释放一个 PID
	```c
	static int alloc_pidmap(struct pid_namespace *pid_ns)
	{
		int i, offset, max_scan, pid, last = pid_ns->last_pid;
		struct pidmap *map;
	
		pid = last + 1;
		if (pid >= pid_max)
			pid = RESERVED_PIDS;
		offset = pid & BITS_PER_PAGE_MASK;
		map = &pid_ns->pidmap[pid/BITS_PER_PAGE];
		/*
		 * If last_pid points into the middle of the map->page we
		 * want to scan this bitmap block twice, the second time
		 * we start with offset == 0 (or RESERVED_PIDS).
		 */
		max_scan = DIV_ROUND_UP(pid_max, BITS_PER_PAGE) - !offset;
		for (i = 0; i <= max_scan; ++i) {
			if (unlikely(!map->page)) {
				void *page = kzalloc(PAGE_SIZE, GFP_KERNEL);
				/*
				 * Free the page if someone raced with us
				 * installing it:
				 */
				spin_lock_irq(&pidmap_lock);
				if (!map->page) {
					map->page = page;
					page = NULL;
				}
				spin_unlock_irq(&pidmap_lock);
				kfree(page);
				if (unlikely(!map->page))
					return -ENOMEM;
			}
			if (likely(atomic_read(&map->nr_free))) {
				for ( ; ; ) {
					if (!test_and_set_bit(offset, map->page)) {
						atomic_dec(&map->nr_free);
						set_last_pid(pid_ns, last, pid);
						return pid;
					}
					offset = find_next_offset(map, offset);
					if (offset >= BITS_PER_PAGE)
						break;
					pid = mk_pid(pid_ns, map, offset);
					if (pid >= pid_max)
						break;
				}
			}
			if (map < &pid_ns->pidmap[(pid_max-1)/BITS_PER_PAGE]) {
				++map;
				offset = 0;
			} else {
				map = &pid_ns->pidmap[0];
				offset = RESERVED_PIDS;
				if (unlikely(last == offset))
					break;
			}
			pid = mk_pid(pid_ns, map, offset);
		}
		return -EAGAIN;
	}
	
	static void free_pidmap(struct upid *upid)
	{
		int nr = upid->nr;
		struct pidmap *map = upid->ns->pidmap + nr / BITS_PER_PAGE;
		int offset = nr & BITS_PER_PAGE_MASK;
	
		clear_bit(offset, map->page);
		atomic_inc(&map->nr_free);
	}
	```

### 进程关系
- 如果进程 A 分支形成进程 B，进程 A 称之为父进程而进程 B 则是子进程。
	- 如果进程 B 再次分支建立另一个进程 C，进程 A 和 进程 C 之间有时称为 祖孙 关系。
- 如果进程 A 分支若干次形成几个子进程 B1,B2,...Bn,各个Bi进程之间的关系称之为兄弟进程。
```c
<sched.h>
struct task_struct {
...
	struct list_head children;   /* 子进程链表 */
	struct list_head sibling;    /* 链接到父进程的子进程链表 */
...
}
```

- 以下将讨论 fork 和 exec 系列系统调用的实现。通常这些调用不是由应用程序直接发出的，而是通用一个中间层调用，\
	即负责与内核通信的 C 标准库。

#### 1. 进程复制
- 传统的 UNIX 中用于复制进程的系统调用是 fork。但它并不是 Linux 为此实现的唯一调用，实际上 Linux 实现了 3 个。
	- fork：建立父进程的一个完整副本，然后作为子进程执行。为减少调用相关工作量，采用 写时复制(copy-on-write) 技术。
	- vfork：类似 fork，但并不创建父进程数据的副本。父子进程之间共享数据。(不再使用)
	- clone：产生线程，可以对父子进程之间的共享、复制进行精确控制。
- 写时复制：防止在 fork 执行时将父进程的所有数据复制到子进程。
	- 进程通常只使用了其内存页的一小部分。调用 fork 时，内核通常对父进程的每个内存页，都为子进程创建一个相同的副本。
	- 如果进程复制之后立即 exec 加载新程序，那么之前复制的内存就会浪费。
	- 内核采用仅复制页表的方式，而不复制进程的整个地址空间。
	- 只要一个进程试图向复制的内存页写入，处理器会向内核报告错误访问(缺页异常)。
		- 内核检查该页是否可以用读写模式访问，还是只读模式。如果只读，则向进程报告段错误。
- 执行系统调用
	- fork、vfork、clone 系统调用的入口点分别是 sys_fork、sys_vfork、sys_clone 函数。
	- 上述函数的任务是从处理器寄存器中提取由用户空间提供的信息，调用体系结构无关的 do_fork 函数。
	```c
	kernel/fork.c
	long do_fork(unsigned long clone_flags,    // 标志集合，用于指定控制复制过程的一些属性。
			unsigned long stack_start, // 用户状态下栈的起始地址。
			struct pt_regs *regs,      // 执行寄存器集合的指针，其中以原始形式保存了调用参数。
			unsigned long stack_size,  // 用户状态下栈的大小。通常不必要，设置为0。
			int __user *parent_tidptr, // 指向用户空间中地址的两个指针。分别指向父子进程的 PID。
			int __user *child_tidptr)


	/* Usage:IA-32 */
	asmlinkage int sys_fork(struct pt_regs regs)
	{
		return do_fork(SIGCHLD, regs.esp, &regs, 0, NULL, NULL);
	}
	```
- do_fork 的实现
	- 所有 3 个 fork 机制最终都调用了 kernel/fork.c 中的 do_fork(体系无关函数)
	- do_fork
		--> copy_process
		--> 确定 PID
		--> 初始化 vfork 的完成处理程序和 ptrace 标志
		--> wake_up_new_task
		--> 是否设置了 CLONE_VFORK 标志 --> wait_for_completion

- 复制进程
	- 在 do_fork 中大多数工作是由 copy_process 函数完成的：
	- copy_process
		--> 检查标志
		--> dup_task_struct
		--> 检查资源限制
		--> 初始化 task_struct
		--> sched_fork
		--> 复制/共享进程的每个部分
			--> copy_semundo
			--> copy_files
			--> copy_fs
			--> copy_sighand
			--> copy_signal
			--> copy_mm
			--> copy_namespaces
			--> copy_thread
		--> 设置各个 ID、进程关系 等







# driver_register

- 根据 `drv->name、drv->bus` ，在 `drv->bus` 上找这个 `name` 是否已经被注册

- 如果没被注册，则调用 `bus_add_driver(drv)` 注册

- 然后将这个 drv 添加到 group

- 然后更新这个 drv 的 kobj

  ```c
  int driver_register(struct device_driver *drv)
  {
  	int ret;
  	struct device_driver *other;
  
  	BUG_ON(!drv->bus->p);
  
  	if ((drv->bus->probe && drv->probe) ||
  	    (drv->bus->remove && drv->remove) ||
  	    (drv->bus->shutdown && drv->shutdown))
  		printk(KERN_WARNING "Driver '%s' needs updating - please use "
  			"bus_type methods\n", drv->name);
  
  	other = driver_find(drv->name, drv->bus);
  	if (other) {
  		printk(KERN_ERR "Error: Driver '%s' is already registered, "
  			"aborting...\n", drv->name);
  		return -EBUSY;
  	}
  
      /*
       * 1. 根据 drv->bus 找到 bus
       * 2. malloc struct driver_private 私有数据
       * 3. 为申请出来的 priv 初始化一个 klist
       * 4. priv->driver = drv;初始化的 driver 指向这个要 add 的 driver
       * 5. drv->p = priv;driver 的p也就是私有数据，指向刚刚申请的
       * 6. priv->kobj.kset = bus->p->drivers_kset;
       * 7. kobject_init_and_add(&priv->kobj, &driver_ktype, NULL, "%s", drv->name);
       */
  	ret = bus_add_driver(drv);
  	if (ret)
  		return ret;
  	ret = driver_add_groups(drv, drv->groups);
  	if (ret) {
  		bus_remove_driver(drv);
  		return ret;
  	}
  	kobject_uevent(&drv->p->kobj, KOBJ_ADD);
  
  	return ret;
  }
  EXPORT_SYMBOL_GPL(driver_register);
  ```

  



# Kobject

```c
static struct kobject *example_kobj;

example_kobj = kobject_create_and_add("kobject_example", kernel_kob);
	/* malloc struct kobject and init */
	kobject_create()
		struct kobject *kobj = kzalloc(sizeof(*kobj), GFP_KERNEL);
		kobject_init(kobj, &dynamic_kobj_ktype);
			kobject_init_internal(kobj);
				kfef_init(&kobj->kref);
					kref = 1;
				INIT_LIST_HEAD(&kobj->entry);
				kobj->state_in_sysfs = 0;
				kobj->state_add_uevent_sent = 0;
				kobj->state_remove_uevent_sent = 0;
				kobj->state_initialized = 1;
	/* 配 kobj->name，根据 kobj 找到 kset，然后链表链到 kset 上，如果没有 kset,就跳过 */
	kobject_add(kobj, parent, "%s", name);
		va_start(args, fmt);
		kobject_add_varg(kobj, parent, fmt, args);
			kobject_set_name_vargs(kobj, fmt, vargs)
				kobj->name = s;
			kobj->parent = parent;
			kobject_add_internal(kobj);
				parent = kobject_get(kobj->parent);
				/* 根据 kobj，找到它的 kset，然后将kobj 添加到 kset 的链表 */
				kobj_kset_join(kobj):
					kset_get(kobj->kset);
					spin_lock();
					list_add_tail(&kobj->entry, &kobj->kset->list);
					spin_unlock():
			create_dir();
			kobj->stage_in_sysfs = 1;
				sysfs_create_dir_ns(kobj, kobject_namespace(kobj));
		va_end(arg);

retval = sysfs_create_group(example_kobj, &attr_group);


example_kset = kset_create_and_add("kset_example", NULL, kernel_kobj);
	kset_create()
		kset = kazlloc(sizeof(*kset), GFP_KERNEL)
		kobject_set_name(&kset->kobj, "%s", name)
			kobj->name = s;
		kset->uevent_ops = NULL;
		kset->kobj.parent = parent_kobj; // kernel_kobj
		kset->kobj.ktype = &kset_type;
		kset->kobj.kset = NULL;
	kset_register()
		kset_init(k);
			kobject_init_internal(&k->kobj);
			INIT_LIST_HEAD(&k->list);
			spin_lock_init(&k->list_lock);
		/* 将 kobject 加到 kset */
		kobject_add_internal(&k->kobj);
			kobj_kset_join(kobj):
			kobj->parent = parent;
			create_dir(kobj):
		kobject_uevent(&k->kobj, KOBJ_ADD);
			add_uevent_var()
			kobj->state_add_uevent_sent = 1;
foo_obj = create_foo_obj("foo");
	kzalloc()
	foo->kobj.kset = example_kset;
	kobject_init_and_add()
	kobject_uevent()
bar_obj = create_foo_obj("bar");
baz_obj = create_foo_obj("baz");
```




# Cmdline

- cmdline 有两种方式提供：一种是由 bootloader 提供，一种是由设备树的 chosen 节点提供。优先使用 bootloader 提供的 cmdline。
	- cmdline 由 bootloader 提供，存储在某片内存中，而这个内存的地址会用 R2 寄存器保存。
		- 纠错：是设备树的地址保存在 R2 寄存器
	- 设备树中获取 cmdline：
		- 找到设备数的 chosen 节点，找到 bootargs 内容，将其拷贝为 cmdline，cmdline 的最大长度是 2k(可配置)
```c
	void __init setup_arch(char **cmdline_p)
{
	const struct machine_desc *mdesc;

	setup_processor();
	mdesc = setup_machine_fdt(__atags_pointer);

	...

	/* populate cmd_line too for later use, preserving boot_command_line */
	strlcpy(cmd_line, boot_command_line, COMMAND_LINE_SIZE);
	*cmdline_p = cmd_line;
}

const struct machine_desc * __init setup_machine_fdt(unsigned int dt_phys)
{
	const struct machine_desc *mdesc, *mdesc_best = NULL;

	if (!dt_phys || !early_init_dt_verify(phys_to_virt(dt_phys)))
		return NULL;

	mdesc = of_flat_dt_match_machine(mdesc_best, arch_get_next_mach);

	early_init_dt_scan_nodes();

	return mdesc;
}

int __init early_init_dt_scan_chosen(unsigned long node, const char *uname,
				     int depth, void *data)
{
	int l;
	const char *p;

	pr_debug("search \"chosen\", depth: %d, uname: %s\n", depth, uname);

	if (depth != 1 || !data ||
	    (strcmp(uname, "chosen") != 0 && strcmp(uname, "chosen@0") != 0))
		return 0;

	early_init_dt_check_for_initrd(node);

	/* Retrieve command line */
	p = of_get_flat_dt_prop(node, "bootargs", &l);
	if (p != NULL && l > 0)
		strlcpy(data, p, min((int)l, COMMAND_LINE_SIZE));

	/*
	 * CONFIG_CMDLINE is meant to be a default in case nothing else
	 * managed to set the command line, unless CONFIG_CMDLINE_FORCE
	 * is set in which case we override whatever was found earlier.
	 */
#ifdef CONFIG_CMDLINE
#if defined(CONFIG_CMDLINE_EXTEND)
	strlcat(data, " ", COMMAND_LINE_SIZE);
	strlcat(data, CONFIG_CMDLINE, COMMAND_LINE_SIZE);
#elif defined(CONFIG_CMDLINE_FORCE)
	strlcpy(data, CONFIG_CMDLINE, COMMAND_LINE_SIZE);
#else
	/* No arguments from boot loader, use kernel's  cmdl*/
	if (!((char *)data)[0])
		strlcpy(data, CONFIG_CMDLINE, COMMAND_LINE_SIZE);
#endif
#endif /* CONFIG_CMDLINE */

	pr_debug("Command line is: %s\n", (char*)data);

	/* break now */
	return 1;
}
	```

