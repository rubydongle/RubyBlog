---
title: PPU概述
categories:
  - 技术文章
date: 2019-09-03 11:24:42
tags:
---

## 概述
NES PPU是Picture Processing Unit的简称。  

It has its own address space, which typically contains 10 kilobytes of memory: 8 kilobytes of ROM or RAM on the Game Pak (possibly more with one of the common mappers) to store the shapes of background and sprite tiles, plus 2 kilobytes of RAM in the console to store a map or two. Two separate, smaller address spaces hold a palette, which controls which colors are associated to various indices, and OAM (Object Attribute Memory), which stores the position, orientation, shape, and color of the sprites, or independent moving objects. These are internal to the PPU itself, and while the palette is made of static memory, OAM uses dynamic memory (which will slowly decay if the PPU is not rendering data).

PPU外部：  
PPU有自己的地址空间，常见的PPU包含10kb内存内存空间,这10kb一般来自NES卡带：  
其中8kb在游戏卡带上的ROM或RAM上，用来存储背景或sprite贴图。  
另外的2kb在NES卡带上的RAM存储，作为nametable地址映射到PPU的$2000-2FFF。
下面是PPU内存映射关系
|Address range	|Size	|Description
|----      	|----	|
|$0000-$0FFF	|$1000	|Pattern table 0
|$1000-$1FFF	|$1000	|Pattern table 1
|$2000-$23FF	|$0400	|Nametable 0
|$2400-$27FF	|$0400	|Nametable 1
|$2800-$2BFF	|$0400	|Nametable 2
|$2C00-$2FFF	|$0400	|Nametable 3
|$3000-$3EFF	|$0F00	|Mirrors of $2000-$2EFF
|$3F00-$3F1F	|$0020	|Palette RAM indexes
|$3F20-$3FFF	|$00E0	|Mirrors of $3F00-$3F1F



PPU内部：  
还有两个独立的小地址空间用来存储palette和OAM(Object Attribute Memory)。  
OAM用来存储sprites或独立移动对象的位置，方向，形状和颜色。  
这两个独立的小地址空间的内存在PPU内部，palette由static memory构成，OAM使用dynamic memory。  

## 2.寄存器
PPU对CPU有8个寄存器可见，这些寄存器通过内存映射的方式映射到CPU的地址空间$2000-$2700。


## 3.PPU内部内存映射
PPU内部的有16kb的内存空间，从$0000到$3FFF。PPU可以直接访问这些内存空间，CPU也可以通过$2006和$2007访问寄存器来访问。 
PPUADDR	$2006	aaaa aaaa	PPU read/write address (two writes: most significant byte, least significant byte)  
PPUDATA	$2007	dddd dddd	PPU data read/write
第一步写入地址到$2006,由于a寄存器是8位，所以要写两次。 
第二步在上一步设定的地址读取或写入数据  

如下面的代码我们设置PPUADDR为$2000,然后写入数据,PPU的内部地址$2000是Nametable 0.
写入数据后PPUADDR的位置会递增，所以我们下面的代码相当于将PPU的Nametable 0中的内容全部设置为0.    
$2000-$23FF	$0400	Nametable 0  

```
        lda #$20  ;设置PPU memory Nametable 0 $2000-23FF
        sta $2006
        lda #$00
        sta $2006
        ; empty nametable
        lda #0
        ldy #30 ; 30 rows
        :
                ldx #32 ; 32 columns
                :
                        sta $2007
                        dex
                        bne :-
                dey
                bne :--
```

## .Pattern Table(背景和sprites的贴片图)
pattern table是一块连接到PPU的内存区域，这块区域定义了tiles的形状。这些tiles构成背景和sprites。
每个tile在pattern table中占用16bytes，分成两部分。  
第一部分控制bit0，第二部分控制bit1.像素颜色是0的是背景或者透明的。  

## 背景图
名称表(Nametables)
NES有2kb的内存给PPU使用，通常映射到Nametable的地址空间($2000-$2FFF),
属性表(Attribute tables)


## PPU内部内存palette
## PPU内部内存-OAM
PPU内部有256byte的OAM(Object Attribute Memroy)内存,这块内存里的数据确定如何渲染sprites。  
CPU可以通过已经映射到CPU内存空间的内存映射寄存器OAMADDR($2003)、OAMDATA($2004)和OAMDMA($4014)来操作它。  
OAM可以看成是有64个元素的数组，每个元素包含4byte。每个元素含义如下：  

|Address Low Nibble	|Description
|----              	|----
|$00, $04, $08, $0C	|Sprite Y coordinate
|$01, $05, $09, $0D	|Sprite tile #
|$02, $06, $0A, $0E	|Sprite attribute
|$03, $07, $0B, $0F	|Sprite X coordinate

DMA OAM，CPU通过$4014可以通过DMA的方式写入数据到OAM内存区。  
如向$4014写入$XX会将CPU内存页$XX00-XXFF的数据通过DMA的方式写入到PPU的OAM区域。一般通过DMA的方式传输CPU内部ram，游戏卡带上的ROM和RAM区域的数据也可以通过DMA方式传递到PPU的OAM中。  
