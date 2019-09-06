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
arm64在debugfs中提供了文件...供调试打印出内核页表项。  
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
当通过cat输出...文件内容时我们可以看到通过ptdump_show输出的页表项的内容。  
arch/arm/include/asm/ptdump.h
```
struct ptdump_info {
        struct mm_struct                *mm;
        const struct addr_marker        *markers;
        unsigned long                   base_addr;
};
```




