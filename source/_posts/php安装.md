---
title: php安装
categories:
  - 技术文章
date: 2019-09-17 22:26:21
tags:
---

# 获取需要的文件
从[php下载网站](https://www.php.net/downloads.php)获取需要的php文件或者通过git从[php git网站](https://www.php.net/git.php)下载源码。  
```
git clone https://github.com/php/php-src.git

PHP 7.1: git checkout PHP-7.1
PHP 7.2: git checkout PHP-7.2
PHP 7.3: git checkout PHP-7.3
PHP 7.4: git checkout PHP-7.4
PHP HEAD: git checkout master
```
run ./buildconf to generate the configure script. This may take several moments.

# 编译源码
1. 使用yum安装除libiconv-devel之外的其它lib库
[root@leaf mysql]# yum install -y openssl openssl-devel zlib-devel libxml2-devel libjpeg-devel libjpeg-turbo-devel libiconv-devel freetype-devel libpng-devel gd-devel libcurl-devel libxslt-devel

2. 编译安装libiconv-devel
[root@leaf tools]# wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz
[root@leaf tools]# tar zxf libiconv-1.14.tar.gz 
[root@leaf tools]# cd libiconv-1.14
[root@leaf libiconv-1.14]# ./configure --prefix=/usr/local/libiconv
[root@leaf libiconv-1.14]# make
[root@leaf libiconv-1.14]# make install

3.安装libmcrypt库

[root@leaf ~]# wget -O /etc/yum.repos.d/epel.repo http://mirrors.aliyun.com/repo/epel-6.repo
[root@leaf ~]# yum install -y libmcrypt-devel

4.安装mhash加密扩展库

[root@leaf ~]# yum install -y mhash
5.安装mcrypt加密扩展库

[root@leaf ~]# yum install -y mcrypt

6.配置编译
```
./configure \
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
```

7.编译
```
[root@vultr php-src]# ln -s /lnmp/mysql/lib/libmysqlclient.so.18 /usr/lib64/
[root@vultr php-src]# touch ext/phar/phar.phar
[root@vultr php-src]# make 
[root@vultr php-src]# make install
```

## 配置与启动
1.设置PHP安装目录软链接
```
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


2.拷贝PHP配置文件到PHP默认目录
```
[root@vultr php-src]# cp php.ini-production /lnmp/php/lib/php.ini
[root@vultr php-src]# ls -l /lnmp/php/lib
总用量 76
drwxr-xr-x 15 root root  4096 9月  17 15:25 php
-rw-r--r--  1 root root 72020 9月  17 15:26 php.ini
```
3.配置php-fpm.conf文件
```
[root@vultr php-src]# cd /lnmp/php/etc/
[root@vultr etc]# ls
pear.conf  php-fpm.conf.default  php-fpm.d
[root@vultr etc]# cp php-fpm.conf.default php-fpm.conf

[root@vultr etc]# cd php-fpm.d/
[root@vultr php-fpm.d]# cp www.conf.default www.conf

```

4.启动PHP服务
```
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


## LNMP环境测试
修改/lnmp/nginx/conf下的nginx.conf
在location / {}后面添加一个location
```
        location ~ .*\.(php|php5)?$ {
            root   html;
            fastcgi_pass 127.0.0.1:9000;
            fastcgi_index index.php;
            include fastcgi.conf;
        }
```
检查对配置的修改是否正确
```
[root@vultr conf]# nginx -t
nginx: the configuration file /lnmp/nginx-1.16.1/conf/nginx.conf syntax is ok
nginx: configuration file /lnmp/nginx-1.16.1/conf/nginx.conf test is successful
```
重启nginx  
```
[root@vultr conf]# service nginx restart
Restarting nginx (via systemctl):                          [  OK  ]
```
向/lnmp/nginx/html目录中写入一个test_info.php用于测试php工作情况。  
```
echo "<?php phpinfo(); ?>" >/lnmp/nginx/html/test_info.php
```
用浏览器打开http://alphaepoch.com/test_info.php可以看到工作正常了  



## 问题
system libzip must be upgraded to version >= 0.11  
yum  -y remove libzip-devel
然后从官网下载并编译安装

wget https://libzip.org/download/libzip-1.3.2.tar.gz
tar xvf libzip-1.3.2.tar.gz
cd libzip-1.3.2
./configure
make && make install

# 参考文档
1.[从零开始完整搭建LNMP环境+WordPress部署](https://blog.51cto.com/xpleaf/1903115)
