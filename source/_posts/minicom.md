---
title: minicom
categories:
  - 技术文章
date: 2019-09-30 20:21:58
tags:
---

# USB转串口
```bash
ruby@batman:~$ ls -alh /dev/ttyUSB0 
crw-rw---- 1 root dialout 188, 0 9月  30 20:14 /dev/ttyUSB0
```

# 配置
运行minicom -s 开始启动
进入Serial port setup
设置为串口设备A -    Serial Device      : /dev/ttyUSB0
设置波特E -    Bps/Par/Bits       : 115200 8N1
设置Hard Flow Control为 No，否则在minicom上无法输入字符。F - Hardware Flow Control : No 
```
+-----------------------------------------------------------------------+                                                               
| A -    Serial Device      : /dev/ttyUSB0                              |                                                               
| B - Lockfile Location     : /var/lock                                 |                                                               
| C -   Callin Program      :                                           |                                                               
| D -  Callout Program      :                                           |                                                               
| E -    Bps/Par/Bits       : 115200 8N1                                |                                                               
| F - Hardware Flow Control : No                                        |                                                               
| G - Software Flow Control : No                                        |                                                               
|                                                                       |                                                               
|    Change which setting?                                              |                                                               
+-----------------------------------------------------------------------+
```
从开发板向PC传输文件

[root@EmbedSky /]# sz init 

然后在你PC的path路径下多了一个文件init，path是你从PC上启动minicom时的路径

13.从PC向开发板传输文件

[root@EmbedSky /]# rz

Ctrl+A后输入Z，进入minicom 主菜单界面，选择S

upload中选择zmodem，其实我是随便选择了一种模式

[Select one or more files for upload]界面按下空格键即可选择一个要传输的文件

选择[OKay]开始传输
