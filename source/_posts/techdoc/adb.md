---
title: adb
categories:
  - 技术文章
date: 2019-09-11 23:51:33
tags:
  - Android
---

## Target侧
Target侧使用adbd daemon接收并处理Host侧通过adb命令发送过来的请求。  
adbd启动于/system/core/adb/daemon/main.cpp  
```cpp
int adbd_main(int server_port) {
    umask(0);

    signal(SIGPIPE, SIG_IGN);

    init_transport_registration();

    // We need to call this even if auth isn't enabled because the file
    // descriptor will always be open.
    adbd_cloexec_auth_socket();

    if (ALLOW_ADBD_NO_AUTH && !android::base::GetBoolProperty("ro.adb.secure", false)) {
        auth_required = false;
    }

    adbd_auth_init();

    // Our external storage path may be different than apps, since
    // we aren't able to bind mount after dropping root.
    const char* adb_external_storage = getenv("ADB_EXTERNAL_STORAGE");
    if (adb_external_storage != nullptr) {
        setenv("EXTERNAL_STORAGE", adb_external_storage, 1);
    } else {
        D("Warning: ADB_EXTERNAL_STORAGE is not set.  Leaving EXTERNAL_STORAGE"
          " unchanged.\n");
    }

    drop_privileges(server_port);

    bool is_usb = false;
    if (access(USB_FFS_ADB_EP0, F_OK) == 0) {
        // Listen on USB.
        usb_init();
        is_usb = true;
    }

    // If one of these properties is set, also listen on that port.
    // If one of the properties isn't set and we couldn't listen on usb, listen
    // on the default port.
    std::string prop_port = android::base::GetProperty("service.adb.tcp.port", "");
    if (prop_port.empty()) {
        prop_port = android::base::GetProperty("persist.adb.tcp.port", "");
    }

    int port;
    if (sscanf(prop_port.c_str(), "%d", &port) == 1 && port > 0) {
        D("using port=%d", port);
        // Listen on TCP port specified by service.adb.tcp.port property.
        setup_port(port);
    } else if (!is_usb) {
        // Listen on default port.
        setup_port(DEFAULT_ADB_LOCAL_TRANSPORT_PORT);
    }

    D("adbd_main(): pre init_jdwp()");
    init_jdwp();
    D("adbd_main(): post init_jdwp()");

    D("Event loop starting");
    fdevent_loop();

    return 0;
}

int main(int argc, char** argv) {
    while (true) {
        static struct option opts[] = {
            {"root_seclabel", required_argument, nullptr, 's'},
            {"device_banner", required_argument, nullptr, 'b'},
            {"version", no_argument, nullptr, 'v'},
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", opts, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 's':
            root_seclabel = optarg;
            break;
        case 'b':
            adb_device_banner = optarg;
            break;
        case 'v':
            printf("Android Debug Bridge Daemon version %d.%d.%d\n", ADB_VERSION_MAJOR,
                   ADB_VERSION_MINOR, ADB_SERVER_VERSION);
            return 0;
        default:
            // getopt already prints "adbd: invalid option -- %c" for us.
            return 1;
        }
    }

    close_stdin();

    debuggerd_init(nullptr);
    adb_trace_init(argv);

    D("Handling main()");
    return adbd_main(DEFAULT_ADB_PORT);
}
```
adbd的启动过程包括下面的几步：  
- 通过init_transport_registration()初始化  
- 通过adbd_cloexec_auth_socket()  
- 通过adbd_auth_init()  
- drop_privileges()  
- 检查USB可用后，通过usb_init()开始监听USB上的请求数据。  
- 启动adb tcp通信功能，通过setup_port(port)监听socket的adb请求数据。  
- 通过init_jdwp()开启jdwp控制。  
- 通过fdevent_loop()进入循环。  
1.init_transport_registration()  
  该模块在/system/core/adb/transport.cpp文件中。  
  在init函数中通过socketpair()创建了用来通信的socket，并且加入到fdevent模块中。  
2.adbd_auth模块负责通信的权限管理。  
3.drop_privileges()  
  Target默认使用5037端口进行通信。  
4.adbd通过/dev/usb-ffs/adb/目录下的设备节点文件进行数据通信。  
  ```
  #define USB_FFS_ADB_PATH "/dev/usb-ffs/adb/"
  #define USB_FFS_ADB_EP(x) USB_FFS_ADB_PATH #x

  #define USB_FFS_ADB_EP0 USB_FFS_ADB_EP(ep0)
  #define USB_FFS_ADB_OUT USB_FFS_ADB_EP(ep1)
  #define USB_FFS_ADB_IN USB_FFS_ADB_EP(ep2)
  ```
  usb_init()  
5.adbd也可以通过tcp进行连接。  
  ```
  #define DEFAULT_ADB_LOCAL_TRANSPORT_PORT 5555
  ```
6.jdwp是和Java进程进行debug交互的协议。  
7.当一切准备好后通过fdevent_loop()处理adb的每一条请求。  

目前支持两种adb通信，USB通信和TCP Socket通信。在adb/transport.cpp中对通信方式抽象成struct atransport，构建USB和socket类型的通信后通过register_transport进行注册。  
参考class atransport(adb/transport.h)


#### USB通信
adb/daemon/usb.c h
使用USB进行adb通信是，数据封装在struct aio_block，通信处理通过struct usb_handle提供。  
```
struct aio_block {
    std::vector<struct iocb> iocb;
    std::vector<struct iocb*> iocbs;
    std::vector<struct io_event> events;
    aio_context_t ctx;
    int num_submitted;
    int fd;
};

struct usb_handle {
    usb_handle() : kicked(false) {
    }

    std::condition_variable notify;
    std::mutex lock;
    std::atomic<bool> kicked;
    bool open_new_connection = true;

    int (*write)(usb_handle* h, const void* data, int len);
    int (*read)(usb_handle* h, void* data, int len);
    void (*kick)(usb_handle* h);
    void (*close)(usb_handle* h);

    // FunctionFS
    int control = -1;
    int bulk_out = -1; /* "out" from the host's perspective => source for adbd */
    int bulk_in = -1;  /* "in" from the host's perspective => sink for adbd */

    // Access to these blocks is very not thread safe. Have one block for both the
    // read and write threads.
    struct aio_block read_aiob;
    struct aio_block write_aiob;
};
```
![img](/files/adb/targetUsbOp.png)
Target 针对连接上的USB设备可以执行写、读、踢掉、关闭操作。  
Android提供/dev/usb-ffs/adb/目录供adb通信使用。  
![img](/files/adb/usbcommdev.png)
- 构建并注册USB atransport
```
void register_usb_transport(usb_handle* usb, const char* serial, const char* devpath,
                            unsigned writeable) {
    atransport* t = new atransport((writeable ? kCsOffline : kCsNoPerm));

    D("transport: %p init'ing for usb_handle %p (sn='%s')", t, usb, serial ? serial : "");
    init_usb_transport(t, usb);
    if (serial) {
        t->serial = strdup(serial);
    }

    if (devpath) {
        t->devpath = strdup(devpath);
    }

    {
        std::lock_guard<std::recursive_mutex> lock(transport_lock);
        pending_list.push_front(t);
    }

    register_transport(t);
}

// This should only be used for transports with connection_state == kCsNoPerm.
void unregister_usb_transport(usb_handle* usb) {
    std::lock_guard<std::recursive_mutex> lock(transport_lock);
    transport_list.remove_if([usb](atransport* t) {
        if (auto connection = dynamic_cast<UsbConnection*>(t->connection.get())) {
            return connection->handle_ == usb && t->GetConnectionState() == kCsNoPerm;
        }
        return false;
    });
}
```

#### tcp通信


## Host侧
