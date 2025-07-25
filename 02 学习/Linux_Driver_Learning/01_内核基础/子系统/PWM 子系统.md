


# Example

## test
- sysfs
```shell
$ echo 3 > /sys/class/pwm/pwmchip0/export
$ ls /sys/class/pwm/pwmchip
pwmchip0/  pwmchip8/
$ ls /sys/class/pwm/pwmchip0/
device/     npwm        pwm3/       uevent
export      power/      subsystem/  unexport
$ ls /sys/class/pwm/pwmchip0/pwm3/
capture     enable      polarity    uevent
duty_cycle  period      power
$ cd /sys/class/pwm/pwmchip0/pwm3/
$ cat enable
0
$ cat period
0
$ echo 10000 > period
$ echo 4000 > duty_cycle
$ echo 1 > enable
$ echo 0 > enable

```



- pwm-regulator
```shell
$ cd /sys/devices/platform/
$ ls
Fixed MDIO bus.0         f6e40000.etm             fcc00000.dma-controller
alarmtimer               f6f10000.debug           fd000000.eth
arm-pmu                  f6f40000.etm             firmware:android
cpu_vol_user_ctl         f8000000.dw_sdhci        firmware:optee
cpufreq-dt               f8400000.spi             gpu_vol_user_ctl
cstfunnel_cluster        f9400000.usb             gx_mem_info
f0400000.sram            f9800000.usb             gxosal
f1000000.ddr-monitor     fa400000.gpio0           power
f1200000.noc             fa401000.gpio1           psci
f5c00000.gpu             fa402000.gpio2           pwm_regulator0
f6840000.csetr           fa403000.gpio3           pwm_regulator1
f6850000.catu            fa420000.gx_irr          pwm_regulator2
f6870000.cstfunnel_main  fa431000.timer           reg-dummy
f68a0000.csetf           fa441000.timer           regulatory.0
f68c0000.cstmc_cluster   fa450000.watchdog        serial8250
f68f0000.csstm           fa460000.rtc             snd-soc-dummy
f6c10000.debug           fc800000.dw_i2c          test_vol_user_ctl
f6c40000.etm             fc810000.dw_i2c          timer
f6d10000.debug           fc820000.dw_i2c          uevent
f6d40000.etm             fc830000.dw_i2c
f6e10000.debug           fc880000.serial

$ cd test_vol_user_ctl/
$ ls
driver           name             state            voltage
driver_override  of_node          subsystem
modalias         power            uevent
$ cat name
test_vol_userspace
$ cat voltage
1020000
$ cat state
disabled
$ echo 900000 > voltage
$ echo 1 > state
$ echo 840000 > voltage
$ echo 0 > state


$ cd cpu_vol_user_ctl/
$ ls
driver           name             state            voltage
driver_override  of_node          subsystem
modalias         power            uevent
$ cat voltage
1000000
$ echo 910000 > voltage
$ echo 1000000 > voltage
$ echo 910000 > voltage

```
