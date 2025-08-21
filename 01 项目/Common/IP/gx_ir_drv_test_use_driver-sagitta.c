#include <common.h>
#include <command.h>
#include <cli_common.h>
#include <device.h>
#include <gx_delay.h>
#include <gx_clock.h>
#include <driver/board.h>
#include <gx_timer.h>
#include <gx_irq.h>
#include <soc.h>
#if defined(CONFIG_TARGET_APUS_NRE_FPGA_V1)
#include <gx_keyscan.h>
#endif

#include <gx_ir.h>

#define GX_IR_UD_DATA_TICK        100000 ///< ir clk / ud_div = 100KHz
#define GX_IR_UD_DATA_TICK_US     10 ///< 10 us
#define GX_IR_UD_CARR_TICK        1000000 ///< 1MHz
#define GX_IR_UD_CARR_TICK_US     1 ///< 1 us

typedef struct {
	uint8_t testsel;    /*!< 模拟接收测试通不通电压到 test ana，0：vpcas，1：vcom */
	uint8_t isel;       /*!< 模拟接收参考电流控制，0：1uA，1：0.5uA */
	uint8_t vcmsel;     /*!< 模拟接收参考电压档位，3bit 有效长度 */
	uint8_t drv_cap;    /*!< 发送驱动能力，4bit 有效长度，范围 0~16 */
	uint8_t vstset;     /*!< 模拟发射电路软启动控制，开启后减少过冲 */
	uint8_t testmod_en; /*!< 模拟发射电路测试模式，1：进入测试模式，发射管常开；0：关闭测试模式 */
}GX_IR_TEST_INIT_CFG_T;

GX_IR_CFG_T cfg;
static uint32_t gx_ir_context_src_freq = 0;
static uint32_t *g_data;                                     /*!< 异步发送数据指针 */
static uint32_t g_len;                                     /*!< 异步发送数据长度 */
static uint32_t g_count;                                   /*!< 异步发数据计数 */

#define GX_IR_CALC_DIV(src, clk)     (((src)*10/(clk)+5)/10)

/*灵活模式接收FIFO满中断回调函数*/
int gx_ir_test_ud_rx_full_cb(void* ir_dev)
{
	printf("rx fifo full\r\n");
	return 0;
}

/*灵活模式接收FIFO空中断回调函数*/
int gx_ir_test_ud_rx_empty_cb(void* ir_dev)
{
//	printf("rx fifo empty\r\n");
	return 0;
}

/*灵活模式发送FIFO到达阈值回调函数*/
int gx_ir_test_ud_tx_thr_cb(void* ir_dev)
{
	int spare = gx_ir_get_udtx_fifo_depth() - gx_ir_get_udtx_fifo_count();
	while((spare > 0) && (g_count < g_len))
	{
		gx_ir_udtx_fifo_put(g_data[g_count]);
		g_count++;
		spare--;
	}
	printf("tx thr\r\n");
	return 0;
}

/*灵活模式发送FIFO满回调函数*/
int gx_ir_test_ud_tx_full_cb(void* ir_dev)
{
	printf("tx fifo full\r\n");
	return 0;
}

/*灵活模式发送FIFO空回调函数*/
int gx_ir_test_ud_tx_empty_cb(void* ir_dev)
{
	if(g_count == g_len)
		gx_ir_udtx_write_endflag();
	printf("tx fifo empty\r\n");
	return 0;
}

static volatile uint8_t g_send_std_done = 0;
/*标准模式发送完成回调回调函数*/
int gx_ir_test_std_tx_done_cb(void* ir_dev)
{
	printf("std tx done\r\n");
	g_send_std_done = 1;
	return 0;
}

/* 接收完成回调函数 */
int gx_ir_test_recv_std_cb(void* ir_dev, unsigned int data, unsigned char type, unsigned char error)
{
	uint8_t repeat = 0;

	repeat = !!(type & 0x80);
	type = type & 0x7f;
	printf("data:%x, type:%s%s. ", data, type == 0 ? "9012" :
						type == 1 ? "NEC" :
						type == 2 ? "RC5" :
						type == 3 ? "RC6" : "unknown",
						repeat ? " REPEAT" : "");
	if(error & GX_HAL_IR_CUSTOM_CODE_ERR) ///< custom code error
	{
		printf("custom code error!\r\n");
	}
	if(error & GX_HAL_IR_COMMAND_CODE_ERR) ///< command code error
	{
		printf("command code error!\r\n");
	}
	if(error & GX_HAL_IR_PORT_DET_ERR) ///< port_det_err
	{
		printf("port_det_error!\r\n");
	}
	if(error & GX_HAL_IR_STD_RX_TIMEOUT) ///< port_det_err
	{
		printf("time out!\r\n");
	}
	if(error == 0) ///< port_det_err
	{
		printf("\r\n");
	}

	return 0;
}

int gx_ir_test_recv_std_err_cb(void* ir_dev, unsigned char error)
{
	if(error & GX_HAL_IR_CUSTOM_CODE_ERR) ///< custom code error
	{
		printf("custom code error!\r\n");
	}
	if(error & GX_HAL_IR_COMMAND_CODE_ERR) ///< command code error
	{
		printf("command code error!\r\n");
	}
	if(error & GX_HAL_IR_PORT_DET_ERR) ///< port_det_err
	{
		printf("port_det_error!\r\n");
	}
	if(error & GX_HAL_IR_STD_RX_TIMEOUT) ///< port_det_err
	{
		printf("time out!\r\n");
	}
	return 0;
}

/*标准模式中断接收*/
int gx_ir_test_recv_std_it(void)
{
	GX_HAL_IR_RX_CFG_T rx;

	memset(&rx, 0, sizeof(rx));
#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	rx.rx_thr_start = 0xbff;
	rx.rx_thr_data = 0xbff;
	rx.deb_legth = 0x17;
	rx.demod_legth = 0x07;
	rx.std_rx_tov = 180*1000;
#else
	rx.rx_thr_start = 0x7ff;
	rx.rx_thr_data = 0x7ff;
	rx.deb_legth = 0x0f;
	rx.demod_legth = 0x07;
	rx.std_rx_tov = 120*1000;
#endif
	rx.demod_en = 1;
#if defined(CONFIG_TARGET_APUS_FPGA_V1) || defined(CONFIG_TARGET_APUS_NRE_FPGA_V1) \
	|| defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)\
	|| defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	rx.invert_en = 1;
#else
	rx.invert_en = 0;
#endif
	rx.deb_en = 1;
	rx.kdc_cmp_en = 1;
	rx.cc_cmp_en = 1;

	gx_ir_recv_std_it_start(&rx);

	return 0;
}

/*标准模式接收,带解调*/
int gx_ir_test_recv_std(void)
{
	GX_HAL_IR_RX_CFG_T rx;
	uint32_t data = 0;
	uint8_t type = 0;
	uint8_t repeat = 0;
	int ret =0;

	memset(&rx, 0, sizeof(rx));
#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	rx.rx_thr_start = 0xbff;
	rx.rx_thr_data = 0xbff;
	rx.deb_legth = 0x17;
	rx.demod_legth = 0x07;
	rx.std_rx_tov = 180*1000;
#else
	rx.rx_thr_start = 0x7ff;
	rx.rx_thr_data = 0x7ff;
	rx.deb_legth = 0x0f;
	rx.demod_legth = 0x07;
	rx.std_rx_tov = 120*1000;
#endif
	rx.demod_en = 1;
#if defined(CONFIG_TARGET_APUS_FPGA_V1) || defined(CONFIG_TARGET_APUS_NRE_FPGA_V1) \
	|| defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)\
	|| defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	rx.invert_en = 1; //翻转电平
#else
	rx.invert_en = 0; //不翻转电平
#endif
	rx.deb_en = 1;

	gx_ir_recv_cfg(&rx, GX_HAL_IR_RECV_STD_MODE, GX_HAL_IR_ON);
	while(1)
	{
		ret = gx_ir_recv_data_std(&rx, (void*)&data, &type, 0);
		repeat = !!(type & 0x80);
		type = type & 0x7f;
		printf("%s(%x),recv type: %s %s, data: %x\r\n", ((ret==0)||(ret==0x10)) ? "Done" : "Error", ret,
			type == 0 ? "9012" :
			type == 1 ? "NEC" :
			type == 2 ? "RC5" :
			type == 3 ? "RC6" : "unknown",
			repeat ? "REPEAT" : "", data); //bit0~6, 0：9012， 1：NEC， 2：RC5， 3：RC6; bit7, 1：重复码, 0:键值
	}
}

/*标准模式接收,不带解调*/
int gx_ir_test_recv_std_no_demod(void)
{
	GX_HAL_IR_RX_CFG_T rx;
	uint32_t data = 0;
	uint8_t type = 0;
	int ret =0;
	uint8_t repeat = 0;

	memset(&rx, 0, sizeof(rx));
#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	rx.rx_thr_start = 0xbff;
	rx.rx_thr_data = 0xbff;
	rx.deb_legth = 0x17;
	rx.std_rx_tov = 180*1000;
#else
	rx.rx_thr_start = 0x7ff;
	rx.rx_thr_data = 0x7ff;
	rx.deb_legth = 0x0f;
	rx.std_rx_tov = 120*1000;
#endif
	rx.demod_legth = 0x07;
	rx.demod_en = 0;
#if defined(CONFIG_TARGET_APUS_FPGA_V1) || defined(CONFIG_TARGET_APUS_NRE_FPGA_V1) \
	|| defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)\
	|| defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	rx.invert_en = 1; //翻转电平
#else
	rx.invert_en = 0; //不翻转电平
#endif
	rx.deb_en = 1;
	gx_ir_recv_cfg(&rx, GX_HAL_IR_RECV_STD_MODE, GX_HAL_IR_ON);
	while(1)
	{
		ret = gx_ir_recv_data_std(&rx, (void*)&data, &type, 0);
		repeat = !!(type & 0x80);
		type = type & 0x7f;
		printf("%s(%x),recv type: %s %s, data: %x\r\n", ((ret==0)||(ret==0x10)) ? "Done" : "Error", ret,
			type == 0 ? "9012" :
			type == 1 ? "NEC" :
			type == 2 ? "RC5" :
			type == 3 ? "RC6" : "unknown",
			repeat ? "REPEAT" : "", data); //bit0~6, 0：9012， 1：NEC， 2：RC5， 3：RC6; bit7, 1：重复码, 0:键值
	}
}

/*灵活模式接收,带解调*/
int gx_ir_test_recv_ud(void)
{
	GX_HAL_IR_RX_CFG_T rx;
	uint32_t data[256] = {0};
	GX_HAL_IR_UD_DATA_T *ir_ud = (GX_HAL_IR_UD_DATA_T *)data;

	int ret = 0;

	memset(&rx, 0, sizeof(rx));
#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	rx.rx_thr_start = 0xbff;
	rx.rx_thr_data = 0xbff;
	rx.deb_legth = 0x17;
	rx.demod_legth = 0x07;
	rx.ud_rx_tov = 15000;
#else
	rx.rx_thr_start = 0x7ff;
	rx.rx_thr_data = 0x7ff;
	rx.deb_legth = 0x0f;
	rx.demod_legth = 0x07;
	rx.ud_rx_tov = 10000;
#endif
#if defined(CONFIG_TARGET_APUS_FPGA_V1) || defined(CONFIG_TARGET_APUS_NRE_FPGA_V1) \
	 || defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1) \
	 || defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	rx.invert_en = 1;
#else
	rx.invert_en = 0;
#endif
	rx.demod_en = 1;
	rx.deb_en = 1;
	rx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	rx.ud_rxfifo_thr = 0;
	gx_ir_recv_cfg(&rx, GX_HAL_IR_RECV_UD_MODE, GX_HAL_IR_ON);
	gx_ir_ud_rx_flush();
	ret = gx_ir_recv_data_ud(&rx, (void*)data, 256, 0);
	printf("ret %d,recv data:\r\n", ret);
	if(ret > 0)
	{
	//	print_buffer(0, data, 4, ret, 4);
		for(int i = 0; i < ret; i++)
		{
			printf("%d:data %x, h %d us, l %d us\r\n", i, data[i],
				GX_HAL_IR_UD_RAW2HIGH_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US), \
				GX_HAL_IR_UD_RAW2LOW_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US));
		}
	}
	return ret;
}

/*灵活模式接收,不带解调*/
int gx_ir_test_recv_ud_no_demod(void)
{
	GX_HAL_IR_RX_CFG_T rx;
	uint32_t data[64] = {0};
	GX_HAL_IR_UD_DATA_T *ir_ud = (GX_HAL_IR_UD_DATA_T *)data;
	int ret =0;

	memset(&rx, 0, sizeof(rx));
#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	rx.rx_thr_start = 0xbff;
	rx.rx_thr_data = 0xbff;
	rx.deb_legth = 0x17;
	rx.ud_rx_tov = 15000;
#else
	rx.rx_thr_start = 0x7ff;
	rx.rx_thr_data = 0x7ff;
	rx.deb_legth = 0x0f;
	rx.ud_rx_tov = 10000;
#endif
	rx.demod_legth = 0x07;
	rx.demod_en = 0;
#if defined(CONFIG_TARGET_APUS_FPGA_V1) || defined(CONFIG_TARGET_APUS_NRE_FPGA_V1) \
	 || defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)\
	 || defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	rx.invert_en = 1;
#else
	rx.invert_en = 0;
#endif
	rx.deb_en = 1;
	rx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	rx.ud_rxfifo_thr = 0;
	gx_ir_recv_cfg(&rx, GX_HAL_IR_RECV_UD_MODE, GX_HAL_IR_ON);
	gx_ir_ud_rx_flush();
	ret = gx_ir_recv_data_ud(&rx, (void*)data, 64, 0);
	printf("ret %d,recv data:\r\n", ret);
	if(ret > 0)
	{
	//	print_buffer(0, data, 4, ret, 4);
		for(int i = 0; i < ret; i++)
		{
			printf("%d:data %x, h %d us, l %d us\r\n", i, data[i],
				GX_HAL_IR_UD_RAW2HIGH_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US), \
				GX_HAL_IR_UD_RAW2LOW_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US));
		}
	}
	return ret;
}

static volatile uint8_t g_ud_rx_done = 0;
static volatile uint8_t g_ud_rx_carryfreq = 0;

#define  GX_HAL_IR_UD_RX_BUFF_LENGTH  256   // 2 的 n 次方
#define  GX_HAL_IR_UD_RX_BUFF_LENGTH_MASK  (GX_HAL_IR_UD_RX_BUFF_LENGTH-1)

typedef struct {
	unsigned int recv_ud_r;                                 /*!< 接收自定模式波形 buffer 的读位置指示 */
	unsigned int recv_ud_w;                                 /*!< 接收自定模式波形 buffer 的写位置指示 */
	unsigned int recv_ud_buff[GX_HAL_IR_UD_RX_BUFF_LENGTH]; /*!< 接收自定模式波形 buffer */
} GX_IR_TEST_RING_BUFFER_T;

GX_IR_TEST_RING_BUFFER_T gx_ir_test_ring_buff = {0};

unsigned int gx_ir_test_read_ud_rx_buf(unsigned int *msg_buf, unsigned int *wpos, unsigned int *rpos,
						unsigned int *msg, unsigned int len)
{
	unsigned int i = 0;

	if(len > GX_HAL_IR_UD_RX_BUFF_LENGTH)
		len = GX_HAL_IR_UD_RX_BUFF_LENGTH;

	for(i = 0; i < len; i++)
	{
		if(*wpos == *rpos)
			break;
		memcpy(&msg[i], &msg_buf[*rpos],sizeof(uint32_t));
		*rpos = (*rpos + 1)  & GX_HAL_IR_UD_RX_BUFF_LENGTH_MASK;
	}
	return i;
}

unsigned int gx_ir_test_write_ud_rx_buf(unsigned int *msg_buf, unsigned int *wpos, unsigned int *rpos,
						unsigned int *msg, unsigned int len)
{
	unsigned int i = 0;

	if(len > GX_HAL_IR_UD_RX_BUFF_LENGTH)
		len = GX_HAL_IR_UD_RX_BUFF_LENGTH;

	for(i = 0; i < len; i++)
	{
		memcpy(&msg_buf[*wpos], &msg[i],sizeof(uint32_t));
		*wpos = (*wpos + 1) & GX_HAL_IR_UD_RX_BUFF_LENGTH_MASK;
		if(*wpos == *rpos)
			*rpos = (*rpos + 1) & GX_HAL_IR_UD_RX_BUFF_LENGTH_MASK;
	}

	return i;
}

unsigned int gx_ir_test_get_ud_recv_data(unsigned int *data, unsigned int len)
{
	GX_IR_TEST_RING_BUFFER_T *ring_buf = (GX_IR_TEST_RING_BUFFER_T *)cfg.user_data;
	if((data == NULL) || (len == 0))
		return 0;

	return gx_ir_test_read_ud_rx_buf(ring_buf->recv_ud_buff, &ring_buf->recv_ud_w, &ring_buf->recv_ud_r, data, len);
}

unsigned int gx_ir_test_write_ud_recv_data(unsigned int *data, unsigned int len)
{
	GX_IR_TEST_RING_BUFFER_T *ring_buf = (GX_IR_TEST_RING_BUFFER_T *)cfg.user_data;
	if((data == NULL) || (len == 0))
		return 0;

	return gx_ir_test_write_ud_rx_buf(ring_buf->recv_ud_buff, &ring_buf->recv_ud_w, &ring_buf->recv_ud_r, data, len);
}

void gx_ir_test_flush_recv_data_buffer(void)
{
	GX_IR_TEST_RING_BUFFER_T *ring_buf = (GX_IR_TEST_RING_BUFFER_T *)cfg.user_data;
	ring_buf->recv_ud_r = ring_buf->recv_ud_w;
}

//灵活模式接收阈值中断回调函数
int gx_ir_test_recv_ud_thr_cb(void* ir_dev)
{
	uint32_t val = 0;
	uint16_t cnt = gx_ir_get_udrx_fifo_count();
	while(cnt--)
	{
		val = gx_ir_get_udrx_fifo_data();
		gx_ir_test_write_ud_recv_data((unsigned int*)&val, 1);
	}
	if(g_ud_rx_carryfreq)
	{
		gx_ir_recv_ud_stop();
		g_ud_rx_carryfreq = 0;
		g_ud_rx_done =1;
	}

	return 0;
}

//灵活模式接收中断回调函数
int gx_ir_test_recv_ud_cb(void* ir_dev)
{
	uint32_t val = 0;
	uint16_t cnt = gx_ir_get_udrx_fifo_count();
	while(cnt--)
	{
		val = gx_ir_get_udrx_fifo_data();
		gx_ir_test_write_ud_recv_data((unsigned int*)&val, 1);
	}
	g_ud_rx_done =1;
	return 0;
}

/*灵活模式中断接收，通过中断方式接收一串红外数据，红外模块先将波形数据存入 FIFO，接收完成后产生完成中断，然后将数据读出*/
int gx_ir_test_recv_ud_it(void)
{
	GX_HAL_IR_RX_CFG_T rx;
	uint32_t data[256] = {0};
	GX_HAL_IR_UD_DATA_T *ir_ud = (GX_HAL_IR_UD_DATA_T *)data;
	uint32_t ret =0;

	memset(&rx, 0, sizeof(rx));
#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	rx.rx_thr_start = 0xbff;
	rx.rx_thr_data = 0xbff;
	rx.deb_legth = 0x17;
	rx.demod_legth = 0x07;
	rx.ud_rx_tov = 22000;
#else
	rx.rx_thr_start = 0x7ff;
	rx.rx_thr_data = 0x7ff;
	rx.deb_legth = 0x0f;
	rx.demod_legth = 0x07;
	rx.ud_rx_tov = 15000;
#endif
#if defined(CONFIG_TARGET_APUS_FPGA_V1) || defined(CONFIG_TARGET_APUS_NRE_FPGA_V1) \
	 || defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)\
	 || defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	rx.invert_en = 1;
#else
	rx.invert_en = 0;
#endif
	rx.demod_en = 1;
	rx.deb_en = 1;
	rx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	rx.ud_rxfifo_thr = 2;
	gx_ir_ud_rx_flush();
	gx_ir_recv_ud_it_start(&rx);
	while (1)
	{
		if(g_ud_rx_done == 0)
			continue;
		g_ud_rx_done = 0;
		printf("ud rx done.\r\n");

		ret = gx_ir_test_get_ud_recv_data((void*)data, 256);
		if(ret)
		{
			for(int i = 0; i < ret; i++)
			{
				printf("%d:data %x, h %d us, l %d us\r\n", i, data[i],
					GX_HAL_IR_UD_RAW2HIGH_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US), \
					GX_HAL_IR_UD_RAW2LOW_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US));
			}
		}
	}
}

int gx_ir_test_recv_ud_and_carrfreq_it(void)
{
	GX_HAL_IR_RX_CFG_T rx;
	uint32_t data[256] = {0};
	GX_HAL_IR_UD_DATA_T *ir_ud = (GX_HAL_IR_UD_DATA_T *)data;
	uint32_t ret =0;
	uint32_t freq =0;
#if !defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1) && !defined(CONFIG_TARGET_SAGITTA_NRE_V1)
	uint8_t cnt =0;
#endif

	memset(&rx, 0, sizeof(rx));
#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	rx.rx_thr_start = 0xbff;
	rx.rx_thr_data = 0xbff;
	rx.deb_legth = 0x17;
	rx.demod_legth = 0x07;
	rx.ud_rx_tov = 22000;
#else
	rx.rx_thr_start = 0x7ff;
	rx.rx_thr_data = 0x7ff;
	rx.deb_legth = 0x0f;
	rx.demod_legth = 0x07;
	rx.ud_rx_tov = 15000;
#endif
#if defined(CONFIG_TARGET_APUS_FPGA_V1) || defined(CONFIG_TARGET_APUS_NRE_FPGA_V1) \
	 || defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)\
	 || defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	rx.invert_en = 1;
#else
	rx.invert_en = 0;
#endif
	rx.demod_en = 1;
	rx.deb_en = 1;
	rx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	rx.ud_rxfifo_thr = 2;
	gx_ir_ud_rx_flush();
	gx_ir_recv_ud_it_start(&rx);
	while (1)
	{
		if(g_ud_rx_done == 0)
			continue;
		g_ud_rx_done = 0;
		printf("ud rx done.\r\n");

		ret = gx_ir_test_get_ud_recv_data((void*)data, 256);
		if(ret)
		{
			for(int i = 0; i < ret; i++)
			{
				printf("%d:data %x, h %d us, l %d us\r\n", i, data[i],
					GX_HAL_IR_UD_RAW2HIGH_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US), \
					GX_HAL_IR_UD_RAW2LOW_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US));
			}
		#if defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1) || defined(CONFIG_TARGET_SAGITTA_NRE_V1)
			uint32_t duty =0;
			gx_ir_get_carrfreq(&freq, &duty);
			freq = (freq + 5) / 10;
			printf("carry freq: %d.%.2dKHz, duty:%d%%\n", freq/100, freq%100, duty);
			break;
		#else
			if(cnt >= 2)
			{
				for(int i = 0; i < ret; i++)
				{
					freq += ir_ud[i].cycle_cnt;
				}
				//取平均值，freq = (1/(t*GX_IR_UD_CARRY_TICK_US/ret))*1000KHz, 保留两位小数=*1000+5
				freq = ((ret*1000*1000) / (freq * GX_IR_UD_CARR_TICK_US) + 5) / 10;
				printf("carry freq: %d.%.2dKHz\n", freq/100, freq%100);
				break;
			}
			cnt++;
		#endif
		}
	#if !defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1) && !defined(CONFIG_TARGET_SAGITTA_NRE_V1)
		if(cnt >= 2)
		{
			gx_hal_ir_recv_ud_stop(ir_dev);
			rx.demod_en = 0;
			rx.deb_en = 0;
			rx.ud_clk_div = gx_ir_context_src_freq/GX_IR_UD_CARR_TICK-1;
			//清除开启接收载波波形前的数据，防止有数据在上次读取后被接收
			gx_hal_ir_ud_rx_flush(ir_dev);
			gx_ir_test_flush_recv_data_buffer(ir_dev);
			g_ud_rx_carryfreq = 1;
			gx_hal_ir_recv_ud_it_start(ir_dev, &rx);
		}
	#endif
	}
	return 0;
}

static int gx_ir_test_recv_ud_send(void)
{
	GX_HAL_IR_RX_CFG_T rx;
	uint32_t data[256] = {0};
	GX_HAL_IR_UD_DATA_T *ir_ud = (GX_HAL_IR_UD_DATA_T *)data;
	int ret = 0;

	memset(&rx, 0, sizeof(rx));
#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	rx.rx_thr_start = 0xbff;
	rx.rx_thr_data = 0xbff;
	rx.deb_legth = 0x17;
	rx.demod_legth = 0x07;
	rx.ud_rx_tov = 15000;
#else
	rx.rx_thr_start = 0x7ff;
	rx.rx_thr_data = 0x7ff;
	rx.deb_legth = 0x0f;
	rx.demod_legth = 0x07;
	rx.ud_rx_tov = 10000;
#endif
#if defined(CONFIG_TARGET_APUS_FPGA_V1) || defined(CONFIG_TARGET_APUS_NRE_FPGA_V1) \
	 || defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)\
	 || defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	rx.invert_en = 1;
#else
	rx.invert_en = 0;
#endif
	rx.demod_en = 1;
	rx.deb_en = 1;
	rx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	rx.ud_rxfifo_thr = 0;
	gx_ir_recv_cfg(&rx, GX_HAL_IR_RECV_UD_MODE, GX_HAL_IR_ON);
	gx_ir_ud_rx_flush();
	ret = gx_ir_recv_data_ud(&rx, (void*)data, 256, 0);
	printf("ret %d,recv data:\r\n", ret);
	if(ret > 0)
	{
	//	print_buffer(0, data, 4, ret, 4);
		for(int i = 0; i < ret; i++)
		{
			printf("%d:data %x, h %d us, l %d us\r\n", i, data[i],
				GX_HAL_IR_UD_RAW2HIGH_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US), \
				GX_HAL_IR_UD_RAW2LOW_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US));
		}
	}
	else
		return ret;

	GX_HAL_IR_TX_CFG_T tx;
	memset(&tx, 0, sizeof(tx));
	tx.modulation_en = 1;
	tx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	tx.ud_txfifo_thr = 3;
	tx.data = (void*)data;
	tx.data_len = ret;
	gx_ir_send_data_ud(&tx);

	return ret;
}

static int gx_ir_test_recv_ud_send_it(void)
{
	GX_HAL_IR_RX_CFG_T rx;
	uint32_t data[256] = {0};
	GX_HAL_IR_UD_DATA_T *ir_ud = (GX_HAL_IR_UD_DATA_T *)data;
	uint32_t ret =0;

	memset(&rx, 0, sizeof(rx));
#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	rx.rx_thr_start = 0xbff;
	rx.rx_thr_data = 0xbff;
	rx.deb_legth = 0x17;
	rx.demod_legth = 0x07;
	rx.ud_rx_tov = 22000;
#else
	rx.rx_thr_start = 0x7ff;
	rx.rx_thr_data = 0x7ff;
	rx.deb_legth = 0x0f;
	rx.demod_legth = 0x07;
	rx.ud_rx_tov = 1500;
#endif
#if defined(CONFIG_TARGET_APUS_FPGA_V1) || defined(CONFIG_TARGET_APUS_NRE_FPGA_V1) \
	|| defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)\
	|| defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	rx.invert_en = 1;
#else
	rx.invert_en = 0;
#endif
	rx.demod_en = 1;
	rx.deb_en = 1;
	rx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	rx.ud_rxfifo_thr = 2;
	gx_ir_ud_rx_flush();
	gx_ir_recv_ud_it_start(&rx);
	while (1)
	{
		if(g_ud_rx_done == 0)
			continue;
		g_ud_rx_done = 0;

		ret = gx_ir_test_get_ud_recv_data((void*)data, 256);
		if(ret)
		{
			for(int i = 0; i < ret; i++)
			{
				printf("%d:data %x, h %d us, l %d us\r\n", i, data[i],
					GX_HAL_IR_UD_RAW2HIGH_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US), \
					GX_HAL_IR_UD_RAW2LOW_DURATION(ir_ud[i], GX_IR_UD_DATA_TICK_US));
			}
			break;
		}
	}

	GX_HAL_IR_TX_CFG_T tx;
	memset(&tx, 0, sizeof(tx));
	tx.modulation_en = 1;
	tx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	tx.ud_txfifo_thr = 3;
	tx.data = (void*)data;
	tx.data_len = ret;
	return gx_ir_send_ud_it_start(&tx);
}

/*灵活模式发送,带调制*/
int gx_ir_test_send_ud(void)
{
	GX_HAL_IR_TX_CFG_T tx;
	uint32_t data[64] = {0x04b00320,0x00640032,0x00c80032,0x00c80032,0x00c80032, /* 0111 */
					0x00c80032,0x00640032,0x00640032,0x00c80032, /* 1001 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00640032,0x00640032,0x00640032,0x00640032, /* 0000 */
					0x00c80032,0x00c80032,0x00c80032,0x00c80032, /* 1111 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00c80032,0x00640032,0x00640032,0x00c80032, /* 1001 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00c80032};
	gx_ir_ud_tx_wait_done();
	memset(&tx, 0, sizeof(tx));
	tx.modulation_en = 1;
	tx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	tx.ud_txfifo_thr = 3;
	tx.data = (void*)data;
	tx.data_len = 34;
	gx_ir_send_data_ud(&tx);

	return 0;
}

/*灵活模式发送,不带调制*/
int gx_ir_test_send_ud_nomod(void)
{
	GX_HAL_IR_TX_CFG_T tx;
	uint32_t data[64] = {0x04b00320,0x00640032,0x00c80032,0x00c80032,0x00c80032, /* 0111 */
					0x00c80032,0x00640032,0x00640032,0x00c80032, /* 1001 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00640032,0x00640032,0x00640032,0x00640032, /* 0000 */
					0x00c80032,0x00c80032,0x00c80032,0x00c80032, /* 1111 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00c80032,0x00640032,0x00640032,0x00c80032, /* 1001 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00c80032};
	gx_ir_ud_tx_wait_done();
	memset(&tx, 0, sizeof(tx));
	tx.modulation_en = 0;
	tx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	tx.ud_txfifo_thr = 3;
	tx.data = (void*)data;
	tx.data_len = 34;
	gx_ir_send_data_ud(&tx);

	return 0;
}

/*标准模式发送,带调制*/
int gx_ir_test_send_std(uint8_t type)
{
	if(type > 3)
		return -1;
	GX_HAL_IR_TX_CFG_T tx;
	uint32_t data[4] = {0x23456789,0xeeff1122,0x1b2b3b4b,0x90441c02};
	memset(&tx, 0, sizeof(tx));
	tx.type = type;
	tx.modulation_en = 1;
	tx.data = (void*)data;
	tx.data_len = 4;
	gx_ir_send_data_std(&tx, gx_mdelay, 0);

	return 0;
}

/*标准模式发送,带调制*/
int gx_ir_test_send_std_it(uint8_t type)
{
	GX_HAL_IR_TX_CFG_T tx;
	uint32_t data[4] = {0xfd016c93, 0xe0efe01, 0xd0002, 0x02};
	uint8_t i = 0;

	memset(&tx, 0, sizeof(tx));
	tx.type = type;
	tx.modulation_en = 1;
	tx.data = (void*)&data[type];
	tx.data_len = 1;
	g_send_std_done = 0;
	gx_ir_send_data_std_it_start(&tx);
#if 0 //检查结束的方式
	while(g_send_std_done == 0);
#else
	//检查3次，启动后需要几个clock才会开始发送，busy才会置位。
	while(i < 3)
	{
		while(gx_ir_is_stdtx_busy());
		i++;
	}
#endif

	return 0;
}

/*标准模式发送,不带调制*/
int gx_ir_test_send_std_nomod(uint8_t type)
{
	GX_HAL_IR_TX_CFG_T tx;
	uint32_t data[4] = {0x23456789,0xeeff1122,0x1b2b3b4b,0x90441c02};
	memset(&tx, 0, sizeof(tx));
	tx.type = type;
	tx.modulation_en = 0;
	tx.data = (void*)data;
	tx.data_len = 4;
	gx_ir_send_data_std(&tx, gx_mdelay, 200);

	return 0;
}

/*标准模式发送数据和重复码，带调制*/
int gx_ir_test_send_std_repeat(uint8_t type, uint8_t enable)
{
	GX_HAL_IR_TX_CFG_T tx;
	uint32_t data[1] = {0x23456789};
	memset(&tx, 0, sizeof(tx));
	if(enable)
	{
		tx.type = type;
		tx.modulation_en = 1;
		tx.data = (void*)data;
		tx.data_len = 1;
		gx_ir_send_data_std(&tx, gx_mdelay, 200);
	}

	gx_ir_std_repeat_ctrl(!!enable);

	return 0;
}

/*标准模式发送数据和重复码，不带调制*/
int gx_ir_test_send_std_repeat_nomod(uint8_t type, uint8_t enable)
{
	GX_HAL_IR_TX_CFG_T tx;
	uint32_t data[1] = {0x23456789};
	memset(&tx, 0, sizeof(tx));
	if(enable)
	{
		tx.type = type;
		tx.modulation_en = 0;
		tx.data = (void*)data;
		tx.data_len = 1;
		gx_ir_send_data_std_it_start(&tx);
	}
	gx_udelay(1);//异步模式，启动后模块需要几个clock才能启动发送，不能立即置位repeat，否则数据可能发不出去，只有repeat。

	gx_ir_std_repeat_ctrl(!!enable);

	return 0;
}

volatile static uint8_t g_ud_tx_done = 0;
/*灵活模式发送完成回调函数*/
int gx_ir_test_send_ud_done_cb(void* ir_dev)
{
	printf("send ud done\r\n");
	gx_ir_send_ud_stop();
	g_ud_tx_done = 1;
	return 0;
}

/*灵活模式中断发送，带调制*/
int gx_ir_test_send_ud_it(void)
{
	GX_HAL_IR_TX_CFG_T tx;
	uint32_t data[64] = {0x04b00320,0x00640032,0x00c80032,0x00c80032,0x00c80032, /* 0111 */
					0x00c80032,0x00640032,0x00640032,0x00c80032, /* 1001 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00640032,0x00640032,0x00640032,0x00640032, /* 0000 */
					0x00c80032,0x00c80032,0x00c80032,0x00c80032, /* 1111 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00c80032,0x00640032,0x00640032,0x00c80032, /* 1001 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00c80032};
	gx_ir_ud_tx_wait_done();
	memset(&tx, 0, sizeof(tx));
	tx.modulation_en = 1;
	tx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	tx.ud_txfifo_thr = 3;
	tx.data = (void*)data;
	tx.data_len = 34;
	g_ud_tx_done = 0;
	g_count = gx_ir_get_udtx_fifo_depth();
	g_len = tx.data_len;
	g_data = data;
	gx_ir_send_ud_it_start(&tx);

	while(g_ud_tx_done == 0);
	return 0;
}

/*灵活模式中断发送，不带调制*/
int gx_ir_test_send_ud_nomod_it(void)
{
	GX_HAL_IR_TX_CFG_T tx;
	uint32_t data[64] = {0x04b00320,0x00640032,0x00c80032,0x00c80032,0x00c80032, /* 0111 */
					0x00c80032,0x00640032,0x00640032,0x00c80032, /* 1001 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00640032,0x00640032,0x00640032,0x00640032, /* 0000 */
					0x00c80032,0x00c80032,0x00c80032,0x00c80032, /* 1111 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00c80032,0x00640032,0x00640032,0x00c80032, /* 1001 */
					0x00c80032,0x00c80032,0x00640032,0x00640032, /* 1100 */
					0x00c80032};
	gx_ir_ud_tx_wait_done();
	memset(&tx, 0, sizeof(tx));
	tx.modulation_en = 0;
	tx.ud_clk_div = GX_IR_CALC_DIV(gx_ir_context_src_freq, GX_IR_UD_DATA_TICK)-1;
	tx.ud_txfifo_thr = 3;
	tx.data = (void*)data;
	tx.data_len = 34;
	g_ud_tx_done = 0;
	g_count = gx_ir_get_udtx_fifo_depth();
	g_len = tx.data_len;
	g_data = data;
	gx_ir_send_ud_it_start(&tx);
	while(g_ud_tx_done == 0);

	return 0;
}

/*标准模式发送完成回调回调函数*/
int gx_ir_test_recv_std_then_send(void);
int gx_ir_test_recv_std_then_send_tx_done_cb(void* ir_dev)
{
	return gx_ir_test_recv_std_then_send();
}

int gx_ir_test_recv_std_then_send_timer_cb(void* pdata)
{
	GX_HAL_IR_TX_CFG_T *tx = (GX_HAL_IR_TX_CFG_T *)pdata;
	printf("send type=%d,data=%x\r\n",tx->type, *(tx->data));
	return gx_ir_send_data_std_it_start(tx);
}

static GX_HAL_IR_TX_CFG_T gx_ir_test_recv_std_then_send_tx = {0};
static uint32_t gx_ir_test_recv_std_then_send_data = 0;
/* 接收完成回调函数 */
int gx_ir_test_recv_std_then_send_cb(void* ir_dev, unsigned int data, unsigned char type, unsigned char error)
{
	if(error & GX_HAL_IR_CUSTOM_CODE_ERR) ///< custom code error
	{
		printf("custom code error!\r\n");
	}
	if(error & GX_HAL_IR_COMMAND_CODE_ERR) ///< command code error
	{
		printf("command code error!\r\n");
	}
	if(error & GX_HAL_IR_PORT_DET_ERR) ///< port_det_err
	{
		printf("port_det_error!\r\n");
	}
	if(error & GX_HAL_IR_STD_RX_TIMEOUT) ///< port_det_err
	{
		printf("time out!\r\n");
	}
	if(error == 0) ///< port_det_err
	{
		GX_HAL_IR_TX_CFG_T *tx = &gx_ir_test_recv_std_then_send_tx;
		memset(tx, 0, sizeof(GX_HAL_IR_TX_CFG_T));
		tx->type = type & 0x7f;
		tx->modulation_en = 1;
		gx_ir_test_recv_std_then_send_data = data;
		tx->data = (void*)&gx_ir_test_recv_std_then_send_data;
		tx->data_len = 1;
		// 延时1ms后再发送
		gx_timer_register(gx_ir_test_recv_std_then_send_timer_cb, 1, tx, GX_TIMER_MODE_SINGLE);
	}

	return 0;
}

int gx_ir_test_recv_std_then_send(void)
{
	GX_HAL_IR_RX_CFG_T rx;

	cfg.std_rx_done_cb = gx_ir_test_recv_std_then_send_cb;
	cfg.std_tx_done_cb = gx_ir_test_recv_std_then_send_tx_done_cb;

	memset(&rx, 0, sizeof(rx));
#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	rx.rx_thr_start = 0xbff;
	rx.rx_thr_data = 0xbff;
	rx.deb_legth = 0x17;
	rx.demod_legth = 0x07;
	rx.std_rx_tov = 180*1000;
#else
	rx.rx_thr_start = 0x7ff;
	rx.rx_thr_data = 0x7ff;
	rx.deb_legth = 0x0f;
	rx.demod_legth = 0x07;
	rx.std_rx_tov = 120*1000;
#endif
	rx.demod_en = 1;
#if defined(CONFIG_TARGET_APUS_FPGA_V1) || defined(CONFIG_TARGET_APUS_NRE_FPGA_V1)\
	 || defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)\
	 || defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	rx.invert_en = 1;
#else
	rx.invert_en = 0;
#endif
	rx.deb_en = 1;
	rx.kdc_cmp_en = 1;
	rx.cc_cmp_en = 1;
	gx_ir_recv_std_it_start(&rx);

	return 0;
}


static uint8_t  gx_ir_test_init_flag = 0;

int gx_ir_test_init(GX_IR_TEST_INIT_CFG_T *init_cfg)
{

#if defined(CONFIG_TARGET_FORNAX_NRE_FPGA_V1) || defined(CONFIG_TARGET_FORNAX_NRE_V1)
	gx_clk_mod_set_src(GX_CLK_MOD_IRC, GX_CLK_MOD_SRC_12M);
	gx_clk_mod_set_gate(GX_CLK_MOD_IRC, 1);
	printf("src freq=%d\n",gx_clk_mod_get_freq(GX_CLK_MOD_IRC));
	padmux_set(8, 6);
	padmux_set(9, 6);
	padmux_set(10, 6);
#elif defined(CONFIG_TARGET_SAGITTA_NRE_FPGA_V1)
	gx_clk_mod_set_src(GX_CLK_MOD_IRC, GX_CLK_MOD_SRC_OSC);
	gx_clk_mod_set_div(GX_CLK_MOD_IRC, 3);
	gx_clk_mod_set_gate(GX_CLK_MOD_IRC, 1);
	padmux_set(3, 123); //IR_TX
	padmux_set(4, 122); //IR_RX
	padmux_set(16, 124); //IR_EN
#elif defined(CONFIG_TARGET_SAGITTA_NRE_V1)
	padmux_set(16, 123); //IR_TX
	padmux_set(35, 1);   //IR_RX
	padmux_set_io_analog_enable(35, 1); //IR_RX
#else
	gx_clk_mod_set_src(GX_CLK_MOD_IRC, GX_CLK_MOD_SRC_32M);
	gx_clk_mod_set_div(GX_CLK_MOD_IRC, 4);
	gx_clk_mod_set_gate(GX_CLK_MOD_IRC, 1);
#endif

	gx_ir_context_src_freq = gx_clk_mod_get_freq(GX_CLK_MOD_IRC);
	printf("gx_ir_context_src_freq=%d\n",gx_ir_context_src_freq);
	memset(&cfg, 0, sizeof(cfg));

	cfg.std_rx_done_cb = gx_ir_test_recv_std_cb;
	cfg.std_rx_err_cb = gx_ir_test_recv_std_err_cb;
	cfg.ud_rx_thr_cb = gx_ir_test_recv_ud_thr_cb;
	cfg.ud_rx_done_cb = gx_ir_test_recv_ud_cb;
	cfg.ud_tx_done_cb = gx_ir_test_send_ud_done_cb;
	cfg.std_tx_done_cb = gx_ir_test_std_tx_done_cb;
	cfg.ud_rx_full_cb = gx_ir_test_ud_rx_full_cb;
	cfg.ud_rx_empty_cb = gx_ir_test_ud_rx_empty_cb;
	cfg.ud_tx_thr_cb = gx_ir_test_ud_tx_thr_cb;
	cfg.ud_tx_full_cb = gx_ir_test_ud_tx_full_cb;
	cfg.ud_tx_empty_cb = gx_ir_test_ud_tx_empty_cb;
	cfg.user_data = &gx_ir_test_ring_buff;

	return gx_ir_init(&cfg);
}

static int do_gx_ir_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;

	if (argc < 2)
		return -1;

	if (strcmp(argv[0], "gx_ir"))
		return -1;

	if(gx_ir_test_init_flag == 0)
	{
		GX_IR_TEST_INIT_CFG_T init_cfg = {0, 0, 1, 3, 1, 0};
		if(gx_ir_test_init(&init_cfg) == 0)
			gx_ir_test_init_flag = 1;
	}

	if (!strcmp(argv[1], "init"))
	{
		GX_IR_TEST_INIT_CFG_T init_cfg = {0, 0, 1, 3, 1, 0};
		if(argc > 2)
			init_cfg.testsel = simple_strtoul(argv[2], NULL, 0);
		if(argc > 3)
			init_cfg.isel = simple_strtoul(argv[3], NULL, 0);
		if(argc > 4)
			init_cfg.vcmsel = simple_strtoul(argv[4], NULL, 0);
		if(argc > 5)
			init_cfg.drv_cap = simple_strtoul(argv[5], NULL, 0);
		if(argc > 6)
			init_cfg.vstset = simple_strtoul(argv[6], NULL, 0);
		if(argc > 7)
			init_cfg.testmod_en = simple_strtoul(argv[7], NULL, 0);
		gx_ir_test_init(&init_cfg);
	}
	else if (!strcmp(argv[1], "recv_std"))
	{
		ret = gx_ir_test_recv_std();
	}
	else if (!strcmp(argv[1], "recv_std_it"))
	{
		gx_ir_test_recv_std_it();
	}
	else if (!strcmp(argv[1], "recv_std_nodem"))
	{
		ret = gx_ir_test_recv_std_no_demod();
	}
	else if (!strcmp(argv[1], "recv_std_close"))
	{
		gx_ir_recv_std_stop();
	}
	else if (!strcmp(argv[1], "recv_ud"))
	{
		ret = gx_ir_test_recv_ud();
	}
	else if (!strcmp(argv[1], "recv_ud_nodem"))
	{
		ret = gx_ir_test_recv_ud_no_demod();
	}
	else if (!strcmp(argv[1], "recv_ud_it"))
	{
		gx_ir_test_recv_ud_it();
	}
	else if (!strcmp(argv[1], "recv_ud_carr_it"))
	{
		gx_ir_test_recv_ud_and_carrfreq_it();
	}
	else if (!strcmp(argv[1], "recv_ud_close"))
	{
		gx_ir_recv_ud_stop();
	}
	else if (!strcmp(argv[1], "recv_ud_send"))
	{
		gx_ir_test_recv_ud_send();
	}
	else if (!strcmp(argv[1], "recv_ud_send_it"))
	{
		gx_ir_test_recv_ud_send_it();
	}
	else if (!strcmp(argv[1], "send_std"))
	{
		uint8_t type = 0;
		if(argc > 2)
			type = simple_strtoul(argv[2], NULL, 0);

		gx_ir_test_send_std(type);
	}
	else if (!strcmp(argv[1], "send_std_it"))
	{
		uint32_t type = 0;
		if(argc > 2)
			type = simple_strtoul(argv[2], NULL, 0);
		gx_ir_test_send_std_it(type);
	}
	else if (!strcmp(argv[1], "send_std_nomod"))
	{
		uint32_t type = 0;
		if(argc > 2)
			type = simple_strtoul(argv[2], NULL, 0);
		gx_ir_test_send_std_nomod(type);
	}
	else if (!strcmp(argv[1], "send_ud"))
	{
		gx_ir_test_send_ud();
	}
	else if (!strcmp(argv[1], "send_ud_it"))
	{
		gx_ir_test_send_ud_it();
	}
	else if (!strcmp(argv[1], "send_ud_nomod"))
	{
		gx_ir_test_send_ud_nomod();
	}
	else if (!strcmp(argv[1], "send_ud_nomod_it"))
	{
		gx_ir_test_send_ud_nomod_it();
	}
	else if (!strcmp(argv[1], "send_repeat"))
	{
		uint32_t type = simple_strtoul(argv[2], NULL, 0);
		uint32_t en = simple_strtoul(argv[3], NULL, 0);
		if(type > 3)
		{
			printf("type should be 0~3\r\n");
			return -1;
		}
		gx_ir_test_send_std_repeat(type, en);
	}
	else if (!strcmp(argv[1], "send_repeat_nomod"))
	{
		uint32_t type = simple_strtoul(argv[2], NULL, 0);
		uint32_t en = simple_strtoul(argv[3], NULL, 0);
		if(type > 3)
		{
			printf("type should be 0~3\r\n");
			return -1;
		}
		gx_ir_test_send_std_repeat_nomod(type, en);
	}
#if defined(CONFIG_TARGET_APUS_NRE_FPGA_V1)
	else if (!strcmp(argv[1], "key_send_std"))
	{
		uint32_t type = 0;
		if(argc > 2)
			type = simple_strtoul(argv[2], NULL, 0);
		gx_ir_test_key_send_std(type);
	}
#endif
	else if (!strcmp(argv[1], "recv_std_then_send"))
	{
		gx_ir_test_recv_std_then_send();
	}
	else if (!strcmp(argv[1], "set_freq"))
	{
		uint32_t freq = 0;
		uint32_t duty = 0;
		if(argc < 3)
			return -1;
		freq = simple_strtoul(argv[2], NULL, 0);
		duty = simple_strtoul(argv[3], NULL, 0);
		gx_ir_set_tx_carr_freq(freq, duty);
	}
	else if (!strcmp(argv[1], "read_reg"))
	{
		uint32_t addr = 0;
		uint32_t val = 0;
		if(argc < 2)
			return -1;
		addr = simple_strtoul(argv[2], NULL, 0);
		val = readl(addr);
		printf("%x: %x\r\n", addr, val);
	}
	else if (!strcmp(argv[1], "write_reg"))
	{
		uint32_t addr = 0;
		uint32_t val = 0;
		if(argc < 3)
			return -1;
		addr = simple_strtoul(argv[2], NULL, 0);
		val = simple_strtoul(argv[3], NULL, 0);
		writel(val, addr);
		printf("write done.\r\n");
	}
	else
		return -1;

	return ret;
}

U_BOOT_CMD(
	gx_ir, 8, 1, do_gx_ir_test,
	"test gx_ir",
	"init [testsel] [isel] [vcmsel] [drv_cap] [vstset] [testmod_en] - initializing the gx_ir device. \n"
	"     testsel: 1 enable receive module connect with tast ana pin. 0[default] close.\n"
	"     isel: select reference current, 0[default] is 1uA, 1 is 0.5uA. \n"
	"     vcmsel: select reference voltage, 3bit value.[default=1]\n"
	"     drv_cap: tx drive strength, 4bit value.[default=3]\n"
	"     vstset: soft start control of the transmitting circuit, 1[default] is on, 0 is off.\n"
	"     testmod_en: test mode,tx is always on. 1 is enable, 0 is disable.\n"
	"recv_std        - receive standard protocal\n"
	"recv_std_it     - receive standard protocal with interrupt\n"
	"recv_std_nodem  - receive standard protocal without demodulation\n"
	"recv_std_close  - close receive standard module.\n"
	"recv_ud         - receive not standard protocal wave data.\n"
	"recv_ud_nodem   - receive not standard protocal wave data without demodulation.\n"
	"recv_ud_it      - receive not standard protocal wave data  with interrupt.\n"
	"recv_ud_carr_it - receive not standard protocal wave data and carrier wave requency with interrupt.\n"
	"recv_ud_close   - close receive not standard module.\n"
	"recv_ud_send    - receive not standard protocal wave data then send same data.\n"
	"recv_ud_send_it - receive not standard protocal wave data then send same data with interrupt.\n"
	"send_std [type] - send standard protocal, type: 0[default] is 9012, 1 is NEC, 2 is RC5, 3 is RC6.\n"
	"send_std_it [type] - send standard protocal with interrupt, type: 0[default] is 9012, 1 is NEC, 2 is RC5, 3 is RC6.\n"
	"send_std_nomod [type] - send standard protocal without modulation, type: 0[default] is 9012, 1 is NEC, 2 is RC5, 3 is RC6.\n"
	"send_ud         - send not standard protocal wave data.\n"
	"send_ud_it      - send not standard protocal wave data with interrupt.\n"
	"send_ud_nomod   - send not standard protocal wave data without modulation.\n"
	"send_ud_nomod_it   - send not standard protocal wave data with interrupt and without modulation.\n"
	"send_repeat <type> <en> - send standard protocal repeat wave.\n"
	"            type： 0 is 9012, 1 is NEC, 2 is RC5, 3 is RC6. en: 1 is enable, 0 is disable.\n"
	"send_repeat_nomod <type> <en> - send standard protocal repeat wave without modulation.\n"
	"            type： 0 is 9012, 1 is NEC, 2 is RC5, 3 is RC6. en: 1 is enable, 0 is disable.\n"
	"key_send_std <type> - send standard protocal by keysacn, type: 0[default] is 9012, 1 is NEC, 2 is RC5, 3 is RC6.\n"
	"set_freq <freq> <duty>  - set tx frequence and duty. freq: 1954Hz~8000000Hz, duty:1~100\n"
	"recv_std_then_send      - receive standard protocal, then send the data in the same way.\n"
	"read_reg <addr> - read register value\n"
	"write_reg <addr> <val>  - write a value to address\n"
);

