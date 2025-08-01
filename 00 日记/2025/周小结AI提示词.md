
# AI 模板
```
**【周小结生成提示词】**

**目标：** 根据指定日期范围内的每日工作记录，生成一份符合特定格式和口吻的周小结。

**输入信息：**

1.  **周报周期：** 请明确周小结所涵盖的开始日期和结束日期（例如：5月6日至5月9日）。
2.  **每日工作记录文件路径：** 提供存放每日工作记录的目录路径（例如：`2025-5/`）。
3.  **每日工作记录文件名格式：** 确认每日记录的文件名格式（例如：`YYYY-MM-DD-星期X.md`）。
4.  **周小结主要内容来源：** 指出从每日记录的哪个部分提取工作内容（例如：主要从“今日工作安排”部分提取）。
5.  **任务状态标记：**
    *   进行中：`%{color:blue}【进行中】%`
    *   已完成：`%{color:green}【已完成】%`
    *   （如果日报中没有明确状态，默认或根据上下文判断，通常默认为“进行中”）
6.  **任务ID：** 如果日报中包含任务ID（例如 `#123456`），请在对应任务描述后附上。

**输出要求：**

1.  **返回格式：** 以 Markdown 源代码形式返回。
2.  **整体结构：**
    *   `* 本周小结（MM.DD~MM.DD）`
    *   `** 芯片研发：`
        *   `*** [具体芯片项目，如：Virgo 芯片研发]`
            *   `**** %{color:状态颜色}%【状态描述】% [具体工作内容描述] [#任务ID (可选)]`
        *   `*** [其他芯片项目]`
    *   `** 软件研发`
        *   `*** 软件基础平台`
        *   `*** GoXceed软件平台`
        *   `*** RobotOS软件平台`
    *   `* 下周计划`
    *   `** 芯片研发：`
        *   `*** [具体芯片项目，如：Virgo 芯片研发]`
            *   `**** [下周计划任务1]`
            *   `**** [下周计划任务2]`
        *   `*** [其他芯片项目]`
    *   `** 软件研发`
        *   `*** 软件基础平台`
        *   `*** GoXceed软件平台`
        *   `*** RobotOS软件平台`
3.  **口吻和描述：** 保持与用户提供的示例周小结（图片或文本）一致的专业口吻和描述风格。
4.  **“下周计划”部分：**
    *   根据本周未完成的工作和合理的预期来生成。
    *   **不需要**包含任何“请在此处补充...”之类的占位提示。如果某一项下没有具体计划，则该项下为空。
5. 以下是一个示例：

* 本周小结（4.7~4.11）
** 芯片研发：
*** Virgo 芯片研发
*** Sagitta 芯片研发
**** %{color:green}【已完成】% sagitta iodma 增加 pwm 功能，包括输出普通 pwm、互补 pwm、带死区的互补 pwm。 #405374
**** %{color:green}【已完成】% sagitta 实现事业部要求的 ws2812 灯效，包括彩虹灯、呼吸灯、按键灯、流水灯、固定颜色。 #405374
** 软件研发
*** 软件基础平台
*** GoXceed软件平台
**** %{color:green}【已完成】% 支持 nor flash BY25FQ64ES，已完成 #405529
**** %{color:green}【已完成】% 妙兵想把数据拷贝到 ddr 0 地址，但是发现 flash 驱动没有把数据拷到 0 地址。这是正常的，spinand 就是不允许往 0 地址写数据。 解决 spinand 驱动在 a55 编译器中的报错问题。 #406170
*** RobotOS软件平台
**** %{color:green}【已完成】% apus 支持 nor flash ZB25LQ128C，已完成 #405463



* 下周计划
** 芯片研发：
*** Virgo 芯片研发
**** virgo_ner 外设测试
*** Sagitta 芯片研发
** 软件研发
*** GoXceed软件平台
*** RobotOS软件平台








**操作步骤（AI 执行）：**

1.  根据提供的日期范围，定位并读取相应的每日工作记录文件。
2.  解析每个日报中指定部分（如“今日工作安排”）的内容。
3.  汇总本周完成及进行中的工作，注意提取任务状态和任务ID（如果存在）。
4.  基于本周工作进展，合理规划下周初步计划。
5.  严格按照用户指定的 Markdown 格式组织内容，并生成周小结的源代码。

```


