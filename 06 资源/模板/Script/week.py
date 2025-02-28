import os
import re
from datetime import datetime, timedelta
import glob

class DailyNoteParser:
    def __init__(self):
        self.tasks = []
        self.progress = []
        self.problems = []
        self.tech_summary = []
        self.knowledge = []
        self.tomorrow_plan = []
        self.extensions = []
        self.date = None
    
    def parse_file(self, file_path):
        """解析单个日记文件"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # 提取日期（从文件名或文件内容）
            filename = os.path.basename(file_path)
            date_match = re.search(r'(\d{4}-\d{2}-\d{2})', filename)
            if date_match:
                self.date = datetime.strptime(date_match.group(1), '%Y-%m-%d')
            
            # 提取各个部分内容
            # 任务部分
            tasks_match = re.search(r'# \*\*🔧 今日工作安排\*\*(.*?)(?=# \*\*📌|$)', content, re.DOTALL)
            if tasks_match:
                self.tasks = self._clean_and_extract_items(tasks_match.group(1))
            
            # 任务进展
            progress_match = re.search(r'# \*\*📌 任务进展\*\*(.*?)(?=# \*\*⚠️|$)', content, re.DOTALL)
            if progress_match:
                self.progress = self._clean_and_extract_items(progress_match.group(1))
            
            # 问题与解决
            problems_match = re.search(r'# \*\*⚠️ 问题与解决\*\*(.*?)(?=# \*\*💡|$)', content, re.DOTALL)
            if problems_match:
                self.problems = self._clean_and_extract_items(problems_match.group(1))
            
            # 技术总结
            tech_match = re.search(r'# \*\*💡 技术总结\*\*(.*?)(?=# \*\*📚|$)', content, re.DOTALL)
            if tech_match:
                self.tech_summary = self._clean_and_extract_items(tech_match.group(1))
            
            # 知识关联
            knowledge_match = re.search(r'# \*\*📚 知识关联\*\*(.*?)(?=# \*\*📌 明日|$)', content, re.DOTALL)
            if knowledge_match:
                self.knowledge = self._clean_and_extract_items(knowledge_match.group(1))
            
            # 明日计划
            plan_match = re.search(r'# \*\*📌 明日计划\*\*(.*?)(?=# \*\*💬|$)', content, re.DOTALL)
            if plan_match:
                self.tomorrow_plan = self._clean_and_extract_items(plan_match.group(1))
            
            # 扩展记录
            ext_match = re.search(r'# \*\*💬 扩展记录\*\*(.*?)(?=$)', content, re.DOTALL)
            if ext_match:
                self.extensions = self._clean_and_extract_items(ext_match.group(1))
            
            return True
        except Exception as e:
            print(f"解析文件 {file_path} 时出错: {e}")
            return False
    
    def _clean_and_extract_items(self, text):
        """清理文本并提取列表项"""
        if not text.strip():
            return []
        
        # 去除空行
        lines = [line.strip() for line in text.split('\n') if line.strip()]
        
        # 处理列表项
        items = []
        current_item = ""
        
        for line in lines:
            if line.startswith('-') or line.startswith('1.') or line.startswith('2.') or line.startswith('*'):
                if current_item:
                    items.append(current_item.strip())
                current_item = line
            else:
                current_item += " " + line if current_item else line
        
        if current_item:
            items.append(current_item.strip())
        
        return items if items else [text.strip()]

class WeeklySummaryGenerator:
    def __init__(self, daily_notes_path, output_path):
        """
        初始化周小结生成器
        
        参数:
        daily_notes_path: 日记文件所在目录
        output_path: 输出的周小结保存路径
        """
        self.daily_notes_path = daily_notes_path
        self.output_path = output_path
        self.daily_notes = []
    
    def get_week_days(self, date=None):
        """获取指定日期所在周的所有日期"""
        if date is None:
            date = datetime.now()
        
        # 计算本周的起始日期 (周一)
        start_of_week = date - timedelta(days=date.weekday())
        
        # 生成本周的所有日期 (周一至周日)
        return [start_of_week + timedelta(days=i) for i in range(7)]
    
    def find_daily_notes(self, week_dates):
        """查找指定日期范围内的日记文件"""
        all_files = glob.glob(os.path.join(self.daily_notes_path, "*.md"))
        weekly_files = []
        
        date_formats = ['%Y-%m-%d', '%Y%m%d', '%d-%m-%Y', '%d%m%Y']
        
        for file_path in all_files:
            filename = os.path.basename(file_path)
            file_date = None
            
            # 尝试从文件名中提取日期
            for date_format in date_formats:
                try:
                    date_match = re.search(r'(\d{2,4}[-_]?\d{2}[-_]?\d{2,4})', filename)
                    if date_match:
                        potential_date = date_match.group(1).replace('_', '-')
                        try:
                            file_date = datetime.strptime(potential_date, date_format)
                            break
                        except ValueError:
                            continue
                except Exception:
                    continue
            
            # 如果找到日期且在本周范围内
            if file_date and any(file_date.date() == d.date() for d in week_dates):
                weekly_files.append(file_path)
        
        return weekly_files
    
    def process(self, specific_date=None):
        """处理并生成周小结"""
        # 获取本周日期
        week_dates = self.get_week_days(specific_date)
        week_start = week_dates[0].strftime('%Y-%m-%d')
        week_end = week_dates[-1].strftime('%Y-%m-%d')
        
        # 查找本周的日记文件
        daily_note_files = self.find_daily_notes(week_dates)
        if not daily_note_files:
            print(f"未找到 {week_start} 至 {week_end} 期间的日记文件")
            return False
        
        # 解析每个日记文件
        for file_path in daily_note_files:
            parser = DailyNoteParser()
            if parser.parse_file(file_path):
                self.daily_notes.append(parser)
        
        if not self.daily_notes:
            print("未能成功解析任何日记文件")
            return False
        
        # 生成周小结
        self._generate_summary(week_start, week_end)
        return True
    
    def _generate_summary(self, week_start, week_end):
        """生成周小结内容并保存"""
        output_file = os.path.join(self.output_path, f"周小结_{week_start}_至_{week_end}.md")
        
        # 整合所有任务
        all_tasks = []
        for note in self.daily_notes:
            all_tasks.extend(note.tasks)
        unique_tasks = list(set(all_tasks))
        
        # 整合所有进展
        all_progress = []
        for note in self.daily_notes:
            all_progress.extend(note.progress)
        
        # 整合所有问题与解决
        all_problems = []
        for note in self.daily_notes:
            all_problems.extend(note.problems)
        
        # 整合所有技术总结
        all_tech = []
        for note in self.daily_notes:
            all_tech.extend(note.tech_summary)
        
        # 整合所有知识关联
        all_knowledge = []
        for note in self.daily_notes:
            all_knowledge.extend(note.knowledge)
        
        # 获取最后一天的明日计划作为下周计划
        next_week_plan = []
        if self.daily_notes:
            # 按日期排序，取最新的明日计划
            sorted_notes = sorted(self.daily_notes, key=lambda x: x.date if x.date else datetime.min)
            next_week_plan = sorted_notes[-1].tomorrow_plan
        
        # 写入文件
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(f"# 工作周小结 ({week_start} 至 {week_end})\n\n")
            
            # 写入本周主要任务
            f.write("## 本周主要任务\n\n")
            for i, task in enumerate(unique_tasks, 1):
                f.write(f"{task}\n")
            f.write("\n")
            f.write("\n")
            f.write("\n")
            
            # 写入任务进展
            f.write("## 任务进展\n\n")
            for progress in all_progress:
                f.write(f"- {progress}\n")
            f.write("\n")
            
            # 写入问题与解决
            f.write("## 问题与解决\n\n")
            for problem in all_problems:
                if problem.strip():
                    f.write(f"- {problem}\n")
            f.write("\n")
            
            # 写入技术总结
            f.write("## 技术总结\n\n")
            for tech in all_tech:
                if tech.strip():
                    f.write(f"- {tech}\n")
            f.write("\n")
            
            # 写入知识关联
            f.write("## 知识关联\n\n")
            for knowledge in all_knowledge:
                if knowledge.strip():
                    f.write(f"- {knowledge}\n")
            f.write("\n")
            
            # 写入下周计划
            f.write("## 下周计划\n\n")
            for plan in next_week_plan:
                if plan.strip():
                    f.write(f"- {plan}\n")
            f.write("\n")
            
            print(f"周小结已生成: {output_file}")

if __name__ == "__main__":
    # 使用示例
    daily_notes_path = "../../../00 日记/2025/2025-2/"  # 日记文件存放目录
    output_path = "."     # 周小结输出目录
    
    # 确保输出目录存在
    os.makedirs(output_path, exist_ok=True)
    
    # 生成本周的周小结
    generator = WeeklySummaryGenerator(daily_notes_path, output_path)
    generator.process()
