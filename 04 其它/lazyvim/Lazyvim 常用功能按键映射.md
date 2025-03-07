# Lazyvim 快捷键记录 


## 复制到系统剪切板：
- 进入 Visual 模式，选中要复制的内容，按下 `y` 

## 窗口切换：
- `Ctrl h j k l`

## 禅宗模式：
- `<Space> u z`

## 全局搜索文件：替换 fd 和 Ctrl+p
- `<Space> <Space>`


## 全局搜索：Grep
- `<Space> /` ：当前代码的根目录搜索 


## 切换当前的 buffer 到下一个或上一个：
- `Shift h/l`

## 当前打开的 buffer：`\\`
- `<Space> f b`：显示所有当前打开的 buffer
- `<Ctrl> b e`：目录树的方式显示当前所有打开的 buffer


## 打开终端：`<Space> 1`
- 方法 1：`<Space> f t`：home 目录
	- `<Space> f t`：cwd目录
- 方法 2：`<Ctrl> /`
	- 建议使用：可以打开终端，也可以隐藏终端



## 搜索和替换
- `<Space> s r`


## Git：git diff
- `<Space> g g`


## 打开函数列表：
`Ctrl o`


## 高亮单词：
-  `<Space> m`：高亮/取消高亮 
- `<Space> m c`：取消所有高亮 



## 使用 compiledb 生成 compile_commands. json
```shell
compiledb -n make -f /path/to/Makefile # 不编译，仅仅生成compile_commands.json compiledb make -f /path/to/Makefile #真的去编译
```



# todo:

## [mason.nvim  梅森.nvim](https://github.com/williamboman/mason.nvim.git)[​](https://www.lazyvim.org/keymaps#masonnvim "Direct link to masonnvim")

| Key  钥匙      | Description  描述 | Mode  模式 |     |
| ------------ | --------------- | -------- | --- |
| `<leader>cm` | Mason  石匠       | **n**    | j   |

| Key  钥匙      | Description  描述                           | Mode  模式 |
| ------------ | ----------------------------------------- | -------- |
| `<leader>st` | Todo  托多                                  | **n**    |
| `<leader>sT` | Todo/Fix/Fixme  待办事项/修复/修复我               | **n**    |
| `<leader>xt` | Todo (Trouble)  都都（麻烦）                    | **n**    |
| `<leader>xT` | Todo/Fix/Fixme (Trouble)  Todo/修复/修复我（问题） | **n**    |
| `[t`         | Previous Todo Comment  之前的 Todo 评论        | **n**    |
| `]t`         | Next Todo Comment  下一个待办事项评论              | **n**    |
