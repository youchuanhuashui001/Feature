#waiting

- nor flash 擦除大小为 64KB
- 分区时大小和地址都要 64KB 对齐，但事业部经常会只分 4KB，或 512B，这种情况下擦除分区会影响相邻分区。
- linux mtd 要求分区大小是  erase_size，在 64KB 擦除时，只分4KB 的分区就会是 Read only，但要确保 rootfs 分区是 64KB 对齐；

- 如果linux 打开了 4K 擦除的选项，那么 mtd 分区就可以 以 4K 对齐了