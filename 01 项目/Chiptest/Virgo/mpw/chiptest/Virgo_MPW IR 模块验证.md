---
tags:
  - Virgo_MPW模块验证
---
# 验证进度：
- 总验证点:: 6
- 已完成:: 6
- 备注:: 无




# 功能测试

### gxloader：irr_v1、irr_v2 测试通过

> 参考：[[Virgo_NRE IRR 模块验证]]


- gxloader：irr_v1、irr_v2 都测试 ok
	- .config 打开 irq，打开 irr
	- 去掉 bootmenu. c 中的 break


### linux：测试通过
- 首先同步代码，包括 gxtest、gxsecure、gxavdev.... 一大堆
- 使用 gxtest/stb/goxceed/cases/service/periphery/irr/cmd/ 目录下的测试 case
- 在 gxtest/stb/goxceed 目录下先打开 confs/def_config 文件中的 `USE_SHELL_CLI_SHELL=y` 选项
- `./build config -m irr`
- `./build arm linux virgo 3502 dvbs bin`
- 生成一个整 bin，将整 bin 烧到 flash，启动之后会启动测试 shell
- 输入命令：`irr` 则会提示对应的命令格式
- 能够正常读到键值
- **注 1：需要使用 cli shell，而不是 gxshell**
- **注 2：编译 gxtest 之前需要先将基础库都编译通过，建议使用 `./make.sh gxtest-chiptest arm linux virgo 3502 dvbs bin`**
- **注 3：若没成功启动，则手动执行 `dvb/out.elf`**



### ecos： 测试通过

- ecos：只需要测试 irr_v1 即可
- 先更新代码
- `./make. sh gxtest-chiptest arm linux virgo 3502 dvbs bin`
- 然后编译 gxtest 的 bin
- `./build arm ecos virgo 3502 dvbs bin`
- 与 linux 测试方式相同




# 修改点：
- IR
	- 驱动中的模块频率需要调成 198MHz
	- 管脚复用：C05



# 问题
## 【已解决】IRR_V2 无法获取键值
- 模块频率配置的是 198MHz，引脚复用也配了，IRR 可以正常用
- 这两个模块的时钟是同一个吗？引脚应该也是同一个？

- 烧到 flash 之后，再启动可以正常读取了

## 【已解决】C04、C05 两个引脚都可以复用成 IR，之前说的那个耐 5V？
- C05 可以正确读到键值
- C04 不行
![[Pasted image 20250701140328.png]]
- 只焊了电阻 R257，没有焊 R166，所以 C05 可以用
- C05 是耐 5v 的 IR GPIO，可以在 3.3v 工作，也可以在 5v 工作
- C04 是 3.3v 的 IR GPIO

