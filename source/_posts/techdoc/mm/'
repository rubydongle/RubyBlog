---
title: arm64页表
categories:
  - 技术文章
date: 2019-09-05 17:38:21
tags:
  - Linux
  - 内核
  - 内存
---

## dump arm64内核页表项
arm64在debugfs中提供了文件/sys/debug/kernel/mm/kernel_page_tables供调试打印出内核页表项。  
arch/arm64/mm/ptdump_debugfs.c
```
#include <linux/debugfs.h>                  
#include <linux/seq_file.h>     
                       
#include <asm/ptdump.h>     
                                                       
static int ptdump_show(struct seq_file *m, void *v)    
{                                         
        struct ptdump_info *info = m->private;      
        ptdump_walk_pgd(m, info);    
        return 0;                                                       
}                    
DEFINE_SHOW_ATTRIBUTE(ptdump);    
     
void ptdump_debugfs_register(struct ptdump_info *info, const char *name)    
{                                                                  
        debugfs_create_file(name, 0400, NULL, info, &ptdump_fops);    
} 
```
当通过cat输出kernel_page_tables文件内容时我们可以看到通过ptdump_show输出的页表项的内容。  
arch/arm/include/asm/ptdump.h
```
struct ptdump_info {
        struct mm_struct                *mm;
        const struct addr_marker        *markers;
        unsigned long                   base_addr;
};
```

kernel_page_tables文件是在arch/arm64/mm/dump.c文件中创建的。  
```
static struct ptdump_info kernel_ptdump_info = {
        .mm             = &init_mm,
        .markers        = address_markers,
        .base_addr      = VA_START,
};

static int ptdump_init(void)
{
        ptdump_initialize();
        ptdump_debugfs_register(&kernel_ptdump_info, "kernel_page_tables");
        return 0;
}
device_initcall(ptdump_init);

```
传入的ptdump_info指向的内存描述符是内核进程的init_mm.  
init_mm是在mm/init-mm.c中定义的。  
```
/*      
 * For dynamically allocated mm_structs, there is a dynamically sized cpumask
 * at the end of the structure, the size of which depends on the maximum CPU
 * number the system can see. That way we allocate only as much memory for
 * mm_cpumask() as needed for the hundreds, or thousands of processes that
 * a system typically runs.
 *
 * Since there is only one init_mm in the entire system, keep it simple
 * and size this cpu_bitmask to NR_CPUS.   
 */
struct mm_struct init_mm = {
        .mm_rb          = RB_ROOT,
        .pgd            = swapper_pg_dir,
        .mm_users       = ATOMIC_INIT(2),
        .mm_count       = ATOMIC_INIT(1),
        .mmap_sem       = __RWSEM_INITIALIZER(init_mm.mmap_sem),
        .page_table_lock =  __SPIN_LOCK_UNLOCKED(init_mm.page_table_lock),
        .arg_lock       =  __SPIN_LOCK_UNLOCKED(init_mm.arg_lock),
        .mmlist         = LIST_HEAD_INIT(init_mm.mmlist),
        .user_ns        = &init_user_ns,
        .cpu_bitmap     = { [BITS_TO_LONGS(NR_CPUS)] = 0},
        INIT_MM_CONTEXT(init_mm)
};
```
可以看到内核进程的页全局目录指向swapper_pg_dir。  
arm64上swapper_pg_dir所在的位置在编译链接脚本文件中就指定了,并且占用一个page页的内存。  
arch/arm64/kernel/vmlinux.lds.S  
```
SECTIONS
{
        
......
        .text : {                       /* Real text segment            */
                _stext = .;             /* Text and read-only data      */
                        __exception_text_start = .;
                        *(.exception.text)
                        __exception_text_end = .;
                        IRQENTRY_TEXT
                        SOFTIRQENTRY_TEXT
                        ENTRY_TEXT
                        TEXT_TEXT
                        SCHED_TEXT
                        CPUIDLE_TEXT
                        LOCK_TEXT
                        KPROBES_TEXT
                        HYPERVISOR_TEXT
                        IDMAP_TEXT
                        HIBERNATE_TEXT
                        TRAMP_TEXT
                        *(.fixup)
                        *(.gnu.warning)
                . = ALIGN(16);
                *(.got)                 /* Global offset table          */
        }

        . = ALIGN(SEGMENT_ALIGN);
        _etext = .;                     /* End of text section */

        RO_DATA(PAGE_SIZE)              /* everything from this point to     */
        EXCEPTION_TABLE(8)              /* __init_begin will be marked RO NX */
        NOTES

        . = ALIGN(PAGE_SIZE);
        idmap_pg_dir = .;
        . += IDMAP_DIR_SIZE;

......
        swapper_pg_dir = .;
        . += PAGE_SIZE;
        swapper_pg_end = .;

        . = ALIGN(SEGMENT_ALIGN);
        __init_begin = .;
        __inittext_begin = .;

        INIT_TEXT_SECTION(8)
        .exit.text : {
                ARM_EXIT_KEEP(EXIT_TEXT)
        }
```
dump相当于从内存swapper_pg_dir起步，一层层进入获取内核进程页表的内容。  
进入walk_pgd的代码，我们可以看到其相当于，读取每个页全局目录项，再进去walk pud，查看每个页上级目录。  
当某个页全局目录项没有内容时候，直接通过note_page打印信息。  
```
static void walk_pgd(struct pg_state *st, struct mm_struct *mm,
                     unsigned long start)
{
        unsigned long end = (start < TASK_SIZE_64) ? TASK_SIZE_64 : 0;
        unsigned long next, addr = start;
        pgd_t *pgdp = pgd_offset(mm, start);

        do {
                pgd_t pgd = READ_ONCE(*pgdp);
                next = pgd_addr_end(addr, end);

                if (pgd_none(pgd)) {
                        note_page(st, addr, 1, pgd_val(pgd));
                } else {
                        BUG_ON(pgd_bad(pgd));
                        walk_pud(st, pgdp, addr, next);
                }
        } while (pgdp++, addr = next, addr != end);
}

```
遍历页上级目录和遍历页全局目录项一样，也是再次进去通过walk_pmd()遍历里面的每一个页中间目录。
页中间目录中存储的是页表项了，遍历页中间目录中通过walk_pte遍历每个页表项的内容。  
```
static void walk_pte(struct pg_state *st, pmd_t *pmdp, unsigned long start,
                     unsigned long end)
{
        unsigned long addr = start;
        pte_t *ptep = pte_offset_kernel(pmdp, start);

        do {
                note_page(st, addr, 4, READ_ONCE(pte_val(*ptep)));
        } while (ptep++, addr += PAGE_SIZE, addr != end);
}
```
通过note_page将页表项里面的内容输出。  






### 参考文章：  
[https://www.cnblogs.com/pengdonglin137/p/9157639.html](https://www.cnblogs.com/pengdonglin137/p/9157639.html)
