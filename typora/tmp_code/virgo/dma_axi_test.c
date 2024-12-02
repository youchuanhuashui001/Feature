// 用于修改 axi dma 测试程序，comtest 校验数据
#include <driver/dma.h>
#include <driver/gx_dma_axi.h>
#include "dw_dmac_axi.h"

static DW_DMAC_CH_CONFIG axi_dma_config;
static int dma_ready=0;

static void dma_test_complele(void* pdata)
{
	dw_dma_release_channel((uint32_t)pdata);
	dma_ready = 1;
	printf("dma int transfer finish\n");
}

static int compare_data(uint8_t *src, uint8_t *dst, uint32_t len) {
	uint32_t i;
	for (i = 0; i < len; ++i) {
		if (src[i] != dst[i]) {
			printf("MEMORY COMPARE ERROR: src = 0x%x, dst = 0x%x, index = 0x%x\n",
				src[i], dst[i], i);
			return -1;
		}
	}
	return 0;
}

int axi_do_dma(uint8_t *src, uint8_t *dest, uint32_t len, uint32_t mode, uint32_t channel, uint32_t is_printf)
{
	uint8_t *temp;
	uint32_t temp_len = 0;
	int i;

	if(!src || !dest)
		return -1;

	printf("src addr : 0x%x, dest addr : 0x%x, len : %d KB, channel: %d\n",\
		(uint32_t)src, (uint32_t)dest, len >> 10,channel);

	temp = src;
	for(i=0;i<len;i++){
		*temp++ = i;
	}
	memset(dest,0, len);
	if (1 == is_printf) {
		temp = src;
		for(i=0;i<len;i++) {
			if(i%16 == 0)
				printf("\n");
			printf("0x%2x ", *temp++);
		}

		temp = dest;
		for(i=0;i<len;i++) {
			if(i%16 == 0)
				printf("\n");
			printf("0x%2x ", *temp++);
		}
		printf("\n");
	}

	switch(axi_dma_config.src_trans_width) {
		case DWAXIDMAC_TRANS_WIDTH_8:
			temp_len = len;
			break;
		case DWAXIDMAC_TRANS_WIDTH_16:
			temp_len = len/2;
			break;
		case DWAXIDMAC_TRANS_WIDTH_32:
			temp_len = len/4;
			break;
		case DWAXIDMAC_TRANS_WIDTH_64:
			temp_len = len/8;
			break;
		default:
			printf("src_trans_width error\n");
			return -1;
	}

	switch(mode) {
		case 0:
			axi_dma_config.flow_ctrl = 0x00;
			axi_dma_config.mblk_type = 0;
			dw_dma_channel_config(channel, &axi_dma_config);
			dw_dma_xfer_poll(dest,src,temp_len,channel);
			printf("memcpy block.\n");
			break;
		case 1:
			axi_dma_config.flow_ctrl = 0x00;
			axi_dma_config.mblk_type = DWAXIDMAC_MBLK_TYPE_LL;
			dw_dma_channel_config(channel, &axi_dma_config);
			dw_dma_xfer_poll_for_lli(dest,src,temp_len,channel);
			printf("memcpy link list.\n");
			break;
		case 2:
			dma_ready = 0;
			axi_dma_config.flow_ctrl = 0x00;
			axi_dma_config.mblk_type = 0;
			dw_dma_channel_config(channel, &axi_dma_config);
			dw_dma_register_complete_callback(channel,dma_test_complele,(void*)channel);
			dw_dma_xfer_int(dest,src,temp_len,channel);
			while(!dma_ready)
			{
				mdelay(50);
			}
			printf("memcpy block irq.\n");
			break;
		case 3:
			dma_ready = 0;
			axi_dma_config.flow_ctrl = 0x00;
			axi_dma_config.mblk_type = DWAXIDMAC_MBLK_TYPE_LL;
			dw_dma_channel_config(channel, &axi_dma_config);
			dw_dma_register_complete_callback(channel,dma_test_complele,(void*)channel);
			dw_dma_xfer_int_for_lli(dest,src,temp_len,channel);
			while(!dma_ready)
			{
				mdelay(50);
			}
			printf("memcpy link list irq.\n");
			break;
		default:
			printf("mode error\n");
			return -1;
	}

	if (1 == is_printf) {
		temp = dest;
		for(i=0;i<len;i++) {
			if(i%16 == 0)
				printf("\n");
			printf("0x%2x ", *temp++);
		}
		printf("\n");
	}

	for(i = 0; i < len; ++i) {
		if(*src != *dest) {
			printf("MEMORY COMPLE ERROR: src = %x, dest = %x, index = 0x%x\n",\
				(uint32_t)src, (uint32_t)dest, i);
			break;
		}
		++src;
		++dest;
	}

	return 0;
}

int axi_do_dma_config(DW_DMAC_CH_CONFIG dma_config)
{
	axi_dma_config.src_trans_width    = dma_config.src_trans_width;
	axi_dma_config.dst_trans_width    = dma_config.dst_trans_width;
	axi_dma_config.src_msize          = dma_config.src_msize;
	axi_dma_config.dst_msize          = dma_config.dst_msize;
	axi_dma_config.src_addr_update    = dma_config.src_addr_update;
	axi_dma_config.dst_addr_update    = dma_config.dst_addr_update;
	axi_dma_config.src_master_select  = dma_config.src_master_select;
	axi_dma_config.dst_master_select  = dma_config.dst_master_select;

	return 0;
}

int axi_do_memcpy_test(uint8_t *src, uint8_t *dst, uint32_t test_len, uint32_t channel, uint32_t test_count)
{
	int i;
	int start_time, end_time;
	int test_time_ms;
	int test_total_size = 0;

	test_total_size = test_len * test_count / 1024;

	switch(axi_dma_config.src_trans_width) {
		case DWAXIDMAC_TRANS_WIDTH_8:
			test_len = test_len;
			break;
		case DWAXIDMAC_TRANS_WIDTH_16:
			test_len = test_len/2;
			break;
		case DWAXIDMAC_TRANS_WIDTH_32:
			test_len = test_len/4;
			break;
		case DWAXIDMAC_TRANS_WIDTH_64:
			test_len = test_len/8;
			break;
		default:
			printf("src_trans_width error\n");
			return -1;
	}

	printf("[test env]\n");
	printf("test_src_addr = 0x%x\n", src);
	printf("test_dst_addr = 0x%x\n", dst);
	printf("test_len = %dB\n", test_len);
	printf("test_count = %dB\n", test_count);
	printf("test_total_size = %dKB\n", test_total_size);

	axi_dma_config.mblk_type = 0;
	dw_dma_channel_config(channel, &axi_dma_config);
	start_time = gx_time_get_ms();
	for (i = 0; i < test_count; i++)
		dw_dma_xfer_poll(dst,src,test_len,channel);
	end_time = gx_time_get_ms();

	printf("\n[test result]\n");
	test_time_ms = end_time - start_time;
	printf("test_time = %dms\n", test_time_ms);
	printf("test_speed = %dKB/s\n", test_total_size * 1000 / test_time_ms);

	return 0;
}

int axi_dma_comtest(void)
{
	uint8_t trans_width[] = {0, 1, 2};          ///< dma操作位宽 0:8bit  1:16bit  2:32bit  3:64bit
	uint8_t trans_width_index;

	uint8_t src_msize[] = {0, 1, 2, 3, 4, 5};   ///< dma 源通道burst 0:1 item  1:4 items   2:8 items   3: 16 items .... 7:256 items
	uint8_t src_msize_index;
	uint8_t dst_msize[] = {0, 1, 2, 3, 4, 5};   ///< dma 目的通道burst 0:1 item  1:4 items   2:8 items   3: 16 items .... 7:256 items
	uint8_t dst_msize_index;

	uint8_t src_addr_update = 0;                ///< dma 源通道地址更新方式 0:addr inc   1:addr no inc
	uint8_t dst_addr_update = 0;                ///< dma 目的通道地址更新方式 0:addr inc   1:addr no inc

	uint8_t src_master_select[] = {0};       ///< dma 源通道master选择 0:axi master 1   1: axi master 2
	uint8_t src_master_select_index = 0;
	uint8_t dst_master_select[] = {0};       ///< dma 目的通道master选择 0:axi master 1   1: axi master 2
	uint8_t dst_master_select_index = 0;

	uint32_t max_channel = DW_DMAC_MAX_CH - 1;   ///< dma 最大通道,例如：0~15, 设置成 15
	uint32_t channel_index;

	uint8_t dst_addr_index;
	uint8_t src_addr_index;

	uint32_t src_addr[] = {0x200000, 0x280000, 0x300000, 0x380000};
	uint32_t dst_addr[] = {0x400000, 0x480000, 0x500000, 0x580000};

	uint32_t test_len = 0x10000;
	int ret;

	uint8_t *tmp_data = NULL;
	int i;

	for(src_addr_index=0; src_addr_index < sizeof(src_addr)/sizeof(src_addr[0]); src_addr_index++) {
		for(dst_addr_index=0; dst_addr_index < sizeof(dst_addr)/sizeof(dst_addr[0]); dst_addr_index++) {
			for(src_master_select_index=0; src_master_select_index < sizeof(src_master_select)/sizeof(src_master_select[0]); src_master_select_index++) {
				for(dst_master_select_index=0; dst_master_select_index < sizeof(dst_master_select)/sizeof(dst_master_select[0]); dst_master_select_index++) {
					for(channel_index=0; channel_index <= max_channel; channel_index++) {
						for(src_msize_index=0; src_msize_index < sizeof(src_msize)/sizeof(src_msize[0]); src_msize_index++) {
							for(dst_msize_index=0; dst_msize_index < sizeof(dst_msize)/sizeof(dst_msize[0]); dst_msize_index++) {
								for(trans_width_index = 0; trans_width_index< sizeof(trans_width)/sizeof(trans_width[0]); trans_width_index++) {
									printf("[test env]\n");
									printf("src_addr = 0x%x\n", src_addr[src_addr_index]);
									printf("dst_addr = 0x%x\n", dst_addr[dst_addr_index]);
									printf("channel  = %d\n", channel_index);
									printf("len = %d B\n", test_len);
									printf("src_master = %d\n", src_master_select[src_master_select_index]);
									printf("dst_master = %d\n", dst_master_select[dst_master_select_index]);
									printf("trans_width = %d\n", trans_width[trans_width_index]);
									printf("src_msize = %d\n", src_msize[src_msize_index]);
									printf("dst_msize = %d\n", dst_msize[dst_msize_index]);
									axi_dma_config.src_trans_width  = trans_width[trans_width_index];
									axi_dma_config.dst_trans_width  = trans_width[trans_width_index];
									axi_dma_config.src_msize  = src_msize[src_msize_index];
									axi_dma_config.dst_msize  = dst_msize[dst_msize_index];
									axi_dma_config.src_addr_update  = src_addr_update;
									axi_dma_config.dst_addr_update  = dst_addr_update;
									axi_dma_config.src_master_select  = src_master_select[src_master_select_index];
									axi_dma_config.dst_master_select  = dst_master_select[dst_master_select_index];

									tmp_data = (uint8_t *)src_addr[src_addr_index];
									for (i = 0; i < test_len; i++) {
										*tmp_data = i;
									}

									memset((void *)dst_addr[dst_addr_index], 0, test_len);
									axi_dma_config.flow_ctrl = 0x00;
									axi_dma_config.mblk_type = 0;
									dw_dma_channel_config(channel_index, &axi_dma_config);
									dw_dma_xfer_poll((void *)dst_addr[dst_addr_index],(void *)src_addr[src_addr_index],test_len,channel_index);
									if (compare_data((uint8_t *)dst_addr[dst_addr_index], (uint8_t *)src_addr[src_addr_index], test_len) < 0)
										return -1;
									printf("dma poll memcpy finish.\n");

									memset((void *)dst_addr[dst_addr_index], 0, test_len);
									axi_dma_config.flow_ctrl = 0x00;
									axi_dma_config.mblk_type = DWAXIDMAC_MBLK_TYPE_LL;
									dw_dma_channel_config(channel_index, &axi_dma_config);
									dw_dma_xfer_poll_for_lli((void *)dst_addr[dst_addr_index],(void *)src_addr[src_addr_index],test_len,channel_index);
									if (compare_data((uint8_t *)dst_addr[dst_addr_index], (uint8_t *)src_addr[src_addr_index], test_len) < 0)
										return -1;
									printf("dma poll link list memcpy finish.\n");

#ifdef CONFIG_ENABLE_IRQ
									memset((void *)dst_addr[dst_addr_index], 0, test_len);
									dma_ready = 0;
									axi_dma_config.flow_ctrl = 0x00;
									axi_dma_config.mblk_type = 0;
									dw_dma_channel_config(channel_index, &axi_dma_config);
									dw_dma_register_complete_callback(channel_index,dma_test_complele,(void*)channel_index);
									dw_dma_xfer_int((void *)dst_addr[dst_addr_index],(void *)src_addr[src_addr_index],test_len,channel_index);
									while(!dma_ready)
									{
										mdelay(50);
									}
									if (compare_data((uint8_t *)dst_addr[dst_addr_index], (uint8_t *)src_addr[src_addr_index], test_len) < 0)
										return -1;
									printf("dma interrupt memcpy finish.\n");

									memset((void *)dst_addr[dst_addr_index], 0, test_len);
									dma_ready = 0;
									axi_dma_config.flow_ctrl = 0x00;
									axi_dma_config.mblk_type = DWAXIDMAC_MBLK_TYPE_LL;
									dw_dma_channel_config(channel_index, &axi_dma_config);
									dw_dma_register_complete_callback(channel_index,dma_test_complele,(void*)channel_index);
									dw_dma_xfer_int_for_lli((void *)dst_addr[dst_addr_index],(void *)src_addr[src_addr_index],test_len,channel_index);
									while(!dma_ready)
									{
										mdelay(50);
									}
									if (compare_data((uint8_t *)dst_addr[dst_addr_index], (uint8_t *)src_addr[src_addr_index], test_len) < 0)
										return -1;
									printf("dma interrupt link list memcpy finish.\n");
#endif
								}
							}
						}
					}
				}
			}
		}
	}
	printf("dma memcopy test success !\n");
	return 0;
}

