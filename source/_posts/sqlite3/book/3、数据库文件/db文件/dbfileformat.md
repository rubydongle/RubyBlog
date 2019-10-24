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

** Each btree pages is divided into three sections:  The header, the
** cell pointer array, and the cell content area.  Page 1 also has a 100-byte
** file header that occurs before the page header.
**
**      |----------------|
**      | file header    |   100 bytes.  Page 1 only.
**      |----------------|
**      | page header    |   8 bytes for leaves.  12 bytes for interior nodes
**      |----------------|
**      | cell pointer   |   |  2 bytes per cell.  Sorted order.
**      | array          |   |  Grows downward
**      |                |   v
**      |----------------|
**      | unallocated    |
**      | space          |
**      |----------------|   ^  Grows upwards
**      | cell content   |   |  Arbitrary order interspersed with freeblocks.
**      | area           |   |  and free space fragments.
**      |----------------|
**
** The page headers looks like this:
**
**   OFFSET   SIZE     DESCRIPTION
**      0       1      Flags. 1: intkey, 2: zerodata, 4: leafdata, 8: leaf
**      1       2      byte offset to the first freeblock
**      3       2      number of cells on this page
**      5       2      first byte of the cell content area
**      7       1      number of fragmented free bytes
**      8       4      Right child (the Ptr(N) value).  Omitted on leaves.


## b-tree page

B树页头包含6个部分
1.100个字节的数据库文件头(只有page1包含)
    The 100-byte database file header (found on page 1 only)
2.8个或12个字节的b-树页头。
    8 bytes for leaves.  12 bytes for interior nodes
    The 8 or 12 byte b-tree page header
    格式如下：

|偏移(Offset)|大小(Size)|描述|
|---|---|---|
|0|1|偏移为0的这一个字节描述该B树页类型 2(0x02)-->Interior索引B-树页  5(0x05)-->Interior表B-树页 10(0x0a)-->leaf索引B-树页 13(0x0d)-->leaf表B-树页|
|1|2|第一个FreeBlock的起始位置，如果没有FreeBlock就是0|
|3|2|该页上的Cell数目|
|5|2|指定了Cell内容区域的起始处|
|7|1|No. of fragmented free bytes|
|8|4|最右侧的指针，这个值只有Interior B-树页才有，在其他类型的页上忽略|

![img](/images/sqlitebook/db文件/rootpageparsed1.png)
![img](/images/sqlitebook/db文件/rootpageorigin1.png)

- 偏移0的一个字节是0d表明该页的类型是Leaf table
- 偏移1的两个字节为00 00表示没有FreeBlock
- 偏移3的2个字节为00 01表明该页上存储了1个Cell
- 偏移5的两个字节为0f c4,指定了Cell内容的其实位置在0f c4
![img](/images/sqlitebook/db文件/rootpagecellcontent1-1.png)
![img](/images/sqlitebook/db文件/rootpagecellcontent1-2.png)
- 偏移为7的1个字节为00 表明fragmented free bytes数为0
- 由于这是一个Leaf Table B-Tree类型的页，所以偏移为8的内容可以忽略。


3.
The cell pointer array
Unallocated space
The cell content area
The reserved region.
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

