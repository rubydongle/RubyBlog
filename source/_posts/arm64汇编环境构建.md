---
title: arm64汇编环境构建
categories:
  - 技术文章
date: 2019-11-29 14:55:34
tags:
---

## 编译工具链接
下载地址:https://www.linaro.org/downloads/
wget https://releases.linaro.org/components/toolchain/gcc-linaro/latest-7/gcc-linaro-7.4-2019.02.tar.xz
安装gcc需要GMP、MPFR、MPC这三个库，可从ftp://gcc.gnu.org/pub/gcc/infrastructure/下载相应的压缩包。由于MPFR依赖GMP，而MPC依赖GMP和MPFR，所以要先安装GMP，其次MPFR，最后才是MPC。
```bash
./configure --prefix=/home/xxx/soft
./configure --prefix=/home/xxx/soft --with-gmp=/home/xxx/soft
等
```

直接下载编译工具链:
wget https://releases.linaro.org/components/toolchain/binaries/latest-7/aarch64-linux-gnu/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu.tar.xz


## 安装qemu
```bash
sudo apt install qemu-user
```
qemu-user可以模拟
qemu-aarch64

## 编译代码,执行
```c
.text

.global main
main:
        ldr x0, addr_of_keep_x30
        str x30, [x0]

        ldr x0, addr_of_msg
        bl puts

        ldr x0, addr_of_keep_x30
        ldr x30, [x0]

        mov w0, #0
        ret


addr_of_msg: .dword msg
addr_of_keep_x30: .dword keep_x30
.data
msg: .asciz "hello world!\n"
keep_x30: .dword 0
```
将hello.S文件编译成.o文件
```c
ruby@batman:~$ aarch64-linux-gnu-as -o hello.o hello.S
```
现在使用aarch64-linux-gcc进行编译（-static标志很重要）
```c
ruby@batman:~$ aarch64-linux-gnu-gcc -static -o hello hello.o
ruby@batman:~$ file helloworld
helloworld: ELF 64-bit LSB executable, ARM aarch64, version 1 (SYSV), statically linked, for GNU/Linux 3.7.0, BuildID[sha1]=84c0a30aadb5863cfff7bda9ebe68d77635cbd39, with debug_info, not stripped
```

## 执行
qemu-aarch64 helloworld
或直接执行helloworld
或adb push 到arm64手机中执行
