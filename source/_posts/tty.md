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

终端线路规程：
线路规程操作
```c
struct tty_ldisc_ops {
        int     magic;
        char    *name;
        int     num;
        int     flags;

        /*
         * The following routines are called from above.
         */
        int     (*open)(struct tty_struct *);
        void    (*close)(struct tty_struct *);
        void    (*flush_buffer)(struct tty_struct *tty);
        ssize_t (*read)(struct tty_struct *tty, struct file *file,
                        unsigned char __user *buf, size_t nr);
        ssize_t (*write)(struct tty_struct *tty, struct file *file,
                         const unsigned char *buf, size_t nr);
        int     (*ioctl)(struct tty_struct *tty, struct file *file,
                         unsigned int cmd, unsigned long arg);
        long    (*compat_ioctl)(struct tty_struct *tty, struct file *file,
                                unsigned int cmd, unsigned long arg);
        void    (*set_termios)(struct tty_struct *tty, struct ktermios *old);
        unsigned int (*poll)(struct tty_struct *, struct file *,
                             struct poll_table_struct *);
        int     (*hangup)(struct tty_struct *tty);

        /*
         * The following routines are called from below.
         */
        void    (*receive_buf)(struct tty_struct *, const unsigned char *cp,
                               char *fp, int count);
        void    (*write_wakeup)(struct tty_struct *);
        void    (*dcd_change)(struct tty_struct *, unsigned int);
        int     (*receive_buf2)(struct tty_struct *, const unsigned char *cp,
                                char *fp, int count);

        struct  module *owner;

        int refcount;
};
```
