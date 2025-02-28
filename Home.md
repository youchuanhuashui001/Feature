---
banner: "![[06 资源/file/wallhaven-l85vk2_1920x1080.png]]"
banner_y: 0.51
---

```dataviewjs
let ftMd = dv.pages("").file.sort(t => t.cday)[0]
let total = parseInt([new Date() - ftMd.ctime] / (60*60*24*1000))
let totalDays = " 您已使用 *Obsidian* "+total+" 天，"
let nofold = '!"misc/templates"'
let allFile = dv.pages(nofold).file
let totalMd = "共创建 "+
	allFile.length+" 篇笔记"
let totalTag = allFile.etags.distinct().length+" 个标签"

dv.paragraph(
	totalDays+totalMd+"、"+totalTag+""
)
```



## 学习任务
```dataview
list
from "03 备忘"
sort file.ctime desc
```



## Todo
- [ ] Virgo 外设对接 
- [ ] scpu flash 速度测试 case 不需要关心数据对错，所以直接测试整片 flash 即可，目前测试只测了 2 个 block，数据不准确 



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

[^1]: 
