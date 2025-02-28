## 本周日记（ {{date:YYYY-MM-DD}} ）

<%*
const startDate = tp.date.now("YYYY-MM-DD", -7); // 获取7天前日期
const endDate = tp.date.now("YYYY-MM-DD");
-%>

### 日期范围：<%= startDate %> 至 <%= endDate %>

<%*
const files = app.vault.getFiles().filter(file => 
  file.path.startsWith("00 日记/2025/2025-2") &&  // 替换路径
  file.name >= startDate && 
  file.name <= endDate
);
files.sort((a, b) => a.name.localeCompare(b.name));
-%>

<% files.forEach(file => { -%>
- [[<%= file.name.replace('.md', '') %>]]
<% }) %>

