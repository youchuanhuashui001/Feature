# aSan 工具

## 简介
aSan (AddressSanitizer) 是 Google 开发的一种快速内存错误检测工具，可以帮助开发者发现内存越界访问、内存泄漏等问题。详细信息请参考 [aSan 项目主页](https://github.com/google/sanitizers/)。
## 使用方法

1. **工具链选择**  
    * 当前使用的编译工具链是 `arm-linux-uclibcgnueabihf-`，但由于 uClibc 缺少许多 aSan 需要的函数，因此需要切换到基于 libc 的 `arm-linux-gnueabihf-` 工具链。  
2. **重新编译**  
    * 工具链更换后，GoXceed 和 Solution 中的所有代码需要重新编译。
    * 编译时为需要检测的文件添加编译参数：`-fsanitize=address -g`  (注意：不能和 `-static` 参数同时使用。)
3. **动态库配置**
    * 运行程序之前，需要将 ELF 文件所需的动态库拷贝到根文件系统的 `lib` 目录中。  
    * `arm-linux-gnueabihf-` 工具链中没有 `ldd` 命令，可以使用 `arm-linux-uclibcgnueabihf-ldd` 查看所需的动态库。  
4. **禁用特定功能**  
    * 如果不需要检查 `read` 越界问题，可以参考[相关解决方案](https://github.com/google/sanitizers/issues/752)。  
5. **运行程序**  
    * 编译完成后，直接运行程序，aSan 将实时检测内存错误。 
    * 如果运行 `.elf` 文件时提示 `./test: not found`，可能是动态库路径有误。  
    * 如果提示 `Permission denied`，请检查动态库是否具有可执行权限。

## 示例
### 示例 1：简单内存错误检测

代码示例： 
```c
#include <stdio.h>
#include <stdlib.h>

int main()
{
    char *p = NULL;
    printf("Hello world!\n");

    p = malloc(100);
    *p = 20;
    printf("*p: 0x%x\n", *p);
    free(p);
    *p = 18;
    printf("*p: 0x%x\n", *p);

    return 0;
}
```
编译命令：
```shell
arm-linux-gnueabihf-gcc test.c -o test -fsanitize=address -g
```
运行结果：
![[Pasted image 20250116111715.png]]

### 示例 2：在 solution 中使用 aSan
#### 修改部分：
以下是对 `solution/scripts/inc.Makefile.conf.mak` 文件的修改：  
```diff
- LDFLAGS += -static
+ LIBS += -ldl

deps: $(SRC)
    @echo "Generating new dependency file...";
    @-rm -f deps;
    @for f in $(SRC); do \
        OBJ=$(GXSRC_PATH)/output/objects/`basename $$f|sed -e 's/\.c/\.o/'`; \
        echo $$OBJ: $$f>> deps; \
        echo '    @echo -e "compiling \033[032m[$(CC)]\033[0m": ' $$f >> deps; \
+       if [ `basename $$f` = "app.c" ] || \
+          [ `basename $$f` = "app_config.c" ] || \
+          [ `basename $$f` = "app_file_list.c" ] || \
+          [ `basename $$f` = "app_full_screen.c" ] || \
+          [ `basename $$f` = "app_main_menu.c" ] || \
+          [ `basename $$f` = "app_sysinfo.c" ] || \
+          [ `basename $$f` = "app_system_setting_opt.c" ] || \
+          [ `basename $$f` = "app_upgrade. c" ] || \
+          [ `basename $$f` = "full_screen. c" ] || \
+          [ `basename $$f` = "main. c" ] || \
+          [ `basename $$f` = "app_net_upgrade_protocol_file. c" ] || \
+          [ `basename $$f` = "app_net_upgrade_protocol_ftp. c" ] || \
+          [ `basename $$f` = "app_net_upgrade_protocol_http. c" ] || \
+          [ `basename $$f` = "app_net_upgrade_protocol_https. c" ] || \
+          [ `basename $$f` = "app_net_upgrade. c" ] || \
+          [ `basename $$f` = "app_net_upgrade_common. c" ] || \
+          [ `basename $$f` = "app_net_upgrade_menu. c" ] || \
+          [ `basename $$f` = "app_net_upgrade_xml_parse. c" ]; then \
-           echo '    $(CC) $$(CFLAGS) -c -o $$@ $$^'>> deps; \
+           echo '    $(CC) $$(CFLAGS) -fsanitize=address -g -c -o $$@ $$^'>> deps; \
        else \
            echo '    $(CC) $$(CFLAGS) -c -o $$@ $$^'>> deps; \
        fi \
    done

$(BIN): $(OBJS)
    @echo "LDFLAGS: "$(LDFLAGS)
-   $(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@ -Wl,-Map,$(GXSRC_PATH)/output/$(BIN). map
+   $(LD) $(OBJS) $(LDFLAGS) $(LIBS) -fsanitize=address -g -o $@ -Wl,-Map,$(GXSRC_PATH)/output/$(BIN). map
    @mv $@ $(GXSRC_PATH)/output
```
#### 运行结果 ：
```
Part: DATA, start addr=0x00f10000, total_size=0x001f0000, offset=0x001ef000
=================================================================
==821==ERROR: AddressSanitizer: heap-buffer-overflow on address 0xab10fba4 at pc 0xb6aca85b bp 0xb52cdd20 sp 0xb52cdd24
READ of size 4096 at 0xab10fba4 thread T33 (flash_update_pa)
    #0 0xb6aca859 in __interceptor_write /home/tcwg-buildslave/workspace/tcwg-make-release/label/docker-trusty-i386-tcwg-build/target/arm-linux-gnueabihf/snapshots/gcc-linaro-4.9-2013

0xab10fba4 is located 0 bytes to the right of 676-byte region [0xab10f900,0xab10fba4)
allocated by thread T33 (flash_update_pa) here:
    #0 0xb6aeca8b in __interceptor_malloc /home/tcwg-buildslave/workspace/tcwg-make-release/label/docker-trusty-i386-tcwg-build/target/arm-linux-gnueabihf/snapshots/gcc-linaro-4.9-206

Thread T33 (flash_update_pa) created by T16 (GuiViewConsoleS) here:
    #0 0xb6ac6911 in __interceptor_pthread_create /home/tcwg-buildslave/workspace/tcwg-make-release/label/docker-trusty-i386-tcwg-build/target/arm-linux-gnueabihf/snapshots/gcc-linar3

Thread T16 (GuiViewConsoleS) created by T2 (init) here:
    #0 0xb6ac6911 in __interceptor_pthread_create /home/tcwg-buildslave/workspace/tcwg-make-release/label/docker-trusty-i386-tcwg-build/target/arm-linux-gnueabihf/snapshots/gcc-linar3

Thread T2 (init) created by T1 here:
    #0 0xb6ac6911 in __interceptor_pthread_create /home/tcwg-buildslave/workspace/tcwg-make-release/label/docker-trusty-i386-tcwg-build/target/arm-linux-gnueabihf/snapshots/gcc-linar3

Thread T1 created by T0 here:
    #0 0xb6ac6911 in __interceptor_pthread_create /home/tcwg-buildslave/workspace/tcwg-make-release/label/docker-trusty-i386-tcwg-build/target/arm-linux-gnueabihf/snapshots/gcc-linar3

SUMMARY: AddressSanitizer: heap-buffer-overflow /home/tcwg-buildslave/workspace/tcwg-make-release/label/docker-trusty-i386-tcwg-build/target/arm-linux-gnueabihf/snapshots/gcc-linaro-e
Shadow bytes around the buggy address:
  0x35621f20: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x35621f30: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x35621f40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x35621f50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x35621f60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x35621f70: 00 00 00 00[04]fa fa fa fa fa fa fa fa fa fa fa
  0x35621f80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x35621f90: fa fa fa[   40.710510] 
[   40.710510] [VDEC-ERR] videodisp_close:514                                                           
 fa fa fa fa fa fa fa fa fa fa fa fa fa                                                                 
  0x35621fa0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa                                           
  0x35621fb0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa                                           
  0x35621fc0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Heap right redzone:      fb
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack partial redzone:   f4
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Contiguous container OOB:fc
  ASan internal:           fe
==821==ABORTING
```

