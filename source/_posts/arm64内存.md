---
title: arm64内存
categories:
  - 技术文章
date: 2019-12-02 10:21:58
tags:
---

## 1.初始化mmu
```c
/*
 * Enable the MMU.
 *
 *  x0  = SCTLR_EL1 value for turning on the MMU.
 *  x1  = TTBR1_EL1 value for turning on the MMU if CONFIG_PG_DIR_RO=y.
 *
 * Returns to the caller via x30/lr. This requires the caller to be covered
 * by the .idmap.text section.
 *
 * Checks if the selected granule size is supported by the CPU.
 * If it isn't, park the CPU
 */
ENTRY(__enable_mmu)
#if defined(CONFIG_PG_DIR_RO)
        mrs     x2, ID_AA64MMFR0_EL1
        ubfx    x2, x2, #ID_AA64MMFR0_TGRAN_SHIFT, 4
#else
        mrs     x1, ID_AA64MMFR0_EL1
        ubfx    x2, x1, #ID_AA64MMFR0_TGRAN_SHIFT, 4
#endif
        cmp     x2, #ID_AA64MMFR0_TGRAN_SUPPORTED
        b.ne    __no_granule_support
#if defined(CONFIG_PG_DIR_RO)
        update_early_cpu_boot_status 0, x2, x3
        adrp    x2, idmap_pg_dir
        phys_to_ttbr x1, x1
        phys_to_ttbr x2, x2
        msr     ttbr0_el1, x2                   // load TTBR0
        msr     ttbr1_el1, x1                   // load TTBR1
#else
        update_early_cpu_boot_status 0, x1, x2
        adrp    x1, idmap_pg_dir
        adrp    x2, swapper_pg_dir
        msr     ttbr0_el1, x1                   // load TTBR0
        msr     ttbr1_el1, x2                   // load TTBR1
#endif
        isb
        msr     sctlr_el1, x0
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

aarch64 寄存器参考http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.100403_0200_00_en/ste1440662673049.html
linux kernel 寄存器头文件arch/arm64/include/asm/sysreg.h

### [寄存器SCTLR_EL1](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.100403_0200_00_en/ste1440662673049.html)
  `SCTLR_EL1`是一个对整个系统(包括内存系统)进行控制的寄存器,它是一个32位的寄存器，其它的系统可能叫做控制寄存器C.


### [寄存器TTBR1_EL1](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.100403_0200_00_en/lau1458646831483.html)

### [ID_AA64MMFR0_EL1(AArch64 Memory Model Feature Register 0,EL1)](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.100403_0200_00_en/lau1443437067853.html) 

### [TTBR0_EL1(Translation Table Base Register 0, EL1)](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.100442_0100_00_en/lau1443448587788.html)
