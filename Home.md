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



## 学习任务
- [[02 学习/schedule/学习计划|学习计划]]



## Todo
- [ ] cursor
	- [ ] [[02 学习/cursor/cursorrules/cursorrules|cursorrules]]
	- [ ] [[agent]]
- [ ] virgo
	- [ ] 外设对接
	- [ ] rom spinor 驱动优化，用双倍速。spinand 驱动整理
	- [ ] flash spi 要加 ddr 模式
- [ ] sagitta
	- [ ] 给 dma 用的 buffer 需要 aligned cache_line，否则会出现 dma buffer 和其它数据共用同一个 cache line 的问题
	- [ ] 做 dma 操作之前会先将 dma buffer clean & invalid 到内存，但是和它共用一条 cache line 的后面的代码会访问这个 cache line，导致把这里 dma buffer 又读到了 cache；等到 dma 操作完成后，cache line 的数据和内存的数据(dma 搬过去的数据)又不一样了；如果此时 invalid 就会丢失栈的其它部分，因为共用一条 cache line 的后面的代码会用这里来存东西，可能会被破坏，导致跑飞



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
