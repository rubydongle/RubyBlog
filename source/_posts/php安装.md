---
title: php安装
categories:
  - 技术文章
date: 2019-09-17 22:26:21
tags:
---

# 安装依赖库
```bash
[root@vultr php-src]# yum install -y openssl openssl-devel zlib-devel libxml2-devel libjpeg-devel libjpeg-turbo-devel libiconv-devel freetype-devel libpng-devel gd-devel libcurl-devel libxslt-devel

# 编译安装libiconv-devel
[root@vultr ~]# wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz
[root@vultr ~]# tar xvf libiconv-1.14.tar.gz
[root@vultr ~]# cd libiconv-1.14/
[root@vultr libiconv-1.14]# ./configure -prefix=/usr/local/libiconv
[root@leaf libiconv-1.14]# make && make install

# 安装libmcrypt库
[root@vultr libiconv-1.14]# yum install -y libmcrypt-devel

# 安装mhash加密扩展库
[root@vultr libiconv-1.14]# yum install -y mhash

# 安装mcrypt加密扩展库
[root@vultr libiconv-1.14]# yum install -y mcrypt
```

# 获取源码
从[php下载网站](https://www.php.net/downloads.php)获取需要的php文件或者通过git从[php git网站](https://www.php.net/git.php)下载源码。  
```bash
git clone https://github.com/php/php-src.git

PHP 7.1: git checkout PHP-7.1
PHP 7.2: git checkout PHP-7.2
PHP 7.3: git checkout PHP-7.3
PHP 7.4: git checkout PHP-7.4
PHP HEAD: git checkout master
```
run ./buildconf to generate the configure script. This may take several moments.

# 编译安装
```bash
[root@vultr php-src]#./configure \
--prefix=/lnmp/php7.1 \
--with-iconv-dir=/usr/local/libiconv \
--with-freetype-dir \
--with-jpeg-dir \
--with-png-dir \
--with-zlib \
--with-libxml-dir=/usr \
--with-mysqli \
--with-pdo-mysql \
--with-pdo-sqlite \
--enable-xml \
--disable-rpath \
--enable-safe-mode \
--enable-bcmath \
--enable-shmop \
--enable-sysvsem \
--enable-inline-optimization \
--with-curl \
--with-curlwrappers \
--enable-mbregex \
--enable-fpm \
--enable-mbstring \
--with-mcrypt \
--with-gd \
--enable-gd-native-ttf \
--with-openssl \
--with-mhash \
--enable-pcntl \
--enable-sockets \
--with-xmlrpc \
--enable-zip \
--enable-soap \
--enable-short-tags \
--enable-zend-multibyte \
--enable-static \
--with-xsl \
--with-fpm-user=nginx \
--with-fpm-group=nginx \
--enable-ftp

//[root@vultr php-src]# ln -s /lnmp/mysql/lib/libmysqlclient.so.18 /usr/lib64/
//[root@vultr php-src]# touch ext/phar/phar.phar
[root@vultr php-src]# make 
[root@vultr php-src]# make install
```

# 配置与启动
## 1.设置PHP安装目录软链接
```bash
[root@vultr php-src]# ln -s /lnmp/php7.1/ /lnmp/php
[root@vultr php-src]# ls -al /lnmp
总用量 16
drwxr-xr-x   4 root  root  4096 9月  17 15:22 .
dr-xr-xr-x. 20 root  root  4096 9月  17 13:15 ..
lrwxrwxrwx   1 mysql mysql   19 9月  17 13:32 mysql -> /lnmp/mysql-5.5.54/
drwxr-xr-x  14 root  root  4096 9月  17 13:23 mysql-5.5.54
lrwxrwxrwx   1 root  root    13 9月  17 15:22 php -> /lnmp/php7.1/
drwxr-xr-x   9 root  root  4096 9月  17 15:20 php7.1
```
## 2.拷贝PHP配置文件到PHP默认目录
```bash
[root@vultr php-src]# cp php.ini-production /lnmp/php/lib/php.ini
[root@vultr php-src]# ls -l /lnmp/php/lib
总用量 76
drwxr-xr-x 15 root root  4096 9月  17 15:25 php
-rw-r--r--  1 root root 72020 9月  17 15:26 php.ini
```

## 3.配置php-fpm.conf文件
```bash
[root@vultr php-src]# cd /lnmp/php/etc/
[root@vultr etc]# ls
pear.conf  php-fpm.conf.default  php-fpm.d
[root@vultr etc]# cp php-fpm.conf.default php-fpm.conf

[root@vultr etc]# cd php-fpm.d/
[root@vultr php-fpm.d]# cp www.conf.default www.conf

```

## 4.启动PHP服务
```bash
[root@vultr php-fpm.d]# /lnmp/php/sbin/php-fpm
[root@vultr php-fpm.d]# ps -ef | grep php-fpm
root     29810     1  0 15:37 ?        00:00:00 php-fpm: master process (/lnmp/php7.1/etc/php-fpm.conf)
nginx    29811 29810  0 15:37 ?        00:00:00 php-fpm: pool www
nginx    29812 29810  0 15:37 ?        00:00:00 php-fpm: pool www
root     29820  1613  0 15:38 pts/1    00:00:00 grep --color=auto php-fpm

[root@vultr php-fpm.d]#  netstat -lntup | grep 9000
tcp        0      0 127.0.0.1:9000          0.0.0.0:*               LISTEN      29810/php-fpm: mast
```
至此，PHP也安装完成了！LNMP的各个组件都安装好了，下面就要对LNMP环境进行测试了。


# LNMP环境测试
修改/lnmp/nginx/conf下的nginx.conf
在location / {}后面添加一个location
```bash
# location test
        location ~ .*\.(php|php5)?$ {
            root   html;
            fastcgi_pass 127.0.0.1:9000;
            fastcgi_index index.php;
            include fastcgi.conf;
        }
```
检查对配置的修改是否正确，并重启nginx
```bash
[root@vultr conf]# nginx -t
nginx: the configuration file /lnmp/nginx-1.16.1/conf/nginx.conf syntax is ok
nginx: configuration file /lnmp/nginx-1.16.1/conf/nginx.conf test is successful

[root@vultr conf]# service nginx restart
Restarting nginx (via systemctl):                          [  OK  ]
```
向/lnmp/nginx/html目录中写入一个test_info.php用于测试php工作情况。  
```
echo "<?php phpinfo(); ?>" >/lnmp/nginx/html/test_info.php
```
用浏览器打开[http://alphaepoch.com/test_info.php](http://alphaepoch.com/test_info.php)可以看到工作正常了  
![img](/files/nginx/phpworkok.PNG)

# 设置php-fpm开机子启动
开机自启动的脚本在源码目录中的php-src/sapi/fpm/init.d.php-fpm
```bash
# 添加控制脚本
[root@spyderman php-src]# cp sapi/fpm/init.d.php-fpm /etc/init.d/php-fpm
[root@spyderman php-src]# chmod a+x /etc/init.d/php-fpm 
[root@spyderman php-src]# pkill php-fpm
[root@spyderman php-src]# /etc/init.d/php-fpm start
Starting php-fpm  done


# 设置开机自启动
[root@spyderman php-src]# chkconfig --list

注：该输出结果只显示 SysV 服务，并不包含
原生 systemd 服务。SysV 配置数据
可能被原生 systemd 配置覆盖。 

      要列出 systemd 服务，请执行 'systemctl list-unit-files'。
      查看在具体 target 启用的服务请执行
      'systemctl list-dependencies [target]'。

aegis          	0:关	1:关	2:开	3:开	4:开	5:开	6:关
mysqld         	0:关	1:关	2:开	3:开	4:开	5:开	6:关
netconsole     	0:关	1:关	2:关	3:关	4:关	5:关	6:关
network        	0:关	1:关	2:开	3:开	4:开	5:开	6:关
nginx          	0:关	1:关	2:开	3:开	4:开	5:开	6:关
[root@spyderman php-src]# chkconfig --add /etc/init.d/php-fpm 
[root@spyderman php-src]# chkconfig php-fpm on
[root@spyderman php-src]# chkconfig --list

注：该输出结果只显示 SysV 服务，并不包含
原生 systemd 服务。SysV 配置数据
可能被原生 systemd 配置覆盖。 

      要列出 systemd 服务，请执行 'systemctl list-unit-files'。
      查看在具体 target 启用的服务请执行
      'systemctl list-dependencies [target]'。

aegis          	0:关	1:关	2:开	3:开	4:开	5:开	6:关
mysqld         	0:关	1:关	2:开	3:开	4:开	5:开	6:关
netconsole     	0:关	1:关	2:关	3:关	4:关	5:关	6:关
network        	0:关	1:关	2:开	3:开	4:开	5:开	6:关
nginx          	0:关	1:关	2:开	3:开	4:开	5:开	6:关
php-fpm        	0:关	1:关	2:开	3:开	4:开	5:开	6:关

# 测试
[root@spyderman php-src]# service php-fpm stop
Gracefully shutting down php-fpm . done
[root@spyderman php-src]# service php-fpm start
Starting php-fpm  done
[root@spyderman php-src]# service php-fpm restart
Gracefully shutting down php-fpm . done
Starting php-fpm  done
[root@spyderman php-src]# service php-fpm status
php-fpm (pid 19336) is running...

```



# 问题解决
1.安装PHP 7.3时发生system libzip must be upgraded to version >= 0.11  
解决方法：  
```bash
yum  -y remove libzip-devel  
```
然后从官网下载并编译安装  
```bash
wget https://libzip.org/download/libzip-1.3.2.tar.gz
tar xvf libzip-1.3.2.tar.gz
cd libzip-1.3.2
./configure
make && make install
```

2.在低于1G内存的机器上安装php 7 make时报错:make: *** [ext/fileinfo/libmagic/apprentice.lo] Error 1
这是因为系统的内存不足1G造成的,然后需要加上 --disable-fileinfo,然重新编译即可；

# 参考文档
1.[从零开始完整搭建LNMP环境+WordPress部署](https://blog.51cto.com/xpleaf/1903115)
