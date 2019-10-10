---
title: busybox
categories:
  - 技术文章
date: 2019-10-09 23:26:09
tags:
---

# 下载busybox
[BusyBox官网](https://busybox.net/)的[Download Source](https://git.busybox.net/busybox/)提供了下载代码的链接。
![img](/files/busybox/busyboxwebsite.png)
最新的busybox版本是1.31.0，我们使用该版本的Source Code进行构建。
在Linux终端可以通过下面的命令下载,并解压：
```bash
ruby@batman:/work$ wget https://busybox.net/downloads/busybox-1.31.0.tar.bz2
ruby@batman:/work$ tar xvf busybox-1.31.0.tar.bz2 
```
解压后里面的内容如下:
![img](/files/busybox/busyboxsourcedir.png)


# 构建编译环境
我们希望构建的busybox运行在arm环境下，这就需要arm交叉编译工具链gcc-arm-linux-gnueabi.
在Ubuntu上可以通过下面的命令进行安装
```bash
ruby@batman:~$ sudo apt-get install gcc-arm-linux-gnueabi
```

# 编译Busybox
由于我们希望构建运行在arm环境下的busybox，所以需要先设置交叉编译工具链：
```bash
ruby@batman:/work/busybox-1.31.0$ export ARCH=arm
ruby@batman:/work/busybox-1.31.0$ export CROSS_COMPILE=arm-linux-gnueabi-
ruby@batman:/work/busybox-1.31.0$ make menuconfig
```
![img](/files/busybox/busyboxcompileconfig.png)

我们希望编译出来的使用静态链接，进入Settings--->Build Options,将Build static binary (no shared libs) (NEW)选上,保存配置到.config文件。
![img](/files/busybox/busyboxcompileconfigstaticbinary.png)
当配置好后输入make命令就开始执行busybox的编译了,编译完成后执行make install将其安装到_install文件夹。
```bash
ruby@batman:/work/busybox-1.31.0$ make
......
  CC      util-linux/volume_id/volume_id.o
  CC      util-linux/volume_id/xfs.o
  AR      util-linux/volume_id/lib.a
  LINK    busybox_unstripped
Static linking against glibc, can't use --gc-sections
Trying libraries: crypt m resolv
 Library crypt is not needed, excluding it
 Library m is needed, can't exclude it (yet)
 Library resolv is needed, can't exclude it (yet)
Final link with: m resolv
  DOC     busybox.pod
  DOC     BusyBox.txt
  DOC     busybox.1
  DOC     BusyBox.html

ruby@batman:/work/busybox-1.31.0$ make install
......
  ./_install//usr/sbin/ubirsvol -> ../../bin/busybox
  ./_install//usr/sbin/ubiupdatevol -> ../../bin/busybox
  ./_install//usr/sbin/udhcpd -> ../../bin/busybox


--------------------------------------------------
You will probably need to make your busybox binary
setuid root to ensure all configured applets will
work properly.
--------------------------------------------------
ruby@batman:/work/busybox-1.31.0$ tree  _install/ -L 1
_install/
├── bin
├── linuxrc -> bin/busybox
├── sbin
└── usr

3 directories, 1 file
```

# 构建initramfs/initrd
rootfs下常见的目录有dev、etc、mnt
所以我们进入_install中创建这些目录
```bash
mkdir etc
mkdir dev
mkdir mnt
mkdir -p etc/init.d
```

在_install/etc/init.d目录下创建一个rcS文件,并写入如下内容
```bash
mkdir -p /proc
mkdir -p /tmp
mkdir -p /mnt
/bin/mount -a
mkdir -p /dev/pts
mount -t devpts devpts /dev/pts
echo /sbin/mdev > /proc/sys/kernel/hotplug
mdev -s
```
修改rcS的执行权限
```bash
chmod a+x rcS
```
在_install/etc目录下创建一个fstab文件,并写入如下内容
```
proc /proc proc defaults 0 0
tmpfs /tmp tmpfs defaults 0 0
sysfs /sys sysfs defaults 0 0
tmpfs /dev tmpfs defaults 0 0
debugfs /sys/kernel/debug debugfs defaults 0 0
```

在_install/etc目录下创建一个inittab文件,并写入如下内容
```bash
::sysinit:/etc/init.d/rcS
::respawn:-/bin/sh
::askfirst:-/bin/sh
::ctrlaltdel:/bin/umount -a -r
```
在_install/dev目录下创建设备节点
```bash
cd _install/dev
sudo mknod console c 5 1
sudo mknod null c 1 3
```

http://blog.17study.com.cn/2017/12/28/qemu-on-mac/
https://www.xilinx.com/support/documentation/sw_manuals/xilinx2018_2/ug1169-xilinx-qemu.pdf
