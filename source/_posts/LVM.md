---
title: LVM
categories:
  - 技术文章
date: 2019-09-15 22:55:47
tags:
---

# LVM概述
LVM(Logical Volume Manager)是逻辑卷管理的简称，它是对磁盘分区进行管理的一种机制。  
现在有两个Linux版本的LVM,分别是 LVM1,LVM2。  
LVM1是一种已经被认为稳定了几年的成熟产品，LVM2 是最新最好的LVM版本。 
LVM2几乎完全向后兼容使用LVM1创建的卷，除了快照(在升级到LVM 2之前，必须删除快照卷)。  

在Linux上磁盘设备通过设备节点/dev/hdX,/dev/sdX等暴露到用户空间。  
传统的磁盘使用固定大小的分区，分区后的设备节点为/dev/h(s)dX1,/dev/h(s)dX2等，其后面的数字表示分区号。  
```bash
ruby@batman:~$ ls -al /dev/sd*
brw-rw---- 1 root disk 8,  0 9月  22  2019 /dev/sda
brw-rw---- 1 root disk 8,  1 9月  22  2019 /dev/sda1
brw-rw---- 1 root disk 8,  2 9月  22  2019 /dev/sda2
brw-rw---- 1 root disk 8,  3 9月  22  2019 /dev/sda3
brw-rw---- 1 root disk 8, 16 9月  22  2019 /dev/sdb
brw-rw---- 1 root disk 8, 17 9月  22  2019 /dev/sdb1
brw-rw---- 1 root disk 8, 18 9月  22  2019 /dev/sdb2
brw-rw---- 1 root disk 8, 19 9月  22  2019 /dev/sdb3
brw-rw---- 1 root disk 8, 20 9月  22  2019 /dev/sdb4
brw-rw---- 1 root disk 8, 32 9月  22  2019 /dev/sdc
brw-rw---- 1 root disk 8, 33 9月  22  2019 /dev/sdc1
```
我的电脑上安装了三块硬盘，分别是sda、sdb、sdc，其中sda上有4个分区sda1、sda2、sda3。  
每块磁盘可以分区成四个以内的主分区,或者3个主分区和一个扩展分区，而这个扩展分区又可以分成若干个逻辑分区。  
主分区、扩展分区、逻辑分区之间的区别可以参考文章[硬盘主分区，扩展分区和逻辑分区之间的区别介绍](https://zhidao.baidu.com/question/620811777413948052.html)
通过fdisk可以对磁盘进行管理。  
```bash
ruby@batman:~$ sudo fdisk /dev/sda

欢迎使用 fdisk (util-linux 2.31.1)。
更改将停留在内存中，直到您决定将更改写入磁盘。
使用写入命令前请三思。


命令(输入 m 获取帮助)： p
Disk /dev/sda：465.8 GiB，500107862016 字节，976773168 个扇区
单元：扇区 / 1 * 512 = 512 字节
扇区大小(逻辑/物理)：512 字节 / 512 字节
I/O 大小(最小/最佳)：512 字节 / 512 字节
磁盘标签类型：dos
磁盘标识符：0xb0e76c31

设备       启动      起点      末尾     扇区  大小 Id 类型
/dev/sda1            2048  97867775 97865728 46.7G 83 Linux
/dev/sda2  *     97867776 136929279 39061504 18.6G 83 Linux
/dev/sda3       136929280 174678015 37748736   18G 82 Linux swap / Solaris

命令(输入 m 获取帮助)：
```
磁盘sda有456GiB空间，分区情况通过p打印出来。我们没有使用全部的空间，只是使用了不到100G创建了3个主分区。  
后面可以通过n命令使用未分配的空间创建新的分区。  
需要注意的是我们已经使用完了3个主分区号，此时只能创建一个扩展分区(最好包括剩下的所有空间),在这个扩展分区中创建若干个逻辑分区。  

传统的分区方法创建的分区有如下问题,LVM能完美解决：
1.硬盘在分区完成后想重新调整大小十分麻烦。
> 比如我们对sda有两个分区sda4、sda5分别是10G空间，在使用了一段时间后发现sda4分区不够用了，sda5分区还没有使用,这时想把sda5和sda6合并成一个分区就很麻烦。  

2.不能突破物理硬盘空间大小的限制创建分区。
> 如果有两个磁盘分别是500G，那么我们创建的分区空间大小必定小于500G。  

# LVM中的概念

# 使用LVM
## 创建分区
1.将剩下的空间都划分到扩展分区中去
```bash
ruby@batman:~$ sudo fdisk /dev/sda

欢迎使用 fdisk (util-linux 2.31.1)。
更改将停留在内存中，直到您决定将更改写入磁盘。
使用写入命令前请三思。


命令(输入 m 获取帮助)： p
Disk /dev/sda：465.8 GiB，500107862016 字节，976773168 个扇区
单元：扇区 / 1 * 512 = 512 字节
扇区大小(逻辑/物理)：512 字节 / 512 字节
I/O 大小(最小/最佳)：512 字节 / 512 字节
磁盘标签类型：dos
磁盘标识符：0xb0e76c31

设备       启动      起点      末尾     扇区  大小 Id 类型
/dev/sda1            2048  97867775 97865728 46.7G 83 Linux
/dev/sda2  *     97867776 136929279 39061504 18.6G 83 Linux
/dev/sda3       136929280 174678015 37748736   18G 82 Linux swap / Solaris

命令(输入 m 获取帮助)： n
分区类型
   p   主分区 (3个主分区，0个扩展分区，1空闲)
   e   扩展分区 (逻辑分区容器)
选择 (默认 e)： e

已选择分区 4
第一个扇区 (174678016-976773167, 默认 174678016): 
上个扇区，+sectors 或 +size{K,M,G,T,P} (174678016-976773167, 默认 976773167): 

创建了一个新分区 4，类型为“Extended”，大小为 382.5 GiB。

命令(输入 m 获取帮助)： p
Disk /dev/sda：465.8 GiB，500107862016 字节，976773168 个扇区
单元：扇区 / 1 * 512 = 512 字节
扇区大小(逻辑/物理)：512 字节 / 512 字节
I/O 大小(最小/最佳)：512 字节 / 512 字节
磁盘标签类型：dos
磁盘标识符：0xb0e76c31

设备       启动      起点      末尾      扇区   大小 Id 类型
/dev/sda1            2048  97867775  97865728  46.7G 83 Linux
/dev/sda2  *     97867776 136929279  39061504  18.6G 83 Linux
/dev/sda3       136929280 174678015  37748736    18G 82 Linux swap / Solaris
/dev/sda4       174678016 976773167 802095152 382.5G  5 扩展

命令(输入 m 获取帮助)：

```
上面的操作创建了扩展分区sda4，大小为382.G,扩展分区可以直接格式化文件系统，然后挂载了,但是我们不是直接使用。

2.在扩展分区创建两个逻辑分区
针对扩增分区的382.5G空间,创建两个100G的逻辑分区sda5和sda6，后面针对sda5和sda6进行LVM管理。  
```bash
ruby@batman:~$ sudo fdisk /dev/sda

欢迎使用 fdisk (util-linux 2.31.1)。
更改将停留在内存中，直到您决定将更改写入磁盘。
使用写入命令前请三思。


命令(输入 m 获取帮助)： p
Disk /dev/sda：465.8 GiB，500107862016 字节，976773168 个扇区
单元：扇区 / 1 * 512 = 512 字节
扇区大小(逻辑/物理)：512 字节 / 512 字节
I/O 大小(最小/最佳)：512 字节 / 512 字节
磁盘标签类型：dos
磁盘标识符：0xb0e76c31

设备       启动      起点      末尾      扇区   大小 Id 类型
/dev/sda1            2048  97867775  97865728  46.7G 83 Linux
/dev/sda2  *     97867776 136929279  39061504  18.6G 83 Linux
/dev/sda3       136929280 174678015  37748736    18G 82 Linux swap / Solaris
/dev/sda4       174678016 976773167 802095152 382.5G  5 扩展

命令(输入 m 获取帮助)： n
所有主分区都在使用中。
添加逻辑分区 5
第一个扇区 (174680064-976773167, 默认 174680064): 
上个扇区，+sectors 或 +size{K,M,G,T,P} (174680064-976773167, 默认 976773167): +100G

创建了一个新分区 5，类型为“Linux”，大小为 100 GiB。

命令(输入 m 获取帮助)： n
所有主分区都在使用中。
添加逻辑分区 6
第一个扇区 (384397312-976773167, 默认 384397312): 
上个扇区，+sectors 或 +size{K,M,G,T,P} (384397312-976773167, 默认 976773167): +100G

创建了一个新分区 6，类型为“Linux”，大小为 100 GiB。

命令(输入 m 获取帮助)： p
Disk /dev/sda：465.8 GiB，500107862016 字节，976773168 个扇区
单元：扇区 / 1 * 512 = 512 字节
扇区大小(逻辑/物理)：512 字节 / 512 字节
I/O 大小(最小/最佳)：512 字节 / 512 字节
磁盘标签类型：dos
磁盘标识符：0xb0e76c31

设备       启动      起点      末尾      扇区   大小 Id 类型
/dev/sda1            2048  97867775  97865728  46.7G 83 Linux
/dev/sda2  *     97867776 136929279  39061504  18.6G 83 Linux
/dev/sda3       136929280 174678015  37748736    18G 82 Linux swap / Solaris
/dev/sda4       174678016 976773167 802095152 382.5G  5 扩展
/dev/sda5       174680064 384395263 209715200   100G 83 Linux
/dev/sda6       384397312 594112511 209715200   100G 83 Linux

命令(输入 m 获取帮助)： w
分区表已调整。
正在同步磁盘。
```
要想sda5和sda6支持LVM，需要改变类型将Id改为8e(Linux LVM)。  
```bash
ruby@batman:~$ sudo fdisk /dev/sda

欢迎使用 fdisk (util-linux 2.31.1)。
更改将停留在内存中，直到您决定将更改写入磁盘。
使用写入命令前请三思。


命令(输入 m 获取帮助)： p
Disk /dev/sda：465.8 GiB，500107862016 字节，976773168 个扇区
单元：扇区 / 1 * 512 = 512 字节
扇区大小(逻辑/物理)：512 字节 / 512 字节
I/O 大小(最小/最佳)：512 字节 / 512 字节
磁盘标签类型：dos
磁盘标识符：0xb0e76c31

设备       启动      起点      末尾      扇区   大小 Id 类型
/dev/sda1            2048  97867775  97865728  46.7G 83 Linux
/dev/sda2  *     97867776 136929279  39061504  18.6G 83 Linux
/dev/sda3       136929280 174678015  37748736    18G 82 Linux swap / Solaris
/dev/sda4       174678016 976773167 802095152 382.5G  5 扩展
/dev/sda5       174680064 384395263 209715200   100G 83 Linux
/dev/sda6       384397312 594112511 209715200   100G 83 Linux

命令(输入 m 获取帮助)： t
分区号 (1-6, 默认  6): 
Hex 代码(输入 L 列出所有代码)： L

 0  空              24  NEC DOS         81  Minix / 旧 Linu bf  Solaris        
 1  FAT12           27  隐藏的 NTFS Win 82  Linux swap / So c1  DRDOS/sec (FAT-
 2  XENIX root      39  Plan 9          83  Linux           c4  DRDOS/sec (FAT-
 3  XENIX usr       3c  PartitionMagic  84  OS/2 隐藏 或 In c6  DRDOS/sec (FAT-
 4  FAT16 <32M      40  Venix 80286     85  Linux 扩展      c7  Syrinx         
 5  扩展            41  PPC PReP Boot   86  NTFS 卷集       da  非文件系统数据 
 6  FAT16           42  SFS             87  NTFS 卷集       db  CP/M / CTOS / .
 7  HPFS/NTFS/exFAT 4d  QNX4.x          88  Linux 纯文本    de  Dell 工具      
 8  AIX             4e  QNX4.x 第2部分  8e  Linux LVM       df  BootIt         
 9  AIX 可启动      4f  QNX4.x 第3部分  93  Amoeba          e1  DOS 访问       
 a  OS/2 启动管理器 50  OnTrack DM      94  Amoeba BBT      e3  DOS R/O        
 b  W95 FAT32       51  OnTrack DM6 Aux 9f  BSD/OS          e4  SpeedStor      
 c  W95 FAT32 (LBA) 52  CP/M            a0  IBM Thinkpad 休 ea  Rufus 对齐     
 e  W95 FAT16 (LBA) 53  OnTrack DM6 Aux a5  FreeBSD         eb  BeOS fs        
 f  W95 扩展 (LBA)  54  OnTrackDM6      a6  OpenBSD         ee  GPT            
10  OPUS            55  EZ-Drive        a7  NeXTSTEP        ef  EFI (FAT-12/16/
11  隐藏的 FAT12    56  Golden Bow      a8  Darwin UFS      f0  Linux/PA-RISC  
12  Compaq 诊断     5c  Priam Edisk     a9  NetBSD          f1  SpeedStor      
14  隐藏的 FAT16 <3 61  SpeedStor       ab  Darwin 启动     f4  SpeedStor      
16  隐藏的 FAT16    63  GNU HURD 或 Sys af  HFS / HFS+      f2  DOS 次要       
17  隐藏的 HPFS/NTF 64  Novell Netware  b7  BSDI fs         fb  VMware VMFS    
18  AST 智能睡眠    65  Novell Netware  b8  BSDI swap       fc  VMware VMKCORE 
1b  隐藏的 W95 FAT3 70  DiskSecure 多启 bb  Boot Wizard 隐  fd  Linux raid 自动
1c  隐藏的 W95 FAT3 75  PC/IX           bc  Acronis FAT32 L fe  LANstep        
1e  隐藏的 W95 FAT1 80  旧 Minix        be  Solaris 启动    ff  BBT            
Hex 代码(输入 L 列出所有代码)： 8e

已将分区“Linux”的类型更改为“Linux LVM”。

命令(输入 m 获取帮助)： t
分区号 (1-6, 默认  6): 5
Hex 代码(输入 L 列出所有代码)： 8e

已将分区“Linux”的类型更改为“Linux LVM”。

命令(输入 m 获取帮助)： p
Disk /dev/sda：465.8 GiB，500107862016 字节，976773168 个扇区
单元：扇区 / 1 * 512 = 512 字节
扇区大小(逻辑/物理)：512 字节 / 512 字节
I/O 大小(最小/最佳)：512 字节 / 512 字节
磁盘标签类型：dos
磁盘标识符：0xb0e76c31

设备       启动      起点      末尾      扇区   大小 Id 类型
/dev/sda1            2048  97867775  97865728  46.7G 83 Linux
/dev/sda2  *     97867776 136929279  39061504  18.6G 83 Linux
/dev/sda3       136929280 174678015  37748736    18G 82 Linux swap / Solaris
/dev/sda4       174678016 976773167 802095152 382.5G  5 扩展
/dev/sda5       174680064 384395263 209715200   100G 8e Linux LVM
/dev/sda6       384397312 594112511 209715200   100G 8e Linux LVM

命令(输入 m 获取帮助)： w
分区表已调整。
正在同步磁盘。
```
再次查看sda5和sda6的类型变成了"Linux LVM"。  
```bash
ruby@batman:~$ sudo fdisk -l /dev/sda
Disk /dev/sda：465.8 GiB，500107862016 字节，976773168 个扇区
单元：扇区 / 1 * 512 = 512 字节
扇区大小(逻辑/物理)：512 字节 / 512 字节
I/O 大小(最小/最佳)：512 字节 / 512 字节
磁盘标签类型：dos
磁盘标识符：0xb0e76c31

设备       启动      起点      末尾      扇区   大小 Id 类型
/dev/sda1            2048  97867775  97865728  46.7G 83 Linux
/dev/sda2  *     97867776 136929279  39061504  18.6G 83 Linux
/dev/sda3       136929280 174678015  37748736    18G 82 Linux swap / Solaris
/dev/sda4       174678016 976773167 802095152 382.5G  5 扩展
/dev/sda5       174680064 384395263 209715200   100G 8e Linux LVM
/dev/sda6       384397312 594112511 209715200   100G 8e Linux LVM
```

## 准备物理卷(PV)
上面创建的两个Linnux LVM类型的分区sda5、sda6。下面的命令将这两个分区作为物理卷。  
```bash
# 1.创建物理卷
ruby@batman:~$ sudo pvcreate /dev/sda5
  Physical volume "/dev/sda5" successfully created.
ruby@batman:~$ sudo pvcreate /dev/sda6
  Physical volume "/dev/sda6" successfully created.

# 2.查看物理卷
ruby@batman:~$ sudo pvdisplay 
  "/dev/sda6" is a new physical volume of "100.00 GiB"
  --- NEW Physical volume ---
  PV Name               /dev/sda6
  VG Name               
  PV Size               100.00 GiB
  Allocatable           NO
  PE Size               0   
  Total PE              0
  Free PE               0
  Allocated PE          0
  PV UUID               jMB66h-sS5R-zLbf-JxO1-2bHd-SUKx-p9zKsr
   
  "/dev/sda5" is a new physical volume of "100.00 GiB"
  --- NEW Physical volume ---
  PV Name               /dev/sda5
  VG Name               
  PV Size               100.00 GiB
  Allocatable           NO
  PE Size               0   
  Total PE              0
  Free PE               0
  Allocated PE          0
  PV UUID               SUZdHm-dNv0-dduX-tlED-R67G-uUz6-RxlHjP

# 3.当不需要一个物理卷时可以移除
ruby@batman:~$ sudo pvremove /dev/sda6
```
## 准备卷组
将上面创建的两个物理卷构成一个卷组vg1。  
```bash
# 1.创建卷组
ruby@batman:~$ sudo vgcreate vg1 /dev/sda5 /dev/sda6
  Volume group "vg1" successfully created

# 2.查看卷组信息
ruby@batman:~$ sudo vgdisplay
  --- Volume group ---
  VG Name               vg1
  System ID             
  Format                lvm2
  Metadata Areas        2
  Metadata Sequence No  1
  VG Access             read/write
  VG Status             resizable
  MAX LV                0
  Cur LV                0
  Open LV               0
  Max PV                0
  Cur PV                2
  Act PV                2
  VG Size               199.99 GiB
  PE Size               4.00 MiB
  Total PE              51198
  Alloc PE / Size       0 / 0   
  Free  PE / Size       51198 / 199.99 GiB
  VG UUID               dbHKlI-c3vX-Mmus-fbxd-N3o2-pb9P-Okh54d

# 3.可以用下面的卷组删除卷组
ruby@batman:~$ sudo vgremove vg1
  Volume group "vg1" successfully removed

```

上面创建的卷组有200G空间，物理卷给卷组提供空间，后面通过LVM创建的逻辑卷从卷组获取空间。  
只要这个卷组还有空间就能随意创建逻辑卷，即使卷组空间不足了也可以通过vgextend命令去扩展卷组。  

## 创建逻辑卷
我们创建一个大小为80G的逻辑卷lv1，逻辑卷的空间来源于卷组vg1。  
```bash
ruby@batman:~$ sudo lvcreate -L 80G -n lv1 vg1
  Logical volume "lv1" created.
ruby@batman:~$ sudo lvdisplay 
  --- Logical volume ---
  LV Path                /dev/vg1/lv1
  LV Name                lv1
  VG Name                vg1
  LV UUID                qPzeK7-gKJW-b2MG-xjuq-J4AB-xPnd-ZRT6PC
  LV Write Access        read/write
  LV Creation host, time batman, 2019-09-22 09:24:30 +0800
  LV Status              available
  # open                 0
  LV Size                80.00 GiB
  Current LE             20480
  Segments               1
  Allocation             inherit
  Read ahead sectors     auto
  - currently set to     256
  Block device           253:0
```
逻辑卷通过设备文件/dev/vg1/lv1暴露到用户空间，后面可以像使用普通磁盘分区一样使用逻辑卷。  
格式化文件系统、挂载等。  
```bash
ruby@batman:~$ sudo mkfs.ext4 /dev/vg1/lv1 
mke2fs 1.44.1 (24-Mar-2018)
创建含有 20971520 个块（每块 4k）和 5242880 个inode的文件系统
文件系统UUID：f55b4b6e-9329-425d-b583-89b05aeb2cc1
超级块的备份存储于下列块： 
	32768, 98304, 163840, 229376, 294912, 819200, 884736, 1605632, 2654208, 
	4096000, 7962624, 11239424, 20480000

正在分配组表： 完成                            
正在写入inode表： 完成                            
创建日志（131072 个块） 完成
写入超级块和文件系统账户统计信息： 已完成 

ruby@batman:~$ sudo mkdir /work
ruby@batman:~$ sudo mount /dev/vg1/lv1 /work/
ruby@batman:~$ mount|grep lv1
/dev/mapper/vg1-lv1 on /work type ext4 (rw,relatime)

```
将逻辑卷挂载到挂载点/work后，就可以进入/work目录使用了。  
当不再要使用时可以通过lvremove删除一个逻辑卷。  
```bash
ruby@batman:~$ sudo umount /work 
ruby@batman:~$ sudo lvremove /dev/vg1/lv1
Do you really want to remove and DISCARD active logical volume vg1/lv1? [y/n]: y
  Logical volume "lv1" successfully removed
```
将下面的内容加入/etc/fstab后系统能在开机后自动挂载逻辑卷到文件系统。  
```bash
# 创建挂载点/work
ruby@batman:~$ sudo mkdir /work
ruby@batman:~$ sudo chmod 777 /work

# 设置开机挂载
root@batman:~# echo "/dev/vg1/lv1 /work auto nosuid,nodev,nofail 0 0">> /etc/fstab

# 测试挂载
ruby@batman:~$ sudo mount /work
ruby@batman:/work$ df -h . 
文件系统             容量  已用  可用 已用% 挂载点
/dev/mapper/vg1-lv1   79G   57M   75G    1% /work

```






# 参考  
https://linux.cn/article-3218-1.html
