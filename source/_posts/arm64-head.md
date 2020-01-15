---
title: arm64 head
categories:
  - 技术文章
date: 2019-12-10 11:16:25
tags:
---

## el2_setup
```c
/*
 * If we're fortunate enough to boot at EL2, ensure that the world is
 * sane before dropping to EL1.
 *
 * Returns either BOOT_CPU_MODE_EL1 or BOOT_CPU_MODE_EL2 in w0 if
 * booted in EL1 or EL2 respectively.
 */
ENTRY(el2_setup)
        msr     SPsel, #1                       // We want to use SP_EL{1,2}
        mrs     x0, CurrentEL
        cmp     x0, #CurrentEL_EL2
        b.eq    1f
        mov_q   x0, (SCTLR_EL1_RES1 | ENDIAN_SET_EL1)
        msr     sctlr_el1, x0
        mov     w0, #BOOT_CPU_MODE_EL1          // This cpu booted in EL1
        isb
        ret

1:      mov_q   x0, (SCTLR_EL2_RES1 | ENDIAN_SET_EL2)
        msr     sctlr_el2, x0
```
判断当前是否运行在EL2异常模式、是就跳转到1位置处.否则就返回了。
AArch64没有与ARMv7-A当前程序状态寄存器（CPSR）的直接等价物。在AArch64中，传统CPSR的组件作为可独立访问的字段提供。 这些统称为处理器状态（PSTATE）。 也有对PSTATE元素进行操作的指令。
AArch64的处理器状态或PSTATE字段具有以下定义：

|名字|	描述|
|---|---|
|N	|负数条件标志。
|Z	|零状态标志。
|C	|进位条件标志。
|V	|溢出条件标志。
|D	|调试屏蔽位。
|A	|SError屏蔽位。
|I	|IRQ屏蔽位。
|F	|FIQ屏蔽位。
|SS	|Software Step位。
|IL	|非法执行状态位。
|EL（2）|	异常级别。
|nRW	|执行状态0 = 64位1 = 32位
|SP	|堆栈指针选择器。 0 = SP_EL0 1 = SP_ELn

PSTATE字段使用专用寄存器访问。 可直接使用MRS指令读/写专用寄存器。
专用寄存器是：

|专用寄存器	|说明	|PSTATE字段
|---|---|---|
|CurrentEL	|存放当前的异常级别。	|EL
|DAIF	|指定当前的中断屏蔽位。	|D，A，I，F
|NZCV	|保持条件标志。	|N，Z，C，V
|SPSel	|在EL1或更高版本中，在当前异常级别的SP和SP_EL0之间进行选择。	|SP
例如，要访问SPSel使用如下代码
```c
MRS X0, SPSel //把SPSel读到X0
MSR SPSel, X0 //把X0写入SPSel
```
操作SPSel寄存器用来设置堆栈指针选择器,0 = SP_EL0 1 = SP_ELn, `el2_setup`将其设置为1
下面通过指令`mrs x0, CurrentEL`读取了当前异常级别的寄存器读取放在x0寄存器中。
```c
        msr     SPsel, #1                       // We want to use SP_EL{1,2}
        mrs     x0, CurrentEL
        cmp     x0, #CurrentEL_EL2
        b.eq    1
```
如果当前运行模式不是EL2，那么就是运行在EL1模式下，此时直接ret
```c
        mov_q   x0, (SCTLR_EL1_RES1 | ENDIAN_SET_EL1)
        msr     sctlr_el1, x0
        mov     w0, #BOOT_CPU_MODE_EL1          // This cpu booted in EL1
        isb
        ret
```
系统控制寄存器（SCTLR）用于控制标准内存和系统设备，并为在硬件内核中实现的功能提供状态信息。
设置该寄存器为SCTLR_EL1_RES1 | ENDIAN_SET_EL1,并设置w0寄存器为BOOT_CPU_MODE_EL1
ISB指令是指令同步隔离。最严格：它会清洗流水线，以保证所有它前面的指令都执行完毕之后，才执行它后面的指令。

不分析el2模式的setup代码

## set_cpu_boot_mode_flag
```c
/*
 * Sets the __boot_cpu_mode flag depending on the CPU boot mode passed
 * in w0. See arch/arm64/include/asm/virt.h for more info.
 */
set_cpu_boot_mode_flag:
        adr_l   x1, __boot_cpu_mode
        cmp     w0, #BOOT_CPU_MODE_EL2
        b.ne    1f
        add     x1, x1, #4
1:      str     w0, [x1]                        // This CPU has booted in EL1
        dmb     sy
        dc      ivac, x1                        // Invalidate potentially stale cache line
        ret
ENDPROC(set_cpu_boot_mode_flag)
```
__boot_cpu_mode在文件arch/arm64/include/asm/virt.h中

```c
/*
 * __boot_cpu_mode records what mode CPUs were booted in.
 * A correctly-implemented bootloader must start all CPUs in the same mode:
 * In this case, both 32bit halves of __boot_cpu_mode will contain the
 * same value (either 0 if booted in EL1, BOOT_CPU_MODE_EL2 if booted in EL2).
 *
 * Should the bootloader fail to do this, the two values will be different.
 * This allows the kernel to flag an error when the secondaries have come up.
 */
extern u32 __boot_cpu_mode[2];
```
在上一步el2_setup中w0寄存器中存储的内容是BOOT_CPU_MODE_EL1.此时b.ne 1f会跳转到标记1处执行。
即将w0寄存器中的内容写入x1寄存器所指向的内存处，也就是__boot_cpu_mode被设置为BOOT_CPU_MODE_EL1.
```c
1:      str     w0, [x1]                        // This CPU has booted in EL1
        dmb     sy
        dc      ivac, x1                        // Invalidate potentially stale cache line
        ret
```
dmb, dc指令保证数据写入到内存完成。之后通过ret指令返回.

DC CVAU, Xn
将data cache中数据真正刷入内存；

## __create_page_tables
http://gngshn.github.io/2017/11/30/arm64-linux%E5%90%AF%E5%8A%A8%E6%B5%81%E7%A8%8B%E5%88%86%E6%9E%9005-%E9%85%8D%E7%BD%AE%E5%86%85%E6%A0%B8%E5%90%AF%E5%8A%A8%E7%9A%84%E4%B8%B4%E6%97%B6%E9%A1%B5%E8%A1%A8/
http://www.wowotech.net/armv8a_arch/turn-on-mmu.html

当没有打开MMU的时候，cpu在进行取指以及数据访问的时候是直接访问物理内存或者IO memory。虽然64bit的CPU理论上拥有非常大的address space，但是实际上用于存储kernel image的物理main memory并没有那么大，一般而言，系统的main memory在低端的一小段物理地址空间中，如上图右侧的图片所示。当打开MMU的时候，cpu对memory系统的访问不能直接触及物理空间，而是需要通过一系列的Translation table进行翻译。虚拟地址空间分成三段，低端是0x00000000_00000000～0x0000FFFF_FFFFFFFF，用于user space。高端是0xFFFF0000_00000000～0xFFFFFFFF_FFFFFFFF，用于kernel space。中间的一段地址是无效地址，对其访问会产生MMU fault。虚拟地址空间如上图右侧的图片所示。

Linker感知的是虚拟地址，在将内核的各个object文件链接成一个kernel image的时候，kernel image binary code中访问的都是虚拟地址，也就是说kernel image应该运行在Linker指定的虚拟地址空间上。问题来了，kernel image运行在那个地址上呢？实际上，将kernel image放到kernel space的首地址运行是一个最直观的想法，不过由于各种原因，具体的arch在编译内核的时候，可以指定一个offset（TEXT_OFFSET），对于ARM64而言是512KB（0x00080000），因此，编译后的内核运行在0xFFFF8000_00080000的地址上。系统启动后，bootloader会将kernel image copy到main memory，当然，和虚拟地址空间类似，kernel image并没有copy到main memory的首地址，也保持了一个同样size的offset。现在，问题又来了：在kernel的开始运行阶段，MMU是OFF的，也就是说kernel image是直接运行在物理地址上的，但是实际上kernel是被linker链接到了虚拟地址上去的，在这种情况下，在没有turn on MMU之前，kernel能正常运行吗？可以的，如果kernel在turn on MMU之前的代码都是PIC的，那么代码实际上是可以在任意地址上运行的。你可以仔细观察turn on MMU之前的代码，都是位置无关的代码。

解决了MMU turn on之前的问题，现在我们可以准备“穿越”了。真正打开MMU就是一条指令而已，就是将某个system register的某个bit设定为1之类的操作。这样我们可以把相关指令分成两组，turn on mmu之前的绿色指令和之后的橘色指令，如下图所示：
![img](http://www.wowotech.net/content/uploadfile/201510/34e38e720d5cc7776246207c6cdc92bd20151024043506.gif)

由于现代CPU的设计引入了pipe， super scalar，out-of-order execution，分支预测等等特性，实际上在turn on MMU的指令执行的那个时刻，该指令附近的指令的具体状态有些混乱，可能绿色指令执行的数据加载在实际在总线上发起bus transaction的时候已经启动了MMU，本来它是应该访问physical address space的。而也有可能橘色的指令提前执行，导致其发起的memory操作在MMU turn on之前就完成。为了解决这些混乱，可以采取一种投机取巧的办法，就是建立一致性映射：假设kernel image对应的物理地址段是A～B这一段，那么在建立页表的时候就把A～B这段虚拟地址段映射到A～B这一段的物理地址。这样，在turn on MMU附近的指令是毫无压力的，无论你是通过虚拟地址还是物理地址，访问的都是同样的物理memory。

还有一种方法，就是清楚的隔离turn on MMU前后的指令，那就是使用指令同步工具，如下：
![img](http://www.wowotech.net/content/uploadfile/201510/1814f128c7e1cf94cacff66e8824056220151024043507.gif)


指令屏障可以清清楚楚的把指令的执行划分成三段，第一段是绿色指令，在执行turn on mmu指令执行之前全部完成，随后启动turn on MMU的指令，随后的指令屏障可以确保turn on MMU的指令完全执行完毕（整个计算机系统的视图切换到了虚拟世界），这时候才启动橘色指令的取指、译码、执行等操作。


在创建页表使能MMU之前，程序都是直接访问内存地址的，要使用MMU就必须首先设置页表
设置初始化页表
xzr和wzr是零寄存器,wzr是32位的xzr是64位的。
```c
/*
 * Setup the initial page tables. We only setup the barest amount which is
 * required to get the kernel running. The following sections are required:
 *   - identity mapping to enable the MMU (low address, TTBR0)
 *   - first few MB of the kernel linear mapping to jump to once the MMU has
 *     been enabled
 */
__create_page_tables:
        mov     x28, lr		// LR通常称X30为程序链接寄存器，保存子程序结束后需要执行的下一条指令

        /*
         * Invalidate the init page tables to avoid potential dirty cache lines
         * being evicted. Other page tables are allocated in rodata as part of
         * the kernel image, and thus are clean to the PoC per the boot
         * protocol.
         */
        adrp    x0, init_pg_dir		// 页表起始位置
        adrp    x1, init_pg_end		// 页表结束位置
        sub     x1, x1, x0		// 页表大小记录到x1寄存器中
        bl      __inval_dcache_area	// 注释1

        /*
         * Clear the init page tables. 清除init页表
         */
        adrp    x0, init_pg_dir
        adrp    x1, init_pg_end
        sub     x1, x1, x0
1:      stp     xzr, xzr, [x0], #16
        stp     xzr, xzr, [x0], #16
        stp     xzr, xzr, [x0], #16
        stp     xzr, xzr, [x0], #16
        subs    x1, x1, #64
        b.ne    1b			// 清除完毕

```
原始的init页表清除干净后，就要开始初始化了
```c
        mov     x7, SWAPPER_MM_MMUFLAGS

        /*
         * Create the identity mapping.
         */
        adrp    x0, idmap_pg_dir		// 页全局目录
        adrp    x3, __idmap_text_start          // __pa(__idmap_text_start)

#ifdef CONFIG_ARM64_USER_VA_BITS_52
        mrs_s   x6, SYS_ID_AA64MMFR2_EL1
        and     x6, x6, #(0xf << ID_AA64MMFR2_LVA_SHIFT)
        mov     x5, #52
        cbnz    x6, 1f
#endif
        mov     x5, #VA_BITS
1:
        adr_l   x6, vabits_user
        str     x5, [x6]
        dmb     sy
        dc      ivac, x6                // Invalidate potentially stale cache line

        /*
         * VA_BITS may be too small to allow for an ID mapping to be created
         * that covers system RAM if that is located sufficiently high in the
         * physical address space. So for the ID map, use an extended virtual
         * range in that case, and configure an additional translation level
         * if needed.
         *
         * Calculate the maximum allowed value for TCR_EL1.T0SZ so that the
         * entire ID map region can be mapped. As T0SZ == (64 - #bits used),
         * this number conveniently equals the number of leading zeroes in
         * the physical address of __idmap_text_end.
         */
        adrp    x5, __idmap_text_end
        clz     x5, x5
        cmp     x5, TCR_T0SZ(VA_BITS)   // default T0SZ small enough?
        b.ge    1f                      // .. then skip VA range extension

        adr_l   x6, idmap_t0sz
        str     x5, [x6]
        dmb     sy
        dc      ivac, x6                // Invalidate potentially stale cache line
```

注释1:
    函数__inval_dcache_area申明在arch/arm64/include/asm/cacheflush.h,实现在arch/arm64/mm/cache.S如下:
    ```c
    cacheflush.h
    extern void __inval_dcache_area(void *addr, size_t len);

    cache.S
    /*
 *      __inval_dcache_area(kaddr, size)
 *
 *      Ensure that any D-cache lines for the interval [kaddr, kaddr+size)
 *      are invalidated. Any partial lines at the ends of the interval are
 *      also cleaned to PoC to prevent data loss.
 *
 *      - kaddr   - kernel address
 *      - size    - size in question
 */
ENTRY(__inval_dcache_area)
        /* FALLTHROUGH */

/*
 *      __dma_inv_area(start, size)
 *      - start   - virtual start address of region
 *      - size    - size in question
 */
__dma_inv_area:
        add     x1, x1, x0
        dcache_line_size x2, x3
        sub     x3, x2, #1
        tst     x1, x3                          // end cache line aligned?
        bic     x1, x1, x3
        b.eq    1f
        dc      civac, x1                       // clean & invalidate D / U line
1:      tst     x0, x3                          // start cache line aligned?
        bic     x0, x0, x3
        b.eq    2f
        dc      civac, x0                       // clean & invalidate D / U line
        b       3f
2:      dc      ivac, x0                        // invalidate D / U line
3:      add     x0, x0, x2
        cmp     x0, x1
        b.lo    2b
        dsb     sy
        ret
ENDPIPROC(__inval_dcache_area)
ENDPROC(__dma_inv_area)
```
__inval_dcache_area函数有两个参数,就是调用时寄存器x0,x1的内容。

## __cpu_setup 
该调用在arch/arm64/mm/proc.S中
函数主要开启MMU
```c
/*
 *      __cpu_setup
 *
 *      Initialise the processor for turning the MMU on.  Return in x0 the
 *      value of the SCTLR_EL1 register.
 */
        .pushsection ".idmap.text", "awx"
ENTRY(__cpu_setup)
        tlbi    vmalle1                         // Invalidate local TLB
        dsb     nsh

        mov     x0, #3 << 20
        msr     cpacr_el1, x0                   // Enable FP/ASIMD
        mov     x0, #1 << 12                    // Reset mdscr_el1 and disable
        msr     mdscr_el1, x0                   // access to the DCC from EL0
        isb                                     // Unmask debug exceptions now,
        enable_dbg                              // since this is per-cpu
        reset_pmuserenr_el0 x0                  // Disable PMU access from EL0
        /*
         * Memory region attributes for LPAE:
         *
         *   n = AttrIndx[2:0]
         *                      n       MAIR
         *   DEVICE_nGnRnE      000     00000000
         *   DEVICE_nGnRE       001     00000100
         *   DEVICE_GRE         010     00001100
         *   NORMAL_NC          011     01000100
         *   NORMAL             100     11111111
         *   NORMAL_WT          101     10111011
         */
        ldr     x5, =MAIR(0x00, MT_DEVICE_nGnRnE) | \
                     MAIR(0x04, MT_DEVICE_nGnRE) | \
                     MAIR(0x0c, MT_DEVICE_GRE) | \
                     MAIR(0x44, MT_NORMAL_NC) | \
                     MAIR(0xff, MT_NORMAL) | \
                     MAIR(0xbb, MT_NORMAL_WT)
        msr     mair_el1, x5
        /*
         * Prepare SCTLR
         */
        mov_q   x0, SCTLR_EL1_SET
        /*
         * Set/prepare TCR and TTBR. We use 512GB (39-bit) address range for
         * both user and kernel.
         */
        ldr     x10, =TCR_TxSZ(VA_BITS) | TCR_CACHE_FLAGS | TCR_SMP_FLAGS | \
                        TCR_TG_FLAGS | TCR_KASLR_FLAGS | TCR_ASID16 | \
                        TCR_TBI0 | TCR_A1 | TCR_KASAN_FLAGS
        tcr_clear_errata_bits x10, x9, x5

#ifdef CONFIG_ARM64_USER_VA_BITS_52
        ldr_l           x9, vabits_user
        sub             x9, xzr, x9
        add             x9, x9, #64
#else
        ldr_l           x9, idmap_t0sz
#endif
        tcr_set_t0sz    x10, x9

        /*
         * Set the IPS bits in TCR_EL1.
         */
        tcr_compute_pa_size x10, #TCR_IPS_SHIFT, x5, x6
#ifdef CONFIG_ARM64_HW_AFDBM
        /*
         * Enable hardware update of the Access Flags bit.
         * Hardware dirty bit management is enabled later,
         * via capabilities.
         */
        mrs     x9, ID_AA64MMFR1_EL1
        and     x9, x9, #0xf
        cbz     x9, 1f
        orr     x10, x10, #TCR_HA               // hardware Access flag update
1:
#endif  /* CONFIG_ARM64_HW_AFDBM */
        msr     tcr_el1, x10
        ret                                     // return to head.S
ENDPROC(__cpu_setup)
```

## __primary_switch
```c
__primary_switch:
#ifdef CONFIG_RANDOMIZE_BASE
        mov     x19, x0                         // preserve new SCTLR_EL1 value
        mrs     x20, sctlr_el1                  // preserve old SCTLR_EL1 value
#endif

        adrp    x1, init_pg_dir
        bl      __enable_mmu			// 使能mmu
#ifdef CONFIG_RELOCATABLE
        bl      __relocate_kernel
#ifdef CONFIG_RANDOMIZE_BASE
        ldr     x8, =__primary_switched
        adrp    x0, __PHYS_OFFSET
        blr     x8

        /*
         * If we return here, we have a KASLR displacement in x23 which we need
         * to take into account by discarding the current kernel mapping and
         * creating a new one.
         */
        pre_disable_mmu_workaround
        msr     sctlr_el1, x20                  // disable the MMU
        isb
        bl      __create_page_tables            // recreate kernel mapping

        tlbi    vmalle1                         // Remove any stale TLB entries
        dsb     nsh

        msr     sctlr_el1, x19                  // re-enable the MMU
        isb
        ic      iallu                           // flush instructions fetched
        dsb     nsh                             // via old mapping
        isb

        bl      __relocate_kernel
#endif
#endif
        ldr     x8, =__primary_switched
        adrp    x0, __PHYS_OFFSET
        br      x8
ENDPROC(__primary_switch)

```
具体打开MMU的代码在__enable_mmu函数中如下：
```c
/*
 * Enable the MMU.
 *
 *  x0  = SCTLR_EL1 value for turning on the MMU.
 *  x1  = TTBR1_EL1 value
 *
 * Returns to the caller via x30/lr. This requires the caller to be covered
 * by the .idmap.text section.
 *
 * Checks if the selected granule size is supported by the CPU.
 * If it isn't, park the CPU
 */
ENTRY(__enable_mmu)
        mrs     x2, ID_AA64MMFR0_EL1
        ubfx    x2, x2, #ID_AA64MMFR0_TGRAN_SHIFT, 4
        cmp     x2, #ID_AA64MMFR0_TGRAN_SUPPORTED
        b.ne    __no_granule_support
        update_early_cpu_boot_status 0, x2, x3
        adrp    x2, idmap_pg_dir
        phys_to_ttbr x1, x1
        phys_to_ttbr x2, x2
        msr     ttbr0_el1, x2                   // load TTBR0	-> idmap_pg_dir
        offset_ttbr1 x1
        msr     ttbr1_el1, x1                   // load TTBR1	-> init_pg_dir
        isb
        msr     sctlr_el1, x0			// 开启MMU
        isb
        /*
         * Invalidate the local I-cache so that any instructions fetched
         * speculatively from the PoC are discarded, since they may have
         * been dynamically patched at the PoU.
         */
        ic      iallu
        dsb     nsh
        isb
        ret
ENDPROC(__enable_mmu)
```
有两个协处理器寄存器用来存放一级页表基地址，TTBR0和TTBR1.操作系统把虚拟内存划分为内核空间和用户空间，TTBR0存放用户空间的一级页表基址，`TTBR1`存放内核空间的一级页表基址。
TTBR(Translation Table Base Register)
In this model, the virtual address space is divided into two regions:
- 0x0 -> 1<<(32-N) that TTBR0 controls
- 1<<(32-N) -> 4GB that TTBR1 controls.
N的大小由TTBCR寄存器决定。0x0 -> 1<<(32-N)为用户空间，由TTBR0控制，1<<(32-N) -> 4GB为内核空间，由TTBR1控制。
N的大小与一级页表大小的关系图如下：
![img](https://images0.cnblogs.com/i/513816/201408/170030589833174.png)
操作系统为用户空间的每个进程分配各自的页表，即每个进程的一级页表基址是不一样的，故当发生进程上下文切换时，TTBR0需要被存放当前进程的一级页表基址；TTBR1中存放的是内核空间的一级页表基址，内核空间的一级页表基址是固定的，故TTBR1中的基址值不需要改变。



## 参考
[TTBR0](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0433c/CIHFDBEJ.html)
[https://www.jianshu.com/p/ba548d03c357](https://www.jianshu.com/p/ba548d03c357)
[系统控制寄存器SCTLR](https://www.jianshu.com/p/885913b7201c)
https://developer.arm.com/docs/den0024/latest/armv8-registers/processor-state
[ARM指令之精髓DMB,DSB,ISB指令](http://www.voidcn.com/article/p-ailxnscc-yo.html)
[ADRP指令 ](https://www.jianshu.com/p/e5452c97cfbd)
[dc指令](https://www.cnblogs.com/smartjourneys/p/6841090.html)
[MMU地址映射过程详述](https://www.cnblogs.com/tanghuimin0713/p/3917178.html)


