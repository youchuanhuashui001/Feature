
## leo_mini、leo、apus 获取 block_size 实际为 4K
- apus、leo_mini 等芯片的驱动在使用接口 `getinfo(BLOCK_SIZE)` 获取 block_size 时获取到的 block_size 为 4k，而不是 64k



## 各芯片 Flash 不同阶段工作频率

| Chip     | ROM  | Stage1                  | Stage2 | 备注                                                                        |
| -------- | ---- | ----------------------- | ------ | ------------------------------------------------------------------------- |
| Leo_mini | 3Mhz | 50Mhz<br>(读 ID : 25MHz) | 50MHz  | Stage1、Stage2 同频，可以配置为 200MHz、150MHz、100MHz。<br>目前配置为 200MHz，4分频，相位范围：1~4 |
