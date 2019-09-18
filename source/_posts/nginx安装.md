---
title: nginx安装
date: 2019-09-17 15:09:07
categories:
  - 技术文章
tags:
  - 服务器
  - nginx
---

# 编译环境
## 1.CentOS:  
### 安装开发工具包和依赖库  
```bash
[root@vultr ~]# yum groupinstall -y 'Development Tools'
[root@vultr ~]# yum install -y pcre pkre-devel openssl openssl-devel
```

## 2.Ubuntu：
### 安装开发工具包和依赖库  
```bash
[root@vultr ~]# apt-get install build-essential
```


# 编译安装
## 1.创建ngnix用户和组
```bash
[root@vultr ~]# useradd nginx -s /sbin/nologin -M
[root@vultr ~]# tail -1 /etc/passwd
nginx:x:1000:1000::/home/nginx:/sbin/nologin
```

## 2.下载nginx
进入[nginx下载页](https://nginx.org/en/download.html)选择合适的nginx版本下载到本地,并进行解压。  
```bash
[root@vultr ~]# wget https://nginx.org/download/nginx-1.16.1.tar.gz
[root@vultr ~]# tar xvf nginx-1.16.1.tar.gz
```

## 3.编译安装nginx
将nginx安装到/lnmp目录下，首先要创建安装的目录
```bash
[root@vultr ~]# mkdir -p /lnmp/nginx-1.16.1
```
在nginx编译前需要运行configure脚本进行配置生成MakeFile文件。通过./configure –help可以查看帮助选项。  
配置好后直接执行make就启动了编译，编译完成后执行make install就将编译好的nginx安装到了lnmp目录。  
最后通过软链接将/lnmp/nginx-1.16.1链接到/lnmp/nginx。    
```bash
[root@vultr nginx-1.16.1]# ./configure --user=nginx --group=nginx --prefix=/lnmp/nginx-1.16.1 --with-http_stub_status_module --with-http_ssl_module
[root@vultr nginx-1.16.1]# make
[root@vultr nginx-1.16.1]# make install

[root@vultr nginx-1.16.1]# ln -s /lnmp/nginx-1.16.1/ /lnmp/nginx
```

## 4.将nginx添加到环境变量
将下面的代码添加到/etc/bashrc  
```bash
if [ -d "/lnmp/nginx/sbin" ] ; then
    PATH="/lnmp/nginx/sbin:$PATH"
fi
```
然后执行source /etc/bashrc。  
```bash
[root@vultr nginx-1.16.1]# source /etc/bashrc
[root@vultr nginx-1.16.1]# nginx -v
nginx version: nginx/1.16.1
```

# 启动nginx
## 1.初步测试nginx服务器
```bash
[root@vultr nginx-1.16.1]# nginx -t    # 检查配置文件
nginx: the configuration file /lnmp/nginx-1.16.1/conf/nginx.conf syntax is ok
nginx: configuration file /lnmp/nginx-1.16.1/conf/nginx.conf test is successful
[root@vultr nginx-1.16.1]# nginx    # 启动Nginx服务
[root@vultr nginx-1.16.1]# netstat -lntup | grep 80
tcp        0      0 0.0.0.0:80              0.0.0.0:*               LISTEN      29382/nginx: master
[root@vultr nginx-1.16.1]# curl localhost
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
    body {
        width: 35em;
        margin: 0 auto;
        font-family: Tahoma, Verdana, Arial, sans-serif;
    }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>

<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>

<p><em>Thank you for using nginx.</em></p>
</body>
</html>
```

## 2.开启80端口
通过下面的命令开启80端口：
```bash
firewall-cmd --zone=public --add-port=80/tcp --permanent
firewall-cmd --reload
```
命令参数的含义如下：  
--zone #作用域  
--add-port=80/tcp #添加端口，格式为：端口/通讯协议  
--permanent #永久生效，没有此参数重启后失效  

--reload #重启防火墙  

此时在浏览器中输入绑定的域名，或ip地址能正常访问页面。
![img](/files/nginx/installok.PNG)

## 3.设置nginx开机自启动
从nginx的网站获取[自启动脚本](https://www.nginx.com/resources/wiki/start/topics/examples/redhatnginxinit/)将其保存到/etc/init.d/nginx中。  
修改/etc/init.d/nginx中nginx可执行文件的位置和配置文件的位置，并修改其可执行属性。
```
#1.修改/etc/init.d/nginx文件
nginx="/lnmp/nginx/sbin/nginx"  #修改成nginx执行程序的路径。
NGINX_CONF_FILE="/lnmp/nginx/conf/nginx.conf" #修改成nginx.conf文件的路径。

#2.修改/etc/init.d/nginx文件的执行权限
[root@vultr nginx-1.16.1]# chmod a+x /etc/init.d/nginx

#3.修改/lnmp/nginx/conf/nginx.conf中pid文件的路径
#pid        logs/nginx.pid;
pid         /var/run/nginx.pid;
```
后面就可以通过下面的脚本命令来控制nginx了  
/etc/init.d/nginx start|stop|status
```
[root@vultr nginx-1.16.1]# /etc/init.d/nginx start
Starting nginx (via systemctl):                            [  OK  ]
[root@vultr nginx-1.16.1]# /etc/init.d/nginx status
● nginx.service - SYSV: NGINX is an HTTP(S) server, HTTP(S) reverse proxy and IMAP/POP3 proxy server
   Loaded: loaded (/etc/rc.d/init.d/nginx; bad; vendor preset: disabled)
   Active: active (running) since Wed 2019-09-18 08:14:05 UTC; 4s ago
     Docs: man:systemd-sysv-generator(8)
  Process: 29799 ExecStart=/etc/rc.d/init.d/nginx start (code=exited, status=0/SUCCESS)
 Main PID: 29841 (nginx)
   CGroup: /system.slice/nginx.service
           ├─29841 nginx: master process /lnmp/nginx/sbin/nginx -c /lnmp/nginx/conf/nginx.conf
           └─29843 nginx: worker process

Sep 18 08:14:05 vultr.guest systemd[1]: Starting SYSV: NGINX is an HTTP(S) server, HTTP(S) reverse proxy and IMAP/POP3 proxy server...
Sep 18 08:14:05 vultr.guest nginx[29799]: Starting nginx: [  OK  ]
Sep 18 08:14:05 vultr.guest systemd[1]: PID file /var/run/nginx.pid not readable (yet?) after start.
Sep 18 08:14:05 vultr.guest systemd[1]: Started SYSV: NGINX is an HTTP(S) server, HTTP(S) reverse proxy and IMAP/POP3 proxy server.
[root@vultr nginx-1.16.1]# /etc/init.d/nginx stop
Stopping nginx (via systemctl):                            [  OK  ]
```
完成脚本管理nginx服务后还需要将nginx加入到chkconfig管理列表中，然后设置开机自启动    
```bash
[root@vultr nginx-1.16.1]# chkconfig --add /etc/init.d/nginx
[root@vultr nginx-1.16.1]# chkconfig nginx on

[root@vultr nginx-1.16.1]# service nginx start
Starting nginx (via systemctl):                            [  OK  ]
[root@vultr nginx-1.16.1]# service nginx stop
Stopping nginx (via systemctl):                            [  OK  ]
[root@vultr nginx-1.16.1]# service nginx restart
Restarting nginx (via systemctl):                          [  OK  ]
```

至此，nginx的安装和设置就完成了。  

----
# 参考文档
1.[从零开始完整搭建LNMP环境+WordPress部署](https://blog.51cto.com/xpleaf/1903115)  


