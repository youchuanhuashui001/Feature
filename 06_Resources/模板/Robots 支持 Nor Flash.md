# Flash 信息
- Name：
- JEDEC_ID：
- OTP：
- UID：
- Write_Protect：
- Quad：
- Quad：
	- [ ] 0x05：Read SR1
	- [ ] 0x35：Read SR2
	- [ ] 0x01：Read SR1+SR2
	- [ ] 0x31：Write SR2
- 最高频率：


# Apus
```
* 补丁：{{patch()}}
|/7.ZB25LQ16A |4,2,1倍速读写擦测试||
|xip读写测试||
|写保护测试||
|uid读取||
|单页多次写测试||
|掉电300次测试||
|休眠唤醒测试||

---

* 不同频率下的 sample_delay 范围测试：
| core电压 | 实测VDD09电压(V) | 时钟源pll | cpu分频系数 | cpu频率 | xip时钟源分频系数 | xip内部分频 | xip工作频率 | xip sample delay范围值 | xip sample delay建议值 | 备注                                   |
| 1.0       |                 | 440       | 4           | 110     | 2                 | 2           | 110         |                        |                        |    无可用相位                          |
| 1.1       |                 | 160       | 1           | 160     | 1                 | 2           | 80          |      3, 4, 5           |         4              |    读写擦测试通过                      |
| 1.1       |                 | 144       | 1           | 144     | 1                 | 2           | 72          |      3, 4, 5           |         4              |    读写擦测试通过                      |
| 1.1       |                 | 128       | 1           | 128     | 1                 | 2           | 64          |      2, 3, 4, 5        |         3              |    读写擦测试通过                      |
| 1.0       |                 | 96        | 1           | 96      | 1                 | 2           | 48          |         2, 3, 4        |         3              |    读写擦测试通过                      |
| 0.95      |                 | 64        | 1           | 64      | 1                 | 2           | 32          |         1, 2, 3        |         2              |    左侧为 1.0v 测试情况，读写擦测试通过|
| 0.9       |                 | 32        | 1           | 32      | 1                 | 2           | 16          |      0, 1, 2           |         1              |    读写擦测试通过                      |


* 注：此处的相位使用 apus socket 板测得，仅供参考；具体客户板子的相位需要重新在客户板子上测试。




* 手册描述掉电至 xxx v 后保持 xxx us 即可完成复位

* 手册描述最高工作频率为：xxx MHz
```

- [ ] 手册上传至云盘
- [ ] 更新支持列表： https://git.nationalchip.com/redmine/projects/robot_os/wiki/FLASH%E6%94%AF%E6%8C%81%E5%88%97%E8%A1%A8



# Leo/Leo_mini/grus
```
* scpu 补丁：{{patch()}}
* vsp 补丁：{{patch()}}



|测试项|测试结果|
|4,2倍速读写擦测试||
|xip测试||
|写保护测试||
|uid读取||
|单页多次写测试||
|掉电300次测试||


* 相位测试：
** 测试条件：读写擦测试 1000 次不出错
** 硬件：leo_mini socket 板
|模块频率|模块分频|工作频率|相位范围|
|200M   |4     |50M    |1、2、3|
|150M   |2     |75M    |1、2|
|150M   |4     |37.5M  |0、1、2、3 |

```

- [ ] 手册上传至云盘
- [ ] 更新支持列表： https://git.nationalchip.com/redmine/projects/robot_os/wiki/FLASH%E6%94%AF%E6%8C%81%E5%88%97%E8%A1%A8



# Flash 支持时需要用到的某些补丁：

- Apus 测试 flash 跑极限频率代码修改
	- https://git.nationalchip.com/gerrit/#/c/112577
- Apus 测试 10ms 休眠唤醒
	- `scpu` 中使用 ` apus_nre_xip_1v_deconfig ` 编译，打开 ` apus pmu test ` 测试选项
	- `main.c` 中将 ` pmu_reboot_for_10ms_test ` 函数 `extern` 过来，并在 `app_start` 之前调用
- leo_mini flash 跑 130Mhz，时钟代码修改
	- https://git.nationalchip.com/gerrit/#/c/123165/
- fornax 使用 psram spi 对接 flash 测试
	- https://git.nationalchip.com/gerrit/#/c/124025/2
