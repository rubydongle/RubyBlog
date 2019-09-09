---
title: Linux内存初始化
categories:
  - 技术文章
date: 2019-08-29 17:38:21
tags:
  - Linux
  - 内核
  - 内存
---

## 1.进程内存描述符struct mm_struct
一个进程的内存信息通过struct mm_struct描述和管理，我们称其为进程描述符
一个进程的虚拟地址空间主要由两个数据结来描述。
一个是最高层次的：mm_struct，一个是较高层次的：vm_area_structs。
最高层次的mm_struct结构描述了一个进程的整个虚拟地址空间。
较高层次的结构vm_area_truct描述了虚拟地址空间的一个区间（简称虚拟区）。
每个进程只有一个mm_struct结构，在每个进程的task_struct结构中，有一个指向该进程的结构。可以说，mm_struct结构是对整个用户空间的描述。

## 2.启动
Linux内核启动于init/main.c中的start_kernel函数，内存系统也启动于此。  
```
asmlinkage __visible void __init start_kernel(void)
{
        char *command_line;
        char *after_dashes;
......
/*
 * Interrupts are still disabled. Do necessary setups, then
 * enable them
 */
        boot_cpu_init();
        page_address_init();
        pr_notice("%s", linux_banner);
        setup_arch(&command_line);
        /*
         * Set up the the initial canary ASAP:
         */
        boot_init_stack_canary();
        mm_init_cpumask(&init_mm);
        setup_command_line(command_line);
......
        build_all_zonelists(NULL, NULL);
        page_alloc_init();
......
        /*
         * These use large bootmem allocations and must precede
         * kmem_cache_init()
         */
        setup_log_buf(0);
        pidhash_init();
        vfs_caches_init_early();
        sort_main_extable();
        trap_init();
        mm_init();
......
}
```
setup_arch函数中会调用bootmem_init来初始化启动内存：  
arch/arm64/kernel/setup.c  
```
void __init setup_arch(char **cmdline_p)
{
        pr_info("Boot CPU: AArch64 Processor [%08x]\n", read_cpuid_id());

        sprintf(init_utsname()->machine, UTS_MACHINE); // 初始化内核线程的内存描述符
        init_mm.start_code = (unsigned long) _text;
        init_mm.end_code   = (unsigned long) _etext;
        init_mm.end_data   = (unsigned long) _edata;
        init_mm.brk        = (unsigned long) _end;
......
        paging_init();  //初始化分页系统，主要是设置页表，初始化zone memory maps以及设置zero page。

        acpi_table_upgrade();

        /* Parse the ACPI tables for possible boot-time configuration */
        acpi_boot_table_init();

        if (acpi_disabled)
                unflatten_device_tree();

        bootmem_init();  //bootmem初始化
......
}

```
mm_init_cpumask(&init_mm)操作的参数init_mm是一个mm_struct结构。
init_mm是内核线程的mm_struct数据结构，或内核线程的映射，这是因为在这个映射中不包含任何用户空间的映射。
系统中所有的内核线程都共享同一个映射，那就是init_mm。特别的，内核线程init的mm_struct数据结构就是init_mm。而mm_struct结构中，则有个指针pgd指向它的首层页面目录。 
系统中只有一个init_mm结构对象，他在mm/init-mm.c中申明。
```
struct mm_struct init_mm = {
        .mm_rb          = RB_ROOT,
#ifdef CONFIG_SPECULATIVE_PAGE_FAULT
        .mm_rb_lock     = __RW_LOCK_UNLOCKED(init_mm.mm_rb_lock),
#endif
        .pgd            = swapper_pg_dir,
        .mm_users       = ATOMIC_INIT(2),
        .mm_count       = ATOMIC_INIT(1),
        .mmap_sem       = __RWSEM_INITIALIZER(init_mm.mmap_sem),
        .page_table_lock =  __SPIN_LOCK_UNLOCKED(init_mm.page_table_lock),
        .mmlist         = LIST_HEAD_INIT(init_mm.mmlist),
        .user_ns        = &init_user_ns,
        INIT_MM_CONTEXT(init_mm)
};
```
函数mm_init_cpumask是围绕这个结构体对象执行的,它清理内核线程的内存描述符中cpu_vm_mask_var字段，将其置零。  
```
static inline void mm_init_cpumask(struct mm_struct *mm)
{
#ifdef CONFIG_CPUMASK_OFFSTACK
        mm->cpu_vm_mask_var = &mm->cpumask_allocation;
#endif
        cpumask_clear(mm->cpu_vm_mask_var);
}
```

下面通过build_all_zonelists 构建zonelist。
为什么需要zonelist？
之前bootmem_init初始化的时候，已经初始化了内存节点的zone成员，该成员是一个struct zone数组，存放该内存节点的zone信息。  
对于uma系统来说，这已经够了，因为uma系统只有一个本地内存节点，所有zone的信息都存放在本地内存节点的zone成员中。  
对于numa系统来说，除了本地内存节点，还可以存在一个或多个远端内存节点，本地内存节点的zone成员并不会存放远端内存节点的zone信息。所以，这里引入zonelist的概念，本地内存节点和远端内存节点的zone统一挂在zonelist链表上。  
什么是pageset？
每一个cpu会维护一个boot_pageset成员，这是一个struct per_cpu_pageset结构体，用于描述每个cpu维护的page集合。该函数中只会对pageset做一些最简单的初始化
下面是build_all_zonelists的过程  
```
build_all_zonelists
   |----->set_zonelist_order();
   |   |----->current_zonelist_order = ZONELIST_ORDER_ZONE;
   |   |      对于uma系统来说，仅存在一个内存节点，所以按zone来排序;对于numa系统来说，可以按node来排序，也可以按zone来排序
   |----->build_all_zonelists_init();
   |   |----->__build_all_zonelists(NULL);
   |   |   |----->pg_data_t *pgdat = NODE_DATA(nid);
   |   |   |----->build_zonelists(pgdat);
   |   |   |   |----->zonelist = &pgdat->node_zonelists[ZONELIST_FALLBACK];
   |   |   |   |----->j = build_zonelists_node(pgdat, zonelist, 0);
   |   |   |   |      遍历该内存节点所有的zone，将所有的zone挂到该内存节点的node_zonelists上去
   |   |   |   |   |----->zoneref_set_zone(zone, &zonelist->_zonerefs[nr_zones++]);
   |   |   |   |   |----->return nr_zones;
   |   |   |   |   |     传进来的nr_zones加上挂到node_zonelists上的zone之后，返回
   |   |   |   |----->zonelist->_zonerefs[j].zone = NULL;
   |   |   |   |----->zonelist->_zonerefs[j].zone_idx = 0;
   |   |   |   |      标记node_zonelists上zone的结束
   |   |   |   |----->zonelist->_zonerefs[j].zone = NULL;
   |   |   |   |----->zonelist->_zonerefs[j].zone_idx = 0;
   |   |   |----->setup_pageset(&per_cpu(boot_pageset, cpu), 0);
   |   |   |      遍历每一个cpu，初始化其pageset
   |   |   |   |----->pageset_init(p);
   |   |   |   |   |----->pcp->count = 0;
   |   |   |   |   |      page数量初始化为0
   |   |   |   |   |----->INIT_LIST_HEAD(&pcp->lists[migratetype]);
   |   |   |   |   |      每一种迁移类型对于一个链表，这里初始化所有迁移类型对应的链表
   |   |   |   |----->pageset_set_batch(p, batch);
   |   |   |   |   |----->pageset_update(&p->pcp, 6 * batch, max(1UL, 1 * batch));
   |   |   |   |   |   |----->pcp->batch = 1;
   |   |   |   |   |   |      chunk size初始化为1
   |   |   |   |   |   |----->pcp->high = high;
   |   |   |   |   |   |      page上限初始化为0
   |   |   |   |   |   |----->pcp->batch = batch;
   |   |   |   |   |   |      chunk size初始化为1
```
接下来page_alloc_init
