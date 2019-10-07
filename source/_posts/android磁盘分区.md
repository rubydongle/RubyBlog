---
title: android磁盘分区
categories:
  - 技术文章
date: 2019-10-07 17:18:16
tags:
---

# 磁盘设备
PD1829:/ # ls /dev/block/
bootdevice loop7    ram2 sda10 sda6 sdd   sde14 sde23 sde5 sdf5
by-name    platform ram3 sda11 sda7 sdd1  sde15 sde24 sde6 vold
dm-0       ram0     ram4 sda12 sda8 sdd2  sde16 sde25 sde7 zram0
loop0      ram1     ram5 sda13 sda9 sdd3  sde17 sde26 sde8
loop1      ram10    ram6 sda14 sdb  sde   sde18 sde27 sde9
loop2      ram11    ram7 sda15 sdb1 sde1  sde19 sde28 sdf
loop3      ram12    ram8 sda2  sdb2 sde10 sde2  sde29 sdf1
loop4      ram13    ram9 sda3  sdc  sde11 sde20 sde3  sdf2
loop5      ram14    sda  sda4  sdc1 sde12 sde21 sde30 sdf3
loop6      ram15    sda1 sda5  sdc2 sde13 sde22 sde4  sdf4

可以通过Andorid自带的parted工具进行操作,查看分区以及作用。
```bash
PD1901:/ # parted /dev/block/mmcblk0
GNU Parted 2.4
Using /dev/block/mmcblk0
Welcome to GNU Parted! Type 'help' to view a list of commands.
(parted) print
print
Model: MMC DV6DMB (sd/mmc)
Disk /dev/block/mmcblk0: 125GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt

Number  Start   End     Size    File system  Name       Flags
 1      32.8kB  1081kB  1049kB               boot_para
 2      1081kB  68.2MB  67.1MB               recovery
 3      68.2MB  68.7MB  524kB                para
 4      68.7MB  89.7MB  21.0MB               expdb
 5      89.7MB  90.7MB  1049kB               frp
 6      90.7MB  124MB   33.6MB  ext4         nvcfg
 7      124MB   191MB   67.1MB  ext4         nvdata
 8      191MB   202MB   10.5MB               backup
 9      202MB   269MB   67.1MB               survival
10      269MB   279MB   10.5MB               reserved
11      279MB   347MB   67.1MB  ext2         cust
12      347MB   380MB   33.6MB               metadata
13      380MB   389MB   8389kB  ext4         protect1
14      389MB   403MB   14.1MB  ext4         protect2
15      403MB   411MB   8389kB               seccfg
16      411MB   461MB   50.3MB  ext4         persist
17      461MB   463MB   2097kB               sec1
18      463MB   467MB   3146kB               proinfo
19      467MB   467MB   524kB                efuse
20      467MB   572MB   105MB                md1img
21      572MB   573MB   1049kB               spmfw
22      573MB   574MB   1049kB               scp1
23      574MB   575MB   1049kB               scp2
24      575MB   576MB   1049kB               sspm_1
25      576MB   577MB   1049kB               sspm_2
26      577MB   594MB   16.8MB               gz1
27      594MB   611MB   16.8MB               gz2
28      611MB   678MB   67.1MB               nvram
29      678MB   683MB   5243kB               lk
30      683MB   688MB   5243kB               lk2
31      688MB   755MB   67.1MB               boot
32      755MB   764MB   8389kB               logo
33      764MB   772MB   8389kB               dtbo
34      772MB   778MB   5243kB               tee1
35      778MB   789MB   11.0MB               tee2
36      789MB   2399MB  1611MB  ext2         vendor
37      2399MB  7768MB  5369MB  ext2         system
38      7768MB  7776MB  8389kB               vbmeta
39      7776MB  8045MB  268MB   ext4         cache
40      8045MB  125GB   117GB                userdata
41      125GB   125GB   45.1MB               otp
42      125GB   125GB   16.8MB               flashinfo
```

通过dd可以将特定分区的内容读取出来：
```
130|PD1901:/ # dd if=/dev/block/mmcblk0p42 of=/data/local/tmp/flashinfo
```

挂在文件系统
```bash
mkdir userd
127|PD1901:/data/local/tmp/userd # mount -o loop -t ext4 userdata userd
PD1901:/data/local/tmp/userd # ls
backup backup_stage lost+found recovery
```
