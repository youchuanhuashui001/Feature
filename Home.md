---
banner: "![[06 资源/file/wallhaven-l85vk2_1920x1080.png]]"
banner_y: 0.472
banner_x:
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


---


## 学习任务
- [[02 学习/schedule/学习计划|学习计划]]



---

## 工作安排
```dataview
TASK
FROM "00 日记/2025/工作安排.md"
WHERE !completed
```


---



# 最近编辑
```dataview
table WITHOUT ID file.link AS "标题",file.mtime as "时间"
from !"10 归档" and !"1 看板"
sort file.mtime desc
limit 5
```

---

# 最近创建
```dataview
table WITHOUT ID file.link AS "标题",file.ctime as "时间"
from !"10 归档" and !"1 看板"
sort file.ctime desc
limit 5
```


---

# 日记

```dataview
table WITHOUT ID file.link AS "日记", dateformat(file.ctime,"DD") AS "创建时间", dateformat(file.mtime,"DD  HH:mm:ss") AS "最近修改时间"
from "00 日记" 
where dateformat(file.ctime,"MM-dd") = dateformat(date(today),"MM-dd") 
limit 100
```

[^1]: 
