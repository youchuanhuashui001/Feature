
# **什么是 `.cursorrules` 文件？**

`.cursorrules` 文件是用于 Cursor AI 代码编辑器的一种配置文件，用于定义自定义规则，以指导 AI 生成代码的行为。通过定制这些规则，您可以使 AI 的行为更符合您的特定需求和偏好。

**为什么要使用 `.cursorrules` 文件？**

*   **定制 AI 行为：** 根据项目需求定制 AI 的响应，提供更相关和准确的代码建议。
*   **保持一致性：** 通过定义编码标准和最佳实践，确保 AI 生成的代码符合项目风格指南。
*   **上下文感知：** 为 AI 提供项目的重要背景信息，例如常用方法、架构决策或特定库，从而实现更明智的代码生成。
*   **提高生产力：** 通过明确定义的规则，AI 可以生成需要更少手动编辑的代码，从而加快开发过程。
*   **团队协调：** 在团队项目中，共享 `.cursorrules` 文件可确保所有团队成员获得一致的 AI 帮助，促进编码实践的凝聚力。
*   **项目特定知识：** 包含有关项目结构、依赖关系或独特要求的信息，帮助 AI 提供更准确、更相关的建议。

**如何使用 `.cursorrules` 文件？**

在项目的根目录中创建一个 `.cursorrules` 文件，并在其中定义您的自定义规则。这将使您能够利用上述优势，并增强您使用 Cursor AI 的编码体验。


**方法一：手动安装**

1.  **选择 `.cursorrules` 文件：** 浏览可用的 `.cursorrules` 文件，找到适合您项目需求的规则文件。
2.  **复制到项目根目录：** 将选定的 `.cursorrules` 文件复制到您的项目的根目录中。
3.  **自定义规则：** 根据您的特定项目要求，对 `.cursorrules` 文件中的规则进行自定义修改。

**方法二：使用 vscode-cursor-rules 扩展**

1.  **安装扩展：** 在 VS Code 中安装 `vscode-cursor-rules` 扩展。
2.  **添加 `.cursorrules` 文件：**
    *   打开命令面板（Cmd+Shift+P 或 Ctrl+Shift+P）。
    *   输入 "Cursor Rules: Add .cursorrules"。
    *   选择并下载适合您需要的 `.cursorrules` 文件。
3.  **自定义规则：** 根据您的特定项目要求，对 `.cursorrules` 文件中的规则进行自定义修改。


# 比较好用的 .cursorrules
- Thinking Claude:让克劳德在回答之前全面思考！ https://github.com/richards199999/Thinking-Claude/blob/main/model_instructions/v5.1-extensive-20241201.md  



# 添加 .cursorrules 前后对比：提示词统一为 (请你帮我写一个贪吃蛇小游戏)

## 添加前
![[Pasted image 20250304194018.png]]

我来帮你创建一个简单但完整的贪吃蛇游戏。我们将使用 Python 和 Pygame 库来实现这个游戏。首先，让我创建必要的文件。

首先，我们需要创建一个新的 Python 文件和 requirements. txt 文件。

```pip
requirements:requirements.txt
pygame==2.5.2
```


现在，让我们创建贪吃蛇游戏的主文件：

```python:snake_game.py
import pygame
import random
import sys

# 初始化Pygame
pygame.init()

# 定义颜色
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (255, 0, 0)
GREEN = (0, 255, 0)

# 游戏设置
WINDOW_WIDTH = 800
WINDOW_HEIGHT = 600
GRID_SIZE = 20
GRID_WIDTH = WINDOW_WIDTH // GRID_SIZE
GRID_HEIGHT = WINDOW_HEIGHT // GRID_SIZE
SNAKE_SPEED = 10

# 创建游戏窗口
screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
pygame.display.set_caption('贪吃蛇游戏')
clock = pygame.time.Clock()

class Snake:
	def __init__(self):
		self.length = 1
		self.positions = [(GRID_WIDTH // 2, GRID_HEIGHT // 2)]
		self.direction = random.choice([UP, DOWN, LEFT, RIGHT])
		self.color = GREEN
		self.score = 0

	def get_head_position(self):
		return self.positions[0]

	def update(self):
		cur = self.get_head_position()
		x, y = self.direction
		new = ((cur[0] + x) % GRID_WIDTH, (cur[1] + y) % GRID_HEIGHT)
		if new in self.positions[3:]:
			return False
		self.positions.insert(0, new)
		if len(self.positions) > self.length:
			self.positions.pop()
		return True

	def reset(self):
		self.length = 1
		self.positions = [(GRID_WIDTH // 2, GRID_HEIGHT // 2)]
		self.direction = random.choice([UP, DOWN, LEFT, RIGHT])
		self.score = 0

	def render(self):
		for p in self.positions:
			pygame.draw.rect(screen, self.color, 
				(p[0] * GRID_SIZE, p[1] * GRID_SIZE, GRID_SIZE, GRID_SIZE))

class Food:
	def __init__(self):
		self.position = (0, 0)
		self.color = RED
		self.randomize_position()

	def randomize_position(self):
		self.position = (random.randint(0, GRID_WIDTH-1), 
			random.randint(0, GRID_HEIGHT-1))

	def render(self):
		pygame.draw.rect(screen, self.color, 
			(self.position[0] * GRID_SIZE, 
			self.position[1] * GRID_SIZE, GRID_SIZE, GRID_SIZE))

# 定义方向
UP = (0, -1)
DOWN = (0, 1)
LEFT = (-1, 0)
RIGHT = (1, 0)

def main():
	snake = Snake()
	food = Food()
	font = pygame.font.Font(None, 36)

	while True:
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				pygame.quit()
				sys.exit()
			elif event.type == pygame.KEYDOWN:
				if event.key == pygame.K_UP and snake.direction != DOWN:
					snake.direction = UP
				elif event.key == pygame.K_DOWN and snake.direction != UP:
					snake.direction = DOWN
				elif event.key == pygame.K_LEFT and snake.direction != RIGHT:
					snake.direction = LEFT
				elif event.key == pygame.K_RIGHT and snake.direction != LEFT:
					snake.direction = RIGHT

		# 更新蛇的位置
		if not snake.update():
			snake.reset()
			food.randomize_position()

		# 检查是否吃到食物
		if snake.get_head_position() == food.position:
			snake.length += 1
			snake.score += 1
			food.randomize_position()

		# 绘制
		screen.fill(BLACK)
		snake.render()
		food.render()
		
		# 显示分数
		score_text = font.render(f'分数: {snake.score}', True, WHITE)
		screen.blit(score_text, (10, 10))

		pygame.display.update()
		clock.tick(SNAKE_SPEED)

if __name__ == '__main__':
	main()
```


让我们创建一个 README 文件来说明如何运行游戏：

```markdown:README.md
# 贪吃蛇游戏

一个使用Python和Pygame制作的简单贪吃蛇游戏。

## 安装依赖

```bash
pip install -r requirements.txt
```

## 运行游戏

```bash
python snake_game.py
```

## 游戏控制

- 使用方向键控制蛇的移动方向
- 吃到红色食物可以增加长度和分数
- 撞到自己会重新开始游戏

## 游戏特性

- 显示实时分数
- 穿墙效果
- 流畅的控制体验
- 随机生成食物
```


我已经创建了一个完整的贪吃蛇游戏！这个游戏包含以下特点：

1. 基本游戏功能：
   - 使用方向键控制蛇的移动
   - 吃到食物会增长并得分
   - 碰到自己会重新开始
   - 可以穿墙

2. 游戏界面：
   - 800x600像素的游戏窗口
   - 黑色背景
   - 绿色的蛇
   - 红色的食物
   - 左上角显示分数

3. 文件结构：
   - `snake_game.py`: 主游戏代码
   - `requirements.txt`: 项目依赖
   - `README.md`: 使用说明

要开始游戏，请按照以下步骤操作：

1. 首先安装依赖：
```bash
pip install -r requirements.txt
```

1. 运行游戏：
```bash
python snake_game.py
```

游戏控制说明：
- ↑ 向上移动
- ↓ 向下移动
- ← 向左移动
- → 向右移动

如果你想要调整游戏难度，可以修改代码中的 `SNAKE_SPEED` 变量（默认值为 10）。数值越大，蛇移动得越快，游戏难度越高。

需要我为你解释代码的具体实现或者修改任何游戏参数吗？


## 添加后
请你帮我写一个贪吃蛇小游戏


![[Pasted image 20250304193048.png]]


```thinking
好的,让我来思考如何实现一个贪吃蛇游戏。这是一个经典的小游戏,需要考虑几个关键要素:

首先是游戏的基本组成部分:
1. 蛇本身 - 需要用一个数据结构来表示蛇的身体,每个部分的位置
2. 食物 - 随机生成在地图上的点
3. 地图边界 - 需要定义游戏区域的大小
4. 移动控制 - 需要处理键盘输入来控制蛇的移动方向

我觉得可以使用Python的pygame库来实现,它提供了很好的游戏开发功能。让我想想具体的实现细节:

蛇的表示方式,我可以用一个列表来存储蛇身体的每个部分的坐标。蛇头在列表的第一个位置,移动时整条蛇跟随蛇头移动。

食物可以用random模块随机生成坐标。但要注意不能生成在蛇身上。

对于碰撞检测,需要检查:
1. 蛇头是否撞到墙
2. 蛇头是否撞到自己的身体
3. 蛇头是否吃到食物

移动逻辑:
1. 获取键盘输入改变方向
2. 蛇头按当前方向移动一格
3. 如果吃到食物,蛇身增长
4. 如果没吃到食物,蛇尾移除

我需要创建几个主要的文件:
1. 主游戏文件
2. requirements.txt 声明依赖
3. README.md 说明游戏玩法

让我先创建这些基础文件...
```

让我先创建一个新的游戏文件:

```python:snake_game.py
import pygame
import random
import sys

# 初始化pygame
pygame.init()

# 定义颜色
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (255, 0, 0)
GREEN = (0, 255, 0)

# 定义游戏参数
BLOCK_SIZE = 20  # 每个方块的大小
WINDOW_WIDTH = 800
WINDOW_HEIGHT = 600
FPS = 10  # 游戏帧率

# 创建游戏窗口
screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
pygame.display.set_caption('贪吃蛇')
clock = pygame.time.Clock()

class Snake:
    def __init__(self):
        self.length = 1
        self.positions = [(WINDOW_WIDTH//2, WINDOW_HEIGHT//2)]  # 蛇的初始位置
        self.direction = random.choice([UP, DOWN, LEFT, RIGHT])  # 随机初始方向
        self.color = GREEN
        self.score = 0

    def get_head_position(self):
        return self.positions[0]

    def update(self):
        cur = self.get_head_position()
        x, y = self.direction
        new = ((cur[0] + (x*BLOCK_SIZE)) % WINDOW_WIDTH, (cur[1] + (y*BLOCK_SIZE)) % WINDOW_HEIGHT)
        if new in self.positions[3:]:  # 检查是否撞到自己
            return False
        self.positions.insert(0, new)
        if len(self.positions) > self.length:
            self.positions.pop()
        return True

    def reset(self):
        self.length = 1
        self.positions = [(WINDOW_WIDTH//2, WINDOW_HEIGHT//2)]
        self.direction = random.choice([UP, DOWN, LEFT, RIGHT])
        self.score = 0

    def render(self):
        for p in self.positions:
            pygame.draw.rect(screen, self.color, (p[0], p[1], BLOCK_SIZE, BLOCK_SIZE))

class Food:
    def __init__(self):
        self.position = (0, 0)
        self.color = RED
        self.randomize_position()

    def randomize_position(self):
        self.position = (random.randint(0, (WINDOW_WIDTH-BLOCK_SIZE)//BLOCK_SIZE) * BLOCK_SIZE,
                        random.randint(0, (WINDOW_HEIGHT-BLOCK_SIZE)//BLOCK_SIZE) * BLOCK_SIZE)

    def render(self):
        pygame.draw.rect(screen, self.color, (self.position[0], self.position[1], BLOCK_SIZE, BLOCK_SIZE))

# 定义方向
UP = (0, -1)
DOWN = (0, 1)
LEFT = (-1, 0)
RIGHT = (1, 0)

def main():
    snake = Snake()
    food = Food()
    running = True

    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP and snake.direction != DOWN:
                    snake.direction = UP
                elif event.key == pygame.K_DOWN and snake.direction != UP:
                    snake.direction = DOWN
                elif event.key == pygame.K_LEFT and snake.direction != RIGHT:
                    snake.direction = LEFT
                elif event.key == pygame.K_RIGHT and snake.direction != LEFT:
                    snake.direction = RIGHT

        # 更新蛇的位置
        if not snake.update():
            snake.reset()
            food.randomize_position()

        # 检查是否吃到食物
        if snake.get_head_position() == food.position:
            snake.length += 1
            snake.score += 1
            food.randomize_position()

        # 绘制
        screen.fill(BLACK)
        snake.render()
        food.render()
        
        # 显示分数
        font = pygame.font.Font(None, 36)
        score_text = font.render(f'分数: {snake.score}', True, WHITE)
        screen.blit(score_text, (10, 10))
        
        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()
    sys.exit()

if __name__ == '__main__':
    main()
```


```thinking
很好,我已经创建了游戏的主要代码文件。现在让我创建requirements.txt来声明依赖:
```

```pip requirements:requirements.txt
pygame==2.5.2
```


```thinking
最后,我需要创建一个README文件来说明游戏的玩法和安装步骤:
```

```markdown:README.md
# 贪吃蛇游戏

一个使用Python和Pygame实现的经典贪吃蛇游戏。

## 游戏特点

- 简单直观的操作方式
- 实时显示得分
- 碰撞检测（撞墙或撞到自己会重新开始）
- 流畅的游戏体验

## 安装步骤

1. 确保你已安装Python 3.6或更高版本
2. 克隆或下载此仓库
3. 安装依赖:
   ```bash
   pip install -r requirements.txt
   ```

## 如何运行

在终端中运行:
```bash
python snake_game.py
```

## 游戏控制

- 使用方向键控制蛇的移动方向
- 按ESC键退出游戏
- 吃到食物(红色方块)可以增加长度和分数
- 撞到自己会重新开始游戏

## 游戏规则

1. 控制蛇移动收集食物
2. 每吃到一个食物得1分
3. 撞到自己会重新开始
4. 穿过边界会从对面出现

祝你玩得开心！
```


```thinking
好的,我已经创建了所有必要的文件。让我总结一下这个贪吃蛇游戏的主要特点:

1. 游戏界面:
   - 800x600像素的游戏窗口
   - 黑色背景
   - 绿色的蛇身
   - 红色的食物
   - 白色的分数显示

2. 游戏机制:
   - 使用方向键控制蛇的移动
   - 蛇可以穿过边界
   - 吃到食物会增加长度和分数
   - 撞到自己会重新开始
   - 实时显示当前分数

3. 技术实现:
   - 使用Pygame库实现图形界面
   - Snake类管理蛇的状态和行为
   - Food类处理食物的生成和显示
   - 主循环处理游戏逻辑和渲染

这个实现包含了贪吃蛇游戏的所有基本元素,玩家可以通过简单的安装步骤开始游戏。让我向用户解释如何开始玩这个游戏...
```

我已经为您创建了一个完整的贪吃蛇游戏！这个游戏包含了以下文件：

1. `snake_game.py` - 游戏主程序
2. `requirements.txt` - 依赖文件
3. `README.md` - 游戏说明文档

要开始玩游戏，请按照以下步骤操作：

1. 首先确保您的电脑已安装Python 3.6或更高版本
2. 安装游戏依赖：
   ```bash
   pip install -r requirements.txt
   ```
3. 运行游戏：
   ```bash
   python snake_game.py
   ```

游戏控制方式：
- 使用方向键(↑↓←→)控制蛇的移动
- 吃到红色食物可以增加分数和蛇的长度
- 如果撞到自己会自动重新开始
- 蛇可以穿过边界从另一边出现

游戏特色：
- 实时显示得分
- 流畅的游戏体验
- 简单直观的操作
- 经典的贪吃蛇玩法

祝您玩得开心！如果您有任何问题或需要帮助，请随时告诉我。


# 总结
- 让 claud 思考起来，并“说出”内心的思考过程，也能更加清楚 claud 的编程思路
- 使用 `.cursorrules` 文件可以在提交提示词时添加许多附加信息