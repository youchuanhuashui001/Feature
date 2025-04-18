
- id
	- 0x5e4018
- uid
	- 128bit
- wp
- quad
	- 0x01：可以最多连续写 3 个字节
- xip:
	- bb：4 dummy cycles  104MHz
	- eb：6 dummy cycles 104MHz
	- 实际测试 0xbb 不需要 dummy，0xeb 需要 4 个 dummy

```
* 补丁：{{patch(124235)}}
|/6.ZB25LQ16A |4,2,1倍速读写擦测试|ok|
|xip读写测试|ok|
|写保护测试|ok|
|uid读取|ok|
|单页多次写测试|ok|
|掉电300次测试||
|休眠唤醒测试||

---

* 不同频率下的 sample_delay 范围测试：
| core电压 | 实测VDD09电压(V) | 时钟源pll | cpu分频系数 | cpu频率 | xip时钟源分频系数 | xip内部分频 | xip工作频率 | xip sample delay范围值 | xip sample delay建议值 | 备注                                   |
| 1.0       |                 | 440       | 4           | 110     | 2                 | 2           | 110         |                        |                        |    无可用相位                          |
| 1.1       |                 | 160       | 1           | 160     | 1                 | 2           | 80          |      3, 4, 5           |         4              |    读写擦测试通过                      |
| 1.1       |                 | 144       | 1           | 144     | 1                 | 2           | 72          |      3, 4, 5           |         4              |    读写擦测试通过                      |
| 1.1       |                 | 128       | 1           | 128     | 1                 | 2           | 64          |      2, 3, 4       |         3              |    读写擦测试通过                      |
| 1.0       |                 | 96        | 1           | 96      | 1                 | 2           | 48          |         2, 3, 4        |         3              |    读写擦测试通过                      |
| 0.95      |                 | 64        | 1           | 64      | 1                 | 2           | 32          |         0, 1, 2, 3        |         1              |    左侧为 1.0v 测试情况，读写擦测试通过|
| 0.9       |                 | 32        | 1           | 32      | 1                 | 2           | 16          |      0, 1, 2           |         1              |    读写擦测试通过                      |


* 注：此处的相位使用 apus socket 板测得，仅供参考；具体客户板子的相位需要重新在客户板子上测试。




* 手册描述掉电至 xxx v 后保持 xxx us 即可完成复位

* 手册描述最高工作频率为：xxx MHz
```

- [x] 手册上传至云盘
- [x] 更新支持列表： https://git.nationalchip.com/redmine/projects/robot_os/wiki/FLASH%E6%94%AF%E6%8C%81%E5%88%97%E8%A1%A8


