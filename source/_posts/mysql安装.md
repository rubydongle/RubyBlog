---
title: mysql安装
categories:
  - 技术文章
date: 2019-09-17 21:05:57
tags:
---

# 安装
## 1.创建mysql用户和组
```bash
[root@vultr ~]# groupadd mysql
[root@vultr ~]# useradd -s /sbin/nologin -g mysql -M mysql
[root@vultr ~]# tail -1 /etc/passwd
mysql:x:1001:1001::/home/mysql:/sbin/nologin
```

## 2.下载MySQL
从[MySQL下载地址](https://dev.mysql.com/downloads/mysql)下载需要的MySQL。  
```bash
[root@vultr ~]# wget https://dev.mysql.com/get/Downloads/MySQL-5.5/mysql-5.5.54-linux2.6-x86_64.tar.gz
[root@vultr ~]# ls -l mysql-5.5.54-linux2.6-x86_64.tar.gz 
-rw-r--r--. 1 root root 185911232 3月   3 13:34 mysql-5.5.54-linux2.6-x86_64.tar.gz

[root@vultr ~]# tar xf mysql-5.5.54-linux2.6-x86_64.tar.gz 
[root@vultr ~]# mv mysql-5.5.54-linux2.6-x86_64 /lnmp/mysql-5.5.54
[root@vultr ~]# ln -s /lnmp/mysql-5.5.54/ /lnmp/mysql
```

## 3.配置MySQL
- 初始化配置文件
```bash
[root@vultr mysql]# cp /lnmp/mysql/support-files/my-small.cnf /etc/my.cnf
cp: overwrite ‘/etc/my.cnf’? y
```

- 初始化数据库文件
```bash
[root@vultr mysql-5.5.54]# mkdir -p /lnmp/mysql/data
[root@vultr mysql-5.5.54]# chown -R mysql.mysql /lnmp/mysql
[root@vultr mysql-5.5.54]# yum install -y libaio # 安装MySQL依赖库
[root@vultr mysql-5.5.54]# /lnmp/mysql/scripts/mysql_install_db --basedir=/lnmp/mysql --datadir=/lnmp/mysql/data --user=mysql
Installing MySQL system tables...
190917 13:35:30 [Note] Ignoring --secure-file-priv value as server is running with --bootstrap.
190917 13:35:30 [Note] /lnmp/mysql/bin/mysqld (mysqld 5.5.54) starting as process 15154 ...
OK
Filling help tables...
190917 13:35:30 [Note] Ignoring --secure-file-priv value as server is running with --bootstrap.
190917 13:35:30 [Note] /lnmp/mysql/bin/mysqld (mysqld 5.5.54) starting as process 15161 ...
OK

```

- 配置并启动MySQL数据库
```bash
#(1)设置MySQL启动脚本
[root@vultr mysql-5.5.54]# cp support-files/mysql.server /etc/init.d/mysqld
[root@vultr mysql-5.5.54]# chmod +x /etc/init.d/mysqld 
[root@vultr mysql-5.5.54]# ls -l /etc/init.d/mysqld 
-rwxr-xr-x 1 root root 10875 9月  17 13:39 /etc/init.d/mysqld

#(2)替换启动脚本中MySQL默认的安装路径/usr/local/mysql为/lnmp/mysql
sed -i 's#/usr/local/mysql#/lnmp/mysql#g' /lnmp/mysql/bin/mysqld_safe /etc/init.d/mysqld

#(3)启动MySQL数据库
[root@vultr mysql-5.5.54]#  /etc/init.d/mysqld start
Starting MySQL.Logging to '/lnmp/mysql/data/vultr.guest.err'.
. SUCCESS! 

#（4）检查MySQL数据库是否启动
[root@vultr mysql-5.5.54]# netstat -lntup | grep mysql
tcp        0      0 0.0.0.0:3306            0.0.0.0:*               LISTEN      15849/mysqld 

#（5）查看日志
[root@vultr mysql]# tail /lnmp/mysql/data/vultr.guest.err
InnoDB: Creating foreign key constraint system tables
InnoDB: Foreign key constraint system tables created
190918  8:52:53  InnoDB: Waiting for the background threads to start
190918  8:52:54 InnoDB: 5.5.54 started; log sequence number 0
190918  8:52:54 [Note] Server hostname (bind-address): '0.0.0.0'; port: 3306
190918  8:52:54 [Note]   - '0.0.0.0' resolves to '0.0.0.0';
190918  8:52:54 [Note] Server socket created on IP: '0.0.0.0'.
190918  8:52:54 [Note] Event Scheduler: Loaded 0 events
190918  8:52:54 [Note] /lnmp/mysql/bin/mysqld: ready for connections.
Version: '5.5.54'  socket: '/tmp/mysql.sock'  port: 3306  MySQL Community Server (GPL)

#（6）设置MySQL开机启动
[root@vultr ~]# chkconfig --add mysqld
[root@vultr ~]# chkconfig mysqld on
[root@vultr ~]# chkconfig --list mysqld
mysqld          0:off   1:off   2:on    3:on    4:on    5:on    6:off
```

- 配置mysql命令的全局使用路径（注意这里配置的是命令，前面配置的只是启动脚本）
```
[root@vultr ~]# echo 'export PATH=/lnmp/mysql/bin:$PATH' >>/etc/profile
[root@vultr ~]# source /etc/profile
```

## 4.登陆MySQL测试
```bash
[root@vultr ~]# mysql

# MySQL基本安全优化，为root用户设置密码
[root@vultr ~]# mysqladmin -u root password '123456'

# 连接mysql，查看数据库
[root@vultr mysql]# mysql -u root -p
Enter password:
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 3
Server version: 5.5.54 MySQL Community Server (GPL)

Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mysql> show database;
ERROR 1064 (42000): You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'database' at line 1
mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| test               |
+--------------------+
4 rows in set (0.00 sec)
```

# 参考文档
1.[从零开始完整搭建LNMP环境+WordPress部署](https://blog.51cto.com/xpleaf/1903115)
