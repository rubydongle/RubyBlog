---
title: shadowsocks使用
categories:
  - 使用方法
date: 2019-09-15 09:52:33
tags:
  - ShadowSocks
---

## Ubuntu 18安装shasowsocks-qt5
1.添加软件源  
执行sudo add-apt-repository ppa:hzwhuang/ss-qt5添加shadowsocks-qt5的软件源  
然后执行下面命令修改文件名：  
```bash
sudo mv /etc/apt/sources.list.d/hzwhuang-ubuntu-ss-qt5-bionic.list /etc/apt/sources.list.d/hzwhuang-ubuntu-ss-qt5-xenial.list
```
修改hzwhuang-ubuntu-ss-qt5-xenial.list文件中的内容，将里面的bionic改成为xenial。  

2.执行安装
```
sudo apt-get update
sudo apt-get install shadowsocks-qt5
```
安装完成后我们就能看见安装的图图标了。  

## 浏览器流量走shadowsocks
通过设置浏览器的代理可以让浏览器的流量走sock5或http(s)代理。

## 终端流量走shadowsocks
参考[https://blog.fazero.me/2015/09/15/让终端走代理的几种方法/](https://blog.fazero.me/2015/09/15/让终端走代理的几种方法/)  
1.如果shadowsocks中设置的是http(s)代理,在终端中直接运行下面的命令就可以了  
export http_proxy=http://proxyAddress:port
export https_proxy=http://proxyAddress:port
这个办法的好处是简单直接，并且影响面很小（只对当前终端有效，退出就不行了）。  

2.如果使用的是socks5代理，在当前终端运行以下命令，那么wget curl 这类网络命令都会经过socks5代理  
export ALL_PROXY=socks5://127.0.0.1:1080

## 全局代理工具
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
```
export ALL_PROXY=socks5://127.0.0.1:1080
export HTTP_PROXY=socks5://127.0.0.1:1080
export HTTPS_PROXY=socks5://127.0.0.1:1080
```
然后执行source ~/bin/proxyconfig.sh终端的环境就设置好了。  

