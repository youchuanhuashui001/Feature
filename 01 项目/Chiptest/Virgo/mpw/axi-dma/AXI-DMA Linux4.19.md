- 默认统一使用链表的形式传输

- memcpy
	- set_desc_for_master(hw_desc, DMA_MEM_TO_MEM);
- slave_sg
	- dw_axi_dma_set_hw_desc
- cyclic
	- dw_axi_dma_set_hw_desc
- llp --> master0



- channel：8
- dma_if：40

- 使能通道
	- CHENREG
- 配置寄存器
	- CHx_CFG2

- commit：495e18b
```
495e18b
```

- reg_map_8_channels = true
- use_cfg2 = true

- config_write
	- cfg2
- chan_disable
	- CHENREG
- chan_enable
	- CHENREG