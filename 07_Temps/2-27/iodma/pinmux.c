#include <gx_padmux.h>

#define ARRAY_SIZE(x)     (sizeof(x) / sizeof((x)[0]))

/* Default IOMATRIX config DONOT edit */
/*
| NUM | FUNC          | NUM | FUNC          | NUM | FUNC          | NUM | FUNC          |
| --- | ------------- | --- | ------------- | --- | ------------- | --- | ------------- |
| 8   | IDLE          | 9   |               | 10  | uart0_tx      | 11  | uart0_rx      |
| 12  |               | 13  |               | 14  | uart1_tx      | 15  | uart1_rx      |
| 16  | uart1_cts     | 17  | uart1_rts     | 18  | i2c0_scl     | 19  | i2c0_sda      |
| 20  | i2c1_scl      | 21  | i2c1_sda      | 22  | i2c2_scl     | 23  | i2c2_sda      |
| 24  | pwm[0]        | 25  | pwm[1]        | 26  | pwm[2]       | 27  | pwm[3]        |
| 28  | pwm[4]        | 29  | pwm[5]        | 30  | pwm[6]       | 31  | pwm[7]        |
| 32  | qdec0_a       | 33  | qdec0_b       | 34  | qdec1_a      | 35  | qdec1_b       |
| 36  | qdec2_a       | 37  | qdec2_b       | 38  | key_col[0]   | 39  | key_col[1]    |
| 40  | key_col[2]    | 41  | key_col[3]    | 42  | key_col[4]   | 43  | key_col[5]    |
| 44  | key_col[6]    | 45  | key_col[7]    | 46  | iodma[0]     | 47  | iodma[1]      |
| 48  | iodma[2]      | 49  | iodma[3]      | 50  | iodma[4]     | 51  | iodma[5]      |
| 52  | iodma[6]      | 53  | iodma[7]      | 54  | qdec0_led    | 55  | qdec1_led     |
| 56  | qdec2_led     | 57  |               | 58  | key_row[0]   | 59  | key_row[1]    |
| 60  | key_row[2]    | 61  | key_row[3]    | 62  | key_row[4]   | 63  | key_row[5]    |
| 64  | key_row[6]    | 65  | key_row[7]    | 66  | SD_D4        | 67  | SD_D5         |
| 68  | SD_D6         | 69  | SD_D7         | 70  | qspi2_cs0_m  | 71  | qspi2_cs1_m   |
| 72  | qspi2_cs2_m   | 73  | spi3_clk      | 74  | spi3_mosi    | 75  | spi3_miso     |
| 76  |               | 77  |               | 78  | spi3_cs      | 79  | qspi2_clk_m   |
| 80  | qspi2_mosi_m  | 81  | qspi2_miso_m  | 82  | qspi2_wp_m   | 83  | qspi2_hold_m  |
| 84  | qspi2_cs_s    | 85  | qspi2_clk_s   | 86  | qspi2_mosi_s | 87  | qspi2_miso_s  |
| 88  | qspi2_wp_s    | 89  | qspi2_hold_s  | 90  | i2s_bclk     | 91  | i2s_lrclk     |
| 92  | i2s_din       | 93  | i2s_dout      | 94  |              | 95  | SD_RSTN       |
| 96  | SD_CLK        | 97  | SD_CMD        | 98  | SD_D0        | 99  | SD_D1         |
| 100 | SD_D2         | 101 | SD_D3         | 102 | SD_CCLK_RX   | 103 | i2s_mclk      |
| 104 |               | 105 | JTAG_TMS      | 106 | JTAG_TCK     | 107 | lcd_cs        |
| 108 | lcd_wr        | 109 | lcd_rs        | 110 | lcd_rd       | 111 |               |
| 112 | ANT0          | 113 | ANT1          | 114 | ANT2         | 115 | ANT3          |
| 116 | RF_TXEN       | 117 | RF_RXEN       | 118 | LPC_O        | 119 |               |
| 120 |               | 121 | SD_CARDDETECT | 122 | IR_RX        | 123 | IR_TX         |
| 124 | IR_EN         | 125 | SD_WP         | 126 | lcd_d[0]     | 127 | lcd_d[1]      |
| 128 | lcd_d[2]      | 129 | lcd_d[3]      | 130 | lcd_d[4]     | 131 | lcd_d[5]      |
| 132 | lcd_d[6]      | 133 | lcd_d[7]      | 134 |              | 135 |               |
*/

__attribute__((unused))
static const GX_PIN_CONFIG pin_table[] = {
//   id     func    | func 0      | func 1      | func 2           | func 3     | func 4   | func 5             | func 6   | func 7   |
    { 0,    0},     // xo_en      | idle        | i_adc_test_clk  | gpio[0]    | reserve   | wireless_debug[0]  | reserve  | reserve  |
    { 1,    47},     // idle       | idle        | i_adc_test_d[0] | gpio[1]    | reserve   | wireless_debug[1]  | reserve  | reserve  |
    { 2,    0},     // idle       | idle        | i_adc_test_d[1] | gpio[2]    | reserve   | wireless_debug[2]  | reserve  | reserve  |
    { 3,    48},     // idle       | idle        | i_adc_test_d[2] | gpio[3]    | reserve   | wireless_debug[3]  | reserve  | reserve  |
    { 4,    49},     // idle       | idle        | i_adc_test_d[3] | gpio[4]    | reserve   | wireless_debug[4]  | reserve  | reserve  |
    { 5,    0},     // uart0_rx   | idle        | i_adc_test_d[4] | gpio[5]    | reserve   | wireless_debug[5]  | reserve  | reserve  |
    { 6,    0},     // uart0_tx   | idle        | i_adc_test_d[5] | gpio[6]    | reserve   | wireless_debug[6]  | reserve  | reserve  |
    { 7,    50},     // boot       | idle        | i_adc_test_d[6] | gpio[7]    | reserve   | wireless_debug[7]  | reserve  | reserve  |
    { 8,    51},     // idle       | idle        | key_col[8]      | gpio[8]    | reserve   | wireless_debug[8]  | reserve  | reserve  |
    { 9,    52},     // idle       | idle        | key_col[9]      | gpio[9]    | reserve   | wireless_debug[9]  | reserve  | reserve  |
    { 10,   0},     // jtag_tms   | idle        | idle            | gpio[10]   | reserve   | wireless_debug[10] | reserve  | reserve  |
    { 11,   0},     // jtag_tck   | idle        | idle            | gpio[11]   | reserve   | wireless_debug[11] | reserve  | reserve  |
    { 12,   53},     // idle       | idle        | key_col[10]     | gpio[12]   | reserve   | wireless_debug[12] | reserve  | reserve  |
    { 13,   0},     // idle       | idle        | key_col[11]     | gpio[13]   | reserve   | wireless_debug[13] | reserve  | reserve  |
    { 14,   0},     // idle       | idle        | idle            | gpio[14]   | reserve   | wireless_debug[14] | reserve  | reserve  |
    { 15,   0},     // idle       | idle        | rc_xo_mux_clk   | gpio[15]   | reserve   | wireless_debug[15] | reserve  | reserve  |
    { 16,   0},     // idle       | idle        | iodma[9]        | gpio[16]   | reserve   | wireless_debug[16] | reserve  | reserve  |
    { 17,   0},     // sys_i2c_sda| idle        | idle            | gpio[17]   | reserve   | wireless_debug[17] | reserve  | reserve  |
    { 18,   0},     // idle       | idle        | key_col[12]     | gpio[18]   | reserve   | wireless_debug[18] | reserve  | reserve  |
    { 19,   0},     // idle       | idle        | key_col[13]     | gpio[19]   | reserve   | wireless_debug[19] | reserve  | reserve  |
    { 20,   0},     // idle       | idle        | key_col[14]     | gpio[20]   | reserve   | wireless_debug[20] | reserve  | reserve  |
    { 21,   0},     // idle       | idle        | key_col[15]     | gpio[21]   | reserve   | wireless_debug[21] | reserve  | reserve  |
    { 22,   0},     // idle       | idle        | pll_clk_out     | gpio[22]   | reserve   | wireless_debug[22] | reserve  | reserve  |
    { 23,   0},     // idle       | idle        | low_clk         | gpio[23]   | reserve   | wireless_debug[23] | reserve  | reserve  |
    { 24,   0},     // pin_rst_b  | idle        | idle            | gpio[24]   | reserve   | wireless_debug[24] | reserve  | reserve  |
    { 25,   0},     // sys_i2c_scl| idle        | idle            | gpio[25]   | reserve   | wireless_debug[25] | reserve  | reserve  |
    { 26,   0},     // idle       | idle        | key_col[16]     | gpio[26]   | reserve   | wireless_debug[26] | reserve  | reserve  |
    { 27,   0},     // idle       | idle        | key_col[17]     | gpio[27]   | reserve   | wireless_debug[27] | reserve  | reserve  |
    { 28,   0},     // idle       | idle        | key_col[18]     | gpio[28]   | reserve   | wireless_debug[28] | reserve  | reserve  |
    { 29,   0},     // idle       | idle        | key_col[19]     | gpio[29]   | reserve   | wireless_debug[29] | reserve  | reserve  |
    { 30,   0},     // idle       | idle        | iodma[10]       | gpio[30]   | reserve   | wireless_debug[30] | reserve  | reserve  |
    { 31,   0},     // idle       | idle        | iodma[11]       | gpio[31]   | reserve   | wireless_debug[31] | reserve  | reserve  |
    { 32,   0},     // idle       | idle        | idle            | gpio[32]   | reserve   | wireless_debug[32] | reserve  | reserve  |
    { 33,   0},     // idle       | idle        | idle            | gpio[33]   | reserve   | wireless_debug[33] | reserve  | reserve  |
    { 34,   0},     // idle       | idle        | iodma[12]       | gpio[34]   | reserve   | wireless_debug[34] | reserve  | reserve  |
    { 35,   0},     // idle       | ir          | idle            | gpio[35]   | reserve   | wireless_debug[35] | reserve  | reserve  |
    { 36,   0},     // onoff      | idle        | idle            | gpio[36]   | reserve   | wireless_debug[36] | reserve  | reserve  |
    { 37,   0},     // idle       | idle        | iodma[13]       | gpio[37]   | reserve   | i_adc_test_d[7]   | reserve  | reserve  |
    { 38,   0},     // idle       | idle        | iodma[14]       | gpio[38]   | reserve   | i_adc_test_d[8]   | reserve  | reserve  |
    { 39,   0},     // idle       | idle        | iodma[15]       | gpio[39]   | reserve   | i_adc_test_d[9]   | reserve  | reserve  |
    { 40,   0},     // idle       | idle        | mspi1_cs        | gpio[40]   | reserve   | i_adc_test_d[10]  | reserve  | reserve  |
    { 41,   0},     // idle       | idle        | mspi1_clk       | gpio[41]   | reserve   | i_adc_test_d[11]  | reserve  | reserve  |
    { 42,   0},     // idle       | idle        | mspi1_mosi      | gpio[42]   | reserve   | i_adc_test_d[12]  | reserve  | reserve  |
    { 43,   0},     // idle       | idle        | mspi1_miso      | gpio[43]   | reserve   | i_adc_test_d[13]  | reserve  | reserve  |
    { 44,   0},     // idle       | idle        | mspi1_wp        | gpio[44]   | reserve   | reserve           | reserve  | reserve  |
    { 45,   46},     // idle       | idle        | mspi1_hold      | gpio[45]   | reserve   | reserve           | reserve  | reserve  |
    { 46,   0},     // idle       | idle        | iodma[8]        | gpio[46]   | reserve   | reserve           | reserve  | reserve  |
    { 47,   0},     // idle       | idle        | idle            | gpio[47]   | reserve   | reserve           | reserve  | reserve  |
};

int pinmux_set(int pin_id, int function)
{
	return padmux_set(pin_id, function);
}

int pinmux_get(int pin_id)
{
	return padmux_get(pin_id);
}

void pinmux_init(void)
{
	padmux_init(pin_table, ARRAY_SIZE(pin_table));
}

