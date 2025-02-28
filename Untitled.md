
# 周小结（ {{DATE:YYYY-MM-DD}} 至 {{DATE+6d:YYYY-MM-DD}} ）

## 本周核心任务
```dataview
LIST WITHOUT ID task
FROM "00 日记/2025/2025-2"
WHERE file.name = this.file.name OR file.name LIKE "2025-02-%"
FLATTEN file.tasks.text AS task
WHERE meta(task.section).subpath = "🔧 今日核心任务"
AND file.day >= date({{DATE:YYYY-MM-DD}}) AND file.day <= date({{DATE+6d:YYYY-MM-DD}})
```

