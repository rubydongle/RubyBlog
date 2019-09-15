---
title: LVM
categories:
  - 技术文章
date: 2019-09-15 22:55:47
tags:
---

## 查看硬盘使用情况
```
root@batman:/home/ruby# fdisk -l /dev/sda
Disk /dev/sda：465.8 GiB，500107862016 字节，976773168 个扇区
单元：扇区 / 1 * 512 = 512 字节
扇区大小(逻辑/物理)：512 字节 / 512 字节
I/O 大小(最小/最佳)：512 字节 / 512 字节
磁盘标签类型：dos
磁盘标识符：0x3d759fa5

设备       启动     起点      末尾      扇区   大小 Id 类型
/dev/sda1           2048  97867775  97865728  46.7G 83 Linux
/dev/sda2  *    97867776 312711167 214843392 102.5G 83 Linux
```

## 参考  
https://linux.cn/article-3218-1.html
