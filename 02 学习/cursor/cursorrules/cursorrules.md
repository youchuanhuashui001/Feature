
大家好，今天和大家分享一下 cursor 的 cursorrules 功能，在分享之前，我先说一下我在使用 cursor 的过程中遇到的一些问题：

目前使用 cursor 中遇到的一些问题：
- 有时候我贴给 cursor 一些英文信息，它就会直接用英文来回答，但我其实想它一直用中文回答
- 让它帮我生成代码的时候，生成的代码格式和我现有代码的格式不一样
- 给出的代码技术栈不是我想要的
像上面的这些情况，我都需要重新和它对话，来让它按照我的要求重新给出答案

那么什么是 cursorrules 呢？
- 用来预设提示词，我们可以把刚刚的那些都整理成提示词放到 .cursorrules 文件
	- 始终用中文回答
	- 生成代码时需要和现有的代码格式保持一致

- 这些都是我们自己特有的一些需求，目前还有很多开源的 cursorrules

- 打开之后看一下，翻译看一下里面写的是什么
	- 放到 cursor 中作为 .cursorrules
	- 因为模型里面其实训练了各种各样的代码仓库，你给了这样的限定条件之后就让 claud 在生成代码的时候更适合你自己了

目前好像还有新的基于项目的 rules

## 1：使用 cursorrules
打开提示词网站
开源的提示词，配置成某种角色，目前就配置成了用 python 的专家，主要用哪几个库来完成功能

上面这些都是开源的提示词，如果不满足你自己的要求，你可以和 ai 对话，让它帮你修改你的 cursorrules。

## 2. Think-Claud
介绍一个我目前常用的 cursorrules，也是一个开源的，功能是：让 claud 在回答问题之前先自己思考一下，并给出思考过程。我们在看思考过程的时候就可以知道 claud 有没有正确的了解我们提的需求。
对比前后的交互效果，同样是写一个贪吃蛇小游戏






















```
- 每次都用中文回复

- 生成的代码格式需要和现有的保持一致

- 每次回答之前都先分析我真正的需求是什么，并反馈给我
```




https://github.com/richards199999/Thinking-Claude/blob/main/model_instructions/v5.1-extensive-20241201.md




# Cursorrules 使用分享案例

一. 案例说明：  
分享 Cursor 的 Cursorrules 使用方法，主要内容如下：  
1. 介绍 cursorrules 的使用方法
2. 使用开源的 cursorrules 限定 cursor 的角色
3. 使用 Think-claud 作为 cursorrules 开发贪吃蛇小游戏的
二. 开发难点：  
不熟悉 Python 语言以及开发 GUI 需要用到的 taga 库。  
三. 和 Chat、Composer 开发以及传统开发对比：  
4. 传统开发需要学习 Python 基础语法、toga API 使用方法。  
5. Chat、Composer 需手动将实际运行时的报错、现象反馈回去；Agent 能够自行运行代码、记录运行时的报错、迭代代码。  
3. **只需一次对话，就可以生成满足需求的代码。**



### CursorRules 功能开发案例

**一. 案例说明：**  
本视频展示了如何使用 CursorRules 定制 AI 功能，主要内容如下：
1. 介绍 CursorRules 的基本功能和配置方法。
2. 通过集成开源的 CursorRules，将 Cursor AI 限定为特定角色。
3. 结合 Think-claud 优化 Cursor AI 的交互体验。

**二. 开发难点：**  
不熟悉 CursorRules 的具体配置方式以及如何集成开源规则。

**三. 与传统配置和直接使用 AI 对比：**

1. 传统配置需要手动编写所有规则，耗时长且容易出错。
2. 直接使用 AI 难以精确控制其行为，无法完全满足特定需求。
3. 使用 CursorRules 只需简单配置和集成开源规则，就能快速定制 AI 行为，大幅提升开发效率。




# CursorRules 功能分享案例

一. 案例说明：
	展示如何使用 CursorRules 作为预设提示词定制 AI 功能，主要内容如下：
	1. 介绍 CursorRules 的基本功能和配置方法。
	2. 集成开源的 CursorRules，将 Cursor 限定为特定角色。
	3. 使用 Thinking-claud 优化 Cursor 的交互体验。
二. 开发难点：
	不熟悉 CursorRules 的具体配置方式以及如何集成开源规则。
三. 和传统交互方式对比：
	4. 直接与 Cursor 交互时会使用默认规则，当与我们的要求不一致时，需要再次对话让 Cursor 重新输出，耗时长且容易出错。
	5. 直接与 Cursor 交互难以精确控制其行为，无法完全满足特定需求。
	6. 使用 CursorRules 只需简单配置和集成开源规则，就能快速定制 AI 行为，大幅提升开发效率。
