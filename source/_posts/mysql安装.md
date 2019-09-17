---
title: mysql安装
categories:
  - 技术文章
date: 2019-09-17 21:05:57
tags:
---

https://blog.51cto.com/xpleaf/1903115
1.创建MySQL用户的账号
```
[root@vultr ~]# groupadd mysql
[root@vultr ~]# useradd -s /sbin/nologin -g mysql -M mysql
[root@vultr ~]# tail -l /etc/passwd
......
mysql:x:1000:1000::/home/mysql:/sbin/nologin
```

2.下载MySQL，解压并移动到目标目录
https://dev.mysql.com/downloads/mysql/
```
[root@vultr ~]# wget https://dev.mysql.com/get/Downloads/MySQL-5.5/mysql-5.5.54-linux2.6-x86_64.tar.gz
[root@vultr ~]# ls -l mysql-5.5.54-linux2.6-x86_64.tar.gz 
-rw-r--r--. 1 root root 185911232 3月   3 13:34 mysql-5.5.54-linux2.6-x86_64.tar.gz

[root@vultr ~]# tar xf mysql-5.5.54-linux2.6-x86_64.tar.gz 
[root@vultr ~]# mv mysql-5.5.54-linux2.6-x86_64 /lnmp/mysql-5.5.54
[root@vultr ~]# ln -s /application/mysql-5.5.54/ /application/mysql
```

3.初始化MySQL
- 初始化配置文件
```
[root@vultr mysql-5.5.54]# cp support-files/my-small.cnf /etc/my.cnf
cp：是否覆盖"/etc/my.cnf"？ y
```

- 初始化数据库文件
```
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
```
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
???????
[root@leaf mysql]# tail -10 /application/mysql/data/leaf.err 
InnoDB: Creating foreign key constraint system tables
InnoDB: Foreign key constraint system tables created
170304  7:00:28  InnoDB: Waiting for the background threads to start
170304  7:00:29 InnoDB: 5.5.54 started; log sequence number 0
170304  7:00:29 [Note] Server hostname (bind-address): '0.0.0.0'; port: 3306
170304  7:00:29 [Note]   - '0.0.0.0' resolves to '0.0.0.0';
170304  7:00:29 [Note] Server socket created on IP: '0.0.0.0'.
170304  7:00:29 [Note] Event Scheduler: Loaded 0 events
170304  7:00:29 [Note] /application/mysql/bin/mysqld: ready for connections.
Version: '5.5.54'  socket: '/tmp/mysql.sock'  port: 3306  MySQL Community Server (GPL)

#（6）设置MySQL开机启动

```
[root@vultr ~]# chkconfig --add mysqld
[root@vultr ~]# chkconfig mysqld on
[root@vultr ~]# chkconfig --list mysqld
```

#（7）配置mysql命令的全局使用路径（注意这里配置的是命令，前面配置的只是启动脚本）
```
[root@vultr ~]# echo 'export PATH=/lnmp/mysql/bin:$PATH' >>/etc/profile
[root@vultr ~]# source /etc/profile
```

#（8）登陆MySQL测试
[root@vultr ~]# mysql
```

- MySQL基本安全优化
```
#(1)为root用户设置密码
[root@vultr ~]# mysqladmin -u root password '123456'
```
