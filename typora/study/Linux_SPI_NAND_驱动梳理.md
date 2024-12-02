# Linux SPI NAND 驱动梳理



## 1. 驱动程序流程



```c
static const struct of_device_if spinand_of_table[] = {
    {.compatible = "spinand"},
    { }
};
MODULE_DEVICE_TABLE(of, spinand_of_table);

static struct spi_driver spi_nand_driver = {
    .probe = spinand_probe,
    .remove = spinand_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = spinand_of_table,
    },
};


module_spi_driver(spi_nand_driver);




```

​		

## 2. 数据结构：

​		在 probe 函数中组织好 mtd_info 结构，使用 `mtd_device_parse_register` 注册 mtd 设备

- ` struct mtd_device_parse_register`

```c
int mtd_device_parse_register(struct mtd_info *mtd, const char * const *types,
                        ┆     struct mtd_part_parser_data *parser_data,
                        ┆     const struct mtd_partition *parts,
                        ┆     int nr_parts)
{
        struct mtd_partitions parsed;
        int ret; 
                                                                                                               
        mtd_set_dev_defaults(mtd);

        memset(&parsed, 0, sizeof(parsed));

        ret = parse_mtd_partitions(mtd, types, &parsed, parser_data);
        if ((ret < 0 || parsed.nr_parts == 0) && parts && nr_parts) {
                /* Fall back to driver-provided partitions */
                parsed = (struct mtd_partitions){
                        .parts          = parts,
                        .nr_parts       = nr_parts,
                };   
        } else if (ret < 0) { 
                /* Didn't come up with parsed OR fallback partitions */
                pr_info("mtd: failed to find partitions; one or more parsers reports errors (%d)\n",
                        ret);
                /* Don't abort on errors; we can still use unpartitioned MTD */
                memset(&parsed, 0, sizeof(parsed));
        }    

        ret = mtd_add_device_partitions(mtd, &parsed);
        if (ret)
                goto out; 

        /*   
        ┆* FIXME: some drivers unfortunately call this function more than once.
        ┆* So we have to check if we've already assigned the reboot notifier.
        ┆*
        ┆* Generally, we can make multiple calls work for most cases, but it
        ┆* does cause problems with parse_mtd_partitions() above (e.g.,
        ┆* cmdlineparts will register partitions more than once).
        ┆*/
        WARN_ONCE(mtd->_reboot && mtd->reboot_notifier.notifier_call,
                ┆ "MTD already registered\n");
        if (mtd->_reboot && !mtd->reboot_notifier.notifier_call) {
                mtd->reboot_notifier.notifier_call = mtd_reboot_notifier;
                register_reboot_notifier(&mtd->reboot_notifier);
        }    

out:
        /* Cleanup any parsed partitions */
        mtd_part_parser_cleanup(&parsed);
        return ret;
}

```

​		SPI NAND 也需要分区表，而分区表是通过 CMD_LINE 信息传递过来的，所以要通过 `parse_mtd_partitions` 函数将 loader 传递的 CMD_LINE (包含 partition 信息)转换成 partition 结构，再使用 `mtd_add_device_partitions` 函数将转换完成的 partition 结构依次新建分区。





- `mtd_set_dev_defaults`

```c
static void mtd_set_dev_defaults(struct mtd_info *mtd)                                                                                                                                                                          
{
        if (mtd->dev.parent) {
                if (!mtd->owner && mtd->dev.parent->driver)
                        mtd->owner = mtd->dev.parent->driver->owner;
                if (!mtd->name)
                        mtd->name = dev_name(mtd->dev.parent);
        } else {
                pr_debug("mtd device won't show a device symlink in sysfs\n");
        }
}
```



- `parse_mtd_partitions`

```c
int parse_mtd_partitions(struct mtd_info *master, const char *const *types,
                        ┆struct mtd_partitions *pparts,
                        ┆struct mtd_part_parser_data *data)
{
        struct mtd_part_parser *parser;
        int ret, err = 0;

        if (!types)
                types = default_mtd_part_types;

        for ( ; *types; types++) {
                pr_debug("%s: parsing partitions %s\n", master->name, *types);
                parser = mtd_part_parser_get(*types);
                if (!parser && !request_module("%s", *types))
                        parser = mtd_part_parser_get(*types);
                pr_debug("%s: got parser %s\n", master->name,
                        ┆parser ? parser->name : NULL);
                if (!parser)
                        continue;
                ret = (*parser->parse_fn)(master, &pparts->parts, data);
                pr_debug("%s: parser %s: %i\n",
                        ┆master->name, parser->name, ret);
                if (ret > 0) {
                        printk(KERN_NOTICE "%d %s partitions found on MTD device %s\n",
                        ┆      ret, parser->name, master->name);
                        pparts->nr_parts = ret;
                        pparts->parser = parser;
                        return 0;
                }
                mtd_part_parser_put(parser);
                /*
                ┆* Stash the first error we see; only report it if no parser
                ┆* succeeds
                ┆*/
                if (ret < 0 && !err)
                        err = ret;
        }
        return err;
}

```

- `(*parser->parser_cn)`

```c
static struct mtd_part_parser cmdline_parser = {                                                                                                                                       
        .parse_fn = parse_cmdline_partitions,                                                                                                                                          
        .name = "cmdlinepart",                                                                                                                                                         
};
```



```c

```

