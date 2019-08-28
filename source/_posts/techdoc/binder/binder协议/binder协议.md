---
title: binder协议
date: 2019-08-28 09:53:47
categories:
- 技术文章
tags:
- Android
- Binder
---

## 概述
Binder是一个基于CS架构模型的通信框架，Client发送BC_命令到binder驱动，Server侧从binder驱动接收BR_命令并处理，处理完成后通过BC_命令返回结果给binder驱动，binder驱动充当桥梁负责Client侧和Server侧数据传递。 
参考内核代码include/uapi/linux/android/binder.h
- Client侧能发送的BC_命令有如下：  

    |  命令名                       | 含义                                 | 参数类型                          |
    |  :----                        | :----                                | :----                             |
    | BC_TRANSACTION                | Binder事务，Client向Server侧发送请求 | struct binder_transaction_data    |
    | BC_REPLY                      | 事务的应答，即Server对Client请求回复 | struct binder_transaction_data    |
    | BC_ACQUIRE_RESULT             | 暂未实现                             | /                                 |
    | BC_FREE_BUFFER                | 同时驱动释放buffer                   | binder_uintptr_t                  |
    | BC_INCREFS                    | 弱引用计数+1                         | __u32                             |
    | BC_ACQUIRE                    | 强引用计数+1                         | __u32                             |
    | BC_RELEASE                    | 强引用计数-1                         | __u32                             |
    | BC_DECREFS                    | 弱引用计数-1                         | __u32                             |
    | BC_INCREFS_DONE               | 回复BR_INCREFS的命令                 | struct binder_ptr_cookie          |
    | BC_ACQUIRE_DONE               | 回复BR_ACQUIRE的命令                 | struct binder_ptr_cookie          |
    | BC_ATTEMPT_ACQUIRE            | 暂未实现                             | /                                 |
    | BC_REGISTER_LOOPER            | 通知驱动binder子线程准备完毕         | void                              |
    | BC_ENTER_LOOPER               | 通知驱动binder主线程主板完毕         | void                              |
    | BC_EXIT_LOOPER                | 通知驱动binder线程退出               | void                              |
    | BC_REQUEST_DEATH_NOTIFICATION | 请求接收特定handle(服务)的死亡通知   | struct binder_handle_cookie       |
    | BC_CLEAR_DEATH_NOTIFICATION   | 取消对特定handle(服务)死亡通知的接收 | struct binder_handle_cookie       |
    | BC_DEAD_BINDER_DONE           | 已经处理完死亡通知                   | binder_uintptr_t                  |
    | BC_TRANSACTION_SG             | 分散/聚集 I/O                        | struct binder_transaction_data_sg |
    | BC_REPLY_SG                   | 回复BC_TRANSACTION_SG                | struct binder_transaction_data_sg |

SG IO参考
https://blog.csdn.net/u012432778/article/details/47323805
https://lore.kernel.org/patchwork/patch/757477/
https://wiki.jikexueyuan.com/project/java-nio/scatter-gather.html

- Server侧能接受处理的BR_命令有如下：  

    |  命令名                          | 含义                                                                              | 参数类型                       |
    |  :----                           | :----                                                                             | :----                          |
    | BR_ERROR                         | 发生错误，通过参数传递错误码                                                      | __s32                          |
    | BR_OK                            | 操作正常完成                                                                      | void                           |
    | BR_TRANSACTION                   | 通知进程收到Binder事务请求(Server侧)                                              | struct binder_transaction_data |
    | BR_REPLY                         | 通知进程收到Binder事务请求的回复(Client侧)                                        | struct binder_transaction_data |
    | BR_ACQUIRE_RESULT                | 暂未实现                                                                          | /                              |
    | BR_DEAD_REPLY                    | 告诉发送方最近通过BC_TRANSACTION发送数据的接收方已死亡                            | void                           |
    | BR_TRANSACTION_COMPLETE          | binder驱动对于接收请求的确认回复                                                  | void                           |
    | BR_INCREFS                       | 弱引用计数+1请求                                                                  | struct binder_ptr_cookie       |
    | BR_ACQUIRE                       | 强引用计数+1请求                                                                  | struct binder_ptr_cookie       |
    | BR_RELEASE                       | 强引用计数-1请求                                                                  | struct binder_ptr_cookie       |
    | BR_DECREFS                       | 弱引用计数-1请求                                                                  | struct binder_ptr_cookie       |
    | BR_ATTEMPT_ACQUIRE               | 暂未实现                                                                          | /                              |
    | BR_NOOP                          | 不执行任何操作，并检查下一条命令                                                  | void                           |
    | *BR_SPAWN_LOOPER*                | *binder驱动找不到空闲的任务线程，通知Binder进程创建一个新的binder子线程*          | void                           |
    | BR_FINISHED                      | 暂未实现                                                                          | /                              |
    | BR_DEAD_BINDER                   | 接收到死亡通知                                                                    | binder_uintptr_t               |
    | BR_CLEAR_DEATH_NOTIFICATION_DONE | 清理死亡通知完成                                                                  | binder_uintptr_t               |
    | BR_FAILED_REPLY                  | 告诉发送方最近通过BC_TRANSACTION发送的数据失败，比如no memory，但是没参数说明原因 | void                           |
