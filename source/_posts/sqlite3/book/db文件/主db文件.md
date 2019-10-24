

# 最简单的数据库文件
我们通过sqlite命令创建一个数据库，这个数据库中创建一个表info，并插入一条记录。
```sql
ruby@batman:~$ sqlite3 first.db
SQLite version 3.22.0 2018-01-22 18:45:57
Enter ".help" for usage hints.
sqlite> CREATE TABLE info (name TEXT, age INT);
sqlite> INSERT INTO info VALUES ('james', 23);
sqlite> .header on
sqlite> SELECT * FROM info;
name|age
james|23
sqlite> .quit

ruby@batman:~$ hexdump -C first.db
00000000  53 51 4c 69 74 65 20 66  6f 72 6d 61 74 20 33 00  |SQLite format 3.|
00000010  10 00 01 01 00 40 20 20  00 00 00 02 00 00 00 02  |.....@  ........|
00000020  00 00 00 00 00 00 00 00  00 00 00 01 00 00 00 04  |................|
00000030  00 00 00 00 00 00 00 00  00 00 00 01 00 00 00 00  |................|
00000040  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000050  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 02  |................|
00000060  00 2e 1c b0 0d 00 00 00  01 0f c4 00 0f c4 00 00  |................|
00000070  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000fc0  00 00 00 00 3a 01 06 17  15 15 01 59 74 61 62 6c  |....:......Ytabl|
00000fd0  65 69 6e 66 6f 69 6e 66  6f 02 43 52 45 41 54 45  |einfoinfo.CREATE|
00000fe0  20 54 41 42 4c 45 20 69  6e 66 6f 20 28 6e 61 6d  | TABLE info (nam|
00000ff0  65 20 54 45 58 54 2c 20  61 67 65 20 49 4e 54 29  |e TEXT, age INT)|
00001000  0d 00 00 00 01 0f f5 00  0f f5 00 00 00 00 00 00  |................|
00001010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00001ff0  00 00 00 00 00 09 01 03  17 01 6a 61 6d 65 73 17  |..........james.|
00002000

ruby@batman:~$ ls -alh first.db 
-rw-r--r-- 1 ruby ruby 8.0K 10月 24 00:34 first.db
```
这个最简单的数据库有2^13个字节即8kb大小,通过这个最简单的数据库来看下内容的存储。
### db文件的前100个字节存数据库头
```
00000000  53 51 4c 69 74 65 20 66  6f 72 6d 61 74 20 33 00  |SQLite format 3.|
00000010  10 00 01 01 00 40 20 20  00 00 00 02 00 00 00 02  |.....@  ........|
00000020  00 00 00 00 00 00 00 00  00 00 00 01 00 00 00 04  |................|
00000030  00 00 00 00 00 00 00 00  00 00 00 01 00 00 00 00  |................|
00000040  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000050  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 02  |................|
00000060  00 2e 1c b0                                       |....|
00000064
```
数据库文件头的定义如下：

|偏移(Offset)|大小(Size)|描述(Description)|
|---|---|---|
|0	|16	|头字符串"SQLite format 3\000" The header string: "SQLite format 3\000"
|16	|2	|数据库页大小，单位byte.必须是512~32768之间2^xxx,如果是1表示页大小为65536
|18	|1	|File format write version. 1 for legacy; 2 for WAL.
|19	|1	|File format read version. 1 for legacy; 2 for WAL.
|20	|1	|Bytes of unused "reserved" space at the end of each page. Usually 0.
|21	|1	|Maximum embedded payload fraction. Must be 64.
|22	|1	|Minimum embedded payload fraction. Must be 32.
|23	|1	|Leaf payload fraction. Must be 32.
|24	|4	|File change counter.
|28	|4	|Size of the database file in pages. The "in-header database size".
|32	|4	|Page number of the first freelist trunk page.
|36	|4	|Total number of freelist pages.
|40	|4	|The schema cookie.
|44	|4	|The schema format number. Supported schema formats are 1, 2, 3, and 4.
|48	|4	|Default page cache size.
|52	|4	|The page number of the largest root b-tree page when in auto-vacuum or incremental-vacuum modes, or zero otherwise.
|56	|4	|The database text encoding. A value of 1 means UTF-8. A value of 2 means UTF-16le. A value of 3 means UTF-16be.
|60	|4	|The "user version" as read and set by the user_version pragma.
|64	|4	|True (non-zero) for incremental-vacuum mode. False (zero) otherwise.
|68	|4	|通过命令PRAGMA application_id设置的"Application ID"
|72	|20	|保留用作后续扩展，必须是0.
|92	|4	|The version-valid-for number.
|96	|4	|SQLite 版本号


