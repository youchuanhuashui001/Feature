




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




```base
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
    columnSize:
      file.name: 708
      file.tags: 125

```



```base
views:
  - type: table
    name: note
    filters:
      and:
        - file.hasTag("note")

```


```base
views:
  - type: table
    name: waiting
    filters:
      and:
        - file.hasTag("waiting")

```



```base
views:
  - type: table
    name: study
    filters:
      and:
        - file.hasTag("study")

```



## 备忘录
```tasks
not done

path includes 03 备忘/备忘录

full mode

```


---


## Index
[[02 学习/schedule/学习计划|学习计划]]
[[Virgo_MPW 芯片测试进度]]
[[Linux 驱动学习主页]]

---





