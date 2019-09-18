---
title: 安装WordPress
categories:
  - 技术文章
date: 2019-09-18 10:20:35
tags:
---

WordPress的安装基于LNMP环境的基础上  
我们创建一个目录/lnmp/wwwroot用于存储WordPress
```
[root@vultr lnmp]# mkdir wwwroot
```

## 获取wordpress
```
[root@vultr lnmp]# cd wwwroot/
[root@vultr wwwroot]# ls
[root@vultr wwwroot]# wget https://cn.wordpress.org/wordpress-4.7.3-zh_CN.tar.gz
[root@vultr wwwroot]# tar xvf wordpress-4.7.3-zh_CN.tar.gz 

#解压后修改权限
[root@vultr conf]# chown -R nginx.nginx /lnmp/wwwroot/wordpress/
```
## 创建nginx配置文件
在nginx的配置目录创建www/wordpress.conf
```
[root@vultr conf]# mkdir /lnmp/nginx/conf/www
[root@vultr conf]# touch /lnmp/nginx/conf/www/wordpress.conf
```
在创建的文件中写入下面的内容  
```
server {
        listen       8080;
        server_name  localhost;
        location / {
            root   /lnmp/wwwroot/wordpress;
            index index.php index.html index.htm;
        }
        location ~ .*\.(php|php5)?$ {
            root   /lnmp/wwwroot/wordpress;
            fastcgi_pass 127.0.0.1:9000;
            fastcgi_index index.php;
            include fastcgi.conf;
        }
    }
```
将wordpress的配置添加到nginx配置的http段,修改/lnmp/nginx/conf/nginx.conf  
```
http {
    include       mime.types;
    default_type  application/octet-stream;
    sendfile        on;
    keepalive_timeout  65;
# ......
# 加入下面的内容
    include www/wordpress.conf;
}
```
重启nginx
service nginx restart

## 创建数据库
1.登录MySQL,并创建数据库wordpress
```
[root@vultr php-fpm.d]# mysql -u root -p
Enter password:

mysql> create database wordpress;
Query OK, 1 row affected (0.00 sec)

mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| test               |
| wordpress          |
+--------------------+
5 rows in set (0.01 sec)
```

2.创建管理用户
```
mysql> grant all on wordpress.* to wordpress@'localhost' identified by '123456';
Query OK, 0 rows affected (0.00 sec)

mysql> show grants for wordpress@'localhost';
+------------------------------------------------------------------------------------------------------------------+
| Grants for wordpress@localhost                                                                                   |
+------------------------------------------------------------------------------------------------------------------+
| GRANT USAGE ON *.* TO 'wordpress'@'localhost' IDENTIFIED BY PASSWORD '*6BB4837EB74329105EE4568DDA7DC67ED2CA2AD9' |
| GRANT ALL PRIVILEGES ON `wordpress`.* TO 'wordpress'@'localhost'                                                 |
+------------------------------------------------------------------------------------------------------------------+
2 rows in set (0.00 sec)
```

3.刷新MySQL用户权限  
```
mysql> flush privileges;
Query OK, 0 rows affected (0.01 sec)
```

4.检查MySQL登录用户
```
mysql> select user,host from mysql.user;
+-----------+-------------+
| user      | host        |
+-----------+-------------+
| root      | 127.0.0.1   |
| root      | ::1         |
|           | localhost   |
| root      | localhost   |
| wordpress | localhost   |
|           | vultr.guest |
| root      | vultr.guest |
+-----------+-------------+
7 rows in set (0.00 sec)
```


## 安装wordpress
打开8080端口
```
开启端口 :
firewall-cmd --zone=public --add-port=8080/tcp --permanent

命令含义：
--zone #作用域
--add-port=80/tcp #添加端口，格式为：端口/通讯协议
--permanent #永久生效，没有此参数重启后失效

重启防火墙
firewall-cmd --reload
```
在网页中输入http://alphaepoch.com:8080启动安装页面



## 问题处理
您的 PHP 似乎没有安装运行 WordPress 所必需的 MySQL 扩展”处理方法：
https://www.php.net/manual/en/set.mysqlinfo.ph
编译PHP时 configure指定
 --with-mysqli \
 --with-pdo-mysql \
