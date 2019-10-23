---
title: sqlite
categories:
  - 技术文章
date: 2019-10-16 11:06:50
tags:
---

# 数据库文件格式
[Database File Format](https://www.sqlite.org/draft/fileformat2.html)
SQLite数据库内容通常存放在磁盘上称作"main database file"的单个文件中。
```bash
root@batman:~/sqlitedatabasestudy# sqlite3 my.db
SQLite version 3.11.0 2016-02-15 17:29:24
Enter ".help" for usage hints.
sqlite> .header on
sqlite> CREATE TABLE info(name TEXT, age INT);
sqlite> .tables
info
sqlite> INSERT INTO info (name, age) VALUES ('ruby', 25);
sqlite> SELECT * FROM info;
name|age
ruby|25
sqlite> .exit

root@batman:~/sqlitedatabasestudy# ls -alh
total 4.0K
drwxrwxrwx 0 root root 4.0K Oct 16 11:22 .
drwxrwxrwx 0 root root 4.0K Oct 16 11:15 ..
-rwxrwxrwx 1 root root 2.0K Oct 16 11:25 my.db
```
这个创建的my.db文件就是主数据库文件。
在使用*事务*特性时，SQLite会存储额外的信息到一个称作*回滚日志*(rollback journal)的文件中。当SQLite运行在预写式日志WAL模式(Write-Ahead Logging)时，是一个write-ahead log文件。

事务操作：
- 回滚日志模式
```
root@NJVV-11048100:/mnt/f/sqlitedatabasestudy# sqlite3 my.db
SQLite version 3.11.0 2016-02-15 17:29:24
Enter ".help" for usage hints.
sqlite> BEGIN;
sqlite> INSERT INTO info (name, age) VALUES('lily', 21)
   ...> ;
sqlite> INSERT INTO info (name, age) VALUES('lei', 24);
sqlite> COMMIT;
sqlite> .header on
sqlite> SELECT * FROM info;
name|age
ruby|25
lily|21
lei|24
```
在BEGIN命令后COMMIT命令之前会生成一个回滚日志文件,当COMMIT后就会删除
进入数据库再次启动一个事务，可以看到回滚日志的内容如下：
```bash
root@batman:~/sqlitedatabasestudy# ls
my.db  my.db-journal

root@batman:~/sqlitedatabasestudy# hexdump -C my.db-journal
00000000  00 00 00 00 00 00 00 00  00 00 00 00 b5 b8 d8 c5  |................|
00000010  00 00 00 02 00 00 02 00  00 00 04 00 00 00 00 00  |................|
00000020  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000200  00 00 00 02 0d 00 00 00  03 03 e3 00 03 f6 03 ec  |................|
00000210  03 e3 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000220  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
000005e0  00 00 00 00 00 00 00 07  03 03 13 01 6c 65 69 18  |............lei.|
000005f0  08 02 03 15 01 6c 69 6c  79 15 08 01 03 15 01 72  |.....lily......r|
00000600  75 62 79 19 b5 b8 d8 c5                           |uby.....|
00000608
```
- 预写式日志模式(WAL)
日志模式通过PRAGMA设置  
[sqlite-pragma](https://www.runoob.com/sqlite/sqlite-pragma.html)
```bash
root@batman:~/sqlitedatabasestudy# sqlite3 my-wal.db
SQLite version 3.11.0 2016-02-15 17:29:24
Enter ".help" for usage hints.
sqlite> PRAGMA journal_mode = WAL;
wal
sqlite> BEGIN;
sqlite> INSERT INTO info (name, age) VALUES('ruby', 30);

root@batman:~/sqlitedatabasestudy# ls -alh
total 40K
drwxrwxrwx 0 root root 4.0K Oct 16 14:06 .
drwxrwxrwx 0 root root 4.0K Oct 16 11:15 ..
-rwxrwxrwx 1 root root 2.0K Oct 16 13:13 my.db
-rwxrwxrwx 1 root root 2.0K Oct 16 14:05 my-wal.db
-rwxrwxrwx 1 root root  32K Oct 16 14:06 my-wal.db-shm
-rwxrwxrwx 1 root root    0 Oct 16 14:06 my-wal.db-wal

root@batman:~/sqlitedatabasestudy# hexdump -C my-wal.db
00000000  53 51 4c 69 74 65 20 66  6f 72 6d 61 74 20 33 00  |SQLite format 3.|
00000010  04 00 02 02 00 40 20 20  00 00 00 02 00 00 00 02  |.....@  ........|
00000020  00 00 00 00 00 00 00 00  00 00 00 01 00 00 00 04  |................|
00000030  00 00 00 00 00 00 00 00  00 00 00 01 00 00 00 00  |................|
00000040  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000050  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 02  |................|
00000060  00 2d f1 b8 0d 00 00 00  01 03 c5 00 03 c5 00 00  |.-..............|
00000070  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
000003c0  00 00 00 00 00 39 01 06  17 15 15 01 57 74 61 62  |.....9......Wtab|
000003d0  6c 65 69 6e 66 6f 69 6e  66 6f 02 43 52 45 41 54  |leinfoinfo.CREAT|
000003e0  45 20 54 41 42 4c 45 20  69 6e 66 6f 28 6e 61 6d  |E TABLE info(nam|
000003f0  65 20 54 45 58 54 2c 20  61 67 65 20 49 4e 54 29  |e TEXT, age INT)|
00000400  0d 00 00 00 01 03 f6 00  03 f6 00 00 00 00 00 00  |................|
00000410  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
000007f0  00 00 00 00 00 00 08 01  03 15 01 6a 6f 6e 65 0d  |...........jone.|
00000800
root@batman:~/sqlitedatabasestudy# hexdump -C my-wal.db-shm
00000000  18 e2 2d 00 00 00 00 00  00 00 00 00 01 00 00 00  |..-.............|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020  00 00 00 00 00 00 00 00  38 07 18 06 35 93 db 09  |........8...5...|
00000030  18 e2 2d 00 00 00 00 00  00 00 00 00 01 00 00 00  |..-.............|
00000040  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000050  00 00 00 00 00 00 00 00  38 07 18 06 35 93 db 09  |........8...5...|
00000060  00 00 00 00 00 00 00 00  ff ff ff ff ff ff ff ff  |................|
00000070  ff ff ff ff ff ff ff ff  00 00 00 00 00 00 00 00  |................|
00000080  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00008000
root@batman:~/sqlitedatabasestudy# hexdump -C my-wal.db-wal

```

1. 页(Pages)
主数据库文件由一个或多个页(pages)组成，

2. 数据库头(The Database Header)
前100个字节是数据库的文件头
```
root@batman:~/sqlitedatabasestudy# hexdump -C -n 100 my.db
00000000  53 51 4c 69 74 65 20 66  6f 72 6d 61 74 20 33 00  |SQLite format 3.|
00000010  04 00 02 02 00 40 20 20  00 00 00 04 00 00 00 02  |.....@  ........|
00000020  00 00 00 00 00 00 00 00  00 00 00 01 00 00 00 04  |................|
00000030  00 00 00 00 00 00 00 00  00 00 00 01 00 00 00 00  |................|
00000040  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000050  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 04  |................|
00000060  00 2d f1 b8                                       |.-..|
00000064
```

3.回滚日志
4.Write-Ahead Log
