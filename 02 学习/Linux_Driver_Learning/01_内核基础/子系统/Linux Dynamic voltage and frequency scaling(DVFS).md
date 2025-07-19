
参考链接：
- https://blog.csdn.net/youthcowboy/article/details/135632530?spm=1001.2101.3001.6650.5&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7Ebaidujs_baidulandingword%7ECtr-5-135632530-blog-136207792.235%5Ev43%5Epc_blog_bottom_relevance_base1&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7Ebaidujs_baidulandingword%7ECtr-5-135632530-blog-136207792.235%5Ev43%5Epc_blog_bottom_relevance_base1&utm_relevant_index=9
- http://www.wowotech.net/pm_subsystem/cpufreq_overview.html


Linux 相关文件：
```c
include/linux/cpufreq.h
drivers/cpufreq/sfi-cpufreq.c
drivers/cpufreq/mediatek-cpufreq.c
drivers/cpufreq/omap-cpufreq.c
drivers/cpufreq/s3c24xx-cpufreq.c

drivers/cpufreq/cpufreq.c
drivers/cpufreq/gx-suspmod.c
drivers/cpufreq/cpufreq_conservative.c
drivers/cpufreq/dummy-cpufreq.c
drivers/cpufreq/cpufreq-dt.c

drivers/cpufreq/cpufreq_performance.c
drivers/cpufreq/cpufreq_ondemand.c
drivers/cpufreq/speedstep-centrino.c
drivers/cpufreq/cpufreq_userspace.c

Documentation/cpu-freq/cpufreq-stats.txt
Documentation/cpu-freq/cpu-drivers.txt

Documentation/power/opp.txt


kernel/sched/cpufreq_schedutil.c

```




- 用户态应用程序可以通过相应的接口调整 CPU 运行频率，例如工具包**cpufrequtils**



Linux 动态变频管理 cpufreq 提供了操作系统级别的变频功能，同时需要用户层来制定和执行策略。cpufreq后台进程cpufreqd 就是用来监测系统的运行状况，并根据不同的状况设定 CPU 的工作频率的。 

要想在用户层使用后台进程cpufreqd，首先需要在配置Linux内核编译选项时选择CPU Frequency scaling功能选项, 其内有一些子选项，包含governor的选择和是否Enable CPUfreq debugging。 cpufreq有五种governor，要使cpufreqd正常运行，至少需要选择performance governor；在测试阶段往往希望看到系统变频信息，那么需要Enable CPUfreq debugging。在Enable CPUfreq debugging的前提下，在u-boot的linux启动参数bootargs加上loglevel=8 cpufreq.debug=<value>，就可以看到cpufreq的相应级别的运行信息。 <value>的值可以是 1、2、4，也可以是它们的或（3、5、6、7），具体意义如下：

1 to activate CPUfreq core debugging,

2 to activate CPUfreq drivers debugging（这是和SEP4020相关的调试）, and  

4 to activate CPUfreq governor debugging

后台进程 cpufreqd 需要用到三个库，分别是cpufreqd-2.2.1、cpufrequtils-002和 sysfsutils-2.1.0，这三个库都是 GNU 免费开源软件，我们对 cpufreqd-2.2.1、cpufrequtils-002 进行了一些修改，由于编译这三个库还是蛮麻烦的，因此我在君正发布的cpufreqd的开发包上对arm平台进行二次编译，这样减少了不少工作（做事要站在巨人的肩膀上，呵呵）首先你可以君正的官方ftp上下载这个包。

附：在开启DVFS的时候做老化测试的时候出现死机的状况，这可能在配置核心电压的时候出现死机的状况，所以在配置核心电压的时候可以稍微高一点！一般电压浮动是上下5%！如果不行还可以稍微高一点！