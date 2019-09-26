---
title: tty
categories:
  - 技术文章
date: 2019-09-26 16:34:55
tags:
---

# ttyprintk伪终端
从sysfs中可以看到有一个名叫ttyprintk的伪终端设备。
```bash
ruby@spiderman:~$ cat /sys/class/tty/ttyprintk/uevent 
MAJOR=5
MINOR=3
DEVNAME=ttyprintk
```
该伪设备实现在linux内核代码的drivers/char/ttyprintk.c
该伪设备提供了printk打印日志的功能。  
在内核中通过printk打印日志是通过这个伪终端设备实现的,可以直接向ttyprintk设备文件写入内容，将log写入到kernel的log系统中。
```bash
root@spiderman:~# echo "hello world">/dev/ttyprintk
ruby@spiderman:~$ dmesg|tail
[ 6063.717497] rfkill: input handler enabled
[ 6089.724255] rfkill: input handler disabled
[ 6093.387465] rfkill: input handler enabled
[ 6094.890316] rfkill: input handler disabled
[24278.435621] [U] hello world
```

伪设备

初始化了端口
        tty_port_init(&tpk_port.port);
        tpk_port.port.ops = &null_ops;

        tty_port_link_device(&tpk_port.port, ttyprintk_driver, 0); 
