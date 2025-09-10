set *(volatile unsigned long*)0xfa490080 = 0x2         // CS-->0

set *(volatile unsigned long*)0xf8400008 = 0x0         // SSIENR
set *(volatile unsigned long*)0xf8400000 = 0x80000C07  // CTRLR0
set *(volatile unsigned long*)0xf8400004 = 0x2         // CTRLR1
set *(volatile unsigned long*)0xf8400014 = 0x4         // BAUDR
set *(volatile unsigned long*)0xf8400018 = 0x0         // TXFTLR
set *(volatile unsigned long*)0xf84000F4 = 0x0         // SPI_CTRLR0
set *(volatile unsigned long*)0xf8400008 = 0x1         // SSIENR
set *(volatile unsigned long*)0xf8400010 = 0x1         // SER
set *(volatile unsigned long*)0xf8400060 = 0x9F        // DR
