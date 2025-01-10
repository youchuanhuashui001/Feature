---

mindmap-plugin: basic

---

# ahb dma

## cmd
- init 已经在  common/main.c 中调了
- 配置全部都是用 GX_DMA_AHB_CH_CONFIG 来做的，也就是 drv-xxx 中定义的
- 调用的接口都是 drv-xxx 中实现的

## drv-apus
- .h
    - 基本和 hal .h 中实现了一套相同的宏定义，这里是给上层用的，上层不能直接用 hal，需要用 drv-xxx 这套里面的东西
- .c
    - 定义一个 GX_HAL_DMA_AHB dma_dev
    - Debug 相关的代码
    - init
        - 定义一个 GX_HAL_DMA_AHB_INIT_CFG init_cfg = {0};
        - 打开模块时钟
        - 初始化 init_cfg
        - 调用 gx_hal_dw_dma_ahb_init(&dma_dev, &init_cfg)
        - gx_request_irq(IRQ_NUM_DMA, gx_dma_handler, &dma_dev);
            - gx_hal_dw_dma_ahb_irq_handler(pdata);
    - 这里面主要实现的是一些会提供给上层的接口，例如 init、select_channel、release_channel、xfer、wait_complete
        - 这里的接口对应的功能也都是调用 hal 来实现的

## hal
- .h
    - 定义 GX_HAL_DMA_AHB、GX_HAL_DMA_AHB_INIT_CFG、GX_HAL_DMA_AHB_CONFIG
- .c
    - 实现了所有的功能函数，函数的参数都是 GX_HAL_DMA_AHB *dev, GX_HAL_DMA_AHB_INIT_CFG *cfg 形式
    - 函数全都是传一个 dev + 参数