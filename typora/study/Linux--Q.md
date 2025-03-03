# Linux--Q

1. 什么是启动管理器？Linux是用的哪一个？

2. Linux中，一般当前目录---即./ 并不在PATH变量中，所以要运行当前目录下的二进制文件或脚本文件，也需要加上路径。例如：运行当前目录下的`setup.sh`，需要输入`./setup.sh`。

3. 批处理文件：可以把一些指令写成.sh文件然后给予可执行权限。执行.sh文件就相当于执行了其中的命令。

4. #!/bin/bash-->用于表明要使用的shell。

5. `gcc -o`:指定生成文件的名字

6. 编译工具链和目标程序运行相同的架构平台，就叫本地编译。如果不同就是交叉编译。

7. `ARM-GCC`是针对arm平台的一款编译器，它是编译工具链的一个分支

8. `arm-gcc`分类：第二项:`linxu none`(平台) 第三项:`gnu : glibc`  `eabi`:应用二进制标准接口 hf:支持硬浮点平台

9. 在不同实验平台上使用`apt install gcc`的时候安装的就是不同的工具链，如果在`x86`架构下就是装的`x86`,在arm架构下就是`arm-linux`

10. 动态库的制作：`gcc -shared -o libxxx.so xxx.o`

11. 编译时要指定动态库的路径-L (例如./) -l:要链接哪个库

12. 运行时也要指定动态库的路径，修改`export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./`

13. 编译时要指定头文件的路径:-I ,运行时不需要指定,因为在预处理的时候已经包进来了

14. `echo 'main(){}'| gcc -E -v -  `// 它会列出头文件目录、库目录(LIBRARY_PATH)

15. `objdump`命令是用查看目标文件或者可执行的目标文件的构成的`gcc`工具

16. `objcopy`工具使用`BFD库`读写目标文件，它可以将一个目标文件的内容拷贝到另外一个目标文件当中。`objcopy`通过它的选项来控制其不同的动作，它可以将目标文件拷贝成和原来的文件不一样的格式

17. 每一个链接过程都由链接脚本(linker script, 一般以lds作为文件的后缀名) 控制. 链接脚本主要用于规定如何把输入文件内的section放入输出文件内, 并控制输出文件内各部分在程序地址空间内的布局

18. 链接器把一个或多个输入文件合成一个输出文件.输入文件: 目标文件或链接脚本文件. 输出文件: 目标文件或可执行文件.

19. Makefile
    
    ```makefile
    1.赋值
        =   赋值
        := 即时赋值，即后面再赋值就没用
        ?= 如果前面未赋值，就赋值，已赋值就不赋值
        += 变量追加
        “%”表示长度任意的非空字符串
        .PHONY <伪目标名>
    
    2.clean
        clean:像是c语言的一个label
    
    3.变量
        $(objects)：引用变量
    
    4.注释
        #在makefile中是行注释
        如果你要在你的 Makefile 中使用“#”字符，可以用反斜框进行转义，如：“\#”。
    
    5.包含子makefile
        在 Makefile 使用 include 关键字可以把别的 Makefile 包含进来`include <filename>`
        如果命令太长，你可以使用反斜框（‘\’）作为换行符.
    
    6.make工作步骤
        GNU 的 make 工作时的执行步骤入下：（想来其它的 make 也是类似）  
            1、读入所有的 Makefile。 
            2、读入被 include 的其它 Makefile。 
            3、初始化文件中的变量。 
            4、推导隐晦规则，并分析所有规则。 
            5、为所有的目标文件创建依赖关系链。 
            6、根据依赖关系，决定哪些目标要重新生成。 
            7、执行生成命令
    
    7.查找目录
        VPATH:如果定义了这个变量，那么，make
        就会在当当前目录找不到的情况下，到所指定的目录中去找寻文件了。 
        VPATH = src:../headers 
    
        上面的的定义指定两个目录，“src”和“../headers”，make 会按照这个顺序进行搜
        索。目录由“冒号”分隔。
    
    8.自动寻找头文件
        “-M”的选项，即自动找寻源文件中包含的头文件，并生成一个依赖关系。
    
    9.忽略出错
        忽略命令的出错，我们可以在 Makefile 的命令行前加一个减号“-”
    
    10.嵌套执行make
        在一些大的工程中，我们会把我们不同模块或是不同功能的源文件放在不同的目录中，
    我们可以在每个目录中都书写一个该目录的 Makefile，这有利于让我们的 Makefile 变得更
    加地简洁，而不至于把所有的东西全部写在一个 Makefile 中，这样会很难维护我们的
    Makefile，这个技术对于我们模块编译和分段编译有着非常大的好处。
        subsystem: 
        cd subdir && $(MAKE) ---先进入子目录，再执行make
    
    11.传递变量到子make
        如果你要传递变量到下级 Makefile 中，那么你可以使用这样的声明： 
            export <variable ...> 
        如果你不想让某些变量传递到下级 Makefile 中，那么你可以这样声明： 
            unexport <variable ...>
        ps：1.如果你要传递所有的变量，那么，只要一个 export 就行了。后面什么也不用跟，表示
    传递所有的变量。
            2.有两个变量，一个是 SHELL，一个是 MAKEFLAGS，这两个变量不管你是否 export，其总是要传递到下层 Makefile 中。
    
    12.变量命名
        变量的命名字可以包含字符、数字，下划线（可以是数字开头），但不应该含有“:”
        “#”、“=”或是空字符（空格、回车等）。变量是大小写敏感的
    
    13.变量
        变量在声明时需要给予初值，而在使用时，需要给在变量名前加上“$”符号，但最好
    用小括号“()”或是大括号“{}”把变量给包括起来
        eg:objects = program.o foo.o utils.o 
           program : $(objects)
    
    14.环境变量
        make 运行时的系统环境变量可以在 make 开始运行时被载入到 Makefile 文件中。
    
    15.条件判断
        使用条件判断，可以让 make 根据运行时的不同情况选择不同的执行分支。条件表达式
    可以是比较变量的值，或是比较变量和常量的值。
        eg:ifeq ($(CC),gcc)
    
    16.函数调用    
        eg:$(<function> <arguments>) / ${<function> <arguments>}
    
    17.链接脚本
        用于描述文件应该如何被链接在一起形成最终的可执行文件。其主要目的是描述输入文件中的段如何被映射到输出文件中，并且控制输出文件中的内存排布。
    
       SECTIONS{ 
        . = 0X10000000; 
        .text : {*(.text)}
        . = 0X30000000; 
        .data ALIGN(4) : { *(.data) } 
        .bss ALIGN(4) : { *(.bss) } 
        } 
    
        第 1 行我们先写了一个关键字“SECTIONS”，后面跟了一个大括号，这个大括号和第 7 行的大括号是一对，这是必须的。看起来就跟 C 语言里面的函数一样。
        第 2 行对一个特殊符号“.”进行赋值，“.”在链接脚本里面叫做定位计数器，默认的定位
    计数器为 0。我们要求代码链接到以 0X10000000 为起始地址的地方，因此这一行给“.”赋值0X10000000，表示以 0X10000000 开始，后面的文件或者段都会以 0X10000000 为起始地址开始链接。
        第 3 行的“.text”是段名，后面的冒号是语法要求，冒号后面的大括号里面可以填上要链接到“.text”这个段里面的所有文件，“*(.text)”中的“*”是通配符，表示所有输入文件的.text段都放到“.text”中。
        第 4 行，我们的要求是数据放到 0X30000000 开始的地方，所以我们需要重新设置定位计数器“.”，将其改为 0X30000000。如果不重新设置的话会怎么样？假设“.text”段大小为 0X10000，那么接下来的.data 段开始地址就是 0X10000000+0X10000=0X10010000，这明显不符合我们的要求。所以我们必须调整定位计数器为 0X30000000。 
        第 5 行跟第 3 行一样，定义了一个名为“.data”的段，然后所有文件的“.data”段都放到这里面。但是这一行多了一个“ALIGN(4)”，这是什么意思呢？这是用来对“.data”这个段的起始地址做字节对齐的，ALIGN(4)表示 "4 字节对齐" 。也就是说段“.data”的起始地址要能被 4 整除，一般常见的都是 ALIGN(4)或者 ALIGN(8)，也就是 4 字节或者 8 字节对齐。
        第 6 行定义了一个“.bss”段，所有文件中的“.bss”数据都会被放到这个里面，“.bss”数据就是那些定义了但是没有被初始化的变量。
        我们本试验的链接脚本要求如下：
            ①、链接起始地址为 0X87800000。 
            ②、start.o 要被链接到最开始的地方，因为 start.o 里面包含这第一个要执行的命令。
        根据要求，在 Makefile 同目录下新建一个名为“imx6ul.lds”的文件，然后在此文件里面输
    入如下所示代码：
            imx6ul.lds 链接脚本代码
            SECTIONS { 
                 = 0X87800000; 
                 .text : 
                        { 
                            start.o 
                            main.o 
                            *(.text)
                        } 
                .rodata ALIGN(4) : {*(.rodata*)} 
                .data ALIGN(4) : { *(.data) } 
                __bss_start = .; 
                .bss ALIGN(4) : { *(.bss) *(COMMON) } 
                __bss_end = .;
            }
        第 2 行设置定位计数器为0X87800000，因为我们的链接地址就是0X87800000。
        第5行设置链接到开始位置的文件为start.o，因为 start.o 里面包含着第一个要执行的指令，所以一定要链接到最开始的地方。
        第 6 行是 main.o这个文件，其实可以不用写出来，因为 main.o 的位置就无所谓了，可以由编译器自行决定链接位置。
        第 11、13 行有“__bss_start”和“__bss_end”这两个东西？这个是什么呢？“__bss_start”
    和“__bss_end”是符号，第 11、13 这两行其实就是对这两个符号进行赋值，其值为定位符“.”，这两个符号用来保存.bss 段的起始地址和结束地址。前面说了.bss 段是定义了但是没有被初始化的变量，我们需要手动对.bss 段的变量清零的，因此我们需要知道.bss 段的起始和结束地址，这样我们直接对这段内存赋 0 即可完成清零。通过第 11、13 行代码，.bss 段的起始地址和结束地址就保存在了“__bss_start”和“__bss_end”中，我们就可以直接在汇编或者 C 文件里面使用这两个符号。
    
        源文件首先会生成中间目标文件，再由中间目标文件生成执行文件。在编译
    时，编译器只检测程序语法，和函数、变量是否被声明。如果函数未被声明，编译器会给出
    一个警告，但可以生成 Object File。而在链接程序时，链接器会在所有的 Object File 中
    找寻函数的实现
    ```

pastsubst:
    $(patsubst <pattern>,<replacement>,<text>) 
    名称：模式字符串替换函数——patsubst。 
    功能：查找<text>中的单词（单词以“空格”、“Tab”或“回车”“换行”分隔）是否符
    合模式<pattern>，如果匹配的话，则以<replacement>替换。这里，<pattern>可以包括通
    配符“%”，表示任意长度的字串。如果<replacement>中也包含“%”，那么，<replacement>
    中的这个“%”将是<pattern>中的那个“%”所代表的字串。（可以用“\”来转义，以“\%”
    来表示真实含义的“%”字符） 
    返回：函数返回被替换过后的字符串。 
    示例： 
    $(patsubst %.c,%.o,x.c.c bar.c) 
    把字串“x.c.c bar.c”符合模式[%.c]的单词替换成[%.o]，返回结果是“x.c.o bar.o” 

addprefix:
    $(addprefix <prefix>,<names...>) 
    名称：加前缀函数——addprefix。 
    功能：把前缀<prefix>加到<names>中的每个单词后面。 
    返回：返回加过前缀的文件名序列。 
    示例：$(addprefix src/,foo bar)返回值是“src/foo src/bar”。

## Kconfig语法

1. CONFIG宏变量参数
    bool:表示该该宏只能选择y(编译内核)或者n(不编译)，不能选择m(编译为模块)
    tristate:表示该宏可以设置y/m/n三种模式
    string:表示该宏可以设为一串字符，比如#define CONFIG_XXX "config test"
    hex:表示该宏可以设为一个十六进制，比如#define CONFIG_XXX 0x1234
    int:表示该宏可以设为一个整数，比如#define CONFIG_XXX 1234
2. 常用参数
    default y:表示默认是勾上的，当然也可以写为default m或者default n
    help: 帮助信息
    depends on:依赖项，比如depends on XXX表示当前宏需要CONFIG_XXX宏打开的前提下，才能设置它(注意依赖项的config参数只有bool或tristate才有效)
    seltct:反依赖项，和depends on刚好相反，比如selection XXX表示当前宏如果是y或者m，则会自动设置XXX=y或者m(注意参数只有bool或tristate才有效)
    choice:会生成一个单选框，里面通过多选一的方式来选择config(注意choice中的config参数只能bool或tristate)
    prompt:提示信息，如果对于choice而言，则会用来当作一个单选框入口点的标签
    range:设置用户输入的数据范围，比如range 0 100 表示数据只能位于0～100
    menuconfig:menuconfig XXX和config XXX类似，唯一不同的时该选项除了能设置y/m/n外，还可以实现菜单效果(能回车进入该项内部)

## LD

1. 概述：
   
   1. 链接脚本的一个主要目的时描述输入文件中的节如何被映射到输出文件中，并控制输出文件的内存分布。
   2. 链接器总是使用链接器脚本的，如果你不自己提供，链接器回使用一个缺省的脚本，这个脚本是被编译进练级器可执行文件的。可以使用`--verbose`命令行选项行选项来显示缺省的链接器脚本的内容。
   3. 可以通过使用`-T`命令行选项爱嗯来提供自己的链接脚本。也可以通过把链接脚本作为一个链接器的输入文件来隐式地使用它。
   4. 链接器把多个输入文件合并成单个输出文件。(多目标文件-->可执行文件  输入节-->输出节)
   5. 一个目标文件中的每一个节都有一个名字和一个大小尺寸。某一个节可能被表示为`loadable`，含义是在输出文件被执行时，这个节应当载入到内存中去。一个没有内容的节可能是`allocatable`，含义是内存中必须为这个节开辟一块空间，但是没有实际的内容载入到这里。
   6. 每一个loadabke或locatable的输出节有两个地址。第一个是`VMA`或称为虚拟内存地址。这是当输出文件运行时节所拥有的地址。第二个是`LMA`或称为载入内存地址。这是这个节即将要载入的内存地址。
   7. 可以通过使用`-h`选项爱嗯的`objdump`来查看目标文件中的节。

2. 链接脚本的格式：
   
   1. 文件名或格式名之类的字符串一般可以被直接键入。
   2. 可以用分号分隔命令，空格一般被忽略。
   3. 如果文件名包含特殊字符，比如用作分隔符的都好，你可以把文件名放到双引号中。
   4. 用/* */表示注释

3. 示例：
    SECTIONS 
     { 
     . = 0x10000;                 /* 对定位计数器赋值，就把0x10000设为定位计数器的现有值 */
     .text : { *(.text) }         /* 把所有输入文件中的.text输入节都放在.text中 */
     . = 0x8000000;             /* 对定位计数器重新赋值 */
     .data : { *(.data) }         /* 把所有输入文件中的.data输入节和.bss输入节都放到对应的地方 */
     .bss : { *(.bss) } 
     }

4. 语法：
   
   1. `ENTRY`：在运行一个程序时第一个被执行到的指令称为"入口点"。参数是一个符号名：
       ENTRY（SYMBOL）
      ld有多种方法设置进程入口地址, 按一下顺序: (编号越前, 优先级越高)
      
      1. ld命令行的-e选项
      2. 连接脚本的ENTRY(SYMBOL)命令
      3. 如果定义了start符号, 使用start符号值
      4. 如果存在.text section, 使用.text section的第一字节的位置值
      5. 使用值0
   
   2. `INCLUDE FILENAME`:在当前包含链接脚本文件FILENAME。在当前路径下或用`-L`选项指定的所有路径下搜索这个文件，可以嵌套达10层。
   
   3. `MEMORY`：描述目标平台上内存块的位置与长度。
      
      1. 链接器在缺省状态被配置为允许分配所有可用的内存块。可以使用`MEMORY`命令重新配置这个设置。
      2. 用来描述哪些内存区域可以被链接器使用，哪些内存区域是要避免使用的。然后可以把节分配到特定的内存区域中。链接器回基于内存区域设置节的地址，对于太满的区域，会提示警告信息。 
         3.一个链接脚本最多可以包含一次`MEMORY`命令。但是可以在命令中随心所欲定义仍以多的内存块。
      
      MEMORY
      {
       NAME[(ATTR)]:ORIGIN = ORIGIN, LENGTH = LEN
       ...
      }
       NAME:用在链接脚本中应用内存区域的名字，退出了链接脚本，区域名就没有任何实际意义。
       ATTR：字符串是一个可选的属性列表，它指出是否为一个没有在链接脚本中进行显示映射的输入段使用一个特定的内存区域。
      
           ATTR字符串必须包含下面字符中的一个，且必须只包含一个：
               `R' 只读节。
               `W' 可读写节。
               `X' 可执行节。
               `A' 可分配节。
               `I' 已初始化节。
               `L' 同‘I’ 
               `!' 对前一个属性值取反
