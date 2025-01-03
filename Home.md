
今天是 **`=dateformat(date(today),"DD")`**，`=date(today).year` 年已经过去了 `=(date(today)-date(date(today).year + "-01-01")).days` 天.





## 学习任务
```dataview
list
from "03 备忘"
sort file.ctime desc
```



## Todo                                                                                                                          
- Virgo 外设对接 
```dataview
TASK
WHERE !completed
```




## 进行中

```dataview
list
from #doing
sort file.ctime desc
```


# 最近编辑
```dataview
table WITHOUT ID file.link AS "标题",file.mtime as "时间"
from !"10 归档" and !"1 看板"
sort file.mtime desc
limit 5
```

# 最近创建
```dataview
table WITHOUT ID file.link AS "标题",file.ctime as "时间"
from !"10 归档" and !"1 看板"
sort file.ctime desc
limit 5
```



# 日记

```dataview
table WITHOUT ID file.link AS "日记", dateformat(file.ctime,"DD") AS "创建时间", dateformat(file.mtime,"DD  HH:mm:ss") AS "最近修改时间"
from "00 日记" 
where dateformat(file.ctime,"MM-dd") = dateformat(date(today),"MM-dd") 
limit 100
```
