---
title: NesExample代码分析
categories:
  - 技术文章
date: 2019-08-31 09:51:19
tags:
---

## 1.代码使用
代码位置存在于
[https://github.com/rubydongle/Nes-ca65-Example.git](https://github.com/rubydongle/Nes-ca65-Example.git)   
内部写好了Makefile文件，按照[http://wiki.nesdev.com/w/index.php/Installing_CC65](http://wiki.nesdev.com/w/index.php/Installing_CC65)中的提示下载好编译工具链后，使用make命令就可以直接构建了。  
分析此代码的目的是为了学习ATT汇编的格式，以及 6502 processor汇编语言。  
6502的指令集可以参考[http://wiki.nesdev.com/w/index.php/6502_instructions](http://wiki.nesdev.com/w/index.php/6502_instructions)  
其中下面的目录不错 [http://obelisk.me.uk/6502/reference.html](http://obelisk.me.uk/6502/reference.html)  

[http://nesdev.com/6502_cpu.txt](http://nesdev.com/6502_cpu.txt)

6502的寄存器可以参考下面文章：
[http://wiki.nesdev.com/w/index.php/CPU_registers](http://wiki.nesdev.com/w/index.php/CPU_registers)  
[http://obelisk.me.uk/6502/registers.html](http://obelisk.me.uk/6502/registers.html)

6502的内存寻址方式可以参考下面文章：
[http://obelisk.me.uk/6502/addressing.html](http://obelisk.me.uk/6502/addressing.html)

数字格式  
$十六进制表示 例如$40对应二进制%0100,0000  
%二进制表示 例如%10001000  

## 2.初始化  
初始化的代码块在reset中  
```
reset:
	sei       ; mask interrupts                    1....
	lda #0    ;                                    2....
	sta $2000 ; disable NMI                        3....
	sta $2001 ; disable rendering
	sta $4015 ; disable APU sound
	sta $4010 ; disable DMC IRQ
	lda #$40
	sta $4017 ; disable APU IRQ
	cld       ; disable decimal mode
	ldx #$FF
	txs       ; initialize stack
	; wait for first vblank
	bit $2002
	:
		bit $2002
		bpl :-
	; clear all RAM to 0
	lda #0
	ldx #0
	:
		sta $0000, X
		sta $0100, X
		sta $0200, X
		sta $0300, X
		sta $0400, X
		sta $0500, X
		sta $0600, X
		sta $0700, X
		inx
		bne :-
	; place all sprites offscreen at Y=255
	lda #255
	ldx #0
	:
		sta oam, X
		inx
		inx
		inx
		inx
		bne :-
	; wait for second vblank
	:
		bit $2002
		bpl :-
	; NES is initialized, ready to begin!
	; enable the NMI for graphical updates, and jump to our main program
	lda #%10001000
	sta $2000
	jmp main
```
- 1.首先通过sei指令禁用了中断  
    sei指令和cli指令是相对应的，前者用来禁止中断，后者用来使能中断。  
    使用sei指令后状态寄存器中的I(Interrupt Disable)标志位被设置为了1。 
    状态寄存器是一个8位的寄存器，其中7位被使用用来标记状态，分别是C(Carry Flag)、Z(Zero Flag)、I(Interrupt Disable)、D(Decimal Mode Flag)、B(Brak Command)、V(Overflow Flag)、N(Negative Flag)。  
    关于I标记为详细参考[http://obelisk.me.uk/6502/registers.html#I](http://obelisk.me.uk/6502/registers.html#I)

- 2.将寄存器a中内容置为0  
    上面我们使用了状态寄存器中的I标志位，第二步我们就使用了寄存器a。  
    寄存器a是Accumulator累加寄存器，它和状态寄存器一样也是一个8位的寄存器，它在使用中和状态寄存器的C、Z、V、N等一起完成工作。  
    lda指令将一个字节8位的内容加载到寄存器A中，当加载内容是0时会设置状态寄存器中的ZFlag，当加载的内容是负时会设置状态寄存器中的NFlag(这个是根据A的bit 7是否被设置来判定的)。  
    lda可以支持下面类型的寻址模式：  
    - Immediate
        立即寻址方式:程序中直接指定一个8位的常量赋值到寄存器a中，它的格式是一个#后面跟着一个数字。例如：
        ```
        LDA #10         ;Load 10 ($0A) into the accumulator
        LDX #LO LABEL   ;Load the LSB of a 16 bit address into X
        LDY #HI LABEL   ;Load the MSB of a 16 bit address into Y
        ```
        这一步我们的代码通过lda #0指令将寄存器a中的内容全部设置为0。
    - Zero Page
    - Zero Page,X
    - Absolute
    - Absolute,X
    - Absolute,Y
    - (Indirect,X)
    - (Indirect,Y)

- 3.将寄存器a中的内容写入内存  
    和lda指令对应的是sta，它的作用是将寄存器a的内容存入相应的内存地址中，sta的使用不会对状态寄存器中的内容有所影响。  
    lda可以支持下面类型的寻址模式：
    - Zero Page
    - Zero Page,X
    - Absolute
        绝对寻址方式：指令的操作数是一个16位的数指定的存储器中有效的目的地址,通过$和一个16位数指定，如$AD $F6 $31 $31F6。下面的指令将地址为$31F6的值载入累加器A:   
        ```
        LDA $31F6       ;将地址为$31F6的值载入累加器, 即 A = [$31F6]
        ```
    - Absolute,X
    - Absolute,Y
    - (Indirect,X)
    - (Indirect),Y
    我们的example代码中通过绝对地址操作内存的如下：
    ```
    sta $2000 ; disable NMI
    sta $2001 ; disable rendering
    sta $4015 ; disable APU sound
    sta $4010 ; disable DMC IRQ
    lda #$40
    sta $4017 ; disable APU IRQ
    ```
    其操作了内存地址$2000和$4000区域，这两个区域分别映射到PPU和APU的寄存器上的。  
    关于APU参考[http://wiki.nesdev.com/w/index.php/APU](http://wiki.nesdev.com/w/index.php/APU)
    关于PPU参考[http://wiki.nesdev.com/w/index.php/PPU](http://wiki.nesdev.com/w/index.php/PPU)和(http://wiki.nesdev.com/w/index.php/PPU_registers)[http://wiki.nesdev.com/w/index.php/PPU_registers]
    APU的内存映射关系如下：  

    | Registers | Channel | Units |
    | :----     | :----   | :---- |
    |$4000-$4003|Pulse 1  |Timer, length counter, envelope, sweep
    |$4004-$4007|Pulse 2  |Timer, length counter, envelope, sweep
    |$4008-$400B|Triangle |Timer, length counter, linear counter
    |$400C-$400F|Noise    |Timer, length counter, envelope, linear feedback shift register
    |$4010-$4013|DMC      |Timer, memory reader, sample buffer, output unit
    |$4015      |All      |Channel enable and length counter status
    |$4017      |All      |Frame counter

    PPU的内存映射关系如下：  
    | Common Name |Address      |Bits           |Notes
    | ----       |:----        |:----          |:----
    |PPUCTRL	  |$2000	|VPHB SINN	|NMI enable (V), PPU master/slave (P), sprite height (H), background tile select (B), sprite tile select (S), increment mode (I), nametable select (NN)
    |PPUMASK	  |$2001	|BGRs bMmG	|color emphasis (BGR), sprite enable (s), background enable (b), sprite left column enable (M), background left column enable (m), greyscale (G)
    |PPUSTATUS	  |$2002	|VSO- ----	|vblank (V), sprite 0 hit (S), sprite overflow (O); read resets write pair for $2005/$2006
    |OAMADDR	  |$2003	|aaaa aaaa	|OAM read/write address
    |OAMDATA	  |$2004	|dddd dddd	|OAM data read/write
    |PPUSCROLL	  |$2005	|xxxx xxxx	|fine scroll position (two writes: X scroll, Y scroll)
    |PPUADDR	  |$2006	|aaaa aaaa	|PPU read/write address (two writes: most significant byte, least significant byte)
    |PPUDATA	  |$2007	|dddd dddd	|PPU data read/write
    |OAMDMA	  |$4014	|aaaa aaaa	|OAM DMA high address

  上面的将$2000和$2001内存地址的两个字节中写入0，相当于操作了PPU
  PPU Controller($2000)寄存器每个位的功能如下：  
  ```
  7  bit  0
  ---- ----
  VPHB SINN
  |||| ||||
  |||| ||++- Base nametable address
  |||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
  |||| |+--- VRAM address increment per CPU read/write of PPUDATA
  |||| |     (0: add 1, going across; 1: add 32, going down)
  |||| +---- Sprite pattern table address for 8x8 sprites
  ||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
  |||+------ Background pattern table address (0: $0000; 1: $1000)
  ||+------- Sprite size (0: 8x8 pixels; 1: 8x16 pixels)
  |+-------- PPU master/slave select
  |          (0: read backdrop from EXT pins; 1: output color on EXT pins)
  +--------- Generate an NMI at the start of the
           vertical blanking interval (0: off; 1: on)
  ```
  PPU Mask($2001)寄存器中每个位的功能如下：
  ```
  7  bit  0
  ---- ----
  BGRs bMmG
  |||| ||||
  |||| |||+- Greyscale (0: normal color, 1: produce a greyscale display)
  |||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide
  |||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide
  |||| +---- 1: Show background
  |||+------ 1: Show sprites
  ||+------- Emphasize red
  |+-------- Emphasize green
  +--------- Emphasize blue
  ```
  观察后面的操作，代码如下：
  ```
  lda #$40
  sta $4017 ; disable APU IRQ
  ```
  相当于把0100,0000写入了,内存位置$4017即APU的Frame Counter寄存器。可以参考[http://wiki.nesdev.com/w/index.php/APU_Frame_Counter](http://wiki.nesdev.com/w/index.php/APU_Frame_Counter)了解。  
  ```
  7  bit  0
  ---- ----
  MI-- ----
  |||| ||||
  |+-------- IRQ inhibit flag(I).
  |          If set, the frame interrupt flag is cleared, otherwise it is unaffected.
  +--------- Sequencer Mode(M,0=4-step, 1=5-step)
  ```
  参考寄存器位flag的意义，我们知道把$40写入内存地址$4017相当于禁止了APU的IRQ。  

4.操作状态寄存器Decimal Mode Flag  
  后续我们通cld指令将状态寄存器的Decimal Mode Flag设置为0，由于CPU刚启动时这个标志位是未知的，所以我们要重置下。   
  While the decimal mode flag is set the processor will obey the rules of Binary Coded Decimal (BCD) arithmetic during addition and subtraction.
  The flag can be explicitly set using 'Set Decimal Flag' (SED) and cleared with 'Clear Decimal Flag' (CLD).
  ```
       ; disable decimal mode
  ```
5.初始化程序栈  
  ```
  ldx #$FF
  txs       ; initialize stack
  ```
  初始化程序栈我们用到了两个寄存器，[Index Register X]()和[Stack Pointer]()
  Stack Pointer存入当前栈指针，用于后续JSR，和RTS指令使用。 
  首先我们将x寄存器中设置为$FF，这个是我们设置的程序栈栈顶的位置,然后通过txs将x寄存器中的内容存入Stack Pointer寄存器，后续我们通过JSR跳转子程序，就用这个栈。

6.等待PPU准备好
  ```
  ; wait for first vblank
  bit $2002
  :
  	bit $2002
    	bpl :-
  ```
  可以看到这个相当于是一个循环，读取$2002内存位置中数据的状态，PPU中这个寄存器的含义描述如下：
  ```
  7  bit  0
  ---- ----
  VSO. ....
  |||| ||||
  |||+-++++- Least significant bits previously written into a PPU register
  |||        (due to register not being updated for this address)
  ||+------- Sprite overflow. The intent was for this flag to be set
  ||         whenever more than eight sprites appear on a scanline, but a
  ||         hardware bug causes the actual behavior to be more complicated
  ||         and generate false positives as well as false negatives; see
  ||         PPU sprite evaluation. This flag is set during sprite
  ||         evaluation and cleared at dot 1 (the second dot) of the
  ||         pre-render line.
  |+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps
  |          a nonzero background pixel; cleared at dot 1 of the pre-render
  |          line.  Used for raster timing.
  +--------- Vertical blank has started (0: not in vblank; 1: in vblank).
             Set at dot 1 of line 241 (the line *after* the post-render
             line); cleared after reading $2002 and at dot 1 of the
             pre-render line.
  ```
该寄存器的第7位标记vblank是否开始了，这个是PPU是否准备好接收数据的标志。  

BIT(Bit Test)
Z=A&M, N=M7,V=M6
bit指令用于检测指定内存地址位的情况，执行后会影响状态寄存器的下列标志位：  
- Z(Zero Flag): Set if the result if the AND is zero.
- V(Overflow Flag): Set to bit 6 of the memory value.
- N(Negative Flag): Set to bit 7 of the memory value.
当vertical blank开始时，$2002内存处的bit 7会被设置为1,执行BIT $(2002)后会引起状态寄存器的N标志位被设置为1。  

后面通过bpl指令来检测状态寄存器的N标志来进行跳转。  
BPL(Branch if Positive):
bpl指令的作用是当N标志位被清除，会将偏移量加到程序计数器寄存器(PC),触发跳转到新的位置。  

当vblank没开始时N Flag是0会触发跳转  
当vblank开始时N Flag是1不会触发跳转，程序继续执行下去。  

所以上段代码相当于检查到vblank是否开始，没有开始跳转回去继续执行检测。当vblank开始时跳出循环，往下执行代码。  

7.清空内存数据
```
	; clear all RAM to 0
	lda #0
	ldx #0
	:
		sta $0000, X
		sta $0100, X
		sta $0200, X
		sta $0300, X
		sta $0400, X
		sta $0500, X
		sta $0600, X
		sta $0700, X
		inx
		bne :-
```
这里使用到了一个新的寻址方式Absolute,X  
The address to be accessed by an instruction using X register indexed absolute addressing is computed by taking the 16 bit address from the instruction and added the contents of the X register. For example if X contains $92 then an STA $2000,X instruction will store the accumulator at $2092 (e.g. $2000 + $92).

X寄存器是一个8 bit寄存器，可以存储$00到$FF，之后再+1就会溢出了。
inx指令的作用是将X寄存器中的数进行+1操作，当为0时会设置状态寄存器的Z Flag，当7 bit被设置时状态寄存器的N Flag会被设置。  
%0000,0000->Zero Flag Seted.
%1000,0000->Negative Flag Seted.

