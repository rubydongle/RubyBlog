---
title: Ubuntu通过shadowsocks-qt5科学上网
categories:
  - 使用方法
date: 2019-09-15 09:52:33
tags:
  - ShadowSocks
---

# 在Ubuntu 18上安装shasowsocks-qt5
## 1.添加软件源  
shadowsocks-qt5默认不在Ubuntu的软件源中，需要首先添加软件源。
```bash
#添加shadowsocks-qt5的软件源  
ruby@spiderman:~$sudo add-apt-repository ppa:hzwhuang/ss-qt5

# 修改文件名
ruby@spiderman:~$sudo mv /etc/apt/sources.list.d/hzwhuang-ubuntu-ss-qt5-bionic.list /etc/apt/sources.list.d/hzwhuang-ubuntu-ss-qt5-xenial.list

# 将hzwhuang-ubuntu-ss-qt5-xenial.list文件中的bionic改成xenial
ruby@spiderman:~$sed -i 's#bionic#xenial#g'  /etc/apt/sources.list.d/hzwhuang-ubuntu-ss-qt5-xenial.list
```

## 2.执行安装
软件源设置好后，直接执行下面的命令就能完成安装了，安装完成后就能通过图标启动shadowsocks-qt5了。    
```
sudo apt-get update
sudo apt-get install shadowsocks-qt5
```

# 浏览器科学上网
通过设置浏览器的代理可以让浏览器的流量走sock5或http(s)代理。

# 终端科学上网
参考[https://blog.fazero.me/2015/09/15/让终端走代理的几种方法/](https://blog.fazero.me/2015/09/15/让终端走代理的几种方法/)  

1.如果shadowsocks中设置的是http(s)代理,在终端中直接运行下面的命令就可以了  
```bash
export http_proxy=http://proxyAddress:port
export https_proxy=http://proxyAddress:port
```
这个办法的好处是简单直接，并且影响面很小（只对当前终端有效，退出就不行了）。  

2.如果使用的是socks5代理，在当前终端运行以下命令，那么wget curl 这类网络命令都会经过socks5代理  
```bash
export ALL_PROXY=socks5://127.0.0.1:1080
```

# 全局代理科学上网
https://blog.pandll.com/article/c33/
1.privoxy
apt-get install privoxy

配置privoxy
vi /etc/privoxy/config

1336 forward-socks5t / 127.0.0.1:1080 .

监听接口默认开启的 localhost：8118

启动privoxy
//开启privoxy 服务就行
sudo service privoxy start
// 设置http 和 https 全局代理
export http_proxy=’http://localhost:8118’
export https_proxy=’https://localhost:8118’


2.genpac  
https://blog.csdn.net/u010260808/article/details/84573184

3.自己脚本设置环境变量  
将下面的代码放到 ~/bin/proxyconfig.sh文件中  
```bash
export ALL_PROXY=socks5://127.0.0.1:1080
export HTTP_PROXY=socks5://127.0.0.1:1080
export HTTPS_PROXY=socks5://127.0.0.1:1080
```
然后执行source ~/bin/proxyconfig.sh终端的环境就设置好了。  


# 常见问题
1.Android source Code 无法下载解决
首先设置shadowsocks代理模式为http(s),然后配置代理HTTP(S)_PROXY=http(s)://127.0.0.1:1080 ，再通过下面的命令设置git的代理。
```bash
git config --global http.proxy http://127.0.0.1:1080
git config --global https.proxy https://127.0.0.1:1080
# git config --global http.proxy 'socks5://127.0.0.1:1080' 
# git config --global https.proxy 'socks5://127.0.0.1:1080'
```
也可以编辑~/.gitconfig文件
```bash
[http]
        proxy = http://127.0.0.1:1080
[https]
        proxy = https://127.0.0.1:1080

```
完成后就能下载了。  
