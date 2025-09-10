# ARM 体系结构与编程



## 1. ARM 概述及其基本编程模型

- CPSR：(Current Program Status Register) 当前程序状态寄存器。
- SPSR：(Saved Program Status Register) 备份的程序状态寄存器，用于在程序异常中断程序时，保存被中断程序的程序状态。

- 使用 `MRS 指令` 和 `MSR 指令` 用于访问 `CPSR 寄存器和 SPSR 寄存器`
- SIMD 功能扩展：适合使用电池供电的高性能的便携式设备。此功能扩展为包括音频/视频处理在内的应用系统提供了优化功能。



### 1. 1 ARM 处理器的运行模式

- ARM 处理器共有 7 种运行模式，如下所示：

  ![选区_258](image/选区_258.png)

### 1.2 ARM 寄存器介绍

- ARM 处理器共有 37 个寄存器，其中包括：

  - 31 个通用寄存器，包括程序计数器 PC 在内，这些寄存器都是 32 位寄存器。

  - 6 个状态寄存器，这些寄存器都是 32 位寄存器，但目前只使用了其中 12 位。

  - ARM 处理器共有 7 种不同的处理器模式，在每一种处理器模式中有一组相应的寄存器组。任何时刻(也就是任意的处理器模式下)，可见的寄存器包括 15 个通用寄存器(R0 ~ R14)、一个或两个状态寄存器以及程序计数器(PC)。

  - 在所有的寄存器中，有些是各模式共用的同一个物理寄存器，有一些寄存器是各模式自己拥有的地里的物理寄存器。一下是各种处理器模式下可见的寄存器情况：

    ![选区_259](image/选区_259.png)



#### 1.2.1 通用寄存器

- 通用寄存器可以分为下面 3 类：
  - 未备份寄存器，包括 R0 ~ R7
  - 备份寄存器，包括 R8 ~ R14
  - 程序计数器 PC，即 R15



- 未备份寄存器：

  - 未备份寄存器包括 R0 ~ R7。对每一个未备份寄存器来说，在所有的处理器模式下指的都是同一个物理寄存器。**在异常中断造成处理器模式切换时，由于不同的处理器模式使用相同的物理寄存器，可能造成寄存器中数据被破坏。**未备份寄存器没有被系统用于特别的用途，任何可采用通用寄存器的应用场合都可以使用未备份寄存器。

- 备份寄存器：

  - 对于备份寄存器 `R8 ~ R12` 来说，每个寄存器对应两个不同的物理寄存器。例如：当使用 `FIQ` 模式下的寄存器时，R8 和 R9 分别记作 `R8_fiq、R9_fiq` ，当使用用户模式下的寄存器时，寄存器 R8 和 R9 分别记作 `R8_usr、R9_usr` 等。在这两种情况下，使用的是不同的物理寄存器。系统没有将这几个寄存器用于任何特殊用途，但是当中断处理非常简单，仅仅使用 `R8 ~ R14` 寄存器时， FIQ 处理程序可以不必执行保存和恢复现场的指令，从而可以使中断处理过程非常迅速。

    > 这些寄存器只是用于 `FIQ` 模式下，优势就是当切换到 FIQ 模式下，用户不需要额外的保存现场和恢复现场，因为所有的寄存器都是用的两套寄存器，速度非常快。

  - 对于备份寄存器 `R13` 来说，很多模式下都是有额外的寄存器的，R13 在 ARM 中常用作 `SP 栈指针`。每一种异常模式拥有自己的物理的 `R13` 。应用程序初始化对应的 `R13` 使其指向该异常模式专用的栈地址。当进入异常模式时，可以将需要使用的寄存器保存在 R13 所指的栈中。当退出异常处理程序时，将保存在 R13 所指的栈中的寄存器值弹出。

    > 由于每一种模式都有自己的栈指针 `SP` ，进入到不同的异常模式下，都有其专用的栈地址能够使用。
    >
    > **前提是需要在汇编中先切换到每个模式，然后配置各个模式下的 `SP` 所指向的地址 **

  - 对于备份寄存器 `R14` 来说， R14 又被称为`链接寄存器(Link Register, LR)`，在 ARM 体系中具有下面两种特殊的作用：

    1. 每一种处理器模式自己的物理 `R14` 中存放着**当前子程序的返回地址。**当通过 `BL / BLX` 指令调用子程序时，`R14` 被设置成该子程序的返回地址。在子程序中，当把 R14 的值赋值给程序计数器 PC 时，子程序即返回。

       ```assembly
       #1. 执行下面任何一条指令：
       	MOV PC, LR		# 将 R14 的值赋值给 PC，也就是回到调用当前子程序的地方
       	BX LR           # 跳转到调用当前子程序的地方
       	
       #2. 在子程序入口使用下面的指令将 PC 保存到栈中：
       	STMFD SP!, {<register>,LR}	# 将指定的寄存器和 LR 的值保存到栈上， SP！ 表示栈指针自动递减。 {<register>, LR} 是一个寄存器列表，用逗号分隔，指定要保存的寄存器
       	LDMFD SP!, {<register>,PC}  # 从栈上恢复指定的寄存器的值，并将 PC 设置为栈上保存的值。 SP！ 表示栈指针自动递减。
       	# 函数调用时，STMFD 指令将当前函数的寄存器值保存到栈上，以便函数返回时能够正常恢复。
       	# 函数返回时，LDMFD 指令从栈上恢复之前保存的寄存器值，并将程序控制权返回到调用函数的位置。
       ```

    2. 当异常中断发生时，该异常模式特定的物理 R14 被设置成该异常模式将要返回的地址，对于有些异常模式，R14 的值可能与将返回的地址有一个常数的偏移量。具体的返回方式与上面子程序返回方式基本相同。

  - 对于备份寄存器 `R15` 来说， R15 又被称为 `程序计数器 R15`，又被记作 `PC`。

    - 由于 ARM 采用了流水线机制，当正确读取了 `PC` 的值后，该值为当前指令地址值 + 8 个字节。也就是说，对于 ARM 指令集来说，PC 指向了当前指令的下两条指令的地址。由于 ARM 指令是字对齐的，PC 值的第 0 位和 第 1 位总为 0。
    - 当使用指令 `STR/STM` 保存 `R15`时，保存的可能是当前指令地址值 +8 字节，也可能保存的是当前地址值 +12 字节。到底是哪种方式，取决于芯片的具体设计方式。同一芯片中只能有一种方式，要么是 +8，要么是 +12。



#### 1.2.2 程序状态寄存器

- `CPSR(当前程序状态寄存器)` 可以再任何处理器模式下被访问。它包含了条件标志位、中断禁止位、当前处理器模式标志以及其他的一些控制和状态位。每一种处理器模式下都有一个专用的物理状态寄存器，称为`SPSR(备份程序状态寄存器)`。当特定的异常中断发生时，这个寄存器用于保存当前程序状态寄存器的内容。在异常中断程序退出时，可以用 `SPSR` 中保存的值来恢复 `CPSR`。

- 由于用户模式和系统模式不是异常中断模式，所以它们没有 `SPSR` 。当在用户模式或系统模式中访问 `SPSR` 时，将会产生不可预知的后果。

- CPSR 和 SPSR 的格式相同，如下：

  ![选区_260](image/选区_260.png)

![选区_262](image/选区_262.png)

![选区_263](image/选区_263.png)

![选区_264](image/选区_264.png)



### 1.3 ARM 体系的异常中断

​	在 ARM 体系中，通常有以下 3 种方式控制程序的执行流程：

- 在正常程序执行过程中，每执行一条 ARM 指令，程序计数寄存器(PC) 的值 +4 个字节；每执行一条 `Thumb` 指令，程序计数寄存器(PC) 的值 +2 个字节。整个过程是按顺序执行的。

- 通过跳转指令，程序可以跳转到特定的地址标号处执行，或者跳转到特定的子程序处执行。其中 `B 指令` 用于执行跳转操作；`BL 指令` 在执行跳转操作的同时，保存子程序的返回地址；`BX 指令` 在执行跳转操作的同时，根据目标地址的最低位可以将程序状态切换到 `Thumb` 状态；`BLX 指令` 执行 3 个操作，跳转到目标地址处执行，保存子程序的返回地址，根据目标地址的最低位可以将程序状态切换到 `Thumb` 状态。

  > **BL命令执行过程： **
  >
  > 1. 将当前指令的下一条指令的地址 (即 PC+4) 保存到 LR 寄存器中。这个地址就是子程序执行完后需要返回的地址。
  > 2. 将要跳转的子程序入口地址赋值给 PC，从而实现跳转到子程序的执行。
  > 3. 子程序执行完成后，将 LR 中保存的地址加载到 PC 中，从而返回到调用 BL 指令的下一条指令的地址，继续执行后续的指令。

- 当异常中断发生时，系统执行完当前指令后，将跳转到相应的异常中断处理程序处还行。在异常中断处理程序执行完成后，程序返回到发生中断的指令的下一条指令处执行。**在进入异常中断处理程序时，要保存被中断的程序的执行现场，在从异常中断处理程序退出时，要恢复被中断的程序的执行现场。**



#### 1.3.1 ARM 中异常中断的种类

- 当各个异常中断同时发生时，可以根据各异常中断的优先级选择响应优先级最高的异常中断。

ARM 体系中的异常中断如下所示：

![选区_265](image/选区_265.png)



#### 1.3.2 ARM 处理器对异常中断的响应过程

​	ARM 处理器对异常中断的响应过程如下所述：

1. 保存处理器当前状态、中断屏蔽位以及各条件标志位。这是通过将当前程序状态寄存器 CPSR 的内容保存到将要执行的异常中断对应的 SPSR 寄存器中实现的。各异常中断有自己的物理 SPSR 寄存器。<font color=red>(将CPSR 的内容保存到对应模式的 SPSR)</font>

2. 设置当前程序状态寄存器 CPSR 中相应的位。包括设置 CPSR 中的位，使处理器进入相应的执行模式；设置 CPSR 中的位，禁止 IRQ 中断，当进入 FIQ 模式时，禁止 FIQ 中断。

3. 将寄存器 `lr_mode` 设置成返回地址。

4. 将 PC 设置成该异常中断的中断向量地址，从而跳转到相应的异常中断处理程序处执行。

   上述的处理器对异常中断的响应过程可以用如下的伪代码描述：

   ``` 
   R14_<exception_mode> = return link
   SPSR_<exception_mode> = CPSR
   CPSR[4:0] = exception mode number
   CPSR[5] = 0     # 执行 ARM 指令
   if <exception_mode> == Reset or FIQ then
   	CPSR[6] = 1	# 禁止新的 FIQ 中断
   CPSR[7] = 1	    # 禁止新的 IRQ 中断
   PC = exception vector address
   ```



#### 1.3.3 从异常中断处理程序中返回

​	从异常中断处理程序中返回包括一下两个基本操作：

1. 恢复被中断的程序的处理器状态，即将`SPSR_mode` 寄存器内容复制到 `CPSR` 中

2. 返回到发生异常中断的指令的下一条指令处执行，即把 `lr_mode` 寄存器的内容复制到程序计数器 PC 中

   实际上，当异常中断发生时，PC 所指的位置对于不同的异常中断是不同的。同样，返回地址对于各种不同的异常中断也是不同的。



#### 1.3.4 异常中断处理过程

- ARM 处理器相应当前的中断，将 `CPSR` 寄存器的内容保存到对应的异常中断的 `SPSR` 寄存器
- 设置 `CPSR` 的值，进入到对应的处理器模式(IRQ、FIQ、Undefine、Data_abort 等)，禁止 IRQ、FIQ
- 配置返回地址到对应模式的 `LR` 寄存器
- 将 PC 跳转到异常中断的中断向量地址，从而执行相应的中断处理程序
- 执行处理程序。。。。
- 将当前模式特有的`SPSR` 寄存器的值恢复到 `CPSR` 中
- 将当前模式特有的 `LR` 寄存器的值加载到 `PC` 中，从而回到中断前处执行







### 1.4 ARM 体系中的存储系统



#### 1.4.1 ARM 体系中的存储空间

​	ARM 体系使用单一的地址空间。该地址空间的大小为 `2^32` 个 8 位字节。这些字节单元的地址都是无符号的 32 位数值，取值范围是 `0 ~ 2^32 - 1`

​	各存储单元的地址作为 32 位的无符号数，可以进行常规的整数运算。这些运算的结果进行 `2^32` 取模。也就是说，运算结果发生上溢出和下溢出时，地址将会发生回环。



#### 1.4.2 ARM 存储器格式

​	ARM 体系中，每个字单元中包含 4 个字节单元或两个半字单元。一个半字单元包含两个字节单元。在字单元中，4 个字节哪一个是高位字节，哪一个是低位字节则有两种不同的格式：`Big-endian 、 Little-endian`。



#### 1.4.3 非对齐的存储访问操作

​	在 ARM 中，通常希望字单元的地址是字对齐的(地址的低两位为0b00，也就是 4 字节对齐)，半字单元的地址是半字对齐的(地址的最低位为0b0，也就是2字节对齐)。如果没遵循以上规则，则是非对齐访问的存储访问操作。

1. 非对齐的指令预取操作
   - 如果是 ARM 指令状态，写入到 PC 中的值是非字对齐的，要么指令执行的结果不可预测，要么地址值中最低两位被忽略
   - 如果是 Thumb 指令状态，写入到 PC 中的值是非半字对齐的，要么指令执行的结果不可预测，要么地址值中最低位被忽略

2. 非对齐的数据访问操作

   对于 `Load/Store` 操作，如果是非对齐的数据访问操作，可能有以下情况：

   - 执行结果不可预测
   - 忽略字单元地址的低两位的值，即访问地址为(Address AND 0xFFFFFFFC)的字单元
   - 忽略半字单元地址的最低位的值，即访问地址为(Address AND 0xFFFFFFFE)的字单元



## 2. ARM 指令分类及其寻址方式



### 2.1 ARM 指令集概要介绍

- ARM 指令集可以分为跳转指令、数据处理指令、程序状态寄存器(PSR)传输指令、`Load/Store` 指令、协处理器指令和异常中断产生指令 6 类。

- ARM 指令的一般编码规则：

  - ARM 指令字长为固定的 32 位。一条典型的 ARM 指令编码格式如下：

    ![选区_266](image/选区_266.png)

- ARM 指令的条件码域：

  - 大多数 ARM 指令都可以有条件地执行，也就是根据 CPSR 中的条件标志位决定是否执行该指令。当条件满足时执行该指令，条件不满足时该指令被当做一条 NOP 指令，这时处理器进行判断中断请求等操作，然后转向下一条指令。

  - 每一条 ARM 指令包含 4 位的条件码，如下所示：

    ![选区_267](image/选区_267.png)





## 3.ARM 指令集介绍



### 3.1 ARM 指令集

​	ARM 指令集可以分为 6 类，即跳转指令、数据处理指令、程序状态寄存器(PSR)传输指令、Load/Store 指令、协处理器指令和异常中断产生指令。



#### 3.1.1 跳转指令

​	在 ARM 中，有两种方式可以实现程序的跳转：一种是跳转指令；一种是直接向PC寄存器中写入目标地址值。

​	通过直接向 PC 寄存器中写入目标地址值，可以实现在 4GB 的地址空间中任意跳转，这种跳转指令又称为长跳转。如果在长跳转指令之前使用 `MOV LR, PC` 等指令，可以保存将来返回的地址值，就实现了在 4GB 的地址空间中的子程序调用。

​	ARM 的跳转指令可以从当前指令向前或向后 32MB 的地址空间跳转。这类跳转指令有以下 4 种：

- B：跳转指令，仅跳转
- BL：带返回的跳转指令，跳转的同时还将当前 PC 寄存器的值保存到 LR 寄存器中
- BLX：带返回和状态切换的跳转指令
- BX：带状态切换的跳转指令





#### 3.1.2 数据处理指令

​	数据处理指令又大致分为 3 类：数据传送指令，如 MOV；算术逻辑运算指令，如 ADD、SUB 和 AND 等；比较指令，如 TST。

​	数据传送指令用于向寄存器中传入一个常数。该指令包括一个目标寄存器和一个源操作数。

​	算术逻辑运算指令通常包括一个目标寄存器和两个源操作数。其中一个源操作数为寄存器的值。算术逻辑运算指令将运算结果存入目标寄存器，同时更新 CPSR 中相应的条件标志位。

​	比较指令不保存运算结果，只更新 CPSR 中相应的条件标志位。

​				![选区_270](image/选区_270.png)



#### 3.1.3 乘法指令

​	ARM 有两类乘法指令：一类为 32 位的乘法指令，即乘法操作的结果为 32位；另一类为 64 位的乘法指令，即乘法操作的结果为 64 位。两类指令共有以下 6 条：

- MUL：32 位乘法指令
  - 实现两个 32 位数的乘积，并将结果存放到一个 32 位的寄存器中，同时可以根据运算结果设置 CPSR 寄存器中相应的条件标志位。<font color=red>(带 S 则更新 CPSR)</font>
  - 例如：`MUL R0,R1,R2 ; R0=R1*R2   MULS R0,R1,R2 ; R0=R1*R2，同时设置 CPSR 中 N位和 Z位`
- MLA：32 位带加数的乘法指令
  - 实现两个 32位数的乘积，再将乘积加上第 3 个操作数，并将结果存放到一个 32 位的寄存器中，同时可以根据运算结果设置 CPSR 寄存器中相应的条件标志位。<font color=red>(带 S 则更新 CPSR)</font>
  - 例如：`MLA R0,R1,R2,R3  ; R0=R1*R2+R3`
- SMULL：64 位有符号数乘法指令
  - 实现两个 32位有符号数的乘积，乘积结果的高32位存放到一个 32位的寄存器的`<RdHi>` 中，乘积结果的低32位存放到另一个 32位的寄存器`<RdLo>` 中，同时，可以根据运算结果设置 CPSR 寄存器中相应的条件标志位。<font color=red>(带 S 则更新 CPSR)</font>
  - 例如：`SMULL R1,R2,R3,R4  ; R1=(R3*R4)的低32位、R2=(R3*R4)的高32位`
- SMLAL：64 位带加数的有符号数乘法指令
  - 实现将两个 32位的有符号的 64位乘积结果与 `<RdHi>` 和 `<RdLo>`中的64位数相加，加法结果的高 32位存放到一个 32位的寄存器中，乘法结果的低 32位存放到另一个 32位的寄存器`<RdLo>` 中，同时，可以根据运算结果设置 CPSR 寄存器中相应的条件标志位。<font color=red>(带 S 则更新 CPSR)</font>
- UMULL：64 位无符号乘法指令
- UMLAL：64 位带加数的无符号数乘法指令



#### 3.1.4 状态寄存器访问指令

​	ARM 中有两条指令用于在状态寄存器和通用寄存器之间传送数据。

- 状态寄存器中，有些位是目前没有使用的，但在将来的版本中可能会使用，因此用户程序不要修改这些位。
- 程序不能通过直接修改 CPSR 中的 T 控制为直接将程序状态切换到 Thumb 状态，必须通过 BX 等指令完成程序状态的切换。
- 通过修改状态寄存器是通过 `读取、修改、写回` 的操作序列来实现的。
- 状态寄存器访问指令包括：`MRS：状态寄存器到通用寄存器的传送指令、MSR：通用寄存器到状态寄存器的传送指令`



- MRS
  - MRS 指令用于将状态寄存器的内容传送到通用寄存器中
  - `MRS {<cond>} <Rd>, CPSR`   `MRS{<cond>} <Rd>, SPSR`
- MSR
  - MSR 指令用于将通用寄存器的内容或一个立即数传送到状态寄存器中
  - `MSR {<cond>} CPSR_<mode>, #立即数` `MSR {<cond>} CPSR_<mode>, <Rm>` 
  - `MSR {<cond>} SPSR_<mode>, #立即数` `MSR {<cond>} SPSR_<mode>, <Rm>` 



#### 3.1.5 Load/Store 内存访问指令

​	Load 指令用于从内存中读取数据放入寄存器中；Store 指令用于将寄存器中的数据保存到内存。

- LDR：字数据读取指令

  - LDR 指令用于从内存中讲一个32位的字读取到指令中的目标寄存器中。

    ![选区_271](image/选区_271.png)

- LDRB：字节数据读取指令

  - LDRB 指令用于从内存中讲一个 8位的字节数据读取到指令中的目标寄存器中。并将寄存器的高 24位清零。

    ![选区_273](image/选区_273.png)

- LDRBT：用户模式的字节数据读取指令

  - LDRBT 指令用于从内存中将一个 8位的字节数据读取到指令中的目标寄存器中。并将寄存器的高 24位清零。当在特权级的处理器模式下使用本指令时，内存系统将该操作当做一般用户模式下的内存访问操作。
  - 异常中断程序是在特权级的处理器模式下执行的，这时，如果需要按照用户模式权限访问内存，可以使用 LDRBT 指令。

- LDRH：半字数据读取指令

  - LDRH 指令用于从内存中讲一个 16位的半字数据读取到指令中的目标寄存器中，并将寄存器的高 16位清零。
  - 如果指令中的内存地址不是半字对齐的，指令会产生不可预知的结果。

- LDRSB：有符号的字节数据读取指令

- LDRSH：有符号的半字节数据读取指令

- LDRT：用户模式的子数据读取指令

- STR：子数据写入指令

  - STR 指令用于将一个 32位的字数据写入到指令中指定的内存单元。
  - `STR R0, [R1, #0x100]  ; 将 R0 中的字数据保存到内存单元 R0+0x100 中`
  - `STR R0, [R1], #8      ; 将 R0 中的字数据保存到内存单元 R1 中，R1=R1+8`

- STRB：字节数据写入指令

  - STRB 指令用于将寄存器中低8位的字节数据写入到指令中指定的内存单元，该字节数据为指令中存放源操作数的寄存器的低 8位。
  - `STRB R3, [R5, #0x200]  ; 将 R3 中的低 8位数据保存到内存单元 R5+0x200 中`
  - `STRB R3, [R5, #0x200]! ; 将 R3 中的低 8位数据保存到内存单元 R5+0x200 中，R5=R5+0x200`

- STRBT：用户模式的字节数据写入指令

- STRH：半字数据写入指令

- STRT：用户模式子数据写入指令





#### 3.1.6 批量 Load/Store 内存访问指令

​	批量 Load 内存访问指令可以一次从连续的内存单元中读取数据，传送到指令中的内存列表中的各个寄存器中。

​	批量 Store 内存访问指令可以将指令中寄存器列表中的各个寄存器值写入到内存中，内存的地址由指令中的寻址模式确定。

- LDM(1)：批量内存字数据读取指令
  - LDM(1) 指令将数据从连续的内存单元中读取到指令中寄存器列表中的各寄存器中。它主要用于块数据的读取、数据栈操作以及从子程序中返回的操作。
- LDM(2)：用户模式的批量内存字数据读取指令
- LDM(3)：带状态寄存器的批量内存字数据读取指令
- STM(1)：批量内存字数据写入指令
  - STM(1) 指令将指令中寄存器列表中的各寄存器数值写入到连续的内存单元中。它主要用于块数据的写入、数据栈操作，以及进入子程序时保存相关的寄存器的操作。
- STM(2)：用户模式的批量内存字数据写入指令





#### 3.1.7 信号量操作指令

​	信号量用于进程间的同步和互斥。对信号量的操作通常要求是一个原子操作，即在一条指令中完成信号量的读取和修改操作。

- SWP：交换指令

  - SWP 指令用于将一个内存字单元的内容读取到一个寄存器中，同时将另一个寄存器的内容写入到该内存单元中。

    ![选区_274](image/选区_274.png)

- SWPB：字节交换指令



#### 3.1.8 异常中断产生指令

- SWI：软中断指令。SWI 用于产生 SWI 异常中断，ARM 正式通过这种机制实现在用户模式中对操作系统中特权模式的程序的调用。
- BKPT：断电中断指令。主要用于产生软件断点，供调试程序使用。



#### 3.1.8 ARM 协处理器指令

​	ARM 支持 16 个协处理器。在程序执行的过程中，每个协处理器忽略属于 ARM 处理器和其它协处理器的指令。当一个协处理器硬件不能执行属于它的协处理器指令时，将产生未定义指令异常中断，在该异常中断处理程序中，可以通过软件模式该硬件操作。比如，如果系统中不包含向量浮点运算器，则可以选择浮点运算软件模拟包，来支持向量的浮点运算。

- CDP：协处理器数据操作指令

- LDC：协处理器数据读取指令

- STC：协处理器数据写入指令

- MCR：ARM 寄存器到协处理器寄存器的数据传送指令。**将数据从 ARM 寄存器加载到协处理器**

  ![选区_275](image/选区_275.png)

- MRC：协处理器寄存器到ARM寄存器的数据传送指令。**将协处理器的数据加载到 ARM 寄存器**

  ![选区_276](image/选区_276.png)





## 4. ARM 汇编

### 4.1 伪操作

​	ARM 汇编语言源程序中，语句由指令、伪操作和宏指令组成。同样 ARM 中宏指令也通过伪操作定义的。

#### 4.1.1 符号定义伪操作

​	符号定义伪操作用于定义 ARM 汇编程序中的变量，对变量进行赋值以及定义寄存器名称。包括一下伪操作：

- GBLA、GBLL 及 GBLS：声明全局变量
  - 用于声明一个 ARM 程序中的全局变量，并且对其进行初始化。
- LCLA、LCLL 及 LCLS：声明局部变量
  - 用于声明一个 ARM 程序中的局部变量，并且对齐进行初始化。
- SETA、SETL 及 SETS：给变量赋值
  - 用于给一个 ARM 程序中的变量赋值
- RLIST：为通用寄存器列表定义名称
- CN：为协处理器的寄存器定义名称
- CP：为协处理器定义名称
- DN 及 SN：为 VFP 的寄存器定义名称
  - DN：用来为一个双精度的 VFP 寄存器定义名称
  - SN：用来为一个单精度的 VFP 寄存器定义名称
- FN：为 FPA 的浮点寄存器定义名称



#### 4.1.2 数据定义伪操作

​	数据定义伪操作包括以下具体的伪操作：

- LTORG：声明一个数据缓冲池的开始
- MAP：定义一个结构化的内存表的首地址
- FIELD：定义结构化的内存表中的一个数据域
- SPACE：分配一块内存单元，并用 0 初始化
- DCB：分配一段字节的内存单元，并用指定的数据初始化
- DCD 及 DCDU：分配一段字的内存单元，并用指定的数据初始化
- DCDO：分配一段字的内存单元，并将各单元的内容初始化成该单元相对于静态基值寄存器的偏移量
- DCFD 及 DCFDU：分配一段双字的内存单元，并用双精度的浮点数据初始化
- DCFS 及 DCFSU：分配一段字的内存单元，并用双精度的浮点数据初始化
- DCI：分配一段字节的内存单元，用指定的数据初始化，指定内存单元中存放的是代码，而不是数据
- DCQ  及 DCQU：分配一段双字的内存单元，并用 64 位的整数数据初始化
- DCW 及 DCWU：分配一段半字的内存单元，并用指定的数据初始化
- DATA：在代码段中使用数据。现已不再使用，仅用于保持向前兼容



#### 4.1.3 汇编控制伪操作

- IF、ELSE 即 ENDIF
- WHILE 及 WEND
- MACRO 及 MEND
  - MACRO 标识宏定义的开始，MEND 标识宏定义的结束。两个联合起来定义的一段代码，称为宏定义体，程序中通过宏指令多次调用该代码段

- MEXIT
  - 用于从宏中跳转出去



#### 4.1.4 数据帧描述伪操作

​	主要用于调试。



#### 4.1.5 信息报告伪操作

- ASSERT
  - 在汇编编译器对汇编程序的第二遍扫描中，如果其中的条件不成立，ASSERT 伪操作将报告该错误信息。
- INFO
  - 在汇编处理过程的第一遍扫描或者第二遍扫描时报告诊断信息。
- OPT
  - 通过 OPT 伪操作，可以在原程序中设置列表选项。



#### 4.1.6 其它伪操作

- CODE16 及 CODE32
  - CODE16：告诉汇编编译器后面的指令序列为 16 位的 Thunb 指令
  - CODE32：告诉汇编编译器后面的指令序列为 32 位的 ARM 指令
- EQU
  - 为数字常量、基于寄存器的值和程序中的标号定义一个字符名称。作用类似于 `#define`，用于为一个常量定义字符名称
- AREA
  - 用于定义一个代码段或数据段

- ENTRY
  - 指定程序的入口点，一个程序中至少要有一个 ENTRY，但一个源文件中最多只能有一个 ENTRY
- END
  - 告诉编译器已经到了源程序结尾
- ALIGN
  - 通过添加补丁字节使当前位置满足一定的对齐方式

- EXPORT 及 GLOBAL
  - EXPORT 声明一个符号可以被其他文件应用，相当于声明了一个全局变量。GLOBAL 声明一个全局变量，但只能本文件使用。
- IMPORT
  - 告诉编译器当前的符号不是在本源文件中定义的，而是在其它源文件中定义的，在本源文件中可能引用该符号，而且不论本源文件是否实际引用该符号，该符号都被加入到本源文件的符号表中。
- EXTERN
  - 告诉编译器当前的符号不是在本源文件中定义的，而是在其它源文件中定义的，在本源文件中可能引用该符号。如果本源文件没有实际引用该符号，该符号将不会被加入到本源文件的符号表中。
- GET 及 INCLUDE
  - GET 将一个源文件包含到当前源文件中，并将被包含的文件在其当前位置进行汇编处理。INCLUDE 相同。

- INCBIN
  - INCBIN 将一个文件包含到当前源文件中，被包含的文件不进行汇编处理。
- KEEP
  - KEEP 告诉编译器将局部符号包含在目标文件的符号表中。



### 4.2 ARM 汇编语言伪指令

​	ARM 中伪指令不是真正的 ARM 指令或者 Thumb 指令，这些伪指令在汇编编译器对源程序进行汇编处理时被替换成对应的 ARM 或 Thumb 指令。ARM 伪指令包括 `ADR、ADRL、LDR 和 NOP`

- ADR(小范围的地址读取伪指令)
  - 将基于 PC 的地址值或基于寄存器的地址读取到寄存器中

- ADRL(中等范围的地址读取伪指令)
- LDR(大范围的地址读取伪指令)
  - 将一个 32位的常数或者一个地址值读取到寄存器中
- NOP(空操作伪指令)
  - NOP 伪指令在汇编时，将被替换成 ARM 中的空操作，比如：`MOV R0, R0`



## 5. 存储器系统



### 5.1 MMU

​	在 ARM 系统中，MMU 主要完成以下工作：

- 虚拟存储空间到物理存储空间的映射。在 ARM 中采用了页式虚拟存储管理。它把虚拟地址空间分成一个个固定大小的块，每一块称为一页，把物理内存的地址空间也分成同样大小的页。页的大小可以分为粗粒度和细粒度两种。MMU 就要实现从虚拟地址到物理地址的转换。
- 存储李访问权限的控制。
- 设置虚拟存储空间的缓冲的特性。

![选区_286](image/选区_286.png)

> 页表：在内存中，表中的每一行对应虚拟存储空间的一个页，这个行包括虚页对应的实页的地址。
>
> 快表：一个很快的小的页表，也在内存中，称为 TLB。
>
> CPU 访问内存时，会先在 TLB 中找地址映射，TLB 中找不到才会去页表中找，找到之后把页表中的加载到 TLB 中，这样 CPU 下次访问内存时，用的就是 TLB 中保存的地址映射。



- 使能 MMU 时存储访问过程
  - 根据TLB和页表得到要访问的虚拟地址对应的物理地址
  - 根据 TBL 和页表中的 `C(Cacheable)控制位和B(Bufferable)控制位` 决定是否缓存该内存访问的结果。
  - 根据存取权限控制为和域访问控制位确定该内存访问是否被允许。如果该内存访问不被允许，CP15 向 ARM 处理器报告存储访问中止。
  - 对于不允许 Cache 的存储访问，直接用 TLB 和页表翻译得到的物理地址；如果允许 Cache 的存储访问，Cache 命中的情况下则直接使用 Cache 中的数据，忽略物理地址，如果 Cache 未命中的情况下则根据 TLB 和页表得到的物理地址访问内存，并且把这块数据读到 Cache 中。

- 禁止 MMU 时存储访问过程
  - 当禁止 MMU 时，如果禁止 Cache 和 Write Buffer，则存储访问不考虑 `C、B`位；如果使能 Cache 和 Write Buffer，则数据访问时，C = 0, B = 0,指令读取时，如果使用分开的 TLB，则 C = 1，如果使用统一的 TLB，则 C = 0。
  - 存储访问不进行权限控制，MMU 也不会产生存储访问中止信号。
  - 所有的物理地址和虚拟地址相等，即使用普通存储模式。



### 5.2 Cache

​	Cache 是全部用硬件来实现的。Cache 与主存之间以块(Cache Line)为单位进行数据交换，当 CPU 读取数据或指令时，它同时将读到的数据或指令保存到一个 Cache 块中。这样，当 CPU 第二次需要读取相同的数据时，它可以从相应的 Cache 块中得到相应的数据。因为 Cache 的速度远远大于主存储器的速度，系统的整体性能就得到很大的提高。实际上，在程序中，通常相邻的一段时间内 CPU 访问相同数据的概率是很大的，这种规律称为事件局部性。

​	不同系统中，Cache 的块大小也是不同的。通常 Cache 的块大小为几个字。这样，当 CPU 从主存中读取一个字的数据时，它将会把主存中与 Cache Line 相同大小的数据读取到 Cache 的 Cache Line 中。比如：Cache Line 大小为 64，CPU 从主存第 n 处读取数据，将同时把 n 附近的数据也读到 Cache Line 中。实际上，在程序中，CPU 访问相邻的存储空间的数据的概率是很大的，这种概率称为空间局部性。

​	写缓冲区是由一些高速的存储器构成的。它主要用来优化向主存中的写入操作。当CPU 进行向主存中的写入操作时，它先将数据写入到`Write Buffer`中，由于 `Write Buffer` 的访问速度很快，这种写入操作的速度将很高。然后 CPU 就可以进行下面的操作。写缓冲区在适当的时候以较低的速度将数据写入到主存中相应的位置。



#### 5.2.1 Cache 工作原理

- CPU 想要访问内存中的数据时，CPU 先将内存地址传到 Cache，通过地址变换部件将内存地址转换成在 Cache 中的地址，如果变换成功，则称为 Cache 命中，使用 Cache 中的地址去访问 Cache，从 Cache 中取出数据送出给 CPU。如果变换失败，则成为 Cache 未命中，通过内存地址去访问内存，将数据取出装入到 Cache 中，同时返回给 CPU。如果 Cache 满了，则采用某种 Cache 替换策略替换出去。

#### 5.2.2 Cache 地址映像和变换方法

- 全相联映像方式
  - 主存中任意一块可以映像到 Cache 中的任意一块的位置上。
- 直接映像方式
  - 主存中的一块只能映像到 Cache 中的一个特定的块中。

- 组相联映像方式
  - 把主存和 Cache 按同样大小划分成组(Set)，每一个组都由相同的块数组成。
  - 由于主存储器的容量比 Cache 的容量大得多，因此，主存的组数要比 Cache 的组数多。从主存的组到 Cache 的组之间采用直接映像方式。在主存中的一组与 Cache 中的一组之间建立了直接映像关系之后，在两个相应的组内部采用全相联映像方式。

### 5.2.3 Cache 分类和特点

- 统一/独立的数据 Cache 和指令 Cache
  - 如果一个存储系统中指令预取时使用的 Cache 和数据读写时使用的 Cache 是同一个 Cache，这时，称系统使用了统一的 Cache。
  - 如果一个UN出系统中指令预取时使用的 Cache 和数据读写时使用的 Cache 是各自独立的，这是，称系统使用了独立的 Cache。分别叫做：ICache、DCache

- 写通(Write-through)和写回(Write-back)
  - 写回：CPU 在执行写操作时，被写的数据只写入 Cache，不写入主存。仅当需要替换时，才把已经修改的 Cache 块写回到主存中。
  - 写通：CPU 在执行写操作时，必须把数据同时写入 Cache 和主存中。这样，Cache 的`dirty` 位也不需要修改。



### 5.2.4 存储系统一致性问题

​	当存储系统中引入了 Cache 和 Write Buffer 时，同一地址单元的数据可能在系统中有多个副本，分别保存在 Cache 中、Write Buffer 中及内存中。如果系统采用了独立的数据 Cache 和指令 Cache，同一地址单元的数据还可能在数据 Cache 和指令 Cache 中有不同的版本。位于不同物理位置的同一地址单元的数据可能会不同，是的数据读操作可能得到的不是系统中最新的数值，这样就带来了存储系统中数据的一致性问题。



## 5.3 快速上下文切换

​	快速上下文切换技术通过修改系统中不同进程的虚拟地址，避免在进行进程间切换时造成的虚拟地址到物理地址的重映射，从而提高系统的性能。

​	通常情况下，如果两个进程占用的虚拟地址空间有重叠，系统在这两个进程之间进行切换时，必须进行虚拟地址到物理地址的重映射。而虚拟地址到物理地址重映射涉及到重建 MMU 中的页表，而且 Cache 和 TLB 中的内容都必须 `individual`。这些操作将带来巨大的系统开销。

​	快速上下文切换技术的引入避免了这种开销。它位于 CPU 和 MMU 之间，如果两个进程使用了同样的虚拟地址空间，则对 CPU 而言，两个进程使用了同样的虚拟地址空间。快速上下文切换机构对各进程的虚拟地址进行变换，这样，系统中除了 CPU 之外的部分看到的是经过快速上下文切换机制变换的虚拟地址。快速上下文切换机制将各进程的虚拟空间变换成不同的虚拟空间。这样，在进行进程间切换时，就不需要进行虚拟地址到物理地址的重映射了。

![选区_288](image/选区_288.png)



## 5.4 与存储系统相关的程序设计指南

### 5.4.1 地址空间

![选区_289](image/选区_289.png)

### 5.4.2 非对齐的存储访问操作

​	字单元的地址是自对齐的(0b00)，半字单元的地址是半字对齐的(0b0)。在存储访问操作中，如果存储单元的地址没有遵循上述对齐规则，则称为非对齐的存储访问操作。

- 非对齐的指令预取操作
  - 当处理器是 ARM 状态时，如果写入到 PC 中的值是非字对齐的，要么执行结果不可预测，要么值的最低两位被忽略。
  - 当处理器是 Thumb 状态时，如果写入到 PC 中的值是非半字对齐的，要么执行结果不可预测，要么值的最低一位被忽略。
- 非对齐的数据访问操作
  - 对于 LOAD/STORE 操作，如果是非对齐的数据访问操作，可能有以下结果：
    - 执行结果不可预知
    - 忽略字单元地址的低两位的值
    - 忽略半字单元你地址的低位的值

### 5.4.3 指令预取和自修改代码

​	ARM 中允许指令预取，在 CPU 执行当前指令的同时，可以从存储器中预取其后的若干条指令，具体预取多少条指令，不同的 ARM 实现中有不同的数值。

​	<font color=red>**当用户读取 PC 寄存器的值时，返回的是当前指令下面第 2 条指令的地址。对于 ARM 指令来说，读取 PC 寄存器的值时，返回当前指令地址值 + 8个字节。Thumb 指令是 +4 个字节。**</font>

​	预取的指令不一定能够得到执行。比如当前指令完成后，如果发生了异常中断，程序会跳转到异常中断处理程序处执行，当前预取的指令将被抛弃。或者如果执行了跳转指令，则当前预取的指令也将被抛弃。



## 6. 中断

![选区_290](image/选区_290.png)



### 6.1 ARM 处理器对异常中断的响应过程

​	ARM 处理器对异常中断的响应过程如下：

- 保存处理器当前状态、中断屏蔽位以及各条件标志位。通过将CPSR 的内容保存到将要执行的异常中断对应的 SPSR寄存器中实现的。各异常中断有自己的物理 SPSR 寄存器。

- 设置CPSR 寄存器中对应的位。包括设置 CPSR 中的位，使处理器进入相应的执行模式；设置 CPSR 中的位，禁止 IRQ 中断，当进入 FIQ 模式时，禁止 FIQ 中断。

- 将寄存器 `lr_mode` 设置成返回地址。

- 将 PC 设置成该异常中断的中断向量地址，从而跳转到相应的异常中断处理程序处执行。

  - `bl undefine_irq_handle`

  ![选区_291](image/选区_291.png)

  

 	从异常中断处理程序中返回包括下面两个基本操作：

- 恢复被中断的程序的处理器状态，即把 SPSR_mode 寄存器内容复制到 CPSR 中。
- 返回到发生异常中断的指令的下一条指令处执行，即把 `lr_mode` 寄存器的内容复制到 PC 中。

















# Gxloader

描述需要用到的基础汇编指令：

- `.global` 申明一个全局 ResetEntry 标号，后面可以在其它的地方用到
- `.extern` 申明 `entry` 是一个外部函数，和 c 语言相同的用法
- `.word`：分配一段字内存空间，并用 expr 初始化字内存单元

  - `.word irq`：分配一个 4 字节空间，里面方得知是 `irq` 函数的地址

- `mrs` ：读 `cpsr/spsr` 寄存器到通用寄存器
- `msr`：将通用寄存器的值写入到 `cpsr/spsr` 寄存器

- `isb`：刷新流水线和预取缓冲区。保证在 isb 之后的指令全部都重新从 cache 中取

- `mcrr`：将 arm 通用寄存器写入到协处理器

- `bl`： 带链接的跳转，跳转过去只有可以通过 `movs pc, lr` 跳转回来继续执行下一条指令

- `UBFX（无符号位字段提取）` ：将一个寄存器中的相邻位写入第二个寄存器的最小有效位，零扩展到 32 位

- Start.S

  ```assembly
  #define __ASSEMBLY__
  #include <config.h>
  
  	.global ResetEntry
  	.extern entry
  #define CR_V	                     (1 << 13)	/* Vectors relocated to 0xffff0000	*/
  
  // 异常向量表
  ResetEntry:
  	b	reset
  	ldr pc, _undefined_instruction
  	ldr pc, _software_interrupt
  	ldr pc, _prefetch_abort
  	ldr pc, _data_abort
  	ldr pc, _not_used
  	ldr pc, _irq
  	ldr pc, _fiq
  
  _undefined_instruction: .word undefined_instruction
  _software_interrupt:    .word software_interrupt
  _prefetch_abort:    .word prefetch_abort
  _data_abort:        .word data_abort
  _not_used:      .word not_used
  _irq:           .word irq
  _fiq:           .word fiq
  
  clear_cntvoff:
  	/* CNTVOFF Only accessible from Hyp mode, or from Monitor mode when SCR.NS is set to 1 */
  	/* enter monitor mode */
  	mrs r2, cpsr
  	bic r2, r2, #0x1f
  	orr r2, r2, #0x16           @ 0x16: monitor mode
  	msr cpsr, r2
  
  	/* set SCR.NS bit, copy from uboot */
  	mrc p15, 0, r5, c1, c1, 0   @ read SCR Secure Configure Register
  	bic r5, r5, #0x4a           @ clear IRQ, EA, nET bits
  	orr r5, r5, #0x31           @ enable NS, AW, FW bits
  	                            @ FIQ preserved for secure mode
  	mcr p15, 0, r5, c1, c1, 0   @ write SCR (with NS bit set)
  	isb
  
  	/* clear CNTVOFF */
  	movne   r4, #0
  	mcrr p15, 4, r4, r4, c14    @ Reset CNTVOFF to zero
  	isb
  
  	/* clear SCR.NS for secure state */
  	bic r5, r5, #0x01
  	mcr p15, 0, r5, c1, c1, 0  @ write SCR (with NS bit set)
  	isb
  
  	/* back to svc mode */
  	bic r2, r2, #0x1f
  	orr r2, r2, #0x13          @ 0x13: svc mode
  	msr cpsr, r2
  	movs pc, lr
  
  reset:
  
  	// for jlink
  	ldr r0, =(REG_ARM_JTAG_CTL - GX_REG_VIRTUAL_BASE1)
  	mov r1, #0xff
  	str r1, [r0]
  
  	/* clear CNTOFF for arm generic timer */
  	bl clear_cntvoff
  
  	// halt cpu1/2/3
  	mrc     p15, 0, r0, c0, c0, 5
  	ubfx    r0, r0, #0, #2
  	cmp     r0, #0
  	wfine
  
  	// Disable Watchdog
  	ldr r0, =(REG_BASE_WDT - GX_REG_VIRTUAL_BASE1)
  	mov r1, #0
  	str r1, [r0]
  
  	// Disable Interrupts (FIQ and IRQ), also set the cpu to SVC32 mode, except if in HYP mode already
  	mrs r0, cpsr
  	and r1, r0, #0x1f            // mask mode bits
  	teq r1, #0x1a                // test for HYP mode
  	bicne   r0, r0, #0x1f        // clear all mode bits
  	orrne   r0, r0, #0x13        // set SVC mode
  	orr r0, r0, #0xc0            // disable FIQ and IRQ
  	msr cpsr,r0
  
  	cpsie A                         @ Enable non-precise data aborts
  
  	// Set V=0 in CP15 SCTLR register - for VBAR to point to vector
  	mrc	p15, 0, r0, c1, c0, 0	 // Read CP15 SCTLR Register
  // base address:0x00000000
  	bic	r0, #CR_V		@ V = 0
  	mcr	p15, 0, r0, c1, c0, 0	 // Write CP15 SCTLR Register
  
  	// Set vector address in CP15 VBAR register
  	ldr	r0, =ResetEntry
  // 16bit align
  	mcr	p15, 0, r0, c12, c0, 0	 // Set VBAR
  
  #if (defined(CONFIG_ENABLE_LIB) && !defined(CONFIG_ENABLE_GDB_DEBUG))
  	#--- Initialise stack register, stack_base is __bss_end_+1M
  	ldr     r0, =__bss_end__
  	add     r0, r0, #0x100000
  	mov     sp, r0
  #if defined(CONFIG_ENABLE_IRQ)
  	mrs r0, cpsr
  	bic r0, r0, #0xc0            // enable FIQ and IRQ
  	msr cpsr,r0
  #endif
  	#b main                # for ota
  #else
  	// Invalidate the I-Cache
  	mov     r0, #0
  	mcr     p15, 0, r0, c7, c5, 0
  	// Invalidate the D-Cache
  	mrc     p15, 1, r0, c0, c0, 0       // Read the Cache Size Identification register (CCSIDR)
  	mov     r0, r0, LSL #10
  	mov     r0, r0, LSR #23             // Mask off to leave the NumSets
  	mov     r2, #0x0                    // Set r2 to initial MVA (Way=0, Set=0)
  	mov     r1, #0                      // Use r1 as loop counter for WAYs
  	mov     r3, #0x0                    // Use r3 as a loop counter for SETs
  
  invalidate_cache_loop:
  	mcr     p15, 0, r2, c7, c6, 2       // DCISW - Invalidate data cache by set/way
  	add     r2, r2, #0x0020             // Increment the SET field
  	add     r3, r3, #1                  // Increment loop counter
  	cmp     r3, r0                      // Compare loop counter with num_sets
  	ble     invalidate_cache_loop       // If (loop_counter =< num_sets) branch
  										// Prepare register for next pass
  	add     r2, r2, #0x40000000         // Increment WAY field
  	and     r2, r2, #0xC0000000         // Clear the rest of the register (clear the SET field)
  	mov     r3, #0                      // Reset loop counter
  
  	add     r1, r1, #1
  	cmp     r1, #4
  	bne     invalidate_cache_loop
  
  	mov		r4,#0xffffffff
  	mcr		p15,0,r4,c3,c0,0        // domain
  
  	// Invalidate TLB
  	mov     r0, #0
  	mcr     p15, 0, r0, c8, c7, 0   // Invalidate entire unified TLB
  	mcr     p15, 0, r0, c8, c6, 0   // Invalidate entire data TLB
  	mcr     p15, 0, r0, c8, c5, 0   // Invalidate entire instruction  TLB
  
  	// set stage1 stack
  	ldr	sp, =STAGE1_STACK_TOP_ADDR_NOMMU
  
  #ifndef CONFIG_ENABLE_GDB_DEBUG
  	# initialize PLL
  	bl    gx_setup_pll_mini_controller
  	# initialize DRAM
  	bl    gx_setup_sdram_controller
  #endif
  
  mmu_set:
  	// Set Table Base Control Register
  	mov     r0, #0
  	mcr     p15, 0, r0, c2, c0, 2
  
  	// Page table modifications
  	// Stored in memory is a set of page tables that flat-maps memory
  	ldr     r0, =(HARDWARE_PAGE_TABLE_BASE - GX_REG_VIRTUAL_BASE1)        // Location of master copy of TTB
  	// Set location of L1 page table
  	mcr     p15, 0, r0, c2, c0, 0
  
  
  	mrc p15, 0, r0, c1, c0, 1 // Read Auxiliary Control Register
  	orr     r0, r0, #0x40               // enable SMP bit
  #ifdef	ENABLE_SECURE_ALIGN
  	orr     r0, r0, #0x0800             // Set L2RADIS 11bit
  	orr     r0, r0, #0x1000             // Set L1RADIS 12bit
  #endif
  	mcr p15, 0, r0, c1, c0, 1 // Write Auxiliary Control Register
  
  	// enable mmu
  	dsb
  	mrc     p15, 0, r0, c1, c0, 0       // Read SCTLR - System Control Register
  	orr     r0, r0, #0x01               // Set M bit (bit 0)
  	mcr     p15, 0, r0, c1, c0, 0       // Write SCTLR
  	isb
  
  	// enable cache
  	mrc     p15, 0, r0, c1, c0, 0       // Read System Control Register configuration data
  	orr     r0, r0, #0x0004             // Set C bit
  	orr     r0, r0, #0x1000             // Set I bit
  	mcr     p15, 0, r0, c1, c0, 0       // Write System Control Register configuration data
  
  #ifdef ENABLE_DENALI_ELF
  	bl    gx_setup_pll_full_controller
  	b .
  #endif
  
  #if (defined(CONFIG_ENABLE_MEMORY_TEST) || defined(CONFIG_ENABLE_MEMORY_PHASE_TEST))
  	#memory test program shall not set stack pointer to address at dram
  #else
  	// set stage2 stack
  	ldr	sp, =STACK_TOP_ADDR
  #endif
  
  #ifndef CONFIG_ENABLE_GDB_DEBUG
  #if defined(BOOT_FROM_FLASH)
  	# copy bootloader into DRAM
  	# loading priority: SPI -> NOR -> NAND
  	bl    gx_copy_bootloader
  #endif
  #endif
  
  	#jump to C entry in the RAM
  #if defined(CONFIG_ENABLE_IRQ)
  	mrs r0, cpsr
  	bic r0, r0, #0xc0            // enable FIQ and IRQ
  	msr cpsr,r0
  #endif
  
  #ifdef CONFIG_ENABLE_LIB
  	b main
  #else
  	b entry
  #endif
  #endif
  
  /*
   * exception handlers
   */
  undefined_instruction:
  software_interrupt:
  prefetch_abort:
  data_abort:
  not_used:
  	b .
  
  #ifndef CONFIG_ENABLE_IRQ
  irq:
  fiq:
  	b .
  #else
  @
  @ IRQ stack frame.
  @
  #define S_FRAME_SIZE	72
  
  #define S_OLD_R0	68
  #define S_PSR		64
  #define S_PC		60
  #define S_LR		56
  #define S_SP		52
  
  #define S_IP		48
  #define S_FP		44
  #define S_R10		40
  #define S_R9		36
  #define S_R8		32
  #define S_R7		28
  #define S_R6		24
  #define S_R5		20
  #define S_R4		16
  #define S_R3		12
  #define S_R2		8
  #define S_R1		4
  #define S_R0		0
  
  #define MODE_SVC 0x13
  #define I_BIT	 0x80
  
  #define IRQ_STACK_START  (IRQ_STACK_TOP_ADDR - 4)
  #define FIQ_STACK_START  (IRQ_STACK_START - CONFIG_STACKSIZE_IRQ)
  
  /*
   * use bad_save_user_regs for abort/prefetch/undef/swi ...
   * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
   */
  	.macro	irq_save_user_regs
  	sub	sp, sp, #S_FRAME_SIZE
  	stmia	sp, {r0 - r12}			@ Calling r0-r12
  	@ !!!! R8 NEEDS to be saved !!!! a reserved stack spot would be good.
  	add	r8, sp, #S_PC
  	stmdb	r8, {sp, lr}^		@ Calling SP, LR
  	str	lr, [r8, #0]		@ Save calling PC
  	mrs	r6, spsr
  	str	r6, [r8, #4]		@ Save CPSR
  	str	r0, [r8, #8]		@ Save OLD_R0
  	mov	r0, sp
  	.endm
  
  	.macro	irq_restore_user_regs
  	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
  	mov	r0, r0
  	ldr	lr, [sp, #S_PC]			@ Get PC
  	add	sp, sp, #S_FRAME_SIZE
  	subs	pc, lr, #4		@ return & move spsr_svc into cpsr
  	.endm
  
  	.macro get_irq_stack			@ setup IRQ stack
  	ldr	sp, =IRQ_STACK_START
  	.endm
  
  	.macro get_fiq_stack			@ setup FIQ stack
  	ldr	sp, =FIQ_STACK_START
  	.endm
  
  /*
   * exception handlers
   */
  	.align	5
  irq:
  	get_irq_stack
  	irq_save_user_regs
  	bl	gx_irq_handler
  	irq_restore_user_regs
  
  	.align	5
  fiq:
  	get_fiq_stack
  	/* someone ought to write a more effiction fiq_save_user_regs */
  	irq_save_user_regs
  	bl	gx_fiq_handler
  	irq_restore_user_regs
  #endif
  	.end
  ```

  







# GIC

ARM 提携结构定义了通用中断控制器(GIC)，该控制器包括一组用于管理单核或多和系统中的中断的硬件资源。

GIC 提供了内存映射寄存器，可用于管理中断源和行为，以及用于将中断路由到各个 CPU 核。它使软件能够屏蔽，启用和禁用来自各个中断源的中断，以(在硬件中)对各个中断源进行优先级排序和生成软件触发中断，还提供对 `TrustZone` 安全性拓展的支持。

GIC 接收系统级中断的产生，并可以发信号通知给它所连接的内个内核，从而有可能导致 IRQ 和 FIQ 异常发生。

- 分发器：系统中所有中断源都连接到这里，通过配置分发器的寄存器来控制各个中断源的属性(优先级、状态、安全性、路由信息和使能状态等等)。分发器将中断输出到 `CPU 接口`，后者决定将哪个中断转发给 CPU 核
- CPU 接口：CPU 核通过控制器的 CPU 接口单元接收中断。这里的寄存器用于屏蔽、识别和控制转发到 CPU 核的中断的状态。



- 中断类型：SGI、PPI、SPI
  - SGI： 通过软件写到某个寄存器来触发中断。常用于 cpu 核间通信
  - PPI： cpu 核的私有中断
  - SPI：外设生成的，可以路由到其它核。中断号为 32~1020。终端类型可以使边沿触发或电平触发
- 中断状态：
  - 非活动状态：中断没触发
  - 挂起：中断已出发，但是还没有处理
  - 活动：已被内核接收并正在处理的中断
  - 活动和挂起：cpu 正在处理中断，但是又来了一个相同的中断



GIC 作为内存映射的外围设备，被软件访问。所有内核都可以访问公共的 `distributor`，但是 `cpu interface` 是每个核都有专用的。



## 分发器(Distributor)

分发器主要用于配置各个中断的属性。

- 中断优先级：决定将哪个中断转发给 cpu 接口
- 中端配置：电平触发还是边沿触发
- 中断目标：可以将中断发给哪些 cpu 核
- 中断启用/禁用
- 中断安全性：将中断分配给 Secure 还是 Normal
- 中断状态：提供优先级屏蔽，可以防止低于某个优先级的中断发送给 cpu 核





## CPU Interface



## 软件配置

### 初始化

`Distributor` 和 `cpu interface` 在复位时均被禁用。复位后，必须初始化 GIC，才能将中断传递给 CPU 核。

对于 `Distributor` 软件必须配置优先级、目标和、安全性并启用单个中断

对于 `cpu interface` 软件必须配置优先级和抢占

为了使某个中断可以触发 cpu 核，必须将各个中断，`Distributor、cpu interface` 全部使能，并将 CPSR 中断屏蔽位清零

![img](image/shapes%2F6ad3e2c3c60f0904f37f85814c9fe41224db8ad6%2F4754fe7f5e0e1f2f03408173e30a391c.png)



### GIC 处理中断

当 cpu 核接受到中断时，会跳到中断向量表执行。

顶层中断处理程序读 cpu 接口模块的 `intrerupt acknowledge register` 来获取中断 id，然后在 `Distributor` 中标记为 `active` 状态。然后执行对应的中断处理程序。执行完了之后将中断 id 写入 `cpu interface` 中的 `End of Interrupt register` 中断结束寄存器表示中断处理完成。将当前中断移除 `active` 状态。

```
handler
	read intrerrupt acknowledge registe(interrupt id)
		interrupt id --> active
			interrupt_id --> handler
				write intrerrupt id to End of Interrupt register
					interrupt id --> no active
end
```





#### Distributor Register

- GICD_CTLR：用于使能 `Group0、1`
- GICD_TYPER：是否安全扩展
- GICD_IIDR：版本号
- GICD_IGROUPRn：配置中断为 `Group0/1`
- GICD_ISENABLERn：配置使能相应中断的转发`(Distributor --> Cpu interface)`以及获取中断是否使能转发状态
- GICD_ICENABLERn：配置禁用相应中断的转发
- GICD_ISACTIVERn：配置激活相应的中断
- GICD_ICACTIVERn：配置停用相应的中断
- GICD_IPRIORITYRn：配置中断优先级
- GICD_ITARGETSRn：配置中断可转发的 cpu 接口
- GICD_ICFGRn：配置中断触发类型：电平/边沿
- ICPIDR2：标识寄存器

```
GICD_CTLR
	1 EnableGrp1  R/W 用于将 pending Group1 中断从 Distributor 转发到 cpu interface
	0 EnableGrp0  R/W                    0 
	
GICD_TYPER
	15:11 LSPI    R    如果 gic 实现了安全拓展，此字段是可锁定 spi 的最大数量，0~31
					   如果 gic 没有实现安全拓展，此字段保留
	10   SecurityExtn R 0：未实施安全扩展
						1：实施了安全扩展
	7:5  Cpu number  R 表示 cpu interface 数量 = Cpu number + 1
	3:0  ITLineNumber R GIC 支持的最大中断数。max = 32*(ITLineNumber + 1)
	
GICD_IIDR
	31:24 ProductID R   产品表示 ID
	19:16 Variant   R   产品的主版本号
	15:12 Revision  R   产品的此版本号
	11:0  Implementer R JEP106 代码。11:8=0x4;7=0; 6:0=0x3b

// 中断号 irq，寄存器 n，位：b
// n = irq / 32
// b = irq % 32
GICD_IGROUPRn
	31:0 Group status bits R/W 组状态为，对应每个位：
						   0：相应中断为 Group0
						   1：         Group1

// 中断号 irq 和上面一样的算法
GICD_ISENABLERn
	31:0 Set-enable bits R/W 对于 SPI 和 PPI，每 bit 控制对应中断的转发行为：
								Distributor --> CPU interface
							  R：0 禁止转发；1：使能转发
							  W：0 无效；    1：使能转发
                    
GICD_ICENABLERn
	31:0 Clear-enable bits R/W R：0 禁止转发；1：使能转发
							   W：0 无效；    1：禁用转发
							   
GICD_ISACTIVERn
	31:0 Set-active bits R/W
								R: 0 对应中断不是 active 状态；1：是 active
								W：0 无效     1：把相应中断设为 active 状态
					
GICD_ICACTIVERn
	31:0 Clear-active bits R/W
								R: 0 相应中断不是 active 状态；1：是 active 状态
								W：0 无效      1：把相应中断设为 deactive
								
GICD_IPRIORITYRn
	31:24 Priority offset3 R/W  对于每个中断，都有 8bit 描述优先级。值越小优先级越高
	23:16 Priority offset2 R/W  
	15:8  Priority offset1 R/W  
	 7:0  Priority offset0 R/W  
	 
GICD_ITARGETSRn
	31:24 Cpu targets offset3 R/W 对于每个中断，都有 8bit 描述可以这个中断可以转发给那些cpu
								  8bit 中每 bit 代表对应的 cpu。0x3 --> cpu0、cpu1
								  
GICD_ICFGRn
	2F+1:2F Int_config number.F R/W 每个中断 2bit 描述。
	                                Int_config[1] 0：电平；1：边沿
	                                Int_config[0] 保留
	                              
ICPIDR2
	7:4  ArchRev  R/W             GIC 架构版本 0x1:GICv1;0x2:GICv2
```



#### Cpu interface Register

- GICC_CTLR: 配置中断的 `enable/disable`
- GICC_PMR: 配置中断优先级筛选器，大于配置值的中断才能发给 cpu
- GICC_BPR：配置中断优先级拆成组优先级和子优先级
- GICC_IAR：获取中断 id 以及产生中断的 cpu id`(SGI 类中断)`
- GICC_EOIR：配置中断处理完成

```
GICC_CTLR
	9  EOImodeNS     R/W    保留对 GICC_EOIR 和 GICC_DIR 寄存器的非安全访问
							0：
    6  IRQBypDisGrp1 R/W    当 Cpu interface 的 irq 信号禁用时，控制是否向处理器发送 bypass IRQ 信号
    						0：发送 bypass irq 信号给处理器；1：不发送
    5  FIQBypDisGro1 R/W    fiq disable，--> bypass fiq --> process
    0  NONE          R/W    0：disable interrupt；1：enable interrupt
    
    
GICC_PMR
	7:0 NONE         R/W   配置中断优先级掩码，大于这个值的中断才会发给 cpu
	
GICC_BPR
	2:0 Binary point R/W   控制如何将 8bit 优先级拆成组优先级和子优先级
	
GICC_IAR
	12:10 CPUID      R     对于 SGI 类中断，表示 cpu n 发出中断
	9:0   intr id    R     中断 id
	
GICC_EOIR
	12:10 CPUID      W
	9:0   eoi int id W     中断 ID
```



### init

- 配置 `Distributor、Cpu interface`基地址，探测最大中断号
- Disable 所有的中断转发
- 清除所有的 `pending` 状态
- 设置中断屏蔽级别`PMR`
- 使能 `Group0`组的中断，启用 `cpu` 接口向连接的处理器发送 `Group0` 中断的信号

```c
void gic_op_init(uint32_t gic_base)
{
	uint32_t n;
	struct gic_data *gd = &global_gd;

	memset(&global_gd, 0, sizeof(struct gic_data));

    // gd->gicd_base = gic_base + 0x1000
    // gd->gicc_base = gic_base + 0x2000
    	// disable all interrupt:GICC_CTLR = 0
    	// probe 最大的中断号，使能中断转发，又禁止，看是否成功
    // gd->max_id    = max_id
	gic_init_base_addr(gd, gic_base);

	for (n = 0; n <= gd->max_it / NUM_INTS_PER_REG; n++) {

		/* Disable interrupts */
		__raw_writel(0xffffffff, gd->gicd_base + GICD_ICENABLER(n));

		/* Make interrupts non-pending */
		__raw_writel(0xffffffff, gd->gicd_base + GICD_ICPENDR(n));
	}

	/* Set the priority mask to permit Non-secure interrupts, and to
	 * allow the Non-secure world to adjust the priority mask itself
	 */
#if defined(CFG_ARM_GICV3)
	write_icc_pmr(0x80);
	write_icc_ctlr(GICC_CTLR_ENABLEGRP0 | GICC_CTLR_ENABLEGRP1 |
		       GICC_CTLR_FIQEN);
#else
    // 0x80 = 128，只有高四位有效？ 0x8?
	__raw_writel(0x80, gd->gicc_base + GICC_PMR);

	/* Enable GIC */
	__raw_writel(GICC_CTLR_ENABLEGRP0, gd->gicc_base + GICC_CTLR);
#endif
    // enable group0
	__raw_writel(__raw_readl(gd->gicd_base + GICD_CTLR) | GICD_CTLR_ENABLEGRP0, gd->gicd_base + GICD_CTLR);
}
```





### request_irq

- 中断号 `+32`
- 将中断号、中断服务函数、参数、类型都拷贝到 h
- disable `Distributor --> Cpu interface` 的转发
- 清除中断的 `pending` 状态
- 将要注册的中断划分到 `group0`
- 允许此中断路由到所有的 `cpu` 接口
- 设置此中断的优先级
- 将 h 加到全局的中断链表中
- enable `Distributor --> Cpu interface` 转发

```c
int gic_op_request(uint32_t interrupt, enum interrupt_type type,
		interrupt_handler_t handler, void *data)
{
	int ret = -1;
	struct gic_data *gd = &global_gd;
	struct itr_handler *h = malloc(sizeof(struct itr_handler));
	if (!h) {
		printf("error: %s %s %d\n", __FILE__, __func__, __LINE__);
		goto exit;
	}

#ifdef INTC_NUM_MODFIY
    // 中断号 +32
	interrupt += INTC_NUM;
#endif
	h->it = interrupt;
	h->handler = handler;
	h->data = data;
	h->type = type;

	if (h->it >= gd->max_it) {
		printf("error: %s %s %d\n", __FILE__, __func__, __LINE__);
		goto exit;
	}

    // disable distributor --> cpu interface
    // clean pending
    // it --> group0
	gic_it_add(gd, h->it, type);
	/* Set the CPU mask to deliver interrupts to any online core */
    // route it to all cpu(0xff --> 11111111 11111111)
	gic_it_set_cpu_mask(gd, h->it, 0xff);
    // prio --> 0x1
	gic_it_set_prio(gd, h->it, 0x1);

    // h --> handlers.link
	SLIST_INSERT_HEAD(&handlers, h, link);

	ret = 0;
exit:
	if ((ret < 0) && (h)) {
		free(h);
		h = NULL;
	}
	return ret;
}

static void gic_it_enable(struct gic_data *gd, uint32_t it)
{
	uint32_t idx = it / NUM_INTS_PER_REG;
	uint32_t mask = 1 << (it % NUM_INTS_PER_REG);

	if (it >= NUM_SGI) {
		/*
		 * Not enabled yet, except Software Generated Interrupt
		 * which is implementation defined
		 */
		assert(!(__raw_readl(gd->gicd_base + GICD_ISENABLER(idx)) & mask));
	}

	/* Enable the interrupt */
    // enable distributor --> cpu interface
	__raw_writel(mask, gd->gicd_base + GICD_ISENABLER(idx));
}

void gic_op_enable(uint32_t interrupt)
{
	struct gic_data *gd = &global_gd;

#ifdef INTC_NUM_MODFIY
	interrupt += INTC_NUM;
#endif
	if (interrupt >= gd->max_it) {
		printf("error: %s %s %d\n", __FILE__, __func__, __LINE__);
		return;
	}

	gic_it_enable(gd, interrupt);
}


```





### process_irq

- 发生中断，pc 跳转到异常向量表的对应位置执行
- 配置 irq 模式的堆栈
- 保存寄存器到堆栈，用于后续跳转到 c 函数
- 读 GICC_IAR 寄存器获取中断 id
- 根据中断 id 从注册的中断链表中找到对应的中断处理信息，执行 handler
- 写入 GICC_EOIR 寄存器表示中断处理完成
- 恢复堆栈，pc 继续执行触发中断前的下一条指令

```c
ResetEntry:
	b	reset
	ldr pc, _undefined_instruction
	ldr pc, _software_interrupt
	ldr pc, _prefetch_abort
	ldr pc, _data_abort
	ldr pc, _not_used
	ldr pc, _irq
	ldr pc, _fiq

_undefined_instruction: .word undefined_instruction
_software_interrupt:    .word software_interrupt
_prefetch_abort:    .word prefetch_abort
_data_abort:        .word data_abort
_not_used:      .word not_used
_irq:           .word irq
_fiq:           .word fiq

    
	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	@ !!!! R8 NEEDS to be saved !!!! a reserved stack spot would be good.
	add	r8, sp, #S_PC
	stmdb	r8, {sp, lr}^		@ Calling SP, LR
	str	lr, [r8, #0]		@ Save calling PC
	mrs	r6, spsr
	str	r6, [r8, #4]		@ Save CPSR
	str	r0, [r8, #8]		@ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4		@ return & move spsr_svc into cpsr
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, =IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, =FIQ_STACK_START
	.endm
    
/*
 * exception handlers
 */
	.align	5
irq:
	// 配置 irq 模式的栈
	get_irq_stack
    // sp = sp - 72
    // 保存 sp、r0~r12
    // r8 = sp + 60
    // 把 sp、lr 依次存到 r8 所在的地址
    // r8 = lr
    // r6 = spsr
    // r8 + 4 = spsr
    // r8 + 8 = r0
    // r0 = sp
	irq_save_user_regs
	bl	gx_irq_handler
    // 从 sp 基址出栈，r0~lr 全部出栈
    // lr = sp + 60
    // sp = sp + 72
    // pc = lr - 4
	irq_restore_user_regs

	.align	5
fiq:
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
	bl	gx_fiq_handler
	irq_restore_user_regs
#endif
	.end


        
        
static void git_it_handle(uint32_t it)
{
	struct gic_data *gd = &global_gd;
    // 从链表中根据 interrupt id 找到对应的 handler
	struct itr_handler *h = find_handler(it);
	uint32_t vector = it;

	if (!h) {
		printf("error : not found interrupt %d, and disable it!\n", it);
		gic_it_disable(gd, it);
		return;
	}

#ifdef INTC_NUM_MODFIY
	vector -= INTC_NUM;
#endif
    // 调用中断处理函数，也就是 request 时注册的  handler
	if ((h->handler)(vector, h->data) != HANDLED) {
		printf("error : interrupt %d handle error, and disable it!\n", it);
		gic_it_disable(gd, it);
	}
}
 
       
void gic_op_irq_handle(void)
{
	uint32_t aiar;
	uint32_t id;
	struct gic_data *gd = &global_gd;
	
    // read GICC_IAR，表示开始处理中断
	aiar = gic_read_iar(gd);
	id = aiar & GICC_IAR_IT_ID_MASK;

	if (id < gd->max_it)
		git_it_handle(id);
	else
		printf("ignoring interrupt %d", id);

    // write GICC_EOIR 表示中断处理完成
	gic_write_eoir(gd, aiar);
}

```











# 内存排序

ARM 架构的旧处理器按程序顺序执行所有指令，并且每条指令在下一条指令启动之前完全执行。较新的处理器采用了许多与指令执行顺序和内存访问执行方式相关的优化。

- 重新排序内存访问：**内核执行 load 和 store 指令的顺序不一定与外部设备看到访问的顺序相同。**

举例：

```
STR R12, [R1]            Ins1
LDR R0, [SP], #4         Ins2
LDR R2, [R3,#8]          Ins3

Ins1:存储指令
Ins2：加载指令，未命中 cache
Ins3：加载指令，命中 cache

这三条指令的顺序可能是：Ins1 执行对外部存储器的写入操作，可能先进行读取操作，执行 Ins2 的时候没有命中 cache 就会执行 Ins3，命中 Cache 可能先执行 Ins3，然后 Ins2 重新加载数据到 Cache，最后执行 Ins1 的写入操作
Ins3 --> Ins2 --> Ins1
```

根据上面的例子可以看出会出现指令执行顺序不一致的情况，可以通过 `指令屏障` 的方式来显示等待指令执行操作完成。



## 内存类型



Cortex-A 系列处理器采用弱序内存模型。可以通过将特定内存区域标记为 `Strong-orderd` 类型保证按照指令按照顺序执行。

所有的内存区域都配置为以下三种类型之一：

- `Strongly-orderd`：强序
- `Device`：设备
- `Normal`：正常

对于 Normal 内存可以指定内存是否为 `Sharable(其它代理(dma)可访问)`。

下表中的 A1 和 A2 是两个不重叠的地址，A1 比 A2 先写，但实际写入顺序可以不一样：

| A1 / A2        | Normal         | Device         | Strongly-orded |
| -------------- | -------------- | -------------- | -------------- |
| Normal         | 没有强制的顺序 | 没有强制的顺序 | 没有强制的顺序 |
| Device         | 没有强制的顺序 | 按顺序执行     | 按顺序执行     |
| Strongly-orded | 没有强制的顺序 | 按顺序执行     | ·按顺序执行    |

- 如果 A1 是一个 Device 类型，A2 是 `Normal`类型则写入顺序没有强制要求，如果 A2 是 `Device/Strongly-orded` 类型则必须按照顺序执行写入操作

分别介绍这三种类型：

- `Strongly-orded and Device memory`
  - 对于这两种类型的访问具有相同的内存排序模型。规则如下：
    - 将保留访问的数量和大小。访问将是原子的，并且不会中途中断。
    - 读取和访问操作都可能对系统产生副作用。访问永远不会被 cache，永远不会执行推测访问。
    - 访问不能不对齐，也就是必须对齐
    - 到达 `Device memory` 的访问顺序保证与指令顺序相对应。
  - Device 和 Strongly-orderd 唯一的区别是：
    - 对于 `Strongly-orderd` 的写入只有在到达写入外设访问的外围设备或内存组件时才能完成。
      - 必须写到外设或者内存才算执行完成
    - 允许对 `Device memory` 的写入在到达写入访问的 外设或内存组件之间完成。
      - 可以在写到外设或内存之前就算完成
  - 系统外设几乎总是映射为 `Device memory`。
- `Normal memory`
  - 正常内存用于描述内存系统的大部分。所有 rom 和 ram 设备都被视为普通内存。
    - 可以重复读取和写入
    - 可以预取和推测访问，而没有副作用。**不会执行推测写入。**
    - 可以执行未对齐的访问
    - 核心硬件可以将多个访问合并成较小数量、更大的访问。例如：多个单字节写入可以合并成单个双字写入
  - Normal memory 还必须具有描述的 `cachebility`  属性。
  - 可共享性
    - Normal memory 必须指定为 `shareable/ Non-shareable`。
      - `Non-shareable` 的内存区域是仅由此内核使用的区域。对这片区域的访问不需要和其它内核保持一致。
      - 如果与其它内核共享，则需要软件处理一致性问题。



## 内存屏障

内存屏障是一条指令，它要求内核在程序中内存屏障指令之前和之后发生的内存操作之间应用排序约束。也被称为内存栅栏。

- 数据同步屏障(DSB)：强制所有核心等待所有待处理的显示数据访问完成，然后才能执行其它指令。对预取指令没影响。
  - 避免后面的数据访问指令运行
  - 比 DMB 更严格，仅当所有在它前面的存储器访问操作都执行完毕后，才执行在它后面的指令(任何指令都要等待存储器访问操作)
- 数据存储配置(DMB)：确保在屏障之前按顺序执行的内存访问在系统中被观察到，然后才会执行 barrier 之后的任何显式内存访问。
  - 避免后面的指令运行，充当内存屏障
  - 仅当所有在它前面的存储器访问都执行完毕后，才提交在它后面的存储器访问操作。
- 指令同步屏障(ISB)：刷新内核中的指令流水线和预取缓冲区，在指令完成后访问的指令全是从 cache 或 memory 中重新获取的。
  - 清空流水线



- Linux 处理方式：
  - 读取内存屏障(rmb)：屏障在执行屏障之后出现的任何读取之前完成。
  - 写入内存屏障(wmb)：就是 dsb
  - 内存屏障：就是 dmb

- Cache 一致性影响：
  - 在 dma 启动之前，必须显示清除 cache 中的脏数据
  - 如果 dma 正在复制由 core 读取的数据，需要确保 core 的 dcache 中不包含过时的数据，也就是 dma 要访问到新的 core 的 dcache 数据
  - dma 写内存不会更新 cache，需要内核在启动 dma 之前从 cache 中清除受影响的内存区域或者使其 无效
  - 由于 ARMv7-A 处理器都会执行推测访问，因此在使用 dma 后也有必要失效 cache。
- 复制代码的问题：将程序从一个位置复制到另一个位置，或者修改内存中的代码。没有硬件机制维护一致性，必须通过使受影响的区域无效来时 cache 中的代码失效，并确保写入的代码实际到达 memory。
- 编译器重排优化：内存屏障仅适用于内存访问的硬件重新排序。插入内存屏障指令可能不会对编译器的操作重新排序产生任何直接影响。











# 异常处理

异常就是要求内核停止正常执行，转到执行与每种异常类型关联的专用软件处理程序(异常处理程序)的任何情况。通过需要采用补救措施或特权软件更新系统状态以确保系统平稳运行。这称为异常处理。处理异常后，特权软件会准备 内核 在接受异常之前恢复它正在执行的任何操作。

正常程序执行时，pc 在地址空间递增，程序中的显示分支会修改执行流程。发生异常时，此先确定的执行顺序将被打断，并暂时切换到异常处理程序来处理异常。



## 异常类型

ARMv7-A 和 ARMv7-R 支持多种处理器模式：`FIQ、IRQ、Supervisor、Abort、Undefined、System` 六种特权模式，以及非特权模式。

当发生异常时，内核会保存当前状态和返回地址，进入特定模式，并可能禁用硬件中断。异常处理程序从称为异常向量的固定内存地址开始。可以在特权模式下将一组异常向量的位置编程到系统寄存器中，并在发生相应异常时自动执行它们。

存在以下类型的异常：

- Interrupts：
  - ARMv7-A 内核上提供两种类型的中断，称为 IRQ 和 FIQ。
  - FIQ 的优先级高于 IRQ。由于 FIQ 在异常向量表中的位置以及 FIQ 模式下可用的 备份 register 多，因此FIQ 具有一些潜在的速度优势。
  - FIQ 和 IRQ 都是到 core 的物理信号，当置位时，如果使能，则 core 将执行相应的异常。几乎所有系统，都将使用中断控制器将所有中断源连接。中断控制器对终端进行仲裁和优先级排序，进而提供一个串行化的单个信号，然后连接到内核的 IRQ / FIQ 信号。
  - IRQ 和 FIQ 中断的发生与内核在任何给定时间执行的软件没有直接关系，因为归类为 `异步异常`。
- Aborts：
  - 指令获取失败(prefetch aborts) 或 数据访问失败 (data aborts) 时生成 `Aborts`。它们可以来自外部内存系统，在内存访问时给出错误相应，或者由 mmu 生成。
  - 获取指令时，可以在管道中将其标记为 aborted。仅当 core 尝试执行 prefetch abort 异常时，才会采取该异常。异常发生在指令执行之前。如果在中止的指令到达管道的执行阶段之前刷新管道，则不会发生中止异常。当执行 load 或 store 指令时，会发生数据中止异常，并被视为在尝试读取或写入数据后发生。
- Reset：
  - 所有内核都有一个 reset 输入，并且在它们被 reset 后立即接收 reset 异常。它是优先级最高的异常，无法屏蔽。
  - 此异常用于在上电后在内核上执行代码以对其进行初始化。
- 异常生成指令：
  - 执行某些指令可以产生异常。执行此类执行通常是为了陷入到更高权限级别运行的环境：
    - `Supervisor Call(SVC)` 指令是用户模式程序能够请求操作系统服务。
  - 任何执行内核无法识别的指令的尝试都会生成 `UNDEFINED` 异常。

发生异常时，核心执行与该异常对应的处理程序。内存中存储处理程序的位置称为异常向量。在 ARM 架构中，异常向量存储在一个表中，称为异常向量表。因此，单个异常的向量可以位于距表开头的固定偏移量处。表的基址由特权软件在系统寄存器中编程，以便内核可以在发生异常时找到相应的处理程序。

异常向量表通常位于内存映射底部，从 `0x00 ~ 0x1C` 的对齐地址中。

![2024-11-05_19-48](image/2024-11-05_19-48.png)

![2024-11-05_19-48_1](image/2024-11-05_19-48_1.png)

进入异常处理程序时 CPSR 的 I 位 和 F 位状态：

![2024-11-05_19-49](image/2024-11-05_19-49.png)

异常向量表：

- 上面的表格给出了异常向量表中各异常的偏移量。这是内核在引发异常时跳转到的指令表。这些指令位于内存中的特定位置，默认向量表基址是 `0x00000000`，但大多数内核都允许将基地址移动到 `0xffff0000`。所有 `Cortex-A` 系列处理器都允许这样做，并且它是 Linux 内核选择的默认地址。
- 每个异常类型都有一个字节的地址关联。因此，对于每个异常，只能在向量表中放置 1 条指令 (或 2 条 Thumb 指令)。因此向量表内容一般都是下面两种形式：
  - `B<label>`：这将执行 `PC 关联分支`。适用于调用内存中足够接近的异常处理程序代码，以至于 branch 指令中提供的 24 位字段足够大。
    - b 的目标地址有限制，好像是 24位，所以 b 的时候不要调的距离当前 pc 太远
  - `LDR PC, [PC,#offset]`：这将从内存位置加载 PC，该内存位置的地址是相对于异常指令的地址定义的。允许将异常处理程序放置在完整 32 位内存空间内的任意地址
    - 可以加载 32位内存空间的任何地址，但是相对于 b 多了指令周期



- FIQ、IRQ：

  - FIQ 保留给需要保证快速响应时间的单个高优先级中断源。
  - IRQ 用于系统中的所有其它中断。
  - 由于 FIQ 是向量表中的最后一个条目，因此 FIQ 处理程序可以直接放在异常向量表的末尾位置，并从该地址按顺序运行。这样避免了分支指令和其它相关的延迟，从而加快了 FIQ 响应时间。以及 FIQ 模式下的镜像寄存器很多，也可以加快执行速度。
  - IRQ 和 FIQ 的另一个区别是 FIQ 处理程序预计不会生成其它异常。
  - Linux 一般不用 FIQ，但是不禁用 FIQ，保留接口

- return：

  - LR 寄存器用于在处理异常后为 PC 存储适当的返回地址。必须按照下表根据发生的异常类型修改值：

    ![2024-11-05_20-11](image/2024-11-05_20-11.png)





## 异常处理

发生异常时，ARM 内核会自动执行以下操作：

1. 将 CPSR 复制到 `SPSR_<mode>`
2. 将返回地址存储在新模式的 `LR_<mode>` 寄存器
3. 将 CPSR 模式位修改为异常类型关联的模式
   - 其它 CPSR 模式位设置为 CP15 中确定的值
   - T 为设置为 CP15 TE 位给出的值
   - J 位被清除，E 位设置为 EE 位的值
4. 将 PC 设置为指向异常向量表中的相关指令：

![2024-11-05_20-14](image/2024-11-05_20-14.png)

当处于新模式时，内核访问的寄存器将是新模式的特有寄存器。异常处理程序总是在进入异常时立即将 register 保存到堆栈。FIQ 因为寄存器多，所以可能不需要保留堆栈。

要从异常处理程序返回，必须以原子方式进行两个单独的操作：

1. 从保存的 SPSR 恢复 CPSR
2. 将 PC 设置为返回地址偏移量。例如是 IRQ 模式：`SUBS PC,R14, #4`

![2024-11-05_20-17](image/2024-11-05_20-17.png)

有多种方法可以实现此目的。

- 可以使用数据处理之灵调整 lr 并将其复制到 pc：

  - `SUBS pc, lr, #4`：指定 S 表示同时将 SPSR 复制到 CPSR

  - 如果异常处理程序入口代码使用堆栈保存了相关的一些寄存器，可以使用带有 `^` 限定符的 `load multiple` 指令返回。

    ```
    LDMFD sp! {pc}^
    LDMFD sp!, {R0-R12,pc}^
    
    ^:表示同时将 SPSR 复制到 CPSR
    ```





## Linux 异常程序流程

在启动过程中，内核会分配一个 4KB 的页面作为 vector 页面。它将此映射到异常向量、虚拟地址0xFFFF0000或0x00000000的位置。这是由

devicemaps_init（） 在文件 arch/arm/mm/mmu.c 中。这是在 ARM 系统启动的早期调用的。









# 中断处理

## 外部中断请求

通常，只有当适当的 CPSR 禁用位（分别为 F 和 I 位）被清除并且相应的输入被置位时，才能采取中断异常。

CPS 指令提供了一种简单的机制来启用或禁用由 CPSR A、I 和 F 位（分别为异步中止、IRQ 和 FIQ）控制的异常。

CPS IE 或 CPS ID 将分别启用或禁用异常。要启用或禁用的异常使用一个或多个字母 A、I 和 F 指定，不会修改相应字母的异常。

在 Cortex-A 系列处理器中，可以配置内核，以便 FIQ 无法被软件屏蔽。这称为 Non-Maskable FIQ，由硬件配置输入信号控制，该信号在内核复位时采样。在采取 FIQ 例外时，它们仍将被自动屏蔽。

简单的中断处理：

1. 外部硬件引发 IRQ 异常。核心自动执行几个步骤。当前执行模式下的 PC 内容存储在 LR_IRQ 中。将 CPSR 寄存器复制到 SPSR_IRQ。CPSR 内容更新，以便模式位反映 IRQ 模式，而 I 位设置为屏蔽其他 IRQ。PC 被设置为向量表中的 IRQ 条目。
   - 硬件自己做的
2. 执行向量表中 IRQ 项（中断处理程序的分支）处的指令。
   - 跳到异常向量表对应条目的汇编指令 `b xxx_handler`
3. 中断处理程序保存被中断程序的上下文，也就是说，它将任何将被处理程序损坏的 registers 推送到堆栈上。当处理程序完成执行时，这些 registers 将从堆栈中弹出。
   - 这里就是跳到中断处理 c 函数之前先保存堆栈，执行完了之后再出栈
4. 中断处理程序确定必须处理的中断源，并调用相应的设备驱动程序。
   - 一般是之前会注册对应中断号的中断服务程序。这里根据中断号跳到对应的中断服务函数
5. 通过将 SPSR_IRQ 复制到 CPSR，并恢复之前保存的上下文，最后从 LR_IRQ 中恢复 PC，准备核心切换到之前的执行状态。
   - 出栈，并且恢复 CPSR、PC



一个简单的中断处理函数：

![2024-11-05_20-32](image/2024-11-05_20-32.png)





嵌套中断：在保存了堆栈之后，手动打开 CPSR 中的 中断使能位











# Boot Code

内核从 Reset 中运行出来后是运行在裸机的情况，本章介绍 bootloader 如何加载和运行 Linux 内核。



## 引导裸机系统

当内核被重置后，它将在异常向量表中的重置向量位置开始执行`(0x00000000 / 0xffff0000)`。reset 处理程序代码必须执行以下部分或全部操作：

- 在多核系统中，使非主核进入休眠状态，需要参阅 `Booting SMP systems on page 18-17`
- 初始化异常向量
- 初始化内存系统，包括 MMU
- 初始化 core 模式堆栈和寄存器
- 初始化相关的 IO 设备
- 执行 `NEON/VFP` 的相关初始化
- 使能中断
- 更改 Core 模式或状态
- 处理 Secure 世界所需的任何设置
- 调用 main() 应用程序






# SP
SP：堆栈指针，指向当前堆栈的顶部 (即最后压入的数据地址)

```

-----
 栈  向下增长 
-----
 堆  向上增长
-----


```

## **1. SP 的基本特性**

- ​**寄存器名称**：`SP`（即 `R13`），但在不同处理器模式下（如 IRQ、SVC、FIQ 等）有 ​**独立的 SP**。
- ​**作用**：
    - 指向当前堆栈的顶部（即最后压入的数据地址）。
    - 支持 ​**函数调用时的参数传递、局部变量存储和返回地址保存**。
    - 在 ​**中断/异常处理**​ 时自动保存上下文（如程序计数器、寄存器状态）。

---

## ​**2. 堆栈的增长方向**

ARMv7 的堆栈默认采用 ​**满递减堆栈（Full Descending, FD）​**：

- ​**满（Full）​**：SP 指向最后一个压入堆栈的有效数据。
- ​**递减（Descending）​**：堆栈向 ​**低地址方向**​ 增长（即 `PUSH` 操作时 SP 减小）。

例如：

asm

```asm
PUSH {R0}    ; 将 R0 存入堆栈，SP = SP - 4
POP {R0}     ; 从堆栈恢复 R0，SP = SP + 4
```

---

## ​**3. 不同模式下的 SP**

ARMv7 有 ​**多种处理器模式**​（如 User、IRQ、SVC、FIQ 等），每个模式有 ​**独立的 SP**：

|模式|用途|对应的 SP（R13）|
|---|---|---|
|​**User**​|用户程序|`SP_usr`|
|​**IRQ**​|中断处理|`SP_irq`|
|​**SVC**​|系统调用（如 OS 内核）|`SP_svc`|
|​**FIQ**​|快速中断|`SP_fiq`|
|​**Abort**​|内存访问异常|`SP_abt`|
|​**Undefined**​|未定义指令异常|`SP_und`|
|​**System**​|特权模式（同 User）|共享 `SP_usr`|

- ​**关键点**：
    - 在 ​**异常/中断触发时**，处理器自动切换到对应模式，并 ​**使用该模式的 SP**。
    - 若未初始化某模式的 SP（如 IRQ 模式），可能导致堆栈溢出或数据损坏。



## 4. ARMv7 堆栈指针（SP）在压栈和出栈时的变化详解

在 ARMv7 架构中，堆栈默认采用 ​**满递减（Full Descending, FD）​**​ 模式，即：

- ​**压栈（PUSH）​**：SP ​**先减**​ 4 字节（32 位），再存入数据。
- ​**出栈（POP）​**：SP ​**先取**​ 数据，再 ​**增**​ 4 字节。

以下通过具体示例展示 SP 的变化过程：

---

### ​**1. 单寄存器压栈与出栈**

假设初始时 `SP = 0x8000`，寄存器 `R0 = 0x12345678`。

**步骤 1：压栈 `R0`**

asm

```asm
PUSH {R0}   ; 将 R0 压入堆栈
```

- ​**SP 变化**：
    - 压栈前：`SP = 0x8000`
    - 压栈后：`SP = 0x8000 - 4 = 0x7FFC`
- ​**内存变化**：
    - `0x7FFC` 处存储 `R0` 的值 `0x12345678`。

**步骤 2：出栈 `R0`**

asm

```asm
POP {R0}    ; 从堆栈恢复 R0
```

- ​**SP 变化**：
    - 出栈前：`SP = 0x7FFC`
    - 出栈后：`SP = 0x7FFC + 4 = 0x8000`
- ​**数据恢复**：
    - `R0` 的值恢复为 `0x12345678`。

---

### ​**2. 多寄存器压栈与出栈**

假设初始时 `SP = 0x8000`，`R0 = 0xAABBCCDD`，`R1 = 0x11223344`。

**步骤 1：压栈 `R0, R1`**

asm

```asm
PUSH {R0, R1}   ; 压入 R0 和 R1
```

- ​**SP 变化**：
    - 压栈前：`SP = 0x8000`
    - 压栈后：`SP = 0x8000 - 8 = 0x7FF8`
- ​**内存变化**：
    - `R0` 存储在 `0x7FFC`（较高地址）。
    - `R1` 存储在 `0x7FF8`（较低地址）。
- ​**内存布局**：
    
    markdown
    
    ```markdown
    地址      数据
    0x7FFC | 0xAABBCCDD (R0)
    0x7FF8 | 0x11223344 (R1)
    ```
    

**步骤 2：出栈 `R1, R0`**

asm

```asm
POP {R1, R0}    ; 恢复 R1 和 R0
```

- ​**SP 变化**：
    - 出栈前：`SP = 0x7FF8`
    - 出栈后：`SP = 0x7FF8 + 8 = 0x8000`
- ​**数据恢复**：
    - `R1` 恢复为 `0x11223344`（来自 `0x7FF8`）。
    - `R0` 恢复为 `0xAABBCCDD`（来自 `0x7FFC`）。

---

### ​**3. 寄存器顺序对数据恢复的影响**

若压栈和出栈顺序不一致，会导致数据错乱：

**错误示例**：

asm

```asm
PUSH {R0, R1}   ; 存储 R0=0xAABBCCDD, R1=0x11223344
POP {R0, R1}    ; 错误顺序！
```

- ​**结果**：
    - `R0` 恢复为 `0x11223344`（来自 `0x7FF8`）。
    - `R1` 恢复为 `0xAABBCCDD`（来自 `0x7FFC`）。
- ​**结论**：寄存器顺序被交换，导致数据错误。

---


## MMU
