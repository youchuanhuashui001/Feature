
<br>
<br>

>**`="今天是 " + dateformat(date(now), "yyyy年MM月dd日，cccc")`** 
>计算机里没有什么黑魔法，所有我不清楚的，只是我现在还没有去搞懂里面的逻辑。
>总有一天，我会搞懂其中的所有东西。
><div class="container"> <div class="column" style="flex: 2;">


<br>
<br>

```base
views:
  - type: table
    name: todo
    filters:
      and:
        - file.hasTag("todo")
    order:
      - file.name
      - file.tags
      - file.ctime
    columnSize:
      file.name: 707
      file.tags: 123
  - type: table
    name: Doing
    filters:
      and:
        - file.hasTag("doing")
        - '!file.inFolder("06 资源")'

```


<br>
<br>

```base
formulas:
  未命名: ""
views:
  - type: table
    name: doing
    filters:
      and:
        - file.hasTag("doing")
        - '!file.inFolder("06 资源")'
    order:
      - file.name
      - file.tags
      - file.ctime
    sort:
      - property: formula.未命名
        direction: DESC
    columnSize:
      file.name: 708
      file.tags: 401

```

<br>
<br>

```base
views:
  - type: table
    name: note
    filters:
      and:
        - file.hasTag("note")
    order:
      - file.name
      - file.tags
      - file.ctime
    columnSize:
      file.name: 710
      file.tags: 125

```

<br>
<br>


```base
views:
  - type: table
    name: waiting
    filters:
      and:
        - file.hasTag("waiting")
    order:
      - file.name
      - file.tags
      - file.ctime
    columnSize:
      file.name: 713
      file.tags: 125

```

<br>
<br>

```base
views:
  - type: table
    name: study
    filters:
      and:
        - file.hasTag("study")
    order:
      - file.name
      - file.tags
      - file.ctime
    columnSize:
      file.name: 715
      file.tags: 123

```





---


## 概览
[[02 学习/schedule/学习计划|学习计划]]
[[Virgo_MPW 芯片测试进度]]
[[Linux 驱动学习主页]]

---





