```
# 修改指令前缀

set -g prefix C-w #

unbind C-w # C-b 即 Ctrl+b 键，unbind 意味着解除绑定

bind C-w send-prefix # 绑定 Ctrl+f 为新的指令前缀

unbind %

bind | split-window -h # 使用|竖屏，方便分屏

unbind '"'

bind - split-window -v # 使用-横屏，方便分屏

#bind -r k select-pane -U # 绑定k为↑
#
#bind -r j select-pane -D # 绑定j为↓
#
#bind -r h select-pane -L # 绑定h为←
#
#bind -r l select-pane -R # 绑定l为→
# 直接使用 Ctrl + hjkl 切换窗格，不需要前缀键
bind -n C-h select-pane -L
bind -n C-j select-pane -D
bind -n C-k select-pane -U
bind -n C-l select-pane -R

set -g default-terminal "xterm-256color" # 保持 vim 配色

set-option -g mouse on # 使能鼠标滚动
```