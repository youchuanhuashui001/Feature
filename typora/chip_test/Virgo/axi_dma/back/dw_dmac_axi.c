#include <dma.h>
#include <driver/gx_dma_axi.h>
#include "dw_dmac_axi.h"
#include "interrupt.h"


//static DW_DMAC_CH_CONFIG channel_config[DW_DMAC_MAX_CH];

#define DMA_CHANNEL_LLP_SIZE     384    // (16*1024*1024/(4*1024*1024-1)+1)*sizeof(dw_lli_t)+DCACHE_LINE_SIZE
#define DCACHE_LINE_SIZE         64

#define IRQ_NUM_DMA  91
#define DMA_BLOCK_TS 0x3FFFFF

#ifndef roundup
#define roundup(x, y)     ((((x) + ((y) - 1)) / (y)) * (y))
#endif

typedef struct {
	u8             chan_used[DW_DMAC_MAX_CH];
	u8             llp_buf[DW_DMAC_MAX_CH][DMA_CHANNEL_LLP_SIZE];
	dw_lli_t      *lli[DW_DMAC_MAX_CH];
} DW_DMAC_PRIV_T;

static DW_DMAC_PRIV_T dw_dmac_axi_priv_var;
static DW_DMAC_PRIV_T *priv = &dw_dmac_axi_priv_var;

static void (*dw_dma_callback[DW_DMAC_MAX_CH])(void *);
static void *dw_dma_callback_param[DW_DMAC_MAX_CH];

static int is_debug = 0;

void dw_list_set(dw_lli_t *lli, dw_lli_t *head, int32_t count, int len, u8 wide)
{
	dw_lli_t *plli = head;
	unsigned int index[2] = { DMA_BLOCK_TS, 0};
	unsigned long long sar_add  = index[0x01 & ((lli)->ctl_lo >> 4)] * (wide);
	unsigned long long dar_add  = index[0x01 & ((lli)->ctl_lo >> 6)] * (wide);
	int32_t i = 0;
	while(i < count-1){
		plli->sar    = (lli)->sar + sar_add * i;
		plli->dar    = (lli)->dar + dar_add * i;
		plli->ctl_lo = (lli)->ctl_lo;
		plli->ctl_hi = (lli)->ctl_hi;
		plli->block_ts_lo = DMA_BLOCK_TS - 1;
		plli->llp    = (unsigned long long)((uint32_t)(plli + 1) - GX_DRAM_VIRTUAL_OFFSET);
		++plli; ++i;
	}
	plli->sar         = (lli)->sar + sar_add * i;
	plli->dar         = (lli)->dar + dar_add * i;
	plli->ctl_lo      = (lli)->ctl_lo;
	plli->ctl_hi      = (lli)->ctl_hi | (1 << 30);
	plli->block_ts_lo = len % DMA_BLOCK_TS? (len % DMA_BLOCK_TS - 1) : (DMA_BLOCK_TS - 1);
	plli->llp         = 0;
}

static inline unsigned long long dma_read_reg(unsigned int reg)
{
	unsigned long long value;
	value = *((volatile unsigned long long *)(reg));
	return value;
}

static inline void dma_write_reg(unsigned int reg, unsigned long long value)
{
	*((volatile unsigned long long *)(reg)) = value;
	return ;
}

static inline int dma_set_bit_value(unsigned long long *reg_value, int bit_start,
	unsigned long long mask,unsigned long set_value)
{
	unsigned long long temp=0;
	unsigned long long value = (unsigned long long)set_value;
	*reg_value &= ~mask;
	if((value<<bit_start) > mask) {
		printf("error value\n");
		return -1;
	}
	temp = (value<<bit_start) & mask;
	*reg_value |= temp;
	return 0;
}

#if 0
static void dw_chan_clean(int ch)
{
	//CHAN_CLEAN(&priv->dw_dmac->CLEAR, |=,  0x01  << ch );
	//CHAN_CLEAN(&priv->dw_dmac->RAW,   &=, ~(0x01 << ch));
	//CHAN_CLEAN(&priv->dw_dmac->MASK,  |=,  0x101 << ch);
}


static void display_interrupt(int dma_channel)
{
	printf("channel %d isr int status en is %llx\n",dma_channel, dma_read_reg(DMA_CHXINTSTATUSEN(dma_channel)));
	printf("channel %d isr int status is %llx\n",dma_channel, dma_read_reg(DMA_CHXINTSTATUS(dma_channel)));
	printf("channel %d isr int signal en is %llx\n",dma_channel, dma_read_reg(DMA_CHXINTSIGNALEN(dma_channel)));
	printf("channel %d isr int clear is %llx\n",dma_channel, dma_read_reg(DMA_CHXINTCLEAR(dma_channel)));	
}
#endif

static void dma_reset(void)
{
	unsigned long long temp;
	dma_write_reg(DMA_RESETREG, 0x01);
	while(1) {
		temp = dma_read_reg(DMA_RESETREG);
		if(0 == temp)
			break;
	}
}

static void display_regs(int dma_channel)
{
	printf("DMA_CHENREG is %llx\n", dma_read_reg(DMA_CHENREG));
	printf("DMA_CHXINTSTATUS is %llx\n", dma_read_reg(DMA_CHXINTSTATUS(dma_channel)));
	printf("DMA_CHXCTL is %llx\n",dma_read_reg(DMA_CHXCTL(dma_channel)));
	printf("DMA_CHXCFG reg is %llx\n",dma_read_reg(DMA_CHXCFG(dma_channel)));
	printf("DMA_CHENREG reg is %llx\n",dma_read_reg(DMA_CHENREG));
	printf("DMA_CHXSAR is %llx\n",dma_read_reg(DMA_CHXSAR(dma_channel)));
	printf("DMA_CHXDAR is %llx\n",dma_read_reg(DMA_CHXDAR(dma_channel)));
}

static void dw_dma_enable(void)
{
	unsigned long long val;

	val = dma_read_reg(DMA_CFGREG);
	val |= (INT_EN_MASK|DMAC_EN_MASK);
	dma_write_reg(DMA_CFGREG, val);
}

#if 0
static void dw_dma_disable(void)
{
	unsigned long long val;

	val = dma_read_reg(DMA_CFGREG);
	val &= ~INT_EN_MASK;
	dma_write_reg(DMA_CFGREG, val);
}
#endif

static void dw_dma_channel_enable(int dma_channel)
{
	unsigned long long reg_value;

	reg_value = dma_read_reg(DMA_CHENREG);
#if DW_DMAC_MAX_CH <= 8
	reg_value |= ((1<<DMAC_CHAN_EN_SHIFT) |(1<<DMAC_CHAN_EN_WE_SHIFT)) << dma_channel;
#elif (DW_DMAC_MAX_CH > 8) && (DW_DMAC_MAX_CH <= 16)
	reg_value |= ((1<<DMAC_CHAN_EN_SHIFT) |(1<<DMAC_CHAN16_EN_WE_SHIFT)) << dma_channel;
#endif
	dma_write_reg(DMA_CHENREG, reg_value);
}

static void dw_dma_channel_suspend(int dma_channel)
{
	unsigned long long reg_value;

	reg_value = dma_read_reg(DMA_CHENREG);
	reg_value |= ((1<<DMAC_CHAN_SUSP_SHIFT) |(1<<DMAC_CHAN_SUSP_WE_SHIFT)) << dma_channel;
	dma_write_reg(DMA_CHENREG, reg_value);

}

static void dw_dma_channel_disable(int dma_channel)
{
	unsigned long long reg_value;
	reg_value = dma_read_reg(DMA_CHENREG);
#if DW_DMAC_MAX_CH <= 8
	reg_value &= ~((1<<DMAC_CHAN_EN_SHIFT) << dma_channel);
	reg_value |= ((1<<DMAC_CHAN_EN_WE_SHIFT) << dma_channel);
#elif (DW_DMAC_MAX_CH > 8) && (DW_DMAC_MAX_CH <= 16)
	reg_value &= ~((1<<DMAC_CHAN_EN_SHIFT) << dma_channel);
	reg_value |= ((1<<DMAC_CHAN16_EN_WE_SHIFT) << dma_channel);
#endif

	dma_write_reg(DMA_CHENREG, reg_value);
}


static void dw_dma_enable_int(int dma_channel)
{
	unsigned long long reg_value;

	reg_value = dma_read_reg(DMA_CHXINTSIGNALEN(dma_channel));
	reg_value = DWAXIDMAC_IRQ_DMA_TRF;
	dma_write_reg(DMA_CHXINTSIGNALEN(dma_channel),reg_value);
}

static void dw_dma_disable_int(int dma_channel)
{
	unsigned long long reg_value;

	reg_value = dma_read_reg(DMA_CHXINTSIGNALEN(dma_channel));
	reg_value &= ~(DWAXIDMAC_IRQ_DMA_TRF);
	dma_write_reg(DMA_CHXINTSIGNALEN(dma_channel),reg_value);
}

static void dw_dma_disable_all_int(int dma_channel)
{
	unsigned long long reg_value;

	reg_value = dma_read_reg(DMA_CHXINTSIGNALEN(dma_channel));
	reg_value &= ~(DWAXIDMAC_IRQ_ALL);
	dma_write_reg(DMA_CHXINTSIGNALEN(dma_channel),reg_value);
}

static void dw_dma_enable_status(int dma_channel)
{
	unsigned long long reg_value;

	reg_value = dma_read_reg(DMA_CHXINTSTATUSEN(dma_channel));
	reg_value = DWAXIDMAC_IRQ_DMA_TRF | DWAXIDMAC_IRQ_SUSPENDED | DWAXIDMAC_IRQ_DISABLED;
	reg_value = DWAXIDMAC_IRQ_ALL;
	dma_write_reg(DMA_CHXINTSTATUSEN(dma_channel),reg_value);
}

static void dw_dma_clear_int(int dma_channel)
{
	unsigned long long reg_value;

	reg_value = DWAXIDMAC_IRQ_ALL;
	dma_write_reg(DMA_CHXINTCLEAR(dma_channel),reg_value);
}

static enum interrupt_return dw_dma_complete(uint32_t irq, void *pdata)
{
	int channel;
	int status;

	for (channel=0; channel<DW_DMAC_MAX_CH; channel++) {
		status = dma_read_reg(DMA_CHXINTSTATUS(channel));
		if(0 != status) {
			//printf("channel %d isr int status is %llx\n",channel, status);
			dw_dma_clear_int(channel);
			dw_dma_disable_int(channel);
			if (dw_dma_callback[channel])
				dw_dma_callback[channel](dw_dma_callback_param[channel]);
		}
	}
	//printf("channel %d dma_complete int\n",channel);
	return HANDLED;
}

int dw_dma_select_channel(void)
{
	int i = 0;
	uint32_t irq_state;

	for(i=0;i<DW_DMAC_MAX_CH;i++) {
		if (priv->chan_used[i] == 0) {
			priv->chan_used[i] = 1;
			return i;
		}
	}
	printf("no enough channel\n");
	return -1;
}

void dw_dma_release_channel(int channel)
{
	uint32_t irq_state;

	priv->chan_used[channel] = 0;
}

void dw_dma_register_complete_callback(int channel, void *func, void *param)
{
	dw_dma_callback[channel] = func;
	dw_dma_callback_param[channel] = param;
	return;
}

int dw_dma_channel_config(int dma_channel, DW_DMAC_CH_CONFIG *config)
{
	unsigned long long reg_value;
	int ret = 0;

	/////////////////////////////////////////////////////////////
	//now setting CHXCTL REG
	reg_value = dma_read_reg(DMA_CHXCTL(dma_channel));

	//set msize
	ret = dma_set_bit_value(&reg_value,CH_CTL_L_DST_MSIZE_POS,CH_CTL_L_DST_MSIZE_MASK,config->dst_msize);
	ret |= dma_set_bit_value(&reg_value,CH_CTL_L_SRC_MSIZE_POS,CH_CTL_L_SRC_MSIZE_MASK,config->src_msize);
	//set tr width
	ret |= dma_set_bit_value(&reg_value,CH_CTL_L_DST_WIDTH_POS,CH_CTL_L_DST_WIDTH_MASK,config->dst_trans_width);
	ret |= dma_set_bit_value(&reg_value,CH_CTL_L_SRC_WIDTH_POS,CH_CTL_L_SRC_WIDTH_MASK,config->src_trans_width);

	//set dst/src addr increase
	if(DWAXIDMAC_CH_CTL_L_INC == config->dst_addr_update) {
		ret |= dma_set_bit_value(&reg_value,CH_CTL_L_DST_INC_POS,CH_CTL_L_DST_INC_POS_MASK,0x0);
	} else if(DWAXIDMAC_CH_CTL_L_NOINC == config->dst_addr_update) {
		ret |= dma_set_bit_value(&reg_value,CH_CTL_L_DST_INC_POS,CH_CTL_L_DST_INC_POS_MASK,0x1);
	} else {
		printf("error addr dst inc param\n");
		ret |= -1;
	}
	if(DWAXIDMAC_CH_CTL_L_INC == config->src_addr_update) {
		ret |= dma_set_bit_value(&reg_value,CH_CTL_L_SRC_INC_POS,CH_CTL_L_SRC_INC_POS_MASK,0x0);
	} else if(DWAXIDMAC_CH_CTL_L_NOINC == config->src_addr_update) {
		ret |= dma_set_bit_value(&reg_value,CH_CTL_L_SRC_INC_POS,CH_CTL_L_SRC_INC_POS_MASK,0x1);
	} else {
		printf("error addr src inc param\n");
		ret |= -1;
	}

	//AXI master select //mem2mem is 0x1 0x01
	if(AXI_MASTER_1 == config->dst_master_select) {
		ret |= dma_set_bit_value(&reg_value,CH_CTL_L_DST,CH_CTL_L_DST_MASK,0x0);
	} else if(AXI_MASTER_2 == config->dst_master_select) {
		ret |= dma_set_bit_value(&reg_value,CH_CTL_L_DST,CH_CTL_L_DST_MASK,0x1);
	} else {
		printf("error dst mater param\n");
	}
	if(AXI_MASTER_1 == config->src_master_select) {
		ret |= dma_set_bit_value(&reg_value,CH_CTL_L_SRC,CH_CTL_L_SRC_MASK,0x0);
	} else if(AXI_MASTER_2 == config->src_master_select) {
		ret |= dma_set_bit_value(&reg_value,CH_CTL_L_SRC,CH_CTL_L_SRC_MASK,0x1);
	} else {
		printf("error src master param\n");
	}
	dma_write_reg(DMA_CHXCTL(dma_channel),reg_value);

	////////////////////////////////////////////////////////////////
	//now setting CHXCFG REG
	reg_value = dma_read_reg(DMA_CHXCFG(dma_channel));
	//set dir and dataflow
	ret |= dma_set_bit_value(&reg_value,CH_CFG_L_TT_FC_POS,CH_CFG_L_TT_FC_MASK,config->flow_ctrl);

	//set src/dst handshaking mode.
	//hardware or software
	if(DWAXIDMAC_HS_SEL_HW == config->dst_hs_select) {
		ret |= dma_set_bit_value(&reg_value,CH_CFG_L_HS_SEL_DST_POS,CH_CFG_L_HS_SEL_DST_POS_MASK,0x0);
	} else if(DWAXIDMAC_HS_SEL_SW == config->dst_hs_select) {
		ret |= dma_set_bit_value(&reg_value,CH_CFG_L_HS_SEL_DST_POS,CH_CFG_L_HS_SEL_DST_POS_MASK,0x1);
	} else {
		printf("error dst handshake mode\n");
		ret |= -1;
	}
	if(DWAXIDMAC_HS_SEL_HW == config->src_hs_select) {
		ret |= dma_set_bit_value(&reg_value,CH_CFG_L_HS_SEL_SRC_POS,CH_CFG_L_HS_SEL_SRC_POS_MASK,0x0);
	} else if(DWAXIDMAC_HS_SEL_SW == config->src_hs_select) {
		ret |= dma_set_bit_value(&reg_value,CH_CFG_L_HS_SEL_SRC_POS,CH_CFG_L_HS_SEL_SRC_POS_MASK,0x1);
	} else {
		printf("error src handshake mode\n");
		ret |= -1;
	}
	//set dst/src per
#if (DW_DMAC_MAX_CH <= 8) && (DW_DMAC_NUM_HS_IF <= 16)
	ret |= dma_set_bit_value(&reg_value,CH_CFG_L_DST_PER_POS,CH_CFG_L_DST_PER_MASK,config->dst_per);
	ret |= dma_set_bit_value(&reg_value,CH_CFG_L_SRC_PER_POS,CH_CFG_L_SRC_PER_MASK,config->src_per);
#elif (DW_DMAC_MAX_CH >= 1) && ((DW_DMAC_MAX_CH > 16) || (DW_DMAC_NUM_HS_IF > 16))
	ret |= dma_set_bit_value(&reg_value,CH_CFG2_L_DST_PER_POS,CH_CFG2_L_DST_PER_MASK,config->dst_per);
	ret |= dma_set_bit_value(&reg_value,CH_CFG2_L_SRC_PER_POS,CH_CFG2_L_SRC_PER_MASK,config->src_per);
#endif

	if (DWAXIDMAC_MBLK_TYPE_LL == config->mblk_type) {
		ret |= dma_set_bit_value(&reg_value,CH_CFG_L_SRC_MULTBLK_TYPE_POS,CH_CFG_L_SRC_MULTBLK_TYPE_MASK,config->mblk_type);
		ret |= dma_set_bit_value(&reg_value,CH_CFG_L_DST_MULTBLK_TYPE_POS,CH_CFG_L_DST_MULTBLK_TYPE_MASK,config->mblk_type);
	} else {
		ret |= dma_set_bit_value(&reg_value,CH_CFG_L_SRC_MULTBLK_TYPE_POS,CH_CFG_L_SRC_MULTBLK_TYPE_MASK,0x0);
		ret |= dma_set_bit_value(&reg_value,CH_CFG_L_DST_MULTBLK_TYPE_POS,CH_CFG_L_DST_MULTBLK_TYPE_MASK,0x0);
	}

	dma_write_reg(DMA_CHXCFG(dma_channel),reg_value);

	return ret;
}

int dw_dma_xfer(void *dst, void *src, int len, int dma_channel)
{
	unsigned long long reg_value;

	dw_dma_enable();
	dw_dma_clear_int(dma_channel);
	dw_dma_enable_status(dma_channel);

	//set src/dst address
	reg_value = (unsigned long long)((uint32_t)src - GX_DRAM_VIRTUAL_OFFSET);
	dma_write_reg(DMA_CHXSAR(dma_channel),reg_value);

	reg_value = (unsigned long long)((uint32_t)dst - GX_DRAM_VIRTUAL_OFFSET);
	dma_write_reg(DMA_CHXDAR(dma_channel),reg_value);

	//set data len
	reg_value = (unsigned long long)(len-1);
	dma_write_reg(DMA_CHXBLKTS(dma_channel),reg_value);

	if(1 == is_debug)
		display_regs(dma_channel);

	dcache_flush();
	dw_dma_channel_enable(dma_channel);

	return 0;
}

int dw_dma_xfer_for_lli(void *dst, void *src, int len, int dma_channel)
{
	int32_t lli_cnt;
	dw_lli_t lli;
	unsigned long long ctl = dma_read_reg(DMA_CHXCTL(dma_channel));

	dw_dma_enable();
	dw_dma_clear_int(dma_channel);
	dw_dma_enable_status(dma_channel);

	lli.sar    = (unsigned long long)((uint32_t)src - GX_DRAM_VIRTUAL_OFFSET);
	lli.dar    = (unsigned long long)((uint32_t)dst - GX_DRAM_VIRTUAL_OFFSET);
	lli.ctl_lo = (unsigned int)(ctl & 0xFFFFFFFF);
	lli.ctl_hi = (unsigned int)((ctl >> 32) & 0xFFFFFFFF | 0x80000000);

	lli_cnt = len / DMA_BLOCK_TS + !!(len % DMA_BLOCK_TS);

	// TODO: for any length. max length 16MB
	if ((lli_cnt * sizeof(dw_lli_t)) > (DMA_CHANNEL_LLP_SIZE-DCACHE_LINE_SIZE))
		return -1;

	dw_list_set(&lli, priv->lli[dma_channel], lli_cnt, len, 1 << ((ctl >> CH_CTL_L_SRC_WIDTH_POS) & 0x7));

	dma_write_reg(DMA_CHXLLP(dma_channel),(unsigned long long)((uint32_t)(priv->lli[dma_channel]) - GX_DRAM_VIRTUAL_OFFSET));

	if(1 == is_debug)
		display_regs(dma_channel);

	dcache_flush();
	dw_dma_channel_enable(dma_channel);

	return 0;
}
// the procedure(channel disable prior to transfer completion without suspend)
// follows the guide in the page 90 of the user manual
int dw_dmac_channel_abort(int dma_channel)
{
	unsigned long long reg_value;

	dw_dma_channel_suspend(dma_channel);

	//wait disable finished
	while(1) {
		reg_value = dma_read_reg(DMA_CHXINTSTATUS(dma_channel));
		if ((DWAXIDMAC_IRQ_SUSPENDED & reg_value) > 0)
			break;
	}

	//disable dma channel
	dw_dma_channel_disable(dma_channel);

	//wait disable finished
	while(1) {
		reg_value = dma_read_reg(DMA_CHXINTSTATUS(dma_channel));
		if ((DWAXIDMAC_IRQ_DISABLED & reg_value) > 0 )
			break;
	}

	dw_dma_clear_int(dma_channel);
	return 0;
}

int dw_dma_wait_complete_timeout(int dma_channel, unsigned int timeout_ms)
{
	unsigned long long reg_value;
	unsigned int start;
	start = gx_time_get_ms();
	while((gx_time_get_ms() - start) < timeout_ms) {
		reg_value = dma_read_reg(DMA_CHXINTSTATUS(dma_channel));
		reg_value &= (DWAXIDMAC_IRQ_DMA_TRF);
		if (DWAXIDMAC_IRQ_DMA_TRF == reg_value) {
			dw_dma_clear_int(dma_channel);
			dw_dma_release_channel(dma_channel);
			return 0;
		}
	}
	printf("dma timeout and abort\n");
	dw_dmac_channel_abort(dma_channel);
	return -1;
}


int dw_dma_wait_complete(int dma_channel)
{
	unsigned long long reg_value;

	while(1) {
		reg_value = dma_read_reg(DMA_CHXINTSTATUS(dma_channel));
		reg_value &= (DWAXIDMAC_IRQ_DMA_TRF);
		if (DWAXIDMAC_IRQ_DMA_TRF == reg_value) {
			break;
		}
	}

	dw_dma_clear_int(dma_channel);
	dw_dma_release_channel(dma_channel);
	return 0;
}


int dw_dma_xfer_poll(void *dst, void *src, int len, int dma_channel)
{
	dw_dma_xfer(dst,src,len,dma_channel);
	dw_dma_wait_complete(dma_channel);

	return 0;
}

int dw_dma_xfer_poll_for_lli(void *dst, void *src, int len, int dma_channel)
{
	dw_dma_xfer_for_lli(dst,src,len,dma_channel);
	dw_dma_wait_complete(dma_channel);

	return 0;
}

int dw_dma_xfer_int(void *dst, void *src, int len, int dma_channel)
{
	unsigned long long reg_value;

	//dw_dma_clear_int(dma_channel);
	dw_dma_enable_int(dma_channel);
	dw_dma_enable_status(dma_channel);

	//set src/dst address
	reg_value = (unsigned long long)((uint32_t)src - GX_DRAM_VIRTUAL_OFFSET);
	dma_write_reg(DMA_CHXSAR(dma_channel),reg_value);
	reg_value = (unsigned long long)((uint32_t)dst - GX_DRAM_VIRTUAL_OFFSET);
	dma_write_reg(DMA_CHXDAR(dma_channel),reg_value);

	//set data len
	reg_value = (unsigned long long)(len-1);
	dma_write_reg(DMA_CHXBLKTS(dma_channel),reg_value);

	if(1 == is_debug)
		display_regs(dma_channel);

	dw_dma_enable();
	dcache_flush();
	dw_dma_channel_enable(dma_channel);

	return 0;
}

int dw_dma_xfer_int_for_lli(void *dst, void *src, int len, int dma_channel)
{
	unsigned long long reg_value;
	int32_t lli_cnt;
	dw_lli_t lli;
	unsigned long long ctl = dma_read_reg(DMA_CHXCTL(dma_channel));

	//dw_dma_clear_int(dma_channel);
	dw_dma_enable_int(dma_channel);
	dw_dma_enable_status(dma_channel);

	lli.sar    = (unsigned long long)((uint32_t)src - GX_DRAM_VIRTUAL_OFFSET);
	lli.dar    = (unsigned long long)((uint32_t)dst - GX_DRAM_VIRTUAL_OFFSET);
	lli.ctl_lo = (unsigned int)(ctl & 0xFFFFFFFF);
	lli.ctl_hi = (unsigned int)((ctl >> 32) & 0xFFFFFFFF | 0x80000000);

	lli_cnt = len / DMA_BLOCK_TS + !!(len % DMA_BLOCK_TS);

	// TODO: for any length. max length 16MB
	if ((lli_cnt * sizeof(dw_lli_t)) > (DMA_CHANNEL_LLP_SIZE-DCACHE_LINE_SIZE))
		return -1;

	dw_list_set(&lli, priv->lli[dma_channel], lli_cnt, len, 1 << ((ctl >> CH_CTL_L_SRC_WIDTH_POS) & 0x7));

	dma_write_reg(DMA_CHXLLP(dma_channel),(unsigned long long)((uint32_t)(priv->lli[dma_channel]) - GX_DRAM_VIRTUAL_OFFSET));

	if(1 == is_debug)
		display_regs(dma_channel);

	dw_dma_enable();
	dcache_flush();
	dw_dma_channel_enable(dma_channel);

	return 0;
}

void dw_dmac_init(void)
{
	int i;

	dma_reset();

	memset(priv, 0, sizeof(DW_DMAC_PRIV_T));
	for (i = 0; i < DW_DMAC_MAX_CH; i++) {
		priv->lli[i] = (dw_lli_t *)roundup((unsigned int)priv->llp_buf[i], DCACHE_LINE_SIZE);
		priv->chan_used[i] = 0;
		dw_dma_disable_all_int(i);
	}

#ifdef CONFIG_ENABLE_IRQ
	gx_request_interrupt(IRQ_NUM_DMA, IRQ, dw_dma_complete, NULL);
#endif
	is_debug = 0;
}

void dw_dmac_debug(int debug)
{
	is_debug = debug;
}

