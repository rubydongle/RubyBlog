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
Binder是一个基于CS架构模型的通信框架，Client发送BC_命令到Server侧，Server侧接收BR_命令并处理，处理完成后通过BC_命令返回结果给Client侧。  
- Client侧能发送的BC_命令有如下：  

    |  命令名   | 含义  |
    |  :----:  | :----:  |
    | BC_TRANSACTION  | xxx |
    | BC_REPLY  | xxx |
    | BC_ACQUIRE_RESULT|xxx|
    | BC_FREE_BUFFER |xxx|
    | BC_INCREFS |xxx|
    | BC_ACQUIRE |xxx|
    | BC_RELEASE |xxx|
    | BC_DECREFS |xxx|
    | BC_INCREFS_DONE|xxx|
    | BC_ACQUIRE_DONE |xxx|
    | BC_ATTEMPT_ACQUIRE |xxx|
    | BC_REGISTER_LOOPER |xxx|
    | BC_ENTER_LOOPER |xxx|
    | BC_EXIT_LOOPER |xxx|
    | BC_REQUEST_DEATH_NOTIFICATION |xxx|
    | BC_CLEAR_DEATH_NOTIFICATION |xxx|
    | BC_DEAD_BINDER_DONE |xxx|
    | BC_TRANSACTION_SG |xxx|
    | BC_REPLY_SG |xxx|
- Server侧能接受处理的BR_命令有如下：  

    |  命令名   | 含义  |
    |  :----:  | :----:  |
    | BR_ERROR |xxx|
    | BR_OK |xxx|
