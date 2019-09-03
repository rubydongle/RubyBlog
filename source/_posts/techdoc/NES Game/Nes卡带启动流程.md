---
title: Nes卡带启动流程
categories:
  - 技术文章
date: 2019-09-03 11:24:42
tags:
---
## 引子
NES镜像是存储在卡带上的，游戏系统是怎么启动卡带上的游戏镜像程序的呢？  

## 游戏卡带
通过CPU内存映射我们可以看到，卡带被映射到CPU的内存空间$4020-$FFFF。  
游戏卡带空间包括程序ROM区，程序RAM区和mapper寄存器。  

## 中断向量表
中断是一个很重要的东西，CPU中定义的中断向量表的位置在卡带的末端空间：  
- $FFFA-$FFFB:NMI中断向量
- $FFFC-$FFFD:Reset中断向量
- $FFFE-$FFFF:IRQ/BRK中断向量

## 启动过程
我们知道$FFFA-$FFFF是卡带上的空间，并且也是中断向量表的位置。
这表明如果CPU收到中断会执行游戏卡段上相应位置的代码，比如CPU接收到Reset中断，它会从$FFFC读取中断处理程序的地址，然后将PC设置为中断处理程序的地址，执行中断处理程序。  
这么说应该很明显了，游戏机通电，执行相应的硬件初始化操作后，会发送一个Reset的中断，CPU接收到中断后，跳转到卡带上的代码，并执行。这样就将CPU的控制器交给了在游戏卡带上的程序。

## 如何实现
我们需要在$FFFC处申明Reset程序，并实现Reset程序就行了，参考example.s的实现  
```
.segment "VECTORS"
.word nmi
.word reset
.word irq

.segment "CODE"
reset:
	sei       ; mask interrupts
	lda #0
	sta $2000 ; disable NMI
```
我们在代码中申明了一个VECTORS段，这个段里面存储了nmi、reset和irq的位置，并且实现了reset操作。  
下面我们需要将VECTORS段指定在中断向量表的位置。这是通过链接配置文件example.cfg完成的。  
```
MEMORY {
......
    PRG:    start = $8000,  size = $8000, type = ro, file = %O, fill = yes, fillval = $00;
......
}

SEGMENTS {
......
    VECTORS:  load = PRG, type = ro,  start = $FFFA;
......
}
```
在链接配置文件，我们指定了中断向量表的起始位置为$FFFA.  
