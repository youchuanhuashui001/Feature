# RTC 模块测试

## 1. 基本信息

- **Redmine 问题号**: `#408040`
- **芯片型号/项目名称**: `Virgo_NRE  Beta` 
- **被测模块**: `RTC`
- **固件/FPGA 镜像版本**:  
	- a7： #404698 vu9p_fpga_top_sub_a7_20250524.bit test7 a7核 flash中48M
	- a55: #404698 vu9p_fpga_top_sub_a55_20250524.bit test8 a55核 flash中48M
- **相关文档**: [[rtc ip]]

## 2. 测试目标
简要描述本次测试的主要目的和范围。
- 兼容 32 位、64 位驱动、测试代码
- RTC 模块验证

## 3. 测试环境

### 3.1 硬件环境

- **FPGA 型号**: `VU9P-05` 

### 3.2 软件环境
```
commit c8c3f8dcc3b0587775e99383adc177c5804cf808 (HEAD -> virgonre, refs/published/virgonre)
Author: tanxzh <tanxzh@nationalchip.com>
Date:   Wed Apr 16 20:20:38 2025 +0800

    406477: Virgo NRE Flash_SPI 模块验证
            1. 新增 ddr 模式
            2. 测试程序支持 64 位
    
    Change-Id: Iff4f3a0f3de42b5cfced0c780313511093660454

commit bbb3a47e7060674f40d776b6e05979adccd18671
Merge: bdc426ea 8444e4ca
Author: 刘非 <liufei@nationalchip.com>
Date:   Sun Apr 27 15:10:15 2025 +0800

    Merge "406515: gxloader 支持 nos lib 的编译" into develop2

commit 8444e4cabfddb2a9ce696f5f0b539cb29e8e745a
Author: yemb <yemb@nationalchip.com>
Date:   Wed Apr 16 16:13:28 2025 +0800

    406515: gxloader 支持 nos lib 的编译
    
    Change-Id: I2c9a76e8f50a46c8e4c67bfa18de5495d9c9f1e9

commit bdc426ea3806369fb99bc6fa4e4c190a7f1fc2bc
Author: yangmch <yangmch@nationalchip.com>
Date:   Fri Mar 14 16:26:33 2025 +0800

    398922: 添加gxloader 支持 eMMC 和 SD支持
    
    Change-Id: Ie240b97650bfbb1572bcaa7e3f0a3029207fb54a
```


## 4. 测试用例与结果

### 测试用例 1: 产生 1HZ 计数
- **测试命令：**
	- 打开 `.config` 中的 `ENABLE_RTC = y`
    - 输入命令：`rtc init`
    - 输入命令：`rtc read`
    - 输入命令：`rtc read`
- **实际结果**:
	- 1Hz 计数模式下，rtc 1s 计数一次，实际测试与配置一致
- **状态**: a7、a55 测试通过
- **备注**:

---

### 测试用例 2: 分频配置
- **测试命令**:
	- 打开 `.config` 中的 `ENABLE_RTC = y`
	- 手动修改 `gx_hal_init` 中的预分频值为 32768/4，则 1S 计数 4 次
    - 输入命令：`rtc init`
    - 输入命令：`rtc read`
    - 输入命令：`rtc read`
- **实际结果**:
    - 实际测试与配置一致
- **状态**: a7、a55 测试通过
- **备注**:
 

---

### 测试用例 3: 闹钟功能
- **测试命令**:
	- 闹钟功能依赖中断，因此需要开启中断 `CONFIG_ENABLE_IRQ = y， ENABLE_RTC = y`
	- `rtc init`
	- `rtc set_alarm 10`
- **实际结果**:
    - 10s 后触发一次中断，与配置一致。
    - 单次中断
- **状态**: a7、a55 测试通过


---



## 7. 问题与缺陷记录


### 【已解决】问题 1：预分频配置寄存器错误
- 本该配置 CPSR 寄存器，配成了 CPCVR (预分频寄存器当前计数值)，而由于 CPSR 寄存器的默认值是 32768，所以预分频 1HZ 可以正常产生，但无法产生其它分频
```diff
                // 每 1s rtc 计数器自增一次
-               WRITE_REG((uint32_t)dev->regs + RTC_CPCVR, dev->pclk);
+               //WRITE_REG((uint32_t)dev->regs + RTC_CPSR, dev->pclk);
```


### 【已解决】问题 2：RTC A55 平台下用 V11 JLINK 有时候无法读到寄存器的值
- 将频率降到 1000 也不行，降到 100 后可以了


### 【已解决】问题 3：RTC NRE A7、A55 不计数
- 初始化配置：
	- 读 0x0c(CCR) 寄存器，并或上 1<<4 写入
	- 将 32768 == 0x8000 写入寄存器 0x20 (RTC_CPSR)
	- 读 0x0c (CCR) 寄存器，并或上 1<<2 写入
- 读操作：
	- 读 0x0 (CCVR ) 寄存器，不会更新


- 用 final 的 bit 测试：ok
![[Pasted image 20250515160001.png]]


- 更新 bit ：
	- | [vu9p_fpga_top_sub_a7_20250524.bit](https://git.nationalchip.com/redmine/attachments/331123) (49.6 MB) [vu9p_fpga_top_sub_a7_20250524.bit](https://git.nationalchip.com/redmine/attachments/download/331123/vu9p_fpga_top_sub_a7_20250524.bit "下载")|test7 a7核 flash中48M|Y 余海建, 2025-05-26 09:12|[删除](https://git.nationalchip.com/redmine/attachments/331123 "删除")|
	- | [vu9p_fpga_top_sub_a55_20250524.bit](https://git.nationalchip.com/redmine/attachments/331124) (62.7 MB) [vu9p_fpga_top_sub_a55_20250524.bit](https://git.nationalchip.com/redmine/attachments/download/331124/vu9p_fpga_top_sub_a55_20250524.bit "下载")|test8 a55核 flash中48M|Y 余海建, 2025-05-26 09:14|
	- 测试 ok
![[485c1f1b36e78f038fb0e713785f9065.jpg]]





## 8. 测试总结与结论

- **测试覆盖率评估 (简述)**: A7 测试了所有功能，A55 除闹钟功能外都测试了
- **遗留问题**: 闹钟功能由于依赖中断，而 A55 中断基础环境还未准备好，所以 A55 闹钟功能未测
- **总体测试结果**: 通过

## 9. 附件

- `[[链接到Log文件]]`
- `[[链接到配置文件]]`
- `[[链接到逻辑分析仪捕获文件.sal 或 示波器截图]]`
- `[[链接到其他相关截图或数据]]`
```