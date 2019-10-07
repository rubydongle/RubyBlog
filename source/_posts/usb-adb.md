---
title: usb-adb
categories:
  - 技术文章
date: 2019-10-07 11:09:15
tags:
---


$ insmod g_ffs.ko idVendor=<ID> iSerialNumber=<string> functions=mtp,hid
$ mkdir /dev/ffs-mtp && mount -t functionfs mtp /dev/ffs-mtp
$ ( cd /dev/ffs-mtp && mtp-daemon ) &
$ mkdir /dev/ffs-hid && mount -t functionfs hid /dev/ffs-hid
$ ( cd /dev/ffs-hid && hid-daemon ) &

PD1829:/dev/usb-ffs/adb # ls
ep0 ep1 ep2

mount -t functions adb /dev/usb-ffs/adb

130|PD1829:/dev/usb-ffs/adb # mount|grep adb
adb on /dev/usb-ffs/adb type functionfs (rw,relatime)


drivers/usb/gadget/functions.c
drivers/usb/gadget/function/f_fs.c


system/core/adb/daemon/usb.cpp
```c
static void usb_ffs_open_thread(void* x) {
    struct usb_handle* usb = (struct usb_handle*)x;

    adb_thread_setname("usb ffs open");

    while (true) {
        // wait until the USB device needs opening
        std::unique_lock<std::mutex> lock(usb->lock);
        while (!usb->open_new_connection) {
            usb->notify.wait(lock);
        }
        usb->open_new_connection = false;
        lock.unlock();

        while (true) {
            if (init_functionfs(usb)) {
                LOG(INFO) << "functionfs successfully initialized";
                break;
            }
            std::this_thread::sleep_for(1s);
        }

        LOG(INFO) << "registering usb transport";
        register_usb_transport(usb, 0, 0, 1);
    }

    // never gets here
    abort();
}

static void usb_ffs_init() {
    D("[ usb_init - using FunctionFS ]");

    usb_handle* h = new usb_handle();

    if (android::base::GetBoolProperty("sys.usb.ffs.aio_compat", false)) {
        // Devices on older kernels (< 3.18) will not have aio support for ffs
        // unless backported. Fall back on the non-aio functions instead.
        h->write = usb_ffs_write;
        h->read = usb_ffs_read;
    } else {
        h->write = usb_ffs_aio_write;
        h->read = usb_ffs_aio_read;
        aio_block_init(&h->read_aiob);
        aio_block_init(&h->write_aiob);
    }
    h->kick = usb_ffs_kick;
    h->close = usb_ffs_close;

    D("[ usb_init - starting thread ]");
    std::thread(usb_ffs_open_thread, h).detach();
}
```

#

参考：
[Android KitKat 4.4平台开发-添加USB ADB和MTP功能支持](https://www.51dev.com/android/19665)
[RK3399 Android8.1源码 ADB消息传输，函数调用跟踪](https://www.cnblogs.com/guanglun/p/10967930.html)
https://lwn.net/Articles/382480/
