- SPI_CTRLR0 在 tx 的时候配成了 8bit 地址，rx 的时候没有配回去，导致在读 id 的时候读错，实际为 0x204018，读成了 0x401820

- 需要确认 linux 中的 spi 驱动和 loader 的 spi 驱动和 ecos 的 spi 驱动

