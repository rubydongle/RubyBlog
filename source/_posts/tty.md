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
串口终端：
参考：https://blog.csdn.net/wangjingyu00711/article/details/41648825

在boot loader 程序的设计与实现中，没有什么能够比从串口终端正确地收到打印信息能更令人激动了。
此外，向串口终端打印信息也是一个非常重要而又有效的调试手段。
但是，我们经常会碰到串口终端显示乱码或根本没有显示的问题。造成这个问题主要有两种原因：  
(1) boot loader 对串口的初始化设置不正确。
(2) 运行在 host 端的终端仿真程序对串口的设置不正确，这包括：波特率、奇偶校验、数据位和停止位等方面的设置。

此外，有时也会碰到这样的问题，那就是：在 boot loader 的运行过程中我们可以正确地向串口终端输出信息，但当 boot loader 启动内核后却无法看到内核的启动输出信息。对这一问题的原因可以从以下几个方面来考虑：
(1) 首先请确认你的内核在编译时配置了对串口终端的支持，并配置了正确的串口驱动程序。
(2) 你的 boot loader 对串口的初始化设置可能会和内核对串口的初始化设置不一致。此外，对于诸如 s3c44b0x 这样的 CPU，CPU 时钟频率的设置也会影响串口，因此如果 boot loader 和内核对其 CPU 时钟频率的设置不一致，也会使串口终端无法正确显示信息。
(3) 最后，还要确认 boot loader 所用的内核基地址必须和内核映像在编译时所用的运行基地址一致，尤其是对于 uClinux 而言。假设你的内核映像在编译时用的基地址是 0xc0008000，但你的 boot loader 却将它加载到 0xc0010000 处去执行，那么内核映像当然不能正确地执行了。

N_TTY线路规程为串行子系统提供了终端的支持。  
```bash
ruby@batman:~$ cat /proc/tty/drivers 
/dev/tty             /dev/tty        5       0 system:/dev/tty
/dev/console         /dev/console    5       1 system:console
/dev/ptmx            /dev/ptmx       5       2 system
/dev/vc/0            /dev/vc/0       4       0 system:vtmaster
dbc_serial           /dev/ttyDBC   241       0 serial
dbc_serial           /dev/ttyDBC   242       0 serial
ttyprintk            /dev/ttyprintk   5       3 console
max310x              /dev/ttyMAX   204 209-224 serial
serial               /dev/ttyS       4 64-111 serial
pty_slave            /dev/pts      136 0-1048575 pty:slave
pty_master           /dev/ptm      128 0-1048575 pty:master
unknown              /dev/tty        4 1-63 console
```
