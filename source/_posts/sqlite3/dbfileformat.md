---
title: dbfileformat
categories:
  - 技术文章
date: 2019-10-20 09:15:21
tags:
---


B树结构
```c
	
struct Btree {
  sqlite3 *db;
  BtShared *pBt;  // 这棵btree可共享的内容
  u8 inTrans;
  u8 sharable;
  u8 locked;
  u8 hasIncrblobCur;
  int wantToLock;
  int nBackup;
  u32 iDataVersion;
  Btree *pNext;
  Btree *pPrev;
#ifndef SQLITE_OMIT_SHARED_CACHE
  BtLock lock;
#endif
};
```

```c
struct BtShared {
  Pager *pPager;
  sqlite3 *db;
  BtCursor *pCursor;
  
}
```


摘自https://sqlite.org/src/artifact/91806f01fd1145a9



# 页
https://sqlite.org/fileformat2.html#pages
页的编号从1开始，最大页号是2147483646 (231 - 2)
主数据库中的每一个页只做下面的一个作用：
- The lock-byte page
- A freelist page
    - A freelist trunk page
    - A freelist leaf page
- A b-tree page
    - A table b-tree interior page
    - A table b-tree leaf page
    - An index b-tree interior page
    - An index b-tree leaf page
- A payload overflow page
- A pointer map page


## b-tree page
在数据库头的偏移52位置
52	4	The page number of the largest root b-tree page when in auto-vacuum or incremental-vacuum modes, or zero otherwise.

A b-tree page is divided into regions in the following order:

The 100-byte database file header (found on page 1 only)
The 8 or 12 byte b-tree page header
The cell pointer array
Unallocated space
The cell content area
The reserved region.

磁盘相关、文件系统相关
https://sqlite.org/c3ref/io_methods.html
xSectorSize()方法返回设备的扇区大小。
针对unix api其实现在src/os_unix.c中
通过系统调用fstatvfs实现的
man fstatvfs
```c
int statvfs(const char *path, struct statvfs *buf);
int fstatvfs(int fd, struct statvfs *buf);

struct statvfs {
    unsigned long f_bsize;	/* Filesystem block size */
    unsigned long f_frsize;
    fsblkcnt_t f_blocks;
}

```

