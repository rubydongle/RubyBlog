---
title: arm64启动
categories:
  - 技术文章
date: 2019-11-29 10:19:11
tags:
---

## linux启动过程
入口在arch/arm64/kernel/head.S,启动定义了section _head
```c
/*
 * Kernel startup entry point.
 * ---------------------------
 *
 * The requirements are:
 *   MMU = off, D-cache = off, I-cache = on or off,
 *   x0 = physical address to the FDT blob.
 *
 * This code is mostly position independent so you call this at
 * __pa(PAGE_OFFSET + TEXT_OFFSET).
 *
 * Note that the callee-saved registers are used for storing variables
 * that are useful before the MMU is enabled. The allocations are described
 * in the entry routines.
 */
        __HEAD
_head:
        /*
         * DO NOT MODIFY. Image header expected by Linux boot-loaders.
         */
#ifdef CONFIG_EFI
        /*
         * This add instruction has no meaningful effect except that
         * its opcode forms the magic "MZ" signature required by UEFI.
         */
        add     x13, x18, #0x16
        b       stext
#else
        b       stext                           // branch to kernel start, magic
        .long   0                               // reserved
#endif
```
都是直接跳转到stext，stext也是这段代码中的代码
```c
        __INIT

        /*
         * The following callee saved general purpose registers are used on the
         * primary lowlevel boot path:
         *
         *  Register   Scope                      Purpose
         *  x21        stext() .. start_kernel()  FDT pointer passed at boot in x0
         *  x23        stext() .. start_kernel()  physical misalignment/KASLR offset
         *  x28        __create_page_tables()     callee preserved temp register
         *  x19/x20    __primary_switch()         callee preserved temp registers
         */
ENTRY(stext)
        bl      preserve_boot_args
        bl      el2_setup                       // Drop to EL1, w0=cpu_boot_mode
        adrp    x23, __PHYS_OFFSET
        and     x23, x23, MIN_KIMG_ALIGN - 1    // KASLR offset, defaults to 0
        bl      set_cpu_boot_mode_flag
        bl      __create_page_tables
        /*
         * The following calls CPU setup code, see arch/arm64/mm/proc.S for
         * details.
         * On return, the CPU will be ready for the MMU to be turned on and
         * the TCR will have been set.
         */
        bl      __cpu_setup                     // initialise processor
        b       __primary_switch
ENDPROC(stext)
```
stext中的代码包含下面几个部分
1. `preserve_boot_args`
  将bootloader设置的启动参数存储起来
2. `el2_setup`
3. `set_cpu_boot_mode_flag`
4. `__create_page_tables`
  创建页表
5. `__cpu_setup`
6. `__primary_switch`
  最终调用__primary_switched，并在其中执行start_kernel执行后面的c代码

## preserve_boot_args
通过bl跳转到preserve_boot_args，会将bootloader通过寄存器x0 .. x3寄存器传递过来的启动参数保存起来。
```c
/*      
 * Preserve the arguments passed by the bootloader in x0 .. x3
 */     
preserve_boot_args: 
        mov     x21, x0                         // x21=FDT

        adr_l   x0, boot_args                   // record the contents of
        stp     x21, x1, [x0]                   // x0 .. x3 at kernel entry
        stp     x2, x3, [x0, #16]

        dmb     sy                              // needed before dc ivac with
                                                // MMU off

        add     x1, x0, #0x20                   // 4 x 8 bytes 执行后x1是boot_args数组的最后一个元素的地址
        b       __inval_cache_range             // tail call
ENDPROC(preserve_boot_args)

```
设置后寄存器x21中为FDT扁平设备树
boot_args定义在arch/arm64/kernel/setup.c中,adr_l后，x0中就存储boot_args的地址了.
```c
/*
 * The recorded values of x0 .. x3 upon kernel entry.
 */
u64 __cacheline_aligned boot_args[4];
```
这样bootloader传递过来的启动参数就通过stp指令存储到boot_args了
dmb指令的作用如下

| 指令名 | 功能描述 |
|:---:|:---:|
|DMB | 数据存储器隔离。DMB 指令保证： 仅当所有在它前面的存储器访问操作都执行完毕后，才提交(commit)在它后面的存储器访问操作。|
|DSB | 数据同步隔离。比 DMB 严格： 仅当所有在它前面的存储器访问操作都执行完毕后，才执行在它后面的指令（亦即任何指令都要等待存储器访 问操作——译者注）|
|ISB | 指令同步隔离。最严格：它会清洗流水线，以保证所有它前面的指令都执行完毕之后，才执行它后面的指令。|

## el2_setup
在ARMv8中User,FIQ, IRQ, Abort, Undefined,System这些模式统统取消，被EL0,EL1,EL2,EL3四种特权特权模式取代(跟之前的特权模式/非特权模式相比，实际上特权级还是增加了的)。与x86的ring0-ring3刚好相反，在这四种模式中EL0是最低权限，一般给用户态程序用的。EL1稍高，一般给内核用的。EL2/EL3分别是Hypervisor(虚拟化相关)和secure monitor(安全相关)的模式，一般linux中是可以不用的。

![img](http://bimg.chinaaet.com/weiqi7777/blog/20170408/636272810004878729257270.png)
> EL0 ：  运行应用程序
> EL1 ：  运行操作系统
> EL2 ：  运行虚拟机
> EL3 ：  运行安全管理

参考:https://blog.csdn.net/lidan113lidan/article/details/48677735
参考:http://blog.chinaaet.com/weiqi7777/p/5100051412

## set_cpu_boot_mode_flag

## __create_page_tables
创建页表

## __cpu_setup

## __primary_switch


## 参考
在arm汇编中
> ENTRY:`ENTRY`伪指令用于指定汇编程序的入口点。在一个完整的汇编程序中至少要一个entry（也可以多个，当有多个entry时，程序的真正入口点由链接器指定），但在一源文件里最多只能有一个entry（可以没有）。
> ENDPROC:`ENDPROC`伪指令用于通知编译器已经到了源程序的结尾。

https://www.jianshu.com/p/2f4a5f74ac7a
ARM64经常用到的汇编指令:
```c
MOV    X1，X0         ;将寄存器X0的值传送到寄存器X1
ADD    X0，X1，X2     ;寄存器X1和X2的值相加后传送到X0
SUB    X0，X1，X2     ;寄存器X1和X2的值相减后传送到X0

AND    X0，X0，#0xF    ; X0的值与0xF相位与后的值传送到X0
ORR    X0，X0，#9      ; X0的值与9相位或后的值传送到X0
EOR    X0，X0，#0xF    ; X0的值与0xF相异或后的值传送到X0

LDR    X5，[X6，#0x08]        ；ld：load; X6寄存器加0x08的和的地址值内的数据传送到X5
LDP  x29, x30, [sp, #0x10]    ; ldp :load pair ; 一对寄存器, 从内存读取数据到寄存器

STR X0, [SP, #0x8]         ；st:store,str:往内存中写数据（偏移值为正）; X0寄存器的数据传送到SP+0x8地址值指向的存储空间
STUR   w0, [x29, #-0x8]   ;往内存中写数据（偏移值为负）
STP  x29, x30, [sp, #0x10]    ;store pair，存放一对数据, 入栈指令

CBZ  ;比较（Compare），如果结果为零（Zero）就转移（只能跳到后面的指令）
CBNZ ;比较，如果结果非零（Non Zero）就转移（只能跳到后面的指令）
CMP  ;比较指令，相当于SUBS，影响程序状态寄存器CPSR 

B/BL  ;绝对跳转#imm， 返回地址保存到LR（X30）
RET   ;子程序返回指令，返回地址默认保存在LR（X30）
```

https://www.cnblogs.com/LoyenWang/p/11483948.html
